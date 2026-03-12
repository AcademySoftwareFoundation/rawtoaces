// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "lens_correction.h"
#include "lens_correction_cache.h"

#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imagebufalgo_util.h>
#include <lensfun/lensfun.h>
#include <assert.h>

namespace rta
{
namespace util
{

static lfDatabase *Database;

void reset_database()
{
    delete Database;
    Database = nullptr;
}

const lfModifier *modifier_from_spec(
    const OIIO::ImageSpec &spec,
    bool                   inverse,
    bool                   enable_vignetting,
    bool                   enable_distortion,
    bool                   enable_aberration,
    std::string           &error_message )
{
    std::string camera_make    = spec.get_string_attribute( "cameraMake" );
    std::string camera_model   = spec.get_string_attribute( "cameraModel" );
    std::string lens_make      = spec.get_string_attribute( "lensMake" );
    std::string lens_model     = spec.get_string_attribute( "lensModel" );
    float       focal_length   = spec.get_float_attribute( "focalLength" );
    float       aperture       = spec.get_float_attribute( "aperture" );
    float       focus_distance = spec.get_float_attribute( "focus" );

    if ( focus_distance == 0 )
        focus_distance = 1000.f;

    if ( Database == nullptr )
    {
        Database                = new lfDatabase();
        bool loaded_succesfully = false;

        // Try loading the default path first.
        if ( Database->Load() == LF_NO_ERROR )
            loaded_succesfully = true;

        std::vector<std::string> paths;
        char                    *path = getenv( "RAWTOACES_LENSFUNDB_PATH" );
        if ( path != nullptr )
        {
#if defined( WIN32 ) || defined( WIN64 )
            const std::string separator = ";";
#else
            const std::string separator = ":";
#endif
            OIIO::Strutil::split( path, paths, separator );
        }

        for ( const auto &each_path: paths )
        {
#if ( LF_VERSION <= ( ( 0 << 24 ) | ( 3 << 16 ) | ( 4 << 8 ) ) )
            if ( Database->LoadDirectory( each_path.c_str() ) )
                loaded_succesfully = true;
#else
            if ( Database->Load( each_path.c_str() ) == LF_NO_ERROR )
                loaded_succesfully = true;
#endif // LF_VERSION
        }

        if ( !loaded_succesfully )
        {
            error_message =
                "Lensfun DB not found, please provide the path to "
                "the database directory via the "
                "RAWTOACES_LENSFUNDB_PATH environment variable.";
            return nullptr;
        }
    }

    const lfCamera **cameras =
        Database->FindCamerasExt( camera_make.c_str(), camera_model.c_str() );

    if ( cameras == nullptr )
    {
        error_message = "Lens correction data for the camera make '" +
                        camera_make + "' and model '" + camera_model +
                        "' not found " + "in the database.";
        return nullptr;
    }

    auto cam = cameras[0];

    const lfLens **lenses =
        Database->FindLenses( cam, lens_make.c_str(), lens_model.c_str() );

    if ( lenses == nullptr )
    {
        error_message = "Lens correction data for the lens make '" + lens_make +
                        "'(optional) and model '" + lens_model +
                        "' not found in the database.";
        return nullptr;
    }

    auto lens = lenses[0];

    int requested_flags   = 0;
    int initialised_flags = 0;

#if ( LF_VERSION <= ( ( 0 << 24 ) | ( 3 << 16 ) | ( 4 << 8 ) ) )

    if ( enable_vignetting )
        requested_flags |= LF_MODIFY_VIGNETTING;

    if ( enable_distortion )
        requested_flags |= LF_MODIFY_DISTORTION;

    if ( enable_aberration )
        requested_flags |= LF_MODIFY_TCA;

    lfModifier *mod = new lfModifier(
        lens, cam->CropFactor, spec.full_width, spec.full_height );
    if ( mod != nullptr )
    {
        initialised_flags = mod->Initialize(
            lens,
            LF_PF_F32,
            focal_length,
            aperture,
            focus_distance,
            1.0,
            LF_UNKNOWN,
            requested_flags,
            inverse );
    }
#else
    lfModifier *mod = new lfModifier(
        lens,
        focal_length,
        cam->CropFactor,
        spec.full_width,
        spec.full_height,
        LF_PF_F32,
        inverse );

    if ( mod != nullptr )
    {
        if ( enable_vignetting )
        {
            requested_flags |= LF_MODIFY_VIGNETTING;
            initialised_flags |=
                mod->EnableVignettingCorrection( aperture, focus_distance );
        }

        if ( enable_distortion )
        {
            requested_flags |= LF_MODIFY_DISTORTION;
            initialised_flags |= mod->EnableDistortionCorrection();
        }

        if ( enable_aberration )
        {
            requested_flags |= LF_MODIFY_TCA;
            initialised_flags |= mod->EnableTCACorrection();
        }
    }
#endif // LF_VERSION

    if ( requested_flags != initialised_flags )
    {
        if ( mod != nullptr )
        {
            delete mod;
            mod = nullptr;
        }
    }

    if ( mod == nullptr )
    {
        error_message = "Failed to initialise lens correction transform.";
    }

    return mod;
}

bool solve_vignette_map(
    const OIIO::ImageSpec &spec,
    bool                   inverse,
    cache::ImageBufData   &cache_data,
    std::string           &error_message )
{
    int             nthreads = 1;
    OIIO::ImageBuf &dst      = cache_data;
    dst.reset( spec );

    const lfModifier *modifier =
        modifier_from_spec( spec, inverse, true, false, false, error_message );

    if ( modifier == nullptr )
        return false;

    OIIO::ImageBufAlgo::parallel_image(
        dst.roi(), nthreads, [&]( OIIO::ROI roi ) {
            OIIO::ImageBuf::Iterator<float> iterator( dst, roi );
            size_t                          count = roi.width();

            std::vector<float> buff( count );

            for ( int y = roi.ybegin; y < roi.yend; y++ )
            {
                for ( size_t x = 0; x < count; x++ )
                {
                    buff[x] = 1.0;
                }

                modifier->ApplyColorModification(
                    buff.data(),
                    (float)( roi.xbegin - spec.full_x ) + 0.5f,
                    (float)( y - spec.full_y ) + 0.5f,
                    roi.width(),
                    1,
                    LF_CR_1( INTENSITY ),
                    0 );

                size_t i = 0;
                for ( int x = roi.xbegin; x < roi.xend; x++ )
                {
                    for ( int c = roi.chbegin; c < roi.chend; c++ )
                    {
                        iterator[c] = buff[i];
                    }
                    iterator++;
                    i++;
                }
            }
        } );

    delete modifier;
    return true;
}

std::pair<bool, const OIIO::ImageBuf &> fetch_vignette_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    cache::LensDescriptor descriptor = spec;

