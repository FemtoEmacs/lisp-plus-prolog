/*
 *   This file is part of the CxProlog system

 *   InstructionsInfo.h
 *   by A.Miguel Dias - 2005/08/15
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

/* Instruction info database.
   The preprocessor allows this information to be used in different
   ways depending on the definition of InstInfo that is in force at
   each place of inclusion.

   Technique borrowed from the inclusion of "bits/mathcalls.h"
   in "/usr/include/math.h" on the gcc include files.
*/

/* PROCEDURAL & CONTROL INSTRUCTIONS */
	InstInfo(	Nop,					""		),
	InstInfo(	FAllocate,				""		),
	InstInfo(	Proceed,				"@"		),
	InstInfo(	DeallocProceed,			"@"		),
	InstInfo(	LocalJump,				"l@"	),
	InstInfo(	EnsureFreeSpace,		"ne"	),
	InstInfo(	Call,					"pe"	),
	InstInfo(	Execute,				"p@"	),
	InstInfo(	DeallocExecute,			"p@"	),
	InstInfo(	CallVar,				"e"		),
	InstInfo(	ExecuteVar,				"@"		),
	InstInfo(	DeallocExecuteVar,		"@"		),
	InstInfo(	CallCleanupContinuation,""		),
	InstInfo(	CallCleanupFail,		""		),
	InstInfo(	WxWidgetsCall,			"lll"	),
	InstInfo(	WxWidgetsCall2,			"lll"	),

	InstInfo(	Cut,					""		),
/*	InstInfo(	MetaCut,				""		), */
/*	InstInfo(	DynamicCut,				""		), */
	InstInfo(	PutCutLevel,			"x"		),

	InstInfo(	EmptyPred,				"f@"	),
	InstInfo(	Fail,					""		),
	InstInfo(	FailMesg,				""		),

/* GET INSTRUCTIONS */
	InstInfo(	GetYInit,				"y"		),
	InstInfo(	GetYVariable,			"yx"	),
	InstInfo(	GetXValue,				"xx"	),
	InstInfo(	GetYValue,				"yx"	),
	InstInfo(	GetZValue,				"zx"	),
	InstInfo(	GetAtomic,				"tx"	),
	InstInfo(	GetNil,					"x"		),
	InstInfo(	GetStructure,			"fx"	),
	InstInfo(	GetList,				"x"		),

/* PUT INSTRUCTIONS */
	InstInfo(	PutXVariable,			"xx"	),
	InstInfo(	PutXVariableOne,		"x"		),
	InstInfo(	PutYVariable,			"yx"	),
	InstInfo(	PutXValue,				"xx"	),
	InstInfo(	PutYValue,				"yx"	),
	InstInfo(	PutZValue,				"zx"	),
	InstInfo(	PutUnsafeValue,			"yx"	),
	InstInfo(	PutAtomic,				"tx"	),
	InstInfo(	PutNil,					"x"		),
	InstInfo(	PutStructure,			"fx"	),
	InstInfo(	PutList,				"x"		),

/* UNIFY INSTRUCTIONS */
	InstInfo(	UnifyVoid,				"n"		),
	InstInfo(	UnifyVoidOne,			""		),
	InstInfo(	UnifyXVariable,			"x"		),
	InstInfo(	UnifyYVariable,			"y"		),
	InstInfo(	UnifyXLocalValue,		"x"		),
	InstInfo(	UnifyYLocalValue,		"y"		),
	InstInfo(	UnifyXValue,			"x"		),
	InstInfo(	UnifyYValue,			"y"		),
	InstInfo(	UnifyZValue,			"z"		),
	InstInfo(	UnifyAtomic,			"t"		),
	InstInfo(	UnifyNil,				""		),

/* BUILD INSTRUCTIONS */
	InstInfo(	BuildVoid,				"n"		),
	InstInfo(	BuildVoidOne,			""		),
	InstInfo(	BuildXVariable,			"x"		),
	InstInfo(	BuildYVariable,			"y"		),
	InstInfo(	BuildXValue,			"x"		),
	InstInfo(	BuildYValue,			"y"		),
	InstInfo(	BuildZValue,			"z"		),
	InstInfo(	BuildXLocalValue,		"x"		),
	InstInfo(	BuildYLocalValue,		"y"		),
	InstInfo(	BuildAtomic,			"t"		),
	InstInfo(	BuildNil,				""		),

/* CONTEXT INSTRUCTIONS */
	InstInfo(	UndefPred,				"f@"	),
	InstInfo(	UndefPredEnd,			""		),
	InstInfo(	Import,					"t@"	),
	InstInfo(	ImportEnd,				""		),
	InstInfo(	CtxSwitch,				""		),
	InstInfo(	CtxExtension,			""		),
	InstInfo(	CtxExtensionEnd,		""		),
	InstInfo(	CtxEmpty,				""		),
	InstInfo(	CtxEmptyEnd,			""		),
	InstInfo(	CtxDown,				""		),
	InstInfo(	CtxDownEnd,				""		),
	InstInfo(	HCtxPush,				""		),
	InstInfo(	HCtxPushEnd,			""		),
	InstInfo(	HCtxEnter,				""		),
	InstInfo(	HCtxEnterEnd,			""		),
	InstInfo(	ContextualEmptyPred,	"f@"	),
	InstInfo(	ContextualDynamicEnter,	"cn@"	),
	InstInfo(	ContextualDynamicElse,	"cn-------NN....."),

/* INDEXING INSTRUCTIONS */
	InstInfo(	MakeIndex,				"p@"	),
	InstInfo(	TryMeElse,				"cn"	),
	InstInfo(	RetryMeElse,			"cn"	),
	InstInfo(	TrustMe,				".n"	),
	InstInfo(	Try,					"l"		),
	InstInfo(	Retry,					"l"		),
	InstInfo(	Trust,					"l"		),
	InstInfo(	DynamicEnter,			"cn@"	),
	InstInfo(	DynamicElse,			"cn-------NN....."),
	InstInfo(	DynamicEnterIndexed,	"cccc@"	),
	InstInfo(	DynamicElseIndexed,		"cn-------NN....."),
	InstInfo(	DynamicIUEnter,			"cn@"	),
	InstInfo(	DynamicIUElse,			"cn"	),
	InstInfo(	DynamicIUEnterIndexed,	"cccc@"	),
	InstInfo(	DynamicIUElseIndexed,	"cn"	),
	InstInfo(	SwitchOnTerm0,			"llll@"	),
	InstInfo(	SwitchOnTerm1,			"llll"	),
	InstInfo(	SwitchOnTerm2,			"llll"	),
	InstInfo(	SwitchOnAtomic,			"H"		),
	InstInfo(	SwitchOnStructure,		"H"		),
	InstInfo(	DiscardAndFail,			""		),

/* DEBUGGING INSTRUCTIONS */
	InstInfo(	DebugExit,				""		),
	InstInfo(	DebugRedo,				""		),
	InstInfo(	DebugRetry,				""		)
