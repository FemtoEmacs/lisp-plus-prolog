/*
 *   This file is part of the CxProlog system

 *   Stack.c
 *   by A.Miguel Dias - 2000/11/30
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL

 *   CxProlog is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.

 *   CxProlog is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "CxProlog.h"

/* STACK */

typedef struct Stack {
	ExtraDef(Stack) ;
	Hdl begin, end, first, last ;
} Stack ;

#define cStackPt(x)				((StackPt)(x))

#define StackBegin(s)			((s)->begin)
#define StackEnd(s)				((s)->end)
#define StackFirst(s)			((s)->first)
#define StackLast(s)			((s)->last)

static ExtraTypePt stackType ;


/* PRIVATE FUNCTIONS */

#define StackWords(capacity)		(capacity)

static Size StackCapacity(StackPt s)
{
	return StackEnd(s) - StackBegin(s) ;
}

static void StackInit(StackPt s, Size capacity)
{
	StackBegin(s) = Allocate(StackWords(capacity), false) ;
	StackEnd(s) = StackBegin(s) + capacity ;
	StackFirst(s) = StackBegin(s) ;
	StackLast(s) = StackBegin(s) ;
}

static void StackExpand(StackPt s)
{
	Hdl b = StackBegin(s) ;
	Hdl f = StackFirst(s) ;
	Hdl l = StackLast(s) ;
	Hdl h ;
	Size oldCapacity = StackCapacity(s) ;
	StackInit(s, 2 * oldCapacity) ;
	for( h = StackBegin(s) ; f < l ; *h++ = *f++ ) ;
	StackLast(s) = h ;
	Release(b, StackWords(oldCapacity)) ;
}


static void StackDisable(StackPt s)
{
	if( ExtraDisable(s) ) {
		StackClear(s) ;
		Release(StackBegin(s), StackWords(StackCapacity(s))) ;
	}
}

static void StackPark(StackPt s)
{
	Hdl f, b ;
	for( b = StackBegin(s), f = StackFirst(s) ; f < StackLast(s) ; *b++ = *f++ ) ;
	StackFirst(s) = StackBegin(s) ;
	StackLast(s) = b ;
}

static void StackWrite(StreamPt stm, StackPt s)
{
	Hdl h ;
	StreamWrite(stm, "%s", ExtraAsStr(s)) ;
	StreamWrite(stm, "     (current capacity %ld)\n", StackCapacity(s)) ;
	for( h = StackFirst(s) ; h < StackLast(s) ; h++ )
		StreamWrite(stm, "\t%s\n", TermAsStr(*h)) ;
}

static Size StackSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(Stack) ;
}

static void StacksBasicGCMarkContents(VoidPt x)
{
	StackPt s = cStackPt(x) ;
	register Hdl h ;
	for( h = StackFirst(s) ; h < StackLast(s) ; h++ )
		TermBasicGCMark(*h) ;
}

static Bool StackBasicGCDelete(VoidPt x)
{
	StackPt s = cStackPt(x) ;
	StackDisable(s) ;
	return true ;
}


/* MAIN OPERATIONS */

Size StackSize(StackPt s)
{
	return StackLast(s) - StackFirst(s) ;
}

StackPt StackNew()
{
	StackPt s = ExtraNew(stackType, 0) ;
	StackInit(s, 4) ;
	return s ;
}

void StackClear(StackPt s)
{
	register Hdl h ;
	for( h = StackFirst(s) ; h < StackLast(s) ; h++ )
		ReleaseTerm(*h) ;
	StackLast(s) = StackFirst(s) ;
}

void StackFilter(StackPt s, BFunVV filter, VoidPt x)
{
	Hdl a, z, l = StackLast(s) ;
	for( a = z = StackFirst(s) ; a < l ; a++ )
		if( filter(*a, x) )
			*z++ = *a ;
		else ReleaseTerm(*a) ;
	StackLast(s) = z ;
}

Bool StackTop(StackPt s, Hdl t)
{
	if( StackFirst(s) < StackLast(s) ) {
		*t = *(StackLast(s)-1) ;
		return true ;
	}
	else return false ;
}

Bool StackFromTop(StackPt s, int idx, Hdl t)
{
	if( StackFirst(s) < StackLast(s) - idx ) {
		*t = *(StackLast(s)-idx-1) ;
		return true ;
	}
	else return false ;
}

