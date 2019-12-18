/*
 *   This file is part of the CxProlog system

 *   Instructions.c
 *   by A.Miguel Dias - 2002/03/28
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

#if COMPASS
#define OLD_TRANSPARENT_CUT		1
#endif

/* STACKS CONTROL */

#define PushHVar()			( ResetVar(H), cPt(H++) )
#define PushH(v)			Push(H, v)
#define PopH()				Pop(H)
#define TopH()				Top(H)
#define GrowH(n)			Grow(H, n)



/* ARGUMENT ACCESS */

#define GetPt()			(*P++)
#define LookPt()		(*P)
#define LookHdl()		cHdl(*P)
#define GetHdl()		cHdl(*P++)
#define LookClause()	cClausePt(*P)
#define GetClause()		cClausePt(*P++)
#define GetWord()		cWord(*P++)
#define LookWord()		cWord(*P)
#define SkipWord()		(P++)
#define LookPred()		cPredicatePt(*P)
#define GetFunctor()	cFunctorPt(*P++)
#define LookFunctor()	cFunctorPt(*P)



/* SMALL CODE SEGMENT */

#define InstLoc(i)		(smallCodeSegm + i##Pos)

static Hdl smallCodeSegm ;  /* Small code segment */

static void InitSmallCodeSegment(void)
{
#define LastPos 23
	Hdl c = smallCodeSegm = Allocate(LastPos + 1, false) ; /* must have alloc addr */
#define UndefPredEndPos				1
	*c++ = cPt(WordsOf(Environment) + 1) ;	/* 1 cell local-var env - CTX14 */
	*c++ = cPt(UndefPredEnd) ;
#define ImportEndPos				3
	*c++ = cPt(WordsOf(Environment) + 1) ;	/* 1 cell local-var env - CTX1 */
	*c++ = cPt(ImportEnd) ;
#define CtxExtensionEndPos			5
	*c++ = cPt(WordsOf(Environment) + 1) ;	/* 1 cell local-var env - CTX1 */
	*c++ = cPt(CtxExtensionEnd) ;
#define CtxEmptyEndPos				7
	*c++ = cPt(WordsOf(Environment) + 2) ;	/* 2 cell local-var env - CTX124 */
	*c++ = cPt(CtxEmptyEnd) ;
#define CtxDownEndPos				9
	*c++ = cPt(WordsOf(Environment) + 1) ;	/* 1 cell local-var env - CTX124 */
	*c++ = cPt(CtxDownEnd) ;
#define HCtxPushEndPos				11
	*c++ = cPt(WordsOf(Environment) + 0) ;	/* 0 cell local-var env - CTX124 */
	*c++ = cPt(HCtxPushEnd) ;
#define HCtxEnterEndPos				13
	*c++ = cPt(WordsOf(Environment) + 2) ;	/* 2 cell local-var env - CTX124 */
	*c++ = cPt(HCtxEnterEnd) ;
#define CallCleanupContinuationPos	15
	*c++ = cPt(WordsOf(Environment) + 0) ;	/* 0 cell local-var env */
	*c++ = cPt(CallCleanupContinuation) ;
#define CallCleanupFailPos			17
	*c++ = cPt(WordsOf(Environment) + 0) ;	/* 0 cell local-var env */
	*c++ = cPt(CallCleanupFail) ;
#define FailPos						19
	*c++ = cPt(WordsOf(Environment) + 0) ;	/* 0 cell local-var env */
	*c++ = cPt(Fail) ;
#define FailMesgPos					21
	*c++ = cPt(WordsOf(Environment) + 0) ;	/* 0 cell local-var env */
	*c++ = cPt(FailMesg) ;
#define DeallocProceedPos			23
	*c++ = cPt(WordsOf(Environment) + 1) ;	/* 1 cell local-var env */
	*c++ = cPt(DeallocProceed) ;
}



/* AUXILIARY FUNCTIONS */

static void PrepareCallError(Pt t) {
	t = Drf(t) ;
	if( IsNumber(t) )
		TypeError2("CALLABLE", t, "NUMBER in call/1: %s", TermAsStr(t)) ;
	elif( IsVar(t) )
		TypeError2("CALLABLE", t, "Unbound VARIABLE in call/1: %s", TermAsStr(t)) ;
	elif( IsExtra(t) )
		TypeError2("CALLABLE", t, "EXTRA in call/1: '%s'", TermAsStr(t)) ;
	else InternalError("PrepareCallError") ;
}

static FunctorPt PrepareCall(register Pt t)
{
	VarValue(t) ;
redo:
	if( IsStruct(t) ) {
		if( IsUnitParam(t) ) {
			t = Drf(Z(XUnitParam(t))) ;
			goto redo ;
		}
		else {
			register int s = XStructArity(t) ;
			while( s-- )
				Xc(s) = XStructArg(t,s) ;
			return XStructFunctor(t) ;
		}
	}
	elif( IsAtom(t) ) {
		return LookupFunctor(XAtom(t), 0) ;
	}
	elif( IsList(t) ) {
		X0 = XListHead(t) ;
		X1 = XListTail(t) ;
		return listFunctor ;
	}
	elif(  t == tNilAtom && nilIsSpecial_flag )
		return nilIsSpecialFunctor ;
	else
		PrepareCallError(t) ;
	return nil ; /* avoids warning */
}

static FunctorPt PrepareCallExtraArgsInList(Pt t, Pt list)
{
	FunctorPt f = PrepareCall(t) ;
	int arity = FunctorArity(f) ;
	for( list = Drf(list) ; IsList(list) ; list = Drf(XListTail(list)) )
		Xc(arity++) = Drf(XListHead(list)) ;
	if( list != tNilAtom )
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	return LookupFunctor(FunctorAtom(f), arity) ;
}

