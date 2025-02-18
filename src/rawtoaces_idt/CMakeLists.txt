cmake_minimum_required(VERSION 3.10)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_library( ${RAWTOACESIDTLIB} ${DO_SHARED}
    rta.cpp

    # Make the headers visible in IDEs. This should not affect the builds.
    ../../include/rawtoaces/define.h
    ../../include/rawtoaces/mathOps.h
    ../../include/rawtoaces/rta.h
    ../../include/rawtoaces/metadata.h
)

set_property(TARGET ${RAWTOACESIDTLIB} PROPERTY CXX_STANDARD 17)

# Eigen is also used in rawtoaces_util and rawtoaces_util2, so setting it public here
target_link_libraries(
    ${RAWTOACESIDTLIB}
    PUBLIC
        Eigen3::Eigen
    PRIVATE
        Boost::boost
        Boost::system
	    nlohmann_json::nlohmann_json
)

# Ceres is also used in rawtoaces_util and rawtoaces_util2, so setting it public here
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
    ${PROJECT_SOURCE_DIR}/include/rawtoaces/metadata.h
    DESTINATION include/rawtoaces
)

install( TARGETS ${RAWTOACESIDTLIB} DESTINATION lib )
