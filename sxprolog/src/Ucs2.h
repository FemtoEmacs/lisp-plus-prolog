/*
 *   This file is part of the CxProlog system

 *   Ucs2.h
 *   by A.Miguel Dias - 2008/06/16
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Ucs2_
#define _Ucs2_

void Ucs2Encode(CharPt *s, WChar c, Bool be) ;
WChar Ucs2Decode(CharPt *s, Bool be) ;
WChar Ucs2DecodeN(CharPt *s, Size n, Bool be) ;

WChar Ucs2FileGet(FILE *file, Str fName, Bool be) ;
void Ucs2FilePut(FILE *file, WChar c, Str fName, Bool be) ;

#endif
