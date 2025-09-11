cmake_minimum_required(VERSION 3.10)

#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

enable_testing()

add_executable( test_installed_core
    test_installed_core.cpp
)

target_link_libraries(
    test_installed_core
    PUBLIC
        rawtoaces_core
        OpenImageIO::OpenImageIO
)

# Enable coverage for this test if coverage is enabled
if( ENABLE_COVERAGE AND COVERAGE_SUPPORTED )
    setup_coverage_flags(test_installed_core)
endif()

add_test ( NAME test_installed_core COMMAND test_installed_core )
