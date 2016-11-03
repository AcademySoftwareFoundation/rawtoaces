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

#define INV_255 (1.0/(double) 255.0)
#define INV_65535 (1.0/(double) 65535.0)

#ifndef NO_ACESCONTAINER
#include <aces/aces_Writer.h>
#endif

#ifndef NO_OPENEXR
#include <OpenEXR/half.h>
#endif

#ifdef WIN32
// suppress sprintf-related warning. sprintf() is permitted in sample code
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <valarray>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdexcept>
#include "lib/idt.h"

#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#endif

#include <libraw/libraw.h>
#ifdef WIN32
#define snprintf _snprintf
#include <windows.h>
#endif

using namespace std;
using namespace idt;

// Beginning -- For DNG chromatic adoption matrix calculations //
#define sign(x)		((x) > 0 ? 1 : ( (x) < 0 ? (0-1) : 0))
#define countSize(a)	(sizeof(a) / sizeof((a)[0]))

valarray<float>  cameraCalibration1DNG = valarray<float>(1.0f, 9);
valarray<float>  cameraCalibration2DNG = valarray<float>(1.0f, 9);
valarray<float>  cameraToXYZMtx        = valarray<float>(1.0f, 9);
valarray<float>  xyz2rgbMatrix1DNG     = valarray<float>(1.0f, 9);
valarray<float>  xyz2rgbMatrix2DNG     = valarray<float>(1.0f, 9);
valarray<float>  analogBalanceDNG      = valarray<float>(1.0f, 3);
valarray<float>  neutralRGBDNG         = valarray<float>(1.0f, 3);
valarray<float>  cameraXYZWhitePoint   = valarray<float>(1.0f, 3);
valarray<float>  calibrateIllum        = valarray<float>(1.0f, 2);

template<class T>
void printValarray (const valarray<T>&va, string s, int num)
{
    assert (num <= va.size());
    printf("%s:", s.c_str());
    for (int i=0; i<num; i++) {
        printf("%f ", va[i]);
    }
    printf(";\n");
    return;
}

static const float RobertsonMired[] = {1.0e-10f,10.0f,20.0f,30.0f,40.0f,50.0f,60.0f,70.0f,80.0f,90.0f,100.0f,
    125.0f,150.0f,175.0f,200.0f,225.0f,250.0f,275.0f,300.0f,325.0f,350.0f,375.0f,400.0f,425.0f,450.0f,475.0f,500.0f,525.0f,550.0f,575.0f,600.0f};

valarray<float> matrixBradford( )
{
    float CBradford[] = {
        0.8951f,  0.2664f,  -0.1614f,
        -0.7502f, 1.7135f,  0.0367f,
        0.0389f,  -0.0685f, 1.0296f
    };
    
    return valarray<float>(CBradford, 9);
};

static const float Robertson_uvtTable[][3] = {
    { 0.18006f, 0.26352f, -0.24341f},
    { 0.18066f, 0.26589f, -0.25479f},
    { 0.18133f, 0.26846f, -0.26876f},
    { 0.18208f, 0.27119f, -0.28539f},
    { 0.18293f, 0.27407f, -0.3047f},
    { 0.18388f, 0.27709f, -0.32675f},
    { 0.18494f, 0.28021f, -0.35156f},
    { 0.18611f, 0.28342f, -0.37915f},
    { 0.18740f, 0.28668f, -0.40955f},
    { 0.18880f, 0.28997f, -0.44278f},
    { 0.19032f, 0.29326f, -0.47888f},
    { 0.19462f, 0.30141f, -0.58204f},
    { 0.19962f, 0.30921f, -0.70471f},
    { 0.20525f, 0.31647f, -0.84901f},
    { 0.21142f, 0.32312f, -1.0182f},
    { 0.21807f, 0.32909f, -1.2168f},
    { 0.22511f, 0.33439f, -1.4512f},
    { 0.23247f, 0.33904f, -1.7298f},
    { 0.24010f, 0.34308f, -2.0637f},
    { 0.24792f, 0.34655f, -2.4681f},
    { 0.25591f, 0.34951f, -2.9641f},
    { 0.26400f, 0.35200f, -3.5814f},
    { 0.27218f, 0.35407f, -4.3633f},
    { 0.28039f, 0.35577f, -5.3762f},
    { 0.28863f, 0.35714f, -6.7262f},
    { 0.29685f, 0.35823f, -8.5955f},
    { 0.30505f, 0.35907f, -11.324f},
    { 0.31320f, 0.35968f, -15.628f},
    { 0.32129f, 0.36011f, -23.325f},
    { 0.32931f, 0.36038f, -40.77f},
    { 0.33724f, 0.36051f, -116.45f}
};

