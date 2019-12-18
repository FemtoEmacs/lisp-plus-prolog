/*
 *   This file is part of the CxProlog system

 *   Utf16.c
 *   by A.Miguel Dias - 2008/06/13
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

#define HasTag(x)		( ((x) & 0xf800) == 0xd800 )
#define HasTagHigh(x)	( ((x) & 0xfc00) == 0xd800 )
#define HasTagLow(x)	( ((x) & 0xfc00) == 0xdc00 )
#define UnTag(x)		( (x) & 0x03ff )
#define TagHigh(x)		( 0xd800 | UnTag(x) )
#define TagLow(x)		( 0xdc00 | UnTag(x) )


static void PreEncodePair(CharPt *s, register WChar c, Bool be)
{
	register UCharPt u = cUCharPt(*s) ;
	*s += 2 ;
	if( be ) {
		u[1] = c & 0xff ;
		u[0] = c >> 8 ;
	}
	else {
		u[0] = c & 0xff ;
		u[1] = c >> 8 ;
	}
}

static WChar PreDecodePair(CharPt *s, Bool be)
{
	register UCharPt u = cUCharPt(*s) ;
	*s += 2 ;
	if( be )
		return (u[0] << 8) + u[1] ;
	else
		return (u[1] << 8) + u[0] ;
}

/* utf16 only supports the 21 bits of the Unicode range */

void Utf16Encode(CharPt *s, WChar c, Bool be)
{
	ValidateUnicodeRange(c, true) ;
	if( c < 0x10000 )
		PreEncodePair(s, c, be) ;
	else {
		c -= 0x10000 ;
		PreEncodePair(s, TagHigh(c >> 10), be) ;
		PreEncodePair(s, TagLow(c), be) ;
	}
}

WChar Utf16Decode(CharPt *s, Bool be)
{
	int h, l ;
	WChar c ;
	h = PreDecodePair(s, be) ;
	if( !HasTag(h) )
		return h ;
	if( !HasTagHigh(h) )
		Error("Invalid utf16 multibyte sequence") ;
	l = PreDecodePair(s, be) ;
	if( !HasTagLow(l) )
		Error("Invalid utf16 multibyte sequence") ;
	c = 0x10000 + (UnTag(h) << 10) + UnTag(l) ;
	return CharFixCode(c) ;
}

WChar Utf16DecodeN(CharPt *s, Size n, Bool be)
{
	int h, l ;
	WChar c ;
	if( n < 2 )
		Error("Truncated utf16 multibyte sequence") ;
	h = PreDecodePair(s, be) ;
	if( !HasTag(h) )
		return h ;
	if( !HasTagHigh(h) )
		Error("Invalid utf16 multibyte sequence") ;
	if( n < 4 )
		Error("Truncated utf16 multibyte sequence") ;
	l = PreDecodePair(s, be) ;
	if( !HasTagLow(l) )
		Error("Invalid utf16 multibyte sequence") ;
	c = 0x10000 + (UnTag(h) << 10) + UnTag(l) ;
	return CharFixCode(c) ;
}

WChar Utf16FileGet(FILE *file, Str fName, Bool be)
{
	Str4 str ;
	CharPt s = str ;
	int h, l ;
	WChar c ;
	switch( fread(str, 1, 2, file) ) {
		case 0: return EOF ;
		case 1: FileError("Truncated utf16 multibyte sequence in input file '%s'", fName) ;
		case 2: break ;
		default: return IInternalError("Utf16FileGet") ;
	}
	h = PreDecodePair(&s, be) ;
	if( !HasTag(h) )
		return h ;
	if( !HasTagHigh(h) )
		Error("Invalid utf16 multibyte sequence") ;
	switch( fread(str + 2, 1, 2, file) ) {
		case 0: case 1:
			FileError("Truncated utf16 multibyte sequence in input file '%s'", fName) ;
		case 2: break ;
		default: return IInternalError("Utf16FileGet (2)") ;
	}
	l = PreDecodePair(&s, be) ;
	if( !HasTagLow(l) )
		Error("Invalid utf16 multibyte sequence (2)") ;
	c = 0x10000 + (UnTag(h) << 10) + UnTag(l) ;
	return CharFixCode(c) ;
}

void Utf16FilePut(FILE *file, WChar c, Str fName, Bool be)
{
	Str4 str ;
	CharPt s = str, t ;
	Utf16Encode(&s, c, be) ;
	for( t = str ; t < s ; t++ )
		if( fputc(*t, file) == EOF )
			FileError("Could not write to file '%s'", fName) ;
}
