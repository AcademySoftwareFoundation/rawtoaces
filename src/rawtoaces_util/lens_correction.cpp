// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "lens_correction.h"
#include "lens_correction_cache.h"

#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imagebufalgo_util.h>
#include <lensfun/lensfun.h>
#include <cmath>
#include <mutex>
#include <memory>
#include <type_traits>
#include <vector>

namespace rta
{
namespace util
{

namespace
{
std::mutex                  database_mutex;
std::shared_ptr<lfDatabase> database;
} // namespace

void reset_database()
{
    std::lock_guard<std::mutex> lock( database_mutex );
    database.reset();
}

static std::shared_ptr<const lfModifier> modifier_from_spec(
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

    if ( spec.width <= 0 || spec.height <= 0 || spec.full_width <= 0 ||
         spec.full_height <= 0 || spec.depth != 1 || spec.deep )
    {
        error_message =
            "Lens correction requires a non-empty, flat 2D image with a valid full window.";
        return nullptr;
    }

    // Serialize database loading and lookup, including the cache-disabled path.
    // Modifiers keep the database alive while maps are calculated outside this lock.
    std::lock_guard<std::mutex> lock( database_mutex );
    if ( !database )
    {
        auto candidate           = std::make_shared<lfDatabase>();
        bool loaded_successfully = false;

        // Try loading the default path first.
        if ( candidate->Load() == LF_NO_ERROR )
            loaded_successfully = true;

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
            if ( candidate->LoadDirectory( each_path.c_str() ) )
                loaded_successfully = true;
#else
            if ( candidate->Load( each_path.c_str() ) == LF_NO_ERROR )
                loaded_successfully = true;
#endif // LF_VERSION
        }

        if ( !loaded_successfully )
        {
            error_message =
                "Lensfun DB not found, please provide the path to "
                "the database directory via the "
                "RAWTOACES_LENSFUNDB_PATH environment variable.";
            return nullptr;
        }
        database = std::move( candidate );
    }

    std::unique_ptr<const lfCamera *, decltype( &lf_free )> cameras(
        database->FindCamerasExt( camera_make.c_str(), camera_model.c_str() ),
        &lf_free );

    if ( !cameras || !cameras.get()[0] )
    {
        error_message = "Lens correction data for the camera make '" +
                        camera_make + "' and model '" + camera_model +
                        "' not found " + "in the database.";
        return nullptr;
    }

    auto cam = cameras.get()[0];

    std::unique_ptr<const lfLens *, decltype( &lf_free )> lenses(
        database->FindLenses(
            cam,
            lens_make.empty() ? nullptr : lens_make.c_str(),
            lens_model.c_str() ),
        &lf_free );

    if ( !lenses || !lenses.get()[0] )
    {
        error_message = "Lens correction data for the lens make '" + lens_make +
                        "'(optional) and model '" + lens_model +
                        "' not found in the database.";
        return nullptr;
    }

    auto lens = lenses.get()[0];

    int requested_flags = 0;
    if ( enable_vignetting )
        requested_flags |= LF_MODIFY_VIGNETTING;
    if ( enable_distortion )
        requested_flags |= LF_MODIFY_DISTORTION;
    if ( enable_aberration )
        requested_flags |= LF_MODIFY_TCA;

    int initialised_flags = 0;
#if ( LF_VERSION <= ( ( 0 << 24 ) | ( 3 << 16 ) | ( 4 << 8 ) ) )
    auto mod = std::make_unique<lfModifier>(
        lens, cam->CropFactor, spec.full_width, spec.full_height );
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
#else
    auto mod = std::make_unique<lfModifier>(
        lens,
        focal_length,
        cam->CropFactor,
        spec.full_width,
        spec.full_height,
        LF_PF_F32,
        inverse );
    if ( enable_vignetting )
        initialised_flags |=
            mod->EnableVignettingCorrection( aperture, focus_distance );
    if ( enable_distortion )
        initialised_flags |= mod->EnableDistortionCorrection();
    if ( enable_aberration )
        initialised_flags |= mod->EnableTCACorrection();
#endif // LF_VERSION

