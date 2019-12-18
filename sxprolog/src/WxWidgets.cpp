/*
 *   This file is part of the CxProlog system

 *   WxWidgetsBase.cpp
 *   by A.Miguel Dias - 2005/11/25
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL

 *   it under the terms of the GNU General Public License as published by
 *   CxProlog is free software; you can redistribute it and/or modify
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


#if USE_WXWIDGETS
#	include <wx/wx.h>
#endif

extern "C" {
#	include "CxProlog.h"
}

#if USE_WXWIDGETS

/* We assume that a wx-predicate never returns the null C++ object. 
   So, a Prolog program never needs to be concerned with this
   special object. (As for the null Java object, matters are different
   because Java can be called directly from Prolog, no need to write a
   java-predicate.)
*/

/* WXOBJ */

static ExtraTypePt wxobjType ;

#define cWxObjPt(x)			((WxObjPt)(x))

#define WxObjValue(w)		(cWxObjPt(w)->value)

static Bool LookupWxObjAux(CVoidPt x, CVoidPt ref)
{
	return WxObjValue(x) == ref ;
}
WxObjPt LookupWxObj(VoidPt obj)	/* pre: obj must be an alive wx object or nil */
{
	register WxObjPt w ;
	UChar slot = cWord(obj) >> 3 ;
	if( (w = cWxObjPt(ExtraFindFirst(wxobjType, slot, LookupWxObjAux, obj))) == nil ) {
		w = cWxObjPt(ExtraNew(wxobjType, slot)) ;
		WxObjValue(w) = obj ;
	}
	return w ;
}

Pt MakeWxObj(VoidPt obj)	/* pre: obj must be an alive wx object or nil */
{
	return TagExtra(wxobjType, LookupWxObj(obj)) ;
}

static void CreateNullStuff(void)
{
	WxObjPt nullWxObj = cWxObjPt(ExtraNew(wxobjType, 0)) ;
	WxObjValue(nullWxObj) = nil ;
	ExtraPermanent(nullWxObj) ;
}

void WxDeleted(VoidPt obj) /* Available to the Gui thread */
{				/* pre: obj must has really been deleted (will not reappear) */
    register WxObjPt w ;
    UChar slot = cWord(obj) >> 3 ;
	if( (w = cWxObjPt(ExtraFindFirst(wxobjType, slot, LookupWxObjAux, obj))) != nil )
		ExtraDisable(w) ;
}

VoidPt XTestWxObj(Pt t) /* t may represent an alive or dead object */
{
	return WxObjValue((XTestExtra(wxobjType, t))) ;
}

static Size WxObjSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(WxObj) ;
}

static Bool WxObjBasicGCDelete(VoidPt x)
{
	return ExtraIsDisabled(x) ; /* @@@ */
}



/* CxProlog Event Queue */

/* Warning: the event handling loop of wxWidgets runs on the Gui thread
			and should not modify any part of the CxProlog global state. It
			should also handle silently any errors. */

#define eventQueueCapacity	4096	/* Allows around 100 events in the queue */
#define eventEndMark		TagStruct(0)

static CharPt qBegin, qEnd, qFirst, qLast ;
static int nEvents ;

static wxMutex qMutex ;
static wxCondition qCondition(qMutex) ;
static Fun eventNotifier = nil ;

static void EventQueueReset(void)
{
	qFirst = qBegin ;
	qLast = qBegin ;
	nEvents = 0 ;
}

static void EventQueueNew(void)
{
	qBegin = cCharPt(Allocate(eventQueueCapacity, false)) ;
	qEnd = qBegin + eventQueueCapacity ;
	EventQueueReset() ;
}

static void EventQueuePark(void) /* Available to the Gui thread */
{
	if( qFirst != qBegin ) {
		register CharPt f, b ;
		for( b = qBegin, f = qFirst ; f < qLast ; *b++ = *f++ ) ;
		qFirst = qBegin ;
		qLast = b ;
	}
}

static void EventQueueDeleteFirst(void) /* Available to the Gui thread */
{
	for(;;)
		switch( *qFirst++ ) {
			case '.': qFirst += 0 ; break ;
			case 'z': goto exit ;
			case 'i': qFirst += sizeof(int) ; break ;
			case 'f': qFirst += sizeof(double) ; break ;
			case 't': qFirst += sizeof(Pt) ; break ;
			case 'o': qFirst += sizeof(VoidPt) ; break ;
			case 's': qFirst += strlen(qFirst) + 1 ; break ;
			default: /* ignore */ ; break ;
		}
exit:
	nEvents-- ;
}

static Bool EventQueueMakeSpace(int w) /* Available to the Gui thread */
{
	if( qLast + w > qEnd ) {
		EventQueuePark() ;
		while( qLast + w > qEnd && nEvents > 0 ) {
			EventQueueDeleteFirst() ;
			EventQueuePark() ;
		}
		if( qLast + w > qEnd )
			return false ;
	}
	return true ;
}

