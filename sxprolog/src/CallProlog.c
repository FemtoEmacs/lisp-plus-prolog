/*
 *   This file is part of the CxProlog system

 *   CallProlog.c
 *   by A.Miguel Dias - 2003/06/20
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

/*
Possible outcomes of the functions CallProlog, CallPrologMulti, CallPrologMultiNext, CallPrologMultiStop.

PrologEvent = 
	peSucc			->	. the goal has succeed
						. a result becomes available if there was one setup by set_result/1
						. extra instantiation of the goal becomes available only in the "multi" version
	peFailure		->	. the goal has failed
						. there is no result, even if there is one setup by set_result/1
						. no extra instantiation of the goal becomes available
	peException		->	. an uncaught exception has occurred
						. no result and no extra instantiation, like in peFailure
						. a subsequent call to WriteException would prints the
							usual "uncaught exception" message
	peInterrupt		->  . the user has issued a Ctrl-C interrupt.
						. no result and no extra instantiation, like in peFailure
	peAbort			->  . the predicate abort/0 has been called, or the user issued a Ctrl-C interrupt.
						. no result and no extra instantiation, like in peFailure
	peExit			->  . the predicate exit/0 has been called
						. no result and no extra instantiation, like in peFailure
	peHalt			->	. the predicate halt/0 has been called.
						. no result and no extra instantiation, like in peFailure
	peFatalError	->  . there was an internal fatal error.
						. no result and no extra instantiation, like in peFailure
	peThreadError	->  . call from outside the CxProlog thread in CallProlog/CallPrologMulti
	peOtherError	->  . call of CallPrologMultiNext without a previous CallPrologMulti
						. call of CallPrologStr with an invalid term-string
} PrologEvent ;
*/

#include "CxProlog.h"

Pt vars ;

#define DEBUG	0

static jmp_buf *currJB = nil ;
static long mainThreadId = 0 ;
static Bool ignoreEmergencyExit = false ;
static Pt currentResult ;

int callLevel = 0 ;
Bool hasATopLevel = false ;
Bool3 normalFinish = undefined3 ;

Str PrologEventAsStr(PrologEvent ev)
{
	switch( ev ) {
		case peNone: return "peNone" ;
		case peSucc: return "peSucc" ;
		case peFailure: return "peFailure" ;
		case peException: return "peException" ;
		case peInterrupt: return "peInterrupt" ;
		case peAbort: return "peAbort" ;
		case peExit: return "peExit" ;
		case peHalt: return "peHalt" ;
		case peFatalError: return "peFatalError" ;
		case peThreadError: return "peThreadError" ;
		case peOtherError: return "peOtherError" ;
		case peContinue: return "peContinue" ;
		case peForceFail: return "peForceFail" ;
		case peMoreSuccess: return "peMoreSuccess" ;
		case peReturnSuccess: return "peReturnSuccess" ;
		case peReturnFailure: return "peReturnFailure" ;
		default: return "Unknown PrologEvent" ;
	}
}

PrologEvent CallFunThruProlog(Fun fun)
{		 /* Internally called at OnGuiApp::GuiCall() */
	PrologEvent ev ;
	jmp_buf newJB ;
	jmp_buf *saveJB = currJB ;
	currJB = &newJB ;
	if( (ev = setjmp(*currJB)) == peNone )
		fun() ;
	currJB = saveJB ;
	return ev ;
}

Pt ZTermFromStrThruProlog(Str goal) {
	volatile Pt t = nil ;
	jmp_buf newJB ;
	jmp_buf *saveJB = currJB ;
	currJB = &newJB ;
	if( setjmp(*currJB) == peNone )
		t = ZTermFromStr(goal) ;
	currJB = saveJB ;
	return t ;
}

static void RestartAll(void)
{
	CutAll() ;
	ScratchRestart() ;
	DebugRestart() ;
	FlagsRestart() ;
	InterLineRestart() ;
	ConsultRestart() ;
	LvrRestart() ;
	ClauseGC(true) ;
	ActiveThreadReset() ;
}

