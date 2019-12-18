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

#define GC	1

static Pt trailFreeVar ;
static Size gcTestRate = 0 ;

#if GC

#define IsUnit(t)	(IsVar(t) && !IsStacksRef(t))

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

Bool IsAlreadyForwarded(Pt t) {
   t = XPt(t) ;
   return  Le(newGlobalBegin, t) && Lt(t, newGlobalEnd) ;
}

/* Prep grow */

static void growNewGlobal(Hdl *next, Hdl *scan, Size request, Str mesg)
{
	//Mesg("growNewGlobal %s", mesg) ;
	Size newGlobalSize = newGlobalEnd - newGlobalBegin ;
	Size globalOffset2 ;
	Hdl saveStacksBegin, saveH ;
	Hdl newGlobalBegin2, newGlobalEnd2 ;
	Size newGlobalSize2 = request != 0 ? request : 2 * newGlobalSize ;
	Hdl a ;

/* Create new area */
	newGlobalBegin2 = AllocateSegment(newGlobalSize2, &newGlobalEnd2) ;
	globalOffset2 = newGlobalBegin2 - newGlobalBegin ;
	
/* Copy and relocate to new area everything except the original global stack */
	saveH = H ;
	saveStacksBegin = stacksBegin ;
	H = *next ;
	stacksBegin = newGlobalBegin ;
	RelocateStacks(globalOffset2, 0) ;
	stacksBegin = saveStacksBegin ;
	H = saveH ;

	for( a = stacksBegin ; a < H ; a++ )
		if( (IsVar(*a) || IsCompound(*a)) && IsAlreadyForwarded(*a) )
			*a += globalOffset2 ;

	ReleaseSegment(newGlobalBegin, newGlobalSize) ;
	newGlobalBegin = newGlobalBegin2 ;
	newGlobalEnd = newGlobalEnd2 ;
	*next += globalOffset2 ;
	if( scan != nil )
		*scan += globalOffset2 ;
}


/* Prep grow */

static Hdl CopyRootsSegm ( Hdl next, Hdl start, Hdl end, Bool local){
	/* @@@@ falta verificar se nos finalizadores há roots */
	register Hdl a ;
	for( a = start; a >= end ; a-- )
	{
//		if( next >= newGlobalEndZZ )
//			Mesg("RRRRRRRRRRR %ld", next - newGlobalEndZZ) ;
//Mesg("%lx %lx", a, next) ;
		if( IsEmpty(*a) ) ;
		elif( IsVar(*a) ) {
			if( IsGlobalRef(*a) ) {
	//			Mesg("global var") ;
				if (local && IsAChoicePointHAddr(a))
					WriteNothing("%lx Ignored H In Choise Point  \n", a);
				else if( IsAlreadyForwarded(cPt(**a)) ){
					*a = XPt(**a) ; // forward the address on stack - the Xpt is because of the tag list on forward pointers
					WriteNothing("%lx Forwarded GlobalRef \n", a);
				}
				else {
				//WriteNothing("is a not forwarded var %lx %lx", cPt(**a), next-1) ;
					WriteNothing("%lx Copied GlobalRef \n", a);
					if( next + 1 >= newGlobalEnd ) /* Grow if necessary */
						growNewGlobal(&next, nil, 0, "var") ;
					*next++ = cPt(**a) ; //copy the root to the new location
					*cHdl3(*a) = next-1 ; // Forward the original object address
					*a = cPt(next-1) ; // forward the address on stack
				}					
			}
			else WriteNothing("%lx Ignored NormalVAR \n", a);
		}
		// id(X) :- true.   p :- true, id([X]), id([Y]), X = Y, Y = 99, mshow(global), true.
		elif( IsAtomic(*a) || IsInstruction(*a) ) WriteNothing("%lx AtomicOrInstruct \n", a);
		elif( IsCompound(*a) ) {
			if( IsList(*a) ) {
		//		Mesg("list") ;
				if( IsAlreadyForwarded(XListHead(*a)) && IsList(XListHead(*a)))  //check if is already forwarded and if the FP has a tag list
				{	//	WriteNothing(" [list is forwarded %lx %lx] ", *a, XListHead(*a)) ;
						WriteNothing("%lx Forwarded List \n", a);
						*a = TagList(XListHead(*a)) ;
				} //forward the address on stack extracting the tag
				else {
				//	WriteNothing(" [list is not forwarded %lx %lx] ", *a, XListHead(*a)) ;
					if( next + 3 >= newGlobalEnd ) /* Grow if necessary */
						growNewGlobal(&next, nil, 0, "list") ;
					if (IsUnit(XHdl(*a)[-1]) ) {
						WriteNothing("Is a UNIT %lx \n",(XHdl(*a)[-1]));
						WriteNothing("%lx copied UNIT -1 field \n", a);
						*next++=XHdl(*a)[-1];  //Copy the hidden field above the Unit
					}
				/*	if (*a==C){ //To update C Reg correctly
						C=TagList(next) ;
						WriteNothing("&&&&&&&&&&&&&&&&&&&&&&6 Updating C register from %lx to %lx \n", *a, TagList(next-2));
					}*/
					WriteNothing("%lx Copied List \n", a);
					*next++ = XListHead(*a) ;  //copy the head to the new location
					*next++ = XListTail(*a) ;	//copy the tail to the new location
					XListHead(*a) = TagList(cPt(next-2)) ;	// forward the original object address with the list tag
					WriteNothing("Vou fazer forward do stack de %lx para %lx \n", *a, (next-2) );
					*a = TagList(next-2) ; // forward the address on stack

				}
			}		
			elif( IsStruct(*a) ) {
		//		Mesg("struct") ;
				if( IsAlreadyForwarded(*XHdl(*a)) ){
					*a = TagStruct(*XHdl(*a)) ;  //forward the address on stack extracting the tag to follow the correct address
					WriteNothing("%lx Forwarded Struct \n", a);}
				else {

					WriteNothing("%lx Copied Struct \n", a);
					FunctorPt f = XStructFunctor(*a) ;
					Hdl args ;
					int i, n = FunctorArity(f) ;
					if( next + 1 + n >= newGlobalEnd ) /* Grow if necessary */
						growNewGlobal(&next, nil, 0, "struct") ;		
					args = XStructArgs(*a) ;
					*next++ = cPt(f) ;	//copy the functor to the new location
					for( i = 0 ; i < n ; i++ )
						*next++ = args[i] ; //copy all the args to the new location
					args[-1] = cPt(next-1-n) ;  // put on the functor place the forward address
					*a = TagStruct(next-1-n) ; // forward the address on stack
				}
			}
			else
				InternalError("CopyRoots") ;
		}
		else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}
	return next;
}

