/*
 *   This file is part of the CxProlog system

 *   Arith.h
 *   by A.Miguel Dias - 1989/12/05
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Arith_
#define _Arith_

/* MAIN OPERATIONS */
Pt Evaluate(Pt t) ;

/* INIT */
Bool ExtendedPrecisionMathFunctions(void) ;
Bool IsIntFloat(PFloat f) ;
Pt Evaluate(Pt t) ;
void ArithInit(void) ;

#endif
