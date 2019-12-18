/*
 *   This file is part of the CxProlog system

 *   TestLib.c
 *   by A.Miguel Dias - 2009/10/20
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

#if TESTLIB

#include "CxProlog.h"

#define DEBUG		0
#define GOAL_LIMIT	100

static void ShowGoal(int n, Str goal)
{
	if( n > 1 ) Write("    ") ;
#if DEBUG
	Write("GOAL%d[%d] = %s, H = %lx, B = %lx, L = %lx\n", n, CallLevel(), goal, H, B, L) ;
#else
	Write("GOAL = %s\n", goal) ;
#endif
}

static void ShowRes(int n, PrologEvent pe, Str vars, Str out)
{
	Pt t ;
	if( n > 1 ) Write("    ") ;
	if( pe == peException ) WriteException(nil) ;
	if( n > 1 ) Write("    ") ;
#if DEBUG
	Write("-EV%d[%d] = %s, H = %lx, B = %lx, L = %lx\n", n, CallLevel(), PrologEventAsStr(pe), H, B, L) ;
	if( n > 1 ) Write("    ") ;
	Write("RESULT%d = %s\n", n, TermAsStr(CallPrologGetResult())) ;
	if( n > 1 ) Write("    ") ;
	Write("OUTPUT%d = %s", n, out[0] == '\0' ? "*NONE*\n" : out) ;
	if( n > 1 ) Write("    ") ;
	Write("VARS%d = %s",  n, vars[0] == '\0' ? "*NONE*\n" : vars) ;
#else
	Write("-EV = %s\n", PrologEventAsStr(pe)) ;
	if( (t = CallPrologGetResult()) != tNoResult )
		Write("RES = %s\n", TermAsStr(t)) ;
	if( out[0] != '\0' ) {
		if( n > 1 ) Write("    ") ;
		Write("OUT=%s", out) ;
	}
	if( vars[0] != '\0' ) {
		if( n > 1 ) Write("    ") ;
		Write("VARS= %s",  vars) ;
	}
#endif
}

static void CallPM(Str goal) {
	PrologEvent pe ;
	Str vars, out ;
	int n = 0 ;
	Write("--------\n") ;
	ShowGoal(0, goal) ;
	for( pe = CallPrologMultiStrTop(goal, &vars, &out) ; pe == peSucc ; pe = CallPrologMultiNextTop(&vars, &out) )
	{
		ShowRes(0, pe, vars, out) ;
		if( n++ == GOAL_LIMIT ) {
			Write("___*** STOPPED ***___\n") ;
			pe = CallPrologMultiStopTop(&vars, &out) ;
			break ;
		}
	}
	ShowRes(0, pe, vars, out) ;	
	Write("--------\n\n") ;
}

static void CallPM2(Str goal1, Str goal2) {
	PrologEvent pe1, pe2 ;
	Str vars1, vars2, out1, out2 ;
	Write("--------\n") ;
	ShowGoal(1, goal1) ;
	for( pe1 = CallPrologMultiStrTop(goal1, &vars1, &out1) ; pe1 == peSucc ; pe1 = CallPrologMultiNextTop(&vars1, &out1) )
	{
		ShowRes(1, pe1, vars1, out1) ;
		{
			ShowGoal(2, goal2) ;
			for( pe2 = CallPrologMultiStrTop(goal2, &vars2, &out2) ; pe2 == peSucc ; pe2 = CallPrologMultiNextTop(&vars2, &out2) )
				ShowRes(2, pe2, vars2, out2) ;
			ShowRes(2, pe2, vars2, out2) ;
		}	
	}
	ShowRes(1, pe1, vars1, out1) ;
	Write("--------\n\n") ;
	
}

static void CallP(int z, Str goal) {
	PrologEvent pe ;
	Str vars, out ;
	Write("--------\n") ;
	ShowGoal(0, goal) ;
	pe = CallPrologStrTop(goal, &vars, &out) ;
	ShowRes(0, pe, vars, out) ;	
	Write("--------\n\n") ;
}

static void Test0(void)
{
	CallPM("true") ;
	CallPM("fail") ;
	CallPM("see('no such file')") ;
	CallPM("exit") ;
	CallPM("halt") ;
	CallPM("abort") ;
	CallPM("'$$_fatal_error'(aa)") ;
	CallPM("e r r o r") ;
	CallPM("writeln(a)") ;
	CallPM("writeln(a); writeln(b); (!, fail); writeln(c)") ;
}

static void Test1(void)
{
	static CharPt code = "						\
		ola :- writeln(ola).					\
		ole :- writeln(ole).					\
												\
		double(X,Y) :- Y is 2 * X.				\
												\
		q(f(3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3)).	\
		q(f(0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3)).	\
		q(f(a)).								\
		q(f(a)).								\
		q(f(a,a,a,a,a)).						\
		q(f(a)).								\
		q(f(a)).								\
	" ;

	ZBasicLoadStr(code) ;
	CallPM("ola, ole") ;
	CallPM("double(5,Y)") ;
	CallPM("context(X), q(Y), writeln(X)") ;
}

static void Test2(void)
{
	CallPM("'$$_call_prolog_through_c'(writeln(a))") ;
	CallPM("'$$_call_prolog_through_c'(fail)") ;
	CallPM("'$$_call_prolog_through_c'(see(aa))") ;
	CallPM("'$$_call_prolog_through_c'('$$_call_prolog_through_c'(see(aa)))") ;
}


static void Test3(void)
{
	static CharPt code = "			\
		n(0).						\
		n(X) :- n(Y), X is Y+1.		\
	" ;

	ZBasicLoadStr(code) ;
	CallPM("n(X)") ;
}

static void Test4(void)
{
	static CharPt code = "			\
		r(0) :- set_result(aaa).	\
		r(1) :- set_result(bbb).	\
		r(0) :- set_result(ccc).	\
	" ;

	ZBasicLoadStr(code) ;
	CallPM("r(0)") ;
}

static void Test5(void)
{
	static CharPt code = "			\
		zx(a).						\
		zx(b).						\
		zx(c).						\
		zy(111).					\
		zy(222).					\
	" ;
	
	ZBasicLoadStr(code) ;
	CallPM2("zx(X)", "zy(Y)") ;
}



int main(int argc, char **argv) {
	StartProlog(argc, argv, nil) ;
	Test1() ;
	StopProlog() ;
	Write("Finished tests\n") ;
	return 0 ;	
}

#endif
