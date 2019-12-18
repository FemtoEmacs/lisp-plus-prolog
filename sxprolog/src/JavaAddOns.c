/*
 *   This file is part of the CxProlog system

 *   JavaAddOns.c
 *   by Henrique Oliveira, A.Miguel Dias - 2006/09/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL

 *   it under the terms of the GNU General Public License as published by
 *   CxProlog is free software; you can redistribute it and/or modify
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

#if USE_JAVA

static Str addOnJavaBuiltinsStr = "										\
																			\
/* GENERAL PURPOSE PREDICATES */											\
																			\
jgui_file_chooser_simple_msg(Msg, Fich) :-									\
	java_call('prolog/JavaGeneral',												\
				'fileChooserSimpleMsg:(Ljava/lang/String;)Ljava/lang/String;', \
				[Msg], VarFich),											\
	java_convert('Ljava/lang/String;', VarFich, Fich),						\
	Fich \\= 'null'.														\
																			\
jgui_file_chooser_multiple_simple(List) :-									\
	java_call('prolog/JavaGeneral',												\
				'fileChooserMultiple:()[Ljava/lang/String;',				\
				[], VarList),												\
	java_convert('[Ljava/lang/String;', VarList, List).						\
																			\
jgui_file_chooser_multiple_msg(Msg, List) :-								\
	java_call('prolog/JavaGeneral',												\
				'fileChooserMultipleMsg:(Ljava/lang/String;)[Ljava/lang/String;', \
				[Msg], VarList),											\
	java_convert('[Ljava/lang/String;', VarList, List).						\
																			\
jgui_directory_chooser_msg(Msg, Dir) :-										\
	java_call('prolog/JavaGeneral',												\
				'fileChooserDirectoryMsg:(Ljava/lang/String;)Ljava/lang/String;', \
				[Msg], VarDir),												\
	java_convert('Ljava/lang/String;', VarDir, Dir),						\
	Dir \\= 'null'.															\
																			\
jgui_alert(Msg) :-															\
	java_call('prolog/JavaGeneral',												\
				'doAlert:(Ljava/lang/String;)V',							\
				[Msg], _).													\
																			\
getResult(0, error).														\
getResult(1, ok).															\
getResult(2, cancel).														\
getResult(3, yes).															\
getResult(4, no).															\
																			\
mymember(_, []) :- !, fail.													\
mymember(X, [X|_]) :- !.													\
mymember(X, [Y|Ys]) :- X \\= Y, mymember(X, Ys).							\
																			\
jgui_choice(Msg, Result) :-													\
	java_call('prolog/JavaGeneral',												\
				'doChoice:(Ljava/lang/String;ZZZ)I',						\
				[Msg,true,false,false], Choice),							\
	getResult(Choice, Result).												\
																		  ""\
getArgs(List, Msg, [Msg,true,false,false]) :-								\
	mymember(yesno, List),													\
	not(mymember(ok, List)),													\
	not(mymember(cancel,List)), !.											\
																			\
getArgs(List, Msg, [Msg,true,false,true]) :-								\
	mymember(yesno, List),													\
	not(mymember(ok, List)),													\
	mymember(cancel,List), !.												\
																			\
getArgs(List, Msg, [Msg,false,true,true]) :-								\
	not(mymember(yesno, List)),												\
	mymember(ok, List),														\
	mymember(cancel,List), !.												\
																			\
getArgs(_, Msg, [Msg,false,false,false]).									\
																			\
jgui_choice_list(List, Msg, Result) :-										\
	getArgs(List, Msg, Args),												\
	java_call('prolog/JavaGeneral',												\
				'doChoice:(Ljava/lang/String;ZZZ)I',						\
				Args, Choice),												\
	getResult(Choice, Result).												\
																			\
jgui_choice_yes_no_cancel(Msg, Result) :-									\
	jgui_choice_list([yesno, cancel], Msg, Result).							\
																			\
jgui_choice_ok_cancel(Msg, Result) :-										\
	jgui_choice_list([ok, cancel], Msg, Result).							\
																			\
