/*
 *   This file is part of the CxProlog system

 *   Buffer.c
 *   by A.Miguel Dias - 2001/02/22
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

/* BUFFER */

typedef struct Buffer {
	ExtraDef(Buffer) ;
	UCharPt begin, end ;
	UCharPt last, position ;
	Str encoding ;
} Buffer ;

#define cBufferPt(x)			((BufferPt)(x))

#define BufferBegin(b)			((b)->begin)
#define BufferEnd(b)			((b)->end)
#define BufferLast(b)			((b)->last)
#define BufferPosition(b)		((b)->position)
#define BufferEncoding(b)		((b)->encoding)

static ExtraTypePt bufferType ;


/* PRIVATE FUNCTIONS */

#define BufferWords(byteCapacity)	Words(byteCapacity)

static Size BufferCapacity(BufferPt b)
{
	return BufferEnd(b) - BufferBegin(b) ;
}

static void BufferInit(BufferPt b, Size byteCapacity,
						Size positionOffset, Size lastOffset)
{
	BufferBegin(b) = Allocate(BufferWords(byteCapacity), false) ;
	BufferEnd(b) = BufferBegin(b) + byteCapacity ;
	BufferPosition(b) = BufferBegin(b) + positionOffset ;
	BufferLast(b) = BufferBegin(b) + lastOffset ;
}

static void BufferExpand(BufferPt b, PInt minIdx)
{
	PInt cap ;
	UCharPt bb = BufferBegin(b) ;
	UCharPt be = BufferEnd(b) ;
	Size positionOffset = BufferPosition(b) - bb ;
	Size lastOffset = BufferLast(b) - bb ;
	register UCharPt s, h ;
	Size oldCapacity = BufferCapacity(b) ;
	for( cap = 2 * oldCapacity ; cap <= minIdx ; cap *= 2 ) ;
	BufferInit(b, cap, positionOffset, lastOffset) ;
	for( h = BufferBegin(b), s = bb ; s < be ; *h++ = *s++ ) ;
	Release(bb, BufferWords(oldCapacity)) ;
}

static void BufferDisable(BufferPt b)
{
	if( ExtraDisable(b) ) {
		BufferClear(b) ;
		Release(BufferBegin(b), BufferWords(BufferCapacity(b))) ;
	}
}

static void BufferWrite(StreamPt srm, BufferPt b)
{
	register UCharPt pt ;
	StreamWrite(srm, "%s", ExtraAsStr(b)) ;
	StreamWrite(srm, "     (current capacity %ld, pos=%ld, last=%ld)\n",
					BufferCapacity(b),
					BufferPosition(b)-BufferBegin(b),
					BufferLast(b)-BufferBegin(b)) ;
	if( BufferBegin(b) < BufferLast(b) ) {
		StreamWrite(srm, "%d", *BufferBegin(b)) ;
		for( pt = BufferBegin(b) + 1 ; pt < BufferLast(b) ; pt++ )
			StreamWrite(srm, ",%d", *pt) ;
	}
	StreamPut(srm, '\n') ;
}

static Size BufferSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(Buffer) ;
}

static Bool BufferBasicGCDelete(VoidPt x)
{
	BufferPt b = cBufferPt(x) ;
	BufferDisable(b) ;
	return true ;
}


/* MAIN OPERATIONS */

Size BufferSize(BufferPt b)
{
	return BufferLast(b) - BufferBegin(b) ;
}

UCharPt BufferContents(BufferPt b)
{
	return BufferBegin(b) ;
}

BufferPt BufferNew()
{
	BufferPt b = ExtraNew(bufferType, 0) ;
	BufferInit(b, 4, 0, 0) ;
	BufferEncoding(b) = DefaultEncoding() ;
	return b ;
}

void BufferSetEncoding(BufferPt b, Str encoding)
{
	switch( EncodingGet(encoding) ) {
		case unicodeEncoding: {
			BufferEncoding(b) = EncodingMake("unicode_be") ;
			break ;
		}
		case utf16Encoding: {
			BufferEncoding(b) = EncodingMake("utf16be") ;
			break ;
		}
		case utf32Encoding: {
			BufferEncoding(b) = EncodingMake("utf32be") ;
			break ;
		}
		default: {
			BufferEncoding(b) = encoding ;
			break ;
		}
	}
}

