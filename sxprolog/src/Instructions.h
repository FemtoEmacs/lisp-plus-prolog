/*
 *   This file is part of the CxProlog system

 *   Instructions.h
 *   by A.Miguel Dias - 2002/03/28
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Instructions_
#define _Instructions_

extern Inst
#define InstInfo(inst, args)	inst
#include "InstructionsInfo.h"
#undef InstInfo
;
extern Inst FirstInst, LastInst ;
extern InstPt FailAddr, FailMesgAddr ;

#define IsInstruction(t)			( InRange(t,FirstInst,LastInst) )

#define Discard()					CutTo(Bf(B))

void SetupFinalizer(FunV proc, VoidPt arg) ;
void CutTo(ChoicePointPt cp) ;
void CutAll(void) ;
Str GetInstInfo(Inst inst, Str *types) ;
Str GetInstNameSearch(InstPt code) ;
void InstructionsInit(void) ;
void InstructionsInit2(void) ;

#endif
