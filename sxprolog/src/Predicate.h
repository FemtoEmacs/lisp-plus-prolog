/*
 *   This file is part of the CxProlog system

 *   Predicate.h
 *   by A.Miguel Dias - 1989/11/14
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Predicates_
#define _Predicates_

typedef enum PredOnError {
	POE_EXCEPTION, POE_MESSAGE, POE_FAIL, POE_HOOK
} PredOnError ;

typedef struct Predicate {
	Inst startInst ;				/* Starting segment of code */
	Pt args[4] ;					/* Args 0, 1, 2 and 3	*/
	ClausePt clauses ;				/* Clause list (it's also the argument 4) */
	FunctorPt functor ;				/* Predicate functor (it's also the argument 5) */
	ClausePt lastClause ;			/* Speeds up assertz/1 */
	Hdl index ;						/* Predicate index. nil if no index */

	struct Predicate *nextF ;		/* Next predicate in functor list */

	struct Unit *unit ;			/* Predicate unit */
	struct Predicate *nextU ;		/* Next predicate in unit */
	AtomPt consultFile ;			/* Predicate consult filename */

	Word16 consultGen : 16 ;		/* Predicate consult generation */
	unsigned int nIndexable : 2 ;	/* Predicate number of indexable arguments */
	Bool dynamic : 1 ;				/* Predicate is dynamic */
	Bool multifile : 1 ;			/* Predicate is multifile  */
	Bool discontiguous : 1 ;		/* Predicate is discontiguous */
	Bool visible : 1 ;				/* Predicate is visible in its unit */
	Bool gopen : 1 ;				/* Predicate is gopen in its unit (CTX2) */
	Bool xover : 1 ;				/* Predicate is xover (CTX4) */
	Bool hasDynamicChain : 1 ;		/* Predicate has dynamic clause chain */
	Bool logical : 1 ;				/* Logical or immediate update semantic view */
	Bool keepSource : 1 ;			/* Keep the source for its clauses  */
	Bool permanent : 1 ;			/* Predicate is permanent (static built-in) */
	Bool traceable : 1 ;			/* Predicate is traceable */
	Bool core : 1 ;					/* Predicate is core */
	Bool abolished : 1 ;			/* Predicate has been abolished */
	PredOnError onError : 2 ;		/* Predicate handles error, how? */
} Predicate, *PredicatePt ;

#define cPredicatePt(x)				( (PredicatePt)(x) )

#define PredStartInst(pr)			( (pr)->startInst )
#define PredStartInstArgs(pr)		( (pr)->args )
#define PredCode(pr)				( cHdl(&PredStartInst(pr)) )
#define PredIndex(pr)				( (pr)->index )

#define PredNextF(pr)				( (pr)->nextF )
#define PredUnit(pr)				( (pr)->unit )
#define PredNextU(pr)				( (pr)->nextU )
#define PredConsultFile(pr)			( (pr)->consultFile )
#define PredFunctor(pr)				( (pr)->functor )
#define PredAtom(pr)				( FunctorAtom(PredFunctor(pr)) )
#define PredArity(pr)				( FunctorArity(PredFunctor(pr)) )
#define PredName(pr)				( FunctorName(PredFunctor(pr)) )
#define PredIsBuiltin(pr)			( FunctorIsBuiltin(PredFunctor(pr)) )
#define PredIsSpy(pr)				( FunctorIsSpy(PredFunctor(pr)) )
#define PredIsMeta(pr)				( FunctorIsMeta(PredFunctor(pr)) )
#define PredClauses(pr)				( (pr)->clauses )
#define PredHasClauses(pr)			( PredClauses(pr) != nil )
#define PredLastClause(pr)			( (pr)->lastClause )
#define PredFrom(pt, field)			cPredicatePt(cCharPt(pt) -		\
							(cCharPt(&(cPredicatePt(0)->field)) - cCharPt(0)))

