/*
 *   This file is part of the CxProlog system

 *   Machine.c
 *   by A.Miguel Dias - 1989/11/25
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL

 *   CxProlog is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.

 *   CxProlog is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "CxProlog.h"


/* MACHINE REGISTERS */

Hdl P, CP, H, HB, TR, S ;
EnvironmentPt E ;
ChoicePointPt B, B0, R, L ;
MixReg Q, Z ;
FinalizerPt F ;
Pt C, CH, D, X[maxX], saveC ;
Pt GlobalClock ;
Pt ZT ; /* Reserved for use in the function "ZPushCompoundTerm" */
Hdl stacksBegin0, stacksEnd0 ;
Hdl stacksBegin, stacksEnd ;
Hdl trailBegin, trailEnd ;
static Pt trailFreeVar ;
static Hdl saveH, saveTR, saveHB ;
static Size hostSpeed ;
int N ;
Bool running = false ;

void MachineRun()
{
	running = true ;
	SysTraceMachineRun() ;
#if USE_THREADED_CODE
	JumpNext() ;
#else
	for(;;) {
		InstRun() ; InstRun() ; InstRun() ; InstRun() ; InstRun() ;
		InstRun() ; InstRun() ; InstRun() ; InstRun() ; InstRun() ;
#if 1
	extern Bool threadSwitchRequest ;
	if( threadSwitchRequest )
		ThreadConcurrencyHandle();
#endif		
	}
#endif
	InternalError("MachineRun") ;
}

void CheckHost()
{
	if( sizeof(Word) != sizeof(Pt) )
		FatalError("sizeof(Word) != sizeof(Pt)") ;
	if( cWord(&stacksBegin0) % sizeof(Word) != 0 )
		Warning("Machine registers are not aligned") ;
	if( cWord(ScratchStart()) % sizeof(Word) != 0 )
		FatalError("Memory areas are not aligned") ;
	if( sizeof(Size) < sizeof(size_t) )
		Warning("'Size' type too small") ;
}

void DumpRegisters()
{
	Mesg(" P = %lx, H = %lx, B = %lx, B(P) = %lx, E = %lx, C = %s ",
				P, H, B, Bf(P), E, TermAsStr(C)) ;
}

/* ZCheckHostSpeed: Roughly evaluates the speed of the host machine.
   The "speed" is the number of times the machine manages to parse and build
   some term from a string and then convert the term back to a string. The
   amount of time given to this is 0.04 sec.
*/

void ZCheckHostSpeed()
{
	double tt = CpuTime() ;
	hostSpeed = 0 ;
	HSave() ;
	do {
		Str s = "a(P,T,O):-a(0,P),a(O,T,P,L,R),a(T,P,L,R),a(0,T)." ;
		TermAsStr(ZTermFromStr(s)) ;
		HRestore() ;
		hostSpeed++ ;
	} while( CpuTime() - tt < 0.04 ) ;
	if( hostSpeed == 0 ) hostSpeed = 1 ;
}

void ContextSetEmpty()
{
	saveC = C ;
	C = tNilAtom ;
}

void ContextRestore()
{
	C = saveC ;
	saveC = nil ;   /* Makes GC more precise */
}



/* RelocateHandler Table */

#define maxRelocateStacksHandlers  20

static RelocateStacksProc RelocateStacksHdl[maxRelocateStacksHandlers] ;
static Str RelocateStacksName[maxRelocateStacksHandlers] ;
static int nRelocateStacksHdl = 0 ;

void InstallRelocateStacksHandler(Str name, RelocateStacksProc p)
{
	if( nRelocateStacksHdl == maxRelocateStacksHandlers )
		InternalError("Too many RELOCATE STACK handlers: increase the table capacity") ;
	RelocateStacksHdl[nRelocateStacksHdl] = p ;
	RelocateStacksName[nRelocateStacksHdl++] = name ;
	
}