    if ( requested_flags != initialised_flags )
    {
        error_message = "Failed to initialise lens correction transform.";
        return nullptr;
    }
    return std::shared_ptr<const lfModifier>(
        mod.release(), [keep_alive = database]( const lfModifier *modifier ) {
            delete modifier;
        } );
}

namespace
{
template <typename T>
bool lens_parameter(
    T                     &value,
    const OIIO::ImageSpec &spec,
    const char            *attribute,
    const char            *description,
    std::string           &error,
    bool                   required = true )
{
    if ( value == T() )
    {
        if constexpr ( std::is_same_v<T, float> )
            value = spec.get_float_attribute( attribute );
        else
            value = spec.get_string_attribute( attribute );
    }
    if ( value == T() && required )
    {
        error = std::string( "Missing the " ) + description +
                " in the file metadata.";
        return false;
    }
    if constexpr ( std::is_same_v<T, float> )
    {
        if ( !std::isfinite( value ) || value < 0.0f )
        {
            error = std::string( "Invalid " ) + description +
                    ": expected a finite positive value.";
            return false;
        }
    }
    return true;
}
} // namespace

bool apply_lens_corrections(
    OIIO::ImageBuf                 &dst,
    const OIIO::ImageBuf           &src,
    const ImageConverter::Settings &settings,
    std::string                    &error_message )
{
    using Type   = ImageConverter::Settings::LensCorrectionType;
    auto enabled = [&]( Type type ) {
        return ( settings.lens_correction_types & type ) != Type::None;
    };
    const auto &spec           = src.spec();
    auto        camera_make    = settings.custom_camera_make;
    auto        camera_model   = settings.custom_camera_model;
    auto        lens_make      = settings.custom_lens_make;
    auto        lens_model     = settings.custom_lens_model;
    float       focal_length   = settings.custom_focal_length;
    float       aperture       = settings.custom_aperture;
    float       focus_distance = settings.custom_focus_distance;

    // Resolve overrides and metadata once. Missing optional fields do not set errors.
    if ( !lens_parameter(
             camera_make,
             spec,
             "cameraMake",
             "camera manufacturer name",
             error_message ) ||
         !lens_parameter(
             camera_model,
             spec,
             "cameraModel",
             "camera model name",
             error_message ) ||
         !lens_parameter(
             lens_make,
             spec,
             "lensMake",
             "lens manufacturer name",
             error_message,
             false ) ||
         !lens_parameter(
             lens_model,
             spec,
             "lensModel",
             "lens model name",
             error_message ) ||
         !lens_parameter(
             focal_length,
             spec,
             "focalLength",
             "focal length value",
             error_message ) )
        return false;
    if ( enabled( Type::Vignetting ) &&
         ( !lens_parameter(
               aperture, spec, "aperture", "aperture value", error_message ) ||
           !lens_parameter(
               focus_distance,
               spec,
               "focus",
               "focus distance value",
               error_message,
               false ) ) )
        return false;

    if ( !src.initialized() || spec.width <= 0 || spec.height <= 0 ||
         spec.full_width <= 0 || spec.full_height <= 0 || spec.depth != 1 ||
         spec.deep || spec.nchannels <= 0 )
    {
        error_message =
            "Lens correction requires a non-empty, flat 2D image with a valid full window.";
        return false;
    }
    const auto lens_spec = init_lens_map_spec(
        spec,
        1,
        camera_make,
        camera_model,
        lens_make,
        lens_model,
        focal_length,
        aperture,
        focus_distance );

    // Each stage writes to private storage. Commit only once all stages succeed.
    OIIO::ImageBuf        working;
    const OIIO::ImageBuf *input = &src;
    auto                  run   = [&]( Type type, auto apply ) {
        if ( !enabled( type ) )
            return true;
        if ( !apply(
                 working,
                 *input,
                 lens_spec,
                 settings.verbosity,
                 settings.disable_cache,
                 error_message ) )
            return false;
        input = &working;
        return true;
    };
    if ( !run( Type::Vignetting, apply_vignette_map ) ||
         !run( Type::Aberration, apply_aberration_map ) ||
         !run( Type::Distortion, apply_distortion_map ) )
        return false;
    if ( input == &src )
    {
        error_message = "No supported lens correction types selected.";
        return false;
    }
    working.set_name( src.name() );
    working.specmod().extra_attribs = spec.extra_attribs;
    dst                             = std::move( working );
    return true;
}

std::shared_ptr<const OIIO::ImageBuf> solve_vignette_map(
    const OIIO::ImageSpec &spec, bool inverse, std::string &error_message )
{
    int nthreads = 0;

    const auto modifier =
        modifier_from_spec( spec, inverse, true, false, false, error_message );

    if ( modifier == nullptr )
        return nullptr;

    auto            result = std::make_shared<OIIO::ImageBuf>();
    OIIO::ImageBuf &dst    = *result.get();
    dst.reset( spec );

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

    return result;
}

std::shared_ptr<const OIIO::ImageBuf> fetch_vignette_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    cache::LensDescriptor descriptor = spec;