Bool InMainThread(Bool warn)
{
	if( OSGetTid() == mainThreadId )
		return true ;
#if COMPASS
	RefreshProlog() ;
	if( OSGetTid() == mainThreadId )
		return true ;	
#endif
	if( warn )
		Warning("Ignored illegal reentrant call from thread %ld. Main thread is %ld",
					OSGetTid(), mainThreadId) ;
	return false ;
}

/* If this instance of CxProlog resulted form a fork you
   may need to call this right after the fork */

void RefreshProlog()
{
	static Bool done = false ;	/* only once */
	if( !done ) {
		mainThreadId = OSGetTid() ;
		done = true ;
	}
}

static void ExitProlog(void)
{
	static Bool done = false ; 
	if( !done ) {
		done = true ;
		ProcessesFinish() ;
		StreamsSane() ;
		InterLineFinish() ;
		DeleteConsole() ;
		GoHome(false) ; /* for DJGPP */
	}
}

void SendPrologEvent(PrologEvent ev)
{
	if( currJB == nil ) {
		if( !ignoreEmergencyExit ) {
			if( ev == peException )
				WriteException(nil) ;
			Mesg("Emergency exit: No event handler for '%s'",
				PrologEventAsStr(ev)) ;
			exit(0) ;
		}
	}
	else
		longjmp(*currJB, ev) ;
}

void EventContinue()	{ SendPrologEvent(peContinue) ; }
void EventForceFail()	{ SendPrologEvent(peForceFail) ; }
void EventException()	{ SendPrologEvent(peException) ; }
void EventInterrupt()	{ SendPrologEvent(peInterrupt) ; }
void EventAbort()		{ SendPrologEvent(peAbort) ; }
void EventExit()		{ SendPrologEvent(peExit) ; }
void EventHalt()		{ SendPrologEvent(peHalt) ; }
void EventFatalError()	{ SendPrologEvent(peFatalError) ; }

static Pt GatherVarsPrepare(Pt t)
{
	Pt v = MakeUnStruct(LookupFunctorByName("$vars", 1), VarNames()) ;
	Pt a = MakeUnStruct(LookupFunctorByName("assert", 1), v) ;
	AbolishPredicate(LookupPredicateByName("$vars", 1), true) ;
	return MakeBinStruct(commaFunctor, t, a) ;
}

static Str GatherVars(void)
{
	BigStr2Open() ;	
#if 1
	Pt v = ClauseSource(PredClauses(LookupPredicateByName("$vars", 1))) ;
	Pt t = XStructArg(v, 0) ;
	for( t = Drf(t) ; IsList(t) ; t = Drf(XListTail(t)) ) {
		Hdl args ;
		XTestStruct(XListHead(t), &args) ;
		BigStr2AddStr(SubtermAsStrN(args[0], v)) ;
		BigStr2AddStr(" = ") ;
		BigStr2AddStr(SubtermAsStrN(args[1], v)) ;
		BigStr2AddStr("\n") ;
	}
#endif
	AbolishPredicate(LookupPredicateByName("$vars", 1), true) ;
	return BigStr2Close() ;
}

static int LevelPointCount(void)
{
	ChoicePointPt b ;
	int n = 0 ;
	for( b = L ; !IsEndOfChain(b) ; b = cChoicePointPt(ChoicePointArg(b, 5)) )
		n++ ;
	return n ;
}

#define LevelPointSize		7
#define ResultPos			4

static void LevelPointPush(Hdl saveH)
{
	if( DEBUG || callLevel > 0 )
		Info(4, "Reentrant Prolog call, level %d", callLevel) ;
	/* Saves X0-X3 */
	X4 = tNoResult ;				/* Result */
	X5 = cPt(P) ;					/* Save P */
	X6 = cPt(L) ;					/* Save L */
	CreateChoicePoint(PredCode(CheckPredicateByName("$$_return_failure", 0)), LevelPointSize) ;
	Bf(H) = saveH ;
	L = B ;
	callLevel++ ;
}

