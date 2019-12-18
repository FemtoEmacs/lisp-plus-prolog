/*
 *   This file is part of the CxProlog system

 *   File.c
 *   by A.Miguel Dias - 2007/01/01
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

/* FILE */

typedef struct File {
	ExtraDef(File) ;
	FILE *file ;
	WChar buffer ;
	Str encoding ;
	Str name ;
	int mode ;
	Bool bom ;
	Bool allowsNumEscapes ;
} File ;

#define cFilePt(x)				((FilePt)(x))

#define FileFILE(f)				((f)->file)
#define FileBuffer(f)			((f)->buffer)
#define FileEncoding(f)			((f)->encoding)
#define FileName(f)				((f)->name)
#define FileMode(f)				((f)->mode)
#define FileBom(f)				((f)->bom)
#define FileAllowsNumEscapes(f)	((f)->allowsNumEscapes)

#define NO_CHAR					cWChar(EOF)


static ExtraTypePt fileType ; /* @@@ Slave of streamType, will delete. */


/* PRIVATE FUNCTIONS */

static void FileDisable(FilePt f, Bool force)
{
	if( ExtraDisable(f) ) {
		if( !force
			&& (FileMode(f) == mWrite || FileMode(f) == mAppend)
			&& fflush(FileFILE(f)) != 0 )
				FileError("Could not close file '%s'", FileName(f)) ;
		fclose(FileFILE(f)) ;
	}
}

static Size FileSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(File) ;
}

static void FileUnget(FilePt f, WChar c)
{
	FileBuffer(f) = c ;
}

static FILE *FileOpen(Str name, int mode)
{
	Str32 m ;
	strcpy(m, mode==mWrite ? "wb" : mode==mRead ? "rb" : "ab") ;
	return fopen(StrExternalize(cCharPt(name)), m) ;
}

static void ReadBom(FilePt f)
{
	FILE *file = FileFILE(f) ;
	int c ;
	switch( c = fgetc(file) ) {
		case 0xEF: { /* EF BB BF ---> UTF8 */
			if( fgetc(file) == 0xBB && fgetc(file) == 0xBF ) {
				FileEncoding(f) = EncodingMake("utf8") ;
				return ;
			}
			break ;
		}
		case 0xFE: { /* EF FF ---> UTF16-BE */
			if( fgetc(file) == 0xFF ) {
				if( EncodingGet(FileEncoding(f)) == unicodebeEncoding
						|| EncodingGet(FileEncoding(f)) == unicodeleEncoding )
					FileEncoding(f) = EncodingMake("unicode_be") ;
				else
					FileEncoding(f) = EncodingMake("utf16be") ;
				return ;
			}
			break ;
		}
		case 0x00: { /* 00 00 FE FF ---> UTF32-BE */
			if( fgetc(file) == 0x00 && fgetc(file) == 0xFE && fgetc(file) == 0xFF ) {
				FileEncoding(f) = EncodingMake("utf32be") ;
				return ;
			}
			break ;
		}
		case 0xFF: { /* FF FE 00 0 ---> UTF32-LE or UTF16-LE */
			if( fgetc(file) == 0xFE ) {
				if( fgetc(file) == 0x00 && fgetc(file) == 0x00 ) {
					FileEncoding(f) = EncodingMake("utf32le") ;
					return ;
				}
				else {
					if( EncodingGet(FileEncoding(f)) == unicodebeEncoding
							|| EncodingGet(FileEncoding(f)) == unicodeleEncoding )
						FileEncoding(f) = EncodingMake("unicode_le") ;
					else
						FileEncoding(f) = EncodingMake("utf16le") ;
					rewind(file) ;
					if( fgetc(file) != 0xFF || fgetc(file) != 0xFE )
						InternalError("ReadBom") ;
					return ;
				}
				break ;
			}
			break ;
		}
	}

/* No BOM found */
	FileBom(f) = false ;
	rewind(file) ;
}

static void WriteBom(FilePt f)
{
	FILE *file = FileFILE(f) ;
	Str fName = FileName(f) ;
	switch( EncodingGet(FileEncoding(f)) ) {
		case utf8Encoding: {
			if( fputs("\xEF\xBB\xBF", file) == EOF )
				FileError("Could not write utf8 BOM to file '%s'", fName) ;
			break ;
		}
		case utf16beEncoding:
		case unicodebeEncoding: {
			Ucs2FilePut(file, 0xFEFF, fName, true) ;
			break ;
		}
		case utf16leEncoding:
		case unicodeleEncoding: {
			Ucs2FilePut(file, 0xFEFF, fName, false) ;
			break ;
		}
		case utf32beEncoding: {
			Utf32FilePut(file, 0x0000FEFF, fName, true) ;
			break ;
		}
		case utf32leEncoding: {
			Utf32FilePut(file, 0x0000FEFF, fName, false) ;
			break ;
		}
		case unicodeEncoding:
		case utf16Encoding:
		case utf32Encoding: {
			InternalError("WriteBom") ;
			break ;
		}
		default: {
			FileBom(f) = false ;
			break ;
		}
	}
}


