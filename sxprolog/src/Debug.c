/*
 *   This file is part of the CxProlog system

 *   Debug.c
 *   by A.Miguel Dias - 2000/05/05
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

#define IsDebugCP(b)	( *(b)->P == DebugRetry )

typedef enum { eError, eOff, eCall, eNext, eFail, eExit, eRedo, eCut } DebugEvent ;
typedef enum { mIgnore, mCreep, mSkip, mQuasiSkip, mLeap, mInterrupt } DebugMode ;

typedef struct DebugFrame { /* LIVES ON LOCAL STACK */
	Pt exitInst ;   /* first */
	Pt redoInst ;   /* second */
	Pt retryInst ;  /* third */
	Pt /*DebugEvent*/ event ;	/* Beware of garbage collection */
	Pt /*Char*/ type ;
	Pt lineN ;
	PredicatePt pred ;
	Hdl callCP ;
	Pt callC ;
	Pt callCH ;
	Hdl redoTR ; /* can move */
	Hdl redoP ;
	Pt frameN ;
	Pt exceptionToCatch ;
	Pt exceptionHandler ;
	ClausePt currClause ;
	struct DebugFrame *father ;
} DebugFrame, *DebugFramePt ;

#define cDebugFramePt(p)		((DebugFramePt)(p))

static DebugEvent topLeash = eNext ;
static DebugFramePt DD ;
static DebugMode mode ;
static Pt skippingTo ;
static Size DDN ;
static Bool interruptActivated = false ;
static Size lineN ;

static void DebugReset()
{
	DD = nil ;
	DDN = 1 ;
	lineN = 1 ;
	mode = mIgnore ;
}

static void RelocateDebugger(Size globalOffset, Size localOffset)
{
	Unused(globalOffset) ;
	if( DD != nil )
		DD = cVoidPt(cPt(DD) + localOffset) ;
	if( localOffset != 0 && IsLocalRef(P) )
		P = cVoidPt(cPt(P) + localOffset) ;
	if( localOffset != 0 && IsLocalRef(CP) ) {
		CP = cVoidPt(cPt(CP) + localOffset) ;
	}
}

static ChoicePointPt DF2CP(DebugFramePt d)
{
	int arity = PredArity(d->pred) ;
/* Make space for special buffer needed by c-nondeterm predicates */
	if( PredIsCNonDeterm(d->pred) ) arity = PredCNonDetermAMem(d->pred) ;
	elif( PredIsLogical(d->pred) ) arity++ ;
/* This is the choice point that cover the debug frame */
	return cChoicePointPt(cPt(d) - arity) - 1 ;
}

static DebugFramePt CP2DF(ChoicePointPt b)
{
	return IsDebugCP(b) ? cDebugFramePt(b->P - 2) : nil ;
}

static Str EventName(DebugEvent e)
{
	switch( e ) {
		case eCall: return "call" ;
		case eNext: return "next" ;
		case eFail: return "fail" ;
		case eExit: return "exit" ;
		case eRedo: return "redo" ;
		case eCut:  return " cut" ;
		default: return InternalError("EventName") ;
	}
}

static DebugEvent StrToTopLeash(Str s)
{
	switch( s[0] ) {
		case 'o': if( StrEqual(s, "off") ) return eOff ; else break ;
		case 'l': if( StrEqual(s, "loose") ) return eCall ; else break ;
		case 'h': if( StrEqual(s, "half") ) return eNext ; else break ;
		case 't': if( StrEqual(s, "tight") ) return eFail ; else break ;
		case 'f': if( StrEqual(s, "full") ) return eExit ; else break ;
	}
	return eError ;
}

static Str TopLeashToStr(DebugEvent e)
{
	switch( e ) {
		case eError: return "error" ;
		case eOff: return "off" ;
		case eCall: return "loose (call)" ;
		case eNext: return "half (call, next)" ;
		case eFail: return "tight (call, next, fail)" ;
		case eExit: return "full (call, next, fail, exit)" ;
		default: return InternalError("WriteLeash") ;
	}
}

static void Spaces(int n)
{
	n = n % 50 + 4 ;
	while( n-- )
		WriteStd(" ") ;
}

