/*
 *   This file is part of the CxProlog system

 *   Arith.c
 *   by A.Miguel Dias - 1989/12/05
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
#include <math.h>

#if !defined(__APPLE__) && defined(FP_ILOGB0)
#define HAS_LONG_DOUBLE_OPS		1
#endif

#if HAS_LONG_DOUBLE_OPS
#define MATH(f)		f##l
typedef PFloat OpsFloat ;
#else
#define MATH(f)		f
typedef double OpsFloat ;
#endif

/* PRIVATE FUNCTIONS */

/*
ISO Evaluable Functors:

Functor						Signature			Operation
~~~~~~~						~~~~~~~~~			~~~~~~~~~
(+)/2						I x I --> I			Addition
(+)/2						F x F --> F			Addition
(+)/2						F x I --> F			Addition
(+)/2						I x F --> F			Addition
(-)/2						I x I --> I			Subtraction
(-)/2						F x F --> F			Subtraction
(-)/2						F x I --> F			Subtraction
(-)/2						I x F --> F			Subtraction
(*)/2						I x I --> I			Multiplication
(*)/2						F x F --> F			Multiplication
(*)/2						F x I --> F			Multiplication
(*)/2						I x F --> F			Multiplication
(//)/2						I x I --> I			Integer Division
(/)/2						I x I --> F			Division
(/)/2						F x F --> F			Division
(/)/2						F x I --> F			Division
(/)/2						I x F --> F			Division
(rem)/2						I x I --> I			Remainder
(mod)/2						I x I --> I			Modulus
(-)/1						I --> I				Negation
(-)/1						F --> F				Negation
(abs)/1						I --> I				Absolute Value
(abs)/1						F --> F				Absolute Value
(sign)/1					I --> I				Sign
(sign)/1					F --> F				SIGN
(float_integer_part)/1		F --> I				integer part
(float_fractional_part)/1	F --> F				fractional part
(float)/1					I --> F				float coercion.
(float)/1					F --> F				float coercion.
(floor)/1					F --> I				floor.
(truncate)/1				F --> I				truncate.
(round)/1					F --> I				round.
(ceiling)/1					F --> I				ceiling.

Other arithmetic and Bitwise functors
Functor	Template(s)	 Name
(**)/2 '**'(int-exp, int-exp) = float
'**'(float-exp, int-exp) = float
'**'(int-exp, float-exp) = float
'**'(float-exp, float-exp) = float Power
sin/1	sin(float_exp) = float
sin(int_exp) = float sine
cos/1	cos(float_exp) = float
cos(int_exp) = float cosine
atan/1	atan(float_exp) = float
atan(int_exp) = float arc tangent
exp/1	exp(float_exp) = float
exp(int_exp) = float exponentiation
log/1	log(float_exp) = float
log(int_exp) = float log
sqrt/1	sqrt(float_exp) = float
sqrt(int_exp) = float square root
(>>)/2 '>>'(int_exp, int_exp) = integer bitwise right shift
(<<)/2 '<<'(int_exp, int_exp) = integer bitwise left shift
(/\)/2 '/\\'(int-exp, int-exp) = integer bit-wise and
(\/)/2 '\\/'(int-exp, int-exp) = integer bit-wise or
(\)/1 '\\'(int-exp) = integer bitwise complement
*/

static Pt ArithRound(Pt arg)
{ /* works even if ISOC99 is not available */
	OpsFloat pint ;
	PFloat pfrac = MATH(modf)(XAsFloat(arg), &pint) ;
	return MakeFloat(pfrac >= 0.5 ? pint + 1
					: pfrac <= -0.5 ? pint - 1 : pint) ;
}

static Pt ArithAbs(Pt arg)
{
	if( IsInt(arg) ) {
		PInt i = XInt(arg) ;
		return i >= 0 ? arg : MakeInt(-i) ;
	}
	else {
		PFloat f = XFloat(arg) ;
		return f >= 0 ? arg : MakeFloat(-f) ;
	}
}

static Pt ArithSign(Pt arg)
{
	PFloat f = XAsFloat(arg) ;
	return f == 0 ? zeroIntPt
		  : f > 0 ? oneIntPt
		  : minusOneIntPt ;
}

