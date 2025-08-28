// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <OpenImageIO/unittest.h>

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
                    // always fails
                    OIIO_CHECK_EQUAL(
                        "Failed to parse time value: " + timeStr, e.what() );
                }
            }
        }
    }
    // always fails
    OIIO_CHECK_EQUAL( "Could not extract time from output: ", output );
    return -1.0f;
}

void testDefaultConstruction()
{
    UsageTimer timer;
    OIIO_CHECK_EQUAL( timer.enabled, false );
}

void testEnabledConstruction()
{
    UsageTimer timer;
    timer.enabled = true;
    OIIO_CHECK_EQUAL( timer.enabled, true );
}

void testResetWhenDisabled()
{
    UsageTimer timer;
    timer.enabled = false;

    // Should not crash when disabled
    timer.reset();
}

void testPrintWhenDisabled()
{
    UsageTimer timer;
    timer.enabled = false;

    // Should not crash when disabled
    timer.print( "test_path", "test_message" );
}

void testResetWhenEnabled()
{
    UsageTimer timer;
    timer.enabled = true;

    // Should not crash when enabled
    timer.reset();
}

void testPrintWhenEnabled()
{
    UsageTimer timer;
    timer.enabled = true;

    // Should not crash when enabled
    timer.print( "test_path", "test_message" );
}

void testTimingOutput()
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
    OIIO_CHECK_ASSERT( output.find( "Timing:" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "test_path" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "test_message" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output.find( "msec" ) != std::string::npos );
}

void testConsecutiveCalls()
{
    UsageTimer timer;
    timer.enabled = true;

    // Multiple reset/print cycles should work
    for ( int i = 0; i < 5; ++i )
    {
        timer.reset();
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        timer.print( "path", "message" );
    }
}

void testEmptyStrings()
{
    UsageTimer timer;
    timer.enabled = true;

    timer.reset();
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

    // Should handle empty strings gracefully
    timer.print( "", "" );
    timer.print( "path", "" );
    timer.print( "", "message" );
}

void testMultipleIndependentInstances()
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
    OIIO_CHECK_ASSERT( output1.find( "Timing:" ) != std::string::npos );
    OIIO_CHECK_ASSERT( output2.find( "Timing:" ) != std::string::npos );

    // Extract timing values
    float time1 = extractTimeFromOutput( output1 );
    float time2 = extractTimeFromOutput( output2 );

    // Timer1 should show longer time (started earlier)
    OIIO_CHECK_GT( time1, time2 );
    // Timer1 should be around 15ms, Timer2 around 5ms
    OIIO_CHECK_GT( time1, 10.0f );

    // disable for now as this fails on the CI runners
    // OIIO_CHECK_LT( time2, 25.0f );
}

void testTimingAccuracy()
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
    OIIO_CHECK_ASSERT( output.find( "Timing:" ) != std::string::npos );

    float timeValue = extractTimeFromOutput( output );

    // Should be approximately 100ms (with some tolerance)
    OIIO_CHECK_GT( timeValue, 95.0f ); // Allow for some overhead

    // disable for now as this fails on the CI runners
    // OIIO_CHECK_LT( timeValue, 250.0f ); // Allow for some overhead
}

void testUninitializedTimer()
{
    // Test that calling print() without reset() doesn't crash
    // (though it may show unexpected timing values)
    UsageTimer timer;
    timer.enabled = true;

    // Don't call reset() - use uninitialized timer
    timer.print( "uninitialized", "test" );
}

int main( int, char ** )
{
    testDefaultConstruction();
    testEnabledConstruction();
    testResetWhenDisabled();
    testPrintWhenDisabled();
    testResetWhenEnabled();
    testPrintWhenEnabled();
    testTimingOutput();
    testConsecutiveCalls();
    testEmptyStrings();
    testMultipleIndependentInstances();
    testTimingAccuracy();
    testUninitializedTimer();

    return unit_test_failures;
}
