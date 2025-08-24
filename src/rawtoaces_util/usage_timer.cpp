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
        struct timeval start_timeval;
        gettimeofday( &start_timeval, NULL );
        _start_time = (double)start_timeval.tv_sec * 1000.0 +
                      (double)start_timeval.tv_usec / 1000.0;
#else
        LARGE_INTEGER unit, start_timeval;
        QueryPerformanceCounter( &start_timeval );
        QueryPerformanceFrequency( &unit );
        _start_time =
            (double)start_timeval.QuadPart * 1000.0 / (double)unit.QuadPart;
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
        double end_time = (double)end_timeval.tv_sec * 1000.0 +
                          (double)end_timeval.tv_usec / 1000.0;
#else
        LARGE_INTEGER unit, end_timeval;
        QueryPerformanceCounter( &end_timeval );
        QueryPerformanceFrequency( &unit );
        double end_time =
            (double)end_timeval.QuadPart * 1000.0 / (double)unit.QuadPart;
#endif

        double diff_msec = end_time - _start_time;

        std::cerr << "Timing: " << path << "/" << message << ": " << std::fixed
                  << std::setprecision( 3 ) << diff_msec << std::defaultfloat
                  << std::setprecision( (int)std::cout.precision() ) << "msec"
                  << std::endl;
    }
}

} //namespace util
} //namespace rta
