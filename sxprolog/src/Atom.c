/*
 *   This file is part of the CxProlog system

 *   Atom.c
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

/*
 * PERMANENT/TEMPORARY ATOM RULES:
 * Atoms occurring in source code (clauses) are kept as 'permanent atoms'.
 * Ivar names are kept as 'permanent atoms'.
 * Unit names are kept as 'permanent atoms'.
 * Operator names are kept as 'permanent atoms'.
 * Predicate names are kept as 'permanent atoms'.
 * All other atoms are kept as 'temporary atoms'.
 *
 * There is a garbage collector of temporary atoms that safely recycles
 * again and again the space taken by transient text.
 */

#include "CxProlog.h"


/* ATOMS */

#define atomHashTableCapacity		512

static ExtraTypePt atomType ;

Pt	tNilAtom, tEmptyAtom, tEofAtom, tCutAtom,
	tFalseAtom, tTrueAtom, tAbsentAtom,
	tFailAtom, tOnAtom, tOffAtom, tMinusAtom, tUnderAtom,
	tDotAtom, tBracketsAtom, tQuestionAtom, tEllispisAtom,
	tLessAtom, tEqualAtom, tGreaterAtom,
	tFileAtom, tDirAtom, tErrorAtom, tGoingAtom,
	tKilledAtom, tCompletedAtom, tFailedAtom, tMainAtom,
	tBinaryAtom, tTextAtom, tOctetAtom, tEofCodeAtom, tResetAtom,
	tVoidAtom, tNullAtom, tNoResult, tMarkAtom,
	tVarAtom, tCyclicAtom, tBadTermAtom,
	tExceptionAtom, tMessageAtom, tHookAtom,
	tUserAtom, tUserInputAtom, tUserOutputAtom, tUserErrorAtom,
	tReadAtom, tWriteAtom, tAppendAtom ;

#define AtomWordsLen(len)	(WordsOf(Atom) + Words((len) + 1))
#define AtomWords(name)		AtomWordsLen(strlen(name))

static Bool LookupTempAtomAux(CVoidPt x, CVoidPt ref)
{
	return StrEqual(AtomName(x), cCharPt(ref)) ;
}
AtomPt LookupTempAtom(Str name)
{
	int slot = StrHash(name) % atomHashTableCapacity ;	/* hash function */
	register AtomPt a ;
	if( (a = ExtraFindFirst(atomType, slot, LookupTempAtomAux, name)) == nil ) {
		a = ExtraNewWithSize(atomType, AtomWords(name) + 2, slot) ;
		AtomFunctors(a) = nil ;
		AtomToUnit(a) = nil ;
		AtomToAlias(a) = nil ;
		AtomToIVar(a) = nil ;
		AtomToOperator(a) = nil ;
		strcpy(AtomName(a), name) ;
	}
	return a ;
}

AtomPt LookupAtom(Str name)
{
	AtomPt a = LookupTempAtom(name) ;
	ExtraPermanent(a) ;
	return a ;
}

static Size _len = 0 ;
static Bool LookupTempAtomLenAux(CVoidPt x, CVoidPt ref)
{
	return AtomName(x)[_len] == '\0' && StrEqualN(AtomName(x), cCharPt(ref), _len) ;
}
static AtomPt LookupTempAtomLen(Str name, Size len) /* ascii len */
{
	int slot = StrHashLen(name, len) % atomHashTableCapacity ;	/* hash function */
	register AtomPt a ;
	_len = len ;
	if( (a = ExtraFindFirst(atomType, slot, LookupTempAtomLenAux, name)) == nil ) {
		a = ExtraNewWithSize(atomType, AtomWordsLen(len) + 2, slot) ;
		AtomFunctors(a) = nil ;
		AtomToUnit(a) = nil ;
		AtomToAlias(a) = nil ;
		AtomToIVar(a) = nil ;
		AtomToOperator(a) = nil ;
		strncpy(AtomName(a), name, len) ;
		AtomName(a)[len] = '\0' ;
	}
	return a ;
}