static Pt ArithIntPart(Pt arg)
{
	OpsFloat pint ;
	MATH(modf)(XAsFloat(arg), &pint) ;
	return MakeFloat(pint) ;
}

static Pt ArithFracPart(Pt arg)
{
	OpsFloat pint ;
	return MakeFloat(MATH(modf)(XAsFloat(arg), &pint)) ;
}

static Pt ArithIVar(Pt arg)
{
	Pt val ;
	if( IsAtom(arg) && (val = IVarGet(XAtom(arg))) != nil ) {
		if( IsNumber(val) )
			return val ;
		else
			return TypeError2("EVALUABLE", arg,
				"Contents of IVAR '%s' is not numeric", XAtomName(arg)) ;
	}
	else 
		return TypeError2("EVALUABLE", arg,
					"Unknown IVAR '%s' in expression", XAtomName(arg)) ;
}


static VoidPt TermArithError(Pt t)
{
	Str mesg, value ;
	if( IsAtom(t) ) {
		mesg = "Unknown ATOM '%s' in expression" ;
		value = XAtomName(t) ;
		t = MakeSlashTerm(XTermFunctor(t)) ;
	}
	elif( IsExtra(t) ) {
		mesg = "Unknown EXTRA '%s' in expression" ;
		value = XExtraAsStr(t) ;
	}
	elif( IsStruct(t) ) {
		mesg = "Unknown FUNCTOR '%s' in expression" ;
		value = XStructNameArity(t) ;
		t = MakeSlashTerm(XTermFunctor(t)) ;
	}
	elif( IsVar(t) ) {
		mesg = "Unbound VARIABLE in expression" ;
		value = "" ;
	}
	elif( IsList(t) ) {
		mesg = "Invalid LIST in expression" ;
		value = "" ;
	}
	else 	
		return InternalError("TermArithError") ;
	
	return TypeError2("EVALUABLE", t, mesg, value) ;
}


#define chk(s)	StrEqual(name, s)

static Pt EvalArith0(Pt t)
{
	Str name = XAtomName(t) ;
	switch( name[0] ) {
	  case 'c':	if( chk("cputime") ) return MakeFloat(CpuTime()) ;
				if( chk("currtime") ) return MakeLLInt(AbsoluteTime()) ;
	  case 'e':	if( chk("e") ) return eFloatPt ;
				if( chk("epsilon") ) return epsilonFloatPt ;
	  case 'f':	if( chk("float_size") ) return MakeInt(floatSize) ;
	  case 'g':	if( chk("global") ) return MakeInt(WordsAsBytes(LocalStackUsed())) ;
	  case 'h':	if( chk("heapused") )return MakeInt(WordsAsBytes(ProgramSpaceUsed())) ;
	  case 'i':	if( chk("int_size") ) return MakeInt(intSize) ;
				if( chk("inf") ) return infFloatPt ;
	  case 'l':	if( chk("local") ) return MakeInt(WordsAsBytes(GlobalStackUsed())) ;
	  case 'm':	if( chk("max_int") ) return maxIntPt ;
				if( chk("min_int") ) return MakeInt(minInt) ;
	  case 'n':	if( chk("nan") ) return nanFloatPt ;
	  case 'p':	if( chk("pi") ) return piFloatPt ;
	  case 'r':	if( chk("random") ) return MakeFloat(FloatRandom()) ;
	}
	return TermArithError(t) ;
}