    auto &vignette_cache     = cache::get_vignette_cache();
    vignette_cache.disabled  = disable_cache;
    vignette_cache.verbosity = verbosity;

    const std::pair<bool, OIIO::ImageBuf> &result =
        vignette_cache.fetch( descriptor, [&]( OIIO::ImageBuf &buffer ) {
            return solve_vignette_map( spec, false, buffer, error_message );
        } );

    return result;
}

OIIO::ImageSpec init_lens_map_spec(
    const OIIO::ImageSpec &src_spec,
    int                    channels,
    const std::string     &camera_make,
    const std::string     &camera_model,
    const std::string     &lens_make,
    const std::string     &lens_model,
    float                  focal_length,
    float                  aperture,
    float                  focus_distance )
{
    OIIO::ImageSpec spec(
        src_spec.width, src_spec.height, channels, OIIO::TypeDesc::FLOAT );
    spec.x              = src_spec.x;
    spec.y              = src_spec.y;
    spec.full_x         = src_spec.full_x;
    spec.full_y         = src_spec.full_y;
    spec.full_width     = src_spec.full_width;
    spec.full_height    = src_spec.full_height;
    spec["cameraMake"]  = camera_make;
    spec["cameraModel"] = camera_model;
    spec["lensMake"]    = lens_make;
    spec["lensModel"]   = lens_model;
    spec["aperture"]    = aperture;
    spec["focalLength"] = focal_length;
    spec["focus"]       = focus_distance;
    return spec;
}

bool apply_vignette_map(
    OIIO::ImageBuf       &dst_buffer,
    const OIIO::ImageBuf &src_buffer,
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &lens_make,
    const std::string    &lens_model,
    float                 focal_length,
    float                 aperture,
    float                 focus_distance,
    int                   verbosity,
    bool                  disable_cache,
    std::string          &error_message )
{
#if OIIO_VERSION < OIIO_MAKE_VERSION( 3, 1, 0 )
    const int nchannels = 3;
#else
    const int nchannels = 1;
#endif

    OIIO::ImageSpec spec = init_lens_map_spec(
        src_buffer.spec(),
        nchannels,
        camera_make,
        camera_model,
        lens_make,
        lens_model,
        focal_length,
        aperture,
        focus_distance );

    const auto &result =
        fetch_vignette_map( spec, verbosity, disable_cache, error_message );
    bool        success      = result.first;
    const auto &vignette_map = result.second;

    if ( !success )
    {
        error_message = "Failed to create the vignette map. " + error_message;
        return false;
    }

#if OIIO_VERSION < OIIO_MAKE_VERSION( 3, 1, 0 )
    success = OIIO::ImageBufAlgo::mul( dst_buffer, src_buffer, vignette_map );
#else
    success =
        OIIO::ImageBufAlgo::scale( dst_buffer, src_buffer, vignette_map, {} );
#endif
    if ( !success )
    {
        error_message = "Failed to apply the vignette map.";
        return false;
    }

    return true;
}

bool solve_distortion_map(
    const OIIO::ImageSpec &spec,
    bool                   inverse,
    cache::ImageBufData   &cache_data,
    std::string           &error_message )
{
    int             nthreads = 1;
    OIIO::ImageBuf &dst      = cache_data;
    dst.reset( spec );

    const lfModifier *modifier =
        modifier_from_spec( spec, inverse, false, true, false, error_message );

    if ( modifier == nullptr )
        return false;

    const float offsets[2] = { (float)spec.full_x, (float)spec.full_y };

    const float scales[2] = { 1.0f / spec.full_width, 1.0f / spec.full_height };

    OIIO::ImageBufAlgo::parallel_image(
        dst.roi(), nthreads, [&]( OIIO::ROI roi ) {
            OIIO::ImageBuf::Iterator<float> iterator( dst, roi );
            size_t                          count = roi.width() * 2 * 3;

            std::vector<float> buff( count );

            for ( int y = roi.ybegin; y < roi.yend; y++ )
            {
                modifier->ApplyGeometryDistortion(
                    (float)( roi.xbegin - spec.full_x ) + 0.5f,
                    (float)( y - spec.full_y ) + 0.5f,
                    roi.width(),
                    1,
                    buff.data() );

                size_t i = 0;
                for ( int x = roi.xbegin; x < roi.xend; x++ )
                {
                    for ( int j = 0; j < 2; j++ )
                    {
                        float val = buff[i++];
                        val       = ( val + offsets[j] ) * scales[j];
                        iterator[roi.chbegin + j] = val;
                    }
                    iterator++;
                }
            }
        } );

    delete modifier;
    return true;
}

std::pair<bool, const OIIO::ImageBuf &> fetch_distortion_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    cache::LensDescriptor descriptor = spec;

