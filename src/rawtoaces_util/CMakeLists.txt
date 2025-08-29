cmake_minimum_required(VERSION 3.12)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

set( UTIL_PUBLIC_HEADER
    ../../include/rawtoaces/image_converter.h
    ../../include/rawtoaces/usage_timer.h
)

add_library ( ${RAWTOACES_UTIL_LIB} ${DO_SHARED}
    image_converter.cpp
    usage_timer.cpp

    # Make the headers visible in IDEs. This should not affect the builds.
    ${UTIL_PUBLIC_HEADER}
)
 
target_link_libraries ( ${RAWTOACES_UTIL_LIB}
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        OpenImageIO::OpenImageIO
)

target_compile_definitions( rawtoaces_util PRIVATE RAWTOACES_VERSION="${RAWTOACES_VERSION}" )

target_include_directories( ${RAWTOACES_UTIL_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:./include>
)

set_target_properties( ${RAWTOACES_UTIL_LIB} PROPERTIES
    OUTPUT_NAME "rawtoaces_util"
    EXPORT_NAME "rawtoaces_util"
    PUBLIC_HEADER "${UTIL_PUBLIC_HEADER}"
    SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
    VERSION ${RAWTOACES_VERSION}
)

install( TARGETS ${RAWTOACES_UTIL_LIB}
    EXPORT RAWTOACESTargets
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION include/rawtoaces
)
