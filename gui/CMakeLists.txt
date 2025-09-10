cmake_minimum_required(VERSION 3.16)

project(RAWtoACES_GUI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Concurrent
    Network
)

# Automatically handle Qt's MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find RAWtoACES libraries
find_package(RAWTOACES REQUIRED)

# Optional: LibRaw for RAW previews
find_package(libraw QUIET)

include(CTest)
option(RAWTOACES_GUI_BUILD_TESTS "Build GUI unit tests" ON)

# Application sources
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/FileListWidget.cpp
    src/ParameterWidget.cpp
    src/ConversionWorker.cpp
    src/ImageViewer.cpp
    src/ProgressDialog.cpp
    src/SettingsManager.cpp
    src/utils/FileUtils.cpp
    src/utils/ImageUtils.cpp
    src/utils/RawPreview.cpp
)

set(HEADERS
    src/MainWindow.h
    src/FileListWidget.h
    src/ParameterWidget.h
    src/ConversionWorker.h
    src/ImageViewer.h
    src/ProgressDialog.h
    src/SettingsManager.h
    src/utils/FileUtils.h
    src/utils/ImageUtils.h
    src/utils/RawPreview.h
)

set(RESOURCES
    resources/icons.qrc
)

# Create executable
add_executable(rawtoaces-gui
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link libraries
target_link_libraries(rawtoaces-gui
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Network
    rawtoaces_core
    rawtoaces_util
)

if(libraw_FOUND)
    target_compile_definitions(rawtoaces-gui PRIVATE USE_LIBRAW)
    target_include_directories(rawtoaces-gui PRIVATE ${libraw_INCLUDE_DIRS})
    target_link_libraries(rawtoaces-gui ${libraw_LIBRARIES})
endif()

# Include directories
target_include_directories(rawtoaces-gui PRIVATE
    src
    include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Installation
install(TARGETS rawtoaces-gui
    RUNTIME DESTINATION bin
)

# macOS app bundle
if(APPLE)
    set_target_properties(rawtoaces-gui PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
endif()

if(RAWTOACES_GUI_BUILD_TESTS)
    add_subdirectory(tests)
endif()