jgui_get_text(Msg, Result) :-												\
	java_call('prolog/JavaGeneral',												\
				'getText:(Ljava/lang/String;)Ljava/lang/String;',			\
				[Msg], Text),												\
	java_convert('Ljava/lang/String;', Text, Result),						\
	Result \\= 'null'.														\
																			\
																		  ""\
/* GRAPHICAL WINDOW PREDICATES */											\
																			\
jgui_gfx_create(Title, X0, Y0, Width, Height, Window) :-					\
	integer(X0), integer(Y0), integer(Width), integer(Height),				\
	java_convert('Ljava/lang/String;', ObjTitle, Title),					\
	java_call('prolog/JavaGraphicalWindow',										\
				'<init>:(Ljava/lang/String;IIII)V',							\
				[ObjTitle, X0, Y0, Width, Height], Window).					\
																			\
jgui_gfx_clear(Window) :-													\
	java_call(Window,														\
				'gfxClear:()V',												\
				[], _).														\
																			\
check_colors(R,G,B) :-														\
	integer(R),																\
	integer(G),																\
	integer(B),																\
	0 =< R, R =< 255,														\
	0 =< G, G =< 255,														\
	0 =< B, B =< 255.														\
																			\
jgui_gfx_putpixel(Window, X, Y, R, G, B, PixelObj) :-						\
	integer(X),																\
	integer(Y),																\
	check_colors(R,G,B),													\
	java_call(Window,														\
				'gfxPutPixel:(IIIII)Lprolog/JavaGraphicalObject;',					\
				[X, Y, R, G, B], PixelObj).									\
																		  ""\
jgui_gfx_line(Window, X0, Y0, X1, Y1, R, G, B, LineObj) :-					\
	integer(X0),															\
	integer(Y0),															\
	integer(X1),															\
	integer(Y1),															\
	check_colors(R,G,B),													\
	java_call(Window,														\
				'gfxLine:(IIIIIII)Lprolog/JavaGraphicalObject;',					\
				[X0,Y0,X1,Y1,R,G,B], LineObj).								\
																			\
jgui_gfx_circle(Window, X, Y, Radius, R, G, B, CircleObj) :-				\
	integer(X),																\
	integer(Y),																\
	integer(Radius),														\
	check_colors(R,G,B),													\
	java_call(Window,														\
				'gfxCircle:(IIIIII)Lprolog/JavaGraphicalObject;',					\
				[X,Y,Radius,R,G,B], CircleObj).								\
																			\
jgui_gfx_rectangle(Window, X, Y, Width, Height, R, G, B, RectObj) :-		\
	integer(X),																\
	integer(Y),																\
	integer(Width),															\
	integer(Height),														\
	check_colors(R,G,B),													\
	java_call(Window,														\
				'gfxRectangle:(IIIIIII)Lprolog/JavaGraphicalObject;',				\
				[X,Y,Width,Height,R,G,B], RectObj).							\
																		  ""\
jgui_gfx_circle_filled(Window, X, Y, Radius, R, G, B, CircleObj) :-			\
	integer(X),																\
	integer(Y),																\
	integer(Radius),														\
	check_colors(R,G,B),													\
	java_call(Window,														\
				'gfxCircleFilled:(IIIIII)Lprolog/JavaGraphicalObject;',			\
				[X,Y,Radius,R,G,B], CircleObj).								\
																			\
jgui_gfx_rectangle_filled(Window, X, Y, Width, Height, R, G, B, RectObj) :-	\
	integer(X),																\
	integer(Y),																\
	integer(Width),															\
	integer(Height),														\
	check_colors(R,G,B),													\
	java_call(Window,														\
				'gfxRectangleFilled:(IIIIIII)Lprolog/JavaGraphicalObject;',		\
				[X,Y,Width,Height,R,G,B], RectObj).							\
																			\
jgui_gfx_draw_text(Window, Text, X, Y, TextObj) :-							\
	integer(X),																\
	integer(Y),																\
	java_call(Window,														\
				'gfxDrawText:(Ljava/lang/String;II)Lprolog/JavaGraphicalObject;',	\
				[Text,X,Y], TextObj).										\
																			\
jgui_gfx_close(Window) :-													\
	java_call(Window,														\
				'gfxClose:()V',												\
				[], _).														\
																			\
																			\