static Pt EventQueueGetItem(void)
{
	Pt t ;
	if( qFirst < qLast )
		switch( *qFirst++ ) {
			case '.':
				return nil ;
			case 'z':
				return eventEndMark ;
			case 'i':
				t = MakeInt(*(int *)qFirst) ;
				qFirst += sizeof(int) ;
				return t ;
			case 'f':
				t = MakeFloat(*(double *)qFirst) ;
				qFirst += sizeof(double) ;
				return t ;
			case 't':
				t = *(Pt *)qFirst ;
				qFirst += sizeof(Pt) ;
				return t ;
			case 'o':
				t = MakeWxObj(*(VoidPt *)qFirst) ;
				qFirst += sizeof(VoidPt) ;
				return t ;
			case 's':
				t = MakeTempAtom(qFirst) ;
				qFirst += strlen(qFirst) + 1 ;
				return t ;
			default:
				break ;
		}
	return (Pt)InternalError("EventQueueGetItem") ;
}

/* The first argument of WxPostEvent is a format string, e.g. "so.si", that
   specifies the structure and type of a Prolog structured term (a tree).
   The ensuing arguments are the actual values of the nodes of the tree
   (a tree that will be build later by the ForeignEventGet(.) function). Of course,
   the sequence of arguments should match the sequence of type specifiers
   in the format string. The supported type specifiers, that can occur in
   the format string, are the following:
		'i' - integer
		'f' - float
		't' - atomic prolog term
		'o' - wx object
		's' - string
   The special character '.' is instrumental in defining the structure of
   the tree. The structure and type of a Prolog tree is recursively defined
   using the following rules:
        1. The first character of the format string is interpreted as the
		   type of the root, one of the following: 'i', 'f', 't', 'o', 's'.
        2. Next appears the type specifications of the subtrees.
        3. As a final rule, every sequence of type specifications of subtrees
		   is finished by a '.' mark.
	All inner nodes must be strings because they will be converted to
    Prolog functors. On the other hand, the final '.'s of the format
	string may be omitted, so that "so.si" becomes equivalent to "so.si...".
	Example:
		WxPostEvent("so.si", "event", this, "tick", 54) ;
						------> event(1'WXOBJ_407f0e40,tick(54))
*/

void WxPostEvent(Str fmt, ...) /* Available to the Gui thread */
{
	wxMutexLocker lock(qMutex) ;
	va_list v ;
	va_start(v, fmt) ;
	while( *fmt )
		switch(*fmt++ ) {
			case '.':
				if( !EventQueueMakeSpace(1) ) goto abort ;
				*qLast++ = '.' ;
				break ;
			case 'i':
				if( !EventQueueMakeSpace(1 + sizeof(int)) ) goto abort ;
				*qLast++ = 'i' ;
				*(int *)qLast = va_arg(v, int) ;
				qLast += sizeof(int) ;
				break ;
			case 'f':
				if( !EventQueueMakeSpace(1 + sizeof(double)) ) goto abort ;
				*qLast++ = 'f' ;
				*(double *)qLast = va_arg(v, double) ;
				qLast += sizeof(double) ;
				break ;
			case 't':
				if( !EventQueueMakeSpace(1 + sizeof(Pt)) ) goto abort ;
				*qLast++ = 't' ;
				*(Pt *)qLast = va_arg(v, Pt) ;
				qLast += sizeof(Pt) ;
				break ;
			case 'o':
				if( !EventQueueMakeSpace(1 + sizeof(VoidPt)) ) goto abort ;
				*qLast++ = 'o' ;
				*(VoidPt *)qLast = va_arg(v, VoidPt) ;
				qLast += sizeof(VoidPt) ;
				break ;
			case 's': {
				Str s = va_arg(v, CharPt) ;
				int len = strlen(s) + 1 ;
				if( !EventQueueMakeSpace(1 + len) ) goto abort ;
				*qLast++ = 's' ;
				strcpy(qLast, s) ;
				qLast += len ;
				break ;
			}
			default:
				goto abort ;
				break ;
		}

	if( !EventQueueMakeSpace(1) ) goto abort ;
	*qLast++ = 'z' ;
    va_end(v) ;
	nEvents++ ;
	if( nEvents == 1 ) {
		qCondition.Signal() ;
		if( eventNotifier != nil )
			eventNotifier() ;
	}
	return ;
abort:
    va_end(v) ;
	EventQueueReset() ;
}

void WxSetEventNotifier(Fun f)
{
	eventNotifier = f ;
}