static void LevelPointPop(void)
{
	callLevel-- ;
	if( B != L )
		InternalError("PopLevelPoint") ;
	RestoreState(LevelPointSize) ;
	Discard() ;
	P = cHdl(X5) ;					/* Restore P */
	L = cChoicePointPt(X6) ;		/* Restore L */
	if( DEBUG || callLevel > 0 )
		Info(4, "Exit reentrant Prolog call, level %d", callLevel) ;
}

static PrologEvent CallPrologHandleEvents(void)
{
	PrologEvent ev ;
	Size lvrLevel = LvrPushN(nil) ;
    jmp_buf newJB ;
    jmp_buf *saveJB = currJB ;
	currJB = &newJB ;
	ev = setjmp(*currJB) ;
/* longjmp destination */
	LvrRestore(lvrLevel) ;
#if DEBUG
	Mesg("%s %p", PrologEventAsStr(ev), L) ;
#endif	
	switch( ev ) {
		case peNone: {	/* Startup */
			MachineRun() ;
			break ;
		}
		case peContinue: {
			MachineRun() ;
			break ;
		}
		case peForceFail: {
			P = Bf(P) ;
			MachineRun() ;
			break ;
		}
		case peException:
		case peInterrupt:
		case peAbort:
		case peExit:
		case peHalt:
		case peFatalError: {
			ThreadSwitchToRoot() ;
			currentResult = tNoResult ;
			currJB = saveJB ;
			CutTo(L) ;
			LevelPointPop() ;
			return ev ;
		}
		case peMoreSuccess: {
			currentResult = ChoicePointArg(L, ResultPos) ;
			currJB = saveJB ;
			return peSucc ;
		}
		case peReturnSuccess: {
			currentResult = ChoicePointArg(L, ResultPos) ;
			currJB = saveJB ;
			LevelPointPop() ;
			return peSucc ;			
		}
		case peReturnFailure: {
			currentResult = tNoResult ;
			currJB = saveJB ;
			LevelPointPop() ;
			return peFailure ;			
		}
		default:
			break ;
	}
	FatalError("CallPrologHandleEvents") ;
	return peOtherError ;
}

static PrologEvent CallPrologEnter(Pt goal, Bool multi, Hdl saveH)
{
	PrologEvent res ;
#if DEBUG
	Mesg(" CALL[%d] - %s ", callLevel, TermAsStr(goal)) ;
	DumpRegisters() ;
#endif
	if( !InMainThread(true) )
		return peThreadError ;

	LevelPointPush(saveH) ;
	X0 = goal ;
	P = PredCode(CheckPredicateByName(
					multi ? "$$_call_prolog_multi" : "$$_call_prolog", 1)) ;
	res = CallPrologHandleEvents() ;
#if DEBUG
	Mesg(" EXIT[%d] - EV = %s ", callLevel, PrologEventAsStr(res)) ;
	DumpRegisters() ;
#endif
	return res ; 
}

static PrologEvent CallPrologNext(void)
{
	if( IsEndOfChain(L) )
		return peOtherError ;
	else {
		P = Bf(P) ;
#if DEBUG
		Mesg(" RECALL[%d]", callLevel) ;
		DumpRegisters() ;
#endif
		return CallPrologHandleEvents() ;
	}
}

static PrologEvent CallPrologStop(void)
{
	if( IsEndOfChain(L) )
		return peOtherError ;
	else {
		currentResult = tNoResult ;
		CutTo(L) ;
		LevelPointPop() ;
		return peFailure ;
	}
}

void CallPrologSetResult(Pt t)
{
	if( IsEndOfChain(L) )
		FatalError("CallPrologSetResult") ;
	else {
		Pt p = ChoicePointArg(L, ResultPos)	;
		ReleaseTerm(p) ;
		p = AllocateTermForAssign(t) ;
		ChoicePointArg(L, ResultPos) = p ;
	}
}

