/*
 *   This file is part of the CxProlog system

 *   Utf8.c
 *   by A.Miguel Dias - 2004/12/19
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL

 *   CxProlog is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.

 *   CxProlog is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "CxProlog.h"

#define IsContinuationByte(s)	( (*cUCharPt(s) & 0xc0) == 0x80 )
#define TagContinuation(b)		( 0x80 | ((b) & 0x3f) )

/* ENCODE UTF-8 */

/* Converts a wide character to a UTF-8 sequence */

/* The Unicode range only uses 21 bits, but we support 31 bit-chars */

void Utf8Encode(CharPt *s, WChar c)
{
	register UCharPt u = cUCharPt(*s) ;
	ValidateUnicodeRange(c, false) ;
	if( c < 0x80 ) {				/* 7 bits */
		*s += 1 ;
		u[0] = c ;
	}
	elif( c < 0x800 ) {				/* 11 bits */
		*s += 2 ;
		u[0] = 0xc0 + (c >> 6) ;
		u[1] = TagContinuation(c) ;
	}
	elif( c < 0x10000 ) {			/* 16 bits */
		*s += 3 ;
		u[0] = 0xe0 + (c >> 12) ;
		u[1] = TagContinuation(c >> 6) ;
		u[2] = TagContinuation(c) ;
	}
	elif( c < 0x200000 ) {			/* 21 bits */
		*s += 4 ;
		u[0] = 0xf0 + (c >> 18) ;
		u[1] = TagContinuation(c >> 12) ;
		u[2] = TagContinuation(c >> 6) ;
		u[3] = TagContinuation(c) ;
	}
	elif( c < 0x4000000 ) {			/* 26 bits */
		*s += 5 ;
		u[0] = 0xf8 + (c >> 24) ;
		u[1] = TagContinuation(c >> 18) ;
		u[2] = TagContinuation(c >> 12) ;
		u[3] = TagContinuation(c >> 6) ;
		u[4] = TagContinuation(c) ;
	}
	else {							/* 31 bits */
		*s += 6 ;
		u[0] = 0xfc + (c >> 30) ;
		u[1] = TagContinuation(c >> 24) ;
		u[2] = TagContinuation(c >> 18) ;
		u[3] = TagContinuation(c >> 12) ;
		u[4]  = TagContinuation(c >> 6) ;
		u[5]  = TagContinuation(c) ;
	}
}


/* DECODE UTF-8 */

/* Length of a UTF-8 sequence from the first byte. Illegal bytes have a one. */

static UChar utf8Len[256] =
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /*invalid*/
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /*invalid*/
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1,
};

/* Converts a UTF-8 sequence to a wide character.  */

WChar Utf8Decode(CharPt *s)
{
	register UCharPt u = cUCharPt(*s) ;
	WChar c ;
	if( *u < 0x80 ) {
		*s += 1 ;
		return u[0] ;
	}

#define u(n)		(u[n] & 0x3f)
	switch( utf8Len[*u] ) {
		case 2: {
			*s += 2 ;
			c = ((u[0] & 0x1f) << 6) + u(1) ;
			break ;
		}
		case 3: {
			*s += 3 ;
			c = ((u[0] & 0x0f) << 12) + (u(1) << 6) + u(2) ;
			break ;
		}
		case 4: {
			*s += 4 ;
			c = ((u[0] & 0x07) << 18) + (u(1) << 12) + (u(2) << 6)
						+ u(3) ;
			break ;
		}
		case 5: {
			*s += 5 ;
			c = ((u[0] & 0x03) << 24) + (u(1) << 18) + (u(2) << 12)
						+ (u(3) << 6) + u(4) ;
			break ;
		}
		case 6: {
			*s += 6 ;
			c = ((u[0] & 0x01) << 30) + (u(1) << 24) + (u(2) << 18)
						+ (u(3) << 12) + (u(4) << 6) + u(5) ;
			break ;
		}
		default:
			return IInternalError("Utf8Decode") ;
	}
	return CharFixCode(c) ;
}

