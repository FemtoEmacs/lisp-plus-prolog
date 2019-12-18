/*
 *   This file is part of the CxProlog system

 *   IVar.c
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

typedef struct IVar {
	struct IVar *next ;		/* Next imperative var in the ivar list */
	AtomPt atom ;			/* Variable name */
	Pt value ;				/* Value of imperative variable */
	Bool isConstant ;		/* ivar is a constant */
	Bool isBuiltin ;		/* ivar is built-in, cannot be locally redefined */
} IVar, *IVarPt ;

#define IVarIsBuiltin(iv)	(iv)->isBuiltin
#define IVarIsDefined(iv)	( (iv) != nil && (iv)->value != tUndefinedIVarAtom )

#define cIVarPt(p)			((IVarPt)(p))

static IVarPt ivarList = nil ;
static AtomPt reservedAtom ;
static Pt tUndefinedIVarAtom ;

static IVarPt IVarNew(AtomPt atom)
{
	register IVarPt iv ;
	if( atom == reservedAtom )
		Error("Reserved name. Cannot define ivar '%s'", AtomName(reservedAtom)) ;
	iv = Allocate(WordsOf(IVar), false) ;
	iv->atom = atom ;
	iv->value = tUndefinedIVarAtom ;
	iv->isConstant = false ;
	iv->isBuiltin = false ;
	AtomToIVar(atom) = iv ;
	ExtraPermanent(atom) ;
	iv->next = ivarList ;
	ivarList = iv ;
	return iv ;
}

static IVarPt LookupIVar(AtomPt atom)
{
	return AtomToIVar(atom) == nil
				? IVarNew(atom)
				: AtomToIVar(atom) ;
}

static void IVarChangeValue(IVarPt iv, Pt value)
{
	ReleaseTerm(iv->value) ;
	iv->value = value ;
}

void IVarSet(AtomPt atom, Pt value, Bool cons)
{
	register IVarPt iv = LookupIVar(atom) ;
	if( iv->isConstant )
		ImperativeError("Attempt to change constant-ivar '%s'", AtomName(atom)) ;
	IVarChangeValue(iv, AllocateTermForAssign(value)) ;
	iv->isConstant = cons ;
}

static void IVarCondSet(AtomPt atom, Pt value, Bool cons)
{
	register IVarPt iv = LookupIVar(atom) ;
	if( IVarIsDefined(iv) ) return ;
	if( iv->isConstant )
		ImperativeError("Attempt to change constant-ivar '%s'", AtomName(atom)) ;
	IVarChangeValue(iv, AllocateTermForAssign(value)) ;
	iv->isConstant = cons ;
}

void IVarForceSet(AtomPt atom, Pt value, Bool cons)
{
	register IVarPt iv = LookupIVar(atom) ;
	IVarChangeValue(iv, AllocateTermForAssign(value)) ;
	iv->isConstant = cons ;
}

static void IVarReversibleSet(AtomPt atom, Pt value)
{
	register IVarPt iv = LookupIVar(atom) ;
	if( iv->isConstant )
		ImperativeError("Attempt to change constant-ivar '%s'", AtomName(atom)) ;
/* Push a trailed ivar, saving old value, which is not deleted */
	TrailIVar(atom, iv->value) ;
	iv->value = AllocateTermForAssign(value) ;
}
void IVarReversibleRestore(AtomPt atom, Pt oldValue) /* Called from Machine.c */
{
	register IVarPt iv = LookupIVar(atom) ;
	IVarChangeValue(iv, oldValue) ; /* restore trailed ivar */
}

static void IVarDelete(AtomPt atom)
{
	register IVarPt iv = AtomToIVar(atom) ;
	if( IVarIsDefined(iv) ) {
		if( IVarIsBuiltin(iv) )
			ImperativeError("Attempt to undefine built-in ivar '%s'",
										AtomName(atom)) ;
		IVarChangeValue(iv, tUndefinedIVarAtom) ;
		iv->isConstant = false ;
	}
}

Pt IVarGet(AtomPt atom)
{
	register IVarPt iv = AtomToIVar(atom) ;
	if( IVarIsDefined(iv) )
		return iv->value ;
	else return nil ;
}

static AtomPt IVarWith(Pt t)
{
	register IVarPt iv ;
	doseq(iv, ivarList, iv->next)
		if( Identical(t, iv->value) )
			return iv->atom ;
	return nil ;
}

