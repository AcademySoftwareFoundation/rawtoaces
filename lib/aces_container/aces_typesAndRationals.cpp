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
// cd_typesAndRationals.cpp
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

#include "aces_typesAndRationals.h"

#include <cmath>
#include <cassert>
#include <stdint.h>
#include <limits>

//	======================================

srational::	srational ( ) 
: n ( 0 ), d ( 1 ) 
{ 
}


srational::	srational (  int32 n1,  uint32 d1 ) 
: n ( n1 ), d ( d1 ) 
{ 
	if ( n == 0 && d == 0) 
		d = 1;
	assert ( d != 0 );
}


srational::srational ( urational ur )
{
	srational r ( ur.n, ur.d );
	n = r.n;
	d = r.d;
}

srational::	srational ( double F )
{
	double aF = abs ( F );
	
	assert ( aF <= std::numeric_limits<uint32>::max() );
	
	if ( aF == 0.0f ) {
		d = 1;		
	} else if ( aF <= 1.0f ) {
		d = std::numeric_limits<uint32>::max();
	} else {
		d = (uint32) ( std::numeric_limits<uint32>::max() / aF );
	}
	n = (int32) ( F * d ); 
}

srational::	operator double() const
{
	if(d == 0) return double(n);
	double nd = double(n) / double(d);
	return nd;
}	

ostream& operator<< ( ostream& s, const srational r )
{
	return s << r.n << "/" << r.d;
}

//	======================================

urational::	urational ( ) 
: n ( 0 ), d ( 1 ) 
{ 
}


urational::	urational ( uint32 n1, uint32 d1 ) 
: n ( n1 ), d ( d1 ) 
{
	if ( n == 0 && d == 0) 
		d = 1;
	assert ( d != 0 );
}

urational::	urational ( srational r )
{
	urational ur ( r.n, r.d );
	n = ur.n;
	d = ur.d;
}

urational::	urational ( double F )
{
	assert ( 0 <= F && F <= std::numeric_limits<uint32>::max()/*UINT32_MAX*/ );
	
	if ( F == 0.0f ) {
		d = 1;		
	} else if ( F <= 1.0f ) {
		d = std::numeric_limits<uint32>::max()/*UINT32_MAX*/ ;
	} else {
		d = (uint32) ( std::numeric_limits<uint32>::max()/*UINT32_MAX*/ / F );
	}
	n = (uint32) ( F * d ); 
}

urational::	operator double() const
{
	if(d == 0) return double(n);	
	double nd = double(n) / double(d);
	return nd;
}	

ostream& operator<< ( ostream& s, const urational r )
{
	return s << r.n << "/" << r.d;
}

//	======================================

