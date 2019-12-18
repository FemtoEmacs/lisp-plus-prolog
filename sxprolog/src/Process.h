/*
 *   This file is part of the CxProlog system

 *   Process.h
 *   by A.Miguel Dias - 2002/01/12
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Process_
#define _Process_

Bool ProcessIsChild(void) ;
void ProcessMesgToFather(int info) ;
void ProcessesInit(void) ;
void ProcessesFinish(void) ;

#endif
