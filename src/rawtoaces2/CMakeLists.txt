cmake_minimum_required(VERSION 3.10)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_executable( rawtoaces2
    main.cpp
)

set_property(TARGET rawtoaces2 PROPERTY CXX_STANDARD 17)

target_include_directories( rawtoaces2
    PUBLIC
        ${OpenImageIO_INCLUDE_DIR}
)

target_link_libraries ( rawtoaces2
    PUBLIC
#        OpenImageIO::OpenImageIO
        rawtoaces_util2
)


set_target_properties(rawtoaces2 PROPERTIES XCODE_SCHEME_ARGUMENTS "${CMAKE_XCODE_SCHEME_ARGUMENTS}" )

install( TARGETS rawtoaces2 DESTINATION bin )
