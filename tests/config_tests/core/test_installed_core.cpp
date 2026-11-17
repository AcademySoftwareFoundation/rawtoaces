// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <rawtoaces/rawtoaces_core.h>

// A simple test to confirm that the library has been linked successfully.
int main( int, char ** )
{
    rta::core::XYZD65Solver solver;
    bool                    result = solver.calculate_transform();

    return result ? 0 : 1;
}
