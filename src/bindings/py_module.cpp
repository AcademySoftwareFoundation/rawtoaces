// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_core.h"
#include "py_util.h"

NB_MODULE( rawtoaces, m )
{
    m.doc() = R"""(
    Python bindings for RAW image conversion and metadata/spectral solving.

    Methods accepting OpenImageIO objects require a compatible OpenImageIO
    3.2+ build. Those methods are absent from earlier builds; see
    :ref:`python-oiio-availability` for the capability table.
    )""";
    core_bindings( m );
    util_bindings( m );
}
