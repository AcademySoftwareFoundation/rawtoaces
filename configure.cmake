# Until we get some of these modules into the upstream packages, put them here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/share/CMake")

find_package ( nlohmann_json CONFIG REQUIRED )
find_package ( OpenImageIO   CONFIG REQUIRED )

if ( RTA_ENABLE_EIGEN )
    find_package ( Eigen3 CONFIG REQUIRED )
endif ( RTA_ENABLE_EIGEN )

if ( RTA_ENABLE_CERES )
    find_package ( Ceres CONFIG REQUIRED )
endif ()

if ( RTA_ENABLE_LENSFUN )
    find_package ( PkgConfig REQUIRED )
    pkg_check_modules( lensfun REQUIRED lensfun>=0.3.2 )
endif ( RTA_ENABLE_LENSFUN )

if (RTA_BUILD_PYTHON_BINDINGS)

    if ( CMAKE_VERSION VERSION_LESS 3.18 )
        set( DEV_MODULE Development )
    else ()
        set( DEV_MODULE Development.Module )
    endif ()

    if ( RTA_PYTHON_VERSION STREQUAL "" )
        find_package ( Python 3.8 COMPONENTS Interpreter ${DEV_MODULE} REQUIRED )
    else ()
        find_package ( Python ${RTA_PYTHON_VERSION} EXACT COMPONENTS Interpreter ${DEV_MODULE} REQUIRED )
    endif()

    execute_process (
        COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)
    find_package ( nanobind CONFIG REQUIRED )
endif ()
