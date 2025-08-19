// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifndef _ACESRENDER_h__
#define _ACESRENDER_h__

#include <rawtoaces/rta.h>

#include <unordered_map>
#include <libraw/libraw.h>

using namespace rta;

void create_key( unordered_map<string, char> &keys );
void usage( const char *prog );

class LibRawAces : virtual public LibRaw
{
public:
    LibRawAces(){};
    ~LibRawAces(){};

    void show() { printf( "I am here with LibRawAces.\n" ); }
};

class AcesRender
{
public:
    static AcesRender &getInstance();

    int configureSettings( int argc, char *argv[] );
    int fetchCameraSenPath( const libraw_iparams_t &P );
    int fetchIlluminant( const char *illumType = "na" );

    int openRawPath( const char *pathToRaw );
    int unpack( const char *pathToRaw );
    int dcraw();

    int  prepareIDT( const libraw_iparams_t &P, float *M );
    int  prepareWB( const libraw_iparams_t &P );
    int  preprocessRaw( const char *path );
    int  postprocessRaw();
    void outputACES( const char *path );

    void initialize( const dataPath &dp );
    void setPixels( libraw_processed_image_t *image );
    void gatherSupportedIllums();
    void gatherSupportedCameras();
    void printLibRawCameras();
    void applyWB( float *pixels, int bits, uint32_t total );
    void applyIDT( float *pixels, int bits, uint32_t total );
    void applyCAT( float *pixels, int channel, uint32_t total );
    void acesWrite( const char *name, float *aces, float ratio = 1.0 ) const;

    float *renderACES();
    float *renderDNG();
    float *renderNonDNG();
    float *renderIDT();

    const vector<string>            getSupportedIllums() const;
    const vector<string>            getSupportedCameras() const;
    const vector<vector<double>>    getIDTMatrix() const;
    const vector<vector<double>>    getCATMatrix() const;
    const vector<double>            getWB() const;
    const libraw_processed_image_t *getImageBuffer() const;
    const struct Option             getSettings() const;

private:
    AcesRender();
    ~AcesRender();
    static AcesRender &getPrivateInstance();

    const AcesRender &operator=( const AcesRender &acesrender );

    char                     *_pathToRaw;
    core::Idt                *_idt;
    libraw_processed_image_t *_image;
    LibRawAces               *_rawProcessor;

    Option                 _opts;
    vector<vector<double>> _idtm;
    vector<vector<double>> _catm;
    vector<double>         _wbv;
    vector<string>         _illuminants;
    vector<string>         _cameras;
};
#endif
