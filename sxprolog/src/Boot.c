/*
 *   This file is part of the CxProlog system

 *   Boot.c
 *   by A.Miguel Dias - 2003/06/11
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

/* The following built-in predicates should not be redefined, not even inside
	an alternative boot file */

static Str coreBuiltinsStr = "					\
												\
/* ACTIVATION */							  ""\
												\
'$$_call_prolog'(G) :-	once(G), '$$_return_success'.	\
'$$_call_prolog_multi'(G) :- G, '$$_more_success'.		\
												\
'$$_run_thread'(G) :- 							\
	once(G),									\
	'$$_active_thread_set_success', fail.		\
'$$_run_thread'(_) :-							\
	'$$_active_thread_return'.					\
												\
'$$_lux0' :-									\
	'$$_tests0',								\
	try('$$_handle_arg'(quiet)),				\
	try('$$_handle_arg'(boot)),					\
	try('$$_handle_arg'(sysgoal)),				\
	dynamic('$$_top_call'/1), 					\
	dict_new('$$_gensym_dict'),					\
	'$$_enter_user_mode', 	 					\
	try('$cxprolog_initialise'),				\
	try('$$_handle_arg'(test)),					\
	try('$$_handle_arg'(script)),				\
	try('$$_handle_arg'(goal)).					\
												\
'$$_lux1' :-									\
	try('$cxprolog_top_level_goal').			\
												\
'$$_tests0' :-									\
	rename_builtin('$$_test_rename_builtin', 0,	\
				'$$_test_rename_builtin2').		\
												\
'$$_handle_arg'(quiet) :-						\
	os_arg(['-q','--quiet'],'')					\
	 -> set_prolog_flag(info_messages, silent).	\
'$$_handle_arg'(boot) :-						\
	os_arg('--boot',X)							\
	-> call_on_empty_context(silent_consult(X))	\
			; '$$_default_boot'.				\
'$$_handle_arg'(sysgoal) :-						\
	os_arg(['-y','--sysgoal'],X)				\
			-> atom_term(X,C), call(C).			\
'$$_handle_arg'(test) :-						\
	os_arg('--test',_)							\
			-> '$$_test_instalation'.			\
'$$_handle_arg'(script) :-						\
	is_script(S) -> silent_consult(S).			\
'$$_handle_arg'(goal) :-						\
	os_arg(['-g','--goal'],X)					\
			-> atom_term(X,C), call(C).			\
												\
is_script(S) :-									\
	os_arg(['-s','--script'], S)				\
  ; os_args([_, S]), slice(S,-3,-1,'.pl').		\
												\
exit_script :-									\
	is_script(_), '$$_consult_level'(1)			\
		-> exit									\
		; true.									\
												\
exit_script_fast :-								\
	is_script(_), '$$_consult_level'(1)			\
		-> '$$_exit_script_fast'				\
		; true.									\
												\
												\
/* CORE PREDICATES	*/						  ""\
												\
\\+ G :- G, !, fail.							\
\\+ _.											\
												\
not(G) :- \\+ G.								\
												\
try(G) :- G, !.									\
try(G).											\
												\
once(G) :- G, !.								\
												\
gen(G) :- G, fail.								\
gen(G).											\
												\
possible(G) :- \\+ G, !, fail.					\
possible(_).									\
												\
with_ivar(I,V,G) :-								\
	'$$_ivar_get'(I,X), I &:= V, G, I &:= X.	\
												\
If -> Then :-									\
	If, !,'$$_transparent_call'(Then).			\
If -> Then.	/* PATCHED */						\
												\
If *-> Then :- 									\
	If, '$$_transparent_call'(Then).			\
												\
If -> Then ; Else :-							\
	If, !, '$$_transparent_call'(Then).			\
If -> Then ; Else :-							\
	!, '$$_transparent_call'(Else).				\
												\
If *-> Then ; Else :-							\
	!,											\
	gensym('$$_softcut', IVar),					\
	IVar:=false,								\
	(	If,										\
		IVar:=true,								\
		'$$_transparent_call'(Then)				\
	;											\
		IVar=:X,								\
		ivar_delete(IVar),						\
		X==false,								\
		'$$_transparent_call'(Else)				\
	).											\
												\
A;_ :- '$$_transparent_call'(A).				\
_;B :- '$$_transparent_call'(B).				\
												\
A,B :-											\
	'$$_transparent_call'(A),					\
	'$$_transparent_call'(B).				  ""\
												\
T =.. [F|As] :- nonvar(T), !,					\
	functor(T,F,N),	'$$_univg'(As,T,0,N).		\
T =.. [F|As] :-									\
	nonvar(As), '$$_univ'(As,T,F,0).			\
'$$_univg'(L,_,N,N) :- !, L = [].				\
'$$_univg'([A|As],T,M,N) :-						\
	M1 is M+1, arg(M1,T,A),						\
	'$$_univg'(As,T,M1,N).						\
'$$_univ'([],T,F,N) :- functor(T,F,N), !.		\
'$$_univ'([A|As],T,F,N) :-						\
	nonvar(As),									\
	N1 is N+1, '$$_univ'(As,T,F,N1), !,			\
	arg(N1,T,A).								\
												\
(A :- B) :- B == true, !, '$$_user_assert'(A).	\
(A :- B) :- '$$_user_assert'((A :- B)).			\
'$$_user_assert'(C) :-							\
	assertz(C),									\
	numbervars(C,0,_),							\
	writeq(user,C),								\
	writeln(user,' asserted.'),					\
	abort.									\
												\
'$$_check_curr_unit_missing' :-					\
	predicate_property(X, undefined),			\
	predicate_property(X, full_name(Pred)),		\
	warning(['Predicate ''', Pred, ''' is used',\
		' but undefined in its unit']),			\
	fail.										\
'$$_check_curr_unit_missing'.					\
												\
check_missing :-								\
	current_unit(U),							\
		'$$_ctx_ext'(U,'$$_check_curr_unit_missing'),	\
	fail.										\
check_missing.									\
											  ""\
'$$_check_curr_unit_imports'(H) :-				\
	predicate_property(H, imported_from(U)),	\
	predicate_property(H, full_name(Pred)),		\
	functor(U,NU,AU), functor(UU,NU,AU),		\
	NU \\== '$$_unit_parameter',				\
	(current_unit(UU)							\
	->  ('$$_ctx_ext'(U,predicate_property(H,visible))	\
		-> true									\
		; warning(['Imported predicate ''',		\
				Pred, ''' is not visible in ',	\
				'unit ''', NU/AU, ''''])		\
		)										\
	; warning(['Predicate ''', Pred, ''' is ',	\
		'imported from nonexistent unit ''',	\
		NU/AU, ''''])							\
	),											\
	fail.										\
'$$_check_curr_unit_imports'(_).				\
												\
check_imports :-								\
	current_unit(U),							\
		'$$_ctx_ext'(U,'$$_check_curr_unit_imports'(_)),	\
	fail.										\
check_imports.								  ""\
												\
check_globals.	/* TODO */						\
												\
visible_predicate(P) :-							\
	predicate_property(P, visible).				\
imported_predicate(P,UD) :-						\
	predicate_property(P, imported_from(UD)).	\
builtin_predicate(P) :-							\
	predicate_property(P, built_in).			\
system_predicate(P) :-							\
	predicate_property(P, built_in).			\
												\
mutable_builtin(N,A) :-							\
	dynamic(N/A).								\
												\
nodebug :- set_prolog_flag(debug, off).			\
notrace :- set_prolog_flag(debug, off).			\
debug :- set_prolog_flag(debug, debug).			\
trace :- set_prolog_flag(debug, trace).			\
												\
zpf :-											\
	call((logtalk_load(library(all_loader)))),	\
	writeln('-------------------------------'),	\
	call((logtalk_load([pflat]))).				\
												\
/* CONTEXTS	*/								  ""\
												\
context_top(U) :- context([U|_]).				\
unit_param(I,P) :-								\
	context([U|_]), arg(I,U,P).					\
unit_arity(A) :-								\
	context([U|_]), functor(U,_,A).				\
show_context :- context(X), writeqln(X).		\
show_context(S) :-								\
	context(X), writeqln(S,X).					\
												\
call_on_context([],G) :-						\
	call_on_empty_context(G).					\
call_on_context([U|C],G) :-						\
	call_on_context(C,'$$_ctx_ext'(U,G)).		\
												\
import(from(P,U))	:- import(P,U).				\
												\
/* HISTORIC CONTEXT	*/						  ""\
												\
show_hcontext :- hcontext(X), writeqln(X).		\
show_hcontext(S) :-								\
	hcontext(X), writeqln(S,X).					\
												\
/* OPERATORS */								  ""\
												\
current_op(Pri, Type, Op) :-					\
	'$$_current_op_aux'(Op),					\
	'$$_op'(Type, Num),							\
	'$$_is_op'(Op, Num, Pri, Left, Right),		\
	'$$_op'(Type, Pri, Left, Right).			\
												\
'$$_op'( fx, 0).								\
'$$_op'( fy, 0).								\
'$$_op'( xf, 2).								\
'$$_op'( yf, 2).								\
'$$_op'(xfy, 1).								\
'$$_op'(xfx, 1).								\
'$$_op'(yfx, 1).								\
												\
'$$_op'( fx, Q, _, P) :- succ(P, Q).			\
'$$_op'( fy, Q, _, Q).							\
'$$_op'(xf,  Q, P, _) :- succ(P, Q).			\
'$$_op'(yf,  Q, Q, _).							\
'$$_op'(xfy, Q, P, Q) :- succ(P, Q).			\
'$$_op'(xfx, Q, P, P) :- succ(P, Q).			\
'$$_op'(yfx, Q, Q, P) :- succ(P, Q).			\
												\
/* CHECK INSTALATION */						  ""\
												\
'$$_test_instalation' :-						\
	zpl, silent_consult(test_all), exit.		\
" ;


