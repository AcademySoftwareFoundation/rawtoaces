cmake_minimum_required(VERSION 3.10)

#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

enable_testing()

add_executable( test_core
    core_test.cpp
)

target_link_libraries(
    test_core
    PUBLIC
        rawtoaces_core
        OpenImageIO::OpenImageIO
)

add_test ( NAME test_core COMMAND test_core )
