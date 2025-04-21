// SPDX-License-Identifier: apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifndef _USAGE_h__
#define _USAGE_h__

int my_progress_callback(
    void *d, enum LibRaw_progress p, int iteration, int expected )
{
    char *passed = (char *)( d ? d : "default string" );

    printf(
        "CB: %s  pass %d of %d (data passed=%s)\n",
        libraw_strprogress( p ),
        iteration,
        expected,
        passed );

    if ( iteration == 0 )
        printf(
            "start_timevaling %s (expecting %d iterations)\n",
            libraw_strprogress( p ),
            expected );
    else if ( iteration == expected - 1 )
        printf( "%s finished\n", libraw_strprogress( p ) );

    ///    if(++cnt>10) return 1;

    return 0;
};

// timer
#ifndef WIN32
static struct timeval start_timeval, end_timeval;
void                  timerstart_timeval( void )
{
    gettimeofday( &start_timeval, NULL );
}

void timerprint( const char *msg, const char *filename )
{
    gettimeofday( &end_timeval, NULL );
    float msec = ( end_timeval.tv_sec - start_timeval.tv_sec ) * 1000.0f +
                 ( end_timeval.tv_usec - start_timeval.tv_usec ) / 1000.0f;

    printf( "Timing: %s/%s: %6.3f msec\n", filename, msg, msec );
}
#else
LARGE_INTEGER start_timeval;
void          timerstart_timeval( void )
{
    QueryPerformanceCounter( &start_timeval );
}

void timerprint( const char *msg, const char *filename )
{
    LARGE_INTEGER unit, end_timeval;

    QueryPerformanceCounter( &end_timeval );
    QueryPerformanceFrequency( &unit );

    float msec = (float)( end_timeval.QuadPart - start_timeval.QuadPart );
    msec /= (float)unit.QuadPart / 1000.0f;
    printf( "Timing: %s/%s: %6.3f msec\n", filename, msg, msec );
};
#endif

#endif