/* The following built-in predicates may be redefined inside
   an alternative boot file */

static Str predefinedBuiltinsStr = "			\
												\
/* UTILITIES */								  ""\
												\
app([], L, L).									\
app([H|T], L, [H|R]) :- app(T, L, R).			\
												\
forall(G1, G2) :- \\+ (G1, \\+ G2).				\
												\
retractall(X) :- retract(X), fail.				\
retractall(X) :- retract((X:-_)), fail.			\
retractall(_).									\
												\
'$bind'([Name=Name|Rest]) :- '$bind'(Rest).		\
'$bind'([]).									\
												\
findall(T,G,L) :-								\
	queue_new(Q),								\
	forall(G, queue_put(Q,T)),					\
	queue_as_list(Q, L),						\
	queue_delete(Q).							\
												\
setof(T,G,S) :-									\
	bagof(T, G, B),								\
	sort(B, S).									\
												\
gensym(Prefix, Atom) :-							\
	(dict_get('$$_gensym_dict',Prefix,N)		\
		-> true ; N = 0),						\
	succ(N,M),									\
	dict_set('$$_gensym_dict',Prefix,M),		\
	concat([Prefix,M], Atom).					\
												\
gensym(Atom) :- gensym('%', Atom).				\
												\
add_pl(user,user) :- !.							\
add_pl(user_input,user) :- !.					\
add_pl(A,A) :- slice(A,-3,-1,'.pl'), !.			\
add_pl(A,A) :- fs_exists_file(A), !.			\
add_pl(A,B) :-									\
	concat([A,'.pl'], B), fs_exists_file(B), !.	\
add_pl(A,A).									\
												\
/* CONSULT */								  ""\
												\
[].												\
[-File|Files] :- !, reconsult(File), Files.		\
[File|Files] :- consult(File), Files.			\
												\
compile(Files) :- consult(Files).				\
												\
consult(Files) :-								\
	'$$_map_consult'(Files,						\
		'$$_consult_one',false,false,false).	\
												\
silent_consult(Files) :-						\
	'$$_map_consult'(Files,						\
		'$$_consult_one',false,true,false).		\
												\
ensure_loaded(Files) :-							\
	'$$_map_consult'(Files,						\
		'$$_consult_one',false,false,true).		\
												\
reconsult(Files) :-	/* for compatibility */		\
	'$$_map_consult'(Files,						\
		'$$_consult_one',true,false,false).		\
												\
silent_reconsult(Files) :-						\
	'$$_map_consult'(Files,						\
		'$$_consult_one',true,true,false).		\
												\
'$enter_consult'(S) :-							\
	'$$_enter_consult'(S,false,false,false).	\
												\
'$exit_consult' :-								\
	'$$_consult_get_initialization_list'(L),	\
	'$$_exit_consult_initializions'(L),			\
	'$$_exit_consult'.							\
'$$_exit_consult_initializions'([]).			\
'$$_exit_consult_initializions'([H|T]) :-		\
	(:- H),										\
	'$$_exit_consult_initializions'(T).			\
												\
'$$_map_consult'([],_,_,_,_) :- !.				\
'$$_map_consult'([File|Files],G,B,FS,EL) :- !,	\
	'$$_map_consult'(File,G,B,FS,EL),			\
	'$$_map_consult'([File|Files],G,B,FS,EL).	\
'$$_map_consult'(File,G,B,FS,EL) :-				\
	call(G,File,B,FS,EL).						\
												\
'$$_consult_one'(File,B,FS,EL) :-				\
	'$$_check_consult'(B),						\
	add_pl(File, F), 							\
	( F == user -> S = user ; open(F,read,S) ),	\
	( '$$_enter_consult'(S,B,FS,EL)				\
	-> '$$_do_consult_one'(F,S,B)				\
	;  true ),									\
	close(S).									\
												\
'$$_do_consult_one'(F,S,B) :-					\
	'$$_consult_prep_report'(T0,H0),			\
/* now skip #!cxprolog ... first line */		\
	( peek(S,0'#) -> get_line(S,_) ; true ),	\
	'$$_c_loop',								\
	( B==true -> K=reconsulted ; K=consulted ),	\
	'$$_consult_do_report'(T0,H0,F,K),			\
	'$exit_consult'.							\
											  ""\
'$$_include_one'(F) :-							\
	open(F,read,S),								\
	'$$_consult_prep_report'(T0,H0),			\
	'$$_enter_include'(S),						\
	'$$_c_loop',								\
	'$$_consult_do_report'(T0,H0,F,included),	\
	'$$_exit_include',							\
	close(S).									\
												\
'$$_consult_prep_report'(T0,H0) :-				\
	T0 is cputime, H0 is heapused.				\
												\
'$$_consult_do_report'(_,_,_,_) :-				\
	'$$_consult_is_silent', !.					\
'$$_consult_do_report'(T0,H0,F,K) :-			\
	Time is cputime - T0,						\
	Heap is heapused - H0,						\
	write(user,'% File '), writeq(user,F),		\
	write(user, ' '),							\
	write(user, K),								\
	write(user, ', '),							\
	write(user,Time), write(user,' sec '),		\
	write(user,Heap), writeln(user,' bytes').	\
											  ""\
'$$_c_loop' :-									\
	repeat,										\
		catch('$$_c_do_loop', X,				\
			(write_exception(X), fail)),		\
	!.											\
												\
'$$_c_do_loop' :-								\
	consulting(S),								\
	repeat,										\
		read(S, Term),							\
	'$$_c_term'(Term), !.						\
												\
'$$_c_term'(X) :- var(X), !,					\
	'$consult_clause'(X).						\
'$$_c_term'(end_of_file) :- !.					\
'$$_c_term'(eof) :- !.							\
'$$_c_term'(unit(U)) :- !,						\
	varnames(Vars), '$bind'(Vars),				\
	create_unit(U), '$$_ctx_ext'(U,'$$_c_loop'). \
'$$_c_term'(multifile(Spec)) :- !,				\
	multifile(Spec), fail.						\
'$$_c_term'(discontiguous(Spec)) :- !,			\
	discontiguous(Spec), fail.					\
'$$_c_term'(visible(Spec)) :- !,				\
	visible(Spec), fail.						\
'$$_c_term'(xnew(Spec)) :- !,					\
	xnew(Spec), fail.							\
'$$_c_term'(xover(Spec)) :- !,					\
	xover(Spec), fail.							\
'$$_c_term'(gopen(Spec)) :- !,					\
	gopen(Spec), fail.							\
'$$_c_term'(gclose(Spec)) :- !,					\
	gclose(Spec), fail.							\
'$$_c_term'(hidden(Spec)) :- !,					\
	hidden(Spec), fail.							\
'$$_c_term'(import(from(Spec,U))) :- !,			\
	import(Spec,U), fail.						\
'$$_c_term'(dynamic(Spec)) :- !,				\
	dynamic(Spec), fail.						\
'$$_c_term'(dynamic_iu(Spec)) :- !,				\
	dynamic_iu(Spec), fail.						\
'$$_c_term'(index(Spec,N)) :- !,				\
	index(Spec,N), fail.						\
'$$_c_term'((:-X)) :- !,						\
	'$consult_directive'(X), fail.				\
'$$_c_term'((?-X)) :- !,						\
	varnames(V), question(X,V), !, fail.		\
'$$_c_term'(X) :- '$consult_clause'(X), fail.	\
												\
'$consult_directive'(initialization(G)) :- !,	\
	try('$$_consult_store_initialization'(G)).	\
'$consult_directive'(include(F)) :- !,			\
	try('$$_include_one'(F)).					\
'$consult_directive'(G) :-						\
	try((:-G)), !.								\
												\
/* LISTING */								  ""\
												\
all :-											\
	current_unit(U),							\
		'$$_ctx_ext'(U,listing),				\
	fail.										\
all.											\
												\
list :- listing.								\
												\
listing :-										\
	'$$_listing_header','$$_listing'(_).		\
												\
listing(V) :- var(V),!,'$$_listing_clauses'(V).	\
listing(A) :- atom(A), !, '$$_listing_atom'(A).	\
listing(F/N) :- !, listing(F,N).				\
listing(H) :- '$$_listing_clauses'(H).			\
listing(F,N) :-									\
	functor(H,F,N), '$$_listing'(H).			\
												\
'$$_listing_header' :-							\
	unit_spec(USpec)							\
		->	write('**** '),						\
			write(unit(USpec)),					\
			writeln(' ****************')		\
		;	true.								\
											  ""\
'$$_listing_atom'(A) :-							\
	forall(current_predicate(A,H), '$$_listing_multifiles'(H)),		\
	forall(current_predicate(A,H), '$$_listing_discontiguous'(H)),	\
	forall(current_predicate(A,H), '$$_listing_visibles'(H)),		\
	forall(current_predicate(A,H), '$$_listing_xnew'(H)),			\
	forall(current_predicate(A,H), '$$_listing_xover'(H)),			\
	forall(current_predicate(A,H), '$$_listing_gopen'(H)),			\
	forall(current_predicate(A,H), '$$_listing_gclose'(H)),			\
	forall(current_predicate(A,H), '$$_listing_hidden'(H)),			\
	forall(current_predicate(A,H), '$$_listing_imports'(H)),		\
	forall(current_predicate(A,H), '$$_listing_dynamics'(H)),		\
	forall(current_predicate(A,H), '$$_listing_dynamics_iu'(H)),	\
	forall(current_predicate(A,H), '$$_listing_clauses'(H)), nl.	\
												\
'$$_listing'(H) :-								\
	'$$_listing_multifiles'(H),					\
	'$$_listing_discontiguous'(H),				\
	'$$_listing_visibles'(H),					\
/*	'$$_listing_xnew'(H),	*/					\
	'$$_listing_xover'(H),						\
	'$$_listing_gopen'(H),						\
	'$$_listing_gclose'(H),						\
/*	'$$_listing_hidden'(H),	*/					\
	'$$_listing_imports'(H),					\
	'$$_listing_dynamics'(H),					\
	'$$_listing_dynamics_iu'(H),				\
	'$$_listing_clauses'(H), nl.				\
												\
'$$_listing_kind'(H,K) :-						\
	queue_new(Q),								\
	forall(predicate_property(H, K),			\
		(functor(H,F,A), queue_put(Q,F/A))),	\
	(queue_as_list(Q,D), D \\== []				\
	 -> write(':- '), write(K), write(' '),	  ""\
		'$$_listing_decls'(D), writeln('.')		\
	 ;  true),									\
	 queue_delete(Q).							\
												\
'$$_listing_decls'([H]) :- !, write(H).			\
'$$_listing_decls'([H|T]) :-					\
	write(H), write(', '),						\
	'$$_listing_decls'(T).						\
												\
'$$_listing_multifiles'(H) :-					\
	'$$_listing_kind'(H, multifile).			\
												\
'$$_listing_discontiguous'(H) :-				\
	'$$_listing_kind'(H, discontiguous).		\
												\
'$$_listing_visibles'(H) :-						\
	'$$_listing_kind'(H, visible).				\
												\
'$$_listing_imports'(H) :-						\
	predicate_property(H, imported_from(U)),	\
	functor(H,F,A),								\
	writeq((:-import(from(F/A,U)))), writeln('.'),\
	'$$_check_curr_unit_imports'(H),			\
	fail.										\
'$$_listing_imports'(_).						\
												\
'$$_listing_xnew'(H) :-							\
	'$$_listing_kind'(H, xnew).					\
												\
'$$_listing_xover'(H) :-						\
	'$$_listing_kind'(H, xover).				\
												\
'$$_listing_gopen'(H) :-						\
	'$$_listing_kind'(H, gopen).				\
												\
'$$_listing_gclose'(H) :-						\
	'$$_listing_kind'(H, gclose).				\
												\
'$$_listing_hidden'(H) :-						\
	'$$_listing_kind'(H, hidden).				\
												\
'$$_listing_dynamics'(H) :-						\
	'$$_listing_kind'(H, dynamic).				\
												\
'$$_listing_dynamics_iu'(H) :-					\
	'$$_listing_kind'(H, dynamic_iu).			\
												\
'$$_listing_clauses'(H) :-						\
	current_predicate(_, H),				  ""\
		clause(H,B),							\
			portray_clause((H:-B)),				\
	fail.										\
'$$_listing_clauses'(_).						\
												\
portray_clause((H:-true)) :- !,					\
	numbervars(H, 0, _),						\
	writeq(H),writeln('.').						\
portray_clause((H:-B)) :-						\
	numbervars((H:-B), 0, _),					\
	writeq(H), writeln(' :-'),					\
	'$$_portray_body'(B),						\
	writeln('.').								\
												\
'$$_portray_body'(X) :- var(X), !,				\
	tab(8), writeq(X).							\
'$$_portray_body'((X,Xs)) :-	!,				\
	tab(8), writeq(X), writeln(','),			\
	'$$_portray_body'(Xs).						\
'$$_portray_body'(X) :-							\
	tab(8), writeq(X).							\
												\
/* QUESTION INTERACTION */					  ""\
												\
(:-G) :- G, !.									\
(:-G) :-										\
	write(user_error,'? '),						\
	writeqln(user_error,G).						\
												\
(?-G) :- G.										\
												\
question((:-G),_) :-							\
	!, (:-G).									\
question(G,[]) :-								\
	G, deterministic(D),						\
	write(user,'yes'), '$$_morez'(D), !.		\
question(G,[H|T]) :-							\
	G, deterministic(D),						\
	'$$_show_vars'([H|T]), '$$_morez'(D), !.	\
question(_,_) :-								\
	writeln(user,'no').							\
												\
'$$_morez'(true) :- writeln(user,'').			\
'$$_morez'(false) :-							\
	'$$_getnb'(X), '$$_more'(X).				\
'$$_more'(0';) :- !, skip(user,10), fail.		\
'$$_more'(10) :- !, writeln(user,'...').		\
'$$_more'(-1) :- !, loud_end_of_file.			\
'$$_more'(_) :- skip(user,10), '$$_more'(10).	\
'$$_getnb'(X) :-								\
	prompt(P, '? '),							\
	repeat, get0(user,X), X	\\= 32, !,			\
	prompt(_, P).								\
												\
'$$_show_vars'([]).								\
'$$_show_vars'([S]) :- !,						\
	'$$_show_one_var'(S).						\
'$$_show_vars'([H|T]) :-						\
	'$$_show_one_var'(H), nl(user),				\
	'$$_show_vars'(T).							\
'$$_show_one_var'(Var=Val) :-					\
	write(user,Var),							\
	write(user,' = '),							\
	writeq(user,Val),							\
	write(user,' ').							\
												\
/* BAGOF									  ""\
	Adapted from C-Prolog: David Warren,		\
	Fernando Pereira, R.A.O'Keefe.	*/			\
												\
Variable^Goal :- Goal.							\
												\
bagof(Template, Generator, Bag) :-				\
	'$$_excess_vars0'(Generator, Template,[], FreeVars),	\
	writeln(FreeVars),	\
	FreeVars \\== [], !,						\
	Key =.. ['$$_'|FreeVars],					\
	findall(Key-Template, Generator, Bags0),	\
	keysort(Bags0, Bags),						\
	'$$_pick'(Bags, Key, Bag).					\
bagof(Template, Generator, Bag) :-				\
	findall(Template, Generator, Bag0),			\
	Bag0 \\== [],								\
	Bag = Bag0.									\
												\
'$$_pick'(Bags, Key, Bag) :-					\
	Bags \\== [],								\
	'$$_parade'(Bags, Key1, Bag1, Bags1),		\
	'$$_decide'(Key1, Bag1, Bags1, Key, Bag).	\
												\
'$$_parade'([K-X|L1], K, [X|B], L) :- !,		\
	'$$_parade'(L1, K, B, L).					\
'$$_parade'(L, K, [], L).						\
												\
'$$_decide'(Key, Bag, [], Key, Bag) :- !.		\
'$$_decide'(Key, Bag, Bags, Key, Bag).			\
'$$_decide'(_, _, Bags, Key, Bag) :-			\
	'$$_pick'(Bags, Key, Bag).					\
												\
'$$_excess_vars0'(X^P, Y, L0, L) :- !,			\
	'$$_excess_vars0'(P, (X,Y), L0, L).			\
'$$_excess_vars0'(T, X, L0, L) :- 				\
	'$$_excess_vars'(T, X, L0, L).				\
												\
'$$_excess_vars'(T, X, L0, L) :- 				\
	var(T), !,									\
	'$$_excess_vars2'(T, X, L0, L).				\
'$$_excess_vars'(setof(X,P,S), Y, L0, L) :- !,	\
	'$$_excess_vars0'(P, (X,Y), L0, L1),		\
	'$$_excess_vars'(S, (X,Y), L1, L).			\
'$$_excess_vars'(bagof(X,P,S), Y, L0, L) :- !,	\
	'$$_excess_vars0'(P, (X,Y), L0, L1),		\
	'$$_excess_vars'(S, (X,Y), L1, L).			\
'$$_excess_vars'(T, X, L0, L) :-				\
	functor(T, _, N), 							\
	'$$_rem_excess_vars'(N, T, X, L0, L).		\
												\
'$$_rem_excess_vars'(0, _, _, L, L) :- !.		\
'$$_rem_excess_vars'(N, T, X, L0, L) :-			\
	succ(M, N),									\
	arg(N, T, T1),								\
	'$$_excess_vars'(T1, X, L0, L1),			\
	'$$_rem_excess_vars'(M, T, X, L1, L).		\
												\
'$$_excess_vars2'(T, X, L0, L) :-				\
	\\+ subterm(T, X), !,						\
	'$$_introduce'(T, L0, L).					\
'$$_excess_vars2'(T, X, L0, L0).				\
												\
'$$_introduce'(X, L, L) :-						\
	subterm(X, L), !.							\
'$$_introduce'(X, L, [X|L]).					\
												\
/* UNIT TESTS */							  ""\
												\
'$$_utests_start'  :-							\
	'$$_utests_errors' := 0.					\
												\
'$$_utests_inc'  :-								\
	'$$_utests_errors' =: E						\
	-> E1 is E + 1,	'$$_utests_errors' := E1	\
	;  true.									\
												\
'$$_utests_ok'  :-								\
	'$$_utests_errors' =: E,					\
	E == 0.										\
												\
(G <<: Guard) :- Guard -> G ; true.				\
												\
(G ::: Check) :-								\
	copy_term([G, Check], [G1, Check1]),		\
	numbervars([G1, Check1], 1, _),				\
	'$$_call_o'(G, ResG, OutG),					\
	'$$_call_o'(Check, ResC, OutC),				\
	'$$_diagnostic'(G1, Check1, ResG, ResC, OutG, OutC).	\
												\
'$$_call'(G, Res) :-							\
	catch(G, E, Z=1),							\
	( Z==1 -> Res = throw(E) ; Res = true ),	\
	!.											\
'$$_call'(_, fail).								\
												\
'$$_call_o'(G, Res, Out) :-						\
	buffer_new(B),								\
	open_buffer_stream(B, write, S),			\
	current_output(Save),						\
	set_output(S),								\
	'$$_call'(G, Res),							\
	set_output(Save),							\
	close(S),									\
	buffer_as_atom(B,Out),						\
	buffer_delete(B).							\
											  ""\
'$$_diagnostic'(G, Check, Res, Res, Out, Out) :- !.		\
'$$_diagnostic'(G, Check, ResG, ResC, OutG, OutC) :-	\
	'$$_utests_inc',							\
	format('~40w~n', '+'),						\
	format(':- ~q~n::: ~q.~n', [G, Check]),		\
	(ResG \\== ResC								\
		-> format('RESULT MISMATCH!~n'),		\
		   format('  RESULT1: ~q~n', ResG),		\
		   format('  RESULT2: ~q~n', ResC)		\
		; true									\
	),											\
	(OutG \\== OutC								\
		-> format('OUTPUT MISMATCH!~n'),		\
		   format('  OUTPUT1: ~q~n', OutG),		\
		   format('  OUTPUT2: ~q~n', OutC)		\
		; true									\
	),											\
	format('~40w~n', '+').						\
												\
ztest :- features, flags, zpl, [test_all].		\
" ;


