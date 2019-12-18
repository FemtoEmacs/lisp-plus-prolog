/*
 *   This file is part of the CxProlog system

 *   Unit.c
 *   by A.Miguel Dias - 2000/04/02
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

UnitPt unitList = nil, systemUnit, emptyUnit, bottomUnit ;
static UnitPt mainUnit ;

static UnitPt NewUnit(AtomPt atom, int arity)
{
	UnitPt u = Allocate(WordsOf(Unit) + arity, false) ;
	UnitPreds(u) = nil ;
	UnitFunctor(u) = LookupFunctor(atom, arity) ;
	UnitAnonymousLoc(u) = nil ;
	UnitAnonymousClauses(u)	= nil ;
	UnitIsDefined(u) = false ;
	UnitIsHidden(u) = false ;
	UnitIsPermanent(u) = false ;
	UnitIsSystem(u) = false ;
	while( arity-- )
		UnitParam(u, arity) = tQuestionAtom ;
	AtomToUnit(atom) = u ;
	ExtraPermanent(atom) ;
	UnitNext(u) = unitList ;
	unitList = u ;
	return u ;
}

static void DeleteFirstUnit(void) {
	UnitPt u = unitList ;
	AtomToUnit(UnitAtom(u)) = nil ;
	unitList = UnitNext(u) ;	
	Release(u, WordsOf(Unit) + UnitArity(u)) ;
}

static UnitPt NewAnonymousUnit(Pt term, Hdl pushLoc)
{
	if( term == tNilAtom || IsList(term) ) {
		UnitPt u ;
		doseq(u, unitList, UnitNext(u))
			if( UnitIsAnonymous(u) && Compare(term, UnitAnonymousClauses(u)) == 0 )
				return u ;
	/* not found */
		u = NewUnit(LookupAtom(GStrFormat("$$_anonymous")), 0) ;
		UnitAnonymousLoc(u) = pushLoc ;
		UnitAnonymousClauses(u)	= AllocateTermForAssign(term) ;
		UnitIsDefined(u) = true ;
		return u ;
	}
	else
		return Error("Invalid anonymous unit") ;
}

static void CheckUnitArity(UnitPt u, int arity)
{
	if( UnitArity(u) != arity )
		DatabaseError("Unit '%s' was previously %s with a different arity",
					UnitName(u),
					UnitIsDefined(u) ? "defined" : "used") ;
}

static Bool Duplicates(Hdl params, int arity)
{
	register int i, j ;
	dotimes(i, arity)
		for( j = i + 1 ; j < arity ; j++ )
			if( params[i] == params[j] )
				return true ;
	return false ;
}

static UnitPt DefineUnit(AtomPt atom, int arity, Hdl params)
{
	int i ;
	UnitPt u ;
/* Check unit name */
	if( !cx_islower(CharFirst(AtomName(atom))) && !Booting() )
		DatabaseError("Unit names must start with a lowercase letter") ;
/* Check if already exists */	
	if( (u = AtomToUnit(atom)) != nil ) {
		CheckUnitArity(u, arity) ;
		return u ;
	}	
/* New unit */	
	u = NewUnit(atom, arity) ;
	dotimes(i, arity) {
		AtomPt a = XTestAtom(params[i]) ;
		if( !cx_isupper(CharFirst(AtomName(a))) ) {
			DeleteFirstUnit() ;
			DatabaseError("Unit parameter names must start with an uppercase letter") ;
		}
		UnitParam(u, i) = TagAtom(a) ;
	}
	if( Duplicates(UnitParams(u), arity) ) {
		DeleteFirstUnit() ;
		DatabaseError("All unit argument names be distinct") ;
	}
	UnitIsDefined(u) = true ;
	return u ;
}

Bool IsCurrUnitParameter(Pt name)
{
	register UnitPt u = CurrUnit() ;
	register int n = UnitArity(u) ;
	while( n-- )
		if( UnitParam(u, n) == name )
			return true ;
	return false ;
}

