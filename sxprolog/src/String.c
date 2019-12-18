/*
 *   This file is part of the CxProlog system

 *   String.c
 *   by A.Miguel Dias - 2004/12/12
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

int StrHash(Str s)
{
	register UCharPt u = cUCharPt(s) ;
	register int val = 0 ;
#if 1
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;

	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;

	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;

	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
	if( *u == '\0' ) return val ; else val += *u++ ;
#else
	while( *u ) val += *u++ ;
#endif
	return val ;
}

int StrHashLen(Str s, Size len)
{
	CharPt p = cCharPt(s) ;
	int save = p[len], val ;
	p[len] = '\0' ;
	val = StrHash(p) ;
	p[len] = save ;
	return val ;
}

CharPt StrAllocate(Str str)
{
	return strcpy(Allocate(Words(strlen(str) + 1), true), str) ;
}

int StrSeqLength(Str *vs)
{
	register int i ;
	for( i = 0 ; *vs != nil ; i++, vs++ ) ;
	return i ;
}

int StrSeqGetIdx(Str a, Str *vs)
{
	register int i ;
	for( i = 0 ; *vs != nil ; i++, vs++ )
		if( StrEqual(a, *vs) )
			return i ;
	return -1 ;
}

Str StrSeqGetVal(int i, Str *vs)
{
	register int j ;
	for( j = 0 ; *vs != nil ; j++, vs++ )
		if( j == i )
			return *vs ;
	InternalError("StrSeqGetVal") ;
	return nil ;
}

Str StrSeqFormat(Str pre, Str sep, Str pos, Str *vs)
{
	GStrOpen() ;
	GStrAddStr(pre) ;
	for( ; *vs != nil ; vs++ ) {
		GStrAddStr(*vs) ;
		GStrAddStr(sep) ;
	}
	GStrBack(strlen(sep)) ;
	GStrAddStr(pos) ;
	return GStrClose() ;
}

static CharPt StrSkipByte(CharPt s, int b)
{
	for( ; *s == b ; s++ ) ;
	return s ;
}

static CharPt StrSkipNotByte(CharPt s, int b)
{
	for( ; *s != '\0' && *s != b ; s++ ) ;
	return s ;
}

int StrCountElems(CharPt s, int sep)
{
	int n = 0 ;
	for(;;) {
		s = StrSkipByte(s, sep) ;
		if( *s == '\0' ) break ;
		s = StrSkipNotByte(s, sep) ;
		n++ ;
		if( *s == '\0' ) break ;
	}
	return n ;
}

void StrSplitElems(Str str, Str *v, int b)
{
	CharPt s = StrAllocate(str) ;
	for(;;) {
		s = StrSkipByte(s, b) ;
		if( *s == '\0' ) break ;
		*v++ = cCharPt(s) ;
		s = StrSkipNotByte(s, b) ;
		if( *s == '\0' ) break ;
		*s++ = '\0' ;
	}
}


/* GROWINGS TEMPORARY STRINGS */

#define maxGStr				12	/* 10 or larger! */
#define initGStrCapacities	(scratchDebugging_flag ? 16000 : 64) /* bytes */

static CharPt gStrs[maxGStr] ;
static Size gCapacities[maxGStr] ; /* in bytes */
static int gCurr, gStrInUse ;
CharPt gStrBegin, gStrEnd, gStrPt ;

static void GStrInit(void)
{
	int i ;
	dotimes(i, maxGStr) {
		gStrs[i] = Allocate(Words(initGStrCapacities), false) ;
		gCapacities[i] = initGStrCapacities ;
	}
	gCurr = 0 ;
}

static void GStrResize(int i, Size newCapacity)
{
	Release(gStrs[i], Words(gCapacities[i])) ;
	gStrs[i] = Allocate(Words(newCapacity), false) ;
	gCapacities[i] = newCapacity ;
}

Size GStrCapacity()
{
	Size n = 0 ;
	int i ;
	dotimes(i, maxGStr)
		n += Words(gCapacities[i]) ;
	return n ;
}

static void GStrEnsureFreeSpace(Size requiredFreeSpace)
{
	Size freeSpace = gStrEnd - gStrPt ;
	if( freeSpace < requiredFreeSpace ) {
		Size usedSpace = gStrPt - gStrBegin ;
		Size oldStrCapacity = gCapacities[gStrInUse] ;
		Size newStrCapacity = oldStrCapacity * 2 ;
		while( newStrCapacity - usedSpace < requiredFreeSpace )
			newStrCapacity *= 2 ;
		gStrs[gStrInUse] = Reallocate(gStrBegin, Words(oldStrCapacity),
													Words(newStrCapacity)) ;
		gCapacities[gStrInUse] = newStrCapacity ;
	
		gStrBegin = gStrs[gStrInUse] ;
		gStrPt = gStrBegin + usedSpace ;
		gStrEnd = gStrBegin + newStrCapacity - MB_LEN_MAX - 1 ;
	}
}

