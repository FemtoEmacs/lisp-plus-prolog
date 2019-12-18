/*
 *   This file is part of the CxProlog system

 *   Number.c
 *   by A.Miguel Dias - 2001/02/27
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
#include <locale.h>
#include <math.h>

#if !defined(__APPLE__) && defined(FP_ILOGB0)
#define HAS_LONG_DOUBLE_OPS		1
#endif


/* BOOL, CHAR, CODE & BYTE */

Pt MakeBool(Bool b)
{
	return b ? tTrueAtom : tFalseAtom ;
}

Pt MakeChar(WChar c)
{
	Str32 b ;
	CharPt s = b ;
	if( c == EOF ) return tEofAtom ;
	CharEncode(s, c) ;
	*s = '\0' ;
	return MakeAtom(b) ;
}

WChar XChar(Pt t) /* pre: IsAtom(t) */
{
	Str s = XAtomName(t) ;
	WChar c = CharDecode(s) ;
	if( c == '\0' || *s == '\0') return c ;
	else return -1 ;
}

Pt MakeCode(WChar c) /* pre: c is valid code */
{
	if( c == '\n' ) c = 10 ;
	else if( c == EOF ) c = eofCode_flag ;
	return EncodeInt(c) ;
}

WChar XCode(Pt t) /* pre: IsCode(t) */
{
	WChar c = DecodeInt(t) ;
	if( c == 10 ) c = '\n' ;
	return c ;
}

Pt MakeByte(register int c) /* pre: IsByte(c) */
{
	if( c == EOF ) c = -1 ;
	return EncodeInt(c) ;
}

int XByte(Pt t) /* pre: IsByte(t) */
{
	return DecodeUInt(t) ;
}


/* INT */

int intSize ;
static Str32 intFormat, lLIntFormat ;

Pt minusOneIntPt, zeroIntPt, oneIntPt, twoIntPt, threeIntPt,
   maxIntPt, maxUIntPt, minIntPt ;

Pt MakeInt(PInt n)
{
	if( minInt <= n && n <= maxInt )
		return EncodeInt(n) ;
	else return MakeFloat(n) ;
}

Pt MakeUInt(PInt n)
{
	if( n < 0 ) Error("Negative argument in MakeUInt") ;
	if( n <= maxUInt )
		return EncodeInt(n) ;
	else return MakeFloat(n) ;
}

Pt MakeLLInt(LLInt n) /* used in the java interface */
{
	if( minInt <= n && n <= maxInt )
		return EncodeInt(n) ;
	else {
		PFloat f = cPFloat(n) ;
		if( IsIntFloat(f) )
			return MakeFloat(f) ;
	}
	return Error(GStrFormat("Cannot encode the large-int '%s'", lLIntFormat), n) ;
}

PInt XInt(Pt t) /* pre: IsInt(t) */
{
	return DecodeInt(t) ;
}

PInt XUInt(Pt t) /* pre: IsNat(t) */
{
	return DecodeUInt(t) ;
}

PInt XAsInt(Pt t) /* pre: IsNumber(t) */
{
	if( IsInt(t) ) return XInt(t) ;
	return cPInt(XFloat(t) + 0.5) ;
}

LLInt XAsLLInt(Pt t) /* pre: IsNumber(t) */
{
	if( IsInt(t) ) return XInt(t) ;
	else {
		PFloat f = XFloat(t) ;
		if( IsIntFloat(f) )
			return (LLInt)f ;
		else {
			Error("Invalid large-int '%s'", XNumberAsStr(t)) ;
			return 0 ;
		}
	}
}

int CompareInt(PInt i1, PInt i2)
{
	return i1 == i2 ? 0 : i1 > i2 ? 1 : -1 ;
}

static void IntsInit(void)
{
	intSize = sizeof(Word) * 8 - 3 ;
	strcpy(intFormat, "%ld") ;
	strcpy(lLIntFormat, LongLongs() ? "%lld" : "%ld") ;
}


/* FLOAT */

/* CxProlog uses the highest precision float numbers available in C.
   The fact that floats are atomic terms is a bit of a problem.
   In CxProlog each atomic term must fit one machine word and
   atomic terms comparison must be implemented by simple word
   comparison. So we need to store each float as a unique entry
   in an hash table and represent that float by a pointer to the
   corresponding unique entry.
*/

static ExtraTypePt floatType ;

int floatSize ;
Pt nanFloatPt, infFloatPt, eFloatPt, piFloatPt, epsilonFloatPt ;
static PFloat nanFloat, infFloat, ninfFloat ;
static Str32 floatFormat, intFloatFormat ;

typedef struct FloatEntry {
	ExtraDef(FloatEntry) ;
	PFloat value ;						/* Float value */
} FloatEntry, *FloatEntryPt ;

