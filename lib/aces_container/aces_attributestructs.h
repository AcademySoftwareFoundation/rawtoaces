// Copyright © 2015 Academy of Motion Picture Arts and Sciences ("A.M.P.A.S.").
// Portions contributed by others as indicated. All rights reserved.
// 
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted, subject 
// to acceptance of this license. Performance of any of the aforementioned acts
// indicates acceptance to be bound by the following terms and conditions:
// 
// * Copies of source code, in whole or in part, must retain the above copyright
// notice, this list of conditions and the Disclaimer of Warranty.
// 
// * Use in binary form must retain the above copyright notice, this list of
// conditions and the Disclaimer of Warranty in the documentation and/or other
// materials provided with the distribution.
// 
// * Nothing in this license shall be deemed to grant any rights to trademarks,
// copyrights, patents, trade secrets or any other intellectual property of
// A.M.P.A.S. or any contributors, except as expressly stated herein.
// 
// * Neither the name "A.M.P.A.S." nor the name of any other contributors to 
// this software may be used to endorse or promote products derivative of or 
// based on this software without express prior written permission of A.M.P.A.S. 
// or the contributors, as appropriate.
// 
// This license shall be construed pursuant to the laws of the State of 
// California, and any disputes related thereto shall be subject to the 
// jurisdiction of the courts therein.
// 
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT 
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
// PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL 
// A.M.P.A.S., OR ANY CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, RESITUTIONARY, OR CONSEQUENTIAL 
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY SPECIFICALLY
// DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER RELATED TO PATENT OR
// OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACES CONTAINER REFERENCE
// IMPLEMENTATION, OR APPLICATIONS THEREOF, HELD BY PARTIES OTHER THAN 
// A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
//
// Derived from : 
//
// cd_openEXRattributestructs.h
// ACES Container Writer
//
// and 
//
// cd_chromaticites.h
// ACES Container Writer
//
// License Terms for the ACES Container Writer 
// 
// The ACES Container Writer is provided by Adobe Systems Incorporated under the 
// following terms and conditions:
// 
// Copyright © 2015 Adobe Systems Incorporated ("Adobe"). All rights reserved.
// 
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted, subject 
// to acceptance of this license. Performance of any of the aforementioned acts
// indicates acceptance to be bound by the following terms and conditions:
// 
// * Copies of source code, in whole or in part, must retain the above copyright
// notice, this list of conditions and the Disclaimer of Warranty.
// 
// * Use in binary form must retain the above copyright notice, this list of
// conditions and the Disclaimer of Warranty in the documentation and/or other
// materials provided with the distribution.
// 
// * Nothing in this license shall be deemed to grant any rights to trademarks,
// copyrights, patents, trade secrets or any other intellectual property of 
// Adobe or any contributors, except as expressly stated herein.
// 
// * Neither the name "Adobe" nor the name of any other contributors to this
// software may be used to endorse or promote products derivative of or based on
// this software without express prior written permission of Adobe or the
// contributors, as appropriate.
// 
// This license shall be construed pursuant to the laws of the State of 
// California, and any disputes related thereto shall be subject to the 
// jurisdiction of the courts therein.
// 
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY ADOBE AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
// AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL ADOBE, OR ANY 
// CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, RESITUTIONARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, ADOBE SPECIFICALLY
// DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER RELATED TO PATENT OR
// OTHER INTELLECTUAL PROPERTY RIGHTS IN ACES CONTAINER WRITER, OR
// APPLICATIONS THEREOF, HELD BY PARTIES OTHER THAN ADOBE, WHETHER DISCLOSED OR
// UNDISCLOSED.

//	=====================================================================
//  structures for the attributes in the Aces header
//	=====================================================================


#ifndef __aces_oeStructs__
#define __aces_oeStructs__

//#include "aces_iostat.h"
#include "aces_typesAndRationals.h"

#include <string>
#include <vector>


const uint64 maxAcesHeaderSize = 1048576;

//	======================================

struct box2i {
	int32 xMin;
	int32 yMin;
	int32 xMax;
	int32 yMax;
	
	box2i () 
	{
		xMin = yMin = xMax = yMax = 0;
	}		
};

//	======================================

struct channelInfo {
	string	name;	// Shall be one of R, G, B
	int32	pixelType;	
	uint32	pLinear;
	int32	xSampling;
	int32	ySampling;
	
	// fixed values, shall be {1, 0, 1, 1}
	channelInfo()
	{ 
		pixelType = xSampling = ySampling = 1;
		pLinear = 0;
	}
};

ostream& operator<< ( ostream& s, const channelInfo& v );

typedef vector <channelInfo> chlist;

ostream& operator<< ( ostream& s, const chlist& v );

//	======================================
//	compression and line order must be distinct types for wrtAttr, thus struct instead of char.

struct compression {
	unsigned char c;	// shall be 0
};

//	======================================

struct lineOrder {
	char l;		//	0 .. 1
};

//	======================================

struct keycode {
	int32 filmMfcCode;		//  0 .. 99
	int32 filmType;			//	0 .. 99
	int32 prefix;			//	0 .. 999999
	int32 count;			//	0 .. 9999
	int32 perfOffset;		//	1 .. 119
	int32 perfsPerFrame;	//	1 .. 15
	int32 perfsPerCount;	//	20 .. 120
	
