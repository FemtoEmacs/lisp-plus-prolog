/*
 *   This file is part of the CxProlog system

 *   GCollection.c
 *   by Andre Castro, A.Miguel Dias - 2010/03/12
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNLExtra

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

#define GC	0

#if GC

static Hdl newGlobalBegin, newGlobalEnd ;

static Pt extraBits = nil ;

void MakeExtraBits(Size nWords) {
    Hdl bitsEnd ;
    extraBits = AllocateSegment(nWords, &bitsEnd) ;
    ClearWords(extraBits, nWords) ;
}

void DeleteExtraBits(Size nWords) {
    ReleaseSegment(extraBits, nWords) ;
    extraBits = nil ;
}

#define bitsof(T) (sizeof(T) * 8)

void MarkPt(Hdl h) {
    Size index = h - stacksBegin ;
    Size word = index / bitsof(Word) ;
    int bit = index % bitsof(Word) ;
    extraBits[word] |= (1 << bit) ;
}

Bool IsMarked(Hdl h) {
    Size index = h - stacksBegin ;
    Size word = index / bitsof(Word) ;
    int bit = index % bitsof(Word) ;
    return !!(extraBits[word] & (1 << bit)) ;
}

void PrintBit(Size nWords) {
	int i, j ;
	for( i = 0 ; i < nWords ; i++ ) {
		for( j = 0 ; j < bitsof(Word) ; j++ )
			printf("%d", IsMarked(stacksBegin + i * bitsof(Word) + j)) ;			
		printf("\n") ;
	}
}

void TestBits(void) {
    MakeExtraBits(16) ;
	MarkPt(stacksBegin+32) ;
	MarkPt(stacksBegin+1+32) ;
	MarkPt(stacksBegin+3+32) ;
	PrintBit(16) ;
	DeleteExtraBits(16) ;
}

static void ShowWord(Hdl h, Bool terms)
{
	Pt t = *h ;
	Write("  %08lx: ", h) ;
	Write("(%08lx) ", t) ;
	if( IsEmpty(t) )
		Write("EMPTY %lx\n", t) ;
	elif( t == nil )
		Write("NULL\n") ;
	elif( IsVar(t) ) {
		if( IsLocalRef(t) )
			Write("LOCALVAR %lx\n", t) ;
		elif( IsGlobalRef(t) )
			Write("GLOBALVAR %lx\n", t) ;
		elif( IsTrailRef(t) )
			Write("TRAILREF %08lx\n", t) ;
		elif( FunctorCheck(t) )
			Write("FUNCTOR %s\n", FunctorNameArity(cFunctorPt(t))) ;
		elif( UnitCheck(t) )
			Write("UNIT %s\n", UnitSignature(cUnitPt(t))) ;
		else
			Write("CODE %lx\n", t) ;
	}
	elif( IsAtomicStrict(t) )
		Write("ATOMIC %s\n", TermAsStr(t)) ;
	elif( IsExtra(t) )
		Write("EXTRA %s\n", XExtraAsStr(t)) ;
	elif( IsStruct(t) ) {
		if( terms )
			Write("STRUCT %s  <%lx>\n", TermAsStr(t), XPt(t)) ;
		else
			Write("STRUCT %lx\n", XPt(t)) ;
	}
	elif( IsList(t) ) {
		if( terms )
			Write("LIST %s  <%lx>\n", TermAsStr(t), XPt(t)) ;
		else
			Write("LIST %lx\n", XPt(t)) ;
	}
	else Write("????? %lx\n", t) ;
}



static void RelocateOtherRegs(Size globalOffset)
{
	if( IsCompound(C) ) C += globalOffset ;
	if( IsCompound(CH) ) CH += globalOffset ;
	H += globalOffset ;
	HB += globalOffset ;
}

static void RelocateTrail(Size globalOffset)
{
	register Hdl h ;

/* Relocate the trailed vars */
	for( h = trailBegin ; h < TR ; h++ )
		if( IsGlobalRef(*h) ) *h += globalOffset ;

/* Relocate the cut finalizers */
	for( h = cHdl(F) ; h < trailEnd ; h++ )
		if( IsGlobalRef(*h) || IsGlobalCompound(*h) )
			*h += globalOffset ;
}

static void RelocateXRegs(Size globalOffset)
{
	register int i ;
	dotimes(i, maxX)
		if( IsGlobalRef(Xc(i)) || IsCompound(Xc(i)) )
			Xc(i) += globalOffset ;
}

