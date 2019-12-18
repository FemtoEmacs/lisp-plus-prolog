/*
 *   This file is part of the CxProlog system

 *   Contexts1.c
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

#include "CxProlog.h"


#if CONTEXTS == 1


/* PUBLIC FUNCTIONS */

PredicatePt LookupPredicateForMetaCall(FunctorPt f)
{
/* The debugger requires a real descriptor to be created... */
	return LookupPredicate(f) ;
}

Bool DebugCallOverride(PredicatePt pr)
{
	Unused(pr) ;
	return false ;
}

/* AUXILIARY FUNCTIONS */

static void SetVisible(PredicatePt pr)
{
	if( PredName(pr)[0] == '$' )
		DatabaseError("Cannot make '%s' visible because the name "
						"starts with \'$\'", PredNameArity(pr)) ;

	ChangingCurrUnit("declaring visible", pr) ;

	if( PredIsBuiltin(pr) )
		DatabaseError("Cannot set '%s' visible because it is a built-in",
												PredNameArity(pr)) ;
	elif( PredHandleConsult(pr, !PredIsVisible(pr)) )
		/* nothing */ ;
	elif( !PredIsVisible(pr) && PredHasClauses(pr) )
		DatabaseError("Cannot set '%s' visible because it already has clauses",
												PredNameArity(pr)) ;
	PredIsVisible(pr) = true ;
	if( PredIsUndefined(pr) ) {
		BindPredicateAsEmpty(pr) ;
		PredWasAbolished(pr) = false ;
	}
}

static void SetImport(PredicatePt pr, Pt unit) /* pre: TermIsGround(unit) */
{
	ChangingCurrUnit("importing", pr) ;

	if( PredIsBuiltin(pr) )
		DatabaseError("Cannot import '%s' because it is a built-in",
												PredNameArity(pr)) ;
	elif( PredHandleConsult(pr, !PredIsImported(pr)) )
		/* nothing */ ;
	elif( PredIsDynamic(pr) )
		DatabaseError("Cannot import '%s' because it is already dynamic",
												PredNameArity(pr)) ;
	elif( PredIsMultifile(pr) )
		DatabaseError("Cannot import '%s' because it is already multifile",
												PredNameArity(pr)) ;
	elif( PredIsImported(pr) && Compare(GetImportTerm(pr), unit) )
		DatabaseError("Cannot import '%s' from two different unit specifiers",
												PredNameArity(pr)) ;
	elif( PredHasClauses(pr) )
		DatabaseError("Cannot import '%s' because it already has clauses",
												PredNameArity(pr)) ;
	PredNIndexable(pr) = 0 ;
	BindPredicate(pr, Import, unit) ;
	PredWasAbolished(pr) = false ;
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

static Size PVisibleAux(FunctorPt f)
{
	SetVisible(LookupPredicate(f)) ;
	return 1 ;
}
static void PVisible()
{
	ForEachInSpec(X0, PVisibleAux, false) ;
	JumpNext() ;
}

static Pt importUnitAux ;
static Size PImportAux(FunctorPt f) /* Import unit in importUnitAux */
{
	SetImport(LookupPredicate(f), importUnitAux) ;
	return 1 ;
}
static void PImport()
{
	if( !TermIsGround(X1) )
		DatabaseError("Import declaration from non-ground unit specifier '%s'",
												TermAsStr(X1)) ;
	importUnitAux = AllocateTermForAssert(X1) ;
	ForEachInSpec(X0, PImportAux, false) ;
	JumpNext() ;
}

static void PContexts()
{
	MustBe( UnifyWithNumber(X0, oneIntPt) ) ;
}

void ContextsInit()	/* Forces NoCurrUnit() */
{
	C = tNilAtom ;
}

void ContextsInit2()
{
	ResetContextOperators() ;
	InstallCBuiltinPred("visible", 1, PVisible) ;
	InstallCBuiltinPred("import", 2, PImport) ;
	InstallCBuiltinPred("$$_ctx_ext", 2, InstDecode(CtxExtension)) ;
	InstallCBuiltinPred(">>", 2, InstDecode(CtxExtension)) ;
	InstallCBuiltinPred("call_on_empty_context", 1, InstDecode(CtxEmpty)) ;
	InstallCBuiltinPred("down", 1, InstDecode(CtxDown)) ;
	InstallCBuiltinPred("<>", 2, InstDecode(CtxSwitch)) ;
	InstallCBuiltinPred(">", 1, InstDecode(HCtxPush)) ;
	InstallCBuiltinPred("<", 1, InstDecode(HCtxEnter)) ;

	InstallCBuiltinPred("gopen", 1, PReserved) ;
	InstallCBuiltinPred("gclose", 1, PReserved) ;

	InstallCBuiltinPred("$$_contexts", 1, PContexts) ;
}

#endif