/* MAIN OPERATIONS */

FilePt FileNew(FILE *file, Str name, int mode, Str encoding, Bool3 bom)
{
	FilePt f ;
	Bool diskFile = file == nil ;
	Pt at = MakeTempAtom(name) ;
	name = XAtomName(at) ; /* name string will be protected by stream */
	if( diskFile && mode == mRead && !OSExists(name) )
		ExistenceError("source_sink", at, "No such file '%s'", name) ;
	if( diskFile && (file = FileOpen(name, mode)) == nil )
		FileError("Cannot open file '%s'", name) ;
	f = ExtraNew(fileType, 0) ;
	FileFILE(f) = file ;
	FileBuffer(f) = NO_CHAR ;
	FileSetEncoding(f, encoding) ;
	FileBom(f) = false ;
	FileName(f) = cCharPt(name) ;
	FileMode(f) = mode ;
	if( FileIsATty(f) )
		FileAllowsNumericEscapes(f) ;
	if( diskFile && UNDERSTAND_EXTERNAL_ENCODINGS ) {
		if( mode == mWrite ) {
			if( bom == true3 ) {	/* Default is false */
				FileBom(f) = true ;
				WriteBom(f) ;
			}
		}
		else {
			if( bom != false3 ) {	/* Default is true */
				FileBom(f) = true ;
				ReadBom(f) ;
			}
		}
	}
	return f ;
}

void FileSetEncoding(FilePt f, Str encoding)
{
	switch( EncodingGet(encoding) ) {
		case unicodeEncoding: {
			FileEncoding(f) = EncodingMake("unicode_be") ;
			break ;
		}
		case utf16Encoding: {
			FileEncoding(f) = EncodingMake("utf16be") ;
			break ;
		}
		case utf32Encoding: {
			FileEncoding(f) = EncodingMake("utf32be") ;
			break ;
		}
		default: {
			FileEncoding(f) = encoding ;
			break ;
		}
	}
}

Str FileGetEncoding(FilePt f)
{
	return FileEncoding(f) ;
}

Bool FileGetBom(FilePt f)
{
	return FileBom(f) ;
}

void FileDelete(FilePt f, Bool force)
{
	FileDisable(f, force) ;
}

FILE *FileAsFILE(FilePt f)
{
	return FileFILE(f) ;
}

Bool FileIsATty(FilePt f)
{
	return OSIsATty(FileFILE(f)) ;
}

void FileAllowsNumericEscapes(FilePt f)
{
	FileAllowsNumEscapes(f) = true ;
}


/* SEQUENTIAL READ OPERATIONS */

WChar FileGetByte(FilePt f)
{
	return fgetc(FileFILE(f)) ;
}

WChar FilePeekByte(FilePt f)
{
	return ungetc(fgetc(FileFILE(f)), FileFILE(f)) ;
}

Size FileGetNBytes(FilePt f, VoidPt v, Size n, Bool isText)
{
	Unused(isText) ;
	return fread(v, 1, n, FileFILE(f)) ;
}

static WChar FileGet(FilePt f)
{
	FILE *file = FileFILE(f) ;
	Str fName = FileName(f) ;
	if( FileBuffer(f) != NO_CHAR ) {
		WChar c = FileBuffer(f) ;
		FileBuffer(f) = NO_CHAR ;
		return c ;
	}
	switch( EncodingGet(FileEncoding(f)) ) {
		case octetEncoding: {
			FileError("This operation is not available for binary streams") ;
			return 0 ;
		}
		case asciiEncoding: {
			int c ;
			if( (c = fgetc(file)) > 127 )
				Warning("Non-ascii char (%d) found in ascii file '%s'", c, fName) ;
			return c ;
		}
		case latin1Encoding: {
			return fgetc(file) ;
		}
		case utf8Encoding: {
			return Utf8FileGet(file, fName) ;
		}
		case utf16Encoding: {
			FileError("The encoding 'utf16' is ambiguous") ;
			return 0 ;
		}
		case utf16beEncoding: {
			return Utf16FileGet(file, fName, true) ;
		}
		case utf16leEncoding: {
			return Utf16FileGet(file, fName, false) ;
		}
		case utf32Encoding: {
			FileError("The encoding 'utf32' is ambiguous") ;
			return 0 ;
		}
		case utf32beEncoding: {
			return Utf32FileGet(file, fName, true) ;
		}
		case utf32leEncoding: {
			return Utf32FileGet(file, fName, false) ;
		}
		case unicodeEncoding: {
			FileError("The encoding 'unicode' is ambiguous") ;
			return 0 ;
		}
		case unicodebeEncoding: {
			return Ucs2FileGet(file, fName, true) ;
		}
		case unicodeleEncoding: {
			return Ucs2FileGet(file, fName, false) ;
		}
		default: {
			return FileGetUsingCLib(file, FileEncoding(f), fName) ;
		}
	}
}

