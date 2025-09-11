cmake_minimum_required(VERSION 3.10)

#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

enable_testing()

add_executable( test_installed_util
    test_installed_util.cpp
)

target_link_libraries(
    test_installed_util
    PUBLIC
        rawtoaces_util
        OpenImageIO::OpenImageIO
)

# Enable coverage for this test if coverage is enabled
if( ENABLE_COVERAGE AND COVERAGE_SUPPORTED )
    setup_coverage_flags(test_installed_util)
endif()

add_test ( NAME test_installed_util COMMAND test_installed_util )
