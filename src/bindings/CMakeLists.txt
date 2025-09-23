cmake_minimum_required ( VERSION 3.12 )
include_directories ( "${CMAKE_CURRENT_SOURCE_DIR}" )

nanobind_add_module( rawtoaces_bindings
    py_util.cpp py_util.h
    py_core.cpp py_core.h
    py_module.cpp
)

target_link_libraries( rawtoaces_bindings
    PUBLIC
        ${RAWTOACES_UTIL_LIB}
)

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
    target_compile_options ( nanobind-static    PRIVATE -Wno-pedantic )
    target_compile_options ( rawtoaces_bindings PRIVATE -Wno-pedantic )
    if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 10.0 )
        target_compile_options ( nanobind-static    PRIVATE -Wno-error=zero-length-bounds )
        target_compile_options ( rawtoaces_bindings PRIVATE -Wno-error=zero-length-bounds )
    endif ()
elseif ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" )
    target_compile_options ( nanobind-static    PRIVATE -Wno-error=zero-length-array )
    target_compile_options ( rawtoaces_bindings PRIVATE -Wno-error=zero-length-array )
endif()

set_target_properties( rawtoaces_bindings PROPERTIES
    OUTPUT_NAME "rawtoaces"
    #EXPORT_NAME "rawtoaces"
    #SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
    #VERSION ${RAWTOACES_VERSION}
)

install(
    TARGETS rawtoaces_bindings
    EXPORT RAWTOACESTargets
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION include/rawtoaces
)
