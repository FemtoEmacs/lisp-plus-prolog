/*
 *   This file is part of the CxProlog system

 *   Unify.c
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

Bool UnifyWithNumber(register Pt t, Pt numb) /* pre: numb is a number */
{
	VarValue(t) ;
	if( t == numb )
		return true ;
	elif( IsVar(t) ) {
		Assign(t, numb) ;
		return true ;
	}
	else
		return false ;
}

Bool UnifyWithAtomic(register Pt t, Pt at) /* pre: at is atomic */
{
	VarValue(t) ;
	if( t == at )
		return true ;
	elif( IsVar(t) ) {
		Assign(t, at) ;
		return true ;
	}
	else
		return false ;
}

#if 0
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
#endif

Bool UnifyStruct(register Pt t, FunctorPt functor, Hdl args)
{		/* t is usually a structure */
	VarValue(t) ;
	if( functor == listFunctor )
		return IsList(t) && UnifyN(XListArgs(t), args, 2) ;
	elif( FunctorArity(functor) == 0 )
		return IsAtom(t) && XAtom(t) == FunctorAtom(functor) ;
	elif( IsThisStruct(t, functor) )
		return UnifyN(XStructArgs(t), args, FunctorArity(functor)) ;
	else
		return false ;
}

Bool UnifyVar(register Pt t1, Pt t2)
{		/* t1 is usually a variable */
	VarValue(t1) ;
	return IsVar(t1) && Unify(t1, t2) ;
}

Bool Unifiable(Pt t1, Pt t2)
{
	Bool b ;
	TrailAllVarsStart() ;
	b = Unify(t1, t2) ;
	TrailAllVarsRestore() ;
	return b ;
}

Bool UnifiableN(Hdl h1, Hdl h2, int n)
{
	Bool b ;
	TrailAllVarsStart() ;
	b = UnifyN(h1, h2, n) ;
	TrailAllVarsRestore() ;
	return b ;
}


Bool UnifyN(Hdl h1, Hdl h2, int n)
{
	while( n-- )
		if( !Unify(h1[n], h2[n]) )
			return false ;
	return true ;
}

Bool Unify(Pt term1, Pt term2) /* Handles cyclic terms */
{
	if( occursCheck_flag )
		return UnifyWithOccursCheck(term1, term2) ;
	else {
		Cyclic2_VarDecl(term1, term2) ;
		register Pt t2 ;
		UseScratch() ;
		for(;;) {
			VarValue2(t1, *h1) ;
			VarValue2(t2, *h2) ;

			if( t1 == t2 )
				/* Next */ ;

			elif( IsVar(t1) ) {
				if( IsVar(t2) ) {
					if( Lt(t1,t2) )
						if( IsLocalVar(t1) )
							Assign(t1,t2)
						else Assign(t2,t1)
					else
						if( IsLocalVar(t2) )
							Assign(t2,t1)
						else Assign(t1,t2)
				}
				else Assign(t1, t2) ;
			}

			elif( IsVar(t2) ) {
				Assign(t2, t1) ;
			}

			elif( IsStruct(t1) && IsStruct(t2) && XStructFunctor(t1)==XStructFunctor(t2) ) {
				Cyclic2_PrepareArgs(XStructArgs(t1), XStructArgs(t2), XStructArity(t1), t2) ;
				continue ;
			}

			elif( IsList(t1) && IsList(t2) ) {
				Cyclic2_PrepareArgs(XListArgs(t1), XListArgs(t2), 2, t2) ;
				continue ;
			}

			else {	/* Will fail */
				Cyclic2_Cleanup() ;
				FreeScratch() ;
				return false ;	/* Failure */
			}

			Cyclic2_Next() ;
		}
	finish:
		FreeScratch() ;
		return true ;	/* Success */
	}
}

/* UnifyWithOccursCheck could be defined as
#define UnifyWithOccursCheck(t1, t2)	Unify(t1,t2) && !TermIsCyclic(t1)
  but our implementation catches the cycles more quickly */
