/*
 *   This file is part of the CxProlog system

 *   Extra.h
 *   by A.Miguel Dias - 2002/01/01
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Extra_
#define _Extra_

typedef Size (*ExtraFun)(CVoidPt) ;
typedef Bool (*ExtraCond)(CVoidPt, CVoidPt) ;

typedef struct ExtraType {
	Word tag ;
	Str name ;
	ExtraFun sizeFun ;
	FunV gcMarkFun ;
	BFunV gcDeleteFun ;
	VoidPt *hashTable ;
	Size hashTableSize ;
	Bool supportAliasing ;
} ExtraType, *ExtraTypePt ;

#define ExtraTypeTag(e)				((e)->tag)
#define ExtraTypeName(e)			((e)->name)
#define ExtraTypeSizeFun(e)			((e)->sizeFun)
#define ExtraTypeGCMarkFun(e)		((e)->gcMarkFun)
#define ExtraTypeGCDeleteFun(e)		((e)->gcDeleteFun)
#define ExtraTypeHashTable(e)		((e)->hashTable)
#define ExtraTypeHashTableSize(e)	((e)->hashTableSize)
#define ExtraTypeHashTableSlot(e,s)	(*(ExtraPt *)(&ExtraTypeHashTable(e)[s]))
#define ExtraTypeSupportAliasing(e)	((e)->supportAliasing)
#define ExtraTypeOverrideTag(e)		((e)->overrideTag)

#define ExtraDef(Type)				\
	unsigned int tag : 8 ;			\
	Bool disabled : 1 ;				\
	Bool hidden : 1 ;				\
	Bool permanent : 1 ;			\
	Bool gcMarked : 1 ;				\
	Bool special : 1 ;				\
	struct Type *next

typedef struct Extra {
	ExtraDef(Extra) ;
} Extra, *ExtraPt, **ExtraHdl ;

struct Atom ;

#define cExtraPt(x)					((ExtraPt)(x))
#define cExtraHdl(x)				((ExtraHdl)(x))

#define ExtraTag(x)					(cExtraPt(x)->tag)
#define ExtraIsDisabled(x)			(cExtraPt(x)->disabled)
#define ExtraIsHidden(x)			(cExtraPt(x)->hidden)
#define ExtraIsPermanent(x)			(cExtraPt(x)->permanent)
#define ExtraIsGCMarked(x)			(cExtraPt(x)->gcMarked)
#define ExtraIsSpecial(x)			(cExtraPt(x)->special)
#define ExtraNext(x)				(cExtraPt(x)->next)

#define XExtraTag(t)				ExtraTag(XExtra(t))

ExtraTypePt ExtraTypeNew(Str name, ExtraFun sizeFun,
							FunV gcMarkFun, BFunV gcDeleteFun, Size htSize) ;
void ExtraTypeDoesNotSupportAliasing(ExtraTypePt e) ;
Bool IsThisExtra(ExtraTypePt e, Pt t) ;
VoidPt ExtraNewWithSize(ExtraTypePt e, Size size, int slot) ;
VoidPt ExtraNew(ExtraTypePt e, int slot) ;
Pt TagExtraAuto(VoidPt x) ;
Bool ExtraDisable(VoidPt x) ;
void ExtraHide(VoidPt x) ;
void ExtraPermanent(VoidPt x) ;
void ExtraNotPermanent(VoidPt x) ;
void ExtraSpecial(VoidPt x) ;
Str ExtraAsStr(VoidPt x) ;
VoidPt ExtraGetFirst(ExtraTypePt e) ;	/* pre: only one slot */
VoidPt ExtraGetNext(VoidPt e) ;
Size ExtraForEach(ExtraTypePt e, ExtraFun proc) ;
VoidPt ExtraFindFirst(ExtraTypePt e, int slot, ExtraCond cond, CVoidPt arg) ;
void ExtraPNDCurrent(ExtraTypePt e, BFunV bfun, int arity, int resPos) ;
void ExtraShow(ExtraTypePt e, ExtraFun fun) ;

/* EXTRAS GARBAGE COLLECTION */
void ExtraGCHandlerInstall(Str name, Fun p) ;
void ExtraGCAddDelta(Size size) ;
void ExtraGCMark(VoidPt x) ;
void ExtraGCMarkRange(Hdl a, Hdl z) ;
int ExtraGCClearNotMarked(ExtraTypePt e) ;
void ExtraGC(void) ;
void GCollectionUpdateFlag(int newValue) ;

/* PT OPERATIONS */
VoidPt XTestExtraGen(ExtraTypePt e, register Pt t, Bool useAlias, Bool err) ;
Bool XExtraCheck(ExtraTypePt e, Pt t) ;
VoidPt XTestExtra(ExtraTypePt e, Pt t) ;
VoidPt XTestExtraNoAlias(ExtraTypePt e, Pt t) ;
VoidPt XTestAnyExtra(Pt t) ;
VoidPt XTestAnyStrictExtra(Pt t) ;
Str XExtraTypeName(Pt t) ;
Str XExtraAsStr(Pt t) ;
Pt MakeExtraFromStr(Str s) ;
void BindVarWithExtra(Pt t, VoidPt x) ;
VoidPt ExtraTypeError(ExtraTypePt e, Str alt, Pt found) ;
void ExtraInit(void) ;
void ExtraInit2(void) ;

#endif
