cmake_minimum_required(VERSION 3.12)

add_definitions (-DBOOST_TEST_DYN_LINK)

################################################################################

add_executable (
	Test_SpectralData
	test_SpectralData.cpp
)

target_link_libraries(
    Test_SpectralData
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_SpectralData COMMAND Test_SpectralData )

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
        OpenImageIO::OpenImageIO
)

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

################################################################################

add_executable (
	Test_UsageTimer
	testUsageTimer.cpp
)

target_link_libraries(
    Test_UsageTimer
    PUBLIC
        ${RAWTOACES_UTIL_LIB}
        Boost::unit_test_framework
)

add_test ( NAME Test_UsageTimer COMMAND Test_UsageTimer )
