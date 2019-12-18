/*
 *   This file is part of the CxProlog system

 *   Stack.h
 *   by A.Miguel Dias - 2000/11/30
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Stack_
#define _Stack_

typedef struct Stack *StackPt ;

/* MAIN OPERATIONS */
Size StackSize(StackPt s) ;
StackPt StackNew(void) ;
void StackClear(StackPt s) ;
void StackFilter(StackPt s, BFunVV filter, VoidPt x) ;
Bool StackTop(StackPt s, Hdl t) ;
Bool StackFromTop(StackPt s, int idx, Hdl t) ;
void StackPush(StackPt s, Pt t) ;
Bool StackPop(StackPt s) ;

/* TEST, EXTRACT & INIT */
Bool IsStack(Pt t) ;
StackPt XTestStack(Pt t) ;
void StacksInit(void) ;

#endif