static FunctorPt PrepareCallExtraArgs(register Pt t, int extra)
{
	VarValue(t) ;
redo:
	if( IsStruct(t) ) {
		if( IsUnitParam(t) ) {
			t = Drf(Z(XUnitParam(t))) ;
			goto redo ;
		}
		else {
			register int s = XStructArity(t) ;
			FunctorPt f = LookupFunctor(XStructAtom(t), s + extra) ;
			if( s > 1 ) {
				s += extra ;
				while( extra-- )
					Xc(--s) = Xc(extra + 1) ;
			}
			while( s-- )
				Xc(s) = XStructArg(t,s) ;
			return f ;
		}
	}
	elif( IsAtom(t) ) {
		FunctorPt f = LookupFunctor(XAtom(t), extra) ;
		register int i ;
		dotimes(i, extra)
			Xc(i) = Xc(i + 1) ;
		return f ;
	}
	elif( IsList(t) ) {
		FunctorPt f = LookupFunctor(XAtom(tDotAtom), 2 + extra) ;
		register int s = 2 + extra ;
		while( extra-- )
			Xc(--s) = Xc(extra + 1) ;
		X0 = XListHead(t) ;
		X1 = XListTail(t) ;
		return f ;
	}
	else PrepareCallError(t) ;
	return nil ; /* avoids warning */
}

static void ExecutePred(PredicatePt pr)
{
	ZEnsureFreeSpaceOnStacks(0, PredArity(pr), true) ;
	B0 = B ;
	if( Attention() && AttentionHandle(pr) ) return ;
	P = PredCode(pr) ;
}

static void ExecuteVarPredOpaque(PredicatePt pr)
{
	ZEnsureFreeSpaceOnStacks(0, PredArity(pr), true) ;
	B0 = B ;
#if !OLD_TRANSPARENT_CUT
#if 0
	Mesg("PrepCut %lx,  E %x", B0, E) ;
#endif
	AllocEnv() ;		/* Add marked environment */
	CP = InstLoc(DeallocProceed) ;
	Y(OutPerm(0)) = tMarkAtom ;
#endif
	if( Attention() && AttentionHandle(pr) ) return ;
	P = PredCode(pr) ;
}

#if unused
static void ExecuteVarPredTransparent(PredicatePt pr)
{
	ZEnsureFreeSpaceOnStacks(0, PredArity(pr), true) ;
	B0 = Ef(B0) ;
	AllocEnv() ;		/* Add marked environment */
	CP = InstLoc(DeallocProceed) ;
	Y(OutPerm(0)) = tMarkAtom ;
	if( Attention() && AttentionHandle(pr) ) return ;
	P = PredCode(pr) ;
}
#endif

static PredicatePt HandleUndefPredCall(FunctorPt f, Pt ctx)
{
	switch( unknown_flag ) {
		case 0: /* error */
			ExistenceError("procedure", MakeSlashTerm(f),
				"Predicate '%s' is not available in the context %s",
						FunctorNameArity(f), TermAsStr(ctx)) ;
		case 1: /* fail */
			return undefPred ;		/* causes failure */
		case 2: /* warning */
			Warning("Predicate '%s' is not available in the context %s",
						FunctorNameArity(f), TermAsStr(ctx)) ;
			return undefPred ;		/* causes failure */
	}

	if( undefWarnings_flag )
		Warning("Predicate '%s' is not available in the context %s",
						FunctorNameArity(f), TermAsStr(ctx)) ;
	return undefPred ;		/* causes failure */
}

/* Finalizers ARE TO BE USED ONLY with non-deterministic
   predicates written in C. A finalizer is activated
   when the choice-point of the associated a predicate
   is discarded. This can happen only in the instructions
   Cut, MetaCut, DiscardAndFail and Throw. */

void SetupFinalizer(FunV proc, VoidPt arg)
{
	if( Gt(TR,F-1) ) TrailExpand() ;
	F-- ;
	F->cp = B ;
	F->proc = proc ;
	F->arg = arg ;
}

static void ZFinalizeUntilHere(ChoicePointPt here)
{
	if( Lt(F,trailEnd) ) {	/* If has finalizers */
		if( here == nil ) {
		   while( Lt(F,trailEnd) ) {
				FinalizerPt f = F++ ;
				SetChoicePoint(f->cp) ;
				f->proc(f->arg) ; /* Call finalizer; stacks may grow */
			}
		}
		else {
			Size n ;
			FinalizerPt f ;
		/* Supports relocation of F and here */
			for( n = 0, f = F ; Lt(f,trailEnd) && Lt(f->cp,here) ; n++, f++ ) ;
			if( n > 0 ) {
#if 0
				Mesg("Handling %d finalizers", n) ;
#endif
				while( n-- ) {
					f = F++ ;
					SetChoicePoint(f->cp) ;
					f->proc(f->arg) ; /* Call finalizer */
				}
			}
		}
	}
}


/* PROCEDURAL & CONTROL INSTRUCTIONS */

/* This macro creates an open ended predicate environment which will
   progressivelly shrink along the predicate code. The current size
   of this environment is determined by the second argument to each
   Call instruction in the predicate body code. */

static void NopInst()
{
	JumpNext() ;
}

static void LocalJumpInst()
{
	P = LookHdl() ;
	JumpNext() ;
}

static void FAllocateInst()
{
	AllocEnv() ;
	JumpNext() ;
}

