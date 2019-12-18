/*
 *   This file is part of the CxProlog system

 *   Flags.h
 *   by A.Miguel Dias - 2000/08/22
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Flags_
#define _Flags_


extern PInt
	debug_flag,
	onInterrupt_flag,
	occursCheck_flag,
	keepSource_flag,
	floatDisplayPrec_flag,
	integerRoundingFunction_flag,
	eofCode_flag,
	gCollection_flag,
	nIndexable_flag,

	compatibleIfThen_flag,
#if COMPASS
	compatibleStrings_flag,
#endif
	nilIsSpecial_flag,
	barIsSemicolon_flag,
	cutIsSolo_flag,
	extraSpacesInTerms_flag,
	forceQuoted_flag,
	characterEscapes_flag,
	doubleQuotes_flag,
	charConversion_flag,
	fileNameVariables_flag,

	onError_flag,
	unknown_flag,
	infoMessages_flag,

	javaAvailable_flag,
	wxWidgetsAvailable_flag,

	sysTrace_flag,
	testRelocation_flag,
	testGCollection_flag,
	testInvariants_flag,

	allDynamic_flag,
	oldUpdate_flag,
	isoErrors_flag,

	memoryWarnings_flag,
	oldFashionedSeeing_flag,
	forceVisibility_flag,
	semanticVariant_flag,
	localOperators_flag,
	undefWarnings_flag ;

extern PInt
	stacksDebugging_flag,
	trailDebugging_flag,
	scratchDebugging_flag,
	indexDebugging_flag,
	gcDebugging_flag,
	sysTraceDebugging_flag,
	testRelocationDebugging_flag,
	testInvariantsDebugging_flag,
	infoMessagesDebugging_flag ;

void FlagsShow(void) ;
void FlagsRestart(void) ;
void FlagsInit(void) ;
void FlagsInit2(void) ;
void FlagsInit3(void) ;

#endif