Bool UnifyWithOccursCheck(Pt term1, Pt term2) /* Handles cyclic terms */
{
	Cyclic2_VarDecl(term1, term2) ;
	register Pt t2 ;
	UseScratch() ;
	for(;;) {
		VarValue2(t1, *h1) ;
		VarValue2(t2, *h2) ;

		if( t1 == t2 ) {
			if( !OccursCheck(t1) )
				goto fail ;
		}

		elif( IsVar(t1) ) {
			if( IsVar(t2) ) {
				if( Lt(t1,t2) )
					if( IsLocalVar(t1) )
						Assign(t1,t2)
					else Assign(t2,t1)
				else
					if( IsLocalVar(t2) )
						Assign(t2,t1)
					else Assign(t1,t2)
			}
			else {
				Assign(t1, t2) ;
				if( !OccursCheck(t2) )
					goto fail ;
			}
		}

		elif( IsVar(t2) ) {
			Assign(t2, t1) ;
			if( t1 == tCyclicAtom || !OccursCheck(t1) )
				goto fail ;
		}

		elif( IsStruct(t1) && IsStruct(t2) && XStructFunctor(t1)==XStructFunctor(t2) ) {
			Cyclic2_PrepareArgs(XStructArgs(t1), XStructArgs(t2), XStructArity(t1), tCyclicAtom) ;
			continue ;
		}

		elif( IsList(t1) && IsList(t2) ) {
			Cyclic2_PrepareArgs(XListArgs(t1), XListArgs(t2), 2, tCyclicAtom) ;
			continue ;
		}

		/* elif( t == tCyclicAtom ) fail ; */

		else {	/* Will fail */
fail:		Cyclic2_Cleanup() ;
			FreeScratch() ;
			return false ;	/* Failure */
		}

		Cyclic2_Next() ;
	}
finish:
	FreeScratch() ;
	return true ;	/* Success */
}

/* Identical(.) is like Unify(.), except that the var-handling section
   has simply been removed. */
Bool Identical(Pt term1, Pt term2) /* Handles cyclic terms */
{
	Cyclic2_VarDecl(term1, term2) ;
	register Pt t2 ;
	UseScratch() ;
	for(;;) {
		VarValue2(t1, *h1) ;
		VarValue2(t2, *h2) ;

		if( t1 == t2 )
			/* Next */ ;

		elif( IsStruct(t1) && IsStruct(t2) && XStructFunctor(t1)==XStructFunctor(t2) ) {
			Cyclic2_PrepareArgs(XStructArgs(t1), XStructArgs(t2), XStructArity(t1), t2) ;
			continue ;
		}

		elif( IsList(t1) && IsList(t2) ) {
			Cyclic2_PrepareArgs(XListArgs(t1), XListArgs(t2), 2, t2) ;
			continue ;
		}

		else {	/* Will fail */
			Cyclic2_Cleanup() ;
			FreeScratch() ;
			return false ;	/* Failure */
		}

		Cyclic2_Next() ;
	}
finish:
	FreeScratch() ;
	return true ;	/* Success */
}