static void EnsureFreeSpaceInst()
{
	Z.w = GetWord() ;
	if( LookPt() != nil ) {	/* hasEnvironment */
		Q.h = CP ;
		CP = P + 1 ;
	}
	ZEnsureFreeSpaceOnStacks(Z.w, -1, false) ; /* @@@ REDO because GC */
	if( LookPt() != nil )	/* hasEnvironment */
		CP = Q.h ;
	SkipWord() ;
	JumpNext() ;
}

static void CallInst()
{
	CP = P + 2 ;	/* Skip arguments */
	ExecutePred(LookPred()) ;
	JumpNext() ;
}

static void ExecuteInst()
{
	ExecutePred(LookPred()) ;
	JumpNext() ;
}

static void DeallocExecuteInst()
{
	DeallocEnv() ;
	ExecutePred(LookPred()) ;
	JumpNext() ;
}

static void CallVarInst()
{
	CP = P + 1 ;	/* Skip argument */
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCall(X0))) ;
	JumpNext() ;
}

static void ExecuteVarInst()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCall(X0))) ;
	JumpNext() ;
}

static void DeallocExecuteVarInst()
{
	DeallocEnv() ;
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCall(X0))) ;
	JumpNext() ;
}

static void PApply()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgsInList(X0, X1)));
	JumpNext() ;
}

static void PTransparentCall()
{
	ExecutePred(LookupPredicateForMetaCall(PrepareCall(X0))) ;
	JumpNext() ;
}

static void POpaqueCall()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCall(X0))) ;
	JumpNext() ;
}

static void PCall2()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 1))) ;
	JumpNext() ;
}

static void PCall3()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 2))) ;
	JumpNext() ;
}

static void PCall4()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 3))) ;
	JumpNext() ;
}

static void PCall5()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 4))) ;
	JumpNext() ;
}

static void PCall6()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 5))) ;
	JumpNext() ;
}

static void PCall7()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 6))) ;
	JumpNext() ;
}

static void PCall8()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 7))) ;
	JumpNext() ;
}

static void PCall9()
{
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCallExtraArgs(X0, 8))) ;
	JumpNext() ;
}

static void PCallCleanup()
{
	AllocEnv() ;
	CP = InstLoc(CallCleanupContinuation) ;
	CreateChoicePoint(InstLoc(CallCleanupFail), 0) ; /* Create aux CP */
	SetupFinalizer((FunV)CallPrologTransparent, X1) ;  /*@@@@@ Needs checking callable(X1) */
	ExecuteVarPredOpaque(LookupPredicateForMetaCall(PrepareCall(X0))) ;
	JumpNext() ;
}
static void CallCleanupContinuationInst()
{
	if( Ef(B0) == Bf(B) ) /* Check if B is aux CP */
		CutTo(Ef(B0)) ;
	Jump(DeallocProceed) ;
}
static void CallCleanupFailInst()
{
	CutTo(Ef(B0)) ;
	DeallocEnv() ;
	DoFail() ;
}

void CutTo(ChoicePointPt cp)
{
	if( B < cp ) { /* Is current B newer? */
		Size lvrLevel = LvrPush(&cp) ;
		ZFinalizeUntilHere(cp) ;
		SetChoicePoint(cp) ;
		LvrRestore(lvrLevel) ;
	}
}

void CutAll()
{
	ZFinalizeUntilHere(nil) ;
}

static void CutInst()
{
	if( !DebugCut(Ef(B0)) )
		CutTo(Ef(B0)) ; /* Cut to the B at clause entry */
	JumpNext() ;
}

static void PMetaCut()
{
	if( !DebugCut(cChoicePointPt(X0)) )
		CutTo(cChoicePointPt(X0)) ; /* Cut to the B at clause entry */
	JumpNext() ;
}

static void PDynamicCut()
{
#if OLD_TRANSPARENT_CUT
	Error("Dynamic '!/0' is not supported") ;
	JumpNext() ;
#else
	EnvironmentPt eSave = E ;
	for( ; !IsEndOfChain(E) ; E = Ef(E) )
		if( Y(OutPerm(0)) == tMarkAtom ) {
			ChoicePointPt cutTo = Ef(B0) ;
	#if 0
			Mesg("-- Cut %lx,  E %x", cutTo, E) ;
	#endif
			E = eSave ;
			if( !DebugCut(cutTo) )
				CutTo(cutTo) ; /* Cut to the B at clause entry */
			JumpNext() ;
		}
	E = eSave ;
	JumpNext() ;
#endif
}

static void PutCutLevelInst()
{
	X(GetHdl()) = TagStruct(H) ; /* Inject call to '$$_meta_cut/1' predicate */
	PushH(metaCutFunctor) ;
	PushH(Ef(B0)) ;	/* Sole instance of global stack pointing to local stack */
	JumpNext() ;
}

static void ProceedInst()
{
	P = CP ;
	JumpNext() ;
}

static void DeallocProceedInst()
{
	DeallocEnv() ;
	P = CP ;
	JumpNext() ;
}

static void EmptyPredInst()
{
	DoFail() ;
}

static void FailInst()
{
	DoFail() ;
}

static void FailMesgInst()
{
	DoFail() ;
}



/* PUT INSTRUCTIONS */

static void PutYVariableInst()
{
	Q.h = &Y(GetWord()) ;
	X(GetHdl()) = ResetVar(Q.h) ;
	JumpNext() ;
}

static void PutXVariableInst()
{
	Q.h = GetHdl() ;
	X(GetHdl()) = X(Q.h) = PushHVar() ;
	JumpNext() ;
}

static void PutXVariableOneInst()
{
	X(GetHdl()) = PushHVar() ;
	JumpNext() ;
}

