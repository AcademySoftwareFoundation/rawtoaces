// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces project.

#pragma once

#include <iostream>

inline int unit_test_failures = 0;

template <typename Actual, typename Expected>
inline void rta_check_equal(
    const Actual   &actual,
    const Expected &expected,
    const char     *expression,
    const char     *file,
    int             line )
{
    if ( actual != expected )
    {
        std::cout << file << ":" << line << "\n";
        std::cout << "FAILED: " << expression << " (" << actual << ") == ("
                  << expected << ")\n";
        unit_test_failures++;
    }
}

#define RTA_CHECK_ASSERT( x )                                                  \
    do                                                                         \
    {                                                                          \
        if ( !( x ) )                                                          \
        {                                                                      \
            std::cout << __FILE__ << ":" << __LINE__ << "\n";                  \
            std::cout << "FAILED: " << #x << "\n";                             \
            unit_test_failures++;                                              \
        }                                                                      \
    } while ( 0 )

#define RTA_CHECK_EQUAL( x, y )                                                \
    rta_check_equal( ( x ), ( y ), #x " == " #y, __FILE__, __LINE__ )
