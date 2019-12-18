/*
 *   This file is part of the CxProlog system

 *   Exception.h
 *   by A.Miguel Dias - 2003/08/20
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Exception_
#define _Exception_

/* MAIN OPERATIONS */
Pt BuildExceptionTermV(Str errKind, Str op, Str variant, Pt culprit,
												Str fmt, va_list v) ;
Pt ClearExceptionPred(Pt exc) ;
Pt GetExceptionTerm(Pt exc) ;
Str GetExceptionString(Pt exc) ;
Str GetExceptionMesg(Pt exc) ;
void WriteException(Pt exc) ;
void Throw(Pt exc) ;
void Rethrow(void) ;

#if COMPASS
void ThrowPrologException(Pt exc) ;
void ThrowPrologExceptionMesgV(Str kind, Str fmt, va_list v) ;
void ThrowPrologExceptionMesg(Str kind, Str fmt, ...) ;
#endif

/* INIT */
void ExceptionsInit(void) ;

#endif
