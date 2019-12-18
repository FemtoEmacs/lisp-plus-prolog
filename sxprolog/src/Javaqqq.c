/*
 *   This file is part of the CxProlog system

 *   Java.c
 *   by A.Miguel Dias - 2004/07/25
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

#include "CxProlog.h"

#if USE_JAVA
#include "jni.h"

#if 0
dynamic link to jvm.dll at run time, for example by using LoadLibrary:
HINSTANCE hinstLib = LoadLibrary(TEXT("D:\\desired_jvm\\jre6\\bin\\client\\jvm.dll"));
typedef jint (JNICALL *PtrCreateJavaVM)(JavaVM **, void **, void *);
PtrCreateJavaVM ptrCreateJavaVM = (PtrCreateJavaVM)GetProcAddress(hinstLib,"JNI_CreateJavaVM");
jint res = ptrCreateJavaVM(&jvm, (void**)&env, &vm_args);


http://www.codeproject.com/Articles/17352/JVM-Launcher
https://forums.oracle.com/forums/thread.jspa?threadID=1546479
http://www.velocityreviews.com/forums/t692288-correct-way-to-load-jvm-dll-in-a-jni-application.html
#endif


/* Global JNI support variables */

static JNIEnv *env ;
static JavaVM *jvm ;
static Fun eventNotifier = nil ;
static jclass systemClass, classClass, objectClass ;
static jclass stringClass, swingUtilitiesClass, noClassDefFoundClass ;
static jclass booleanClass, byteClass, charClass, shortClass ;
static jclass intClass, longClass, floatClass, doubleClass, voidClass ;
static jclass booleanObjClass, byteObjClass, charObjClass, shortObjClass ;
static jclass intObjClass, longObjClass, floatObjClass, doubleObjClass ;
static jclass constructorClass, methodClass, fieldClass ;
static jclass noClassDefFoundErrorClass ;
static jclass prologClass, prologExceptionClass ;
static jmethodID systemClass_identityHashCode ;
static jmethodID classClass_getName, classClass_isArray ;
static jmethodID classClass_isPrimitive, classClass_getComponentType ;
static jmethodID classClass_getConstructors, classClass_getMethods ;
static jmethodID classClass_getFields ;
static jmethodID objectClass_toString ;
static jmethodID swingUtilitiesClass_invokeAndWait ;
static jmethodID booleanObjClass_booleanValue, byteObjClass_byteValue ;
static jmethodID charObjClass_charValue, shortObjClass_shortValue ;
static jmethodID intObjClass_intValue, longObjClass_longValue ;
static jmethodID floatObjClass_floatValue, doubleObjClass_doubleValue ;
static jmethodID prologClass_GetNextEvent, prologClass_HowManyEvents ;
static jmethodID prologClass_DiscardEvents, prologClass_Init ;
static jmethodID prologClass_CallPrologStr ;
static jmethodID prologExceptionClass_throwPrologException ;

static Str ns, nskind ; /* current call signature and its kind, for error messages */


/* JNI macros */

#define JIsInstanceOf(o,c)			(*env)->IsInstanceOf(env, o, c)
#define JFindClass(n)				(*env)->FindClass(env, n)
#define JGetObjectClass(o)			(*env)->GetObjectClass(env, o)
#define JIsSameObject(o1,o2)		(*env)->IsSameObject(env, o1, o2)
#define JNewObjectA(c,id,args)		(*env)->NewObjectA(env, c, id, args)

#define JExceptionOccurred()		(*env)->ExceptionOccurred(env)
#define JExceptionClear()			(*env)->ExceptionClear(env)
#define JExceptionDescribe()		(*env)->ExceptionDescribe(env)

#define JGetMethodID(c,n,s)			(*env)->GetMethodID(env, c, n, s)
#define JGetStaticMethodID(c,n,s)	(*env)->GetStaticMethodID(env, c, n, s)
#define JCallMethod(k,c,id)			(*env)->Call##k##Method(env, c, id)
#define JCallMethodA(k,w,o,id,args) ((w)?(*env)->CallStatic##k##MethodA(env, o, id, args) \
										:(*env)->Call##k##MethodA(env, o, id, args))

#define JGetFieldID(c,n,s)			(*env)->GetFieldID(env, c, n, s)
#define JGetStaticFieldID(c,n,s)	(*env)->GetStaticFieldID(env, c, n, s)
#define JGetField(k,w,o,id)			((w)?(*env)->GetStatic##k##Field(env, o, id) \
										:(*env)->Get##k##Field(env, o, id))
#define JSetField(k,w,o,id,args)	((w)?(*env)->SetStatic##k##Field(env, o, id, args) \
										:(*env)->Set##k##Field(env, o, id, args))

#define JGetStringUTFChars(o)		(*env)->GetStringUTFChars(env, o, nil)
#define JReleaseStringUTFChars(o,str) (*env)->ReleaseStringUTFChars(env, o, str)
#define JNewStringUTF(n)			(*env)->NewStringUTF(env, n)

#define JNewArray(k,size)			(*env)->New##k##Array(env, size)
#define JNewObjectArray(c,size)		(*env)->NewObjectArray(env, size, c, nil)
#define JArrayGet(k,a,i,b)			(*env)->Get##k##ArrayRegion(env, a, i, 1, cVoidPt(b))
#define JObjectArrayGet(a,i,b)		((*b) = (*env)->GetObjectArrayElement(env, a, i))
#define JArraySet(k,a,i,vo,vn)		if( vo == nil || *(vn) != *(vo) ) \
										(*env)->Set##k##ArrayRegion(env, a, i, 1, cVoidPt(vn))
#define JObjectArraySet(a,i,vo,vn)  if( vo == nil || !JIsSameObject(*(vn), *(vo)) ) \
                                        (*env)->SetObjectArrayElement(env, a, i, *(vn))

#define JGetArrayLength(a)			(*env)->GetArrayLength(env, a)

#define GetHashCode(o)				(*env)->CallStaticIntMethod(env, systemClass,	\
														systemClass_identityHashCode, o)
#define DeleteLocalRef(lref)		(*env)->DeleteLocalRef(env, lref)

/* JOBJ */

static ExtraTypePt jobjType ;

typedef struct JObj {
	ExtraDef(JObj) ;
	jobject value ;
	int hash ;
} JObj, *JObjPt ;

#define cJObjPt(x)			((JObjPt)(x))

#define JObjValue(j)		(cJObjPt(j)->value)
#define JObjHash(j)			(cJObjPt(j)->hash)

static Pt tNullJObj ;

static Bool JObjNewAux(CVoidPt x, CVoidPt ref)
{
	return JObjHash(x) == *(int *)ref ;
}
static Pt MakeJObj(jobject obj, Bool perm)
{
	if( obj == nil )
		return tNullAtom ;
	else {
		register JObjPt j ;
		int hash = GetHashCode(obj) ;
		UChar slot = hash ;
		if( (j = ExtraFindFirst(jobjType, slot, JObjNewAux, cVoidPt(&hash))) != nil ) {
			if( JObjValue(j) != obj ) /* has to do with JNI inner workings */
				DeleteLocalRef(obj) ;
		}
		else {
			j = ExtraNew(jobjType, slot) ;
			JObjValue(j) = obj ;
			JObjHash(j) = hash ;
			if( perm )
				ExtraPermanent(j) ;
		}
		return TagExtra(jobjType,j) ;
	}
}

