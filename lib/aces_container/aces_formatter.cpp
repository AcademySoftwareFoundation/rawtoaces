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
// cd_openEXRformatter.cpp
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

#include "aces_formatter.h"

#include "aces_log.h"

#include <iostream>
#include <cassert>

struct scanLineHeader {
	int32	yCoordinate;
	int32	pixelDataSize;
};


//	=====================================================================
//	Get image dimensions height width,  min y

void aces_formatter:: getImageDimensions() 
{
	imageHeight = hi.dataWindow.yMax - hi.dataWindow.yMin + 1;
	imageWidth = hi.dataWindow.xMax - hi.dataWindow.xMin + 1;
	yMin = hi.dataWindow.yMin;	
}

//	Calculate sizes for bytes per pixel, bytes per scanline

void aces_formatter:: getSizes ()
{	
	uint32		pixelSize = 0;
	uint32		pixelSizeByType[] = { sizeof(uint32), sizeof(halfBytes), sizeof(real32) };
	
	for (size_t i = 0; i < hi.channels.size(); i++)
	{
		assert ( hi.channels[i].pixelType <= 2 );
		pixelSize += pixelSizeByType [ hi.channels[i].pixelType ];
	}
	
	scanLineSize = imageWidth * pixelSize;	
	scanLineSizeOH = sizeof(scanLineHeader) + scanLineSize;	
	assert( sizeof(scanLineHeader) == 8 ); 
}

//	Calculate all offsets to scan lines
//		and the total size of the file

void  aces_formatter:: createLineOffsetTableAndOffsets ()
{
	//	you need to know where the header ends before this can be calculated
	uint32		lineOffsetTableSize = imageHeight * sizeof( uint64 );
	beginScanLineStoragePosition = (uint32) lineOffsetTablePosition + lineOffsetTableSize;
	
	LineOffsetTable.resize ( imageHeight );	
	for (uint32 i = 0; i < imageHeight; i++) {
		LineOffsetTable [i] = (uint64) beginScanLineStoragePosition +  i * scanLineSizeOH;
	}	
	endScanLineStoragePosition = (uint64) beginScanLineStoragePosition + imageHeight * scanLineSizeOH;
	assert ( endScanLineStoragePosition <= outputBufferSize );
	assert ( lineOffsetTablePosition < endScanLineStoragePosition );
}

//	=====================================================================
//	initialize output stream
//	write metadata part of output image
//	get scan line storage locations
//	
//	return actual file size
//

uint64 aces_formatter:: 
writeAllButScanlines ( const acesHeaderInfo & hi1, 
					  char * pOutputBuffer1, 
					  uint64 outputBufferSize1 ) 	
{
	hi					= hi1;
	pOutputBuffer		= pOutputBuffer1;
	outputBufferSize	= outputBufferSize1;
	
	getImageDimensions ();
	getSizes ();

	// write to file
	writeHeader ( hi, pOutputBuffer, outputBufferSize );
	
	// create line offset table, and write after header
		
	createLineOffsetTableAndOffsets ();
	
	writeLineOffsetTable ( LineOffsetTable );
	
	return endScanLineStoragePosition;
}

//	=====================================================================
//	Reorder scan line values by color
//	Store RGB[A] row by index in output image buffer

void aces_formatter::  writeHalfLine ( const halfBytes * rgbHalfRow, uint32 row)
{
	const halfBytes * pR = rgbHalfRow;
	const halfBytes * pREnd = pR + hi.channels.size() * imageWidth;
	
	scanLineHeader sh = { yMin + row, scanLineSize };
	
	char * scanlineStart = pOutputBuffer + LineOffsetTable [row];
	
	* (scanLineHeader *) scanlineStart = sh;
	
	char * pScan  = scanlineStart + sizeof( scanLineHeader );
	//	========================================
	//	Performance critical section
	//	========================================			

	// only support 3 or 4 channel files in ACES
	if ( hi.channels.size() == 4 )
	{
		halfBytes * pScanA = (halfBytes *) pScan;
		halfBytes * pScanB = pScanA + imageWidth;
		halfBytes * pScanG = pScanB + imageWidth;
		halfBytes * pScanR = pScanG + imageWidth;

		while ( pR < pREnd )
		{
			*pScanR++ = *pR++;
			*pScanG++ = *pR++;
			*pScanB++ = *pR++;
			*pScanA++ = *pR++;
		}
	}
	else
	{
		halfBytes * pScanB = (halfBytes *) pScan;
		halfBytes * pScanG = pScanB + imageWidth;
		halfBytes * pScanR = pScanG + imageWidth;
	
		while ( pR < pREnd )
		{
			*pScanR++ = *pR++;
			*pScanG++ = *pR++;
			*pScanB++ = *pR++;
		}
	}
	
	//	========================================
 	//	End of Performance critical section
	//	======================================== 	
	return;
}

//	=====================================================================
//	set up scan line header
//	return pointer to pixel data

halfBytes * aces_formatter::  spaceForScanLine ( uint32 row )
{
	scanLineHeader sh = { yMin + row, scanLineSize };
	
	char * scanlineStart = pOutputBuffer + LineOffsetTable [row];
	
	* (scanLineHeader *) scanlineStart = sh;
	
	char * pScan  = scanlineStart + sizeof( scanLineHeader );	
		
	return (halfBytes *) pScan;
}



