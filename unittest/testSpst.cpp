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

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <rawtoaces/mathOps.h>
#include <rawtoaces/rta.h>

using namespace std;
using namespace rta;

BOOST_AUTO_TEST_CASE( TestSpst_DefaultConstructor )
{
    char   *brand;
    char   *model;
    uint8_t increment = 5;
    uint8_t len       = 4;

    brand = (char *)malloc( len + 1 );
    memset( brand, 0x0, len );
    memcpy( brand, "test", len );
    brand[len] = '\0';

    model = (char *)malloc( len + 1 );
    memset( model, 0x0, len );
    memcpy( model, "test", len );
    model[len] = '\0';

    vector<RGBSen> rgbsen;
    for ( int i = 0; i < 81; i++ )
    {
        rgbsen.push_back( RGBSen( 1.0, 1.0, 1.0 ) );
    }

    // Default Constructor
    Spst *spstobject1 = new Spst();

    spstobject1->setBrand( brand );
    spstobject1->setModel( model );
    spstobject1->setWLIncrement( increment );
    spstobject1->setSensitivity( rgbsen );

    //    boost::test_tools::const_string csBrand(spstobject1->getBrand());
    //    BOOST_CHECK_THROW( csBrand.at( csBrand.size()+1 ), std::out_of_range );

    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject1->getWLIncrement() ), 5 );
    BOOST_CHECK_EQUAL( int( spstobject1->getSensitivity().size() ), 81 );

    vector<RGBSen> rgbsen_cp = spstobject1->getSensitivity();
    delete spstobject1;

    FORI( 81 )
    {
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._BSen, 1.0 );
    }
};

BOOST_AUTO_TEST_CASE( TestSpst_Constructor2 )
{
    char   *brand;
    char   *model;
    uint8_t increment = 5;
    uint8_t len       = 4;

    brand = (char *)malloc( len + 1 );
    memset( brand, 0x0, len );
    memcpy( brand, "test", len );
    brand[len] = '\0';

    model = (char *)malloc( len + 1 );
    memset( model, 0x0, len );
    memcpy( model, "test", len );
    model[len] = '\0';

    vector<RGBSen> rgbsen;
    FORI( 81 )
    {
        rgbsen.push_back( RGBSen( 1.0, 1.0, 1.0 ) );
    }

    // Constructor 2
    Spst spstobject2( brand, model, increment, rgbsen );

    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject2.getWLIncrement() ), increment );
    BOOST_CHECK_EQUAL( int( spstobject2.getSensitivity().size() ), 81 );

    vector<RGBSen> rgbsen_cp = spstobject2.getSensitivity();

    FORI( 81 )
    {
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._BSen, 1.0 );
    }
};

BOOST_AUTO_TEST_CASE( TestSpst_CopyConstructor )
{
    char   *brand;
    char   *model;
    uint8_t increment = 5;
    uint8_t len       = 4;

    brand = (char *)malloc( len + 1 );
    memset( brand, 0x0, len );
    memcpy( brand, "test", len );
    brand[len] = '\0';

    model = (char *)malloc( len + 1 );
    memset( model, 0x0, len );
    memcpy( model, "test", len );
    model[len] = '\0';

    vector<RGBSen> rgbsen;
    for ( int i = 0; i < 81; i++ )
    {
        rgbsen.push_back( RGBSen( 1.0, 1.0, 1.0 ) );
    }

    // Default Constructor
    Spst *spstobject1 = new Spst();

    spstobject1->setBrand( brand );
    spstobject1->setModel( model );
    spstobject1->setWLIncrement( increment );
    spstobject1->setSensitivity( rgbsen );

    Spst spstobject3( *spstobject1 );

    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getBrand(), "test" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getModel(), "test" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject3.getWLIncrement() ), 5 );
    BOOST_CHECK_EQUAL( int( spstobject3.getSensitivity().size() ), 81 );

    vector<RGBSen> rgbsen_cp = spstobject3.getSensitivity();

    delete spstobject1;

    FORI( 81 )
    {
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._RSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._GSen, 1.0 );
        BOOST_CHECK_EQUAL( rgbsen_cp[i]._BSen, 1.0 );
    }
};

