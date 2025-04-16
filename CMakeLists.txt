cmake_minimum_required(VERSION 3.5)
project( RAWTOACES )

set( RAWTOACES_MAJOR_VERSION 1 )
set( RAWTOACES_MINOR_VERSION 0 )
set( RAWTOACES_PATCH_VERSION 0 )
set( RAWTOACES_VERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION} )

set(RAWTOACESIDTLIB "rawtoaces_idt")
set(RAWTOACESLIB "rawtoaces_util")

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


# generated config files end up in binary dir so to find them, need
# to add to include path
include_directories( "${PROJECT_SOURCE_DIR}/include" )

add_definitions( -DPACKAGE="RAWTOACES" -DVERSION="${RAWTOACES_VERSION}" )
add_subdirectory("src/${RAWTOACESIDTLIB}")
add_subdirectory("src/${RAWTOACESLIB}")


 
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

if ( APPLE OR UNIX )
	install (DIRECTORY data DESTINATION include/rawtoaces)
endif()

### to build rawtoaces ###

add_executable( rawtoaces
    main.cpp
)

target_include_directories( rawtoaces
    PUBLIC
        ${AcesContainer_INCLUDE_DIRS}
)

target_link_libraries ( rawtoaces
    PUBLIC
        ${RAWTOACESLIB}
    INTERFACE
        Boost::headers
)

if ( LIBRAW_CONFIG_FOUND )
    target_link_libraries ( rawtoaces PUBLIC libraw::raw )
else ()
    target_link_directories(rawtoaces PUBLIC ${libraw_LIBRARY_DIRS} )
    target_link_libraries(rawtoaces PUBLIC ${libraw_LIBRARIES} ${libraw_LDFLAGS_OTHER} )
endif ()

enable_testing()
add_subdirectory(unittest)

install( TARGETS rawtoaces DESTINATION bin )

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


