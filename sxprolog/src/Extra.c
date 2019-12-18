/*
 *   This file is part of the CxProlog system

 *   Extra.c
 *   by A.Miguel Dias - 2002/01/01
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

#define XExtraIsHidden(x)			ExtraIsHidden(XExtra(t))


/* EXTRA-TYPES TABLE */

#define maxExtraTypes				16

#define ExtraTypeOf(x)				(allExtraTypes + ExtraTag(x))


static ExtraType allExtraTypes[maxExtraTypes] ;
static int nExtraTypes = 0 ;

ExtraTypePt ExtraTypeNew(Str name, ExtraFun sizeFun,
							FunV gcMarkFun, BFunV gcDeleteFun, Size htSize)
{
	if( testGCollection_flag == 2 )
		Mesg("%d %s", nExtraTypes, name) ;
	if( nExtraTypes < maxExtraTypes ) {
		register ExtraTypePt e = allExtraTypes + nExtraTypes ;
		register int i ;
		ExtraTypeTag(e) = nExtraTypes ;
		ExtraTypeName(e) = name ;
		ExtraTypeSizeFun(e) = sizeFun ;
		ExtraTypeGCMarkFun(e) = gcMarkFun ;
		ExtraTypeGCDeleteFun(e) = gcDeleteFun ;
		ExtraTypeHashTable(e) = Allocate(htSize, false) ;
		dotimes(i, htSize)
			ExtraTypeHashTable(e)[i] = nil ;
		ExtraTypeHashTableSize(e) = htSize ;
		ExtraTypeSupportAliasing(e) = true ;
		nExtraTypes++ ;
		return e ;
	}
	return InternalError("Too many EXTRA TYPES: increase the table capacity") ;
}

void ExtraTypeDoesNotSupportAliasing(ExtraTypePt e)
{
	ExtraTypeSupportAliasing(e) = true ;
}


/* EXTRA-TERMS MANAGEMENT */

#define doextra_slot(e, s, x)									\
		doseq(x, ExtraTypeHashTableSlot(e,s), ExtraNext(x))

#define doextra_slot_cont(xs, x)								\
		doseq(x, ExtraNext(xs), ExtraNext(x))

#define doextra(e, i, x)										\
	dotimes(i, ExtraTypeHashTableSize(e))						\
		doextra_slot(e, i, x)

#define doextra2(e, i, z, x)									\
	dotimes(i, ExtraTypeHashTableSize(e))						\
		for( z = &ExtraTypeHashTableSlot(e,i) ; (x = *z) != nil ; )

static Bool separatorWritten ;

static Bool ExtraCheck(ExtraTypePt e, VoidPt ref)
{
	int i ;
	register ExtraPt x ;
	doextra(e, i, x)
		if( x == ref )
			return true ;
	return false ;
}

Bool IsThisExtra(ExtraTypePt e, register Pt t)
{
	VarValue(t) ;
	return IsExtra(t) && XExtraTag(t) == ExtraTypeTag(e) && !XExtraIsHidden(e) ;
}

VoidPt ExtraNewWithSize(ExtraTypePt e, Size size, int slot)
{
/* Allocates at even word boundary, allowing 3 bits for the extra-tag */
	ExtraPt x = Allocate(size, false) ;
	ExtraPt *z = (ExtraPt *)(&ExtraTypeHashTable(e)[slot]) ;
/* Link in the apropriate slot */
	x->next = *z ;
	*z = x ;
/* Initialize */
	ExtraTag(x) = ExtraTypeTag(e) ;
	ExtraIsDisabled(x) = false ;
	ExtraIsHidden(x) = false ;
	ExtraIsPermanent(x) = false ;
	ExtraIsSpecial(x) = false ;
	ExtraGCAddDelta(size) ;
	return x ;
}

VoidPt ExtraNew(ExtraTypePt e, int slot)
{
	return ExtraNewWithSize(e, ExtraTypeSizeFun(e)(nil), slot) ;
}

Pt TagExtraAuto(VoidPt x)
{
/* Must update if the representation of the extras change... */
	return(TagExtra(nil, x)) ;
}

