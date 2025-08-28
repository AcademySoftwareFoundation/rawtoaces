cmake_minimum_required(VERSION 3.12)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

set( CORE_PUBLIC_HEADER
    ../../include/rawtoaces/define.h
    ../../include/rawtoaces/mathOps.h
    ../../include/rawtoaces/rawtoaces_core.h
    ../../include/rawtoaces/spectral_data.h
)

add_library( ${RAWTOACES_CORE_LIB} ${DO_SHARED}
    rawtoaces_core.cpp
    spectral_data.cpp

    # Make the headers visible in IDEs. This should not affect the builds.
    ${CORE_PUBLIC_HEADER}
    rawtoaces_core_priv.h
)

target_link_libraries(
    ${RAWTOACES_CORE_LIB}
    PUBLIC
        Boost::boost
        Boost::system
        Imath::Imath
        Imath::ImathConfig
        Eigen3::Eigen
)

if ( ${Ceres_VERSION_MAJOR} GREATER 1 )
    target_link_libraries( ${RAWTOACES_CORE_LIB} PUBLIC Ceres::ceres )
else ()
    target_include_directories(${RAWTOACES_CORE_LIB} PUBLIC ${CERES_INCLUDE_DIRS})
    target_link_libraries(${RAWTOACES_CORE_LIB} PUBLIC ${CERES_LIBRARIES})
endif ()

target_include_directories( ${RAWTOACES_CORE_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:./include>
)

set_target_properties( ${RAWTOACES_CORE_LIB} PROPERTIES
    OUTPUT_NAME "rawtoaces_core"
    EXPORT_NAME "rawtoaces_core"
    PUBLIC_HEADER "${CORE_PUBLIC_HEADER}"
    SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
    VERSION ${RAWTOACES_VERSION}
)

install( TARGETS ${RAWTOACES_CORE_LIB}
    EXPORT RAWTOACESTargets
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION include/rawtoaces
)