Str BufferGetEncoding(BufferPt b)
{
	return BufferEncoding(b) ;
}

void BufferDelete(BufferPt b)
{
	BufferDisable(b) ;
}

void BufferClear(BufferPt b)
{
	BufferLast(b) = BufferBegin(b) ;
	BufferPosition(b) = BufferBegin(b) ;
}

int BufferGet(BufferPt b, PInt idx)
{
	if( BufferBegin(b) + idx < BufferLast(b) )
		return BufferBegin(b)[idx] ;
	else return 0 ;
}

void BufferSet(BufferPt b, PInt idx, int i)
{
	if( BufferBegin(b) + idx >= BufferEnd(b) )
		BufferExpand(b, idx) ;
	if( BufferBegin(b) + idx >= BufferLast(b) ) {
		UCharPt idxPos = BufferBegin(b) + idx ;
		register UCharPt pt ;
		for( pt = BufferLast(b) ; pt < idxPos ; pt++ )
			*pt = 0 ;
		BufferLast(b) = idxPos + 1 ;
	}
	BufferBegin(b)[idx] = i ;
}

void BufferResize(BufferPt b, Size newSize)
{
	if( newSize == 0 )
		BufferClear(b) ;
	else {
		int i = BufferGet(b, newSize-1) ;
		BufferSet(b, newSize-1, i) ; /* forces grow if necessary */
		BufferLast(b) = BufferBegin(b) + newSize ;
		if( BufferPosition(b) > BufferLast(b) )
			BufferPosition(b) = BufferLast(b) ;
	}
}


/* SEQUENTIAL OPERATIONS */

void BufferEnsureCapacity(BufferPt b, Size freeSpace)
{
	if( BufferPosition(b) + freeSpace >= BufferEnd(b) )
		BufferExpand(b, BufferPosition(b) - BufferBegin(b) + freeSpace - 1) ;
}

void BufferSetSizeUnsafe(BufferPt b, Size size)
{
	BufferLast(b) = BufferBegin(b) + size ;
}

void BufferReset(BufferPt b)
{
	BufferPosition(b) = BufferBegin(b) ;
}

void BufferRewrite(BufferPt b)
{
	BufferClear(b) ;
}

void BufferAppend(BufferPt b)
{
	BufferPosition(b) = BufferLast(b) ;
}


/* SEQUENTIAL READ OPERATIONS */

Bool BufferAtEnd(BufferPt b)
{
	return BufferPosition(b) >= BufferLast(b) ;
}

WChar BufferGetByte(BufferPt b)
{
	if( BufferPosition(b) >= BufferLast(b) ) return EOF ;
	return *BufferPosition(b)++ ;
}

WChar BufferPeekByte(BufferPt b)
{
	if( BufferPosition(b) >= BufferLast(b) ) return EOF ;
	return *BufferPosition(b) ;
}

Size BufferGetNBytes(BufferPt b, VoidPt v, Size n)
{
	if( BufferLast(b) - BufferPosition(b) < n )
		n = BufferLast(b) - BufferPosition(b) ;
	CopyBytes(v, cCharPt(BufferPosition(b)), n) ;
	BufferPosition(b) += n ;
	return n ;
}

