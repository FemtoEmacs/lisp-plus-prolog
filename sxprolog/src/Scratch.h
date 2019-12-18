/*
 *   This file is part of the CxProlog system

 *   Scratch.h
 *   by A.Miguel Dias - 2001/02/22
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Scratch_
#define _Scratch_

/* The Scratch Pad is a single block of memory, but it is internally
   organized as a stack of memory segments so that is can be used reentrantly.
   The segment at the top is the segment currently active. The
   Scratch Pad grows automatically when needed. */

extern Hdl scratchBegin, scratchEnd, scratchActive, scratchPt ;
extern Hdl saveScratchPt, originalScratch ;

/* SCRATCH HANDLING */

Bool ScratchExpand(void) ;
void ScratchMakeRoom(Size nWords) ;
Size ScratchCapacity(void) ;
Hdl UseScratch(void);
Hdl FreeScratch(void) ;
void ScratchRestart(void) ;
void ScratchInit(void) ;
void ScratchInit2(void) ;


/* SCRATCH SAVE */

#define ScratchSave()			( saveScratchPt = scratchPt )
#define ScratchRestore()		( scratchPt = saveScratchPt )
#define ScratchDistToSaved()	( Df(scratchPt, saveScratchPt) )
#define ScratchRegister()		( originalScratch = scratchBegin )
#define ScratchHasMoved()		( originalScratch != scratchBegin )


/* SCRATCH AS PT-STACK */

#define ScratchCheck()			((scratchPt<scratchEnd) ? true : ScratchExpand())
#define ScratchPush(pt)			( ScratchCheck(), Push(scratchPt, pt) )
#define ScratchPop()			( Pop(scratchPt) )
#define ScratchBack(n)			( Grow(scratchPt, -(n)) )
#define ScratchReset()			( scratchPt = scratchActive )
#define ScratchStart()			( scratchActive )
#define ScratchCurr()			( scratchPt )
#define ScratchTop()			( Top(scratchPt) )
#define ScratchXTop(n)			( XTop(scratchPt, n) )
#define ScratchUsed()			( Df(scratchPt, scratchActive) )


/* SCRATCH AS CODE-BUFFER */

#define Gen0(c)					( ScratchPush(c) )
#define Gen1(c1,c2)				(Gen0(c1), Gen0(c2))
#define Gen2(c1,c2,c3)			(Gen1(c1, c2), Gen0(c3))
#define Gen3(c1,c2,c3,c4)		(Gen2(c1, c2, c3), Gen0(c4))
#define Gen4(c1,c2,c3,c4,c5)	(Gen3(c1, c2, c3, c4), Gen0(c5))

#endif
