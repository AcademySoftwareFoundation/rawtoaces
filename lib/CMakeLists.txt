include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${CERES_INCLUDE_DIRS}")

if ( UNIX AND NOT APPLE )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

add_library( rawtoacesIDT SHARED
    rta.cpp 
)

if ( Ceres_FOUND )
  target_link_libraries( rawtoacesIDT ${CERES_LIBRARIES} )
endif()

install( FILES
	 	 define.h
		 mathOps.h
         rta.h	
    DESTINATION include/RAWTOACES )

install( TARGETS rawtoacesIDT DESTINATION lib )

