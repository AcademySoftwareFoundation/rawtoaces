cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(rawtoaces_gui_tests
    test_basic.cpp
    ../src/ParameterWidget.cpp
    ../src/utils/ImageUtils.cpp
    ../src/utils/RawPreview.cpp
    ../src/utils/FileUtils.cpp
)

target_include_directories(rawtoaces_gui_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(rawtoaces_gui_tests
    Qt6::Test
    Qt6::Widgets
)

add_test(NAME rawtoaces_gui_tests COMMAND rawtoaces_gui_tests)
