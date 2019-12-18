/*
 *   This file is part of the CxProlog system

 *   TermWrite.c
 *   by A.Miguel Dias - 1992/02/23
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

static int character_escapes ;
static int ignore_ops ;
static int numbervars ;
static int portray ;
static int quoted ;

static Size termMaxDepth, listMaxLength ;
static Size currTermDepth ;
static PredicatePt portrayPred ;

static StreamPt output ;


/* Write Options */

static void SetWriteOptions(Pt t)
{
	t = TestList(t) ;
	for( ; IsList(t) ; t = Drf(XListTail(t)) ) {
		Pt e = Drf(XListHead(t)) ;
		Str opt ;
		Pt arg ;
		if( IsStruct(e)
			&& XStructArity(e) == 1
			&& (opt = XStructName(e))
			&& (arg = Drf(XStructArg(e,0))) )
				switch( opt[0] ) {
					case 'c':
						if( StrEqual(opt, "character_escapes") && XAtomBool(&character_escapes, arg) )
							continue ;
						break ;
					case 'i':
						if( StrEqual(opt, "ignore_ops") && XAtomBool(&ignore_ops, arg) )
							continue ;
						break ;
					case 'n':
						if( StrEqual(opt, "numbervars") && XAtomBool(&numbervars, arg) )
							continue ;
						break ;
					case 'p':
						if( StrEqual(opt, "portray") && XAtomBool(&portray, arg) )
							continue ;
						break ;
					case 'q':
						if( StrEqual(opt, "quoted") && XAtomBool(&quoted, arg) )
							continue ;
						break ;
				}
		FileError("Invalid write_term option '%s'", TermAsStr(e)) ;
	}
}


/* Term Write */
	
static Str AnalizeAtom(AtomPt atom, Bool *dirty, Bool *allSymbols)
{
	Str name = AtomName(atom), s = name ;
	int first = CharDecode(s) ;
	Bool d = false, a = false ;
	switch( CharType(first) ) {
		case _SO: {
			a = s[0] == '\0' ;
			d = quoted && !a ;
			break ;
		}
		case '|': {
			a = s[0] == '\0' ;
			d = quoted && !a ;
			break ;
		}
		case '{': {
			a = s[0] == '}' && s[1] == '\0' ;
			d = quoted && !a ;
			break ;
		}
		case '[': {
			a = s[0] == ']' && s[1] == '\0' && !nilIsSpecial_flag ;
			d = quoted && !a ;
			break ;
		}
		case _LC: {
			if( quoted )
				while( *s != '\0' ) {
					int c = CharDecode(s) ;
					if( !cx_isalnum(c) ) {
						d = true ;
						break ;
					}
				}
			break ;
		}
		case _SY: {
			if( first == '.' && s[0] == '\0' )
				d = quoted ;
			else {
				a = true ;
				while( *s != '\0' ) {
					int c = CharDecode(s) ;
					if( !cx_issymbol(c) ) {
						d = quoted ;
						a = false ;
						break ;
					}
				}
			}
			break ;
		}
		default: d = quoted ; break ;
	}
	*dirty = d ;
	*allSymbols = a ;
	return name ;
}

#define AddChar(c)		(StreamPut(output, c))
#define AddStr(s)		(StreamPutStr(output, s))

static void AddEscapeSequence(WChar c)
{
	switch( c ) {
		case '\a':
			AddChar('\\') ; AddChar('a') ; break ;
		case '\b':
			AddChar('\\') ; AddChar('b') ; break ;
		case '\f':
			AddChar('\\') ; AddChar('f') ; break ;
		case '\n':
			AddChar('\\') ; AddChar('n') ; break ;
		case '\r':
			AddChar('\\') ; AddChar('r') ; break ;
		case '\t':
			AddChar('\\') ; AddChar('t') ; break ;
		case '\v':
			AddChar('\\') ; AddChar('v') ; break ;
		case '\'':
			AddChar('\\') ; AddChar('\'') ; break ;
		case '\\':
			AddChar('\\') ; AddChar('\\') ; break ;
		default:
			if( c < ' ' || CharType(c) == __I  )
				AddStr(CharAsNumericEscape(c)) ;
			else AddChar(c) ;
			break ;
	}
}

