# Until we get some of these modules into the upstream packages, put them here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/share/CMake")

find_package( PkgConfig QUIET )

find_package( IlmBase COMPONENTS Half QUIET )
if(IlmBase_FOUND)
  message( STATUS "found IlmBase, version ${IlmBase_VERSION}" )
  include_directories( ${IlmBase_INCLUDE_DIRS} )
  link_directories( ${IlmBase_LIBRARY_DIRS} )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IlmBase_CFLAGS}" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IlmBase_CFLAGS}" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${IlmBase_LDFLAGS}" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${IlmBase_LDFLAGS}" )
  set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${IlmBase_LDFLAGS}" )
else()
  message( STATUS "IlmBase not found, will download from github and install it by its default settings" )
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

find_package( AcesContainer QUIET )
if (AcesContainer_FOUND)
  message( STATUS "Found AcesContainer, version ${AcesContainer_VERSION}" )
else()
  message( STATUS "AcesContainer not found, will download from github and install it by its default settings" )
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

#ExternalProject_Add_Step(
#  project_aces_container CopyToBin
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${GLOBAL_OUTPUT_PATH}/project_aces_container/bin ${GLOBAL_OUTPUT_PATH}
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${GLOBAL_OUTPUT_PATH}/project_aces_container/lib ${GLOBAL_OUTPUT_PATH}
#  DEPENDEES install
#)

  set (AcesContainer_FOUND TRUE)
  set (AcesContainer_INCLUDE_DIRS  "${ACES_EXTERNAL_BUILD}")
  set (AcesContainer_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/project_aces_container-prefix/src/project_aces_container-build")
endif()

find_package( libraw QUIET )
if (libraw_FOUND)
  message( STATUS "found LibRaw, version ${libraw_VERSION}" )
else()
  message( STATUS "libraw not found, will download from github and install it by its default settings" )
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
