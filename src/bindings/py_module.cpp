// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "py_core.h"
#include "py_util.h"

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/paramlist.h>

NB_MODULE( rawtoaces, m )
{
    nanobind::module_::import_( "OpenImageIO" );

    if ( !nanobind::type<OIIO::ImageBuf>().is_valid() )
    {
        throw nanobind::import_error(
            "OIIO::ImageBuf is not registered with nanobind." );
    }

    if ( !nanobind::type<OIIO::ParamValueList>().is_valid() )
    {
        throw nanobind::import_error(
            "OIIO::ParamValueList is not regisered with nanobind." );
    }

    if ( !nanobind::type<OIIO::ImageSpec>().is_valid() )
    {
        throw nanobind::import_error(
            "OIIO::ImageSpec is not registered with nanobind." );
    }

    core_bindings( m );
    util_bindings( m );
}
