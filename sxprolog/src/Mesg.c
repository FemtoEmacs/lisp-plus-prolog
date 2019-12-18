/*
 *   This file is part of the CxProlog system
 *   Mesg.c

 *   by A.Miguel Dias - 2000/04/02
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

typedef enum {
	instantiationErr, typeErr, domainErr, existenceErr,
	permissionErr, representationErr,  evaluationErr,
	resourceErr,  syntaxErr,  systemErr, genErr
} ErrorKind ;

Pt catchTerm ;
jmp_buf catchJB;
Bool catchActive = false;


/*
Pt Catch(Bool on) {
	catchActive = on ;
	return on && setjmp(jb) != 0 ? jbTerm : nil ;
}
* */

static void ErrorMesgV(Str kind, Bool showPred, Str fmt, va_list v)
{	/* The memory manager may have not been initialized yet.
	   Or this maight be called when there is no memory available.
	   So, we cannot relly on GStrFormat here. */
	Str1K newFmt ;
	PredicatePt pr ;

	if( fmt == nil ) fmt = "(null)" ;

	strcpy(newFmt, "{") ;
	if( kind != nil )
		strcat(newFmt, kind) ;
	if( showPred && (pr = CurrCPred()) != nil )
		sprintf(newFmt + strlen(newFmt), " (%s)", PredNameArity(pr)) ;
	if( strlen(newFmt) > 1 )
		strcat(newFmt, ": ") ;
	strcat(newFmt, fmt) ;
#if 0
	if( errno != 0 ) {
		sprintf(newFmt + strlen(newFmt), " (%s)", sys_errlist[errno]) ;
		errno = 0 ;
	}
#endif
	strcat(newFmt, ".}\n") ;
	StreamWriteV(userErr, newFmt, v) ;
}

Pt ErrorEventPrepareV(Str errKind, Str op, Str variant, Pt culprit,
												Str fmt, va_list v)
{
	ScratchRestart() ;
	EncodingsRestart() ;
	switch( catchActive ? 0 : onError_flag ) {
		case 0: {
			if( Booting() )
				ErrorMesgV("ERROR DURING BOOT", true, fmt, v) ;
			return Booting() ? nil : BuildExceptionTermV(errKind,
											op, variant, culprit, fmt, v) ;
		}
		case 1: {
			ErrorMesgV(errKind, true, fmt, v) ;
			return nil ;
		}
		case 2: {
			return nil ;
		}
		case 3: {
			return BuildExceptionTermV(errKind, nil, variant, culprit, fmt, v) ;
		}
		default:
			InternalError("ErrorEventPrepareV") ;
	}
	return nil ;
}

VoidPt ErrorEventTrigger(Pt exc)
{
	if( catchActive ) {
		catchTerm = exc ;
		longjmp(catchJB, 1) ;
	}
	switch( onError_flag ) {
		case 0: {
			if( Booting() )
				EventFatalError() ;
			else Throw(exc) ;
			break ;
		}
		case 1: {
			EventForceFail() ;
			break ;
		}
		case 2: {
			EventForceFail() ;
			break ;
		}
		case 3: {
			PredicatePt onError =
				LookupPredicateForMetaCall(LookupFunctorByName("on_error", 1)) ;
			X0 = exc ;
			P = PredCode(onError) ;
			EventContinue() ;
			break ;
		}
		default: InternalError("ErrorEventTrigger") ;
	}
	return nil ;
}

void Mesg(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	ErrorMesgV(nil, false, fmt, v) ;
	va_end(v) ;
}

void MesgP(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	vfprintf(stderr, fmt, v) ;
	fprintf(stderr, "\n") ;
	va_end(v) ;
}

void MesgW(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	ErrorMesgV(nil, false, fmt, v) ;
	va_end(v) ;
	getchar() ;
}

void MesgT(Pt t)
{
	if( t == nil )
		Mesg("NIL") ;
	else
		Mesg("%s", TermAsStr(t)) ;
}

void BasicInfo(Str fmt, ...)
{
	if( infoMessages_flag >= 0 ) {
		va_list v ;
		va_start(v, fmt) ;
		WriteErr("%% ") ;
		StreamWriteV(userErr, fmt, v) ;
		WriteErr(".\n") ;
		va_end(v) ;
	}
}

void Info(int level, Str fmt, ...)
{
	if( infoMessages_flag >= level ) {
		va_list v ;
		va_start(v, fmt) ;
		ErrorMesgV("INFO", false, fmt, v) ;
		va_end(v) ;
	}
}

