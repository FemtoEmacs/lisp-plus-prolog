/*
 *   This file is part of the CxProlog system

 *   TermCyclic.c
 *   by A.Miguel Dias - 2007/08/14
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



/*** Macros for handling a cyclic term ***
 *
 * These macros support the depth-first transversal of a term. At any
 * instant, the current path, from the root to the current subterm,
 * is temporary marked (using the atom tCyclicAtom) so that any cyclic
 * reference can be detected. A cyclic reference is always an upwards
 * reference.
 *
 * Marking a subterm requires accessing the location, inside the term,
 * of the reference to the subterm. This small detail forced a
 * radical change in some algorithms used in CxProlog, as of version 0.96.
 */

#define Cyclic1_VarDecl(term)											\
	register Pt t ;														\
	register Hdl h = &(term), end = &(term) + 1

#define Cyclic1_PrepareArgs(args, arity)								\
	Do(	ScratchPush(*h) ;			/* Save the term being visited */	\
		*h = tCyclicAtom ;			/* Mark it as a visited term */		\
		ScratchPush(end) ;			/* Save sequence end */				\
		ScratchPush(h) ;			/* Save current pointer */			\
		h = (args) ;				/* New sequence to transverse */	\
		end = h + (arity) ;			/* New sequence end */				\
	)

#define Cyclic1_Next()													\
	Do( while( ++h == end ) {			/* Process all ended seqs */	\
			if( ScratchUsed() == 0 )	/* Nothing more in the stack? */\
				goto finish ;											\
			h = cHdl(ScratchPop()) ;	/* Restore current pointer */	\
			end = cHdl(ScratchPop()) ;	/* Restore sequence end */		\
			*h = ScratchPop() ;			/* Restore the visited term */	\
		}																\
	)

#define Cyclic1_Cleanup()												\
	Do( while( ScratchUsed() != 0 ) { /* Process remaining stack */		\
			h = cHdl(ScratchPop()) ;  /* Restore pointer to visited */	\
			Ignore(ScratchPop()) ;	  /* Ignore sequence end */			\
			*h = ScratchPop() ;		  /* Restore visited term */		\
		}																\
	)

#define Cyclic1_NextF(code)												\
	Do( while( ++h == end ) {			/* Process all ended seqs */	\
			if( ScratchUsed() == 0 )	/* Nothing more in the stack? */\
				goto finish ;											\
			code ;														\
			h = cHdl(ScratchPop()) ;	/* Restore current pointer */	\
			end = cHdl(ScratchPop()) ;	/* Restore sequence end */		\
			*h = ScratchPop() ;			/* Restore the visited term */	\
		}																\
	)


/*** Macros for handling a pair of cyclic terms ***
 *
 * These macros support the depth-first transversal of 2 terms in parallel.
 * At any instant, the current path, from the root to the current subterm,
 * is temporary marked, usually by replacing the subterms in the current
 * path with matching subterms in the second term.
 */

#define Cyclic2_VarDecl(term1, term2)									\
	register Pt t1 ;													\
	register Hdl h1 = &(term1), h2 = &(term2), end1 = &(term1) + 1

#define Cyclic2_PrepareArgs(args1, args2, arity, replacement)			\
	Do(	ScratchPush(t1) ;		/* Save subterm being visited in 1 */	\
		*h1 = replacement ;		/* Mark it as a visited term */			\
		ScratchPush(end1) ;		/* Save sequence end 1 */				\
		ScratchPush(h1) ;		/* Save current pointer 1 */			\
		ScratchPush(h2) ;		/* Save current pointer 2 */			\
		h1 = (args1) ;			/* New sequence to transverse 1 */		\
		end1 = h1 + (arity) ;	/* New sequence end 1 */				\
		h2 = (args2) ;			/* New sequence to transverse 2 */		\
	)
	
#define Cyclic2_Next()													\
	Do( while( ++h1 == end1 ) {		/* Process all ended seqs 1 */		\
			if( ScratchUsed() == 0 )	/* Nothing more in the stack? */\
				goto finish ;											\
			h2 = cHdl(ScratchPop()) ;	/* Restore current pointer 2 */	\
			h1 = cHdl(ScratchPop()) ;	/* Restore current pointer 1 */	\
			end1 = cHdl(ScratchPop()) ;	/* Restore sequence end 1 */	\
			*h1 = ScratchPop() ;		/* Restore visited subterm 1 */	\
		}																\
		++h2 ;							/* Advance current pointer 2 */	\
	)

#define Cyclic2_Cleanup()												\
	Do( while( ScratchUsed() != 0 ) {	/* Restore initial term 1 */	\
			Ignore(ScratchPop()) ;		/* Ignore current pointer 2  */	\
			h1 = cHdl(ScratchPop()) ;	/* Restore current pointer 1 */	\
			Ignore(ScratchPop()) ;		/* Ignore sequence end 1 */		\
			*h1 = ScratchPop() ;		/* Restore visited term 1 */	\
		}																\
	)



/*** Macros for handling a cyclic term with perfect cycle discovery ***
 *
 * These macros support the depth-first transversal of a term. At any
 * and have the advantage of discovering each cycle as soon as possible.
 * However they are a little less efficient and more hard to use than
 * the macros in the "Cyclic1_*" family.
 *
 * Note that the macros in the "Cyclic1_*" family are not suitable to some
 * applications (like finding the size of a term) because each cycle is
 * discovered one level too late deep in the rational tree.
 */

#define Cyclic1P_VarDecl(term)											\
	register Pt t ;														\
	register Hdl h = &(term), end = &(term) + 1