Pt WxGetEvent(void)
{
	Pt elem, t ;
	FunctorPt f ;
	wxMutexLocker lock(qMutex) ;
	if( nEvents == 0 ) qCondition.Wait() ;
	ScratchSave() ;
	elem = nil ;
	for(;;) {
		if( elem == eventEndMark
			|| (elem = EventQueueGetItem()) == eventEndMark
			|| elem == nil )
		{
			int a = ScratchCurr() - ScratchStart() - 1 ;
			t = *ScratchStart() ;
			if( a > 0 ) {
				if( !IsAtom(t) ) {
					EventQueueReset() ;
					Error("Invalid wxWidgets serialized term") ;
				}
				f = LookupFunctor(XAtom(t), a) ;
				t = MakeStruct(f, ScratchStart() + 1) ;
			}
			FreeScratch() ;
			if( ScratchDistToSaved() == 0 ) break ;
			else ScratchPush(t) ;
		}
		else {
			UseScratch() ;
			ScratchPush(elem) ;
		}
	}
	nEvents-- ;
	return t ;
}

int WxHowManyEvents(void)
{
	return nEvents ;
}

void WxDiscardAllEvents(void)
{
	wxMutexLocker lock(qMutex) ;
	EventQueueReset() ;
}



/* String conversions */

Str WxStrInternalize(const wxString& s) /* Available to the Gui thread */
{
	// old wx return StrInternalizeW((WCharPt)(s.wc_str().data())) ;
	return StrInternalizeW((WCharPt)(s.wc_str())) ;
/*
wxString::wchar_str
wxWritableWCharBuffer wchar_str() const

Returns an object with string data that is implicitly convertible to char* pointer.
Note that any change to the returned buffer is lost and so this function is only
usable for passing strings to legacy libraries that don't have const-correct API.
Use wxStringBuffer if you want to modify the string.


wxString::wc_str() 
wxString::wc_str

const wchar_t* wc_str(wxMBConv& conv) const

const wxWCharBuffer wc_str(wxMBConv& conv) const

Returns wide character representation of the string. In ANSI build, converts
using conv's cMB2WC method and returns wxWCharBuffer. In Unicode build, this
function is same as c_str. The macro wxWX2WCbuf is defined as the correct return
type (without const).
*/
}


/* The CxProlog Thread */

#if !wxUSE_THREADS
#	error "Thread support required by CxProlog with wxWidgets."
#endif

static Bool hasWxApp = false ;

class CxPrologThread : public wxThread
{
public:
	ExitCode Entry() {
		int i ;
/* @@@ handling arguments is not quite wright yet. */
		MemoryInit();
		StringInit() ;
		CharHdl av = (CharHdl)Allocate(wxTheApp->argc, false) ;
		for( i = 0 ; i < wxTheApp->argc ; i++ )
			av[i] = StrAllocate(WxStrInternalize(wxTheApp->argv[i])) ;
		if( RunInteractiveProlog(wxTheApp->argc, av) != 0 )
			hasWxApp = false ;
		return (wxThread::ExitCode)0 ;
	}
	void OnExit() {
		hasWxApp = false ;
		wxTheApp->ExitMainLoop() ;
		wxWakeUpIdle() ;        /* Without this the application freezes. */
	}
} ;

/* The GUI Thread

   The GUI library is not multi-thread aware in some systems, such as Windows.
   Therefore all the GUI activity must take place within the same GUI thread.
   The CxProlog thread must request a GUI service by sending a event to the GUI
   thread and waiting for the completion of that service - this is synchronous
   communication. The function WxGuiCall(.) encapsulates all these details
   and is used in the CxProlog thread to call GUI services. */

enum { ID_GuiCall } ;

class GuiApp: public wxApp
{
public:
	GuiApp() : wxApp(), guiMutex(), guiCondition(guiMutex),
				guiEvent(wxEVT_COMMAND_MENU_SELECTED, ID_GuiCall) {
	}
	bool OnInit() {
	/* Flags that the wx-application has been created. Must be here, before
		the creation of the CxProlog thread */
		hasWxApp = true ;
	/* Creates the CxProlog thread */
		wxThread *thr = new CxPrologThread() ;
		if( thr == nil
			|| thr->Create() != wxTHREAD_NO_ERROR
			|| thr->Run() != wxTHREAD_NO_ERROR
			|| !hasWxApp )
				return FALSE ;
	/* CxProlog survives the deletion of all windows */
		SetExitOnFrameDelete(false) ;
	/* At least one window must exist to activate the event handler */
		new wxFrame(nil, -1, wxT("invisible window")) ;
	/* Blocks all signals in the GUI thread so that the MAIN thread
       will receive all of them */
		OSBlockSignals() ;
	/* Now, activates the event handler */
		return hasWxApp ;
	}
	void GuiCall(Fun f) {	/* Called in the CxProlog thread */
		fun = f ;
		wxPostEvent(wxTheApp, guiEvent) ;
		wxMutexLocker lock(guiMutex) ;
		if( fun != nil )			/* Signal not yet issued? */
			guiCondition.Wait() ;		/* CxProlog waits completion */
		if( pevent != peNone )
			SendPrologEvent(pevent) ;	/* Propagate delayed event */
	}
	void OnGuiCall(wxCommandEvent& WXUNUSED(aEvent)) { /* Runs in the GUI thread */
		pevent = CallFunThruProlog(fun) ;
		wxMutexLocker lock(guiMutex) ;
		guiCondition.Signal() ;	/* GUI signals completion */
		fun = nil ;				/* Marks signal already issued */
	}
private:
	PrologEvent pevent ;
	wxMutex guiMutex ;
	wxCondition guiCondition ;
	Fun fun ;
	wxCommandEvent guiEvent ;
	DECLARE_EVENT_TABLE()
} ;

