// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include "../src/rawtoaces_util/lens_correction.h"
#include "../src/rawtoaces_util/lens_correction_cache.h"
#include "../src/rawtoaces_util/rawtoaces_util_priv.h"

#include "test_utils.h"
#include <OpenImageIO/unittest.h>
#include <OpenImageIO/imagebufalgo.h>

const std::string camera_make    = "RTA_TestCameraMake";
const std::string camera_model   = "RTA_TestCameraModel";
const std::string lens_make      = "RTA_TestLensMake";
const std::string lens_model     = "RTA_TestLensModel";
const float       focal_length   = 12.0f;
const float       aperture       = 3.5f;
const float       focus_distance = 1000.0f;

#define VIGN_PARAMS                                                            \
    camera_make, camera_model, lens_make, lens_model, focal_length, aperture,  \
        focus_distance

#define DIST_PARAMS                                                            \
    camera_make, camera_model, lens_make, lens_model, focal_length

void test_default_path()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec source_spec( 10, 10, 1 );
    OIIO::ImageBuf  source_image( source_spec );

    // Check some old camera and lens which would be likely present in many
    // versions of the database.
    OIIO::ImageSpec map_spec = rta::util::init_lens_map_spec(
        source_spec,
        1,
        "Panasonic",
        "DMC-GF2",
        "Panasonic",
        "Lumix G Vario 14-42mm f/3.5-5.6 II Asph. Mega OIS",
        12.0f,
        3.5f,
        1000.0f );

    std::string error_message;
    auto        result =
        rta::util::solve_vignette_map( map_spec, false, error_message );

#if defined( WIN32 ) || defined( WIN64 )
    OIIO_CHECK_ASSERT( !result );
    ASSERT_CONTAINS( error_message, std::string( "Lensfun DB not found" ) );
#else
    OIIO_CHECK_ASSERT( result );
    OIIO_CHECK_EQUAL( error_message, std::string( "" ) );
#endif
}

void test_camera_not_found()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec source_spec( 10, 10, 1 );
    OIIO::ImageBuf  source_image( source_spec );

    OIIO::ImageSpec map_spec = rta::util::init_lens_map_spec(
        source_spec,
        1,
        camera_make,
        "unknown_camera_model",
        lens_make,
        lens_model,
        focal_length );

    std::string error_message;
    auto        result =
        rta::util::solve_vignette_map( map_spec, false, error_message );

    OIIO_CHECK_ASSERT( !result );
    ASSERT_CONTAINS(
        error_message, "Lens correction data for the camera make '" );
    ASSERT_CONTAINS( error_message, "not found" );
}

void test_lens_not_found()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec source_spec( 10, 10, 1 );
    OIIO::ImageBuf  source_image( source_spec );

    OIIO::ImageSpec map_spec = rta::util::init_lens_map_spec(
        source_spec,
        1,
        camera_make,
        camera_model,
        lens_make,
        "unknown_lens_model",
        focal_length );

    std::string error_message;
    auto        result =
        rta::util::solve_vignette_map( map_spec, false, error_message );

    OIIO_CHECK_ASSERT( !result );
    ASSERT_CONTAINS(
        error_message, "Lens correction data for the lens make '" );
    ASSERT_CONTAINS( error_message, "not found" );
}

void test_init_failed()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec source_spec( 10, 10, 1 );
    OIIO::ImageBuf  source_image( source_spec );

    OIIO::ImageSpec map_spec = rta::util::init_lens_map_spec(
        source_spec,
        1,
        camera_make,
        camera_model,
        lens_make,
        "RTA_BadLensModel",
        1.0f,
        1.0f,
        1.0f );

    std::string error_message;
    auto        result =
        rta::util::solve_vignette_map( map_spec, false, error_message );

    OIIO_CHECK_ASSERT( !result );
    ASSERT_CONTAINS(
        error_message, "Failed to initialise lens correction transform" );
}