WChar BufferGetChar(BufferPt b)
{
	CharPt s ;
	WChar c ;
	if( BufferPosition(b) >= BufferLast(b) ) return EOF ;
	s = cCharPt(BufferPosition(b)) ;
	switch( EncodingGet(BufferEncoding(b)) ) {
		case octetEncoding: {
			Error("This operation is not available for binary buffers") ;
			c = 0 ;		/* Avoids warning */
			break ;
		}
		case asciiEncoding: {
			if( (c = *cUCharPt(s++)) > 127 )
				Warning("Non-ascii char (%d) in ascii buffer '%s'", c, ExtraAsStr(b)) ;
			break ;
		}
		case latin1Encoding: {
			c = *cUCharPt(s++) ;
			break ;
		}
		case utf8Encoding: {
			c = Utf8DecodeN(&s, BufferLast(b) - BufferPosition(b)) ;
			break ;
		}
		case utf16Encoding: {
			FileError("The encoding utf16 is ambiguous") ;
			c = 0 ;		/* Avoids warning */
			break ;
		}
		case utf16beEncoding: {
			c = Utf16DecodeN(&s, BufferLast(b) - BufferPosition(b), true) ;
			break ;
		}
		case utf16leEncoding: {
			c = Utf16DecodeN(&s, BufferLast(b) - BufferPosition(b), false) ;
			break ;
		}
		case utf32Encoding: {
			FileError("The encoding utf32 is ambiguous") ;
			c = 0 ;		/* Avoids warning */
			break ;
		}
		case utf32beEncoding: {
			c = Utf32DecodeN(&s, BufferLast(b) - BufferPosition(b), true) ;
			break ;
		}
		case utf32leEncoding: {
			c = Utf32DecodeN(&s, BufferLast(b) - BufferPosition(b), false) ;
			break ;
		}
		case unicodeEncoding: {
			FileError("The encoding unicode is ambiguous") ;
			c = 0 ;		/* Avoids warning */
			break ;
		}
		case unicodebeEncoding: {
			c = Ucs2DecodeN(&s, BufferLast(b) - BufferPosition(b), true) ;
			break ;
		}
		case unicodeleEncoding: {
			c = Ucs2DecodeN(&s, BufferLast(b) - BufferPosition(b), false) ;
			break ;
		}
		default: {
			c = StrGetCharUsingCLib(&s, BufferEncoding(b)) ;
			break ;
		}
	}
	BufferPosition(b) = cUCharPt(s) ;
	return c ;
}

WChar BufferPeekChar(BufferPt b)
{
	UCharPt pos = BufferPosition(b) ;
	WChar c = BufferGetChar(b) ;
	BufferPosition(b) = pos ;
	return c ;
}


/* SEQUENTIAL WRITE OPERATIONS */

void BufferPutByte(BufferPt b, WChar c)
{
	if( BufferPosition(b) == BufferEnd(b) )
		BufferExpand(b, 0) ;
	*BufferPosition(b)++ = c ;
	if( BufferPosition(b) > BufferLast(b) )
		BufferLast(b) = BufferPosition(b) ;
}

Size BufferPutNBytes(BufferPt b, VoidPt v, Size n)
{
	BufferEnsureCapacity(b, n) ;
	CopyBytes(cCharPt(BufferPosition(b)), v, n) ;
	BufferPosition(b) += n ;
	if( BufferPosition(b) > BufferLast(b) )
		BufferLast(b) = BufferPosition(b) ;
	return n ;
}

void BufferPutChar(BufferPt b, WChar c)
{
	CharPt s ;
	BufferEnsureCapacity(b, MB_LEN_MAX) ;
	s = cCharPt(BufferPosition(b)) ;
	switch( EncodingGet(BufferEncoding(b)) ) {
		case octetEncoding: {
			FileError("This operation is not available for binary buffers") ;
			break ;
		}
		case asciiEncoding: {
			if( !InRange(c, 0, 127) )
				FileError("Non-ascii char (%d) in ascii buffer '%s'", c, ExtraAsStr(b)) ;
			*s++ = c ;
			break ;
		}
		case latin1Encoding: {
			if( !InRange(c, 0, 255) )
				FileError("Non-latin1 char (%d) in latin1 buffer '%s'", c, ExtraAsStr(b)) ;
			*s++ = c ;
			break ;
		}
		case utf8Encoding: {
			Utf8Encode(&s, c) ;
			break ;
		}
		case utf16Encoding: {
			FileError("The encoding utf16 is ambiguous") ;
			break ;
		}
		case utf16beEncoding: {
			Utf16Encode(&s, c, true) ;
			break ;
		}
		case utf16leEncoding: {
			Utf16Encode(&s, c, false) ;
			break ;
		}
		case utf32Encoding: {
			FileError("The encoding utf32 is ambiguous") ;
			break ;
		}
		case utf32beEncoding: {
			Utf32Encode(&s, c, true) ;
			break ;
		}
		case utf32leEncoding: {
			Utf32Encode(&s, c, false) ;
			break ;
		}
		case unicodeEncoding: {
			FileError("The encoding unicode is ambiguous") ;
			break ;
		}
		case unicodebeEncoding: {
			Ucs2Encode(&s, c, true) ;
			break ;
		}
		case unicodeleEncoding: {
			Ucs2Encode(&s, c, false) ;
			break ;
		}
		default: {
			StrPutCharUsingCLib(&s, c, BufferEncoding(b)) ;
			break ;
		}
	}
	BufferPosition(b) = cUCharPt(s) ;
	if( BufferPosition(b) > BufferLast(b) )
		BufferLast(b) = BufferPosition(b) ;
}

