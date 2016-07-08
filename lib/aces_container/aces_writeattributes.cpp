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
// cd_writeOpenEXRattributes.cpp
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

#include "aces_writeattributes.h"
#include "aces_log.h"
#include "aces_timing.h"

#include "aces_md5.hh"

#include <cassert>

using namespace std;

//	=====================================================================
//	Created once and shared across images and clips

aces_writeattributes:: aces_writeattributes()
{
	//	Not that we really need a stream. We could just use a char * pointer.
	
    outputBuffer = new char[maxAcesHeaderSize + 2 * sizeof(int32)];
    
    pos = 0;

	UseLongAttributeNamesAndTypes = false;
	
	DetectHostByteOrder (  );
}

aces_writeattributes:: ~aces_writeattributes()
{

}

//	=====================================================================
//	Manage streams
//	=====================================================================

//	set the image output buffer as the target buffer for the string stream
void aces_writeattributes:: SetStreamBuffer( char * outputBuffer1, uint64 outputBufferSize1 )
{
    if (outputBuffer1) {
        delete [] outputBuffer;
        outputBuffer = outputBuffer1;
    }
    
	outputBufferSize = outputBufferSize1;
}

//	=====================================================================

streampos aces_writeattributes:: StreamPosition()
{
    return (streampos) pos;
}

//	=====================================================================

void aces_writeattributes:: SetStreamPosition( const streampos position )
{
    pos = (uint64) position;
}

//	=====================================================================

inline void aces_writeattributes:: writeChar	( const char value )
{
    assert( pos <= maxAcesHeaderSize + 2 * sizeof(int32) );
    outputBuffer[pos++] = value;
}

//	=====================================================================
//  write in LittleEndianOrder
//	=====================================================================

//	Swap multi-byte values

void aces_writeattributes:: DetectHostByteOrder ( void )
{
	uint16 orderTest =  1;	//  1 is in ls byte
	HostByteOrderIsLittleEndian = * (uint8 *) & orderTest == 1; 
}

void aces_writeattributes:: write2Bytes	( const uint16 * value )
{
    assert( pos <= maxAcesHeaderSize + 2 * sizeof(int32) );

	if (HostByteOrderIsLittleEndian) {
        
        for (int i = 0; i < 2; i++) {
            outputBuffer[pos++] = ((char *) value)[i];
        }
	}	else {
		writeChar ( ((char *) value) [1] );
		writeChar ( ((char *) value) [0] );
	}
}

void aces_writeattributes:: write4Bytes	( const uint32 * value )
{
    assert( pos <= maxAcesHeaderSize + 2 * sizeof(int32) );

	if (HostByteOrderIsLittleEndian) {
        
        for (int i = 0; i < 4; i++) {
            outputBuffer[pos++] = ((char *) value)[i];
        }
	}	else {
		writeChar ( ((char *) value) [3] );
		writeChar ( ((char *) value) [2] );
		writeChar ( ((char *) value) [1] );
		writeChar ( ((char *) value) [0] );
	}
}

void aces_writeattributes:: write8Bytes	( const uint64 * value )
{
    assert( pos <= maxAcesHeaderSize + 2 * sizeof(int32) );

	if (HostByteOrderIsLittleEndian) {
        
        for (int i = 0; i < 8; i++) {
            outputBuffer[pos++] = ((char *) value)[i];
        }
	}	else {
		writeChar ( ((char *) value) [7] );
		writeChar ( ((char *) value) [6] );
		writeChar ( ((char *) value) [5] );
		writeChar ( ((char *) value) [4] );
		writeChar ( ((char *) value) [3] );
		writeChar ( ((char *) value) [2] );
		writeChar ( ((char *) value) [1] );
		writeChar ( ((char *) value) [0] );
	}
}

//	=====================================================================
//  write basic data types
//	=====================================================================

void aces_writeattributes:: writeBasicType	( const unsigned char value )
{
	writeChar ( value );
}

void aces_writeattributes:: writeBasicType	( const char value )
{
	writeChar ( value );
}

void aces_writeattributes:: writeBasicType	( const int16 value )
{
	write2Bytes ( (uint16 *) &value );
}

void aces_writeattributes:: writeBasicType	( const uint16 value )
{
	write2Bytes ( (uint16 *) &value );
}

