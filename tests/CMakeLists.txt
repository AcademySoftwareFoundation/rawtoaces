cmake_minimum_required(VERSION 3.12)

# Enable coverage for all test targets if coverage is enabled
if( ENABLE_COVERAGE AND COVERAGE_SUPPORTED )
    message(STATUS "Enabling code coverage for test targets")
    
    # Helper function to setup coverage for test targets
    function(setup_test_coverage target_name)
        setup_coverage_flags(${target_name})
    endfunction()
else()
    # Dummy function when coverage is disabled
    function(setup_test_coverage target_name)
        # Do nothing
    endfunction()
endif()

################################################################################

add_executable (
	Test_core_usage
	core_usage.cpp
)

target_link_libraries(
    Test_core_usage
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_core_usage)
add_test ( NAME Test_core_usage COMMAND Test_core_usage )

################################################################################
add_executable (
	Test_util_usage
	util_usage.cpp
)

target_link_libraries(
    Test_util_usage
    PUBLIC
        ${RAWTOACES_UTIL_LIB}
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_util_usage)
add_test ( NAME Test_util_usage COMMAND Test_util_usage )

################################################################################
add_executable (
	Test_SpectralData
	test_SpectralData.cpp
)

target_link_libraries(
    Test_SpectralData
    PUBLIC
        ${RAWTOACES_CORE_LIB}
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_SpectralData)
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
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_IDT)
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
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_Illum)
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
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_DNGIdt)
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
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_Math)
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
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_Logic)
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
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_Misc)
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
        OpenImageIO::OpenImageIO
)

setup_test_coverage(Test_UsageTimer)
add_test ( NAME Test_UsageTimer COMMAND Test_UsageTimer )



################################################################################
# Coverage report generation
if( ENABLE_COVERAGE AND COVERAGE_SUPPORTED )
    # Include config_tests subdirectories
    add_subdirectory(config_tests/core)
    add_subdirectory(config_tests/util)

    generate_coverage_report()
endif()
