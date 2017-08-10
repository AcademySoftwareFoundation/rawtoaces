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
#include <boost/test/floating_point_comparison.hpp>

#include "../lib/mathOps.h"

using namespace std;

BOOST_AUTO_TEST_CASE ( Test_InvertD ) {
    double a = 1.0;
    BOOST_CHECK_CLOSE ( invertD(a), 1.0, 1e-9 );
    
    double b = 1000.0;
    BOOST_CHECK_CLOSE ( invertD(b), 0.001, 1e-9 );
    
    double c = 1000000.0;
    BOOST_CHECK_CLOSE ( invertD(c), 0.000001, 1e-9 );
};

BOOST_AUTO_TEST_CASE ( Test_Clip ) {
    double a = 254.9;
    BOOST_CHECK_CLOSE ( clip(a, 255.0), a, 1e-5 );
    
    double b = 255.1;
    BOOST_CHECK_CLOSE ( clip(b, 255.0), 255.0, 1e-5 );
    
    double c = 63355.0;
    BOOST_CHECK_CLOSE ( clip(c, 63355.0), c, 1e-5 );
};

BOOST_AUTO_TEST_CASE ( Test_IsSquare ) {
    vector < vector < double > > a;
    a.resize(2);
    FORI(2) a[i].resize(2);
    BOOST_CHECK_EQUAL ( isSquare(a), 1 );
    
    FORI(2) a[i].resize(1);
    BOOST_CHECK_EQUAL ( isSquare(a), 0 );
};

BOOST_AUTO_TEST_CASE ( Test_Cross ) {
    double a[2] = { 1.0, 3.0 };
    double b[2] = { 1.0, 6.5 };
    
    vector < double > av(a, a+2);
    vector < double > bv(b, b+2);

    double cross_test = cross(av, bv);
    BOOST_CHECK_CLOSE ( cross_test, 3.50, 1e-5 );
};

BOOST_AUTO_TEST_CASE ( Test_InvertVM ) {
    double M[3][3] = {
        { 0.0188205,  8.59E-03,   9.58E-03 },
        { 0.0440222,  0.0166118,  0.0258734 },
        { 0.1561591,  0.046321,   0.1181466 }
    };
    double M_Inverse[3][3] = {
        { -844.264597,  631.004958,  -69.728531 },
        { 1282.403375,  -803.858096,  72.055546 },
        { 613.114494,  -518.860936,  72.376689 }
    };
    
    vector < vector < double > > MV(3, vector < double > ( 3 ));
    FORIJ(3, 3) MV[i][j] = M[i][j];
    
    vector < vector < double > > MV_Inverse = invertVM (MV);
    
    FORI(3) {
        BOOST_CHECK_CLOSE ( MV_Inverse[i][0], M_Inverse[i][0], 1e-5 );
        BOOST_CHECK_CLOSE ( MV_Inverse[i][1], M_Inverse[i][1], 1e-5 );
        BOOST_CHECK_CLOSE ( MV_Inverse[i][2], M_Inverse[i][2], 1e-5 );
    }
};

BOOST_AUTO_TEST_CASE ( Test_DiagVM ) {
    double M[3][3] = {
        { 1.0, 0.0, 0.0 },
        { 0.0, 2.0, 0.0 },
        { 0.0, 0.0, 3.0 }
    };
    
    double vd[3] = { 1.0, 2.0, 3.0 };
    vector < double > MV(vd, vd+3);
    vector < vector < double > > MVD = diagVM (MV);
    
    FORI(3) {
        BOOST_CHECK_CLOSE ( MVD[i][0], M[i][0], 1e-5 );
        BOOST_CHECK_CLOSE ( MVD[i][1], M[i][1], 1e-5 );
        BOOST_CHECK_CLOSE ( MVD[i][2], M[i][2], 1e-5 );
    }
};

BOOST_AUTO_TEST_CASE ( Test_TransposeVec ) {
    double M[6][3] = {
        { 1.0, 0.0, 0.0 },
        { 0.0, 2.0, 0.0 },
        { 0.0, 0.0, 3.0 },
        { 1.0, 1.0, 2.0 },
        { 2.0, 2.0, 3.0 },
        { 3.0, 3.0, 4.0 }
    };

    double MT[3][6] = {
        { 1.0000000000,  0.0000000000,  0.0000000000,  1.0000000000,  2.0000000000,  3.0000000000 },
        { 0.0000000000,  2.0000000000,  0.0000000000,  1.0000000000,  2.0000000000,  3.0000000000 },
        { 0.0000000000,  0.0000000000,  3.0000000000,  2.0000000000,  3.0000000000,  4.0000000000 }
    };
    
    vector < vector < double > > MV(6, vector < double > ( 3 ));
    FORIJ(6, 3) MV[i][j] = M[i][j];
    vector < vector < double > > MVT = transposeVec (MV);
    
    FORIJ(3, 6)
        BOOST_CHECK_CLOSE ( MVT[i][j], MT[i][j], 1e-5 );
};

BOOST_AUTO_TEST_CASE ( Test_SumVector ) {
    double M[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    vector < double > MV(M, M+10);
    
    double sum = sumVector(MV);
    BOOST_CHECK_CLOSE ( sum, 55.0000, 1e-5 );
};

BOOST_AUTO_TEST_CASE ( Test_ScaleVectorMax ) {
    double M[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    double M_Scaled[10] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
    vector < double > MV(M, M+10);
    
    scaleVectorMax (MV);
    FORI(MV.size())
        BOOST_CHECK_CLOSE ( M_Scaled[i], MV[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE ( Test_ScaleVectorMin ) {
    double M[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    vector < double > MV(M, M+10);
    
    scaleVectorMin (MV);
    FORI(MV.size())
        BOOST_CHECK_CLOSE ( M[i], MV[i], 1e-5 );
};