OIIO::ImageBuf make_source_image()
{
    constexpr int image_margin = 50;
    constexpr int image_width  = 800;
    constexpr int image_height = 600;

    OIIO::ImageSpec spec(
        image_width + image_margin * 3,
        image_height + image_margin * 3,
        3,
        OIIO::TypeDesc::FLOAT );

    spec.full_x      = image_margin;
    spec.full_y      = image_margin;
    spec.full_width  = image_width;
    spec.full_height = image_height;

    // Create a test image first.
    OIIO::ImageBuf source_image;
    OIIO::ImageBufAlgo::checker(
        source_image,
        image_margin,
        image_margin,
        1,
        { 0.1f },
        { 0.9f },
        0,
        0,
        0,
        spec.roi() );
    source_image.specmod() = spec;

    return source_image;
}

OIIO::ImageBuf create_test_image(
    const OIIO::ImageBuf &src_image,
    bool                  do_vignetting,
    bool                  do_aberration,
    bool                  do_distortion )
{
    const auto    &src_spec  = src_image.spec();
    OIIO::ImageBuf tmp_image = src_image;

    // Apply the requested types of lens phenomena in the reverse order of what
    // we do in correction: distortion -> aberration -> vignetting.

    if ( do_distortion )
    {
        // Create a geometric distortion map.
        OIIO::ImageSpec map_spec =
            rta::util::init_lens_map_spec( src_spec, 2, DIST_PARAMS );

        std::string error_message;
        auto        distort_map =
            rta::util::solve_distortion_map( map_spec, true, error_message );
        OIIO_CHECK_EQUAL( error_message, "" );
        OIIO_CHECK_ASSERT( distort_map );

        // Apply the distortion map to the source image.
        OIIO::ImageBuf distorted_image = OIIO::ImageBufAlgo::st_warp(
            tmp_image, *distort_map.get(), nullptr, 0, 1 );

        tmp_image = distorted_image;
    }

    if ( do_aberration )
    {
        // Create a chromatic aberration map.
        OIIO::ImageSpec map_spec =
            rta::util::init_lens_map_spec( src_spec, 6, DIST_PARAMS );

        std::string error_message;
        auto        aberration_map =
            rta::util::solve_aberration_map( map_spec, true, error_message );
        OIIO_CHECK_ASSERT( aberration_map );

        // Apply the distortion map to the source image.
        OIIO::ImageBuf distorted_image( src_spec );
        OIIO::ROI      dst_roi = src_spec.roi();
        for ( int i = 0; i < 3; i++ )
        {
            dst_roi.chbegin = i;
            dst_roi.chend   = i + 1;

            OIIO::ImageBufAlgo::st_warp(
                distorted_image,
                tmp_image,
                *aberration_map.get(),
                nullptr,
                i * 2,
                i * 2 + 1,
                false,
                false,
                dst_roi );
        }

        tmp_image = distorted_image;
    }

    if ( do_vignetting )
    {

        // Create a vignette map.
#if OIIO_VERSION < OIIO_MAKE_VERSION( 3, 1, 0 )
        const int nchannels = 3;
#else
        const int nchannels = 1;
#endif

        OIIO::ImageSpec map_spec =
            rta::util::init_lens_map_spec( src_spec, nchannels, VIGN_PARAMS );

        std::string error_message;
        auto        vignette_map =
            rta::util::solve_vignette_map( map_spec, true, error_message );
        OIIO_CHECK_ASSERT( vignette_map );

        // Apply vignetting to the source image.
        OIIO::ImageBuf distorted_image;

#if OIIO_VERSION < OIIO_MAKE_VERSION( 3, 1, 0 )
        OIIO::ImageBufAlgo::mul(
            distorted_image, tmp_image, *vignette_map.get() );
#else
        OIIO::ImageBufAlgo::scale(
            distorted_image, tmp_image, *vignette_map.get() );
#endif

        tmp_image = distorted_image;
    }

    return tmp_image;
}