#define PredConsultGen(pr)			( (pr)->consultGen )
#define PredNIndexable(pr)			( (pr)->nIndexable )
#define PredIsIndexable(pr)			( PredNIndexable(pr) > 0 )
#define PredIsDynamic(pr)			( (pr)->dynamic )
#define PredIsMultifile(pr)			( (pr)->multifile )
#define PredIsDiscontiguous(pr)		( (pr)->discontiguous )
#define PredIsVisible(pr)			( (pr)->visible )
#define PredIsGOpen(pr)				( (pr)->gopen )
#define PredIsGClose(pr)			( PredIsVisible(pr) && !PredIsGOpen(pr) )
#define PredIsXNew(pr)				( !(pr)->xover )
#define PredIsXOver(pr)				( (pr)->xover )
#define PredHasDynamicChain(pr)		( (pr)->hasDynamicChain )
#define PredIsLogical(pr)			( (pr)->logical )
#define PredKeepSource(pr)			( (pr)->keepSource )
#define PredIsPermanent(pr)			( (pr)->permanent )
#define PredIsTraceable(pr)			( (pr)->traceable )
#define PredIsCore(pr)				( (pr)->core )
#define PredWasAbolished(pr)		( (pr)->abolished )
#define PredOnError(pr)				( (pr)->onError )

#define PredIsMutableBuiltin(pr)	( PredIsBuiltin(pr) && PredIsDynamic(pr) )

/* There are 6 exclusive kinds of predicates:
	Local		- have clauses, may be is visible or dynamic
	Imported	- no clauses, may be visible
	C determ	- no clauses, always visible
	C nondeterm	- have 2 clauses, always visible
	Undefined	- no clauses, not visible, not dynamic
	Global		- any number of clauses, not visible, may be dynamic (ctx2)
*/
#define PredIsImported(pr)			( PredStartInst(pr) == Import )
#define GetImportTerm(pr)			( PredStartInstArgs(pr)[0] )
#define PredIsC(pr)					( PredStartInstArgs(pr)[0] == Proceed )
#define PredIsCNonDeterm(pr)		( PredStartInst(pr) == PutNil )
#define PredIsUndefined(pr)			( PredStartInst(pr) == UndefPred )

#define PredCNonDetermAMem(pr)		( PredStartInstArgs(pr)[2][2] )

extern PredicatePt undefPred ;

void BindPredicate(PredicatePt pr, Inst inst, Pt a0) ;
void BindPredicateFull(PredicatePt pr, Inst inst, Pt a0, Pt a1, Pt a2, Pt a3) ;
void BindPredicateAsEmpty(PredicatePt pr) ;
void BindPredicateWithLocalJump(PredicatePt pr, VoidPt code) ;
void BindPredicateAsUndef(PredicatePt pr) ;
PredicatePt FindPredicateInUnit(struct Unit *u, FunctorPt f) ;
PredicatePt FindPredicate(FunctorPt f) ;
PredicatePt FindPredicateByName(Str n, int a) ;
PredicatePt LookupPredicateInUnit(struct Unit *u, FunctorPt f) ;
PredicatePt LookupPredicate(FunctorPt f) ;
PredicatePt LookupPredicateByName(Str n, int a) ;
PredicatePt CheckPredicate(FunctorPt f) ;
PredicatePt CheckPredicateByName(Str n, int a) ;
Size PredLength(PredicatePt pr) ;
Str PredNameArity(PredicatePt pr) ;
Pt PredNameArityTerm(PredicatePt pr) ;
Bool PredIgnoredInGeneration(PredicatePt pr) ;
void MarkStaticBuiltinsAsPermanent(void) ;
void AbolishPredicate(PredicatePt pr, Bool force) ;
void CompatibleIfThenUpdateFlag(int newValue) ;
PredicatePt PredNextUPlusBuiltins(PredicatePt pr) ;
ClausePt AddNewClause(Pt source, Bool end, Bool consulting, Bool topLevel) ;
void SetDynamic(PredicatePt pr, Bool logical) ;
void SetMultifile(PredicatePt pr) ;
void SetDiscontiguous(PredicatePt pr) ;
void SetNIndexable(PredicatePt pr, int nIndexable) ;
void SetClauseList(Pt list, Bool end, Bool visible) ;
FunctorPt ClearNDetermFunctor(FunctorPt f) ;
PredOnError XTestOnError(Pt t) ;
AtomPt PredOnErrorAsAtom(PredicatePt pr) ;

void SetMetaPredicate(Str n, int a) ;
void SetCorePredicate(Str n, int a) ;
PredicatePt InstallCBuiltinPred(Str name, int arity, Fun cProc) ;
PredicatePt InstallGNDeterCBuiltinPred(Str name, int arity, int extraA, Fun cProc) ;
PredicatePt InstallNDeterCBuiltinPred(Str name, int arity, Fun cProc) ;
PredicatePt FindCPredByInst(Inst inst) ;
PredicatePt CurrCPred(void) ;
Str CurrCPredNameArity(void) ;
void PredicatesInit(void) ;

#endif