    auto &distortion_cache     = cache::get_distortion_cache();
    distortion_cache.disabled  = disable_cache;
    distortion_cache.verbosity = verbosity;

    const std::pair<bool, OIIO::ImageBuf> &result =
        distortion_cache.fetch( descriptor, [&]( OIIO::ImageBuf &buffer ) {
            return solve_distortion_map( spec, false, buffer, error_message );
        } );

    return result;
}

bool apply_distortion_map(
    OIIO::ImageBuf       &dst_buffer,
    const OIIO::ImageBuf &src_buffer,
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &lens_make,
    const std::string    &lens_model,
    float                 focal_length,
    int                   verbosity,
    bool                  disable_cache,
    std::string          &error_message )
{
    const int nthreads = 0;

    OIIO::ImageSpec spec = init_lens_map_spec(
        src_buffer.spec(),
        2,
        camera_make,
        camera_model,
        lens_make,
        lens_model,
        focal_length );

    const auto &result =
        fetch_distortion_map( spec, verbosity, disable_cache, error_message );
    bool        success        = result.first;
    const auto &distortion_map = result.second;

    if ( !success )
    {
        error_message = "Failed to create the distortion map. " + error_message;
        return false;
    }

    if ( dst_buffer.initialized() )
        spec = dst_buffer.spec();
    else
        spec = src_buffer.spec();

    OIIO::ImageBuf temp_buffer( spec );

    success = OIIO::ImageBufAlgo::st_warp(
        temp_buffer,
        src_buffer,
        distortion_map,
        nullptr,
        0,
        1,
        false,
        false,
        {},
        nthreads );

    if ( !success )
    {
        error_message = "Failed to apply the distortion map.";
        return false;
    }

    dst_buffer = temp_buffer;
    return true;
}

