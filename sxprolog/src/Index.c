/*
 *   This file is part of the CxProlog system

 *   Index.c
 *   by A.Miguel Dias - 1990/01/26
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

/*
 *  INDEX RULES:
 *  When first called, every predicate is assumed to be static and its
 *  index is built. But if the predicate is changed afterwards
 *  (using retract or assert) then the predicate is now considered dynamic and
 *  its index is removed. A dynamic predicate has no index because it would be
 *	too inefficient to keep rebuilding the index again and again.
*/

#include "CxProlog.h"

#define initialIndexTableCapacity	256

typedef struct {
	Pt idxArg ;
	ClausePt first ;
	short nClauses ;
	short version ;
} IndexElem, *IndexElemPt ;

#define IsFreeIndexElem(ie)		( (ie)->version != currVersion )

#define IndexTableWords(capacity)	((capacity) * WordsOf(IndexElem))

static int indexTableCapacity ;
static IndexElemPt indexTable ;
static IndexElem lists, vars ;
static int currVersion, currArg ;

static void IndexTableInit()
{
	register IndexElemPt el ;
	dotable(el, indexTable, indexTableCapacity)
		el->version = 0 ;
	lists.version = 0 ;
	vars.version = 0 ;
	currVersion = 1 ;
}

static void IndexTableExpand()
{
	int oldCapacity = indexTableCapacity ;
	int newCapacity = oldCapacity * 2 ;
	MemoryGrowInfo("index table", IndexTableWords(oldCapacity), IndexTableWords(newCapacity)) ;
	Release(indexTable, IndexTableWords(oldCapacity)) ;
	indexTable = Allocate(IndexTableWords(newCapacity), false) ;
	indexTableCapacity = newCapacity ;
	IndexTableInit() ;
}

static void IndexTableRefresh()
{
	if( ++currVersion == 0 )
		IndexTableInit() ;
}

static int Hash(Pt t)
{
	return ( cWord(t) >> 2 ) % indexTableCapacity ;
}

static IndexElemPt FindEntry(register Pt t)
{
	register int i, h = Hash(t) ;
	for( i = h ; i < indexTableCapacity ; i++ )
		if( IsFreeIndexElem(&indexTable[i]) || indexTable[i].idxArg == t )
			return &indexTable[i] ;
	for( i = 0 ; i < h ; i++ )
		if( IsFreeIndexElem(&indexTable[i]) || indexTable[i].idxArg == t )
			return &indexTable[i] ;
	return nil ;
}

static void InsertIndexElem(IndexElemPt ie, ClausePt cl, Pt t)
{
	if( IsFreeIndexElem(ie) ) {
		ie->idxArg = t ;
		ie->first = cl ;
		ie->nClauses = 1 ;
		ie->version = currVersion ;
	}
	else
		ie->nClauses++ ;
}

#if unused
static void IndexTableList()
{
	register IndexElemPt el ;
	dotable(el, indexTable, indexTableCapacity)
		if( !IsFreeIndexElem(el) )
			Write("%s %d\n", TermTypeStr(el->idxArg), el->nClauses) ;
}
#endif

static ClausePt FindMatch(ClausePt cl, Pt t)
{
	for( ;
		!(IsVar(ClauseIdxInfo(cl,currArg)) || ClauseIdxInfo(cl,currArg) == t) ;
			cl = ClauseNext(cl) ) ;
	return cl ;
}

static Hdl GenIndexChain(ClausePt clauses, IndexElemPt ie, int nVarClauses)
{
	int nClauses = ie->nClauses + nVarClauses ;
	ClausePt cl = nVarClauses == 0 ? ie->first
							: FindMatch(clauses, ie->idxArg) ;

	if( nClauses == 1 )
		return ClauseCodeSkipHeader(cl) ;
	else {
		Hdl res = ScratchCurr() ;
		Gen1(Try, ClauseCodeSkipHeader(cl)) ;
		while( --nClauses > 1 ) {
			cl = FindMatch(ClauseNext(cl), ie->idxArg) ;
			Gen1(Retry, ClauseCodeSkipHeader(cl)) ;
		}
		cl = FindMatch(ClauseNext(cl), ie->idxArg) ;
		Gen1(Trust, ClauseCodeSkipHeader(cl)) ;
		return res ;
	}
}

static Size UpPower2(Size n)
{
	int p = 2 ;
	while( p < n ) p <<= 1 ;
	return p ;
}

#if unused
static void PrologHashList(PrologHashTable ph, int size)
{
	PrologHashTable el ;
	dotable(el, ph, size)
		if( el->value != nil ) {
			if( IsAtomic(el->value) )
				Write("\t\t%s %lx\n", TermAsStr(el->value), el->address) ;
			else
				Write("\t\t%s %lx\n",
						FunctorNameArity(cFunctorPt(el->value)),
						el->address) ;
		}
}
#endif

