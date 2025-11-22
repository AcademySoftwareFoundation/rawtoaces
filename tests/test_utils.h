// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <functional>
#include <string>

/// Wrapper function that captures stderr output from a callable action
/// @param action A callable (function, lambda, etc.) that may write to stderr
/// @return The captured stderr output as a string
std::string capture_stderr( std::function<void()> action );
