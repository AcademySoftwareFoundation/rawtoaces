///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
//
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted,
// subject to acceptance of this license. Performance of any of the
// aforementioned acts indicates acceptance to be bound by the following
// terms and conditions:
//
//  * Copies of source code, in whole or in part, must retain the
//    above copyright notice, this list of conditions and the
//    Disclaimer of Warranty.
//
//  * Use in binary form must retain the above copyright notice,
//    this list of conditions and the Disclaimer of Warranty in the
//    documentation and/or other materials provided with the distribution.
//
//  * Nothing in this license shall be deemed to grant any rights to
//    trademarks, copyrights, patents, trade secrets or any other
//    intellectual property of A.M.P.A.S. or any contributors, except
//    as expressly stated herein.
//
//  * Neither the name "A.M.P.A.S." nor the name of any other
//    contributors to this software may be used to endorse or promote
//    products derivative of or based on this software without express
//    prior written permission of A.M.P.A.S. or the contributors, as
//    appropriate.
//
// This license shall be construed pursuant to the laws of the State of
// California, and any disputes related thereto shall be subject to the
// jurisdiction of the courts therein.
//
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO
// EVENT SHALL A.M.P.A.S., OR ANY CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, RESITUTIONARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY
// SPECIFICALLY DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER
// RELATED TO PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACADEMY
// COLOR ENCODING SYSTEM, OR APPLICATIONS THEREOF, HELD BY PARTIES OTHER
// THAN A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
///////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <rawtoaces/mathOps.h>

using namespace std;

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