bool solve_aberration_map(
    const OIIO::ImageSpec &spec,
    bool                   inverse,
    cache::ImageBufData   &cache_data,
    std::string           &error_message )
{
    int             nthreads = 0;
    OIIO::ImageBuf &dst      = cache_data;
    dst.reset( spec );

    const lfModifier *modifier =
        modifier_from_spec( spec, inverse, false, false, true, error_message );

    if ( modifier == nullptr )
        return false;

    const float offsets[6] = { (float)spec.full_x, (float)spec.full_y,
                               (float)spec.full_x, (float)spec.full_y,
                               (float)spec.full_x, (float)spec.full_y };

    const float scales[6] = {
        1.0f / spec.full_width, 1.0f / spec.full_height,
        1.0f / spec.full_width, 1.0f / spec.full_height,
        1.0f / spec.full_width, 1.0f / spec.full_height,
    };

    OIIO::ImageBufAlgo::parallel_image(
        dst.roi(), nthreads, [&]( OIIO::ROI roi ) {
            OIIO::ImageBuf::Iterator<float> iterator( dst, roi );
            size_t                          count = roi.width() * 2 * 3;

            std::vector<float> buff( count );

            for ( int y = roi.ybegin; y < roi.yend; y++ )
            {

                modifier->ApplySubpixelDistortion(
                    (float)( roi.xbegin - spec.full_x ) + 0.5f,
                    (float)( y - spec.full_y ) + 0.5f,
                    roi.width(),
                    1,
                    buff.data() );

                int i = 0;
                for ( int x = roi.xbegin; x < roi.xend; x++ )
                {
                    for ( int j = 0; j < 6; j++ )
                    {
                        float val = buff[i++];
                        val       = ( val + offsets[j] ) * scales[j];
                        iterator[roi.chbegin + j] = val;
                    }
                    iterator++;
                }
            }
        } );

    delete modifier;
    return true;
}

std::pair<bool, const OIIO::ImageBuf &> fetch_aberration_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    cache::LensDescriptor descriptor = spec;

    auto &aberration_cache     = cache::get_aberration_cache();
    aberration_cache.disabled  = disable_cache;
    aberration_cache.verbosity = verbosity;

    const std::pair<bool, OIIO::ImageBuf> &result =
        aberration_cache.fetch( descriptor, [&]( OIIO::ImageBuf &buffer ) {
            return solve_aberration_map( spec, false, buffer, error_message );
        } );

    return result;
}

bool apply_aberration_map(
    OIIO::ImageBuf       &dst_buffer,
    const OIIO::ImageBuf &src_buffer,
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &lens_make,
    const std::string    &lens_model,
    float                 focal_length,
    int                   verbosity,
    bool                  disable_cache,
    std::string          &error_message )
{
    OIIO::ImageSpec spec = init_lens_map_spec(
        src_buffer.spec(),
        6,
        camera_make,
        camera_model,
        lens_make,
        lens_model,
        focal_length );

    const auto &result =
        fetch_aberration_map( spec, verbosity, disable_cache, error_message );
    bool        success        = result.first;
    const auto &aberration_map = result.second;

    if ( !success )
    {
        error_message =
            "Failed to create the chromatic aberration map. " + error_message;
        return false;
    }

    if ( dst_buffer.initialized() )
        spec = dst_buffer.spec();
    else
        spec = src_buffer.spec();

    OIIO::ImageBuf temp_buffer( spec );

    OIIO::ROI roi = spec.roi();

    for ( int i = 0; i < spec.nchannels; i++ )
    {
        int s_chan  = ( i * 2 ) % 6;
        roi.chbegin = i;
        roi.chend   = i + 1;
        success     = OIIO::ImageBufAlgo::st_warp(
            temp_buffer,
            src_buffer,
            aberration_map,
            nullptr,
            s_chan,
            s_chan + 1,
            false,
            false,
            roi );
        if ( !success )
        {
            error_message = "Failed to apply the chromatic aberration map.";
            return false;
        }
    }

    dst_buffer = temp_buffer;
    return true;
}

} // namespace util
} // namespace rta