void test_vignetting()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    // Create a test image first.
    OIIO::ImageBuf source_image = make_source_image();
    OIIO::ImageBuf distorted_image =
        create_test_image( source_image, true, false, false );

    // Check that the image is now vignetted.
    OIIO::ImageBufAlgo::CompareResults comp1 = OIIO::ImageBufAlgo::compare(
        source_image, distorted_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_GT( comp1.meanerror, 0.11 );

    // Calculate and apply devignetting.
    std::string    error_message;
    OIIO::ImageBuf undistorted_image;
    bool           success = rta::util::apply_vignette_map(
        undistorted_image,
        distorted_image,
        VIGN_PARAMS,
        1,
        true,
        error_message );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( error_message, "" );

    // Check that vignetting has been removed.
    OIIO::ImageBufAlgo::CompareResults comp2 = OIIO::ImageBufAlgo::compare(
        source_image, undistorted_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_LE( comp2.meanerror, 1e-7 );
}

void test_distortion()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    // Create a test image first.
    OIIO::ImageBuf source_image = make_source_image();
    OIIO::ImageBuf distorted_image =
        create_test_image( source_image, false, false, true );

    // Check that the image is now distorted.
    OIIO::ImageBufAlgo::CompareResults comp1 = OIIO::ImageBufAlgo::compare(
        source_image, distorted_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_GT( comp1.meanerror, 0.15 );

    // Calculate and apply distortion correction.
    std::string    error_message;
    OIIO::ImageBuf undistorted_image;
    bool           success = rta::util::apply_distortion_map(
        undistorted_image,
        distorted_image,
        DIST_PARAMS,
        0,
        true,
        error_message );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( error_message, "" );

    // Check that un-distort reduces the error.
    OIIO::ImageBufAlgo::CompareResults comp2 = OIIO::ImageBufAlgo::compare(
        source_image, undistorted_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_LE( comp2.meanerror, 0.0104 );
}

void test_aberration()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    // Create a test image first.
    OIIO::ImageBuf source_image = make_source_image();
    OIIO::ImageBuf distorted_image =
        create_test_image( source_image, false, true, false );

    // Check that the image is now distorted.
    OIIO::ImageBufAlgo::CompareResults comp1 = OIIO::ImageBufAlgo::compare(
        source_image, distorted_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_GT( comp1.meanerror, 0.001 );

    // Calculate and apply chromatic aberration correction.
    std::string    error_message;
    OIIO::ImageBuf undistorted_image;
    bool           success = rta::util::apply_aberration_map(
        undistorted_image,
        distorted_image,
        DIST_PARAMS,
        0,
        true,
        error_message );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( error_message, "" );

    // Check that un-distort reduces the error.
    OIIO::ImageBufAlgo::CompareResults comp2 = OIIO::ImageBufAlgo::compare(
        source_image, undistorted_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_LE( comp2.meanerror, 0.0003 );
}

namespace rta
{

void test_print_helpers()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec spec;
    std::string     output =
        capture_stderr( [&]() { std::cerr << spec << std::endl; } );
    ASSERT_CONTAINS( output, "<ImageSpec>" );
}

void test_imagespec_comparison()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec spec1( 200, 100, 1 );
    spec1["cameraMake"]  = "same camera make";
    spec1["cameraModel"] = "same same model";
    spec1["lensMake"]    = "same lens make";
    spec1["lensModel"]   = "same lens model";
    spec1["focalLength"] = 10.0f;
    spec1["aperture"]    = 20.0f;
    spec1["focus"]       = 30.0f;

    OIIO::ImageSpec spec2 = spec1;
    OIIO_CHECK_ASSERT( spec1 == spec2 );

    spec2       = spec1;
    spec2.width = 201;
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2        = spec1;
    spec2.height = 101;
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2               = spec1;
    spec2["cameraMake"] = "different camera make";
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2                = spec1;
    spec2["cameraModel"] = "different camera model";
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2             = spec1;
    spec2["lensMake"] = "different lens make";
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2              = spec1;
    spec2["lensModel"] = "different lens model";
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2                = spec1;
    spec2["focalLength"] = 11.0f;
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2             = spec1;
    spec2["aperture"] = 21.0f;
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );

    spec2          = spec1;
    spec2["Focus"] = 31.0f;
    OIIO_CHECK_ASSERT( !( spec1 == spec2 ) );
}
} // namespace rta