valarray<float> multiplyMatrix(valarray<float> mtxA, valarray<float> mtxB, size_t K=3)
{
    size_t rows = mtxA.size()/K;
    size_t cols = mtxB.size()/K;
    
    assert (rows * K == mtxA.size());
    assert (K * cols == mtxB.size());
    
    valarray<float> mtxC(rows * cols);
    float * pA = &mtxA[0];
    float * pB = &mtxB[0];
    float * pC = &mtxC[0];
    
    for (size_t r = 0; r < rows; r++) {
        for (size_t cArB = 0; cArB < K; cArB++) {
            for (size_t c = 0; c < cols; c++) {
                pC[r * cols + c] += pA[r * K + cArB] * pB[cArB * cols + c];
            }
        }
    }
    
    return mtxC;
};

valarray<float> invertMatrix(const valarray<float> &mtx)
{
    assert ( mtx.size() == 9 );
    
    float CinvMtx[] = {
        - mtx[5] * mtx[7] + mtx[4] * mtx[8],
        + mtx[2] * mtx[7] - mtx[1] * mtx[8],
        - mtx[2] * mtx[4] + mtx[1] * mtx[5],
        + mtx[5] * mtx[6] - mtx[3] * mtx[8],
        - mtx[2] * mtx[6] + mtx[0] * mtx[8],
        + mtx[2] * mtx[3] - mtx[0] * mtx[5],
        - mtx[4] * mtx[6] + mtx[3] * mtx[7],
        + mtx[1] * mtx[6] - mtx[0] * mtx[7],
        - mtx[1] * mtx[3] + mtx[0] * mtx[4]
    };
    
    valarray<float> invMtx(CinvMtx, mtx.size());
    
    float det = mtx[0] * invMtx[0] + mtx[1] * invMtx[3] + mtx[2] * invMtx[6];
    
    // pay attention to this
    assert ( det != 0 );
    invMtx /= det;			// divide requires valarray
    
    return invMtx;
};

valarray<float> getDiagonalMatrix(const valarray<float> &vectorA)
{
    size_t length = vectorA.size();
    
    valarray<float> diagonal(0.0f, length * length);
    
    for (size_t rmtxA = 0; rmtxA < length; rmtxA++) {
        diagonal[rmtxA * length + rmtxA] = vectorA [rmtxA];
    }
    
    return diagonal;
};

valarray<float> xyToXYZ (const valarray<float> &xy)
{
    valarray<float> XYZ(3);
    XYZ[0] = xy[0];
    XYZ[1] = xy[1];
    XYZ[2] = 1 - xy[0] - xy[1];
    
    return XYZ;
};

valarray<float> uvToxy (const valarray<float> &uv)
{
    float xyS[] ={3.0f, 2.0f};
    valarray<float> xyScale (xyS, 2);
    
    return (xyScale * uv) / (2 * uv [0] - 8 * uv[1] + 4);
};

valarray<float> uvToXYZ (const valarray<float> &uv)
{
    return xyToXYZ(uvToxy (uv) );
};