static void ExtraRelease(ExtraTypePt e, VoidPt x)
{
	if( testGCollection_flag == 2 ) {
		if( !separatorWritten ) {
			Mesg("-------------------") ;
			separatorWritten = true ;
		}
		Mesg("%lx released %s '%s'", x, ExtraAsStr(x),
			IsAtom(TagAtom(x)) ? XAtomName(TagAtom(x)) :
			IsFloat(TagFloat(x)) ? XNumberAsStr(TagFloat(x)) : "") ;
	}
	Release(x, ExtraTypeSizeFun(e)(x)) ;
}

Bool ExtraDisable(VoidPt x)
{	/*	A disabled extra is an extra that has been deleted but remains
		alive because some other object is still pointing to it, preventing
		it from being garbage collected.
		Every disabled extra is automatically an hidden extra.
	*/
	if( ExtraIsDisabled(x) )
		return false ;
	if( ExtraIsPermanent(x) )
		ImperativeError("Attempt to delete the permanent object '%s'",
													ExtraAsStr(x)) ;
	ExtraIsDisabled(x) = true ;
	ExtraIsHidden(x) = true ;
	return true ;
}

void ExtraHide(VoidPt x)
{	/* An hidden extra is not seen by Prolog. Such an extra is intended
   to be used only in C-code. A disabled extra is also hidden. */
	ExtraIsHidden(x) = true ;
}

void ExtraPermanent(VoidPt x)
{
	ExtraIsPermanent(x) = true ;
}

void ExtraNotPermanent(VoidPt x)
{
	ExtraIsPermanent(x) = false ;
}

void ExtraSpecial(VoidPt x)
{
	ExtraIsSpecial(x) = true ;
}

Str ExtraAsStr(VoidPt x)
{
	ExtraTypePt e = ExtraTypeOf(x) ;
	if( ExtraIsDisabled(x) )
		return GStrFormat("1'%s_%lx_deleted", ExtraTypeName(e), cWord(x)) ;
	else
		return GStrFormat("1'%s_%lx", ExtraTypeName(e), cWord(x)) ;
}

VoidPt ExtraGetFirst(ExtraTypePt e)	/* pre: only one slot */
{		/* Apply to the non-hidden */
	register ExtraPt x ;
	if( ExtraTypeHashTableSize(e) != 1 )
		InternalError("ExtraGetFirst") ;
	doextra_slot(e, 0, x)
		if( !ExtraIsHidden(x) )
				return x ;
	return nil ;
}

VoidPt ExtraGetNext(VoidPt x)	/* pre: only one slot */
{		/* Apply to the non-hidden */
	register ExtraPt xs = x ;
	doextra_slot_cont(xs, x)
		if( !ExtraIsHidden(x) )
				return x ;
	return nil ;
}

Size ExtraForEach(ExtraTypePt e, ExtraFun fun)
{		/* Apply to the non-disabled */
	if( e == nil ) {	/* Do it for all extra types */
		int i ;
		Size n = 0 ;
		dotimes(i, nExtraTypes)
			n += ExtraForEach(allExtraTypes + i, fun) ;
		return n ;
	}
	else {				/* Do it only for the extra type e */
		int i ;
		Size n = 0 ;
		register ExtraPt x ;
		doextra(e, i, x)
			if( !ExtraIsDisabled(x) )
				n += fun(x) ;
		return n ;
	}
}

VoidPt ExtraFindFirst(ExtraTypePt e, int slot, ExtraCond cond, CVoidPt arg)
{		/* Apply to the non-disabled */
	if( slot == -1 ) {
		int i ;
		register ExtraPt x ;
		doextra(e, i, x)
			if( !ExtraIsDisabled(x) && cond(x, arg) )
				return x ;
	}
	else {
		register ExtraPt x ;
		doextra_slot(e, slot, x)
			if( !ExtraIsDisabled(x) && cond(x, arg) )
				return x ;
	}
	return nil ;
}

