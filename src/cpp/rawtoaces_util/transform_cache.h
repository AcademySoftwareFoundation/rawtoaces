// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <rawtoaces/rawtoaces_core.h>

#include "cache_base.h"

namespace rta
{
namespace cache
{

// -----------------------------------------------------------------------------
// WB from illuminant cache
// -----------------------------------------------------------------------------

using CameraAndIlluminantDescriptor = std::tuple<
    std::string, // camera make
    std::string, // camera model
    std::string  // illuminant
    >;

using WBFromIlluminantData = std::array<double, 3>;

cache::Cache<CameraAndIlluminantDescriptor, WBFromIlluminantData> &
get_WB_from_illuminant_cache();

// -----------------------------------------------------------------------------
// Illuminant from WB cache
// -----------------------------------------------------------------------------

using CameraAndWBDescriptor = std::tuple<
    std::string,          // camera make
    std::string,          // camera model
    std::array<double, 3> // white balancing weights
    >;

using IlluminantAndWBData = std::pair<
    std::string,          // illuminant
    std::array<double, 3> // white balancing weights
    >;

cache::Cache<CameraAndWBDescriptor, IlluminantAndWBData> &
get_illuminant_from_WB_cache();

// -----------------------------------------------------------------------------
// Matrix from illuminant cache
// -----------------------------------------------------------------------------

using MatrixData = std::array<std::array<double, 3>, 3>;

cache::Cache<CameraAndIlluminantDescriptor, MatrixData> &
get_matrix_from_illuminant_cache();

// -----------------------------------------------------------------------------
// Matrix from DNG metadata
// -----------------------------------------------------------------------------

using MetadataDescriptor = rta::core::Metadata;

using MatrixData = std::array<std::array<double, 3>, 3>;

cache::Cache<MetadataDescriptor, MatrixData> &
get_matrix_from_dng_metadata_cache();

} // namespace cache
} // namespace rta
