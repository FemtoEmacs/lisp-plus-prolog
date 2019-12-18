/*
 *   This file is part of the CxProlog system

 *   Gui.c
 *   by A.Miguel Dias - 2006/09/02
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

/* Java GUI

Once a Swing component has been realized, all code that might
affect or depend on the state of that component should be executed in the
event-dispatching thread.

Realized means that the component has been painted on-screen, or that it
is ready to be painted. A Swing component that's a top-level window is
realized by having one of these methods invoked on it: setVisible(true),
show, or pack. Once a window is realized, all the components that it
contains are realized. Another way to realize a component is to add
it to a container that's already realized. You'll see examples of
realizing components later.
*/

static char *generalPredicatesStr = "			\
gui_file_chooser_simple_msg(Msg, Fich) :-		\
	@@gui_file_chooser_simple_msg(Msg, Fich).	\
gui_file_chooser_multiple_simple(List) :-		\
	@@gui_file_chooser_multiple_simple(List).	\
gui_file_chooser_multiple_msg(Msg, List) :-		\
	@@gui_file_chooser_multiple_msg(Msg, List).	\
gui_directory_chooser_msg(Msg, Dir) :-			\
	@@gui_directory_chooser_msg(Msg, Dir).		\
gui_alert(Msg) :-								\
	@@gui_alert(Msg).							\
gui_choice(Msg, Result) :-						\
	@@gui_choice(Msg, Result).					\
gui_choice_list(List, Msg, Result) :-			\
	@@gui_choice_list(List, Msg, Result).		\
gui_choice_yes_no_cancel(Msg, Result) :-		\
	@@gui_choice_yes_no_cancel(Msg, Result).	\
gui_choice_ok_cancel(Msg, Result) :-			\
	@@gui_choice_ok_cancel(Msg, Result).		\
gui_get_text(Msg, Result) :-					\
	@@gui_get_text(Msg, Result).				\
" ;

static char *graphicPredicatesStr = "										\
gui_gfx_create(Title, X0, Y0, Width, Height, Window) :-						\
	@@gui_gfx_create(Title, X0, Y0, Width, Height, Window).					\
gui_gfx_putpixel(Window, X, Y, R, G, B, Pixel) :-							\
	@@gui_gfx_putpixel(Window, X, Y, R, G, B, Pixel).						\
gui_gfx_line(Window, X0, Y0, X1, Y1, R, G, B, Line) :-						\
	@@gui_gfx_line(Window, X0, Y0, X1, Y1, R, G, B, Line).					\
gui_gfx_circle(Window, X, Y, Radius, R, G, B, Circle) :-					\
	@@gui_gfx_circle(Window, X, Y, Radius, R, G, B, Circle).				\
gui_gfx_rectangle(Window, X, Y, Width, Height, R, G, B, Rectangle) :-		\
	@@gui_gfx_rectangle(Window, X, Y, Width, Height, R, G, B, Rectangle).	\
gui_gfx_circle_filled(Window, X, Y, Radius, R, G, B, Circle) :-				\
	@@gui_gfx_circle_filled(Window, X, Y, Radius, R, G, B, Circle).			\
gui_gfx_rectangle_filled(Window, X, Y, Width, Height, R, G, B, Rectangle) :- \
	@@gui_gfx_rectangle_filled(Window, X, Y, Width, Height, R, G, B, Rectangle). \
gui_gfx_draw_text(Window, Text, X, Y, T) :-									\
	@@gui_gfx_draw_text(Window, Text, X, Y, T).								\
gui_gfx_button(Window, Text, X, Y, Button) :-								\
	@@gui_gfx_button(Window, Text, X, Y, Button).							\
gui_gfx_list(Window, Items, X, Y, List) :-									\
	@@gui_gfx_list(Window, Items, X, Y, List).								\
gui_gfx_list_getitem(List, Item) :-											\
	@@gui_gfx_list_getitem(List, Item).										\
gui_gfx_delete_object(Window, Object) :-									\
	@@gui_gfx_delete_object(Window, Object).								\
gui_gfx_clear(Window) :-													\
	@@gui_gfx_clear(Window).												\
gui_gfx_close(Window) :-													\
	@@gui_gfx_close(Window).												\
" ;

