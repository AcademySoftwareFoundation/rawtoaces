// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifndef _ACESRENDER_h__
#define _ACESRENDER_h__

#include <rawtoaces/rawtoaces_core.h>
#include <rawtoaces/define.h>

#include <unordered_map>
#include <libraw/libraw.h>

using namespace rta;

void create_key( std::unordered_map<std::string, char> &keys );
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

    const std::vector<std::string>         getSupportedIllums() const;
    const std::vector<std::string>         getSupportedCameras() const;
    const std::vector<std::vector<double>> getIDTMatrix() const;
    const std::vector<std::vector<double>> getCATMatrix() const;
    const std::vector<double>              getWB() const;
    const libraw_processed_image_t        *getImageBuffer() const;
    const struct Option                    getSettings() const;

private:
    AcesRender();
    ~AcesRender();
    static AcesRender &getPrivateInstance();

    const AcesRender &operator=( const AcesRender &acesrender );

    char                     *_pathToRaw;
    core::Idt                *_idt;
    libraw_processed_image_t *_image;
    LibRawAces               *_rawProcessor;

    Option                           _opts;
    std::vector<std::vector<double>> _idtm;
    std::vector<std::vector<double>> _catm;
    std::vector<double>              _wbv;
    std::vector<std::string>         _illuminants;
    std::vector<std::string>         _cameras;
};
#endif
