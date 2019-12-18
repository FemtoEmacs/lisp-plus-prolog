/*
 *   This file is part of the CxProlog system

 *   ImperativeVar.h
 *   by A.Miguel Dias - 2000/04/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _ImperativeVar_
#define _ImperativeVar_

void IVarSet(AtomPt atom, Pt value, Bool cons) ;
void IVarForceSet(AtomPt atom, Pt value, Bool cons) ;
void IVarReversibleRestore(AtomPt atom, Pt oldValue) ;
Pt IVarGet(AtomPt atom) ;
void IVarsWithWrite(Pt t) ;
void IVarsInit(void) ;

#endif