static void PutXValueInst()
{
	Q.h = GetHdl() ;
	X(GetHdl()) = X(Q.h) ;
	JumpNext() ;
}

static void PutYValueInst()
{
	Z.w = GetWord() ;
	X(GetHdl()) = Y(Z.w) ;
	JumpNext() ;
}

static void PutZValueInst()
{
	Z.w = GetWord() ;
	X(GetHdl()) = Z(Z.w) ;
	JumpNext() ;
}

static void PutUnsafeValueInst()
{
	VarValue2(D, Y(GetWord())) ;
	if( IsVar(D) && IsCurrEnvVar(D) ) {
		Assign(D, X(GetHdl()) = PushHVar()) ;
		JumpNext() ;
	}
	else {
		X(GetHdl()) = D ;
		JumpNext() ;
	}
}

static void PutAtomicInst()
{
	Q.t = GetPt() ;
	X(GetHdl()) = Q.t ;
	JumpNext() ;
}

static void PutNilInst()
{
	X(GetHdl()) = tNilAtom ;
	JumpNext() ;
}

static void PutStructureInst()
{
	PushH(GetPt()) ;
	X(GetHdl()) = TagStruct(H-1) ;
	JumpNext() ;
}

static void PutListInst()
{
	X(GetHdl()) = TagList(H) ;
	JumpNext() ;
}



/* GET INSTRUCTIONS */

static void GetYInitInst()
{
	Y(GetWord()) = tNilAtom ;
	JumpNext() ;
}

static void GetYVariableInst()
{
	Z.w = GetWord() ;
	Y(Z.w) = X(GetHdl()) ;
	JumpNext() ;
}

static void GetXValueInst()
{
	Q.h = GetHdl() ;
	Ensure( Unify(X(GetHdl()), X(Q.h)) ) ;
	VarValue2(D, X(Q.h)) ;
	X(Q.h) = D ;
	JumpNext() ;
}

static void GetYValueInst()
{
	Z.w = GetWord() ;
	MustBe( Unify(Y(Z.w), X(GetHdl())) ) ;
}

static void GetZValueInst()
{
	Z.w = GetWord() ;
	MustBe( Unify(Z(Z.w), X(GetHdl())) ) ;
}

static void GetAtomicInst()
{
	Q.t = GetPt() ;
	VarValue2(D, X(GetHdl())) ;
	if( IsVar(D) ) {
		Assign(D, Q.t) ;
		JumpNext() ;
	}
	MustBe( D == Q.t ) ;
}

static void GetNilInst()
{
	VarValue2(D, X(GetHdl())) ;
	if( IsVar(D) ) {
		Assign(D, tNilAtom) ;
		JumpNext() ;
	}
	MustBe( D == tNilAtom ) ;
}
static void GetStructureInst()
{
	Q.f = GetFunctor() ;		/* Get functor */
	VarValue2(D, X(GetHdl())) ;
	if( IsVar(D) ) {
		Assign(D, TagStruct(H)) ;
		PushH(Q.t) ;
		S = nil ;	/* set write mode */
		JumpNext() ;
	}
	Ensure( IsThisStruct(D, Q.f) ) ;
	S = XStructArgs(D) ;
	JumpNext() ;
}

static void GetListInst()
{
	VarValue2(D, X(GetHdl())) ;
	if( IsVar(D) ) {
		Assign(D, TagList(H)) ;
		S = nil ;	/* set write mode */
		JumpNext() ;
	}
	Ensure( IsList(D) ) ;
	S = XListArgs(D) ;
	JumpNext() ;
}



/* UNIFY INSTRUCTIONS */

static void UnifyVoidInst()
{
	if( S == nil ) { /* write mode */
		Z.w = GetWord() ;
		while( Z.w-- )
			Ignore(PushHVar()) ;
		JumpNext() ;
	}
	else {
		S += GetWord() ;
		JumpNext() ;
	}
}

static void UnifyVoidOneInst()
{
	if( S == nil ) { /* write mode */
		Ignore(PushHVar()) ;
		JumpNext() ;
	}
	else {
		S++ ;
		JumpNext() ;
	}
}

static void UnifyXVariableInst()
{
	if( S == nil ) { /* write mode */
		X(GetHdl()) = PushHVar() ;
		JumpNext() ;
	}
	else {
		X(GetHdl()) = *S++ ;
		JumpNext() ;
	}
}

static void UnifyYVariableInst()
{
	if( S == nil ) { /* write mode */
		Y(GetWord()) = PushHVar() ;
		JumpNext() ;
	}
	else {
		Y(GetWord()) = *S++ ;
		JumpNext() ;
	}
}

static void UnifyXValueInst()
{
	if( S == nil ) { /* write mode */
		PushH(X(GetHdl())) ; /* oc: a(f(X), f(f(X))). */
		if( occursCheck_flag )
			Ensure( OccursCheck(TopH()) ) ;
		JumpNext() ;
	}
	else {
		Ensure( Unify(*S++, X(Q.h = GetHdl())) ) ;
		VarValue2(D, X(Q.h)) ;
		X(Q.h) = D ;
		JumpNext() ;
	}
}

static void UnifyYValueInst()
{
	if( S == nil ) { /* write mode */
		PushH(Y(GetWord())) ; /* oc: b(X,f(X)) :- true, writeln(X). */
		if( occursCheck_flag )
			Ensure( OccursCheck(TopH()) ) ;
		JumpNext() ;
	}
	else {
		MustBe( Unify(*S++, Y(GetWord())) ) ;
	}
}