Pt LookupCurrUnitParameter(Pt name)
{
	register UnitPt u = CurrUnit() ;
	register int n = UnitArity(u) ;
	while( n-- )
		if( UnitParam(u, n) == name )
			return MakeUnStruct(unitParamFunctor, MakeInt(n+1)) ;
	return nil ;
}

CharPt UnitSignature(UnitPt u)
{
	if( UnitArity(u) == 0 )
		return UnitName(u) ;
	else {
		CharPt s = GStrFormat("%s(", UnitName(u)) ;
		int i ;
		dotimes(i, UnitArity(u))
			s = GStrFormat("%s%s, ", s, XAtomName(UnitParam(u,i))) ;
		strcpy(s + strlen(s) - 2, ")") ;
		return s ;
	}
}

UnitPt TermToUnit(register Pt term, Hdl pushLoc)
{
	register UnitPt u ;
	VarValue(term) ;
	if( term == tNilAtom || IsList(term) ) {
		u = NewAnonymousUnit(term, pushLoc) ;
	}
	elif( IsAtom(term) ) {
		if( (u = AtomToUnit(XAtom(term))) == nil )
			DatabaseError("Unit '%s' is undefined", XAtomName(term)) ;
		else CheckUnitArity(u, 0) ;
	}
	elif( IsStruct(term) ) {
		if( (u = AtomToUnit(XStructAtom(term))) == nil )
			DatabaseError("Unit '%s' is undefined", XStructName(term)) ;
		else CheckUnitArity(u, XStructArity(term)) ;
	}
	else {
		u = nil ;
		if( IsVar(term) ) DatabaseError("Unit name is not bound") ;
		elif( IsNumber(term) ) DatabaseError("Unit name is a NUMBER") ;
		elif( IsExtra(term) ) DatabaseError("Unit name is an EXTRA") ;
		else InternalError("TermToUnit") ;
	}
	return u ;
}

void UnitsGC(Hdl top)
{
	register UnitPt u ;
	doseq(u, unitList, UnitNext(u))
		if( Gt(UnitAnonymousLoc(u),top) || Ne(*UnitAnonymousLoc(u),u) )
			{} ;
}

Bool UnitCheck(VoidPt ref)
{
	register UnitPt u ;
	doseq(u, unitList, UnitNext(u))
		if( u == ref ) return true ;
	return false ;
}

Size CountUnits(void)
{
	register UnitPt u ;
	register Size n = 0 ;
	doseq(u, unitList, UnitNext(u))
		if( UnitIsVisible(u) )
			n++ ;
	return n ;
}

Size CountPredicates(void)
{
	register UnitPt u ;
	register PredicatePt pr ;
	register Size n = 0 ;
	doseq(u, unitList, UnitNext(u))
		if( UnitIsVisible(u) )
			doseq(pr, UnitPreds(u), PredNextU(pr))
				n++ ;
	return n ;
}

Size CountClauses(void)
{
	register UnitPt u ;
	register PredicatePt pr ;
	register Size n = 0 ;
	doseq(u, unitList, UnitNext(u))
		if( UnitIsVisible(u) )
			doseq(pr, UnitPreds(u), PredNextU(pr))
				n += PredLength(pr) ;
	return n ;
}
	
void ChangingCurrUnit(Str error, PredicatePt pr)
{
	if( UnitIsPermanent(CurrUnit()) ) {
		if( NoCurrUnit() )
			DatabaseError("No current unit when %s predicate '%s'",
											error, PredNameArity(pr)) ;
		else
			DatabaseError("Attempt to change immutable unit '%s'",
											UnitSignature(CurrUnit())) ;
	}
}

void ShowContexts()
{
	Write("HIS = %s\n", TermAsStr(CH)) ;
	Write("CTX = %s\n", TermAsStr(C)) ;
}

void SemanticVarianteUpdateFlag(int newValue)
{
	semanticVariant_flag = newValue ;
	if( semanticVariant_flag == 3 )
		forceVisibility_flag = true ;
}

static void AbolishAll(void)
{
	UnitPt u ;
	PredicatePt pr ;
	doseq(u, unitList, UnitNext(u))
		if( UnitIsVisible(u) )
			doseq(pr, UnitPreds(u), PredNextU(pr))
				AbolishPredicate(pr, false) ;
}


