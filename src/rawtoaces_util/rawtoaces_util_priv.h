// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

// Contains the declarations of the private functions,
// exposed here for unit-testing.

namespace rta
{
namespace util
{

/// Collects image files from a given path (file or directory) into batches.
///
/// This function processes either a single file or a directory containing image files.
/// For directories, it iterates through all files and adds valid image files to batches.
/// For single files, it adds the file to the first batch if it's valid.
///
/// @param path The path to process (file or directory)
/// @param batches Reference to a vector of batches to populate with valid file paths
/// @return true if the path was processed successfully, false otherwise
bool collect_image_files(
    const std::string &path, std::vector<std::vector<std::string>> &batches );

} // namespace util
} // namespace rta
