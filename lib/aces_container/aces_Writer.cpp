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
// cd_openEXRWriter.cpp
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

#include "aces_crossplat.h"

#include "aces_Writer.h"

#include "aces_timing.h"
#include "aces_errors.h"
#include "aces_log.h"

#include <fstream>
#include <cstdio>
#include <ctime>
#include <cassert>

#ifdef _WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif

#include <fcntl.h>


//	=====================================================================
aces_Writer:: aces_Writer() { 
	pOutputBuffer = NULL; 
}

//	=====================================================================
aces_Writer:: ~aces_Writer() { 
	delete[] pOutputBuffer; 
}

//	=====================================================================
/*	provides info for preparing image data
 output:
 default acesHeaderInfo including chromaticity coordinates (ACES)
 */

acesHeaderInfo aces_Writer:: getDefaultHeaderInfo ()
{
	error = 0;
	return acesHeaderInfo();
}

//	=====================================================================
/*	set up the output for a clip
 
 
 inputs:
 image dimensions
 static metadata:
 output folder name with trailing /
 processing:
 clear output folder
 calculate buffer sizes
 set up output buffer
 locations for static metadata
 special for Aces: link table
 */

err aces_Writer:: configure( const MetaWriteClip & clipMeta )
{
	aces_timing timer;
	//	generic
	error = 0;
	outputRows = clipMeta.outputRows;
	outputCols = clipMeta.outputCols;
	hi = clipMeta.hi;
	
	assert ( outputRows > 0 );
	assert ( outputCols > 0 );
	
	outputFilenames = clipMeta.outputFilenames;
	
	// Aces
	hi.dataWindow.yMax = hi.dataWindow.yMin + outputRows - 1;
	hi.dataWindow.xMax = hi.dataWindow.xMin + outputCols - 1;
	hi.displayWindow = hi.dataWindow;
	switch ( hi.channels.size() )
	{
		case 3:
			if ( hi.channels[0].name != "B" || hi.channels[1].name != "G" ||
				 hi.channels[2].name != "R" )
				return aces_ErrorBadChannels;
			break;
		case 4:
			if ( hi.channels[0].name != "A" || hi.channels[1].name != "B" ||
				 hi.channels[2].name != "G" || hi.channels[3].name != "R" )
				return aces_ErrorBadChannels;
			break;
		case 6:
			if ( hi.channels[0].name != "B" || hi.channels[1].name != "G" ||
				 hi.channels[2].name != "R" || hi.channels[3].name != "left.B" ||
				 hi.channels[4].name != "left.G" || hi.channels[5].name != "left.R" )
				return aces_ErrorBadChannels;
			break;
		case 8:
			if ( hi.channels[0].name != "A" || hi.channels[1].name != "B" ||
				 hi.channels[2].name != "G" || hi.channels[3].name != "R" ||
				 hi.channels[4].name != "left.A" || hi.channels[5].name != "left.B" ||
				 hi.channels[6].name != "left.G" || hi.channels[7].name != "left.R" )
				return aces_ErrorBadChannels;
			break;
		default:
			return aces_ErrorBadChannels;
			break;
	}
	outputBufferSize = outputRows * (8 + 2 * 4 + outputCols * hi.channels.size() * 2) + 1100000;	
	
	assert ( outputBufferSize < 100e6 );
	
	delete[] pOutputBuffer;
	pOutputBuffer = new char [ (size_t) outputBufferSize ];
	assert ( pOutputBuffer != NULL );
	
	stat.timeConfigure += timer.time(); 

	return error;
}

//	=====================================================================
/*	Prepare for exporting a new image
 
 Call once for each image
 
 inputs:
 file name
 frame number for file name
 time code
 date-time
 processing:
 Convert metadata to Aces formats
 Refreshes the output buffer with dynamic metadata
 Clear row counters etc used by storeImageRow()
 */