void RelocateStacks(Size globalOffset, Size localOffset)
{
	int i ;
#if 0
	if( cHdl(B)[6] != 0 ) {
		MachineShow("global") ;
		MachineShow("local") ;
		MachineShow("trail") ;
		MesgW("B = %lx, E = %lx, TR = %lx", B, E, TR) ;
	}
#endif
	dotimesrev(i, nRelocateStacksHdl) { /* Must be in reverse order */
#if 0
		if( i == nRelocateStacksHdl - 1 )
			Mesg("...............") ;
		Mesg("%2d - %s", i, RelocateStacksName[i]) ;
#endif
		RelocateStacksHdl[i](globalOffset, localOffset) ;
	}
	SysTraceWrite("*** RelocateStacks ***\n") ;
}



/* RelocateRange function */

static void RelocateRange(Hdl low, Hdl high, Size targetOffset,
			Size globalOffset, Size localOffset, Bool allowEmpty, Bool global)
{
	Bool up = targetOffset <= 0 ;
	int delta = up ? 1 : -1 ;
	register Hdl a = up ? low : high - 1 ;
	register Hdl z = a + targetOffset ;
	Hdl stop = up ? high : low - 1 ;
	for( ; a != stop ; a += delta, z += delta ) {
		if( IsEmpty(*a) ) {
			if( allowEmpty )
				*z = *a ;
			else
				InternalError("RelocateRange: Invalid empty word") ;
		}
		elif( IsVar(*a) ) {
			if( localOffset != 0 && IsLocalRef(*a) ) {
				if( global ) {
					/* Sole instance of global stack pointing to local stack */
					if( a > low && a[-1] == cPt(metaCutFunctor) )
						*z = *a + localOffset ;
					else
					 InternalError("RelocateRange: Invalid local at global") ;
				}
				else
					*z = *a + localOffset ;
			}
			elif( globalOffset != 0 && IsGlobalRef(*a) ) *z = *a + globalOffset ;
			else *z = *a ;
		}
		elif( globalOffset != 0 && IsGlobalCompound(*a) ) *z = *a + globalOffset ;
		else
			*z = *a ;
	}
}



/* LOCAL VARS RELOCATE STACK */

#define lvrInitialCapacity		64

static Hdl3 lvrBegin, lvrEnd, lvrPt ;

static void LvrInit(void)
{
	lvrBegin = Allocate(lvrInitialCapacity, false) ;
	lvrEnd = lvrBegin + lvrInitialCapacity ;
	lvrPt = lvrBegin ;
}

static void LvrExpand(void)
{
	Size oldCapacity = lvrEnd - lvrBegin ;
	Size newCapacity = oldCapacity * 2 ;
	Hdl3 newLvrBegin, newLvrEnd ;
	MemoryGrowInfo("local vars relocate stack", oldCapacity, newCapacity) ;
	newLvrBegin = Reallocate(lvrBegin, oldCapacity, newCapacity) ;
	newLvrEnd = newLvrBegin + newCapacity ;
	lvrPt += newLvrBegin - lvrBegin ;
	lvrBegin = newLvrBegin ;
	lvrEnd = newLvrEnd ;
}

Size LvrPush(VoidPt addr)
{
	if( lvrPt == lvrEnd )
		LvrExpand() ;
	*lvrPt++ = addr ;
	return lvrPt - lvrBegin - 1 ;
}

Size LvrPushN(VoidPt first, ...)
{
	if( first == nil )
		return lvrPt - lvrBegin ;
	else {
		VoidPt pt ;
		va_list v ;
		Size lvrLevel = LvrPush(first) ;
		va_start(v, first) ;
		while( (pt = va_arg(v, VoidPt)) != nil )
			LvrPush(pt) ;
		va_end(v) ;
		return lvrLevel ;
	}
}

void LvrRestore(Size n)
{
	lvrPt = lvrBegin + n ;
}

void LvrRestart()
{
	lvrPt = lvrBegin ;
}

static void RelocateLvrTable(Size globalOffset, Size localOffset)
{
	register Hdl3 h ;
	for( h = lvrBegin ; h < lvrPt ; h++ ) {
		if( IsVar(**h) ) {
			if( localOffset != 0 && IsLocalRef(**h) ) **h += localOffset ;
			elif( globalOffset != 0 && IsGlobalRef(**h) ) **h += globalOffset ;
			else { /* nothing */ } ;
		}
		elif( globalOffset != 0 && IsGlobalCompound(**h) ) **h += globalOffset ;
		else { /* nothing */ } ;
	}
}