void prepareMatrices()
{
    if (cameraCalibration1DNG.size() != 0 ) {
        xyz2rgbMatrix1DNG = multiplyMatrix(cameraCalibration1DNG, xyz2rgbMatrix1DNG, 3);
    }
    if (cameraCalibration2DNG.size() != 0 ) {
        xyz2rgbMatrix2DNG = multiplyMatrix(cameraCalibration2DNG, xyz2rgbMatrix2DNG, 3);
    }
    if (analogBalanceDNG.size() != 0 ) {
        xyz2rgbMatrix1DNG = multiplyMatrix(getDiagonalMatrix(analogBalanceDNG), xyz2rgbMatrix1DNG, 3);
        xyz2rgbMatrix2DNG = multiplyMatrix(getDiagonalMatrix(analogBalanceDNG), xyz2rgbMatrix2DNG, 3);
    }
};

valarray<float> XYZtoCameraWeightedMatrix(const float &mir,
                                          const float &mir1,
                                          const float &mir2)
{
    float weight = max(0.0f, min(1.0f, (mir1 - mir)/(mir1 - mir2)));
    return xyz2rgbMatrix1DNG + weight * (xyz2rgbMatrix2DNG - xyz2rgbMatrix1DNG);
};

float cross(const valarray <float> &vectorA, const valarray <float> &vectorB)
{
    assert (vectorA.size() == 2 && vectorB.size() == 2 );
    return vectorA[0] * vectorB[1] - vectorA[1] * vectorB[0];
};

float cctToMired(float cct)
{
    return 1.0e6f/cct;
};

float robertsonLength(const valarray <float> &uv, const float* uvt)
{
    float t = uvt[2];
    valarray<float> slope(2);
    slope[0] = - sign(t) / sqrt(1 + t * t);
    slope[1] = t * slope[0];
    
    valarray<float> uvR(uvt, 2);
    return cross(slope, uv - uvR) ;
};

valarray<float> colorTemperatureToXYZ (const float cct)
{
    float mired = 1.0e6f/cct;
    valarray<float> uv(2);
    int Nrobert=static_cast<int>(countSize(Robertson_uvtTable));
    int i;
    
    for (i = 0; i < Nrobert; i++) {
        if (RobertsonMired [i] >= mired)
            break;
    }
    if (i <= 0)
        uv = valarray <float> (Robertson_uvtTable [0], 2);
    else if (i >= Nrobert)
        uv = valarray <float> (Robertson_uvtTable [Nrobert - 1], 2);
    else {
        float weight = (mired - RobertsonMired [i-1]) / (RobertsonMired [i] - RobertsonMired [i-1]);
        uv = weight * valarray <float>(Robertson_uvtTable[i], 2) + (1 - weight) * valarray<float> ( Robertson_uvtTable[i-1], 2);
    }
    
    return uvToXYZ(uv);
};

valarray<float> XYZTouv(const valarray<float> &XYZ )
{
    float uvS[] = {4.0f, 6.0f};
    valarray <float> uvScale(uvS, 2);
    return (uvScale * XYZ[slice(0,2,1)])/(XYZ[0] + 15 * XYZ[1] + 3 * XYZ[2]) ;
};

float lightSourceToColorTemp(const unsigned short tag)
{
    if(tag >= 32768)
        return (static_cast<float>(tag))-32768.0f;
    
    uint16_t LightSourceEXIFTagValues[][2] = {
        { 0,	5500},	//	"Unidentified"
        { 1,	5500},	//	"Daylight"
        { 2,	3500},	//	"Fluorescent light"
        { 3,	3400},	//	"Tungsten Lamp"
        {10,	5550},	//	"Flash"
        {17,	2856},	//	"Standard Illuminant A"
        {18,	4874},	//	"Standard Illuminant B"
        {19,	6774},	//	"Standard Illuminant C"
        {20,	5500},	//	"D55 Illuminant"
        {21,	6500},	//	"D65 Illuminant"
        {22,	7500}	//  "D75 Illuminant"
    };
    
    for (uint16_t i = 0; i < static_cast<int>(countSize(LightSourceEXIFTagValues)); i++) {
        if (LightSourceEXIFTagValues[i][0] == (uint16_t)tag){
            return (float)LightSourceEXIFTagValues[i][1];
        }
    }
    
    // If not found, return "Not identified"
    return 5500.0f;
};

