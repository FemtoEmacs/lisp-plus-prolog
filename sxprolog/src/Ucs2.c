/*
 *   This file is part of the CxProlog system

 *   Ucs2.c
 *   by A.Miguel Dias - 2008/06/16
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

/* Ucs2 only supports the basic 16 bit range of Unicode */

void Ucs2Encode(CharPt *s, WChar c, Bool be)
{
	register UCharPt u = cUCharPt(*s) ;
	ValidateUnicodeRange(c, true) ;
	if( c >= 0x10000 )
		Error("Not a valid ucs-2 char (%ld = 0x%lx)", c, c) ;
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

WChar Ucs2Decode(CharPt *s, Bool be)
{
	register UCharPt u = cUCharPt(*s) ;
	WChar c ;
	*s += 2 ;
	if( be )
		c = (u[0] << 8) + u[1] ;
	else
		c = (u[1] << 8) + u[0] ;
	return CharFixCode(c) ;
}

WChar Ucs2DecodeN(CharPt *s, Size n, Bool be)
{
	if( n < 2 )
		Error("Truncated ucs2 multibyte sequence") ;
	return Ucs2Decode(s, be) ;
}

WChar Ucs2FileGet(FILE *file, Str fName, Bool be)
{
	Str4 str ;
	CharPt s = str ;
	switch( fread(str, 1, 2, file) ) {
		case 0: return EOF ;
		case 1: FileError("Truncated ucs2 multibyte sequence in input file '%s'", fName) ;
		case 2: break ;
		default: return IInternalError("Ucs2FileGet") ;
	}
	return Ucs2Decode(&s, be) ;
}

void Ucs2FilePut(FILE *file, WChar c, Str fName, Bool be)
{
	Str4 str ;
	CharPt s = str, t ;
	Ucs2Encode(&s, c, be) ;
	for( t = str ; t < s ; t++ )
		if( fputc(*t, file) == EOF )
			FileError("Could not write to file '%s'", fName) ;
}
