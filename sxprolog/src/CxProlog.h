/*
 *   This file is part of the CxProlog system

 *   CxProlog.h
 *   by A.Miguel Dias - 2000/04/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _CxProlog_
#define _CxProlog_


/* OS DEPENDENCY DEFINES */

#if __linux__ || __APPLE__ || __FreeBSD__
/* Compiler: gcc >= 2.95 */
#define OS_UNIX							1
#define UNDERSTAND_EXTERNAL_ENCODINGS	1	/* for encondings beyond latin-1 */
#include <unistd.h>

#elif _WIN32
/* Compiler: gcc >= 2.95, MinGW */
#define OS_WIN							1
#define UNDERSTAND_EXTERNAL_ENCODINGS	1	/* for encondings beyond latin-1 */
#include <unistd.h>

#else
#define OS_UNKNOWN						1
#define UNDERSTAND_EXTERNAL_ENCODINGS	0
#endif


/* LIMIT DEFINES */

#define maxFunctorArity					64
#define maxPredArity					32
#define maxTermSize						64 K /* K symbols */

/* ALL INCLUDES */

#include <errno.h>	/* Must be the first because it undefines __USE_ISOC99 */

#define __USE_ISOC99					1
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <signal.h>

/* Utilities */
#include "Util.h"
#include "Memory.h"
#include "Utf8.h"
#include "Utf16.h"
#include "Utf32.h"
#include "Ucs2.h"
#include "String.h"
#include "Scribbling.h"
#include "Scratch.h"
#include "Clock.h"
#include "Version.h"
#include "Table.h"

/* Prolog Types */
#include "Extra.h"
#include "Atom.h"
#include "Number.h"
#include "Term.h"

/* Extra Types */
#include "Nil.h"
#include "Null.h"
#include "Array.h"
#include "Buffer.h"
#include "Dict.h"
#include "File.h"
#include "Process.h"
#include "Queue.h"
#include "Stack.h"
#include "Thread.h"

/* Input & Output */
#include "Locale.h"
#include "StreamProperty.h"
#include "InterLine.h"
#include "Stream.h"
#include "Mesg.h"

/* Reader & Writer */
#include "Character.h"
#include "VarDictionary.h"
#include "Operator.h"
#include "TermRead.h"
#include "TermWrite.h"

/* Prolog Database */
#include "Clause.h"
#include "Predicate.h"
#include "PredicateProperty.h"
#include "Consult.h"
#include "Unit.h"
#include "Alias.h"
#include "IVar.h"

/* Compiler */
#include "CodeGen.h"
#include "Compiler.h"
#include "Disassembler.h"

/* Abstract Machine */
#include "Arith.h"
#include "Unify.h"
#include "GCollection.h"
#include "Flags.h"
#include "Machine.h"
#include "Instructions.h"
#include "Index.h"
#include "Exception.h"

/* Contexts */
#ifndef CONTEXTS
#	define CONTEXTS					1
#endif

#if CONTEXTS == 0
#	include "Contexts0.h"
#elif CONTEXTS == 1
#	include "Contexts1.h"
#elif CONTEXTS == 2
#	include "Contexts2.h"
#elif CONTEXTS == 3
#	include "Contexts3.h"
#elif CONTEXTS == 4
#	include "Contexts4.h"
#endif

/* Debug */
#include "Attention.h"
#include "Debug.h"
#include "SysTrace.h"

/* Startup */
#include "Boot.h"

/* Inter-Operatibility */
#include "CallProlog.h"
#include "ForeignEvent.h"
#include "PythonForeign.h"
#include "Java.h"
#include "JavaAddOns.h"
#include "WxWidgets.h"
#include "WxWidgetsAddOns.h"
#include "WxWidgetsDemo.h"

/* Graphical User Interface */
#include "Gui.h"

/* OS */
#include "FileSys.h"
#include "Net.h"
#include "CmdLine.h"
#include "OS.h"

#endif
