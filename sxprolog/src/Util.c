/*
 *   This file is part of the CxProlog system

 *   Util.c
 *   by A.Miguel Dias - 1989/11/14
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


/* Handle BYTES & WORDS */

void ClearBytes(VoidPt v, register Size len)
{
	register CharPt s = v ;
	while( len-- ) *s++ = 0 ;
}

void ClearWords(VoidPt h, register Size len)
{
	register Hdl hh = h ;
	while( len-- ) *hh++ = nil ;
}

Bool CheckIfZeroedBytes(VoidPt v, register Size len)
{
	register CharPt s = v ;
	while( len-- )
		if( *s++ != 0 )
			return false ;
	return true ;
}

void CopyBytes(register CharPt z, register Str a, Size len)
{
	while( len-- )
		*z++ = *a++ ;
}

void CopyWords(register Hdl z, register Hdl a, Size len)
{
	while( len-- )
		*z++ = *a++ ;
}

void CopyWordsReloc(Hdl zz, Hdl aa, Size len)
{
	register Hdl z = zz, a = aa, an = a + len ;
	register Size offset = z - a ;
	while( len-- )
		if( InRange(*a, cPt(aa), cPt(an)) )
			*z++ = *a++ + offset ;
		else
			*z++ = *a++ ;
}

void ShiftWords(Hdl h, Size len, Size offset)
{
	register Hdl a, z ;
	for( a = h + len - 1, z = a + offset ; len-- ; a--, z-- )
		*z = *a ;
}

Bool LongLongs()
{
#if defined(LLONG_MAX)
	return true ;
#else
	return false ;
#endif
}

void ShowSizes()
{
	Write("sizeof(Word) = %d\n", sizeof(Word)) ;
	Write("sizeof(Pt) = %d\n", sizeof(Pt)) ;
	Write("sizeof(PInt) = %d\n", sizeof(PInt)) ;
	Write("sizeof(PFloat) = %d\n", sizeof(PFloat)) ;
	Write("sizeof(Size) = %d\n", sizeof(Size)) ;
	Write("sizeof(WChar) = %d\n", sizeof(WChar)) ;
	Write("sizeof(Bool) = %d\n", sizeof(Bool)) ;
	Write("sizeof(Char) = %d\n", sizeof(Char)) ;
	Write("sizeof(LLInt) = %d\n", sizeof(LLInt)) ;
	Write("sizeof(Word16) = %d\n", sizeof(Word16)) ;
}
