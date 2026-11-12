# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces project.

#
# This file tests the cmake config files being exported/installed by rawtoaces.
# It simulates a project using the rawtoaces library as a dependency.
#
# DO NOT include it into a parent CMakeLists.txt.
#
# Usage: 
#    cmake -S . -B build -D rawtoaces_DIR=PATH_TO_INSTALL_DIR 

cmake_minimum_required(VERSION 3.12)

if( NOT DEFINED CMAKE_CXX_STANDARD )
    set(CMAKE_CXX_STANDARD 17)
endif()

project( rawtoaces_config_test )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options ( /W0 )
    add_compile_options ( /utf-8 )
    add_compile_definitions( NOMINMAX )
endif()

enable_testing()
add_subdirectory(core)
add_subdirectory(util)
