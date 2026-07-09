
#
# This file tests the cmake config files being exported/installed by rawtoaces.
# It simulates a project using the rawtoaces library as a dependency.
#
# DO NOT include it into a parent CMakeLists.txt.
#
# Usage: 
#    cmake -S . -B build -D rawtoaces_DIR=PATH_TO_INSTALL_DIR 

cmake_minimum_required(VERSION 3.12)
project( rawtoaces_config_test )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options ( /W0 )
    add_compile_options ( /utf-8 )
    add_compile_definitions( NOMINMAX )
endif()

find_package ( RAWTOACES CONFIG REQUIRED )

enable_testing()
add_subdirectory(core)
add_subdirectory(util)
