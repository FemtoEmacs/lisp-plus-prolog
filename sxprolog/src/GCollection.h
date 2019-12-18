/*
 *   This file is part of the CxProlog system

 *   GCollection.h
 *   by Andre Castro, A.Miguel Dias - 2010/03/12
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _GCollection_
#define _GCollection_

Bool ZGCollection(Size ensureThisExtraSpace, int arity, Bool precision) ;
Bool ZTestGCollection(Size ensureThisExtraSpace, int arity, Bool precision) ;
void GCollectionInit(void) ;

#endif