#define Cyclic1P_PrepareArgs(args, arity)								\
	Do(	ScratchPush(end) ;			/* Save sequence end */				\
		ScratchPush(h) ;			/* Save current pointer */			\
		h = (args) ;				/* New sequence to transverse */	\
		end = h + (arity) ;			/* New sequence end */				\
		ScratchPush(h) ;			/* Save arg0 location */			\
		ScratchPush(*h) ;			/* Save arg0 */						\
		*h = tCyclicAtom ;			/* Mark subterm as being visited */	\
	)

#define Cyclic1P_Next()													\
	Do( while( ++h == end ) {			/* Process all ended seqs */	\
			if( ScratchUsed() == 0 )	/* Nothing more in the stack? */\
				goto finish ;											\
			t = ScratchPop() ;			/* Get original arg0 */			\
			*cHdl(ScratchPop()) = t ;	/* Restore original arg0 */		\
			h = cHdl(ScratchPop()) ;	/* Restore current pointer */	\
			end = cHdl(ScratchPop()) ;	/* Restore sequence end */		\
		}																\
	)

#define Cyclic1P_Cleanup()												\
	Do( while( ScratchUsed() != 0 ) {	/* Restore initial term */		\
			t = ScratchPop() ;			/* Get original arg0 */			\
			*cHdl(ScratchPop()) = t ;	/* Restore original arg0 */		\
			Ignore(ScratchPop()) ;		/* Ignore current pointer  */	\
			Ignore(ScratchPop()) ;		/* Restore current pointer */	\
			Ignore(ScratchPop()) ;		/* Ignore sequence end */		\
		}																\
	)



/*** Macros for handling a pair cyclic term with perfect cycle discovery ***
 *
 * These macros support the depth-first transversal of 2 terms in parallel
 * and have the advantage of discovering each cycle as soon as possible.
 * However they are a little less efficient and more hard to use than
 * the macros in the "Cyclic2_*" family.
 *
 * Note that the macros in the "Cyclic2_*" family are not suitable to some
 * applications (like copying a term) because each cycle is discovered
 * one level too late deep in the rational tree.
 */

#define Cyclic2P_VarDecl(term1, term2)									\
	register Pt t1 ;													\
	register Hdl h1 = &(term1), h2 = &(term2), end1 = &(term1) + 1

#define Cyclic2P_PrepareArgs(args1, args2, arity, replacement)			\
	Do(	ScratchPush(end1) ;			/* Save sequence end 1 */			\
		ScratchPush(h1) ;			/* Save current pointer 1 */		\
		ScratchPush(h2) ;			/* Save current pointer 2 */		\
		h1 = (args1) ;				/* New sequence to transverse 1 */	\
		end1 = h1 + (arity) ;		/* New sequence end 1 */			\
		h2 = (args2) ;				/* New sequence to transverse 2 */	\
		ScratchPush(h1) ;			/* Save arg0 location */			\
		ScratchPush(*h1) ;			/* Save arg0 */						\
		*h1 = replacement ;			/* Mark subterm as being visited */	\
	)

#define Cyclic2P_Next()													\
	Do( while( ++h1 == end1 ) {		/* Process all ended seqs 1 */		\
			if( ScratchUsed() == 0 )	/* Nothing more in the stack? */\
				goto finish ;											\
			t1 = ScratchPop() ;			/* Get original arg0 */			\
			*cHdl(ScratchPop()) = t1 ;	/* Restore original arg0 */		\
			h2 = cHdl(ScratchPop()) ;	/* Restore current pointer 2 */	\
			h1 = cHdl(ScratchPop()) ;	/* Restore current pointer 1 */	\
			end1 = cHdl(ScratchPop()) ;	/* Restore sequence end 1 */	\
		}																\
		++h2 ;							/* Advance current pointer 2 */	\
	)

#define Cyclic2P_Cleanup()												\
	Do( while( ScratchUsed() != 0 ) {	/* Restore initial term 1 */	\
			t1 = ScratchPop() ;			/* Get original arg0 */			\
			*cHdl(ScratchPop()) = t1 ;	/* Restore original arg0 */		\
			Ignore(ScratchPop()) ;		/* Ignore current pointer 2  */	\
			Ignore(ScratchPop()) ;		/* Restore current pointer 1 */	\
			Ignore(ScratchPop()) ;		/* Ignore sequence end 1 */		\
		}																\
	)



/*** Macros for handling a pair of acyclic terms (NOT USED) ***
 *
 * These macros support depth-first transversals of 2 acyclic terms in parallel.
 * Simpler and faster version of the "Cyclic2" family of macros.
 */

#define ACyclic2_VarDecl(term1, term2)									\
	register Pt t1 ;													\
	register Hdl h1 = &(term1), h2 = &(term2), end1 = &(term1) + 1

#define ACyclic2_PrepareArgs(args1, args2, arity)						\
	Do(	ScratchPush(end1) ;		/* Save sequence end 1 */				\
		ScratchPush(h1) ;		/* Save current pointer 1 */			\
		ScratchPush(h2) ;		/* Save current pointer 2 */			\
		h1 = (args1) ;			/* New sequence to transverse 1 */		\
		end1 = h1 + (arity) ;	/* New sequence end 1 */				\
		h2 = (args2) ;			/* New sequence to transverse 2 */		\
	)

#define ACyclic2_Next()													\
	Do( while( ++h1 == end1 ) {		/* Process all ended seqs 1 */		\
			if( ScratchUsed() == 0 )	/* Nothing more in the stack? */\
				goto finish ;											\
			h2 = cHdl(ScratchPop()) ;	/* Restore current pointer 2 */	\
			h1 = cHdl(ScratchPop()) ;	/* Restore current pointer 1 */	\
			end1 = cHdl(ScratchPop()) ;	/* Restore sequence end 1 */	\
		}																\
		++h2 ;							/* Advance current pointer 2 */	\
	)
