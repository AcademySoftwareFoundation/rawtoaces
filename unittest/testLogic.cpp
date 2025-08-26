// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <rawtoaces/mathOps.h>

using namespace rta::core;

// This is a unit test to check the proper functioning of different logic in different functions.

// To test the validity of getCAT function
BOOST_AUTO_TEST_CASE( Test_getCAT )
{
    double D60[3] = { 0.952646074569846, 1.0000, 1.00882518435159 };

    double D65[3] = { 0.95047, 1.0000, 1.08883 };

    double final_matrix[3][3] = { { 1.0119, 0.0080, -0.0157 },
                                  { 0.0058, 1.0014, -0.0063 },
                                  { -0.00033664, -0.0010, 0.9278 } };

    vector<double> src( D65, D65 + 3 );
    vector<double> des( D60, D60 + 3 );
    // FORI(3) src[i]= D65[i];
    // FORI(3) des[i]= D60[i];

    vector<vector<double>> final_Output_getCAT = getCAT( src, des );

    vector<double> destination( 3, 0 );

    FORI( 3 )
    destination[i] = final_Output_getCAT[i][0] * src[0] +
                     final_Output_getCAT[i][1] * src[1] +
                     final_Output_getCAT[i][2] * src[2];

    BOOST_CHECK_CLOSE( destination[0], des[0], 1e-9 );
    BOOST_CHECK_CLOSE( destination[1], des[1], 1e-9 );
    BOOST_CHECK_CLOSE( destination[2], des[2], 1e-9 );

    FORI( 3 )
    {
        BOOST_CHECK_CLOSE( final_Output_getCAT[i][0], final_matrix[i][0], 5 );
        BOOST_CHECK_CLOSE( final_Output_getCAT[i][1], final_matrix[i][1], 5 );
        BOOST_CHECK_CLOSE( final_Output_getCAT[i][2], final_matrix[i][2], 5 );
    }
};
