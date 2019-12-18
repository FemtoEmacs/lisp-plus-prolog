/*
 *   This file is part of the CxProlog system

 *   ImperativeVar.h
 *   by A.Miguel Dias - 2007/12/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Alias_
#define _Alias_

ExtraPt AliasGet(ExtraTypePt e, Pt atom) ;
void AliasSet(Pt atom, VoidPt extra) ;
void AliasUnset(Pt atom, VoidPt extra) ;
AtomPt AliasSearch(VoidPt extra) ;	/* Slow */
void AliasedWithWrite(Pt t) ;
void AliasInit(void) ;

#endif