/* SHOW MACHINE STATE */

void ShowWord(Hdl h, Bool terms)
{
	Pt t = *h ;
	Write("  %08lx: ", h) ;
	Write("(%08lx) ", t) ;
	if( IsEmpty(t) )
		Write("EMPTY %lx\n", t) ;
	elif( t == nil )
		Write("NULL\n") ;
	elif( IsVar(t) ) {
		if( IsLocalRef(t) )
			Write("LOCAL_VAR %lx\n", t) ;
		elif( IsGlobalRef(t) )
			Write("GLOBAL_VAR %lx\n", t) ;
		elif( IsTrailRef(t) )
			Write("TRAILREF %lx\n", t) ;
		elif( FunctorCheck(t) )
			Write("FUNCTOR %s\n", FunctorNameArity(cFunctorPt(t))) ;
		elif( UnitCheck(t) )
			Write("UNIT %s\n", UnitSignature(cUnitPt(t))) ;
		elif( t == trailFreeVar )
			Write("CODE_FREE_VAR %lx\n", t) ;
		else
			Write("CODE %lx\n", t) ;
	}
	elif( IsAtomicStrict(t) )
		Write("ATOMIC %s\n", TermAsStr(t)) ;
	elif( IsExtra(t) )
		Write("EXTRA %s\n", XExtraAsStr(t)) ;
	elif( IsStruct(t) ) {
		if( terms )
			Write("STRUCT %s  <%lx>\n", TermAsStr(t), XPt(t)) ;
		else
			Write("STRUCT %lx\n", XPt(t)) ;
	}
	elif( IsList(t) ) {
		if( terms )
			Write("LIST %s  <%lx>\n", TermAsStr(t), XPt(t)) ;
		else
			Write("LIST %lx\n", XPt(t)) ;
	}
	else Write("?_?_?_? %lx\n", t) ;
}

void ShowMachineRange(Str n, Hdl a, Hdl z)
{
	Write("%s [%lx, %lx]:\n", n, a, z) ;
	if( a <= z ) {
		for( ; a < z ; a++ )
			ShowWord(a, false) ;
	}
	else {
		for( a-- ; a >= z ; a-- )
			ShowWord(a, false) ;
	}
	Write("%s ----------------------\n", n) ;
}	
		
Bool MachineShow(Str s)
{
	if( StrEqual(s, "local") )
		ShowMachineRange(s, stacksEnd, TopOfLocalStack()) ;
	elif( StrEqual(s, "global") )
		ShowMachineRange(s, stacksBegin, H) ;
	elif( StrEqual(s, "trail") )
		ShowMachineRange(s, trailBegin, TR) ;
	elif( StrEqual(s, "finalizers") )
		ShowMachineRange(s, cHdl(F), trailEnd) ;
	elif( StrEqual(s, "x") )
		ShowMachineRange(s, X, FindEmpty(X) + 1) ;
	else return false ;
	return true ;
}

void MachineShowReg(void) {
	Write("H = %p, HB = %p, TR = %p, E = %p, B = %p, B0 = %p, R = %p, L = %p\n",
			H, HB, TR, E, B, B0, R, L);
}


/* TRAIL */

/* The trail contain references to bound logic variables located at the
   local stack and to bound logic variables located at the global stack.
   Also contains trailed ivars, represented by pairs (saved_value, atom).
*/

void TrailIVar(AtomPt atom, Pt oldValue)
{
	PushTrail(oldValue) ;
	PushTrail(TagAtom(atom)) ;
}

void TrailRestore(Hdl until)
{
	register Pt t ;
/*
	if( !InRange(until, trailBegin, TR) ) {
		Mesg("trailBegin = %lx TR = %lx until = %lx", trailBegin, TR, until) ;
		InternalError("TrailRestore") ;
	}
*/
	while( TR != until ) {
		t = Pop(TR) ;
		if( IsAtom(t) ) /* is this a trailed ivar? */
			IVarReversibleRestore(XAtom(t), Pop(TR)) ;
		else ResetVar(t) ;
	}
}

void TrailAllVarsStart()	/* pre: used only on indivisible operations */
{
	saveTR = TR ;
	saveHB = HB ;
	HB = stacksEnd ;	/* Forces trailing of all vars */
}

