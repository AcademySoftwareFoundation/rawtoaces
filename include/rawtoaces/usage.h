///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
//
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted,
// subject to acceptance of this license. Performance of any of the
// aforementioned acts indicates acceptance to be bound by the following
// terms and conditions:
//
//  * Copies of source code, in whole or in part, must retain the
//    above copyright notice, this list of conditions and the
//    Disclaimer of Warranty.
//
//  * Use in binary form must retain the above copyright notice,
//    this list of conditions and the Disclaimer of Warranty in the
//    documentation and/or other materials provided with the distribution.
//
//  * Nothing in this license shall be deemed to grant any rights to
//    trademarks, copyrights, patents, trade secrets or any other
//    intellectual property of A.M.P.A.S. or any contributors, except
//    as expressly stated herein.
//
//  * Neither the name "A.M.P.A.S." nor the name of any other
//    contributors to this software may be used to endorse or promote
//    products derivative of or based on this software without express
//    prior written permission of A.M.P.A.S. or the contributors, as
//    appropriate.
//
// This license shall be construed pursuant to the laws of the State of
// California, and any disputes related thereto shall be subject to the
// jurisdiction of the courts therein.
//
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO
// EVENT SHALL A.M.P.A.S., OR ANY CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, RESITUTIONARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY
// SPECIFICALLY DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER
// RELATED TO PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACADEMY
// COLOR ENCODING SYSTEM, OR APPLICATIONS THEREOF, HELD BY PARTIES OTHER
// THAN A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
///////////////////////////////////////////////////////////////////////////

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
