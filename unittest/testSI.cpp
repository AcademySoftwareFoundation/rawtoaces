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

BOOST_AUTO_TEST_CASE ( TestSpstDefaultConstructor ) {
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
    Spst * spstobject1 = new Spst ();
    
    spstobject1->setBrand(brand);
    spstobject1->setModel(model);
    spstobject1->setWLIncrement(increment);
    spstobject1->setSensitivity(rgbsen);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject1->getWLIncrement() ), 5 );
    BOOST_CHECK_EQUAL( int( spstobject1->getSensitivity().size() ), 81 );
    
    vector < RGBSen > rgbsen_cp = spstobject1->getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_EQUAL( rgbsen_cp[i].RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].BSen, 1.0 );
    }
};

BOOST_AUTO_TEST_CASE ( TestSpstConstructor2 ) {
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
    
    // Constructor 2
    Spst spstobject2 (brand, model, increment, rgbsen);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject2.getWLIncrement() ), increment );
    BOOST_CHECK_EQUAL( int( spstobject2.getSensitivity().size() ), 81 );
    
    vector < RGBSen > rgbsen_cp = spstobject2.getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_EQUAL( rgbsen_cp[i].RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].BSen, 1.0 );
    }
};

BOOST_AUTO_TEST_CASE ( TestSpstCopyConstructor ) {
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
    Spst * spstobject1 = new Spst ();
    
    spstobject1->setBrand(brand);
    spstobject1->setModel(model);
    spstobject1->setWLIncrement(increment);
    spstobject1->setSensitivity(rgbsen);
    
    Spst spstobject3 (*spstobject1);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject3.getWLIncrement() ), 5 );
    BOOST_CHECK_EQUAL( int( spstobject3.getSensitivity().size() ), 81 );

    vector < RGBSen > rgbsen_cp = spstobject3.getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_EQUAL( rgbsen_cp[i].RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i].BSen, 1.0 );
    }
};

BOOST_AUTO_TEST_CASE( TestDataAccess ) {
    char * brand1, * brand2, * brand3;
    char * model1, * model2, * model3;
    uint8_t len = 6;
    
    brand1 = (char *) malloc(len+1);
    memset(brand1, 0x0, len);
    memcpy(brand1, "", len);
    brand1[len] = '\0';
    
    brand2 = (char *) malloc(len+1);
    memset(brand2, 0x0, len);
    memcpy(brand2, "b2", len);
    brand2[len] = '\0';
    
    brand3 = (char *) malloc(len+1);
    memset(brand3, 0x0, len);
    memcpy(brand3, "brand3", len);
    brand3[len] = '\0';
    
    model1 = (char *) malloc(len+1);
    memset(model1, 0x0, len);
    memcpy(model1, "", len);
    model1[len] = '\0';
    
    model2 = (char *) malloc(len+1);
    memset(model2, 0x0, len);
    memcpy(model2, "m2", len);
    model2[len] = '\0';
    
    model3 = (char *) malloc(len+1);
    memset(model3, 0x0, len);
    memcpy(model3, "model3", len);
    model3[len] = '\0';
    
    vector < RGBSen > rgbsen1, rgbsen2, rgbsen3;
    for (int i=0; i<81; i++) {
        rgbsen1.push_back( RGBSen( 1.00000001, 1.0, 0.999999999 ) );
        rgbsen2.push_back( RGBSen( 1.0, 1.0, 1.0) );
        rgbsen3.push_back( RGBSen( -0.9999999999999, 1e-3, 1.0000000000001) );

    }
    
    Spst * spstobject1 = new Spst ();
    
    spstobject1->setBrand(brand1);
    spstobject1->setModel(model1);
    spstobject1->setWLIncrement(5);
    spstobject1->setSensitivity(rgbsen1);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getBrand(), "" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getModel(), "" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject1->getWLIncrement() ), 5 );
    
    vector < RGBSen > rgbsen_cp = spstobject1->getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_CLOSE (rgbsen_cp[i].RSen, 1.00000001, 1e-5);
        BOOST_CHECK_CLOSE( rgbsen_cp[i].GSen, 1.0, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i].BSen, 0.999999999, 1e-5);
    }
    
    Spst spstobject2(brand2, model2, 10, rgbsen2);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getBrand(), "b2" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getModel(), "m2" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject2.getWLIncrement() ), 10 );
    
    rgbsen_cp.clear();
    rgbsen_cp = spstobject2.getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_CLOSE (rgbsen_cp[i].RSen, 1.0, 1e-5);
        BOOST_CHECK_CLOSE( rgbsen_cp[i].GSen, 1.0, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i].BSen, 1.0, 1e-5);
    }
    
    Spst spstobject3(spstobject2);
    
    spstobject3.setBrand(brand3);
    spstobject3.setModel(model3);
    spstobject3.setWLIncrement(20);
    spstobject3.setSensitivity(rgbsen3);
    
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getBrand(), "brand3" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getModel(), "model3" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject3.getWLIncrement() ), 20 );
    
    rgbsen_cp.clear();
    rgbsen_cp = spstobject3.getSensitivity();
    
    FORI (81) {
        BOOST_CHECK_CLOSE (rgbsen_cp[i].RSen, -0.9999999999999, 1e-5);
        BOOST_CHECK_CLOSE( rgbsen_cp[i].GSen, 1e-3, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i].BSen, 1.0000000000001, 1e-5);
    }
};

BOOST_AUTO_TEST_CASE ( TestIDTLoadSpst ) {
    uint8_t len = 6;
    char * brand = (char *) malloc(len+1);

    memset(brand, 0x0, len);
    memcpy(brand, "nikon", len);
    brand[len] = '\0';
    
    char * model = (char *) malloc(len+1);
    memset(model, 0x0, len);
    memcpy(model, "d200", len);
    model[len] = '\0';
    
    Idt * idtTest = new Idt();
    
    const string path = "../../unittest/"
                        "materials/Nikon_D200_380_780_5_test";
    idtTest->loadCameraSpst ( path, brand, model );
    
    const Spst spstTest = idtTest->getCameraSpst();
    const vector <RGBSen> rgbsenTest = spstTest.getSensitivity();
    
    BOOST_CHECK_EQUAL( int( rgbsenTest.size() ), 81 );

};


