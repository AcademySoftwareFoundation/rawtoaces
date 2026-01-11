// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "transform_cache.h"

namespace rta
{
namespace cache
{

cache::Cache<CameraAndIlluminantDescriptor, WBFromIlluminantData>
    WB_from_illuminant_cache( "WB from illuminant" );

cache::Cache<CameraAndWBDescriptor, IlluminantAndWBData>
    illuminant_from_WB_cache( "illuminant from WB" );

cache::Cache<CameraAndIlluminantDescriptor, MatrixData>
    matrix_from_illuminant_cache( "matrix from illuminant" );

} // namespace cache
} // namespace rta