static void UnifyZValueInst() /* @@@ */
{
	if( S == nil ) { /* write mode */
		VarValue2(D, Z(GetWord())) ;
		if( IsVar(D) && IsLocalVar(D) ) {
			Mesg("LOCAL Z") ;
			Assign(D, PushHVar()) ;
			JumpNext() ;
		}
		else {
			PushH(D) ; /* oc */
			JumpNext() ;
		}
	}
	else
		MustBe( Unify(*S++, Z(GetWord())) ) ;
}

static void UnifyXLocalValueInst()
{
	if( S == nil ) { /* write mode */
		VarValue2(D, X(Q.h = GetHdl())) ;
		if( IsVar(D) && IsLocalVar(D) ) {
			Assign(D, X(Q.h) = PushHVar()) ;
			JumpNext() ;
		}
		else {
			PushH(X(Q.h) = D) ;	/* oc: c(X, f(X)). */
			if( occursCheck_flag )
				Ensure( OccursCheck(TopH()) ) ;
			JumpNext() ;
		}
	}
	else {
		Ensure( Unify(*S++, X(Q.h = GetHdl())) ) ;
		VarValue2(D, X(Q.h)) ;
		X(Q.h) = D ;
		JumpNext() ;
	}
}

static void UnifyYLocalValueInst()
{
	if( S == nil ) { /* write mode */
		VarValue2(D, Y(GetWord())) ;
		if( IsVar(D) && IsLocalVar(D) ) {
			Assign(D, PushHVar()) ;
			JumpNext() ;
		}
		else {
			PushH(D) ;	/* oc: d(X, f(X)) :- true, writeln(X). */
			if( occursCheck_flag )
				Ensure( OccursCheck(TopH()) ) ;
			JumpNext() ;
		}
	}
	else
		MustBe( Unify(*S++, Y(GetWord())) ) ;
}

static void UnifyAtomicInst()
{
	if( S == nil ) { /* write mode */
		PushH(GetPt()) ;
		JumpNext() ;
	}
	else {
		Q.t = GetPt() ;
		VarValue2(D, *S++) ;
		if( IsVar(D) ) {
			Assign(D, Q.t) ;
			JumpNext() ;
		}
		MustBe( D == Q.t ) ;
	}
}

static void UnifyNilInst()
{
	if( S == nil ) { /* write mode */
		PushH(tNilAtom) ;
		JumpNext() ;
	}
	else {
		VarValue2(D, *S++) ;
		if( IsVar(D) ) {
			Assign(D, tNilAtom) ;
			JumpNext() ;
		}
		MustBe( D == tNilAtom ) ;
	}
}



/* BUILD INSTRUCTIONS */

static void BuildVoidInst()
{
	Z.w = GetWord() ;
	while( Z.w-- )
		Ignore(PushHVar()) ;
	JumpNext() ;
}

static void BuildVoidOneInst()
{
	Ignore(PushHVar()) ;
	JumpNext() ;
}

static void BuildXVariableInst()
{
	X(GetHdl()) = PushHVar() ;
	JumpNext() ;
}

static void BuildYVariableInst()
{
	Y(GetWord()) = PushHVar() ;
	JumpNext() ;
}

static void BuildXValueInst()
{
	PushH(X(GetHdl())) ; /* oc: no. */
	JumpNext() ;
}

static void BuildYValueInst()
{
	PushH(Y(GetWord())) ; /* oc: no. */
	JumpNext() ;
}

static void BuildZValueInst()
{
	PushH(Z(GetWord())) ; /* oc: no. */
	JumpNext() ;
}

static void BuildXLocalValueInst()
{
	VarValue2(D, X(Q.h = GetHdl())) ;
	if( IsVar(D) && IsLocalVar(D) ) {
		Assign(D, X(Q.h) = PushHVar()) ;
		JumpNext() ;
	}
	else {
		PushH(X(Q.h) = D) ; /* oc: no. */
		JumpNext() ;
	}
}

static void BuildYLocalValueInst()
{
	VarValue2(D, Y(GetWord())) ;
	if( IsVar(D) && IsLocalVar(D) ) {
		Assign(D, PushHVar()) ;
		JumpNext() ;
	}
	else {
		PushH(D) ; /* oc: no. */
		JumpNext() ;
	}
}

static void BuildAtomicInst()
{
	PushH(GetPt()) ;
	JumpNext() ;
}

static void BuildNilInst()
{
	PushH(tNilAtom) ;
	JumpNext() ;
}



/* CONTEXT INSTRUCTIONS */

#if CONTEXTS == 0
#	include "ContextsInsts0.ch"
#elif CONTEXTS == 1
#	include "ContextsInsts1.ch"
#elif CONTEXTS == 2
#	include "ContextsInsts2.ch"
#elif CONTEXTS == 3
#	include "ContextsInsts3.ch"
#elif CONTEXTS == 4
#	include "ContextsInsts4.ch"
#else
#	include "ContextsInsts1.ch"
#endif



/* INDEXING INSTRUCTIONS */

static void MakeIndexInst()
{
	DoIndex(LookPred()) ;
	P-- ;
	JumpNext() ;
}

static void TryMeElseInst()
{
	Q.h = ClauseCode(GetClause()) ;
	Z.w = GetWord() ;
	CreateChoicePoint(Q.h, Z.w) ;
	JumpNext() ;
}

static void RetryMeElseInst()
{
	Bf(P) = ClauseCode(GetClause()) ;
	RestoreState(GetWord()) ;
	JumpNext() ;
}

static void TrustMeInst()
{
	SkipWord() ;
	RestoreState(GetWord()) ;
	SetChoicePoint(Bf(B)) ;
	JumpNext() ;
}

static void TryInst()
{
	Q.h = GetHdl() ;
	CreateChoicePoint(P, Q.t[-1]) ;
	P = Q.h ;
	JumpNext() ;
}