#define	cFloatEntryPt(f)			((FloatEntryPt)f)

#define FloatEntryValue(f)			(cFloatEntryPt(f)->value)
#define XFloatEntryValue(t)			FloatEntryValue(XPt(t))


static Bool LookupFloatEntryAux(CVoidPt x, CVoidPt ref)
{
	return FloatEntryValue(x) == *cPFloatPt(ref) ;
}
static FloatEntryPt LookupFloatEntry(PFloat r)
{
	register UCharPt u = cUCharPt(&r) ;
	UChar slot = u[1] + u[3] + u[5] + u[7] + (floatSize >= 80 ? u[9] : 0) ; /* hash */
	FloatEntryPt f ;
	if( (f = ExtraFindFirst(floatType, slot, LookupFloatEntryAux, &r)) == nil ) {
		f = ExtraNew(floatType, slot) ;
		FloatEntryValue(f) = r ;
	}
	return f ;
}

Pt MakeFloat(PFloat r)
{
	register PInt i = cPInt(r) ;
	if( i != r || i < minInt || i > maxInt )
		return TagFloat(LookupFloatEntry(r)) ;
	else return MakeInt(i) ;
}

static Pt MakePermFloat(PFloat r)
{
	register PInt i = cPInt(r) ;
	if( i != r || i < minInt || i > maxInt ) {
		FloatEntryPt f = LookupFloatEntry(r) ;
		ExtraPermanent(f) ;
		return TagFloat(f) ;
	}
	else return MakeInt(i) ;
}

PFloat XFloat(Pt t) /* pre: IsFloat(t) */
{
	return XFloatEntryValue(t) ;
}

PFloat XAsFloat(Pt t) /* pre: IsNumber(t) */
{
	if( IsInt(t) ) return cPFloat(XInt(t)) ;
	return XFloatEntryValue(t) ;
}

Str FloatAsStr(Char fmt, PFloat f)
{
	switch( fmt ) {
		case 'e':
		case 'E':
		case 'f':
		case 'F':
		case 'g':
		case 'G': {
#if HAS_LONG_DOUBLE_OPS
			Char ffmt[] = "%ll_" ;
			ffmt[3] = fmt ;
#else
			Char ffmt[] = "%l_" ;
			ffmt[2] = fmt ;
#endif
			return GStrFormat(ffmt, f) ;
		}
		default:
			return InternalError("FloatAsStr") ;
	}
}

int CompareFloat(PFloat r1, PFloat r2)
{
	if( r1 == r2 ) return 0 ;
	if(	r1 < r2 ) return -1 ;
	if( r1 > r2 ) return 1 ;
	return -2 ;
}

int CalculateFloatSize()
{
	register unsigned int i, n = 0 ;
	PFloat f = 1.0101 ;
	register Str s = cCharPt(&f) ;
	f *= f ;
	dotimes(i, sizeof(PFloat))
		if( *s++ != 0 )
			n++ ;
	return n * 8 ;
}

static Str BuildFloatFormat(int precision)
{
#if HAS_LONG_DOUBLE_OPS
	if( precision > 0 )
		return GStrFormat("%%.%dllg", precision) ;
	else
		return GStrFormat("%%.%dllf", -precision) ;
#else
	if( precision > 0 )
		return GStrFormat("%%.%dlg", precision) ;
	else
		return GStrFormat("%%.%dlf", -precision) ;
#endif
}

void FloatDisplayPrecUpdateFlag(int newValue)
{
	floatDisplayPrec_flag = newValue ;
	strcpy(floatFormat, BuildFloatFormat(floatDisplayPrec_flag)) ;
}

static PFloat CalcEpsilon(void)
{
	PFloat ep = 1.0, one = 1.0 ;
	for( ; one + ep > one ; ep /= 2 ) ;
	return ep * 2 ;
}

static Size FloatSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(FloatEntry) ;
}

static Bool FloatBasicGCDelete(VoidPt x)
{
	Unused(x) ;
	return true ;
}

static void FloatsInit(void)
{
	floatType = ExtraTypeNew("FLOAT", FloatSizeFun, nil, FloatBasicGCDelete, 256) ;
	ExtraTypeDoesNotSupportAliasing(floatType) ;
	floatSize = CalculateFloatSize() ;
	strcpy(floatFormat, BuildFloatFormat(floatDisplayPrec_flag)) ;
	strcpy(intFloatFormat, BuildFloatFormat(80)) ;
}


/* NUMBER */

/* The use of 'setlocale' makes the sintax of real numbers immune to the
   current locale configuration. Note that 'setlocale' is very fast if the
   locale doesn't really change, as it will be the case almost always. */

