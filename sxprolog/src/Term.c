/*
 *   This file is part of the CxProlog system

 *   Term.c
 *   by A.Miguel Dias - 1989/11/14
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
#include "TermCyclic.h"


/* GLOBAL STACK CONTROL */

#define PushHVar()			( ResetVar(H), cPt(H++) )
#define PushH(v)			Push(H, v)
#define PopH()				Pop(H)
#define GrowH(n)			Grow(H, n)
#define SizeH()				(H - stacksBegin)


/* EMPTY */

void EmptyRange(register Hdl a, register Hdl z)
{
	while( a < z )
		*a++ = EmptyCell ;
}

void EmptyRangeN(register Hdl a, register Size n)
{
	while( n-- )
		*a++ = EmptyCell ;
}

void CopyUntilEmpty(register Hdl z, register Hdl a)
{
	while( !IsEmpty(*z++ = *a++) ) ;
}

Hdl FindEmpty(register Hdl a)
{
	while( !IsEmpty(*a++) ) ;
	return a - 1 ;
}

VoidPt AllocateSegmentEmpty(Size nWords, VoidPt end)
{
	VoidPt mem = AllocateSegment(nWords, end) ;
	EmptyRangeN(mem, nWords) ;
	return mem ;
}


/* VARS */

static Pt cFreeVar = nil ; /* free var outside stacks */
static Pt limA, limB ;

Pt Drf(register Pt t)
{
	VarValue(t) ;
	return t ;
}

Pt GetCodeFreeVar()
{
	if( cFreeVar == nil ) {
		cFreeVar = Allocate(1, false) ; /* must have alloc addr too */
		ResetVar(cFreeVar) ;
	}
	return cFreeVar ;
}

void PrepareDrfChecked(register Pt term)
{
	VarValue(term) ;
	if( IsAllocCompound(term) ) {
		limA = XPt(term) ;
		limB = limA + MemBlockSize(limA) ;
	}
	else {
		limA = cPt(stacksBegin) ;
		limB = cPt(stacksEnd) ;
	}
}

Pt DrfChecked(register Pt t)
{
	while( IsVar(t) && Le(limA,t) && Lt(t,limB) && IsLink(t) )
		DrfVar(t) ;
	if( IsVar(t) && !(Le(limA,t) && Lt(t,limB)) )
		t = Eq(t,cFreeVar) ? cFreeVar : tBadTermAtom ;
	return t ;
}

Pt DrfChecked2(register Pt t, Hdl limA, Hdl limB)
{
	while( IsVar(t) && Le(limA,t) && Lt(t,limB) && IsLink(t) )
		DrfVar(t) ;
	if( IsVar(t) && !(Le(limA,t) && Lt(t,limB)) )
		return nil ;
	return t ;
}

static void RelocateDrfCheckedLimits(Size globalOffset, Size localOffset)
{
	if( globalOffset != 0 && Eq(limA, stacksBegin) )
		limA += globalOffset ;
	if( localOffset != 0 && Eq(limB, stacksEnd) )
		limB += localOffset ;
}

Pt MakeVar()
{
	CheckFreeSpaceOnStacks(1) ;
	return PushHVar() ;
}

Str VarName(Pt t)
{
	if( IsVar(t) ) {
		if( IsLocalRef(t) )
			return GStrFormat("_L%d", Df(stacksEnd, t) ) ;
		elif( IsGlobalRef(t) )
			return GStrFormat("_G%d", Df(t, stacksBegin) ) ;
		elif( Eq(t,cFreeVar) )
			return GStrFormat("_0") ;
		else /* is in alloc term */
			return GStrFormat("_0%d", Df(t,limA)) ;
	}
	else return InternalError("VarName") ;
}

Bool IsVarName(Str s)
{
	return( s[0] == '_' || InRange(s[0],'A','Z') ) ;
}


/* STRUCTS */

Pt MakeStruct(FunctorPt functor, Hdl args) /* pre: args in stacks */
{
	if( functor == listFunctor ) {
		CheckFreeSpaceOnStacks(2) ;
		PushH(args[0]) ;
		PushH(args[1]) ;
		return TagList(H - 2) ;
	}
	elif( FunctorArity(functor) == 0 )
		return TagAtom(FunctorAtom(functor)) ;
	else {
		register int i, arity = FunctorArity(functor) ;
		CheckFreeSpaceOnStacks(arity + 1) ;
		PushH(functor) ;
		dotimes(i, arity)
			PushH(args[i]) ;
		return TagStruct(H - arity - 1) ;
	}
}

Pt MakeCleanStruct(FunctorPt functor)
{
	if( functor == listFunctor ) {
		CheckFreeSpaceOnStacks(2) ;
		Ignore(PushHVar()) ;
		Ignore(PushHVar()) ;
		return TagList(H - 2) ;
	}
	elif( FunctorArity(functor) == 0 )
		return TagAtom(FunctorAtom(functor)) ;
	else {
		register int i, arity = FunctorArity(functor) ;
		CheckFreeSpaceOnStacks(arity + 1) ;
		PushH(functor) ;
		dotimes(i, arity)
			Ignore(PushHVar()) ;
		return TagStruct(H - arity - 1) ;
	}
}

Pt MakeUnStruct(FunctorPt functor, Pt arg) /* pre: arg in stacks */
{
	CheckFreeSpaceOnStacks(2) ;
	PushH(functor) ;
	PushH(arg) ;
	return TagStruct(H - 2) ;
}

Pt MakeBinStruct(FunctorPt functor, Pt arg0, Pt arg1) /* pre: args in stacks */
{
	if( functor == listFunctor )
			return MakeList(arg0, arg1) ;

	CheckFreeSpaceOnStacks(3) ;
	PushH(functor) ;
	PushH(arg0) ;
	PushH(arg1) ;
	return TagStruct(H - 3) ;
}

Pt MakeSlashTerm(FunctorPt f)
{
	return MakeBinStruct(slashFunctor,
					TagAtom(FunctorAtom(f)),
					MakeInt(FunctorArity(f))) ;
}

Pt MakeList(Pt head, Pt tail) /* pre: args in stacks */
{
	CheckFreeSpaceOnStacks(2) ;
	PushH(head) ;
	PushH(tail) ;
	return TagList(H - 2) ;
}

Pt MakeTriStruct(FunctorPt functor, Pt arg0, Pt arg1, Pt arg2) /* pre: args in stacks */
{
	CheckFreeSpaceOnStacks(4) ;
	PushH(functor) ;
	PushH(arg0) ;
	PushH(arg1) ;
	PushH(arg2) ;
	return TagStruct(H - 4) ;
}

Str XStructNameArity(Pt t)
{
	return GStrFormat("%s/%d", XStructName(t), XStructArity(t)) ;
}

void SplitNeckTerm(Pt c, Hdl parts)
{
	c = Drf(c) ;
	if( IsThisStruct(c, neckFunctor) ) {
		parts[0] = Drf(XStructArg(c,0)) ;
		parts[1] = Drf(XStructArg(c,1)) ;
	}
	else {
		parts[0] = c ;
		parts[1] = tTrueAtom ;
	}
}