Size AtomForEach(ExtraFun fun)
{
	return ExtraForEach(atomType, fun) ;
}

Size ForEachInSpec(Pt t, FunctorFun fun, Bool allowAtoms)
{
	Size n = 0 ;
	t = Drf(t) ;
	if( allowAtoms && IsAtom(t) ) {
		if( t == tNilAtom ) ;
		else {
			FunctorPt f ;
			doseq(f, AtomFunctors(XAtom(t)), f->nextArity)
				n += fun(f) ;
		}
	}
	elif( IsThisStruct(t, slashFunctor) )
		n += fun(XTestSlash(t)) ;
	elif( IsThisStruct(t, commaFunctor) ) {
		n += ForEachInSpec(XStructArg(t,0), fun, allowAtoms) ;
		n += ForEachInSpec(XStructArg(t,1), fun, allowAtoms) ;
	}
	elif( IsList(t) ) {
		for( ; IsList(t) ; t = Drf(XListTail(t)) )
			n += ForEachInSpec(XListHead(t), fun, allowAtoms) ;
		if( t != tNilAtom )
			TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	}
	else Error("Invalid predicate indicator") ;
	return n ;
}

static void PNDCurrentAtom()
{
	ExtraPNDCurrent(atomType, nil, 1, 0) ;
	JumpNext() ;
}

static Size AtomSizeFun(CVoidPt x)
{
	if( x == nil )
		InternalError("AtomSizeFun") ;
	return AtomWords(AtomName(x)) ;
}

static Bool AtomBasicGCDelete(VoidPt x)
{
	Unused(x) ;
	return true ;
}

static Pt MakeTempAtomLen(Str name, Size len) /* ascii len */
{
	return TagAtom(LookupTempAtomLen(name, len)) ;
}

static Pt MakeTempAtomSlice(Str s, Size a, Size b) /* [a,b[  unicode positions */
{
	CharPt ss = cCharPt(s) ;
	CharPt start = CharPos(ss, a) ;
	CharPt end = CharPos(start, b-a) ;
	return MakeTempAtomLen(start, end - start) ;
}


/* FUNCTORS */

FunctorPt commaFunctor, semicolonFunctor, arrowFunctor, neckFunctor,
		commandFunctor, slashFunctor, colonFunctor, listFunctor,
		stringFunctor, metaCutFunctor, eqFunctor, barFunctor, minusFunctor,
		hifenFunctor, plusFunctor, bracketsFunctor, parFunctor, varFunctor,
		primitiveFunctor, unitParamFunctor, emptyFunctor, errorFunctor,
		infoFunctor, onceFunctor, tryFunctor, callFunctor,
		ctxPushFunctor, ctxSwitchFunctor, ctxHEnterFunctor, ctxHExitFunctor,
		nilIsSpecialFunctor, iVarFunctor, streamPositionFunctor ;

static Bool FunctorCheckAux(CVoidPt x, CVoidPt fref)
{
	AtomPt a = cAtomPt(x) ;
	register FunctorPt f ;
	doseq(f, AtomFunctors(a), f->nextArity)
		if( fref == f ) return true ;
	return false ;
}
Bool FunctorCheck(CVoidPt ref)
{
	return ExtraFindFirst(atomType, -1, FunctorCheckAux, ref) != nil ;
}

FunctorPt FindFunctor(AtomPt atom, int arity)
{
	register FunctorPt f ;
	doseq(f, AtomFunctors(atom), f->nextArity)
		if( FunctorArity(f) == arity ) return f ;
	return nil ;
}

