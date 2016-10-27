include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_library( rawtoacesIDT SHARED
   idt.cpp 
)
