// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

// clang-format off
#ifdef WIN32
#    define DISABLE_DEPRECATED_WARNINGS                                        \
        _Pragma( "warning ( push )" )                                          \
        _Pragma( "warning ( disable: 4996 )" )
#    define ENABLE_WARNINGS                                                    \
        _Pragma( "warning ( pop )" )
#else
#    define DISABLE_DEPRECATED_WARNINGS                                        \
        _Pragma( "GCC diagnostic push" )                                       \
        _Pragma( "GCC diagnostic ignored \"-Wdeprecated-declarations\"" )
#    define ENABLE_WARNINGS                                                    \
        _Pragma( "GCC diagnostic pop" )
#endif
// clang-format on
