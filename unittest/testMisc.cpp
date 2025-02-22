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

#include <OpenImageIO/unittest.h>

#include <rawtoaces/define.h>

using namespace std;

void test_OpenDir()
{
    std::filesystem::path absolutePath =
        std::filesystem::canonical( "../../data/illuminant" );

    std::filesystem::path absolutePath_test = std::filesystem::canonical(
        "../../data/illuminant/iso7589_stutung_380_780_5.json" );

    vector<string> fPaths = openDir( absolutePath.string() );

    OIIO_CHECK_EQUAL( fPaths.size(), 1 );
    vector<string>::iterator it = fPaths.begin();
    OIIO_CHECK_EQUAL( absolutePath_test.string(), *it );
};

void test_LowerCase()
{
    char text[]      = "RAWTOACES";
    char text_test[] = "rawtoaces";

    lowerCase( text );
    FORI( strlen( text ) - 1 )
    OIIO_CHECK_EQUAL( text[i], text_test[i] );
};

void test_IsNumeric()
{
    const char val1[] = "1234567890";
    OIIO_CHECK_EQUAL( true, isNumeric( val1 ) );
    const char val2[] = "123456789A";
    OIIO_CHECK_FALSE( isNumeric( val2 ) );
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
    OIIO_CHECK_FALSE( isCTLetterDigit( val4 ) );

    const char val5 = '.';
    OIIO_CHECK_FALSE( isCTLetterDigit( val5 ) );
};

void test_IsValidCT()
{
    string val1 = "D6500";
    OIIO_CHECK_EQUAL( true, isValidCT( val1 ) );

    string val2 = "d6500";
    OIIO_CHECK_EQUAL( true, isValidCT( val2 ) );

    string val3 = "3200K";
    OIIO_CHECK_EQUAL( true, isValidCT( val3 ) );

    string val4 = "32.00K";
    OIIO_CHECK_FALSE( isValidCT( val4 ) );

    string val5 = "6500";
    OIIO_CHECK_EQUAL( true, isValidCT( val5 ) );

    string val6 = "65";
    OIIO_CHECK_EQUAL( true, isValidCT( val6 ) );

    string val7 = "iso-3200";
    OIIO_CHECK_EQUAL( true, isValidCT( val7 ) );

    string val8 = "iso_3200";
    OIIO_CHECK_FALSE( isValidCT( val8 ) );

    string val9 = "d65k";
    OIIO_CHECK_FALSE( isValidCT( val9 ) );
};

void test_PathsFinder()
{
    dataPath                 dps = pathsFinder();
    vector<string>::iterator it  = dps.paths.begin();

#ifdef WIN32
    string first = ".";
#else
    string first = "/usr/local/include/rawtoaces/data";
#endif

    OIIO_CHECK_EQUAL( first, *it );
};

int main( int, char ** )
{
    test_OpenDir();
    test_LowerCase();
    test_IsNumeric();
    test_IsCTLetterDigit();
    test_IsValidCT();
    test_PathsFinder();

    return unit_test_failures;
}