float XYZToColorTemperature(const valarray<float> &XYZ)
{
    valarray<float> uv(XYZTouv(XYZ));
    int Nrobert = static_cast<int>(countSize(Robertson_uvtTable));
    int i;
    
    float mired;
    float RDthis = 0, RDprevious = 0;
    
    for(i = 0; i < Nrobert; i++) {
        if ((RDthis = robertsonLength(uv, Robertson_uvtTable[i])) <= 0.0)
            break;
        RDprevious = RDthis;
    }
    if(i <= 0)
        mired = RobertsonMired[0];
    else if(i >= Nrobert)
        mired = RobertsonMired[Nrobert - 1];
    else
        mired = RobertsonMired[i-1]+RDprevious*(RobertsonMired[i]-RobertsonMired[i-1])/(RDprevious-RDthis);
    
    float cct = 1.0e6f/mired;
    cct = max(2000.0f, min(50000.0f, cct));
    
    return cct;
};

valarray <float> findXYZtoCameraMtx(valarray<float> neutralRGB)
{
    if(calibrateIllum.size() == 0) {
        printf("Found no calibration illuminants.");
        return xyz2rgbMatrix1DNG;
    }
    
    if(neutralRGB.size() == 0) {
        printf("Found no neutral RGB value.");
        return xyz2rgbMatrix1DNG;
    }
    
    float cct1 = lightSourceToColorTemp(static_cast<const unsigned short>(calibrateIllum[0]));
    float cct2 = lightSourceToColorTemp(static_cast<const unsigned short>(calibrateIllum[1]));
    
    float mir1 = cctToMired(cct1);
    float mir2 = cctToMired(cct2);
    
    float maxMir = cctToMired(2000.0f);
    float minMir = cctToMired(50000.0f);
    
    float lomir = max(minMir, min(maxMir, min(mir1, mir2)));
    float himir = max(minMir, min(maxMir, max(mir1, mir2)));
    float mirStep = max(5.0f, (himir - lomir)/50.0f);
    
    float mir = 0.0f, lastMired = 0.0f, estimatedMired = 0.0f, lerror = 0.0f, lastError = 0.0f, smallestError = 0.0f;
    
    for (mir=lomir; mir<himir; mir+=mirStep) {
        // error = distance between the sampling mired (mir) and the mired of the white balance as returned through the weightred matrices
        lerror = mir - cctToMired(XYZToColorTemperature(multiplyMatrix(invertMatrix(XYZtoCameraWeightedMatrix(mir, mir1, mir2)), neutralRGBDNG)));

        if (lerror==0) {
            // no error. We found the exact mired
            estimatedMired = mir;
            break;
        }
        if (mir!=lomir && lerror * lastError<=0) {
            estimatedMired = mir+lerror/(lerror-lastError) * (mir-lastMired);
            break;
        }
        if (mir == lomir || abs(lerror) < abs(smallestError) )	{
            estimatedMired = mir ;
            smallestError = lerror;
        }
        lastError = lerror;
        lastMired = mir;
    }
    
    valarray<float> xyzToCameraRGBMatrix(XYZtoCameraWeightedMatrix(estimatedMired, mir1, mir2));
    
    return xyzToCameraRGBMatrix;
};

valarray<float> matrixRGBtoXYZ(const float chromaticities[][2])
{
    valarray<float> rXYZ(xyToXYZ(valarray<float> (chromaticities[0], 2)));
    valarray<float> gXYZ(xyToXYZ(valarray<float> (chromaticities[1], 2)));
    valarray<float> bXYZ(xyToXYZ(valarray<float> (chromaticities[2], 2)));
    valarray<float> wXYZ(xyToXYZ(valarray<float> (chromaticities[3], 2)));
    
    valarray<float> rgbMtx(9);
    rgbMtx[slice(0, 3, 3)] = rXYZ;
    rgbMtx[slice(1, 3, 3)] = gXYZ;
    rgbMtx[slice(2, 3, 3)] = bXYZ;
    
    valarray<float> normalizedWhite(wXYZ / wXYZ[1]);
    valarray<float> channelgains(multiplyMatrix(invertMatrix(rgbMtx), normalizedWhite, 3));
    valarray<float> colorMatrix(multiplyMatrix(rgbMtx, getDiagonalMatrix(channelgains), 3));
    
    return colorMatrix;
};

