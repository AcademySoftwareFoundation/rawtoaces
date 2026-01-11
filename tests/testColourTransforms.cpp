// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include "test_utils.h"
#include <OpenImageIO/unittest.h>

#include "../src/rawtoaces_util/colour_transforms.h"

void test_configure_spectral_solver()
{}

int main( int, char ** )
{
    test_configure_spectral_solver();

    return unit_test_failures;
}
