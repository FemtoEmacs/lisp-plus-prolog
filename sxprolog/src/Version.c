/*
 *   This file is part of the CxProlog system

 *   Version.c
 *   by A.Miguel Dias - 2004/06/28
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

static Str32 vapplName = "XXXX" ;
static int vmajor = 0 ;
static int vminor = 0 ;
static int vpatch = 0 ;
static int vmaturity = 0 ;
static int vvariant = 1 ;

void VersionSet(Str applName, int major, int minor, int patch, int maturity, int variant)
{
	if( applName != nil ) strncpy(vapplName, applName, 30) ;
	vmajor = major ;
	vminor = minor ;
	vpatch = patch ;
	vmaturity = maturity ;
	vvariant = variant ;
}

int VersionGet()
{
	return vmajor * 10000 + vminor * 100 + vpatch * 10 + vmaturity ;
}

static Pt MakeExtraList(void)
{
	Pt list = tNilAtom ;
	Str s ;
	if( vvariant != 1 ) {
		s = GStrFormat("variant_%d", vvariant) ;
		list = MakeList(MakeAtom(s), list) ;		
	}
	switch( vmaturity ) {
		case -1: s = "rc" ; break ;
		case -2: s = "beta" ; break ;
		case -3: s = "alfa" ; break ;
		case -4: s = "development" ; break ;
		default: s = nil ; break ;
	}
	if( s != nil )
		list = MakeList(MakeAtom(s), list) ;
	return list ;
}
	
Str VersionString()
{
	Hdl saveH = H ;
	Pt e = MakeExtraList() ;
	Str s = GStrFormat("%s version %d.%d", vapplName, vmajor, vminor) ;
	if( vpatch != 0 )
		s = GStrFormat("%s.%d", s, vpatch) ;
	if( e != tNilAtom ) {
		s = GStrFormat("%s %s", s, TermAsStr(e)) ;
		H = saveH ;
	}
	return s ;
}

Pt VersionTerm()
{
	FunctorPt f = LookupFunctorByName("cxprolog", 4) ;
	Pt args[4] ;	
	args[0] = MakeInt(vmajor) ;
	args[1] = MakeInt(vminor) ;
	args[2] = MakeInt(vpatch) ;
	args[3] = MakeExtraList() ;
	return MakeStruct(f, args) ;
}





void ShowVersion()
{
	Write("%s\n", VersionString()) ;
}


/* CXPROLOG C'BUILTINS */

static void PVersion()
{
	Write("%s\n", VersionString()) ;
	JumpNext() ;
}

static void PVersion1()
{
	MustBe( UnifyWithAtomic(X0, MakeAtom(VersionString()))) ;
	JumpNext() ;
}

void VersionInit()
{
	InstallCBuiltinPred("version", 0, PVersion) ;
	InstallCBuiltinPred("version", 1, PVersion1) ;
}