static void AddOpenPar(Bool unaryParClash)
{
	if( unaryParClash )
		AddChar(' ') ;	
	AddChar('(') ;	
}

static void AddQStr(Str s, Bool dirty, Bool opClashL, Bool opClashR)
{
	if( dirty ) {
		WChar c ;
		AddChar('\'') ;
		if( character_escapes )
			while((c = CharDecode(s)) != '\0')
				AddEscapeSequence(c) ;
		else
			while((c = CharDecode(s)) != '\0') {
				if( c == '\'' )
					AddChar(c) ; /* Twice */
				AddChar(c) ;
			}
		AddChar('\'') ;
	}
	else {
		if( opClashL && (cx_issymbol(CharFirst(s)) || CharFirst(s) == '(') ) AddChar(' ') ;
		AddStr(s) ;
		if( opClashR && cx_issymbol(CharLast(s)) ) AddChar(' ') ;
	}
}

static void AddAtom(AtomPt atom, Bool opClashL, Bool opClashR)
{
	Bool dirty, allSymbols ;
	Str name = AnalizeAtom(atom, &dirty, &allSymbols) ;
	AddQStr(name, dirty, opClashL, opClashR) ;
}

static void DoWriteTerm(register Pt term, int p, Bool opClashL, Bool opClashR, Bool parAtom, Bool unaryParClash) ;

static void WriteStruct(Pt term, int m, Bool opClashL, Bool opClashR, Bool unaryParClash)
{
	register FunctorPt func = XStructFunctor(term) ;
	register AtomPt atom = FunctorAtom(func) ;
	int p, lp, rp ;
	Size lvrLevel = LvrPush(&term) ;	/* because of portray/1 */
	switch( FunctorArity(func) ) {
		case 0: InternalError("WriteStruct") ;
		case 1: {
			if( ignore_ops )
				goto rawL ;
			elif( func == varFunctor ) {
				Pt t = DrfChecked(XStructArg(term, 0)) ;
				if( !numbervars )
					goto rawL ;
				elif( IsAtom(t) )
					AddStr(XAtomName(t)) ;
				elif( IsInt(t) ) {
					PInt i = XInt(t) ;
					if( i >= 0 ) {
						AddChar('A' + i % 26) ;
						if( i >= 26 )
							AddStr(GStrFormat("%d", i / 26)) ;
					}
					else goto rawL ;
				}
				else goto rawL ;
			}
			elif( func == bracketsFunctor ) {
				AddChar('{') ;
				DoWriteTerm(XStructArg(term, 0), maxPrec, false, false, false, false) ;
				AddChar('}') ;
			}
			elif( func == stringFunctor ) {
				AddChar('"') ;
				DoWriteTerm(XStructArg(term, 0), maxPrec, false, false, false, false) ;
				AddChar('"') ;
			}
			elif( func == unitParamFunctor ) {
				AddStr(XAtomName(UnitParam(CurrUnit(), XUnitParam(term)))) ; /*@@@*/
			}
			elif( func == metaCutFunctor ) {
				AddChar('!') ;
			}
			elif( (p = Prefix(atom, &rp)) != 0 ) {
				Bool dirty, allSymbols ;
				Str name = AnalizeAtom(atom, &dirty, &allSymbols) ;
				if( p > m ) { AddChar('(') ; opClashL = opClashR = false ; }
				if( allSymbols ) {
					AddQStr(name, dirty, opClashL, false) ;	/* ex: a-> -b */
					DoWriteTerm(XStructArg(term, 0), rp, true, opClashR, false, true) ;
				}
				else {
					AddQStr(name, dirty, opClashL, false) ;
					AddChar(' ') ;	/* ex: a->not not b */
					DoWriteTerm(XStructArg(term, 0), rp, false, opClashR, false, false) ;
				}
				if( p > m ) AddChar(')') ;
			}
			elif( (p = Postfix(atom, &lp)) != 0 ) {
				Bool dirty, allSymbols ;
				Str name = AnalizeAtom(atom, &dirty, &allSymbols) ;
				if( p > m ) { AddChar('(') ; opClashL = opClashR = false ; }
				if( allSymbols ) {
					DoWriteTerm(XStructArg(term, 0), lp, opClashL, true, false, false) ;
					AddQStr(name, dirty, false, opClashR) ; /* ex: a! ->b */
				}
				else {
					DoWriteTerm(XStructArg(term, 0), lp, opClashL, false, false, false) ;
					AddChar(' ') ;
					AddQStr(name, dirty, false, opClashR) ;
				}
				if( p > m ) AddChar(')') ;
			}
			else goto rawL ;
			break ;
		}
		case 2: {
			if( ignore_ops )
				goto rawL ;
			elif( (p = Infix(atom, &lp, &rp)) != 0 ) {
				Bool dirty, allSymbols ;
				Str name = AnalizeAtom(atom, &dirty, &allSymbols) ;
				if( p > m ) { AddOpenPar(unaryParClash) ; opClashL = opClashR = false ; }
				if( func == commaFunctor ) {
					DoWriteTerm(XStructArg(term, 0), lp, opClashL, false, false, false) ;
					AddChar(',') ;
					if( extraSpacesInTerms_flag ) AddChar(' ') ;
					DoWriteTerm(XStructArg(term, 1), rp, false, opClashR, false, false) ;
				}
				elif( allSymbols ) {
					DoWriteTerm(XStructArg(term, 0), lp, opClashL, true, true, false) ;
					AddQStr(name, false, false, false) ;
					DoWriteTerm(XStructArg(term, 1), rp, true, opClashR, false, false) ;
				}
				else {
					DoWriteTerm(XStructArg(term, 0), lp, opClashL, false, true, false) ;
					AddChar(' ') ;
					AddQStr(name, dirty, false, false) ;
					AddChar(' ') ;
					DoWriteTerm(XStructArg(term, 1), rp, false, opClashR, false, false) ;
				}
				if( p > m ) AddChar(')') ;
			}
			else goto rawL ;
			break ;
		}
		default: {
			int i ;
rawL:		AddAtom(atom, opClashL, false) ;
			AddChar('(') ;
			dotimes(i, XStructArity(term)) {
				if( i != 0 ) {
					AddChar(',') ;
					if( extraSpacesInTerms_flag ) AddChar(' ') ;
				}
				DoWriteTerm(XStructArg(term, i), subPrec, false, false, false, false) ;
			}
			AddChar(')') ;
			break ;
		}
	}
	LvrRestore(lvrLevel) ;
}

