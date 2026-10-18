// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "transform_cache.h"

namespace rta
{

// This is required for the cache to build, but only used for logging.
// Not sure if we'll ever need all values of the Metadata object printed out,
// so putting in a placeholder for now. We can populate this later if needed
// for debugging.
std::ostream &
operator<<( std::ostream &os, const rta::core::Metadata &data [[maybe_unused]] )
{
    return os << std::string( "<Metadata>" );
} // LCOV_EXCL_LINE - bug in coverage tool

bool operator==(
    const rta::core::Metadata &data1, const rta::core::Metadata &data2 )
{
    if ( data1.baseline_exposure != data2.baseline_exposure )
        return false;
    if ( data1.neutral_RGB != data2.neutral_RGB )
        return false;
    for ( size_t i = 0; i < 2; i++ )
    {
        const auto &c1 = data1.calibration[i];
        const auto &c2 = data2.calibration[i];

        if ( c1.illuminant != c2.illuminant )
            return false;
        if ( c1.camera_calibration_matrix != c2.camera_calibration_matrix )
            return false;
        if ( c1.XYZ_to_RGB_matrix != c2.XYZ_to_RGB_matrix )
            return false;
    }

    return true;
}

namespace cache
{

cache::Cache<CameraAndIlluminantDescriptor, WBFromIlluminantData> &
get_WB_from_illuminant_cache()
{
    static cache::Cache<CameraAndIlluminantDescriptor, WBFromIlluminantData>
        WB_from_illuminant_cache( "WB from illuminant" );
    return WB_from_illuminant_cache;
} // LCOV_EXCL_LINE - bug in coverage tool

cache::Cache<CameraAndWBDescriptor, IlluminantAndWBData> &
get_illuminant_from_WB_cache()
{
    static cache::Cache<CameraAndWBDescriptor, IlluminantAndWBData>
        illuminant_from_WB_cache( "illuminant from WB" );
    return illuminant_from_WB_cache;
} // LCOV_EXCL_LINE - bug in coverage tool

cache::Cache<CameraAndIlluminantDescriptor, MatrixData> &
get_matrix_from_illuminant_cache()
{
    static cache::Cache<CameraAndIlluminantDescriptor, MatrixData>
        matrix_from_illuminant_cache( "matrix from illuminant" );
    return matrix_from_illuminant_cache;
} // LCOV_EXCL_LINE - bug in coverage tool

cache::Cache<MetadataDescriptor, MatrixData> &
get_matrix_from_dng_metadata_cache()
{
    static cache::Cache<MetadataDescriptor, MatrixData>
        matrix_from_dng_metadata_cache( "matrix from DNG metadata" );
    return matrix_from_dng_metadata_cache;
} // LCOV_EXCL_LINE - bug in coverage tool

} // namespace cache
} // namespace rta
