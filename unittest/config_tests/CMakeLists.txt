
#
# This file tests the cmake config files being exported/installed by rawtoaces.
# It simulates a project using the rawtoaces library as a dependency.
#
# DO NOT include it into a parent CMakeLists.txt.
#
# Usage: 
#    cmake -S . -B build -D rawtoaces_DIR=PATH_TO_INSTALL_DIR 

cmake_minimum_required(VERSION 3.10)
project( rawtoaces_config_test )


# Until we get some of these modules into the upstream packages, put them here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../../cmake/modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/share/CMake")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options ( /W0 )
    add_compile_options ( /utf-8 )
    add_compile_definitions( NOMINMAX )
endif()

find_package ( Boost REQUIRED
    COMPONENTS
        system
        filesystem
        unit_test_framework
)

# the Ceres::ceres alias in not getting defined on the config side
find_package ( Ceres CONFIG REQUIRED )

# the find_dependency() on the config side doesn't seem to work on Windows
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    find_package (libraw CONFIG QUIET )
    if (libraw_FOUND )
        set ( LIBRAW_CONFIG_FOUND TRUE )
    else ()
        find_package(libraw MODULE REQUIRED)
    endif ()
endif ()

find_package(RAWTOACES CONFIG REQUIRED)

enable_testing()
add_subdirectory(core)
add_subdirectory(util)