void aces_writeattributes:: writeBasicType	( const int32 value )
{
	write4Bytes ( (uint32 *) &value );
}

void aces_writeattributes:: writeBasicType	( const  uint32 value )
{
	write4Bytes ( (uint32 *) &value );
}

void aces_writeattributes:: writeBasicType	( const real32 value )
{
	write4Bytes ( (uint32 *) &value );
}

void aces_writeattributes:: writeBasicType	( const uint64 value )
{
	write8Bytes ( (uint64 *) &value );
}

void aces_writeattributes:: writeBasicType	( const real64 value )
{
	write8Bytes ( (uint64 *) &value );
}

// =====================================================
//	write string without and with terminating 0

void aces_writeattributes:: writeStringNZ	( const string & value )
{    
    assert( pos <= maxAcesHeaderSize + 2 * sizeof(int32) );
    
    for (int32 i = 0; i < value.size(); i++) {
        outputBuffer[pos++] = ((char *)(value.c_str()))[i];
    }
}

void aces_writeattributes:: writeStringZ	( const string & value )
{
	writeStringNZ	( value );
	writeChar ( 0 );
}

// =====================================================
//  write attribute header
// =====================================================

void aces_writeattributes:: wrtAttrHeader ( const string & attributeName, 
												const string & attributeType, 
												const uint32 attributeSize )
{
	writeStringZ	( attributeName );
	writeStringZ	( attributeType );
	writeBasicType	( attributeSize );
	//	Log( setw(24) << attributeName << " = " << attributeType 
	//	<< " ( " << attributeSize << " bytes )" );
}

// =====================================================
//  write attribute types
// =====================================================

//	Integers
// =====================================================
void aces_writeattributes:: wrtAttr	( const string & attributeName, const uint8 & value )
{	
	wrtAttrHeader	( attributeName, "unsignedChar",  sizeof(uint8) );
	writeChar			( value );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const int16 & value )
{	
	wrtAttrHeader	( attributeName, "short",  sizeof(int16) );
	writeBasicType			( value );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const uint16 & value )
{	
	wrtAttrHeader	( attributeName, "unsignedShort",  sizeof(uint16) );
	writeBasicType			( value );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const int32 & value )
{	
	wrtAttrHeader	( attributeName, "int",  sizeof(int32) );
	writeBasicType			( value );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const uint32 & value )
{	
	wrtAttrHeader	( attributeName, "unsignedInt",  sizeof(uint32) );
	writeBasicType			( value );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const uint64 & value )
{	
	wrtAttrHeader	( attributeName, "unsignedLong",  sizeof(uint64) );
	writeBasicType			( value );	
}

// =====================================================
//	Reals
// =====================================================
void aces_writeattributes:: wrtAttr	( const string & attributeName, const real32 & value )
{	
	wrtAttrHeader	( attributeName, "float",  sizeof(real32) );
	writeBasicType			( value );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const real64 & value )
{	
	wrtAttrHeader	( attributeName, "double",  sizeof(real64) );
	writeBasicType			( value );	
}	

void aces_writeattributes:: wrtAttr	( const string & attributeName, const srational & value )
{
	wrtAttrHeader	( attributeName, "rational", 2 * sizeof(int32) );
	writeBasicType			( value.n );
	writeBasicType			( value.d );
}

// =====================================================
//	Structures
// =====================================================
void aces_writeattributes:: wrtAttr	( const string & attributeName, const box2i & value )
{
	wrtAttrHeader	( attributeName, "box2i", 4 * sizeof(int32) );
	writeBasicType			( value.xMin );
	writeBasicType			( value.yMin );
	writeBasicType			( value.xMax );
	writeBasicType			( value.yMax );
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const chlist & channels )
{
	uint32 attributeSize = 1;
	for (size_t i = 0; i < channels.size(); i++)
		attributeSize += channels[i].name.size() + 1 + 4 * sizeof(int32);
	
	wrtAttrHeader	( attributeName , "chlist", attributeSize );	

	for (size_t i = 0; i < channels.size(); i++)
	{
		writeStringZ		( channels[i].name );
		writeBasicType		( channels[i].pixelType );
		writeBasicType		( channels[i].pLinear );
		writeBasicType		( channels[i].xSampling );
		writeBasicType		( channels[i].ySampling );
	}
	writeChar ( 0 );
}	

