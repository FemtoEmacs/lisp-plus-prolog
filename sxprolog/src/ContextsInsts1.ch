/*
 *   This file is part of the CxProlog system

 *   ContextsInsts1.ch
 *   by A.Miguel Dias - 2006/05/07
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2007 A.Miguel Dias, CITI, DI/FCT/UNL

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

/* This file is included in "Instructions.c" */


/*****************************************************************************

*** CONTEXTS 1 : INTRODUCTION

CTX1 is the language described in [L.Monteiro, A.Porto: "Contextual Logic
Programming", in Proc. 6 ICLP, Lisboa, Portugal, MIT Press, 1989].


*** CONTEXTS 1 : CONSTRUCTIONS

	unit u(P).
	visible p/1.
	import p/1 from u(222).
	U>>G
	U<>G
	>G
	<G
	down(G)
	call_on_empty_context(G)


*** CONTEXTS 1 : SEMANTIC RULES

                   H, U.C |- g
(Extension)     ------------------
                  H, C |- U >> g


                  H, U.C |- G
(Local/Sys)     ---------------		g<-G in (local(U)) U system)
                  H, U.C |- g


                  H, F.C |- g
(Import)        ---------------		g<-_ in import(U,F)
                  H, U.C |- g


                    H, F.C |- g
(Switch)        --------------------
                  H, _.C |- F <> g


                   H, C |- g
(CxtDep)        ---------------		g<-_ not-in (local(U)) U system U import(U,F))
                  H, U.C |- g


                  C.H, C |- g
(SaveHist)      ---------------
                  H, C |- > g


                   H, C |- g
(RestoreHist)   ----------------
                  C.H, _ |- < g


                  H, C |- g     H, C |- G
(Conj)          ---------------------------
                       H, C |- (g,G)


                           [], [] |- g
(Empty)         -------------------------------------
                  [], _ |- call_on_empty_context(g)


(GetC)          ----------------------
                  _, C |- context(C)


(GetH)          -----------------------
                  H, _ |- hcontext(H)

Notes:
  Inside each unit, the clauses of each predicate are explored first to last (rule Local/Sys).
  The database predicates act over the "current unit": CurrUnit = top(C).
  These rules ommit the details of unification, for simplicity.

*****************************************************************************/


/* AUXILIARY FUNCTIONS */

static PredicatePt CtxSearchBelow(FunctorPt f) /* for the context instructions */
{
	Pt c = C != tNilAtom ? CtxNext(C) : tNilAtom ;
	for( ; c != tNilAtom ; c = CtxNext(c) ) { /* no need to deref */
		PredicatePt pr = FindPredicateInUnit(CtxTopUnit(c), f) ;
		if( pr != nil && (PredIsVisible(pr) || forceVisibility_flag) ) {
			C = c ;
			return pr ;
		}
	}
	return HandleUndefPredCall(f, C) ;
}

static PredicatePt CtxSearch(FunctorPt f) /* for the context instructions */
{
	Pt c ;
	if( FunctorIsBuiltin(f) ) {
		return FunctorPreds(f) ;
	}
	for( c = C ; c != tNilAtom ; c = CtxNext(c) ) { /* no need to deref */
		PredicatePt pr = FindPredicateInUnit(CtxTopUnit(c), f) ;
		if( pr != nil && (PredIsVisible(pr) || forceVisibility_flag) ) {
			C = c ;
			return pr ;
		}
	}
	return HandleUndefPredCall(f, C) ;
}

/* CONTEXT 1 INSTRUCTIONS */

static void UndefPredInst()
{
	AllocEnv() ;
	CP = InstLoc(UndefPredEnd) ;
	Y(OutPerm(0)) = C ;

	ExecutePred(CtxSearchBelow(LookFunctor())) ;
	JumpNext() ;
}

static void UndefPredEndInst()
{
	C = Y(OutPerm(0)) ;
	Jump(DeallocProceed) ;
}

