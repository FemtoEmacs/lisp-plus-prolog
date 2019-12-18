/*
 *   This file is part of the CxProlog system

 *   VarDictionary.h
 *   by A.Miguel Dias - 2000/04/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _VarDictionary_
#define _VarDictionary_

/* TEMPORARY VARIABLES */

#define AllocR(i)		registerInUse[i] = true
#define FreeR(i)		registerInUse[i] = false
#define InUseR(i)		registerInUse[i]

extern Bool registerInUse[] ;

void TempVarAllocInit(void) ;
void ReserveFirstNRegs(int n) ;
int FindR(void) ;


/* VAR DICTIONARY */

typedef struct {
	Pt var ;
	short nOccurrences ;
	short firstGoal, lastGoal, lastGoalArg ;
	Bool isTemp, isUnsafe, hasGlobalValue ;
	Bool wasReferenced ;
	short permIndex, tempIndex ;
} VarDescriptor, *VarDescriptorPt ;

#define VarDictSize()		(varDictSize)
#define doVarDict(vd)		dotable(vd, varDict, varDictSize)

extern VarDescriptorPt varDict ;
extern int varDictSize ;

void VarDictInit(void) ;
void VarDictReset(void) ;
VarDescriptorPt VarDictFind(Pt var) ;
VarDescriptorPt VarDictFindVarUsingTemp(int i) ;
void VarDictEnterVar(Pt var, int goalN, int argN) ;
Bool VarDictWasReferenced(VarDescriptorPt vd) ;
void VarDictList(void) ;

#endif