static Hdl CopyMarkedRootsSegm ( Hdl next, Hdl start, Hdl end, Bool local){
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
				if (local && IsAChoicePointHAddr(a))
					WriteNothing("%lx Ignored H In Choice Point  \n", a);
				else {
					if( IsAlreadyForwarded(XPt(cPt(**a))) ){
					*a = XPt(**a) ; // forward the address on stack
					WriteNothing("%lx Forwarded GlobalRef %lx \n", a, *a);
					}
					else {
						while (temp >= stacksBegin && IsMarked(temp)) {
							temp--;
							WriteNothing("Going down to %lx \n", temp);
						} //walk back towards lower address until an unmarked cell is found
						temp++; //put focus again in the last found marked cell
						while (temp <= H && IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
							WriteNothing("Gone up to %lx \n", temp);
							if (temp==XHdl(*a)){
								*a = next ;
							} // forward the address on stack
							*next++ = cPt(*temp) ; //copy the root to the new location
							*cHdl3(temp) = next-1 ; // Forward the original object address
							temp++;
						}

					}
				}
			}
		}
		elif( IsAtomic(*a) || IsInstruction(*a) ) ;
		elif( IsCompound(*a) ) {
			if( IsList(*a) ) {
				if( IsAlreadyForwarded(XListHead(*a))   ){ //&& IsList(XListHead(*a))
					*a = TagList(XListHead(*a)) ;  //forward the address on stack extracting the tag that putting again the right one
					WriteNothing("%lx Forwarded List %lx \n", a, XPt(*a));
				}
				else {
					while (temp >= stacksBegin && IsMarked(temp)) {temp--; WriteNothing("Going down to %lx \n", temp);} //walk back towards lower address until an unmarked cell is found
						temp++; //put focus again in the last found marked cell
						while (temp <= H && IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
							WriteNothing("Gone up to %lx \n", temp);
							if (temp == XHdl(*a)){
								WriteNothing("temp %lx    *a %lx    taglist %lx \n", temp, XHdl(*a), TagList((next)) );
								*next++ =XListHead(*a) ; //copy the root to the new location
								XListHead(*a) = TagList(cPt(next-1)) ; // Forward the original object address
								*a = TagList(next-1) ; // forward the address on stack

								temp++;
							}
							else{
								*next++ = cPt(*temp) ; //copy the root to the new location
								*cHdl3(temp) = next-1 ; // Forward the original object address
								temp++;
							}
						}
				}
			}
			elif( IsStruct(*a) ) {
				if( IsAlreadyForwarded(*XHdl(*a)) ){
					*a = TagStruct(*XHdl(*a)) ;  //forward the address on stack extracting the tag to follow the correct address
					WriteNothing("%lx Forwarded Struct %lx \n", a, XPt(*a));
				}
				else {
					while (temp >= stacksBegin && IsMarked(temp)) {temp--; WriteNothing("Going down to %lx \n", temp);} //walk back towards lower address until an unmarked cell is found
						temp++; //put focus again in the last found marked cell
						while (temp <= H && IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
							WriteNothing("Gone up to %lx \n", temp);
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

static Hdl CopyRootsNonLocalPrecise(Hdl next, int arity) {
//Dot("CopyRootsNonLocalPrecise ") ;
	Hdl lvrs = cHdl(LvrAccess()) ;
	Size n = LvrGetSize() ;
	while( --n > 0 )
		next = CopyRootsSegm(next, &lvrs[n], &lvrs[n], false) ;  /* @@@ */
	next = CopyRootsSegm(next, &C, &C, false) ;	
	next = CopyRootsSegm(next, &CH, &CH, false) ;
	if( arity != -1 ){
		//WriteNothing("Vou tratar dos X sabendo a aridade %d\n", arity);
		next = CopyRootsSegm(next, X + arity -1, X, false) ;
		//WriteNothing("Fim do tratamento dos X sabendo a aridade \n");
	}
//Mesg("exit") ;
	return next ;
}

static Hdl CopyMarkedRootsNonLocalPrecise(Hdl next, int arity) {
//	Hdl lvrs = cHdl(LvrAccess()) ;
//	Size n = LvrGetSize() ;
//	while( --n > 0 )
//		next = CopyMarkedRootsSegm(next, &lvrs[n], &lvrs[n], false) ;  /* @@@ */
	next = CopyMarkedRootsSegm(next, &C, &C, false) ;
//	next = CopyMarkedRootsSegm(next, &CH, &CH, false) ;
//	if( arity != -1 ){
		next = CopyMarkedRootsSegm(next, X + arity -1, X, false) ;
//	}
	return next ;
}

static Bool GCIsFunctor(Pt t)
{
	return DrfChecked2(t, stacksBegin, H) == nil ;
}

static Bool ValidGlobalTerm(Pt t) {
	if( IsVar(t) )
		return IsGlobalRef(t) && !GCIsFunctor(*cHdl(t)) ;
	elif( IsStruct(t) )
		return IsGlobalRef(XPt(t)) && GCIsFunctor(*XHdl(t)) ;
	elif( IsList(t) )
		return IsGlobalRef(XPt(t))
				&& !GCIsFunctor(XListHead(t)) && !GCIsFunctor(XListTail(t)) ;
	else
		return false ;
}

static Hdl CopyRoots(Hdl next, int arity) {
#if 0
	arity = -1 ;
#endif
	if( ( arity == -1) ) {
/* Imprecise case */
		extern Pt ZT ;
		Write("UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFEUNSAFE UNSAFEUNSAFE UNSAFE UNSAFE UNSAFE\n");

		int i ;
/* Save validity status */
		Bool XValid[maxX] ;
		Bool saveCValid = ValidGlobalTerm(saveC) ;
		Bool SValid = ValidGlobalTerm(S) ;
		Bool ZdotTValid = ValidGlobalTerm(Z.t) ;
		Bool ZTValid = ValidGlobalTerm(ZT) ;
		for( i = 0 ; i < maxX && Xc(i) != nil ; i++ ) {
			XValid[i] = ValidGlobalTerm(Xc(i)) ;
			if( XValid[i] ) WriteNothing("     valid\n") ;
			else WriteNothing("     invalid\n") ;
		}
		
		next = CopyRootsSegm(next, stacksEnd - 1, TopOfLocalStack(), true) ;  //search for roots on local stack has to be first
		WriteNothing("Vou tratar dos X sem saber a aridade \n");
		for( i = 0 ; i < maxX && Xc(i) != nil ; i++ )
			if( XValid[i] )
				next = CopyRootsSegm(next, &Xc(i), &Xc(i), false) ;
		WriteNothing("Fim do tratamento dos X sem saber a aridade \n");

		if( saveCValid )
			next = CopyRootsSegm(next, &saveC, &saveC, false) ;
		if( SValid )
			next = CopyRootsSegm(next, &S, &S, false) ;
		if( ZdotTValid)
			next = CopyRootsSegm(next, &Z.t, &Z.t, false) ;
		if( ZTValid )
			next = CopyRootsSegm(next, &ZT, &ZT, false) ;
		

	}
	else {
	//	Mesg("aaa") ;
		next = CopyRootsSegm(next, stacksEnd - 1, TopOfLocalStack(), true) ;  //search for roots on local stack has to be first
	//	Mesg("bbb") ;
	}
	next = CopyRootsNonLocalPrecise(next, arity) ;

	return next ;
	
}

static Hdl CopyMarkedRoots(Hdl next, int arity) {
/////////////////////////////////////////////////////
	if( ( arity == -1) ) {
/* Imprecise case */
		extern Pt ZT ;
		Write("UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFE UNSAFEUNSAFE UNSAFEUNSAFE UNSAFE UNSAFE UNSAFE\n");
		int i ;
/* Save validity status */
		Bool XValid[maxX] ;
		Bool saveCValid = ValidGlobalTerm(saveC) ;
		Bool SValid = ValidGlobalTerm(S) ;
		Bool ZdotTValid = ValidGlobalTerm(Z.t) ;
		Bool ZTValid = ValidGlobalTerm(ZT) ;
		for( i = 0 ; i < maxX && Xc(i) != nil ; i++ ) {
			XValid[i] = ValidGlobalTerm(Xc(i)) ;
		}
		next = CopyMarkedRootsSegm(next, stacksEnd - 1, TopOfLocalStack(), true) ;  //search for roots on local stack has to be first
		for( i = 0 ; i < maxX && Xc(i) != nil ; i++ )
			if( XValid[i] )
				next = CopyMarkedRootsSegm(next, &Xc(i), &Xc(i), false) ;
		if( saveCValid )
			next = CopyMarkedRootsSegm(next, &saveC, &saveC, false) ;
		if( SValid )
			next = CopyMarkedRootsSegm(next, &S, &S, false) ;
		if( ZdotTValid)
			next = CopyMarkedRootsSegm(next, &Z.t, &Z.t, false) ;
		if( ZTValid )
			next = CopyMarkedRootsSegm(next, &ZT, &ZT, false) ;
	}
	else {
		next = CopyMarkedRootsSegm(next, stacksEnd - 1, TopOfLocalStack(), true) ;  //search for roots on local stack has to be first
	}
	next = CopyMarkedRootsNonLocalPrecise(next, arity) ;

	return next ;

/////////////////////////////////////////////////////////////

	//next = CopyMarkedRootsSegm(next, stacksEnd - 1, TopOfLocalStack(), true) ;
	//return CopyMarkedRootsSegm(next, X + arity -1, X, false) ;

}

static Hdl CopyChildren (Hdl scan, Hdl next){//

	while (scan<next){
	//	if( next >= newGlobalEndZZ )
	//		Mesg("CCCCCCCCCCCC %ld", next - newGlobalEndZZ) ;
		if( IsEmpty(*scan) ) ;
		elif( IsVar(*scan) ) { //do nothing if it is just a self pointing var, just increase scan
			if( IsGlobalRef(*scan) ) { //if it is a global ref there are things to do
				if( IsAlreadyForwarded(cPt(**scan)) ){
					WriteNothing("%lx CC Forwarded Global Ref \n", scan);
					*scan = cPt(**scan) ;} // forward the address on tospace
				elif (IsLink(*scan)) { // is a link to something else other than itself
					if( next + 1 >= newGlobalEnd ) /* Grow if necessary */
						growNewGlobal(&next, &scan, 0, "var") ;
					*next++ = cPt(**scan) ; //copy the object to the new location
					*cHdl3(*scan) = next-1 ; // Forward the original object address
					*scan = cPt(next-1) ; // forward the address on tospace
					WriteNothing("%lx CC Copied Global Ref <link> \n", scan);
				}
				else { //points to itself
					*cHdl3(*scan)= scan;
					*scan=cPt(scan);
					WriteNothing("%lx CC Copied Global Ref <self reference> \n", scan);
				}
			}
			//WriteNothing("%lx CC Ignored Normal Var \n", scan);
			scan++; //go to the next scan object

		}
		elif( IsAtomic(*scan) || IsInstruction(*scan) ) {WriteNothing("%lx CC Atomic/Instruction \n", scan);scan++; } //go to the next scan object;
		elif( IsCompound(*scan) ) {
			if( IsList(*scan) ) {
				if( IsAlreadyForwarded(*XHdl(*scan)) && IsList(XListHead(*scan)) ){
					*scan = TagList(*XHdl(*scan)) ;  //forward the address on tospace extracting the tag to follow the correct address
					WriteNothing("%lx CC Forwarded  List \n", scan);
					scan++; //go to the next scan object

				}
				else {
					if( next + 3 >= newGlobalEnd ) /* Grow if necessary */
						growNewGlobal(&next, &scan, 0, "list") ;
					if (IsUnit(XHdl(*scan)[-1]))
						*next++=XHdl(*scan)[-1];
					*next++ = XListHead(*scan) ;  //copy the head to the new location
					*next++ = XListTail(*scan) ;	//copy the tail to the new location
					XListHead(*scan) = TagList(cPt(next-2)) ;	// forward the original object address
					*scan = TagList(next-2) ; // forward the address on tospace
					WriteNothing("%lx CC Copied List \n", scan);
					scan++; //one at a time - it is the algorith way

				}
			}
			elif( IsStruct(*scan) ) {
				if( IsAlreadyForwarded(*XHdl(*scan)) ){
					*scan = TagStruct(*XHdl(*scan)) ;  //forward the address on stack extracting the tag to follow the correct address
					WriteNothing("%lx CC Forwarded Struct \n", scan);
					scan++; //go to  the next scan object
				}
				else {
					FunctorPt f = XStructFunctor(*scan) ;
					Hdl args ;
					int i, n = FunctorArity(f) ;
					if( next + 1 + n >= newGlobalEnd ) /* Grow if necessary */
						growNewGlobal(&next, &scan, 0, "struct") ;
					args = XStructArgs(*scan) ;
					*next++ = cPt(f) ;	//copy the functor to the new location
					for( i = 0 ; i < n ; i++ )
						*next++ = args[i] ; //copy all the args to the new location
					args[-1] = cPt(next-1-n) ;  // put on the functor place the forward address
					*scan = TagStruct(next-1-n) ; // forward the address on tospace
					//scan = scan + n + 1; //go to the next scan object
					WriteNothing("%lx CC Copied Struct \n", scan);
					scan++; //one at a time - it is the algorith way
				}
			}
			else
				InternalError("CopyChildren") ;
		}
		else {ShowWord(scan,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}
	return next;
}

static Hdl CopyMarkedChildren (Hdl scan , Hdl next){//
	//Cheney copy with two particularities - when a marked cell is visited do
	// 1- scan backward until a unmarked cell is found
	// 2- scan forward and copy marked cells into tospace until a unmarked cell is found
	Hdl temp;
	while (scan<next){
		temp = XHdl(*scan);
		if( IsEmpty(*scan) ) ;
			elif( IsVar(*scan) ) { //do nothing if it is just a self pointing var, just increase scan
				if( IsGlobalRef(*scan) ) { //if it is a global ref there are things to do
					if( IsAlreadyForwarded(XPt(cPt(**scan))) ){
						WriteNothing("%lx CC Forwarded Global Ref \n", scan);
						*scan = cPt(**scan) ;} // forward the address on tospace
					else {
						if (IsLink(*scan)) { // is a link to something else other than itself
							while (temp >= stacksBegin && IsMarked(temp)) {temp--; WriteNothing(" CC Going down to %lx \n", temp);} //walk back towards lower address until an unmarked cell is found
								temp++; //put focus again in the last found marked cell
							while (temp <= H && IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
								WriteNothing(" CC Gone up to %lx \n", temp);
								if (temp==*scan)
									*scan = next ; // forward the address on stack
								*next++ = cPt(*temp) ; //copy the object to the new location
								*cHdl3(temp) = next-1 ; // Forward the original object address
								//temp = next-1 ; // forward the address on tospace
								temp++;
							}
						}
						else{
							*cHdl3(*scan)= scan;
							*scan=cPt(scan);
						}
					}
				}
				scan++; //go to the next scan object
			}
			elif( IsAtomic(*scan) || IsInstruction(*scan) ) scan++; //go to the next scan object;
			elif( IsCompound(*scan) ) {
				if( IsList(*scan) ) {

					if( IsAlreadyForwarded(*XHdl(*scan)) ) {//&& IsList(XListHead(*scan)
						WriteNothing("%lx CC Forwarded List \n", scan);
						*scan = TagList(*XHdl(*scan)) ;  //forward the address on tospace extracting the tag to follow the correct address
						scan++; //go to the next scan object
					}
					else {
						while (temp >= stacksBegin && IsMarked(temp)) {temp--; WriteNothing(" CC Going down to %lx \n", temp);} //walk back towards lower address until an unmarked cell is found
						temp++; //put focus again in the last found marked cell
						while (temp <= H && IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
							WriteNothing(" CC Gone up to %lx \n", temp);
							if (temp == XHdl(*scan)){

								*next++ =XListHead(*scan) ; //copy the root to the new location
								XListHead(*scan) = TagList(cPt(next-1)) ; // Forward the original object address
								*scan = TagList(next-1) ; // forward the address on stack

								temp++;
							}
							else{
								*next++ = cPt(*temp) ; //copy the root to the new location
								*cHdl3(temp) = next-1 ; // Forward the original object address
								temp++;
							}

						}
						scan++;
					}
				}
				elif( IsStruct(*scan) ) { ///////////////////////////////////////////////
					if( IsAlreadyForwarded(*XHdl(*scan)) ){
						WriteNothing("%lx CC Forwarded Struct \n", scan);
						*scan = TagStruct(*XHdl(*scan)) ;  //forward the address on stack extracting the tag to follow the correct address
						scan++; //go to the next scan object
					}
					else {
						while (temp >= stacksBegin && IsMarked(temp)) {temp--; WriteNothing(" CC Going down to %lx \n", temp);} //walk back towards lower address until an unmarked cell is found
							temp++; //put focus again in the last found marked cell
							while (temp <= H && IsMarked(temp)){ //walk forward towards higher address copying all cells until an unmarked cell is found
								WriteNothing(" CC Gone up to %lx \n", temp);
								if (temp==XHdl(*scan))
									*scan = TagStruct(next) ; // forward the address on stack
								*next++ = cPt(*temp) ; //copy the root to the new location
								*cHdl3(temp) = next-1 ; // Forward the original object address
								temp++;
							}
							scan++;
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


	//if (GCIsFunctor(a)) return;
tailRecursiveCall:

	if( IsVar(a) ) {
		if( IsMarked(XHdl(a)) || IsAChoicePointHAddr(a)) ;
			// do nothing, it is already markedCopyRootsSegm
		else {
			MarkPt(XHdl(a));
			WriteNothing("%lx MR Var \n", a);
			WriteNothing ("IsLINK $$$$$$$$$$$$$ %lx ----- %lx \n", *a , a);
			if (IsLink(a)){
				MarkRecursive(*XHdl(a)); //call mark recursive with the argument of the cell pointed by the Var
			}
		}
	}
	elif( IsAtomic(a) || IsInstruction(a) );
	elif( IsCompound(a) ) {
		if( IsList(a) ) {
			if( IsMarked(XHdl(a)) && IsMarked(XHdl(a)+1)); // we have to see if the tail is also marked because the head could be marked by a Var
				// do nothing, it is already marked
			else {
				if (IsUnit(XHdl(a)[-1]) ){
					MarkPt((XHdl(a)-1));
					//Write("%lx Marked unit\n", (XHdl(a)-1));
				}
				//Write("%lx MR List Head\n", XHdl(a));
				MarkPt(XHdl(a));
				MarkRecursive(XListHead(a));
				MarkPt((XHdl(a)+1));
			//	Write("%lx MR List Tail  %lx %lx\n", &(XListTail(a)), stacksBegin, H);
				a = XListTail(a) ;
				goto tailRecursiveCall ;
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
				WriteNothing("%lx MR Struct Functor \n", (args-1));
				for( i = 0 ; i < n - 1 ; i++ ){
					MarkPt(args+i); 
					WriteNothing("%lx MR Struct Args \n", (args+i));
					MarkRecursive(args[i]);
				}
				MarkPt(args+n-1); 
				a = args[n-1] ;
				goto tailRecursiveCall ;
			}
		}
		else
			InternalError("MarkPhase") ;
	}
	else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}

}


static void MarkPhase(Hdl start, Hdl end){

	register Hdl a ;
	for( a = start ; a >= end ; a-- ) // going through the stack because it's the source for roots of the "trees"
	{
		if( IsEmpty(*a) ) ;
		elif( IsVar(*a) ) {
			if( IsGlobalRef(*a) ) {
				if (IsAChoicePointHAddr(a)){WriteNothing("%lx Ignored H in Choice Point \n", a);}
				else{
				//Write("%lx Found a Global Ref \n", a);
				MarkRecursive(cHdl(*a));
				}

			}
		}
		elif( IsAtomic(*a) || IsInstruction(*a) ) ;
		elif( IsCompound(*a) ) {
			if( IsList(*a) ) {
				//Write("%lx Found a List  \n", a);
				MarkRecursive(*a);

			}
			elif( IsStruct(*a) ) {
				//Write("%lx Found a Struct  \n", a);
				MarkRecursive(*a);

			}
			else
				InternalError("MarkPhase") ;
		}
		else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}

}

/*
static void MarkPhase(Hdl start, Hdl end){

	register Hdl a, lTop = TopOfLocalStack() ;
	for( a = stacksEnd - 1; a >= lTop ; a-- ) // going through the stack because it's the source for roots of the "trees"
	{
		if( IsEmpty(*a) ) ;
		elif( IsVar(*a) ) {
			if( IsGlobalRef(*a) ) {
				if (IsAChoicePointHAddr(a)){WriteNothing("%lx Ignored H in Choice Point \n", a);}
				else{
				WriteNothing("%lx Found a Global Ref \n", a);
				MarkRecursive(cHdl(*a));
				}

			}
		}
		elif( IsAtomic(*a) || IsInstruction(*a) ) ;
		elif( IsCompound(*a) ) {
			if( IsList(*a) ) {
				WriteNothing("%lx Found a List  \n", a);
				MarkRecursive(*a);

			}
			elif( IsStruct(*a) ) {
				WriteNothing("%lx Found a Struct  \n", a);
				MarkRecursive(*a);

			}
			else
				InternalError("MarkPhase") ;
		}
		else {ShowWord(a,false) ; InternalError("RelocateLocalStack (>=0)") ;}
	}

}
*/


static Hdl CopyPhase(int arity){

	register Hdl scan=newGlobalBegin;
	register Hdl next=newGlobalBegin;

	//check for roots
	next = CopyMarkedRoots(next, arity);
	//follow children
	next = CopyMarkedChildren(scan, next);


	return next;

}

static void UpdateTrailRefs() {
	register Hdl h ;	
	for( h = trailBegin ; h < TR ; h++ )
		if( IsGlobalRef(*h) )
			*h = IsAlreadyForwarded(**h) ? XPt(**h) : trailFreeVar ;		
	/* Nothing to do with the finalizers (proc, callable, cp) */ 	
	/* The choicepoints do not need readjusting of the TR field */
}

static void ResetHsInChoicePoints(Hdl newH){
	register ChoicePointPt b ;
	for( b = B ; !IsEndOfChain(b) ; b = ChoicePointField(b, B) )
		ChoicePointField(b, H) = newH;
}

static void RunMarkAndCopy(int arity){

	Hdl next ;

	MakeExtraBits(newGlobalEnd - newGlobalBegin) ;

	Hdl lvrs = cHdl(LvrAccess()) ;
	Size n = LvrGetSize() ;
	//MarkPhase(X + arity -1, X);

	//MarkPhase(stacksEnd - 1, TopOfLocalStack());
//////////////////////////////////////////////////////////////

	if( ( arity == -1) ) {
/* Imprecise case */
		extern Pt ZT ;
		Write("Imprecise Imprecise Imprecise Imprecise Imprecise Imprecise Imprecise Imprecise \n");
		int i ;
/* Save validity status */
		Bool XValid[maxX] ;
		Bool saveCValid = ValidGlobalTerm(saveC) ;
		Bool SValid = ValidGlobalTerm(S) ;
		Bool ZdotTValid = ValidGlobalTerm(Z.t) ;
		Bool ZTValid = ValidGlobalTerm(ZT) ;
		for( i = 0 ; i < maxX && Xc(i) != nil ; i++ ) {
			XValid[i] = ValidGlobalTerm(Xc(i)) ;
		}


		MarkPhase(stacksEnd - 1, TopOfLocalStack());

		for( i = 0 ; i < maxX && Xc(i) != nil ; i++ )
			if( XValid[i] )
				MarkPhase(&Xc(i), &Xc(i));

		if( saveCValid )
			MarkPhase(&saveC, &saveC);
		if( SValid )
			MarkPhase(&S, &S);
		if( ZdotTValid)
			MarkPhase( &Z.t, &Z.t);
		if( ZTValid )
			MarkPhase(&ZT, &ZT);
	}
	else {
		MarkPhase(stacksEnd - 1, TopOfLocalStack());
		MarkPhase(  X + arity -1, X);
	}


//	while( --n > 0 )
//		MarkPhase( &lvrs[n], &lvrs[n]);
	MarkPhase( &C, &C);
	MarkPhase( &CH, &CH);


///////////////////////////////////////////////////////////////////

	next = CopyPhase(arity);
	UpdateTrailRefs();
	H = next ;
	ResetHsInChoicePoints(H);
	DeleteExtraBits(newGlobalEnd - newGlobalBegin) ;
}

static void RunChenneyCopy(int arity){
	Hdl next = CopyRoots(newGlobalBegin, arity);  // check for roots
	next = CopyChildren(newGlobalBegin, next);    // follow children  
	UpdateTrailRefs();
	H = next ;
	ResetHsInChoicePoints(H);
}




static Hdl RunCollector(int arity) {
#if 0
	RunChenneyCopy(arity);
#elif 1
	RunMarkAndCopy(arity);
#else
	RelocateStacks(newGlobalBegin - stacksBegin, 0) ;
#endif
	return H ;
}

static void DoGCollection(int arity)
{
	Hdl saveStacksBegin, saveStacksEnd ;
	Hdl saveH = H, saveNewH ;
	Size originalGlobalSize = H - stacksBegin ;
	Size originalGenLocalSize = stacksEnd - H ;
	Size newGlobalSize ;
	PFloat threshold = 0.8 ;
	Size beforeGC, afterGC, newStacksSize;
	
	Hdl newStacksBegin, newStacksEnd, stacksBegin0;


	beforeGC=H-stacksBegin;
//	Mesg("Antes da GC %ld ", beforeGC );

	newGlobalBegin = AllocateSegment(originalGlobalSize, &newGlobalEnd) ;
//	Mesg("RUNNING COLLECTOR");

	H = RunCollector(arity) ;
	newGlobalSize = H - newGlobalBegin ;
	if( newGlobalSize <= originalGlobalSize ) {
		saveStacksBegin = stacksBegin ;
		stacksBegin = newGlobalBegin ;
		RelocateStacks(saveStacksBegin - newGlobalBegin, 0) ;
		stacksBegin = saveStacksBegin ;	
		ReleaseSegment(newGlobalBegin, newGlobalEnd - newGlobalBegin) ;

		afterGC=H-stacksBegin;
//		Mesg("Depois da GC %ld ", afterGC );
		PFloat racio = afterGC/(PFloat)beforeGC;
//		 Mesg("racio que recuperei foi  de  %lf percent ", 100*(1- afterGC/(double)beforeGC) );

		if (gcTestRate == 0 && racio > threshold){
		//	Mesg("Not enought space recovered. Going to Grow");
		//	Mesg("Stack size is %ld and the Heap size is %ld",stacksEnd-TopOfLocalStack(), H-stacksBegin );
			newStacksSize=stacksEnd-stacksBegin;
		//	Mesg("I was using %ld words and Im groing to double to %ld", newStacksSize, newStacksSize*2);
			stacksBegin0=stacksBegin;
			newStacksBegin = AllocateSegment(newStacksSize*2, &newStacksEnd) ;
			RelocateStacks(newStacksBegin - stacksBegin, newStacksEnd - stacksEnd) ;
			MemoryGrowInfo("stacks", newStacksSize, newStacksSize*2) ;
			ReleaseSegment(stacksBegin0, newStacksSize) ;
			stacksBegin = stacksBegin0 = newStacksBegin ;
			stacksEnd = stacksEnd0 = newStacksEnd ;
		}

	}
	else {
		if( originalGenLocalSize > newGlobalEnd - H ) {
		// Mesg("#@!#@!#@!#@!#@!#@!#@!# %ld %ld", originalGenLocalSize, newGlobalEnd - H) ;
			growNewGlobal(&H, nil, originalGenLocalSize + (H - newGlobalBegin), "QQQQ") ;
		}
		Mesg("LLLLLLLLLLLLLLLLLLLLLLLL") ;
		saveStacksBegin = stacksBegin ;
		saveStacksEnd = stacksEnd ;
		saveNewH = H ;
		H = saveH ;
		RelocateStacks(0, newGlobalEnd - stacksEnd) ;		
		stacksBegin = newGlobalBegin ;
		stacksEnd = newGlobalEnd ;
		H = saveNewH ;
		ReleaseSegment(saveStacksBegin, saveStacksEnd - saveStacksBegin) ;
		//Mesg("L DDDDDDDDDONE") ;
	}
}

#endif

Bool ZGCollection(Size ensureThisExtraSpace, int arity, Bool precision)
{
#if GC
	Unused(ensureThisExtraSpace) ;
	Unused(precision) ;
	if( Booting() )
		return false ;
/*	if( arity == -1 ) {
		Error("Global stack overflow (GCollection)") ;
		return false ;
	}*/
	
//	Mesg("ZGCollection enter") ;
//	MesgW("Antes") ;
//	StatisticsShow() ;
 //	Mesg("********************  antes %ld", H-stacksBegin);
//	MachineShow("x");
//	MachineShow("local");
//	MachineShow("global");
	DoGCollection(arity) ;
//	Mesg("******************** depois %ld", H-stacksBegin);

	//MesgW("************************** GC recovered  %lf  ", 100*(1-(afterGC/(double)beforeGC)));
//	MachineShow("global");
//	MachineShow("local");
//	Mesg("Depois") ;
//	StatisticsShow() ;
//	Mesg("ZGCollection exit") ;
	return true ;
#else
	Unused(arity) ;
	Unused(precision) ;
	return ZStacksExpansion(ensureThisExtraSpace) ;	
#endif



}

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
	ZGCollection(0, 0, true) ;
	JumpNext() ;
}

static void PGCollectionRate()
{
	gcTestRate = XTestNat(X0) ;
	Mesg("gc_rate %ld", gcTestRate) ;
	TestRelocationUpdateFlag(gcTestRate > 0) ;
	JumpNext() ;
}

void GCollectionInit()
{
	trailFreeVar = GetCodeFreeVar() ;
	InstallCBuiltinPred("gc", 0, PGCollection) ;
	InstallCBuiltinPred("gcr", 1, PGCollectionRate) ;
}
