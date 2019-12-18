/*
 *   This file is part of the CxProlog system

 *   Exception.c
 *   by A.Miguel Dias - 2003/08/20
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

/* PRIVATE FUNCTIONS */

/* http://www.sju.edu/~jhodgson/prolog_proj/exceptions.html */

static Pt currentException = nil ; 
static Pt currentStackTrace = nil ; 

Pt GetExceptionTerm(Pt exc) {
	if( exc == nil )
		exc = currentException ;
	if( exc == nil )
		return nil ;
	exc = Drf(exc) ;
	if( IsThisStruct(exc, errorFunctor) )
		return Drf(XStructArg(exc, 0)) ;
	else
		return exc ;
}

Str GetExceptionString(Pt exc) {
	if( exc == nil )
		exc = currentException ;
	if( exc == nil )
		return nil ;
	exc = Drf(exc) ;
	if( IsThisStruct(exc, errorFunctor) )
		return XTestAtomName(XStructArg(exc, 1)) ;
	else
		return TermAsStr(exc) ;
}

Str GetExceptionMesg(Pt exc) {
	Pt e0, e1, i0, i1, s0, s1 ;
	Str n0 ;
	if( exc == nil )
		exc = currentException ;
	if( (exc = Drf(exc) )
		&& IsThisStruct(exc, errorFunctor)
		&& (e0 = Drf(XStructArg(exc, 0)))
		&& (e1 = Drf(XStructArg(exc, 1)))
		&& IsThisStruct(e1, infoFunctor)
		&& (i0 = Drf(XStructArg(e1, 0)))
		&& (i1 = Drf(XStructArg(e1, 1)))
		&& (n0 = IsAtom(e0) ? XAtomName(e0) :
					IsStruct(e0) ? XStructName(e0) : nil)
		&&	(	(i0 == tEmptyAtom && (s0 = s1 = tNilAtom))
				||
				(IsThisStruct(i0, slashFunctor)
				&& (s0 = Drf(XStructArg(i0, 0)))
				&& (s1 = Drf(XStructArg(i0, 1)))
				&& IsAtom(s0) && IsInt(s1))
			)
		&& IsAtom(i1) ) {
			if( i0 == tEmptyAtom )
				return GStrFormat("%% %s: %s",
							GStrToUpper(n0),
							XAtomName(i1)) ;
			else
				return GStrFormat("%% %s (%s/%d): %s",
							GStrToUpper(n0),
							XAtomName(s0),
							XInt(s1),
							XAtomName(i1)) ;
	}
	else
		return GStrFormat("%% EXCEPTION: %s", TermAsStr(exc)) ;
}

void WriteException(Pt exc)
{
	Pt st = currentStackTrace ;
	if( exc == nil )
		exc = currentException ;
	if( exc == nil )
		InternalError("WriteException") ;

	WriteErr("%s\n", GetExceptionMesg(exc)) ;

#if 0
	if( errno != 0 ) {
		WriteErr("[[[%s]]]\n", sys_errlist[errno]) ;
		errno = 0 ;
	}
#endif

	if( st != nil ) {
		st = Drf(st) ;
		if( IsList(st) ) {
			Pt t ;
			WriteErr(" ### STACK TRACE ###\n") ;
			for( t = st ; IsList(t) ; t = Drf(XListTail(t)) )
				WriteErr("    %s\n", SubtermAsStrQ(XListHead(t), st)) ;
		}
	}
}

Pt BuildExceptionTermV(Str errKind, Str op, Str variant, Pt culprit,
												Str fmt, va_list v)
{
	Pt prTerm, infoTerm, errorTerm ;
	Str mesg = GStrFormatV(fmt, v) ;
	PredicatePt pr = CurrCPred() ;

	if( pr == nil ) prTerm = tEmptyAtom ;
	else prTerm = MakeSlashTerm(ClearNDetermFunctor(PredFunctor(pr))) ;

	infoTerm = MakeBinStruct(infoFunctor, prTerm, MakeTempAtom(mesg)) ;
	if( culprit == nil && variant == nil )
		errorTerm = MakeTempAtom(errKind) ;
	elif( culprit == nil )
		errorTerm = MakeUnStruct(
						LookupFunctorByName(errKind, 1),
						MakeAtom(variant)) ;
	elif( variant == nil )
		errorTerm = MakeUnStruct(
						LookupFunctorByName(errKind, 1),
						culprit) ;
	elif( op == nil )
		errorTerm = MakeBinStruct(
						LookupFunctorByName(errKind, 2),
						MakeAtom(variant), culprit) ;
	else
		errorTerm = MakeTriStruct(
						LookupFunctorByName(errKind, 3),
						MakeAtom(op), MakeAtom(variant), culprit) ;
		
	return MakeBinStruct(errorFunctor, errorTerm, infoTerm) ;
}

Pt ClearExceptionPred(Pt exc)
{
	XStructArg(XStructArg(exc,1),0) = tEmptyAtom ;
	return exc ;
}


#define ExceptionPointSize	5

