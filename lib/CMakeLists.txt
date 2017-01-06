include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${CERES_INCLUDE_DIRS}")

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

add_library( rawtoacesCAT SHARED
   cat.cpp 
)

if ( ceres_FOUND )
  target_link_libraries( rawtoacesCAT ${CERES_LIBRARIES} )
  target_link_libraries( rawtoacesCAT ${CERES_LDFLAGS_OTHER} )
endif()

install( TARGETS rawtoacesCAT DESTINATION lib )

