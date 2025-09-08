# Copyright Contributors to the rawtoaces Project.
# SPDX-License-Identifier: Apache-2.0

# This module provides functionality to set up code coverage reporting using gcov + lcov

# Check if coverage is supported
function(check_coverage_support)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND 
       NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND 
       NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        message(WARNING "Code coverage is only supported for GCC, Clang, and AppleClang compilers")
        return()
    endif()
    
    # Check if gcov is available
    find_program(GCOV_PATH gcov)
    if(NOT GCOV_PATH)
        message(WARNING "gcov not found. Code coverage will not be available.")
        return()
    endif()
    
    # Check if lcov is available
    find_program(LCOV_PATH lcov)
    if(NOT LCOV_PATH)
        message(WARNING "lcov not found. HTML coverage reports will not be available.")
        return()
    endif()
    
    # Check if genhtml is available
    find_program(GENHTML_PATH genhtml)
    if(NOT GENHTML_PATH)
        message(WARNING "genhtml not found. HTML coverage reports will not be available.")
        return()
    endif()
    
    set(COVERAGE_SUPPORTED TRUE PARENT_SCOPE)
    message(STATUS "Code coverage support enabled")
endfunction()

# Set up coverage flags
function(setup_coverage_flags target_name)
    if(NOT COVERAGE_SUPPORTED)
        return()
    endif()
    
    # Use gcov for all compilers (including AppleClang)
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
        $<$<CONFIG:Debug>:-g -O0>
    )
    
    target_link_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
    )
    
    # Add coverage data directory to RPATH for proper .gcda file generation
    set_target_properties(${target_name} PROPERTIES
        BUILD_RPATH_USE_ORIGIN TRUE
    )
endfunction()

# Generate coverage report
function(generate_coverage_report)
    if(NOT COVERAGE_SUPPORTED)
        message(WARNING "Coverage is not supported or not enabled")
        return()
    endif()
    
    # Find all test executables
    get_property(test_list DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY TESTS)
    
    # Create coverage directory
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/coverage)
    
    # Use lcov for all compilers (including AppleClang with gcov)
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E echo "Running tests to generate coverage data..."
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure || true
        COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage report..."
        COMMAND ${LCOV_PATH} --ignore-errors inconsistent,unsupported,format,unused,corrupt --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
        COMMAND ${LCOV_PATH} --ignore-errors inconsistent,unsupported,format,unused,corrupt --extract ${CMAKE_BINARY_DIR}/coverage/coverage.info '*/src/rawtoaces_*' '*/include/rawtoaces/*' --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_temp.info
        COMMAND ${LCOV_PATH} --ignore-errors inconsistent,unsupported,format,unused,corrupt --remove ${CMAKE_BINARY_DIR}/coverage/coverage_temp.info '*/tests/*' --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info
        COMMAND ${GENHTML_PATH} --ignore-errors inconsistent,unsupported,corrupt,category ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info --output-directory ${CMAKE_BINARY_DIR}/coverage/html
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in ${CMAKE_BINARY_DIR}/coverage/html/index.html"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report"
    )
    
    
endfunction()

# Initialize coverage support
check_coverage_support()
