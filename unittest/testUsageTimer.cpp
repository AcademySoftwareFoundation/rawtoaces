// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#define BOOST_TEST_MODULE UsageTimerTest
#include <boost/test/unit_test.hpp>
#include <rawtoaces/usage_timer.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using namespace rta::util;

// Helper function to extract time value from output
float extractTimeFromOutput( const std::string &output )
{
    // The output format is: "Timing: path/message: 105.012msec"
    // Find the last colon and extract the time value
    size_t lastColon = output.rfind( ": " );
    if ( lastColon != std::string::npos )
    {
        size_t timeStart = lastColon + 2;
        size_t timeEnd   = output.find( "msec", timeStart );
        if ( timeStart != std::string::npos && timeEnd != std::string::npos )
        {
            std::string timeStr =
                output.substr( timeStart, timeEnd - timeStart );
            // Trim whitespace
            timeStr.erase( 0, timeStr.find_first_not_of( " \t" ) );
            timeStr.erase( timeStr.find_last_not_of( " \t" ) + 1 );

            if ( !timeStr.empty() )
            {
                try
                {
                    return std::stof( timeStr );
                }
                catch ( const std::exception &e )
                {
                    BOOST_FAIL(
                        "Failed to parse time value: " << timeStr << " - "
                                                       << e.what() );
                }
            }
        }
    }
    BOOST_FAIL( "Could not extract time from output: " << output );
    return -1.0f;
}

BOOST_AUTO_TEST_SUITE( UsageTimerTestSuite )

BOOST_AUTO_TEST_CASE( TestDefaultConstruction )
{
    UsageTimer timer;
    BOOST_CHECK_EQUAL( timer.enabled, false );
}

BOOST_AUTO_TEST_CASE( TestEnabledConstruction )
{
    UsageTimer timer;
    timer.enabled = true;
    BOOST_CHECK_EQUAL( timer.enabled, true );
}

BOOST_AUTO_TEST_CASE( TestResetWhenDisabled )
{
    UsageTimer timer;
    timer.enabled = false;

    // Should not crash when disabled
    BOOST_CHECK_NO_THROW( timer.reset() );
}

BOOST_AUTO_TEST_CASE( TestPrintWhenDisabled )
{
    UsageTimer timer;
    timer.enabled = false;

    // Should not crash when disabled
    BOOST_CHECK_NO_THROW( timer.print( "test_path", "test_message" ) );
}

BOOST_AUTO_TEST_CASE( TestResetWhenEnabled )
{
    UsageTimer timer;
    timer.enabled = true;

    // Should not crash when enabled
    BOOST_CHECK_NO_THROW( timer.reset() );
}

BOOST_AUTO_TEST_CASE( TestPrintWhenEnabled )
{
    UsageTimer timer;
    timer.enabled = true;

    // Should not crash when enabled
    BOOST_CHECK_NO_THROW( timer.print( "test_path", "test_message" ) );
}

BOOST_AUTO_TEST_CASE( TestTimingOutput )
{
    UsageTimer timer;
    timer.enabled = true;

    timer.reset();

    // Sleep for a known duration
    std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

    // Capture output to verify timing
    std::stringstream buffer;
    std::streambuf   *old = std::cerr.rdbuf( buffer.rdbuf() );

    timer.print( "test_path", "test_message" );

    std::cerr.rdbuf( old );
    std::string output = buffer.str();

    // Should contain timing information
    BOOST_CHECK( output.find( "Timing:" ) != std::string::npos );
    BOOST_CHECK( output.find( "test_path" ) != std::string::npos );
    BOOST_CHECK( output.find( "test_message" ) != std::string::npos );
    BOOST_CHECK( output.find( "msec" ) != std::string::npos );
}

BOOST_AUTO_TEST_CASE( TestConsecutiveCalls )
{
    UsageTimer timer;
    timer.enabled = true;

    // Multiple reset/print cycles should work
    for ( int i = 0; i < 5; ++i )
    {
        BOOST_CHECK_NO_THROW( timer.reset() );
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        BOOST_CHECK_NO_THROW( timer.print( "path", "message" ) );
    }
}

BOOST_AUTO_TEST_CASE( TestEmptyStrings )
{
    UsageTimer timer;
    timer.enabled = true;

    timer.reset();
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

    // Should handle empty strings gracefully
    BOOST_CHECK_NO_THROW( timer.print( "", "" ) );
    BOOST_CHECK_NO_THROW( timer.print( "path", "" ) );
    BOOST_CHECK_NO_THROW( timer.print( "", "message" ) );
}

BOOST_AUTO_TEST_CASE( TestMultipleIndependentInstances )
{
    // This test verifies that multiple instances work independently
    UsageTimer timer1;
    UsageTimer timer2;

    timer1.enabled = true;
    timer2.enabled = true;

    // Start timer1 first
    timer1.reset();
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

    // Start timer2 later
    timer2.reset();
    std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );

    // Capture outputs to verify different timing
    std::stringstream buffer1, buffer2;
    std::streambuf   *old = std::cerr.rdbuf( buffer1.rdbuf() );

    timer1.print( "path1", "message1" );
    std::cerr.rdbuf( buffer2.rdbuf() );
    timer2.print( "path2", "message2" );

    std::cerr.rdbuf( old );

    std::string output1 = buffer1.str();
    std::string output2 = buffer2.str();

    // Both should contain timing information
    BOOST_CHECK( output1.find( "Timing:" ) != std::string::npos );
    BOOST_CHECK( output2.find( "Timing:" ) != std::string::npos );

    // Extract timing values
    float time1 = extractTimeFromOutput( output1 );
    float time2 = extractTimeFromOutput( output2 );

    // Timer1 should show longer time (started earlier)
    BOOST_CHECK_GT( time1, time2 );
    // Timer1 should be around 15ms, Timer2 around 5ms
    BOOST_CHECK_GT( time1, 10.0f );
    BOOST_CHECK_LT( time2, 10.0f );
}

BOOST_AUTO_TEST_CASE( TestTimingAccuracy )
{
    UsageTimer timer;
    timer.enabled = true;

    timer.reset();

    // Sleep for a known duration
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Capture output to verify timing
    std::stringstream buffer;
    std::streambuf   *old = std::cerr.rdbuf( buffer.rdbuf() );

    timer.print( "test_path", "test_message" );

    std::cerr.rdbuf( old );
    std::string output = buffer.str();

    // Should contain timing information
    BOOST_CHECK( output.find( "Timing:" ) != std::string::npos );

    float timeValue = extractTimeFromOutput( output );

    // Should be approximately 100ms (with some tolerance)
    BOOST_CHECK_GT( timeValue, 95.0f );  // Allow for some overhead
    BOOST_CHECK_LT( timeValue, 110.0f ); // Allow for some overhead
}

BOOST_AUTO_TEST_CASE( TestUninitializedTimer )
{
    // Test that calling print() without reset() doesn't crash
    // (though it may show unexpected timing values)
    UsageTimer timer;
    timer.enabled = true;

    // Don't call reset() - use uninitialized timer
    BOOST_CHECK_NO_THROW( timer.print( "uninitialized", "test" ) );
}

BOOST_AUTO_TEST_SUITE_END()
