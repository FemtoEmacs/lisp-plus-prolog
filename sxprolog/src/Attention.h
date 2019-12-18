/*
 *   This file is part of the CxProlog system

 *   Attention.h
 *   by A.Miguel Dias - 2001/04/24
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Attention_
#define _Attention_

#define Attention()				(attention)
#define Interrupted()			(interrupted)
#define Booting()				(booting)

extern Bool attention, interrupted, booting ;

void RunProtected(Fun fun) ;
void InterruptOff(void) ;
void InterruptOn(void) ;
Bool InterruptHandle(void) ;
Bool AttentionHandle(PredicatePt pr) ;
void AtentionInit(void) ;

#endif