FunctorPt LookupFunctor(AtomPt atom, int arity)
{
	register FunctorPt f ;
/* Find functor */
	doseq(f, AtomFunctors(atom), f->nextArity)
		if( FunctorArity(f) == arity ) return f ;
/* New functor */
	if( arity > maxFunctorArity )
		RepresentationError("max_arity", MakeInt(maxFunctorArity),
						"Highest arity (%d) exceeded on functor '%s/%d'",
						maxFunctorArity, AtomName(atom), arity) ;
	f = Allocate(WordsOf(Functor), false) ;
	FunctorAtom(f) = atom ;
	FunctorArity(f) = arity ;
	FunctorPreds(f) = nil ;
	FunctorIsBuiltin(f) = false ;
	FunctorIsMeta(f) = false ;
/*	FunctorIsInLine(f) = false ; */
	FunctorIsSpy(f) = false ;
/* Update functor atom */
	ExtraPermanent(atom) ;
	f->nextArity = AtomFunctors(atom) ;
	AtomFunctors(atom) = f ;
	return f ;
}

FunctorPt FindFunctorByName(Str name, int arity)
{
	return FindFunctor(LookupAtom(name), arity) ;
}

FunctorPt LookupFunctorByName(Str name, int arity)
{
	return LookupFunctor(LookupAtom(name), arity) ;
}

Str FunctorNameArity(FunctorPt f)
{
	return GStrFormat("%s/%d", FunctorName(f), FunctorArity(f)) ;
}

FunctorFun funaux ;
static Size ForEachFunctorAux(CVoidPt x)
{
	AtomPt a = cAtomPt(x) ;
	Size n = 0 ;
	register FunctorPt f ;
	doseq(f, AtomFunctors(a), f->nextArity)
		n += funaux(f) ;
	return n ;
}
Size ForEachFunctor(FunctorFun fun)
{
	funaux = fun ;
	return ExtraForEach(atomType, ForEachFunctorAux) ;
}

Size SpyOff(FunctorPt f)
{
	if( FunctorIsSpy(f) ) {
		WriteStd("%% Spy point removed from %s.\n", FunctorNameArity(f)) ;
		FunctorIsSpy(f) = false ;
		return 1 ;
	}
	return 0 ;
}

Size SpyOn(FunctorPt f)
{
	if( FunctorIsBuiltin(f) ) {
		 WriteStd("%% Cannot place spy point on built-in '%s'.\n",
										FunctorNameArity(f)) ;
		return 0 ;
	}
	if( !FunctorIsSpy(f) ) {
		WriteStd("%% Spy point on %s.\n", FunctorNameArity(f)) ;
		FunctorIsSpy(f) = true ;
		return 1 ;
	}
	return 0 ;
}

void NoSpyAll()
{
	ForEachFunctor(SpyOff) ;
}

static Size WriteFunctorSpyPoint(FunctorPt f)
{
	if( FunctorIsSpy(f) ) {
		WriteStd("%%           %s\n", FunctorNameArity(f)) ;
		return 1 ;
	}
	else return 0 ;
}
void WriteSpyPoints()
{
	WriteStd("%% Spy points on:\n") ;
	if( ForEachFunctor(WriteFunctorSpyPoint) == 0 )
		WriteStd("%%        %% none %%\n") ;
}