static void WriteListNoOps(Pt term)
{
	Size lvrLevel = LvrPush(&term) ;
	AddChar('.') ;
	AddChar('(') ;
	DoWriteTerm(XListHead(term), subPrec, false, false, false, false) ;
	AddChar(',') ;
	if( extraSpacesInTerms_flag ) AddChar(' ') ;
	DoWriteTerm(XListTail(term), subPrec, false, false, false, false) ;
	AddChar(')') ;
	LvrRestore(lvrLevel) ;
}

static void WriteList(Pt term)
{
	register Size i ;
	Size lvrLevel = LvrPush(&term) ;
	AddChar('[') ;
	dotimes(i, listMaxLength) {
		DoWriteTerm(XListHead(term), subPrec, false, false, false, false) ;
		term = DrfChecked(XListTail(term)) ;
		if( !IsList(term) ) break ;
		AddChar(',') ;
		if( extraSpacesInTerms_flag ) AddChar(' ') ;
	}
	if( i == listMaxLength )
		AddStr("...") ;
	elif( term != tNilAtom ) {
		AddChar('|') ;
		if( extraSpacesInTerms_flag ) AddChar(' ') ;
		DoWriteTerm(term, subPrec, false, false, false, false) ;
	}
	AddChar(']') ;
	LvrRestore(lvrLevel) ;
}

