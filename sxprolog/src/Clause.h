/*
 *   This file is part of the CxProlog system

 *   Clause.h
 *   by A.Miguel Dias - 1989/11/14
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Clause_
#define _Clause_

typedef struct Clause {
	struct Predicate *owner ;		/* Clause predicate */
	Pt idxInfo[3] ;					/* First three arguments info */
	Pt source ;						/* Clause source */
	AtomPt consultFile ;			/* Clause consult filename */
/* The code. Must be at the end of the clause */
	Inst startInst ;				/* TryMeElse, RetryMeElse, TrustMe */
	struct Clause *next ;			/* Next clause */
	Word arity ;					/* Clause arity */
} Clause, *ClausePt ;

#define cClausePt(x)				( (ClausePt)(x) )

#define ClauseOwner(c)				( (c)->owner )
#define ClauseFunctor(c)			( PredFunctor(ClauseOwner(c)) )
#define ClauseIdxInfo(c,n)			( ((c)->idxInfo)[n] )
#define ClauseSource(c)				( (c)->source )
#define ClauseConsultFile(c)		( (c)->consultFile )
#define ClauseInst(c)				( (c)->startInst )
#define ClauseNext(c)				( (c)->next )
#define ClauseArity(c)				( (c)->arity )
#define ClauseNextDeleted(c)		cClausePt( ClauseIdxInfo(c,0) )
#define SetClauseNextDeleted(c,x)	do{ ClauseIdxInfo(c,0) = cPt(x) ; }while(0)
#define ClauseBirth(c)				( ClauseIdxInfo(c,1) )
#define ClauseDeath(c)				( ClauseIdxInfo(c,2) )
#define ClauseFrom(pt, field)		cClausePt(cCharPt(pt) -			\
								(cCharPt(&(cClausePt(0)->field)) - cCharPt(0)))

#define ClauseCode(c)				( &ClauseInst(c) )
#define ClauseCodeSkipHeader(c)		( cHdl(ClauseCode(c)) + 3 ) /* skip TryMeElse l n */
#define IsClauseLoop(c)				( ClauseNext(c) == c )

#define PredHasOneClause(p)			( PredHasClauses(p) &&	\
										ClauseNext(PredClauses(p)) == nil )
#define PredHasMultipleClauses(p)	( PredHasClauses(p) &&	\
										ClauseNext(PredClauses(p)) != nil )

#define ClauseIsAlive2(c,t)			( cWord(ClauseBirth(c)) < cWord(t)		\
										&& cWord(t) <= cWord(ClauseDeath(c)) )
#define IsEndOfTime(t)				( t == maxUIntPt )
#define ClauseSetAlive(c)			( ClauseDeath(c) = maxUIntPt )
#define ClauseIsAlive(c)			IsEndOfTime(ClauseDeath(c))

extern ClausePt failClause ;

ClausePt InstallClause(struct Predicate *pr, Hdl code, Size codeLen, Pt head,
														Pt source, Bool end) ;
void ReinstallClausesAsStatic(struct Predicate *pr) ;
void ReleaseClauseSource(ClausePt c) ;
void DeleteClause(ClausePt cl) ;
void ClauseGC(Bool blind) ;
void DeleteClausesAndIndex(struct Predicate *pr) ;
Size DeletedClausesCount(void) ;
void ClausesInit(void) ;

#endif
