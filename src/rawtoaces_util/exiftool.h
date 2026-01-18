// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#pragma once

#include <OpenImageIO/imagebuf.h>

namespace rta
{
namespace util
{
namespace exiftool
{

/// Fetch the metadata for the given attribute names from the image file using ExifTool, populate the
/// fetched metadata into the given ImageSpec.
/// @param spec
///     Destination image buffer's header to update with the fetched metadata.
/// @param path
///     Path to the raw image file to fetch metadata from.
/// @param keys
///     List of attribute names to fetch metadata for.
/// @return
///    `true` if fetched successfully.
bool fetch_metadata(
    OIIO::ImageSpec                &spec,
    const std::string              &path,
    const std::vector<std::string> &keys );

} // namespace exiftool
} // namespace util
} // namespace rta