PInt XUnitParam(register Pt t) /* PInt for result used in code generation */
{
	int i ;
	t = Drf(t) ;
	if( !IsUnitParam(t) )
		TypeError("UNIT-PARAMETER", nil) ;
	i = XTestInt(XStructArg(t, 0)) ;
	if( !InRange(i,1,UnitArity(CurrUnit())) )
		TypeError("UNIT-PARAMETER", nil) ;
	return i-1 ;
}


/* LISTS */

#define ListBuildForwardDecl(list)			\
	Pt list = tNilAtom ;					\
	register Hdl zCont = &list ;
	
#define ListBuildForwardAdd(t)				\
	do {									\
		*zCont = MakeList(t, tNilAtom) ;	\
		zCont = H-1 ;						\
	} while(0)

#define ListBuildReverseDecl(list)			\
	register Pt list = tNilAtom
	
#define ListBuildReverseAdd(list, t)		\
	do {									\
		PushH(t) ;							\
		PushH(list) ;						\
		list = TagList(H - 2) ;				\
	} while(0)


Pt ArrayToOpenList(register Hdl array, register Size n) /* pre: n > 0 */
{
	CheckFreeSpaceOnStacks(2 * n) ;
	ListBuildReverseDecl(list) ;
	list = array[ --n ] ;
	while( n-- )
		ListBuildReverseAdd(list, array[ n ]) ;
	return list ;
}

Pt ArrayToList(register Hdl array, register Size n)
{
	CheckFreeSpaceOnStacks(2 * n) ;
	ListBuildReverseDecl(list) ;
	while( n-- )
		ListBuildReverseAdd(list, array[ n ]) ;
	return list ;
}

Pt ArrayToListRev(register Hdl array, register Size n)
{
	CheckFreeSpaceOnStacks(2 * n) ;
	ListBuildReverseDecl(list) ;
	while( n-- )
		ListBuildReverseAdd(list, *array++) ;
	return list ;
}

Hdl ListToArray(Pt list, Size *len)
{
	UseScratch() ;
	for( list = Drf(list) ; IsList(list) ; list = Drf(XListTail(list)) )
		ScratchPush(Drf(XListHead(list))) ;
	if( list != tNilAtom )
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	*len = ScratchUsed() ;
	return FreeScratch() ;
}

Size ListLength(register Pt list)
{
	Size n = 0 ;
	for( list = Drf(list) ; IsList(list) ; list = Drf(XListTail(list)) )
		n++ ;
	if( list != tNilAtom )
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	return n ;
}

Bool ListCheck(register Pt list)
{
	for( list = Drf(list) ; IsList(list) ; list = Drf(XListTail(list)) ) ;
	return list == tNilAtom ;
}

static Bool Belongs(Pt t, register Pt list)	/* pre: t already deref */
{
	for( list = Drf(list) ; IsList(list) ; list = Drf(XListTail(list)) )
		if( t == Drf(XListHead(list)) ) return true ;
	if( list != tNilAtom )
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	return false ;
}


/* PSTRINGS */

Pt StringToPString(Str s)
{
	ListBuildForwardDecl(list) ;
	while( *s )
		ListBuildForwardAdd(MakeCode(CharDecode(s))) ;
	return list ;
}

Str PStringToString(Pt l)
{
	BigStrOpen() ;
	for( l = Drf(l) ; IsList(l) ; l = Drf(XListTail(l)) ) {
		WChar c = XTestCode(XListHead(l)) ;
		BigStrAddChar(CharFixCode(c)) ;
	}
	if( l != tNilAtom )
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	return BigStrClose() ;
}


/* ASTRINGS */

Pt StringToAString(Str s)
{
	ListBuildForwardDecl(list) ;
	while( *s )
		ListBuildForwardAdd(MakeChar(CharDecode(s))) ;
	return list ;
}

Str AStringToString(Pt l)
{
	BigStrOpen() ;
	for( l = Drf(l) ; IsList(l) ; l = Drf(XListTail(l)) ) {
		WChar c = XTestChar(XListHead(l)) ;
		BigStrAddChar(CharFixCode(c)) ;
	}
	if( l != tNilAtom )
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	return BigStrClose() ;
}


/* INTER-VAR-TABLE for CompoundTermSize */

/* During the copy of the term, the inter-var table must be capable to grow.
   However, this table cannot be relocated. Therefore we use a linked list
   of tables.

   Without this table, that is using direct var links from the original
   term to its copy, the following error would happen:

		[main] ?- z :- assert(f(X,f(X))).
		z:-assert(f(A,f(A))) asserted.
		[main] ?- z.
		{ERROR (assert/1): You cannot assert a cyclic term.}
*/

#define interVarTableCapacity		32

#define InterVarTable_VarDecl()										\
	Hdl interVarTablePt = interVarTable->vars ;						\
	Hdl interVarTableEnd = interVarTablePt + interVarTableCapacity

typedef struct InterVarTable {
	Pt vars[interVarTableCapacity] ;
	struct InterVarTable *next ;
} InterVarTable, *InterVarTablePt ;

static InterVarTablePt interVarTable ;

static void InterVarTableInit(void)
{
	interVarTable = Allocate(WordsOf(InterVarTable), false) ;
	interVarTable->next = nil ;
}

static void InterVarTableExpand(Hdl *tPt, Hdl *tEnd)
{
	InterVarTablePt t = (InterVarTablePt)(*tEnd - interVarTableCapacity) ;
	if( t->next == nil ) {
		t->next = Allocate(WordsOf(InterVarTable), false) ;
		t->next->next = nil ;
	}
	t = t->next ;
	*tPt = t->vars ;
	*tEnd = t->vars + interVarTableCapacity ;
}


/* TERMS */

Hdl termSegm ;  /* Small buffer used to build terms */

Str TermTypeStr(Pt t)
{
	t = Drf(t) ;
	return
		t == nil	? "NIL-POINTER" :
		IsVar(t)	? "VAR" :
		IsStruct(t)	? "STRUCT" :
		IsList(t)	? "LIST" :
		IsAtom(t)	? "ATOM" :
		IsInt(t)	? "INT" :
		IsFloat(t)	? "FLOAT" :
		IsExtra(t)	? (char *)XExtraTypeName(t) :
					  "UNKNOWN"
	;
}

Bool TermIsCyclic(Pt term) /* Handles cyclic terms */
{
	Cyclic1_VarDecl(term) ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		elif( t == tCyclicAtom ) { /* Cycle found */
			Cyclic1_Cleanup() ;
			FreeScratch() ;
			return true ;
		}
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
	return false ;
}

Bool OccursCheck(Pt term)
{
	if( TermIsCyclic(term) ) {
		if( occursCheck_flag == 2 )
			Error("[Occurs check] Attempt to build a cyclic term") ;
		return false ;
	}
	return true ;
}

