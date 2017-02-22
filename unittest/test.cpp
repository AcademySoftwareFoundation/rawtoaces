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

//BOOST_AUTO_TEST_CASE ( TestFloatingPoint ) {
//    double dnum = 2.00001;
//    BOOST_CHECK_CLOSE (dnum, 2.0000, 1e-3);
//}

BOOST_AUTO_TEST_CASE ( TestSpstObject ) {
//    Spst spstobject;
    
    char * brand;
    char * model;
    uint8_t increment = 5;
    uint8_t len = 4;
    
    brand = (char *) malloc(len+1);
    memset(brand, 0x0, len);
    memcpy(brand, "test", len);
    brand[len] = '\0';
    
    model = (char *) malloc(len+1);
    memset(model, 0x0, len);
    memcpy(model, "test", len);
    model[len] = '\0';
    
    vector < RGBSen > rgbsen;
    for (int i=0; i<81; i++) {
        rgbsen.push_back( RGBSen(1.0, 1.0, 1.0) );
    }
    
    // Default Constructor
    Spst spstobject1;
    
    spstobject1.setBrand(brand);
    spstobject1.setModel(model);
    spstobject1.setWLIncrement(5);
    spstobject1.setSensitivity(rgbsen);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1.getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1.getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject1.getWLIncrement() ), 5 );
    BOOST_CHECK_EQUAL( int( spstobject1.getSensitivity().size() ), 81 );
    
    vector < RGBSen > rgbsen_cp = spstobject1.getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_EQUAL( rgbsen_cp[i].RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].BSen, 1.0 );
    }

    
    // Constructor 2
    Spst spstobject2 (brand, model, increment, rgbsen);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject2.getWLIncrement() ), 5 );
    BOOST_CHECK_EQUAL( int( spstobject2.getSensitivity().size() ), 81 );
    
    rgbsen_cp.clear();
    
    rgbsen_cp = spstobject2.getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_EQUAL( rgbsen_cp[i].RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].BSen, 1.0 );
    }
    
    Spst spstobject3 (spstobject1);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject3.getWLIncrement() ), 5 );
    BOOST_CHECK_EQUAL( int( spstobject3.getSensitivity().size() ), 81 );
}