static void RelocateGlobalStack(Size globalOffset) {
	register Hdl a, z ;

	if( globalOffset >= 0 )
		for( a = H - 1, z = a + globalOffset ; a >= stacksBegin ; a--, z-- )
		{
			if( IsEmpty(*a) )
				InternalError("RelocateGlobalStack (Empty at GS)") ;
			elif( IsVar(*a) ) {
				if( IsGlobalRef(*a) ) *z = *a + globalOffset ;
				else *z = *a ;
			}
			elif( IsAtomic(*a) ) *z = *a ;
			elif( IsCompound(*a) ) *z = *a + globalOffset ;
			else InternalError("RelocateGlobalStack (>=0)") ;
		}
	else
		for( a = stacksBegin, z = a + globalOffset ; a < H ; a++, z++ )
		{
			if( IsEmpty(*a) )
				InternalError("RelocateGlobalStack (Empty at GS 2)") ;
			elif( IsVar(*a) ) {
				if( IsGlobalRef(*a) ) *z = *a + globalOffset ;
				else *z = *a ;
			}
			elif( IsAtomic(*a) ) *z = *a ;
			elif( IsCompound(*a) ) *z = *a + globalOffset ;
			else InternalError("RelocateGlobalStack (<0 2)") ;
		}
}


static void RelocateLocalStack(Size globalOffset)
{
	register Hdl a, lTop = TopOfLocalStack() ;

	for( a = stacksEnd - 1; a >= lTop ; a-- )
	{
		if( IsEmpty(*a) ) ;
		elif( IsVar(*a) ) {
			if( IsGlobalRef(*a) ) *a += globalOffset ;
		}
		elif( IsAtomic(*a) || IsInstruction(*a) ) ;
		elif( IsCompound(*a) ) *a += globalOffset ;
		else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}
}

static void RelocateStacks(Size globalOffset)
{	
	RelocateLocalStack(globalOffset) ;
	RelocateGlobalStack(globalOffset) ;
	RelocateXRegs(globalOffset) ;
	RelocateTrail(globalOffset) ;
	RelocateOtherRegs(globalOffset) ;
}

#define IsAlreadyForwarded(t)	( Le(newGlobalBegin, t) && Lt(t, newGlobalEnd) )

static Hdl CopyRootsSegm ( Hdl next, Hdl start, Hdl end){
	register Hdl a ;
	for( a = start; a >= end ; a-- )
	{
		if( IsEmpty(*a) ) ;
		elif( IsVar(*a) ) {
			if( IsGlobalRef(*a) ) {
				if( IsAlreadyForwarded(cPt(**a)) )
					*a = cPt(**a) ; // forward the address on stack
				else {
					*next++ = cPt(**a) ; //copy the root to the new location
					*cHdl3(*a) = next-1 ; // Forward the original object address
					*a = cPt(next-1) ; // forward the address on stack
				}						
			}
		}
		elif( IsAtomic(*a) || IsInstruction(*a) ) ;
		elif( IsCompound(*a) ) {
			if( IsList(*a) ) {
				if( IsAlreadyForwarded(*XHdl(*a)) )
					*a = TagList(*XHdl(*a)) ;  //forward the address on stack extracting the tag
				else {
					*next++ = XListHead(*a) ;  //copy the head to the new location
					*next++ = XListTail(*a) ;	//copy the tail to the new location
					XListHead(*a) = cPt(next-2) ;	// forward the original object address
					*a = TagList(next-2) ; // forward the address on stack !!!!!!!!!!!!!!!!!!!!!!
				}
			}		
			elif( IsStruct(*a) ) {
				if( IsAlreadyForwarded(*XHdl(*a)) )
					*a = TagStruct(*XHdl(*a)) ;  //forward the address on stack extracting the tag to follow the correct address
				else {
					FunctorPt f = XStructFunctor(*a) ;
					Hdl args = XStructArgs(*a) ;
					int i, n = FunctorArity(f) ;
					*next++ = cPt(f) ;	//copy the functor to the new location
					for( i = 0 ; i < n ; i++ )
						*next++ = args[i] ; //copy all the args to the new location
					args[-1] = cPt(next-1-n) ;  // put on the functor place the forward address
					*a = TagStruct(next-1-n) ; // forward the address on stack !!!!!!!!!!!!!!!!!!!!!!
				}
			}
			else
				InternalError("CopyRoots") ;
		}
		else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}
	return next;
}

