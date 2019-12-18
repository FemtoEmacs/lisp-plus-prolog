/*
 *   This file is part of the CxProlog system

 *   Locale.h
 *   by A.Miguel Dias - 2004/12/31
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Locale_
#define _Locale_


/* LOCALES */

#define SystemLocale()		( systemLocale )

extern Str systemLocale ;

Str SetLocaleCollate(Str l) ;
void LocaleInit(void) ;
void LocaleInit2(void) ;


/* ENCODINGS */

typedef enum
{
	octetEncoding,
	asciiEncoding,
	latin1Encoding,
	utf8Encoding,
	utf16beEncoding,
	utf16leEncoding,
	utf32beEncoding,
	utf32leEncoding,
/* Ambiguous */
	utf16Encoding,
	utf32Encoding,
	unicodeEncoding,	/* UCS-2 */
/* Compatibility */
	unicodebeEncoding,	/* UCS-2 BE */
	unicodeleEncoding,	/* UCS-2 LE */
	lastEncoding	/* Marks the end */
} CharEncoding ;

#define SystemEncoding()	( systemEncoding )
#define DefaultEncoding()	( defaultEncoding )

extern Str systemEncoding, defaultEncoding ;

#define EncodingGet(l)		((l)[0])
Str EncodingMake(Str enc) ;
Str EncodingName(Str l) ;
void EncodingsRestart(void) ;
void SetDefaultEncoding(Str enc) ;

WChar FileGetUsingCLib(FILE *file, Str locale, Str fName) ;
void FilePutUsingCLib(FILE *file, WChar c, Str locale, Str fName) ;

WChar StrGetCharUsingCLib(CharHdl s, Str locale) ;
void StrPutCharUsingCLib(CharHdl s, WChar c, Str locale) ;
WChar StrGetChar(CharHdl s, Str encoding) ;

Str StrInternalize(CharPt s) ;
Str StrInternalizeConv(CharPt s, WChar oldChar, WChar newChar) ;
Str StrInternalizeW(WCharPt s) ;
Str StrExternalize(Str s) ;

#endif