int GStrExpand()
{
	GStrEnsureFreeSpace(gStrEnd - gStrPt + 1) ;
	return 0 ;
}

void GStrShow(void)
{
	int i ;
	Write("gstr curr = %d\n", gCurr) ;
	dotimes(i, maxGStr)
		Write("gstr[%d] -> capacity = %ld\n", i, gCapacities[i]) ;
}

void GStrOpen()
{
	if( ++gCurr == maxGStr ) gCurr = 0 ;
	gStrInUse = gCurr ;

	gStrBegin = gStrs[gStrInUse] ;
	gStrPt = gStrBegin ;
	gStrEnd = gStrBegin + gCapacities[gStrInUse] - MB_LEN_MAX - 1 ;
}

CharPt GStrClose()
{
	*gStrPt = '\0' ;
	return gStrBegin ;
}

void GStrAddStr(Str s)
{
	while( *s ) {
		GStrCheck() ;
		*gStrPt++ = *s++ ;
	}
}

void GStrAddStrConv(Str s, WChar oldChar, WChar newChar)
{
	if( oldChar == newChar )
		GStrAddStr(s) ;
	else {
		while( *s ) {
			GStrCheck() ;
			if( (*gStrPt++ = *s++) == oldChar )
				gStrPt[-1] = newChar ;
		}
	}
}

void GStrAddBytes(Str s, PInt len)
{
	GStrEnsureFreeSpace(len) ;
	CopyBytes(gStrPt, s, len) ;
	gStrPt += len ;
}

CharPt GStrToUpper(Str s)
{
	WChar w ;
	GStrOpen() ;
	for( ; (w = *s) != '\0' ; s++ )
		GStrAddByte(cx_toupper(w)) ;
	return GStrClose() ;
}

static int My_vsnprintf(char *s, Size size, Str fmt, va_list v)
{
	int res ;
#if defined(va_copy)
	va_list vv ;
	va_copy(vv, v) ;
	res = vsnprintf(s, size, fmt, vv) ;
	va_end(vv) ;
#else
	if( sizeof(va_list) == sizeof(Word) )	/* assume va_list is a lvalue */
		res = vsnprintf(s, size, fmt, v) ;
	else {									/* assume va_list is an array */
		va_list vv ;
		CopyBytes(cCharPt(vv), cCharPt(v), sizeof(va_list)) ;
		res = vsnprintf(s, size, fmt, vv) ;
	}
#endif
	return res < 0 ? res : res + 1 ;
}

CharPt GStrFormatV(Str fmt, va_list v)
{
	int i ;
	Size size ;
	if( ++gCurr == maxGStr ) gCurr = 0 ;
	i = gCurr ;
/* vsnprintf in antique libc ou Visual Studio */
	while( (size = My_vsnprintf(gStrs[i], gCapacities[i], fmt, v)) < 0 )
		GStrResize(i, 2 * gCapacities[i]) ;
/* vsnprintf in modern libc */
	if( size > gCapacities[i] ) {
		GStrResize(i, size) ;
		if( My_vsnprintf(gStrs[i], gCapacities[i], fmt, v) > gCapacities[i] )
			InternalError("GStrFormatV") ;
	}
	return gStrs[i] ;
}

CharPt GStrFormat(Str fmt, ...)
{
	CharPt s ;
	va_list v ;
	va_start(v, fmt) ;
	s = GStrFormatV(fmt, v) ;
	va_end(v) ;
	return  s ;
}

CharPt GStrMake(Str s)
{
	return s == nil ? nil : GStrFormat("%s", s) ;
}

CharPt GStrMakeSegm(CharPt s, CharPt end)
{
	CharPt res ;
	Char save = *end ;
	*end = '\0' ;
	res = GStrFormat("%s", s) ;
	*end = save ;
	return res ;
}



/* BIG STRING */

#define bigStrInitialCapacity	(scratchDebugging_flag ? 100 K : 4 K)	/* bytes */

static Size bigStrCapacity ; /* in bytes */
CharPt bigStrBegin, bigStrEnd, bigStrPt;
CharPt bigStrAPt, bigStrBPt ;

Size BigStrCapacity()
{
	return Words(bigStrCapacity) ;
}

int BigStrExpand()
{
	Size oldStrCapacity = bigStrCapacity ;
	Size newStrCapacity = oldStrCapacity * 2 ;
	CharPt bigStr ;
	MemoryGrowInfo("bigstr", Words(oldStrCapacity), Words(newStrCapacity)) ;
	bigStr = Reallocate(bigStrBegin, Words(oldStrCapacity), Words(newStrCapacity)) ;
	bigStrCapacity = newStrCapacity ;

	bigStrPt += bigStr - bigStrBegin ;
	bigStrAPt += bigStr - bigStrBegin ;
	bigStrBPt += bigStr - bigStrBegin ;
	bigStrBegin = bigStr ;
	bigStrEnd = bigStrBegin + bigStrCapacity - 32 ;
	return 0 ;
}

