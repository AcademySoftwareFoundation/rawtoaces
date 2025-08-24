// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/usage_timer.h>

#include <iostream>
#include <iomanip>

#ifndef WIN32
#    include <sys/time.h>
#else
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

namespace rta
{
namespace util
{

void UsageTimer::reset()
{
    if ( enabled )
    {
#ifndef WIN32
        gettimeofday( &_start_timeval, NULL );
#else
        QueryPerformanceCounter( &_start_timeval );
#endif
        _initialized = true;
    }
}

void UsageTimer::print( const std::string &path, const std::string &message )
{
    if ( enabled && _initialized )
    {
#ifndef WIN32
        struct timeval end_timeval;
        gettimeofday( &end_timeval, NULL );
        float msec = ( end_timeval.tv_sec - _start_timeval.tv_sec ) * 1000.0f +
                     ( end_timeval.tv_usec - _start_timeval.tv_usec ) / 1000.0f;
#else
        LARGE_INTEGER unit, end_timeval;
        QueryPerformanceCounter( &end_timeval );
        QueryPerformanceFrequency( &unit );

        float msec = (float)( end_timeval.QuadPart - _start_timeval.QuadPart );
        msec /= (float)unit.QuadPart / 1000.0f;
#endif

        std::cerr << "Timing: " << path << "/" << message << ": " << std::fixed
                  << std::setprecision( 3 ) << msec << std::defaultfloat
                  << std::setprecision( (int)std::cout.precision() ) << "msec"
                  << std::endl;
    }
}

} //namespace util
} //namespace rta
