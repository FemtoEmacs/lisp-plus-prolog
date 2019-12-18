/*
 *   This file is part of the CxProlog system

 *   Predicate.h
 *   by A.Miguel Dias - 2005/08/27
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Consult_
#define _Consult_

#define ConsultGen()	( consultGen )
#define ConsultFile()	( consultFile )

#define Consulting()	( consultFile != nil )

extern AtomPt consultFile ;
extern Word16 consultGen ;

Bool PredHandleConsult(PredicatePt pr, Bool propChange) ;
void PredHandleDiscontinuous(PredicatePt pr) ;
void ZBasicLoadFile(Str fileName) ;
void ZBasicLoadStr(Str str) ;
void ZBasicLoadBuiltinsStr(Str str) ;
void ConsultRestart(void) ;
void ConsultInit(void) ;

#endif
