/*
 *   This file is part of the CxProlog system

 *   Utf16.h
 *   by A.Miguel Dias - 2008/06/13
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Utf16_
#define _Utf16_

void Utf16Encode(CharPt *s, WChar c, Bool be) ;
WChar Utf16Decode(CharPt *s, Bool be) ;
WChar Utf16DecodeN(CharPt *s, Size n, Bool be) ;

WChar Utf16FileGet(FILE *file, Str fName, Bool be) ;
void Utf16FilePut(FILE *file, WChar c, Str fName, Bool be) ;

#endif