void TrailAllVarsRestore()
{
	TrailRestore(saveTR) ;
	HB = saveHB ; /* restore HB, changed in TrailAllVarsStart */
}

#if unused
static void TrailTidy(void) /* not used */
{
	register Hdl h = Bf(TR) ;
	while( h < TR )
		if( IsToTrailVar(*h) ) h++ ;
		else *h = Pop(TR) ;
}
#endif

static void RelocateAllForTrailExpand(Size trailOffset, Size finalizerOffset)
{
	register Hdl h ;

/* Copy the trail vars */
	CopyWords(trailBegin+trailOffset, trailBegin, TR-trailBegin) ;

/* Copy the cut finalizers */
	CopyWords(cHdl(F)+finalizerOffset, cHdl(F), trailEnd-cHdl(F)) ;

/* Relocate all the trail-refs in the local stack (CPs e DFs)  */
	for( h = TopOfLocalStack() ; h < stacksEnd ; h++ )
		if( IsTrailRef(*h) ) *h += trailOffset ;

/* Relocate the trail/finalizers registers */
	TR += trailOffset ;
	saveTR += trailOffset ;
	F = cFinalizerPt(cHdl(F) + finalizerOffset) ;
}

void TrailExpand()
{
	Size oldTrailSize, newTrailSize ;
	Hdl newTrailBegin, newTrailEnd ;

/* Compute sizes */
	oldTrailSize = trailEnd - trailBegin ;
	newTrailSize = 2 * oldTrailSize ;
	MemoryGrowInfo("trail", oldTrailSize, newTrailSize) ;

/* Allocate the new trail and relocate the machine state  */
	newTrailBegin = AllocateSegmentEmpty(newTrailSize, &newTrailEnd) ;
	RelocateAllForTrailExpand(newTrailBegin - trailBegin, newTrailEnd - trailEnd) ;
	ReleaseSegment(trailBegin, oldTrailSize) ;
	trailBegin = newTrailBegin ;
	trailEnd = newTrailEnd ;
}

static void RelocateTrail(Size globalOffset, Size localOffset)
{
/* Relocate the trailed vars */
	RelocateRange(trailBegin, TR, 0, globalOffset, localOffset, false, false) ;
/* Relocate the cut finalizers */
	RelocateRange(cHdl(F), trailEnd, 0, globalOffset, localOffset, false, false) ;
}

static void TrailBasicGCMark()
{	/* handle saved values of trailed ivars */
	register Hdl h ;
	for( h = TR-1 ; h > trailBegin ; h-- )
		if( IsAtom(*h) ) { /* is this a trailed ivar? */
			ExtraGCMark(XExtra(*h)) ; /* unnecessary, but does not hurt */
			TermBasicGCMark(h[-1]) ;
			h-- ;
		}
}

static void TrailInit(void)
{
	trailFreeVar = GetCodeFreeVar() ;
}


/* X REGISTERS */

/* The X registers contain references to local vars, references to global
   vars, references to terms in the global stack.  */

static void RelocateXRegs(Size globalOffset, Size localOffset)
{
	RelocateRange(X, X + maxX, 0, globalOffset, localOffset, true, false) ;
}


/* OTHER REGISTERS */

static void RelocateOtherRegs(Size globalOffset, Size localOffset)
{
	if( globalOffset != 0 && IsGlobalCompound(Z.t) ) Z.t += globalOffset ; /* must be first */
	/* Q ??? @@@ */
	if( globalOffset != 0 && IsGlobalCompound(ZT) ) ZT += globalOffset ;

	if( globalOffset != 0 && IsGlobalCompound(C) ) C += globalOffset ;
	if( globalOffset != 0 && IsGlobalCompound(saveC) ) saveC += globalOffset ;
	if( globalOffset != 0 && IsGlobalCompound(CH) ) CH += globalOffset ;

	E = cVoidPt(cPt(E) + localOffset) ;
	B = cVoidPt(cPt(B) + localOffset) ;
	B0 = cVoidPt(cPt(B0) + localOffset) ;
	R = cVoidPt(cPt(R) + localOffset) ;
	L = cVoidPt(cPt(L) + localOffset) ;

	H += globalOffset ;
	saveH += globalOffset ;
	saveHB += globalOffset ;
	HB += globalOffset ;
	if( S != nil ) /* read mode */
		S += globalOffset ;
}


