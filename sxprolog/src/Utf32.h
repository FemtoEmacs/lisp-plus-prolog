/*
 *   This file is part of the CxProlog system

 *   Utf32.h
 *   by A.Miguel Dias - 2008/06/13
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Utf32_
#define _Utf32_

void Utf32Encode(CharPt *s, WChar c, Bool be) ;
WChar Utf32Decode(CharPt *s, Bool be) ;
WChar Utf32DecodeN(CharPt *s, Size n, Bool be) ;

WChar Utf32FileGet(FILE *file, Str fName, Bool be) ;
void Utf32FilePut(FILE *file, WChar c, Str fName, Bool be) ;

#endif
