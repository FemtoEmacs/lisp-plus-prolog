/*
 *   This file is part of the CxProlog system

 *   Operator.h
 *   by A.Miguel Dias - 1992/02/23
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Operator_
#define _Operator_

#define maxPrec		1200	/* Max. operator precedence.*/
#define subPrec		 999	/* Max. prec. for subterms. */
#define commaPrec	1000
#define barPrec		1100

typedef struct Operator *OperatorPt ;

void MakePrefixOperator(Str n, int p, Bool a) ;
void MakeInfixOperator(Str n, int p, Bool la, Bool ra) ;
void MakePosfixOperator(Str n, int p, Bool a) ;
int Prefix(AtomPt atom, int *rp) ;
int Infix(AtomPt atom, int *lp, int *rp) ;
int Postfix(AtomPt atom, int *lp) ;
Bool OperatorParenthesised(OperatorPt op) ;
void DefineOperator(int p, Str type, Pt ops) ;
void OperatorsInit(void) ;

#endif
