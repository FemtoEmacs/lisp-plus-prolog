/*
 *   This file is part of the CxProlog system

 *   InterLine.c
 *   by A.Miguel Dias - 2007/09/01
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


/* PROMPTS */

#define AtTopLevelRead()		(topLinePrompt != nil)

static Bool interLineActive = false ;
static Str linePrompt, savelinePrompt, topMainPrompt, topLinePrompt ;

static Str InterLineGetLinePrompt(void)
{
    return linePrompt == nil ? "|: " : linePrompt ;
}

Str InterLineGetPrompt(void)
{
	if( linePrompt != nil )
		return InterLineGetLinePrompt() ;
	elif( topMainPrompt != nil ) {
		Str prompt = topMainPrompt ;
		topMainPrompt = nil ;
		return prompt ;
	}
	elif( topLinePrompt != nil )
		return topLinePrompt ;
	else
		return InterLineGetLinePrompt() ;
}

void InterLineBeginTop(Str main, Str line) {
//	InterLineRestart();
	topMainPrompt = main != nil ? main : "" ;
	topLinePrompt = line != nil ? line : "" ;
//	linePrompt = nil ;
//	InterLinePrepareForNextLine();
}

void InterLineEndTop() {
	topMainPrompt = nil ;
	topLinePrompt = nil ;
}

static void InterLineBeginLinePrompt(Str prompt)
{
#if 0
	Mesg("old %s new %s", linePrompt, prompt) ;
#endif
	savelinePrompt = linePrompt ;
    linePrompt = prompt != nil ? prompt : "" ;
}

static void InterLineEndLinePrompt(void)
{
#if 0
	Mesg("restore %s", savelinePrompt) ;
#endif
    linePrompt = savelinePrompt ;
}

static void InterLinePromptInit()
{
	interLineActive = true ;
	linePrompt = nil ;
	InterLineEndTop() ;

}


/* INTERACTIVE COMMAND LINE */

#if OS_UNIX && USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>

#if RL_READLINE_VERSION < 0x400
#	error "Required library: readline-4.0 or newer."
#endif

static CharPt line, linePt ;

static void r(void)
{
	line = readline(InterLineGetPrompt()) ;
}

static Bool InterLineMore(void)
{
	/* Not sure if locale should be changed. @@@ */
	RunProtected(r) ;
	if( line == nil )
		return false ;	/* EOF */
	linePt = AtTopLevelRead() ? ClearTermText(line, false) : line ;
	if( linePt[0] == '\0' || linePt[1] == '\0' || (linePt[2] == '\0' && linePt[1] != '.') )
		/* Does not insert in history */ ;
	else {
		Bool isRepetition = history_length > 0
					&& strcmp(linePt, history_get(history_length)->line) == 0 ;
		if( !isRepetition )
			add_history(linePt) ;
	}
	return true ;
}

void InterLineChangedUserStreams()
{
	if( StreamKind(userIn) == interactiveFileStream
		&& StreamKind(userOut) == textFileStream ) {
		rl_instream = StreamFILE(userIn) ;
		rl_outstream = StreamFILE(userOut) ;
	}
}

static void InterLinePrepareForNextLine(void)
{
	if( line != nil )
		free(line) ;
	line = nil ;
}

void InterLineFinish(void)
{
	if( interLineActive )		/* Save history for next session */
		write_history(WithPreferencesDirPath("history", nil)) ;
}

static void InterLineInitialize(void)
{
	line = nil ;
	InterLinePrepareForNextLine() ;
	rl_readline_name = "cxprolog" ;		/* For cond. parsing of "~/.inputrc". */
	rl_inhibit_completion = 1 ;			/* Disable completion */
	using_history() ;					/* Initialize history */
										/* Load history from prev. session */
	read_history(WithPreferencesDirPath("history", nil)) ;
}

WChar InterLineGet()
{
	if( line == nil && !InterLineMore() )
		return EOF ;
	else {
		WChar c = StrGetChar(&linePt, SystemEncoding()) ;
		if( c == '\0') {
			InterLinePrepareForNextLine() ;
			return '\n' ;
		}
		else
			return c ;
	}
}

