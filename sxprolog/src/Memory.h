 /*
 *   This file is part of the CxProlog system

 *   Memory.h
 *   by A.Miguel Dias - 1989/11/14
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Memory_
#define _Memory_

VoidPt AllocateSegment(Size nWords, VoidPt end) ;
void ReleaseSegment(VoidPt mem, Size nWords) ;
char *MemPolicy(void) ;

VoidPt Allocate(Size nWords, Bool saveSize) ;
void Release(VoidPt ptr, Size nWords) ;
VoidPt Reallocate(VoidPt ptr, Size oldSize, Size newSize) ;
Size MemBlockSize(VoidPt ptr) ;
Size ProgramSpaceSize(void) ;
Size ProgramSpaceUsed(void) ;
void MallocStats(void) ;
void MemoryInit(void) ;

#endif
