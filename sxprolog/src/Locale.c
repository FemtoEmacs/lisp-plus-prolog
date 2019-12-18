/*
 *   This file is part of the CxProlog system

 *   Locale.c
 *   by A.Miguel Dias - 2004/12/31
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


/* LOCALES */

#include <locale.h>

Str systemLocale, currentLocale ;

static Str EnableLocaleCType(Str l)
{	/* validates and allocates the locale string */
	Str loc ; /* cannot optimize because of fail_on_error */
	if( (loc = setlocale(LC_CTYPE, l)) == nil )
		return Error("Unknown locale %s", l) ;
	else {
		currentLocale = AtomName(LookupAtom(loc)) ;
		return currentLocale ;
	}
}

static Str EnableSystemLocaleCType()
{
	Str loc ; /* cannot optimize because of fail_on_error */
	if( (loc = setlocale(LC_CTYPE, "")) == nil ) {
		Warning("Locale not supported by C library. Using the fallback 'C' locale") ;
		return EnableLocaleCType("C") ;
	}
	else {
		currentLocale = AtomName(LookupAtom(loc)) ;
		return currentLocale ;
	}
}

#if UNDERSTAND_EXTERNAL_ENCODINGS

#define UseLocale(l)		if( (l) != currentLocale ) SetLocaleCType(l)
	
static void SetLocaleCType(Str l)
{	/* pre: l is a previously validated and allocated locale string */
	if( setlocale(LC_CTYPE, l) == nil )
		FatalError("Unknown locale %s", l) ;
	currentLocale = l ;
}

#define FORCE_GLIB			0
#endif

Str SetLocaleCollate(Str l)
{
	Str256 s ;
	strcpy(s, l) ;
#if UNDERSTAND_EXTERNAL_ENCODINGS
	strcat(s, ".utf8") ;
#else
	strcat(s, ".iso88591") ;
#endif
	if( setlocale(LC_COLLATE, s) == nil )
		return nil ;
	return AtomName(LookupAtom(l)) ;
}


/* ENCODINGS */

#if UNDERSTAND_EXTERNAL_ENCODINGS
#include <wchar.h>
static mbstate_t ps ;
#endif

Str systemEncoding ; /* Used for interpreting filenames; OS environment
                      variable names and contents; command-line arguments. */
Str defaultEncoding ; /* Default encoding for files and streams. */

static Char encodings[lastEncoding] ;

static void InitEncodings(void)
{
	CharPt pt = encodings ;
	CharEncoding e ;
	for( e = octetEncoding ; e < lastEncoding ; e++ )
		*pt++ = cChar(e) ;
}

