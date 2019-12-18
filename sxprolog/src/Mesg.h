/*
 *   This file is part of the CxProlog system

 *   Mesg.h
 *   by A.Miguel Dias - 2000/04/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Mesg_
#define _Mesg_

extern Pt catchTerm ;
extern jmp_buf catchJB;
extern Bool catchActive;

#define Catch()	(catchActive = true, (setjmp(catchJB) != 0 ? catchTerm : nil))
#define CatchOff()	(catchActive = false)

Pt ErrorEventPrepareV(Str errKind, Str op, Str variant, Pt culprit,
												Str fmt, va_list v) ;
VoidPt ErrorEventTrigger(Pt exc) ;

void Mesg(Str fmt, ...) ;
void MesgP(Str fmt, ...) ;
void MesgW(Str fmt, ...) ;
void MesgT(Pt t) ;
void BasicInfo(Str fmt, ...) ;
void Info(int level, Str fmt, ...) ;
void MemoryInfo(Str fmt, ...) ;
void MemoryGrowInfo(Str what, Size oldSize, Size newSize) ;
void Warning(Str fmt, ...) ;

VoidPt ExistenceError(Str variant, Pt culprit, Str fmt, ...) ;
VoidPt EvaluationError(Str variant, Str fmt, ...) ;
VoidPt PermissionError(Str op, Str objectType, Pt culprit, Str fmt, ...) ;
VoidPt TypeError(Str expected, Pt culprit) ;
VoidPt TypeError2(Str expected, Pt culprit, Str fmt, ...) ;
int ITypeError(Str expected, Pt culprit) ;
VoidPt DomainError(Str expected, Pt culprit) ;
VoidPt RepresentationError(Str variant, Pt culprit, Str fmt, ...) ;
VoidPt SyntaxError(Str fmt, ...) ;

VoidPt Error(Str fmt, ...) ;
int IError(Str fmt, ...) ;
VoidPt GenericError(Str kind, Str fmt, ...) ;
VoidPt ArithError(Str fmt, ...) ;
VoidPt FileError(Str fmt, ...) ;
VoidPt DatabaseError(Str fmt, ...) ;
VoidPt ImperativeError(Str fmt, ...) ;

VoidPt FatalError(Str fmt, ...) ;
VoidPt InternalError(Str fun) ;
VoidPt UndisclosedError(Str s) ;
int IInternalError(Str fun) ;

void InfoMessagesUpdateFlag(int newValue) ;

void MesgInit(void) ;

#endif