BEGIN_EVENT_TABLE(GuiApp, wxApp)
	EVT_MENU(ID_GuiCall,	GuiApp::OnGuiCall)
END_EVENT_TABLE()

IMPLEMENT_APP(GuiApp)

void WxGuiCall(Fun f)	/* Called in the CxProlog thread */
{
	((GuiApp *)wxTheApp)->GuiCall(f) ;
}

 /* The implementation of all wxWidgets based built-in predicates uses the
   WxWidgetsCall abstract machine instruction. This instruction invokes
   three different functions, in sucession:
	- The first function runs in the CxProlog thread and performs the
	  analysis of the predicate arguments;
	- The second function runs in the GUI thread and implements the required
	  wxWidgets service.
	- The third function runs in the CxProlog thread and builds the results
	  of the predicate.
*/

void WxWidgetsCallInst() /* Abstract machine instruction */
{
	(*cFun(*P++))() ;
	WxGuiCall(*cFun(*P++)) ; /* No Prolog errors allowed here */
	(*cFun(*P++))() ;
	JumpNext() ;
}

void WxWidgetsCall2Inst() /* Abstract machine instruction */
{
	(*cFun(*P++))() ; /* No Prolog errors allowed here */
	(*cFun(*P++))() ; /* No Prolog errors allowed here */
	(*cFun(*P++))() ; /* No Prolog errors allowed here */
	JumpNext() ;
}

void InstallWxGuiBuiltinPredAux(Str n, int a, Fun fINIT, Fun fGUI, Fun fEND)
{
	PredicatePt pr = InstallCBuiltinPred(n, a, fINIT) ;
	BindPredicateFull(pr,
				hasWxApp ? WxWidgetsCall : WxWidgetsCall2,
				cPt(fINIT), cPt(fGUI), cPt(fEND), Proceed) ;
}


/* CXPROLOG C'BUILTINS */

static Size WxObjsAux(CVoidPt x)
{
	Write("pt(0x%08lx)", WxObjValue(x)) ;
	return 1 ;
}
static void PWxObjs()
{
	ExtraShow(wxobjType, WxObjsAux) ;
	JumpNext() ;
}

static void PWxCheckINIT() { /* nothing */ }
static void PWxCheckGUI() {
	wxFrame *w = new wxFrame((wxFrame *)NULL, -1, wxT("Hello"),
										wxPoint(50,50), wxSize(450,340)) ;
									/* Need to redefine Delete... */
	wxTextCtrl *txtCtrl = new wxTextCtrl(w, -1, wxT(""), wxPoint(0, 0),
								wxSize(0, 0), wxTE_MULTILINE | wxTE_READONLY) ;
	txtCtrl->WriteText(wxT("WXWidgets is working!")) ;
	w->Raise() ;
	w->Show(TRUE) ;
}
void PWxCheckEND() { /* nothing */ }

void WxWidgetsInit()
{
#if 0
	Mesg("hasWxApp = %d", hasWxApp) ;
#endif
	wxobjType = ExtraTypeNew("WXOBJ", WxObjSizeFun, nil, WxObjBasicGCDelete, 256) ;
	CreateNullStuff() ;
	EventQueueNew() ;
	InstallCBuiltinPred("wxobjs", 0, PWxObjs) ;
	InstallWxGuiBuiltinPred("wx_check", 0, PWxCheck) ;
	InstallWxGuiBuiltinPred("zwx", 0, PWxCheck) ;

	RegisterForeignInterface("wxWidgets", "wx") ;
}

#else /* USE_WXWIDGETS */

void WxWidgetsCallInst() { Error("WxWidgets is inactive") ; }
void WxWidgetsCall2Inst() { Error("WxWidgets is inactive") ; }
void WxWidgetsInit() { /* nothing */ }

#endif /* USE_WXWIDGETS */
