/* -*- C++ -*-
 * File: rawToACES.cpp
 * Created: Tue Oct 25, 2016
 *
 * simple C++ API sample based on LibRaw
 *

LibRaw is free software; you can redistribute it and/or modify
it under the terms of the one of three licenses as you choose:

1. GNU LESSER GENERAL PUBLIC LICENSE version 2.1
   (See file LICENSE.LGPL provided in LibRaw distribution archive for details).

2. COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0
   (See file LICENSE.CDDL provided in LibRaw distribution archive for details).

3. LibRaw Software License 27032010
   (See file LICENSE.LibRaw.pdf provided in LibRaw distribution archive for details).
 */

#include "rawtoaces.h"

void usage(const char *prog)
{
    printf("rawtoACES\n");
    printf("Usage:  %s [FILE]...\n", prog);
    printf("OR Usage:  %s [OPTION]... [FILE]...\n", prog);
    printf(
"-c float-num       Set adjust maximum threshold (default 0.75)\n"
"-v        Verbose: print progress messages (repeated -v will add verbosity)\n"
"-a        Average the whole image for white balance\n"\
"-A <x y w h> Average a grey box for white balance\n"
"-r <r g b g> Set custom white balance\n"
"-C <r b>  Correct chromatic aberration\n"
"-P <file> Fix the dead pixels listed in this file\n"
"-K <file> Subtract dark frame (16-bit raw PGM)\n"
"-k <num>  Set the darkness level\n"
"-S <num>  Set the saturation level\n"
"-n <num>  Set threshold for wavelet denoising\n"
"-H [0-9]  Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)\n"
"-t [0-7]  Flip image (0=none, 3=180, 5=90CCW, 6=90CW)\n"
"-j        Don't stretch or rotate raw pixels\n"
"-W        Don't automatically brighten the image\n"
"-b <num>  Adjust brightness (default = 1.0)\n"
"-q [0-3]  Set the interpolation quality\n"
"-h        Half-size color image (twice as fast as \"-q 0\")\n"
"-f        Interpolate RGGB as four colors\n"
"-m <num>  Apply a 3x3 median filter to R-G and B-G\n"
"-s [0..N-1] Select one raw image from input file\n"
"-g pow ts Set gamma curve to gamma pow and toe slope ts (default = 2.222 4.5)\n"
"-G        Use green_matching() filter\n"
"-B <x y w h> use cropbox\n"
"-F        Use FILE I/O instead of streambuf API\n"
"-d        Detailed timing report\n"
"-r        User supplied channel multipliers, at least one of them should be 1.0\n"
"-D        Using the coeff matrix from Adobe\n"
"-Q        Specify the path to camera sensitivity data\n"
"-M        Set the value for scaling\n"
"-T        Set the desired color temperature (e.g., D60)\n"

#ifndef WIN32
"-E        Use mmap()-ed buffer instead of plain FILE I/O\n"
#endif
        );
    exit(1);
}

int my_progress_callback(void *d,enum LibRaw_progress p,int iteration, int expected)
{
    char *passed  = (char*)(d?d:"default string"); // data passed to callback at set_callback stage
    
    if(verbosity>2) // verbosity set by repeat -v switches
    {
        printf("CB: %s  pass %d of %d (data passed=%s)\n",libraw_strprogress(p),iteration,expected,passed);
    }
    else if (iteration == 0) // 1st iteration of each step
        printf("start_timevaling %s (expecting %d iterations)\n", libraw_strprogress(p),expected);
    else if (iteration == expected-1)
        printf("%s finished\n",libraw_strprogress(p));
    
    ///    if(++cnt>10) return 1; // emulate user termination on 10-th callback call
    
    return 0; // always return 0 to continue processing
}

// timer
#ifndef WIN32
static struct timeval start_timeval,end_timeval;
void timerstart_timeval(void)
{
    gettimeofday(&start_timeval,NULL);
}
void timerprint(const char *msg,const char *filename)
{
    gettimeofday(&end_timeval,NULL);
    float msec = (end_timeval.tv_sec - start_timeval.tv_sec)*1000.0f + (end_timeval.tv_usec - start_timeval.tv_usec)/1000.0f;
    printf("Timing: %s/%s: %6.3f msec\n",filename,msg,msec);
}
#else
LARGE_INTEGER start_timeval;
void timerstart_timeval(void)
{
    QueryPerformanceCounter(&start_timeval);
}
void timerprint(const char *msg, const char *filename)
{
    LARGE_INTEGER unit,end_timeval;
    QueryPerformanceCounter(&end_timeval);
    QueryPerformanceFrequency(&unit);
    float msec = (float)(end_timeval.QuadPart - start_timeval.QuadPart);
    msec /= (float)unit.QuadPart/1000.0f;
    printf("Timing: %s/%s: %6.3f msec\n",filename,msg,msec);
}

#endif

