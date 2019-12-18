/*
 *   This file is part of the CxProlog system

 *   CmdLine.c
 *   by A.Miguel Dias - 2002/01/19
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

static int argc ;
static CharPt *argv ;
static Str bootFileName = nil ;

void SpecifyBootFile(Str boot)
{
	bootFileName = boot ;
}

static void CmdLineFileShebangExpansion(CharPt cmd, CharPt fname)
{
	FILE *f  ;
	CharPt s = nil ;
	if( (f = fopen(fname, "r")) != nil ) {
		CharPt line = BigStrBegin() ;
		if( fgets(line, 1000, f) != nil && strncmp(line, "#!", 2) == 0 ) {
			StrLast(line) = '\0' ;  /* remove newline */
			s = StrAllocate(line + 2) ;
		}
		fclose(f) ;
	}
	if( s != nil ) {
		int n = StrCountElems(s, ' ') ;
		Str *mem = Allocate(n + 1, false) ;
		StrSplitElems(s, mem, ' ') ;
		mem[0] = cmd ;
		mem[n] = fname ;
		argc = n + 1 ;
		argv = cCharHdl(mem) ;
	}
	else {
		CharHdl mem = Allocate(3, false) ;
		mem[0] = cmd ;
		mem[1] = "--script" ;
		mem[2] = fname ;
		argc = 3 ;
		argv = mem ;
	}
}

static void CmdLineShebangArg1Expansion(int ac, CharHdl av)
{
	Unused(ac) ;
	int n = StrCountElems(av[1], ' ') ;
	Str *mem = Allocate(n + 2, false) ;
	mem[0] = av[0] ;
	StrSplitElems(av[1], mem+1, ' ') ;
	mem[n+1] = av[2] ;
	argc = n + 2 ;
	argv = cCharHdl(mem) ;
}

static Bool GoolPlFileName(Str s)
{
	int n = strlen(s) ;
	return n >= 4 && strncmp(s + n - 3, ".pl", 3) == 0 ;
}

void CmdLineInit(int ac, CharHdl av)
{
	argc = 0 ;
	switch( ac ) {
		case 2:
			if( GoolPlFileName(av[1]) )
				CmdLineFileShebangExpansion(av[0], av[1]) ;
			break ;
		case 3:
			if( StrEqual(av[1], "--script") )
				CmdLineFileShebangExpansion(av[0], av[2]) ;
			else
				CmdLineShebangArg1Expansion(ac, av) ;
			break ;
	}
	if( argc == 0 ) {
		argc = ac ;
		argv = av ;
	}
#if 0
	int i ;
	Mesg("%d", argc) ;
	for( i = 0 ; i < argc ; i++ )
		Mesg("%s", argv[i]) ;
#endif
}

CharPt CmdLineArg(Str sw)
{
	register int i ;
	if( sw[0] == '\0' || sw[0] != '-' )
		Error("First argument should be an atom starting with a '-'") ;
	if( bootFileName != nil && StrEqual("--boot", sw) )
		return cCharPtz(bootFileName) ;
	for( i = 1 ; i < argc ; i++ )
		if( StrEqual(argv[i], sw) ) {
			if( i+1 < argc && argv[i+1][0] != '-' )
				return argv[i+1] ;
			else return "" ;
		}
	return nil ;
}

Pt ZOSGetArgs()
{
	register int i ;
	register Pt list ;
	ZEnsureFreeSpaceOnStacks(2 * argc + 2, 0, true) ; /* stacks may grow */
	list = tNilAtom ;
	if( bootFileName != nil ) {
		list = MakeList(MakeTempAtom(bootFileName), list) ;
		list = MakeList(MakeTempAtom("--boot"), list) ;
	}
	for( i = argc - 1 ; i >= 0 ; i-- )
		list = MakeList(MakeTempAtom(StrInternalize(argv[i])), list) ;
	return list ;
}


/* CXPROLOG C'BUILTINS */

static void POSGetArg()
{
	CharPt s ;
	Pt t = Drf(X0) ;
	if( IsAtom(t) ) {
		MustBe( (s = CmdLineArg(StrExternalize(XAtomName(t)))) != nil
				&& Unify(X1, MakeTempAtom(StrInternalize(s))) ) ;
	}
	elif( IsList(t) ) {
		for( ; IsList(t) ; t = Drf(XListTail(t)) )
			if( (s = CmdLineArg(StrExternalize(XTestAtomName(XListHead(t))))) != nil )
				MustBe( Unify(X1, MakeTempAtom(StrInternalize(s))) ) ;
		DoFail() ;
	}
	else
		TypeError("ATOM", t) ;
}

static void POSGetArgs()
{
	Pt args = ZOSGetArgs() ;
	MustBe( Unify(X0, args) ) ;
}

void CmdLineInit2()
{
	InstallCBuiltinPred("os_arg", 2, POSGetArg) ;
	InstallCBuiltinPred("os_args", 1, POSGetArgs) ;
}
