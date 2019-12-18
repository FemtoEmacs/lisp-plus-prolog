/*
 *   This file is part of the CxProlog system

 *   SysTrace.c
 *   by A.Miguel Dias - 2003/08/06
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

/* flag(sys_trace, _, 1). */


static StreamPt sysTraceStream = nil ;

/* TryOpen is to handle the situation "/tmp/sys_trace" already exists
   and belong to a different user */

static StreamPt TryOpen(Str fName)
{
	Str fn = WithTmpDirPath(fName, "log") ;
	if( OSExists(fn) && !OSPropWritable(fn) ) {
		Warning("Could not open '%s'", fn) ;
		return nil ;
	}
	else {
		StreamPt srm = FileStreamOpen(fn, mWrite, nil) ;
		ExtraPermanent(srm) ;
		FileAllowsNumericEscapes(StreamChannel(srm)) ;
		AliasSet(MakeAtom("$$_sys_trace"), srm) ;
		return srm ;
	}
}

static void EnsureFileIsOpen(void)
{
	if( sysTraceStream == nil ) {
#if 1
		if( (sysTraceStream = TryOpen("sys_trace")) != nil ) ;
		elif( (sysTraceStream = TryOpen("sys_trace1")) != nil ) ;
		elif( (sysTraceStream = TryOpen("sys_trace2")) != nil ) ;
		else
#endif
			sysTraceStream = userOut ;

		infoMessages_flag = 3 ;
		Info(1, "The audit file is '%s'",
								AtomName(StreamPath(sysTraceStream))) ;		
		StreamPt saveCurrOut = currOut ;
		currOut = sysTraceStream ;
		StreamPut(currOut, '\n') ;
		FeaturesShow() ;
		StreamPut(currOut, '\n') ;
		FlagsShow() ;
		StreamPut(currOut, '\n') ;
		StatisticsShow() ;
		StreamPut(currOut, '\n') ;
		currOut = saveCurrOut ;
	}
}

static void DeleteSysTraceFile(void) {
	if( sysTraceStream != nil && sysTraceStream != userOut ) {
		AtomPt path = StreamPath(sysTraceStream) ;
		ExtraNotPermanent(sysTraceStream) ;
		StreamClose(sysTraceStream, nil) ;
		sysTraceStream = nil ;
		FSDelete(AtomName(path)) ;
		SysTraceUpdateFlag(0) ;
	}
}

void SysTraceMachineRun()
{
#if USE_THREADED_CODE
	if( sysTrace_flag == 4 ) {
		Warning("Threaded code is active, so sys_trace level was reduced to 3") ;
		SysTraceUpdateFlag(3) ;
	}
#else
	if( sysTrace_flag == 4 && sysTraceStream != nil )
		for(;;) {
			DisassembleOneInst(sysTraceStream, P) ;
			StreamFlush(sysTraceStream) ;
			InstRun() ;
		}
#endif
}

void SysTraceUpdateFlag(int newValue)
{
	int oldValue = sysTrace_flag ;
	if( sysTraceDebugging_flag > 0 )
		newValue = Max(sysTraceDebugging_flag, newValue) ;
	sysTrace_flag = newValue ;
    if( sysTrace_flag > 0 )
        EnsureFileIsOpen() ;	
	if( sysTrace_flag > 1 ) {
		Attention() = true ;
		if( sysTrace_flag == 4 && Running() )
			EventContinue() ;
	}
	if( sysTrace_flag < 4 && oldValue == 4 && Running() ) {
		Attention() = true ;
		EventContinue() ;
	}
}

void SysTraceHandle(PredicatePt pr)
{
	if( sysTrace_flag > 1 ) {
		Str s ;
		Attention() = true ;
		if( sysTrace_flag == 2 && (!PredIsBuiltin(pr) || PredIsMeta(pr)) ) return ;
		HSave() ;
		s = TermAsStr(MakeStruct(PredFunctor(pr), X)) ;
		HRestore() ;
		StreamPutStrNl(sysTraceStream, s) ;
		StreamFlush(sysTraceStream) ;
	}
}

void SysTraceWrite(Str s)
{
	if( sysTrace_flag > 0 && sysTraceStream != nil && sysTraceStream != currOut ) {
		StreamPutStr(sysTraceStream, s) ;
		StreamFlush(sysTraceStream) ;
	}
}


/* CXPROLOG C'BUILTINS */

static void PDeleteSysTraceFile()
{
	DeleteSysTraceFile() ;
	JumpNext() ;
}


void SysTraceInit()
{
/* These functors are made META only to reduce the output when sys_trace == 1.
   Note it is safe to do this only for zeroary functors. */
	FunctorIsMeta(LookupFunctorByName("fail", 0)) = true ;
	FunctorIsMeta(LookupFunctorByName("false", 0)) = true ;
	FunctorIsMeta(LookupFunctorByName("true", 0)) = true ;

	InstallCBuiltinPred("$$_delete_sys_trace_file", 0, PDeleteSysTraceFile) ;
}
