cmake_minimum_required(VERSION 3.5)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

set( IDT_PUBLIC_HEADER
    ../../include/rawtoaces/define.h
    ../../include/rawtoaces/mathOps.h
    ../../include/rawtoaces/rta.h
)

add_library( ${RAWTOACESIDTLIB} ${DO_SHARED}
    rta.cpp

    # Make the headers visible in IDEs. This should not affect the builds.
    ${IDT_PUBLIC_HEADER}
)

target_link_libraries(
    ${RAWTOACESIDTLIB}
    PUBLIC
        Boost::boost
        Boost::system
        Imath::Imath
        Imath::ImathConfig
        Eigen3::Eigen
)

if ( ${Ceres_VERSION_MAJOR} GREATER 1 )
    target_link_libraries( ${RAWTOACESIDTLIB} PUBLIC Ceres::ceres )
else ()
    target_include_directories(${RAWTOACESIDTLIB} PUBLIC ${CERES_INCLUDE_DIRS})
    target_link_libraries(${RAWTOACESIDTLIB} PUBLIC ${CERES_LIBRARIES})
endif ()

target_include_directories( ${RAWTOACESIDTLIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:./include>
)

set_target_properties( ${RAWTOACESIDTLIB} PROPERTIES
    OUTPUT_NAME "rawtoaces_idt"
    EXPORT_NAME "rawtoaces_idt"
    PUBLIC_HEADER "${IDT_PUBLIC_HEADER}"
    SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
    VERSION ${RAWTOACES_VERSION}
)

install( TARGETS ${RAWTOACESIDTLIB}
    EXPORT RAWTOACESTargets
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION include/rawtoaces
)