static char *textPredicatesStr = "											\
gui_text_create(Title, X0, Y0, Width, Height, ListOfMenus, Window) :-		\
	@@gui_text_create(Title, X0, Y0, Width, Height, ListOfMenus, Window).	\
gui_text_get_file_path(Window, FilePath) :-									\
	@@gui_text_get_file_path(Window, FilePath).								\
gui_text_open_file(Window) :-												\
	@@gui_text_open_file(Window).											\
gui_text_save_file(Window) :-												\
	@@gui_text_save_file(Window).											\
gui_text_save_file_as(Window) :-											\
	@@gui_text_save_file_as(Window).										\
gui_text_get_text(Window, From, To, Text) :-								\
	@@gui_text_get_text(Window, From, To, Text).							\
gui_text_append(Window, Text) :-											\
	@@gui_text_append(Window, Text).										\
gui_text_get_selected_text(Window, Text) :-									\
	@@gui_text_get_selected_text(Window, Text).								\
gui_text_replace(Window, From, To, Text) :-									\
	@@gui_text_replace(Window, From, To, Text).								\
gui_text_set_selection(Window, From, To) :-									\
	@@gui_text_set_selection(Window, From, To).								\
gui_text_close(Window) :-													\
	@@gui_text_close(Window).												\
" ;

#define maxForeignInterfaces				6

typedef struct {
	Str name ;
	Str prefix ;
} ForeignInterface ;

static ForeignInterface foreinInterfaces[maxForeignInterfaces] ;
static int nForeinInterfaces = 0, defaultForeignInterface = -1 ;

void RegisterForeignInterface(Str name, Str prefix)
{
	if( nForeinInterfaces < maxForeignInterfaces ) {
		defaultForeignInterface = nForeinInterfaces ; /* Default is most recent */
		foreinInterfaces[nForeinInterfaces].name = name ;
		foreinInterfaces[nForeinInterfaces].prefix = prefix ;
		nForeinInterfaces++ ;
	}
	else
		InternalError("Too many FOREIGN INTERFACES: increase the table capacity") ;
}

void SetDefaultForeignInterface(Str name)
{
	int i ;
	dotimes(i, nForeinInterfaces)
		if( StrEqual(foreinInterfaces[i].name, name) ) {
			defaultForeignInterface = i ;
			return ;
		}
}

static void ZForeignLoad(Str a, Str prefix)
{
	Str2K buff ;
	register CharPt z = buff ;
	while( (*z++ = *a++) )
		if( (z[-2] == '@' && z[-1] == '@') ) {
				z[-2] = prefix[0] ;
				z[-1] = prefix[1] ;
		}
	ZBasicLoadBuiltinsStr(buff) ;
}

static void ZForeignLoadDefaultForeignInterface()
{
	if( defaultForeignInterface >= 0 ) {
		Str prefix = foreinInterfaces[defaultForeignInterface].prefix ;
		ZForeignLoad(generalPredicatesStr, prefix) ;
		ZForeignLoad(graphicPredicatesStr, prefix) ;
		ZForeignLoad(textPredicatesStr, prefix) ;
	}
}


/* CXPROLOG C'BUILTINS */

static void PForeigns()
{
	int i ;
	ShowVersion() ;
	Write("FOREIGNS:\n") ;
	dotimes(i, nForeinInterfaces) {
		Write("    %s%s\n",
			foreinInterfaces[i].name,
			i == defaultForeignInterface ? " (default)" : "") ;
	}
	JumpNext() ;
}

void GuiInit()
{
#if 1
	SetDefaultForeignInterface("Java") ;
#else
	SetDefaultForeignInterface("wxWidgets") ;
#endif
	ZForeignLoadDefaultForeignInterface() ;
	InstallCBuiltinPred("foreigns", 0, PForeigns) ;
}
