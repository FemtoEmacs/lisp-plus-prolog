/*
 *   This file is part of the CxProlog system

 *   Memory.c
 *   by A.Miguel Dias - 1989/11/14
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

/*
 * THE ELASTIC STRUCTURES:
 *   Code Area (file Memory.c),
 *   ScratchPad (file Scratch.c),
 *   Global/Local stacks (file Machine.c),
 *   Trail/Finalizers stacks (file Machine.c)
 */

#include "CxProlog.h"
#if __GLIBC__
#  include <malloc.h>
#elif __APPLE__
#  include <malloc/malloc.h>
#endif

#define DEBUG	0

/* MEMORY MANAGEMENT */

static Size memTotal = 0, memLimit = 0, programSpace = 0 ;

/* Use OWN_MEMORY_MANAGER or MALLOC ? */
#define USE_OWN_MEMORY_MANAGER		1

#if defined(USE_VALGRIND)
#  undef USE_OWN_MEMORY_MANAGER
#endif

#if USE_OWN_MEMORY_MANAGER && defined(M_MMAP_THRESHOLD)
#  define USE_MMAP		1
#  include <sys/mman.h>
#endif

/* Allocate(nWords, false) must return aligned addresses,
   compatible with the tags of the CxProlog terms:

   1 - MoreMemory aligns the boundries of the current big segment.
   2 - Allocate reserves blocks of 64-bit multiples (this also helps in
	   combating fragmentation in 32-bit machines).
 */

VoidPt AllocateSegment(Size nWords, VoidPt end)
{		/* Requests block of memory to the OS */
#if DEBUG
		Mesg("AllocateSegment = %ldKB", WordsAsKBytes(nWords)) ;
#endif
	if( memTotal + nWords > memLimit )
		FatalError("Out of memory! Failed to allocate an additional %ldKB block", WordsAsKBytes(nWords)) ;

#if USE_MMAP
	CharPt mem = mmap(nil, WordsAsBytes(nWords), PROT_READ|PROT_WRITE,
										MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) ;
	if( mem == MAP_FAILED )
#else
	CharPt mem = malloc(WordsAsBytes(nWords)) ;
	if( mem == nil )
#endif
		FatalError("Out of memory") ;

	*cHdl(end) = cPt(mem) + nWords ;
	memTotal += nWords ;
#if DEBUG
		Mesg("MemTotal = %ldKB", WordsAsKBytes(memTotal)) ;
#endif
	return mem ;
}

void ReleaseSegment(VoidPt mem, Size nWords)
{
#if DEBUG
		Mesg("ReleaseSegment = %ldKB", WordsAsKBytes(nWords)) ;
#endif
#if USE_MMAP
	if( munmap(mem, nWords) == -1 )
		FatalError("Malfunction: could not free memory") ;
#else
	free(mem) ;
#endif
	memTotal -= nWords ;
#if DEBUG
		Mesg("MemTotal = %ldKB", WordsAsKBytes(memTotal)) ;
#endif
}

char *MemPolicy(void)
{
#if USE_OWN_MEMORY_MANAGER
#  if USE_MMAP
	return "MMAP" ;
#  else
	return "HALF-MALLOC" ;
#  endif
#else
	return "MALLOC" ;
#endif
}


/* PROGRAM SPACE */

#if USE_OWN_MEMORY_MANAGER

typedef struct Block {
	Size size ;
	struct Block *next ;
} Block, *BlockPt ;

#define cBlockPt(b)				((BlockPt)(b))

#define BlockSize(b)			( cBlockPt(b)->size )
#define BlockNext(b)			( cBlockPt(b)->next )
#define BlockExternalize(b)		cBlockPt(cPt(b) + 1)
#define BlockInternalize(b)		cBlockPt(cPt(b) - 1)
#define BlockEnd(b)				cBlockPt(cPt(b) + BlockSize(b))

#define initialSegmentSize		64 K /* words */
#define maxSegmentSize			128 K /* words */

static Pt memA = nil, memZ = nil ;
static BlockPt freeList = nil ;

static void MoreMemory(Size atLeastWords)
{
	Size words ;
	Size oldProgramSpace = programSpace ;
	atLeastWords += 3 ; /* may be need for the alignment later */
	for( words = Min(Max(programSpace, initialSegmentSize), maxSegmentSize)
							; words < atLeastWords ; words *= 2 ) ;
	if( memZ - memA > 0 )
		Release(memA, memZ - memA) ;
	memA = AllocateSegment(words, &memZ) ;
	programSpace += words ;
	while( (cWord(memA) & memAlignMask) != 0 ) memA++ ;
	while( (cWord(memZ) & memAlignMask) != 0 ) memZ-- ;
/* Must be at the end to ensure there is memory to run MemoryGrowWarning */
	if( oldProgramSpace > 0 )  /* first growth, from 0, never shown */
		MemoryGrowInfo("program space", oldProgramSpace, programSpace) ;
}

#if DEBUG
#define check(x)	(cWord(x) & memAlignMask ? FatalError("Alignment error") : (x))
#else
#define check(x)	(x)
#endif