static Pt EvalArith1(FunctorPt f, Pt a)
{
	Str name = FunctorName(f) ;
	switch( name[0] ) {
	  case '-':	if( chk("-") ) {
					if( IsInt(a) ) return MakeInt(-XInt(a)) ;
					else return MakeFloat(-XAsFloat(a)) ;
				}
	 case '\\':	if( chk("\\") ) {
					if( IsInt(a) ) return MakeInt(~XInt(a)) ;
					else TypeError2("INT", a,
							"The arg of '\\/1' must be an integer") ;
				}
	  case 'a':	if( chk("asin") ) return MakeFloat(MATH(asin)(XAsFloat(a))) ;
				if( chk("acos") ) return MakeFloat(MATH(acos)(XAsFloat(a))) ;
				if( chk("atan") ) return MakeFloat(MATH(atan)(XAsFloat(a))) ;
				if( chk("abs") ) return ArithAbs(a) ;
	  case 'c':	if( chk("cos") ) return MakeFloat(MATH(cos)(XAsFloat(a))) ;
				if( chk("ceiling") ) return MakeFloat(MATH(ceil)(XAsFloat(a))) ;
	  case 'e':	if( chk("exp") ) return MakeFloat(MATH(exp)(XAsFloat(a))) ;
	  case 'f':	if( chk("floor") ) return MakeFloat(MATH(floor)(XAsFloat(a))) ;
				if( chk("float") ) return a ;
				if( chk("float_integer_part") ) return ArithIntPart(a) ;
				if( chk("float_fractional_part") ) return ArithFracPart(a) ;
	  case 'i':	if( chk("integer") ) return ArithRound(a) ;
	  case 'l':	if( chk("log") ) return MakeFloat(MATH(log)(XAsFloat(a))) ;
				if( chk("log10") ) return MakeFloat(MATH(log10)(XAsFloat(a))) ;
	  case 'r':	if( chk("random") ) {
					if( IsPos(a) ) return MakeInt(IntRandom(XInt(a))) ;
					else TypeError2("INT", a, "'random/1' expected a positive integer") ;
				}
				if( chk("round") ) return ArithRound(a) ;
	  case 's':	if( chk("sqrt") ) return MakeFloat(MATH(sqrt)(XAsFloat(a))) ;
				if( chk("sin") ) return MakeFloat(MATH(sin)(XAsFloat(a))) ;
				if( chk("sign") ) return ArithSign(a) ;
	  case 't':	if( chk("tan") ) return MakeFloat(MATH(tan)(XAsFloat(a))) ;
				if( chk("truncate") ) return ArithIntPart(a) ;
	}
	return TermArithError(MakeUnStruct(f, a)) ;
}

static Pt EvalArith2(FunctorPt f, Pt a1, Pt a2)
{
	Str name = FunctorName(f) ;
	switch( name[0] ) {
	  case '+':	if( chk("+") ) return MakeFloat(XAsFloat(a1) + XAsFloat(a2)) ;
	  case '-':	if( chk("-") ) return MakeFloat(XAsFloat(a1) - XAsFloat(a2)) ;
	  case '*':	if( chk("*") ) return MakeFloat(XAsFloat(a1) * XAsFloat(a2)) ;
				if( chk("**") ) return MakeFloat(pow(XAsFloat(a1), XAsFloat(a2))) ;
	  case '/':	if( chk("/") ) return MakeFloat(XAsFloat(a1) / XAsFloat(a2)) ;
				if( chk("//") ) {
					if( IsInt(a1) && IsInt(a2) ) {
						int den = XInt(a2) ;
						if( den == 0 )
							EvaluationError("zero_divisor", "Divisor is zero") ;
						return MakeInt(XInt(a1) / den) ;
					}
					else TypeError2("INT", IsInt(a1)?a2:a1,
									"Both args of '//2' must be integers") ;
				}
				if( chk("/\\") ) {
					if( IsInt(a1) && IsInt(a2) )
						return MakeInt(XInt(a1) & XInt(a2)) ;
					else TypeError2("INT", IsInt(a1)?a2:a1,
									"Both args of '/\\/2' must be integers") ;
				}
	 case '\\':	if( chk("\\/") ) {
					if( IsInt(a1) && IsInt(a2) )
						return MakeInt(XInt(a1) | XInt(a2)) ;
					else TypeError2("INT", IsInt(a1)?a2:a1,
									"Both args of '/\\//2' must be integers") ;
				}
	  case '^':	if( chk("^") )
					return MakeFloat(pow(XAsFloat(a1), XAsFloat(a2))) ;
	  case '>':	if( chk(">>") ) {
					if( IsInt(a1) && IsInt(a2) )
						return MakeInt(XInt(a1) >> XInt(a2)) ;
					else TypeError2("INT", IsInt(a1)?a2:a1,
									"Both args of '>>/2' must be integers") ;
				}
	  case '<':	if( chk("<<") ) {
					if( IsInt(a1) && IsInt(a2) )
						return MakeInt(XInt(a1) << XInt(a2)) ;
					else TypeError2("INT", IsInt(a1)?a2:a1,
									"Both args of '<</2' must be integers") ;
				}
	  case 'e':	if( chk("exp") )
					return MakeFloat(pow(XAsFloat(a1), XAsFloat(a2))) ;
	  case 'm':	if( chk("mod") ) {
					if( IsInt(a1) && IsInt(a2) ) {
						PInt n, i2, fix ;
						if( (i2 = XInt(a2)) == 0 )
							EvaluationError("zero_divisor", "Divisor is zero") ;
						n = XInt(a1) % i2 ;		/* result has sign of dividend */
						fix = n == 0 || (n > 0) == (i2 > 0) ? 0 : i2 ;
					/* result has sign of divisor */
						return MakeInt(n + fix) ;
					}
					else TypeError2("INT", IsInt(a1)?a2:a1,
									"Both args of 'mod/2' must be integers") ;
				}
				if( chk("max") )
					return XAsFloat(a1) >= XAsFloat(a2) ? a1 : a2 ;
				if( chk("min") )
					return XAsFloat(a1) <= XAsFloat(a2) ? a1 : a2 ;
	  case 'r':	if( chk("rem") ) {
					if( IsInt(a1) && IsInt(a2) ) {
						PInt i2 ;
						if( (i2 = XInt(a2)) == 0 )
							EvaluationError("zero_divisor", "Divisor is zero") ;
						return MakeInt(XInt(a1) % i2) ;						
					}
					else TypeError2("INT", IsInt(a1)?a2:a1,
									"Both args of 'rem/2' must be integers") ;
				}
	}
	return TermArithError(MakeBinStruct(f, a1, a2)) ;
}

