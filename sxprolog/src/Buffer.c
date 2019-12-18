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
	UCharPt readPos, last ;
	Str encoding ;
	Bool autoAdjust ;
} Buffer ;

#define cBufferPt(x)			((BufferPt)(x))

#define BufferBegin(b)			((b)->begin)
#define BufferEnd(b)			((b)->end)
#define BufferLast(b)			((b)->last)
#define BufferReadPos(b)		((b)->readPos)
#define BufferEncoding(b)		((b)->encoding)
#define BufferAutoAdjust(b)		((b)->autoAdjust)

static ExtraTypePt bufferType ;


/* PRIVATE FUNCTIONS */

#define BufferWords(byteCapacity)	Words(byteCapacity)

static Size BufferCapacity(BufferPt b)
{
	return BufferEnd(b) - BufferBegin(b) ;
}

static Size BufferFreeSpace(BufferPt b)
{
	return BufferEnd(b) - BufferLast(b) ;
}

static void BufferInit(BufferPt b, Size byteCapacity,
						Size resPosOffset, Size lastOffset)
{
	BufferBegin(b) = Allocate(BufferWords(byteCapacity), false) ;
	BufferEnd(b) = BufferBegin(b) + byteCapacity ;
	BufferReadPos(b) = BufferBegin(b) + resPosOffset ;
	BufferLast(b) = BufferBegin(b) + lastOffset ;
}

static Bool BufferAdjust(BufferPt b, Size freeSpace)
{
	register UCharPt a, z ;
	if( !BufferAutoAdjust(b) ) return false ;
	if( BufferReadPos(b) - BufferBegin(b) < freeSpace + 100 ) return false ;
	for( a = BufferReadPos(b), z = BufferBegin(b) ; a < BufferLast(b) ; *z++ = *a++ ) ;
	BufferReadPos(b) = BufferBegin(b) ;
	BufferLast(b) = z ;
	return true ;
}

static void BufferExpand(BufferPt b, Size freeSpace)
{
	if( BufferAdjust(b, freeSpace) ) return ;
	PInt cap ;
	UCharPt bb = BufferBegin(b) ;
	UCharPt bl = BufferLast(b) ;
	Size resPosOffset = BufferReadPos(b) - bb ;
	Size lastOffset = bl - bb ;
	register UCharPt s, h ;
	Size oldCapacity = BufferCapacity(b) ;
	Size minCapacity = oldCapacity + (freeSpace - BufferFreeSpace(b));
	for( cap = 2 * oldCapacity ; cap < minCapacity ; cap *= 2 ) ;
	BufferInit(b, cap, resPosOffset, lastOffset) ;
	for( h = BufferBegin(b), s = bb ; s < bl ; *h++ = *s++ ) ;
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
	StreamWrite(srm, "     (current capacity %ld, readPos=%ld, last=%ld)\n",
					BufferCapacity(b),
					BufferReadPos(b)-BufferBegin(b),
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
	BufferAutoAdjust(b) = false ;
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
	BufferReadPos(b) = BufferBegin(b) ;
	BufferLast(b) = BufferBegin(b) ;
}

int BufferGet(BufferPt b, PInt idx)
{
	if( BufferBegin(b) + idx < BufferLast(b) )
		return BufferBegin(b)[idx] ;
	else return 0 ;
}

void BufferSet(BufferPt b, PInt idx, int i)
{
	UCharPt pos = BufferBegin(b) + idx;
	if( pos >= BufferEnd(b) )
		BufferExpand(b, pos - BufferLast(b) + 1) ;  // AMD check please
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
		if( BufferReadPos(b) > BufferLast(b) )
			BufferReadPos(b) = BufferLast(b) ;
	}
}

void BufferShow(BufferPt b)
{
	BufferWrite(userOut, b) ;
}

void BufferAutoAdjustOn(BufferPt b)
{
//	Mesg("BufferAutoAdjustOn");
	BufferAutoAdjust(b) = true ;	
}

Size BufferGetReadPos(BufferPt b)
{
//	Mesg("BufferGetReadPos %d", BufferReadPos(b) - BufferBegin(b));
	return BufferReadPos(b) - BufferBegin(b) ;
}

void BufferSetReadPos(BufferPt b, Size pos)
{
//	Mesg("BufferSetReadPos %d", pos);
	BufferReadPos(b) = BufferBegin(b) + pos ;
}


/* SEQUENTIAL OPERATIONS */

void BufferEnsureFreeSpace(BufferPt b, Size freeSpace)
{
	if( BufferFreeSpace(b) < freeSpace )
		BufferExpand(b, freeSpace) ;
}

