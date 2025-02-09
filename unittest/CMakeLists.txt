cmake_minimum_required(VERSION 3.5)

add_definitions (-DBOOST_TEST_DYN_LINK)

add_executable (
	Test_Spst
	testSpst.cpp
)

set_property(TARGET Test_Spst PROPERTY CXX_STANDARD 17)

target_link_libraries(
    Test_Spst
    PUBLIC
        ${RAWTOACESLIB}
        Boost::boost
        Boost::filesystem
        Boost::unit_test_framework
)

add_executable (
	Test_IDT
	testIDT.cpp
)

set_property(TARGET Test_IDT PROPERTY CXX_STANDARD 17)

target_link_libraries(
    Test_IDT
    PUBLIC
        ${RAWTOACESLIB}
        Imath::Imath
        Imath::ImathConfig
        Boost::boost
        Boost::filesystem
        Boost::unit_test_framework
)

add_executable (
	Test_Illum
	testIllum.cpp
)

set_property(TARGET Test_Illum PROPERTY CXX_STANDARD 17)

target_link_libraries(
    Test_Illum
    PUBLIC
        ${RAWTOACESLIB}
        Boost::boost
        Boost::filesystem
        Boost::unit_test_framework
)

add_executable (
	Test_DNGIdt
	testDNGIdt.cpp
)

set_property(TARGET Test_DNGIdt PROPERTY CXX_STANDARD 17)

target_link_libraries(
    Test_DNGIdt
    PUBLIC
        ${RAWTOACESLIB}
        Boost::boost
        Boost::filesystem
        Boost::unit_test_framework
)

add_executable (
	Test_Math
	testMath.cpp
)

set_property(TARGET Test_Math PROPERTY CXX_STANDARD 17)

target_link_libraries(
    Test_Math
    PUBLIC
        ${RAWTOACESLIB}
        Boost::boost
        Boost::filesystem
        Boost::unit_test_framework
)

add_executable (
    Test_Logic
    testLogic.cpp
)

set_property(TARGET Test_Logic PROPERTY CXX_STANDARD 17)

target_link_libraries(
    Test_Logic
    PUBLIC
        ${RAWTOACESLIB}
        Boost::boost
        Boost::filesystem
        Boost::unit_test_framework
)

add_executable (
	Test_Misc
	testMisc.cpp
)

set_property(TARGET Test_Misc PROPERTY CXX_STANDARD 17)

target_link_libraries(
    Test_Misc
    PUBLIC
        ${RAWTOACESLIB}
        Boost::boost
        Boost::filesystem
        Boost::unit_test_framework
)


if ( ${Ceres_VERSION_MAJOR} GREATER 1 )
    target_include_directories( Test_Spst PUBLIC ${CERES_INCLUDE_DIRS} )
    target_include_directories( Test_IDT  PUBLIC ${CERES_INCLUDE_DIRS} )

    target_link_libraries( Test_Spst PUBLIC Ceres::ceres )
    target_link_libraries( Test_IDT  PUBLIC Ceres::ceres )
else ()
    target_link_libraries( Test_Spst PUBLIC ${CERES_LIBRARIES} )
    target_link_libraries( Test_IDT PUBLIC ${CERES_LIBRARIES} )
endif ()


add_test ( NAME Test_Spst   COMMAND Test_Spst   )
add_test ( NAME Test_IDT    COMMAND Test_IDT    )
add_test ( NAME Test_Illum  COMMAND Test_Illum  )
add_test ( NAME Test_DNGIdt COMMAND Test_DNGIdt )
add_test ( NAME Test_Math   COMMAND Test_Math   )
add_test ( NAME Test_Misc   COMMAND Test_Misc   )


