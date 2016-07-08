include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_library { LibRaw SHARED
	aahd_demosaic.cpp
	dcraw_common.cpp
	dcraw_fileio.cpp
	demosaic_packs.cpp
	dht_demosaic.cpp
	libraw_c_api.cpp
	libraw_cxx.cpp
	libraw_datastream.cpp
	libraw_x3f.cpp
	wf_filtering.cpp
}

set_target_properties {
	VERSION   ${RAWTOACES_VERSION}
  	SOVERSION ${RAWTOACES_VERSION}
}

install ( FILES
	defines.h
	libraw.h
	libraw_alloc.h
	libraw_build.h
	libraw_const.h
	libraw_datastream.h
	libraw_internal.h
	libraw_internal_funcs.h
	libraw_types.h
	libraw_version.h
	var_defines.h
   DESTINATION include/RAWTOACES 
)

install ( TARGETS LibRaw DESTINATION lib )