WChar FileGetChar(FilePt f)
{
	return FileGet(f) ;
}

WChar FilePeekChar(FilePt f)
{
	WChar c = FileGet(f) ;
	if( c != EOF ) FileUnget(f, c) ;
	return c ;
}



/* SEQUENTIAL WRITE OPERATIONS */

void FileFlush(FilePt f)
{
	if( fflush(FileFILE(f)) != 0 )
		FileError("Could not flush file '%s'", FileName(f)) ;
}

void FilePutByte(FilePt f, WChar c)
{
	if( fputc(c, FileFILE(f)) == EOF )
		FileError("Could not write to file '%s'", FileName(f)) ;
}

Size FilePutNBytes(FilePt f, VoidPt v, Size n, Bool isText)
{
	Unused(isText) ;
	return fwrite(v, 1, n, FileFILE(f)) ;
}

void FilePutChar(FilePt f, WChar c)
{
	FILE *file = FileFILE(f) ;
	Str fName = FileName(f) ;
	switch( EncodingGet(FileEncoding(f)) ) {
		case octetEncoding: {
			FileError("This operation is not available for binary streams") ;
			break ;
		}
		case asciiEncoding: {
			if( !InRange(c, 0, 127) ) {
				if( FileAllowsNumEscapes(f) ) /* Instead of giving an error... */
					FilePutCharStr(f, CharAsNumericEscape(c)) ;
				else
					FileError("Unable to write non-ascii char (%d) to ascii file '%s'", c, fName) ;
			}
			elif( fputc(c, file) == EOF )
				FileError("Could not write to file '%s'", fName) ;
			break ;
		}
		case latin1Encoding: {
			if( !InRange(c, 0, 255) ) {
				if( FileAllowsNumEscapes(f) ) /* Instead of giving an error... */
				{
					FilePutCharStr(f, CharAsNumericEscape(c)) ;}
				else
					FileError("Unable to write non-latin1 char (%d) to latin1 file '%s'", c, fName) ;
			}
			elif( fputc(c, file) == EOF )
				FileError("Could not write to file '%s'", fName) ;
			break ;
		}
		case utf8Encoding: {
			Utf8FilePut(file, c, fName) ;
			break ;
		}
		case utf16Encoding: {
			FileError("The encoding utf16 is ambiguous") ;
			break ;
		}
		case utf16beEncoding: {
			Utf16FilePut(file, c, fName, true) ;
			break ;
		}
		case utf16leEncoding: {
			Utf16FilePut(file, c, fName, false) ;
			break ;
		}
		case utf32Encoding: {
			FileError("The encoding utf32 is ambiguous") ;
			break ;
		}
		case utf32beEncoding: {
			Utf32FilePut(file, c, fName, true) ;
			break ;
		}
		case utf32leEncoding: {
			Utf32FilePut(file, c, fName, false) ;
			break ;
		}
		case unicodeEncoding: {
			FileError("The encoding unicode is ambiguous") ;
			break ;
		}
		case unicodebeEncoding: {
			Ucs2FilePut(file, c, fName, true) ;
			break ;
		}
		case unicodeleEncoding: {
			Ucs2FilePut(file, c, fName, false) ;
			break ;
		}
		default: {
			FilePutUsingCLib(file, c, FileEncoding(f), fName) ;
			break ;
		}
	}
}

void FilePutCharStr(FilePt f, Str s)
{
	while( *s )
		FilePutChar(f, CharDecode(s)) ;
}


/* TEST, EXTRACT & INIT */


void FilesInit()
{
	WChar c = EOF ;
	if( c > 0 ) InternalError("EOF != -1") ;
	fileType = ExtraTypeNew("FILE", FileSizeFun, nil, nil /* @@@ */, 1) ;
	ExtraTypeDoesNotSupportAliasing(fileType) ;
}