void test_lens_correction_caches()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO_CHECK_EQUAL(
        rta::cache::get_vignette_cache().name, "lens vignetting" );
    OIIO_CHECK_EQUAL(
        rta::cache::get_distortion_cache().name, "lens geometric distortion" );
    OIIO_CHECK_EQUAL(
        rta::cache::get_aberration_cache().name, "lens chromatic aberration" );

    try
    {
        // Invoke the constructor and destructor to get full function coverage.
        rta::cache::Cache<rta::cache::LensDescriptor, rta::cache::ImageBufData>
            cache1;
    }
    catch ( const std::exception &e )
    {
        // always fails
        OIIO_CHECK_EQUAL( "Unexpected exception: ", std::string( e.what() ) );
    }
}

void test_apply_lens_correction()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    // Create a test image first.
    OIIO::ImageBuf source_image = make_source_image();
    const auto    &src_spec     = source_image.specmod();

    // Create a vignette map.
#if OIIO_VERSION < OIIO_MAKE_VERSION( 3, 1, 0 )
    const int nchannels = 3;
#else
    const int nchannels = 1;
#endif
    OIIO::ImageSpec map_spec =
        rta::util::init_lens_map_spec( src_spec, nchannels, VIGN_PARAMS );

    std::string error_message;
    auto        vignette_map =
        rta::util::solve_vignette_map( map_spec, true, error_message );

    // Apply vignetting to the source image.
    OIIO::ImageBuf distorted_image( src_spec );

#if OIIO_VERSION < OIIO_MAKE_VERSION( 3, 1, 0 )
    OIIO::ImageBufAlgo::mul(
        distorted_image, source_image, *vignette_map.get() );
#else
    OIIO::ImageBufAlgo::scale(
        distorted_image, source_image, *vignette_map.get() );
#endif

    rta::util::ImageConverter converter;
    converter.settings.disable_cache = true;
    converter.settings.lens_correction_types =
        rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;

    auto &distorted_spec          = distorted_image.specmod();
    distorted_spec["cameraMake"]  = camera_make;
    distorted_spec["cameraModel"] = camera_model;
    distorted_spec["lensMake"]    = lens_make;
    distorted_spec["lensModel"]   = lens_model;
    distorted_spec["aperture"]    = aperture;
    distorted_spec["focalLength"] = focal_length;
    distorted_spec["focus"]       = focus_distance;

    OIIO::ImageBuf output_image;
    converter.apply_lens_correction( output_image, distorted_image );

    // Check that vignetting has been removed.
    OIIO::ImageBufAlgo::CompareResults comp2 = OIIO::ImageBufAlgo::compare(
        source_image, output_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_LE( comp2.meanerror, 1e-7 );
}

void check_missing(
    rta::util::ImageConverter &converter,
    OIIO::ImageBuf            &buffer,
    const std::string         &property_name )
{
    bool success = converter.apply_lens_correction( buffer, buffer );
    OIIO_CHECK_ASSERT( !success );
    ASSERT_CONTAINS(
        converter.last_error_message,
        "Missing the " + property_name + " in the file metadata" );
}

void test_apply_lens_correction_fail_init()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    rta::util::ImageConverter converter;
    converter.settings.lens_correction_types =
        rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;
    converter.settings.require_lens_correction = true;
    converter.settings.disable_cache           = true;

    OIIO::ImageBuf buffer;
    check_missing( converter, buffer, "camera manufacturer name" );

    buffer.specmod()["cameraMake"] = camera_make;
    check_missing( converter, buffer, "camera model name" );

    buffer.specmod()["cameraModel"] = camera_model;
    check_missing( converter, buffer, "lens model name" );

    buffer.specmod()["lensModel"] = lens_model;
    check_missing( converter, buffer, "focal length value" );

    buffer.specmod()["focalLength"] = focal_length;
    check_missing( converter, buffer, "aperture value" );
}