static Str GoalAsStr(DebugFramePt d)
{
	Str s ;
	HSave() ;
	s = TermAsStr(MakeStruct(PredFunctor(d->pred), cHdl(DF2CP(d) + 1))) ;
	HRestore() ;
	return s ;
}

static Bool FindDebugFrame(int n, DebugFramePt *res)
{
	register ChoicePointPt b ;
	DebugFramePt d ;
	Pt fn = MakeInt(n) ;
	for( b = B ; !IsEndOfChain(b) ; b = b->B )
		if( (d = CP2DF(b)) != nil && d->frameN == fn ) {
			*res = d ;
			return true ;
		}
	WriteStd("%% No such box.\n") ;
	return false ;
}

static Char CountRemainingClauses(DebugFramePt d)
{
	register ClausePt c ;
	register int n ;
	if( XInt(d->type) != '#' ) return (Char)XInt(d->type) ;
	n = 0 ;
	doseq(c, d->currClause, ClauseNext(c)) {
		if( PredIsLogical(d->pred) ) {
			if( ClauseIsAlive2(c, A(PredArity(d->pred))) ) n++ ;
		}
		else n++ ;
		if( n == 10 ) break ;
	}
	return n <= 9 ? '0' + n : (Char)XInt(d->type) ;
}

static void WriteGoalInfo(DebugFramePt d)
{
	WriteStd("     GOAL: %s\n", GoalAsStr(d)) ;
	WriteStd("     PRED: %s ", PredNameArity(d->pred)) ;
	if( XInt(d->type) == 'b' ) WriteStd("{built_in}\n") ;
	elif( XInt(d->type) == 'i' ) WriteStd("{imported from unit '%s'}\n",
									TermAsStr(GetImportTerm(d->pred))) ;
	elif( XInt(d->type) == 'u' ) WriteStd("{undefined in unit '%s'}\n",
									UnitSignature(CurrUnit())) ;
	else WriteStd("{has %ld clause%s}\n",
									PredLength(d->pred),
									PredLength(d->pred) == 1 ? "" : "s") ;
	WriteStd("  CONTEXT: %s\n", TermAsStr(d->callC)) ;
	WriteStd(" HCONTEXT: %s\n", TermAsStr(d->callCH)) ;
}

static Size NumberOfAncestors(DebugFramePt d)
{
	register Size n ;
	for( n = 0, d = d->father ; d != nil ; d = d->father )
		n++ ;
	return n ;
}

static void WriteAncestors(DebugFramePt d, int m)
{
	int n ;
	for( d = d->father, n = 0 ; d != nil && n < m ; d = d->father, n++ )
		WriteStd("(%3ld) %s\n", XInt(d->frameN), GoalAsStr(d)) ;
	if( n == 0 ) WriteStd("No ancestors.\n") ;
}

Pt BuildStackTrace() /* for catch/4 */
{
	DebugFramePt d ;
	Pt list = tNilAtom ;
	Hdl h = &list + 1 ;
	for( d = DD ; d != nil ; d = d->father ) {
		h[-1] = MakeList(MakeStruct(PredFunctor(d->pred), cHdl(DF2CP(d) + 1)),
							tNilAtom) ;
		h = H ;
	}
	return list ;
}

static Bool DebugLine(DebugEvent e, DebugFramePt d, Bool forceLeash, Bool returned)
{
	d->lineN = MakeInt(++lineN) ;
	Spaces(NumberOfAncestors(d)) ;
	WriteStd("%c%c%c (%3ld) %s: ",
		PredIsSpy(d->pred) ? '*' : ' ',
		returned ? '>' : ' ',
		CountRemainingClauses(d),
		XInt(d->frameN),
		EventName(e)
	) ;
	WriteStd("%s: ", TermAsStr(d->callC)) ;
	WriteStd("%s", GoalAsStr(d)) ;
	if( (int)e <= (int)topLeash || forceLeash || PredIsSpy(d->pred) )
		{ WriteStd(" ? ") ; return true ; }
	else { WriteStd("\n") ; return false ; }
}

