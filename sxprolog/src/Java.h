/*
 *   This file is part of the CxProlog system

 *   Java.h
 *   by A.Miguel Dias - 2004/07/25
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Java_
#define _Java_

#if USE_JAVA

Str JavaClasspath(void) ;
Pt JavaGetEvent(void) ;
int JavaHowManyEvents(void) ;
void JavaDiscardAllEvents(void) ;
void JavaSetEventNotifier(Fun f) ;

#endif

void JavaInit(void) ;

#endif