static Str EncodingMakeFromLocale(Str l)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
  #if !FORCE_GLIB
	Str enc ;
	if( (enc = strchr(l, '.')) != nil ) {
		switch( enc[1] ) {
			case 'i': {
				if( StrEqual(enc, ".iso-8859-1") || StrEqual(enc, ".iso_8859-1")
						|| StrEqual(enc, ".iso-8859-15")
						|| StrEqual(enc, ".iso_8859-15") )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'I': {
				if( StrEqual(enc, ".ISO-8859-1") || StrEqual(enc, ".ISO_8859-1")
						|| StrEqual(enc, ".ISO-8859-15")
						|| StrEqual(enc, ".ISO_8859-15") )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'u': {
				if( StrEqual(enc, ".utf-8") || StrEqual(enc, ".utf8") )
					return encodings + utf8Encoding ;
				break ;
			}
			case'U': {
				if( StrEqual(enc, ".UTF-8") || StrEqual(enc, ".UTF8") )
					return encodings + utf8Encoding ;
				break ;
			}
	#if OS_WIN
			case '1': {
				if( StrEqual(enc, ".1252") )
					return encodings + latin1Encoding ;
				break ;
			}
			case '4': {
				if( StrEqual(enc, ".437") )
					return encodings + latin1Encoding ;
				break ;
			}
			case '8': {
				if( StrEqual(enc, ".850") )
					return encodings + latin1Encoding ;
				break ;
			}
	#endif
		}
	}
	else {
		switch( *l ) {
			case 'b': {
				if( StrEqualN(l, "br_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'c': {
				if( StrEqualN(l, "ca_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'd': {
				if( StrEqualN(l, "de_", 3) || StrEqualN(l, "da_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'e': {
				if( StrEqualN(l, "en_", 3) || StrEqualN(l, "es_", 3)
						|| StrEqualN(l, "eu_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'f': {
				if( StrEqualN(l, "fr_", 3) || StrEqualN(l, "fi_", 3)
													|| StrEqualN(l, "fo_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'i': {
				if( StrEqualN(l, "it_", 3)  )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'n': {
				if( StrEqualN(l, "nl_", 3) || StrEqualN(l, "nn_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'p': {
				if( StrEqualN(l, "pt_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
			case 'u': {
				if( StrEqualN(l, "us_", 3) )
					return encodings + latin1Encoding ;
				break ;
			}
		}
	}

	if( (enc = strchr(l, '@')) != nil ) {
		if( StrEqual(enc, "@euro") )
			return encodings + latin1Encoding ;
	}
  #endif
	return l ; /* The locale becomes the encoding */
#else
	return encodings + latin1Encoding ;
#endif

}

Str EncodingMake(Str enc)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	switch( *enc ) {
		case 's':
			if( StrEqual(enc, "system") )
				return SystemEncoding() ;
			break ;
		case 't':
			if( StrEqual(enc, "text") )
				return DefaultEncoding() ;
			break ;
  #if !FORCE_GLIB
		case 'a':
			if( StrEqual(enc, "ascii") )
				return encodings + asciiEncoding ;
			break ;
		case 'i':
			if( StrEqual(enc, "iso_latin_1") )
				return encodings + latin1Encoding ;
			break ;
		case 'o':
			if( StrEqual(enc, "octet") )
				return encodings + octetEncoding ;
			break ;
		case 'u':
			if( StrEqual(enc, "utf8") )
				return encodings + utf8Encoding ;
			if( StrEqual(enc, "utf16") )
				return encodings + utf16Encoding ;
			if( StrEqual(enc, "utf16be") )
				return encodings + utf16beEncoding ;
			if( StrEqual(enc, "utf16le") )
				return encodings + utf16leEncoding ;
			if( StrEqual(enc, "utf32") )
				return encodings + utf32Encoding ;
			if( StrEqual(enc, "utf32be") )
				return encodings + utf32beEncoding ;
			if( StrEqual(enc, "utf32le") )
				return encodings + utf32leEncoding ;
			if( StrEqual(enc, "unicode") )
				return encodings + unicodeEncoding ;
			if( StrEqual(enc, "unicode_be") )
				return encodings + unicodebeEncoding ;
			if( StrEqual(enc, "unicode_le") )
				return encodings + unicodeleEncoding ;
			break ;
  #endif
	}
	return EnableLocaleCType(enc) ; /* The encoding is assumed to be a locale */
#else
	return encodings + latin1Encoding ;
#endif
}

Str EncodingName(Str l)
{
	switch( l[0] ) {
		case asciiEncoding:
			return "ascii" ;
		case utf8Encoding:
			return "utf8" ;
		case utf16Encoding:
			return "utf16" ;
		case utf16beEncoding:
			return "utf16be" ;
		case utf16leEncoding:
			return "utf16le" ;
		case utf32Encoding:
			return "utf32" ;
		case utf32beEncoding:
			return "utf32be" ;
		case utf32leEncoding:
			return "utf32le" ;
		case octetEncoding:
			return "octet" ;
		case latin1Encoding:
			return "iso_latin_1" ;
		case unicodeEncoding:
			return "unicode" ;
		case unicodebeEncoding:
			return "unicode_be" ;
		case unicodeleEncoding:
			return "unicode_le" ;
		default:
			return l ; /* The locale is THE encoding */
	}
}

void EncodingsRestart()
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	ClearBytes(&ps, sizeof(mbstate_t)) ;
#endif
}

void SetDefaultEncoding(Str enc)
{
	defaultEncoding = enc ;
}

WChar FileGetUsingCLib(FILE *file, Str locale, Str fName)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	WChar c ;
	wchar_t wc ;  /* The width of wchar_t is compiler-specific */
	Char ch ;
	Size len = 0 ;	/* 0 used for EOF detection */
	UseLocale(locale) ;
	do {
		if( (c = fgetc(file)) == EOF ) {
			if( len == 0 )
				return EOF ;
			else {
				len = -1 ; /* Force error */
				break ;
			}
		}
		ch = c ;
	} while( (len = mbrtowc(&wc, &ch, 1, &ps)) == -2 ) ;
	if( len < 0 )
		FileError("Wide character conversion error on input file '%s'",
														fName) ;
	return CharFixCode(wc) ;
#else
	WChar c = fgetc(file) ;
	return CharFixCode(c) ;
#endif
}

void FilePutUsingCLib(FILE *file, WChar c, Str locale, Str fName)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	char buff[MB_LEN_MAX] ;
	Size len ;
	UseLocale(locale) ;
	if( (len = wcrtomb(buff, c, &ps)) == -1 ) {
		if( OSIsATty(file) ) { /* Instead of giving an error... */
			strcpy(buff, CharAsNumericEscape(c)) ;
			len = strlen(buff) ;
		}
		else
			FileError("Could not convert to multibyte sequence on file '%s'", fName) ;
	}
	if( cSize(fwrite(buff, 1, len, file)) != len )
		FileError("Could not write to file '%s'", fName) ;
#else
	if( fputc(c, file) == EOF )
		FileError("Could not write to file '%s'", fName) ;
#endif
}

WChar StrGetCharUsingCLib(CharHdl s, Str locale)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	wchar_t wc ;  /* The width of wchar_t is compiler-specific */
	Size len ;
	UseLocale(locale) ;
	if( (len = mbrtowc(&wc, *s, MB_LEN_MAX, &ps)) <= 0 ) {
		if( len == 0 )
			len = 1 ;	/* L'\0' */
		else
			Error("Could not convert to wide character (StrGetCharInternalize)") ;
	}
	*s += len ;
	return CharFixCode(wc) ;
#else
	return *cUCharPt((*s)++) ;
#endif
}

void StrPutCharUsingCLib(CharHdl s, WChar c, Str locale)
{  /* pre: s has MB_LEN_MAX free slots, at least */
#if UNDERSTAND_EXTERNAL_ENCODINGS
	Size len ;
	UseLocale(locale) ;
	if( (len = wcrtomb(*s, c, &ps)) == -1 )
		Error("Could not convert to multibyte sequence (StrPutCharExternalize)") ;
	*s += len ;
#else
	*(*s)++ = c ;
#endif
}

WChar StrGetChar(CharHdl s, Str encoding)  /* Assume '\0' terminated */
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	switch( EncodingGet(encoding) ) {
		case octetEncoding:		return IError("Operation not available for the encoding octet") ;
		case asciiEncoding:
			if( *cUCharPt(*s) > 127 )
				Warning("Non-ascii char (%d) in ascii string", *cUCharPt(*s)) ;
			return *cUCharPt((*s)++) ;
		case latin1Encoding:	return *cUCharPt((*s)++) ;
		case utf8Encoding:		return Utf8Decode(s) ;
		case utf16Encoding:		return IError("The encoding utf16 is ambiguous") ;
		case utf16beEncoding:	return Utf16Decode(s, true) ;
		case utf16leEncoding:	return Utf16Decode(s, false) ;
		case utf32Encoding:		return IError("The encoding utf32 is ambiguous") ;
		case utf32beEncoding:	return Utf32Decode(s, true) ;
		case utf32leEncoding:	return Utf32Decode(s, false) ;
		case unicodeEncoding: 	return IError("The encoding unicode is ambiguous") ;
		case unicodebeEncoding:	return Ucs2Decode(s, true) ;
		case unicodeleEncoding:	return Ucs2Decode(s, false) ;
		default:				return StrGetCharUsingCLib(s, encoding) ;
	}
#else
	return *cUCharPt((*s)++) ;
#endif
}

Str StrInternalizeConv(CharPt s, WChar oldChar, WChar newChar)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	WChar c ;
	if( s == nil ) return s ;
	GStrOpen() ;
	switch( EncodingGet(SystemEncoding()) ) {
		case octetEncoding: {
			Error("This operation is not available for strings") ;
			break ;
		}
		case asciiEncoding: {
			while( (c = *cUCharPt(s++)) != '\0' ) {
				if( c > 127 )
					Warning("Non-ascii char (%d) in ascii string '%s'", c, s) ;
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case latin1Encoding: {
			while( (c = *cUCharPt(s++)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case utf8Encoding: {
			while( (c = Utf8Decode(&s)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case utf16Encoding: {
			Error("The encoding utf16 is ambiguous") ;
			break ;
		}
		case utf16beEncoding: {
			while( (c = Utf16Decode(&s, true)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case utf16leEncoding: {
			while( (c = Utf16Decode(&s, false)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case utf32Encoding: {
			Error("The encoding utf32 is ambiguous") ;
			break ;
		}
		case utf32beEncoding: {
			while( (c = Utf32Decode(&s, true)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case utf32leEncoding: {
			while( (c = Utf32Decode(&s, false)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case unicodeEncoding: {
			Error("The encoding unicode is ambiguous") ;
			break ;
		}
		case unicodebeEncoding: {
			while( (c = Ucs2Decode(&s, true)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		case unicodeleEncoding: {
			while( (c = Ucs2Decode(&s, false)) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
		default: {
			while( (c = StrGetCharUsingCLib(&s, SystemLocale())) != '\0' ) {
				if( c == oldChar ) c = newChar ;
				GStrAddChar(c) ;
			}
			break ;
		}
	}
	return GStrClose() ;
#else
  /* This is safe for the intended uses */
	register CharPt pt ;
	for( pt = s ; *pt != '\0' ; pt++ )
		if( *pt == oldChar )
			*pt = newChar ;
	return s ;
#endif
}

Str StrInternalize(CharPt s)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS	
	return StrInternalizeConv(s, '\0', '\0') ;
#else
	return s ;
#endif
}

Str StrInternalizeW(WCharPt s)
{
	WChar c ;
	if( s == nil ) InternalError("StrInternalizeW") ;
	GStrOpen() ;
#if OS_WIN
	while( (c = Utf16Decode(cCharHdl(&s), false)) != '\0' )
		GStrAddChar(c) ;
#else
	while( (c = Utf32Decode(cCharHdl(&s), false)) != '\0' )
		GStrAddChar(c) ;
#endif
	return GStrClose() ;
}

Str StrExternalize(Str s)
{
#if UNDERSTAND_EXTERNAL_ENCODINGS
	WChar c ;
	GStrOpen() ;
	switch( EncodingGet(SystemEncoding()) ) {
		case octetEncoding: {
			Error("This operation is not available for strings") ;
			break ;
		}
		case asciiEncoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				if( !InRange(c, 0, 127) )
					FileError("Non-ascii char (%d) in ascii string '%s'", c, s) ;
				GStrCheck() ;
				*GStrCurr()++ = c ;
			}
			break ;
		}
		case latin1Encoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				if( !InRange(c, 0, 255) )
					FileError("Non-latin1 char (%d) in latin1 buffer '%s'", c, s) ;
				GStrCheck() ;
				*GStrCurr()++ = c ;
			}
			break ;
		}
		case utf8Encoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				Utf8Encode(&GStrCurr(), c) ;				
			}
			break ;
		}
		case utf16Encoding: {
			FileError("The encoding utf16 is ambiguous") ;
			break ;
		}
		case utf16beEncoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				Utf16Encode(&GStrCurr(), c, true) ;
			}
			break ;
		}
		case utf16leEncoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				Utf16Encode(&GStrCurr(), c, false) ;
			}
			break ;
		}
		case utf32Encoding: {
			FileError("The encoding utf32 is ambiguous") ;
			break ;
		}
		case utf32beEncoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				Utf32Encode(&GStrCurr(), c, true) ;
			}
			break ;
		}
		case utf32leEncoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				Utf32Encode(&GStrCurr(), c, false) ;
			}
			break ;
		}
		case unicodeEncoding: {
			FileError("The encoding unicode is ambiguous") ;
			break ;
		}
		case unicodebeEncoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				Ucs2Encode(&GStrCurr(), c, true) ;
			}
			break ;
		}
		case unicodeleEncoding: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				Ucs2Encode(&GStrCurr(), c, false) ;
			}
			break ;
		}
		default: {
			while( (c = CharDecode(s)) != '\0' ) {
				GStrCheck() ;
				StrPutCharUsingCLib(&GStrCurr(), c, SystemLocale()) ;
			}
			break ;
		}
	}
	return GStrClose() ;
#else
	return s ;
#endif
}


/* CXPROLOG C'BUILTINS */

static void PLocale()
{
	ShowVersion() ;
	Write("LOCALE:\n    '%s'\n", SystemLocale()) ;
	JumpNext() ;
}

void LocaleInit()
{
	systemLocale = EnableSystemLocaleCType() ;
	InitEncodings() ;
	SetDefaultEncoding(EncodingMakeFromLocale(systemLocale)) ;
	systemEncoding = DefaultEncoding() ;
	EncodingsRestart() ;
}

void LocaleInit2()
{
	InstallCBuiltinPred("locale", 0, PLocale) ;
}