BOOST_AUTO_TEST_CASE( TestSpst_LoadSpst )
{
    uint8_t len   = 6;
    char   *brand = (char *)malloc( len + 1 );

    memset( brand, 0x0, len );
    memcpy( brand, "arri", len );
    brand[len] = '\0';

    char *model = (char *)malloc( len + 1 );
    memset( model, 0x0, len );
    memcpy( model, "d21", len );
    model[len] = '\0';

    Spst                   *spstTest     = new Spst();
    boost::filesystem::path absolutePath = boost::filesystem::absolute(
        "../../data/camera/arri_d21_380_780_5.json" );

    BOOST_CHECK_NO_THROW(
        spstTest->loadSpst( absolutePath.string(), brand, model ) );

    const vector<RGBSen> rgbsenTest = spstTest->getSensitivity();

    double rgb[81][3] = { { 0.000188205, 8.59E-05, 9.58E-05 },
                          { 0.000440222, 0.000166118, 0.000258734 },
                          { 0.001561591, 0.00046321, 0.001181466 },
                          { 0.006218858, 0.001314864, 0.006881015 },
                          { 0.022246734, 0.003696276, 0.031937733 },
                          { 0.049120511, 0.00805609, 0.087988515 },
                          { 0.102812947, 0.017241631, 0.216210301 },
                          { 0.105467801, 0.021953991, 0.276918236 },
                          { 0.117352663, 0.028731455, 0.384008295 },
                          { 0.108489774, 0.036438901, 0.498308108 },
                          { 0.078494347, 0.037473311, 0.485933057 },
                          { 0.06542927, 0.047763009, 0.618489235 },
                          { 0.05126662, 0.057989658, 0.696558624 },
                          { 0.038300854, 0.063272391, 0.711794157 },
                          { 0.036088371, 0.078451972, 0.821540625 },
                          { 0.038076306, 0.099730024, 0.918286066 },
                          { 0.036894365, 0.112097767, 0.818615612 },
                          { 0.044395944, 0.156013174, 0.907103055 },
                          { 0.055918682, 0.217501304, 1 },
                          { 0.060307176, 0.238434493, 0.86480047 },
                          { 0.066779015, 0.269670797, 0.878082723 },
                          { 0.074505107, 0.300101812, 0.874303769 },
                          { 0.07562978, 0.290737255, 0.704674036 },
                          { 0.085791103, 0.328330642, 0.628143997 },
                          { 0.108943209, 0.424666004, 0.588816784 },
                          { 0.138099867, 0.523135173, 0.513082855 },
                          { 0.168736396, 0.591697868, 0.436252915 },
                          { 0.220667659, 0.742521719, 0.392230422 },
                          { 0.268662105, 0.832207187, 0.343540362 },
                          { 0.321560163, 0.912162297, 0.312675861 },
                          { 0.37671682, 0.976493082, 0.304109232 },
                          { 0.410777194, 0.973507973, 0.292240658 },
                          { 0.421878401, 1, 0.291164917 },
                          { 0.388993508, 0.931244461, 0.269598208 },
                          { 0.354154608, 0.889356652, 0.248312101 },
                          { 0.34283344, 0.762661473, 0.213286579 },
                          { 0.380725719, 0.693921344, 0.194295275 },
                          { 0.469885563, 0.5991218, 0.170597248 },
                          { 0.599407862, 0.530315531, 0.155055826 },
                          { 0.713821326, 0.418038191, 0.1317383 },
                          { 0.80813316, 0.340043294, 0.116047887 },
                          { 0.939975954, 0.27676007, 0.104954578 },
                          { 1, 0.217867885, 0.093258038 },
                          { 0.956064245, 0.155062572, 0.076556466 },
                          { 0.894704087, 0.11537981, 0.064641572 },
                          { 0.767742902, 0.089103008, 0.053623886 },
                          { 0.798777151, 0.083004112, 0.052099277 },
                          { 0.763111509, 0.075973825, 0.04909842 },
                          { 0.682557924, 0.067551041, 0.044677337 },
                          { 0.56116663, 0.056571832, 0.0382092 },
                          { 0.436680781, 0.045437665, 0.031713716 },
                          { 0.414781937, 0.042487508, 0.030781211 },
                          { 0.380963428, 0.03912278, 0.029786697 },
                          { 0.305406639, 0.032338965, 0.026385578 },
                          { 0.260012751, 0.028342775, 0.02448327 },
                          { 0.191033296, 0.022001542, 0.020646569 },
                          { 0.141171909, 0.017151907, 0.017480635 },
                          { 0.122396106, 0.01528005, 0.015881482 },
                          { 0.102299712, 0.013443924, 0.01414462 },
                          { 0.07855096, 0.011348793, 0.011965207 },
                          { 0.060474144, 0.009399874, 0.009474274 },
                          { 0.041685047, 0.007185144, 0.006997807 },
                          { 0.028123563, 0.005351653, 0.005182991 },
                          { 0.02203961, 0.004473424, 0.004168945 },
                          { 0.017482165, 0.003764279, 0.003387594 },
                          { 0.012357413, 0.002865598, 0.002507749 },
                          { 0.008721969, 0.001999441, 0.001714727 },
                          { 0.006462905, 0.001438107, 0.001233306 },
                          { 0.00454705, 0.001049424, 0.000918575 },
                          { 0.002933579, 0.000695583, 0.000587696 },
                          { 0.00211892, 0.000533403, 0.000436494 },
                          { 0.001499002, 0.000394215, 0.000315097 },
                          { 0.001022687, 0.000293059, 0.000238467 },
                          { 0.000681853, 0.000211926, 0.000168269 },
                          { 0.000561613, 0.000202539, 0.000170632 },
                          { 0.000384839, 0.000125687, 8.94E-05 },
                          { 0.000286597, 0.000104774, 6.92E-05 },
                          { 0.000269169, 0.000138887, 0.000126057 },
                          { 0.000163058, 6.47E-05, 4.57E-05 },
                          { 0.000149065, 7.26E-05, 5.84E-05 },
                          { 3.71E-05, 0.0, 2.70E-06 } };

    BOOST_CHECK_EQUAL( int( rgbsenTest.size() ), 81 );
    BOOST_CHECK_EQUAL( std::strcmp( spstTest->getBrand(), "arri" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstTest->getModel(), "d21" ), 0 );
    BOOST_CHECK_EQUAL( int( spstTest->getWLIncrement() ), 5 );

    FORI( 81 )
    {
        BOOST_CHECK_CLOSE( double( rgbsenTest[i]._RSen ), rgb[i][0], 1e-5 );
        BOOST_CHECK_CLOSE( double( rgbsenTest[i]._GSen ), rgb[i][1], 1e-5 );
        BOOST_CHECK_CLOSE( double( rgbsenTest[i]._BSen ), rgb[i][2], 1e-5 );
    }

    free( model );
    free( brand );
    delete spstTest;
};

