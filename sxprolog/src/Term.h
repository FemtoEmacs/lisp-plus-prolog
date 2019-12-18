/*
 *   This file is part of the CxProlog system

 *   Term.h
 *   by A.Miguel Dias - 1989/11/14
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */
#ifndef _Term_
#define _Term_

/***** TAGS ******************************************
 *				var ..............................00 *
 *			 struct ..............................01 *
 *			   list ..............................11 *
 * extra|atom|float .............................010 *
 *				int .............................110 *
 *****************************************************/

#if defined(__LP64__)
/* 64-bit machine */
  #define addr_bits			64

  #define tagVar			cWord(0x0000000000000000)
  #define tagStruct			cWord(0x0000000000000001)
  #define tagList			cWord(0x0000000000000003)
  #define tagExtra			cWord(0x0000000000000002)
  #define tagInt			cWord(0x0000000000000006)
  #define tagIntNeg			cWord(0x8000000000000006)

  #define tag1				cWord(0x0000000000000001)
  #define tag2				cWord(0x0000000000000003)
  #define tag3				cWord(0x0000000000000007)
  #define tag4				cWord(0x8000000000000007)
#else
/* 32-bit machine */
  #define addr_bits			32

  #define tagVar			cWord(0x00000000)
  #define tagStruct			cWord(0x00000001)
  #define tagList			cWord(0x00000003)
  #define tagExtra			cWord(0x00000002)
  #define tagInt			cWord(0x00000006)
  #define tagIntNeg			cWord(0x80000006)

  #define tag1				cWord(0x00000001)
  #define tag2				cWord(0x00000003)
  #define tag3				cWord(0x00000007)
  #define tag4				cWord(0x80000007)
#endif

/* Terms with tag tagExtra must be memory aligned (see "Memory.c") */
#define memAlignMask		tag3

#define GetTag1(t)			( cWord(t) & tag1 )
#define GetTag2(t)			( cWord(t) & tag2 )
#define GetTag3(t)			( cWord(t) & tag3 )
#define GetTag4(t)			( cWord(t) & tag4 )

#define XPt(t)				cPt( cWord(t) & ~tag2 )
#define	XHdl(t)				cHdl( cWord(t) & ~tag2 )

#define Tag(pt,tag)			cPt( cWord(pt) | tag )


/* GENERAL */

#define IsNumber(t)			( IsInt(t) || IsFloat(t) )
#define IsAtomic(t)			( GetTag2(t) == tagExtra )
#define IsCompound(t)		( GetTag1(t) == tagStruct )
#define IsAtomicOrCompound(t) ( GetTag2(t) != tagVar )


/* EMPTY */

#define EmptyCell				( cPt(0) )
#define IsEmpty(t)				( cPt(t) == EmptyCell )

void EmptyRange(Hdl a, Hdl z) ;
void EmptyRangeN(Hdl a, Size n) ;
void CopyUntilEmpty(Hdl z, Hdl a) ;
Hdl FindEmpty(Hdl a) ;
VoidPt AllocateSegmentEmpty(Size nWords, VoidPt end) ;


/* ATOM */

#define ExtraAtomTag		0
#define IsAtom(t)			( GetTag3(t) == tagExtra && XExtraTag(t) == ExtraAtomTag )
#define TagAtom(a)			Tag(a, tagExtra)

#define MakeAtom(name)		TagAtom(LookupAtom(name))
#define MakeTempAtom(name)	TagAtom(LookupTempAtom(name))
#define	XAtom(t)			cAtomPt( XPt(t) )
#define XAtomName(t)		AtomName(XAtom(t))


/* VAR */

#define IsVar(t)			( GetTag2(t) == tagVar )

#define IsLink(v)			( *cHdl(v) != cPt(v) )
#define SetVar(v,t)			( *cHdl(v) = cPt(t) )

#define DrfVar(v)			( (v) = *cHdl(v) )
#define VarValue(t)			while( IsVar(t) && IsLink(t) ) DrfVar(t)
#define VarValue2(t,i)		{ t = i ; VarValue(t) ; }

#define Lt(v1,v2)			( cPt(v1) <  cPt(v2) )
#define Le(v1,v2)			( cPt(v1) <= cPt(v2) )
#define Gt(v1,v2)			( cPt(v1) >  cPt(v2) )
#define Ge(v1,v2)			( cPt(v1) >= cPt(v2) )
#define Eq(v1,v2)			( cPt(v1) == cPt(v2) )
#define Ne(v1,v2)			( cPt(v1) != cPt(v2) )
#define Df(v1,v2)			( cPt(v1) - cPt(v2) )
#define Dfh(v1,v2)			( cHdl(v1) - cHdl(v2) )