Pt CallPrologGetResult(void)
{
	return currentResult	;	
}

/* Call once
    - Runs the goal once, only for side effects.
*/
PrologEvent CallProlog(Pt goal)
{		 /* Internally called at portray/1, format/1/2/3, on_interrupt/1 */
	return CallPrologEnter(goal, false, H) ;
}

PrologEvent CallPrologThrow(Pt goal)
{
	PrologEvent ev ;
	if( (ev = CallProlog(goal)) == peException )
		Rethrow() ;
	return ev ;
}

void CallPrologTransparent(Pt goal) /* Ignore result. */
{		/* Internally called from call_cleanup/2 */
#if 0
	Mesg("CallPrologTransparent %s", TermAsStr(goal)) ;
	LvrPush(&goal) ;
	if( CallProlog(goal) == peException ) {
		Mesg("CallPrologTransparent Rethrow %s", TermAsStr(goal)) ;
		Rethrow() ;
	}
	else {
		Mesg("CallPrologTransparent Return %s", TermAsStr(goal)) ;
		LvrPop(&goal) ;
	}
#else
	CallPrologThrow(goal) ;
#endif	
}

PrologEvent CallPrologAtom(Str atom)
{
	return CallProlog(MakeTempAtom(atom)) ;
}

PrologEvent CallPrologStr(Str goal)
{		 /* Internally called by Java.c */
	Hdl saveH = H ;
	Pt t = ZTermFromStrThruProlog(goal) ;
	return t == nil ? peOtherError : CallPrologEnter(t, false, saveH) ;
}

PrologEvent CallPrologStr2(Str goal)
{
	Hdl saveH = H ;
	Pt t = ZTermFromStrThruProlog(goal) ;
	vars = VarNames();
	Mesg(TermAsStr(vars));
	return t == nil ? peOtherError : CallPrologEnter(t, false, saveH) ;
}

PrologEvent CallPrologStrTop(Str goal, Str *vars, Str *out)
{
	PrologEvent res ;
	Hdl saveH = H ;
	Pt t = ZTermFromStrThruProlog(goal) ;
	if( t == nil )
		res = peOtherError ;	
	else {
		if( vars != nil )
			t = GatherVarsPrepare(t) ;
		if( out != nil )
			CaptureOutputStart() ;	
		res = CallPrologEnter(t, false, saveH) ;
		if( out != nil )
			*out  = CaptureOutputEnd() ;
		if( vars != nil )
			*vars = res == peSucc ? GatherVars() : "" ;		
	}
	return res ;
}