static void InitPrologHash(PrologHashElemPt ph, int size)
{
	register PrologHashElemPt el ;
	dotable(el, ph, size) {
		el->value = nil ;
		el->address = nil ;
		el->next = nil ;
	}
}

static void PrologHashInsert(PrologHashTable ph, int size, Pt t, Hdl addr)
/* pre: t not in indexTable */
{
	register PrologHashElemPt h = ph + PrologHash(t,size), x ;

	if( h->value == nil ) {
		h->value = t ;
		h->address = addr ;
		return ;
	}
	dotable(x, ph, size)
		if( x->value == nil ) {
			x->next = h->next ;
			h->next = x ;
			x->value = t ;
			x->address = addr ;
			return ;
		}
	InternalError("PrologHashInsert") ;
}

static Hdl GenConstTableCode(ClausePt clauses, int nConsts,
								int nVarClauses, Hdl onlyVarChain)
{
	register IndexElemPt el ;
	Size hTSize = UpPower2(nConsts) ;
	PrologHashElemPt hTable ;
	Hdl res = ScratchCurr() ;

	Gen2(SwitchOnAtomic, hTSize, onlyVarChain) ;
	hTable = (PrologHashElemPt)(ScratchCurr()) ;
	ScratchMakeRoom(hTSize * WordsOf(PrologHashElem)) ;
	if( ScratchHasMoved() ) return nil ;
	InitPrologHash(hTable, hTSize) ;

	dotable(el, indexTable, indexTableCapacity)
		if( !IsFreeIndexElem(el) && IsAtomic(el->idxArg) ) {
			Hdl chain = GenIndexChain(clauses, el, nVarClauses) ;
			if( ScratchHasMoved() ) return nil ;
			PrologHashInsert(hTable, hTSize, el->idxArg, chain) ;
		}
	return res ;
}

static Hdl GenConstSingleEntryCode(ClausePt clauses, int nVarClauses)
{
	register IndexElemPt el ;
	dotable(el, indexTable, indexTableCapacity)
		if( !IsFreeIndexElem(el) && IsAtomic(el->idxArg) )
			return GenIndexChain(clauses, el, nVarClauses) ;
	return InternalError("GenConstSingleEntryCode") ;
}

static Hdl GenStructTableCode(ClausePt clauses, int nStructs,
									int nVarClauses, Hdl onlyVarChain)
{
	register IndexElemPt el ;
	Size hTSize = UpPower2(nStructs) ;
	PrologHashElemPt hTable ;
	Hdl res = ScratchCurr() ;

	Gen2(SwitchOnStructure, hTSize, onlyVarChain) ;
	hTable = (PrologHashElemPt)(ScratchCurr()) ;
	ScratchMakeRoom(hTSize * WordsOf(PrologHashElem)) ;
	if( ScratchHasMoved() ) return nil ;
	InitPrologHash(hTable, hTSize) ;

	dotable(el, indexTable, indexTableCapacity)
		if( !IsFreeIndexElem(el) && IsStruct(el->idxArg) ) {
			Hdl chain = GenIndexChain(clauses, el, nVarClauses) ;
			if( ScratchHasMoved() ) return nil ;
			PrologHashInsert(hTable, hTSize, XPt(el->idxArg), chain) ;
		}
	return res ;
}

static Hdl GenStructSingleEntryCode(ClausePt clauses, int nVarClauses)
{
	register IndexElemPt el ;
	dotable(el, indexTable, indexTableCapacity)
		if( !IsFreeIndexElem(el) && IsStruct(el->idxArg) )
			return GenIndexChain(clauses, el, nVarClauses) ;
	return InternalError("GenStructSingleEntryCode") ;
}