static Bool InterDebug(DebugEvent e, DebugFramePt d)
{
	int arg ;
	Bool forceLeash, returned ;

	if( debug_flag == 0 ) return false ;
	d->event = MakeInt(e) ;
	switch( mode ) {
		case mIgnore: {
			if( debug_flag == 1 && !PredIsSpy(d->pred) ) return false ;
			forceLeash = returned = false ;
			break ;
		}
		case mCreep: {
			forceLeash = returned = false ;
			break ;
		}
		case mSkip: {
			if( d->frameN != skippingTo ) return false ;
			forceLeash = returned = true ;
			break ;
		}
		case mQuasiSkip: {
			if( d->frameN != skippingTo && !PredIsSpy(d->pred) ) return false ;
			forceLeash = returned = true ;
			break ;
		}
		case mLeap: {
			if( !PredIsSpy(d->pred) ) return false ;
			forceLeash = returned = true ;
			break ;
		}
		case mInterrupt: {
			forceLeash = false ;		/* Interrupt forces leashing */
			returned = false ;
			break ;
		}
		default:
			return IInternalError("InterDebug") ;
	}
	mode = mCreep ;
	for(;;) {
		if( !DebugLine(e, d, forceLeash, returned) ) return false ;
		switch( InterLineGetCommand(nil, &arg) ) {
			case '\n': {				/* creep */
				return false ;
			}
			case 'n': {					/* nodebug */
				DebugUpdateFlag(0) ;
				return false ;
			}
			case 'd': {					/* debug on */
				DebugUpdateFlag(1) ;
				mode = mIgnore ;
				return false ;
			}
			case 't': {					/* trace on */
				DebugUpdateFlag(2) ;
				mode = mIgnore ;
				return false ;
			}
			case 'a': {					/* abort */
				WriteStd("%% Aborted.\n") ;
				EventAbort() ;
				return true ;
			}
			case EOF: {
				WriteEOF() ;
				/* fall through */
			}
			case 'e': {					/* exit */
				WriteStd("%% Bye.\n") ;
				EventExit() ;
				return true ;
			}
			case 'l': {					/* leap */
				mode = mLeap ;
				return false ;
			}
			case 's': {					/* skip */
				if( arg >= 0 && !FindDebugFrame(arg, &d) ) break ;
				mode = mSkip ;
				skippingTo = d->frameN ;
				return false ;
			}
			case 'q': {					/* quasi-skip */
				if( arg >= 0 && !FindDebugFrame(arg, &d) ) break ;
				mode = mQuasiSkip ;
				skippingTo = d->frameN ;
				return false ;
			}
			case 'r': {					/* retry */
				if( arg >= 0 && !FindDebugFrame(arg, &d) ) break ;
				SetChoicePoint(DF2CP(d)) ;
				RestoreState(PredArity(d->pred)) ;
				DD = d->father ;
				DDN = XInt(d->frameN) ;
				SetChoicePoint(Bf(B)) ;	/* Discard */
				if( !DebugCall(d->pred) )
					InternalError("InterDebug") ;
				return true ;
			}
			case 'f': {					/* fail */
				if( arg >= 0 && !FindDebugFrame(arg, &d) ) break ;
				SetChoicePoint(DF2CP(d)) ;
				d->currClause = nil ;
				P = FailAddr ;		/* Fail */
				return true ;
			}
			case 'i': {					/* info */
				WriteGoalInfo(d) ;
				break ;
			}
			case 'g': {					/* ancestors */
				WriteAncestors(d, arg < 0 ? 1000 : arg ) ;
				break ;
			}
			case ':': {					/* statistics */
				StatisticsShow() ;
				break ;
			}
			case '=': {					/* debugging */
				Debugging() ;
				break ;
			}
			case '+': {					/* spy this */
				SpyOn(PredFunctor(d->pred)) ;
				break ;
			}
			case '-': {					/* nospy this */
				SpyOff(PredFunctor(d->pred)) ;
				break ;
			}
			default: {
				WriteStd("\
CxProlog debug options:\n\
      <ret> creep              + spy this             h help\n\
       s<i> skip               - nospy this           d debug\n\
       q<i> quasi-skip         i info                 t trace\n\
       l leap                  g<n> ancestors         n nodebug\n\
       r<i> retry              = debugging            a abort\n\
       f<i> fail               : statistics           e exit\n") ;
/* b   break     !@ command */
				break ;
			}
		}
	}
}

