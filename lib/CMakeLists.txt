include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

add_library( rawtoacesIDT SHARED
   idt.cpp 
)

install( TARGETS rawtoacesIDT DESTINATION lib )