void BigStrOpen()
{
	bigStrPt = bigStrBegin ;
}

CharPt BigStrClose()
{
	*bigStrPt = '\0' ;
	return bigStrBegin ;
}

void BigStrInit()
{
	bigStrBegin = Allocate(Words(bigStrInitialCapacity), false) ;
	bigStrCapacity = bigStrInitialCapacity ;
	bigStrEnd = bigStrBegin + bigStrCapacity - 32 ;
}

void BigStrAddStr(register Str s)
{
	for( ; *s ; s++ )
		BigStrAddByte(*s) ;
}

void BigStrAddStrConv(Str s, WChar oldChar, WChar newChar)
{
	if( oldChar == newChar )
		BigStrAddStr(s) ;
	else {
		for( ; *s ; s++ )
			BigStrAddByte(*s == oldChar ? newChar : *s) ;
	}
}

void BigStrAddStrSlice(Str s, int a, int b) /* [a,b[  unicode positions */
{
	CharPt ss = cCharPt(s) ;
	int n = b - a ;
	ss = cCharPt(CharPos(ss, a)) ; /* Find the start */
	while( n-- > 0 && *ss != '\0' ) { /* Copy */
		BigStrCheck() ;
		CharCopy(bigStrPt, ss) ;
	}
}


/* BIG STRING 2 */

#define bigStr2InitialCapacity	1 K		/* bytes */

static Size bigStr2Capacity ; /* in bytes */
CharPt bigStr2Begin, bigStr2End, bigStr2Pt;
CharPt bigStr2APt, bigStr2BPt ;

Size BigStr2Capacity()
{
	return Words(bigStr2Capacity) ;
}

int BigStr2Expand()
{
	Size oldStr2Capacity = bigStr2Capacity ;
	Size newStr2Capacity = oldStr2Capacity * 2 ;
	CharPt bigStr2 ;
	MemoryGrowInfo("big2str", Words(oldStr2Capacity), Words(newStr2Capacity)) ;
	bigStr2 = Reallocate(bigStr2Begin, Words(oldStr2Capacity), Words(newStr2Capacity)) ;
	bigStr2Capacity = newStr2Capacity ;

	bigStr2Pt += bigStr2 - bigStr2Begin ;
	bigStr2APt += bigStr2 - bigStr2Begin ;
	bigStr2BPt += bigStr2 - bigStr2Begin ;
	bigStr2Begin = bigStr2 ;
	bigStr2End = bigStr2Begin + bigStr2Capacity - 32 ;
	return 0 ;
}

void BigStr2Open()
{
	bigStr2Pt = bigStr2Begin ;
}

CharPt BigStr2Close()
{
	*bigStr2Pt = '\0' ;
	return bigStr2Begin ;
}

void BigStr2Init()
{
	bigStr2Begin = Allocate(Words(bigStr2InitialCapacity), false) ;
	bigStr2Capacity = bigStr2InitialCapacity ;
	bigStr2End = bigStr2Begin + bigStr2Capacity - 32 ;
}

void BigStr2AddStr(register Str s)
{
	for( ; *s ; s++ )
		BigStr2AddByte(*s) ;
}



/* INIT */

void StringInit()
{
	static Bool done = false ;
	if( !done ) {
		GStrInit() ;
		BigStrInit() ;
		BigStr2Init() ;
		done = true ;
	}
}


/*

Character.h:int CharReorderCompare(CharPt a, CharPt b) ;
File.h:CharPt FileGetCharStrInteractive(FilePt f, CharPt line, Size size) ;
FileSys.h:CharPt FileNameInternalize(CharPt s) ;
FileSys.h:void EnsureNativeFileName(CharPt s) ;
FileSys.h:void EnsureIndependentFileName(CharPt s) ;
FileSys.h:CharPt GetFileNameLastComponent(CharPt s) ;
FileSys.h:CharPt ProcessFileName(CharPt s) ;
Locale.h:CharPt StrInternalize(CharPt s) ;
Locale.h:CharPt StrInternalizeConv(CharPt s, WChar oldChar, WChar newChar) ;
Stream.h:void StreamPutStrSegm(StreamPt srm, CharPt s, CharPt end) ;
String.h:CharPt GStrMakeSegm(CharPt s, CharPt end) ;
String.h:void BigStrAddStrSlice(CharPt s, int a, int b) ;
TermRead.h:CharPt ClearTermText(CharPt s, Bool clearDot) ;
Utf8.h:CharPt Utf8Pos(CharPt s, int i) ;
Utf8.h:CharPt Utf8Next(CharPt s) ;
Util.h:void CopyBytes(CharPt zz, Str aa, Size len) ;


*/
