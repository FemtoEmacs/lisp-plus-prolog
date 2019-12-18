/*
 *   This file is part of the CxProlog system

 *   Gui.h
 *   by A.Miguel Dias - 2006/09/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Gui_
#define _Gui_

void RegisterForeignInterface(Str name, Str prefix) ;
void SetDefaultForeignInterface(Str name) ;

/* INIT */
void GuiInit(void) ;

#endif
