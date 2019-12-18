/*
 *   This file is part of the CxProlog system

 *   Consult.c
 *   by A.Miguel Dias - 2005/08/27
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

static StackPt consultStack ;
static QueuePt initializeQueue ;
static Bool reconsulting ; /* associated with the old_update flag */
static Bool silentConsulting ;
static PredicatePt discontinuousPr = nil ;
static Table loadedTable ;
AtomPt consultFile ;
Word16 consultGen = 0 ;	/* consult generation */

#define LoadedEntryAtom(h)		( cAtomPt(*h) )

static void LoadedTableBasicGCMark(void)
{
	Hdl h ;
	TableFor(loadedTable, h)
		ExtraGCMark(XAtom(LoadedEntryAtom(h))) ;
}

static void LoadedTableInit(void)
{
	TableInit(&loadedTable, "ensure_loaded", 1, 16) ;
	ExtraGCHandlerInstall("LOADED", LoadedTableBasicGCMark) ;
}

static void LoadedTableEnter(AtomPt at)
{
	register Hdl h ;
	TableFor(loadedTable, h)
		if( LoadedEntryAtom(h) == at )
			return ;
	TableNewItem(&loadedTable, at) ;
}

static Bool LoadedTableBelongs(AtomPt at)
{
	register Hdl h ;
	TableFor(loadedTable, h)
		if( LoadedEntryAtom(h) == at )
			return true ;
	return false ;
}

Bool PredHandleConsult(PredicatePt pr, Bool propChange)
{
	if( oldUpdate_flag ) {
		if( reconsulting && PredConsultGen(pr) != consultGen ) {
			AbolishPredicate(pr, true) ;
		/* In case the flag oldUpdate_flag keeps changing... */
			PredConsultFile(pr) = ConsultFile() ;
			return true ;
		}
	}

	elif( Consulting() && PredConsultGen(pr) != consultGen ) {
		Bool undef = PredIsUndefined(pr) ;
		AtomPt prev = PredConsultFile(pr) ;
		if( PredIsMultifile(pr) ) {
			if( propChange )
				DatabaseError("Cannot change property of multifile predicate '%s'",
								PredNameArity(pr)) ;
			return false ;
		}
		AbolishPredicate(pr, false) ;
		PredConsultFile(pr) = ConsultFile() ;
		if( !silentConsulting && !undef ) {
			if( prev == nil )
				Warning("Redefined '%s' (previously defined at the top level)",
															PredNameArity(pr)) ;
			else
				Warning("Redefined '%s' (previously loaded from '%s')",
											PredNameArity(pr), AtomName(prev)) ;
		}
		return true ;
	}

	if( PredIsUndefined(pr) )
		PredConsultFile(pr) = ConsultFile() ;
	return false ;
}

void PredHandleDiscontinuous(PredicatePt pr)
{
	if( pr != discontinuousPr ) {
		discontinuousPr = pr ;
		if( !silentConsulting && PredHasClauses(pr) && !PredIsDiscontiguous(pr) && !PredIsMultifile(pr) )
			Warning("Predicate %s has discontiguous clauses", PredNameArity(pr)) ;
	}
}

static void ZBasicLoadStream(StreamPt srm)
{				/* Only suppports clauses, not commands */
	Pt t ;
	AtomPt saveConsultFile = consultFile ;
	Bool saveReconsulting = reconsulting ;
	consultFile = nil ;
	reconsulting = false ;
	HSave() ;
	while( (t = ZReadTerm(srm)) != tEofAtom ) {
		AddNewClause(t, true, true, false) ;
		HRestore() ;
	}
	consultFile = saveConsultFile ;
	reconsulting = saveReconsulting ;
}

void ZBasicLoadFile(Str fileName)
{
	StreamPt srm = FileStreamOpen(fileName, mRead, nil) ;
	ZBasicLoadStream(srm) ;
	StreamClose(srm, nil) ;
}

void ZBasicLoadStr(Str str)
{
	StreamPt srm = StringStreamOpen(str) ;
	ZBasicLoadStream(srm) ;
	StreamClose(srm, nil) ;
}

void ZBasicLoadBuiltinsStr(Str str)
{
	ContextSetEmpty() ;
	ZBasicLoadStr(str) ;
	ContextRestore() ;
	JumpNext() ;
}



/* CXPROLOG C'BUILTINS */