void ExtraPNDCurrent(ExtraTypePt e, BFunV bfun, int arity, int resPos)
{		/* Apply to the non-hidden. Immediate-update semantics. */
	int i ;
	ExtraPt x ;
	if( A(arity) == tNilAtom ) {
		Xc(resPos) = Drf(Xc(resPos)) ;
		if( IsVar(Xc(resPos)) ) {
			i = 0 ;
			x = ExtraTypeHashTableSlot(e, 0) ;
		}
		else {
			x = XTestExtraNoAlias(e, Xc(resPos)) ; /* @@@ */
			Discard() ;
			MustBe( !ExtraIsHidden(x) && (bfun == nil || bfun(x)) ) ;
		}
	}
	else {
		i = XInt(A(arity)) ;
		x = ExtraNext(A(arity+1)) ;
	}
	for(;;) {
/* if x == nil then must find next non-nil */
		while( x == nil && i < ExtraTypeHashTableSize(e) - 1 )
			x = ExtraTypeHashTableSlot(e, ++i) ;
/* in case of unsuccess, stop generation */
		if( x == nil )
			Jump(DiscardAndFail) ;
/* found OK element */
		if( !ExtraIsHidden(x) && (bfun == nil || bfun(x)) )
			break ;
/* last seen is not OK, so advance and start all over again */
		x = ExtraNext(x) ;
	}
	A(arity) = MakeInt(i) ;
	A(arity+1) = cPt(x) ;
	MustBe( Unify(Xc(resPos), TagExtra(e, x)) ) ;
}

void ExtraShow(ExtraTypePt e, ExtraFun fun)
{		/* Apply to the non-hidden */
	Size n = 0, temps = 0, perms = 0 ;
	int tableSize = ExtraTypeHashTableSize(e) ;
	int i ;
	register ExtraPt x ;
	ShowVersion() ;

/* show as list */
	Write("%sS:\n", ExtraTypeName(e)) ;
	doextra(e, i, x)
		if( !ExtraIsHidden(x) ) {
			Write("  %16s %c> ",
				ExtraAsStr(x),
				ExtraIsPermanent(x) ? '=' : '-') ;
			if( fun != nil )
				n += fun(x) ;
			AliasedWithWrite(TagExtra(e, x)) ;
			IVarsWithWrite(TagExtra(e, x)) ;
			Write("\n") ;
		}
	if( n == 0 ) Write("    %% None\n") ;

/* show as hash table sumary */
	if( ExtraTypeHashTableSize(e) == 1 ) JumpNext() ;
	Write("---\n%sS HASH-TABLE SUMMARY:\n", ExtraTypeName(e)) ;
	doextra(e, i, x)
		if( !ExtraIsHidden(x) ) {
			if( ExtraIsPermanent(x) ) perms++ ;
			else temps++ ;
		}
	Write("  %sS are stored in a %d-entry hash table:\n",
										ExtraTypeName(e), tableSize) ;
	Write("    Current number of permanent %sS in the hash table -> %7ld\n",
										ExtraTypeName(e), perms) ;
	Write("    Current number of temporary %sS in the hash table -> %7ld\n",
										ExtraTypeName(e), temps) ;
	Write("    Current average length of the hash chains ->           %.5f\n",
										(perms + temps)/(double)tableSize) ;
	Write("    Current length of the individual hash chains ->") ;
	dotimes(i, tableSize) {
		Size n = 0 ;
		doextra_slot(e, i, x)
			if( !ExtraIsHidden(x) ) n++ ;
		Write(" %d", n) ;
	}
	Write("\n") ;

#if 0
/* show as hash table details */
	Write("---\n%sS HASH-TABLE ENTRIES:\n", ExtraTypeName(e)) ;
	dotimes(i, ExtraTypeHashTableSize(e)) {
		Write("%d - ", i) ;
		doextra_slot(e, i, x)
			if( !ExtraIsHidden(x) )
				Write("%s%s ", ExtraIsPermanent(x) ? "=" : " ", ExtraAsStr(x)) ;
		Write("\n") ;
	}
#endif

	JumpNext() ;
}


/* BASIC GARBAGE COLLECTION */

#define deltaThreshold		(20 K)
#define maxGCHandlers		20

static Str GCHdlNames[maxGCHandlers] ;
static Fun GCHdl[maxGCHandlers] ;
static int nGCHdl = 0 ;
static Size gcDelta = 0 ;

void ExtraGCHandlerInstall(Str name, Fun p)
{
	if( nGCHdl == maxGCHandlers )
		InternalError("Too many GC handlers: increase the table capacity") ;
	GCHdlNames[nGCHdl] = name ;
	GCHdl[nGCHdl] = p ;
	nGCHdl++ ;;
#if 0
	Mesg("ExtraGCHandlerInstall -> %s", name) ;
#endif
}

