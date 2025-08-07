cmake_minimum_required(VERSION 3.5)

add_executable( rawtoaces
    main.cpp
)

target_link_libraries ( rawtoaces
    PUBLIC
        ${RAWTOACESLIB}
    INTERFACE
        Boost::headers
)

if ( LIBRAW_CONFIG_FOUND )
    target_link_libraries ( rawtoaces PUBLIC libraw::raw )
else ()
    target_link_directories(rawtoaces PUBLIC ${libraw_LIBRARY_DIRS} )
    target_link_libraries(rawtoaces PUBLIC ${libraw_LIBRARIES} ${libraw_LDFLAGS_OTHER} )
endif ()


install( TARGETS rawtoaces DESTINATION ${INSTALL_BIN_DIR} )