VoidPt Allocate(Size nWords, Bool saveSize)
{
	register BlockPt *p, q ;
/* add extra word for storing size, if it is the case */
	if( saveSize ) nWords++ ;
/* ensures an even number of words */
	if( odd(nWords) ) nWords++ ;
/* search freeList for a large enough block */
	for( p = &freeList ; ; p = &BlockNext(*p) ) {
		if( *p == nil ) { /* no suitable block was found */
			while( memZ - memA < nWords )
				MoreMemory(nWords) ;
			q = cBlockPt(memA) ;
			BlockSize(q) = nWords ;	/* useless if !saveSize */
			memA += nWords ;

		}
		elif( BlockSize(*p) < nWords )	/* does not fit */
			continue ;
		elif( BlockSize(*p) == nWords ) { /* found block with the exact size */
			q = *p ;
			*p = BlockNext(*p) ;
		}
		elif( saveSize && BlockSize(*p) - nWords <= 2 ) { /* size is good enough */
			q = *p ;
			*p = BlockNext(*p) ;
		}
		else {			/* too big a block: extract segment at the end */
			BlockSize(*p) -= nWords ;
			q = BlockEnd(*p) ;
			BlockSize(q) = nWords ;	/* useless if !saveSize */
		}
		return saveSize ? BlockExternalize(q) : check(q) ;
	}
}

void Release(VoidPt mem, Size nWords)
{	/* nWords == -1 means "use the size stored in the block" */
	register BlockPt p, n, f ;
/* prepare */
	if( nWords == -1 )
		f = BlockInternalize(mem) ;
	else {
		if( odd(nWords) ) nWords++ ; /* ensures an even number of words */
		f = mem ;		/* make a fake block */
		BlockSize(f) = nWords ;
	}

/* is freeList empty? */
	if( freeList == nil ) {
		BlockNext(f) = nil ;
		freeList = f ;
		return ;
	}
/* will f the the first block in freeList? */
	if( f < freeList ) {
		if( BlockEnd(f) == freeList ) {
			BlockSize(f) += BlockSize(freeList) ;
			BlockNext(f) = BlockNext(freeList) ;
		}
		else BlockNext(f) = freeList ;
		freeList = f ;
		return ;
	}
/* search */
	for( p = freeList ; (n = BlockNext(p)) != nil && n < f ; p = n ) ;
/* join to lower block */
	if( BlockEnd(p) == f ) {
		BlockSize(p) += BlockSize(f) ;
		f = p ;
	}
	else BlockNext(p) = f ;
/* join to upper block */
	if( BlockEnd(f) == n ) {
		BlockSize(f) += BlockSize(n) ;
		BlockNext(f) = BlockNext(n) ;
	}
	else BlockNext(f) = n ;
}

VoidPt Reallocate(VoidPt mem, Size oldSize, Size newSize)
{	/* oldSize == -1 means "use the size stored in the block" */
	VoidPt newMem ;
	Bool saveSize = oldSize == -1 ;
	Size copySize = saveSize ? BlockSize(BlockInternalize(mem)) : oldSize ;
	if( copySize >= newSize ) InternalError("Reallocate") ;
	newMem = Allocate(newSize, saveSize) ;
	CopyWords(newMem, mem, copySize) ;
	Release(mem, oldSize) ;
	return newMem ;
}

Size MemBlockSize(VoidPt mem)
{
	return BlockSize(BlockInternalize(mem)) - 1 ;
}

Size ProgramSpaceSize()
{
	return programSpace ;
}

Size ProgramSpaceUsed()
{
	register BlockPt p ;
	register Size size ;
	size = programSpace ;
	doseq(p, freeList, BlockNext(p))
		size -= BlockSize(p) ;
	size -= memZ - memA ;
	return size ;
}

#else
/* USE MALLOC */

VoidPt Allocate(Size nWords, Bool saveSize)
{
	Unused(saveSize) ;
	VoidPt end, mem = AllocateSegment(nWords, &end) ;
	programSpace += nWords ;
	return mem ;
}

void Release(VoidPt mem, Size nWords)
{
	ReleaseSegment(mem, nWords) ;
	programSpace -= nWords ;
}

VoidPt Reallocate(VoidPt mem, Size oldSize, Size newSize)
{
	CharPt newMem ;
	if( (newMem = realloc(mem, WordsAsBytes(newSize))) == nil )
		Error("No more computer memory available") ;
	memTotal += newSize - oldSize ;
	programSpace += newSize - oldSize ;
	return newMem ;
}

Size MemBlockSize(VoidPt ptr) // Used for debugging by PrepareDrfChecked()
{
#if __GLIBC__
	return Words(malloc_usable_size(ptr)) ;
#elif __MINGW32__
	return Words(_msize(ptr)) ;
#else
	Unused(ptr) ;
	return 128 K ;
#endif
}

Size ProgramSpaceSize()
{
	return programSpace ;
}

Size ProgramSpaceUsed()
{
	return programSpace ;
}
#endif

void MallocStats(void)
{
#if __GLIBC__
	malloc_stats() ;
#endif
}

void MemoryInit(void) {
	if( memLimit > 0 ) return ;
	memLimit = totalSystemMemory() / sizeof(Word) / 4 * 3 ;
#if DEBUG
	memoryWarnings_flag = true ;
	Mesg("Mem limit %ldKB", WordsAsKBytes(memLimit)) ;
	int i ;
	for( i = 0 ; i < 0 ; i++ )
		Allocate(100000000, false) ;
#endif
}
