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
// md5.cc
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
//
// Derived from : 
//
// MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
// 
// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
// rights reserved.
// 
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
// 
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
// 
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
// 
// These notices must be retained in any copies of any part of this
// documentation and/or software.
 

#include "aces_md5.hh"

#include <cassert>
#include <sstream>
#include <iomanip>

//	======================================================================================
// MD5 constructor

MD5:: MD5()
{	
	Init ( );	
}

//	======================================================================================
//	======================================================================================
//	Integrated: digest buffer, finalize digest, return binary digest
string MD5::  CalculateMD5Digest ( const uchar_t * buffer, uint64_t bufferLength )
{
	Init ( );
	
	Update   ( buffer, bufferLength );
	
	Finalize ( );
	
	return GetBinaryDigest( );
}

//	======================================================================================
//	======================================================================================
//	Initialize
void MD5:: Init ( )
{
	assert ( sizeof(uint32_t) == 4 );	
	assert ( sizeof(uint64_t) == 8 );	
	assert ( sizeof(uchar_t ) == 1 );	
	
	count[0] = count[1] = 0;
	
	finalized = false;
	
	// Load magic initialization constants
	state[0] = 0x67452301;
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;
}

//	======================================================================================
// MD5 block Update operation. Continues an MD5 message-digest
// operation, processing other message blocks, and updating the
// context.

void MD5:: Update ( const uchar_t * input, uint64_t inputLength ) 
{
	assert(!finalized); 
	
	assert(inputLength < 1ULL << 40);	//	even I have limits ! 
	
	uint64_t inputIndex;			//	0 .. inputLength. Index into input Buffer
	uint32_t bufferIndex;			//	0 .. 63 = index to free slot in MD5 buffer
	uint32_t bufferSpace;			//	1 .. 64 = space in the MD5 buffer
	
	//  mod 64 = number of undigested bytes in MD5 buffer
	bufferIndex = (count[0] >> 3) & 0x3F;	
	bufferSpace = 64 - bufferIndex;
	
	// Transform 64 bytes at a time
	if (inputLength >= bufferSpace) 
	{ 
		// fill MD5 buffer and Transform it
		Memcpy (buffer + bufferIndex, input, bufferSpace);
		Transform (buffer);
		bufferIndex = 0;	//	the buffer is now empty
		
		// Transform input directly, in chunks
		for ( inputIndex = bufferSpace; inputIndex + 63 < inputLength; inputIndex += 64 ) 
			Transform ( input + inputIndex );
	}
	else
		inputIndex = 0;  
	
	// buffer undigested input
	Memcpy( buffer + bufferIndex, input + inputIndex, inputLength - inputIndex );
	
	// Update bit counters
	uint32_t lower29Bits = (uint32_t) inputLength << 3;
	count[0] += lower29Bits;
	if ( count[0] < lower29Bits )	//	did it overflow?
		count[1]++;
	count[1] += inputLength >> 29;
}


//	======================================================================================
// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.

void MD5:: Finalize ()
{	
	assert(!finalized);  
	
	uchar_t		savedCounts[8];
	uint32_t	index;				//	0 .. 63 = index to free slot in MD5 buffer
	uint32_t	padLength;
	
	const uchar_t PADDING[64] = {
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	
	// Save payload length
	Encode (savedCounts, count, 8);
	
	// Pad  mod 64 to  index 56. add 1 .. 64 bytes
	index = (count[0] >> 3) & 0x3f;
	padLength = (index < 56) ? (56 - index) : (120 - index);
	Update (PADDING, padLength);
	
	// Append payload length (excl. padding)
	Update (savedCounts, 8);
	
	//	Block reentry
	finalized = true;	
	
	// clear cleartext
	Memset ( buffer, 0, sizeof(buffer) );
}

//	======================================================================================
string MD5:: GetBinaryDigest()
{	
	assert(finalized);  	
	char		digest[16];		//	the digest
	
	// Store state in digest
	Encode ((uchar_t *) digest, state, 16);
	
	return string( digest, 16 );
}

//	======================================================================================
string MD5:: GetHexDigest()
{	
	string digest ( GetBinaryDigest() );
	
	ostringstream oss;
	
	for (int i = 0; i < 16; i++)
		oss << setfill('0') << setw(2) << hex << (digest[i] & 0xff);
	
	return 	oss.str();
}

//	======================================================================================
ostream& operator<<(ostream &stream, MD5 context)
{	
	return stream << context.GetHexDigest();
}

//	======================================================================================
//	======================================================================================
//	private:

//	======================================================================================
// Constants for MD5Transform routine

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

//	======================================================================================
// MD5 basic transformation. Transforms block into state

void MD5:: Transform ( const uchar_t block[64])
{	
	assert(!finalized);  
	
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
	
	Decode (x, block, 64);
	
	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */
	
	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */
	
	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */
	
	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */
	
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	
	// clear cleartext
	Memset ( (uchar_t *) x, 0, sizeof(x) );	
}

//	======================================================================================
//	Encodes input (uint32_t) into output (uchar_t). len is multiple of 4.
//	in little-endian order
void MD5:: Encode (uchar_t * output,  const uint32_t * input, uint32_t len) 
{	
	uint32_t i, j;
	
	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j]   =  (input[i]        & 0xff);
		output[j+1] = ((input[i] >>  8) & 0xff);
		output[j+2] = ((input[i] >> 16) & 0xff);
		output[j+3] = ((input[i] >> 24) & 0xff);
	}
}

//	======================================================================================
//	Decodes input (uchar_t) into output (uint32_t). len is a multiple of 4.
//	in little-endian order
void MD5:: Decode ( uint32_t * output,  const uchar_t * input, uint32_t len )
{	
	uint32_t i, j;
	
	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = 
		(input[j])          |
		((input[j+1]) <<  8) |
		((input[j+2]) << 16) |
		((input[j+3]) << 24);
}

//	======================================================================================
void MD5:: Memcpy ( uchar_t * output,  const uchar_t * input, uint32_t len )
{	
	for (uint32_t i = 0; i < len; i++)
		output[i] = input[i];
}

//	======================================================================================
void MD5:: Memset ( uchar_t * output,  const uchar_t value, uint32_t len )
{
	for (uint32_t i = 0; i < len; i++)
		output[i] = value;
}

//	======================================================================================
//	F, G, H and I are basic MD5 functions

#define F(b, c, d) ((b & c) | (~b & d))
#define G(b, c, d) ((b & d) | (c & ~d))
#define H(b, c, d) (b ^ c ^ d)
#define I(b, c, d) (c ^ (b | ~d))

//	======================================================================================
//	RotateLeft rotates a left s bits

#define RotateLeft(a, s) ((a << s) | (a >> (32 - s)))

//	======================================================================================
//	FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4

void MD5:: FF ( uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t  s, uint32_t ac )
{
	a += F(b, c, d) + x + ac;
	a = RotateLeft (a, s) + b;
}

void MD5:: GG ( uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac )
{
	a += G(b, c, d) + x + ac;
	a = RotateLeft (a, s) + b;
}

void MD5:: HH ( uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac )
{
	a += H(b, c, d) + x + ac;
	a = RotateLeft (a, s) + b;
}

void MD5:: II ( uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac )
{
	a += I(b, c, d) + x + ac;
	a = RotateLeft (a, s) + b;
}
