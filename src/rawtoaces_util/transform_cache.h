// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

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

extern cache::Cache<CameraAndIlluminantDescriptor, WBFromIlluminantData>
    WB_from_illuminant_cache;

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

extern cache::Cache<CameraAndWBDescriptor, IlluminantAndWBData>
    illuminant_from_WB_cache;

// -----------------------------------------------------------------------------
// Matrix from illuminant cache
// -----------------------------------------------------------------------------

using MatrixData = std::array<std::array<double, 3>, 3>;

extern cache::Cache<CameraAndIlluminantDescriptor, MatrixData>
    matrix_from_illuminant_cache;

} // namespace cache
} // namespace rta