static void PEnterConsult()
{
	register UnitPt u ;
	register PredicatePt pr ;
	register ClausePt cl, nx ;
	StreamPt consultStream = XTestStream(X0, mRead) ;
	Bool rec = XTestBool(X1) ;
	Bool sil = XTestBool(X2) ;
	Bool ensureLoaded = XTestBool(X3) ;
	AtomPt cf = StreamPath(consultStream) ;
	if( ensureLoaded && LoadedTableBelongs(cf) )
		DoFail() ;
	consultFile = cf ;
	reconsulting = rec ;
	if( sil || infoMessages_flag == 0 ) /* force silent consulting? */
		silentConsulting = true ;
	else
		silentConsulting = silentConsulting ;	/* does not change */
	if( reconsulting && !oldUpdate_flag )
		InternalError("PEnterConsult") ;
/*	ExtraPermanent(consultFile) ; */
	StackPush(consultStack, MakeBool(silentConsulting)) ;
	StackPush(consultStack, X1) ;	/* Reconsulting? */
	StackPush(consultStack, X0) ;	/* Filename */
	QueuePut(initializeQueue, tMarkAtom) ;
	LoadedTableEnter(consultFile) ;

/* Increments generation clock and handles overflow if it occurs */
	if( ++consultGen == 0 ) {	/* Handles overflow */
		doseq(u, unitList, UnitNext(u))
			doseq(pr, UnitPreds(u), PredNextU(pr))
				PredConsultGen(pr) = 0 ;
		consultGen = 1 ;
	}

	if( oldUpdate_flag || consultStream == userIn )
		/* nothing */ ;
	else {
			/* Delete all clauses that came from the current consult file */
		doseq(u, unitList, UnitNext(u))
			doseq(pr, UnitPreds(u), PredNextU(pr))
				if( PredIsMultifile(pr) ) {
					doseq(cl, PredClauses(pr), nx) {
						nx = ClauseNext(cl) ;
						if( ClauseConsultFile(cl) == consultFile )
							DeleteClause(cl) ;
					}
				}
				elif( PredConsultFile(pr) == consultFile )
					AbolishPredicate(pr, false) ;
	}
	discontinuousPr = nil ;
	JumpNext() ;
}

static void PExitConsult()
{
	Pt t ;
	if( !StackPop(consultStack) || !StackPop(consultStack) || !StackPop(consultStack) )
		Error("Too many '$exit_consult'") ;
	if( StackTop(consultStack, &t) ) {
		consultFile	= StreamPath(XTestStream(t, mRead)) ;
		StackFromTop(consultStack, 1, &t) ;
		reconsulting = XTestBool(t) ;
		StackFromTop(consultStack, 2, &t) ;
		silentConsulting = XTestBool(t) ;
	}
	else {
		consultFile	= nil ;
		reconsulting = false ;
		silentConsulting = false ;
	}
	discontinuousPr = nil ;
	JumpNext() ;
}

static void PEnterInclude()
{
	StreamPt includeStream ;
	if( consultFile == nil )
		Error("This directive can only be used in consult") ;
	includeStream = XTestStream(X0, mRead) ;
	consultFile = StreamPath(includeStream) ;
	reconsulting = reconsulting ;			/* does not change */
	silentConsulting = silentConsulting ;	/* does not change */
	ExtraPermanent(consultFile) ;
	StackPush(consultStack, MakeBool(silentConsulting)) ;
	StackPush(consultStack, MakeBool(reconsulting)) ;
	StackPush(consultStack, X0) ;
	JumpNext() ;
}

static void PConsulting()
{
	Pt t ;
	MustBe( StackTop(consultStack, &t) && Unify(X0, t) ) ;
}

static void PConsultLevel()
{
	MustBe( UnifyWithNumber(X0, MakeInt(StackSize(consultStack)/3)) ) ;
}

static void PConsultingIsSilent()
{
	MustBe( silentConsulting ) ;
}

static void PConsultClause()
{
	AddNewClause(X0, true, true, false) ;
	JumpNext() ;
}

static void PConsultStoreInit()
{
	if( consultFile == nil )
		Error("This directive can only be used in consult") ;
	QueuePut(initializeQueue, X0) ;
	JumpNext() ;
}

static void PConsultGetInitList()
{
	Pt t = ZGetQueueFrontSectionAsList(initializeQueue, tMarkAtom) ;
	MustBe( t != nil && Unify(X0, t) ) ;
}

static void PCheckConsult()
{
	if( XTestBool(X0) == true && !oldUpdate_flag )
		Error("No reconsult predicate available (in the current operating mode)") ;
	JumpNext() ;
}

void ConsultRestart()
{
	StackClear(consultStack) ;
	QueueClear(initializeQueue) ;
	consultFile = nil ;
	reconsulting = false ;
	silentConsulting = false ;
}

void ConsultInit()
{
	consultStack = StackNew() ;
	ExtraHide(consultStack) ;
	ExtraPermanent(consultStack) ;
	initializeQueue = QueueNew() ;
	ExtraHide(initializeQueue) ;
	ExtraPermanent(initializeQueue) ;
	ConsultRestart() ;
	LoadedTableInit() ;

	InstallCBuiltinPred("$$_enter_consult", 4, PEnterConsult) ;
	InstallCBuiltinPred("$$_exit_consult", 0, PExitConsult) ;
	InstallCBuiltinPred("$$_enter_include", 1, PEnterInclude) ;
	InstallCBuiltinPred("$$_exit_include", 0, PExitConsult) ;
	InstallCBuiltinPred("consulting", 1, PConsulting) ;
	InstallCBuiltinPred("$$_consult_level", 1, PConsultLevel) ;
	InstallCBuiltinPred("$$_consult_is_silent", 0, PConsultingIsSilent) ;
	InstallCBuiltinPred("$consult_clause", 1, PConsultClause) ;
	InstallCBuiltinPred("$$_consult_store_initialization", 1, PConsultStoreInit) ;
	InstallCBuiltinPred("$$_consult_get_initialization_list", 1, PConsultGetInitList) ;
	InstallCBuiltinPred("$$_check_consult", 1, PCheckConsult) ;
}