WChar InterLinePeek()
{
	if( line == nil && !InterLineMore() )
		return EOF ;
	else {
		CharPt save = linePt ;
		WChar c = StrGetChar(&linePt, SystemEncoding()) ;
		linePt = save ;
		return c == '\0' ? '\n' : c ;
	}

}

Str InterLineVersion(void)
{
	return GStrFormat("lib=%x, inc=%x, loc=%d",
			rl_readline_version, RL_READLINE_VERSION, USE_READLINE) ;
}

#else


void InterLineChangedUserStreams()
{
	/* Nothing */
}

static void InterLinePrepareForNextLine(void)
{
	//cc = '\n' ;
}

void InterLineFinish(void)
{
	/* Do nothing */
}

static void InterLineInitialize(void)
{
	return ;
	InterLinePrepareForNextLine() ;
}

WChar InterLineGet()
{ 
	static WChar cc = '\n' ;
	if( cc == '\n' ) {
		StreamPutStr(userOut, InterLineGetPrompt()) ;
	// In case of remote usage...
		StreamFlush(userOut) ;
	}
	for(;;) {
		OSAllowInterruptibleSysCalls() ;
		cc = FileGetChar(StreamChannel(userIn)) ;
		OSDisallowInterruptibleSysCalls() ;
		if( cc < ' ' ) /* this works even with raw input or sockets */
			switch( cc ) {
				case EOF:
					if( InterruptHandle() ) continue ;
					return EOF ;
				case  0:
					continue ;
				case 10:
				case 13:
					return '\n' ;
				case  4: /* CNTL-D */
				case 26: /* CNTL-Z */
					do {	/* skip rest of line */
						cc = FileGetChar(StreamChannel(userIn)) ;
					} while( cc != '\n' ) ;
					return EOF ;
			}
		return cc ;
	}
}

WChar InterLinePeek()
{
	WChar c = FilePeekChar(StreamChannel(userIn))  ;
	if( c < ' ' ) /* this works even with raw input or sockets */
		switch( c ) {
			case 10:
			case 13:
				return '\n' ;
			case EOF:
			case  4: /* CNTL-D */
			case 26: /* CNTL-Z */
				return EOF ;
		}
	return c ;
}

Str InterLineVersion(void)
{
	return "none" ;
}
#endif

WChar InterLineGetCommand(Str prompt, int *arg)
{
	WChar c, res ;
	int n ;
	Bool hasArg ;
	InterruptOff() ;
	StreamFlush(userOut) ;
	StreamFlush(userErr) ;
	InterLinePrepareForNextLine() ;
	InterLineBeginLinePrompt(prompt) ;	
/* Get command */
	while( (c = InterLineGet()) <= ' ' && c != '\n' && c != EOF ) ;
	InterLineEndLinePrompt() ;	
	res = InRange(c,'A','Z') ? (c - 'A' + 'a') : c ;
/* Skip blanks */
	if( c != '\n' && c != EOF )
		while( (c = InterLineGet()) <= ' ' && c != '\n' && c != EOF ) ;
/* Read argument */
	hasArg = false ;
	n = 0 ;
	while( c != '\n' && c != EOF ) {
		if( InRange(c, '0', '9') ) {
			hasArg = true ;
			n = n * 10 + c - '0' ;
		}
		else {
			hasArg = false ;
			break ;
		}
		c = InterLineGet() ;
	}
	if( arg != nil )
		*arg = hasArg ? n : -1 ;
	while( c != '\n' && c != EOF )
		c = InterLineGet() ;
	InterruptOn() ;
	return res ;
}

void InterLineRestart()
{
	return ;
	InterLinePromptInit() ;
	InterLinePrepareForNextLine() ;
}


/* CXPROLOG C'BUILTINS */

static void PPrompt(void)
{
	AtomPt a1 ;
	Ensure( UnifyWithAtomic(X0, MakeAtom(InterLineGetLinePrompt())) ) ;
	a1 = XTestAtom(X1) ;
	ExtraPermanent(a1) ;	/* Ensures the prompt is not gc */
	linePrompt = AtomName(a1) ;
	JumpNext() ;
}


/* INIT */

void InterLineInit()
{
	InterLinePromptInit() ;
	InterLineInitialize() ;
	
	InstallCBuiltinPred("prompt", 2, PPrompt) ;
}
