include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

if ( IlmBase_FOUND )
  add_definitions( -DHAVE_IlmBase=1 )
  include_directories( ${IlmBase_INCLUDE_DIRS} )
endif()

if ( Ceres_FOUND )
  add_definitions( -DHAVE_Ceres=1 )
  include_directories( ${CERES_INCLUDE_DIRS} )
  link_directories( ${CERES_LIBRARY_DIRS} ) 
endif()

if ( AcesContainer_FOUND )
  add_definitions( -DHAVE_ACESFILE=1 )
  include_directories( ${AcesContainer_INCLUDE_DIRS} )
  link_directories( ${AcesContainer_LIBRARY_DIRS} )
endif()

add_library( ${RAWTOACESLIB} SHARED
    	       acesrender.cpp 
)

target_link_libraries( ${RAWTOACESLIB} 
                       ${RAWTOACESIDTLIB} )

if ( IlmBase_FOUND )
 target_link_libraries( ${RAWTOACESLIB} ${IlmBase_LIBRARIES} )
 target_link_libraries( ${RAWTOACESLIB} ${IlmBase_LDFLAGS_OTHER} )
endif()

if ( AcesContainer_FOUND )
  target_link_libraries( ${RAWTOACESLIB} ${AcesContainer_LIBRARIES} )
  target_link_libraries( ${RAWTOACESLIB} ${AcesContainer_LDFLAGS_OTHER} )
endif()

install(FILES
	acesrender.h	
    	
 	DESTINATION include/rawtoaces/include
)

install( TARGETS ${RAWTOACESLIB} DESTINATION lib )
