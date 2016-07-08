add_subdirectory( aces_container )
add_subdirectory( half )
add_subdirectory( libraw )

add_custom_target( RAWTOACES DEPENDS aces_container half libraw )
