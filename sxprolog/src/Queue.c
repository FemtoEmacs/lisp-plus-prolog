/*
 *   This file is part of the CxProlog system

 *   Queue.c
 *   by A.Miguel Dias - 2000/08/12
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

/* QUEUE */

typedef struct Queue {
	ExtraDef(Queue) ;
	Hdl begin, end, first, last ;
} Queue ;

#define cQueuePt(x)				((QueuePt)(x))

#define QueueBegin(q)			((q)->begin)
#define QueueEnd(q)				((q)->end)
#define QueueFirst(q)			((q)->first)
#define QueueLast(q)			((q)->last)

static ExtraTypePt queueType ;


/* PRIVATE FUNCTIONS */

#define QueueWords(capacity)		(capacity)

static Size QueueCapacity(QueuePt q)
{
	return QueueEnd(q) - QueueBegin(q) ;
}

static void QueueInit(QueuePt q, Size capacity)
{
	QueueBegin(q) = Allocate(QueueWords(capacity), false) ;
	QueueEnd(q) = QueueBegin(q) + capacity ;
	QueueFirst(q) = QueueBegin(q) ;
	QueueLast(q) = QueueBegin(q) ;
}

static void QueueExpand(QueuePt q)
{
	Hdl b = QueueBegin(q) ;
	Hdl f = QueueFirst(q) ;
	Hdl l = QueueLast(q) ;
	Hdl h ;
	Size oldCapacity = QueueCapacity(q) ;
	QueueInit(q, 2 * oldCapacity) ;
	for( h = QueueBegin(q) ; f < l ; *h++ = *f++ ) ;
	QueueLast(q) = h ;
	Release(b, QueueWords(oldCapacity)) ;
}

static void QueueDisable(QueuePt q)
{
	if( ExtraDisable(q) ) {
		QueueClear(q) ;
		Release(QueueBegin(q), QueueWords(QueueCapacity(q))) ;
	}
}

static void QueuePark(QueuePt q)
{
	Hdl f, b ;
	for( b = QueueBegin(q), f = QueueFirst(q) ; f < QueueLast(q) ; *b++ = *f++ ) ;
	QueueFirst(q) = QueueBegin(q) ;
	QueueLast(q) = b ;
}

static void QueueWrite(StreamPt stm, QueuePt q)
{
	Hdl h ;
	StreamWrite(stm, "%s", ExtraAsStr(q)) ;
	StreamWrite(stm, "     (current capacity %ld)\n", QueueCapacity(q)) ;
	for( h = QueueFirst(q) ; h < QueueLast(q) ; h++ )
		StreamWrite(stm, "\t%s\n", TermAsStr(*h)) ;
}

static Size QueueSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(Queue) ;
}

static void QueuesBasicGCMarkContents(VoidPt x)
{
	QueuePt q = cQueuePt(x) ;
	register Hdl h ;
	for( h = QueueFirst(q) ; h < QueueLast(q) ; h++ )
		TermBasicGCMark(*h) ;
}

static Bool QueueBasicGCDelete(VoidPt x)
{
	QueuePt q = cQueuePt(x) ;
	QueueDisable(q) ;
	return true ;
}


/* MAIN OPERATIONS */

Size QueueSize(QueuePt q)
{
	return QueueLast(q) - QueueFirst(q) ;
}

QueuePt QueueNew()
{
	QueuePt q = ExtraNew(queueType, 0) ;
	QueueInit(q, 4) ;
	return q ;
}

void QueueClear(QueuePt q)
{
	register Hdl h ;
	for( h = QueueFirst(q) ; h < QueueLast(q) ; h++ )
		ReleaseTerm(*h) ;
	QueueLast(q) = QueueFirst(q) ;
}

void QueueFilter(QueuePt q, BFunVV filter, VoidPt x)
{
	Hdl a, z, l = QueueLast(q) ;
	for( a = z = QueueFirst(q) ; a < l ; a++ )
		if( filter(*a, x) )
			*z++ = *a ;
		else ReleaseTerm(*a) ;
	QueueLast(q) = z ;
}

Bool QueuePeek(QueuePt q, Hdl t)
{
	if( QueueFirst(q) < QueueLast(q) ) {
		*t = *QueueFirst(q) ;
		return true ;
	}
	else return false ;
}

Bool QueueGet(QueuePt q)
{
	if( QueueFirst(q) < QueueLast(q) ) {
		ReleaseTerm(*QueueFirst(q)++) ;
		return true ;
	}
	else return false ;
}

void QueuePut(QueuePt q, Pt t)
{
	if( QueueLast(q) == QueueEnd(q) ) {
		if( QueueFirst(q) != QueueBegin(q) )
			QueuePark(q) ;
		else
			QueueExpand(q) ;
	}
	*QueueLast(q)++ = AllocateTermForAssign(t) ;
}