void getCameraXYZMtxAndWhitePoint(float baseExpo)
{
    cameraToXYZMtx = invertMatrix(findXYZtoCameraMtx(neutralRGBDNG));
    assert(cameraToXYZMtx.sum() != 0 );
    
    cameraToXYZMtx *= pow(2.0, (double)baseExpo);
    
    valarray<float> neutralXYZ(3);
    if (neutralRGBDNG.size() > 0) {
        neutralXYZ = multiplyMatrix(cameraToXYZMtx, neutralRGBDNG);
    } else {
        neutralXYZ = colorTemperatureToXYZ(lightSourceToColorTemp(calibrateIllum[0]));
    }
    
    cameraXYZWhitePoint = neutralXYZ/neutralXYZ[1];
    assert(cameraXYZWhitePoint.sum() != 0);
    
    return;
};

valarray <float> matrixChromaticAdaptation (const valarray <float> &whiteFrom,
                                            const valarray <float> &whiteTo)
{
    valarray <float> diagonal = getDiagonalMatrix(multiplyMatrix(matrixBradford(), whiteTo)/multiplyMatrix(matrixBradford(), whiteFrom));
    valarray <float> invBrd   = invertMatrix(matrixBradford());
    
    return multiplyMatrix(invBrd, multiplyMatrix(diagonal, matrixBradford()));
};

// End -- For DNG chromatic adoption matrix calculations //


float * convert_to_aces_NonDNG(libraw_processed_image_t *image)
{
    uchar * pixel = image->data;
    uint32_t total = image->width*image->height*image->colors;
    float * aces = new (std::nothrow) float[total];
    
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
                aces[i] = XYZ_acesrgb_3[0][0]*(pixel[i]) + XYZ_acesrgb_3[0][1]*(pixel[i+1])
                + XYZ_acesrgb_3[0][2]*(pixel[i+2]);
                aces[i+1] = XYZ_acesrgb_3[1][0]*(pixel[i]) + XYZ_acesrgb_3[1][1]*(pixel[i+1])
                + XYZ_acesrgb_3[1][2]*(pixel[i+2]);
                aces[i+2] = XYZ_acesrgb_3[2][0]*(pixel[i]) + XYZ_acesrgb_3[2][1]*(pixel[i+1])
                + XYZ_acesrgb_3[2][2]*(pixel[i+2]);
            }
        }
        else if (image->colors == 4){
            for(uint32 i = 0; i < total; i+=4 ){
                aces[i] = XYZ_acesrgb_4[0][0]*pixel[i] + XYZ_acesrgb_4[0][1]*pixel[i+1]
                + XYZ_acesrgb_4[0][2]*pixel[i+2] + XYZ_acesrgb_4[0][3]*pixel[i+3];
                aces[i+1] = XYZ_acesrgb_4[1][0]*pixel[i] + XYZ_acesrgb_4[1][1]*pixel[i+1]
                + XYZ_acesrgb_4[1][2]*pixel[i+2] + XYZ_acesrgb_4[1][3]*pixel[i+3];
                aces[i+2] = XYZ_acesrgb_4[2][0]*pixel[i] + XYZ_acesrgb_4[2][1]*pixel[i+1]
                + XYZ_acesrgb_4[2][2]*pixel[i+2] + XYZ_acesrgb_4[2][3]*pixel[i+3];
                aces[i+3] = XYZ_acesrgb_4[3][0]*pixel[i] + XYZ_acesrgb_4[3][1]*pixel[i+1]
                + XYZ_acesrgb_4[3][2]*pixel[i+2] + XYZ_acesrgb_4[3][3]*pixel[i+3];
            }
        }
        else {
            fprintf(stderr, "Currenly support 3 channels and 4 channels. \n");
            return 0;
        }
     }
    
    return aces;
 }

