# Until we get some of these modules into the upstream packages, put them here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/share/CMake")


find_package ( nlohmann_json CONFIG REQUIRED )
find_package ( AcesContainer CONFIG REQUIRED )
find_package ( Eigen3        CONFIG REQUIRED )
find_package ( Imath         CONFIG REQUIRED )
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

find_package (libraw CONFIG QUIET )

if (libraw_FOUND )
    message("STATUS LibRaw config found")
    set ( LIBRAW_CONFIG_FOUND TRUE )
else ()
    message("WARNING LibRaw config not found, trying to find a module.")
    find_package(libraw MODULE REQUIRED)
endif ()
