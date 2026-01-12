// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include "test_utils.h"
#include <OpenImageIO/unittest.h>

#include "../src/rawtoaces_util/cache_base.h"
#include "../src/rawtoaces_util/transform_cache.h"

using Descriptor = std::string;
using Data       = int;

std::string fetch(
    rta::cache::Cache<Descriptor, Data> &cache,
    const Descriptor                    &descriptor,
    const Data                          &in_value,
    Data                                &out_value,
    bool                                &out_success,
    const bool                           in_success = true )
{
    return capture_stderr( [&]() {
        auto result = cache.fetch( descriptor, [&]( Data &data ) {
            data = in_value;
            return in_success;
        } );

        out_success = result.first;
        out_value   = result.second;
    } );
}

void testCache_disabled()
{
    rta::cache::Cache<Descriptor, Data> cache( "cache_name" );
    cache.verbosity = 1;
    cache.disabled  = true;

    int         value;
    bool        success;
    std::string output = fetch( cache, "missing", 42, value, success );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( value, 42 );
    ASSERT_CONTAINS( output, "Cache (cache_name): disabled." );
}

void testCache_missing()
{
    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;

    int         value;
    bool        success;
    std::string output = fetch( cache, "missing", 42, value, success );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( value, 42 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [missing].",
        "Cache (cache_name): not found. Calculating a new entry."
    };
    ASSERT_CONTAINS_ALL( output, expected_output );
}

void testCache_present()
{
    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;

    int  value;
    bool success;
    fetch( cache, "present", 42, value, success );
    std::string output = fetch( cache, "present", -1, value, success );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( value, 42 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [present].",
        "Cache (cache_name): found in cache!"
    };
    ASSERT_CONTAINS_ALL( output, expected_output );
}

void testCache_failed()
{
    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;

    int  value;
    bool success;
    fetch( cache, "missing", 42, value, success, false );

    OIIO_CHECK_ASSERT( !success );
}

void testCache_full()
{
    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;
    cache.capacity  = 3;

    // Insert N+1 entries into an N-element cache.
    int  value;
    bool success;
    fetch( cache, "101", 101, value, success );
    fetch( cache, "102", 102, value, success );
    fetch( cache, "103", 103, value, success );
    fetch( cache, "104", 104, value, success );

    // Confirm that the oldest entry has been removed.
    std::string output = fetch( cache, "101", -1, value, success );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( value, -1 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [101].",
        "Cache (cache_name): not found. Calculating a new entry."
    };
    ASSERT_CONTAINS_ALL( output, expected_output );
}

void testCache_bump()
{
    rta::cache::Cache<std::string, int> cache( "cache_name" );
    cache.verbosity = 1;
    cache.capacity  = 3;

    // Insert N entries into an N-element cache.
    int  value;
    bool success;
    fetch( cache, "101", 101, value, success );
    fetch( cache, "102", 102, value, success );
    fetch( cache, "103", 103, value, success );

    // Bump the oldest entry to move it to the top.
    fetch( cache, "101", -1, value, success );

    // Insert another entry.
    fetch( cache, "104", 104, value, success );

    // Confirm that the bumped entry has not been removed.
    std::string output = fetch( cache, "101", -2, value, success );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( value, 101 );

    // Assert on the expected error message
    std::vector<std::string> expected_output = {
        "Cache (cache_name): searching for an entry [101].",
        "Cache (cache_name): found in cache!"
    };
    ASSERT_CONTAINS_ALL( output, expected_output );
}

// The print helper operators are only reacheable in the namespace 'rta'
using namespace rta;

void testCache_print_helpers()
{
    std::string output = capture_stderr( [&]() {
        std::tuple<std::string, std::string, std::string> tuple = { "a",
                                                                    "b",
                                                                    "c" };
        std::cerr << tuple << std::endl;
    } );
    ASSERT_CONTAINS( output, "a, b, c" );

    output = capture_stderr( [&]() {
        std::tuple<std::string, std::string, std::array<double, 3>> tuple = {
            "a", "b", { 1.1, 2.2, 3.3 }
        };
        std::cerr << tuple << std::endl;
    } );
    ASSERT_CONTAINS( output, "a, b, (1.1, 2.2, 3.3)" );

    rta::core::Metadata metadata;
    output = capture_stderr( [&]() { std::cerr << metadata << std::endl; } );
    ASSERT_CONTAINS( output, "<Metadata>" );
}

void testCache_metadata_comparison()
{
    std::string output = capture_stderr( [&]() {
        std::tuple<std::string, std::string, std::string> tuple = { "a",
                                                                    "b",
                                                                    "c" };
        std::cerr << tuple << std::endl;
    } );
    ASSERT_CONTAINS( output, "a, b, c" );

    output = capture_stderr( [&]() {
        std::tuple<std::string, std::string, std::array<double, 3>> tuple = {
            "a", "b", { 1.1, 2.2, 3.3 }
        };
        std::cerr << tuple << std::endl;
    } );
    ASSERT_CONTAINS( output, "a, b, (1.1, 2.2, 3.3)" );

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

    output = capture_stderr( [&]() { std::cerr << metadata1 << std::endl; } );
    ASSERT_CONTAINS( output, "<Metadata>" );

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
    OIIO_CHECK_EQUAL(
        rta::cache::WB_from_illuminant_cache.name, "WB from illuminant" );
    OIIO_CHECK_EQUAL(
        rta::cache::illuminant_from_WB_cache.name, "illuminant from WB" );
    OIIO_CHECK_EQUAL(
        rta::cache::matrix_from_illuminant_cache.name,
        "matrix from illuminant" );
    OIIO_CHECK_EQUAL(
        rta::cache::matrix_from_dng_metadata_cache.name,
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
        OIIO_CHECK_EQUAL(
            "Could not extract time from output: ", std::string( e.what() ) );
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

    return unit_test_failures;
}
