/*
 *   This file is part of the CxProlog system

 *   Python.c
 *   by Vitor Mexia, Artur Miguel Dias - 2014/02/04
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL

 *   it under the terms of the GNU General Public License as published by
 *   CxProlog is free software; you can redistribute it and/or modify
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

#include "Python.h"
#include "CxProlog.h"
#include <stdbool.h>

static ExtraTypePt pyobjType;

typedef struct PyObj{
	ExtraDef(PyObj);
	PyObject *value;
} PyObj, *PyObjPt;

#define cPyObjPt(x)			((PyObjPt)(x))
#define PyObjValue(py)		( cPyObjPt(py)->value )

static Bool PyObjBasicGCDelete(VoidPt x){
	//Unused(x),
	//return false;
	Py_DECREF(PyObjValue(x));
	return true;
}

static Size PyObjSizeFun(CVoidPt x){
	Unused(x);
	return WordsOf(PyObj);
}

static Pt MakePyObj(PyObject *obj){
	if(obj == nil)
		return tNullAtom;
	else{
		PyObjPt py = ExtraNew(pyobjType, 0);
		PyObjValue(py) = obj;
		ExtraPermanent(py);
		return TagExtra(pyobjType, py);
	}
}

static PyObject *XTestPyObj(register Pt t){
	VarValue(t) ;
	if(t == tNullAtom)
		return nil ;
	else{
		PyObjPt j = cPyObjPt(XTestExtraNoAlias(pyobjType, t));
		return PyObjValue(j);
	}
}

/* Prototypes */
PyObject* CxListToPyTuple(Pt list);
Pt PyListToCxList(PyObject *args);
Pt PyTupleToCxList(PyObject *args);

/* Converts CxProlog to Python */
PyObject* PrologToPython(Pt t){
	t = Drf(t);
	if(t == nil){
		return TypeError("Python compatible value", t);
	}else if(IsList(t)){
		return CxListToPyTuple(t);
	}else if(t==tNilAtom){
		return CxListToPyTuple(t);
	}else if(IsAtom(t)){
		return PyString_FromString(XAtomName(t));
	}else if(IsInt(t)){
		return PyInt_FromLong(XInt(t));
	}else if(IsFloat(t)){
		return PyFloat_FromDouble(XFloat(t));
	}else if(IsThisExtra(pyobjType, t)){
		return XTestPyObj(t);
	}else{ 
		return TypeError("Python compatible value", t);
	}
}

/* Converts Python to CxProlog */
Pt PythonToProlog(PyObject *obj){
	if(PyBool_Check(obj)){
		return MakeBool(PyObject_IsTrue(obj)) ;
	}else if(PyList_Check(obj)){
		return PyListToCxList(obj);
	}else if(PyTuple_Check(obj)){
		return PyTupleToCxList(obj);
	}else if(PyInt_Check(obj)){
		return MakeInt(PyInt_AsLong(obj));
	}else if(PyString_Check(obj)){
		return MakeAtom(PyString_AsString(obj));
	}else if(PyFloat_Check(obj)){
		return MakeFloat(PyFloat_AsDouble(obj));
	}else{
		if(obj == Py_None)
			return tVoidAtom;
		else
			return MakePyObj(obj);
	}
}

/* Converts CxProlog list to Python tuple */
PyObject* CxListToPyTuple(Pt list){
	PyObject *pArgs = PyTuple_New(ListLength(list));
	int n = 0;
	for(list = Drf(list); IsList(list); list = Drf(XListTail(list))){
		PyObject *val = PrologToPython(Drf(XListHead(list)));
		PyTuple_SetItem(pArgs, n++, val);
	}
	if(list != tNilAtom)
		TypeError("PROPERLY-TERMINATED-LIST", nil) ;
	return pArgs ;
}

/* Converts Python list to CxProlog list */
Pt PyListToCxList(PyObject *args){
	int list_size = (int)PyList_Size(args);
	Pt array[list_size];
	
	int i = 0;
	for(i = 0; i < list_size; i++){
		PyObject *tmp = (PyObject*)PyList_GetItem(args, (Py_ssize_t)i);
		array[i] = PythonToProlog(tmp);
		//Py_XDECREF(tmp);
	}

	return ArrayToList(array, list_size);
}

