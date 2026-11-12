// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces project.

#pragma once
#include <iostream>

int unit_test_failures = 0;

#define RTA_CHECK_ASSERT( x )                                                  \
    if ( !( x ) )                                                              \
    {                                                                          \
        std::cout << __FILE__ << ":" << __LINE__ << "\n";                      \
        std::cout << "FAILED: " << #x << "\n";                                 \
        unit_test_failures++;                                                  \
    }

#define RTA_CHECK_EQUAL( x, y )                                                \
    if ( ( x ) != ( y ) )                                                      \
    {                                                                          \
        std::cout << __FILE__ << ":" << __LINE__ << "\n";                      \
        std::cout << "FAILED: " << #x << "==" << #y << "\n";                   \
        unit_test_failures++;                                                  \
    }
