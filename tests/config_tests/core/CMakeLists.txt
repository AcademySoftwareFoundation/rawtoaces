# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces project.

cmake_minimum_required(VERSION 3.12)

enable_testing()

find_package ( RAWTOACES CONFIG REQUIRED COMPONENTS rawtoaces_core )

add_executable( test_installed_core
    test_installed_core.cpp
)

target_link_libraries(
    test_installed_core
    PUBLIC
        rawtoaces_core
)

add_test ( NAME test_installed_core COMMAND test_installed_core )