int Compare(Pt term1, Pt term2) /* Handles cyclic terms */
{
	#define Finish(c)	Do( res = (c) ; goto finish ; )
	Cyclic2_VarDecl(term1, term2) ;
	register Pt t2 ;
	int res = 0 ;	/* Optimistic function */
	UseScratch() ;
	for(;;) {
		VarValue2(t1, *h1) ;
		VarValue2(t2, *h2) ;

		if( t1 == t2 )
			/* Next */ ;

		elif( IsVar(t1) ) {
			if( IsVar(t2) ) {
				if( IsLocalRef(t1) && IsLocalRef(t2) )
					Finish(Gt(t1, t2) ? -1 : 1) ;
				else
					Finish(Gt(t1, t2) ? 1 : -1) ;
			}
			else
				Finish(-1) ;
		}
#if 1
		elif( IsNumber(t1) ) {
			if( IsNumber(t2) )
				Finish(CompareNumber(t1, t2)) ;
			else
				Finish(IsVar(t2) ? 1 : -1) ;
		}
#else
		elif( IsFloat(t1) ) {
			if( IsFloat(t2) )
				Finish(CompareFloat(XFloat(t1), XFloat(t2))) ;
			else
				Finish(IsVar(t2) ? 1 : -1) ;
		}

		elif( IsInt(t1) ) {
			if( IsInt(t2) )
				Finish(CompareInt(XInt(t1), XInt(t2))) ;
			else
				Finish(IsVar(t2) || IsFloat(t2) ? 1 : -1) ;
		}
#endif
		elif( IsAtom(t1) ) {
			if( IsAtom(t2) )
				Finish(StrCompare(XAtomName(t1), XAtomName(t2))) ;
			else
				Finish(IsVar(t2) || IsNumber(t2) ? 1 : -1) ;
		}

		elif( IsCompound(t1) ) {
			if( IsCompound(t2) ) {
				FunctorPt f1 = IsList(t1) ? listFunctor : XStructFunctor(t1) ;
				FunctorPt f2 = IsList(t2) ? listFunctor : XStructFunctor(t2) ;
				if( f1 == f2 ) {
					Hdl args1 = IsList(t1) ? XListArgs(t1) : XStructArgs(t1),
						args2 = IsList(t2) ? XListArgs(t2) : XStructArgs(t2) ;
						Cyclic2_PrepareArgs(args1, args2, FunctorArity(f1), t2) ;
						continue ;
				}
				else
					Finish(FunctorArity(f1) != FunctorArity(f2)
							? CompareInt(FunctorArity(f1), FunctorArity(f2))
							: StrCompare(FunctorName(f1), FunctorName(f2))) ;
			}
			else
				Finish(!IsExtraStrict(t2) ? 1 : -1) ;
		}

		elif( IsExtra(t1) ) {
			if( IsExtraStrict(t2) )
				Finish(Gt(t1, t2) ? 1 : -1) ;
			else
				Finish(1) ;
		}

		else InternalError("Compare (1)") ;

		Cyclic2_Next() ;
	}
finish:
	Cyclic2_Cleanup() ;
	FreeScratch() ;
	return res ;
}

static void QuickSort(Pt *l, Pt *r)	
{
	Pt *a = l, *b = r ;
	Pt pivot = a[(b - a) / 2] ;
	do {
		while( Compare(*a, pivot) < 0 ) a++ ;
		while( Compare(pivot, *b) < 0 ) b-- ;
		if( a <= b ) {
			Pt z = *a ;
			*a++ = *b ;
			*b-- = z ;
		}
	} while( a <= b ) ;
	if( l < b ) QuickSort(l, b) ;
	if( a < r ) QuickSort(a, r) ;
}

static int CompareKeys(Pt t1, Pt t2)
{
	return Compare(XStructArg(t1, 0), XStructArg(t2, 0)) ;
}

static Hdl mergeSortBuffer ;

static void KeyMergeSort(Hdl l, Hdl r)
{
   if(l < r) {
		Hdl m = l + ((r - l) / 2) ;
        KeyMergeSort(l, m) ;
        KeyMergeSort(m + 1, r) ;
	/* merge */	
    	Hdl a = l, b = m + 1, c = mergeSortBuffer ;
    	while( a <= m && b <= r )
			*c++ = CompareKeys(*a, *b) <= 0 ? *a++ : *b++ ;
    	while( a <= m )
			*c++ = *a++ ;
		while( b <= r )
			*c++ = *b++ ;
		for( a = l, c = mergeSortBuffer ; a <= r ; )
			*a++ = *c++ ;
    }
}

static void RemoveDuplicates(Hdl array, Size *len)	/* pre: *len > 1 */
{
	Hdl get, put, stop ;
	stop = array + *len ;
	put = array + 1 ;
	for( get = array + 1 ; get < stop ; get++ )
		if( Compare(put[-1], *get) != 0 )
			*put++ = *get ;
	*len = put - array ;
}

static void CheckAllKey(register Hdl array, Size len)
{
	register Hdl stop ;
	for( stop = array + len ; array < stop ; array++ )
		if( !IsThisStruct(*array, hifenFunctor) )
			TypeError("KEY-LIST", nil) ;
}


/* CXPROLOG C'BUILTINS */