static void CreateNullStuff(void)
{
	JObjPt nullJObj = ExtraNew(jobjType, 0) ;
	JObjValue(nullJObj) = nil ;
	JObjHash(nullJObj) = 0 ;
	ExtraPermanent(nullJObj) ;
	tNullJObj = TagExtra(jobjType, nullJObj) ;
}

static jobject XTestJObj(register Pt t)
{
	VarValue(t) ;
	if( t == tNullAtom )
		return nil ;
	else {
		JObjPt j = cJObjPt(XTestExtraNoAlias(jobjType, t)) ;
		return JObjValue(j) ;
	}
}

static Size JObjSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(JObj) ;
}

static Bool JObjBasicGCDelete(VoidPt x)
{
	JObjPt jo = cJObjPt(x) ;
	DeleteLocalRef(JObjValue(jo)) ;
	return true ;
}


/* Errors & exceptions */

static void PrepareJErrors(Str s, Str k)
{
	ns = s ;
	nskind = k ;
}

static VoidPt JError(Str fmt, ...)
{
	Pt exc ;
	Str fmt2 ;
	va_list v ;
	JExceptionClear() ;
	fmt2 = GStrFormat("<%s '%s'> %s", nskind, ns, fmt) ;
	va_start(v, fmt) ;
	if( isoErrors_flag )
		exc = ErrorEventPrepareV("java_error", nil, nil, nil, fmt, v) ;
	else
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt2, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

static VoidPt InternalJError(Str fmt, ...)
{
	Pt exc ;
	Str fmt2 ;
	va_list v ;
	JExceptionClear() ;
	fmt2 = GStrFormat("<%s '%s'> %s", nskind, ns, fmt) ;
	va_start(v, fmt) ;
	if( isoErrors_flag )
		exc = ErrorEventPrepareV("java_error", nil, nil, nil, fmt, v) ;
	else
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt2, v) ;
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

static VoidPt JThrow(Str variant, Pt culprit, Str fmt, ...)
{
	Pt exc ;
	va_list v ;
	JExceptionClear() ;
	va_start(v, fmt) ;
	if( isoErrors_flag )
		exc = ErrorEventPrepareV("java_exception", nil, variant, culprit, fmt, v) ;
	else {
		Unused(variant) ;
		Unused(culprit) ;
		exc = ErrorEventPrepareV("ERROR", nil, nil, nil, fmt, v) ;
	}
	va_end(v) ;
	return ErrorEventTrigger(exc) ;
}

static VoidPt JErrorSig(void)
{
	return JError("Invalid %s signature", nskind) ;
}


/* Boot time functions */

static jclass GetClassAtBoot(Str name)
{
	jclass cls ;
	if( (cls = JFindClass(name)) != nil ) {
		MakeJObj(cls, true) ;	/* Put reference in the hash table */
		return cls ;
	}
	return InternalJError("Cannot access java class '%s'", name) ;
}

static jclass GetPrimitiveAtBoot(Str name)
{
	jclass cls ;
	if( (cls = JFindClass(name)) != nil ) {
		jfieldID id = JGetStaticFieldID(cls, "TYPE", "Ljava/lang/Class;") ;
		if( id != nil && (cls = JGetField(Object, true, cls, id)) != nil ) {
			MakeJObj(cls, true) ;	/* Put reference in the hash table */
			return cls ;
		}
	}
	return InternalJError("Cannot access primitive java class '%s'", name) ;
}

static jmethodID GetMethodAtBoot(jclass cls, Str name, Str sig)
{
	jmethodID id ;
	if( (id = JGetMethodID(cls, name, sig)) == nil )
		InternalJError("Cannot obtain method %s:%s", name, sig) ;
	return id ;
}

static jmethodID GetStaticMethodAtBoot(jclass cls, Str name, Str sig)
{
	jmethodID id ;
	if( (id = JGetStaticMethodID(cls, name, sig)) == nil )
		InternalJError("Cannot obtain static method %s:%s", name, sig) ;
	return id ;
}


/* Utilitary functions */

static Str GetJavaStringText(jstring obj) /* pre: obj != nil */
{
	Str jstr, str ;
	if( obj == nil || !JIsInstanceOf(obj, stringClass) )
		InternalJError("GetJavaStringText() applied to a non-string") ;
	if( (jstr = cStr(JGetStringUTFChars(obj))) == nil )
		InternalJError("Couldn't access the contents of a java string") ;
	str = GStrMake(jstr) ;
	JReleaseStringUTFChars(obj, jstr) ;
	return str ;
}

static Str GetJavaStringTextEnv(JNIEnv *nenv, jstring obj)
{
	Str jstr, str ;
	JNIEnv *saveEnv = env ;
	env = nenv ;
	if( obj == nil || !JIsInstanceOf(obj, stringClass) ) {
		env = saveEnv ;
		InternalJError("GetJavaStringText() applied to a non-string") ;
	}
	if( (jstr = cStr(JGetStringUTFChars(obj))) == nil ) {
		env = saveEnv ;
		InternalJError("Couldn't access the contents of a java string") ;
	}
	str = GStrMake(jstr) ;
	JReleaseStringUTFChars(obj, jstr) ;
	env = saveEnv ;	
	return str ;
}

static Str ClassName(jclass cls) /* pre: cls != nil */
{
	if( JCallMethod(Boolean, cls, classClass_isPrimitive) ) {
		if( JIsSameObject(cls, intClass) ) return "I" ;
		if( JIsSameObject(cls, doubleClass) ) return "D" ;
		if( JIsSameObject(cls, booleanClass) ) return "Z" ;
		if( JIsSameObject(cls, charClass) ) return "C" ;
		if( JIsSameObject(cls, floatClass) ) return "F" ;
		if( JIsSameObject(cls, voidClass) ) return "V" ;
		if( JIsSameObject(cls, longClass) ) return "J" ;
		if( JIsSameObject(cls, shortClass) ) return "S" ;
		if( JIsSameObject(cls, byteClass) ) return "B" ;
		return InternalJError("Impossible primitive type") ;
	}
	else {
		jobject obj ;
		if( (obj = JCallMethod(Object, cls, classClass_getName)) == nil )
			InternalJError("Call to 'Class.getName' failed") ;
		return GetJavaStringText(obj) ;
	}
}

static Bool CheckException(jclass expectedExceptionClass)
{
	jthrowable exc ;
	jclass cls ;
	jobject obj ;
	if( (exc = JExceptionOccurred()) ) {
#if 0
		JExceptionDescribe() ;
#endif
		JExceptionClear() ;		
		cls = JGetObjectClass(exc) ;
		if( JIsSameObject(cls, expectedExceptionClass) )
			return true ;
		if( (obj = JCallMethod(Object, exc, objectClass_toString)) == nil )
			InternalJError("Call to 'Throwable.toString' failed") ;
		JThrow(ClassName(cls), MakeJObj(exc, false),
						"Exception %s", GetJavaStringText(obj)) ;
	}
	return false ;
}

static jclass FindClass(Str name)
{
	jclass cls ;
	if( name[0] == '%' && name[2] =='\0' ) /* handle primitive types */
		switch( name[1] ) {
			case 'Z': return booleanClass ;
			case 'B': return byteClass ;
			case 'C': return charClass ;
			case 'S': return shortClass ;
			case 'I': return intClass ;
			case 'J': return longClass ;
			case 'F': return floatClass ;
			case 'D': return doubleClass ;
			case 'V': return voidClass ;
			default:  /* nothing */ break ;
		}
	if( (cls = JFindClass(name)) != nil )
		return cls ;
	return CheckException(noClassDefFoundClass)
		? JError("Could not access java class '%s'", name)
		: nil ;
}

static void EnsureInstanceOf(jobject obj, jclass cls) /* pre: cls != nil */
{
	if( !JIsInstanceOf(obj, cls) ) {
		jclass actualClass = JGetObjectClass(obj) ;
		JError("Incompatible object of class '%s' (expected a '%s')",
									ClassName(actualClass), ClassName(cls)) ;
	}
}

#if unused
static void LibClassesDelete(void)
{
	DeleteFilesWithExtension(WithCacheDirPath(nil, nil), ".class") ;
}

static void JavaCompiler(Str path, Bool useCache)
{
	useCache = false ;
	if( OSExists(path) ) {
		Info(1, "Compiling%s '%s'",
				useCache ? " and caching" : "",
				GetFileNameLastComponent(path)) ;
		if( useCache )
			OSRun(GStrFormat("javac -d \"%s\" \"%s\"",
					WithCacheDirPath(nil, nil),
					path)) ;
		else
			OSRun(GStrFormat("javac \"%s\"", path)) ;
	}
	else
		Error("Cannot access java class '%s'", path) ;
}

static void LazyCompileLibDirClass(Str name, Bool check)
{
	if( !check || !OSExists(WithCacheDirPath(name, "class")) )
		JavaCompiler(WithLibDirPath(name, "java"), true) ;
}

static void LazyCompileCurrDirClass(Str name)
{
	Bool useCache = OSPropWritable(WithCurrentDirPath(nil, nil)) ;
	JavaCompiler(WithCurrentDirPath(name, "java"), useCache) ;
}
#endif


/* Initial signature handling */

static jobject GetTarget(register Pt t, Bool *isClass)
{
	VarValue(t) ;
	if( t == tNullAtom )
		return JError("Accessing %s of the null reference", nskind) ;
	elif( IsAtom(t) ) {
		*isClass = true ;
		return FindClass(XTestAtomName(t)) ;
	}
	else {
		jobject obj = XTestJObj(t) ;
		*isClass = JIsInstanceOf(obj, classClass) ;
		return obj ;
	}
}

static void GetNameAndSig(Str n, CharPt name, Str *sig, int maxName)
{
/* Pre-validate sig */
	if( (*sig = strchr(n, ':')) == nil )
		JErrorSig() ;
	if( n == *sig )
		JError("Missing name in %s signature", nskind) ;
/* Get name */
	if( *sig - n > maxName )
		JError("Too long a name in %s signature '%s'", nskind, n) ;
	strncpy(name, n, *sig - n) ;
	name[*sig - n] = '\0' ;
	(*sig)++ ;
}


/* Validation of signatures */

static Str dummySig ;

static jclass SigToClass(Str sig, Str *outSig)
{
	Str stop ;
	jclass cls ;
	switch( *sig ) {
		case 'Z': case 'B': case 'C': case 'S':
		case 'I': case 'J': case 'F': case 'D': {
			stop = sig + 1 ;
			*outSig = stop ;
			break ;
		}
		case 'L': {
			sig++ ;
			if( (stop = strchr(sig, ';')) == nil ) JErrorSig() ;
			*outSig = stop + 1 ;
			break ;
		}
		case '[': {
			for( stop = sig ;  *stop == '[' ; stop++ ) ;
			switch( *stop ) {
				case 'Z': case 'B': case 'C': case 'S':
				case 'I': case 'J': case 'F': case 'D':
					*outSig = ++stop ;
					break ;
				case 'L':
					if( (stop = strchr(sig, ';')) == nil ) JErrorSig() ;
					*outSig = ++stop ;
					break ;
				default:
					JErrorSig() ;
			}
			break ;
		}
		default:
			return JErrorSig() ;
	}
	{
		CharPt st = cCharPt(stop);
		Char save = *st ;
		*st = '\0' ;
		cls = FindClass(sig) ;
		*st = save ;
		return cls ;
	}
}

static void ValidateFieldSig(Str sig, Str *outSig, Bool last)
{
	switch( *sig ) {
		case 'Z': case 'B': case 'C': case 'S':
		case 'I': case 'J': case 'F': case 'D':
			*outSig = sig + 1 ;
			break ;
		case 'L': case '[':
			SigToClass(sig, outSig) ;
			break ;
		default:
			JErrorSig() ;
	}
	if( last && **outSig != '\0' ) JErrorSig() ;
}

static void ValidateMethodSig(Str sig)
{
	if( *sig++ != '(' ) JErrorSig() ;
	while( *sig != '\0' && *sig != ')' )
		ValidateFieldSig(sig, &sig, false) ;
	if( *sig++ != ')' ) JErrorSig() ;
	if( *sig == 'V' || *sig == '@' ) {
		if( sig[1] != '\0' ) JErrorSig() ;
	}
	else ValidateFieldSig(sig, &dummySig, true) ;
}


/* Arrays */

static jstring NewJString(Str name)
{
	jstring jstr ;
	if( (jstr = JNewStringUTF(name)) == nil )
		JError("Could not create java string '%s'", name) ;
	return jstr ;
}

static jarray NewJArray(Str elemSig, int size)
{
	jarray arr ;
	switch( *elemSig ) {
		case 'Z': arr = JNewArray(Boolean, size) ; break ;
		case 'B': arr = JNewArray(Byte, size) ; break ;
		case 'C': arr = JNewArray(Char, size) ; break ;
		case 'S': arr = JNewArray(Short, size) ; break ;
		case 'I': arr = JNewArray(Int, size) ; break ;
		case 'J': arr = JNewArray(Long, size) ; break ;
		case 'F': arr = JNewArray(Float, size) ; break ;
		case 'D': arr = JNewArray(Double, size) ; break ;
		case 'L': case '[': {
			jclass elemClass = SigToClass(elemSig, &dummySig) ;
			arr = JNewObjectArray(elemClass, size) ;
			break ;
		}
		default: return InternalError("NewJArray (default)") ;
	}
	if( arr == nil ) JError("Could not create java array") ;
	return arr ;
}

static jarray NewJArrayC(jclass elemClass, int size)
{
	jarray arr ;
	if( JCallMethod(Boolean, elemClass, classClass_isPrimitive) ) {
		if( JIsSameObject(elemClass, intClass) ) return JNewArray(Int, size) ;
		if( JIsSameObject(elemClass, doubleClass) ) return JNewArray(Double, size) ;
		if( JIsSameObject(elemClass, booleanClass) ) return JNewArray(Boolean, size) ;
		if( JIsSameObject(elemClass, charClass) ) return JNewArray(Char, size) ;
		if( JIsSameObject(elemClass, floatClass) ) return JNewArray(Float, size) ;
		if( JIsSameObject(elemClass, longClass) ) return JNewArray(Long, size) ;
		if( JIsSameObject(elemClass, shortClass) ) return JNewArray(Short, size) ;
		if( JIsSameObject(elemClass, byteClass) ) return JNewArray(Byte, size) ;
		return InternalError("NewJArrayC") ;
	}
	else
		arr = JNewObjectArray(elemClass, size) ;
	if( arr == nil ) JError("Could not create java array") ;
	return arr ;
}

static jvalue GetJArray(Str elemSig, jarray arr, int idx)
{
  jvalue val ;
  if( arr == nil ) JError("Array is the null reference") ;
  switch( *elemSig ) {
	case 'Z': JArrayGet(Boolean, arr, idx, &(val.z)) ; break ;
	case 'B': JArrayGet(Byte, arr, idx, &(val.b)) ; break ;
	case 'C': JArrayGet(Char, arr, idx, &(val.c)) ; break ;
	case 'S': JArrayGet(Short, arr, idx, &(val.s)) ; break ;
	case 'I': JArrayGet(Int, arr, idx, &(val.i)) ; break ;
	case 'J': JArrayGet(Long, arr, idx, &(val.j)) ; break ;
	case 'F': JArrayGet(Float, arr, idx, &(val.f)) ; break ;
	case 'D': JArrayGet(Double, arr, idx, &(val.d)) ; break ;
	case 'L': /* fall through */
	case '[': JObjectArrayGet(arr, idx, &(val.l)) ; break ;
	default: JErrorSig() ;
  }
  return val ;
}

static void SetJArray(Str elemSig, jarray arr, int idx, jvalue *old, jvalue val)
{
  if( arr == nil ) JError("Array is the null reference") ;
  switch( *elemSig ) {
	case 'Z': JArraySet(Boolean, arr, idx, &(old->z), &(val.z)) ; break ;
	case 'B': JArraySet(Byte, arr, idx, &(old->b), &(val.b)) ; break ;
	case 'C': JArraySet(Char, arr, idx, &(old->c), &(val.c)) ; break ;
	case 'S': JArraySet(Short, arr, idx, &(old->s), &(val.s)) ; break ;
	case 'I': JArraySet(Int, arr, idx, &(old->i), &(val.i)) ; break ;
	case 'J': JArraySet(Long, arr, idx, &(old->j), &(val.j)) ; break ;
	case 'F': JArraySet(Float, arr, idx, &(old->f), &(val.f)) ; break ;
	case 'D': JArraySet(Double, arr, idx, &(old->d), &(val.d)) ; break ;
	case 'L': /* fall through */
	case '[': JObjectArraySet(arr, idx, &(old->l), &(val.l)) ; break ;

	default: JErrorSig() ;
  }
}


/* Conversions */

static jvalue PrologToJava(Str sig, Str *outSig, register Pt t, Bool convMore)
{											/* pre: valid signature */
	jvalue val ;
	VarValue(t) ;
	if( t == tNullAtom ) t = tNullJObj ;
	*outSig = sig + 1 ;
	switch( *sig ) {
		case 'Z': val.z = (jboolean)(XTestBool(t) ? JNI_TRUE : JNI_FALSE) ; break ;
		case 'B': val.b = (jbyte)XTestIntRange(t, SCHAR_MIN, SCHAR_MAX) ; break ;
		case 'C': val.c = (jchar)XTestChar(t) ; break ;
		case 'S': val.s = (jshort)XTestIntRange(t, SHRT_MIN, SHRT_MAX) ; break ;
		case 'I': val.i = XTestInt(t) ; break ;
		case 'J': val.j = XTestLLInt(t) ; break ;
		case 'F': val.f = (jfloat)XTestFloat(t) ; break ;
		case 'D': val.d = (jdouble)XTestFloat(t) ; break ;
		case 'L': {
			jclass expectedClass = SigToClass(sig, outSig) ;
			if( convMore && IsAtom(t) ) { /* Convert prolog atom to java string or class */
				if( JIsSameObject(expectedClass, stringClass) )
					val.l = NewJString(XAtomName(t)) ;
				elif( JIsSameObject(expectedClass, classClass) )
					val.l = FindClass(XAtomName(t)) ;
				else  JError("Invalid prolog->java conversion") ;
			}
			else {
				val.l = XTestJObj(t) ;
				EnsureInstanceOf(val.l, expectedClass) ;
			}
			break ;
		}
		case '[': {
			jclass expectedClass = SigToClass(sig, outSig) ;
			Str elemSig = sig + 1 ;
			if( convMore && t == tNilAtom ) /* Convert prolog list to java array */
				val.l = NewJArray(elemSig, 0) ;
			elif( convMore && IsList(t) ) { /* Convert prolog list to java array */
				int i, len = ListLength(t) ;
				val.l = NewJArray(elemSig, len) ;
				for( i = 0 ; i < len ; t = Drf(XListTail(t)), i++ ) {
					jvalue elem = PrologToJava(elemSig, &dummySig, XListHead(t), convMore) ;
					SetJArray(elemSig, val.l, i, nil, elem) ;
				}
			}
			else {
				val.l = XTestJObj(t) ;
				EnsureInstanceOf(val.l, expectedClass) ;
			}
			break ;
		}
		default: InternalError("PrologToJava (default)") ;
	}
	return val ;
}

static Pt JavaToProlog(Str sig, jvalue val, Bool convMore)
{											/* pre: valid signature */
	switch( *sig ) {
		case 'Z': return MakeBool(val.z == JNI_TRUE) ; break ;
		case 'B': return MakeInt(val.b) ; break ;
		case 'C': return MakeChar(val.c) ; break ;
		case 'S': return MakeInt(val.s) ; break ;
		case 'I': return MakeInt(val.i) ; break ;
		case 'J': return MakeLLInt(val.j) ; break ;
		case 'F': return MakeFloat(val.f) ; break ;
		case 'D': return MakeFloat(val.d) ; break ;
		case 'L': {
			jclass expectedClass ;
			if( val.l == nil ) return tNullAtom ;
			expectedClass = SigToClass(sig, &dummySig) ;
			EnsureInstanceOf(val.l, expectedClass) ;
			if( convMore && JIsSameObject(expectedClass, stringClass) )
			{   /* Convert java string to prolog atom */
				return MakeTempAtom(GetJavaStringText(val.l)) ; /* @@@ str is UTF8 */
			}
			elif( convMore && JIsSameObject(expectedClass, classClass) )
			{   /* Convert java class to prolog atom */
				return MakeTempAtom(ClassName(val.l)) ;
			}
			else return MakeJObj(val.l, false) ;
			break ;
		}
		case '[': {
			jclass expectedClass ;
			if( val.l == nil ) return tNullAtom ;
			expectedClass = SigToClass(sig, &dummySig) ;
			EnsureInstanceOf(val.l, expectedClass) ;
			if( convMore ) { /* Convert java array to prolog list */
				Str elemSig = sig + 1 ;
				int len = JGetArrayLength(val.l) ;
				if( len == 0 ) return tNilAtom ;
				else {
					int i ;
					Pt list = tNilAtom ;
					Hdl h = &list + 1 ;
					for( i = 0 ; i < len ; i++ ) {
						jvalue elem = GetJArray(elemSig, val.l, i) ;
						h[-1] = MakeList(JavaToProlog(elemSig, elem, convMore), tNilAtom) ;
						h = H ;
					}
					return list ;
				}
			}
			else return MakeJObj(val.l, false) ;
			break ;
		}
		default: return InternalError("JavaToProlog (default)") ;
	}
}

static void PrologToJavaMultiple(Str sig, Pt t, jvalue *vals, Bool convMore)
{											/* pre: valid signature */
	int i ;
	if( *sig++ != '(' ) InternalError("PrologToJavaMultiple (1)") ;
	for( i = 0, t = Drf(t) ; IsList(t) ; i++, t = Drf(XListTail(t)) ) {
		if( *sig == ')' ) JError("Too many arguments") ;
		else vals[i] = PrologToJava(sig, &sig, XListHead(t), convMore) ;
	}
	if( t != tNilAtom ) JError("Malformed argument list") ;
	if( *sig++ != ')' ) JError("Too few arguments") ;
}

static Pt ConvertPolyObj(jobject obj)
{
	Pt res ;
	if( obj == nil ) return tNullAtom ;

	if( JIsInstanceOf(obj, stringClass) )
		res = MakeTempAtom(GetJavaStringText(obj)) ;
	elif( JIsInstanceOf(obj, intObjClass) )
		res = MakeInt(JCallMethod(Int, obj, intObjClass_intValue)) ;
	elif( JIsInstanceOf(obj, doubleObjClass) )
		res = MakeFloat(JCallMethod(Double, obj, doubleObjClass_doubleValue)) ;
	elif( JCallMethod(Boolean, JGetObjectClass(obj), classClass_isArray) ) {
		jobject elem ;
		int i, len = JGetArrayLength(obj) ;
		Hdl h = &res + 1 ;
		res = tNilAtom ;
		for( i = 0 ; i < len ; i++ ) {
			JObjectArrayGet(obj, i, &elem) ;
			h[-1] = MakeList(ConvertPolyObj(elem), tNilAtom) ;
			h = H ;
		}
	}
	elif( JIsInstanceOf(obj, booleanObjClass) )
		res = MakeBool(JCallMethod(Boolean, obj, booleanObjClass_booleanValue)) ;
	elif( JIsInstanceOf(obj, charObjClass) )
		res = MakeChar(JCallMethod(Char, obj, charObjClass_charValue)) ;
	elif( JIsInstanceOf(obj, floatObjClass) )
		res = MakeFloat(JCallMethod(Float, obj, floatObjClass_floatValue)) ;
	elif( JIsInstanceOf(obj, longObjClass) )
		res = MakeLLInt(JCallMethod(Long, obj, longObjClass_longValue)) ;
	elif( JIsInstanceOf(obj, shortObjClass) )
		res = MakeInt(JCallMethod(Short, obj, shortObjClass_shortValue)) ;
	elif( JIsInstanceOf(obj, byteObjClass) )
		res = MakeInt(JCallMethod(Byte, obj, byteObjClass_byteValue)) ;
	else return MakeJObj(obj, false) ;

	// @@@ DeleteLocalRef(obj) ;
	return res ;
}

static Pt SerializedTermToPrologTerm(jarray arr)
{
	if( arr == nil ) goto error ;
	elif( JCallMethod(Boolean, JGetObjectClass(arr), classClass_isArray) ) {
		int i, len = JGetArrayLength(arr) ;
		jobject elem ;
		FunctorPt f ;
		Pt t ;
		ScratchSave() ;
		for( i = 0 ; ; i++ ) {
			if( i < len )
				JObjectArrayGet(arr, i, &elem) ;
			else
				elem = nil ;

			if( elem == nil ) {
				int a = ScratchCurr() - ScratchStart() - 1 ;
				t = *ScratchStart() ;
				if( a > 0 ) {
					if( !IsAtom(t) ) goto error ;
					f = LookupFunctor(XAtom(t), a) ;
					t = MakeStruct(f, ScratchStart() + 1) ;
				}
				FreeScratch() ;
				if( ScratchDistToSaved() == 0 ) break ;
				else ScratchPush(t) ;
			}
			else {
				UseScratch() ;
				ScratchPush(ConvertPolyObj(elem)) ;
			}
		}
		// @@@ DeleteLocalRef(arr) ;
		return t ;
	}
	else goto error ;

error:
	// @@@ DeleteLocalRef(arr) ;
	return Error("Invalid Java serialized prolog term") ;
}

/*
static jobject PrologTermToSerializedTerm(Pt term)
{
	return nil ;
}
*/

/* Java native methods */

static void JNICALL Native_NotifyEvent(JNIEnv *nenv, jobject self)
{
	Unused(nenv) ;
	Unused(self) ;
	if( eventNotifier != nil )
		eventNotifier() ;
}

static void JNICALL Native_Info(JNIEnv *nenv, jobject self, jstring obj)
{
	Unused(self) ;
	Info(1, GetJavaStringTextEnv(nenv, obj)) ;
}

static void JNICALL Native_Warning(JNIEnv *nenv, jobject self, jstring obj)
{
	Unused(self) ;
	Warning(GetJavaStringTextEnv(nenv, obj)) ;
}

static void JNICALL Native_Error(JNIEnv *nenv, jobject self, jstring obj)
{
	Unused(self) ;
	Error(GetJavaStringTextEnv(nenv, obj)) ;
}

static jboolean JNICALL Native_CallPrologStr(JNIEnv *nenv, jobject self, jstring obj)
{
	PrologEvent pe ;
	Unused(self) ;
/* @@@ result can be peException */
	pe = CallPrologStr(GetJavaStringTextEnv(nenv, obj)) ;
	switch( pe ) {
		case peSucc: return JNI_TRUE ;
		case peException: {
			jvalue args[2], val ;
	//		args[0].l =  JNewArray(Boolean, size) ;/*GetExceptionTerm(nil) ;*/
			args[0].l = JNewObjectArray(stringClass, 1) ;
			
			val.l = NewJString("WWW");
			JObjectArraySet(args[0].l, 0, &(args[0].l), &(val.l)) ;
	
			args[1].l =  NewJString(GetExceptionMesg(nil)) ;
			JCallMethodA(Void,true,prologExceptionClass,prologExceptionClass_throwPrologException,args) ;
			return JNI_TRUE ;
		}
		default: return JNI_FALSE ;
	}
}

static jboolean JNICALL Native_CallPrologSerialized(JNIEnv *nenv, jobject self, jarray arr)
{
	Pt t ;
	JNIEnv *saveEnv = env ;
	env = nenv ;
	Unused(self) ;
	t = SerializedTermToPrologTerm(arr) ;
	env = saveEnv ;
/* @@@ result can be peException */
	return CallProlog(t) == peSucc ? JNI_TRUE : JNI_FALSE ;
}

/* Register native method on the Java class cls */
static void RegisterNativeMethodsOnPrologClass(void)
{
	JNINativeMethod *nm, nativeTable[] = {
		{"NotifyEvent", "()V", Native_NotifyEvent},
		{"CallProlog", "(Ljava/lang/String;)Z", Native_CallPrologStr},
		{"CallProlog", "([Ljava/lang/Object;)Z", Native_CallPrologSerialized},
		{"Info", "(Ljava/lang/String;)V", Native_Info},
		{"Warning",	"(Ljava/lang/String;)V", Native_Warning},
		{"Error", "(Ljava/lang/String;)V", Native_Error},
		{nil, nil, nil}
	} ;
	for( nm = nativeTable ; nm->name != nil ; nm++ )
		(*env)->RegisterNatives(env, prologClass, nm, 1) ;
}


/* Java start & stop */

Str JavaClasspath(void)
{
	Str s ;
	BigStrOpen() ;
	BigStrAddStr(WithLibDirPath("java/prolog", "jar")) ;
	BigStrAddStr(OSPathSeparator()) ;
	BigStrAddStr(".") ;
	BigStrAddStr(OSPathSeparator()) ;
	BigStrAddStr(WithCacheDirPath(nil, nil)) ;
	if( (s = OSGetEnv("CLASSPATH", false)) != nil ) {
		BigStrAddStr(OSPathSeparator()) ;
		BigStrAddStr(s) ;
	}
	return BigStrClose() ;
}

static void JavaStart(void)
{
	JavaVMInitArgs vm_args ;
	JavaVMOption options[1] ;
	JavaVM *j ;
	int res ;
	if( jvm != nil )
		JError("Java interface already running") ;

	options[0].optionString =
			GStrFormat("-Djava.class.path=%s", JavaClasspath()) ;

	vm_args.version = JNI_VERSION_1_2 ;
	vm_args.options = options ;
	vm_args.nOptions = 1 ;
	vm_args.ignoreUnrecognized = JNI_TRUE ;
	res = JNI_CreateJavaVM(&j, (void**)cC99Fix(&env), &vm_args) ;
	InterruptOn() ; /* For some reason needs to be reactivated here */
	if( res != 0 )
		JError("Could not create Java Virtual Machine") ;

/* The System class must be loaded first */
	if( (systemClass = JFindClass("java/lang/System")) == nil )
		JError("Cannot access java class 'java/lang/System'") ;
	systemClass_identityHashCode =
		GetStaticMethodAtBoot(systemClass, "identityHashCode", "(Ljava/lang/Object;)I") ;
	MakeJObj(systemClass, true) ;	/* Only now can be put in the hash table */

/* Now we load some other important classes and methods */
	classClass = GetClassAtBoot("java/lang/Class") ;
	classClass_getName =
		GetMethodAtBoot(classClass, "getName", "()Ljava/lang/String;") ;
	classClass_isArray =
		GetMethodAtBoot(classClass, "isArray", "()Z") ;
	classClass_isPrimitive =
		GetMethodAtBoot(classClass, "isPrimitive", "()Z") ;
	classClass_getComponentType =
		GetMethodAtBoot(classClass, "getComponentType", "()Ljava/lang/Class;") ;
	classClass_getConstructors =
		GetMethodAtBoot(classClass, "getConstructors", "()[Ljava/lang/reflect/Constructor;") ;
	classClass_getMethods =
		GetMethodAtBoot(classClass, "getMethods", "()[Ljava/lang/reflect/Method;") ;
	classClass_getFields =
		GetMethodAtBoot(classClass, "getFields", "()[Ljava/lang/reflect/Field;") ;

	objectClass = GetClassAtBoot("java/lang/Object") ;
	objectClass_toString =
		GetMethodAtBoot(classClass, "toString", "()Ljava/lang/String;") ;
				
	stringClass = GetClassAtBoot("java/lang/String") ;

	swingUtilitiesClass = GetClassAtBoot("javax/swing/SwingUtilities") ;
	swingUtilitiesClass_invokeAndWait =
		GetStaticMethodAtBoot(swingUtilitiesClass, "invokeAndWait", "(Ljava/lang/Runnable;)V") ;

	noClassDefFoundClass = GetClassAtBoot("java/lang/NoClassDefFoundError") ;

	booleanObjClass = GetClassAtBoot("java/lang/Boolean") ;
	byteObjClass = GetClassAtBoot("java/lang/Byte") ;
	charObjClass = GetClassAtBoot("java/lang/Character") ;
	shortObjClass = GetClassAtBoot("java/lang/Short") ;
	intObjClass = GetClassAtBoot("java/lang/Integer") ;
	longObjClass = GetClassAtBoot("java/lang/Long") ;
	floatObjClass = GetClassAtBoot("java/lang/Float") ;
	doubleObjClass = GetClassAtBoot("java/lang/Double") ;

	booleanClass = GetPrimitiveAtBoot("java/lang/Boolean") ;
	byteClass = GetPrimitiveAtBoot("java/lang/Byte") ;
	charClass = GetPrimitiveAtBoot("java/lang/Character") ;
	shortClass = GetPrimitiveAtBoot("java/lang/Short") ;
	intClass = GetPrimitiveAtBoot("java/lang/Integer") ;
	longClass = GetPrimitiveAtBoot("java/lang/Long") ;
	floatClass = GetPrimitiveAtBoot("java/lang/Float") ;
	doubleClass = GetPrimitiveAtBoot("java/lang/Double") ;
	voidClass = GetPrimitiveAtBoot("java/lang/Void") ;

	constructorClass = GetClassAtBoot("java/lang/reflect/Constructor") ;
	methodClass = GetClassAtBoot("java/lang/reflect/Method") ;
	fieldClass = GetClassAtBoot("java/lang/reflect/Field") ;
	noClassDefFoundErrorClass = GetClassAtBoot("java/lang/NoClassDefFoundError") ;

	prologClass = GetClassAtBoot("prolog/Prolog") ;
	prologExceptionClass = GetClassAtBoot("prolog/PrologException") ;
	
	prologExceptionClass_throwPrologException =
		GetStaticMethodAtBoot(prologExceptionClass, "ThrowPrologException",
				"([Ljava/lang/Object;Ljava/lang/String;)V") ;

	RegisterNativeMethodsOnPrologClass() ;

	booleanObjClass_booleanValue =
		GetMethodAtBoot(booleanObjClass, "booleanValue", "()Z") ;
	byteObjClass_byteValue =
		GetMethodAtBoot(byteObjClass, "byteValue", "()B") ;
	charObjClass_charValue =
		GetMethodAtBoot(charObjClass, "charValue", "()C") ;
	shortObjClass_shortValue =
		GetMethodAtBoot(shortObjClass, "shortValue", "()S") ;
	intObjClass_intValue =
		GetMethodAtBoot(intObjClass, "intValue", "()I") ;
	longObjClass_longValue =
		GetMethodAtBoot(longObjClass, "longValue", "()J") ;
	floatObjClass_floatValue =
		GetMethodAtBoot(floatObjClass, "floatValue", "()F") ;
	doubleObjClass_doubleValue =
		GetMethodAtBoot(doubleObjClass, "doubleValue", "()D") ;

	prologClass_GetNextEvent =
		GetStaticMethodAtBoot(prologClass, "GetNextEvent",
												"()[Ljava/lang/Object;") ;
	prologClass_HowManyEvents =
		GetStaticMethodAtBoot(prologClass, "HowManyEvents", "()I") ;
	prologClass_DiscardEvents =
		GetStaticMethodAtBoot(prologClass, "DiscardEvents", "()V") ;
	prologClass_Init =
		GetStaticMethodAtBoot(prologClass, "Init", "()V") ;
	prologClass_CallPrologStr =
		GetStaticMethodAtBoot(prologClass, "CallProlog", "(Ljava/lang/String;)Z") ;

	jvm = j ;	/* Now is oficial: the jvm is active */

	JCallMethodA(Void, true, prologClass, prologClass_Init, nil) ;
	CheckException(nil) ;
}

#if unused
static void JavaStop()
{
	if( jvm ==  nil )
			JError("Java interface is not running") ;
	if( JExceptionOccurred() )
		JExceptionDescribe() ;
	(*jvm)->DestroyJavaVM(jvm) ;
	jvm = nil ;
}
#endif


/* Java events */

void JavaSetEventNotifier(Fun f)
{
	eventNotifier = f ;
}

Pt JavaGetEvent()
{
	jobject obj ;
	if( jvm == nil ) JavaStart() ;
	obj = JCallMethodA(Object, true, prologClass, prologClass_GetNextEvent, nil) ;
	CheckException(nil) ;
	return SerializedTermToPrologTerm(obj) ;
}

int JavaHowManyEvents()
{
	jint i ;
	if( jvm == nil ) JavaStart() ;
	i = JCallMethodA(Int, true, prologClass, prologClass_HowManyEvents, nil) ;
	CheckException(nil) ;
	return i ;
}

void JavaDiscardAllEvents()
{
	if( jvm == nil ) JavaStart() ;
	JCallMethodA(Void, true, prologClass, prologClass_DiscardEvents, nil) ;
	CheckException(nil) ;
}


/* CXPROLOG C'BUILTINS */

static Size JObjsAux(CVoidPt x)
{
	JObjPt j = cJObjPt(x) ;
	Write("pt(0x%08lx), hash(%08ld)", JObjValue(j), JObjHash(j)) ;
	return 1 ;
}
static void PJObjs()
{
	ExtraShow(jobjType, JObjsAux) ;
	JumpNext() ;
}

static void PJavaCall()
{
	jobject target ;
	Bool targetIsClass ;
	Str sig ;
	Char sigLast ;
	Str256 name ;
	jvalue args[10] ;
	jmethodID id ;
	jclass cls ;
	jobject obj ;

	if( jvm == nil ) JavaStart() ;
/* Pre-validate sig and get method name */
	PrepareJErrors(XTestAtomName(X1), "method") ;
	GetNameAndSig(ns, name, &sig, 250) ;
	sigLast = StrLast(sig) ;
/* Get target */
	target = GetTarget(X0, &targetIsClass) ;
/*** Is it a constructor? */
	if( StrEqual(name, "<init>") ) {
/* Yes, it is a constructor */
		PrepareJErrors(ns, "constructor") ;
		if( !targetIsClass )
			JError("The target of a constructor must be a class") ;
/* Get the constructor ID */
		id = JGetMethodID(target, name, sig) ;
		if( id == nil ) { /* No matching constructor? */
			JExceptionClear() ;
/*** Is it the array constructor? */
			if( JCallMethod(Boolean, target, classClass_isArray) )
			{
/* Yes, it is the array constructor */
				jclass elemClass ;
				jarray arr ;
				PrepareJErrors(ns, "array constructor") ;
				if( !StrEqual(sig, "(I)V") )
					JError("The only array constructor is <init>:(I)V") ;
				elemClass = JCallMethod(Object, target, classClass_getComponentType) ;
				PrologToJavaMultiple(sig, X2, args, true) ;
				arr = NewJArrayC(elemClass, args[0].i) ;
				MustBe( UnifyWithAtomic(X3, MakeJObj(arr, false)) ) ;
			}
	/* Give up: issue the apropriate error message */
			ValidateMethodSig(sig) ; /* If signature invalid, report now */
			if( sigLast != 'V' )
				JError("Constructors must return void") ;
									/* Otherwise report missing constructor */
			JError("Inexistent constructor in class '%s'", ClassName(target)) ;
		}
/* Create the new object and return it */
	/* Assertion: at this point the field signature is valid */
		PrologToJavaMultiple(sig, X2, args, true) ;
		obj = JNewObjectA(target, id, args) ;
		if( obj == nil ) CheckException(nil) ;
		MustBe( UnifyWithAtomic(X3, MakeJObj(obj, false)) ) ;
	}
/*** No, it is not a constructor */
/* Get the method ID */
	if( targetIsClass ) {	/* target is a class */
		if( sigLast == '@' && StrEqual(sig, "()@") )
			id = JGetStaticMethodID(target, name, "()[Ljava/lang/Object;") ;
		else
			id = JGetStaticMethodID(target, name, sig) ;
		if( id == nil ) { /* No static method available, search the method in classClass */
			JExceptionClear() ;
			id = JGetMethodID(classClass, name, sig) ;
			targetIsClass = false ; /* The method to call is not static */
			cls = target ; /* For error messages */
		}
		else cls = nil ; /* avoids warning */
	}
	else {	/* target is an object */
		cls = JGetObjectClass(target) ;
		if( sigLast == '@' && StrEqual(sig, "()@") )
			id = JGetMethodID(cls, name, "()[Ljava/lang/Object;") ;
		else
			id = JGetMethodID(cls, name, sig) ;
	}
	if( id == nil ) { /* No matching method? */
		JExceptionClear() ;
		ValidateMethodSig(sig) ; /* If signature invalid, report now */
		JError("Inexistent method in class '%s'", ClassName(cls)) ; /* Otherwise report missing method */
	}
/* Do the call and handle result */
	/* Assertion: at this point the field signature is valid */
	PrologToJavaMultiple(sig, X2, args, true) ;
	switch( sigLast ) {
		case 'Z': {
			jboolean z = JCallMethodA(Boolean, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeBool(z == JNI_TRUE)) ) ;
		}
		case 'B': {
			jbyte b = JCallMethodA(Byte, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeInt(b)) ) ;
		}
		case 'C': {
			jchar c = JCallMethodA(Char, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeChar(c)) ) ;
		}
		case 'S': {
			jshort s = JCallMethodA(Short, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeInt(s)) ) ;
		}
		case 'I': {
			jint i = JCallMethodA(Int, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeInt(i)) ) ;
		}
		case 'J': {
			jlong j = JCallMethodA(Long, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeLLInt(j)) ) ;
		}
		case 'F': {
			jfloat f = JCallMethodA(Float, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeFloat(f)) ) ;
		}
		case 'D': {
			jdouble d = JCallMethodA(Double, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeFloat(d)) ) ;
		}
		case 'V': {
			JCallMethodA(Void, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, tVoidAtom) ) ;
		}
		case ';': { /* cases 'L' & '[' */
			jobject obj = JCallMethodA(Object, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, MakeJObj(obj, false)) ) ;
		}
		case '*': { /* polymorphic result */
			jobject obj = JCallMethodA(Object, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( Unify(X3, ConvertPolyObj(obj)) ) ;
		}
		case '@': { /* prolog term */
			jobject obj = JCallMethodA(Object, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( Unify(X3, SerializedTermToPrologTerm(obj)) ) ;
		}
		default:
			InternalError("PJavaCall") ;
	}
}