static Bool GenIndexSegment(int argN, ClausePt clauses, Hdl switchArgs, Hdl ifVarChain)
{
	IndexElemPt ie ;
	register ClausePt cl ;
	int nClauses, nConsts, nStructs ;
	int nListClauses, nVarsClauses ;
	Hdl onlyVarChain ;

redo:
	IndexTableRefresh() ;
	currArg = argN ;
	nClauses = nConsts = nStructs = 0 ;
	nListClauses = nVarsClauses = 0 ;
	doseq(cl, clauses, ClauseNext(cl)) {
		register Pt t = ClauseIdxInfo(cl,currArg) ;
		if( IsVar(t) ) {
			InsertIndexElem(&vars, cl, cPt(-1)) ;
			nVarsClauses++ ;
		}
		elif( IsStruct(t) ) {
			if( (ie = FindEntry(t)) == nil ) {
				IndexTableExpand() ;
				goto redo ;
			}
			InsertIndexElem(ie, cl, t) ;
			if( ie->nClauses == 1 )
				nStructs++ ;
		}
		elif( IsList(t) ) {
			InsertIndexElem(&lists, cl, TagList(nil)) ;
			nListClauses++ ;
		}
		else {
			if( (ie = FindEntry(t)) == nil ) {
				IndexTableExpand() ;
				goto redo ;
			}
			InsertIndexElem(ie, cl, t) ;
			if( ie->nClauses == 1 )
				nConsts++ ;
		}
		nClauses++ ;
	}
	if( (nClauses - nVarsClauses) < 2
		|| (nClauses - nVarsClauses) < nVarsClauses )
			return false ;

	/* Is onlyVarChain needed? */
	if( (nConsts != 1 || nListClauses == 0 || nStructs != 1) && nVarsClauses > 0 )
		onlyVarChain = GenIndexChain(clauses, &vars, 0) ;
	else onlyVarChain = FailAddr ;

	switchArgs[0] = cPt(
		nConsts == 0
			? onlyVarChain :
		nConsts == 1
			? GenConstSingleEntryCode(clauses, nVarsClauses)
			: GenConstTableCode(clauses, nConsts, nVarsClauses, onlyVarChain)) ;

	switchArgs[1] = cPt(
		nListClauses == 0
			? onlyVarChain
			: GenIndexChain(clauses, &lists, nVarsClauses)) ;

	switchArgs[2] = cPt(
		nStructs == 0
			? onlyVarChain :
		nStructs == 1
			? GenStructSingleEntryCode(clauses, nVarsClauses)
			: GenStructTableCode(clauses, nStructs, nVarsClauses, onlyVarChain)) ;

	switchArgs[3] = cPt(ifVarChain) ;

	return true ;
}

static void DoNoIndex(PredicatePt pr)
{
	if( PredHasOneClause(pr) )
		BindPredicateWithLocalJump(pr, ClauseCodeSkipHeader(PredClauses(pr))) ;
	else
		BindPredicateWithLocalJump(pr, ClauseCode(PredClauses(pr))) ;
}


/* Public */

void IndexInit()
{
	indexTable = Allocate(IndexTableWords(initialIndexTableCapacity), false) ;
	indexTableCapacity = initialIndexTableCapacity ;
	IndexTableInit() ;
}

void NIndexableUpdateFlag(int newValue)
{
	if( indexDebugging_flag )
		newValue = 0 ;
	nIndexable_flag = newValue ;
}

void DoIndex(PredicatePt pr) /* pre: pr has clauses */
{
	Hdl prevSegm, currSegm ;
	Bool hasIndex ;
	
	if( !PredIsIndexable(pr) ) {
		DoNoIndex(pr) ;
		return ;
	}

#if 0
	Mesg("INDEX: %s", PredNameArity(pr)) ;
#endif

	UseScratch() ;

redo:
	ScratchReset() ;
	ScratchRegister() ;
	hasIndex = false ;
	currSegm = ClauseCode(PredClauses(pr)) ;

	if( PredNIndexable(pr) >= 3 && PredArity(pr) >= 3 ) {
		prevSegm = currSegm ;
		currSegm = ScratchCurr() ;
		Gen4(SwitchOnTerm2, 0, 0, 0, 0) ;
		if( GenIndexSegment(2, PredClauses(pr), ScratchCurr() - 4, prevSegm) )
			hasIndex = true ;
		else {
			ScratchBack(5) ;
			currSegm = prevSegm ;
		}
		if( ScratchHasMoved() ) goto redo ;
	}

	if( PredNIndexable(pr) >= 2 && PredArity(pr) >= 2 ) {
		prevSegm = currSegm ;
		currSegm = ScratchCurr() ;
		Gen4(SwitchOnTerm1, 0, 0, 0, 0) ;
		if( GenIndexSegment(1, PredClauses(pr), ScratchCurr() - 4, prevSegm) )
			hasIndex = true ;
		else {
			ScratchBack(5) ;
			currSegm = prevSegm ;
		}
		if( ScratchHasMoved() ) goto redo ;
	}

	prevSegm = currSegm ;
	currSegm = ScratchCurr() ;
	BindPredicateFull(pr, SwitchOnTerm0, nil, nil, nil, nil) ;
	if( GenIndexSegment(0, PredClauses(pr), PredStartInstArgs(pr), prevSegm) )
		hasIndex = true ;
	elif( hasIndex )
		BindPredicateWithLocalJump(pr, prevSegm) ;
	if( ScratchHasMoved() ) goto redo ;

	if( hasIndex ) {
		Gen0(Proceed) ;		/* ListCode requires this as a terminator */
		if( ScratchHasMoved() ) goto redo ;
		if( ScratchUsed() > 1 ) {
			int i ;
			PredIndex(pr) = Allocate(ScratchUsed(), true) ;
			CopyWordsReloc(PredIndex(pr), ScratchStart(), ScratchUsed()) ;
			dotimes(i, 4)
				if( InRange(PredStartInstArgs(pr)[i],
							cPt(ScratchStart()), cPt(ScratchCurr())) )
					PredStartInstArgs(pr)[i] +=
							PredIndex(pr) - ScratchStart() ;
			}
	}
	else DoNoIndex(pr) ;

	FreeScratch() ;
}