static void PUnify()
{
	MustBe( Unify(X0, X1) ) ;
}

static void PNoUnify(void)
{
	MustBe( !Unifiable(X0, X1) ) ;
}


static void PUnifyWithOccursCheck()
{
	MustBe( UnifyWithOccursCheck(X0, X1) ) ;
}

static void PIdentical()
{
	MustBe( Identical(X0, X1) ) ;
}

static void PNotIdentical()
{
	MustBe( !Identical(X0, X1) ) ;
}

static void PLessThan(void)
{
	MustBe( Compare(X0, X1) < 0 ) ;
}

static void PGreaterThan(void)
{
	MustBe( Compare(X0, X1) > 0 ) ;
}

static void PLessOrEqualThan(void)
{
	MustBe( Compare(X0, X1) <= 0 ) ;
}

static void PGreaterOrEqualThan(void)
{
	MustBe( Compare(X0, X1) >= 0 ) ;
}

static void PCompare(void)
{
	int c = Compare(X1, X2) ;
	if( c < 0 ) MustBe( UnifyWithAtomic(X0, tLessAtom) ) ;
	elif( c == 0 ) MustBe( UnifyWithAtomic(X0, tEqualAtom) ) ;
	else MustBe( UnifyWithAtomic(X0, tGreaterAtom) ) ;
}

static void PSort(void)
{
	Size n ;
	Pt list ;
	Hdl array = ListToArray(X0, &n) ;
	UseScratch() ;
	ScratchMakeRoom(n) ; /* protect the space occupied by the array */
	if( n > 1 ) {
		QuickSort(array, array + n - 1) ;
		RemoveDuplicates(array, &n) ;
	}
	ZEnsureFreeSpaceOnStacks(2 * n, -1, false) ; /* stacks may grow */
	list = ArrayToList(array, n) ;
	FreeScratch() ;
	MustBe( Unify(X1, list) ) ;
}

static void PMSort(void)
{
	Size n ;
	Pt list ;
	Hdl array = ListToArray(X0, &n) ;
	UseScratch() ;
	ScratchMakeRoom(n) ; /* protect the space occupied by the array */
	if( n > 1 )
		QuickSort(array, array + n - 1) ;
	ZEnsureFreeSpaceOnStacks(2 * n, -1, false) ; /* stacks may grow */
	list = ArrayToList(array, n) ;
	FreeScratch() ;
	MustBe( Unify(X1, list) ) ;
}

static void PKeySort(void)
{
	Size n ;
	Pt list ;
	Hdl array = ListToArray(X0, &n) ;
	UseScratch() ;
	ScratchMakeRoom(2*n) ; /* protect double the space occupied by the array */
	CheckAllKey(array, n) ;
	if( n > 1 ) {
		mergeSortBuffer = array + n ;
		KeyMergeSort(array, array + n - 1) ;
	}
	ZEnsureFreeSpaceOnStacks(2 * n, -1, false) ; /* stacks may grow */
	list = ArrayToList(array, n) ;
	FreeScratch() ;
	MustBe( Unify(X1, list) ) ;
}

void UnifyInit()
{
	InstallCBuiltinPred("=", 2, PUnify) ;
	InstallCBuiltinPred("\\=", 2, PNoUnify) ;
	InstallCBuiltinPred("unify_with_occurs_check", 2, PUnifyWithOccursCheck) ;
	InstallCBuiltinPred("==", 2, PIdentical) ;
	InstallCBuiltinPred("\\==", 2, PNotIdentical) ;
	InstallCBuiltinPred("@<", 2, PLessThan) ;
	InstallCBuiltinPred("@>", 2, PGreaterThan) ;
	InstallCBuiltinPred("@=<", 2, PLessOrEqualThan) ;
	InstallCBuiltinPred("@>=", 2, PGreaterOrEqualThan) ;
	InstallCBuiltinPred("compare", 3, PCompare) ;
	InstallCBuiltinPred("sort", 2, PSort) ;
	InstallCBuiltinPred("msort", 2, PMSort) ;
	InstallCBuiltinPred("keysort", 2, PKeySort) ;
}