static void RetryInst()
{
	Bf(P) = P + 1 ;
	P = LookHdl() ;
	RestoreState(cWord(P[-1])) ;
	JumpNext() ;
}

static void TrustInst()
{
	P = LookHdl() ;
	RestoreState(cWord(P[-1])) ;
	SetChoicePoint(Bf(B)) ;
	JumpNext() ;
}

#define ClauseMatchThis(cl, t)				\
   ( IsVar(ClauseIdxInfo(cl, 0)) || ClauseIdxInfo(cl, 0) == (t) )

#define ClauseMatchStruct(cl)				\
   ( IsVar(ClauseIdxInfo(cl, 0)) || IsStruct(ClauseIdxInfo(cl, 0)) )

#define ClauseMatchAtomic(cl)				\
   ( IsVar(ClauseIdxInfo(cl, 0)) || IsAtomic(ClauseIdxInfo(cl, 0)) )

#define ClauseAliveMatchThis(cl, t)			\
	( ClauseIsAlive(cl)	&& ClauseMatchThis(cl, t) )

#define ClauseAliveMatchStruct(cl)			\
	( ClauseIsAlive(cl)	&& ClauseMatchStruct(cl) )

#define ClauseAliveMatchAtomic(cl)			\
	( ClauseIsAlive(cl) && ClauseMatchAtomic(cl) )

#define ClauseAlive2MatchThis(cl, clock, t)	\
	( ClauseIsAlive2(cl, clock)	&& ClauseMatchThis(cl, t) )

static void DynamicEnterInst() /* Logical update semantic view */
{
/* First clause always alive */
	Q.c = GetClause() ;
	Z.w = LookWord() ;
	P = ClauseCodeSkipHeader(Q.c) ;
/* Search second clause alive */
	for( Q.c = ClauseNext(Q.c) ; Q.c != nil ; Q.c = ClauseNext(Q.c) ) {
		if( ClauseIsAlive(Q.c) ) { /* There is a second clause */
			CreateChoicePoint(ClauseCode(Q.c), Z.w + 1) ; /* extra A() */
			A(Z.w) = GlobalClock ;	/* Store call-clock in the extra A() */
			JumpNext() ;
		}
	}
	JumpNext() ;
}

static void DynamicElseInst() /* Logical update semantic view */
{
	Q.c = GetClause() ;
	Z.w = GetWord() ;
	RestoreState(Z.w) ;
/* Search next clause alive for the call */
	for( ; Q.c != nil ; Q.c = ClauseNext(Q.c) ) {
		if( ClauseIsAlive2(Q.c, A(Z.w)) ) {
			Bf(P) = ClauseCode(Q.c) ;	/* Setup next alternative */
			JumpNext() ;
		}
	}
	SetChoicePoint(Bf(B)) ;	/* Discard */
	JumpNext() ;
}

static void DynamicEnterIndexedInst() /* Logical update semantic view */
{
	VarValue2(D, X0) ;
	X0 = D ;		/* Necessary for DynamicElseIndexedInst */
	if( IsVar(D) ) {
	/* First clause always alive */
		Q.c = cClausePt(P[3]) ;
		P = ClauseCodeSkipHeader(Q.c) ;
	/* Search second clause alive */
		for( Q.c = ClauseNext(Q.c) ; Q.c != nil ; Q.c = ClauseNext(Q.c) )
			if( ClauseIsAlive(Q.c) ) goto hasSecondClause ;
		JumpNext() ;
	}
	elif( IsStruct(D) ) {
		D = TagStruct(XStructFunctor(D)) ;
		if( P[2] == nil ) {
		/* Search first var or struct clause */
			for( Q.c = cClausePt(P[3]) ; ; Q.c = ClauseNext(Q.c) ) {
				if( Q.c == nil ) { Q.c = failClause ; break ; }
				if( ClauseAliveMatchStruct(Q.c) ) break ;
			}
			P[2] = cPt(Q.c) ;
		}
		else Q.c = cClausePt(P[2]) ;
		for( ; ; Q.c = ClauseNext(Q.c) ) {
			if( Q.c == nil ) DoFail() ;
			if( ClauseAliveMatchThis(Q.c, D) ) break ;
		}
	}
	elif( IsList(D) ) {
		D = TagList(nil) ;
		if( P[1] == nil ) {
		/* Search first clause var or list */
			for( Q.c = cClausePt(P[3]) ; ; Q.c = ClauseNext(Q.c) ) {
				if( Q.c == nil ) { Q.c = failClause ; break ; }
				if( ClauseAliveMatchThis(Q.c, D) ) break ;
			}
			P[1] = cPt(Q.c) ;
		}
		else Q.c = cClausePt(P[1]) ;
	}
	else {
		if( P[0] == nil ) {
		/* Search first var or atomic clause */
			for( Q.c = cClausePt(P[3]) ; ; Q.c = ClauseNext(Q.c) ) {
				if( Q.c == nil ) { Q.c = failClause ; break ; }
				if( ClauseAliveMatchAtomic(Q.c) ) break ;
			}
			P[0] = cPt(Q.c) ;
		}
		else Q.c = cClausePt(P[0]) ;
		for( ; ; Q.c = ClauseNext(Q.c) ) {
			if( Q.c == nil ) DoFail() ;
			if( ClauseAliveMatchThis(Q.c, D) ) break ;
		}
	}
	P = ClauseCodeSkipHeader(Q.c) ;
/* Search second clause relevant */
	for( Q.c = ClauseNext(Q.c) ; Q.c != nil ; Q.c = ClauseNext(Q.c) )
		if( ClauseAliveMatchThis(Q.c, D) ) goto hasSecondClause ;
	JumpNext() ;

hasSecondClause:
	Z.w = ClauseArity(Q.c) ;
	CreateChoicePoint(ClauseCode(Q.c), Z.w + 1) ; /* extra A() */
	A(Z.w) = GlobalClock ;	/* Store call-clock in the extra A() */
	JumpNext() ;
}

