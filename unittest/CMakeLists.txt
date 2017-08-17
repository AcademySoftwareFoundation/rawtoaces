cmake_minimum_required(VERSION 2.8)

find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

if ( UNIX AND NOT APPLE )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

include_directories ( ${CMAKE_CURRENT_SOURCE_DIR}
		      		  ${Boost_INCLUDE_DIRS}
		     		  ${IlmBase_INCLUDE_DIRS} 
		     		  ${AcesContainer_INCLUDE_DIRS}
		     		  ${libraw_INCLUDE_DIRS}
		              ${CERES_INCLUDE_DIRS}
		    )

link_directories ( ${AcesContainer_LIBRARY_DIRS} ${libraw_LIBRARY_DIRS} ${CERES_LIBRARY_DIRS} )

add_definitions (-DBOOST_TEST_DYN_LINK)

add_executable (
	Test_IDT
	testIDT.cpp
)

target_link_libraries ( Test_IDT
						${RAWTOACESLIB}
						${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
						${IlmBase_LIBRARIES}
						${AcesContainer_LIBRARIES}
						${libraw_LIBRARIES}
 						${CERES_LIBRARIES}
)

add_executable (
	Test_Illum
	testIllum.cpp
)

target_link_libraries ( Test_Illum
						${RAWTOACESLIB}
						${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

add_executable (
	Test_Math
	testMath.cpp
)

target_link_libraries ( Test_Math
						${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
 						${CERES_LIBRARIES}
)

add_executable (
	Test_Misc
	testMisc.cpp
)

target_link_libraries ( Test_Misc
						${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
					  
add_test (NAME Test_IDT COMMAND Test_IDT)
add_test (NAME Test_Illum COMMAND Test_Illum)
add_test (NAME Test_Math COMMAND Test_Math)
add_test (NAME Test_Misc COMMAND Test_Misc)