/* LOCAL STACK */

/* The local stack contains:
	Environments
		local vars (Y regs),
			references to local vars (Y),
			references to global vars (Y),
			references to terms ??? (Y),
			temporarily undefined values
		reference to Environment (E),
		reference to executable code (CP),
		reference to ChoicePoint (B0)

	ChoicePoints
		saving X regs and "state cells" of non-deterministics - (A regs),
			references to local vars (Y),
			references to global vars (Y),
			references to terms ??? (Y),
		reference to Environment (E),
		references to executable code (CP, P),
		references to ChoicePoint (B, B0),
		references to terms (C, CH),
		reference to the trail (TR),
		reference to the global stack (H)

	DebugFrames.
		references to executable code
			(exitInst, redoInst, retryInst, callCP, redoP),
		reference to predicate (pred),
		reference to clause (currClause),
		references to terms ??? (callC, callCH, frameN),
		reference to the trail (redoTR),
		reference to the DebugFrame (father),
		int DebugEvent event ;
		int Char type ;
		Size lineN ;
*/

static void RelocateLocalStack(Size globalOffset, Size localOffset)
{
	RelocateRange(TopOfLocalStack(), stacksEnd, localOffset,
									globalOffset, localOffset, true, false) ;
 }



/* GLOBAL STACK */

/* The local stack contains:
	Global vars
	Functors (of structs)
	Unit references
	Terms in the global stack
	Metacut pointer to choicepoint in the local stack
	Terms in the code area ??? not anymore
*/

void HSave()
{
	saveH = H ;
}

void HRestore()
{
	H = saveH ;
}

Size HGrown()
{
	return H - saveH ;
}

void SafeAssignToGlobal(Hdl h, Pt t)
{
	VarValue(t) ;
	if( IsVar(t) && IsLocalVar(t) )
		Assign(t, ResetVar(h))
	else
		*h = t ;
}

static void RelocateGlobalStack(Size globalOffset, Size localOffset) {
	RelocateRange(stacksBegin, H, globalOffset,
									globalOffset, localOffset, false, true) ;
}

Bool IsUnitInUse(UnitPt u)
{
	register Hdl a ;

	for( a = stacksBegin ; a < H ; a++ )
		if( Eq(*a, u) )
			return true ;
	return false ;
}


/* BOTH STACKS */

#define SHOW_RELOC	0

Bool ZStacksExpansion(Size ensureThisExtraSpace)
{
	Size oldStacksSize, newStacksSize ;
	Hdl newStacksBegin, newStacksEnd ;

	oldStacksSize = stacksEnd - stacksBegin ;

	if( stacksBegin != stacksBegin0 ) { /* Only for testRelocation */
#if SHOW_RELOC
		Dot("*") ;
#endif
		newStacksSize = stacksEnd0 - stacksBegin0 ;
		RelocateStacks(stacksBegin0 - stacksBegin, stacksEnd0 - stacksEnd) ;
		stacksBegin = stacksBegin0 ; /* Restore whole stacks */
		stacksEnd = stacksEnd0 ;
		if( newStacksSize - oldStacksSize >= ensureThisExtraSpace )
			return true ;
#if SHOW_RELOC
		Dot("+") ;
#endif
	}

	for( newStacksSize = oldStacksSize * 2 ;
				newStacksSize - oldStacksSize < ensureThisExtraSpace ;
						newStacksSize *= 2) ;
	MemoryGrowInfo("stacks", oldStacksSize, newStacksSize) ;

	newStacksBegin = AllocateSegmentEmpty(newStacksSize, &newStacksEnd) ;
	RelocateStacks(newStacksBegin - stacksBegin, newStacksEnd - stacksEnd) ;
	ReleaseSegment(stacksBegin0, oldStacksSize) ;
	stacksBegin = stacksBegin0 = newStacksBegin ;
	stacksEnd = stacksEnd0 = newStacksEnd ;
	return true ;
}

Bool ZTestStacksExpansion()
{
#if SHOW_RELOC
	Dot(".") ;
#endif
	RelocateStacks(1, -1) ;
	stacksBegin += 1 ;
	stacksEnd += -1 ;
	return true ;
}