float * convert_to_aces_DNG(
//                            libraw_rawdata_t R,
//                            libraw_iparams_t P,
                            libraw_processed_image_t *image,
                            valarray<float> cameraToDisplayMtx
)
{
//    uchar * pixel = (uchar *)(R.raw_image);
//    uint32_t total = (R.sizes.width)*(R.sizes.height)*(P.colors);
    
    uchar * pixel = image->data;
    uint32_t total = image->width*image->height*image->colors;

    float * aces = new (std::nothrow) float[total];
    
    float matrix3[3][3];
    float matrix4[4][4];
    
    for(int i=0; i<cameraToDisplayMtx.size()/3; i++)
        for(int j=0; j<3; j++){
            matrix3[i][j] = cameraToDisplayMtx[i*3+j];
            matrix4[i][j] = cameraToDisplayMtx[i*3+j];
    }
    
    matrix4[0][3]=0.0;
    matrix4[1][3]=0.0;
    matrix4[2][3]=0.0;
    matrix4[3][3]=1.0;
    matrix4[3][0]=0.0;
    matrix4[3][1]=0.0;
    matrix4[3][2]=0.0;
    
    for(uint32_t i = 0; i < total; i++){
        aces[i] = static_cast<float>(pixel[i]);
    }
    
//    printf("R.raw_image: %p\n", R.raw_image);
//    printf("R.color3_image: %p\n", R.color3_image);
//    printf("R.color4_image: %p\n", R.color4_image);

//    for(uint32_t i = 0; i < R.sizes.height; i++){
//        if(P.colors == 3) {
//            for (int j = 0; j < 3; j++){
//                aces[i*3+j] = static_cast<float>(R.color3_image[i][j]);
//            }
//        }
//        else if (P.colors == 4) {
//            for (int j = 0; j < 4; j++){
//                aces[i*4+j] = static_cast<float>(R.color4_image[i][j]);
//            }
//        }
//    }

    
    if(!aces) {
        fprintf(stderr, "The pixel code value may not exist. \n");
        return 0;
    }
    else {
        if (image->colors == 3){
//        if (P.colors == 3){
            for(uint32_t i = 0; i < total; i+=3 ){
                aces[i] = matrix3[0][0]*(aces[i]) + matrix3[0][1]*(aces[i+1])
                + matrix3[0][2]*(aces[i+2]);
                aces[i+1] = matrix3[1][0]*(aces[i]) + matrix3[1][1]*(aces[i+1])
                + matrix3[1][2]*(aces[i+2]);
                aces[i+2] = matrix3[2][0]*(aces[i]) + matrix3[2][1]*(aces[i+1])
                + matrix3[2][2]*(aces[i+2]);
            }
        }
        else if (image->colors == 4){
//        else if (P.colors == 4){
            for(uint32 i = 0; i < total; i+=4 ){
                aces[i] = matrix4[0][0]*aces[i] + matrix4[0][1]*aces[i+1]
                + matrix4[0][2]*aces[i+2] + matrix4[0][3]*aces[i+3];
                aces[i+1] = matrix4[1][0]*aces[i] + matrix4[1][1]*aces[i+1]
                + matrix4[1][2]*aces[i+2] + matrix4[1][3]*aces[i+3];
                aces[i+2] = matrix4[2][0]*aces[i] + matrix4[2][1]*aces[i+1]
                + matrix4[2][2]*aces[i+2] + matrix4[2][3]*aces[i+3];
                aces[i+3] = matrix4[3][0]*aces[i] + matrix4[3][1]*aces[i+1]
                + matrix4[3][2]*aces[i+2] + matrix4[3][3]*aces[i+3];
            }
        }
        else{
            fprintf(stderr, "Currenly support 3 channels and 4 channels. \n");
            return 0;
        }
    }

    return aces;
}

float * prepareAcesData_NonDNG(libraw_processed_image_t *image)
{
    float * pixels = 0;
    pixels = convert_to_aces_NonDNG(image);
    
    return pixels;

}

