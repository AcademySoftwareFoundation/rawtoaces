cmake_minimum_required(VERSION 3.10)

#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

enable_testing()

add_executable( test_util
    util_test.cpp
)

target_link_libraries(
    test_util
    PUBLIC
        rawtoaces_util
        OpenImageIO::OpenImageIO
)

# Enable coverage for this test if coverage is enabled
if( ENABLE_COVERAGE AND COVERAGE_SUPPORTED )
    setup_coverage_flags(test_util)
endif()

add_test ( NAME test_util COMMAND test_util )