Str XNumberAsStr(register Pt t) /* already deref */
{
	if( IsInt(t) )
		return GStrFormat(intFormat, XInt(t)) ;
	elif( IsFloat(t) ) {
		PFloat f = XFloat(t) ;
		if( IsIntFloat(f) ) return GStrFormat(intFloatFormat, f) ;
		elif( f != f ) return "nan" ;
		elif( f == infFloat ) return "inf" ;
		elif( f == ninfFloat ) return "-inf" ;
		else {
			Str loc = setlocale(LC_NUMERIC, "C") ;
			Str res = GStrFormat(floatFormat, f) ;
			setlocale (LC_NUMERIC, loc) ;
			return res ;
		}
	}
	elif( IsThisStruct(t, colonFunctor) ) {
		PFloat f = XTestFloat(XStructArg(t,0)) ;
		Str format = BuildFloatFormat(XTestInt(XStructArg(t,1))) ;
		Str loc = setlocale(LC_NUMERIC, "C") ;
		Str res = GStrFormat(format, f) ;
		setlocale (LC_NUMERIC, loc) ;
		return res ;
	}
	else return InternalError("XNumberAsStr") ;
}

Pt NumberFromStr(Str s)
{
	PFloat d ;
	Str loc ;

	if( InRange(s[0], '0', '9') ) ;
	elif( s[0] == '-' && InRange(s[1], '0', '9') ) ;
	else return nil ;

	loc = setlocale(LC_NUMERIC, "C") ;
#if HAS_LONG_DOUBLE_OPS
	d = strtold(s, (char **)&s) ;
#else
	d = strtod(s, (char **)&s) ;
#endif
	setlocale (LC_NUMERIC, loc) ;
	if( s[0] == '\0' && InRange(s[-1], '0', '9') )
		return MakeFloat(d) ;
	else return nil ;
}

int CompareNumber(register Pt t1, register Pt t2) /* already deref */
{
	if( IsInt(t1) ) {
		if( IsInt(t2) ) return CompareInt(XInt(t1), XInt(t2)) ;
		if(IsFloat(t2) ) return CompareFloat(XInt(t1), XFloat(t2)) ;
	}
	elif( IsFloat(t1) ) {
		if( IsInt(t2) ) return CompareFloat(XFloat(t1), XInt(t2)) ;
		if( IsFloat(t2) ) return CompareFloat(XFloat(t1), XFloat(t2)) ;
	}
	InternalError("CompareNumber") ;
	return false ;
}

void NumbersInit()
{
	IntsInit() ;
	FloatsInit() ;
}


/* CXPROLOG C'BUILTINS */

static Size FloatsAux(CVoidPt x)
{
	FloatEntryPt f = cFloatEntryPt(x) ;
	Write("%18.8lle", FloatEntryValue(f)) ;
	return 1 ;
}
static void PFloats()
{
	ExtraShow(floatType, FloatsAux) ;
	JumpNext() ;
}

static void ShowFloat(PFloat f)
{
	unsigned int i ;
	register Str s = cCharPt(&f) ;
	Write("%18.8lle", f) ;
	dotimes(i, sizeof(PFloat))
		Write("%4d", s[i]) ;
	Write("\n") ;
}

static void PCheckFloats()
{
	int i ;
	PFloat f = 1.01 ;
	dotimes(i,20) {
		ShowFloat(f) ;
		f *= f ;
	}
	ShowFloat(0) ;
	ShowFloat(nanFloat) ;
	ShowFloat(infFloat) ;
	JumpNext() ;
}
void NumbersInit2()
{
	PFloat zero = 0.0 ;
	nanFloat = 0.0/zero ;	/* div by zero */
	infFloat = 1.0/zero ;	/* div by zero */
	ninfFloat = -infFloat ;

	minusOneIntPt = MakeInt(-1) ;
	zeroIntPt = MakeInt(0) ;
	oneIntPt  = MakeInt(1) ;
	twoIntPt = MakeInt(2) ;
	threeIntPt = MakeInt(3) ;
	maxIntPt = MakeInt(maxInt) ;
	maxUIntPt = MakeUInt(maxUInt) ;
	minIntPt = MakeInt(minInt) ;

	nanFloatPt = MakePermFloat(nanFloat) ;
	infFloatPt = MakePermFloat(infFloat) ;
	eFloatPt = MakePermFloat(2.7182818284590452353602874713526625L) ;
	piFloatPt = MakePermFloat(3.1415926535897932384626433832795029L) ;
	epsilonFloatPt = MakePermFloat(CalcEpsilon()) ;

	InstallCBuiltinPred("floats", 0, PFloats) ;
	InstallCBuiltinPred("chk_floats", 0, PCheckFloats) ;
}
