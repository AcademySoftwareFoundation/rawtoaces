cmake_minimum_required(VERSION 3.12)

# Include coverage support if enabled
if( ENABLE_COVERAGE )
    include( ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CodeCoverage.cmake )
endif()

add_executable( rawtoaces
    main.cpp
)

target_link_libraries ( rawtoaces
    PUBLIC
        ${RAWTOACES_UTIL_LIB}
)

# Enable coverage for this executable if coverage is enabled
if( ENABLE_COVERAGE AND COVERAGE_SUPPORTED )
    setup_coverage_flags(rawtoaces)
endif()

install( TARGETS rawtoaces DESTINATION ${INSTALL_BIN_DIR} )