/* The following built-in predicates are defaults that will be
   used only if no alternative boot file is provided */

static Str defaultBuiltinsStr = "					\
													\
'$cxprolog_initialise':-							\
	try((current_prolog_flag(info_messages, silent)	\
			-> true ; version)),					\
	'$env_context' := [main].						\
													\
'$cxprolog_top_level_goal' :-						\
	'$env_context' =: C,							\
	call_on_context(C,'$top_level').				\
													\
/* TOP LEVEL */									  ""\
													\
'$top_level' :-									  ""\
  P0 = '\n',										\
  ( current_prolog_flag(debug, debug)				\
			-> atom_concat(P0,'(debug) ',P1)		\
  ; current_prolog_flag(debug, trace)				\
			-> atom_concat(P0,'(trace) ',P1)		\
  ; P1=P0 ),										\
	context(C), atom_term(A, C),					\
	atom_concat(P1, A, P2),							\
	atom_concat(P2, ' ?- ', P3), 					\
	'$$_top_read'(P3, '| ', G), varnames(V),		\
	try((G==end_of_file -> writeln(user,''), halt)), \
	abolish('$$_top_call'/1),						\
	'$$_top_assert'((								\
	  '$$_top_call'(V):- G							\
	)),												\
	question('$$_top_call'(V),V).					\
													\
'$top_level2' :-									\
	repeat,											\
	catch(try('$top_level'),E,write_exception(E)),	\
	fail.											\
													\
													\
/* HANDLING CONTEXT */							  ""\
													\
push(U) :-											\
	'$$_ctx_ext'(U,true),	/* validate unit U */	\
	'$env_context' =: C,							\
	'$env_context' := [U|C],						\
	abort.											\
pop :-												\
	'$env_context' =: [_|C],						\
	'$env_context' := C,							\
	abort.											\
													\
pwd :- fs_cwd(X), writeqln(X).						\
" ;

