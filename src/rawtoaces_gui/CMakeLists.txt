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

# macOS: build a real .app so Finder/Dock get bundle metadata; menu bar title
# follows setApplicationDisplayName in main.cpp (e.g. "Raw to ACES").
if(APPLE)
    set(RTA_GUI_MACOS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/macos/rawtoaces_gui.icns")
    set_target_properties(rawtoaces_gui PROPERTIES
        MACOSX_BUNDLE TRUE
        # Bundle folder and CFBundleExecutable (Linux/Windows keep target name: rawtoaces_gui).
        OUTPUT_NAME "Raw to ACES"
        MACOSX_BUNDLE_BUNDLE_NAME "Raw to ACES"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.aswf.rawtoaces.gui"
        MACOSX_BUNDLE_INFO_SHORT_VERSION_STRING "${RAWTOACES_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${RAWTOACES_VERSION}"
    )
    if(EXISTS "${RTA_GUI_MACOS_ICON}")
        target_sources(rawtoaces_gui PRIVATE "${RTA_GUI_MACOS_ICON}")
        set_source_files_properties("${RTA_GUI_MACOS_ICON}" PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
        )
        # Base name (no extension) must match Resources/*.icns; independent of OUTPUT_NAME.
        set_target_properties(rawtoaces_gui PROPERTIES
            MACOSX_BUNDLE_ICON_FILE "rawtoaces_gui"
        )
    endif()
endif()

if(APPLE)
    install(TARGETS rawtoaces_gui
        BUNDLE DESTINATION ${INSTALL_BIN_DIR}
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    )
else()
    install(TARGETS rawtoaces_gui DESTINATION ${INSTALL_BIN_DIR})
endif()