void BufferPutCharStr(BufferPt b, Str s)
{
	while( *s )
		BufferPutChar(b, CharDecode(s)) ;
}


/* CXPROLOG C'BUILTINS */

static void PBufferCheck()
{
	MustBe( XExtraCheck(bufferType, X0) ) ;
}

static void PBufferNew()
{
	BindVarWithExtra(X0, BufferNew()) ;
	JumpNext() ;
}

static void PBufferClear()
{
	BufferPt b = XTestExtra(bufferType,X0) ;
	BufferClear(b) ;
	JumpNext() ;
}

static void PBufferDelete()
{
	BufferPt b = XTestExtra(bufferType,X0) ;
	BufferDisable(b) ;
	JumpNext() ;
}

static void PBufferSize()
{
	BufferPt b = XTestExtra(bufferType,X0) ;
	Size size = BufferSize(b) ;
	Size newSize ;
	Ensure( UnifyWithAtomic(X1, MakeInt(size)) ) ;
	newSize = XTestNat(X2) ;
	if( newSize != size )
		BufferResize(b, newSize) ;
	JumpNext() ;
}

static void PBufferSet()
{
	BufferPt b = XTestExtra(bufferType,X0) ;
	BufferSet(b, XTestPosInt(X1)-1, XTestByte(X2)) ;
	JumpNext() ;
}

static void PBufferGet()
{
	BufferPt b = XTestExtra(bufferType,X0) ;
	MustBe( Unify(X2, MakeByte(BufferGet(b, XTestPosInt(X1)-1))) ) ;
}

static void PBufferAsAtom()
{
	BufferPt b = XTestExtra(bufferType,X0) ;
	BufferEnsureCapacity(b, 1) ;
	*BufferLast(b) = '\0' ;
	MustBe( UnifyWithAtomic(X1, MakeTempAtom(cCharPt(BufferBegin(b)))) ) ;
}

static void PBufferWrite()
{
	BufferPt b = XTestExtra(bufferType,X0) ;
	BufferWrite(currOut, b) ;
	JumpNext() ;
}

static void PSBufferWrite()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	BufferPt b = XTestExtra(bufferType,X1) ;
	BufferWrite(srm, b) ;
	JumpNext() ;
}

static void PNDCurrentBuffer()
{
	ExtraPNDCurrent(bufferType, nil, 1, 0) ;
	JumpNext() ;
}

static Size BuffersAux(CVoidPt b)
{
	Write("size(%ld), capacity(%ld)",
		BufferSize(cBufferPt(b)), BufferCapacity(cBufferPt(b))) ;
	return 1 ;
}
static void PBuffers()
{
	ExtraShow(bufferType, BuffersAux) ;
	JumpNext() ;
}


/* TEST, EXTRACT & INIT */

Bool IsBuffer(Pt t)
{
	return IsThisExtra(bufferType, t) ;
}

BufferPt XTestBuffer(Pt t)
{
	return XTestExtra(bufferType, t) ;
}

void BuffersInit()
{
	bufferType = ExtraTypeNew("BUFFER", BufferSizeFun, nil, BufferBasicGCDelete, 1) ;

	InstallCBuiltinPred("buffer", 1, PBufferCheck) ;
	InstallCBuiltinPred("buffer_new", 1, PBufferNew) ;
	InstallCBuiltinPred("buffer_clear", 1, PBufferClear) ;
	InstallCBuiltinPred("buffer_delete", 1, PBufferDelete) ;
	InstallCBuiltinPred("buffer_size", 3, PBufferSize) ;
	InstallCBuiltinPred("buffer_set", 3, PBufferSet) ;
	InstallCBuiltinPred("buffer_get", 3, PBufferGet) ;
	InstallCBuiltinPred("buffer_as_atom", 2, PBufferAsAtom) ;
	InstallCBuiltinPred("buffer_write", 1, PBufferWrite) ;
	InstallCBuiltinPred("buffer_write", 2, PSBufferWrite) ;
	InstallGNDeterCBuiltinPred("current_buffer", 1, 2, PNDCurrentBuffer) ;
	InstallCBuiltinPred("buffers", 0, PBuffers) ;
}