/* Converts Python tuple to CxProlog list */
Pt PyTupleToCxList(PyObject *args){
	int tuple_size = (int)PyTuple_Size(args);
	Pt array[tuple_size];

	int i = 0;
	for( i=0; i < tuple_size; i++) {
		PyObject *tmp = (PyObject*)PyTuple_GetItem(args, (Py_ssize_t)i);
		array[i] = PythonToProlog(tmp);
		//Py_XDECREF(tmp);
	}

	return ArrayToList(array, tuple_size);
}

static bool Check_Python(){
    PyObject *ptype, *pvalue, *ptraceback;
    // fetch python exception information
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    PyErr_Clear();

    if (ptype != NULL) {
        Throw(MakeAtom(PyString_AsString(PyObject_Str(pvalue))));

        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);

        return true;
    }

    Py_XDECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptraceback);

    return false;
}

static void PPythonCall(){
	
	PyObject *pModule, *pFunc, *pValue;
	Pt t0 = Drf(X0);

	//Checks if is receiving a PyObject or Atom
	if(IsAtom(t0)){
		PyObject *pName = PyString_FromString(XAtomName(t0));
		pModule = PyImport_Import(pName);
		Py_DECREF(pName);
	}else if(IsThisExtra(pyobjType, t0)){
		pModule = XTestPyObj(t0);
	}else{
		pModule = NULL;
		TypeError("Module or Object", nil);
	}
	
	Str func = XTestAtomName(X1);
	Pt args = TestList(X2);
	
	if(pModule != NULL){
		pFunc = PyObject_GetAttrString(pModule, func);
		/* pFunc is a new reference */

		if (pFunc && PyCallable_Check(pFunc)){

			pValue = PyObject_CallObject(pFunc, PrologToPython(args));

			Check_Python();

			if(pValue != NULL){
				Pt res = PythonToProlog(pValue);
				Py_DECREF(pValue);
				MustBe(Unify(X3, res)); 
				printf("\n");
			}else{
				Py_XDECREF(pValue);
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				Throw(MakeAtom("Call failed"));
			}
		}else{
			if (PyErr_Occurred())
				Throw(MakeAtom("Cannot find function"));
			}
			Py_XDECREF(pFunc);
			Py_DECREF(pModule);
	}else{
		Py_XDECREF(pModule);
		Throw(MakeAtom("Failed to load"));
	}
}

static void PPythonVar(){
	
	PyObject *pModule, *pValue;

	Pt t0 = Drf(X0);

	//Checks if is receiving a PyObject or Atom
	if(IsAtom(t0)){
		PyObject *pName = PyString_FromString(XAtomName(t0));
		pModule = PyImport_Import(pName);
		Py_DECREF(pName);
	}else if(IsThisExtra(pyobjType, t0)){
		pModule = XTestPyObj(t0);
	}else{
		pModule = NULL;
		TypeError("Module or Object", nil);
	}

	Str var = XTestAtomName(X1);
	PInt wrtSt = XTestInt(X2);
	Pt val = X3;
	
	if(pModule != NULL){
		if(wrtSt == 0){
			pValue = PyObject_GetAttrString(pModule, var);
		}else if(wrtSt == 1){
			PyObject *pVal = PrologToPython(val);
			if(PyObject_SetAttrString(pModule, var, pVal)==-1)
				Throw(MakeAtom("Failed to change variable"));
			Py_DECREF(pVal);
			pValue = PyObject_GetAttrString(pModule, var);
		}else{
			pValue = NULL;
			Throw(MakeAtom("Not a writting status"));
		}

		if(pValue != NULL){
			Pt res = PythonToProlog(pValue);
			Py_DECREF(pValue);
			Py_DECREF(pModule);
			MustBe(Unify(X4, res)); 
			printf("\n");
		}else{
			Py_XDECREF(pValue);
			Py_DECREF(pModule);
			Throw(MakeAtom("Call failed"));
		}
	}else{
		Py_XDECREF(pModule);
		Throw(MakeAtom("Failed to load"));
	}
}

void PythonInit()
{
	pyobjType = ExtraTypeNew("PYOBJ", PyObjSizeFun, nil, PyObjBasicGCDelete, 256) ;
	ExtraTypeDoesNotSupportAliasing(pyobjType) ;
	Py_Initialize() ;
	PySys_SetPath("");
	InstallCBuiltinPred("python_call",4,PPythonCall);
	InstallCBuiltinPred("python_var",5,PPythonVar);
	InstallCBuiltinPred("python_new",4,PPythonCall);
}
