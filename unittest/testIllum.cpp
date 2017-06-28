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
#include "../lib/rta.h"

using namespace std;
using namespace rta;

BOOST_AUTO_TEST_CASE ( TestIllum_cctToxy ) {
    Illum illumObject;
    
    illumObject.path = "NA";
    illumObject.type = "d50";
    illumObject.inc = 5;
    
    vector <double> xy = illumObject.cctToxy(5000);
    
    BOOST_CHECK_CLOSE ( xy[0], 0.3456619734948, 1e-9 );
    BOOST_CHECK_CLOSE ( xy[1], 0.3586032641691, 1e-9 );
};


BOOST_AUTO_TEST_CASE ( TestIllum_calSPD ) {
    Illum illumObject;
    
    illumObject.path = "NA";
    illumObject.type = "d50";
    illumObject.inc = 5;
    
    illumObject.calSPD(5000);
    
    const double spd[107] = { 0.019225738,
                              1.0352915,
                              2.0513572,
                              4.917658,
                              7.783959,
                              11.270833,
                              14.757706,
                              16.357831,
                              17.957956,
                              19.489069,
                              21.020182,
                              22.485917,
                              23.951653,
                              25.461597,
                              26.971542,
                              25.734718,
                              24.497895,
                              27.189138,
                              29.880381,
                              39.600586,
                              49.320790,
                              52.923491,
                              56.526192,
                              58.286369,
                              60.046547,
                              58.937475,
                              57.828404,
                              66.332199,
                              74.835993,
                              81.047061,
                              87.258129,
                              88.940144,
                              90.622160,
                              90.999414,
                              91.376668,
                              93.246352,
                              95.116036,
                              93.542446,
                              91.968857,
                              93.848765,
                              95.728673,
                              96.173021,
                              96.617369,
                              96.874548,
                              97.131728,
                              99.616397,
                              102.101067,
                              101.428540,
                              100.756013,
                              101.536812,
                              102.317610,
                              101.158805,
                              100.000000,
                              98.867249,
                              97.734498,
                              98.325692,
                              98.916887,
                              96.207114,
                              93.497340,
                              95.591391,
                              97.685442,
                              98.475794,
                              99.266145,
                              99.152079,
                              99.038013,
                              97.377991,
                              95.717968,
                              97.285294,
                              98.852620,
                              97.257583,
                              95.662547,
                              96.923519,
                              98.184492,
                              100.590875,
                              102.997258,
                              101.062072,
                              99.126886,
                              93.251272,
                              87.375657,
                              89.486630,
                              91.597602,
                              92.240329,
                              92.883055,
                              84.866448,
                              76.849841,
                              81.678032,
                              86.506224,
                              89.540394,
                              92.574565,
                              85.400043,
                              78.225521,
                              67.956977,
                              57.688434,
                              70.303308,
                              82.918182,
                              80.593862,
                              78.269541,
                              78.909691,
                              79.549841,
                              76.473585,
                              73.397329,
                              68.656977,
                              63.916626,
                              67.344817,
                              70.773009,
                              72.605244,
                              74.437479
    };
    
    vector < double > data = illumObject.getSPD();
    
    FORI(data.size())
        BOOST_CHECK_CLOSE ( data[i], spd[i], 1e-5 );
    
};

