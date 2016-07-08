if ( LibRAW_FOUND )
  add_definitions (-DHAVE_LIBRAW=1 )
  include_directories( ${libRaw_INCLUDE_DIRS} )
  link_directories( ${LibRaw_LIBRARY_DIRS} )
endif() 

if ( OpenEXR_FOUND )
	add_definitions( -DHAVE_OPENEXR=1 )
	include_directories( ${OpenEXR_INCLUDE_DIRS} )
	link_directories( ${OpenEXR_LIBRARY_DIRS} )
endif()

if ( AcesContainer_FOUND )
	add_definitions( -DHAVE_ACESFILE=1 )
	include_directories( ${AcesContainer_INCLUDE_DIRS} )
	link_directories( ${AcesContainer_LIBRARY_DIRS} )
endif()

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/lib/IlmCtl" "${PROJECT_SOURCE_DIR}/lib/IlmCtlMath" "${PROJECT_SOURCE_DIR}/lib/IlmCtlSimd" "${PROJECT_SOURCE_DIR}/lib/dpx" )

add_executable( rawtoaces
  rawtoACES.cpp
)

target_link_libraries( rawtoaces ${IlmBase_LIBRARIES} )
target_link_libraries( rawtoaces ${IlmBase_LDFLAGS_OTHER} )
if ( OpenEXR_FOUND )
  target_link_libraries( rawtoaces ${OpenEXR_LIBRARIES} )
  target_link_libraries( rawtoaces ${OpenEXR_LDFLAGS_OTHER} )
endif()
if ( AcesContainer_FOUND )
  target_link_libraries( rawtoaces ${AcesContainer_LIBRARIES} )
  target_link_libraries( rawtoaces ${AcesContainer_LDFLAGS_OTHER} )
endif()

install( TARGETS rawtoaces DESTINATION bin )