static Bool Portray(Pt term)
{
	int character_escapesSave ;
	int ignore_opsSave ;
	int numbervarsSave ;
	int portraySave ;
	int quotedSave ;
	Size currTermDepthSave ;
	StreamPt outputSave ;
	Bool res ;

	if( !PredHasClauses(portrayPred) ) return false ;
	character_escapesSave = character_escapes ;
	ignore_opsSave = ignore_ops ;
	numbervarsSave = numbervars ;
	portraySave = portray ;
	quotedSave = quoted ;
	currTermDepthSave = currTermDepth ;
	outputSave = output ;
	
	res = CallProlog(MakeUnStruct(PredFunctor(portrayPred),term)) == peSucc ;
	
	character_escapes = character_escapesSave ;
	ignore_ops = ignore_opsSave ;
	numbervars = numbervarsSave ;
	portray = portraySave ;
	quoted = quotedSave ;
	currTermDepth = currTermDepthSave ;
	output = outputSave ;

	return res ;
}

static void DoWriteTerm(Pt term, int p, Bool opClashL, Bool opClashR, Bool parAtom, Bool unaryParClash)
{
	int q, rq ;
	Size lvrLevel = LvrPush(&term) ;
	term = DrfChecked(term) ;
	if( IsAtom(term) )
		if( portray && Portray(term) ) ;
		else {
			AtomPt a = XAtom(term) ;
			if( parAtom && (q = Prefix(a, &rq)) != 0 )
				{ AddChar('(') ; AddAtom(a, false, false) ; AddChar(')') ; }
			else AddAtom(a, opClashL, opClashR) ;
		}
	elif( IsVar(term) )
		AddStr(VarName(term)) ;
	elif( IsNumber(term) )
		if( portray && Portray(term) ) ;
		else {
			Str s = XNumberAsStr(term) ;
			if( opClashL && s[0] == '-' ) { /* ex: >(-1) */
				AddChar('(') ; AddStr(s) ; AddChar(')') ;
			}
			else AddStr(s) ;
		}
	elif( IsExtra(term) )
		if( portray && Portray(term) ) ;
		elif( term == tNilAtom && nilIsSpecial_flag )
			AddStr("[]") ;
		elif( term == tNullAtom )
			AddStr("1'null") ;
		else
			AddStr(XExtraAsStr(term)) ;
	elif( IsStruct(term) ) {
		if( currTermDepth > termMaxDepth ) AddStr("...") ;
		elif( portray && Portray(term) ) ;
		else { currTermDepth++ ;
			   WriteStruct(term, p, opClashL, opClashR, unaryParClash) ;
			   currTermDepth--;
		}
	}
	elif( IsList(term) ) {
		if( currTermDepth > termMaxDepth ) AddStr("...") ;
		elif( portray && Portray(term) ) ;
		elif( ignore_ops ) {
			currTermDepth++ ;
			WriteListNoOps(term) ;
			currTermDepth-- ;
		}
		else {
			currTermDepth++ ;
			WriteList(term) ;
			currTermDepth-- ;
		}
	}
	else
		StreamWrite(output, "<INVALID TERM::%lx>", term) ;
	LvrRestore(lvrLevel) ;
}

static void WriteSubtermMode(StreamPt srm, Pt subterm, Pt term)
{
	output = srm ;
	currTermDepth = 0 ;
	PrepareDrfChecked(term) ;

	/* Centralized call point */
	DoWriteTerm(subterm, maxPrec, false, false, false, 99) ;
}

static void WriteTermMode(StreamPt srm, Pt term)
{
	if( srm == userErr ) {
		StreamFlush(userOut) ;
		WriteSubtermMode(srm, term, term) ;
		StreamFlush(userErr) ;
	}
	else
		WriteSubtermMode(srm, term, term) ;
}

static void WriteTermsMode(StreamPt srm, Pt list)
{
	for( list = Drf(list) ; IsList(list) ; list = Drf(XListTail(list)) ) {
		Pt t = Drf(XListHead(list)) ;
		if( IsList(t) || t == tNilAtom )
			WriteTermsMode(srm, t) ;
		else
			WriteTermMode(srm, t) ;
	}
	if( list != tNilAtom )
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
}

static Str SubtermAsStrMode(Pt subterm, Pt term)
{
	StreamPt s = InnerStreamOpen() ; /* open inner stream afresh */
	WriteSubtermMode(s, subterm, term) ;
	return StreamClose(s, nil) ;
}

static Str TermAsStrMode(Pt term)
{
	StreamPt s = InnerStreamOpen() ; /* open inner stream afresh */
	WriteTermMode(s, term) ;
	return StreamClose(s, nil) ;
}

