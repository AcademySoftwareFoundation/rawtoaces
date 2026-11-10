// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <string>

namespace rta
{
namespace helpers
{

/// Parse a float from a string without throwing.
///
/// Unlike `std::stof`, this never throws: a non-numeric input simply
/// returns `false`.
///
/// @param str  String to parse.
/// @param out  Destination filled on success.
/// @return `true` if a float could be parsed from the start of `str`.
bool parse_float( const std::string &str, float &out );

} // namespace helpers
} // namespace rta