static void DynamicElseIndexedInst() /* Logical update semantic view */
{
	Q.c = GetClause() ;
	Z.w = GetWord() ;
	RestoreState(Z.w) ;
	if( IsVar(X0) ) {
/* Search next clause alive for the call */
		for( ; Q.c != nil ; Q.c = ClauseNext(Q.c) )
			if( ClauseIsAlive2(Q.c, A(Z.w)) ) {
				Bf(P) = ClauseCode(Q.c) ;	/* Setup next alternative */
				JumpNext() ;
			}
	}
	else {
		D = IsStruct(X0) ? TagStruct(XStructFunctor(X0))
		  : IsList(X0) ? TagList(nil)
		  : X0 ;
/* Search next clause relevant for the call */
		for( ; Q.c != nil ; Q.c = ClauseNext(Q.c) )
			if( ClauseAlive2MatchThis(Q.c, A(Z.w), D) ) {
				Bf(P) = ClauseCode(Q.c) ;	/* Setup next alternative */
				JumpNext() ;
			}
	}
/* Discard */
	SetChoicePoint(Bf(B)) ;
	JumpNext() ;
}

static void DynamicIUEnterInst() /* Immediate update semantic view */
{
	Q.c = GetClause() ;
	Z.w = LookWord() ;
	CreateChoicePoint(ClauseCode(Q.c), Z.w) ;
	P = ClauseCodeSkipHeader(Q.c) ;
	JumpNext() ;
}

static void DynamicIUElseInst() /* Immediate update semantic view */
{
	Q.c = GetClause() ;
	if( Q.c == nil )  {
		SetChoicePoint(Bf(B)) ;	/* Discard */
		DoFail() ;
	}
	else {
		Bf(P) = ClauseCode(Q.c) ;	/* Setup next alternative */
		RestoreState(LookWord()) ;
		P = ClauseCodeSkipHeader(Q.c) ;
		JumpNext() ;
	}
}

static void DynamicIUEnterIndexedInst() /* Immediate update semantic view */
{
	VarValue2(D, X0) ;
	X0 = D ;		/* Necessary for DynamicElseIndexedInst */
	if( IsVar(D) )
		Q.c = cClausePt(P[3]) ;
	elif( IsStruct(D) ) {
		D = TagStruct(XStructFunctor(D)) ;
		if( P[2] == nil ) {
		/* Search first var or struct clause */
			for( Q.c = cClausePt(P[3]) ; ; Q.c = ClauseNext(Q.c) ) {
				if( Q.c == nil ) { Q.c = failClause ; break ; }
				if( ClauseMatchStruct(Q.c) ) break ;
			}
			P[2] = cPt(Q.c) ;
		}
		else Q.c = cClausePt(P[2]) ;
		for( ; ; Q.c = ClauseNext(Q.c) ) {
			if( Q.c == nil ) DoFail() ;
			if( ClauseMatchThis(Q.c, D) ) break ;
		}
	}
	elif( IsList(D) ) {
		D = TagList(nil) ;
		if( P[1] == nil ) {
		/* Search first clause var or list */
			for( Q.c = cClausePt(P[3]) ; ; Q.c = ClauseNext(Q.c) ) {
				if( Q.c == nil ) { Q.c = failClause ; break ; }
				if( ClauseMatchThis(Q.c, D) ) break ;
			}
			P[1] = cPt(Q.c) ;
		}
		else Q.c = cClausePt(P[1]) ;
		if( Q.c == failClause ) DoFail() ;
	}
	else {
		if( P[0] == nil ) {
		/* Search first var or atomic clause */
			for( Q.c = cClausePt(P[3]) ; ; Q.c = ClauseNext(Q.c) ) {
				if( Q.c == nil ) { Q.c = failClause ; break ; }
				if( ClauseMatchAtomic(Q.c) ) break ;
			}
			P[0] = cPt(Q.c) ;
		}
		else Q.c = cClausePt(P[0]) ;
		for( ; ; Q.c = ClauseNext(Q.c) ) {
			if( Q.c == nil ) DoFail() ;
			if( ClauseMatchThis(Q.c, D) ) break ;
		}
	}
	Z.w = ClauseArity(Q.c) ;
	CreateChoicePoint(ClauseCode(Q.c), Z.w) ;
	P = ClauseCodeSkipHeader(Q.c) ;
	JumpNext() ;
}

static void DynamicIUElseIndexedInst() /* Immediate update semantic view */
{
	Q.c = GetClause() ;
	if( Q.c == nil )  {
		SetChoicePoint(Bf(B)) ;	/* Discard */
		DoFail() ;
	}
	else {
		D = A(0) ;
		if( !IsVar(D) ) {
			D = IsStruct(D) ? TagStruct(XStructFunctor(D))
			  : IsList(D) ? TagList(nil)
			  : D ;
			for( ; Q.c != nil ; Q.c = ClauseNext(Q.c) )
				if( ClauseMatchThis(Q.c, D) )
					goto setupNextAlternative ;
			SetChoicePoint(Bf(B)) ;	/* Discard */
			DoFail() ;
		}
	}

setupNextAlternative:
	Bf(P) = ClauseCode(Q.c) ;	/* Setup next alternative */
	RestoreState(LookWord()) ;
	P = ClauseCodeSkipHeader(Q.c) ;
	JumpNext() ;
}