static void ImportInst()
{
	if( C == tNilAtom )
		Error("Cannot switch top of emtpy context") ;
	AllocEnv() ;
	CP = InstLoc(ImportEnd) ;
	Y(OutPerm(0)) = C ;

	D = ZPushTerm_ConvUnitParams(P[0]) ;
	Q.u = TermToUnit(D, H) ; /* Beware the H */
	PushH(Q.u) ;	/* Unit is ref stored in the global stack */
	PushH(D) ;		/* Link unit-term in the context list */
	if( semanticVariant_flag == 3 )
		PushH(C) ;			/* Extends context */
	else
		PushH(CtxNext(C)) ;	/* Switches top of context */
	C = TagList(H-2) ;
	ExecutePred(CtxSearch(cFunctorPt(P[5]))) ;
	JumpNext() ;
}

static void ImportEndInst()
{
	C = Y(OutPerm(0)) ;
	Jump(DeallocProceed) ;
}

static void CtxSwitchInst()
{
	if( C == tNilAtom )
		Error("Cannot switch top of empty context") ;
	AllocEnv() ;
	CP = InstLoc(ImportEnd) ;
	Y(OutPerm(0)) = C ;

	Q.u = TermToUnit(X0, H) ;
	PushH(Q.u) ;	/* Unit is ref stored in the global stack */
	PushH(X0) ;		/* Link unit-term in the context list */
	PushH(CtxNext(C)) ;
	C = TagList(H-2) ;
	ExecutePred(CtxSearch(PrepareCall(X1))) ;
	JumpNext() ;
}

static void CtxExtensionInst()
{
	AllocEnv() ;
	CP = InstLoc(CtxExtensionEnd) ;

	Q.u = TermToUnit(X0, H) ; /* Beware the H */
	PushH(Q.u) ;	/* Unit is ref stored in the global stack */
	PushH(X0) ;		/* Link unit-term in the context list */
	PushH(C) ;
	C = TagList(H-2) ;
	ExecutePred(CtxSearch(PrepareCall(X1))) ;
	JumpNext() ;
}

static void CtxExtensionEndInst()
{
	C = CtxNext(C) ;
	Jump(DeallocProceed) ;
}

static void CtxEmptyInst()
{
	AllocEnv() ;
	CP = InstLoc(CtxEmptyEnd) ;
	Y(OutPerm(0)) = C ;

	C = tNilAtom ;
	ExecutePred(CtxSearch(PrepareCall(X0))) ;
	JumpNext() ;
}

static void CtxEmptyEndInst()
{
	C = Y(OutPerm(0)) ;
	Jump(DeallocProceed) ;
}

static void CtxDownInst()
{
	if( C == tNilAtom )
		Error("Cannot use 'down/1' on the emtpy context") ;
	AllocEnv() ;
	CP = InstLoc(CtxDownEnd) ;
	Y(OutPerm(0)) = C ;

	C = XListTail(C) ;
	ExecutePred(CtxSearch(PrepareCall(X0))) ;
	JumpNext() ;
}

static void CtxDownEndInst()
{
	C = Y(OutPerm(0)) ;
	Jump(DeallocProceed) ;
}

static void HCtxPushInst()
{
	AllocEnv() ;
	CP = InstLoc(HCtxPushEnd) ;

	PushH(C) ;
	PushH(CH) ;
	CH = TagList(H-2) ;
	ExecutePred(LookupPredicate(PrepareCall(X0))) ; /* ??? FindPredicate @@@ */
	JumpNext() ;
}

static void HCtxPushEndInst()
{
	CH = CtxNext(CH) ;
	Jump(DeallocProceed) ;
}

static void HCtxEnterInst()
{
	if( CH == tNilAtom )
		Error("Empty historic context: cannot enter") ;
	AllocEnv() ;
	CP = InstLoc(HCtxEnterEnd) ;
	Y(OutPerm(0)) = C ;
	Y(OutPerm(1)) = CH ;

	C = CtxTop(CH) ;
	CH = CtxNext(CH) ;
	ExecutePred(CtxSearch(PrepareCall(X0))) ;
	JumpNext() ;
}

static void HCtxEnterEndInst()
{
	C = Y(OutPerm(0)) ;
	CH = Y(OutPerm(1)) ;
	Jump(DeallocProceed) ;
}

static void ContextualEmptyPredInst()
{
	Jump(EmptyPred) ;
}

static void ContextualDynamicEnterInst()
{
	Jump(DynamicEnter) ;
}

static void ContextualDynamicElseInst()
{
	Jump(DynamicElse) ;
}
