/*
 *   This file is part of the CxProlog system

 *   PredicateProperty.c
 *   by A.Miguel Dias - 2005/07/30
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

#define PredIsUserDefined(pr)	( !PredIsUndefined(pr) && !PredIsBuiltin(pr) )
#define PredIsLocalDefined(pr)	( PredIsUserDefined(pr)	&& PredIsLocal(pr) )

static Bool GetPredIsBuiltin(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsBuiltin(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredOnError(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsBuiltin(pr) ;
	return t == nil ? res : res && Unify(t,
	                  MakeUnStruct(LookupFunctor(atom, 1),
											TagAtom(PredOnErrorAsAtom(pr)))) ;
}

static Bool GetPredIsUserDefined(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsUserDefined(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsLocalDefined(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppLocal
#define ppLocal	false
#endif
	Bool res = ppLocal && PredIsLocalDefined(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsStatic(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = !PredIsDynamic(pr) && !PredIsUndefined(pr) && !PredIsImported(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsDynamic(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsDynamic(pr) && PredIsLogical(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsDynamicIU(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsDynamic(pr) && !PredIsLogical(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredImportedFrom(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppImported
#define ppImported	false
#endif
	Bool res = ppImported && PredIsImported(pr) ;
	if( t == nil || !res ) return res ;
	else {
		Pt it = ZPushTerm(GetImportTerm(pr)) ; /* stacks may grow */
		return Unify(t, MakeUnStruct(LookupFunctor(atom, 1), it)) ;
	}
}