void IVarsWithWrite(Pt t)
{
	if( IVarWith(t) != nil ) {
		register IVarPt iv ;
		doseq(iv, ivarList, iv->next)
			if( Identical(t, iv->value) )
				Write(", ivar(%s)", AtomName(iv->atom)) ;
	}
}

static void IVarsBasicGCMark()
{
	register IVarPt iv ;
	doseq(iv, ivarList, iv->next)
		TermBasicGCMark(iv->value) ;
}


/* CXPROLOG C'BUILTINS */

static void PIVar()
{
	AtomPt atom = XTestAtom(X0) ;
	IVarPt iv = AtomToIVar(atom) ;
	MustBe( IVarIsDefined(iv) ) ;
}

static void PIVarDelete()
{
	AtomPt atom = XTestAtom(X0) ;
	IVarDelete(atom) ;
	JumpNext() ;
}

static void PIVarSet()
{
	AtomPt atom = XTestAtom(X0) ;
	IVarSet(atom, X1, false) ;
	JumpNext() ;
}

static void PIVarConstSet()
{
	AtomPt atom = XTestAtom(X0) ;
	IVarSet(atom, X1, true) ;
	JumpNext() ;
}

static void PIVarCondSet()
{
	AtomPt atom = XTestAtom(X0) ;
	IVarCondSet(atom, X1, false) ;
	JumpNext() ;
}

static void PIVarReversibleSet()
{
	AtomPt atom = XTestAtom(X0) ;
	IVarReversibleSet(atom, X1) ;
	JumpNext() ;
}

static void PIVarGet()
{
	AtomPt atom = XTestAtom(X0) ;
	Pt t = IVarGet(atom) ;
	Ensure( t != nil ) ;
	t = ZPushTerm(t) ; /* stacks may grow */
	MustBe( Unify(X1, t) ) ;
}

static void PIVarGetU()
{
	AtomPt atom = XTestAtom(X0) ;
	Pt t = IVarGet(atom) ;
	if( t == nil )
		t = tUndefinedIVarAtom ;
	t = ZPushTerm(t) ; /* stacks may grow */
	MustBe( Unify(X1, t) ) ;
}

static void PNDCurrentIVar()
{
	Pt t ;
	register IVarPt iv = A(2) == tNilAtom
				? ivarList
				: cIVarPt(A(2))->next ;
	doseq(iv, iv, iv->next)
		if( IVarIsDefined(iv) ) break ;
	A(2) = cPt(iv) ;
	if( iv == nil ) Jump(DiscardAndFail) ;
	Ensure( UnifyWithAtomic(X0, TagAtom(iv->atom)) ) ;
	t = ZPushTerm(iv->value) ; /* stacks may grow */
	MustBe( Unify(X1, t) ) ;

}

static void PIVars()
{
	register IVarPt iv ;
	ShowVersion() ;
	Write("IVARS:\n") ;
	doseq(iv, ivarList, iv->next)
		if( IVarIsDefined(iv) )
			Write(" %s %16.16s -> %1.50s\n",
						iv->isConstant ? "CONST" : "     ",
						AtomName(iv->atom),
						TermAsStr(iv->value)) ;
	JumpNext() ;
}

void IVarsInit()
{
	reservedAtom = XAtom(tUserAtom) ;
	tUndefinedIVarAtom = MakeAtom("$$_undefined_ivar") ;
	ExtraGCHandlerInstall("IVAR", IVarsBasicGCMark) ;
	/* add "ivars." to CxProlog.c/PShow */

	InstallCBuiltinPred("ivar", 1, PIVar) ;
	InstallCBuiltinPred("ivar_delete", 1, PIVarDelete) ;
	InstallCBuiltinPred("?:=", 2, PIVarCondSet) ;
	InstallCBuiltinPred("&:=", 2, PIVarReversibleSet) ;
	InstallCBuiltinPred("#:=", 2, PIVarConstSet) ;
	InstallCBuiltinPred(":=", 2, PIVarSet) ;
	InstallCBuiltinPred("=:", 2, PIVarGet) ;
	InstallCBuiltinPred("$$_ivar_get", 2, PIVarGetU) ;

	InstallNDeterCBuiltinPred("current_ivar", 2, PNDCurrentIVar) ;
	InstallCBuiltinPred("ivars", 0, PIVars) ;
}