    auto &vignette_cache = cache::get_vignette_cache();

    std::shared_ptr<const OIIO::ImageBuf> result = vignette_cache.fetch(
        descriptor,
        [&]() { return solve_vignette_map( spec, false, error_message ); },
        disable_cache,
        verbosity );

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
    OIIO::ImageBuf        &dst_buffer,
    const OIIO::ImageBuf  &src_buffer,
    const OIIO::ImageSpec &lens_spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    OIIO::ImageSpec spec = lens_spec;
    spec.nchannels       = 1;
    spec.default_channel_names();

    const auto result =
        fetch_vignette_map( spec, verbosity, disable_cache, error_message );

    if ( !result )
    {
        error_message = "Failed to create the vignette map. " + error_message;
        return false;
    }

    OIIO::ImageBuf corrected;
    if ( !corrected.copy( src_buffer ) )
    {
        error_message = "Failed to copy the source for vignetting correction.";
        return false;
    }
    const auto &source_spec  = src_buffer.spec();
    const auto &vignette_map = *result;
    // A single gain channel works with every supported OIIO version. Alpha and
    // depth describe coverage/geometry and must not receive an exposure gain.
    OIIO::ImageBufAlgo::parallel_image(
        corrected.roi(), 0, [&]( OIIO::ROI roi ) {
            OIIO::ImageBuf::Iterator<float>      pixel( corrected, roi );
            OIIO::ImageBuf::ConstIterator<float> gain( vignette_map, roi );
            for ( ; !pixel.done(); ++pixel, ++gain )
                for ( int c = roi.chbegin; c < roi.chend; ++c )
                    if ( c != source_spec.alpha_channel &&
                         c != source_spec.z_channel )
                        pixel[c] = pixel[c] * gain[0];
        } );
    dst_buffer = std::move( corrected );
    return true;
}

