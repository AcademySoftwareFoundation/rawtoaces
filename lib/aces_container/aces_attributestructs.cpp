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
// cd_openEXRattributestructs.cpp
// ACES Container Writer
//
// and 
//
// cd_chromaticites.cpp
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

#include "aces_attributestructs.h"

//	======================================

ostream& operator<< ( ostream& s, const chromaticities& v )
{
	return s << "chromaticities <"
	<<   " red("	<< v.red.x	<< ", " << v.red.y
	<< "), green("	<< v.green.x << ", "	<< v.green.y 
	<< "), blue("	<< v.blue.x	<< ", "	<< v.blue.y 
	<< "), white("	<< v.white.x << ", "	<< v.white.y
	<< ") >";
}

//	======================================

ostream& operator<< ( ostream& s, const channelInfo& v )
{
	return s
	<< "  name:\t"			<< v.name 
	<< ", pixelType:\t"		<< v.pixelType 
	<< ", pLinear:\t"			<< v.pLinear
	<< ", xSampling:\t"		<< v.xSampling 
	<< ", ySampling:\t"		<< v.ySampling 
	;
}


ostream& operator<< ( ostream& s, const chlist& v )
{
	s << " <";
	
	for (uint32 i = 0; i < v.size(); i++) {
		if (i > 0) 
			s << ",";
		s << "\n\t\tchannelInfo[" << i << "] : " << v[i];
	}
	return s << "\n\t >";
}



keycode::	keycode()
{
	filmMfcCode = filmType = prefix = count = perfOffset = perfsPerFrame = perfsPerCount = 0;
}

keycode::	keycode ( int32 filmMfcCode1, int32 filmType1, int32 prefix1, int32 count1, 
					 int32 perfOffset1, int32 perfsPerFrame1, int32 perfsPerCount1 )
{
	filmMfcCode = filmMfcCode1; 
	filmType = filmType1; 
	prefix = prefix1; 
	count = count1; 
	perfOffset = perfOffset1; 
	perfsPerFrame = perfsPerFrame1; 
	perfsPerCount = perfsPerCount1;
}

ostream& operator<< ( ostream& s, const keycode& v )
{
	return s << "<"
	<< " filmMfcCode:\t"		<< v.filmMfcCode 
	<< ", filmType:\t"			<< v.filmType 
	<< ", prefix:\t"			<< v.prefix 
	<< ", count:\t"				<< v.count 
	<< ", perfOffset:\t"		<< v.perfOffset 
	<< ", perfsPerFrame:\t"		<< v.perfsPerFrame 
	<< ", perfsPerCount:\t"		<< v.perfsPerCount
	<< " >";
}

//	======================================

timecode::	timecode ( uint32 timeAndFlags1, uint32 userData1 )
{
	timeAndFlags = timeAndFlags1; userData = userData1;
}

ostream& operator<< ( ostream& s, const timecode&  t )
{
	return s << "not implemented";
}

//	======================================

v2f::	v2f ( float x1, float y1 )	
: x ( x1 ), y ( y1 )
{
}

ostream& operator<< ( ostream& s, const v2f& v )
{
	return s << "(" << v.x << "," << v.y << ")";
}

//	======================================

v3f::	v3f ( float x1, float y1, float z1 )	
: x ( x1 ), y ( y1 ), z ( z1 )
{
}

ostream& operator<< ( ostream& s, const v3f& v )
{
	return s << "(" << v.x << "," << v.y << "," << v.z << ")";
}


//	======================================

chromaticities ChromaticitiesForACES  	= { 
	v2f(0.73470f, 0.26530f),
	v2f(0.00000f, 1.00000f),
	v2f(0.00010f,-0.07700f),
	v2f(0.32168f, 0.33767f) };	//aces

acesHeaderInfo:: acesHeaderInfo()
: 
//	initialize required attributes to required, or default, values
acesImageContainerFlag	( 1 ), 
pixelAspectRatio	( 1.0f ),
screenWindowWidth	( 1.0f ),


//	initialize optional attributes to "ignored" values
altitude			( 0.0f ),
aperture			( 0.0f ), 
captureRate 		( 0, 1 ),
convergenceDistance ( 0.0f ),
expTime				( 0.0f ), 
focalLength			( 0.0f ),
focus				( 0.0f ), 
framesPerSecond		( 0, 1 ),
imageCounter		( 0 ), 
imageRotation		( 0.0f ),
interocularDistance ( 0.0f ),
isoSpeed			( 0.0f ), 
latitude			( 0.0f ), 
longitude			( 0.0f ), 
orientation			( 0 ), 
originalImageFlag	( -1 ), 
timecodeRate		( 0 ), 
utcOffset			( 0.0f )

{	
	//	initialize required attributes to required values
	Chromaticities			=	ChromaticitiesForACES;
	Compression.c			=	0;
	LineOrder.l				=   0; 
	
	//	initialize required attributes to default, values
	channels.resize(3);
	for (size_t i = 0; i < channels.size(); i++) {
		channels[i].name = "BGR"[i];
	}
	
}

