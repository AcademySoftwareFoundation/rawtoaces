# Until we get some of these modules into the upstream packages, put them here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/share/CMake")

find_package ( nlohmann_json CONFIG REQUIRED )
find_package ( OpenImageIO   CONFIG REQUIRED )
find_package ( Eigen3        CONFIG REQUIRED )
#find_package ( Imath         CONFIG REQUIRED )
find_package ( Boost         CONFIG REQUIRED
    COMPONENTS
        system
        unit_test_framework
)

if (RTA_CENTOS7_CERES_HACK)
    find_package ( Ceres MODULE REQUIRED )
else ()
    find_package ( Ceres CONFIG REQUIRED )
endif ()
