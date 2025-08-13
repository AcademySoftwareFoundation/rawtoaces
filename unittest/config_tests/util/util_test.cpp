// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <filesystem>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <rawtoaces/acesrender.h>

BOOST_AUTO_TEST_CASE( Test_AcesRender )
{
    char *argv[] = {
        "dummy_binary_path",

    // getting "Subprocess aborted" in unit tests
    // when this is enabled on Mac, the tests fail on linux if
    // skipped completely.
#if __linux__
        "--wb-method",
        "0",
#endif
        "--mat-method",
        "1",
        "dummy_image_path"
    };

    const int argc = sizeof( argv ) / sizeof( argv[0] );

    std::filesystem::path pathToRaw = std::filesystem::absolute(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng" );
    BOOST_CHECK( std::filesystem::exists( pathToRaw ) );

    std::cerr << "IMAGE PATH " << pathToRaw.string() << std::endl;

    AcesRender &acesRender = AcesRender::getInstance();
    acesRender.initialize( dataPath() );

    int result;
    result = acesRender.configureSettings( argc, argv );
    BOOST_CHECK_EQUAL( result, argc - 1 );

    result = acesRender.preprocessRaw( pathToRaw.string().c_str() );
    BOOST_CHECK_EQUAL( result, 0 );

    result = acesRender.postprocessRaw();
    BOOST_CHECK_EQUAL( result, 0 );

    auto ff = acesRender.renderDNG();
    //    delete ff;

    auto idt = acesRender.getIDTMatrix();

    double matrix[3][3] = { { 1.0536466144, 0.0039044182, 0.0049084502 },
                            { -0.4899562165, 1.3614787986, 0.1020844728 },
                            { -0.0024498461, 0.0060497128, 1.0139159537 } };

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            BOOST_CHECK_CLOSE( idt[i][j], matrix[i][j], 1e-5 );
};