void test_apply_lens_correction_fail_generate()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec spec( 10, 10, 3 );
    spec["cameraMake"]  = camera_make;
    spec["cameraModel"] = camera_model;
    spec["lensMake"]    = lens_make;
    spec["lensModel"]   = lens_model;
    spec["aperture"]    = aperture;
    spec["focalLength"] = focal_length;
    spec["focus"]       = focus_distance;

    OIIO::ImageBuf buffer( spec );

    {
        rta::util::ImageConverter converter;
        converter.settings.lens_correction_types =
            rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;
        converter.settings.require_lens_correction = true;
        converter.settings.disable_cache           = true;

        bool success = converter.apply_lens_correction( buffer, buffer );
        OIIO_CHECK_ASSERT( !success );
        ASSERT_CONTAINS(
            converter.last_error_message,
            "Failed to create the vignette map." );
    }

    {
        rta::util::ImageConverter converter;
        converter.settings.lens_correction_types =
            rta::util::ImageConverter::Settings::LensCorrectionType::Distortion;
        converter.settings.require_lens_correction = true;
        converter.settings.disable_cache           = true;

        bool success = converter.apply_lens_correction( buffer, buffer );
        OIIO_CHECK_ASSERT( !success );
        ASSERT_CONTAINS(
            converter.last_error_message,
            "Failed to create the distortion map." );
    }

    {
        rta::util::ImageConverter converter;
        converter.settings.lens_correction_types =
            rta::util::ImageConverter::Settings::LensCorrectionType::Aberration;
        converter.settings.require_lens_correction = true;
        converter.settings.disable_cache           = true;

        bool success = converter.apply_lens_correction( buffer, buffer );
        OIIO_CHECK_ASSERT( !success );
        ASSERT_CONTAINS(
            converter.last_error_message,
            "Failed to create the chromatic aberration map." );
    }
}

void test_apply_lens_correction_fail_apply()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    OIIO::ImageSpec spec( 10, 10, 3 );
    spec["cameraMake"]  = camera_make;
    spec["cameraModel"] = camera_model;
    spec["lensMake"]    = lens_make;
    spec["lensModel"]   = lens_model;
    spec["aperture"]    = aperture;
    spec["focalLength"] = focal_length;
    spec["focus"]       = focus_distance;

    OIIO::ImageBuf src_buffer( spec );

    // Using a deep image buffer as destination makes applying a correction
    // fail.
    OIIO::ImageSpec dst_spec( 1, 1, 1 );
    dst_spec.deep = true;
    OIIO::ImageBuf dst_buffer( dst_spec );

    {
        rta::util::ImageConverter converter;
        converter.settings.lens_correction_types =
            rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;
        converter.settings.require_lens_correction = true;
        converter.settings.disable_cache           = true;

        bool success =
            converter.apply_lens_correction( dst_buffer, src_buffer );
        OIIO_CHECK_ASSERT( !success );
        ASSERT_CONTAINS(
            converter.last_error_message, "Failed to apply the vignette map." );
    }

    {
        rta::util::ImageConverter converter;
        converter.settings.lens_correction_types =
            rta::util::ImageConverter::Settings::LensCorrectionType::Distortion;
        converter.settings.require_lens_correction = true;
        converter.settings.disable_cache           = true;

        bool success =
            converter.apply_lens_correction( dst_buffer, src_buffer );
        OIIO_CHECK_ASSERT( !success );
        ASSERT_CONTAINS(
            converter.last_error_message,
            "Failed to apply the distortion map." );
    }

    {
        rta::util::ImageConverter converter;
        converter.settings.lens_correction_types =
            rta::util::ImageConverter::Settings::LensCorrectionType::Aberration;
        converter.settings.require_lens_correction = true;
        converter.settings.disable_cache           = true;

        bool success =
            converter.apply_lens_correction( dst_buffer, src_buffer );
        OIIO_CHECK_ASSERT( !success );
        ASSERT_CONTAINS(
            converter.last_error_message,
            "Failed to apply the chromatic aberration map." );
    }
}

