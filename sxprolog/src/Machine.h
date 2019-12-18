/*
 *   This file is part of the CxProlog system

 *   Machine.h
 *   by A.Miguel Dias - 1989/11/25
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Machine_
#define _Machine_


/* CHOICEPOINTS */

typedef struct ChoicePoint { /* LIVES ON LOCAL STACK */
	struct Environment *E ; /* must be first */
	Hdl CP ;
	struct ChoicePoint *B, *B0, *R ;
	Pt C, CH ;
	Hdl P, TR, H ;
/*	Pt A[] ;	*/
} ChoicePoint, *ChoicePointPt ;

#define cChoicePointPt(x)	((ChoicePointPt)(x))

#define ChoicePointField(b, field)	( (b)->field )
#define ChoicePointArg(b, idx)		( cHdl((b) + 1)[idx] )

#define Bf(field)			ChoicePointField(B, field)
#define A(idx)				ChoicePointArg(B, idx)

#define IsAChoicePointHAddr(h)		IsTrailRef(h[-1])

#define SaveState(old, next, n)	{			\
				Bf(E) = E ;					\
				Bf(CP) = CP ;				\
				Bf(B) = (old) ;				\
				Bf(B0) = B0 ;				\
				Bf(R) = R ;					\
				Bf(C) = C ;					\
				Bf(CH) = CH ;				\
				Bf(P) = (next) ;			\
				Bf(TR) = TR ;				\
				HB = Bf(H) = H ;			\
				N = (n) ;					\
				while( N-- ) A(N) = Xc(N) ; }

#define RestoreState(n)	{					\
				E = Bf(E) ;					\
				CP = Bf(CP) ;				\
				B0 = Bf(B0) ;				\
				R = Bf(R) ;					\
				C = Bf(C) ;					\
				CH = Bf(CH) ;				\
				H = Bf(H) ;					\
				TrailRestore(Bf(TR)) ;		\
				N = (n) ;					\
				while( N-- ) Xc(N) = A(N) ; }

#define SetChoicePoint(b) {					\
				B = (b) ;					\
				HB = Bf(H) ; }

#define CreateChoicePoint(next, n) {		\
				D = cPt(B) ;	/* save current choice point */		\
				B = cChoicePointPt(TopOfLocalStack() - (n)) - 1 ;	\
				SaveState(cChoicePointPt(D), next, n) ; }


/* ENVIRONMENTS */

typedef struct Environment { /* LIVES ON LOCAL STACK */
/*	Pt Y[] ; */
	struct Environment *E ; /* must be first */
	Hdl CP ;
	struct ChoicePoint* B0 ;
} Environment, *EnvironmentPt ;

#define cEnvironmentPt(x)	((EnvironmentPt)(x))

#define Ef(field)			( E[-1].field )
#define Y(idx)				( cHdl(E)[-(int)(idx)] )
#define OutPerm(permIndex)	( WordsOf(Environment) + permIndex + 1 )

/* This macro creates an open ended predicate environment which will
   progressivelly shrink along the predicate code. The current size
   of this environment is determined by the second argument to each
   Call instruction in the predicate body code. */

#define AllocEnv() Do(						\
	Q.e = E ;	/* save curr env */			\
	E = cEnvironmentPt(TopOfLocalStack()) ;	\
	Ef(E) = Q.e ;							\
	Ef(CP) = CP ;							\
	Ef(B0) = B0 ; )

#define DeallocEnv() Do(					\
	CP = Ef(CP) ;							\
	E = Ef(E) ; )


/* CUT FINALIZERS */

typedef struct Finalizer {
	ChoicePointPt cp ;
	FunV proc ;
	VoidPt arg ;
} Finalizer, *FinalizerPt ;

#define cFinalizerPt(x)		((FinalizerPt)(x))


/* REGISTERS */

typedef union MixReg {
	Pt t ;
	Hdl h ;
	EnvironmentPt e ;
	FunctorPt f ;
	PredicatePt p ;
	ClausePt c ;
	UnitPt u ;
	Word w ;
} MixReg ;

#define maxX				256

#define X(pt)				( *(pt) )
#define Xc(c)				( X[c] )
#define X0					Xc(0)
#define X1					Xc(1)
#define X2					Xc(2)
#define X3					Xc(3)
#define X4					Xc(4)
#define X5					Xc(5)
#define X6					Xc(6)
#define X7					Xc(7)

#define OutTemp(tempIndex)	( X + tempIndex )

extern Hdl P, CP, H, HB, TR, S ;
extern EnvironmentPt E ;
extern ChoicePointPt B, B0, R, L ;
extern MixReg Q, Z ;
extern Pt C, CH, D, X[], saveC ;
extern Pt GlobalClock ;
extern FinalizerPt F ;
extern Hdl stacksBegin0, stacksEnd0 ;
extern Hdl stacksBegin, stacksEnd ;
extern Hdl trailBegin, trailEnd ;
extern int N ;
extern Bool running ;


/* VARIABLES */

/* Parameter of ResetVar is not a Pt, is a proper Var */
#define ResetVar(v)			( *cHdl(v) = cPt(v) )

#define IsToTrailVar(v)		( Lt(v,HB) || Lt(B,v) )
#define IsLocalVar(v)		( Lt(H,v) )
#define IsCurrEnvVar(v)		( Lt(v,E) && IsLocalVar(v) )
#define PushTrail(v)		{ if( Eq(TR,F) ) TrailExpand() ;			\
							  Push(TR,v) ; }