static Pt EvalPre(register Pt t)
{
redo:
	VarValue(t) ;
	if( IsNumber(t) ) return t ;
	if( IsStruct(t) )
		return IsUnitParam(t) ? Z(XUnitParam(t))
			 : IsThisStruct(t, iVarFunctor) ? ArithIVar(XStructArg(t, 0))
			 : t ;
	if( IsAtom(t) ) return EvalArith0(t) ;
	if( IsList(t) && compatibleIfThen_flag && Drf(XListTail(t)) == tNilAtom ) {
		t = XListHead(t) ;
		goto redo ;
	}
	return TermArithError(t) ;
}




/* MAIN OPERATIONS */

Bool ExtendedPrecisionMathFunctions()
{
#if HAS_LONG_DOUBLE_OPS
	return true ;
#else
	return false ;
#endif
}

Bool IsIntFloat(PFloat f)
{
	return f == MATH(floor)(f) ;
}

Pt Evaluate(register Pt t)
{
	UseScratch() ;
	ScratchPush(EvalPre(t)) ;
	for(;;) {
		if( IsStruct(ScratchTop()) ) {
			Pt e = EvalPre(XStructArg(ScratchTop(),0)) ;
			ScratchPush(e) ;
		}

		elif( ScratchUsed() == 1 ) break ;

		elif( IsStruct(ScratchXTop(1)) )
		  switch( XStructArity(ScratchXTop(1)) ) {
			case 1: {
				Pt arg = ScratchPop() ;
				ScratchTop() =
					EvalArith1(XStructFunctor(ScratchTop()), arg) ;
				break ;
			}
			case 2: {
				Pt e = EvalPre(XStructArg(ScratchXTop(1),1)) ;
				ScratchPush(e) ;
				break ;
			}
			default: TermArithError(ScratchXTop(1)) ;
		  }

		else {
			Pt arg2 = ScratchPop() ;
			Pt arg1 = ScratchPop() ;
			ScratchTop() =
				EvalArith2(XStructFunctor(ScratchTop()), arg1, arg2) ;
		}
	}
	t = ScratchPop() ;
	FreeScratch() ;
	return t ;
}


/* CXPROLOG C'BUILTINS */

static void PIs()
{
	Pt t = Drf(X0) ;
	MustBe( UnifyWithNumber(t, Evaluate(X1)) ) ;
}

