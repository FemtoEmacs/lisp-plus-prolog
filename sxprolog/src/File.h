/*
 *   This file is part of the CxProlog system

 *   File.h
 *   by A.Miguel Dias - 2007/01/01
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _File_
#define _File_

typedef struct File *FilePt ;

/* MAIN OPERATIONS */
FilePt FileNew(FILE *file, Str name, int mode, Str encoding, Bool3 bom) ;
void FileSetEncoding(FilePt f, Str encoding) ;
Str FileGetEncoding(FilePt f) ;
Bool FileGetBom(FilePt f) ;
void FileDelete(FilePt f, Bool force) ;
FILE *FileAsFILE(FilePt f) ;
Bool FileIsATty(FilePt f) ;
void FileAllowsNumericEscapes(FilePt f) ;

/* SEQUENTIAL OPERATIONS - only used in Stream.c */
/* sequential read operations */
WChar FileGetByte(FilePt f) ;
WChar FilePeekByte(FilePt f) ;
Size FileGetNBytes(FilePt f, VoidPt v, Size n, Bool isText) ;
WChar FileGetChar(FilePt f) ;
WChar FilePeekChar(FilePt f) ;
/* sequential write operations */
void FileFlush(FilePt f) ;
void FilePutByte(FilePt f, WChar c) ;
Size FilePutNBytes(FilePt f, VoidPt v, Size n, Bool isText) ;
void FilePutChar(FilePt f, WChar c) ;
void FilePutCharStr(FilePt f, Str s) ;

/* TEST, EXTRACT & INIT */
void FilesInit(void) ;

#endif