void Boot(int argc, CharHdl argv)
{
	MemoryInit() ;
	StringInit() ;
	ScribblingInit() ;
	CmdLineInit(argc, argv) ;
	YourPrologue() ;
	FlagsInit() ;
	ExtraInit() ;
	AtomsInit() ;	/* Must be just after ExtraInit() so that ExtraAtomTag == 0 */
	NumbersInit() ;	/* Must be just after AtomsInit() so that ExtraAtomTag == 1 */
	NilInit() ;
	NullInit() ;
	LocaleInit() ;
	FilesInit() ;
	StreamsInit() ;
	ScratchInit() ;
	CheckHost() ;  /* Must be after ScratchInit() */
	InterruptOff() ;
	
	ClockInit() ;
	IndexInit() ;
	VarDictInit() ;
	FunctorsInit() ;
	InstructionsInit() ;
	MachineInit() ;
	ContextsInit() ;
	UnitsInit() ;
	CompilerInit() ;
	MesgInit() ;
	SysTraceInit() ;
/* C predicates can be installed from here onwards */

	OperatorsInit() ;
	MachineInit2() ;
	ExtraInit2() ;
	AtomsInit2() ;
	LocaleInit2() ;
	ScratchInit2() ;
	InstructionsInit2() ;
	ContextsInit2() ;
	UnitsInit2() ;
	ExceptionsInit() ;
	GCollectionInit() ;
	NumbersInit2() ;
	FlagsInit2() ;
	DebugInit() ;
	DisassemblerInit() ;
	TermsInit() ;
	ArithInit() ;
	UnifyInit() ;

	PredicatesInit() ;
	PredicatePropertyInit() ;
	ClausesInit() ;
	ThreadsInit() ;
	TermReadInit() ;
	TermWriteInit() ;

	AliasInit() ;
	IVarsInit() ;
	CharactersInit() ;
	FileSysInit() ;
	StreamsInit2() ;
	StreamPropertyInit() ;
	QueuesInit() ;
	StacksInit() ;
	DictsInit() ;;
	ArraysInit() ;
	BuffersInit() ;
	NetInit() ;
	ProcessesInit() ;
	BootInit() ;
	ConsultInit() ;
	CmdLineInit2() ;
	VersionInit() ;
	CallPrologInit() ;
	AtentionInit() ;
	InterLineInit() ;

/* Create root threads and initialize abstract machine */
	ThreadRootCreate(tTrueAtom, tTrueAtom) ;
	ZBasicLoadBuiltinsStr(coreBuiltinsStr) ;
	CompatibleIfThenUpdateFlag(compatibleIfThen_flag) ;
	ZBasicLoadBuiltinsStr(predefinedBuiltinsStr) ;

/* Require the root thread already created */
	FlagsInit3() ;
	ForeignEventInit() ;
	/*PythonInit() ;*/
	JavaInit() ;
	JavaAddOnsInit() ;
	WxWidgetsInit() ;
	WxAddOnsInit() ;
	WxWidgetsDemoInit() ;
	GuiInit() ;

/* Go! */
	MarkStaticBuiltinsAsPermanent() ;
	ZCheckHostSpeed() ;
	YourExtensions() ;
	InterruptOn() ;
	ActiveThreadReset() ;
}