/* CXPROLOG C'BUILTINS */

static void PCreateUnit()
{
	Pt t0 = Drf(X0) ;
	if( IsAtom(t0) )
		DefineUnit(XAtom(t0), 0, nil) ;
	elif( IsStruct(t0) ) {
		DefineUnit(XStructAtom(t0), XStructArity(t0), XStructArgs(t0)) ;
	}
	else TypeError("ATOM or STRUCT", t0) ;
	JumpNext() ;
}

static void PAbolishAll()
{
	AbolishAll() ;
	JumpNext() ;
}

static void PGetContext()
{
	MustBe( Unify(X0, C) ) ;
}

static void PGetHContext()
{
	MustBe( Unify(X0, CH) ) ;
}

static void PUnitSpec()
{
	UnitPt u = CurrUnit() ;
	Ensure( C != tNilAtom ) ;
	if( UnitIsAnonymous(u) ) {
		Pt list = ZPushTerm(UnitAnonymousClauses(u)) ; /* stacks may grow */
		MustBe( Unify(X0, list) ) ;
	}
	else
		MustBe( Unify(X0, MakeStruct(UnitFunctor(u), UnitParams(u))) ) ;
}

static void PUnitInUse()
{ /* Test predicate */
	UnitPt u = AtomToUnit(XTestAtom(X0)) ;
	if( u == nil ) Mesg("There is no unit '%s'", XTestAtomName(X0)) ;
	elif( IsUnitInUse(u) ) Mesg("Unit '%s' is in use", XTestAtomName(X0)) ;
	else Mesg("Unit '%s' not in use", XTestAtomName(X0)) ;
	JumpNext() ;
}

static void PNDCurrentUnit()
{
	UnitPt u = A(1) == tNilAtom
				? unitList
				: UnitNext(cUnitPt(A(1))) ;
	while( u != nil && !UnitIsVisible(u) )
		u = UnitNext(u) ;
	A(1) = cPt(u) ;
	if( u == nil ) Jump(DiscardAndFail) ;
	MustBe( Unify(X0, MakeStruct(UnitFunctor(u), UnitParams(u))) ) ;
}

static void PUnits(void)
{
	UnitPt u ;
	ShowVersion() ;
	Write("UNITS:\n") ;
	doseq(u, unitList, UnitNext(u))
		if( UnitIsVisible(u) )
			Write("    %s\n", UnitSignature(u) ) ;
	JumpNext() ;
}

static void PShowContexts(void)
{
	ShowContexts() ;
	JumpNext() ;
}

void UnitsInit()
{
	systemUnit = DefineUnit(LookupAtom("$$_system"), 0, nil) ;
	UnitIsHidden(systemUnit) = true ;
	UnitIsSystem(systemUnit) = true ;
	bottomUnit = systemUnit ;

	emptyUnit = DefineUnit(LookupAtom("$$_empty"), 0, nil) ;
	UnitIsHidden(emptyUnit) = true ;
	UnitIsPermanent(emptyUnit) = true ;
	UnitIsSystem(emptyUnit) = true ;

	mainUnit = DefineUnit(XAtom(tMainAtom), 0, nil) ;	/* Define main unit */
	UnitIsSystem(mainUnit) = true ;	
}

void UnitsInit2()
{
	InstallCBuiltinPred("create_unit", 1, PCreateUnit) ;
	InstallCBuiltinPred("abolish_all", 0, PAbolishAll) ;
	InstallCBuiltinPred("context", 1, PGetContext) ;
	InstallCBuiltinPred("hcontext", 1, PGetHContext) ;
	InstallCBuiltinPred("unit_spec", 1, PUnitSpec) ;
	InstallCBuiltinPred("unit_in_use", 1, PUnitInUse) ;
	InstallNDeterCBuiltinPred("current_unit", 1, PNDCurrentUnit) ;
	InstallCBuiltinPred("units", 0, PUnits) ;
	InstallCBuiltinPred("zctx", 0, PShowContexts) ;
}