static Str TermsAsStrMode(Pt list)
{
	StreamPt s = InnerStreamOpen() ; /* open inner stream afresh */
	WriteTermsMode(s, list) ;
	return StreamClose(s, nil) ;
}

static void WriteTermWithOptions(StreamPt srm, Pt term, Pt options)
{
/* Set defaults */
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = forceQuoted_flag ;
/* Change defaults */
	SetWriteOptions(options) ;
/* Write the term */
	WriteTermMode(srm, term) ;
}


/* PUBLIC */

void TermWriteN(StreamPt srm, Pt term)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = forceQuoted_flag ;
	WriteTermMode(srm, term) ;
}

void TermWriteQ(StreamPt srm, Pt term) /* quoted */
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = 1 ;
	WriteTermMode(srm, term) ;
}

void TermWriteP(StreamPt srm, Pt term) /* print */
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 0 ;
	portray = 1 ;
	quoted = forceQuoted_flag ;
	WriteTermMode(srm, term) ;
}

void TermWriteD(StreamPt srm, Pt term) /* display */
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 1 ;
	numbervars = 0 ;
	portray = 0 ;
	quoted = forceQuoted_flag ;
	WriteTermMode(srm, term) ;
}

void TermWriteC(StreamPt srm, Pt term)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 1 ;
	numbervars = 0 ;
	portray = 0 ;
	quoted = 1 ;
	WriteTermMode(srm, term) ;
}

Str TermAsStrN(Pt term)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = forceQuoted_flag ;
	return TermAsStrMode(term) ;
}

Str TermAsStrQ(Pt term)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = 1 ;
	return TermAsStrMode(term) ;
}

Str SubtermAsStrN(Pt subterm, Pt term)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = forceQuoted_flag ;
	return SubtermAsStrMode(subterm, term) ;
}

Str SubtermAsStrQ(Pt subterm, Pt term)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = 1 ;
	return SubtermAsStrMode(subterm, term) ;
}

Str TermsAsStrN(Pt list)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = forceQuoted_flag ;
	return TermsAsStrMode(list) ;
}

Str TermsAsStrQ(Pt list)
{
	character_escapes = characterEscapes_flag ;
	ignore_ops = 0 ;
	numbervars = 1 ;
	portray = 0 ;
	quoted = 1 ;
	return TermsAsStrMode(list) ;
}

Str TermAsStr(Pt term)
{
	return TermAsStrQ(term) ;

}

void SetWriteDepth(Size termDepth, Size listLength)
{
	termMaxDepth = termDepth == 0 ? LONG_MAX : termDepth ;
	listMaxLength = listLength == 0 ? LONG_MAX : listLength ;
}



/* CXPROLOG C'BUILTINS */

static void PWriteTerm()
{
	WriteTermWithOptions(currOut, X0, X1) ;
	JumpNext() ;
}

static void PSWriteTerm()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	WriteTermWithOptions(srm, X1, X2) ;
	JumpNext() ;
}

static void PWrite()
{
	TermWriteN(currOut, X0) ;
	JumpNext() ;
}

static void PSWrite()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteN(srm, X1) ;
	JumpNext() ;
}

static void PWriteln()
{
	TermWriteN(currOut, X0) ;
	StreamPut(currOut, '\n') ;
	JumpNext() ;
}

static void PSWriteln()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteN(srm, X1) ;
	StreamPut(srm, '\n') ;
	JumpNext() ;
}

static void PWriteQ()
{
	TermWriteQ(currOut, X0) ;
	JumpNext() ;
}

static void PSWriteQ()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteQ(srm, X1) ;
	JumpNext() ;
}

static void PWriteQln()
{
	TermWriteQ(currOut, X0) ;
	StreamPut(currOut, '\n') ;
	JumpNext() ;
}

static void PSWriteQln()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteQ(srm, X1) ;
	StreamPut(srm, '\n') ;
	JumpNext() ;
}

static void PPrint()
{
	TermWriteP(currOut, X0) ;
	JumpNext() ;
}

static void PSPrint()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteP(srm, X1) ;
	JumpNext() ;
}

static void PDisplay()
{
	TermWriteD(userOut, X0) ;
	JumpNext() ;
}