BOOST_AUTO_TEST_CASE( TestSpst_DataAccess )
{
    char   *brand1, *brand2, *brand3;
    char   *model1, *model2, *model3;
    uint8_t len = 6;

    brand1 = (char *)malloc( len + 1 );
    memset( brand1, 0x0, len );
    memcpy( brand1, "", len );
    brand1[len] = '\0';

    brand2 = (char *)malloc( len + 1 );
    memset( brand2, 0x0, len );
    memcpy( brand2, "b2", len );
    brand2[len] = '\0';

    brand3 = (char *)malloc( len + 1 );
    memset( brand3, 0x0, len );
    memcpy( brand3, "brand3", len );
    brand3[len] = '\0';

    model1 = (char *)malloc( len + 1 );
    memset( model1, 0x0, len );
    memcpy( model1, "", len );
    model1[len] = '\0';

    model2 = (char *)malloc( len + 1 );
    memset( model2, 0x0, len );
    memcpy( model2, "m2", len );
    model2[len] = '\0';

    model3 = (char *)malloc( len + 1 );
    memset( model3, 0x0, len );
    memcpy( model3, "model3", len );
    model3[len] = '\0';

    vector<RGBSen> rgbsen1, rgbsen2, rgbsen3;
    for ( int i = 0; i < 81; i++ )
    {
        rgbsen1.push_back( RGBSen( 1.00000001, 1.0, 0.999999999 ) );
        rgbsen2.push_back( RGBSen( 1.0, 1.0, 1.0 ) );
        rgbsen3.push_back( RGBSen( -0.9999999999999, 1e-3, 1.0000000000001 ) );
    }

    Spst *spstobject1 = new Spst();

    spstobject1->setBrand( brand1 );
    spstobject1->setModel( model1 );
    spstobject1->setWLIncrement( 5 );
    spstobject1->setSensitivity( rgbsen1 );

    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getBrand(), "" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject1->getModel(), "" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject1->getWLIncrement() ), 5 );

    vector<RGBSen> rgbsen_cp = spstobject1->getSensitivity();

    FORI( 81 )
    {
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._RSen, 1.00000001, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._GSen, 1.0, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._BSen, 0.999999999, 1e-5 );
    }

    Spst spstobject2( brand2, model2, 10, rgbsen2 );

    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getBrand(), "b2" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject2.getModel(), "m2" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject2.getWLIncrement() ), 10 );

    rgbsen_cp.clear();
    rgbsen_cp = spstobject2.getSensitivity();

    FORI( 81 )
    {
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._RSen, 1.0, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._GSen, 1.0, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._BSen, 1.0, 1e-5 );
    }

    Spst spstobject3( spstobject2 );

    spstobject3.setBrand( brand3 );
    spstobject3.setModel( model3 );
    spstobject3.setWLIncrement( 20 );
    spstobject3.setSensitivity( rgbsen3 );

    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getBrand(), "brand3" ), 0 );
    BOOST_CHECK_EQUAL( std::strcmp( spstobject3.getModel(), "model3" ), 0 );
    BOOST_CHECK_EQUAL( int( spstobject3.getWLIncrement() ), 20 );

    rgbsen_cp.clear();
    rgbsen_cp = spstobject3.getSensitivity();

    FORI( 81 )
    {
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._RSen, -0.9999999999999, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._GSen, 1e-3, 1e-5 );
        BOOST_CHECK_CLOSE( rgbsen_cp[i]._BSen, 1.0000000000001, 1e-5 );
    }

    //    free ( model1 );
    //    free ( model2 );
    //    free ( model3 );
    //    free ( brand1 );
    //    free ( brand2 );
    //    free ( brand3 );
    //    delete spstobject1;
};
