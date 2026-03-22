cmake_minimum_required(VERSION 3.16)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

qt_add_library(rawtoaces_gui_lib STATIC
    main_window.cpp
    conversion_thread.cpp
)

target_link_libraries(rawtoaces_gui_lib PUBLIC
    ${RAWTOACES_UTIL_LIB}
    Qt6::Widgets
)

target_include_directories(rawtoaces_gui_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(rawtoaces_gui_lib PUBLIC
    $<$<BOOL:${RTA_ENABLE_LENSFUN}>:RTA_GUI_HAS_LENSFUN=1>
)

qt_add_executable(rawtoaces_gui
    main.cpp
)

target_link_libraries(rawtoaces_gui PRIVATE rawtoaces_gui_lib)

if(WIN32)
    set_target_properties(rawtoaces_gui PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS rawtoaces_gui DESTINATION ${INSTALL_BIN_DIR})
