/*
 *   This file is part of the CxProlog system

 *   ContextsInsts2.ch
 *   by A.Miguel Dias - 2006/05/07
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2007 A.Miguel Dias, CITI, DI/FCT/UNL

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

// ** EXPERIMENTAL **

/* This file is included in "Instructions.c" */


/*****************************************************************************

*** CONTEXTS 2 : INTRODUCTION

CTX2 uses a contexts and an implicit history. It included mechanism for
compose predicate definitions, using clauses suplied by several units
descriptors available in the context.


*** CONTEXTS 2 : CONSTRUCTIONS

Not publicly available yet.


*** CONTEXTS 2 : SEMANTIC RULES

Not publicly available yet.

*****************************************************************************/


/* CONTEXT 2 INSTRUCTIONS */

static void UndefPredInst()
{
	ExecutePred(HandleUndefPredCall(LookFunctor(), C)) ;	
	JumpNext() ;
}

static void UndefPredEndInst()
{
	UndisclosedError("CTX = 2") ;
}

static void ImportInst()
{
	InternalError("ImportInst not available") ;	
}

static void ImportEndInst()
{
	InternalError("ImportEndInst not available") ;	
}

static void CtxSwitchInst()
{
	InternalError("CtxSwitchInst not available") ;	
}

static void CtxExtensionInst()
{
	UndisclosedError("CTX = 2") ;
}

static void CtxExtensionEndInst()
{
	UndisclosedError("CTX = 2") ;
}

static void CtxEmptyInst()
{
	UndisclosedError("CTX = 2") ;
}

static void CtxEmptyEndInst()
{
	UndisclosedError("CTX = 2") ;
}

static void CtxDownInst()
{
	UndisclosedError("CTX = 2") ;
}

static void CtxDownEndInst()
{
	UndisclosedError("CTX = 2") ;
}

static void HCtxPushInst()
{
	UndisclosedError("CTX = 2") ;
}

static void HCtxPushEndInst()
{
	UndisclosedError("CTX = 2") ;
}

static void HCtxEnterInst()
{
	InternalError("HCtxEnterInst not available") ;	
}

static void HCtxEnterEndInst()
{
	InternalError("HCtxEnterEndInst not available") ;	
}

static void ContextualEmptyPredInst()
{
	Jump(EmptyPred) ;
}

static void ContextualDynamicEnterInst()
{
	Jump(DynamicEnter) ;
}

static void ContextualDynamicElseInst()
{
	Jump(DynamicElse) ;
}
