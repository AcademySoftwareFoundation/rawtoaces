// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <OpenImageIO/imagebuf.h>

#include "cache_base.h"

namespace rta
{
namespace cache
{

// -----------------------------------------------------------------------------
// Vignette cache
// -----------------------------------------------------------------------------

using LensDescriptor = OIIO::ImageSpec;

using ImageBufData = OIIO::ImageBuf;

cache::Cache<LensDescriptor, ImageBufData> &get_vignette_cache();

cache::Cache<LensDescriptor, ImageBufData> &get_distortion_cache();

cache::Cache<LensDescriptor, ImageBufData> &get_aberration_cache();

} // namespace cache
} // namespace rta