/* TermBasicGCMark marks all the Extras contained in term */
void TermBasicGCMark(Pt term) /* Handles cyclic terms */
{
	Cyclic1_VarDecl(term) ;
	if( term == nil ) return ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( IsExtra(t) ) {
			ExtraGCMark(XExtra(t)) ;
		}
		elif( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
}

Bool TermIsGround(Pt term) /* Handles cyclic terms */
{
	Cyclic1_VarDecl(term) ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( IsVar(t) ) {
			Cyclic1_Cleanup() ;
			FreeScratch() ;
			return false ;
		}
		elif( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
	return true ;
}

static Pt TermHidePAR(Pt term) /* Handles cyclic terms */
{
	Hdl res = &term ;
	Cyclic1_VarDecl(term) ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			if( XStructFunctor(t) == parFunctor )
				ScratchXTop(2) = cPt(&XStructArg(t, 0)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
	return *res ;
}

static Pt TermShowPAR(Pt term) /* Handles cyclic terms */
{
	Hdl res = &term ;
	Cyclic1_VarDecl(term) ;
	UseScratch() ;
	for(;;) {
		if( IsVar(*h) && Eq((*h)[-1], parFunctor) )
			*h = TagStruct(*h - 1) ;
		VarValue2(t, *h) ;
		if( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
	return *res ;
}

Pt TermVars(Pt term, Size *nVars) /* Handles cyclic terms */
{
	Cyclic1_VarDecl(term) ;
	ListBuildForwardDecl(list) ;
	if( nVars != nil ) *nVars = 0 ;
	TrailAllVarsStart() ; /* trick: trail is instrumental in not repeating vars */
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( IsVar(t) ) {
			if( nVars != nil )
				(*nVars)++ ;
			else
				ListBuildForwardAdd(t) ;
			Assign(t, tNilAtom) ; /* mark var as "already seen var" */
		}
		elif( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
	TrailAllVarsRestore() ;
	return list ;
}

Bool TermContainsThisVar(Pt term, Pt var) /* Handles cyclic terms */
{	/* Pre: var is deref */
	Cyclic1_VarDecl(term) ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( t == var ) {
			Cyclic1_Cleanup() ;
			FreeScratch() ;
			return true ;
		}
		elif( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
	return false ;
}

Bool TermSubTerm(Pt s, Pt term) /* Handles cyclic terms */
{
	Cyclic1_VarDecl(term) ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( Identical(s, t) ) {
			Cyclic1_Cleanup() ;
			FreeScratch() ;
			return true ;
		}
		elif( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
	return false ;
}

Hdl TermSerialize(Pt term, Size *len)
{
	Cyclic1_VarDecl(term) ;
	UseScribbling() ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( IsVar(t) ) {
			ScribblingPush(tVarAtom);
			ScribblingPush(nil);
		}
		elif( IsAtomic(t) ) {
			if( t == tCyclicAtom ) {
				ScribblingTop() = t ;
			}
			else {
				ScribblingPush(t);
				ScribblingPush(nil);
			}
		}
		elif( IsStruct(t) ) {
			ScribblingPush(TagAtom(XStructAtom(t)));
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			ScribblingPush(tDotAtom);			
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		else {
			ScribblingPush(tBadTermAtom);
			ScribblingPush(nil);
		}
		Cyclic1_NextF(ScribblingPush(nil)) ;
	}
finish:
	FreeScratch() ;
	ScribblingDiscardTop(nil);
	/*ScribblingShow(nil);*/
	*len = ScribblingUsed();
	return FreeScribbling() ;
}

Pt TermUnserialize(Hdl array, Size len)
{
	int i = 0 ;
	UseScratch() ;
	ScratchSave() ;
	for( i = 0 ; ScratchDistToSaved() != 1 ; i++ ) {
		if( i < len && array[i] != nil ) {
			UseScratch() ;
			ScratchPush(array[i]) ;
		}
		else {
			int arity = ScratchCurr() - ScratchStart() - 1 ;
			Pt t = *ScratchStart() ;
			if( arity > 0 ) {
				if( IsAtom(t) ) {
					FunctorPt f = LookupFunctor(XAtom(t), arity) ;
					t = MakeStruct(f, ScratchStart() + 1) ;
				}
				else
					return Error("Invalid serialized prolog term") ;
			}
			FreeScratch() ;
			ScratchPush(t) ;
		}
	}
	Pt res = ScratchPop();
	FreeScratch() ;
	return res;
}

static void PSerializeTest()
{
	/* X = a(b(c(1,_,2.9,q(z(X)),f,g))), '$$_serialize_test'(X,Y). */
	Size len ;
	Hdl h = TermSerialize(X0, &len) ;
	Pt t = TermUnserialize(h, len) ;
	MustBe( Unify(X1, t) ) ;
}

static void CompoundTermRelocate(register Hdl z, register Hdl a, Size len)
{	/* pre: 'a' points to the start of a self contained term */
	Hdl a0 = a, aend = a0 + len ;
	register Size offset = z - a ;
	while( len-- )
		if( IsAtomic(*a) || (IsVar(*a) && !InRange(cHdl(*a), a0, aend)) )
			*z++ = *a++ ;
		else
			*z++ = *a++ + offset ;
}

static Size CompoundTermSize(Pt term, Bool convUnitParams) /* Handles cyclic terms */
{
	Cyclic1P_VarDecl(term) ;
	register Size size = 0 ;
	UseScratch() ;
	for(;;) {
		t = *h ;
redo:	VarValue(t) ;
		if( IsStruct(t) ) {
			if( convUnitParams && IsUnitParam(t) ) {
				t = Drf(Z(XUnitParam(t))) ;
				goto redo ;
			}
			else {
				int arity = XStructArity(t) ;
				Hdl args = XStructArgs(t) ;
				t = args[0] ;
				if( t == tCyclicAtom ) /* Already visited? */
					/* MeetCyclicTerm() */ ;
				else {
					Cyclic1P_PrepareArgs(args, arity) ;
					size += arity + 1 ;
					/* t1 already prepared */
					goto redo ;
				}
			}
		}
		elif( IsList(t) ) {
			Hdl args = XListArgs(t) ;
			t = args[0] ;
			if( t == tCyclicAtom ) /* Already visited? */
				/* MeetCyclicTerm() */ ;
			else {
				Cyclic1P_PrepareArgs(args, 2) ;
				size += 2 ;
				/* t1 already prepared */
				goto redo ;
			}
		}
		Cyclic1P_Next() ;
	}
finish:
	FreeScratch() ;
	return size ;
}

static Pt CompoundTermCopy(Pt term, Hdl to, Hdl toLimit, Pt env, Bool permanentExtras,
				Bool convUnitParams, Bool cyclesAllowed, Size *size) /* Handles cyclic terms */
{
	Pt res ;
	Cyclic2P_VarDecl(term, res) ;
	InterVarTable_VarDecl() ;
	Hdl bottom = to ;
	Hdl args ;
	int arity ;
	TrailAllVarsStart() ; /* Trick: trail is instrumental to refreshing vars */
	UseScratch() ;
	for(;;) {
		t1 = *h1 ;
redo:	VarValue(t1) ;
		if( IsStruct(t1) ) {
			if( convUnitParams && IsUnitParam(t1) ) {
				t1 = Z(XUnitParam(t1)) ;
				goto redo ;
			}
			else {
				FunctorPt f = XStructFunctor(t1) ;
				args = XStructArgs(t1) ;
				arity = FunctorArity(f) ;
				t1 = args[0] ; /* Preparing the next t1 */
				if( Le(bottom,t1) && Lt(t1,to) ) { /* Already visited? */
				/*	Mesg("already visited %s", FunctorNameArity(f)) ; */
					/* MeetCyclicTerm() */ ;
					if( !cyclesAllowed )
						goto invalidCycle ;
					*h2 = TagStruct(t1-1) ;	/* Reuse subterm */
				}
				else {
				/*	Mesg("entering %s", FunctorNameArity(f)) ; */
					if( to + 1 + arity > toLimit )	/* Check overflow */
						goto overflow ;
					*h2 = TagStruct(to) ;
					Push(to, f) ;
common:				if( Eq(t1, args) ) {
						/* Mesg("args[0] is VAR-AUTOREF") ; */
						/* If the original args[0] was a variable pointing to
						   itself, the var must be immediatly relocated to
						   another place. Here is a Prolog clause where
						   this situation arises:
								run :- X=f(A,A,X), a:=X, writeln(X). */
						if( interVarTablePt == interVarTableEnd )   /* Table full */
							InterVarTableExpand(&interVarTablePt, &interVarTableEnd) ;
						Assign(t1, interVarTablePt) ;
						*interVarTablePt = ResetVar(to) ;
						interVarTablePt++ ;
					}
					Cyclic2P_PrepareArgs(args, to, arity, cPt(to)) ;
					/* Warning: args[0] has been destroyed by marking */
					Grow(to, arity) ;
				/* t1 already prepared */
					goto redo ; /* t1 already prepared */
				}
			}
		}
		elif( IsList(t1) ) {
			args = XListArgs(t1) ;
			arity = 2 ;
			t1 = args[0] ; /* Preparing the next t1 */
			if( Le(bottom,t1) && Lt(t1,to) ) { /* Already visited? */
			/*	Mesg("already visited LIST") ; */
				if( !cyclesAllowed )
					goto invalidCycle ;
				*h2 = TagList(t1) ;	/* Reuse subterm */
			}
			else {
			/*	Mesg("entering LIST") ; */
				if( to + 2 > toLimit )	/* Check overflow */
					goto overflow ;
				*h2 = TagList(to) ;
				goto common ;
			}
		}
		elif( IsVar(t1) ) {
			if( Le(bottom,t1) && Lt(t1,to) ) /* Var already refreshed */
				*h2 = t1 ;
			elif( env == nil || Belongs(t1, env) ) { /* Var to refresh */
			/* Inter-term indirect linking of vars */
			/* Cannot point directly because of cycle-detection */
				if( interVarTablePt == interVarTableEnd )	/* Table full */
					InterVarTableExpand(&interVarTablePt, &interVarTableEnd) ;
				Assign(t1, interVarTablePt) ;
				*interVarTablePt = ResetVar(h2) ;
				interVarTablePt++ ;
			}
			else *h2 = t1 ; /* This var not to refresh */
		}
		elif( permanentExtras && IsExtra(t1) ) {
			*h2 = t1 ;
			ExtraPermanent(XExtra(t1)) ;
		}
		else {
			*h2 = t1 ;
		}
		Cyclic2P_Next() ;
	}
invalidCycle:
	Cyclic2P_Cleanup() ;	/* Restore term */
	FreeScratch() ;
	TrailAllVarsRestore() ;
	return DatabaseError("You cannot assert a cyclic term") ;
overflow:
/*	Mesg("overflow") ; */
	Cyclic2P_Cleanup() ;	/* Restore term */
	FreeScratch() ;
	TrailAllVarsRestore() ;
	return nil ;
finish:
	FreeScratch() ;
	TrailAllVarsRestore() ;
	if( size != nil )
		*size = to - bottom ;
	return res ;
}

/* ZPushCompoundTerm MAY cause the stacks to grow */
static Pt ZPushCompoundTerm(Pt term, Pt env, Bool permanentExtras,
					Bool convUnitParams, Bool cyclesAllowed)
{
	extern Pt ZT ;  /* This variable is only used here */
	Pt t ;
	Size size ;
	for(ZT = term
		; (t = CompoundTermCopy(ZT, H, H + FreeHSpace(),
						env, permanentExtras, convUnitParams,
						cyclesAllowed, &size)) == nil
		; ZStacksExpansion(0)) ;
	GrowH(size) ;
	ZT = t ;
	ZEnsureFreeSpaceOnStacks(0, -1, false) ;
	return ZT ;		
}

/* AllocateCompoundTerm NEVER causes the stacks to grow */
static Pt AllocateCompoundTerm(Pt term, Pt env, Bool permanentExtras,
					Bool convUnitParams, Bool cyclesAllowed)
{
/* Firstly, tries to use the available space in the global stack as buffer.
   This is the faster technique	*/
	Size size ;
#if 0
	Pt res = CompoundTermCopy(term, H, H + FreeHSpace(), env, permanentExtras, convUnitParams, cyclesAllowed, &size) ;
#else
	Pt res = nil ;
#endif
	if( res != nil ) { /* Sucessfull copy? */
		Hdl mem = Allocate(size, true) ;
		CompoundTermRelocate(mem, H, size) ;
		return IsStruct(res) ? TagStruct(mem) : TagList(mem) ;
	}
	else {
/* In case the buffer is not big enough, a slower but guaranteed
   alternative technique is used */
		Size exactSize = CompoundTermSize(term, convUnitParams) ;
		Hdl mem = Allocate(exactSize, true) ;
		Pt res = CompoundTermCopy(term, mem, mem + exactSize, env, permanentExtras, convUnitParams, cyclesAllowed, &size) ;
		if( res == nil || size != exactSize )
			InternalError("AllocateCompoundTerm") ;
		return res ;
	}
}

Pt AllocateTermForTopLevelAssert(register Pt t)
{
	VarValue(t) ;
	if( IsCompound(t) )
		return AllocateCompoundTerm(t, nil, false, false, false) ;
	elif( IsExtra(t) ) {
		ExtraPermanent(XExtra(t)) ;
		return t ;
	}
	else
		return t ;
}

Pt AllocateTermForAssert(register Pt t)
{
	VarValue(t) ;
	if( IsCompound(t) )
		return AllocateCompoundTerm(t, nil, true, false, false) ;
	elif( IsExtra(t) ) {
		ExtraPermanent(XExtra(t)) ;
		return t ;
	}
	else
		return t ;
}

Pt AllocateTermForAssign(register Pt t)
{
	VarValue(t) ;
	if( IsCompound(t) )
		return AllocateCompoundTerm(t, nil, false, false, true) ;
	elif( IsVar(t) )
		return cFreeVar ;
	else
		return t ;
}

void ReleaseTerm(register Pt t)
{
	if( t == nil ) return ;
	VarValue(t) ;
	if( IsCompound(t) )
		Release(XPt(t), -1) ;
}

static Pt ZPushTermWithEnv(register Pt t, Pt env, Bool convUnitParams)
{
	VarValue(t) ;
	if( IsCompound(t) )
		return ZPushCompoundTerm(t, env, false, convUnitParams, true) ;
	elif( IsVar(t) ) {
		ZEnsureFreeSpaceOnStacks(1, -1, false) ;
		if( env == nil || Belongs(t, env) )
			return PushHVar() ;
		else return t ;
	}
	else
		return t ;
}

Pt ZPushTerm(Pt t)
{
	return ZPushTermWithEnv(t, nil, false) ;
}

Pt ZPushTerm_ConvUnitParams(Pt t)
{
	return ZPushTermWithEnv(t, nil, true) ;
}


/* NUMBER VARS */

static Size numberVarsN ;

static void DoNumberVars(Pt term) /* Handles cyclic terms */
{
	Cyclic1_VarDecl(term) ;
	UseScratch() ;
	for(;;) {
		VarValue2(t, *h) ;
		if( IsVar(t) ) {
			Assign(t, MakeUnStruct(varFunctor, MakeInt(numberVarsN++))) ;
		}
		elif( IsStruct(t) ) {
			Cyclic1_PrepareArgs(XStructArgs(t), XStructArity(t)) ;
			continue ;
		}
		elif( IsList(t) ) {
			Cyclic1_PrepareArgs(XListArgs(t), 2) ;
			continue ;
		}
		/* if( t == tCyclicAtom ) MeetCyclicTerm() ; */
		Cyclic1_Next() ;
	}
finish:
	FreeScratch() ;
}

static Size NumberVars(Pt t, Size start)
{
	numberVarsN = start ;
	DoNumberVars(t) ;
	return( numberVarsN ) ;
}


/* TYPE ERRORS */

Pt TestAtomic(register Pt t)
{
	VarValue(t) ;
	if( IsAtomic(t) ) return t ;
	return TypeError("ATOMIC", t) ;
}

Pt TestAtom(register Pt t)
{
	VarValue(t) ;
	if( IsAtom(t) ) return t ;
	return TypeError("ATOM", t) ;
}

Pt TestAtomOrVar(register Pt t)
{
	VarValue(t) ;
	if( IsVar(t) || IsAtom(t) ) return t ;
	return TypeError("ATOM or VAR", t) ;
}

Pt TestList(register Pt t)
{
	VarValue(t) ;
	if( IsList(t) || t == tNilAtom ) return t ;
	return TypeError("LIST", t) ;
}

Pt TestListOrVar(register Pt t)
{
	VarValue(t) ;
	if( IsVar(t) || IsList(t) || t == tNilAtom ) return t ;
	return TypeError("LIST or VAR", t) ;
}

Pt TestInt(register Pt t)
{
	VarValue(t) ;
	if( IsInt(t) ) return t ;
	return TypeError("INT", t) ;
}

Pt TestIntOrVar(register Pt t)
{
	VarValue(t) ;
	if( IsVar(t) || IsInt(t) ) return t ;
	return TypeError("INT or VAR", t) ;
}

Pt TestPosInt(register Pt t)
{
	VarValue(t) ;
	if( !IsInt(t) )
		return TypeError("INT", t) ;
	if( IsPos(t) ) return t ;
	return DomainError("INT>0", t) ;
}

Pt TestPosIntOrVar(register Pt t)
{
	VarValue(t) ;
	if( IsVar(t) ) return t ;
	if( !IsInt(t) )
		return TypeError("INT", t) ;
	if( IsPos(t) ) return t ;
	return DomainError("INT>0 or VAR", t) ;
}

Pt TestNat(register Pt t)
{
	VarValue(t) ;
	if( !IsInt(t) )
		return TypeError("INT", t) ;
	if( IsNat(t) ) return t ;
	return DomainError("INT>=0", t) ;
}

Pt TestNatOrVar(register Pt t)
{
	VarValue(t) ;
	if( IsVar(t) ) return t ;
	if( !IsInt(t) )
		return TypeError("INT", t) ;
	if( IsNat(t) ) return t ;
	return DomainError("INT>=0 or VAR", t) ;
}

Pt TestIntRange(register Pt t, int a, int z)
{
	Str s ;
	VarValue(t) ;
	if( !IsInt(t) )
		return TypeError("INT", t) ;
	if( InRange(XInt(t), a, z) ) return t ;
	s = GStrFormat("RANGE(%d..%d)", a, z) ;
	return DomainError(s, t) ;
}

Pt TestCode(register Pt t)
{
	VarValue(t) ;
	if( !IsInt(t) )
		return TypeError("INT", t) ;
	if( IsCode(t) ) return t ;
	return RepresentationError("character_code", t, "") ;
}

Pt TestByte(register Pt t)
{
	VarValue(t) ;
	if( IsByte(t) ) return t ;
	return TypeError("BYTE", t) ;
}

Pt TestChar(register Pt t)
{
	VarValue(t) ;
	if( IsAtom(t) && XChar(t) != -1 ) return t ;
	return TypeError("CHAR", t) ;
}

Pt TestCharOrCode(register Pt t)
{
	VarValue(t) ;
	if( IsCode(t) || (IsAtom(t) && XChar(t) != -1) ) return t ;
	return TypeError("CHAR or CODE", t) ;
}

Pt TestVar(register Pt t)
{
	VarValue(t) ;
	if( IsVar(t) ) return t ;
	return TypeError("VAR", t) ;
}

Pt TestNonVar(register Pt t)
{
	VarValue(t) ;
	if( !IsVar(t) ) return t ;
	return TypeError("NON-VAR", t) ;
}

AtomPt XTestAtom(Pt t)
{
	return XAtom(TestAtom(t)) ;
}

Str XTestAtomName(Pt t)
{
	return XAtomName(TestAtom(t)) ;
}

Str XTestAtomNameOrPString(Pt t)
{
	t = Drf(t) ;
	if( IsList(t) || t == tNilAtom )
		return PStringToString(t) ;
	if( IsAtom(t) )
		return XAtomName(t) ;
	return TypeError("ATOM or PROLOG STRING", t) ;
}

Str XTestFileName(register Pt t)
{
	VarValue(t) ;
	if( IsAtom(t) ) return ProcessFileName(XAtomName(t)) ;
	return TypeError("FILENAME", t) ;
}

PInt XTestInt(Pt t)
{
	return XInt(TestInt(t)) ;
}

LLInt XTestLLInt(register Pt t)  /* used in the java interface */
{
	VarValue(t) ;
	if( IsNumber(t) ) return XAsLLInt(t) ;
	return ITypeError("NUMBER", t) ;
}

PInt XTestPosInt(Pt t)
{
	return XInt(TestPosInt(t)) ;
}

PInt XTestNat(Pt t)
{
	return XInt(TestNat(t)) ;
}

WChar XTestCode(Pt t)
{
	return XCode(TestCode(t)) ;
}

PInt XTestByte(Pt t)
{
	return XByte(TestByte(t)) ;
}

WChar XTestChar(Pt t)
{
	return XChar(TestChar(t)) ;
}

WChar XTestCharOrCode(Pt t)
{
	t = TestCharOrCode(t) ;
	return IsCode(t) ? XCode(t) : XChar(t) ;
}

PInt XTestIntRange(Pt t, int a, int z)
{
	return XInt(TestIntRange(t,a,z)) ;
}

PFloat XTestFloat(register Pt t)
{
	VarValue(t) ;
	if( IsInt(t) ) return XInt(t) ;
	if( IsFloat(t) ) return XFloat(t) ;
	return ITypeError("NUMBER", t) ;
}

Bool XTestBool(register Pt t)
{
	VarValue(t) ;
	if( t == tFalseAtom ) return false ;
	if( t == tTrueAtom ) return true ;
	return ITypeError("BOOL", t) ;
}

Bool3 XTestBool3(register Pt t)
{
	VarValue(t) ;
	if( t == tFalseAtom ) return false3 ;
	if( t == tTrueAtom ) return true3 ;
	if( t == tAbsentAtom ) return undefined3 ;
	return ITypeError("BOOL3", t) ;
}

Bool XTestOnOff(register Pt t)
{
	VarValue(t) ;
	if( t == tOnAtom ) return true ;
	if( t == tOffAtom ) return false ;
	return ITypeError("'on/off'", t) ;
}

Str XTestTermName(register Pt t)
{
	VarValue(t) ;
	if( IsAtom(t) ) return XAtomName(t) ;
	if( IsStruct(t) ) return XStructName(t) ;
	if( IsList(t) ) return "." ;
	if( t == tNilAtom && nilIsSpecial_flag )
		return FunctorName(nilIsSpecialFunctor) ;
	return TypeError("STRUCT, LIST or ATOM", t) ;
}

FunctorPt XTestFunctor(register Pt t)
{
	VarValue(t) ;
	if( IsStruct(t) ) return XStructFunctor(t) ;
	if( IsList(t) ) return listFunctor ;
	if( IsAtom(t) ) return LookupFunctor(XAtom(t), 0) ;
	if( t == tNilAtom && nilIsSpecial_flag ) return nilIsSpecialFunctor ;
	return TypeError("STRUCT, LIST or ATOM", t) ;
}

FunctorPt XTestFunctor2(Pt t1, Pt t2)
{
	return( LookupFunctor(XTestAtom(t1), XTestNat(t2)) ) ;
}

FunctorPt XTestSlash(register Pt t)
{
	VarValue(t) ;
	if( IsThisStruct(t, slashFunctor) )
		return XTestFunctor2(XStructArg(t,0), XStructArg(t,1)) ;
	return TypeError("PREDICATE INDICATOR", t) ;
}

void XTestSlashArgs(register Pt t, Hdl a0, Hdl a1)
{
	VarValue(t) ;
	if( !IsThisStruct(t, slashFunctor) )
		TypeError("PREDICATE INDICATOR", t) ;
	*a0 = Drf(XStructArg(t, 0)) ;
	if( !IsVar(*a0) && !IsAtom(*a0) )
		TypeError("PREDICATE INDICATOR", t) ;
	*a1 = Drf(XStructArg(t, 1)) ;
	if( !IsVar(*a1) && !IsNat(*a1) )
		TypeError("PREDICATE INDICATOR", t) ;
}

FunctorPt XTestStruct(register Pt t, Hdl *args)
{
	VarValue(t) ;
	if( IsStruct(t) ) { *args = XStructArgs(t) ; return XStructFunctor(t) ; }
	if( IsList(t) ) { *args = XListArgs(t) ; return listFunctor ; }
	if( IsAtom(t) ) { *args = H ; return LookupFunctor(XAtom(t), 0) ; }
	if( t == tNilAtom && nilIsSpecial_flag )
		{ *args = H ; return nilIsSpecialFunctor; }
	return TypeError("STRUCT, LIST or ATOM", t) ;
}

AtomPt XTermAtom(register Pt t)
{
	VarValue(t) ;
	if( IsAtom(t) ) return XAtom(t) ;
	if( IsStruct(t) ) return XStructAtom(t) ;
	if( IsList(t) ) return XAtom(tDotAtom) ;
	if( t == tNilAtom && nilIsSpecial_flag )
		return FunctorAtom(nilIsSpecialFunctor) ;
	return nil ;
}

FunctorPt XTermFunctor(register Pt t)
{
	VarValue(t) ;
	if( IsStruct(t) ) return XStructFunctor(t) ;
	if( IsList(t) ) return listFunctor ;
	if( IsAtom(t) ) return LookupFunctor(XAtom(t), 0) ;
	if( t == tNilAtom && nilIsSpecial_flag ) return nilIsSpecialFunctor ;
	return nil ;
}

Hdl XTermArgs(register Pt t, int *arity)
{
	VarValue(t) ;
	if( IsStruct(t) ) {
		*arity = XStructArity(t) ;
		return XStructArgs(t) ;
	}
	elif( IsList(t) ) {
		*arity = 2 ;
		return XListArgs(t) ;
	}
	else {
		*arity = 0 ;
		return nil ;
	}
}

Bool XAtomBool(int* out, Pt t)
{
	VarValue(t) ;
	if( t == tTrueAtom ) {
		*out = 1 ;
		return true ;
	}
	elif( t == tFalseAtom ) {
		*out = 0 ;
		return true ;
	}
	else
		return false ;
}

Bool XAtomAlt(int* out, Pt t, ...)
{
	VarValue(t) ;
	if( IsAtom(t) ) {
		Str s, g = XAtomName(t) ;
		int i ;
		va_list v ;
		va_start(v, t) ;
		for( i = 0 ; (s = va_arg(v, CharPt)) != nil ; i++ )
			if( StrEqual(g, s) ) {
				va_end(v) ;
				*out = i ;
				return true ;
			}
		va_end(v) ;
	}
	return false ;
}


/* CXPROLOG C'BUILTINS */

static void PVar()
{
	Pt t = Drf(X0) ;
	MustBe( IsVar(t) ) ;
}

static void PNonVar()
{
	Pt t = Drf(X0) ;
	MustBe( !IsVar(t) ) ;
}

static void PAtom()
{
	Pt t = Drf(X0) ;
	MustBe( IsAtom(t) ) ;
}

static void PPInt()
{
	Pt t = Drf(X0) ;
	MustBe( IsInt(t) ) ;
}

static void PPFloat()
{
	Pt t = Drf(X0) ;
	MustBe( IsFloat(t) ) ;
}

static void PNumber()
{
	Pt t = Drf(X0) ;
	MustBe( IsNumber(t) ) ;
}

static void PAtomic()
{
	Pt t = Drf(X0) ;
	MustBe( IsAtomicStrict(t) ) ;
}

static void PGround()
{
	MustBe( TermIsGround(X0) ) ;
}

static void PCompound()
{
	Pt t = Drf(X0) ;
	MustBe( IsCompound(t) ) ;
}

static void PCallable()
{
	Pt t = Drf(X0) ;
	while( IsUnitParam(t) )
		t = Drf(Z(XUnitParam(t))) ;
	MustBe( IsCompound(t) || IsAtom(t) || (t == tNilAtom && nilIsSpecial_flag) ) ;
}

static void PIsList(void)
{
	MustBe( ListCheck(X0) ) ;
}

static void PFunctor()
{
	Pt t0 = Drf(X0) ;
	if( IsStruct(t0) )
		MustBe( UnifyWithAtomic(X1, TagAtom(XStructAtom(t0))) &&
				UnifyWithNumber(X2, MakeInt(XStructArity(t0))) ) ;
	elif( IsList(t0) )
		MustBe( UnifyWithAtomic(X1, tDotAtom) &&
				UnifyWithNumber(X2, MakeInt(2)) ) ;
	elif( IsAtomic(t0) )
		MustBe( UnifyWithAtomic(X1, t0) && UnifyWithNumber(X2, zeroIntPt) ) ;
	elif( IsVar(t0) ) {
		PInt n = XTestNat(X2) ;
		if( n == 0 )
			MustBe( UnifyWithAtomic(t0, TestAtomic(X1)) ) ;
		else
			MustBe( Unify(t0, MakeCleanStruct(LookupFunctor(XTestAtom(X1), n))) ) ;
	}
	InternalError("PFunctor") ;
}

static void PArg()
{
	PInt n = XTestNat(X0) ; // for domain_error
	Pt t1 = Drf(X1) ;
	if( IsStruct(t1) )
		MustBe( InRange(n,1,XStructArity(t1)) && Unify(X2, XStructArg(t1, n-1)) ) ;
	elif( IsList(t1) )
		MustBe( InRange(n,1,2) && Unify(X2, XListArg(t1, n-1)) ) ;
	else TypeError("STRUCT or LIST", t1) ;
}

static void PIns()
{
	PInt pos = XTestInt(X0) ;
	Pt t, t1 = Drf(X1) ;
	Hdl args, h ;
	FunctorPt f ;
	int newArity, n ;
	if( IsVar(t1) ) {
		f = XTestStruct(X3, &args) ;
		newArity = FunctorArity(f) - 1 ;
		Ensure( InRange(pos,1,newArity+1) ) ;
		t = MakeStruct(LookupFunctor(FunctorAtom(f), newArity), args) ;
		if( (n = newArity - pos) >= 0 ) {
			for( h = H - n ; n-- ; h++ )
				h[-1] = h[0] ;
			h[-1] = args[newArity] ;
		}
		MustBe( Unify(X1, t) && Unify(X2, args[pos-1]) ) ;
	}
	else {
		f = XTestStruct(t1, &args) ;
		newArity = FunctorArity(f) + 1 ;
		Ensure( InRange(pos,1,newArity) ) ;
		t = MakeStruct(LookupFunctor(FunctorAtom(f), newArity), args) ;
		n = newArity - pos ;
		for( h = H ; n-- ; h-- )
			h[-1] = h[-2] ;
		SafeAssignToGlobal(h-1, X2) ;
		MustBe( Unify(X3, t) ) ;
	}
}

static void PInsStart()
{
	Hdl args, h ;
	FunctorPt f = XTestStruct(X0, &args) ;
	int newArity = FunctorArity(f) + 1 ;
	Pt t = MakeStruct(LookupFunctor(FunctorAtom(f), newArity), args) ;
	int n = newArity - 1 ;
	for( h = H ; n-- ; h-- )
		h[-1] = h[-2] ;
	SafeAssignToGlobal(h-1, X1) ;
	MustBe( Unify(X2, t) ) ;
}

static void PInsEnd()
{
	Hdl args ;
	FunctorPt f = XTestStruct(X0, &args) ;
	int newArity = FunctorArity(f) + 1 ;
	Pt t = MakeStruct(LookupFunctor(FunctorAtom(f), newArity), args) ;
	SafeAssignToGlobal(H-1, X1) ;
	MustBe( Unify(X2, t) ) ;
}

static void PAcyclicTerm()
{
	MustBe( !TermIsCyclic(X0) ) ;
}

static void PCyclicTerm()
{
	MustBe( TermIsCyclic(X0) ) ;
}

static void PCopyTerm()
{
	Pt t = ZPushTerm(X0) ; /* stacks may grow */
	MustBe( Unify(X1, t) ) ;
}

static void PCopyTermWithEnv()
{
	Pt t = ZPushTermWithEnv(X0, X1, false) ; /* stacks may grow */
	MustBe( Unify(X2, t) ) ;
}

static void PTermVariables()
{
	Size nVars ;
	TermVars(X0, &nVars) ;
	ZEnsureFreeSpaceOnStacks(2 * nVars, -1, false) ; /* stacks may grow */
	MustBe( Unify(X1, TermVars(X0, nil)) ) ;
}

static void PTermHidePAR()
{
	Pt t = Drf(X0) ;
	if( IsVar(t) )
		MustBe( Unify(X0, TermShowPAR(X1)) ) ;
	else		
		MustBe( Unify(X1, TermHidePAR(X0)) ) ;
}

static void PSubTerm(void)
{
	MustBe( TermSubTerm(X0, X1) ) ;
}

static void PTermSize(void)
{
	register Pt t ;
	int size = 1 ;
	VarValue2(t, X0) ;
	size = IsCompound(t) ? CompoundTermSize(t, false) : 1 ;
	MustBe( UnifyWithNumber(X1, MakeInt(size)) ) ;
}

static Bool Subsumes(Pt general, Pt specific) {
	Pt vs = TermVars(specific, nil) ; /* deferred list */
	if( !UnifyWithOccursCheck(general, specific) )
		return false ;
	Hdl saveTR = TR ;
	for( ; IsList(vs) ; vs = XListTail(vs) ) {
		Pt  v = Drf(XListHead(vs)) ;
		if( IsVar(v) )
			Assign(v, tNilAtom)
		else
			break ;		
	}
	TrailRestore(saveTR) ;
	return vs == tNilAtom ;
}

static void PSubsumesTerm(void)
{
	Hdl saveH = H, saveTR = TR ;
	Bool res = Subsumes(X0, X1) ;
	H = saveH ;
	TrailRestore(saveTR) ;
	MustBe( res ) ;
}

static void PSubsumes(void)
{
	MustBe( Subsumes(X0, X1) ) ;
}

static Bool AtomOrNumber_CodesOrChars(Bool any, Bool atom, Bool codes)
{
	Pt t0 = Drf(X0) ;
	if( IsAtom(t0) && (any || atom) ) {
		Str s = XAtomName(t0) ;
		ZEnsureFreeSpaceOnStacks(2 * CharLen(s), -1, false) ; /* stacks may grow */
		return Unify(X1, codes ? StringToPString(s) : StringToAString(s)) ;
	}
	if( IsNumber(t0) && (any || !atom) ) {
		Str s = XNumberAsStr(t0) ;
		ZEnsureFreeSpaceOnStacks(2 * CharLen(s), -1, false) ; /* stacks may grow */
		return Unify(X1, codes ? StringToPString(s) : StringToAString(s)) ;
	}
	elif( IsVar(t0) ) {
		Pt t1 = Drf(X1) ;
		if( IsList(t1) || t1 == tNilAtom ) {
			Str s = codes ? PStringToString(t1) : AStringToString(t1) ;
			if( any ) {
				t1 = NumberFromStr(s) ;
				if( t1 == nil ) t1 = MakeTempAtom(s) ;
			}
			else t1 = atom ? MakeTempAtom(s) : NumberFromStr(s) ;
			return t1 != nil && UnifyWithAtomic(t0, t1) ;
		}
		else return ITypeError("LIST", t1) ;
	}
	else return ITypeError("ATOM", t0) ;
}

static void PName()
{
	MustBe( AtomOrNumber_CodesOrChars(true, true, true) ) ;
}

static void PAtomCodes()
{
	MustBe( AtomOrNumber_CodesOrChars(false, true, true) ) ;
}

static void PNumberCodes()
{
	MustBe( AtomOrNumber_CodesOrChars(false, false, true) ) ;
}

static void PAtomChars()
{
	MustBe( AtomOrNumber_CodesOrChars(false, true, false) ) ;
}

static void PNumberChars()
{
	MustBe( AtomOrNumber_CodesOrChars(false, false, false) ) ;
}

static void PNumberVars()
{
	MustBe( UnifyWithAtomic(X2, MakeInt(NumberVars(X0, XTestInt(X1)))) ) ;
}

static void PSlice()
{
	Str s = XTestAtomName(X0) ;
	PInt i0 = XTestInt(X1) ;
	PInt i1 = XTestInt(X2) ;
	if( i0 > 0 && i1 > 0 ) ;
	elif( i0 == 0 || i1 == 0 )
		ArithError("The two integer arguments of slice/4 must be non-zero") ;
	else {
		PInt len = CharLen(s) ;
		if( i0 < 0 ) i0 += len + 1 ;
		if( i1 < 0 ) i1 += len + 1 ;
		if( i0 <= 0 && i1 <= 0 )
			MustBe( UnifyWithAtomic(X3, tEmptyAtom) ) ;
		if( i0 <= 0 ) i0 = 1 ;
		if( i1 <= 0 ) i1 = 1 ;
	}
	BigStrOpen() ;
	BigStrAddStrSlice(s, i0-1, i1) ;
	MustBe( Unify(X3, MakeTempAtom(BigStrClose())) ) ;
}

static void PConcat()
{
	MustBe( Unify(X1, MakeTempAtom(TermsAsStrN(X0))) ) ;
}

static void PConcatRev()
{
	MustBe( Unify(X0, MakeTempAtom(TermsAsStrN(X1))) ) ;
}

#if COMPASS
static void PUnique()
{
	static Word w = -2 ;
	Str s = GStrFormat("$%%u%lu", ++w) ;
	MustBe( Unify(X0, MakeTempAtom(s)) ) ;
}
#endif

void TermsInit()
{
	termSegm = Allocate(termSegmSize, false) ; /* must have alloc addr */
	GetCodeFreeVar() ;
	InterVarTableInit() ;
	InstallRelocateStacksHandler("DrfCheckedLimits", RelocateDrfCheckedLimits) ;

	InstallCBuiltinPred("var", 1, PVar) ;
	InstallCBuiltinPred("nonvar", 1, PNonVar) ;
	InstallCBuiltinPred("atom", 1, PAtom) ;
	InstallCBuiltinPred("integer", 1, PPInt) ;
	InstallCBuiltinPred("float", 1, PPFloat) ;
	InstallCBuiltinPred("number", 1, PNumber) ;
	InstallCBuiltinPred("atomic", 1, PAtomic) ;
	InstallCBuiltinPred("ground", 1, PGround) ;
	InstallCBuiltinPred("compound", 1, PCompound) ;
	InstallCBuiltinPred("callable", 1, PCallable) ;
	InstallCBuiltinPred("is_list", 1, PIsList) ;

	InstallCBuiltinPred("functor", 3, PFunctor) ;
	InstallCBuiltinPred("arg", 3, PArg) ;
	InstallCBuiltinPred("ins", 4, PIns) ;
	InstallCBuiltinPred("ins_start", 3, PInsStart) ;
	InstallCBuiltinPred("ins_end", 3, PInsEnd) ;

	InstallCBuiltinPred("acyclic_term", 1, PAcyclicTerm) ;
	InstallCBuiltinPred("cyclic_term", 1, PCyclicTerm) ;
	InstallCBuiltinPred("copy_term", 2, PCopyTerm) ;
	InstallCBuiltinPred("copy_term", 3, PCopyTermWithEnv) ;
	InstallCBuiltinPred("term_variables", 2, PTermVariables) ;
	InstallCBuiltinPred("hide_par", 2, PTermHidePAR) ;
	InstallCBuiltinPred("subterm", 2, PSubTerm) ;
	InstallCBuiltinPred("term_size", 2, PTermSize) ; /* for debug */
	InstallCBuiltinPred("subsumes_term", 2, PSubsumesTerm) ;
	InstallCBuiltinPred("subsumes_chk", 2, PSubsumesTerm) ;
	InstallCBuiltinPred("subsumes", 2, PSubsumes) ;

	InstallCBuiltinPred("name", 2, PName) ;
	InstallCBuiltinPred("atom_codes", 2, PAtomCodes) ;
	InstallCBuiltinPred("number_codes", 2, PNumberCodes) ;
	InstallCBuiltinPred("atom_chars", 2, PAtomChars) ;
	InstallCBuiltinPred("number_chars", 2, PNumberChars) ;

	InstallCBuiltinPred("numbervars", 3, PNumberVars) ;
	InstallCBuiltinPred("slice", 4, PSlice) ;
	InstallCBuiltinPred("concat", 2, PConcat) ;
	InstallCBuiltinPred("===", 2, PConcatRev) ;
	
	InstallCBuiltinPred("$$_serialize_test", 2, PSerializeTest) ;

#if COMPASS
	InstallCBuiltinPred("unique", 1, PUnique) ;
#endif
}
