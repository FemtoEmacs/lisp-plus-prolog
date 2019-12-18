/*
 *   This file is part of the CxProlog system

 *   Character.c
 *   by A.Miguel Dias - 2005/07/05
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

/* WARNING: Forbiden changes:
		- changes involving _DG
		- changes involving chars that represent themselves
		- '.' and '/' must remain _SY
		- 'sub' must remain _EF

	Any other change seems to be safe (albeit probably unnecessary.)
*/

/* Even after the introduction of Unicode support, CxProlog still treats
   in a special way the ISO 8859-1 (Latin-1) characters */

CharTypes _allChars[257] = {
/* eof */
   _EF,
/* nul  soh  stx  etx  eot  enq  ack  bel   bs   ht   nl   vt   np   cr   so   si */
   __I, __I, __I, __I, __I, __I, __I, __I, _BL, _BL, _BL, __I, __I, _BL, __I, __I,
/* dle  dc1  dc2  dc3  dc4  nak  syn  etb  can   em  sub  esc   fs   gs   rs   us */
   __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I,
/*  sp    !   "     #   $    %     &   '    (    )     *    +   ,     -    .    / */
   _BL, _SO,'\"', _SY, _SY, '%', _SY,'\'', '(', ')', _SY, _SY, ',', _SY, _SY, _SY,
/*   0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ? */
   _DG, _DG, _DG, _DG, _DG, _DG, _DG, _DG, _DG, _DG, _SY, _SY, _SY, _SY, _SY, _SY,
/*   @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O */
   _SY, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC,
/*   P    Q    R    S    T    U    V    W    X    Y    Z   [     \   ]    ^     _ */
   _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, '[', _SY, ']', _SY, _UC,
/*   `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o */
   _SY, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC,
/*   p    q    r    s    t    u    v    w    x    y    z   {    |    }     ~  del */
   _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, '{', '|', '}', _SY, __I,
/* 128       130                                               140                */
   __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I,
/* 144                           150                                              */
   __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I, __I,
/* 160                                               170                          */
   _BL, _SY, _SY, _SY, _SY, _SY, _SY, _SY, _SY, _SY, _LC, _SY, _SY, _SY, _SY, _SY,
/* 176                 180                                               190      */
   _SY, _SY, _SY, _SY, _SY, _SY, _SY, _SY, _SY, _SY, _LC, _SY, _SY, _SY, _SY, _SY,
/* 192                                     200                                    */
   _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC, _UC,
/* 208       210                                               220                */
   _SY, _UC, _UC, _UC, _UC, _UC, _UC, _SY, _SY, _UC, _UC, _UC, _UC, _UC, _SY, _SY,
/* 224                           230                                              */
   _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC, _LC,
/* 240                                               250                      255 */
   _SY, _LC, _LC, _LC, _LC, _LC, _LC, _SY, _SY, _LC, _LC, _LC, _LC, _LC, _SY, _LC
} ;

/* Character reorder */

#define reorderTableSize   (1 K)

static Str reorderLocale = nil ;
static WChar *reorderTable = nil ;
static long startCode, endCode ;

#define ReorderTableWords(capacity)		Words((capacity) * sizeof(WChar))

static void ShowReorderState(void)
{
	int nLines, nCols, l, c ;
	ShowVersion() ;
	Write("CHARACTER REORDER STATE:\n") ;
	if( reorderLocale != nil ) {
		Write("    %% Reorder locale = %s\n", reorderLocale) ;
		return ;
	}
	if( reorderTable == nil ) {
		Write("    %% None\n") ;
		return ;
	}
	Write("  The reorder table contains %d entries, from char %05d to char %05d.\n",
			reorderTableSize, startCode, endCode-1) ;
	Write("  Table contents:\n") ;
	nCols = 6 ;
	nLines = DivUp(reorderTableSize, nCols) ;
	dotimes(l, nLines) {
		dotimes(c, nCols) {
			int i = c * nLines + l ;
			if( i < reorderTableSize ) {
				if( startCode == 0 && (InRange(i, 32, 127) || InRange(i-128, 32, 127)) ) {
					Write("      ") ;
					StreamPut(currOut, startCode + i) ;
				}
				else
					Write("  %05d", startCode + i) ;
				Write(":%05d", reorderTable[i]) ;
			}
		}
		Write("\n") ;
	}
}

