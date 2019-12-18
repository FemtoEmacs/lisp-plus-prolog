/*
 *   This file is part of the CxProlog system

 *   Number.h
 *   by A.Miguel Dias - 2001/02/27
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Number_
#define _Number_


/* BOOL, CHAR, CODE & BYTE */

Pt MakeBool(Bool b) ;
Pt MakeChar(WChar c) ;
WChar XChar(Pt t) ;
Pt MakeCode(WChar c) ;
WChar XCode(Pt t) ;
Pt MakeByte(int c) ;
int XByte(Pt t) ;


/* INT */

#define maxInt				cPInt(maxWord>>4)
#define maxUInt				cPInt(maxWord>>3)
#define minInt				(-maxInt - 1)

#define IsPos(t)			( IsNat(t) && (t) != zeroIntPt )
#define IsCode(t)			( IsNat(t) && XInt(t) <= maxChar )
#define IsByte(t)			( IsNat(t) && XInt(t) <= 255 )

#define IncIntPt(t)			cPt(cWord(t) + PreEncodeInt(1))
#define DecIntPt(t)			cPt(cWord(t) - PreEncodeInt(1))

extern
Pt minusOneIntPt, zeroIntPt, oneIntPt, twoIntPt, threeIntPt,
   maxIntPt, maxUIntPt, minIntPt ;

Pt MakeInt(PInt n) ;
Pt MakeUInt(PInt n) ;
Pt MakeLLInt(LLInt n) ;
PInt XInt(Pt t) ;
PInt XUInt(Pt t) ;
PInt XAsInt(Pt t) ;
LLInt XAsLLInt(Pt t) ;
int CompareInt(PInt i1, PInt i2) ;


/* FLOAT */

extern Pt nanFloatPt, infFloatPt, eFloatPt, piFloatPt, epsilonFloatPt ;

Pt MakeFloat(PFloat r) ;
PFloat XFloat(Pt t) ;
PFloat XAsFloat(Pt t) ;
Str FloatAsStr(Char fmt, PFloat f) ;
int CompareFloat(PFloat r1, PFloat r2) ;
int CalculateFloatSize(void) ;
void FloatDisplayPrecUpdateFlag(int newValue) ;


/* NUMBER */

extern int intSize, floatSize ;

Str XNumberAsStr(Pt t) ;
Pt NumberFromStr(Str s) ;
int CompareNumber(Pt t1, Pt t2) ;
void NumbersInit(void) ;
void NumbersInit2(void) ;

#endif
