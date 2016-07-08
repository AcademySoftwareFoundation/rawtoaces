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
// cd_openEXRWriter.h
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
//	Writes an aces image file
//	=====================================================================
//	Set up processing for the selected output format {
//		output image sizes;
//		fixed metadata for output format;
//		initialize variable metadata; (time code, frame number, date-time...)
//		encoding function for output sample;
//		create output folder;
//		set up format specific callbacks:
//			Initialize and fill in metadata for the output image (fixed and variable)
//			Convert a row of CameraRGB to output RGB and add to image
//			finalize output image and write out
//	}
//
//	usage: 
//		{
//			aces_Writer x; 
//			{
//				gather static image info;
//				x.configure ( clipstuff );
//			}
//			while ( thereAreImages ) {
//				get dynamic image info;
//				x.newImageObject ( dynamicMeta );		
//				{
//					for ( uint32 row = 0; row < outputHeight; row++) {
//						get Camera RGB row ( row );
//						x.storeImageRow ( rgbData, row ); 
//					}
//				}
//				x.saveImageObject ( );	
//			}	
//		}


// to do : change this to a writer base class, then subclass for Aces

#ifndef __aces_oeWriter__
#define __aces_oeWriter__

#include "aces_genericWriter.h"
#include "aces_formatter.h"
#include "aces_iostat.h"

//	Create once and share across images and clips
class aces_Writer {
	
	//  image processing funtions
	aces_formatter oef;
	
	//	=====================================================================
	//	generic
	// constant for a clip
	uint32	duration;
	uint32	outputRows;
	uint32	outputCols;
	
	uint64	outputBufferSize;
	char *	pOutputBuffer;
	
	//	specific for each image
	
	uint64	outputFileSize;
	
	//  changes during image processing
	uint32	numberOfRowsWritten;	
	
	//	Statistics
	IOstats stat;
	
	//	=====================================================================
	//	Aces
	// constant for a clip
	acesHeaderInfo hi;
	vector <string> outputFilenames;
	
	//	specific for each image
	string	filename;	
	
public:
	err		error;
	
	aces_Writer(); 
	~aces_Writer(); 
	
	//	=====================================================================
	//	provides default header info
	//	 Call once at the start of a clip
	
	acesHeaderInfo	getDefaultHeaderInfo ();
	
	//	=====================================================================
	//	set up the output for a clip:
	//	 Call once at the start of a clip
	//
	//	inputs:
	//		image dimensions
	//		static metadata
	//		output folder name with trailing /
	//
	//	processing:
	//		create output folder
	//		calculate buffer sizes
	//		set up output buffer
	//		locations for static metadata
	//		special for Aces: link table
	//	returns error code
	
	err	configure( const MetaWriteClip & clipMeta );
	
	//	=====================================================================
	//	Prepare for exporting a new image
	//		call once per image, before first row
	//	inputs:
	//		 time code
	//		 frame number
	//		 date-time
	//		 file name
	//	processing:
	//		Convert metadata to Aces formats
	//		Create aces file structure in output image buffer
	//		Clear row counters etc used by storeImageRow()
	//	returns error code
	
	err newImageObject ( const DynamicMetadata & dynamicMeta );
	
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
	
	err storeHalfRow ( const halfBytes * rgbHalfRow, uint32 row ); 
	
	//	=====================================================================
	//	get pointer to pixel data in a scan line:
	//		call once per row
	//	inputs:
	//		row number within image (to allow for out of order processing )
	
	halfBytes * GetPointerToPixelData ( uint32 row );
	
	//	=====================================================================
	// 	Complete the data structures and write to disk
	//		call once per image, after all rows are completed
	//	inputs:
	//		 none
	//	 processing:
	//		check that all rows were received
	//		update checksums	 
	//		write to disk	 
	//	returns error code
	
	err saveImageObject ( void );
	
	IOstats stats ();
};

#endif
