/*
 *   This file is part of the CxProlog system

 *   Utf8.h
 *   by A.Miguel Dias - 2004/12/19
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Utf8_
#define _Utf8_

#define USE_UTF8_AS_INTERNAL_ENCODING		1

#if UNDERSTAND_EXTERNAL_ENCODINGS && USE_UTF8_AS_INTERNAL_ENCODING
/* Use UTF-8 as the internal encoding of text */
#define maxChar				(0x10ffff)
#define CharType(c)			( ((c)<=255) ? BasicCharType(c) : _LC )
#define CharEncode(s,c)		( Utf8Encode(cCharHdl(&(s)),c) )
#define CharDecode(s)		( Utf8Decode(cCharHdl(&(s))) )
#define CharCopy(z,a)		( Utf8Copy(&(z),&(a)) )
#define CharFirst(s)		( Utf8First(s) )
#define CharLast(s)			( Utf8Last(s) )
#define CharPos(s,i)		( Utf8Pos(s, i) )
#define CharLen(s)			( Utf8Len(s) )
#define CharNxt(s)			( Utf8Next(s) )	/* CharNxt is Windows friendly */
#define CharFixCode(c)		( (c) )

#else
/* Use Latin-1 as the internal encoding of text.
   Chars with code greater than 255 are converted to '?' */
#define maxChar				(255)
#define CharType(c)			( BasicCharType(c) )
#define CharEncode(s,c)		( Ignore(*(s)++ = cChar(c)) )
#define CharDecode(s)		( *cUCharPt((s)++) )
#define CharCopy(z,a)		( Ignore(*(z)++ = *(a)++) )
#define CharFirst(s)		( cUCharPt(s)[0] )
#define CharLast(s)			( cUCharPt(s)[strlen(s)-1] )
#define CharPos(s,i)		( (s) + (i) )
#define CharLen(s)			( strlen(s) )
#define CharNxt(s)			( (s) + 1 )	/* CharNext already used in Windows */
#define CharFixCode(c)		( (c) <= maxChar ? c : '?' )
#endif


/* ENCODE UTF-8 */
void Utf8Encode(CharPt *s, WChar c) ;

/* DECODE UTF-8 */
WChar Utf8Decode(CharPt *s) ;
WChar Utf8DecodeN(CharPt *s, Size n) ;

/* COPY UTF-8 */
void Utf8Copy(CharPt *z, CharPt *a) ;
void Utf8NCopy(CharPt *z, CharPt *a, Size n) ;

/* INDEXING UTF-8 */
WChar Utf8First(Str s) ;
WChar Utf8Last(Str s) ;
CharPt Utf8Pos(CharPt s, int i) ;
Size Utf8Len(Str s) ;
CharPt Utf8Next(CharPt s) ;

/* FILE HANDLING UTF-8 */
WChar Utf8FileGet(FILE *file, Str fName) ;
void Utf8FilePut(FILE *file, WChar c, Str fName) ;

/* UNICODE RANGE */
void ValidateUnicodeRange(WChar c, Bool strict) ;

#endif
