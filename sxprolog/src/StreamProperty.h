/*
 *   This file is part of the CxProlog system

 *   StreamProperty.h
 *   by A.Miguel Dias - 2006/05/31
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _StreamProperty_
#define _StreamProperty_

typedef struct OpenStreamOptions {
	Pt type ;
	Pt reposition ;
	Pt alias ;
	Pt eofAction ;
	Pt encoding ;
	Pt bom ;
} OpenStreamOptions, *OpenStreamOptionsPt ;

typedef struct CloseStreamOptions {
	Pt force ;
} CloseStreamOptions, *CloseStreamOptionsPt ;


OpenStreamOptionsPt StreamPropertyGetDefaultOpenOptions(OpenStreamOptionsPt so) ;
OpenStreamOptionsPt StreamPropertyGetOpenOptions(Pt t, OpenStreamOptionsPt so) ;
CloseStreamOptionsPt StreamPropertyGetDefaultCloseOptions(CloseStreamOptionsPt co) ;
CloseStreamOptionsPt StreamPropertyGetCloseOptions(Pt t, CloseStreamOptionsPt co) ;
void StreamPropertyInit(void) ;

#endif