void ExtraGCAddDelta(Size size)
{
	if( (gcDelta += size) >= deltaThreshold ) {
		Attention() = true ;
		gcDelta = deltaThreshold ;
	}
#if 0
	Mesg("gcDelta = %d  (%d)", gcDelta, size) ;
#endif
}

void ExtraGCMark(VoidPt x)
{
	if( !ExtraIsGCMarked(x) ) {
		ExtraTypePt e = ExtraTypeOf(x) ;
#if 0
		Mesg("marking %s", ExtraAsStr(x)) ;
#endif
		ExtraIsGCMarked(x) = true ;
		if( ExtraTypeGCMarkFun(e) != nil && !ExtraIsDisabled(x) )
			ExtraTypeGCMarkFun(e)(x) ;
	}
}

void ExtraGCMarkRange(register Hdl a, register Hdl z)
{
	for( ; a < z ; a++ ) {
		if( IsEmpty(*a) )
			/* Skip */ ;
		elif( IsExtra(*a) )
			ExtraGCMark(XExtra(*a)) ;
	}
}

int ExtraGCClearNotMarked(ExtraTypePt e)
{		/* Apply to all extras, even to the disabled */
	if( e == nil ) {	/* Do it for all extra types */
		Size n = 0 ;
		int i ;
		dotimes(i, nExtraTypes)
			n += ExtraGCClearNotMarked(allExtraTypes + i) ;
		return n ;
	}
	else {				/* Do it only for the extra type e */
		Size n = 0 ;
		if( ExtraTypeGCDeleteFun(e) != nil ) {
			int i ;
			register ExtraPt *z, x ;
			doextra2(e, i, z, x) {
				if( !ExtraIsGCMarked(x) && ExtraTypeGCDeleteFun(e)(x) ) {
					*z = ExtraNext(x) ;
					ExtraRelease(e, x) ;
					n++ ;
				}
				else z = &ExtraNext(x) ;
			}
		}
		if( n > 0 && testGCollection_flag == 0 )
			MemoryInfo("%d temporary %s%s removed by the garbage collector",
										n, ExtraTypeName(e), n>1 ? "S" : "") ;
		return n ;
	}
}

static void ExtraGCDoIt()
{
	int i, j ;
	register ExtraPt x ;
/* Unmark all */
	dotimes(i, nExtraTypes)
		doextra(allExtraTypes + i, j, x)
			ExtraIsGCMarked(x) = false ;
/* Mark roots */
	dotimes(i, nGCHdl)
		GCHdl[i]() ;
/* Mark permanent extras */
	dotimes(i, nExtraTypes) {
		doextra(allExtraTypes + i, j, x)
			if( ExtraIsPermanent(x) )
				ExtraGCMark(x) ;
	}
/* Clear extras not marked */
	ExtraGCClearNotMarked(nil) ;
/* Reset gcDelta */
	gcDelta = 0 ;
}

void ExtraGC()
{
	if( testGCollection_flag > 0 ) {
		Attention() = true ;	/* Force gc again and again */
		separatorWritten = false ;
		ExtraGCDoIt() ;
	}
	elif( gCollection_flag && gcDelta >= deltaThreshold )
		ExtraGCDoIt() ;
}

void GCollectionUpdateFlag(int newValue)
{
	if( gcDebugging_flag > 0 )
		newValue = 0 ;
	gCollection_flag = newValue ;
}


/* PT OPERATIONS */

VoidPt XTestExtraGen(ExtraTypePt e, register Pt t, Bool useAlias, Bool err)
{
	VarValue(t) ;
	if( IsAtom(t) && useAlias ) {
		VoidPt x ;
		if( (x = AliasGet(e, t)) != nil )
			t = TagExtra(e, x) ;
		elif( (x = IVarGet(XAtom(t))) != nil ) 
			t = x;
		elif( err )
			ImperativeError("Accessing undefined alias or ivar '%s'",
														XAtomName(t)) ;
		else return nil ;
	}

	if( IsExtra(t) && XExtraTag(t) == ExtraTypeTag(e) ) {
		ExtraPt x = XExtra(t) ;
		if( ExtraIsDisabled(x) )
			if( err ) 
				ImperativeError("Invalid operation over deleted object %s",
                                                    ExtraAsStr(x)) ;
			else return nil ;
		else if( ExtraIsHidden(x) )
			if( err )
				ImperativeError("%s object does not exist", ExtraAsStr(x)) ;
			else return nil ;
		else 
			return x ;
	}

	return err ? ExtraTypeError(e, nil, t) : nil ;
}

