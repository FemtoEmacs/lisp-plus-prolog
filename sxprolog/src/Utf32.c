/*
 *   This file is part of the CxProlog system

 *   Utf32.c
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

/* The Unicode range only uses 21 bits, but we support 31 bit-chars */

void Utf32Encode(CharPt *s, WChar c, Bool be)
{
	register UCharPt u = cUCharPt(*s) ;
	ValidateUnicodeRange(c, false) ;
	*s += 4 ;
	if( be ) {
		u[3] = c & 0xff ; c >>= 8 ;
		u[2] = c & 0xff ; c >>= 8 ;
		u[1] = c & 0xff ;
		u[0] = c >> 8 ;
	}
	else {
		u[0] = c & 0xff ; c >>= 8 ;
		u[1] = c & 0xff ; c >>= 8 ;
		u[2] = c & 0xff ; c >>= 8 ;
		u[3] = c >> 8 ;
	}
}

WChar Utf32Decode(CharPt *s, Bool be)
{
	register UCharPt u = cUCharPt(*s) ;
	WChar c ;
	*s += 4 ;
	if( be )
		c = (u[0] << 24) + (u[1] << 16) + (u[2] << 8) + u[3] ;
	else
		c = (u[3] << 24) + (u[2] << 16) + (u[1] << 8) + u[0] ;
	return CharFixCode(c) ;
}

WChar Utf32DecodeN(CharPt *s, Size n, Bool be)
{
	if( n < 4 )
		Error("Truncated utf32 multibyte sequence") ;
	return Utf32Decode(s, be) ;
}



/* FILE HANDLING UTF-32 */

WChar Utf32FileGet(FILE *file, Str fName, Bool be)
{
	Str4 str ;
	CharPt s = str ;
	switch( fread(str, 1, 4, file) ) {
		case 0: return EOF ;
		case 1: case 2: case 3:
			FileError("Truncated utf32 multibyte sequence in input file '%s'", fName) ;
		case 4: break ;
		default: return IInternalError("Utf32FileGet") ;
	}
	return Utf32Decode(&s, be) ;
}

void Utf32FilePut(FILE *file, WChar c, Str fName, Bool be)
{
	Str4 str ;
	CharPt s = str, t ;
	Utf32Encode(&s, c, be) ;
	for( t = str ; t < s ; t++ )
		if( fputc(*t, file) == EOF )
			FileError("Could not write to file '%s'", fName) ;
}
