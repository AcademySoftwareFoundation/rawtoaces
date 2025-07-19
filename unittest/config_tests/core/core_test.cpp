// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <rawtoaces/define.h>
#include <rawtoaces/mathOps.h>
#include <rawtoaces/rta.h>

BOOST_AUTO_TEST_CASE( Test_InvertD )
{
    double a = 1.0;
    BOOST_CHECK_CLOSE( invertD( a ), 1.0, 1e-9 );

    double b = 1000.0;
    BOOST_CHECK_CLOSE( invertD( b ), 0.001, 1e-9 );

    double c = 1000000.0;
    BOOST_CHECK_CLOSE( invertD( c ), 0.000001, 1e-9 );
};
