// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include "../src/rawtoaces_util/cache_base.h"
#include "../src/rawtoaces_util/transform_cache.h"

#include "test_utils.h"
#include <OpenImageIO/unittest.h>

template <class Descriptor, class Data>
std::shared_ptr<const Data> fetch(
    rta::cache::Cache<Descriptor, Data> &cache,
    const Descriptor                    &descriptor,
    const Data                          &in_value,
    const bool                           in_success = true )
{
    return cache.fetch( descriptor, [&]() {
        if ( !in_success )
        {
            return (std::shared_ptr<const Data>)nullptr;
        }
        auto result = std::make_shared<const Data>( in_value );
        return result;
    } );
}

template <class Descriptor, class Data>
std::shared_ptr<const Data> fetch(
    rta::cache::Cache<Descriptor, Data> &cache,
    const Descriptor                    &descriptor,
    const Data                          &in_value,
    std::string                         &out_message,
    const bool                           in_success = true )
{
    std::shared_ptr<const Data> result;

    out_message = capture_stderr(
        [&]() { result = fetch( cache, descriptor, in_value, in_success ); } );

    return result;
}

void testCache_disabled()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;
    cache.disabled  = true;

    std::string key = "missing";
    std::string message;
    auto        output = fetch( cache, key, 42, message );

    OIIO_CHECK_ASSERT( output );
    OIIO_CHECK_EQUAL( *output.get(), 42 );
    ASSERT_CONTAINS( message, "Cache (cache_name): disabled." );
}

void testCache_missing()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;

    std::string key = "missing";
    std::string message;
    auto        output = fetch( cache, key, 42, message );

    OIIO_CHECK_ASSERT( output );
    OIIO_CHECK_EQUAL( *output.get(), 42 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [missing].",
        "Cache (cache_name): not found. Calculating a new entry."
    };
    ASSERT_CONTAINS_ALL( message, expected_output );
}

void testCache_present()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;

    std::string key = "present";
    std::string message;
    fetch( cache, key, 42, message );
    auto output = fetch( cache, key, -1, message );

    OIIO_CHECK_ASSERT( output );
    OIIO_CHECK_EQUAL( *output.get(), 42 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [present].",
        "Cache (cache_name): found in cache!"
    };
    ASSERT_CONTAINS_ALL( message, expected_output );
}

void testCache_failed()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;

    std::string key = "missing";
    std::string message;
    auto        output = fetch( cache, key, 42, message, false );

    OIIO_CHECK_ASSERT( !output );
}

void testCache_full()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;
    cache.capacity  = 3;

    // Insert N+1 entries into an N-element cache.
    std::string message;
    fetch( cache, std::string( "101" ), 101, message );
    fetch( cache, std::string( "102" ), 102, message );
    fetch( cache, std::string( "103" ), 103, message );
    fetch( cache, std::string( "104" ), 104, message );

    // Confirm that the oldest entry has been removed.
    auto output = fetch( cache, std::string( "101" ), -1, message );

    OIIO_CHECK_ASSERT( output );
    OIIO_CHECK_EQUAL( *output.get(), -1 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [101].",
        "Cache (cache_name): not found. Calculating a new entry."
    };
    ASSERT_CONTAINS_ALL( message, expected_output );
}

void testCache_bump()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;
    cache.capacity  = 3;

    // Insert N entries into an N-element cache.
    std::string message;
    fetch( cache, std::string( "101" ), 101, message );
    fetch( cache, std::string( "102" ), 102, message );
    fetch( cache, std::string( "103" ), 103, message );

    // Bump the oldest entry to move it to the top.
    fetch( cache, std::string( "101" ), -1, message );

    // Insert another entry.
    fetch( cache, std::string( "104" ), 104, message );

    // Confirm that the bumped entry has not been removed.
    auto output = fetch( cache, std::string( "101" ), -2, message );

    OIIO_CHECK_ASSERT( output );
    OIIO_CHECK_EQUAL( *output.get(), 101 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [101].",
        "Cache (cache_name): found in cache!"
    };
    ASSERT_CONTAINS_ALL( message, expected_output );
}

void test_cache_threading()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    // Spawn 10000 threads requesting a cache entry for 20 different keys
    // 500 times each. The default cache capacity is 10 elements, so
    // there should be enough collisions and recalculations to trigger a crash
    // if something is modified in an unsafe manner. Also make the cached object
    // an array instead of a single number to further increase chances of a
    // corrupted object.

    constexpr size_t thread_count = 10000;
    constexpr size_t keys_count   = 20;
    constexpr size_t array_size   = 100;

    rta::cache::Cache<size_t, std::array<size_t, array_size>> threaded_cache(
        "threaded cache" );

    auto worker = [&]( size_t index ) -> void {
        size_t key = index % keys_count;

        std::array<size_t, 100> in_value;

        for ( size_t i = 0; i < 100; i++ )
            in_value[i] = key;

        auto output = fetch( threaded_cache, key, in_value, true );

        OIIO_CHECK_ASSERT( output );
        for ( size_t i = 0; i < 100; i++ )
            OIIO_CHECK_EQUAL( ( *output.get() )[i], key );
    };

    std::vector<std::thread> threads;

    for ( size_t index = 0; index < thread_count; index++ )
        threads.emplace_back( worker, index );

    for ( size_t index = 0; index < thread_count; index++ )
        threads[index].join();
}