Pt Drf(Pt t) ;
Pt GetCodeFreeVar(void) ;
void PrepareDrfChecked(Pt term) ;
Pt DrfChecked(Pt t) ;
Pt DrfChecked2(register Pt t, Hdl limA, Hdl limB) ;
Pt MakeVar(void) ;
Str VarName(Pt t) ;
Bool IsVarName(Str s) ;


/* INT */

#define EncodeBits(t)		( cWord(t) << 3 )


#define IsInt(t)			( GetTag3(t) == tagInt )
#define TagInt(i)			Tag(i, tagInt)
#define PreEncodeInt(i)		( cWord(i) << 3 )
#define EncodeInt(i)		TagInt( PreEncodeInt(i) )
#define DecodeInt(t)		( cPInt(t) >> 3 )
#define DecodeUInt(t)		( cWord(t) >> 3 )

#define IsNat(t)			( GetTag4(t) == tagInt )
#define IsNeg(t)			( GetTag4(t) == tagIntNeg )


/* FLOAT */

#define ExtraFloatTag		1
#define IsFloat(t)			( GetTag3(t) == tagExtra && XExtraTag(t) == ExtraFloatTag )
#define TagFloat(f)			Tag(f, tagExtra)


/* STRUCTS */

#define IsStruct(t)			( GetTag2(t) == tagStruct )
#define IsStruct2(t,f,a)	( IsStruct(t) && ((a = XHdl(t) + 1), (f = cFunctorPt(a[-1])), true) )
#define IsStruct3(t,f,a,n)	( IsStruct(t) && ((a = XHdl(t) + 1), (f = cFunctorPt(a[-1])), (n = FunctorArity(f)), true) )
#define TagStruct(st)		Tag(st, tagStruct)

#define XStructFunctor(t)	(*cFunctorHdl(XPt(t)))
#define XStructAtom(t)		FunctorAtom(XStructFunctor(t))
#define XStructName(t)		FunctorName(XStructFunctor(t))
#define XStructArity(t)		FunctorArity(XStructFunctor(t))
#define XStructArgs(t)		(XHdl(t) + 1)
#define XStructArg(t,i)		(XStructArgs(t)[i])

#define IsThisStruct(t,f)	( IsStruct(t) && XStructFunctor(t) == f )
#define IsSameStruct(t1,t2)	( IsStruct(t1) && i(t2) && \
								XStructFunctor(t1) == XStructFunctor(t2) )
#define IsUnitParam(t)		( IsThisStruct(t, unitParamFunctor) )

Pt MakeStruct(FunctorPt functor, Hdl args) ;
Pt MakeCleanStruct(FunctorPt functor) ;
Pt MakeUnStruct(FunctorPt functor, Pt arg) ;
Pt MakeBinStruct(FunctorPt functor, Pt arg0, Pt arg1) ;
Pt MakeSlashTerm(FunctorPt f) ;
Pt MakeTriStruct(FunctorPt functor, Pt arg0, Pt arg1, Pt arg2) ;
Str XStructNameArity(Pt t) ;
void SplitNeckTerm(Pt c, Hdl parts) ;
PInt XUnitParam(Pt t) ;


/* LISTS */

#define IsList(t)			( GetTag2(t) == tagList )
#define IsList2(t,a)		( GetTag2(t) == tagList && ((a = XHdl(t)), true) )
#define TagList(l)			Tag(l, tagList)

#define XListArgs(t)		XHdl(t)
#define XListArg(t,i)		(XListArgs(t)[i])
#define XListHead(t)		XListArg(t,0)
#define XListTail(t)		XListArg(t,1)

Pt MakeList(Pt head, Pt tail) ;
Pt ArrayToOpenList(Hdl array, Size n) ;
Pt ArrayToList(Hdl array, Size n) ;
Pt ArrayToListRev(Hdl array, Size n) ;
Hdl ListToArray(Pt list, Size *len) ;
Size ListLength(Pt list) ;
Bool ListCheck(Pt list) ;


/* EXTRA : (extra types) */

#define IsExtra(t)			( GetTag3(t) == tagExtra )
/* As now e is ignored, but the representation of the extras may change... */
#define TagExtra(e, x)		Tag(x, tagExtra)
#define XExtra(t)			cExtraPt(XPt(t))

