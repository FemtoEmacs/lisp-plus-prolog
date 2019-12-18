/*
 *   This file is part of the CxProlog system

 *   Clock.h
 *   by A.Miguel Dias - 2000/04/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Clock_
#define _Clock_

/* RANDOM */
PFloat FloatRandom(void) ;
PInt IntRandom(PInt range) ;

/* CLOCK */
double CpuTime(void) ;
long AbsoluteTime(void) ;
void ClockInit(void) ;

#endif
