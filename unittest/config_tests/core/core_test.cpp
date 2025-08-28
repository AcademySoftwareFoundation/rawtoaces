// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <OpenImageIO/unittest.h>

#include <rawtoaces/define.h>
#include <rawtoaces/mathOps.h>
#include <rawtoaces/rawtoaces_core.h>

int main( int, char ** )
{
    double a = 1.0;
    OIIO_CHECK_EQUAL_THRESH( rta::core::invertD( a ), 1.0, 1e-9 );

    double b = 1000.0;
    OIIO_CHECK_EQUAL_THRESH( rta::core::invertD( b ), 0.001, 1e-9 );

    double c = 1000000.0;
    OIIO_CHECK_EQUAL_THRESH( rta::core::invertD( c ), 0.000001, 1e-9 );

    return unit_test_failures;
};
