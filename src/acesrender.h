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

#ifndef _ACESRENDER_h__
#define _ACESRENDER_h__

#include <aces/aces_Writer.h>
#include <libraw/libraw.h>
#include "../lib/rta.h"

using namespace rta;

class LibRawAces : virtual public LibRaw {
    public:
        void show () {
            printf ("I am here with LibRawAces.\n");
        }
};

class AcesRender {
    public:
        AcesRender();
        ~AcesRender();
    
        int fetchCameraSenPath( libraw_iparams_t P );
        int fetchIlluminant( const char * illumType = "na" );
    
        int prepareIDT ( libraw_iparams_t P, float * M );
        int prepareWB ( libraw_iparams_t P );
    
        void setOptions (option opts);
        void setPixels (libraw_processed_image_t * image);
        void gatherSupportedIllums ();
        void gatherSupportedCameras ();
        void applyWB  ( float * pixels, int bits, uint32_t total );
        void applyIDT ( float * pixels, int bits, uint32_t total );
        void applyCAT ( float * pixels, int channel, uint32_t total );
        void acesWrite ( const char * name, float *  aces, float ratio = 1.0) const;

        float * renderDNG ( vector < float > cameraToDisplayMtx );
        float * renderNonDNG ();
        float * renderNonDNG_IDT ();
    
        const vector < string > getSupportedIllums () const;
        const vector < string > getSupportedCameras () const;
        const vector < vector < double > > getIDTMatrix () const;
        const vector < vector < double > > getCATMatrix () const;
        const vector < double > getWB () const;
    
    private:
        Idt * _idt;
        libraw_processed_image_t * _image;
        vector < vector < double > > _idtm;
        vector < vector < double > > _catm;
        vector < double > _wbv;
        vector < string > _illuminants;
        vector < string > _cameras;
        option _opts;
};
#endif