static void CharReorderTableReset(int start) /* restart state */
{
	if( reorderLocale != nil ) {
		SetLocaleCollate("C") ;
		reorderLocale = nil ;
	}
	if( start == -1 ) {
		if( reorderTable != nil )
			Release(reorderTable, ReorderTableWords(reorderTableSize)) ;
		reorderTable = nil ;
	}
	else {
		register int i ;
		if( reorderTable == nil )
			reorderTable = Allocate(ReorderTableWords(reorderTableSize), false) ;
		startCode = RoundDown(start, reorderTableSize) ;
		endCode = startCode + reorderTableSize ;
		dotimes(i, reorderTableSize)
			reorderTable[i] = startCode + i ;
	}
}

static void CharReorderItem(int a, int b)
{
	if( reorderTable == nil )
		CharReorderTableReset(a) ;
	if( !InRange(a, startCode, endCode - 1) )
		Error("Char code %d is outside the reorder table range", a) ;
	reorderTable[a - startCode] = b ;
}

static void CharLatin1IgnoreDiacriticals(void)
{
	static UChar diacrTable[] = {
		'A', 192, 198, 'C', 199, 199, 'E', 200, 203,
		'I', 204, 207, 'N', 209, 209, 'O', 210, 214,
		'U', 217, 220, 'Y', 221, 221, '\0' } ;
	UCharPt u ;
	int i ;
	CharReorderTableReset(0) ;
	for( u = diacrTable ; *u != '\0' ; u += 3 )
		for( i = u[1] ; i <= u[2] ; i++ ) {
			reorderTable[i] = u[0] ;
			reorderTable[i + 'a' - 'A'] = u[0] + 'a' - 'A' ;
		}
	reorderTable[255] = 'y' ;
}

#if unused
static void CharLatin1IgnoreDiacriticalsAndCase(void)
{
	register int i ;
	CharLatin1IgnoreDiacriticals() ;
	for( i = 'a' ; i <= 255 ; i++ )
		if( cx_islower(reorderTable[i]) )
			reorderTable[i] -= 'a' - 'A' ;
}
#endif

int CharReorderCompare(Str a, Str b)
{
	if( reorderLocale != nil )
		return strcoll(a, b) ;
	if( reorderTable == nil )
		return strcmp(a, b) ;
	for(;;) {
		int ca = CharDecode(a) ;
		int cb = CharDecode(b) ;
		int rca = InRange(ca, startCode, endCode - 1)
					? reorderTable[ca - startCode]
					: ca ;
		int rcb = InRange(cb, startCode, endCode - 1)
					? reorderTable[cb - startCode]
					: cb ;
		if( rca < rcb ) return -1 ;
		if( rca > rcb ) return 1 ;
		if( ca < cb ) return -1 ;
		if( ca > cb ) return 1 ;
		if( ca == '\0' ) return 0 ;
	}
	return 0 ;
}

char *CharAsNumericEscape(WChar c)
{
	return GStrFormat("\\%03o\\", c) ;
}

void CutIsSoloUpdateFlag(int newValue)
{
	cutIsSolo_flag = newValue ;
	BasicCharType('!') = cutIsSolo_flag ? _SO : _SY ;
}


/* CXPROLOG C'BUILTINS */

static void PCharCode()
{
	Pt t0 = Drf(X0) ;
	Pt t1 = Drf(X1) ;
		
	if( IsVar(t0) )
		if( IsVar(t1) )
			TypeError("CHAR", t0) ;
		else
			MustBe( UnifyWithAtomic(t0, MakeChar(XTestCode(t1))) ) ;
	else
		if( IsVar(t1) )
			MustBe( UnifyWithAtomic(t1, MakeCode(XTestChar(t0))) ) ;
		else
			MustBe( XTestChar(t0) == XTestCode(t1) ) ;
	JumpNext() ;
}

static void PCharReorder0()
{
	ShowReorderState() ;
	JumpNext() ;
}