/* NEW PREDICATES */													  ""\
																			\
jgui_gfx_button(Window, Text, X, Y, ButtonObj) :-							\
	integer(X),																\
	integer(Y),																\
	java_call(Window,														\
				'gfxButton:(Ljava/lang/String;II)Lprolog/JavaGraphicalObject;',	\
				[Text,X,Y], ButtonObj).										\
																			\
jgui_gfx_list(Window, Items, X, Y, ListObj) :-								\
	integer(X),																\
	integer(Y),																\
	java_call(Window,														\
				'gfxList:([Ljava/lang/String;II)Lprolog/JavaGraphicalObject;',		\
				[Items,X,Y], ListObj).										\
																			\
jgui_gfx_list_getitem(List, Item) :-										\
	java_call(List,															\
				'getSelectedItem:()Ljava/lang/String;',						\
				[],ItemObj),												\
	java_convert('Ljava/lang/String;', ItemObj, Item).						\
																			\
jgui_gfx_delete_object(Window, Object) :-									\
	java_call(Window,														\
				'gfxDeleteObject:(Lprolog/JavaGraphicalObject;)V',					\
				[Object], _).												\
																			\
																		  ""\
/* TEXT EDITION WINDOW PREDICATES */										\
																			\
jgui_text_create(Title, X0, Y0, Width, Height, ListOfMenus, Window) :-		\
	java_convert('Ljava/lang/String;', ObjTitle, Title),					\
	java_convert('[[Ljava/lang/String;', ObjListOfMenus, ListOfMenus),		\
	java_call('prolog/JavaTextWindow',												\
				'<init>:(Ljava/lang/String;IIII[[Ljava/lang/String;)V',		\
				[ObjTitle, X0, Y0, Width, Height, ObjListOfMenus], Window).	\
																			\
jgui_text_get_file_path(Window, FilePath) :-								\
	java_call(Window,														\
				'txtGetFilePath:()Ljava/lang/String;',						\
				[], FilePath).												\
																			\
checkResult(0).																\
																			\
jgui_text_open_file(Window) :-												\
	java_call(Window,														\
				'txtOpenFile:()I',											\
				[], Result),												\
	checkResult(Result).													\
																			\
jgui_text_save_file(Window) :-												\
	java_call(Window,														\
				'txtSaveFile:()I',											\
				[], Result),												\
	checkResult(Result).													\
																			\
jgui_text_save_file_as(Window) :-											\
	java_call(Window,														\
				'txtSaveFileAs:()I',										\
				[], Result),												\
	checkResult(Result).													\
																			\
jgui_text_get_text(Window, From, To, Text) :-								\
	integer(From),															\
	integer(To),															\
	java_call(Window,														\
				'txtGetText:(JJ)Ljava/lang/String;',						\
				[From, To], ObjText),										\
	java_convert('Ljava/lang/String;',										\
				ObjText, Text).												\
																			\
jgui_text_append(Window, Text) :-											\
	java_call(Window,														\
				'txtAppend:(Ljava/lang/String;)V',							\
				[Text], _).												  ""\
																			\
jgui_text_get_selected_text(Window, Text) :-								\
	java_call(Window,														\
				'txtGetSelectedText:()Ljava/lang/String;',					\
				[], ObjText),												\
	java_convert('Ljava/lang/String;', ObjText, Text).						\
																			\
jgui_text_replace(Window, From, To, Text) :-								\
	integer(From),															\
	integer(To),															\
	java_call(Window,														\
				'txtReplace:(JJLjava/lang/String;)V',						\
				[From, To, Text], _).										\
																			\
jgui_text_set_selection(Window, From, To) :-								\
	integer(From),															\
	integer(To),															\
	java_call(Window,														\
				'txtSetSelection:(JJ)V',									\
				[From, To], _).												\
																			\
jgui_text_close(Window) :-													\
	java_call(Window,														\
				'txtClose:()V',												\
				[], _).														\
" ;


void JavaAddOnsInit()
{
	ZBasicLoadBuiltinsStr(addOnJavaBuiltinsStr) ;
}

#else

void JavaAddOnsInit()
{
	/* nothing */
}

#endif
