// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_core.h"
#include "py_util.h"

NB_MODULE( rawtoaces, m )
{
    core_bindings( m );
    util_bindings( m );
}