static Hdl CopyMarkedRootsSegm ( Hdl next, Hdl start, Hdl end){
	//Cheney copy with two particularities - when a marked cell is visited do
	// 1- scan backward until a unmarked cell is found
	// 2- scan forward and copy marked cells into tospace until a unmarked cell is found
	register Hdl a , temp;
	for( a = start; a >= end ; a-- )
	{
		temp = XHdl(*a);
		if( IsEmpty(*a) ) ;
		elif( IsVar(*a) ) {
			if( IsGlobalRef(*a) ) {
				if( IsAlreadyForwarded(cPt(**a)) )
					*a = cPt(**a) ; // forward the address on stack
				else {
					while (IsMarked(temp)) temp--; //walk back towards lower address until an unmarked cell is found
					temp++; //put focus again in the last found marked cell
					while (IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
						if (temp==*a)
							*a = next ; // forward the address on stack
						*next++ = cPt(*temp) ; //copy the root to the new location
						*cHdl3(temp) = next-1 ; // Forward the original object address
						temp++;
					}

				}
			}
		}
		elif( IsAtomic(*a) || IsInstruction(*a) ) ;
		elif( IsCompound(*a) ) {
			if( IsList(*a) ) {
				if( IsAlreadyForwarded(*XHdl(*a)) )
					*a = TagList(*XHdl(*a)) ;  //forward the address on stack extracting the tag that putting again the right one
				else {
					while (IsMarked(temp)) temp--; //walk back towards lower address until an unmarked cell is found
						temp++; //put focus again in the last found marked cell
						while (IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
							if (temp==XHdl(*a))
								*a = TagList(next) ; // forward the address on stack
							*next++ = cPt(*temp) ; //copy the root to the new location
							*cHdl3(temp) = next-1 ; // Forward the original object address
							temp++;
						}
				}
			}
			elif( IsStruct(*a) ) {
				if( IsAlreadyForwarded(*XHdl(*a)) )
					*a = TagStruct(*XHdl(*a)) ;  //forward the address on stack extracting the tag to follow the correct address
				else {
					while (IsMarked(temp)) temp--; //walk back towards lower address until an unmarked cell is found
						temp++; //put focus again in the last found marked cell
						while (IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
							if (temp==XHdl(*a))
								*a = TagStruct(next) ; // forward the address on stack
							*next++ = cPt(*temp) ; //copy the root to the new location
							*cHdl3(temp) = next-1 ; // Forward the original object address
							temp++;
						}
				}
			}
			else
				InternalError("CopyRoots") ;
		}
		else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}
	return next;
}

static Hdl CopyRoots(Hdl next, int nX) {
	next = CopyRootsSegm(next, stacksEnd - 1, TopOfLocalStack()) ;
	return CopyRootsSegm(next, X + nX -1, X) ;
}

static Hdl CopyMarkedRoots(Hdl next, int nX) {
	next = CopyMarkedRootsSegm(next, stacksEnd - 1, TopOfLocalStack()) ;
	return CopyMarkedRootsSegm(next, X + nX -1, X) ;
}

