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
// cd_writeOpenEXRattributes.h
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
//	Writes aces attributes
//	=====================================================================

#ifndef __aces_oeAttr__
#define __aces_oeAttr__

#include "aces_attributestructs.h"

#include <sstream>

//	Created once and shared across images and clips
class aces_writeattributes {
	
public:		
	//	Set image and header checksums (conditionally)
	//	Call once per image, after the image is stored in the output buffer
	void	setChecksums					( void );
	
	bool	UseLongAttributeNamesAndTypes;
	
	//	=====================================================================
protected:
	//	To be used by aces_formatter.cpp only
	
	//	Created once and shared across images and clips
	aces_writeattributes ();
	~aces_writeattributes ();
	
	//	Call once per image
	//	create the Aces header structure in an output buffer
	void	writeHeader						( acesHeaderInfo & hi,  char * outputBuffer, uint64 outputBufferSize );
	
	//	Call once per image
	//	add the line offset table to the output buffer
	void	writeLineOffsetTable			( const vector <streampos> LineOffsetTable );
	
	streampos	lineOffsetTablePosition;		// file offset to linOffsetTable
	streampos	beginScanLineStoragePosition;	// file offset to ScanLineStorage
	streampos	endScanLineStoragePosition;		// file offset to end of ScanLineStorage
	
	//	=====================================================================
private:
	
	char *			outputBuffer;
	uint64			outputBufferSize;
    uint64          pos;
	
	void		SetStreamBuffer				( char * outputBuffer, uint64 outputBufferSize );
	streampos	StreamPosition				( void );
	void		SetStreamPosition			( const streampos position );
	
	void writeMagicNumberAndVersion			( void );
	
	//	=====================================================================
	//	write special attributes
	streampos	headerChecksumPosition;
	streampos	imageChecksumPosition;
		
	//	calculate checksums
	void		writeHeaderChecksumPass1	( const string value );		// change to boolean?
	void		setHeaderChecksum			( void );
	void		writeImageChecksumPass1		( const string value );		// change to boolean?


	//	=====================================================================
	//	Write attributes and parts of attributes
	
	bool HostByteOrderIsLittleEndian;
	void DetectHostByteOrder ( void );
	
	inline void writeChar		( const char value );
	void write2Bytes	( const uint16 * value );
	void write4Bytes	( const uint32 * value );
	void write8Bytes	( const uint64 * value );
	
	
	void writeBasicType	( const unsigned char value );
	void writeBasicType	( const char value );
	void writeBasicType	( const int16 value );
	void writeBasicType	( const uint16 value );
	void writeBasicType	( const int32 value );
	void writeBasicType	( const uint32 value );
	void writeBasicType	( const uint64 value );
	void writeBasicType	( const float value );
	void writeBasicType	( const double value );
	
	void writeStringNZ	( const string & value );
	void writeStringZ	( const string & value );
	
	void wrtAttrHeader ( const string & attributeName, const string & attributeType, const uint32 attributeSize );
	
	void wrtAttr	( const string & attributeName, const uint8  & value );
	void wrtAttr	( const string & attributeName, const int16 & value );
	void wrtAttr	( const string & attributeName, const uint16 & value );
	void wrtAttr	( const string & attributeName, const int32 & value );
	void wrtAttr	( const string & attributeName, const uint32 & value );
	void wrtAttr	( const string & attributeName, const uint64 & value );

//	void wrtAttr	( const string & attributeName, const halfBytes & value ); // ambiguous with int16
	void wrtAttr	( const string & attributeName, const float & value );
	void wrtAttr	( const string & attributeName, const double & value );	
	
	void wrtAttr	( const string & attributeName, const box2i & value );	
	void wrtAttr	( const string & attributeName, const chlist & channels );	
	void wrtAttr	( const string & attributeName, const chromaticities & rgbw );	
	void wrtAttr	( const string & attributeName, const compression & value );	
	void wrtAttr	( const string & attributeName, const lineOrder & value );
	void wrtAttr	( const string & attributeName, const keycode & value);
	void wrtAttr	( const string & attributeName, const srational & value );
	void wrtAttr	( const string & attributeName, const string & value );
	void wrtAttr	( const string & attributeName, const vector<string> & value );
	void wrtAttr	( const string & attributeName, const timecode & value );
	void wrtAttr	( const string & attributeName, const v2f & value );
	void wrtAttr	( const string & attributeName, const v3f & value );
	void wrtAttr	( const string & attributeName, const vector<v3f> & value );

};

#endif
