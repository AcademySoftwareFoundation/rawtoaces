cmake_minimum_required(VERSION 3.12)

add_definitions (-DBOOST_TEST_DYN_LINK)

################################################################################

add_executable (
	Test_Spst
	testSpst.cpp
)

target_link_libraries(
    Test_Spst
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_Spst COMMAND Test_Spst )

################################################################################

add_executable (
	Test_IDT
	testIDT.cpp
)

target_link_libraries(
    Test_IDT
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_IDT COMMAND Test_IDT )

################################################################################

add_executable (
	Test_Illum
	testIllum.cpp
)

target_link_libraries(
    Test_Illum
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_Illum COMMAND Test_Illum )

################################################################################

add_executable (
	Test_DNGIdt
	testDNGIdt.cpp
)

target_link_libraries(
    Test_DNGIdt
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

if ( LIBRAW_CONFIG_FOUND )
    target_link_libraries ( Test_DNGIdt PUBLIC libraw::raw )
else ()
    target_include_directories(Test_DNGIdt PUBLIC ${libraw_INCLUDE_DIR} )
    target_link_directories(Test_DNGIdt PUBLIC ${libraw_LIBRARY_DIRS} )
    target_link_libraries(Test_DNGIdt PUBLIC ${libraw_LIBRARIES} ${libraw_LDFLAGS_OTHER} )
endif ()

add_test ( NAME Test_DNGIdt COMMAND Test_DNGIdt )

################################################################################

add_executable (
	Test_Math
	testMath.cpp
)

target_link_libraries(
    Test_Math
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_Math  COMMAND Test_Math )

################################################################################

add_executable (
    Test_Logic
    testLogic.cpp
)

target_link_libraries(
    Test_Logic
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_Logic COMMAND Test_Logic )

################################################################################

add_executable (
	Test_Misc
	testMisc.cpp
)

target_link_libraries(
    Test_Misc
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_Misc COMMAND Test_Misc )
