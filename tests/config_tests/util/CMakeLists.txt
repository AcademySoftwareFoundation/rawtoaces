# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces project.

cmake_minimum_required(VERSION 3.12)

enable_testing()

find_package ( RAWTOACES CONFIG REQUIRED COMPONENTS rawtoaces_util )

add_executable( test_installed_util
    test_installed_util.cpp
)

target_link_libraries(
    test_installed_util
    PUBLIC
        rawtoaces_util
)

add_test ( NAME test_installed_util COMMAND test_installed_util )
