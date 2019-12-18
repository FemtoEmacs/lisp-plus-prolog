/*
 *   This file is part of the CxProlog system

 *   Array.h
 *   by A.Miguel Dias - 2002/12/30
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Array_
#define _Array_

typedef struct Array *ArrayPt ;

/* MAIN OPERATIONS */
Size ArraySize(ArrayPt a) ;
ArrayPt ArrayNew(void) ;
void ArrayClear(ArrayPt a) ;
void ArrayFilter(ArrayPt a, BFunVV filter, VoidPt x) ;
Bool ArrayGet(ArrayPt a, PInt idx, Pt *t) ;
void ArraySet(ArrayPt a, PInt idx, Pt t) ;
Bool ArrayDeleteItem(ArrayPt a, PInt idx) ;

/* TEST, EXTRACT & INIT */
Bool IsArray(Pt t) ;
ArrayPt XTestArray(Pt t) ;
void ArraysInit(void) ;

#endif