void aces_writeattributes:: wrtAttr ( const string & attributeName, const chromaticities & value )
{	
	wrtAttrHeader	( attributeName, "chromaticities", 8 * sizeof(real32) );
	writeBasicType			( value.red.x );
	writeBasicType			( value.red.y );
	writeBasicType			( value.green.x );
	writeBasicType			( value.green.y );
	writeBasicType			( value.blue.x );
	writeBasicType			( value.blue.y );
	writeBasicType			( value.white.x );
	writeBasicType			( value.white.y );	
}	

void aces_writeattributes:: wrtAttr ( const string & attributeName, const compression & value )
{
	wrtAttrHeader	( attributeName, "compression",  sizeof(value.c) );
	writeBasicType			( value.c );	
}

void aces_writeattributes:: wrtAttr ( const string & attributeName, const lineOrder & value )
{
	wrtAttrHeader	( attributeName, "lineOrder",  sizeof(value.l) );
	writeBasicType			( value.l );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const keycode & value)
{
	wrtAttrHeader	( attributeName, "keycode",  7 * sizeof(int32) );
	writeBasicType			( value.filmMfcCode );
	writeBasicType			( value.filmType );
	writeBasicType			( value.prefix );
	writeBasicType			( value.count );
	writeBasicType			( value.perfOffset );
	writeBasicType			( value.perfsPerFrame );
	writeBasicType			( value.perfsPerCount );	
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const string & value )
{
	wrtAttrHeader	( attributeName, "string", value.size() );
	writeStringNZ			( value );
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const vector<string> & value )
{
	uint32	N = value.size();

	uint32 stringTotal = 0;
	for (uint32 i = 0; i < N; i++) {
		stringTotal			+= value[i].size();
	}

	wrtAttrHeader	( attributeName, "stringVector", N * sizeof(int32) + stringTotal );
	
	for (uint32 i = 0; i < N; i++) {
		writeBasicType			( (int32) value[i].size() );
		writeStringNZ			( value[i] );
	}
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const timecode & value )
{
	wrtAttrHeader	( attributeName, "timecode",  2 * sizeof(int32) );
	writeBasicType			( value.timeAndFlags );
	writeBasicType			( value.userData );
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const v2f & value )
{
	wrtAttrHeader	( attributeName, "v2f",  2 * sizeof(real32) );
	writeBasicType			( value.x );
	writeBasicType			( value.y );
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const v3f & value )
{
	wrtAttrHeader	( attributeName, "v3f",  3 * sizeof(real32) );
	writeBasicType			( value.x );
	writeBasicType			( value.y );
	writeBasicType			( value.z );
}

void aces_writeattributes:: wrtAttr	( const string & attributeName, const vector<v3f> & value )
{
	uint32	N = value.size();
	wrtAttrHeader	( attributeName, "v3f",  N * 3 * sizeof(real32) );
	for (uint32 i = 0; i < N; i++) {
		writeBasicType			( value[i].x );
		writeBasicType			( value[i].y );
		writeBasicType			( value[i].z );
	}
}

// =====================================================
//	aces File layout
// =====================================================

//  write file start

void aces_writeattributes:: writeMagicNumberAndVersion ()
{
	SetStreamPosition( 0 );
	headerChecksumPosition = imageChecksumPosition = 0;
	
	const int32 MagicNumber = 20000630;
	const int32	VersionField = 2;
	const int32	LongAttributeFlag = 0x400;
	
	writeBasicType			( MagicNumber );
	
	if ( UseLongAttributeNamesAndTypes ) {
		writeBasicType			( VersionField | LongAttributeFlag );	
	} else {
		writeBasicType			( VersionField );	
	}
}

//	=====================================================
//	Write 2 two-pass attributes that include file metrics
// =====================================================

//	headerChecksum

const string md5ZeroChecksumString(16,0);

void aces_writeattributes:: writeHeaderChecksumPass1 ( const string value)
{
	if ( value == "" )
	{
		headerChecksumPosition = 0;
	} else {
		wrtAttr	(	"headerChecksum", md5ZeroChecksumString ); // write placeholder dummy value
		headerChecksumPosition = StreamPosition() -  (streampos) md5ZeroChecksumString.size();
	}
}