static void DoSwitchOnTerm()
{
	VarValue(D) ;
	if( IsList(D) )
		P = cHdl(P[1]) ;
	elif( IsAtomic(D) )
		P = cHdl(P[0]) ;
	elif( IsVar(D) )
		P = cHdl(P[3]) ;
	else
		P = cHdl(P[2]) ;
}

static void SwitchOnTerm0Inst()
{
	D = X0 ;
	DoSwitchOnTerm() ;
	JumpNext() ;
}

static void SwitchOnTerm1Inst()
{
	D = X1 ;
	DoSwitchOnTerm() ;
	JumpNext() ;
}

static void SwitchOnTerm2Inst()
{
	D = X2 ;
	DoSwitchOnTerm() ;
	JumpNext() ;
}

static void SwitchOnAtomicInst()
{
	register PrologHashTable ht, ht1 ;
	/* Got here from SwitchOnTerm: D already constains Drf(Xi) */
	ht = (PrologHashTable)(P+2) ;
	doseq(ht1, ht + PrologHash(D, cWord(P[0])), ht1->next)
		if( ht1->value == D ) {
			P = ht1->address ;
			JumpNext() ;
		}
	P = cHdl(P[1]) ;
	JumpNext() ;
}

static void SwitchOnStructureInst()
{
	/* Got here from SwitchOnTerm: D already constains Drf(X?) */
	register PrologHashTable ht, ht1 ;
	D = cPt(XStructFunctor(D)) ;
	ht = (PrologHashTable)(P+2) ;
	doseq(ht1, ht + PrologHash(D, cWord(P[0])), ht1->next)
		if( ht1->value == D ) {
			P = ht1->address ;
			JumpNext() ;
		}
	P = cHdl(P[1]) ;
	JumpNext() ;
}

static void DiscardAndFailInst()
{	/* pre: B is the choice point of a non-deterministic C predicate */
	Discard() ;
	DoFail() ;
}



/* DEBUGGING INSTRUCTIONS */

static void DebugExitInst()
{
	DebugExitCode() ;
	JumpNext() ;
}

static void DebugRedoInst()
{
	DebugRedoCode() ;
	JumpNext() ;
}

static void DebugRetryInst()
{
	DebugRetryCode() ;
	JumpNext() ;
}



/* INSTRUCTIONS */

Inst
#define InstInfo(inst, args)	inst
#include "InstructionsInfo.h"
#undef InstInfo
;
Inst FirstInst, LastInst ;
InstPt FailAddr, FailMesgAddr ;

typedef struct {
	Hdl instAddr ;
	Inst inst ;
	Str name, types ;
} InstInfo, *InstInfoPt ;

static InstInfo insts[] = {
#define InstInfo(inst,args)	{ &inst, InstEncode(inst##Inst), #inst, args }
#include "InstructionsInfo.h"
#undef InstInfo
	, {nil,nil,nil,nil}
} ;

void InstructionsInit()
{
	register InstInfoPt pt ;
	FirstInst = InstEncode(NopInst) ;
	LastInst = InstEncode(NopInst) ;
	for( pt = insts ; pt->instAddr != nil ; pt++ ) {
		*pt->instAddr = pt->inst ;
		if( Lt(pt->inst, FirstInst) ) FirstInst = pt->inst ;
		if( Gt(pt->inst, LastInst) ) LastInst = pt->inst ;
	}
	InitSmallCodeSegment() ;
	FailAddr = InstLoc(Fail) ;
	FailMesgAddr = InstLoc(FailMesg) ;
}

Str GetInstInfo(Inst inst, Str *types)
{
	register InstInfoPt pt ;
	PredicatePt pr ;
	for( pt = insts ; pt->instAddr != nil ; pt++ )
		if( pt->inst == inst ) {
			if( types != nil )
				*types = pt->types ;
			return pt->name ;
		}
	if( types != nil ) *types = "" ;
	if( (pr = FindCPredByInst(inst)) != nil )
		return PredNameArity(pr) ;
	else return "UNKNOWN INSTRUCTION" ;
}

Str GetInstNameSearch(InstPt code)
{
	Str s ;
	int i = 4 ;
	do {
		s = GetInstInfo(*code--, nil) ;
	} while( s[0] == 'U' && --i > 0 ) ;
	return s ;
}

void InstructionsInit2()
{
	InstallCBuiltinPred("!", 0, PDynamicCut) ;
	InstallCBuiltinPred("$$_meta_cut", 1, PMetaCut) ;
	InstallCBuiltinPred("$$_transparent_call", 1, PTransparentCall) ;
	InstallCBuiltinPred("$$_opaque_call", 1, POpaqueCall) ;

	InstallCBuiltinPred("apply", 2, PApply) ;
	InstallCBuiltinPred("call", 1, POpaqueCall) ;
#if COMPASS
	InstallCBuiltinPred("call", 2, PApply) ;
	InstallCBuiltinPred("call_x", 2, PCall2) ;
#else
	InstallCBuiltinPred("call", 2, PCall2) ;
#endif
	InstallCBuiltinPred("call", 3, PCall3) ;
	InstallCBuiltinPred("call", 4, PCall4) ;
	InstallCBuiltinPred("call", 5, PCall5) ;
	InstallCBuiltinPred("call", 6, PCall6) ;
	InstallCBuiltinPred("call", 7, PCall7) ;
	InstallCBuiltinPred("call", 8, PCall8) ;
	InstallCBuiltinPred("call", 9, PCall9) ;
	InstallCBuiltinPred("call_cleanup", 2, PCallCleanup) ;
}
