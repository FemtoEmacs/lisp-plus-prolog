/*
 *   This file is part of the CxProlog system

 *   Dict.c
 *   by A.Miguel Dias - 2000/09/29
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

/* DICT */

typedef struct DEntry {
	Pt key ;
	Pt value ;
} DEntry, *DEntryPt ;

typedef struct Dict {
	ExtraDef(Dict) ;
	DEntryPt begin, end, first, last ;
} Dict ;

#define cDictPt(x)				((DictPt)(x))

#define DictBegin(d)			((d)->begin)
#define DictEnd(d)				((d)->end)
#define DictFirst(d)			((d)->first)
#define DictLast(d)				((d)->last)

static ExtraTypePt dictType ;


/* PRIVATE FUNCTIONS */

#define DictWords(capacity)		((capacity) * WordsOf(DEntry))

static Size DictCapacity(DictPt d)
{
	return DictEnd(d) - DictBegin(d) ;
}

static void DictInit(DictPt d, Size capacity)
{
	DictBegin(d) = Allocate(DictWords(capacity), false) ;
	DictEnd(d) = DictBegin(d) + capacity ;
	DictFirst(d) = DictBegin(d) ;
	DictLast(d) = DictBegin(d) ;
}

static void DictExpand(DictPt d, DEntryPt *rel)
{
	DEntryPt b = DictBegin(d) ;
	DEntryPt f = DictFirst(d) ;
	DEntryPt l = DictLast(d) ;
	DEntryPt h ;
	Size oldCapacity = DictCapacity(d) ;
	DictInit(d, 2 * oldCapacity) ;
	for( h = DictBegin(d) ; f < l ; *h++ = *f++ ) ;
	DictLast(d) = h ;
	Release(b, DictWords(oldCapacity)) ;
	*rel = DictBegin(d) + (*rel - b) ;
}

static void DictDisable(DictPt d)
{
	if( ExtraDisable(d) ) {
		DictClear(d) ;
		Release(DictBegin(d), DictWords(DictCapacity(d))) ;
	}
}

static void DictOpenSpace(DictPt d, DEntryPt *here)
{
	register DEntryPt h ;
	if( DictLast(d) == DictEnd(d) )
		DictExpand(d, here) ;
	DictLast(d)++ ;
	for( h = DictLast(d) - 1 ; h > *here ; h-- )
		h[0] = h[-1] ;
}

static void DictCloseSpace(DictPt d, DEntryPt here)
{
	register DEntryPt h ;
	for( h = here + 1 ; h < DictLast(d) ; h++ )
		h[-1] = h[0] ;
	DictLast(d)-- ;
}

static Bool DictFind(DictPt d, Pt k, DEntryPt *here)
{
	register DEntryPt lo, hi, middle ;
	int res ;
	lo = DictFirst(d) ;
	hi = DictLast(d) - 1 ;
	while( lo <= hi ) {
		middle = lo + (hi - lo) / 2 ;
		if( (res = Compare(middle->key, k)) == 0 )
			{ *here = middle ; return true ; }
		elif( res < 0 )
			lo = middle + 1 ;
		else
			hi = middle - 1 ;
	}
	*here = lo ;
	return false ;
}

static void DictWrite(StreamPt stm, DictPt d)
{
	register DEntryPt h ;
	StreamWrite(stm, "%s", ExtraAsStr(d)) ;
	StreamWrite(stm, "     (current capacity %ld)\n", DictCapacity(d)) ;
	for( h = DictFirst(d) ; h < DictLast(d) ; h++ ) {
		StreamWrite(stm, "\t%s -- ", TermAsStr(h->key)) ;
		StreamWrite(stm, "%s\n", TermAsStr(h->value)) ;
	}
}

static Size DictSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(Dict) ;
}

static void DictBasicGCMarkContents(VoidPt x)
{
	DictPt d = cDictPt(x) ;
	register DEntryPt h ;
	for( h = DictFirst(d) ; h < DictLast(d) ; h++ ) {
		TermBasicGCMark(h->key) ;
		TermBasicGCMark(h->value) ;
	}
}

static Bool DictBasicGCDelete(VoidPt x)
{
	DictPt d = cDictPt(x) ;
	DictDisable(d) ;
	return true ;
}


/* MAIN OPERATIONS */

Size DictSize(DictPt d)
{
	return DictLast(d) - DictFirst(d) ;
}

DictPt DictNew()
{
	DictPt d = ExtraNew(dictType, 0) ;
	DictInit(d, 4) ;
	return d ;
}

void DictClear(DictPt d)
{
	register DEntryPt h ;
	for( h = DictFirst(d) ; h < DictLast(d) ; h++ ) {
		ReleaseTerm(h->key) ;
		ReleaseTerm(h->value) ;
	}
	DictLast(d) = DictFirst(d) ;
}

void DictFilter(DictPt d, BFunVV filter, VoidPt x)
{
	DEntryPt a, z, l = DictLast(d) ;
	for( a = z = DictFirst(d) ; a < l ; a++ )
		if( filter(a->value, x) )
			*z++ = *a ;
		else {
			ReleaseTerm(a->key) ;
			ReleaseTerm(a->value) ;
		}
	DictLast(d) = z ;
}