void FunctorsInit()
{
	commaFunctor = LookupFunctorByName(",", 2) ;
	semicolonFunctor = LookupFunctorByName(";", 2) ;
	arrowFunctor = LookupFunctorByName("->", 2) ;
	neckFunctor = LookupFunctorByName(":-", 2) ;
	commandFunctor = LookupFunctorByName(":-", 1) ;
	slashFunctor = LookupFunctorByName("/", 2) ;
	colonFunctor = LookupFunctorByName(":", 2) ;
	listFunctor = LookupFunctorByName(".", 2) ;
	stringFunctor = LookupFunctorByName("\"\"", 1) ;
	metaCutFunctor = LookupFunctorByName("$$_meta_cut", 1) ;
	eqFunctor = LookupFunctorByName("=", 2) ;
	barFunctor = LookupFunctorByName("|", 2) ;
	minusFunctor = LookupFunctorByName("-", 1) ;
	hifenFunctor = LookupFunctorByName("-", 2) ;
	plusFunctor = LookupFunctorByName("+", 1) ;
	bracketsFunctor = LookupFunctorByName("{}", 1) ;
	parFunctor = LookupFunctorByName("$PAR", 1) ;
	varFunctor = LookupFunctorByName("$VAR", 1) ;
	primitiveFunctor = LookupFunctorByName("$$_primitive", 1) ;
	unitParamFunctor = LookupFunctorByName("$$_unit_parameter", 1) ;
	emptyFunctor = LookupFunctorByName("", 0) ;
	errorFunctor = LookupFunctorByName("error", 2) ;
	infoFunctor = LookupFunctorByName("info", 2) ;
	onceFunctor = LookupFunctorByName("once", 1) ;
	tryFunctor = LookupFunctorByName("try", 1) ;
	callFunctor = LookupFunctorByName("call", 1) ;
	ctxPushFunctor = LookupFunctorByName(">>", 2) ;
	ctxSwitchFunctor = LookupFunctorByName("<>", 2) ;
	ctxHEnterFunctor = LookupFunctorByName(">", 1) ;
	ctxHExitFunctor = LookupFunctorByName("<", 1) ;
	nilIsSpecialFunctor = LookupFunctorByName("$$_[]", 0) ;
	iVarFunctor = LookupFunctorByName("ivar", 1) ;
	streamPositionFunctor = LookupFunctorByName("$stream_position", 4) ;
	
}


/* CXPROLOG C'BUILTINS */

static Size AtomsAux(CVoidPt x)
{
	Write("%s", AtomName(x)) ;
	return 1 ;
}
static void PAtoms()
{
	ExtraShow(atomType, AtomsAux) ;
	JumpNext() ;
}

static void PAtomLength()
{
	MustBe( UnifyWithNumber(TestIntOrVar(X1),
						MakeInt(CharLen(XTestAtomName(X0)))) ) ;
}

static void PAtomConcat()
{
	if( A(3) == tNilAtom ) {			/* init */
		X0 = TestAtomOrVar(X0) ;
		X1 = TestAtomOrVar(X1) ;
		if( IsAtom(X0) ) {
			if( IsAtom(X1) ) {					/* ATOM-ATOM handle */
				TestAtomOrVar(X2) ;
				BigStrOpen() ;
				BigStrAddStr(XAtomName(X0)) ;
				BigStrAddStr(XAtomName(X1)) ;
				Discard() ;
				MustBe( UnifyWithAtomic(X2, MakeTempAtom(BigStrClose())) ) ;
			}
			else {								/* ATOM-VAR handle */
				Str s0 = XAtomName(X0) ;
				Str s2 = XTestAtomName(X2) ;
				Size l0 = strlen(s0) ;		/* not CharLen! */
				Size l2 = strlen(s2) ;		/* not CharLen! */
				Discard() ;
				MustBe( l0 <= l2
					&& strncmp(s0, s2, l0) == 0
					&& UnifyWithAtomic(X1, MakeTempAtom(s2 + l0)) ) ;
			}
		}
		else {
			if( IsAtom(X1) ) {					/* VAR-ATOM handle */
				Str s1 = XAtomName(X1) ;
				Str s2 = XTestAtomName(X2) ;
				Size l1 = strlen(s1) ;		/* not CharLen! */
				Size l2 = strlen(s2) ;		/* not CharLen! */
				Discard() ;
				MustBe( l1 <= l2
					&& strcmp(s1, s2 + l2 - l1) == 0
					&& UnifyWithAtomic(X0, MakeTempAtomLen(s2, l2 - l1)) ) ;
			}
			elif( IsVar(X1) ) {					/* VAR-VAR init */
				A(2) = TestAtom(X2) ;
				A(3) = cPt(XAtomName(A(2))) ;
			}
		}
	}

	{											/* VAR-VAR handle */
		Str s2 = XAtomName(A(2)) ;			/* already validated */
		CharPt pos = cCharPt(A(3)) ;
		if( *pos == '\0' )
			Discard() ;
		else
			A(3) = cPt(CharNxt(pos)) ;
		MustBe( UnifyWithAtomic(X0, MakeTempAtomLen(s2, pos - s2))
			&&	UnifyWithAtomic(X1, MakeTempAtom(pos)) ) ;
	}
}

