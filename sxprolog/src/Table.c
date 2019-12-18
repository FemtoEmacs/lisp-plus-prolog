/*
 *   This file is part of the CxProlog system

 *   Table.c
 *   by A.Miguel Dias - 2008/08/24
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

static void TableExpand(TablePt t)
{
	Size oldCapacity = TableEnd(*t) - TableBegin(*t) ;
	Size newCapacity = oldCapacity * 2 ;
	Hdl newVarsBegin, newVarsEnd ;
	MemoryGrowInfo(TableName(*t), oldCapacity, newCapacity) ;
	newVarsBegin = Reallocate(TableBegin(*t), oldCapacity, newCapacity) ;
	newVarsEnd = newVarsBegin + newCapacity ;
	TableLast(*t) = newVarsBegin + (TableLast(*t) - TableBegin(*t)) ;
	TableBegin(*t) = newVarsBegin ;
	TableEnd(*t) = newVarsEnd ;
}

void TableInit(TablePt t, Str name, int itemSize, int initialCapacity)
{
	if( TableName(*t) != nil )
		FatalError("Double initialization of table '%s'", TableName(*t)) ;
	initialCapacity *= itemSize ;
	TableName(*t) = StrAllocate(name) ;
	TableItemSize(*t) = itemSize ;
	TableBegin(*t) = Allocate(initialCapacity, false) ;
	TableLast(*t) = TableBegin(*t) ;
	TableEnd(*t) = TableBegin(*t) + initialCapacity ;
}

int TableNItems(TablePt t)
{
	return (TableLast(*t) - TableBegin(*t)) / TableItemSize(*t) ;
}

void TableReset(TablePt t)
{
	TableLast(*t) = TableBegin(*t) ;
}

Hdl TableNewItem(TablePt t, VoidPt v)
{
	Hdl last ;
	if( TableLast(*t) == TableEnd(*t) )
		TableExpand(t) ;
	last = TableLast(*t) ;
	*last = v ;
	TableLast(*t) += TableItemSize(*t) ;
	return last ; 
}
