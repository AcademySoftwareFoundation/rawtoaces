cmake_minimum_required(VERSION 3.10)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_library ( ${RAWTOACESLIB} ${DO_SHARED}
    acesrender.cpp
    
    # Make the headers visible in IDEs. This should not affect the builds.
    ../../include/rawtoaces/acesrender.h
)

set_property(TARGET ${RAWTOACESLIB} PROPERTY CXX_STANDARD 17)

target_include_directories ( ${RAWTOACESLIB} PRIVATE ${AcesContainer_INCLUDE_DIRS} )
target_link_directories    ( ${RAWTOACESLIB} PUBLIC ${AcesContainer_LIBRARY_DIRS} )
target_link_libraries      ( ${RAWTOACESLIB}
    PUBLIC
        ${AcesContainer_LIBRARIES}
        ${AcesContainer_LDFLAGS_OTHER}
)
 
target_link_libraries ( ${RAWTOACESLIB}
    PUBLIC
        ${RAWTOACESIDTLIB}
    PRIVATE
        Boost::filesystem
        Imath::Imath
        Imath::ImathConfig
)
    
if ( LIBRAW_CONFIG_FOUND )
    target_link_libraries ( ${RAWTOACESLIB} PUBLIC libraw::raw )
else ()
    target_include_directories(${RAWTOACESLIB} PUBLIC ${libraw_INCLUDE_DIR}/..)
    target_link_directories(${RAWTOACESLIB} PUBLIC ${libraw_LIBRARY_DIRS} )
    target_link_libraries(${RAWTOACESLIB} PUBLIC ${libraw_LIBRARIES} ${libraw_LDFLAGS_OTHER} )
endif ()


set_target_properties( ${RAWTOACESLIB} PROPERTIES
  SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
  VERSION ${RAWTOACES_VERSION} )

install(FILES
  ${PROJECT_SOURCE_DIR}/include/rawtoaces/acesrender.h	 	
 	DESTINATION include/rawtoaces
)

install( TARGETS ${RAWTOACESLIB} DESTINATION lib )
