/*
 *   This file is part of the CxProlog system

 *   Index.h
 *   by A.Miguel Dias - 1990/01/26
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Index_
#define _Index_

#define maxIndexable		3

void IndexInit(void) ;
void NIndexableUpdateFlag(int newValue) ;
void DoIndex(PredicatePt pr) ;

#endif
