cmake_minimum_required(VERSION 3.12)

add_executable( rawtoaces
    main.cpp
)

target_link_libraries ( rawtoaces
    PUBLIC
        ${RAWTOACES_UTIL_LIB}
)

install( TARGETS rawtoaces DESTINATION ${INSTALL_BIN_DIR} )
