/*
 *   This file is part of the CxProlog system

 *   OSUnknown.c
 *   by A.Miguel Dias - 2001/06/04
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

#if OS_UNKNOWN

/* INTERRUPT */

void OSAllowInterruptibleSysCalls()
{
}

void OSDisallowInterruptibleSysCalls()
{
}
void OSHandleRunInterruptibleFun(void)
{
}

void OSRunInterruptibleFun(Fun fun, FunI InterrHandler, BFun PostInterrHandler)
{
}


/* FILESYS */

/* OSExists: Also requires that the file is readable which is
    too big a requirement. However this is the best we can
    do using the stdio lib alone */

Bool OSExists(Str fname)
{
	FILE *f;
	if( (f = fopen(FileNameExternalize(fname, false), "r")) != nil ) {
		fclose(f) ;
		return true ;
	}
	return false ;
}

Bool OSMkdir(Str fname)
{
	return false ;
}

Pt OSPropType(Str fname)
{
	return OSExists(fname) ? tNilAtom : nil ;
}

Pt OSPropSize(Str fname)
{
	return minusOneIntPt ;
}

Bool OSPropReadable(Str fname)
{
	return OSExists(fname) ;
}

Bool OSPropWritable(Str fname)
{
	FILE *f;
	if( (f = fopen(FileNameExternalize(fname, false), "r+")) != nil ) {
		fclose(f) ;
		return true ;
	}
	return false ;
}

Pt OSPropTime(Str fname)
{
	Pt t[2] ;
	t[0] = zeroIntPt ;
	t[1] = zeroIntPt ;
	return ArrayToList(t, 2) ;
}

CharPt OSGetCurrDir()
{
	return "" ;
}

Bool OSSetCurrDir(Str s)
{
	return true ;
}

Pt OSFiles()
{
	return tNilAtom ;
}

CharPt OSPrefixDir()
{
	return OSApplDir() ;
}

CharPt OSApplSubdir()
{
	return "" ;
}

void OSDeleteTree(Str dirPath)
{
}

void OSFileSysInit()
{
	Warning("CxProlog compiled in mode OS_UNKNOWN. Most of the OS functionality is disabled") ;
}


/* SPECIAL I/O INPUT */

Bool SetRawInput(FILE *file)
{
	return false ;
}

void UnsetRawInput()
{
}

Bool OSIsATty(FILE *file)
{
	return file == stdin || file == stdout || file == stderr ; /* Incorrect really */
}


/* SOCKETS */

int OSInstallServer(int port, int queueLen)
{
	Error("Sockets not supported on this OS.") ;
	return 0 ;
}

void OSAccept(int server, FILE **r, FILE **w)
{
	Error("Sockets not supported on this OS.") ;
}

void OSUninstallServer(int server)
{
	Error("Sockets not supported on this OS.") ;
}

void OSConnect(Str host, int port, FILE **r, FILE **w)
{
	Error("Sockets not supported on this OS.") ;
}

PInt OSEncodeInt(PInt i)
{
	return i ;
}

PInt OSDecodeInt(PInt i)
{
	return i ;
}


/* PROCESSES/THREADS */

long OSGetPid()
{
	return -1 ;
}

long OSGetPPid()
{
	return -1 ;
}

long OSGetTid()
{
	return -1 ;
}

void OSBlockSignals()
{
}

long OSFork()
{
	Error("Processes not supported on this OS.") ;
	return 0 ;
}

void OSWait()
{
	Error("Processes not supported on this OS.") ;
}

void OSKill(int pid)
{
	Error("Processes not supported on this OS.") ;
}

void OSSleep(Size secs)
{
	Error("Processes not supported on this OS.") ;
}

Bool OSRun(Str command)
{
	StreamFlushAll() ;
	return system(StrExternalize(command)) == 0 ;
}

void OSPipe(int *fd)
{
	Error("Processes not supported on this OS.") ;
}

long OSPipeBufferSize()
{
	Error("Processes not supported on this OS.") ;
	return 0 ;
}

void OSWrite(int fd, VoidPt buf, Size size)
{
	Error("Processes not supported on this OS.") ;
}

Bool OSRead(int fd, VoidPt buf, Size size, Bool blocking)
{
	Error("Processes not supported on this OS.") ;
	return false ;
}

CharPt OSGetEnv(Str envVarName, Bool err)
{
	CharPt res = StrInternalize(getenv(StrExternalize(envVarName))) ;
	if( res == nil && err )
		Error("Environment variable '%s' does not exist", envVarName) ;
	return res ;
}

void OSSetEnv(Str envVarName, Str newValue, Bool err)
{
	CharPt newEnv = GStrFormat("%s=%s", envVarName, newValue) ;
	int res = putenv(StrAllocate(StrExternalize(newEnv))) ;
	if( res != 0 && err )
		Error("Could not change environment variable '%s'", envVarName) ;
}

CharPt OSPathSeparator()
{
	return ":" ;
}

CharPt OSGetUserPath(Str username, Bool err)
{
	return "./" ;
}

CharPt OSGetPreferencesPath(Bool err)
{
	return "./" ;
}

CharPt OSGetCachePath(Bool err)
{
	return "./" ;
}

CharPt OSGetTmpPath(Bool err)
{
	return "./" ;
}



/* MISC */

Size totalSystemMemory(void)
{
	return 1024 * 1024 * 1024 ;  // Assume 1 GB
}

CharPt OSName(void)
{
	return "unknown" ;
}

Bool CreateConsole(void)
{
	return true ;
}

void DeleteConsole(void)
{
	/* Nothing */
}

#endif /* OS_UNKNOWN */