static Bool GetPredIsVisible(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppVisible
#define ppVisible	false
#endif
	Bool res = ppVisible && PredIsUserDefined(pr) && PredIsVisible(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsXNew(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppXNew
#define ppXNew		false
#endif
	Bool res = ppXNew && PredIsUserDefined(pr) && !PredIsXOver(pr) ;
	return  t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsXOver(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppXOver
#define ppXOver		false
#endif
	Bool res = ppXOver && PredIsUserDefined(pr) && PredIsXOver(pr) ;
	return  t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsGOpen(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppGOpen
#define ppGOpen		false
#endif
	Bool res = ppGOpen && PredIsUserDefined(pr) && PredIsVisible(pr) && PredIsGOpen(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsGClose(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppGClose
#define ppGClose	false
#endif
	Bool res = ppGClose && PredIsUserDefined(pr) && PredIsVisible(pr) && !PredIsGOpen(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsHidden(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppHidden
#define ppHidden	false
#endif
	Bool res = ppHidden && PredIsUserDefined(pr) && !PredIsVisible(pr) ;
	return  t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsPrivate(PredicatePt pr, AtomPt atom, Pt t)
{
#ifndef ppPrivate
#define ppPrivate	false
#endif
	Bool res = ppPrivate && PredIsUserDefined(pr) && !PredIsVisible(pr) ;
	return  t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsMultifile(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsMultifile(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredIsDiscontiguous(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsDiscontiguous(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredHasSource(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredKeepSource(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

static Bool GetPredFile(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = !PredIsUndefined(pr) && PredConsultFile(pr) != nil ;
	if( t == nil || !res ) return res ;
	else
		return Unify(t, MakeUnStruct(LookupFunctor(atom, 1),
						TagAtom(PredConsultFile(pr)))) ;
}

static Bool GetNumberOfClauses(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsLocalDefined(pr) || PredIsDynamic(pr) ;
	return t == nil
			? res
			: res && Unify(t, MakeUnStruct(LookupFunctor(atom, 1),
											MakeInt(PredLength(pr)))) ;
}

static Bool GetFullName(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = !PredWasAbolished(pr) ;
	return t == nil
			? res
			: res && Unify(t, MakeUnStruct(LookupFunctor(atom, 1),
											PredNameArityTerm(pr))) ;
}

static Bool GetPredIsUndefined(PredicatePt pr, AtomPt atom, Pt t)
{
	Bool res = PredIsUndefined(pr) && !PredWasAbolished(pr) ;
	return t == nil ? res : res && Unify(t, TagAtom(atom)) ;
}

typedef struct {
	Str name ;
	Bool (*fun)(PredicatePt, AtomPt, Pt) ;
	AtomPt atom ;
} PredProperty, *PredPropertyPt ;

static PredProperty predProperties[] = {
	{ "built_in",			GetPredIsBuiltin,		nil },
	{ "on_error",			GetPredOnError,			nil },
	{ "user_defined",		GetPredIsUserDefined,	nil },
	{ "local",				GetPredIsLocalDefined,	nil },
	{ "static",				GetPredIsStatic,		nil },
	{ "dynamic",			GetPredIsDynamic,		nil },
	{ "dynamic_iu",			GetPredIsDynamicIU,		nil },
	{ "imported_from",		GetPredImportedFrom,	nil },
	{ "visible",			GetPredIsVisible,		nil },
	{ "xnew",				GetPredIsXNew,			nil },
	{ "xover",				GetPredIsXOver,			nil },
	{ "gopen",				GetPredIsGOpen,			nil },
	{ "gclose",				GetPredIsGClose,		nil },
	{ "hidden",				GetPredIsHidden,		nil },
	{ "private",			GetPredIsPrivate,		nil },
	{ "multifile",			GetPredIsMultifile,		nil },
	{ "discontiguous",		GetPredIsDiscontiguous,	nil },
	{ "source",				GetPredHasSource,		nil },
	{ "file",				GetPredFile,			nil },
	{ "undefined",			GetPredIsUndefined,		nil },
	{ "full_name",			GetFullName,			nil },
	{ "number_of_clauses",	GetNumberOfClauses,		nil },
	{ nil,0,0 }
} ;

static PredPropertyPt propBuiltin, propUndefined ;

void WritePredicateProperties(StreamPt srm, PredicatePt pr)
{
	StreamWrite(srm, "%s", PredNameArity(pr)) ;
	if( GetPredIsBuiltin(pr, nil, nil) ) {
		if(PredIsC(pr) )
			StreamWrite(srm, " - c_built_in") ;
		else	
			StreamWrite(srm, " - built_in") ;
		StreamWrite(srm, " - on_error(%s)", AtomName(PredOnErrorAsAtom(pr))) ;
	}		
	if( PredIsMeta(pr) )
		StreamWrite(srm, " - meta") ;
	if( GetPredIsUserDefined(pr, nil, nil) )
		StreamWrite(srm, " - user_defined") ;
	if( GetPredIsLocalDefined(pr, nil, nil) )
		StreamWrite(srm, " - local") ;
	if( GetPredImportedFrom(pr, nil, nil) )
		StreamWrite(srm, " - imported_from(%s)", TermAsStr(GetImportTerm(pr))) ;
	if( GetPredIsVisible(pr, nil, nil) )
		StreamWrite(srm, " - visible") ;
	if( GetPredIsXNew(pr, nil, nil) )
		StreamWrite(srm, " - xnew") ;
	if( GetPredIsXOver(pr, nil, nil) )
		StreamWrite(srm, " - xover") ;
	if( GetPredIsGOpen(pr, nil, nil) )
		StreamWrite(srm, " - gopen") ;
	if( GetPredIsGClose(pr, nil, nil) )
		StreamWrite(srm, " - gclose") ;
	if( GetPredIsHidden(pr, nil, nil) )
		StreamWrite(srm, " - hidden") ;
	if( GetPredIsPrivate(pr, nil, nil) )
		StreamWrite(srm, " - private") ;
	if( GetPredIsStatic(pr, nil, nil) )
		StreamWrite(srm, " - static") ;
	if( GetPredIsDynamic(pr, nil, nil) )
		StreamWrite(srm, " - dynamic") ;
	if( GetPredIsDynamicIU(pr, nil, nil) )
		StreamWrite(srm, " - dynamic_iu") ;
	if( GetPredIsMultifile(pr, nil, nil) )
		StreamWrite(srm, " - multifile") ;
	if( GetPredIsDiscontiguous(pr, nil, nil) )
		StreamWrite(srm, " - discontiguous") ;
	if( GetPredHasSource(pr, nil, nil) )
		StreamWrite(srm, " - source") ;
	if( GetPredFile(pr, nil, nil) )
		StreamWrite(srm, " - file('%s')", AtomName(PredConsultFile(pr))) ;
	if( PredNIndexable(pr) > 0 )
		StreamWrite(srm, " - index(%d)", PredNIndexable(pr)) ;		
	if( GetNumberOfClauses(pr, nil, nil) ) {
		int n = PredLength(pr) ;
		StreamWrite(srm, " - %d clause%s", n, n == 1 ? "" : "s") ;
	}
	if( GetPredIsUndefined(pr, nil, nil) )
		StreamWrite(srm, " - undefined") ;
	if( PredWasAbolished(pr) )
		StreamWrite(srm, " - abolished") ;
	StreamWrite(srm, "\n\n") ;
}

static PredPropertyPt GetProperty(Pt t)
{
	AtomPt a ;
	PredPropertyPt prop ;
	if( (a = XTermAtom(t)) == nil ) return nil ;
	for( prop = predProperties ; prop->name != nil ; prop++ )
		if( prop->atom == a ) return prop ;
	return nil ;
}

static Bool CallProperty(PredPropertyPt prop, PredicatePt pr, Pt t)
{
	return prop->fun(pr, prop->atom, t) ;
}

static void InitPredProperties(void)
{
	PredPropertyPt prop ;
	for( prop = predProperties ; prop->name != nil ; prop++ )
		prop->atom = LookupAtom(prop->name) ;
	propBuiltin = GetProperty(MakeAtom("built_in")) ;
	propUndefined = GetProperty(MakeAtom("undefined")) ;
}



/* CXPROLOG C'BUILTINS */

static void PPredicateProperty()
{
	if( A(2) == tNilAtom ) {			/* init */
		X0 = Drf(X0) ;
		X1 = Drf(X1) ;
		if( IsVar(X0) && IsVar(X1) ) {	/* VAR-VAR init */
			A(2) = zeroIntPt ;
			A(3) = cPt(UnitPreds(CurrUnit())) ;
			A(4) = cPt(cC99Fix(predProperties)) ;
		}
		elif( IsVar(X0) ) {				/* VAR-PROP init */
			A(2) = oneIntPt ;
			A(4) = cPt(GetProperty(X1)) ;
			if( A(4) == nil )
				Jump(DiscardAndFail) ;
			elif( Eq(A(4), propBuiltin) )
				A(3) = cPt(UnitPreds(systemUnit)) ;	/* only the built-ins */
			else
				A(3) = cPt(UnitPreds(CurrUnit())) ;
		}
		elif( IsVar(X1) ) {				/* PRED-VAR init */
			PredicatePt pr = FindPredicate(XTestFunctor(X0)) ;
			if( pr == nil )
				Jump(DiscardAndFail) ;
			else {
				A(2) = twoIntPt ;
				A(3) = cPt(pr) ;
				A(4) = cPt(cC99Fix(predProperties)) ;
			}
		}
		else {											/* PRED-PROP handle */
			PredicatePt pr = FindPredicate(XTestFunctor(X0)) ;
			PredPropertyPt prop = GetProperty(X1) ;
			Discard() ;
			MustBe( pr != nil && prop != nil && CallProperty(prop, pr, X1) ) ;
		}
	}

	if( A(2) == twoIntPt ) {							/* PRED-VAR handle */
		PredicatePt pr = cPredicatePt(A(3)) ;
		PredPropertyPt prop = (PredPropertyPt)(A(4)) ;
		for( ; prop->name != nil ; prop++ )
			if( CallProperty(prop, pr, X1) ) {
				A(4) = cPt(prop+1) ;
				JumpNext() ;
			}
		Jump(DiscardAndFail) ;
	}

	if( A(2) == oneIntPt ) {							/* VAR-PROP handle */
		PredicatePt pr = cPredicatePt(A(3)) ;
		PredPropertyPt prop = (PredPropertyPt)(A(4)) ;
		doseq(pr, pr, PredNextU(pr))
			if( prop == propUndefined || !PredIgnoredInGeneration(pr) )
				if( CallProperty(prop, pr, X1) ) break ;
		if( pr == nil )
			Jump(DiscardAndFail) ;
		else {
			A(3) = cPt(PredNextU(pr)) ;
			MustBe( Unify(X0, MakeCleanStruct(PredFunctor(pr))) ) ;
		}
	}

	if( A(2) == zeroIntPt ) {							/* VAR-VAR handle */
		PredicatePt pr = cPredicatePt(A(3)) ;
		PredPropertyPt prop = (PredPropertyPt)(A(4)) ;
		doseq(pr, pr, PredNextU(pr))
			if( !PredIgnoredInGeneration(pr) ) {
				for( ; prop->name != nil ; prop++ ) {
					if( CallProperty(prop, pr, X1) ) {
						A(3) = cPt(pr) ;
						A(4) = cPt(prop+1) ;
						MustBe( Unify(X0, MakeCleanStruct(PredFunctor(pr))) ) ;
					}
				}
				prop = predProperties ;
			}
		Jump(DiscardAndFail) ;
	}
}

static void PNDCurrentPredicate()
{
	PredicatePt pr ;
	if( A(1) == tNilAtom ) {			/* init */
		X0 = Drf(X0) ;
		if( IsVar(X0) )
			A(1) = minusOneIntPt ;		/* VAR-VAR init */
		else {
			Pt a0, a1 ;
			XTestSlashArgs(X0, &a0, &a1) ;
			if( IsVar(a0) && IsVar(a1) )
				A(1) = minusOneIntPt ;	/* VAR-VAR init */
			elif( IsVar(a0) )
				A(1) = a1 ;				/* VAR-INT init */
			elif( IsVar(a1) )
				A(1) = a0 ;				/* ATOM-VAR init */
			else {								/* atom-int handle */
				PredicatePt pr = FindPredicateInUnit(CurrUnit(), XTestSlash(X0)) ;
				Discard() ;
				MustBe( pr != nil && !PredIgnoredInGeneration(pr) ) ;
			}
		}
		A(2) = cPt(UnitPreds(CurrUnit())) ;
	}

	pr = cPredicatePt(A(2)) ;
	if( A(1) == minusOneIntPt ) {						/* VAR-VAR handle */
		doseq(pr, pr, PredNextU(pr)) {
			if( !PredIgnoredInGeneration(pr) ) break ;
		}
	}
	elif( IsAtom(A(1)) ) {								/* ATOM-VAR handle */
		AtomPt at = XAtom(A(1)) ;
		doseq(pr, pr, PredNextU(pr))
			if( PredAtom(pr) == at && !PredIgnoredInGeneration(pr) ) break ;
	}
	else {												/* VAR-INT handle */
		int n = XInt(A(1)) ;
		doseq(pr, pr, PredNextU(pr))
			if( PredArity(pr) == n && !PredIgnoredInGeneration(pr) ) break ;
	}
	if( pr == nil )
		Jump(DiscardAndFail) ;
	else {
		A(2) = cPt(PredNextU(pr)) ;
		MustBe( Unify(X0, MakeSlashTerm(PredFunctor(pr))) ) ;
	}
}

static void PNDCurrentPredicate2()
{
	PredicatePt pr ;
	if( A(2) == tNilAtom ) {			/* init */
		X0 = Drf(X0) ;
		X1 = Drf(X1) ;
		if( IsVar(X0) && IsVar(X1) )	/* VAR-VAR init */
			A(2) = zeroIntPt ;
		elif( IsVar(X1) )				/* ATOM-VAR init */
			A(2) = cPt(XTestAtom(X0)) ;
		else {									/* var_or_atom-pred handle */
			PredicatePt pr = FindPredicateInUnit(CurrUnit(), XTestFunctor(X1)) ;
			Discard() ;
			MustBe( pr != nil && !PredIgnoredInGeneration(pr)
					&& UnifyWithAtomic(X0, TagAtom(PredAtom(pr))) ) ;
		}
		A(3) = cPt(UnitPreds(CurrUnit())) ;
	}

	pr = cPredicatePt(A(3)) ;
	if( A(2) == zeroIntPt ) {							/* VAR-VAR handle */
		doseq(pr, pr, PredNextU(pr))
			if( !PredIgnoredInGeneration(pr) ) break ;
	}
	else {												/* ATOM-VAR handle */
		AtomPt at = cAtomPt(A(2)) ;
		doseq(pr, pr, PredNextU(pr))
			if( PredAtom(pr) == at && !PredIgnoredInGeneration(pr) ) break ;
	}
	if( pr == nil )
		Jump(DiscardAndFail) ;
	else {
		A(3) = cPt(PredNextU(pr)) ;
		MustBe( UnifyWithAtomic(X0, TagAtom(PredAtom(pr)))
			 && Unify(X1, MakeCleanStruct(PredFunctor(pr))) ) ;
	}
}

#if COMPASS
static void PNDCurrentPredicate903()
{
	PredicatePt pr =
		A(1) == tNilAtom
			? UnitPreds(CurrUnit())
			: PredNextU(cPredicatePt(A(1))) ;
	doseq(pr, pr, PredNextU(pr))
		if( PredHasClauses(pr) ) break ;
	A(1) = cPt(pr) ;
	if( pr == nil )
		Jump(DiscardAndFail) ;
	else
		MustBe( Unify(X0, MakeCleanStruct(PredFunctor(pr))) ) ;
}
#endif

void PredicatePropertyInit()
{
	InitPredProperties() ;
	InstallGNDeterCBuiltinPred("predicate_property", 2, 3, PPredicateProperty) ;
#if COMPASS
	InstallNDeterCBuiltinPred("current_predicate", 1, PNDCurrentPredicate903) ;
	InstallGNDeterCBuiltinPred("current_predicate_x", 1, 2, PNDCurrentPredicate) ;
#else
	InstallGNDeterCBuiltinPred("current_predicate", 1, 2, PNDCurrentPredicate) ;
#endif
	InstallGNDeterCBuiltinPred("current_predicate", 2, 2, PNDCurrentPredicate2) ;
}