/* Boundary of global stack must be included because of relocation */
#define IsLocalRef(v)		( Lt(H,v) && Lt(v,stacksEnd) )
#define IsGlobalRef(v)		( Le(stacksBegin,v) && Le(v,H) )
#define IsStacksRef(v)		( Le(stacksBegin,v) && Lt(v,stacksEnd) )
#define IsTrailRef(v)		( Le(trailBegin,v) && Le(v,F) )

#define IsGlobalCompound(t)	( IsCompound(t) && IsGlobalRef(XPt(t)) )
#define IsAllocCompound(t)	( IsCompound(t) && !IsStacksRef(XPt(t)) )

#define Assign(v,term)		{ if( IsToTrailVar(v) ) PushTrail(v)		\
							  SetVar(v,term) ; }


/* STACKS */

/* Invariant: CP[-1] must represent the size of the curr env whenever E<=B */
#define TopOfLocalStack()	( Lt(B,E) ? cHdl(B) : (cHdl(E)-cWord(CP[-1])) )

/* Each c-built-in predicate can count on a 512-word reserve in the stacks.
   All the other predicates can also count on a 512-word reserve to build terms,
   one choice point and one environment. If 512 words are not enough,
   the compiler inserts a EnsureFreeSpace instruction that checks if a
   stack expansion is necessary. */
#define memReserve		512
#define safeReserve		512

#define FreeHSpace()		( Df(TopOfLocalStack(),H) - safeReserve )
#define FreeHSpace2()		( FreeHSpace() - memReserve )

#define CheckFreeSpaceOnStacks(n)										\
				Ignore( FreeHSpace() < (n) && FatalError("Global stack overflow") )

/* ZEnsureFreeSpaceOnStacks can only be used inside Pfunctions and Zfunctions */
#if 0
#define ZEnsureFreeSpaceOnStacks(n, arity)								\
				( FreeHSpace2() < cSize(n)								\
				  ? ZStacksExpansion(n - FreeHSpace2())					\
				  : false )	
#else
#define ZEnsureFreeSpaceOnStacks(n, arity, precision)					\
				( FreeHSpace2() < cSize(n)								\
				  ? ZGCollection(n - FreeHSpace2(), arity, precision)	\
				  : (testRelocation_flag								\
				  		&& ZTestGCollection(n - FreeHSpace2(), arity, precision)) )
#endif

/* INTERPRETER */

/* experimental */
#define USE_THREADED_CODE	0

#if USE_THREADED_CODE && __i386__
	#define JumpNext()		Do( asm("movl P, %eax") ; asm("addl $4, P") ;	\
								asm("leave") ;								\
								asm("jmp *(%eax)") ; return ; )
	#define Jump(v)			Do( asm("leave") ;								\
								asm("jmp *(%0)" : : "g" (v)) ; return ; )
	#define InstRun()		JumpNext()
#else
	#undef USE_THREADED_CODE
	#define	JumpNext()		return
	#define Jump(v)			Do( (*cFun(v))() ; return ; )
	#define InstRun()		(*cFun(*P++))()
#endif

#define Running()		running
#define DoFail()		Do( P = Bf(P) ; JumpNext() ; )
#define Ensure(c)		Do( if( !(c) ) DoFail() ; )
#define MustBe(c)		Do( if( c ) JumpNext() ; else DoFail() ; )

#define InstEncode(p)	cPt(p)
#define InstDecode(p)	cFun(p)

typedef struct PrologHashElem {
	Pt value ;
	Hdl address ;
	struct PrologHashElem* next ;
} PrologHashElem, *PrologHashElemPt, *PrologHashTable ;

#define PrologHash(t, size)		( (cWord(t) >> 4) & ((size) - 1) )

void MachineRun(void) ;
void CheckHost(void) ;
void DumpRegisters(void) ;
void ZCheckHostSpeed(void) ;
void ContextSetEmpty(void) ;
void ContextRestore(void) ;
void ShowWord(Hdl h, Bool terms) ;
void ShowMachineRange(Str n, Hdl a, Hdl z) ;
Bool MachineShow(Str s) ;
void MachineShowReg(void) ;

typedef void (*RelocateStacksProc)(Size, Size) ;
void InstallRelocateStacksHandler(Str name, RelocateStacksProc p) ;
void RelocateStacks(Size globalOffset, Size localOffset) ;

Size LvrPush(VoidPt addr) ;
Size LvrPushN(VoidPt first, ...) ;
void LvrRestore(Size n) ;
void LvrRestart(void) ;

void TrailIVar(AtomPt atom, Pt value) ;
void TrailRestore(Hdl until) ;
void TrailAllVarsStart(void) ;
void TrailAllVarsRestore(void) ;
void TrailExpand(void) ;

void HSave(void) ;
void HRestore(void) ;
Size HGrown(void) ;
void SafeAssignToGlobal(Hdl h, Pt t) ;

Bool IsUnitInUse(UnitPt u) ;

Bool ZStacksExpansion(Size ensureThisExtraSpace) ;
Bool ZTestStacksExpansion(void) ;
void TestRelocationUpdateFlag(int newValue) ;

void TestGCUpdateFlag(int newValue) ;

void CheckInvariants(void) ;
void TestInvariantsFlag(int newValue) ;

Size LocalStackUsed(void) ;
Size GlobalStackUsed(void) ;

void MachineInit(void) ;
void MachineInit2(void) ;

#endif
