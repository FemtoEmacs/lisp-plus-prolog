/*
 *   This file is part of the CxProlog system

 *   Thread.h
 *   by A.Miguel Dias - 1993/07/15
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Thread_
#define _Thread_

typedef struct Thread *ThreadPt ;
typedef struct Thread *PrologInstancePt ;

#define endOfChainMark			tNilAtom
#define IsEndOfChain(pt)		(*cHdl(pt) == endOfChainMark)

void ThreadRootCreate(Pt startGoal, Pt restartGoal) ;
void ActiveThreadReplace(Pt startGoal, Pt restartGoal) ;
void ActiveThreadReset(void) ;
void ActiveThreadStart(void) ;
void ActiveThreadRestart(void) ;
void ThreadSwitchToRoot(void) ;

void ThreadConcurrencyHandle(void) ;
void ThreadConcurrencyEnable(void) ;
void ThreadConcurrencyDisable(void) ;

void ThreadRunABit(ThreadPt th) ;
void ThreadRunABitByAlias(Str alias) ;
void ThreadInputLine(ThreadPt th, Str line) ;
void ThreadInputLineByAlias(Str alias, Str line) ;
Str ThreadOutputText(ThreadPt th) ;
Str ThreadOutputTextByAlias(Str alias);
void ThreadInputGetCharReset(void);
WChar ThreadInputGetChar(void);
WChar ThreadInputPeekChar(void);

Bool ThreadWaitingInput(ThreadPt th) ;
Bool ThreadWaitingInputByAlias(Str alias) ;

void StatisticsShow(void) ;
void ThreadsInit(void) ;

#endif
