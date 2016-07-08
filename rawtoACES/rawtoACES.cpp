/* -*- C++ -*-
 * File: rawToACES.cpp
 * Created: Sun Mar 23,   2016
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

// # C++ 11:201103L, C++ 97:199711L
#define null_ptr (__cplusplus > 201103L ? (nullptr) : 0)
#define INV_255 (1.0/(double) 255.0)
#define INV_65535 (1.0/(double) 65535.0)

#ifdef WIN32
// suppress sprintf-related warning. sprintf() is permitted in sample code
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef NO_ACESCONTAINER
// #include <aces/aces_Writer.h>
#include "aces_Writer.h"
#endif

#ifndef NO_OPENEXR
// #include <OpenEXR/half.h>
#include "half.h"
#endif 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdexcept>

#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#endif

#include "libraw.h"
#ifdef WIN32
#define snprintf _snprintf
#include <windows.h>
#endif


static const double acesrgb_XYZ_3[3][3] ={
    { 1.0634731317028,      0.00639793641966071,   -0.0157891874506841 },
    { -0.492082784686793,   1.36823709310019,      0.0913444629573544 },
    { -0.0028137154424595,  0.00463991165243123,   0.91649468506889 }
};


static const double acesrgb_XYZ_4[4][4] ={
    { 1.0634731317028,      0.00639793641966071,   -0.0157891874506841,     0.0 },
    { -0.492082784686793,   1.36823709310019,      0.0913444629573544,      0.0 },
    { -0.0028137154424595,  0.00463991165243123,   0.91649468506889,        0.0 },
    { 0.0,                  0.0,                   0.0,                     1.0 }

};


float * convert_to_aces(libraw_processed_image_t *image)
{
    uchar * pixel = image->data;
    uint32_t total = image->width*image->height*image->colors;
    float * aces = new (std::nothrow) float[total];;
    
    for(uint32_t i = 0; i < total; i++ ){
        aces[i] = static_cast<float>(pixel[i]);
    }
    
    if(!aces) {
        fprintf(stderr, "The pixel code value may not exist. \n");
        return 0;
    }
    else {
        if(image->colors == 3){
            for(uint32_t i = 0; i < total; i+=3 ){
                aces[i] = acesrgb_XYZ_3[0][0]*(aces[i]) + acesrgb_XYZ_3[0][1]*(aces[i+1])
                + acesrgb_XYZ_3[0][2]*(aces[i+2]);
                aces[i+1] = acesrgb_XYZ_3[1][0]*(aces[i]) + acesrgb_XYZ_3[1][1]*(aces[i+1])
                + acesrgb_XYZ_3[1][2]*(aces[i+2]);
                aces[i+2] = acesrgb_XYZ_3[2][0]*(aces[i]) + acesrgb_XYZ_3[2][1]*(aces[i+1])
                + acesrgb_XYZ_3[2][2]*(aces[i+2]);
            }
        }
        else if (image->colors == 4){
            for(uint32 i = 0; i < total; i+=4 ){
                aces[i] = acesrgb_XYZ_4[0][0]*pixel[i] + acesrgb_XYZ_4[0][1]*pixel[i+1]
                + acesrgb_XYZ_4[0][2]*pixel[i+2] + acesrgb_XYZ_4[0][3]*pixel[i+3];
                aces[i+1] = acesrgb_XYZ_4[1][0]*pixel[i] + acesrgb_XYZ_4[1][1]*pixel[i+1]
                + acesrgb_XYZ_4[1][2]*pixel[i+2] + acesrgb_XYZ_4[1][3]*pixel[i+3];
                aces[i+2] = acesrgb_XYZ_4[2][0]*pixel[i] + acesrgb_XYZ_4[2][1]*pixel[i+1]
                + acesrgb_XYZ_4[2][2]*pixel[i+2] + acesrgb_XYZ_4[2][3]*pixel[i+3];
                aces[i+3] = acesrgb_XYZ_4[3][0]*pixel[i] + acesrgb_XYZ_4[3][1]*pixel[i+1]
                + acesrgb_XYZ_4[3][2]*pixel[i+2] + acesrgb_XYZ_4[3][3]*pixel[i+3];
            }
        }
        else{
            fprintf(stderr, "Currenly support 3 channels and 4 channels. \n");
            return 0;
        }
     }
    
    return aces;
 }


void aces_write(const char *name,
                float scale,
                libraw_processed_image_t *image)
{
    uint16_t width = image->width;
    uint16_t height = image->height;
    uint8_t channels = image->colors;
    uint8_t bits = image->bits;

    float * pixels = convert_to_aces(image);
    
    halfBytes *in = new (std::nothrow) halfBytes[channels*width*height];
    for (uint32_t i=0; i<channels*width*height; i++) {
        if (bits == 8) {
            pixels[i] = (double)pixels[i] * INV_255;
        }
        else if (bits == 16){
            pixels[i] = (double)pixels[i] * INV_65535;
        }
        
        half tmpV( pixels[i] / 1.0f );
        in[i] = tmpV.bits();
    }
    
    std::vector<std::string> filenames;
    filenames.push_back( name );
    
    aces_Writer x;
    
    MetaWriteClip writeParams;
    
    writeParams.duration				= 1;
    writeParams.outputFilenames			= filenames;
    
    writeParams.outputRows				= height;
    writeParams.outputCols				= width;
    
    writeParams.hi = x.getDefaultHeaderInfo();
    writeParams.hi.originalImageFlag	= 1;
    writeParams.hi.software				= "rawtoACES";
    
    writeParams.hi.channels.clear();
    switch ( channels )
    {
        case 3:
            writeParams.hi.channels.resize(3);
            writeParams.hi.channels[0].name = "B";
            writeParams.hi.channels[1].name = "G";
            writeParams.hi.channels[2].name = "R";
            break;
        case 4:
            writeParams.hi.channels.resize(4);
            writeParams.hi.channels[0].name = "A";
            writeParams.hi.channels[1].name = "B";
            writeParams.hi.channels[2].name = "G";
            writeParams.hi.channels[3].name = "R";
            break;
        case 6:
            throw std::invalid_argument("Stereo RGB support not yet implemented");
            //			writeParams.hi.channels.resize(6);
            //			writeParams.hi.channels[0].name = "B";
            //			writeParams.hi.channels[1].name = "G";
            //			writeParams.hi.channels[2].name = "R";
            //			writeParams.hi.channels[3].name = "left.B";
            //			writeParams.hi.channels[4].name = "left.G";
            //			writeParams.hi.channels[5].name = "left.R";
            //			break;
        case 8:
            throw std::invalid_argument("Stereo RGB support not yet implemented");
            //			writeParams.hi.channels.resize(8);
            //			writeParams.hi.channels[0].name = "A";
            //			writeParams.hi.channels[1].name = "B";
            //			writeParams.hi.channels[2].name = "G";
            //			writeParams.hi.channels[3].name = "R";
            //			writeParams.hi.channels[4].name = "left.A";
            //			writeParams.hi.channels[5].name = "left.B";
            //			writeParams.hi.channels[6].name = "left.G";
            //			writeParams.hi.channels[7].name = "left.R";
            //			break;
        default:
            throw std::invalid_argument("Only RGB, RGBA or stereo RGB[A] file supported");
            break;
    }
    
    DynamicMetadata dynamicMeta;
    dynamicMeta.imageIndex = 0;
    dynamicMeta.imageCounter = 0;
    
    x.configure ( writeParams );
    x.newImageObject ( dynamicMeta );
    
    for ( uint32_t row = 0; row < height; row++) {
        halfBytes *rgbData = in + width*channels*row;
        x.storeHalfRow ( rgbData, row );
    }
    
#if 0
    std::cout << "saving aces file" << std::endl;
    std::cout << "size " << width << "x" << height << "x" << channels << std::endl;
    std::cout << "size " << writeParams.outputCols << "x" << writeParams.outputRows << std::endl;
    std::cout << "duration " << writeParams.duration << std::endl;
    std::cout << writeParams.hi;
    std::cout << "\ndynamic meta" << std::endl;
    std::cout << "imageIndex " << dynamicMeta.imageIndex << std::endl;
    std::cout << "imageCounter " << dynamicMeta.imageCounter << std::endl;
    std::cout << "timeCode " << dynamicMeta.timeCode << std::endl;
    std::cout << "keyCode " << dynamicMeta.keyCode << std::endl;
    std::cout << "capDate " << dynamicMeta.capDate << std::endl;
    std::cout << "uuid " << dynamicMeta.uuid << std::endl;
#endif
    
    x.saveImageObject ( );
}


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
"-D        Using the coeff matrix from Adobe\n"
"-M        Set the value for auto bright\n"

#ifndef WIN32
"-E        Use mmap()-ed buffer instead of plain FILE I/O\n"
#endif
        );
    exit(1);
}

static int verbosity=0;

int cnt=0;
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

int main(int argc, char *argv[])
{
    if(argc==1) usage(argv[0]);
    
    LibRaw RawProcessor;
    int i,arg,c,ret;
    char opm,opt,*cp,*sp;
    int use_bigfile=0, use_timing=0;
    
#ifndef WIN32
    int msize = 0,use_mmap=0;
    void *iobuffer=0;
#endif

#ifdef OUT
#undef OUT
#endif

#define OUT RawProcessor.imgdata.params
    // OUT.use_camera_matrix = 3 * (opm == '+');
    OUT.output_color      = 5;
    OUT.use_camera_wb     = 1;
    OUT.gamm[0]           = 1;
    OUT.gamm[1]           = 1;
    
    argv[argc] = (char*)"";
    for (arg=1; (((opm = argv[arg][0]) - 2) | 2) == '+'; )
        {
            opt = argv[arg++][1];
            if ((cp = strchr (sp=(char*)"cnbrkStqmMHABCg", opt))!=0)
                for (i=0; i < "111411111142"[cp-sp]-'0'; i++)
                    if (!isdigit(argv[arg+i][0]))
                        {
                            fprintf (stderr,"Non-numeric argument to \"-%c\"\n", opt);
                            return 1;
                        }

          switch (opt) 
              {
              case 'v':  verbosity++;  break;
              case 'D':  OUT.use_camera_matrix = 1; break;
              case 'G':  OUT.green_matching = 1; break;
              case 'c':  OUT.adjust_maximum_thr   = (float)atof(argv[arg++]);  break;
              case 'M':  OUT.auto_bright_thr   = (float)atof(argv[arg++]);  break;
              case 'n':  OUT.threshold   = (float)atof(argv[arg++]);  break;
              case 'b':  OUT.bright      = (float)atof(argv[arg++]);  break;
              case 'P':  OUT.bad_pixels  = argv[arg++];        break;
              case 'K':  OUT.dark_frame  = argv[arg++];        break;
              case 'r':
                  for(c=0;c<4;c++) 
                      OUT.user_mul[c] = (float)atof(argv[arg++]);  
                  break;
              case 'C':  
                  OUT.aber[0] = 1 / atof(argv[arg++]);
                  OUT.aber[2] = 1 / atof(argv[arg++]);  
                  break;
              case 'g':  
                  OUT.gamm[0] = 1 / atof(argv[arg++]);
                  OUT.gamm[1] =     atof(argv[arg++]);  
                  break;
              case 'k':  OUT.user_black  = atoi(argv[arg++]);  break;
              case 'S':  OUT.user_sat    = atoi(argv[arg++]);  break;
              case 't':  OUT.user_flip   = atoi(argv[arg++]);  break;
              case 'q':  OUT.user_qual   = atoi(argv[arg++]);  break;
              case 'm':  OUT.med_passes  = atoi(argv[arg++]);  break;
              case 'H':  OUT.highlight   = atoi(argv[arg++]);  break;
              case 's':  OUT.shot_select = abs(atoi(argv[arg++])); break;
              case 'h':  OUT.half_size         = 1;
                  // no break:  "-h" implies "-f" 
              case 'f':  
                  OUT.four_color_rgb    = 1;  
                  break;
              case 'A':  for(c=0; c<4;c++) OUT.greybox[c]  = atoi(argv[arg++]); break;
              case 'B':  for(c=0; c<4;c++) OUT.cropbox[c]  = atoi(argv[arg++]); break;
              case 'a':  OUT.use_auto_wb       = 1;  break;
              case 'j':  OUT.use_fuji_rotate   = 0;  break;
              case 'W':  OUT.no_auto_bright    = 1;  break;
              case 'F':  use_bigfile           = 1; break;
              case 'd':  use_timing            = 1; break;
#ifndef WIN32
              case 'E':  use_mmap              = 1;  break;
#endif
              default:
                  fprintf (stderr,"Unknown option \"-%c\".\n", opt);
                  return 1;
              }
      }
#ifndef WIN32
  putenv ((char*)"TZ=UTC"); // dcraw compatibility, affects TIFF datestamp field
#else
  _putenv ((char*)"TZ=UTC"); // dcraw compatibility, affects TIFF datestamp field
#endif
#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other

  if(verbosity>1)
          RawProcessor.set_progress_handler(my_progress_callback,(void*)"Sample data passed");
#ifdef LIBRAW_USE_OPENMP
  if(verbosity)
          printf ("Using %d threads\n", omp_get_max_threads());
#endif

    for ( ; arg < argc; arg++)
        {
            char outfn[1024];

            if(verbosity) printf("Processing file %s\n",argv[arg]);
            
            timerstart_timeval();
            
#ifndef WIN32
            if(use_mmap)
                {
                    int file = open(argv[arg],O_RDONLY);
                    struct stat st;
                    if(file<0)
                        {
                            fprintf(stderr,"Cannot open %s: %s\n",argv[arg],strerror(errno));
                            continue;
                        }
                    if(fstat(file,&st))
                        {
                            fprintf(stderr,"Cannot stat %s: %s\n",argv[arg],strerror(errno));
                            close(file);
                            continue;
                        }
                    int pgsz = getpagesize();
                    msize = ((st.st_size+pgsz-1)/pgsz)*pgsz;
                    iobuffer = mmap(NULL,msize,PROT_READ,MAP_PRIVATE,file,0);
                    if(!iobuffer)
                        {
                            fprintf(stderr,"Cannot mmap %s: %s\n",argv[arg],strerror(errno));
                            close(file);
                            continue;
                        }
                    close(file);
                    if( (ret = RawProcessor.open_buffer(iobuffer,st.st_size) != LIBRAW_SUCCESS))
                        {
                            fprintf(stderr,"Cannot open_buffer %s: %s\n",argv[arg],libraw_strerror(ret));
                            continue; // no recycle b/c open file will recycle itself
                        }

                }
            else
#endif
                {
                    if(use_bigfile)
                        // force open_file switch to bigfile processing
                        ret = RawProcessor.open_file(argv[arg],1);
                    else
                        ret = RawProcessor.open_file(argv[arg]);
                        
                    if( ret  != LIBRAW_SUCCESS)
                        {
                            fprintf(stderr,"Cannot open %s: %s\n",argv[arg],libraw_strerror(ret));
                            continue; // no recycle b/c open_file will recycle itself
                        }
                }

            if(use_timing)
                timerprint("LibRaw::open_file()",argv[arg]);


            timerstart_timeval();
            if( (ret = RawProcessor.unpack() ) != LIBRAW_SUCCESS)
                {
                    fprintf(stderr,"Cannot unpack %s: %s\n",argv[arg],libraw_strerror(ret));
                    continue;
                }

            if(use_timing)
                timerprint("LibRaw::unpack()",argv[arg]);

            timerstart_timeval();
            
            if (LIBRAW_SUCCESS != (ret = RawProcessor.dcraw_process()))
                {
                    fprintf(stderr,"Cannot do postpocessing on %s: %s\n",argv[arg],libraw_strerror(ret));
                    if(LIBRAW_FATAL_ERROR(ret))
                        continue; 
                }
            if(use_timing)
                timerprint("LibRaw::dcraw_process()",argv[arg]);
            
            if ((cp = strrchr (argv[arg], '.'))) *cp = 0;
            snprintf(outfn,sizeof(outfn),
                     "%s%s",
                     argv[arg], "_aces.exr");

            libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image(&ret);
            
            if(verbosity>=2) // verbosity set by repeat -v switches
            {
                printf("Converting to aces RGB\n");
            }
            else if(verbosity)
            {
                printf("Writing file %s\n",outfn);
            }
            
            aces_write(outfn,1.0,image);
            
           

#ifndef WIN32
            if(use_mmap && iobuffer)
                {
                    munmap(iobuffer,msize);
                    iobuffer=0;
                }
#endif
            
            RawProcessor.recycle(); // just for show this call
        }
    return 0;
}