static void PushExceptionPoint(Pt protected, Pt exceptionOut, Pt handler,
															Pt stackTraceOut)
{
	if( stackTraceOut != nil )		 /* nil means no stack trace wanted */
		TestVar(stackTraceOut) ;
	X0 = protected ;
	X1 = exceptionOut ;
	X2 = handler ;
	X3 = stackTraceOut ;
	X4 = cPt(P) ;					/* Save P */
	CreateChoicePoint(&DiscardAndFail, ExceptionPointSize) ;
	R = B ;							/* Set first in exception chain */
}

static void PopExceptionPoint(void)
{
	CP = ChoicePointField(R, CP) ;		/* Restore CP */
	P = cHdl(ChoicePointArg(R, 4)) ;	/* Restore P */
	if( B == R ) Discard() ;			/* If deterministic, discard */
	R = ChoicePointField(R, R) ;		/* Pop exception point */
}

void Throw(Pt exc)
{
	Pt t ;
	exc = Drf(exc) ;
	if( exc == tVarAtom )
		exc = MakeVar() ;
	elif( IsVar(exc) )
		TypeError("NON-FREE", exc) ;

/* Part of global stack will be destroyed, so exc and st must be saved */
	if( exc != currentException ) {
		if( currentException != nil )
			ReleaseTerm(currentException) ;
		currentException = AllocateTermForAssign(exc) ;
	
		if( currentStackTrace != nil ) {
			ReleaseTerm(currentStackTrace) ;
			currentStackTrace = nil ;
		}
		if( debug_flag != 0 )
			currentStackTrace = AllocateTermForAssign(BuildStackTrace()) ;
	}
/* Search for handler */
	while( R < L ) {
		CutTo(R) ;
		RestoreState(ExceptionPointSize-1) ;/* yes, one less, so not to change P */
		Discard() ;							/* Restore original "catch-time" choicepoint */
		if( Unifiable(currentException, X1) ) goto found ;
	}
/* HANDLER NOT FOUND  */
	EventException() ;

found: /* HANDLER FOUND */
	if( X3 != nil && currentStackTrace != nil )	{	/* Is stack trace wanted? */
		t = ZPushTerm(currentStackTrace) ;
		Ensure( Unify(t, X3) ) ;
	}
	DebugThrow() ;
	t = ZPushTerm(currentException) ;
	if( !Unify(t, X1) ) InternalError("Throw (2)") ;
	X0 = X2 ;					/* Prepare to execute Handler */
	P = &ExecuteVar ;
	EventContinue() ;
}

void Rethrow(void)
{
	Throw(currentException) ;
	InternalError("Rethrow") ;
}

#if COMPASS
void ThrowPrologException(Pt exc)
{
	Throw(exc) ;
}

void ThrowPrologExceptionMesgV(Str kind, Str fmt, va_list v)
{
	Throw(BuildExceptionTermV(kind, nil, nil, nil, fmt, v)) ;
}

void ThrowPrologExceptionMesg(Str kind, Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = BuildExceptionTermV(kind, nil, nil, nil, fmt, v) ;
    va_end(v) ;
	Throw(exc) ;
}
#endif


/* CXPROLOG C'BUILTINS */

static void CatchPopAux()
{
	PopExceptionPoint() ;
	JumpNext() ;
}
static Pt CatchCode[2] =
{
	cPt(WordsOf(Environment)),	/* Anyway, doesn't matter because B is on top */
	cPt(CatchPopAux)
} ;

static void PCatch()
{	/* X0 = ProtectedGoal, X1 = ExceptionTerm, X2 = StackTrace, X3 = Handler */
	PushExceptionPoint(X0, X1, X3, X2) ;
	CP = CatchCode + 1 ;
	Jump(ExecuteVar) ;			/* Execute ProtectedGoal */
}

static void PCatch3()
{	/* X0 = ProtectedGoal, X1 = ExceptionTerm, X2 = Handler */
	PushExceptionPoint(X0, X1, X2, nil) ;
	CP = CatchCode + 1 ;
	Jump(ExecuteVar) ;			/* Execute ProtectedGoal */
}

static void POnException()
{	/* X0 = ExceptionTerm, X1 = ProtectedGoal, X2 = Handler */
	PushExceptionPoint(X1, X0, X2, nil) ;
	CP = CatchCode + 1 ;
	Jump(ExecuteVar) ;			/* Execute ProtectedGoal */
}

static void PThrow()
{
	Throw(X0) ;
	InternalError("PThrow") ;
}

static void PRaiseException()
{
	Throw(X0) ;
	InternalError("PRaiseException") ;
}

static void PWriteException()
{
	WriteException(X0) ;
	JumpNext() ;
}


/* INIT */

void ExceptionsInit()
{
	InstallCBuiltinPred("catch", 3, PCatch3) ;
	InstallCBuiltinPred("catch", 4, PCatch) ;
	InstallCBuiltinPred("on_exception", 3, POnException) ;
	InstallCBuiltinPred("throw", 1, PThrow) ;
	InstallCBuiltinPred("raise_exception", 1, PRaiseException) ;
	InstallCBuiltinPred("write_exception", 1, PWriteException) ;
}
