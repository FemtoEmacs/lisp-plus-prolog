/*
 *   This file is part of the CxProlog system

 *   Contexts2.c
 *   by A.Miguel Dias - 2006/05/07
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

/* ** EXPERIMENTAL ** */

#include "CxProlog.h"

#if CONTEXTS == 2


/* PUBLIC FUNCTIONS */

PredicatePt LookupPredicateForMetaCall(FunctorPt f)
{
	if( FunctorIsBuiltin(f) )	/* Does not change CH */
		return FunctorPreds(f) ;
	if( CH == tNilAtom || CtxTop(CH) == tNilAtom )
		return LookupPredicateInUnit(bottomUnit, f) ;
	return LookupPredicate(f) ;
}

Bool DebugCallOverride(PredicatePt pr)
{
	static int i = 1 ;
	CharPt s ;
	UChar c ;
	if( PredIsBuiltin(pr) )
	return true ;
	c = PredIsUndefined(pr) ? 'u'
	  : PredIsVisible(pr) ? 'v'
	  : PredIsImported(pr) ? 'i'
	  : PredIsBuiltin(pr) ? 'b'
	  : 'l' ;
	WriteStd("   -------- %s\n", TermAsStr(CH)) ;
	WriteStd("   %c (%4ld) %s: ", c, i++, TermAsStr(C)) ;
	HSave() ;
	s = TermAsStr(MakeStruct(PredFunctor(pr), X)) ;
	HRestore() ;
	WriteStd("%s", s) ;
	WriteStd(" ? ") ;
	for(;;)
		switch( InterLineGetCommand(nil, nil) ) {
			case '\n': {					/* creep */
				return true ;
			}
			case 'a': {					/* abort */
				WriteStd("%% Aborted.\n") ;
				EventAbort() ;
				return true ;
			}
			case EOF: {
				WriteEOF() ;
				/* fall through */
			}
			case 'e': {					/* exit */
				WriteStd("%% Bye.\n") ;
				EventExit() ;
				return true ;
			}
			default: {
				WriteStd("%12sOptions: <ret>-creep  a-abort  e-exit ? ", "") ;
				break ;
			}
		}
}


/* AUXILIARY FUNCTIONS */

static void SetVisibleProperty(PredicatePt pr, Bool open)
{
	PredIsVisible(pr) = true ;
	PredIsGOpen(pr) = open ;
	PredNIndexable(pr) = 0 ;	/* @@ WILL CHANGE */
	if( !PredHasDynamicChain(pr) ) {
		PredHasDynamicChain(pr) = true ;
		PredIsLogical(pr) = true ;
	}
}

static void SetVisible(PredicatePt pr, Bool open)
{
	ChangingCurrUnit("declaring visible", pr) ;

	if( PredIsBuiltin(pr) )
		DatabaseError("Cannot set '%s' visible because it is a built-in",
												PredNameArity(pr)) ;
	elif( PredHandleConsult(pr, !PredIsVisible(pr) || PredIsGOpen(pr) != open) )
		/* nothing */ ;
	elif( PredIsImported(pr) )	/* cannot happen in ctx2 */
		DatabaseError("Cannot set '%s' visible because it is an import link",
												PredNameArity(pr)) ;
	elif( !PredIsVisible(pr) && PredHasClauses(pr) )
		DatabaseError("Cannot set '%s' visible because it already has clauses",
												PredNameArity(pr)) ;
	elif( PredIsVisible(pr) && PredIsGOpen(pr) != open )
		DatabaseError("Cannot change gopen/gclose status of visible predicate '%s'",
												PredNameArity(pr)) ;
	SetVisibleProperty(pr, open) ;
	if( PredIsUndefined(pr) ) {
		BindPredicateAsEmpty(pr) ;
		PredWasAbolished(pr) = false ;
	}
}

static void ResetContextOperators(void)
{
/* xfx -- infix non associative */
	MakeInfixOperator("from",		1050, false, false) ;		/* CX import . from . */
/* xfy -- infix right associative */
	MakeInfixOperator(">>",			 400, false, true) ;		/* violates ISO */
/* yfx -- infix left associative */
	/* Nothing */
/* fx -- prefix non associative */
	MakePrefixOperator("visible",	1150, false) ;
	MakePrefixOperator("xnew",		1150, false) ;
	MakePrefixOperator("xover",		1150, false) ;
	MakePrefixOperator("gopen",		1150, false) ;
	MakePrefixOperator("gclose",	1150, false) ;
	MakePrefixOperator("hidden",	1150, false) ;
	MakePrefixOperator("import",	1150, false) ;	/* CX import . from . */
	MakePrefixOperator("unit",		 900, false) ;	/* CX */
/* fy -- prefix associative */
	MakePrefixOperator("down",		 900, true) ;
	MakePrefixOperator(">",			 700, true) ;
	MakePrefixOperator("<",			 700, true) ;
/* xf -- postfix non associative */
	/* Nothing */
}



/* CXPROLOG C'BUILTINS */

static void PReserved()
{
	Error("Reserved predicate name") ;
}

static Size PGOpenAux(FunctorPt f)
{
	SetVisible(LookupPredicate(f), true) ;
	return 1 ;
}
static void PGOpen()
{
	ForEachInSpec(X0, PGOpenAux, false) ;
	JumpNext() ;
}

static Size PGCloseAux(FunctorPt f)
{
	SetVisible(LookupPredicate(f), false) ;
	return 1 ;
}
static void PGClose()
{
	ForEachInSpec(X0, PGCloseAux, false) ;
	JumpNext() ;
}


static void PContexts()
{
	MustBe( UnifyWithNumber(X0, twoIntPt) ) ;
}

void ContextsInit()	/* Forces NoCurrUnit() */
{
	C = tNilAtom ;
	CH = tNilAtom ;
}

void ContextsInit2()
{
	ResetContextOperators() ;
	InstallCBuiltinPred("gopen", 1, PGOpen) ;
	InstallCBuiltinPred("gclose", 1, PGClose) ;
	InstallCBuiltinPred("$$_ctx_ext", 2, InstDecode(CtxExtension)) ;
	InstallCBuiltinPred(">>", 2, InstDecode(CtxExtension)) ;
	InstallCBuiltinPred("call_on_empty_context", 1, InstDecode(CtxEmpty)) ;
	InstallCBuiltinPred("callb", 1, InstDecode(CtxDown)) ;

	InstallCBuiltinPred("visible", 1, PReserved) ;
	InstallCBuiltinPred("import", 2, PReserved) ;

	InstallCBuiltinPred("$$_contexts", 1, PContexts) ;
}

#endif