void aces_writeattributes:: setHeaderChecksum( void ) 
{
	if ( headerChecksumPosition > 0 )
	{
		//	clear out the current value
		SetStreamPosition( headerChecksumPosition );
		writeStringNZ ( md5ZeroChecksumString );
		
		//	update the attribute with the actual value
		SetStreamPosition( headerChecksumPosition );
		MD5			md5;
		assert( lineOffsetTablePosition < 1100000 );
		aces_timing timer;
		
		string md = md5.CalculateMD5Digest ( (const uchar_t*) outputBuffer, lineOffsetTablePosition );
		
//		timer.time("Digesting header data " );
		
		writeStringNZ ( md );
	}
}

//	=====================================================
//	imageChecksum

void aces_writeattributes:: writeImageChecksumPass1 ( const string value )
{
	if ( value == "" )
	{
		imageChecksumPosition = 0;
	} else {
		wrtAttr	(	"imageChecksum", md5ZeroChecksumString ); // write placeholder dummy value
		imageChecksumPosition = StreamPosition() - (streampos) md5ZeroChecksumString.size();
	}
}

void aces_writeattributes:: setChecksums ( )
{
	if ( imageChecksumPosition > 0 )
	{
		//	update the attribute with the actual value
		SetStreamPosition( imageChecksumPosition );
		MD5			md5;
		assert( beginScanLineStoragePosition < endScanLineStoragePosition );
		
		aces_timing timer;
		
		string md = md5.CalculateMD5Digest ( (const uchar_t*) outputBuffer 
											+ (uint32) beginScanLineStoragePosition,
											endScanLineStoragePosition - beginScanLineStoragePosition );
		
//		timer.time("Digesting scan line storage data " ); // 57 ms for 12 MB image
		
		writeStringNZ ( md );
	}		
	setHeaderChecksum ( );	//	imagechecksum affects headerchecksum
}

//	=====================================================================
//  write header and attributes
//	=====================================================================
//	MOVE TO ANOTHER FILE. Which attributes to write is application-specific.

void aces_writeattributes:: writeHeader ( acesHeaderInfo & hi,  
											  char * outputB,
											  uint64 outputBSize )
{
	SetStreamBuffer( outputB, outputBSize );

	writeMagicNumberAndVersion ();
	
	// required aces attributes
	wrtAttr	(	"acesImageContainerFlag",	hi.acesImageContainerFlag );
	wrtAttr	(	"channels",					hi.channels );
	wrtAttr	(	"chromaticities",			hi.Chromaticities );
	wrtAttr	(	"compression",				hi.Compression );
	wrtAttr	(	"dataWindow",				hi.dataWindow );
	wrtAttr	(	"displayWindow",			hi.displayWindow );
	wrtAttr	(	"lineOrder",				hi.LineOrder );
	wrtAttr	(	"pixelAspectRatio",			hi.pixelAspectRatio );
	wrtAttr	(	"screenWindowCenter",		hi.screenWindowCenter );
	wrtAttr	(	"screenWindowWidth",		hi.screenWindowWidth );
		
	// terminator
	writeChar ( 0 );
    
	lineOffsetTablePosition = StreamPosition();
		
	SetStreamPosition( lineOffsetTablePosition );
    
	assert ( lineOffsetTablePosition <= maxAcesHeaderSize + 2 * sizeof(int32) );// max header size
}	

//	=====================================================================
//	write Line Offset Table immediately after header

void aces_writeattributes:: writeLineOffsetTable ( vector <streampos> LineOffsetTable )
{
	SetStreamPosition( lineOffsetTablePosition );

	for ( size_t i = 0; i < LineOffsetTable.size(); i++) {
		writeBasicType ( (uint64) LineOffsetTable [i] );
	}
	
	beginScanLineStoragePosition = StreamPosition();
	
//	LogV(headerChecksumPosition);
//	LogV(imageChecksumPosition);
//
//	LogV(lineOffsetTablePosition);
//	LogV(beginScanLineStoragePosition);
//	LogV(endScanLineStoragePosition);
	
}

