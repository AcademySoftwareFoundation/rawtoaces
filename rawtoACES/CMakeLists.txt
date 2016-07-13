
if ( AcesContainer_FOUND )
	add_definitions( -DHAVE_ACESFILE=1 )
	include_directories( ${AcesContainer_INCLUDE_DIRS} )
	link_directories( ${AcesContainer_LIBRARY_DIRS} )
endif()

if ( libraw_FOUND )
  add_definitions (-DHAVE_LIBRAW=1 )
  include_directories( ${libraw_INCLUDE_DIRS} )
  link_directories( ${libraw_LIBRARY_DIRS} )
endif() 

if ( WIN32 AND NOT CYGWIN )
  include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/lib/half" "${PROJECT_SOURCE_DIR}/lib/libraw" "${PROJECT_SOURCE_DIR}/lib/aces_container" )
endif ()

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${IlmBase_INCLUDE_DIRS}" "${libraw_INCLUDE_DIRS}" )

add_executable( rawtoaces
  rawtoACES.cpp
)

target_link_libraries( rawtoaces ${IlmBase_LIBRARIES} )
target_link_libraries( rawtoaces ${IlmBase_LDFLAGS_OTHER} )
if ( AcesContainer_FOUND )
  target_link_libraries( rawtoaces ${AcesContainer_LIBRARIES} )
  target_link_libraries( rawtoaces ${AcesContainer_LDFLAGS_OTHER} )
endif()

if ( libraw_FOUND )
  target_link_libraries( rawtoaces ${libraw_LIBRARIES} )
  target_link_libraries( rawtoaces ${libraw_LDFLAGS_OTHER} )
endif()

install( TARGETS rawtoaces DESTINATION bin )
