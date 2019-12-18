/*
 *   This file is part of the CxProlog system

 *   CallProlog.h
 *   by A.Miguel Dias -  2003/06/20
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _CallProlog_
#define _CallProlog_


typedef enum {
	peNone = 0,

	peSucc, peFailure,
	peException, peInterrupt,
	peAbort, peExit, peHalt,
	peFatalError, peThreadError,
	peOtherError,

	peContinue, peForceFail, peMoreSuccess,
	peReturnSuccess, peReturnFailure
} PrologEvent ;

#define CallLevel()					(callLevel)
#define HasATopLevel()				(hasATopLevel)
#define NormalFinish()				(normalFinish)

extern int callLevel ;
extern Bool hasATopLevel ;
extern Bool normalFinish ;

Str PrologEventAsStr(PrologEvent ev) ;

Bool InMainThread(Bool warn) ;

void SendPrologEvent(PrologEvent ev) ;
void EventContinue(void) ;
void EventForceFail(void) ;
void EventException(void) ;
void EventInterrupt(void) ;
void EventAbort(void) ;
void EventExit(void) ;
void EventHalt(void) ;
void EventFatalError(void) ;

Pt ZTermFromStrThruProlog(Str goal) ;
PrologEvent CallFunThruProlog(Fun fun) ;

PrologEvent CallProlog(Pt goal) ;
PrologEvent CallPrologThrow(Pt goal) ;
void CallPrologTransparent(Pt goal) ;
PrologEvent CallPrologAtom(Str atom) ;
PrologEvent CallPrologStr(Str goal) ;
PrologEvent CallPrologStrTop(Str goal, Str *vars, Str *out) ;
PrologEvent CallPrologSerialized(Str fmt, ...) ;
void CallPrologSetResult(Pt t) ;
Pt CallPrologGetResult(void) ;

PrologEvent CallPrologMulti(Pt goal) ;
PrologEvent CallPrologMultiNext(void) ;
PrologEvent CallPrologMultiStop(void) ;
PrologEvent CallPrologMultiAtom(Str atom) ;
PrologEvent CallPrologMultiStr(Str goal) ;

PrologEvent CallPrologMultiStrTop(Str goal, Str *vars, Str *out) ;
PrologEvent CallPrologMultiNextTop(Str *vars, Str *out) ;
PrologEvent CallPrologMultiStopTop(Str *vars, Str *out) ;

PrologEvent StartProlog(int argc, CharHdl argv, Fun yourExt) ;
void StopProlog(void) ;
int RunInteractiveProlog(int argc, CharHdl argv) ;
void RefreshProlog(void) ;
void CallPrologInit(void) ;

#endif