WChar Utf8DecodeN(CharPt *s, Size n)
{
	if( n < utf8Len[cUCharPt(*s)[0]] )
		Error("Truncated utf8 multibyte sequence") ;
	return Utf8Decode(s) ;
}



/* COPY UTF-8 */

/* Copies UTF-8 sequence.  */

void Utf8Copy(register CharPt *z, register CharPt *a) /* pre: *a != '\0' */
{
	do {
		*(*z)++ = *(*a)++ ;
	} while( IsContinuationByte(*a) ) ;
}

/* Copies N UTF-8 sequences.  */

void Utf8NCopy(register CharPt *z, register CharPt *a, register Size n)
{
	while( n-- && **a ) {
		do {
			*(*z)++ = *(*a)++ ;
		} while( IsContinuationByte(*a) ) ;
	}
}


/* INDEXING UTF-8 */

/* Gets first character from UTF-8 string.  */

WChar Utf8First(Str s)
{
	return Utf8Decode(cCharHdl(&s)) ;
}

/* Gets last character from UTF-8 string.  */

WChar Utf8Last(Str s)
{
	int len = strlen(s) ;
	if( len == 0 ) return '\0' ;
	s += len - 1 ;
	while( IsContinuationByte(s) ) s-- ;
	return Utf8Decode(cCharHdl(&s)) ;
}

/* Gets the position of the i-th character in a UTF-8 string.  */

CharPt Utf8Pos(register CharPt s, register int i)
{
	if( i >= 0 ) {
		for( ; *s ; s++ ) {
			if( !IsContinuationByte(s) ) {
				if( i == 0 ) return s ;
				else i-- ;
			}
		}
	}
	else {
		for( s--, i++ ; *s ; s-- ) {
			if( !IsContinuationByte(s) ) {
				if( i == 0 ) return s ;
				else i++ ;
			}
		}
	}
	return s ;
}

/* Gets the position of the i-th character in a UTF-8 string.  */

Size Utf8Len(register Str s)
{
	Size len = 0 ;
	for( ; *s ; s++ )
		if( !IsContinuationByte(s) )
			len++ ;
	return len ;
}

/* Gets the position of the i-th character in a UTF-8 string.  */

CharPt Utf8Next(CharPt s)
{
	do {
		s++ ;
	} while( IsContinuationByte(s) ) ;
	return s ;
}


/* FILE HANDLING UTF-8 */

WChar Utf8FileGet(FILE *file, Str fName)
{
	int c = fgetc(file) ;
	if( c < 0x80 )	/* EOF is a particular case */
		return c ;
	else {
		Size lenToRead = utf8Len[c] - 1 ;
		Str8 str ;
		Str s = str ;
		str[0] = c ;
		if( cSize(fread(str + 1, 1, lenToRead, file)) != lenToRead )
			FileError("Truncated utf8 multibyte sequence in input file '%s'", fName) ;
		return Utf8Decode(cCharHdl(&s)) ;
	}
}

void Utf8FilePut(FILE *file, WChar c, Str fName)
{
	Str8 str ;
	CharPt s = str, t ;
	Utf8Encode(&s, c) ;
	for( t = str ; t < s ; t++ )
		if( fputc(*t, file) == EOF )
			FileError("Could not write to file '%s'", fName) ;
}


/* UNICODE RANGE */

void ValidateUnicodeRange(WChar c, Bool strict)
{
	if( c < 0  )
		Error("Invalid unicode char (negative code %ld)", c) ;
#if 1
	Unused(strict) ;
	if( c > maxChar )
		Error("Invalid unicode char (code too large 0x%lx)", c) ;
#else
	if( strict && c > maxChar )
		Error("Invalid unicode char (code too large 0x%lx)", c) ;
#endif
	if( InRange(c, 0xd800, 0xdfff) )
		Error("Invalid unicode char (surrogate code 0x%lx)", c) ;
}