std::shared_ptr<const OIIO::ImageBuf> solve_distortion_map(
    const OIIO::ImageSpec &spec, bool inverse, std::string &error_message )
{
    int nthreads = 0;

    const auto modifier =
        modifier_from_spec( spec, inverse, false, true, false, error_message );

    if ( modifier == nullptr )
        return nullptr;

    const float offsets[2] = { (float)spec.full_x, (float)spec.full_y };

    const float scales[2] = { 1.0f / spec.full_width, 1.0f / spec.full_height };

    std::shared_ptr<OIIO::ImageBuf> result = std::make_shared<OIIO::ImageBuf>();
    OIIO::ImageBuf                 &dst    = *result.get();
    dst.reset( spec );

    OIIO::ImageBufAlgo::parallel_image(
        dst.roi(), nthreads, [&]( OIIO::ROI roi ) {
            OIIO::ImageBuf::Iterator<float> iterator( dst, roi );
            size_t                          count = size_t( roi.width() ) * 2;

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

    return result;
}

std::shared_ptr<const OIIO::ImageBuf> fetch_distortion_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    cache::LensDescriptor descriptor = spec;

    auto &distortion_cache = cache::get_distortion_cache();

    const auto result = distortion_cache.fetch(
        descriptor,
        [&]() { return solve_distortion_map( spec, false, error_message ); },
        disable_cache,
        verbosity );

    return result;
}

bool apply_distortion_map(
    OIIO::ImageBuf        &dst_buffer,
    const OIIO::ImageBuf  &src_buffer,
    const OIIO::ImageSpec &lens_spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    OIIO::ImageSpec spec = lens_spec;
    spec.nchannels       = 2;
    spec.default_channel_names();

    const auto result =
        fetch_distortion_map( spec, verbosity, disable_cache, error_message );

    if ( !result )
    {
        error_message = "Failed to create the distortion map. " + error_message;
        return false;
    }

    spec = src_buffer.spec();

    OIIO::ImageBuf temp_buffer( spec );

    const auto &distortion_map = *result.get();

    bool success = OIIO::ImageBufAlgo::st_warp(
        temp_buffer, src_buffer, distortion_map, nullptr, 0, 1, false, false );

    if ( !success )
    {
        error_message = "Failed to apply the distortion map.";
        return false;
    }

    dst_buffer = std::move( temp_buffer );
    return true;
}

std::shared_ptr<const OIIO::ImageBuf> solve_aberration_map(
    const OIIO::ImageSpec &spec, bool inverse, std::string &error_message )
{
    int nthreads = 0;

    const auto modifier =
        modifier_from_spec( spec, inverse, false, false, true, error_message );

    if ( modifier == nullptr )
        return nullptr;

    const float offsets[6] = { (float)spec.full_x, (float)spec.full_y,
                               (float)spec.full_x, (float)spec.full_y,
                               (float)spec.full_x, (float)spec.full_y };

    const float scales[6] = {
        1.0f / spec.full_width, 1.0f / spec.full_height,
        1.0f / spec.full_width, 1.0f / spec.full_height,
        1.0f / spec.full_width, 1.0f / spec.full_height,
    };

    std::shared_ptr<OIIO::ImageBuf> result = std::make_shared<OIIO::ImageBuf>();
    OIIO::ImageBuf                 &dst    = *result.get();
    dst.reset( spec );

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

    return result;
}

std::shared_ptr<const OIIO::ImageBuf> fetch_aberration_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    cache::LensDescriptor descriptor = spec;

    auto &aberration_cache = cache::get_aberration_cache();

    const auto result = aberration_cache.fetch(
        descriptor,
        [&]() { return solve_aberration_map( spec, false, error_message ); },
        disable_cache,
        verbosity );

    return result;
}

bool apply_aberration_map(
    OIIO::ImageBuf        &dst_buffer,
    const OIIO::ImageBuf  &src_buffer,
    const OIIO::ImageSpec &lens_spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message )
{
    OIIO::ImageSpec spec = lens_spec;
    spec.nchannels       = 6;
    spec.default_channel_names();

    const auto entry =
        fetch_aberration_map( spec, verbosity, disable_cache, error_message );

    if ( !entry )
    {
        error_message =
            "Failed to create the chromatic aberration map. " + error_message;
        return false;
    }

    const auto &aberration_map = *entry.get();

    spec = src_buffer.spec();

    if ( spec.nchannels < 3 ||
         ( spec.alpha_channel >= 0 && spec.alpha_channel < 3 ) ||
         ( spec.z_channel >= 0 && spec.z_channel < 3 ) )
    {
        error_message =
            "Chromatic aberration correction requires RGB channels.";
        return false;
    }
    OIIO::ImageBuf temp_buffer;
    if ( !temp_buffer.copy( src_buffer ) )
    {
        error_message =
            "Failed to copy the source for chromatic aberration correction.";
        return false;
    }

    OIIO::ROI roi = spec.roi();

    for ( int i = 0; i < 3; i++ )
    {
        int s_chan   = i * 2;
        roi.chbegin  = i;
        roi.chend    = i + 1;
        bool success = OIIO::ImageBufAlgo::st_warp(
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

    dst_buffer = std::move( temp_buffer );
    return true;
}

} // namespace util
} // namespace rta
