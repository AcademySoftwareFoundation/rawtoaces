include_directories( "${CMAKE_CURRENT_SOURCE_DIR}"
                     "${CERES_INCLUDE_DIRS}" )

if ( Boost_FOUND )
  add_definitions( -DHAVE_Boost=1 )
  include_directories( ${Boost_INCLUDE_DIRS} )
  link_directories( ${Boost_LIBRARY_DIRS} ) 
endif()

add_library( rawtoacesIDT SHARED
    	     rta.cpp 
)

target_link_libraries( rawtoacesIDT 
                       ${Boost_LIBRARIES}
		               ${CERES_LIBRARIES} )

install(FILES
	 define.h
	 mathOps.h
     rta.h	
    	
 	DESTINATION include/rawtoaces/include
)

