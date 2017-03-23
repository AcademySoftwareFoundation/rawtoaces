
# find_package (Boost REQUIRED)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" 
		      ${Boost_INCLUDE_DIRS}
		     "${CERES_INCLUDE_DIRS}"
)

if ( UNIX AND NOT APPLE )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
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
    	
 	DESTINATION include/rawtoaces
       )

install( TARGETS rawtoacesIDT DESTINATION lib )