/* GARBAGE COLLECTION */

static void MachineStateBasicGCMark()
{
	TrailBasicGCMark() ;								/* trailed ivars */
	ExtraGCMarkRange(stacksBegin, H) ;					/* global stack */
	ExtraGCMarkRange(TopOfLocalStack(), stacksEnd) ;	/* local stack */
	ExtraGCMarkRange(X, X + maxX) ;						/* X regs */
}

/*
void MachineStateUnitGCMark()
{
	register ChoicePointPt cp ;
	for( cp = B ; !EndOfCPChain(cp) ; cp = cp->B )
		if( cp->C != tNilAtom )
			UnitGCMark(cUnitPt(XPt(cp->C)[-1]))
}
*/

static void GlobalStackInvariants(void)
{
	Hdl h ;
return;
	for( h = stacksBegin ; h < H ; h++ ) {
		if( IsVar(*h) )
			if( IsLocalVar(*h) )
				Mesg("INVARIANT BROKEN: Global pointing to local %p @ %p", *h, h) ;
	}
}

void CheckInvariants(void)
{
	if( testInvariants_flag ) {
		GlobalStackInvariants() ;
		Attention() = true ;
	}
}

/* MACHINE STATE */

void TestRelocationUpdateFlag(int newValue)
{
	if( testRelocationDebugging_flag > 0 )
		newValue = Max(testRelocationDebugging_flag, newValue) ;
	testRelocation_flag = newValue ;
	if( testRelocation_flag )
		Attention() = true ;
}

void TestGCUpdateFlag(int newValue)
{
	testGCollection_flag = newValue ;
	if( testGCollection_flag )
		Attention() = true ;
}

void TestInvariantsFlag(int newValue)
{
	if( testInvariantsDebugging_flag > 0 )
		newValue = Max(testInvariantsDebugging_flag, newValue) ;
	testInvariants_flag = newValue ;
	if( testInvariants_flag )
		Attention() = true ;
}

void MachineInit()
{
	EmptyRangeN(X, maxX) ;
	ExtraGCHandlerInstall("MACHINE", MachineStateBasicGCMark) ;
	LvrInit() ;
/* This ordering is mandatory. Don't change it. */
	InstallRelocateStacksHandler("OtherRegs", RelocateOtherRegs) ;
	InstallRelocateStacksHandler("Trail", RelocateTrail) ;
	InstallRelocateStacksHandler("XRegs", RelocateXRegs) ;
	InstallRelocateStacksHandler("GlobalStack", RelocateGlobalStack) ;
	InstallRelocateStacksHandler("LocalStack", RelocateLocalStack) ;
	InstallRelocateStacksHandler("LvrTable", RelocateLvrTable) ;
}

Size LocalStackUsed()
{
	return stacksEnd - TopOfLocalStack() ;
}

Size GlobalStackUsed()
{
	return H - stacksBegin ;
}


/* CXPROLOG C'BUILTINS */

static void PHostSpeed()
{
	Size speed = hostSpeed ;
	if( testRelocation_flag || testGCollection_flag ) speed /= 200 ;
	MustBe( UnifyWithAtomic(X0, MakeInt(speed)) ) ;
}

static void PDeterministic()
{
	Bool value = (B == Ef(B0)) || (Bf(B0) == Ef(B0) && Bf(CP) == Ef(CP)) ;
	MustBe( UnifyWithAtomic(X0, MakeBool(value)) ) ;	
/*
	qq :- deterministic(X), writeln(X).
	qq :- (Z=1;Z=2;Z=3), writeln(Z), deterministic(X), writeln(X).
	qq :- writeln(last), deterministic(X), writeln(X).
*/		
}

static void PMShow()
{
	if( !MachineShow(XTestAtomName(X0)) )
		Error("Unknown option") ;
	JumpNext() ;
}

void MachineInit2()
{
	TrailInit() ;
	InstallCBuiltinPred("host_speed", 1, PHostSpeed) ;
	InstallCBuiltinPred("deterministic", 1, PDeterministic) ;
	InstallCBuiltinPred("mshow", 1, PMShow) ;
}