static Hdl CopyChildren (Hdl scan , Hdl next){//!!!!!!!!!!!!!!!!!!!!!!!!!!!

	while (scan<next){
		if( IsEmpty(*scan) ) ;
		elif( IsVar(*scan) ) { //do nothing if it is just a self pointing var, just increase scan
			if( IsGlobalRef(*scan) ) { //if it is a global ref there are things to do
				if( IsAlreadyForwarded(cPt(**scan)) )
					*scan = cPt(**scan) ; // forward the address on tospace
				elif (IsLink(*scan)) {
					*next++ = cPt(**scan) ; //copy the object to the new location
					*cHdl3(*scan) = next-1 ; // Forward the original object address
					*scan = cPt(next-1) ; // forward the address on tospace
				}
				else {
					*cHdl3(*scan)= scan;
					*scan=cPt(scan);
				}
			}
			scan++; //go to the next scan object
		}
		elif( IsAtomic(*scan) || IsInstruction(*scan) ) scan++; //go to the next scan object;
		elif( IsCompound(*scan) ) {
			if( IsList(*scan) ) {
				if( IsAlreadyForwarded(*XHdl(*scan)) ){
					*scan = TagList(*XHdl(*scan)) ;  //forward the address on tospace extracting the tag to follow the correct address
					scan++; //go to the next scan object
				}
				else {
					*next++ = XListHead(*scan) ;  //copy the head to the new location
					*next++ = XListTail(*scan) ;	//copy the tail to the new location
					XListHead(*scan) = cPt(next-2) ;	// forward the original object address
					*scan = TagList(next-2) ; // forward the address on tospace !!!!!!!!!!!!!!!!!!!!!!
					scan = scan+2; //go to the next scan object
				}
			}
			elif( IsStruct(*scan) ) {
				if( IsAlreadyForwarded(*XHdl(*scan)) ){
					*scan = TagStruct(*XHdl(*scan)) ;  //forward the address on stack extracting the tag to follow the correct address
					scan++; //go to the next scan object
				}
				else {
					FunctorPt f = XStructFunctor(*scan) ;
					Hdl args = XStructArgs(*scan) ;
					int i, n = FunctorArity(f) ;
					*next++ = cPt(f) ;	//copy the functor to the new location
					for( i = 0 ; i < n ; i++ )
						*next++ = args[i] ; //copy all the args to the new location
					args[-1] = cPt(next-1-n) ;  // put on the functor place the forward address
					*scan = TagStruct(next-1-n) ; // forward the address on tospace !!!!!!!!!!!!!!!!!!!!!!
					scan = scan + n + 1; //go to the next scan object
				}
			}
			else
				InternalError("CopyChildren") ;
		}
		else {ShowWord(scan,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}
	return next;
}

static Hdl CopyMarkedChildren (Hdl scan , Hdl next){//!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Cheney copy with two particularities - when a marked cell is visited do
	// 1- scan backward until a unmarked cell is found
	// 2- scan forward and copy marked cells into tospace until a unmarked cell is found
	Hdl temp;
	while (scan<next){
		temp = cHdl(*scan);
		if( IsEmpty(*scan) ) ;
			elif( IsVar(*scan) ) { //do nothing if it is just a self pointing var, just increase scan
				if( IsGlobalRef(*scan) ) { //if it is a global ref there are things to do
					if( IsAlreadyForwarded(cPt(**scan)) )
						*scan = cPt(**scan) ; // forward the address on tospace
					else {
						while (IsMarked(temp)) temp--; //walk back towards lower address until an unmarked cell is found
							temp++; //put focus again in the last found marked cell
							while (IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
								*next++ = cPt(*temp) ; //copy the object to the new location
								*cHdl3(temp) = next-1 ; // Forward the original object address
								temp = next-1 ; // forward the address on tospace
								temp++;
							}
					}
				}
				scan++; //go to the next scan object
			}
			elif( IsAtomic(*scan) || IsInstruction(*scan) ) scan++; //go to the next scan object;
			elif( IsCompound(*scan) ) {
				if( IsList(*scan) ) {
					if( IsAlreadyForwarded(*XHdl(*scan)) ){
						*scan = TagList(*XHdl(*scan)) ;  //forward the address on tospace extracting the tag to follow the correct address
						scan++; //go to the next scan object
					}
					else {
						while (IsMarked(temp)) temp--; //walk back towards lower address until an unmarked cell is found
						temp++; //put focus again in the last found marked cell
						while (IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
							if (temp==XHdl(*scan))
								*scan = TagList(next) ; // forward the address on stack
							*next++ = cPt(*temp) ; //copy the root to the new location
							*cHdl3(temp) = next-1 ; // Forward the original object address
							temp++;
						}
					}
				}
				elif( IsStruct(*scan) ) {
					if( IsAlreadyForwarded(*XHdl(*scan)) ){
						*scan = TagStruct(*XHdl(*scan)) ;  //forward the address on stack extracting the tag to follow the correct address
						scan++; //go to the next scan object
					}
					else {
						while (IsMarked(temp)) temp--; //walk back towards lower address until an unmarked cell is found
							temp++; //put focus again in the last found marked cell
							while (IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
								if (temp==XHdl(*scan))
									*scan = TagList(next) ; // forward the address on stack
								*next++ = cPt(*temp) ; //copy the root to the new location
								*cHdl3(temp) = next-1 ; // Forward the original object address
								temp++;
							}
					}
				}
				else
					InternalError("CopyChildren") ;
			}
			else {ShowWord(scan,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}
	return next;
}

static void MarkRecursive(Pt a){
	//Mark the live data
	//when a structure is encountered mark the functor and all the internal cells
	//if it is just a cell  mark it
	if( IsVar(a) ) {
		if( IsMarked(a)) ;
			// do nothing, it is already marked
		else {
			MarkPt(a);
			MarkRecursive(*cHdl(a)); //call mark recursive with the argument of the cell pointed by the Var
		}
	}
	elif( IsAtomic(a) || IsInstruction(a) );
	elif( IsCompound(a) ) {
		if( IsList(a) ) {
			if( IsMarked(XHdl(a)) );
				// do nothing, it is already marked
			else {
				
				MarkPt(&XListHead(a));
				MarkRecursive(XListHead(a));
				MarkPt(&XListTail(a));
				MarkRecursive(XListTail(a));						
			}
		}
		elif( IsStruct(a) ) {
			if( IsMarked(XHdl(a)));
				// do nothing, it is already marked
			else {
				FunctorPt f = XStructFunctor(a) ;
				Hdl args = XStructArgs(a) ;
				int i, n = FunctorArity(f) ;
				MarkPt(args-1);
				for( i = 0 ; i < n ; i++ ){
					MarkPt(args+i); 
					MarkRecursive(args[i]);
				}
			}
		}
		else
			InternalError("MarkPhase") ;
	}
	else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}

}


static void MarkPhase(void){

	register Hdl a, lTop = TopOfLocalStack() ;
	for( a = stacksEnd - 1; a >= lTop ; a-- ) // going through the stack because it's the source for roots of the "trees"
	{
		if( IsEmpty(*a) ) ;
		elif( IsVar(*a) ) {
			if( IsGlobalRef(*a) ) {
				MarkRecursive(cHdl(*a));
			}
		}
		elif( IsAtomic(*a) || IsInstruction(*a) ) ;
		elif( IsCompound(*a) ) {
			if( IsList(*a) ) {
				MarkRecursive(*a);
			}
			elif( IsStruct(*a) ) {
				MarkRecursive(*a);
			}
			else
				InternalError("MarkPhase") ;
		}
		else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}

}




static Hdl CopyPhase(void){

	register Hdl scan=newGlobalBegin;
	register Hdl next=newGlobalBegin;

	//check for roots
	next = CopyMarkedRoots(next, 0);

	//follow children
	next = CopyMarkedChildren(scan, next);

	return next;

}

static void UpdateTrailRefs() {
	register Hdl h ;

	/* update the trail */
		for( h = trailBegin ; h < TR ; h++ )
			if( IsGlobalRef(*h) )
				*h = cPt(**h);

		for( h = cHdl(F) ; h < trailEnd ; h++ )
			if( IsGlobalRef(*h) || IsGlobalCompound(*h) )
				*h = cPt(**h);
}

static Hdl RunMarkAndCopy(void){
	MakeBits(newGlobalEnd - newGlobalBegin) ;
	MarkPhase();
	CopyPhase();
	UpdateTrailRefs();
	DeleteBits(newGlobalEnd - newGlobalBegin) ;
	return H;
}

static Hdl RunChenneyCopy(void){
	 register Hdl scan=newGlobalBegin;
	 register Hdl next=newGlobalBegin;

	//check for roots
	next = CopyRoots(next, 0);

	//follow children
	next = CopyChildren(scan, next);

	return next;
}



#if 0
static Hdl RunCollector(void) {

	H=RunChenneyCopy();
	UpdateTrailRefs();

	return H;
}
#else
static Hdl RunCollector(void)
{
	Unused(newGlobalEnd) ;
	RelocateStacks(newGlobalBegin - stacksBegin) ;
	return H ;
}
#endif

#endif


Bool ZGCollection(Size ensureThisExtraSpace, int arity, Bool precision)
{
#if GC
	Hdl saveStacksBegin = stacksBegin ;
	newGlobalBegin = AllocateSegment(H - stacksBegin, &newGlobalEnd) ;
	H = RunCollector() ;
	stacksBegin = newGlobalBegin ;
	RelocateStacks(saveStacksBegin - newGlobalBegin) ;
	stacksBegin = saveStacksBegin ;	
	ReleaseSegment(newGlobalBegin, newGlobalEnd - newGlobalBegin) ;
	return true ;
#else
	Unused(arity) ;
	Unused(precision) ;
	return ZStacksExpansion(ensureThisExtraSpace) ;	
#endif
}

static Size gcTestRate = 0 ;

Bool ZTestGCollection(Size ensureThisExtraSpace, int arity, Bool precision)
{
#if GC
	static int n = 0 ;
	if( gcTestRate == 0 )
		return false ;
	if( ++n < gcTestRate )
		return false ;
	else
		n = 0 ;
	ZGCollection(ensureThisExtraSpace, arity, precision) ;
	return true ;
#else
	Unused(ensureThisExtraSpace) ;
	Unused(arity) ;
	Unused(precision) ;
	return ZTestStacksExpansion() ;	
#endif
}


/* CXPROLOG C'BUILTINS */

static void PGCollection()
{
	Write("PGCollection\n") ;
//	GCollection() ;
	gcTestRate = 0 ;
	JumpNext() ;
}

void GCollectionInit()
{
	InstallCBuiltinPred("gc", 0, PGCollection) ;
}