/* CP[-1] remains undefined across every predicate activated in
   debug mode. However this is not a problem as the predicate debug
   choice point always covers the current environment.  */

Bool DebugCall(PredicatePt pr)
{
	register DebugFramePt d ;
	ChoicePointPt saveB ;

	if( debug_flag == 0 ) return false ;
	Attention() = true ;	/* keep attention alive */

	if( !PredIsTraceable(pr) || CallLevel() > 1 ) return false ;

	if( DebugCallOverride(pr) )
		return false ;

	if( PredIsBuiltin(pr) )
		if( DD == nil || (XInt(DD->type) == 'b' && !PredIsMeta(DD->pred)) )
			return false ;

#if 0
debug_flag == 0	Mesg("%d", DDN) ;
#endif

	d = cDebugFramePt(TopOfLocalStack()) - 1 ;	/* push debug frame */
	d->exitInst = DebugExit ;
	d->redoInst = DebugRedo ;
	d->retryInst = DebugRetry ;
	d->pred = pr ;
	d->callCP = CP ;
	d->callC = C ;
	d->callCH = CH ;
	d->lineN = zeroIntPt ;
	d->currClause = PredClauses(pr) ;
	if( PredIsLogical(pr) )
		for( ; d->currClause != nil ; d->currClause = ClauseNext(d->currClause) )
			if( ClauseIsAlive(d->currClause) ) break ;
	d->frameN = MakeInt(DDN++) ;
	d->exceptionToCatch = nil ;
	d->exceptionHandler = nil ;
	d->type = MakeInt(
			  PredIsBuiltin(pr) ? 'b'
			: PredIsImported(pr) ? 'i'
			: PredIsUndefined(pr) ? 'u'
			: '#') ;
	d->father = DD ;
	DD = d ;

	saveB = B ;
	B = DF2CP(d) ; /* New ChoicePoint covers DebugFrame */
		/* B0 not changed on purpose */
	SaveState(saveB, &(d->retryInst), PredArity(pr)) ;
/* Initializes the special buffer needed by c-nondeterm predicates */
	if( PredIsCNonDeterm(pr) ) A(PredArity(pr)) = tNilAtom ;
	if( PredIsLogical(pr) ) A(PredArity(pr)) = GlobalClock ;

	if( InterDebug(eCall, d) ) return true ;
	CP = &(d->exitInst) ;
	P = d->currClause == nil
			? 
		PredCode(pr)	/* c-determ, undef, empty, import */
			: ClauseCodeSkipHeader(d->currClause) ; /* normal, c-nondeterm */

	return true ;
}

Bool DebugCut(ChoicePointPt cp)
{
	DebugFramePt d ;
	ChoicePointPt b ;
	if( DD == nil ) return false ;
	for( b = B ; Lt(b, cp) ; b = b->B ) {
		if( (d = CP2DF(b)) != nil )
			d->currClause = nil ;
	}
	InterDebug(eCut, DD) ;
	return true ;
}

Bool DebugFail()
{	/* Experimental, not used */
	if( DD == nil ) return false ;
	DD->currClause = nil ;
	P = &(DD->retryInst) ;
	return true ;
}

void DebugThrow()
{
	ChoicePointPt b ;
	DebugFramePt d ;
	if( DD == nil ) return ;
	for( b = B ; !IsEndOfChain(b) ; b = b->B )
		if( (d = CP2DF(b)) != nil ) {
			DD = d ;
			mode = mIgnore ;
			return ;
		}
	DD = nil ;
}

void DebugExitCode()
{
	DebugFramePt d = cDebugFramePt(P - 1) ;	/* not top */
	if( InterDebug(eExit, d) ) return ;
	d->redoTR = TR ;
	d->redoP = Bf(P) ;
	Bf(P) = &(d->redoInst) ;
	P = CP = d->callCP ;	/* Proceed */
	DD = d->father ;
}

void DebugRedoCode()
{
	DebugFramePt d = cDebugFramePt(P - 2) ;	/* not top */
	TrailRestore(d->redoTR) ; /* resume the instantiation that was at the exit port */
	Bf(P) = d->redoP ;
	P = FailAddr ;			/* Fail */
	InterDebug(eRedo, d) ;
}