float * prepareAcesData_DNG(libraw_rawdata_t R,
//                            libraw_iparams_t P
                            libraw_processed_image_t *image
                            )
{
    float (*dng_cm1)[3] = R.color.dng_color[0].colormatrix;
    float (*dng_cc1)[4] = R.color.dng_color[0].calibration;
    float (*dng_cm2)[3] = R.color.dng_color[1].colormatrix;
    float (*dng_cc2)[4] = R.color.dng_color[1].calibration;
        
    calibrateIllum[0] = R.color.dng_color[0].illuminant;
    calibrateIllum[1] = R.color.dng_color[1].illuminant;
    valarray<float> XYZToACESMtx(1.0f, 9);
    
    for(int i=0; i<3; i++) {
        neutralRGBDNG[i] = 1.0f/(R.color.cam_mul)[i];
        for (int j=0; j<3; j++){
            xyz2rgbMatrix1DNG[i*3+j] = (dng_cm1)[i][j];
            xyz2rgbMatrix2DNG[i*3+j] = (dng_cm2)[i][j];
            cameraCalibration1DNG[i*3+j] = (dng_cc1)[i][j];
            cameraCalibration1DNG[i*3+j] = (dng_cc2)[i][j];
            XYZToACESMtx[i*3+j] = XYZ_acesrgb_3[i][j];
        }
    }
    
    valarray<float> deviceWhiteV(deviceWhite, 3);
    getCameraXYZMtxAndWhitePoint(R.color.baseline_exposure);
    valarray<float> outputRGBtoXYZMtx(matrixRGBtoXYZ(chromaticitiesACES));
//    valarray<float> XYZToDisplayMtx(invertMatrix(outputRGBtoXYZMtx));
    valarray<float> outputXYZWhitePoint(multiplyMatrix(outputRGBtoXYZMtx, deviceWhiteV));
    valarray<float> chadMtx(matrixChromaticAdaptation(cameraXYZWhitePoint, outputXYZWhitePoint));
    
//    valarray<float> cameraToDisplayMtx(multiplyMatrix(multiplyMatrix(XYZToDisplayMtx, chadMtx), cameraToXYZMtx));
//    valarray<float> cameraToDisplayMtx(multiplyMatrix(multiplyMatrix(XYZToACESMtx, chadMtx), cameraToXYZMtx));
    valarray<float> cameraToDisplayMtx(multiplyMatrix(XYZToACESMtx, chadMtx));
    
    valarray<float> outRGBWhite(multiplyMatrix(cameraToDisplayMtx, multiplyMatrix(invertMatrix(cameraToXYZMtx), cameraXYZWhitePoint)));
    outRGBWhite	= outRGBWhite/outRGBWhite.max();
    
    valarray<float> absdif = abs(outRGBWhite-deviceWhiteV);
    if (absdif.max() >= 0.0001 ) {
        printf("WARNING: The neutrals should come out white balanced.\n");
    }

    assert(cameraToDisplayMtx.sum()!= 0);

//    printValarray(cameraXYZWhitePoint, "cameraXYZWhitePoint", cameraXYZWhitePoint.size());
//    printValarray(outputRGBtoXYZMtx, "outputRGBtoXYZMtx", outputRGBtoXYZMtx.size());
//    printValarray(chadMtx, "chadMtx", chadMtx.size());
//    printValarray(XYZToDisplayMtx, "XYZToDisplayMtx", XYZToDisplayMtx.size());
//    printValarray(cameraToXYZMtx, "cameraToXYZMtx", cameraToXYZMtx.size());
//    printValarray(cameraToDisplayMtx, "cameraToDisplayMtx", cameraToDisplayMtx.size());
    
//    float * pixels = convert_to_aces_DNG(R, P, cameraToDisplayMtx);
    float * pixels = convert_to_aces_DNG(image, cameraToDisplayMtx);

    return pixels;
}