static void PJavaField()
{
	jobject target ;
	Bool targetIsClass ;
	Str sig ;
	Str256 name ;
	jvalue val ;
	jfieldID id ;
	jclass cls ;

	if( jvm == nil ) JavaStart() ;
/* Pre-validate sig and get field name */
	PrepareJErrors(XTestAtomName(X1), "field") ;
	GetNameAndSig(ns, name, &sig, 250) ;
/* Get target */
	target = GetTarget(X0, &targetIsClass) ;
/* Get the field ID */
	if( targetIsClass ) {
		id = JGetStaticFieldID(target, name, sig) ;
		if( id == nil ) { /* No static field available, find a field in classClass */
			JExceptionClear() ;
			id = JGetFieldID(classClass, name, sig) ;
			targetIsClass = false ; /* The field to access is not static */
			cls = target ; /* For error messages */
		}
		else cls = nil ; /* avoids warning */
	}
	else {
		cls = JGetObjectClass(target) ;
		id = JGetFieldID(cls, name, sig) ;
	}
	if( id == nil ) { /* No matching field? */
		JExceptionClear() ;
		ValidateFieldSig(sig, &dummySig, true) ; /* If signature invalid, report now */
		JError("Inexistent field in class '%s'", ClassName(cls)) ; /* Otherwise report missing field */
	}
/* Get/Set the field */
	/* Assertion: at this point the field signature is valid */
	switch( *sig ) {
		case 'Z': {
			Pt old = MakeBool(JGetField(Boolean, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Boolean, targetIsClass, target, id, val.z) ;
			JumpNext() ;
		}
		case 'B': {
			Pt old = MakeByte(JGetField(Byte, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Byte, targetIsClass, target, id, val.b) ;
			JumpNext() ;
		}
		case 'C': {
			Pt old = MakeChar(JGetField(Char, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Char, targetIsClass, target, id, val.c) ;
			JumpNext() ;
		}
		case 'S': {
			Pt old = MakeInt(JGetField(Short, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Short, targetIsClass, target, id, val.s) ;
			JumpNext() ;
		}
		case 'I': {
			Pt old = MakeInt(JGetField(Int, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Int, targetIsClass, target, id, val.i) ;
			JumpNext() ;
		}
		case 'J': {
			Pt old = MakeLLInt(JGetField(Long, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Long, targetIsClass, target, id, val.j) ;
			JumpNext() ;
		}
		case 'F': {
			Pt old = MakeFloat(JGetField(Float, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Float, targetIsClass, target, id, val.f) ;
			JumpNext() ;
		}
		case 'D': {
			Pt old = MakeFloat(JGetField(Double, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Double, targetIsClass, target, id, val.d) ;
			JumpNext() ;
		}
		case 'L': case '[': {
			Pt old = MakeJObj(JGetField(Object, targetIsClass, target, id), false) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3, true) ;
			JSetField(Object, targetIsClass, target, id, val.l) ;
			JumpNext() ;
		}
		default:
			InternalError("PJavaField") ;
	}
}

static void PJavaArray()
{
	Str sig, elemSig ;
	int idx ;
	jvalue old, val ;
	jclass expectedClass ;
	jarray arr ;

	if( jvm == nil ) JavaStart() ;

	PrepareJErrors(XTestAtomName(X0), "array") ;
	sig = ns ;
	elemSig = sig + 1 ;
	ValidateFieldSig(sig, &dummySig, true) ; /* If signature invalid, report now */

	expectedClass = SigToClass(sig, &dummySig) ;
	arr = XTestJObj(X1) ;
	EnsureInstanceOf(arr, expectedClass) ;

	idx = XTestNat(X2) ;

	old = GetJArray(elemSig, arr, idx) ;
	Ensure( UnifyWithAtomic(X3, JavaToProlog(elemSig, old, false)) ) ;

	val = PrologToJava(elemSig, &dummySig, X4, true) ;
	SetJArray(elemSig, arr, idx, &old, val) ;
	JumpNext() ;
}

static void PJavaConvert() /* full conversion java (X1) <->  prolog (X2) */
{
	Str sig ;
	jvalue val ;
	Pt t1 ;
	if( jvm == nil ) JavaStart() ;
	PrepareJErrors(XTestAtomName(X0), "conversion") ;
	sig = ns ;
	ValidateFieldSig(sig, &dummySig, true) ; /* If signature invalid, report now */

	t1 = Drf(X1) ;
	if( *sig == 'L' || *sig == '[' ) {
		if( IsVar(t1) ) { /* convert prolog (X2) -> java (X1) */
			val = PrologToJava(sig, &dummySig, X2, true) ;
			MustBe( UnifyWithAtomic(t1, MakeJObj(val.l, false)) ) ;
		}
		else { /* convert java (X1) -> prolog (X2) */
			val.l = XTestJObj(t1) ;
			MustBe( Unify(X2, JavaToProlog(sig, val, true)) ) ; /* Can be a list */
		}
	}
	else {
		if( IsVar(t1) ) { /* convert prolog (X2) -> java (X1) */
			PrologToJava(sig, &dummySig, X2, true) ; /* only for validation */
			MustBe( UnifyWithAtomic(t1, X2) ) ;
		}
		else { /* convert java (X1) -> prolog (X2) */
			PrologToJava(sig, &dummySig, t1, true) ; /* only for validation */
			MustBe( UnifyWithAtomic(X2, t1) ) ;
		}
	}
}

static void PCallPrologThroughJava()	/* For testing reentrant calls */
{
	jboolean z ;
	jvalue args[1] ;
	if( jvm == nil ) JavaStart() ;
	args[0].l = NewJString(TermAsStr(X0)) ;
	z = JCallMethodA(Boolean, true, prologClass, prologClass_CallPrologStr, args) ;
	CheckException(nil) ;
	MustBe( z == JNI_TRUE ) ;
}

static void PCallPrologThroughJavaSerialized()	/* For testing reentrant calls */
{
	jboolean z ;
	jvalue args[1] ;
	if( jvm == nil ) JavaStart() ;
	args[0].l = NewJString(TermAsStr(X0)) ;
	z = JCallMethodA(Boolean, true, prologClass, prologClass_CallPrologStr, args) ;
	CheckException(nil) ;
	MustBe( z == JNI_TRUE ) ;
}

static void PJavaCompile()
{
	Str comm = GStrFormat("javac -d %s -cp %s %s",
							WithCacheDirPath(nil, nil),
							JavaClasspath(), XTestAtomName(X0)) ;
	Info(1, comm) ;
	OSRun(comm) ;
	JumpNext() ;
}

static void PJavaClean0()
{
	DeleteFilesWithExtension(WithCacheDirPath(nil, nil), ".class") ;
	JumpNext() ;
}

static void PJavaClean1()
{
	OSDeleteTree(GStrFormat("%s%s",
					WithCacheDirPath(nil, nil), XTestAtomName(X0))) ;
	JumpNext() ;
}

static Str moreJavaBuiltinsStr = "											\
																				\
java_classpath(P) :-															\
	java_call('java/lang/System',												\
				'getProperty:(Ljava/lang/String;)Ljava/lang/String;',			\
				['java.class.path'], R),										\
	java_convert('Ljava/lang/String;',R,P).										\
																				\
java_check :-																	\
	java_field('java/lang/System', 'out:Ljava/io/PrintStream;', Out, Out),		\
	java_call(Out, 'print:(Ljava/lang/String;)V', ['CLASSPATH='], _),			\
	java_classpath(P),															\
	java_call(Out, 'println:(Ljava/lang/String;)V', [P], _),					\
	java_call(Out, 'println:(Ljava/lang/String;)V', ['Java is working!'], _).	\
																				\
zjava :- java_check.															\
																				\
'$$_call_prolog_through_java_2'(G) :-	/* For testing reentrant calls 2 */		\
	atom_termq(A, G),															\
	java_call('Prolog', 'CallStr:(Ljava/lang/String;)Z', [A], true).			\
" ;

void JavaInit()
{
	jobjType = ExtraTypeNew("JOBJ", JObjSizeFun, nil, JObjBasicGCDelete, 256) ;
	ExtraTypeDoesNotSupportAliasing(jobjType) ;

	PrepareJErrors("", "") ;
	CreateNullStuff() ;
	jvm = nil ;	/* jvm is inactive at startup */
	InstallCBuiltinPred("jobjs", 0, PJObjs) ;
	InstallCBuiltinPred("java_call", 4, PJavaCall) ;
	InstallCBuiltinPred("java_field", 4, PJavaField) ;
	InstallCBuiltinPred("java_array", 5, PJavaArray) ;
	InstallCBuiltinPred("java_convert", 3, PJavaConvert) ;
	InstallCBuiltinPred("$$_call_prolog_through_java", 1, PCallPrologThroughJava) ;
	InstallCBuiltinPred("$$_call_prolog_through_java_serialized",
								1, PCallPrologThroughJavaSerialized) ;
	InstallCBuiltinPred("java_compile", 1, PJavaCompile) ;
	InstallCBuiltinPred("java_clean", 0, PJavaClean0) ;
	InstallCBuiltinPred("java_clean", 1, PJavaClean1) ;
	ZBasicLoadBuiltinsStr(moreJavaBuiltinsStr) ;

	RegisterForeignInterface("Java", " j") ;
}

#else /* USE_JAVA */

void JavaInit() { /* nothing */ }

#endif /* USE_JAVA */

/*
gcc  -I/usr/java/j2/include:/usr/java/j2/include/linux -L/usr/java/j2/jre/lib/i386/client/ -ljvm Java.c -o Java
LD_LIBRARY_PATH=/usr/java/j2/jre/lib/i386/client/:/usr/java/j2/jre/lib/i386/ Java
*/
