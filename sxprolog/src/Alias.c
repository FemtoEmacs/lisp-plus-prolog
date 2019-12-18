/*
 *   This file is part of the CxProlog system

 *   Alias.c
 *   by A.Miguel Dias - 2007/12/02
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

#define initialAliasCapacity	2

typedef struct Alias {
	AtomPt nextAlias ;
	ExtraPt *last ;
/*	ExtraPt items[] ; */
} Alias, *AliasPt ;

#define	cAliasPt(as)			((AliasPt)as)

#define AliasNextAlias(as)		((as)->nextAlias)
#define AliasFirst(as)			(cExtraHdl(as) + WordsOf(Alias))
#define AliasLast(as)			((as)->last)

#define CellIsFree(c)			((c) == nil || ExtraIsDisabled(c))

static AtomPt aliasList = nil ;

static ExtraHdl AliasCreate(AtomPt a) /* pre: AtomToAlias(a) == nil */
{
	AliasPt as = Allocate(WordsOf(Alias) + initialAliasCapacity, false) ;
	AliasLast(as) = AliasFirst(as) + initialAliasCapacity - 1 ;
	ClearWords(AliasFirst(as), initialAliasCapacity) ;
	AtomToAlias(a) = as ;
	ExtraPermanent(a) ;
	AliasNextAlias(as) = aliasList ;
	aliasList = a ;
	return AliasFirst(as) ; /* Return first free */
}

static ExtraHdl AliasExpand(AtomPt a)
{
	AliasPt as = AtomToAlias(a) ;
	int head = WordsOf(Alias) ;
	int oldAliasCapacity = AliasLast(as) - AliasFirst(as) + 1 ;
	int addedAliasCapacity = oldAliasCapacity ;
	int newAliasCapacity = oldAliasCapacity + addedAliasCapacity ;
	as = Reallocate(as, head + oldAliasCapacity , head + newAliasCapacity) ;
	AliasLast(as) = AliasFirst(as) + newAliasCapacity - 1 ;
	ClearWords(AliasFirst(as) + oldAliasCapacity, addedAliasCapacity) ;
	AtomToAlias(a) = as ;
	return AliasFirst(as) + oldAliasCapacity ; /* Return first free */
}

ExtraPt AliasGet(ExtraTypePt e, Pt atom)
{
	AtomPt a = XTestAtom(atom);
	AliasPt as = AtomToAlias(a) ;
	register ExtraHdl h, stop ;
	if( as != nil )
		for( h = AliasFirst(as), stop = AliasLast(as) ; h <= stop ; h++ )
			if( !CellIsFree(*h) && ExtraTag(*h) == ExtraTypeTag(e) )
				return *h ;
	return nil ;
}

void AliasSet(Pt atom, VoidPt extra)
{
	AtomPt a = XTestAtom(atom);
	AliasPt as = AtomToAlias(a) ;
	register ExtraHdl h, stop ;
	if( ExtraIsSpecial(a) )
		StreamRealiasing(atom, cStreamPt(extra)) ;
/* First time */
	if( as == nil ) {
		*AliasCreate(a) = extra ;
		return ;
	}
/* Find place to insert and do it */
	for( h = AliasFirst(as), stop = AliasLast(as) ; h <= stop ; h++ )
		if( CellIsFree(*h) ) {
			*h = extra ;
			as = nil ;		/* Means 'inserted' */
			break ;
		}
		elif( ExtraTag(*h) == ExtraTag(extra) ) {
			*h = extra ;
			return ;
		}
/* Ensure no more extras of the same type  */
	for( h++ ; h <= stop ; h++ )
		if( !CellIsFree(*h) && ExtraTag(*h) == ExtraTag(extra) ) {
			*h = nil ;
			break ;
		}
	if( as != nil )		/* If not inserted */
		*AliasExpand(a) = extra ;
}

void AliasUnset(Pt atom, VoidPt extra)
{
	AtomPt a = XTestAtom(atom);
	AliasPt as = AtomToAlias(a) ;
	register ExtraHdl h, stop ;
	if( as != nil )
		for( h = AliasFirst(as), stop = AliasLast(as) ; h <= stop ; h++ )
			if( *h == extra ) {
				*h = nil ;
				if( ExtraIsSpecial(a) )
					StreamRealiasing(atom, nil) ;
				return ;
			}
	ImperativeError("'%s' is not an alias for '%s'", AtomName(a), ExtraAsStr(extra)) ;
}

AtomPt AliasSearch(VoidPt extra)	/* Slow */
{
	AtomPt a ;
	for( a = aliasList ; a != nil ; a = AliasNextAlias(AtomToAlias(a)) ) {
		AliasPt as = AtomToAlias(a) ;
		register ExtraHdl h, stop ;
		if( as != nil )
			for( h = AliasFirst(as), stop = AliasLast(as) ; h <= stop ; h++ )
				if( *h == extra )
					return a ;
	}
	return nil ;
}

void AliasedWithWrite(Pt t)
{
	AtomPt a = AliasSearch(XTestAnyExtra(t)) ;
	if( a != nil )
		Write(", alias(%s)", AtomName(a)) ;
}

static void AliasBasicGCMark()
{
	AtomPt a ;
	for( a = aliasList ; a != nil ; a = AliasNextAlias(AtomToAlias(a)) ) {
		AliasPt as = AtomToAlias(a) ;
		register ExtraHdl h, stop ;
		if( as != nil ) {
			for( h = AliasFirst(as), stop = AliasLast(as) ; h <= stop ; h++ )
				if( CellIsFree(*h) )	/* During garbage collection we */
					*h = nil ;			/* must remove disabled extras */
				else					/* to avoid dangling pointers. */
					ExtraGCMark(*h) ;
		}
	}
}

static void PAliases()
{
	AtomPt a ;
	ShowVersion() ;
	Write("ALIASES:\n") ;
	for( a = aliasList ; a != nil ; a = AliasNextAlias(AtomToAlias(a)) ) {
		AliasPt as = AtomToAlias(a) ;
		register ExtraHdl h, stop ;
		if( as != nil ) {
			for( h = AliasFirst(as), stop = AliasLast(as) ; h <= stop ; h++ )
				if( !CellIsFree(*h) )
					Write(" %22.16s -> %1.50s\n",
							AtomName(a),
							TermAsStr(TagExtra(nil, *h))) ;
#if 0
				else Write("nil\n") ;
#endif
		}
	}
	JumpNext() ;
}


/* CXPROLOG C'BUILTINS */

static void PAlias()
{
	AliasSet(X0, XTestAnyStrictExtra(X1)) ;
	JumpNext() ;
}

static void PUnalias()
{
	ExtraPt x1 = XTestAnyStrictExtra(X1) ;
	AliasUnset(X0, x1) ;
	JumpNext() ;
}

void AliasInit()
{
	/* add "aliases." to CxProlog.c/PShow */
	ExtraGCHandlerInstall("ALIAS", AliasBasicGCMark) ;

#if COMPASSxxxxxxxxxxxxx
	InstallCBuiltinPred("aliasx", 2, PAlias) ;
#else
	InstallCBuiltinPred("alias", 2, PAlias) ;
#endif
	InstallCBuiltinPred("unalias", 2, PUnalias) ;
	InstallCBuiltinPred("aliases", 0, PAliases) ;
}