void aces_write(const char * name,
                float    scale,
                uint16_t width,
                uint16_t height,
                uint8_t  channels,
                uint8_t  bits,
                float *  pixels)
{
    halfBytes *in = new (std::nothrow) halfBytes[channels*width*height];
    for (uint32_t i=0; i<channels*width*height; i++) {
        if (bits == 8) {
            pixels[i] = (double)pixels[i] * INV_255;
        }
        else if (bits == 16){
            pixels[i] = (double)pixels[i] * INV_65535;
        }
        
        half tmpV(pixels[i]/1.0f);
        in[i] = tmpV.bits();
    }
    
    std::vector<std::string> filenames;
    filenames.push_back(name);
    
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
    
    x.configure(writeParams);
    x.newImageObject(dynamicMeta);
    
    for(uint32_t row = 0; row < height; row++){
        halfBytes *rgbData = in + width*channels*row;
        x.storeHalfRow (rgbData, row);
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
//"-M        Set the value for auto bright\n"

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
    
    argv[argc] = (char*)"";
    for (arg=1; (((opm = argv[arg][0]) - 2) | 2) == '+'; )
        {
            opt = argv[arg++][1];
            if ((cp = strchr (sp=(char*)"cnbrkStqmHABCgi", opt))!=0)
                for (i=0; i < "111411111142"[cp-sp]-'0'; i++)
                    if (!isdigit(argv[arg+i][0]))
                        {
//                            printf("argv[arg+i][0]: %c, %i\n", opt,(isdigit(argv[arg+i][0])));
                            fprintf (stderr,"Non-numeric argument to \"-%c\"\n", opt);
                            return 1;
                        }

          switch (opt) 
              {
              case 'v':  verbosity++;  break;
              // Adobe Coefficients
              case 'D':  OUT.use_camera_matrix = 1; break;
              case 'G':  OUT.green_matching = 1; break;
              case 'c':  OUT.adjust_maximum_thr   = (float)atof(argv[arg++]);  break;
              // #define LIBRAW_DEFAULT_AUTO_BRIGHTNESS_THRESHOLD 0.01
              // case 'R':  OUT.auto_bright_thr   = (float)atof(argv[arg++]);  break;
              // The camera information can be found from libraw_types.h
              // case 'i':identify_only = 1; break;
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
#define R RawProcessor.imgdata.rawdata
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
            
            OUT.use_camera_matrix  = 3 * (opm == '-');
            OUT.output_color       = 5;
            OUT.highlight          = 0;
            OUT.use_camera_wb      = 1;
            OUT.gamm[0]            = 1;
            OUT.gamm[1]            = 1;
            OUT.no_auto_bright     = 1;
            
            // r option
            if(isdigit(OUT.user_mul[0])|| P1.dng_version){
                OUT.use_camera_wb = 0;
                OUT.use_auto_wb = 0;
            }
            
            if(OUT.use_auto_wb == 1) {
                OUT.use_camera_wb = 0;
            }
            
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
            
            if(verbosity>=2) // verbosity set by repeat -v switches
                printf("Converting to aces RGB\n");
            else if(verbosity)
                printf("Writing file %s\n",outfn);
            
            
            
            libraw_processed_image_t *post_image = RawProcessor.dcraw_make_mem_image(&ret);
            if(use_timing)
                timerprint("LibRaw::dcraw_make_mem_image()",argv[arg]);
            
            if(P1.dng_version == 0) {
                float * aces = prepareAcesData_NonDNG(post_image);
                aces_write(outfn,
                           1.0,
                           post_image->width,
                           post_image->height,
                           post_image->colors,
                           post_image->bits,
                           aces);
            }
            else {
//                float * aces = prepareAcesData_DNG(R,P1);
//                aces_write(outfn,
//                           1.0,
//                           R.sizes.width,
//                           R.sizes.height,
//                           P1.colors,
//                           post_image->bits,
//                           aces);
//                printf("%i,%i,%i\n",post_image->width,post_image->height,post_image->colors);
//                printf("%i,%i,%i\n",R.sizes.width,R.sizes.height,P1.colors);
                
                float * aces = prepareAcesData_DNG(R, post_image);
                aces_write(outfn,
                           1.0,
                           post_image->width,
                           post_image->height,
                           post_image->colors,
                           post_image->bits,
                           aces);
            }
            
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
