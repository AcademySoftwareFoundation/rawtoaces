cmake_minimum_required(VERSION 3.5)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

set( UTIL_PUBLIC_HEADER
    ../../include/rawtoaces/acesrender.h
)

add_library ( ${RAWTOACESLIB} ${DO_SHARED}
    acesrender.cpp

    # Make the headers visible in IDEs. This should not affect the builds.
    ${UTIL_PUBLIC_HEADER}
)

if ( AcesContainer_FOUND )
    target_include_directories ( ${RAWTOACESLIB} PRIVATE ${AcesContainer_INCLUDE_DIRS} )
    target_link_directories    ( ${RAWTOACESLIB} PUBLIC  ${AcesContainer_LIBRARY_DIRS} )
    target_link_libraries      ( ${RAWTOACESLIB}
        PUBLIC
            ${AcesContainer_LIBRARIES}
            ${AcesContainer_LDFLAGS_OTHER}
    )
endif()
 
target_link_libraries ( ${RAWTOACESLIB}
    PUBLIC
        ${RAWTOACESIDTLIB}
    INTERFACE
        Eigen3::Eigen
        Imath::Imath
        Imath::ImathConfig
)
    
if ( LIBRAW_CONFIG_FOUND )
    target_link_libraries ( ${RAWTOACESLIB} PUBLIC libraw::raw )
else ()
    target_include_directories(${RAWTOACESLIB} PUBLIC ${libraw_INCLUDE_DIR} )
    target_link_directories(${RAWTOACESLIB} PUBLIC ${libraw_LIBRARY_DIRS} )
    target_link_libraries(${RAWTOACESLIB} PUBLIC ${libraw_LIBRARIES} ${libraw_LDFLAGS_OTHER} )
endif ()

set_target_properties( ${RAWTOACESLIB} PROPERTIES
    OUTPUT_NAME "rawtoaces_util"
    EXPORT_NAME "rawtoaces_util"
    PUBLIC_HEADER "${UTIL_PUBLIC_HEADER}"
    SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
    VERSION ${RAWTOACES_VERSION}
)

install( TARGETS ${RAWTOACESLIB}
    EXPORT RAWTOACESTargets
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION include/rawtoaces
)