void BufferSetSizeUnsafe(BufferPt b, Size size)
{
	BufferLast(b) = BufferBegin(b) + size ;
}

void BufferReset(BufferPt b)
{
	BufferReadPos(b) = BufferBegin(b) ;
}

void BufferRewrite(BufferPt b)
{
	BufferClear(b) ;
}

void BufferAppend(BufferPt b)
{
	Unused(b) ;
	/* nothing */
}


/* SEQUENTIAL READ OPERATIONS */

Bool BufferAtEnd(BufferPt b)
{
	return BufferReadPos(b) >= BufferLast(b) ;
}

WChar BufferGetByte(BufferPt b)
{
	if( BufferReadPos(b) >= BufferLast(b) ) return EOF ;
	return *BufferReadPos(b)++ ;
}

WChar BufferPeekByte(BufferPt b)
{
	if( BufferReadPos(b) >= BufferLast(b) ) return EOF ;
	return *BufferReadPos(b) ;
}

Size BufferGetNBytes(BufferPt b, VoidPt v, Size n)
{
	if( BufferLast(b) - BufferReadPos(b) < n )
		n = BufferLast(b) - BufferReadPos(b) ;
	CopyBytes(v, cCharPt(BufferReadPos(b)), n) ;
	BufferReadPos(b) += n ;
	return n ;
}

WChar BufferGetChar(BufferPt b)
{
	CharPt s ;
	WChar c ;
	if( BufferReadPos(b) >= BufferLast(b) ) return EOF ;
	s = cCharPt(BufferReadPos(b)) ;
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
			c = Utf8DecodeN(&s, BufferLast(b) - BufferReadPos(b)) ;
			break ;
		}
		case utf16Encoding: {
			FileError("The encoding utf16 is ambiguous") ;
			c = 0 ;		/* Avoids warning */
			break ;
		}
		case utf16beEncoding: {
			c = Utf16DecodeN(&s, BufferLast(b) - BufferReadPos(b), true) ;
			break ;
		}
		case utf16leEncoding: {
			c = Utf16DecodeN(&s, BufferLast(b) - BufferReadPos(b), false) ;
			break ;
		}
		case utf32Encoding: {
			FileError("The encoding utf32 is ambiguous") ;
			c = 0 ;		/* Avoids warning */
			break ;
		}
		case utf32beEncoding: {
			c = Utf32DecodeN(&s, BufferLast(b) - BufferReadPos(b), true) ;
			break ;
		}
		case utf32leEncoding: {
			c = Utf32DecodeN(&s, BufferLast(b) - BufferReadPos(b), false) ;
			break ;
		}
		case unicodeEncoding: {
			FileError("The encoding unicode is ambiguous") ;
			c = 0 ;		/* Avoids warning */
			break ;
		}
		case unicodebeEncoding: {
			c = Ucs2DecodeN(&s, BufferLast(b) - BufferReadPos(b), true) ;
			break ;
		}
		case unicodeleEncoding: {
			c = Ucs2DecodeN(&s, BufferLast(b) - BufferReadPos(b), false) ;
			break ;
		}
		default: {
			c = StrGetCharUsingCLib(&s, BufferEncoding(b)) ;
			break ;
		}
	}
	BufferReadPos(b) = cUCharPt(s) ;
	return c ;
}

WChar BufferPeekChar(BufferPt b)
{
	UCharPt pos = BufferReadPos(b) ;
	WChar c = BufferGetChar(b) ;
	BufferReadPos(b) = pos ;
	return c ;
}

Str BufferGetAllAndClear(BufferPt b)
{
	BigStrOpen();
	BigStrAddStrSlice((Str)BufferBegin(b), 0, BufferLast(b) - BufferBegin(b)) ;
	BufferClear(b);
	return BigStrClose();
}


/* SEQUENTIAL WRITE OPERATIONS */

void BufferPutByte(BufferPt b, WChar c)
{
	BufferEnsureFreeSpace(b, 1);
	*BufferLast(b)++ = c ;
}

Size BufferPutNBytes(BufferPt b, VoidPt v, Size n)
{
	BufferEnsureFreeSpace(b, n) ;
	CopyBytes(cCharPt(BufferLast(b)), v, n) ;
	BufferLast(b) += n ;
	return n ;
}

void BufferPutChar(BufferPt b, WChar c)
{
	CharPt s ;
	BufferEnsureFreeSpace(b, MB_LEN_MAX) ;
	s = cCharPt(BufferLast(b)) ;
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
	BufferLast(b) = cUCharPt(s) ;
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
	BufferEnsureFreeSpace(b, 1) ;
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