void MemoryInfo(Str fmt, ...)
{
	InterruptHandle() ;
	if( memoryWarnings_flag || infoMessages_flag >= 2 ) {
		va_list v ;
		va_start(v, fmt) ;
		ErrorMesgV("MEMORY", false, fmt, v) ;
		va_end(v) ;
	}
}

void MemoryGrowInfo(Str what, Size oldSize, Size newSize)
{
	if( memoryWarnings_flag || infoMessages_flag >= 2 ) {
		Str32 o, n ;
		if( oldSize < 1 K )
			sprintf(o, "%ld bytes", WordsAsBytes(oldSize)) ;
		else sprintf(o, "%ldKB", WordsAsKBytes(oldSize)) ;
		if( newSize < 1 K )
			sprintf(n, "%ld bytes", WordsAsBytes(newSize)) ;
		else sprintf(n, "%ldKB", WordsAsKBytes(newSize)) ;
		if( !Running() )
			MemoryInfo("Expanding %s from %s to %s", what, o, n) ;
		else
			MemoryInfo("Expanding %s from %s to %s [@%s]", what, o, n, GetInstNameSearch(P-1)) ;
	}
}

void Warning(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	ErrorMesgV("WARNING", false, fmt, v) ;
	va_end(v) ;
}

static VoidPt InstantiationError(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	if( isoErrors_flag )
		exc = ErrorEventPrepareV("instantiation_error", nil, nil, nil, fmt, v) ;
	else
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt EvaluationError(Str variant, Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	if( isoErrors_flag )
		exc = ErrorEventPrepareV("evaluation_error", nil, variant, nil, fmt, v) ;
	else {
		Unused(variant) ;
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	}
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt ExistenceError(Str variant, Pt culprit, Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	if( isoErrors_flag )
		exc = ErrorEventPrepareV("existence_error", nil, variant, culprit, fmt, v) ;
	else {
		Unused(variant) ;
		Unused(culprit) ;
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	}
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt PermissionError(Str op, Str objectType, Pt culprit, Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	if( isoErrors_flag )
		exc = ErrorEventPrepareV("permission_error", op, objectType, culprit, fmt, v) ;
	else {
		Unused(objectType) ;
		Unused(culprit) ;
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	}
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

static VoidPt TypeErrorAux(Str errKind, Str expected, Pt culprit, Str fmt, ...)
	{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	if( isoErrors_flag ) {
		Str simpleExpected ;
		if  ( StrEqual(expected, "ATOM") ) simpleExpected = "atom" ;
		elif( StrEqual(expected, "ATOM or VAR") ) simpleExpected = "atom" ;
		elif( StrEqual(expected, "ATOMIC") ) simpleExpected = "atomic" ;
		elif( StrEqual(expected, "STREAM or FILENAME") ) simpleExpected = "atom" ;
		elif( StrEqual(expected, "LIST") ) simpleExpected = "list" ;
		elif( StrEqual(expected, "CHAR") ) simpleExpected = "character" ;
		elif( StrEqual(expected, "INT") ) simpleExpected = "integer" ;
		elif( StrEqual(expected, "INT or VAR") ) simpleExpected = "integer" ;
		elif( StrEqual(expected, "INT>=0 or VAR") ) simpleExpected = "not_less_than_zero" ;
		elif( StrEqual(expected, "INT>=0") ) simpleExpected = "not_less_than_zero" ;
		elif( StrEqual(expected, "INT<MAX_INT") ) simpleExpected = "less_than_max_int" ;
		elif( StrEqual(expected, "STRUCT or LIST") ) simpleExpected = "compound" ;
		elif( StrEqual(expected, "NUMBER") ) simpleExpected = "number" ;
		elif( StrEqual(expected, "EVALUABLE") ) simpleExpected = "evaluable" ;
		elif( StrEqual(expected, "STRUCT, LIST or ATOM") ) simpleExpected = "callable" ;
		elif( StrEqual(expected, "CALLABLE") ) simpleExpected = "callable" ;
		elif( StrEqual(expected, "CODE") ) simpleExpected = "integer" ;
		elif( true ) simpleExpected = "what?" ;
		
		
		elif( StrEqual(expected, "EXTRA") ) simpleExpected = "extra" ;
		elif( StrEqual(expected, "VAR or ALIAS or IVAR") ) simpleExpected = "what?" ;
		elif( StrEqual(expected, "PROPERLY-TERMINATED-LIST") ) simpleExpected = "list" ;
		elif( StrEqual(expected, "UNIT-PARAMETER") ) simpleExpected = "unit?" ;
		elif( StrEqual(expected, "PROPERLY-TERMINATED-LIST") ) simpleExpected = "list" ;
	
		exc = ErrorEventPrepareV(errKind, nil, simpleExpected, culprit, fmt, v) ;
	}
	else
		exc = ErrorEventPrepareV("TYPE ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt TypeError(Str expected, Pt culprit)
{	
	if( isoErrors_flag ) {
		if( IsVar(culprit) )
			return InstantiationError("%s expected", expected) ;
	}
	if( culprit == nil )
		return TypeErrorAux("type_error", expected, culprit,
					"%s expected", expected) ;
	else
		return TypeErrorAux("type_error", expected, culprit,
				"%s expected, found '%s'", expected, TermAsStr(culprit)) ;
}

int ITypeError(Str expected, Pt culprit)
{
	TypeError(expected, culprit) ;
	return 0 ;
}

VoidPt TypeError2(Str expected, Pt culprit, Str fmt, ...)
{	
	va_list v ;
	va_start(v, fmt) ;
	Str mesg = GStrFormatV(fmt, v) ;
	va_end(v) ;
	if( isoErrors_flag ) {
		if( IsVar(culprit) )
			return InstantiationError("%s", mesg) ;
	}
	return TypeErrorAux("type_error", expected, culprit, mesg) ;
}

VoidPt DomainError(Str expected, Pt culprit)
{	
	if( culprit == nil )
		return TypeErrorAux("domain_error", expected, culprit,
					"%s expected", expected) ;
	else
		return TypeErrorAux("domain_error", expected, culprit,
					"%s expected, found '%s'", expected, TermAsStr(culprit)) ;
}

VoidPt RepresentationError(Str variant, Pt culprit, Str fmt, ...)
{	
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	if( isoErrors_flag ) {
		Unused(culprit) ;
		exc = ErrorEventPrepareV("representation_error", nil, variant, nil, fmt, v) ;
	}
	else
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}


VoidPt SyntaxError(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV("SYNTAX ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt Error(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

int IError(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	ErrorEventTrigger(exc) ;
	return 0 ;
}

VoidPt GenericError(Str kind, Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV(kind, nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt ArithError(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt FileError(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt DatabaseError(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV("DATABASE ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt ImperativeError(Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	va_start(v, fmt) ;
	exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

VoidPt FatalError(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	ErrorMesgV("FATAL ERROR", true, fmt, v) ;
	va_end(v) ;
	EventFatalError() ;
	return nil ;
}

VoidPt InternalError(Str fun)
{
	Str256 fmt ;
	sprintf(fmt, "At function %s", fun) ;
	ErrorMesgV("INTERNAL ERROR", true, fmt, nil) ;
	EventFatalError() ;
	return nil ;
}

VoidPt UndisclosedError(Str s)
{
	ErrorMesgV("UNDISCLOSED FEATURE", false, s, nil) ;
	EventFatalError() ;
	return nil ;
}

int IInternalError(Str fun)
{
	InternalError(fun) ;
	return 0 ;
}

void InfoMessagesUpdateFlag(int newValue)
{
	if( infoMessagesDebugging_flag > 0 )
		newValue = Max(infoMessagesDebugging_flag, newValue) ;
	infoMessages_flag = newValue ;
}


/* CXPROLOG C'BUILTINS */

static void PWarning()
{
	Str s ;
	X0 = Drf(X0) ;
	s = IsList(X0) ? TermsAsStrQ(X0) : TermAsStrQ(X0) ;
	Warning("%s", s) ;
	JumpNext() ;
}

static void PError()
{
	Str s ;
	X0 = Drf(X0) ;
	s = IsList(X0) ? TermsAsStrQ(X0) : TermAsStrQ(X0) ;
	Error("%s", s) ;
	JumpNext() ;
}

static void PInfo()
{
	Str s ;
	X0 = Drf(X0) ;
	s = IsList(X0) ? TermsAsStrN(X0) : TermAsStrN(X0) ;
	Info(1, s) ;
	JumpNext() ;
}

void MesgInit()
{
	InstallCBuiltinPred("warning", 1, PWarning) ;
	InstallCBuiltinPred("error", 1, PError) ;
	InstallCBuiltinPred("info", 1, PInfo) ;
}