static void PSubAtom()
{
	PInt total, before, lenght, after ;

	if( A(5) == tNilAtom ) {			/* init */
		total = CharLen(XTestAtomName(X0)) ;
		X1 = TestNatOrVar(X1) ;
		X2 = TestNatOrVar(X2) ;
		X3 = TestNatOrVar(X3) ;
		X4 = TestAtomOrVar(X4) ;
		A(8) = MakeInt(total) ;
		if( IsInt(X1) ) {
			if( IsInt(X2) ) {						/* INT-INT-*** handle */
				before = XInt(X1) ;
				lenght = XInt(X2) ;
				after = total - before - lenght ;
				Discard() ;
				goto unify ;
			}
			elif( IsInt(X3) ) {						/* INT-VAR-INT handle */
				before = XInt(X1) ;
				after = XInt(X3) ;
				lenght = total - before - after ;
				Discard() ;
				goto unify ;
			}
			else {									/* INT-VAR-VAR init */
				A(5) = zeroIntPt ;
				A(6) = zeroIntPt ;
			}
		}
		elif( IsInt(X2) ) {
			if( IsInt(X3) ) {						/* VAR-INT-INT handle */
				lenght = XInt(X2) ;
				after = XInt(X3) ;
				before = total - lenght - after ;
				Discard() ;
				goto unify ;
			}
			else {							/* VAR-INT-VAR init */
				A(5) = oneIntPt ;
				A(6) = zeroIntPt ;
			}
		}
		elif( IsInt(X3) ) {					/* VAR-VAR-INT init */
			A(5) = twoIntPt ;
			A(6) = zeroIntPt ;
		}
		else {								/* VAR-VAR-VAR init */
			A(5) = threeIntPt ;
			A(6) = A(7) = zeroIntPt ;
		}
	}

	total = XInt(A(8)) ;
	if( A(5) == zeroIntPt ) {						/* INT-VAR-VAR handle */
		before = XInt(X1) ;
		lenght = XInt(A(6)) ;
		after = total - before - lenght ;
		if( after == 0 )
			Discard() ;
		else
			A(6) = IncIntPt(A(6)) ;
	}
	elif( A(5) == oneIntPt ) {						/* VAR-INT-VAR handle */
		before = XInt(A(6)) ;
		lenght = XInt(X2) ;
		after = total - before - lenght ;
		if( after == 0 )
			Discard() ;
		else
			A(6) = IncIntPt(A(6)) ;
	}
	elif( A(5) == twoIntPt ) {						/* VAR-VAR-INT handle */
		before = XInt(A(6)) ;
		after = XInt(X3) ;
		lenght = total - before - after ;
		if( lenght == 0 )
			Discard() ;
		else
			A(6) = IncIntPt(A(6)) ;
	}
	elif( A(5) == threeIntPt ) {					/* VAR-VAR-VAR handle */
		before = XInt(A(6)) ;
		lenght = XInt(A(7)) ;
		after = total - before - lenght ;
		if( A(6) == A(8) )
			Discard() ;
		else {
			if( before + lenght < total )
				A(7) = IncIntPt(A(7)) ;
			else { A(6) = IncIntPt(A(6)) ; A(7) = zeroIntPt ; }
		}
	}
	else {		/* only to shut up a warning */
		before = lenght = after = 0 ;
	}

unify:
	MustBe(	InRange(before, 0, total) && UnifyWithNumber(X1, MakeInt(before))
		&&	InRange(lenght, 0, total) && UnifyWithNumber(X2, MakeInt(lenght))
		&&	InRange(after, 0, total)  && UnifyWithNumber(X3, MakeInt(after))
		&&	Unify(X4, MakeTempAtomSlice(XTestAtomName(X0), before, before+lenght)) ) ;
				/* Need to use XTestAtomName(X0) to ensure dereferentiation */
}

