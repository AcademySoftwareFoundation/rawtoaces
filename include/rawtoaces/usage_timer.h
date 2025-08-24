// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <string>

namespace rta
{
namespace util
{

/// A helper class for tracking and reporting execution time.
class UsageTimer
{
public:
    /// Set to `true` to enable tracking.
    bool enabled = false;

    /// Reset the usage timer.
    void reset();

    /// Print a message for a given path with the addition of the time
    /// passed since the last invocation of `reset()`.
    /// - parameter path: The file math to print.
    /// - parameter message: The message to print.
    void print( const std::string &path, const std::string &message );

private:
    double _start_time;
};

} //namespace util
} //namespace rta
