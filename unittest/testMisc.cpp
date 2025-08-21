// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <filesystem>

#include <rawtoaces/define.h>

#define DATA_PATH "../_deps/rawtoaces_data-src/data/"

BOOST_AUTO_TEST_CASE( Test_OpenDir )
{
    std::filesystem::path absolutePath =
        std::filesystem::canonical( DATA_PATH "illuminant" );

    std::filesystem::path absolutePath_test = std::filesystem::canonical(
        DATA_PATH "illuminant/iso7589_stutung_380_780_5.json" );

    std::vector<std::string> fPaths = openDir( absolutePath.string() );

    BOOST_CHECK_EQUAL( fPaths.size(), 1 );
    std::vector<std::string>::iterator it = fPaths.begin();
    BOOST_CHECK_EQUAL( absolutePath_test.string(), *it );
};

BOOST_AUTO_TEST_CASE( Test_LowerCase )
{
    char text[]      = "RAWTOACES";
    char text_test[] = "rawtoaces";

    lowerCase( text );
    FORI( strlen( text ) )
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
    std::string val1 = "D6500";
    BOOST_CHECK_EQUAL( true, isValidCT( val1 ) );

    std::string val2 = "d6500";
    BOOST_CHECK_EQUAL( true, isValidCT( val2 ) );

    std::string val3 = "3200K";
    BOOST_CHECK_EQUAL( true, isValidCT( val3 ) );

    std::string val4 = "32.00K";
    BOOST_CHECK_EQUAL( false, isValidCT( val4 ) );

    std::string val5 = "6500";
    BOOST_CHECK_EQUAL( true, isValidCT( val5 ) );

    std::string val6 = "65";
    BOOST_CHECK_EQUAL( true, isValidCT( val6 ) );

    std::string val7 = "iso-3200";
    BOOST_CHECK_EQUAL( true, isValidCT( val7 ) );

    std::string val8 = "iso_3200";
    BOOST_CHECK_EQUAL( false, isValidCT( val8 ) );

    std::string val9 = "d65k";
    BOOST_CHECK_EQUAL( false, isValidCT( val9 ) );
};

BOOST_AUTO_TEST_CASE( Test_PathsFinder )
{
    dataPath                           dps = pathsFinder();
    std::vector<std::string>::iterator it  = dps.paths.begin();

#ifdef WIN32
    std::string first = ".";
#else
    std::string first = "/usr/local/include/rawtoaces/data";
#endif

    BOOST_CHECK_EQUAL( first, *it );
};