void check_apply_lens_correction(
    bool do_vignetting, bool do_aberration, bool do_distortion )
{
    // Create a test image first.
    OIIO::ImageBuf source_image    = make_source_image();
    OIIO::ImageBuf distorted_image = create_test_image(
        source_image, do_vignetting, do_aberration, do_distortion );

    auto &distorted_spec          = distorted_image.specmod();
    distorted_spec["cameraMake"]  = camera_make;
    distorted_spec["cameraModel"] = camera_model;
    distorted_spec["lensMake"]    = lens_make;
    distorted_spec["lensModel"]   = lens_model;
    distorted_spec["aperture"]    = aperture;
    distorted_spec["focalLength"] = focal_length;
    distorted_spec["focus"]       = focus_distance;

    rta::util::ImageConverter converter;
    converter.settings.require_lens_correction = true;
    converter.settings.disable_cache           = true;

    float target_error = 0;

    std::cout << "  Checking '";
    if ( do_vignetting )
    {
        std::cout << "v";
        converter.settings.lens_correction_types |=
            rta::util::ImageConverter::Settings::LensCorrectionType::Vignetting;
        target_error += 0.0000001f;
    }
    if ( do_aberration )
    {
        std::cout << "a";
        converter.settings.lens_correction_types |=
            rta::util::ImageConverter::Settings::LensCorrectionType::Aberration;

        target_error += 0.0003f;
    }
    if ( do_distortion )
    {
        std::cout << "d";
        converter.settings.lens_correction_types |=
            rta::util::ImageConverter::Settings::LensCorrectionType::Distortion;
        target_error += 0.011f;
    }

    std::cout << "'" << std::endl;

    OIIO::ImageBuf undistorted_image;
    bool           success =
        converter.apply_lens_correction( undistorted_image, distorted_image );

    OIIO_CHECK_ASSERT( success );
    OIIO_CHECK_EQUAL( converter.last_error_message, "" );

    // Check that un-distort reduces the error.
    OIIO::ImageBufAlgo::CompareResults comp2 = OIIO::ImageBufAlgo::compare(
        source_image, undistorted_image, 0.1f, 0.1f, source_image.roi_full() );
    OIIO_CHECK_LE( comp2.meanerror, target_error );
}

void test_apply_lens_correction_success()
{
    std::cout << "\n" << __FUNCTION__ << "\n";

    for ( int do_vignetting = 0; do_vignetting < 2; do_vignetting++ )
    {
        for ( int do_aberration = 0; do_aberration < 2; do_aberration++ )
        {
            for ( int do_distortion = 0; do_distortion < 2; do_distortion++ )
            {
                check_apply_lens_correction(
                    do_vignetting, do_aberration, do_distortion );
            }
        }
    }
}

void set_path( bool enable )
{
    if ( enable )
    {
        // clang-format off
#if defined( WIN32 ) || defined( WIN64 )
        const std::string separator = ";";
#else
        const std::string separator = ":";
#endif
        // clang-format on

        std::string path = "../../tests/materials/lensfun_db";
        path             = path + separator + "../" + path;
        set_env_var( "RAWTOACES_LENSFUNDB_PATH", path );
    }
    else
    {
        unset_env_var( "RAWTOACES_LENSFUNDB_PATH" );
    }
}

int main( int, char ** )
{
    set_path( false );

    test_default_path();
    rta::util::reset_database();

    test_apply_lens_correction_fail_generate();
    rta::util::reset_database();

    set_path( true );

    test_init_failed();
    test_camera_not_found();
    test_lens_not_found();

    test_vignetting();
    test_distortion();
    test_aberration();

    rta::test_print_helpers();
    rta::test_imagespec_comparison();
    test_lens_correction_caches();

    test_apply_lens_correction_fail_init();
    test_apply_lens_correction_fail_apply();
    test_apply_lens_correction_success();

    return unit_test_failures;
}
