// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include "../assert.h"
#include <rawtoaces/rawtoaces_core.h>

void test_XYZD65Solver()
{
    rta::core::XYZD65Solver solver;
    bool                    result = solver.calculate_transform();
    RTA_CHECK_ASSERT( result );

    size_t size = solver.transform_matrix.size();
    RTA_CHECK_EQUAL( size, size_t( 3 ) );
}

int main( int, char ** )
{
    test_XYZD65Solver();

    return unit_test_failures;
}