// The print helper operators are only reacheable in the namespace 'rta'
using namespace rta;

void testCache_print_helpers()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    {
        std::tuple<std::string, std::string, std::string> tuple = { "a",
                                                                    "b",
                                                                    "c" };
        std::stringstream                                 stream;
        stream << tuple << std::endl;
        ASSERT_CONTAINS( stream.str(), "a, b, c" );
    }

    {
        std::tuple<std::string, std::string, std::array<double, 3>> tuple = {
            "a", "b", { 1.1, 2.2, 3.3 }
        };
        std::stringstream stream;
        stream << tuple << std::endl;
        ASSERT_CONTAINS( stream.str(), "a, b, (1.1, 2.2, 3.3)" );
    }

    {
        rta::core::Metadata metadata;
        std::stringstream   stream;
        stream << metadata << std::endl;
        ASSERT_CONTAINS( stream.str(), "<Metadata>" );
    }
}

void testCache_metadata_comparison()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    rta::core::Metadata metadata1 = {
        {
            { 11,
              { 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0 },
              { 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0 } },
            { 21,
              { 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0 },
              { 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0 } },
        },
        { 1.0, 2.0, 3.0 },
        4.0
    };

    std::stringstream stream;
    stream << metadata1 << std::endl;
    ASSERT_CONTAINS( stream.str(), "<Metadata>" );

    rta::core::Metadata metadata2 = metadata1;
    OIIO_CHECK_EQUAL( metadata1, metadata2 );

    metadata2                   = metadata1;
    metadata2.baseline_exposure = 5.0;
    OIIO_CHECK_ASSERT( !( metadata1 == metadata2 ) );

    metadata2                = metadata1;
    metadata2.neutral_RGB[1] = 11.0;
    OIIO_CHECK_ASSERT( !( metadata1 == metadata2 ) );

    metadata2                           = metadata1;
    metadata2.calibration[1].illuminant = 31;
    OIIO_CHECK_ASSERT( !( metadata1 == metadata2 ) );

    metadata2                                             = metadata1;
    metadata2.calibration[1].camera_calibration_matrix[3] = 55.0;
    OIIO_CHECK_ASSERT( !( metadata1 == metadata2 ) );

    metadata2                                     = metadata1;
    metadata2.calibration[1].XYZ_to_RGB_matrix[3] = 55.0;
    OIIO_CHECK_ASSERT( !( metadata1 == metadata2 ) );
}

void testCache_transform_caches()
{
    std::cout << std::endl << __FUNCTION__ << std::endl;

    OIIO_CHECK_EQUAL(
        rta::cache::get_WB_from_illuminant_cache().name, "WB from illuminant" );
    OIIO_CHECK_EQUAL(
        rta::cache::get_illuminant_from_WB_cache().name, "illuminant from WB" );
    OIIO_CHECK_EQUAL(
        rta::cache::get_matrix_from_illuminant_cache().name,
        "matrix from illuminant" );
    OIIO_CHECK_EQUAL(
        rta::cache::get_matrix_from_dng_metadata_cache().name,
        "matrix from DNG metadata" );

    try
    {
        // Invoke the constructor and destructor to get full function coverage.
        rta::cache::Cache<
            rta::cache::CameraAndIlluminantDescriptor,
            rta::cache::WBFromIlluminantData>
            cache1;
        rta::cache::Cache<
            rta::cache::CameraAndIlluminantDescriptor,
            rta::cache::MatrixData>
            cache2;
        rta::cache::Cache<
            rta::cache::CameraAndWBDescriptor,
            rta::cache::IlluminantAndWBData>
            cache3;
        rta::cache::
            Cache<rta::cache::MetadataDescriptor, rta::cache::MatrixData>
                cache4;
    }
    catch ( const std::exception &e )
    {
        // always fails
        OIIO_CHECK_EQUAL( "Unexpected exception: ", std::string( e.what() ) );
    }
}

int main( int, char ** )
{
    testCache_disabled();
    testCache_missing();
    testCache_present();
    testCache_failed();
    testCache_full();
    testCache_bump();
    testCache_print_helpers();
    testCache_metadata_comparison();
    testCache_transform_caches();
    test_cache_threading();

    return unit_test_failures;
}
