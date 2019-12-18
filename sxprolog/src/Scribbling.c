/*
 *   This file is part of the CxProlog system

 *   Scribbling.c
 *   by A.Miguel Dias - 20016/06/05
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


/* Scribbling */

#define scribblingInitialCapacity	(1 K) /* words */

Hdl scribblingBegin, scribblingEnd, scribblingPt ;

Bool ScribblingExpand(void)
{
	Size oldScribblingCapacity = ScribblingCapacity() ;
	Size newScribblingCapacity = oldScribblingCapacity * 2 ;
	MemoryGrowInfo("scribbling", oldScribblingCapacity, newScribblingCapacity) ;
	scribblingBegin = Reallocate(scribblingBegin, oldScribblingCapacity, newScribblingCapacity) ;
	scribblingPt = scribblingBegin + oldScribblingCapacity;
	scribblingEnd = scribblingBegin + newScribblingCapacity ;
	return false ;
}

void ScribblingMakeRoom(Size nWords)
{
	while( scribblingPt + nWords >= scribblingEnd )
		ScribblingExpand() ;
	scribblingPt += nWords ;
}

Size ScribblingCapacity(void)
{
	return Df(scribblingEnd , scribblingBegin) ;
}

void ScribblingRestart(void)
{
	scribblingPt = scribblingBegin ;
}

void ScribblingShow(FunV f)
{
	Hdl h;
	for( h = scribblingBegin ; h < scribblingPt ; h++ )
		if( f != nil )
			f(*h);
		elif( *h == nil )
			Mesg("nil");
		else
			Mesg("%s", TermAsStr(*h)) ;
		
}

void ScribblingDiscardTop(VoidPt v)
{
	while( ScribblingTop() == v )
		Unused(ScribblingPop()) ;
}


/* Scribbling 2 */

#define scribbling2InitialCapacity	(1 K) /* words */

Hdl scribbling2Begin, scribbling2End, scribbling2Pt ;

Bool Scribbling2Expand(void)
{
	Size oldScribbling2Capacity = Scribbling2Capacity() ;
	Size newScribbling2Capacity = oldScribbling2Capacity * 2 ;
	MemoryGrowInfo("scribbling2", oldScribbling2Capacity, newScribbling2Capacity) ;
	scribbling2Begin = Reallocate(scribbling2Begin, oldScribbling2Capacity, newScribbling2Capacity) ;
	scribbling2Pt = scribbling2Begin + oldScribbling2Capacity;
	scribbling2End = scribbling2Begin + newScribbling2Capacity ;
	return false ;
}

void Scribbling2MakeRoom(Size nWords)
{
	while( scribbling2Pt + nWords >= scribbling2End )
		Scribbling2Expand() ;
	scribbling2Pt += nWords ;
}

Size Scribbling2Capacity(void)
{
	return Df(scribbling2End , scribbling2Begin) ;
}

void Scribbling2Restart(void)
{
	scribbling2Pt = scribbling2Begin ;
}

void Scribbling2Show(FunV f)
{
	Hdl h;
	for( h = scribbling2Begin ; h < scribbling2Pt ; h++ )
		if( f != nil )
			f(*h);
		elif( *h == nil )
			Mesg("nil");
		else
			Mesg("%s", TermAsStr(*h)) ;
		
}

void Scribbling2DiscardTop(VoidPt v)
{
	while( Scribbling2Top() == v )
		Unused(Scribbling2Pop()) ;
}


/* CXPROLOG C'BUILTINS */


void ScribblingInit()
{
	scribblingBegin = Allocate(scribblingInitialCapacity, false) ;
	scribblingEnd = scribblingBegin + scribblingInitialCapacity ;
	ScribblingRestart() ;
	
	scribbling2Begin = Allocate(scribbling2InitialCapacity, false) ;
	scribbling2End = scribbling2Begin + scribbling2InitialCapacity ;
	Scribbling2Restart() ;
}
