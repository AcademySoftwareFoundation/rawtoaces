# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the rawtoaces project.

# Documentation build configuration for RAWtoACES
#
# This file provides CMake targets for building documentation locally.
# For Read the Docs builds, the documentation is built directly using
# Sphinx and Doxygen without CMake.

find_package(Doxygen)
find_program(SPHINX_EXECUTABLE sphinx-build)
find_package(Python3 COMPONENTS Interpreter QUIET)

if(DOXYGEN_FOUND AND SPHINX_EXECUTABLE AND Python3_Interpreter_FOUND)
    message(STATUS "Documentation tools found - enabling 'docs' target")

    set(RAWTOACES_DOCS_VERSION "" CACHE STRING "Documentation checkout label (empty selects tag or main development)")
    if(NOT RAWTOACES_DOCS_VERSION)
        execute_process(
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build_scripts/build_docs.py
                --repo-root ${CMAKE_SOURCE_DIR} --print-version
            OUTPUT_VARIABLE RAWTOACES_DOCS_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE DOCS_VERSION_RESULT
        )
        if(NOT DOCS_VERSION_RESULT EQUAL 0)
            message(FATAL_ERROR "Could not determine documentation checkout label")
        endif()
    endif()

    # Configure Doxyfile with project paths
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )

    # Doxygen target - generates XML for Breathe
    add_custom_target(doxygen
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build_scripts/build_docs.py
            --repo-root ${CMAKE_SOURCE_DIR}
            --version ${RAWTOACES_DOCS_VERSION}
            --doxygen-output ${CMAKE_CURRENT_BINARY_DIR}/doxygen
            --doxygen-config ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            --doxygen-executable ${DOXYGEN_EXECUTABLE}
            --doxygen-only
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    # Sphinx target - generates HTML documentation
    add_custom_target(sphinx
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build_scripts/build_docs.py
            --repo-root ${CMAKE_SOURCE_DIR}
            --output-dir ${CMAKE_CURRENT_BINARY_DIR}/html
            --version ${RAWTOACES_DOCS_VERSION}
            --doxygen-output ${CMAKE_CURRENT_BINARY_DIR}/doxygen
            --skip-doxygen
            --sphinx-build ${SPHINX_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating HTML documentation with Sphinx"
        DEPENDS doxygen
        VERBATIM
    )

    # Main docs target
    add_custom_target(docs
        DEPENDS sphinx
        COMMENT "Building documentation"
    )

    # Install documentation
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
        DESTINATION share/doc/rawtoaces
        OPTIONAL
        COMPONENT documentation
    )

else()
    if(NOT DOXYGEN_FOUND)
        message(STATUS "Doxygen not found - documentation target disabled")
    endif()
    if(NOT SPHINX_EXECUTABLE)
        message(STATUS "Sphinx not found - documentation target disabled")
    endif()
    if(NOT Python3_Interpreter_FOUND)
        message(STATUS "Python not found - documentation target disabled")
    endif()
endif()