void FeaturesShow()
{
	Str s ;
	ShowVersion() ;
	Write("FEATURES:\n") ;

	Write("   - OS support: %s\n", OSName()) ;

#if defined(__LP64__)
	Write("   - Memory model: 64 bits\n") ;
#else
	Write("   - Memory model: 32 bits\n") ;
#endif

#if UNDERSTAND_EXTERNAL_ENCODINGS
	Write("   - Understand external encodings: yes\n") ;
#else
	Write("   - Understand external encodings: no\n") ;
#endif

#if USE_JAVA
	Write("   - Java support enabled: yes\n") ;
#else
	Write("   - Java support enabled: no\n") ;
#endif

#if USE_WXWIDGETS
	Write("   - wxWidgets support enabled: yes\n") ;
#else
	Write("   - wxWidgets support enabled: no\n") ;
#endif

#if USE_READLINE
	Write("   - GNU Readline support: yes (%s)\n", InterLineVersion()) ;
#else
	Write("   - GNU Readline support: no\n") ;
#endif

	Write("   - Contexts semantics: %d\n", CONTEXTS) ;

	Write("   - Memory allocation policy: %s\n", MemPolicy()) ;

#if USE_THREADED_CODE
	Write("   - Threaded code: yes\n") ;
#else
	Write("   - Threaded code: no\n") ;
#endif

	Write("   - Integer size: %d bits\n", intSize) ;

	Write("   - Float size: %d bits\n", CalculateFloatSize()) ;
	Write("   - Support long long: %s\n",
			LongLongs() ? "yes" : "no") ;
	Write("   - Extended precision math functions: %s\n",
			ExtendedPrecisionMathFunctions() ? "yes" : "no") ;
	

	Write("   - Home directory: %s\n", WithHomeDirPath("", nil, nil)) ;
	Write("   - Prefix directory: %s\n", WithPrefixDirPath("", nil, nil)) ;
	Write("   - Application directory: %s\n", WithAppDirPath("", nil, nil)) ;
	Write("   - Cache directory: %s\n", WithCacheDirPath(nil, nil)) ;
	Write("   - Temporary directory: %s\n", WithTmpDirPath(nil, nil)) ;

	if( (s = OSGetEnv("PATH", false)) != nil )
		Write("   - PATH : %s\n", s) ;
#if USE_JAVA
	if( (s = JavaClasspath()) != nil )
		Write("   - CLASSPATH: %s\n", s) ;
#endif
	Write("   - PID: %d\n", OSGetPid()) ;
	Write("   - TID: %ld\n", OSGetTid()) ;
#if COMPASS != 0
	Write("   -\n") ;
	Write("   - COMPASS: %d\n", COMPASS) ;
#endif

	JumpNext() ;
}


