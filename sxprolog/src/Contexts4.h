/*
 *   This file is part of the CxProlog system

 *   Contexts4.h
 *   by A.Miguel Dias - 2007/05/25
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Contexts4_
#define _Contexts4_

#define PredIsLocal(pr) ( !PredIsVisible(pr) && !PredIsXOver(pr) )

#define ppVisible		true
#define ppHidden		true
#define ppXNew			true
#define ppXOver			true

#define CtxTopUnit(c)	( cUnitPt(XListArg(c, -1)) )
#define CtxTop(c)		( XListHead(c) )
#define CtxNext(c)		( XListTail(c) )

#define NoCurrUnit()	( (CH == tNilAtom || CtxTop(CH) == tNilAtom) && C == tNilAtom )
#define CurrUnit()		( (CH == tNilAtom || CtxTop(CH) == tNilAtom) ? (C == tNilAtom ? bottomUnit : CtxTopUnit(C)) : CtxTopUnit(CtxTop(CH)) )
#define Z(idx)			( XStructArg(CtxTop(CtxTop(CH)), idx) )

PredicatePt LookupPredicateForMetaCall(FunctorPt f) ;
Bool DebugCallOverride(PredicatePt pr) ;
void ContextsInit(void) ;
void ContextsInit2(void) ;

#endif
