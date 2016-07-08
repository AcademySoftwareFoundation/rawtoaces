include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_library( AcesContainer ${DO_SHARED}
	aces_iostat.cpp
	aces_attributestructs.cpp
	aces_formatter.cpp
	aces_Writer.cpp
	aces_timing.cpp
	aces_typesAndRationals.cpp
	aces_writeattributes.cpp
	aces_md5.cc
)

install ( FILES 
	aces_errors.h
	aces_genericWriter.h
	aces_iostat.h
	aces_items.h
	aces_log.h
	aces_attributestructs.h
	aces_formatter.h
	aces_Writer.h
	aces_timing.h
	aces_typesAndRationals.h
	aces_types.h
	aces_writeattributes.h
  DESTINATION include/RAWTOACES 
)

install( TARGETS AcesContainer DESTINATION lib )
