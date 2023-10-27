cmake_minimum_required(VERSION 3.1)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

if ( Boost_FOUND )
  add_definitions( -DHAVE_Boost=1 )
  include_directories( ${Boost_INCLUDE_DIRS} )
  link_directories( ${Boost_LIBRARY_DIRS} ) 
endif()


add_library( ${RAWTOACESIDTLIB} SHARED
    	     rta.cpp 
)

target_link_libraries( ${RAWTOACESIDTLIB} 
                       ${Boost_LIBRARIES}
		                   ${CERES_LIBRARIES} )

set_target_properties( ${RAWTOACESIDTLIB} PROPERTIES
  SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
  VERSION ${RAWTOACES_VERSION} )

install(FILES
        ${PROJECT_SOURCE_DIR}/include/rawtoaces/define.h
	      ${PROJECT_SOURCE_DIR}/include/rawtoaces/mathOps.h
        ${PROJECT_SOURCE_DIR}/include/rawtoaces/rta.h	

 	DESTINATION include/rawtoaces
)

install( TARGETS ${RAWTOACESIDTLIB} DESTINATION lib )
