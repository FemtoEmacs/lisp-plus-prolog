/*
 *   This file is part of the CxProlog system

 *   Scribbling.h
 *   by A.Miguel Dias - 20016/06/05
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Scribbling_
#define _Scribbling_

/* The Scribblings are contiguous blocks of memory that grow automatically. */

/* Scribbling */

#define UseScribbling()			( scribblingPt = scribblingBegin )
#define FreeScribbling()		( scribblingBegin )

#define ScribblingCheck()		((scribblingPt<scribblingEnd) ? true : ScribblingExpand())
#define ScribblingPush(pt)		( ScribblingCheck(), Push(scribblingPt, pt) )
#define ScribblingPop()			( Pop(scribblingPt) )
#define ScribblingTop()			( Top(scribblingPt) )
#define ScribblingXTop(n)		( XTop(scribblingPt, n) )
#define ScribblingUsed()		( Df(scribblingPt, scribblingBegin) )
#define ScribblingStart()		( scribblingBegin )

extern Hdl scribblingBegin, scribblingEnd, scribblingPt ;

Bool ScribblingExpand(void) ;
void ScribblingMakeRoom(Size nWords) ;
Size ScribblingCapacity(void) ;
void ScribblingRestart(void) ;
void ScribblingShow(FunV f) ;
void ScribblingDiscardTop(VoidPt v) ;


/* Scribbling 2 */

#define UseScribbling2()	( scribbling2Pt = scribbling2Begin )
#define FreeScribbling2()	( scribbling2Begin )

#define Scribbling2Check()	((scribbling2Pt<scribbling2End) ? true : Scribbling2Expand())
#define Scribbling2Push(pt)	( Scribbling2Check(), Push(scribbling2Pt, pt) )
#define Scribbling2Pop()	( Pop(scribbling2Pt) )
#define Scribbling2Top()	( Top(scribbling2Pt) )
#define Scribbling2XTop(n)	( XTop(scribbling2Pt, n) )
#define Scribbling2Used()	( Df(scribbling2Pt, scribbling2Begin) )
#define Scribbling2Start()	( scribbling2Begin )

extern Hdl scribbling2Begin, scribbling2End, scribbling2Pt ;

Bool Scribbling2Expand(void) ;
void Scribbling2MakeRoom(Size nWords) ;
Size Scribbling2Capacity(void) ;
void Scribbling2Restart(void) ;
void Scribbling2Show(FunV f) ;
void Scribbling2DiscardTop(VoidPt v) ;


/* Common */

void ScribblingInit(void) ;

#endif
