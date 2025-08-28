// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <OpenImageIO/unittest.h>
#include <filesystem>

#include <rawtoaces/define.h>

#define DATA_PATH "../_deps/rawtoaces_data-src/data/"

using namespace rta::core;

void test_OpenDir()
{
    std::filesystem::path absolutePath =
        std::filesystem::canonical( DATA_PATH "illuminant" );

    std::filesystem::path absolutePath_test = std::filesystem::canonical(
        DATA_PATH "illuminant/iso7589_stutung_380_780_5.json" );

    std::vector<std::string> fPaths = openDir( absolutePath.string() );

    OIIO_CHECK_EQUAL( fPaths.size(), 1 );
    std::vector<std::string>::iterator it = fPaths.begin();
    OIIO_CHECK_EQUAL( absolutePath_test.string(), *it );
};

void test_LowerCase()
{
    char text[]      = "RAWTOACES";
    char text_test[] = "rawtoaces";

    lowerCase( text );
    FORI( strlen( text ) )
    OIIO_CHECK_EQUAL( text[i], text_test[i] );
};

void test_IsNumeric()
{
    const char val1[] = "1234567890";
    OIIO_CHECK_EQUAL( true, isNumeric( val1 ) );

    const char val2[] = "123456789A";
    OIIO_CHECK_EQUAL( false, isNumeric( val2 ) );
};

void test_IsCTLetterDigit()
{
    const char val1 = '1';
    OIIO_CHECK_EQUAL( true, isCTLetterDigit( val1 ) );

    const char val2 = 'A';
    OIIO_CHECK_EQUAL( true, isCTLetterDigit( val2 ) );

    const char val3 = '-';
    OIIO_CHECK_EQUAL( true, isCTLetterDigit( val3 ) );

    const char val4 = '_';
    OIIO_CHECK_EQUAL( false, isCTLetterDigit( val4 ) );

    const char val5 = '.';
    OIIO_CHECK_EQUAL( false, isCTLetterDigit( val5 ) );
};

void test_IsValidCT()
{
    std::string val1 = "D6500";
    OIIO_CHECK_EQUAL( true, isValidCT( val1 ) );

    std::string val2 = "d6500";
    OIIO_CHECK_EQUAL( true, isValidCT( val2 ) );

    std::string val3 = "3200K";
    OIIO_CHECK_EQUAL( true, isValidCT( val3 ) );

    std::string val4 = "32.00K";
    OIIO_CHECK_EQUAL( false, isValidCT( val4 ) );

    std::string val5 = "6500";
    OIIO_CHECK_EQUAL( true, isValidCT( val5 ) );

    std::string val6 = "65";
    OIIO_CHECK_EQUAL( true, isValidCT( val6 ) );

    std::string val7 = "iso-3200";
    OIIO_CHECK_EQUAL( true, isValidCT( val7 ) );

    std::string val8 = "iso_3200";
    OIIO_CHECK_EQUAL( false, isValidCT( val8 ) );

    std::string val9 = "d65k";
    OIIO_CHECK_EQUAL( false, isValidCT( val9 ) );
};

int main( int, char ** )
{
    test_OpenDir();
    test_LowerCase();
    test_IsNumeric();
    test_IsCTLetterDigit();
    test_IsValidCT();

    return unit_test_failures;
}