void StackPush(StackPt s, Pt t)
{
	if( StackLast(s) == StackEnd(s) ) {
		if( StackFirst(s) != StackBegin(s) )
			StackPark(s) ;
		else
			StackExpand(s) ;
	}
	*StackLast(s)++ = AllocateTermForAssign(t) ;
}

Bool StackPop(StackPt s)
{
	if( StackFirst(s) < StackLast(s) ) {
		ReleaseTerm(*--StackLast(s)) ;
		return true ;
	}
	else return false ;
}


/* CXPROLOG C'BUILTINS */

static void PStackCheck()
{
	MustBe( XExtraCheck(stackType, X0) ) ;
}

static void PStackNew()
{
	BindVarWithExtra(X0, StackNew()) ;
	JumpNext() ;
}

static void PStackClear()
{
	StackClear(XTestExtra(stackType,X0)) ;
	JumpNext() ;
}

static void PStackDelete()
{
	StackPt s = XTestExtra(stackType,X0) ;
	StackDisable(s) ;
	JumpNext() ;
}

static void PStackPush()
{
	StackPush(XTestExtra(stackType,X0), X1) ;
	JumpNext() ;
}

static void PStackPop()
{
	Pt t ;
	StackPt s = XTestExtra(stackType,X0) ;
	Ensure( StackTop(s, &t) ) ;
	t = ZPushTerm(t) ; /* stacks may grow */
	if( !StackPop(s) )
		InternalError("PStackPop") ;
	MustBe( Unify(X1, t) ) ;

}

static void PStackTop()
{
	Pt t ;
	Ensure( StackTop(XTestExtra(stackType,X0), &t) ) ;
	t = ZPushTerm(t) ; /* stacks may grow */
	MustBe( Unify(X1, t) ) ;
}

static void PStackAsList()
{
	StackPt s = XTestExtra(stackType,X0) ;
	Hdl h ;
	Z.t = tNilAtom ;
	for( h = StackFirst(s) ; h < StackLast(s) ; h++ ) {
		Pt t = ZPushTerm(*h) ; /* stacks may grow */
		Z.t = MakeList(t, Z.t) ;
	}
	MustBe( Unify(Z.t, X1) ) ;
}

static void PStackWrite()
{
	StackWrite(currOut, XTestExtra(stackType,X0)) ;
	JumpNext() ;
}

static void PSStackWrite()
{
	StackWrite(XTestStream(X0, mWrite), XTestExtra(stackType,X1)) ;
	JumpNext() ;
}

static void PNDCurrentStack()
{
	ExtraPNDCurrent(stackType, nil, 1, 0) ;
	JumpNext() ;
}

static Size StacksAux(CVoidPt x)
{
	StackPt s = cStackPt(x) ;
	Write("size(%ld), capacity(%ld)", StackSize(s), StackCapacity(s)) ;
	return 1 ;
}
static void PStacks()
{
	ExtraShow(stackType, StacksAux) ;
	JumpNext() ;
}


/* TEST, EXTRACT & INIT */

Bool IsStack(Pt t)
{
	return IsThisExtra(stackType, t) ;
}

StackPt XTestStack(Pt t)
{
	return XTestExtra(stackType, t) ;
}

void StacksInit()
{
	stackType = ExtraTypeNew("STACK", StackSizeFun, StacksBasicGCMarkContents, StackBasicGCDelete, 1) ;
	/* add "stacks." to CxProlog.c/PShow */

	InstallCBuiltinPred("stack", 1, PStackCheck) ;
	InstallCBuiltinPred("stack_new", 1, PStackNew) ;
	InstallCBuiltinPred("stack_clear", 1, PStackClear) ;
	InstallCBuiltinPred("stack_delete", 1, PStackDelete) ;
	InstallCBuiltinPred("stack_push", 2, PStackPush) ;
	InstallCBuiltinPred("stack_pop", 2, PStackPop) ;
	InstallCBuiltinPred("stack_top", 2, PStackTop) ;
	InstallCBuiltinPred("stack_as_list", 2, PStackAsList) ;
	InstallCBuiltinPred("stack_write", 1, PStackWrite) ;
	InstallCBuiltinPred("stack_write", 2, PSStackWrite) ;
	InstallGNDeterCBuiltinPred("current_stack", 1, 2, PNDCurrentStack) ;
	InstallCBuiltinPred("stacks", 0, PStacks) ;
}
