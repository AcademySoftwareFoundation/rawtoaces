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

BOOST_AUTO_TEST_CASE( TestIllum_DefaultConstructor )
{
    Illum *illumObject = new Illum();

    BOOST_CHECK_EQUAL( illumObject->getIllumInc(), 5 );

    delete illumObject;
};

BOOST_AUTO_TEST_CASE( TestIllum_DefaultConstructor2 )
{
    Illum *illumObject = new Illum( "d50" );

    BOOST_CHECK_EQUAL( illumObject->getIllumType(), "d50" );
    BOOST_CHECK_EQUAL( illumObject->getIllumInc(), 5 );

    delete illumObject;
};

BOOST_AUTO_TEST_CASE( TestIllum_IllumType )
{
    Illum *illumObject = new Illum( "d50" );

    illumObject->setIllumType( "3200k" );
    BOOST_CHECK_EQUAL(
        std::strcmp( illumObject->getIllumType().c_str(), "3200k" ), 0 );

    delete illumObject;
};

BOOST_AUTO_TEST_CASE( TestIllum_IllumInc )
{
    Illum *illumObject = new Illum();

    illumObject->setIllumInc( 10 );
    BOOST_CHECK_EQUAL( illumObject->getIllumInc(), 10 );

    delete illumObject;
};

BOOST_AUTO_TEST_CASE( TestIllum_IllumIndex )
{
    Illum *illumObject = new Illum();

    illumObject->setIllumIndex( 10.99999 );

    BOOST_CHECK_CLOSE( illumObject->getIllumIndex(), 10.99999, 1e-5 );

    delete illumObject;
};

BOOST_AUTO_TEST_CASE( TestIllum_cctToxy )
{
    Illum illumObject;

    illumObject.setIllumType( "d50" );

    vector<double> xy = illumObject.cctToxy( 5000 * 1.4387752 / 1.438 );

    BOOST_CHECK_CLOSE( xy[0], 0.3456619734948, 1e-9 );
    BOOST_CHECK_CLOSE( xy[1], 0.3586032641691, 1e-9 );
};