err aces_Writer:: newImageObject ( const DynamicMetadata & imageMeta )
{
	aces_timing timer;
	//	generic
	error = 0;
	assert ( outputBufferSize > 0 );
	
	numberOfRowsWritten = 0;
	outputFileSize = 0;
	
	filename = outputFilenames[ imageMeta.imageIndex ];
		
	// save away some dynamic metadata
	char buffer [80];
	time_t rawtime = time ( NULL );
	strftime (buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%Z", localtime ( &rawtime));
	hi.dateTime		= buffer;
	hi.capDate		= imageMeta.capDate;
	hi.imageCounter = imageMeta.imageCounter;
	hi.keyCode		= imageMeta.keyCode;
	hi.timeCode		= imageMeta.timeCode;
	hi.uuid			= imageMeta.uuid;
	
	//	initialize output stream, write metadata part of output image
	//		get scan line locations, get final image size
	outputFileSize = oef.writeAllButScanlines( hi, pOutputBuffer, outputBufferSize );
	assert ( outputFileSize <= outputBufferSize );
	
	stat.timeNewImageObject += timer.time();
	return error;
}

//	=====================================================================
//	Add a row of output RGB to image:
//		call once per row
//	inputs:
//		row of output RGB values in half (size given in call to configure)
//		row number within image (to allow for out of order processing )
//	processing:
//		Reorder scan line values by color
//		Store RGB row by index in output image buffer
//	returns error code

err aces_Writer:: storeHalfRow ( const halfBytes * rgbHalfRow, uint32 row )
{
	aces_timing timer;
	//	generic
	error = 0;	
	assert ( rgbHalfRow != NULL);
	assert ( row < outputRows );
	assert ( numberOfRowsWritten < outputRows ); 
	
	numberOfRowsWritten++;

	if ( hi.channels.size() > 4 )
		return aces_ErrorNotYetImplemented;

	// Aces
	oef.writeHalfLine ( rgbHalfRow, row );
	
	stat.timeDataProcessing += timer.time(); 
	return error;
}

//	=====================================================================
//	get pointer to a row of output RGB to image:
//		call once per row
//	inputs:
//		row of output RGB values in half (size given in call to configure)
//		row number within image (to allow for out of order processing )

halfBytes * aces_Writer:: GetPointerToPixelData ( uint32 row )
{
	aces_timing timer;
	//	generic
	error = 0;	
	assert ( row < outputRows );
	assert ( numberOfRowsWritten < outputRows ); 
	
	numberOfRowsWritten++;
	
	// Aces
	halfBytes * halfRow = oef.spaceForScanLine ( row );
	
	stat.timeDataProcessing += timer.time(); 
	return halfRow;
}

//	=====================================================================
/* 	Complete the data structures and write to disk
 
 Call once for each image after all rows have been written
 
 inputs:
 none
 processing:
 check that all rows were received
 write to disk	 
 */

err aces_Writer:: saveImageObject ( )
{
	aces_timing timer;
	error = 0;
	//	Make final updates to header
	oef.setChecksums ( );
	stat.timeUpdateHeader += timer.time();
	
	//	generic
	assert ( numberOfRowsWritten == outputRows ); 
	assert ( outputFileSize > 0 );
	assert ( outputBufferSize >= outputFileSize );
	
	// write out the file
	assert ( filename != "" );

	
//	//	========================================
// 	//	Performance critical section
//	//	======================================== 	
//	
//	int outF = open(filename.c_str(), O_WRONLY | O_CREAT | O_NONBLOCK | O_TRUNC );
//	stat.timeOpen += timer.time();
//	
//	write(outF, pOutputBuffer, outputBufferSize);
//	stat.timeWrite += timer.time();
//	
//	close(outF);
//	stat.timeClose += timer.time();
//		
//	//	========================================
// 	//	End of Performance critical section
//	//	======================================== 	
	
	//	========================================
 	//	Performance critical section
	//	======================================== 	
	
	ofstream outFile ( filename.c_str(), fstream::out | fstream::binary | fstream::trunc );		//  2.3 ms per frame
	stat.timeOpen += timer.time();

	if ( !outFile.good() ) error = aces_ErrorOpenFile;
	else {
		outFile.write ( pOutputBuffer, (std::streamsize) outputFileSize );		// 10 ms per frame
		if ( !outFile.good() ) error = aces_ErrorWriteFile;
	}
	stat.timeWrite += timer.time();
	
	outFile.close();
	stat.timeClose += timer.time();	
	
	//	========================================
 	//	End of Performance critical section
	//	======================================== 	
	
	stat.bytesProcessed += outputFileSize;
	numberOfRowsWritten = 0;
	return error;
}

//	=====================================================================

IOstats aces_Writer:: stats ()
{
	IOstats	temp1 = stat;
	stat = IOstats();
	return temp1;
}


