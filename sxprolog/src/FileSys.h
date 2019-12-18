/*
 *   This file is part of the CxProlog system

 *   FileSys.h
 *   by A.Miguel Dias - 2002/01/12
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _FileSys_
#define _FileSys_

Pt PathNameListFromAtom(AtomPt s) ;
AtomPt PathNameListToAtom(Pt list) ;
Bool IsAbsoluteFileName(Str s) ;
AtomPt AbsoluteFileName(Str s) ;
Str FileNameInternalize(CharPt s) ;
Str FileNameExternalize(Str s, Bool sepNotAllowedAtEnd) ;
void EnsureNativeFileName(CharPt s) ;
void EnsureIndependentFileName(CharPt s) ;
Str GetFileNameLastComponent(Str s) ;
Str ProcessFileName(Str s) ;
void GoHome(Bool handleError) ;
Str CurrDirPath(void) ;
void DeleteFilesWithExtension(Str path, Str ext) ;
Str WithAppDirPath(Str path, Str name, Str extension) ;
Str WithHomeDirPath(Str path, Str name, Str extension) ;
Str WithPrefixDirPath(Str path, Str name, Str extension) ;
Str WithCurrentDirPath(Str name, Str extension) ;
Str WithLibDirPath(Str name, Str extension) ;
Str WithPreferencesDirPath(Str name, Str extension) ;
Str WithCacheDirPath(Str name, Str extension) ;
Str WithTmpDirPath(Str name, Str extension) ;
void FSDelete(Str fname) ;
void FileSysInit(void) ;

#endif
