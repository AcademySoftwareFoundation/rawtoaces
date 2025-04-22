// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <rawtoaces/define.h>

using namespace std;

BOOST_AUTO_TEST_CASE( Test_OpenDir )
{
    boost::filesystem::path absolutePath =
        boost::filesystem::canonical( "../../data/illuminant" );

    boost::filesystem::path absolutePath_test = boost::filesystem::canonical(
        "../../data/illuminant/iso7589_stutung_380_780_5.json" );

    vector<string> fPaths = openDir( absolutePath.string() );

    BOOST_CHECK_EQUAL( fPaths.size(), 1 );
    vector<string>::iterator it = fPaths.begin();
    BOOST_CHECK_EQUAL( absolutePath_test.string(), *it );
};

BOOST_AUTO_TEST_CASE( Test_LowerCase )
{
    char text[]      = "RAWTOACES";
    char text_test[] = "rawtoaces";

    lowerCase( text );
    FORI( strlen( text ) - 1 )
    BOOST_CHECK_EQUAL( text[i], text_test[i] );
};

BOOST_AUTO_TEST_CASE( Test_IsNumeric )
{
    const char val1[] = "1234567890";
    BOOST_CHECK_EQUAL( true, isNumeric( val1 ) );

    const char val2[] = "123456789A";
    BOOST_CHECK_EQUAL( false, isNumeric( val2 ) );
};

BOOST_AUTO_TEST_CASE( Test_IsCTLetterDigit )
{
    const char val1 = '1';
    BOOST_CHECK_EQUAL( true, isCTLetterDigit( val1 ) );

    const char val2 = 'A';
    BOOST_CHECK_EQUAL( true, isCTLetterDigit( val2 ) );

    const char val3 = '-';
    BOOST_CHECK_EQUAL( true, isCTLetterDigit( val3 ) );

    const char val4 = '_';
    BOOST_CHECK_EQUAL( false, isCTLetterDigit( val4 ) );

    const char val5 = '.';
    BOOST_CHECK_EQUAL( false, isCTLetterDigit( val5 ) );
};

BOOST_AUTO_TEST_CASE( Test_IsValidCT )
{
    string val1 = "D6500";
    BOOST_CHECK_EQUAL( true, isValidCT( val1 ) );

    string val2 = "d6500";
    BOOST_CHECK_EQUAL( true, isValidCT( val2 ) );

    string val3 = "3200K";
    BOOST_CHECK_EQUAL( true, isValidCT( val3 ) );

    string val4 = "32.00K";
    BOOST_CHECK_EQUAL( false, isValidCT( val4 ) );

    string val5 = "6500";
    BOOST_CHECK_EQUAL( true, isValidCT( val5 ) );

    string val6 = "65";
    BOOST_CHECK_EQUAL( true, isValidCT( val6 ) );

    string val7 = "iso-3200";
    BOOST_CHECK_EQUAL( true, isValidCT( val7 ) );

    string val8 = "iso_3200";
    BOOST_CHECK_EQUAL( false, isValidCT( val8 ) );

    string val9 = "d65k";
    BOOST_CHECK_EQUAL( false, isValidCT( val9 ) );
};

BOOST_AUTO_TEST_CASE( Test_PathsFinder )
{
    dataPath                 dps = pathsFinder();
    vector<string>::iterator it  = dps.paths.begin();

#ifdef WIN32
    string first = ".";
#else
    string first = "/usr/local/include/rawtoaces/data";
#endif

    BOOST_CHECK_EQUAL( first, *it );
};