ostream& operator<< ( ostream& s, const acesHeaderInfo& v )
{
	s 	<< "acesHeaderInfo <"
		<< "\n (required attributes )" 
		<< "\n\tacesImageContainerFlag:\t"	<< v.acesImageContainerFlag 
		<< "\n\tchannels      :\t"			<< v.channels 
		<< "\n\tChromaticities:\t"			<< v.Chromaticities 
		<< "\n\tcompression   :\t"			<< v.Compression.c 
		<< "\n\tlineOrder     :\t"			<< v.LineOrder.l	 
		<< "\n\tpixelAspectRatio:\t"		<< v.pixelAspectRatio 
		<< "\n\tscreenWindowCenter:\t"		<< v.screenWindowCenter 
		<< "\n\tscreenWindowWidth:\t"		<< v.screenWindowWidth
	
		<< "\n (optional attributes )" 
		<< "\n\taltitude   :\t"				<< v.altitude 
		<< "\n\taperture   :\t"				<< v.aperture 
		<< "\n\tartist     :\t"				<< v.artist 
		<< "\n\tcameraFirmwareVersion:\t"	<< v.cameraFirmwareVersion
        << "\n\tcameraIdentifier:\t"		<< v.cameraIdentifier
		<< "\n\tcameraLabel:\t"				<< v.cameraLabel
		<< "\n\tcameraModel:\t"				<< v.cameraModel 
		<< "\n\tcameraUpDirection:\t"		<< v.cameraUpDirection 
		<< "\n\tcameraViewingDirection:\t"  << v.cameraViewingDirection
		<< "\n\tcameraPosition:\t"			<< v.cameraPosition 
		<< "\n\tcameraSerialNumber:\t"		<< v.cameraSerialNumber 	
		<< "\n\tcapDate    :\t"				<< v.capDate 
		<< "\n\tcaptureRate:\t"				<< v.captureRate 
		<< "\n\tcomments   :\t"				<< v.comments 
		<< "\n\tconvergenceDistance:\t"		<< v.convergenceDistance 
		<< "\n\tcopyright  :\t"				<< v.copyright 
		<< "\n\tcreator    :\t"				<< v.creator 
		<< "\n\tdateTime   :\t"				<< v.dateTime 
		<< "\n\texpTime    :\t"				<< v.expTime 
		<< "\n\tfocalLength:\t"				<< v.focalLength 
		<< "\n\tfocus      :\t"				<< v.focus 
		<< "\n\tframesPerSecond:\t"			<< v.framesPerSecond 
		<< "\n\tfree       :\t"				<< v.free 
		<< "\n\theaderChecksum:\t"			<< v.headerChecksum 
		<< "\n\timageChecksum:\t"			<< v.imageChecksum 
		<< "\n\timageCounter:\t"			<< v.imageCounter
		<< "\n\timageRotation:\t"			<< v.imageRotation 
		<< "\n\tinterocularDistance:\t"		<< v.interocularDistance 
		<< "\n\tisoSpeed   :\t"				<< v.isoSpeed
		<< "\n\tkeyCode    :\t"				<< v.keyCode 
		<< "\n\tlatitude   :\t"				<< v.latitude 	
		<< "\n\tlensAttributes:\t"			<< v.lensAttributes 
		<< "\n\tlensMake   :\t"				<< v.lensMake 
		<< "\n\tlensModel  :\t"				<< v.lensModel 
		<< "\n\tlensSerialNumber:\t"		<< v.lensSerialNumber 	
		<< "\n\tlongitude  :\t"				<< v.longitude;
		
	s	<< "\n\tmultiView  :\t";
	if( v.multiView.size() > 0 )
	s	<< v.multiView[0];
	if( v.multiView.size() > 1 )
	s 	<< "," <<  v.multiView[1];
	
	s	<< "\n\torientation:\t"				<< v.orientation
		<< "\n\toriginalImageFlag:\t"		<< v.originalImageFlag 
		<< "\n\towner      :\t"				<< v.owner 
		<< "\n\trecorderFirmwareVersion:\t"	<< v.recorderFirmwareVersion 
		<< "\n\trecorderMake     :\t"		<< v.recorderMake 
		<< "\n\trecorderModel    :\t"		<< v.recorderModel 
		<< "\n\trecorderSerialNumber:\t"	<< v.recorderSerialNumber 
		<< "\n\treelName   :\t"				<< v.reelName 
		<< "\n\tsoftware   :\t"				<< v.software 
		<< "\n\tstorageMediaSerialNumber:\t"<< v.storageMediaSerialNumber 
		<< "\n\ttimeCode   :\t"				<< v.timeCode 
		<< "\n\ttimecodeRate:\t"			<< v.timecodeRate 
		<< "\n\tutcOffset  :\t"				<< v.utcOffset 
		<< "\n\tuuid       :\t"				<< v.uuid 
		<< "\n >\n";
	return s;
}