BOOST_AUTO_TEST_CASE( TestIllum_readSPD )
{
    Illum illumObject;

    boost::filesystem::path illumPath = boost::filesystem::absolute(
        "../../data/illuminant/iso7589_stutung_380_780_5.json" );
    illumObject.readSPD( illumPath.string(), "iso7589" );

    double iso7589[81] = {
        0.0400000000000, 0.0500000000000, 0.0600000000000, 0.0700000000000,
        0.0800000000000, 0.0900000000000, 0.1000000000000, 0.1100000000000,
        0.1200000000000, 0.1325000000000, 0.1450000000000, 0.1575000000000,
        0.1700000000000, 0.1800000000000, 0.1900000000000, 0.2025000000000,
        0.2150000000000, 0.2275000000000, 0.2400000000000, 0.2525000000000,
        0.2650000000000, 0.2800000000000, 0.2950000000000, 0.3075000000000,
        0.3200000000000, 0.3350000000000, 0.3500000000000, 0.3650000000000,
        0.3800000000000, 0.3925000000000, 0.4050000000000, 0.4225000000000,
        0.4400000000000, 0.4550000000000, 0.4700000000000, 0.4850000000000,
        0.5000000000000, 0.5125000000000, 0.5250000000000, 0.5400000000000,
        0.5550000000000, 0.5675000000000, 0.5800000000000, 0.5950000000000,
        0.6100000000000, 0.6225000000000, 0.6350000000000, 0.6475000000000,
        0.6600000000000, 0.6750000000000, 0.6900000000000, 0.7025000000000,
        0.7150000000000, 0.7275000000000, 0.7400000000000, 0.7525000000000,
        0.7650000000000, 0.7750000000000, 0.7850000000000, 0.7975000000000,
        0.8100000000000, 0.8225000000000, 0.8350000000000, 0.8475000000000,
        0.8600000000000, 0.8700000000000, 0.8800000000000, 0.8900000000000,
        0.9000000000000, 0.9100000000000, 0.9200000000000, 0.9275000000000,
        0.9350000000000, 0.9450000000000, 0.9550000000000, 0.9650000000000,
        0.9750000000000, 0.9800000000000, 0.9850000000000, 0.9925000000000,
        1.0000000000000
    };

    BOOST_CHECK_EQUAL( illumObject.getIllumType(), "iso7589" );
    BOOST_CHECK_EQUAL( illumObject.getIllumInc(), 5 );

    vector<double> illumTestData = illumObject.getIllumData();
    BOOST_CHECK_EQUAL( illumTestData.size(), 81 );
    FORI( 81 ) BOOST_CHECK_CLOSE( illumTestData[i], iso7589[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIllum_calDayLightSPD )
{
    Illum illumObject;

    illumObject.setIllumType( "d50" );
    illumObject.setIllumInc( 5 );

    illumObject.calDayLightSPD( 50 );

    const double spd[81] = {
        24.4978949755877,  27.1891380970612,  29.8803812185346,
        39.6005856124086,  49.3207900062826,  52.9234909452740,
        56.5261918842655,  58.2863692933629,  60.0465467024602,
        58.9374754654423,  57.8284042284244,  66.3321986672105,
        74.8359931059968,  81.0470608788071,  87.2581286516173,
        88.9401444234118,  90.6221601952063,  90.9994142932283,
        91.3766683912501,  93.2463519723246,  95.1160355533991,
        93.5424463111289,  91.9688570688586,  93.8487650824512,
        95.7286730960438,  96.1730210585583,  96.6173690210728,
        96.8745483082553,  97.1317275954378,  99.6163972828030,
        102.1010669701683, 101.4285401947830, 100.7560134193976,
        101.5368117967032, 102.3176101740087, 101.1588050870044,
        100.0000000000000, 98.8672487920966,  97.7344975841932,
        98.3256924971888,  98.9168874101845,  96.2071139031367,
        93.4973403960889,  95.5913911756801,  97.6854419552712,
        98.4757935279747,  99.2661451006782,  99.1520792059532,
        99.0380133112281,  97.3779908297348,  95.7179683482415,
        97.2852940116115,  98.8526196749816,  97.2575832997790,
        95.6625469245764,  96.9235194654881,  98.1844920063998,
        100.5908749767109, 102.9972579470220, 101.0620719501407,
        99.1268859532593,  93.2512715124010,  87.3756570715427,
        89.4866296124544,  91.5976021533661,  92.2403288193421,
        92.8830554853180,  84.8664480824888,  76.8498406796596,
        81.6780321648151,  86.5062236499707,  89.5403942033879,
        92.5745647568050,  85.4000426306844,  78.2255205045638,
        67.9569774467047,  57.6884343888457,  70.3033080876698,
        82.9181817864939,  80.5938616113981,  78.2695414363022
    };

    vector<double> data = illumObject.getIllumData();
    FORI( data.size() )
    BOOST_CHECK_CLOSE( data[i], spd[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIllum_calBlackBodySPD )
{
    Illum illumObject;

    illumObject.setIllumType( "3200k" );
    illumObject.setIllumInc( 5 );

    illumObject.calBlackBodySPD( 3200 );

    const double spd[81] = {
        0.3431975190, 0.3748818425, 0.4082252416, 0.4432167268, 0.4798395347,
        0.5180713262, 0.5578844164, 0.5992460295, 0.6421185769, 0.6864599527,
        0.7322238438, 0.7793600511, 0.8278148175, 0.8775311606, 0.9284492064,
        0.9805065220, 1.0336384449, 1.0877784058, 1.1428582447, 1.1988085174,
        1.2555587916, 1.3130379308, 1.3711743665, 1.4298963557, 1.4891322260,
        1.5488106048, 1.6088606344, 1.6692121729, 1.7297959793, 1.7905438844,
        1.8513889471, 1.9122655966, 1.9731097607, 2.0338589799, 2.0944525099,
        2.1548314100, 2.2149386205, 2.2747190285, 2.3341195224, 2.3930890364,
        2.4515785855, 2.5095412906, 2.5669323963, 2.6237092793, 2.6798314508,
        2.7352605507, 2.7899603370, 2.8438966678, 2.8970374799, 2.9493527605,
        3.0008145169, 3.0513967400, 3.1010753667, 3.1498282370, 3.1976350505,
        3.2444773190, 3.2903383182, 3.3352030371, 3.3790581264, 3.4218918459,
        3.4636940107, 3.5044559371, 3.5441703881, 3.5828315187, 3.6204348211,
        3.6569770704, 3.6924562703, 3.7268715994, 3.7602233581, 3.7925129165,
        3.8237426623, 3.8539159505, 3.8830370536, 3.9111111129, 3.9381440909,
        3.9641427249, 3.9891144819, 4.0130675142, 4.0360106171, 4.0579531872,
        4.0789051818
    };

    vector<double> data = illumObject.getIllumData();
    FORI( data.size() )
    BOOST_CHECK_CLOSE( data[i] * 1e-12, spd[i], 1e-5 );
};
