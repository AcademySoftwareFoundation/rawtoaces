// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <OpenImageIO/imageio.h>

// Contains the declarations of the private functions,
// exposed here for unit-testing.

namespace rta
{
namespace util
{

std::vector<std::string> database_paths();

void fix_metadata( OIIO::ImageSpec &spec );

} // namespace util
} // namespace rta