static void PCharReorder1()
{
	Str s = XTestAtomName(X0) ;
	if( StrEqual(s, "none") )
		CharReorderTableReset(-1) ;
	elif( StrEqual(s, "latin1_ignore_diacriticals") )
		CharLatin1IgnoreDiacriticals() ;
	elif( s != nil && *s != '\0' ) {
		Str loc ;
		if( (loc = SetLocaleCollate(s)) == nil )
			Error("Cannot set reorder locale '%s'", s) ;
		reorderLocale = nil ;
		CharReorderTableReset(-1) ;
		reorderLocale = loc ;
	}
	else Error("Unknown reorder option") ;
	JumpNext() ;
}

static void PCharReorder2()
{
	CharReorderItem(XTestCharOrCode(X0), XTestCharOrCode(X1)) ;
	JumpNext() ;
}

static void PCharIsAlnum()
{
	MustBe( cx_isalnum(XTestCharOrCode(X0)) ) ;
}

static void PCharIsAlpha()
{
	MustBe( cx_isalpha(XTestCharOrCode(X0)) ) ;
}

static void PCharIsLower()
{
	MustBe( cx_islower(XTestCharOrCode(X0)) ) ;
}

static void PCharIsUpper()
{
	MustBe( cx_isupper(XTestCharOrCode(X0)) ) ;
}

static void PCharIsDigit()
{
	MustBe( cx_isdigit(XTestCharOrCode(X0)) ) ;
}

static void PCharIsSpace()
{
	MustBe( cx_isspace(XTestCharOrCode(X0)) ) ;
}

static void PCharIsSymbol()
{
	MustBe( cx_issymbol(XTestCharOrCode(X0)) ) ;
}

static void PCharIsPrint()
{
	MustBe( cx_isprint(XTestCharOrCode(X0)) ) ;
}

static void PCharIsPunct()
{
	int c = XTestCharOrCode(X0) ;
	MustBe( cx_ispunct(c) ) ;
}

static void PCharConversion() /* Dummy predicate */
{
	TestCharOrCode(X0) ;
	TestCharOrCode(X1) ;
	/* Do nothing. */
	JumpNext() ;
}

static void PNDCurrentCharConversion() /* Dummy identity predicate */
{
	WChar c ;
	Pt t ;
	if( A(2) == tNilAtom ) {
		X0 = Drf(X0) ;
		X1 = Drf(X1) ;
		if( IsVar(X0) && IsVar(X1) )
			A(2) = minusOneIntPt ;
		else {
			if( !IsVar(X0) ) TestCharOrCode(X0) ;
			if( !IsVar(X1) ) TestCharOrCode(X1) ;
			Discard() ;
			MustBe( Unify(X0, X1) ) ;
		}
	}
	A(2) = IncIntPt(A(2)) ;
	c = XInt(A(2)) ;
	if( c == 256 ) Jump(DiscardAndFail) ;
	t = MakeChar(c) ;
	if( UnifyWithNumber(X0, t) && UnifyWithNumber(X1, t) ) JumpNext() ;
	InternalError("PNDCurrentCharConversion") ;
}

void CharactersInit()
{
	CharReorderTableReset(-1) ;

	InstallCBuiltinPred("char_code", 2, PCharCode) ;

	InstallCBuiltinPred("char_reorder", 0, PCharReorder0) ;
	InstallCBuiltinPred("char_reorder", 1, PCharReorder1) ;
	InstallCBuiltinPred("char_reorder", 2, PCharReorder2) ;

	InstallCBuiltinPred("char_isalnum", 1, PCharIsAlnum) ;
	InstallCBuiltinPred("char_isalpha", 1, PCharIsAlpha) ;
	InstallCBuiltinPred("char_islower", 1, PCharIsLower) ;
	InstallCBuiltinPred("char_isupper", 1, PCharIsUpper) ;
	InstallCBuiltinPred("char_isdigit", 1, PCharIsDigit) ;
	InstallCBuiltinPred("char_isspace", 1, PCharIsSpace) ;
	InstallCBuiltinPred("char_issymbol", 1, PCharIsSymbol) ;
	InstallCBuiltinPred("char_isprint", 1, PCharIsPrint) ;
	InstallCBuiltinPred("char_ispunct", 1, PCharIsPunct) ;

	InstallCBuiltinPred("char_conversion", 2, PCharConversion) ;
	InstallNDeterCBuiltinPred("current_char_conversion", 2, PNDCurrentCharConversion) ;
}
