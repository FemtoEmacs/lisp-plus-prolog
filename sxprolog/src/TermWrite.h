/*
 *   This file is part of the CxProlog system

 *   TermWrite.c
 *   by A.Miguel Dias - 1992/02/23
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _TermWrite_
#define _TermWrite_

void TermWriteN(StreamPt srm, Pt term) ;
void TermWriteQ(StreamPt srm, Pt term) ;
void TermWriteP(StreamPt srm, Pt term) ;
void TermWriteD(StreamPt srm, Pt term) ;
void TermWriteC(StreamPt srm, Pt term) ;

Str TermAsStrN(Pt term) ;
Str TermAsStrQ(Pt term) ;
Str SubtermAsStrN(Pt subterm, Pt term) ;
Str SubtermAsStrQ(Pt subterm, Pt term) ;
Str TermsAsStrN(Pt list) ;
Str TermsAsStrQ(Pt list) ;
Str TermAsStr(Pt term) ;

void SetWriteDepth(Size termDepth, Size listLength) ;
void TermWriteInit(void) ;

#endif
