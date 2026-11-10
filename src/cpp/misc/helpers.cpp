// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "helpers.h"

#include <charconv>

namespace rta
{
namespace helpers
{

bool parse_float( const std::string &str, float &out )
{
    auto [ptr, ec] =
        std::from_chars( str.data(), str.data() + str.size(), out );
    return ec == std::errc();
}

} // namespace helpers
} // namespace rta
