// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "test_utils.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <streambuf>

/// RAII helper class to capture stderr output for testing
class StderrCapture
{
public:
    StderrCapture() : buffer(), old( std::cerr.rdbuf( buffer.rdbuf() ) ) {}

    ~StderrCapture() { std::cerr.rdbuf( old ); }

    /// Get the captured output as a string
    std::string str() const { return buffer.str(); }

private:
    std::stringstream buffer;
    std::streambuf   *old;
};

/// Wrapper function that captures stderr output from a callable action
std::string capture_stderr( std::function<void()> action )
{
    StderrCapture capture;
    action();
    return capture.str();
}
