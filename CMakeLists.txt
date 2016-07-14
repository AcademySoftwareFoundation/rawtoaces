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
#  elseif ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
#  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic" )
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
include( configure.cmake )

# generated config files end up in binary dir so to find them, need
# to add to include path
include_directories( "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" )
# implement find library stuff...

add_definitions( -DPACKAGE="RAWTOACES" -DVERSION="${RAWTOACES_VERSION}" )
if ( WIN32 AND NOT CYGWIN )
	add_subdirectory(lib)
  add_subdirectory(rawtoACES)
endif ()

# Add all targets to the build-tree export set
if ( WIN32 AND NOT CYGWIN )
  export( TARGETS Half Raw AcesContainer FILE "${PROJECT_BINARY_DIR}/RAWTOACESLibraryDepends.cmake" )
endif ()
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


### to build rawtoaces
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

if ( WIN32 AND NOT CYGWIN )
  include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/lib/half" "${PROJECT_SOURCE_DIR}/lib/libraw" "${PROJECT_SOURCE_DIR}/lib/aces_container" )
endif ()

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${IlmBase_INCLUDE_DIRS}" "${libraw_INCLUDE_DIRS}" )

add_executable( rawtoaces
  rawtoACES.cpp
)

target_link_libraries( rawtoaces ${IlmBase_LIBRARIES} )
target_link_libraries( rawtoaces ${IlmBase_LDFLAGS_OTHER} )
if ( AcesContainer_FOUND )
  target_link_libraries( rawtoaces ${AcesContainer_LIBRARIES} )
  target_link_libraries( rawtoaces ${AcesContainer_LDFLAGS_OTHER} )
endif()

if ( libraw_FOUND )
  target_link_libraries( rawtoaces ${libraw_LIBRARIES} )
  target_link_libraries( rawtoaces ${libraw_LDFLAGS_OTHER} )
endif()

install( TARGETS rawtoaces DESTINATION bin )