/* CXPROLOG C'BUILTINS */

static void PEnterUserMode()
{
	if( !Booting() )
		FatalError("Cannot be activated twice") ;
	MarkStaticBuiltinsAsPermanent() ;
	UnitIsPermanent(systemUnit) = true ;
	bottomUnit = emptyUnit ;
	Booting() = false ;
	JumpNext() ;
}

static void PDefaultBoot()
{
	ZBasicLoadBuiltinsStr(defaultBuiltinsStr) ;
	JumpNext() ;
}

static void PEndOfFile()
{
	EventHalt() ;
	JumpNext() ;
}

static void PLoudEndOfFile()
{
	WriteEOF() ;
	EventHalt() ;
	JumpNext() ;
}

static void POSName()
{
	MustBe( Unify(X0, MakeAtom(OSName())) ) ;
}

static void PShow()
{
	ShowVersion() ;
	Write("Show what?\n") ;
	Write(" aliases. arrays.   atoms.       builtins.   char_reorder.\n") ;
	Write(" dicts.   features. flags.       floats.     foreigns.\n") ;
	Write(" ivars.   locale.   ops. queues. statistics. stacks.\n") ;
	Write(" streams. threads.  units.       version.\n") ;
	JumpNext() ;
}

static void PFeatures()
{
	FeaturesShow() ;
	JumpNext() ;
}

static void PLoadLib()
{
	Str libName = WithLibDirPath("lib", "pl") ;
	if( !OSExists(libName) )
		Error("Cannot access '%s'", libName) ;
	ZBasicLoadFile(libName) ;
	Info(1, "Library loaded '%s'", libName) ;
	JumpNext() ;
}

void BootInit()
{
	InstallCBuiltinPred("$$_enter_user_mode", 0, PEnterUserMode) ;
	InstallCBuiltinPred("$$_default_boot", 0, PDefaultBoot) ;
	InstallCBuiltinPred("end_of_file", 0, PEndOfFile) ;
	InstallCBuiltinPred("loud_end_of_file", 0, PLoudEndOfFile) ;
	InstallCBuiltinPred("os_name", 1, POSName) ;
	InstallCBuiltinPred("show", 0, PShow) ;
	InstallCBuiltinPred("features", 0, PFeatures) ;
	InstallCBuiltinPred("load_lib", 0, PLoadLib) ;
}
