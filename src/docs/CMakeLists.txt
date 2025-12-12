# Documentation build configuration for RAWtoACES
#
# This file provides CMake targets for building documentation locally.
# For Read the Docs builds, the documentation is built directly using
# Sphinx and Doxygen without CMake.

find_package(Doxygen)
find_program(SPHINX_EXECUTABLE sphinx-build)

if(DOXYGEN_FOUND AND SPHINX_EXECUTABLE)
    message(STATUS "Documentation tools found - enabling 'docs' target")

    # Configure Doxyfile with project paths
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )

    # Doxygen target - generates XML for Breathe
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    # Sphinx target - generates HTML documentation
    add_custom_target(sphinx
        COMMAND ${SPHINX_EXECUTABLE}
            -b html
            -c ${CMAKE_CURRENT_SOURCE_DIR}
            -D breathe_projects.rawtoaces=${CMAKE_BINARY_DIR}/docs/doxygen/xml
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}/html
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
endif()