void AtomsInit(void)
{
	register int i ;
	Str4 s ;
	atomType = ExtraTypeNew("ATOM", AtomSizeFun, nil, AtomBasicGCDelete, atomHashTableCapacity) ;
	ExtraTypeDoesNotSupportAliasing(atomType) ;
	tNilAtom = MakeAtom("[]") ;
	tEmptyAtom = MakeAtom("") ;
	tEofAtom = MakeAtom("end_of_file") ;
	tCutAtom = MakeAtom("!") ;
	tFalseAtom = MakeAtom("false") ;
	tTrueAtom = MakeAtom("true") ;
	tAbsentAtom = MakeAtom("absent") ;
	tFailAtom = MakeAtom("fail") ;
	tOnAtom = MakeAtom("on") ;
	tOffAtom = MakeAtom("off") ;
	tMinusAtom = MakeAtom("-") ;
	tUnderAtom = MakeAtom("_") ;
	tDotAtom = MakeAtom(".") ;
	tBracketsAtom = MakeAtom("{}") ;
	tQuestionAtom = MakeAtom("?") ;
	tEllispisAtom = MakeAtom("...") ;
	tLessAtom = MakeAtom("<") ;
	tEqualAtom = MakeAtom("=") ;
	tGreaterAtom = MakeAtom(">") ;
	tFileAtom = MakeAtom("file") ;
	tDirAtom = MakeAtom("dir") ;
	tErrorAtom = MakeAtom("error") ;
	tGoingAtom = MakeAtom("going") ;
	tKilledAtom = MakeAtom("killed") ;
	tCompletedAtom = MakeAtom("completed") ;
	tFailedAtom = MakeAtom("failed") ;
	tMainAtom = MakeAtom("main") ;

	tBinaryAtom = MakeAtom("binary") ;
	tTextAtom = MakeAtom("text") ;
	tOctetAtom = MakeAtom("octet") ;
	tEofCodeAtom = MakeAtom("eof_code") ;
	tResetAtom = MakeAtom("reset") ;

	tVoidAtom = MakeAtom("void") ;
	tNullAtom = MakeAtom("null") ;
	tNoResult = MakeAtom("$no_result") ;
	tMarkAtom = MakeAtom("$$_MARK") ;
	tVarAtom = MakeAtom("$$_VAR") ;
	tCyclicAtom = MakeAtom("%%_CYCLIC_%%") ;
	tBadTermAtom = MakeAtom("*BADTERM*") ;
	tExceptionAtom = MakeAtom("exception") ;
	tMessageAtom = MakeAtom("message") ;
	tHookAtom = MakeAtom("hook") ;
	
	tUserAtom = MakeAtom("user") ;
	tUserInputAtom = MakeAtom("user_input") ;
	tUserOutputAtom = MakeAtom("user_output") ;
	tUserErrorAtom = MakeAtom("user_error") ;
	tReadAtom = MakeAtom("read") ;
	tWriteAtom = MakeAtom("write") ;
	tAppendAtom = MakeAtom("append") ;

	s[1] = '\0' ;
	for( i = 'A' ; i <= 'Z' ; i++ ) {
		s[0] = i ; Ignore(MakeAtom(s)) ;
	}
	for( i = '0' ; i <= '9' ; i++ ) {
		s[0] = i ; Ignore(MakeAtom(s)) ;
	}
}

void AtomsInit2()
{
	InstallCBuiltinPred("atoms", 0, PAtoms) ;
	InstallGNDeterCBuiltinPred("current_atom", 1, 2, PNDCurrentAtom) ;
	InstallCBuiltinPred("atom_length", 2, PAtomLength) ;
	InstallGNDeterCBuiltinPred("atom_concat", 3, 1, PAtomConcat) ;
	InstallGNDeterCBuiltinPred("sub_atom", 5, 4, PSubAtom) ;
}