Bool DictGet(DictPt d, Pt k, Pt *t)
{
	DEntryPt here ;
	if( DictFind(d, k, &here) ) {
		*t = here->value ;
		return true ;
	}
	return false ;
}

void DictSet(DictPt d, Pt k, Pt t)
{
	DEntryPt here ;
	if( DictFind(d, k, &here) ) {
		ReleaseTerm(here->value) ;
		here->value = AllocateTermForAssign(t) ;
	}
	else {
		DictOpenSpace(d, &here) ;
		here->key = AllocateTermForAssign(k) ;
		here->value = AllocateTermForAssign(t) ;
	}
}

Bool DictDeleteItem(DictPt d, Pt k)
{
	DEntryPt here ;
	if( DictFind(d, k, &here) ) {
		ReleaseTerm(here->key) ;
		ReleaseTerm(here->value) ;
		DictCloseSpace(d, here) ;
		return true ;
	}
	return false ;
}


/* CXPROLOG C'BUILTINS */

static void PDictCheck()
{
	MustBe( XExtraCheck(dictType, X0) ) ;
}

static void PDictNew()
{
	BindVarWithExtra(X0, DictNew()) ;
	JumpNext() ;
}

static void PDictClear()
{
	DictClear(XTestExtra(dictType,X0)) ;
	JumpNext() ;
}

static void PDictDelete()
{
	DictPt d = XTestExtra(dictType,X0) ;
	DictDisable(d) ;
	JumpNext() ;
}

static void PDictSet()
{
	DictSet(XTestExtra(dictType,X0), TestNonVar(X1), X2) ;
	JumpNext() ;
}

static void PDictGet()
{
	Pt t ;
	Ensure( DictGet(XTestExtra(dictType,X0), TestNonVar(X1), &t) ) ;
	t = ZPushTerm(t) ; /* stacks may grow */
	MustBe( Unify(X2, t) ) ;
}

static void PDictDeleteItem()
{
	MustBe( DictDeleteItem(XTestExtra(dictType,X0), TestNonVar(X1)) ) ;
}

static void PDictAsList()
{
	DictPt d = XTestExtra(dictType,X0) ;
	DEntryPt h ; Pt t ;
	termSegm[0] = cPt(hifenFunctor) ;
	Z.t = tNilAtom ;
	for( h = DictLast(d) - 1 ; h >= DictFirst(d) ; h-- ) {
		termSegm[1] = h->key ;
		termSegm[2] = h->value ;
		t = ZPushTerm(TagStruct(termSegm)) ; /* stacks may grow */
		Z.t = MakeList(t, Z.t) ;
	}
	MustBe( Unify(Z.t, X1) ) ;
}

static void PDictWrite()
{
	DictWrite(currOut, XTestExtra(dictType,X0)) ;
	JumpNext() ;
}

static void PSDictWrite()
{
	DictWrite(XTestStream(X0, mWrite), XTestExtra(dictType,X1)) ;
	JumpNext() ;
}

static void PNDCurrentDict()
{
	ExtraPNDCurrent(dictType, nil, 1, 0) ;
	JumpNext() ;
}

static Size DictsAux(CVoidPt x)
{
	DictPt d = cDictPt(x) ;
	Write("size(%ld), capacity(%ld)", DictSize(d), DictCapacity(d)) ;
	return 1 ;
}
static void PDicts()
{
	ExtraShow(dictType, DictsAux) ;
	JumpNext() ;
}



/* TEST, EXTRACT & INIT */

Bool IsDict(Pt t)
{
	return IsThisExtra(dictType, t) ;
}

DictPt XTestDict(Pt t)
{
	return XTestExtra(dictType, t) ;
}

void DictsInit()
{
	dictType = ExtraTypeNew("DICT", DictSizeFun, DictBasicGCMarkContents, DictBasicGCDelete, 1) ;
	/* add "dicts." to CxProlog.c/PShow */

	InstallCBuiltinPred("dict", 1, PDictCheck) ;
	InstallCBuiltinPred("dict_new", 1, PDictNew) ;
	InstallCBuiltinPred("dict_clear", 1, PDictClear) ;
	InstallCBuiltinPred("dict_delete", 1, PDictDelete) ;
	InstallCBuiltinPred("dict_set", 3, PDictSet) ;
	InstallCBuiltinPred("dict_get", 3, PDictGet) ;
	InstallCBuiltinPred("dict_delete_item", 2, PDictDeleteItem) ;
	InstallCBuiltinPred("dict_as_list", 2, PDictAsList) ;
	InstallCBuiltinPred("dict_write", 1, PDictWrite) ;
	InstallCBuiltinPred("dict_write", 2, PSDictWrite) ;
	InstallGNDeterCBuiltinPred("current_dict", 1, 2, PNDCurrentDict) ;
	InstallCBuiltinPred("dicts", 0, PDicts) ;
}