PrologEvent CallPrologSerialized(Str fmt, ...)
{
	Unused(fmt) ;
#if 0
	va_list v ;
	va_start(v, fmt) ;
	while( *fmt )
		switch(*fmt++ ) {
			case '.':
				if( !EventQueueMakeSpace(1) ) goto abort ;
				*qLast++ = '.' ;
				break ;
			case 'i':
				if( !EventQueueMakeSpace(1 + sizeof(int)) ) goto abort ;
				*qLast++ = 'i' ;
				*(int *)qLast = va_arg(v, int) ;
				qLast += sizeof(int) ;
				break ;
			case 'f':
				if( !EventQueueMakeSpace(1 + sizeof(double)) ) goto abort ;
				*qLast++ = 'f' ;
				*(double *)qLast = va_arg(v, double) ;
				qLast += sizeof(double) ;
				break ;
			case 't':
				if( !EventQueueMakeSpace(1 + sizeof(Pt)) ) goto abort ;
				*qLast++ = 't' ;
				*(Pt *)qLast = va_arg(v, Pt) ;
				qLast += sizeof(Pt) ;
				break ;
			case 'o':
				if( !EventQueueMakeSpace(1 + sizeof(VoidPt)) ) goto abort ;
				*qLast++ = 'o' ;
				*(VoidPt *)qLast = va_arg(v, VoidPt) ;
				qLast += sizeof(VoidPt) ;
				break ;
			case 's': {
				Str s = va_arg(v, CharPt) ;
				int len = strlen(s) + 1 ;
				if( !EventQueueMakeSpace(1 + len) goto abort ;
				*qLast++ = 's' ;
				strcpy(qLast, s) ;
				qLast += len ;
				break ;
			}
			default:
				goto abort ;
				break ;
		}

	if( !EventQueueMakeSpace(1) ) goto abort ;
	*qLast++ = 'z' ;
	va_end(v) ;
	nEvents++ ;
	if( nEvents == 1 ) {
		qCondition.Signal() ;
		if( eventNotifier != nil )
			eventNotifier() ;
	}
	return ;
abort:
	va_end(v) ;


	Pt elem, t ;
	FunctorPt f ;
	ScratchSave() ;
	elem = nil ;
	for(;;) {
		elem = EventQueueGetItem() ;
		{
			int a = ScratchCurr() - ScratchStart() - 1 ;
			t = *ScratchStart() ;
			if( a > 0 ) {
				if( !IsAtom(t) ) {
					EventQueueReset() ;
					Error("Invalid wxWidgets serialized term") ;
				}
				f = LookupFunctor(XAtom(t), a) ;
				t = MakeStruct(f, ScratchStart() + 1) ;
			}
			FreeScratch() ;
			if( ScratchDistToSaved() == 0 ) break ;
			else ScratchPush(t) ;
		}
		else {
			UseScratch() ;
			ScratchPush(elem) ;
		}
	}
	nEvents-- ;
	return t ;
#endif
	return peSucc ;
}


/* Call multi
 - Iterates through all the solutions for the goal.
 - For each solution, the new logical variables instatiations becomes available.
 - The iteration ends with failure and the restoration of the initial Prolog state.
*/

PrologEvent CallPrologMulti(Pt goal)
{
	return CallPrologEnter(goal, true, H) ;
}

PrologEvent CallPrologMultiNext(void)
{
	return CallPrologNext() ;
}

PrologEvent CallPrologMultiStop(void)
{
	return CallPrologStop() ;
}

PrologEvent CallPrologMultiAtom(Str atom)
{
	return CallPrologMulti(MakeTempAtom(atom)) ;
}

PrologEvent CallPrologMultiStr(Str goal)
{
	Hdl saveH = H ;
	Pt t = ZTermFromStrThruProlog(goal) ;
	return t == nil ? peOtherError : CallPrologEnter(t, true, saveH) ;
}

PrologEvent CallPrologMultiStrTop(Str goal, Str *vars, Str *out)
{
	PrologEvent res ;
	Hdl saveH = H ;
	Pt t = ZTermFromStrThruProlog(goal) ;
	if( t == nil ) {
		res = peOtherError ;
		if( out != nil )
			*out  = "" ;
		if( vars != nil )
			*vars  = "" ;
	}		
	else {
		if( vars != nil )
			t = GatherVarsPrepare(t) ;
		if( out != nil )
			CaptureOutputStart() ;	
		res = CallPrologEnter(t, true, saveH) ;
		if( out != nil )
			*out  = CaptureOutputEnd() ;
		if( vars != nil )
			*vars = res == peSucc ? GatherVars() : "" ;		
	}
	return res ;
}

PrologEvent CallPrologMultiNextTop(Str *vars, Str *out)
{
	PrologEvent res ;
	if( out != nil )
		CaptureOutputStart() ;	
	res = CallPrologNext() ;
	if( out != nil )
		*out  = CaptureOutputEnd() ;
	if( vars != nil )
		*vars = res == peSucc ? GatherVars() : "" ;		
	return res ;
}

PrologEvent CallPrologMultiStopTop(Str *vars, Str *out)
{
	if( out != nil )
		*out = "" ;		
	if( vars != nil )
		*vars = "" ;		
	return CallPrologStop() ;
}



/* Start/Stop */

PrologEvent StartProlog(int argc, CharHdl argv, Fun yourExt)
{
	volatile PrologEvent res = peOtherError ;
	jmp_buf newJB ;
	jmp_buf *saveJB = currJB ;
	static Bool started = false ;
/* StartProlog is a no-op if called for a second time */
	if( started  )
		return peSucc ;
	started = true ;
	atexit(ExitProlog) ;
	currentResult = tNoResult ;
	currJB = &newJB ;
	if( setjmp(*currJB) == peNone ) {
/* The CxProlog interpreter can only be used by the thread that has created it */
		mainThreadId = OSGetTid() ;
		VersionSet("CxProlog", 0, 98, 2, -4, CONTEXTS) ;
		Boot(argc, argv) ;
		CheckPredicateByName("$$_lux0", 0) ;
		CheckPredicateByName("$$_lux1", 0) ;
		if( yourExt != nil )
			yourExt() ;
		res = CallPrologAtom("$$_lux0") ;
		CheckPredicateByName("$cxprolog_initialise", 0) ;
		CheckPredicateByName("$cxprolog_top_level_goal", 0) ;
	}
	else
		return peFatalError ;
		
	currJB = saveJB ;
#if DEBUG
		Mesg("RES0 = %s", PrologEventAsStr(res)) ;
#endif
	if( Booting() ) {
		ignoreEmergencyExit = true ;
		Error("Bad boot predicate") ;
		ignoreEmergencyExit = false ;
		res = peFatalError ;
	}
	return res ;
}

void StopProlog()
{
	/* Nothing */
}

int RunInteractiveProlog(int argc, CharHdl argv)
{
	PrologEvent res = StartProlog(argc, argv, nil) ;
	if( res == peException )
		WriteException(nil) ;
	while( res == peSucc || res == peAbort || res == peException ) {
		RestartAll() ;
		HasATopLevel() = true ;
		if( (res = CallPrologAtom("$$_lux1")) == peException )
			WriteException(nil) ;
#if DEBUG
		Mesg("RES1 = %s", PrologEventAsStr(res)) ;
#endif
	}
	if( NormalFinish() == undefined3 )
		NormalFinish() = true3 ;
	if( res == peHalt || res == peFatalError )
		BasicInfo("CxProlog halted") ;
	return res == peFatalError || res == peOtherError ? -1 : 0 ;
}		



/* CXPROLOG C'BUILTINS */

static void PAbort()
{
	EventAbort() ;
	JumpNext() ;
}

static void PExit()
{
	EventExit() ;
	JumpNext() ;
}

static void PHalt()
{
	EventHalt() ;
	JumpNext() ;
}

static void PExitScriptFast()
{
	NormalFinish() = false3 ;
	EventExit() ;
	JumpNext() ;
}

static void PCopyright()
{
	ShowVersion() ;
	Write("Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL\n") ;
	Write("CxProlog is free software and is distributed in the hope that it\n") ;
	Write("will be useful, but WITHOUT ANY WARRANTY; without even the implied\n") ;
	Write("warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n") ;
	Write("See the GNU General Public License for more details.\n") ;
	JumpNext() ;
}

static void PMoreSuccess()
{
	SendPrologEvent(peMoreSuccess) ;
	JumpNext() ;
}

static void PReturnSuccess()
{
	SendPrologEvent(peReturnSuccess) ;
	JumpNext() ;
}

static void PWriteException() /* for code::blocks */
{
	WriteException(nil) ;
	JumpNext() ;
}

static void PReturnFailure()
{
	SendPrologEvent(peReturnFailure) ;
	JumpNext() ;
}

static void PLevel()
{
	if( LevelPointCount() != callLevel )
		InternalError("PLevel") ;
	MustBe( UnifyWithNumber(X0, MakeInt(callLevel)) ) ;
}

static void PSetResult()
{
	CallPrologSetResult(X0) ;
	JumpNext() ;
}

static void PCallPrologThroughC()	/* For testing reentrant calls */
{
	PrologEvent ev ;
	if( (ev = CallPrologEnter(X0, false, H)) == peException )
		Rethrow() ;
	MustBe( ev == peSucc ) ;
}

static void PFatalError()	/* For testing CallProlog */
{
	FatalError("%s", XTestAtomName(X0)) ;
	JumpNext() ;
}

/*
static void test()
{
	ZTermFromStr("o la") ;
}
*/

static void CallPrologTestAux(Bool isStr, VoidPt arg)
{
	WriteStd("B=%lx E=%lx H=%lx\n", B, E, H) ;
	if( isStr ) {
		WriteStd("GOAL= \"%s\"\n", arg) ;
		WriteStd("RES = %s\n", PrologEventAsStr(CallPrologStr2(arg))) ;
		WriteStd("VARS = %s\n\n", TermAsStr(vars)) ;
	}
	else {	
		WriteStd("GOAL= CallFunThruProlog\n") ;
		WriteStd("RES = %s\n\n", PrologEventAsStr(CallFunThruProlog(arg))) ;
	}
}
static void PCallPrologTest()	/* For testing CallProlog */
{
	CallPrologTestAux(true, "X = ola, writeln(X)") ;
#if 0
	CallPrologTestAux(true, "fail") ;
	CallPrologTestAux(true, "throw(exc(1))") ;
	CallPrologTestAux(true, "exit") ;
	CallPrologTestAux(true, "halt") ;
	CallPrologTestAux(true, "abort") ;
	CallPrologTestAux(true, "'$$_fatal_error'('fatal error')") ;
	CallPrologTestAux(true, "o t h e r   e r r o r") ;
	CallPrologTestAux(false, test) ;
	CallPrologTestAux(true, "true") ;
#endif
	JumpNext() ;
}

static void PSizes()
{
	ShowSizes() ;
	JumpNext() ;
}

static void PCrash()
{
	Pt p = cPt(1) ;
	*p = 0 ;
	JumpNext() ;
}

static void PStartTimer()
{
	OSStartTimer(5, nil) ;
	JumpNext() ;
}

static void PStopTimer()
{
	OSStopTimer();
	JumpNext() ;
}

static void PHello()
{
	Mesg("HELLO");
	X0 = X1 = X2 = X3 = X4 = X5 = X6 = nil;
	JumpNext() ;
}

void CallPrologInit()
{
	InstallCBuiltinPred("abort", 0, PAbort) ;
	InstallCBuiltinPred("restart", 0, PAbort) ;
	InstallCBuiltinPred("exit", 0, PExit) ;
	InstallCBuiltinPred("halt", 0, PHalt) ;
	InstallCBuiltinPred("$$_exit_script_fast", 0, PExitScriptFast) ;
	InstallCBuiltinPred("copyright", 0, PCopyright) ;
	InstallCBuiltinPred("$$_more_success", 0, PMoreSuccess) ;
	InstallCBuiltinPred("$$_return_success", 0, PReturnSuccess) ;
	InstallCBuiltinPred("$$_write_exception", 0, PWriteException) ;
	InstallCBuiltinPred("$$_return_failure", 0, PReturnFailure) ;
	InstallCBuiltinPred("$$_level", 1, PLevel) ;
	InstallCBuiltinPred("set_result", 1, PSetResult) ;
	InstallCBuiltinPred("$$_call_prolog_through_c", 1, PCallPrologThroughC) ;
	InstallCBuiltinPred("$$_fatal_error", 1, PFatalError) ;
	InstallCBuiltinPred("$$_call_prolog_test", 0, PCallPrologTest) ;
	InstallCBuiltinPred("$$_sizes", 0, PSizes) ;
	InstallCBuiltinPred("$$_crash", 0, PCrash) ;
	InstallCBuiltinPred("$$_start_timer", 0, PStartTimer) ;
	InstallCBuiltinPred("$$_stop_timer", 0, PStopTimer) ;
	InstallCBuiltinPred("$$_hello", 0, PHello) ;
}

/* http://docs.python.org/c-api/init.html */
