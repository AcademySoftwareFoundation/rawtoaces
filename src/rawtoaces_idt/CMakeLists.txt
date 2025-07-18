cmake_minimum_required(VERSION 3.5)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_library( ${RAWTOACESIDTLIB} ${DO_SHARED}
    rta.cpp

    # Make the headers visible in IDEs. This should not affect the builds.
    ../../include/rawtoaces/define.h
    ../../include/rawtoaces/mathOps.h
    ../../include/rawtoaces/rta.h
)

target_link_libraries(
    ${RAWTOACESIDTLIB}
    PUBLIC
        Boost::boost
        Boost::system
        Boost::filesystem
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


set_target_properties( ${RAWTOACESIDTLIB} PROPERTIES
    SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
    VERSION ${RAWTOACES_VERSION}
)

install(FILES
    ${PROJECT_SOURCE_DIR}/include/rawtoaces/define.h
    ${PROJECT_SOURCE_DIR}/include/rawtoaces/mathOps.h
    ${PROJECT_SOURCE_DIR}/include/rawtoaces/rta.h

 	DESTINATION include/rawtoaces
)

install( TARGETS ${RAWTOACESIDTLIB} DESTINATION ${INSTALL_LIB_DIR} )
