/*
 *   This file is part of the CxProlog system

 *   Disassemble.h
 *   by A.Miguel Dias - 1990/01/20
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Disassemble_
#define _Disassemble_

Hdl DisassembleOneInst(StreamPt srm, Hdl code) ;
void Disassemble(StreamPt srm, Hdl code) ;
void DisassemblePredicate(StreamPt srm, PredicatePt pr) ;
void DisassemblerInit(void) ;

#endif
