/*
 *   This file is part of the CxProlog system

 *   CmdLine.h
 *   by A.Miguel Dias - 2002/01/19
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _CmdLine_
#define _CmdLine_

void SpecifyBootFile(Str boot) ;
void CmdLineInit(int ac, CharHdl av) ;
CharPt CmdLineArg(Str sw) ;
Pt ZOSGetArgs(void) ;
void CmdLineInit2(void) ;

#endif
