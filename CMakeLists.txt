cmake_minimum_required(VERSION 2.8)
project( RAWTOACES )

set( RAWTOACES_MAJOR_VERSION 1 )
set( RAWTOACES_MINOR_VERSION 0 )
set( RAWTOACES_PATCH_VERSION 0 )
set( RAWTOACES_VERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION} )

set( CMAKE_MACOSX_RPATH 1 )

if( MSVC )
  if( CMAKE_CXX_FLAGS MATCHES "/W[0-4]" )
     string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  else() 
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/W0")
  endif()
elseif ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
    set (CMAKE_CXX_FLAGS “${CMAKE_CXX_FLAGS} -w”)
endif ()


## Make install directories overrideable
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
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
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

option(ENABLE_SHARED "Enable Shared Libraries" ON)

if ( ENABLE_SHARED )
	set( DO_SHARED SHARED )
else()
	set( DO_SHARED STATIC )
endif()

# Find packages and set up global settings
if ( APPLE OR UNIX )
  include( configure.cmake )
endif()

# If Half libary is found, we can git-clone it and build it
if ( NOT IlmBase_FOUND )
  include(ExternalProject)
  set (ilmbase_EXTERNAL_BUILD "${CMAKE_CURRENT_BINARY_DIR}/lib/openexr")
  ExternalProject_Add( project_ilmbase
    GIT_REPOSITORY https://github.com/openexr/openexr.git
    GIT_TAG "master"

    SOURCE_DIR "${ilmbase_EXTERNAL_BUILD}"
    CONFIGURE_COMMAND cd "${ilmbase_EXTERNAL_BUILD}/IlmBase" && "${ilmbase_EXTERNAL_BUILD}/IlmBase/bootstrap" 
    BUILD_COMMAND "${ilmbase_EXTERNAL_BUILD}/IlmBase/configure" && make -s "${ilmbase_EXTERNAL_BUILD}/IlmBase"

    CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${GLOBAL_OUTPUT_PATH}/project_ilmbase
  )

  set (IlmBase_FOUND TRUE)
  set (IlmBase_INCLUDE_DIRS "${ilmbase_EXTERNAL_BUILD}/IlmBase/Half")
  set (IlmBase_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/project_ilmbase-prefix/src/project_ilmbase-build/Half")
endif()

# If Aces_Container is not found, we can git-clone it and build it
# if ( NOT AcesContainer_FOUND )
  include(ExternalProject)
  set (ACES_EXTERNAL_BUILD "${CMAKE_CURRENT_BINARY_DIR}/lib/aces_container")
  ExternalProject_Add( project_aces_container
    GIT_REPOSITORY https://github.com/miaoqi/aces_container.git
    GIT_TAG "windowBuildSupport"

    SOURCE_DIR "${ACES_EXTERNAL_BUILD}"
    CONFIGURE_COMMAND mkdir "${ACES_EXTERNAL_BUILD}/build" && cd "${ACES_EXTERNAL_BUILD}/build"
    BUILD_COMMAND cmake "${ACES_EXTERNAL_BUILD}" && make -s "${ACES_EXTERNAL_BUILD}/build"

    CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${GLOBAL_OUTPUT_PATH}/project_aces_container
  )

  set (AcesContainer_FOUND TRUE)
  set (AcesContainer_INCLUDE_DIRS "${ACES_EXTERNAL_BUILD}")
  set (AcesContainer_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/project_aces_container-prefix/src/project_aces_container-build")
# endif()

# If libraw is not found, we can git-clone it and build it
if ( NOT libraw_FOUND )
  include(ExternalProject)
  set (libraw_EXTERNAL_BUILD "${CMAKE_CURRENT_BINARY_DIR}/lib/libraw")
  ExternalProject_Add( project_libraw
    GIT_REPOSITORY https://github.com/LibRaw/LibRaw.git
    GIT_TAG "0.17-stable"

    SOURCE_DIR "${libraw_EXTERNAL_BUILD}"
    CONFIGURE_COMMAND cd "${libraw_EXTERNAL_BUILD}" && "${libraw_EXTERNAL_BUILD}/mkdist.sh"
    BUILD_COMMAND "${libraw_EXTERNAL_BUILD}/configure" && make -s "${libraw_EXTERNAL_BUILD}"

    CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=${GLOBAL_OUTPUT_PATH}/project_libraw
  )

  set (libraw_FOUND TRUE)
  set (libraw_INCLUDE_DIRS "${libraw_EXTERNAL_BUILD}/libraw")
  set (libraw_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/project_libraw-prefix/src/project_libraw-build/lib/.libs") 
endif()

# generated config files end up in binary dir so to find them, need
# to add to include path
include_directories( "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" )
# implement find library stuff...

add_definitions( -DPACKAGE="RAWTOACES" -DVERSION="${RAWTOACES_VERSION}" )

# Add all targets to the build-tree export set
# if ( WIN32 AND NOT CYGWIN )
#  export( TARGETS Half Raw AcesContainer FILE "${PROJECT_BINARY_DIR}/RAWTOACESLibraryDepends.cmake" )
# install ( TARGETS Half Raw AcesContainer EXPORT HRA_targets )
# install ( EXPORT HRA_targets FILE "${PROJECT_BINARY_DIR}/RAWTOACESLibraryDepends.cmake") 
# endif ()
export( PACKAGE RAWTOACES )
 
# Create a RAWTOACESBuildTreeSettings.cmake file for the use from the build tree
file(RELATIVE_PATH CONF_REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
configure_file(config/RAWTOACESBuildTreeSettings.cmake.in "${PROJECT_BINARY_DIR}/RAWTOACESBuildTreeSettings.cmake" @ONLY)
configure_file(config/RAWTOACESConfig.cmake.in "${PROJECT_BINARY_DIR}/RAWTOACESConfig.cmake" @ONLY)
configure_file(config/RAWTOACESConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/RAWTOACESConfigVersion.cmake" @ONLY)

if ( PKG_CONFIG_FOUND )
  configure_file(config/RAWTOACES.pc.in "${PROJECT_BINARY_DIR}/RAWTOACES.pc" @ONLY)
  install( FILES "${PROJECT_BINARY_DIR}/RAWTOACES.pc" DESTINATION lib/pkgconfig COMPONENT dev )
endif()

install( FILES
  "${PROJECT_BINARY_DIR}/RAWTOACESConfig.cmake"
  "${PROJECT_BINARY_DIR}/RAWTOACESConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
if ( WIN32 AND NOT CYGWIN )
install( FILES "${PROJECT_BINARY_DIR}/RAWTOACESLibraryDepends.cmake" DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev )
endif( )


### to build rawtoaces ###
if ( AcesContainer_FOUND )
  add_definitions( -DHAVE_ACESFILE=1 )
  include_directories( ${AcesContainer_INCLUDE_DIRS} )
  link_directories( ${AcesContainer_LIBRARY_DIRS} )
endif()

if ( libraw_FOUND )
  add_definitions (-DHAVE_LIBRAW=1 )
  include_directories( ${libraw_INCLUDE_DIRS} )
  link_directories( ${libraw_LIBRARY_DIRS} )
endif() 

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${IlmBase_INCLUDE_DIRS}" "${AcesContainer_INCLUDE_DIRS}" "${libraw_INCLUDE_DIRS}" )

add_executable( rawtoaces
  rawtoACES.cpp
)

message (STATUS “${IlmBase_INCLUDE_DIRS}”)
message (STATUS “${AcesContainer_LIBRARIES}”)
message (STATUS “${libraw_LIBRARIES}”)
message (STATUS “${PROJECT_SOURCE_DIR}”)
message (STATUS “${PROJECT_BINARY_DIR}”)

if ( IlmBase_FOUND )
 target_link_libraries( rawtoaces ${IlmBase_LIBRARIES} )
 target_link_libraries( rawtoaces ${IlmBase_LDFLAGS_OTHER} )
endif()

if ( AcesContainer_FOUND )
  target_link_libraries( rawtoaces ${AcesContainer_LIBRARIES} )
  target_link_libraries( rawtoaces ${AcesContainer_LDFLAGS_OTHER} )
endif()

if ( libraw_FOUND )
  target_link_libraries( rawtoaces ${libraw_LIBRARIES} )
  target_link_libraries( rawtoaces ${libraw_LDFLAGS_OTHER} )
endif()

install( TARGETS rawtoaces DESTINATION bin )