Bool XExtraCheck(ExtraTypePt e, register Pt t)
{
	return XTestExtraGen(e, t, true, false) != nil ;
}

VoidPt XTestExtra(ExtraTypePt e, Pt t)
{
	return XTestExtraGen(e, t, true, true) ;
}

VoidPt XTestExtraNoAlias(ExtraTypePt e, Pt t)
{
	return XTestExtraGen(e, t, false, true) ;
}

VoidPt XTestAnyExtra(register Pt t)
{
	VarValue(t) ;
	if( IsExtra(t) ) return XExtra(t) ;
	return TypeError("EXTRA", t) ;
}

VoidPt XTestAnyStrictExtra(register Pt t)
{
	VarValue(t) ;
	if( IsExtraStrict(t) ) return XExtra(t) ;
	return TypeError("EXTRA", t) ;
}

Str XExtraTypeName(Pt t)
{
	t = Drf(t) ;
	if( IsExtra(t) )
		return ExtraTypeName(ExtraTypeOf(t)) ;
	return InternalError("XExtraTypeName") ;
}

Str XExtraAsStr(Pt t)
{
	return ExtraAsStr(XExtra(Drf(t))) ;
}

Pt MakeExtraFromStr(Str s)
{
	Str name = s ;
	CharPt sref = nil ;
	Pt ref ;
	ExtraTypePt e = nil ;
	register int i ;

/* Split type-name and ref */
	for( ; *s ; s++ )
		if( *s == '_' ) {
			sref = cCharPt(s) + 1 ;
			break ;
		}
	if( sref == nil ) return nil ;
	ref = cPt(strtoul(sref, nil, 16)) ;

/* Get type decriptor from name */
	sref[-1] = '\0' ;
	dotimes(i, nExtraTypes)
		if( StrEqual(allExtraTypes[i].name, name) ) {
			e = allExtraTypes + i ;
			break ;
		}
	sref[-1] = '_' ;

/* Final tests */
	if( e == nil )
		return nil ;
	if( !ExtraCheck(e, ref) || ExtraIsHidden(ref) )
		return tFalseAtom ;
	return TagExtra(e, ref) ;
}

void BindVarWithExtra(Pt t, VoidPt x)
{
	VarValue(t) ;
	if( IsVar(t) ) {
		if( !UnifyWithAtomic(t, TagExtra(ExtraTypeOf(x), x)) )
			InternalError("BindVarWithExtra") ;
	}
	elif( ExtraTypeSupportAliasing(ExtraTypeOf(x)) ) {
		if( IsAtom(t) )
#if COMPASSxxxxxxxxxxx
			IVarSet(XAtom(t), TagExtra(allExtraTypes + ExtraTag(x), x), false) ;
#else
			AliasSet(t, x) ;
#endif
		elif( IsThisStruct(t, plusFunctor) && IsAtom(Drf(XStructArg(t,0))) )
			AliasSet(XStructArg(t,0), x) ;
		elif( IsThisStruct(t, minusFunctor) && IsAtom(Drf(XStructArg(t,0))) )
			IVarSet(XTestAtom(XStructArg(t,0)), TagExtra(ExtraTypeOf(x), x), false) ;
		else
			TypeError("VAR or ALIAS or IVAR", t) ;
	}
	else
		TypeError("VAR or ALIAS or IVAR", t) ;
}

VoidPt ExtraTypeError(ExtraTypePt e, Str alt, Pt found)
{
	Str te = e == nil ? "EXTRA" : ExtraTypeName(e) ;
	if( alt == nil )
		return TypeError(te, found) ;
	else
		return TypeError(GStrFormat("%s or %s", te, alt), found) ;
}

void ExtraInit()
{
}

/* CXPROLOG C'BUILTINS */

static void PBasicGC()
{
	ExtraGCDoIt() ;
	JumpNext() ;
}

void ExtraInit2()
{
	InstallCBuiltinPred("basic_gc", 0, PBasicGC) ;
	InstallCBuiltinPred("deleted", 1, PBasicGC) ; /* @@@ to do */
}