Pt ZGetQueueFrontSectionAsList(QueuePt q, Pt mark)
{
	Hdl h ;
	Z.t = tNilAtom ;
	for( h = QueueLast(q) - 1 ; h >= QueueFirst(q) ; h-- ) {
		Pt t = ZPushTerm(*h) ; /* stacks may grow */
		if( t == mark ) break ;
		ReleaseTerm(*h) ;
		Z.t = MakeList(t, Z.t) ;
	}
	QueueLast(q) = h ;
	return Z.t ;
}


/* CXPROLOG C'BUILTINS */

static void PQueueCheck()
{
	MustBe( XExtraCheck(queueType, X0) ) ;
}

static void PQueueNew()
{
	BindVarWithExtra(X0, QueueNew()) ;
	JumpNext() ;
}

static void PQueueClear()
{
	QueueClear(XTestExtra(queueType,X0)) ;
	JumpNext() ;
}

static void PQueueDelete()
{
	QueuePt q = XTestExtra(queueType,X0) ;
	QueueDisable(q) ;
	JumpNext() ;
}

static void PQueuePut()
{
	QueuePut(XTestExtra(queueType,X0), X1) ;
	JumpNext() ;
}

static void PQueueGet()
{
	Pt t ;
	QueuePt q = XTestExtra(queueType,X0) ;
	Ensure( QueuePeek(q, &t) ) ;
	t = ZPushTerm(t) ; /* stacks may grow */
	if( !QueueGet(q) )
		InternalError("PQueueGet") ;
	MustBe( Unify(X1, t) ) ;
}

static void PQueuePeek()
{
	Pt t ;
	Ensure( QueuePeek(XTestExtra(queueType,X0), &t) ) ;
	t = ZPushTerm(t) ; /* stacks may grow */
	MustBe( Unify(X1, t) ) ;
}

static void PQueueAsList()
{
	QueuePt q = XTestExtra(queueType,X0) ;
	Hdl h ;
	Z.t = tNilAtom ;
	for( h = QueueLast(q) - 1 ; h >= QueueFirst(q) ; h-- ) {
		Pt t = ZPushTerm(*h) ; /* stacks may grow */
		Z.t = MakeList(t, Z.t) ;
	}
	MustBe( Unify(Z.t, X1) ) ;
}

static void PQueueAsSeq()
{
	QueuePt q = XTestExtra(queueType,X0) ;
	Hdl h ;
	if( QueueSize(q) == 0 ) DoFail() ;
	h = QueueLast(q) - 1 ;
	Z.t = ZPushTerm(*h) ;
	for( h = QueueLast(q) - 2 ; h >= QueueFirst(q) ; h-- ) {
		Pt t = ZPushTerm(*h) ; /* stacks may grow */
		Z.t = MakeBinStruct(commaFunctor, t, Z.t) ;
	}
	MustBe( Unify(Z.t, X1) ) ;
}

static void PQueueWrite()
{
	QueueWrite(currOut, XTestExtra(queueType,X0)) ;
	JumpNext() ;
}

static void PSQueueWrite()
{
	QueueWrite(XTestStream(X0, mWrite), XTestExtra(queueType,X1)) ;
	JumpNext() ;
}

static void PNDCurrentQueue()
{
	ExtraPNDCurrent(queueType, nil, 1, 0) ;
	JumpNext() ;
}

static Size QueuesAux(CVoidPt x)
{
	QueuePt q = cQueuePt(x) ;
	Write("size(%ld), capacity(%ld)", QueueSize(q), QueueCapacity(q)) ;
	return 1 ;
}
static void PQueues()
{
	ExtraShow(queueType, QueuesAux) ;
	JumpNext() ;
}

/* TEST, EXTRACT & INIT */

Bool IsQueue(Pt t)
{
	return IsThisExtra(queueType, t) ;
}

QueuePt XTestQueue(Pt t)
{
	return XTestExtra(queueType, t) ;
}

void QueuesInit()
{
	queueType = ExtraTypeNew("QUEUE", QueueSizeFun, QueuesBasicGCMarkContents, QueueBasicGCDelete, 1) ;
	/* add "queues." to CxProlog.c/PShow */

	InstallCBuiltinPred("queue", 1, PQueueCheck) ;
	InstallCBuiltinPred("queue_new", 1, PQueueNew) ;
	InstallCBuiltinPred("queue_clear", 1, PQueueClear) ;
	InstallCBuiltinPred("queue_delete", 1, PQueueDelete) ;
	InstallCBuiltinPred("queue_put", 2, PQueuePut) ;
	InstallCBuiltinPred("queue_get", 2, PQueueGet) ;
	InstallCBuiltinPred("queue_peek", 2, PQueuePeek) ;
	InstallCBuiltinPred("queue_as_list", 2, PQueueAsList) ;
	InstallCBuiltinPred("queue_as_seq", 2, PQueueAsSeq) ;
	InstallCBuiltinPred("queue_write", 1, PQueueWrite) ;
	InstallCBuiltinPred("queue_write", 2, PSQueueWrite) ;
	InstallGNDeterCBuiltinPred("current_queue", 1, 2, PNDCurrentQueue) ;
	InstallCBuiltinPred("queues", 0, PQueues) ;
}