static void PEq()
{
	MustBe( CompareNumber(Evaluate(X0),Evaluate(X1)) == 0 ) ;
}

static void PNe()
{
	MustBe( CompareNumber(Evaluate(X0),Evaluate(X1)) != 0 ) ;
}

static void PLt()
{
	MustBe( CompareNumber(Evaluate(X0),Evaluate(X1)) == -1 ) ;
}

static void PGt()
{
	MustBe( CompareNumber(Evaluate(X0),Evaluate(X1)) == 1 ) ;
}

static void PLe()
{
	int r = CompareNumber(Evaluate(X0),Evaluate(X1)) ;
	MustBe( r == -1 || r == 0 ) ; /* supports inf and nan */
}

static void PGe()
{
	int r = CompareNumber(Evaluate(X0),Evaluate(X1)) ;
	MustBe( r == 0 || r == 1 ) ; /* supports inf and nan */
}

static void PSucc()
{
	X0 = TestNatOrVar(X0) ;
	X1 = TestNatOrVar(X1) ;
	if( IsInt(X0) ) {
		if( X0 == maxIntPt )
			DomainError("INT<MAX_INT", X0) ;
		MustBe( UnifyWithNumber(X1, IncIntPt(X0)) ) ;
	}
	if( IsInt(X1) )
		MustBe( X1 != zeroIntPt && UnifyWithNumber(X0, DecIntPt(X1)) ) ;
	TypeError("INT>=0",X0) ;
}

static void PPlus()
{
	X0 = TestIntOrVar(X0) ;
	X1 = TestIntOrVar(X1) ;
	X2 = TestIntOrVar(X2) ;
	if( IsInt(X0) && IsInt(X1) )
		MustBe( UnifyWithNumber(X2, MakeInt(XInt(X0) + XInt(X1))) ) ;
	elif( IsInt(X0) && IsInt(X2) )
		MustBe( UnifyWithNumber(X1, MakeInt(XInt(X2) - XInt(X0))) ) ;
	elif( IsInt(X1) && IsInt(X2) )
		MustBe( UnifyWithNumber(X0, MakeInt(XInt(X2) - XInt(X1))) ) ;
	TypeError2("INT", X0, "Arguments are insufficiently instantiated") ;
}

static void PNDBetween()
{
	if( A(3) == tNilAtom ) {
		A(0) = MakeInt(XTestInt(A(0))) ;
		A(1) = Drf(A(1)) ;
		if( !IsInt(A(1)) && (A(1) == infFloatPt || A(1) == MakeAtom("inf")) )
			A(1) = maxIntPt ;
		A(1) = MakeInt(XTestInt(A(1))) ;
		A(2) = Drf(A(2)) ;
		if( IsInt(A(2)) ) {
			Bool b = CompareNumber(A(0), A(2)) <= 0
					&& CompareNumber(A(2), A(1)) <= 0 ;
			Discard() ;
			MustBe( b ) ;	
		}
		elif( !IsVar(A(2)) )
			XTestInt(A(2)) ;
		A(3) = DecIntPt(A(0)) ;
	}
	if( A(3) == A(1) ) Jump(DiscardAndFail) ;
	A(3) = IncIntPt(A(3)) ;
	if( UnifyWithNumber(X2, A(3)) ) JumpNext() ;
	InternalError("PNDBetween") ;
}



/* INIT */

void ArithInit()
{
	InstallCBuiltinPred("is", 2, PIs) ;
	InstallCBuiltinPred("=:=", 2, PEq) ;
	InstallCBuiltinPred("=\\=", 2, PNe) ;	/* Use of escape '\' */
	InstallCBuiltinPred("<", 2, PLt) ;
	InstallCBuiltinPred(">", 2, PGt) ;
	InstallCBuiltinPred("=<", 2, PLe) ;
	InstallCBuiltinPred(">=", 2, PGe) ;
	InstallCBuiltinPred("succ", 2, PSucc) ;
	InstallCBuiltinPred("plus", 3, PPlus) ;
	InstallNDeterCBuiltinPred("between", 3, PNDBetween) ;
}