	keycode ();
	keycode ( int32 filmMfcCode, int32 filmType, int32 prefix, int32 count, int32 perfOffset, int32 perfsPerFrame, int32 perfsPerCount );
};

ostream& operator<< ( ostream& s, const keycode& v );

//	======================================

struct timecode {
	uint32 timeAndFlags;
	uint32 userData;
	
	timecode ( uint32 timeAndFlags1 = 0, uint32 userData1 = 0 );
};

ostream& operator<< ( ostream& s, const timecode& t );

//	======================================

struct v2f {
	float x;
	float y;
	
	v2f ( float x1 = 0, float y1 = 0 );
};

ostream& operator<< ( ostream& s, const v2f& v );

//	======================================

struct v3f {
	float x;
	float y;
	float z;
	
	v3f ( float x1 = 0, float y1 = 0, float z1 = 0 );
};

ostream& operator<< ( ostream& s, const v3f& v );

//	======================================

struct chromaticities {
	v2f red;
	v2f green;
	v2f blue;
	v2f white;
};

ostream& operator<< ( ostream& s, const chromaticities& v );

//	======================================

//	collection of required and optional attributes prepared for aces

struct acesHeaderInfo  {

	acesHeaderInfo (); 
	
	//	required ACES attributes
	int32			acesImageContainerFlag;			//	1
	chlist			channels;			//	B, G, R * {1,0,1,1}
	chromaticities	Chromaticities;		//  use fixed values for ACES	
	compression		Compression;		//	0
	box2i			dataWindow;			//	MUST BE FILLED IN EXPLICITLY
	box2i			displayWindow;		//	MUST BE FILLED IN EXPLICITLY
	lineOrder		LineOrder;			//	0 .. 1
	float			pixelAspectRatio;	
	v2f				screenWindowCenter;
	float			screenWindowWidth;

	//	optional attributes
	//	static or semi-dynamic (may change with image)
	float			altitude;				//	meters above sea	TIFF GPS tags 5 & 6
	float			aperture;				//						TIFF tag 33437 F-number or  51058 tstop
	string			cameraFirmwareVersion;			
	string			cameraIdentifier;			
	string			cameraLabel;			
	string			cameraMake;				//						TIFF tag 271 make
	string			cameraModel;			//						TIFF tag 272 model or  50708 UniqueCameraModel
	v3f				cameraUpDirection;
	v3f				cameraViewingDirection;
	v3f				cameraPosition;					
	string			cameraSerialNumber;		//						TIFF tag 50735	CameraSerialNumber
	srational		captureRate;			//	fps
	string			comments;				//						TIFF tag 270 
	float			convergenceDistance;	//	meters
	string			creator;				//						dublin core
	float			expTime;				//	seconds (>0)		TIFF tag 33434
	float			focalLength;			//	millimeters			TIFF tag 37386
	float			focus;					//	meters (>0)
	srational		framesPerSecond;		//	playback rate fps	TIFF tag 51044	
	string			free;					//	file specific
	string			headerChecksum;			//	file specific
	string			imageChecksum;			//	file specific
	float			imageRotation;			//	degrees
	float			interocularDistance;	//	meters
	float			isoSpeed;				//	exposure index rating setting on camera ~TIFF tag 37397
	float			latitude;				//	degrees North		TIFF GPS tags 1 & 2
	string			lensAttributes;
	string			lensMake;				//						XMP
	string			lensModel;				//						XMP	
	string			lensSerialNumber;		//						XMP		
	float			longitude;				//	degrees East		TIFF GPS tags 3 & 4
	vector<string>	multiView;
	int32			originalImageFlag;		//	1 = unedited original, 0 = changed
	string			owner;
	string			recorderFirmwareVersion;			
	string			recorderMake;			
	string			recorderModel;		
	string			recorderSerialNumber;		
	string			reelName;		
	string			storageMediaSerialNumber;		
	int32			timecodeRate;			//	fps
	float			utcOffset;				//	-seconds, = (UTC time - local time)	TIFF tag 34858
	
	//	dynamic == expected to be unique for each image
	string			capDate;				//	YYYY:MM:DD hh:mm:ss = capture time =  TIFF tag 36867 dateTimeOriginal
	int32			imageCounter;			//						TIFF tag 37393	imageNumber
	keycode			keyCode;	
	timecode		timeCode;				//						TIFF tag 51043
	string			uuid;					//						TIFF tag 42016 ImageUniqueID
	
	
	//	sample custom (TIFF/EP) attributes not defined in the ACES spec
	string			artist;					//						TIFF tag 315	artist
	string			copyright;				//						TIFF tag 33432	copyright
	string			dateTime;				//	YYYY-MM-DDThh:mm:ssTZ when THIS file was created	TIFF 306 dateTime
	int32			orientation;			//	flipped, rotated	TIFF tag 274	orientation
	string			software;				//						TIFF tag 305	software
};

ostream& operator<< ( ostream& s, const acesHeaderInfo& v );

#endif