void DebugRetryCode()
{
	DebugFramePt d = cDebugFramePt(P - 3) ;	/* top */
	RestoreState(PredArity(d->pred)) ;
	if( d->currClause != nil ) {
		d->currClause = ClauseNext(d->currClause) ;
		if( PredIsLogical(d->pred) )
			for( ; d->currClause != nil ; d->currClause = ClauseNext(d->currClause) )
				if( ClauseIsAlive2(d->currClause, A(PredArity(d->pred))) ) break ;
	}
	if( d->currClause == nil ) {
		if( InterDebug(eFail, d) ) return ;
		SetChoicePoint(Bf(B)) ;	/* Discard */
		P = FailAddr ;				/* Fail */
		DD = nil ;
	}
	else {
		if( (XInt(d->event) == eCall
		 || XInt(d->event) == eNext) && XInt(d->lineN) == lineN )
			/* nothing done: hides shallow backtracking */ ;
		elif( InterDebug(eNext, d) ) return ;
		CP = &(d->exitInst) ;
		P = ClauseCodeSkipHeader(d->currClause) ;
		DD = d ;
	}
}

void DebugUpdateFlag(int newValue)
{
	int oldValue = debug_flag ;
	debug_flag = newValue ;
	if( oldValue != newValue ) {
		if( newValue == 0 )
			if( oldValue == 1 ) WriteStd(" [debug mode off] \n") ;
			else WriteStd(" [trace mode off] \n") ;
		elif( newValue == 1 ) WriteStd(" [debug mode on] \n") ;
		else WriteStd(" [trace mode on] \n") ;
	}
	if( (oldValue == 0) != (newValue == 0) ) {
		if( newValue != 0 )
			Attention() = true ;
		DebugReset() ;
	}
}

void DebugInterrupt(int newValue) /* Called from InterruptHandle() */
{
	if( newValue != debug_flag ) {
		if( debug_flag == 0 && newValue > 0 )
			interruptActivated = true ;
		DebugUpdateFlag(newValue) ;
		mode = mIgnore ;
	}
}

void DebugRestart()		/* Activated on cxprolog restart */
{
	DebugReset() ;
	if( interruptActivated ) {	/* Debug off, if interrupt active */
		DebugUpdateFlag(0) ;
		interruptActivated = false ;
	}
}

void Debugging()
{
	ShowVersion() ;
	if( debug_flag == 0 ) {
		WriteStd("%% Debug mode is off.\n") ;
	}
	else {
		WriteStd("%% Debug mode is on.\n") ;
		WriteSpyPoints() ;
		WriteStd("%% Leashing set to %s.\n", TopLeashToStr(topLeash)) ;
	}
}


/* CXPROLOG C'BUILTINS */

static void PDebugging()
{
	Debugging() ;
	JumpNext() ;
}

static void PLeash()
{
	DebugEvent e ;
	if( (e = StrToTopLeash(XTestAtomName(X0))) == eError )
		Error("Invalid leash specification") ;
	topLeash = e ;
	WriteStd("Leashing set to %s.\n", TopLeashToStr(topLeash)) ;
	JumpNext() ;
}

static void PSpy()
{
	ForEachInSpec(X0, SpyOn, true) ;
	if( debug_flag == 0 )
		DebugUpdateFlag(1) ;
	JumpNext() ;
}

static void PNoSpy()
{
	ForEachInSpec(X0, SpyOff, true) ;
	JumpNext() ;
}

static void PNoSpyAll()
{
	NoSpyAll() ;
	JumpNext() ;
}

void DebugInit()
{
	InstallRelocateStacksHandler("Debugger", RelocateDebugger) ;

	DebugReset() ;
	InstallCBuiltinPred("debugging", 0, PDebugging) ;
	InstallCBuiltinPred("leash", 1, PLeash) ;
	InstallCBuiltinPred("spy", 1, PSpy) ;
	InstallCBuiltinPred("nospy", 1, PNoSpy) ;
	InstallCBuiltinPred("nospyall", 0, PNoSpyAll) ;

	PredIsTraceable(LookupPredicateByName(",", 2)) = false ;
}