#define IsAtomicStrict(t)	( IsExtra(t) ? XExtraTag(t) <= ExtraFloatTag : IsInt(t) )
#define IsExtraStrict(t)	( IsExtra(t) && XExtraTag(t) > ExtraFloatTag )


/* TERMS */

#define termSegmSize 10
extern Hdl termSegm ;

Pt StringToPString(Str s) ;
Str PStringToString(Pt l) ;
Pt StringToAString(Str s) ;
Str AStringToString(Pt l) ;
Str TermTypeStr(Pt t) ;

void TermBasicGCMark(Pt t) ;
Bool TermIsCyclic(Pt t) ;
Bool OccursCheck(Pt term) ;
Bool TermIsGround(Pt t) ;
Pt TermVars(Pt t, Size *nVars) ;
Bool TermContainsThisVar(Pt t, Pt var) ;
Bool TermSubTerm(Pt s, Pt t) ;
Hdl TermSerialize(Pt term, Size *len) ;
Pt TermUnserialize(Hdl array, Size len) ;

Pt AllocateTermForTopLevelAssert(Pt t) ;
Pt AllocateTermForAssert(Pt t) ;
Pt AllocateTermForAssign(Pt t) ;
void ReleaseTerm(Pt t) ;
Pt ZPushTerm(Pt t) ;
Pt ZPushTerm_ConvUnitParams(Pt t) ;

Pt TestAtomic(Pt t) ;
Pt TestAtom(Pt t) ;
Pt TestAtomOrVar(Pt t) ;
Pt TestList(Pt t) ;
Pt TestListOrVar(Pt t) ;
Pt TestInt(Pt t) ;
Pt TestIntOrVar(Pt t) ;
Pt TestPosInt(Pt t) ;
Pt TestPosIntOrVar(Pt t) ;
Pt TestNat(Pt t) ;
Pt TestNatOrVar(Pt t) ;
Pt TestIntRange(Pt t, int a, int z) ;
Pt TestCode(Pt t) ;
Pt TestByte(Pt t) ;
Pt TestChar(Pt t) ;
Pt TestCharOrCode(Pt t) ;
Pt TestVar(Pt t) ;
Pt TestNonVar(Pt t) ;

AtomPt XTestAtom(Pt t) ;
Str XTestAtomName(Pt t) ;
Str XTestAtomNameOrPString(Pt t) ;
Str XTestFileName(Pt t) ;
PInt XTestInt(Pt t) ;
LLInt XTestLLInt(Pt t) ;
PInt XTestPosInt(Pt t) ;
PInt XTestNat(Pt t) ;
WChar XTestCode(Pt t) ;
PInt XTestByte(Pt t) ;
WChar XTestChar(Pt t) ;
WChar XTestCharOrCode(Pt t) ;
PInt XTestIntRange(Pt t, int a, int z) ;
PFloat XTestFloat(Pt t) ;
Bool XTestBool(Pt t) ;
Bool3 XTestBool3(Pt t) ;
Bool XTestOnOff(Pt t) ;
Str XTestTermName(Pt t) ;
FunctorPt XTestFunctor(Pt t) ;
int XTestArity(Pt t) ;
FunctorPt XTestFunctor2(Pt t1, Pt t2) ;
FunctorPt XTestSlash(Pt t) ;
void XTestSlashArgs(Pt t, Hdl a0, Hdl a1) ;
FunctorPt XTestStruct(Pt t, Hdl *args) ;
AtomPt XTermAtom(Pt t) ;
FunctorPt XTermFunctor(Pt t) ;
Hdl XTermArgs(Pt t, int *arity) ;
Bool XAtomBool(int* out, Pt t) ;
Bool XAtomAlt(int* out, Pt t, ...) ;

void TermsInit(void) ;

/* SAVE
#define IsFunctor(f)		( IsVar(f) && *cPt(f) == 0 )
#define IsFunctor(f)		( cWord(f) >> 16 == 0 ) ???

#define IsStruct(t)			( (GetTag(t) == tagStruct) && IsFunctor(XHdl(t)[0]) )
#define IsStruct3(t,f,a,n)	( (GetTag(t) == tagStruct) && ((a = XHdl(t)), (f = cFunctorPt(a[0])), (IsFunctor(f) ? a++ : f = listFunctor), (n = FunctorArity(f)), true) )


#define IsList(t)			( (GetTag(t) == tagStruct) && !IsFunctor(XHdl(t)[0]) )
#define IsList2(t,a)		( (GetTag(t) == tagStruct) && ((a = XHdl(t)), (!IsFunctor(a[0]))) )
*/

#endif