static void PSDisplay()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteD(srm, X1) ;
	JumpNext() ;
}

static void PDisplayln()
{
	TermWriteD(userOut, X0) ;
	StreamPut(userOut, '\n') ;
	JumpNext() ;
}

static void PSDisplayln()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteD(srm, X1) ;
	StreamPut(srm, '\n') ;
	JumpNext() ;
}

static void PWriteCanonical()
{
	TermWriteC(userOut, X0) ;
	JumpNext() ;
}

static void PSWriteCanonical()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	TermWriteC(srm, X1) ;
	JumpNext() ;
}

static void PAtomTermN()
{
	Pt t0 = TestAtomOrVar(X0) ;
	Pt t1 = Drf(X1) ;
	if( IsAtom(t0) ) {
		Pt t = t0 == tEmptyAtom ? t0 : ZTermFromStr(XAtomName(t0)) ; /* stacks may grow */
		MustBe( Unify(t1, t) ) ;
	}
	elif( IsVar(t0) && t0 != t1 ) {
		Pt t = MakeTempAtom(TermAsStrN(t1)) ;
		MustBe( UnifyWithAtomic(t0, t) ) ;
	}
	else
		DoFail() ;
}

static void PAtomTermQ()
{
	Pt t0 = TestAtomOrVar(X0) ;
	Pt t1 = Drf(X1) ;
	if( IsAtom(t0) ) {
		Pt t = t0 == tEmptyAtom ? t0 : ZTermFromStr(XAtomName(t0)) ; /* stacks may grow */
		MustBe( Unify(t1, t) ) ;
	}
	elif( IsVar(t0) && t0 != t1 ) {
		Pt t = MakeTempAtom(TermAsStrQ(t1)) ;
		MustBe( UnifyWithAtomic(t0, t) ) ;
	}
	else
		DoFail() ;
}

static void PQuote()
{
	Str n = XTestAtomName(X0) ;
	output = InnerStreamOpen() ;
	AddQStr(n, true, false, false) ;
	MustBe( Unify(X1, MakeTempAtom(StreamClose(output, nil))) ) ;
}

static void PWriteDepth(void)
{
	SetWriteDepth(XTestNat(X0), XTestNat(X1)) ;
	JumpNext() ;
}

void TermWriteInit()
{
	SetWriteDepth(30,999) ;

/* create portray/1 descriptor (is a mutable built-in) */
	portrayPred = LookupPredicateByName("portray", 1) ;
	SetDynamic(portrayPred, true) ;

/* install builti-ns */
	InstallCBuiltinPred("write_term", 2, PWriteTerm) ;
	InstallCBuiltinPred("write_term", 3, PSWriteTerm) ;
	InstallCBuiltinPred("write", 1, PWrite) ;
	InstallCBuiltinPred("write", 2, PSWrite) ;
	InstallCBuiltinPred("writeln", 1, PWriteln) ;
	InstallCBuiltinPred("writeln", 2, PSWriteln) ;
	InstallCBuiltinPred("writeq", 1, PWriteQ) ;
	InstallCBuiltinPred("writeq", 2, PSWriteQ) ;
	InstallCBuiltinPred("writeqln", 1, PWriteQln) ;
	InstallCBuiltinPred("writeqln", 2, PSWriteQln) ;
	InstallCBuiltinPred("print", 1, PPrint) ;
	InstallCBuiltinPred("print", 2, PSPrint) ;
	InstallCBuiltinPred("display", 1, PDisplay) ;
	InstallCBuiltinPred("display", 2, PSDisplay) ;
	InstallCBuiltinPred("displayln", 1, PDisplayln) ;
	InstallCBuiltinPred("displayln", 2, PSDisplayln) ;
	InstallCBuiltinPred("write_canonical", 1, PWriteCanonical) ;
	InstallCBuiltinPred("write_canonical", 2, PSWriteCanonical) ;

	InstallCBuiltinPred("atom_term", 2, PAtomTermN) ;
	InstallCBuiltinPred("atom_termq", 2, PAtomTermQ) ;
	InstallCBuiltinPred("quote", 2, PQuote) ;

	InstallCBuiltinPred("write_depth", 2, PWriteDepth) ;
}
