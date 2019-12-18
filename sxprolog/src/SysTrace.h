/*
 *   This file is part of the CxProlog system

 *   SysTrace.h
 *   by A.Miguel Dias - 2003/08/06
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _SysTrace_
#define _SysTrace_

void SysTraceMachineRun(void) ;
void SysTraceUpdateFlag(int newValue) ;
void SysTraceHandle(PredicatePt pr) ;
void SysTraceWrite(Str s) ;
void SysTraceInit(void) ;

#endif
