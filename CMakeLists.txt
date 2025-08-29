cmake_minimum_required(VERSION 3.12)
project( RAWTOACES )

if( NOT DEFINED CMAKE_CXX_STANDARD )
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set( RAWTOACES_MAJOR_VERSION 1 )
set( RAWTOACES_MINOR_VERSION 1 )
set( RAWTOACES_PATCH_VERSION 0 )
set( RAWTOACES_VERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION} )

set(RAWTOACES_CORE_LIB "rawtoaces_core")
set(RAWTOACES_UTIL_LIB "rawtoaces_util")

set( CMAKE_MACOSX_RPATH 1 )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
#    set(warnings "/W4 /WX /EHsc")
    add_compile_options ( /W0 )
    add_compile_definitions( NOMINMAX )
endif()

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${warnings}")
endif()

## Make install directories overridable
set( INSTALL_LIB_DIR lib CACHE PATH "Install directory for libraries" )
set( INSTALL_BIN_DIR bin CACHE PATH "Install directory for executable binaries" )
set( INSTALL_INCLUDE_DIR include CACHE PATH "Install directory for public header files" )
if( WIN32 AND NOT CYGWIN )
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/RAWTOACES)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Install directory for project CMake files" )

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
# SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

## convert install paths to absolute
foreach( p LIB BIN INCLUDE CMAKE )
  set( var INSTALL_${p}_DIR )
  if( NOT IS_ABSOLUTE "${${var}}" )
    set( ${var} "${CMAKE_INSTALL_PREFIX}/${${var}}" )
  endif()
endforeach()

option( ENABLE_SHARED "Enable Shared Libraries" ON )
option( RTA_CENTOS7_CERES_HACK "Work around broken config in ceres-solver 1.12" OFF )

if ( ENABLE_SHARED )
  set ( DO_SHARED SHARED )
else ()
  set ( DO_SHARED STATIC )
endif ()

include ( configure.cmake )

#################################################
##
##   Build

add_definitions( -DPACKAGE="RAWTOACES" -DVERSION="${RAWTOACES_VERSION}" )
add_subdirectory("src/${RAWTOACES_CORE_LIB}")
add_subdirectory("src/${RAWTOACES_UTIL_LIB}")
add_subdirectory("src/rawtoaces")

enable_testing()
add_subdirectory(tests)

#################################################
##
##   Install RAWTOACES.pc

if ( PKG_CONFIG_FOUND )
  configure_file(config/RAWTOACES.pc.in "${PROJECT_BINARY_DIR}/RAWTOACES.pc" @ONLY)
  install( FILES "${PROJECT_BINARY_DIR}/RAWTOACES.pc" DESTINATION lib/pkgconfig COMPONENT dev )
endif()

#################################################
##
##   Install data

###############################################################################
##
##  Fetch data files from rawtoaces-data
##
include(FetchContent)

FetchContent_Declare(
  rawtoaces_data
  GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/rawtoaces-data
  GIT_TAG 4db0acc872965ecdbbf578664be20abf17c18e8c # v0.1.0
)

FetchContent_MakeAvailable(rawtoaces_data)

###############################################################################

if ( APPLE OR UNIX )
    install (
        DIRECTORY
            ${PROJECT_BINARY_DIR}/_deps/rawtoaces_data-src/data
        DESTINATION share/rawtoaces
    )
    
    install (
        FILES
            ${PROJECT_BINARY_DIR}/_deps/rawtoaces_data-src/LICENSE
        DESTINATION share/rawtoaces/data
    )
endif()


#################################################
##
##   Install targets

set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/
    CACHE PATH "Location of header files" )

install( EXPORT RAWTOACESTargets
    FILE RAWTOACESTargets.cmake
    DESTINATION lib/cmake/RAWTOACES
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    config/RAWTOACESConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RAWTOACESConfig.cmake
    INSTALL_DESTINATION lib/cmake/RAWTOACES
    PATH_VARS INCLUDE_INSTALL_DIR
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RAWTOACESConfigVersion.cmake
    VERSION ${RAWTOACES_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/RAWTOACESConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/RAWTOACESConfigVersion.cmake
    DESTINATION lib/cmake/RAWTOACES
)
