// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "lens_correction_cache.h"

namespace rta
{

// This is required for the cache to build, but only used for logging.
// Not sure if we'll ever need all values of the ImageSpec object printed out,
// so putting in a placeholder for now. We can populate this later if needed
// for debugging.
std::ostream &
operator<<( std::ostream &os, const OIIO::ImageSpec &data [[maybe_unused]] )
{
    return os << std::string( "<ImageSpec>" );
} // LCOV_EXCL_LINE - bug in coverage tool

bool operator==( const OIIO::ImageSpec &data1, const OIIO::ImageSpec &data2 )
{
    if ( data1.width != data2.width )
        return false;
    if ( data1.height != data2.height )
        return false;

    static std::vector<std::string> string_keys = {
        "cameraMake", "cameraModel", "lensMake", "lensModel"
    };

    static std::vector<std::string> float_keys = { "focalLength",
                                                   "aperture",
                                                   "focus" };

    for ( auto &key: string_keys )
    {
        auto v1 = data1.get_string_attribute( key );
        auto v2 = data2.get_string_attribute( key );
        if ( v1 != v2 )
            return false;
    }

    for ( auto &key: float_keys )
    {
        auto v1 = data1.get_float_attribute( key );
        auto v2 = data2.get_float_attribute( key );
        if ( v1 != v2 )
            return false;
    }

    return true;
}

namespace cache
{

cache::Cache<LensDescriptor, ImageBufData> &get_vignette_cache()
{
    static cache::Cache<LensDescriptor, ImageBufData> vignette_cache(
        "lens vignetting" );
    return vignette_cache;
} // LCOV_EXCL_LINE - bug in coverage tool

cache::Cache<LensDescriptor, ImageBufData> &get_distortion_cache()
{
    static cache::Cache<LensDescriptor, ImageBufData> distortion_cache(
        "lens geometric distortion" );
    return distortion_cache;
} // LCOV_EXCL_LINE - bug in coverage tool

cache::Cache<LensDescriptor, ImageBufData> &get_aberration_cache()
{
    static cache::Cache<LensDescriptor, ImageBufData> aberration_cache(
        "lens chromatic aberration" );
    return aberration_cache;
} // LCOV_EXCL_LINE - bug in coverage tool

} // namespace cache
} // namespace rta
