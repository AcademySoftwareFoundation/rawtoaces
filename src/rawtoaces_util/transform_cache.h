// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <rawtoaces/rawtoaces_core.h>

// These need to be declared before including cache_base.h
namespace rta
{
std::ostream &operator<<( std::ostream &os, const rta::core::Metadata &data );
bool          operator==(
    const rta::core::Metadata &data1, const rta::core::Metadata &data2 );
} // namespace rta

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

// -----------------------------------------------------------------------------
// Matrix from DNG metadata
// -----------------------------------------------------------------------------

using MetadataDescriptor = rta::core::Metadata;

using MatrixData = std::array<std::array<double, 3>, 3>;

extern cache::Cache<MetadataDescriptor, MatrixData>
    matrix_from_dng_metadata_cache;

} // namespace cache
} // namespace rta
