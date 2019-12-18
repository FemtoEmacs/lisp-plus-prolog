/*
 *   This file is part of the CxProlog system

 *   ForeignEvent.cpp
 *   by A.Miguel Dias - 2006/01/05
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

#if USE_WXWIDGETS && USE_JAVA

static wxMutex polyMutex ;
static wxCondition polyCondition(polyMutex) ;

static void ForeignEventNotifier(void)
{
	polyCondition.Signal() ;
}

static void SetForeignEventNotifier(void)
{
	WxSetEventNotifier(ForeignEventNotifier) ;
	JavaSetEventNotifier(ForeignEventNotifier) ;
}

Pt ForeignEventGet()
{
	static Bool b = false ;
	wxMutexLocker lock(polyMutex) ;
	if( (b = !b) )
		for(;;) {
			if( WxHowManyEvents() > 0 ) return WxGetEvent() ;
			if( JavaHowManyEvents() > 0 ) return JavaGetEvent() ;
			polyCondition.Wait() ;
		}
	else
		for(;;) {
			if( JavaHowManyEvents() > 0 ) return JavaGetEvent() ;
			if( WxHowManyEvents() > 0 ) return WxGetEvent() ;
			polyCondition.Wait() ;
		}
}

#else

static void SetForeignEventNotifier()
{
	/* nothing */
}

Pt ForeignEventGet()
{
#if USE_WXWIDGETS
	return WxGetEvent() ;
#endif
#if USE_JAVA
	return JavaGetEvent() ;
	Error("There is no GUI active") ;
#endif
	return tNilAtom ;
}

#endif




/* CXPROLOG C'BUILTINS */

#if USE_WXWIDGETS
static void PWxEventGet()
{
	MustBe( Unify(X0, WxGetEvent()) ) ;
}

static void PWxEventIsAvailable()
{
	MustBe( WxHowManyEvents() > 0 ) ;
}

static void PWxDiscardAllEvents()
{
	WxDiscardAllEvents() ;
	JumpNext() ;
}
#endif

#if USE_JAVA
static void PJavaEventGet()
{
	MustBe( Unify(X0, JavaGetEvent()) ) ;
}

static void PJavaEventIsAvailable()
{
	MustBe( JavaHowManyEvents() > 0 ) ;
}

static void PJavaDiscardAllEvents()
{
	JavaDiscardAllEvents() ;
	JumpNext() ;
}
#endif

static void PForeignEventGet()
{
	MustBe( Unify(X0, ForeignEventGet()) ) ;
}

static void PForeignEventIsAvailable()
{
	Bool eventsAvailable = false ;
#if USE_WXWIDGETS
	eventsAvailable = eventsAvailable || WxHowManyEvents() > 0 ;
 #endif
#if USE_JAVA
	eventsAvailable = eventsAvailable || JavaHowManyEvents() > 0 ;
#endif
	MustBe( eventsAvailable ) ;

}

static void PForeignDiscardAllEvents()
{
#if USE_WXWIDGETS
	WxDiscardAllEvents() ;
#endif
#if USE_JAVA
	JavaDiscardAllEvents() ;
#endif
	JumpNext() ;
}

void ForeignEventInit()
{
#if USE_WXWIDGETS
	InstallCBuiltinPred("wx_event_get", 1, PWxEventGet) ;
	InstallCBuiltinPred("wx_event_is_available", 0, PWxEventIsAvailable) ;
	InstallCBuiltinPred("wx_event_discard_all", 0, PWxDiscardAllEvents) ;
#endif
#if USE_JAVA
	InstallCBuiltinPred("java_event_get", 1, PJavaEventGet) ;
	InstallCBuiltinPred("java_event_is_available", 0, PJavaEventIsAvailable) ;
	InstallCBuiltinPred("java_event_discard_all", 0, PJavaDiscardAllEvents) ;
#endif
	InstallCBuiltinPred("foreign_event_get", 1, PForeignEventGet) ;
	InstallCBuiltinPred("foreign_event_is_available", 0, PForeignEventIsAvailable) ;
	InstallCBuiltinPred("foreign_event_discard_all", 0, PForeignDiscardAllEvents) ;
	SetForeignEventNotifier() ;
}
