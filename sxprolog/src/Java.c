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

static JNIEnv *env = nil ;
static JavaVM *jvm = nil ;
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
static jmethodID booleanObjClass_booleanValue, byteObjClass_byteValue,
				charObjClass_charValue, shortObjClass_shortValue,
				intObjClass_intValue, longObjClass_longValue,
				floatObjClass_floatValue, doubleObjClass_doubleValue ;
static jmethodID booleanObjClass_constructor, byteObjClass_constructor,
				charObjClass_constructor, shortObjClass_constructor,
				intObjClass_constructor, longObjClass_constructor,
				floatObjClass_constructor, doubleObjClass_constructor ;
static jmethodID prologClass_GetNextEvent, prologClass_HowManyEvents ;
static jmethodID prologClass_DiscardEvents, prologClass_Init ;
static jmethodID prologClass_CallPrologStr ;
static jmethodID prologExceptionClass_throwPrologException ;

static Str ns, nskind ; /* current call signature and its kind, for error messages */


/* JNI macros */

#define JIsInstanceOf(o,c)			(*env)->IsInstanceOf(env, o, c)
#define JFindClass(n)				(*env)->FindClass(env, n)
#define JGetObjectClass(o)			(*env)->GetObjectClass(env, o)
#define JIsSame(o1,o2)				(*env)->IsSameObject(env, o1, o2)
#define JIsNil(o)					JIsSame(o, nil)
#define JNewObject(c,id,arg)		(*env)->NewObject(env, c, id, arg)
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
#define JArraySet(k,a,i,vo,vn)		if( (vo) == nil || *(vn) != *(vo) ) \
										(*env)->Set##k##ArrayRegion(env, a, i, 1, cVoidPt(vn))
#define JObjectArraySet(a,i,vo,vn)  if( (vo) == nil || !JIsSame(*(vn), *(vo)) ) \
                                        (*env)->SetObjectArrayElement(env, a, i, *(vn))

#define JGetArrayLength(a)			(*env)->GetArrayLength(env, a)

#define GetHashCode(o)				(*env)->CallStaticIntMethod(env, systemClass,	\
														systemClass_identityHashCode, o)
#define JDeleteLocalRef(lref)		(*env)->DeleteLocalRef(env, lref)
#define JDeleteGlobalRef(lref)		(*env)->DeleteGlobalRef(env, lref)
#define JNewGlobalRef(ref)			(*env)->NewGlobalRef(env, ref)
#define JGetObjectRefType(o)		(*env)->GetObjectRefType(env, o)  /* Invalid=0, Local= 1, Global=2, WeakGlobal=3 */


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
static Pt MakeJObj(jobject obj)
{
	if( JIsNil(obj) )
		return tNullAtom ;
	else {
		register JObjPt j ;
		int hash = GetHashCode(obj) ;
		UChar slot = hash ;
		if( (j = ExtraFindFirst(jobjType, slot, JObjNewAux, cVoidPt(&hash))) != nil ) {
			if( JObjValue(j) != obj ) /* has to do with JNI inner workings */
				JDeleteLocalRef(obj) ;
		}
		else {
			j = ExtraNew(jobjType, slot) ;
			JObjValue(j) = JNewGlobalRef(obj) ;
			JObjHash(j) = hash ;
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

static jobject Globalize(jobject obj)
{
	JObjPt j = XTestExtraNoAlias(jobjType, MakeJObj(obj)) ;
	ExtraPermanent(j) ;
	return JObjValue(j) ;
}

static Size JObjSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(JObj) ;
}

static Bool JObjBasicGCDelete(VoidPt x)
{
	JDeleteGlobalRef(JObjValue(x)) ;
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
	if( (cls = JFindClass(name)) != nil )
		return Globalize(cls) ;
	return InternalJError("Cannot access java class '%s'", name) ;
}

static jclass GetPrimitiveAtBoot(Str name)
{
	jclass cls ;
	if( (cls = JFindClass(name)) != nil ) {
		jfieldID id = JGetStaticFieldID(cls, "TYPE", "Ljava/lang/Class;") ;
		if( id != nil && (cls = JGetField(Object, true, cls, id)) != nil )
			return Globalize(cls) ;
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
	if( JIsNil(obj) || !JIsInstanceOf(obj, stringClass) )
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
	if( JIsNil(obj) || !JIsInstanceOf(obj, stringClass) ) {
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
		if( JIsSame(cls, intClass) ) return "I" ;
		if( JIsSame(cls, doubleClass) ) return "D" ;
		if( JIsSame(cls, booleanClass) ) return "Z" ;
		if( JIsSame(cls, charClass) ) return "C" ;
		if( JIsSame(cls, floatClass) ) return "F" ;
		if( JIsSame(cls, voidClass) ) return "V" ;
		if( JIsSame(cls, longClass) ) return "J" ;
		if( JIsSame(cls, shortClass) ) return "S" ;
		if( JIsSame(cls, byteClass) ) return "B" ;
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
		if( JIsSame(cls, expectedExceptionClass) )
			return true ;
		if( (obj = JCallMethod(Object, exc, objectClass_toString)) == nil )
			InternalJError("Call to 'Throwable.toString' failed") ;
		JThrow(ClassName(cls), MakeJObj(exc),
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

static Str GetResultSig(Str sig)
{
	Str s ;
	for( s = sig ; *s != '\0' ; s++ )
		if( *s == ')' )
			return s+1;
	return sig ;
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
		if( JIsSame(elemClass, intClass) ) return JNewArray(Int, size) ;
		if( JIsSame(elemClass, doubleClass) ) return JNewArray(Double, size) ;
		if( JIsSame(elemClass, booleanClass) ) return JNewArray(Boolean, size) ;
		if( JIsSame(elemClass, charClass) ) return JNewArray(Char, size) ;
		if( JIsSame(elemClass, floatClass) ) return JNewArray(Float, size) ;
		if( JIsSame(elemClass, longClass) ) return JNewArray(Long, size) ;
		if( JIsSame(elemClass, shortClass) ) return JNewArray(Short, size) ;
		if( JIsSame(elemClass, byteClass) ) return JNewArray(Byte, size) ;
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

/* tests:
  
 java_call('prolog/Prolog', 'testMixArray:([Ljava/lang/Object;)V', [[12345, 23.56, ola, 1'null]], Res).
 java_convert('Ljava/lang/Boolean;',R,1), java_obj(R, C, V).
 java_convert('Ljava/lang/String;',R,'1'), java_obj(R, C, V).
 java_convert('Ljava/lang/Boolean;',R,true), java_obj(R, C, V).
 java_convert('Ljava/lang/String;',R,123), java_obj(R, C, V).
 java_convert('Ljava/lang/String;',R,1.2e-30), java_obj(R, C, V).
 java_convert('Ljava/lang/Integer;',R,'123'), java_obj(R, C, V).
 java_convert('Ljava/lang/Double;',R,'123.456'), java_obj(R, C, V).
 java_convert('Ljava/lang/Short;',R,'123'), java_obj(R, C, V).
*/

static jobject AtomicToJObject(Pt t, jclass expectedClass)
{
	// test: java_call('prolog/Prolog', 'testMixArray:([Ljava/lang/Object;)V', [[12345, 23.56, ola, 1'null]], Res).
	jobject res = nil ;
	Str32 str ;
	Char c;
	if( IsAtom(t) ) { /* Convert prolog atom to java string or class */
		if( JIsSame(expectedClass, stringClass) || JIsSame(expectedClass, objectClass) )
			res = NewJString(XAtomName(t)) ;
		elif( JIsSame(expectedClass, classClass) )
			res = FindClass(XAtomName(t)) ;
		elif( JIsSame(expectedClass, booleanObjClass) )
			res = JNewObject(booleanObjClass, booleanObjClass_constructor, XTestBool(t)) ;
		elif( JIsSame(expectedClass, intObjClass) ) {
			PInt i;
			if( sscanf(XAtomName(t), "%ld%c", &i, &c) == 1 )
				res = JNewObject(intObjClass, intObjClass_constructor, i) ;
		}
		elif( JIsSame(expectedClass, doubleObjClass) ) {
			double f;
			if( sscanf(XAtomName(t), "%lf%c", &f, &c) == 1 )
				res = JNewObject(doubleObjClass, doubleObjClass_constructor, f) ;
		}
		if( res == nil )
			JError("Invalid prolog->java conversion for atom") ;
	}
	elif( IsInt(t) ) {
		if( JIsSame(expectedClass, objectClass) || JIsSame(expectedClass, intObjClass) )
			res = JNewObject(intObjClass, intObjClass_constructor, XInt(t)) ;
		elif( JIsSame(expectedClass, doubleObjClass) )
			res = JNewObject(doubleObjClass, doubleObjClass_constructor, (double)XInt(t)) ;
		elif( JIsSame(expectedClass, booleanObjClass) )
			res = JNewObject(booleanObjClass, booleanObjClass_constructor, XInt(t)) ;
		elif( JIsSame(expectedClass, charObjClass) )
			res = JNewObject(charObjClass, charObjClass_constructor, XInt(t)) ;
		elif( JIsSame(expectedClass, floatObjClass) )
			res = JNewObject(floatObjClass, floatObjClass_constructor, (double)XInt(t)) ;
		elif( JIsSame(expectedClass, longObjClass) )
			res = JNewObject(longObjClass, longObjClass_constructor, XInt(t)) ;
		elif( JIsSame(expectedClass, shortObjClass) )
			res = JNewObject(shortObjClass, shortObjClass_constructor, XInt(t)) ;
		elif( JIsSame(expectedClass, byteObjClass) )
			res = JNewObject(byteObjClass, byteObjClass_constructor, XInt(t)) ;
		elif( JIsSame(expectedClass, stringClass) ) {
			sprintf(str, "%ld", XInt(t)) ;
			res = NewJString(str) ;
		}
		else JError("Invalid prolog->java conversion for int") ;
	}
	elif( IsFloat(t) ) {
		if( JIsSame(expectedClass, objectClass) || JIsSame(expectedClass, doubleObjClass) )
			res = JNewObject(doubleObjClass, doubleObjClass_constructor, (double)XFloat(t)) ;
		elif( JIsSame(expectedClass, floatObjClass) )
			res = JNewObject(floatObjClass, floatObjClass_constructor, (double)XFloat(t)) ;
		elif( JIsSame(expectedClass, stringClass) ) {
			sprintf(str, "%lg", (double)XFloat(t)) ;
			res = NewJString(str) ;
		}
		else JError("Invalid prolog->java conversion for float") ;
	}
	elif( IsThisExtra(jobjType, t) ) {
		res = XTestJObj(t) ;
		EnsureInstanceOf(res, expectedClass) ;
	}
	else
		JError("Invalid prolog->java conversion for atomic") ;
	return res ;
}

static Pt JObjectToAtomic(jobject obj)
{
	Pt res ;
	jclass cls = JGetObjectClass(obj) ;
	// Mesg("%s", ClassName(cls));

	if( JIsNil(obj) )
		res = tNullAtom ;
	elif( JCallMethod(Boolean, cls, classClass_isArray) )
		res = MakeJObj(obj) ;
	elif( JIsSame(cls, stringClass) )
		res = MakeTempAtom(GetJavaStringText(obj)) ;	
	elif( JIsSame(cls, intObjClass) )
		res = MakeInt(JCallMethod(Int, obj, intObjClass_intValue)) ;
	elif( JIsSame(cls, doubleObjClass) )
		res = MakeFloat(JCallMethod(Double, obj, doubleObjClass_doubleValue)) ;
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
	else
		res = MakeJObj(obj) ;
	return res ;
}

static jvalue PrologToJava(Str sig, Str *outSig, register Pt t)
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
		case 'L': val.l = AtomicToJObject(t, SigToClass(sig, outSig)) ; break ;
		case '[': {
			jclass expectedClass = SigToClass(sig, outSig) ;
			Str elemSig = sig + 1 ;
			if( t == tNilAtom ) /* Convert prolog list to java array */
				val.l = NewJArray(elemSig, 0) ;
			elif( IsList(t) ) { /* Convert prolog list to java array */
				int i, len = ListLength(t) ;
				val.l = NewJArray(elemSig, len) ;
				for( i = 0 ; i < len ; t = Drf(XListTail(t)), i++ ) {
					jvalue elem = PrologToJava(elemSig, &dummySig, XListHead(t)) ;
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
			if( JIsSame(expectedClass, stringClass) )
			{   /* Convert java string to prolog atom */
		Mesg("mmm");
				return MakeTempAtom(GetJavaStringText(val.l)) ; /* @@@ str is UTF8 */
			}
			elif( convMore && JIsSame(expectedClass, classClass) )
			{   /* Convert java class to prolog atom */
				return MakeTempAtom(ClassName(val.l)) ;
			}
			else return MakeJObj(val.l) ;
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
			else return MakeJObj(val.l) ;
			break ;
		}
		default: return InternalError("JavaToProlog (default)") ;
	}
}

static void PrologToJavaMultiple(Str sig, Pt t, jvalue *vals)
{											/* pre: valid signature */
	int i ;
	if( *sig++ != '(' ) InternalError("PrologToJavaMultiple (1)") ;
	for( i = 0, t = Drf(t) ; IsList(t) ; i++, t = Drf(XListTail(t)) ) {
		if( *sig == ')' ) JError("Too many arguments") ;
		else vals[i] = PrologToJava(sig, &sig, XListHead(t)) ;
	}
	if( t != tNilAtom ) JError("Malformed argument list") ;
	if( *sig++ != ')' ) JError("Too few arguments") ;
}

static Pt ConvertPolyObj(jobject obj)	/* destroys arg */
{
	Pt res ;
	if( JIsNil(obj) )
		res = tNullAtom ;
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
	else
		res = JObjectToAtomic(obj) ;
	JDeleteLocalRef(obj) ;
	return res ;
}

static Pt JavaTermUnserialize(jarray arr)	/* destroys arg */
{
	if( JCallMethod(Boolean, JGetObjectClass(arr), classClass_isArray) ) {
		int i, len = JGetArrayLength(arr) ;
		jobject elem ;
		UseScribbling() ;
		for( i = 0 ; i < len ; i++ ) {
			JObjectArrayGet(arr, i, &elem) ;
			ScribblingPush(elem == nil ? nil : ConvertPolyObj(elem));
		}
		JDeleteLocalRef(arr) ;
		return TermUnserialize(FreeScribbling(), len) ;
	}
	else {
		JDeleteLocalRef(arr) ;
		return Error("Invalid Java serialized prolog term") ;
	}
}


static jarray JavaTermSerialize(Pt term)
{
	Size len ;
	int i;
	Str32 elemsClass;
	Hdl serial = TermSerialize(term, &len);
	jarray arr = NewJArray(strcpy(elemsClass, "Ljava/lang/Object;"), len);
	for( i = 0 ; i < len ; i++ ) {
		jobject elem = AtomicToJObject(serial[i], objectClass);
		(*env)->SetObjectArrayElement(env, arr, i, elem);
	}
	return arr ;
}


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

static jboolean HandlePrologCallResult(PrologEvent pe)
{
	switch( pe ) {
		case peSucc:
			return JNI_TRUE ;
		case peException: {
			jvalue args[2], val ;
	//		args[0].l =  JNewArray(Boolean, size) ;/*GetExceptionTerm(nil) ;*/
			args[0].l = JNewObjectArray(stringClass, 1) ;
			val.l = NewJString("---");
			JObjectArraySet(args[0].l, 0, &(args[0].l), &(val.l)) ;
			args[1].l =  NewJString(GetExceptionMesg(nil)) ;
			JCallMethodA(Void, true, prologExceptionClass, prologExceptionClass_throwPrologException, args) ;
			return JNI_TRUE ;
		}
		default:
			return JNI_FALSE ;
	}
}

static jboolean JNICALL Native_CallPrologStr(JNIEnv *nenv, jobject self, jstring obj)
{
	Unused(self) ;
	return HandlePrologCallResult(CallPrologStr(GetJavaStringTextEnv(nenv, obj))) ;
}

static jboolean JNICALL Native_CallPrologSerialized(JNIEnv *nenv, jobject self, jarray arr)
{
	Unused(self) ;
	JNIEnv *saveEnv = env ;
	env = nenv ;
	Pt t = JavaTermUnserialize(arr) ;
	env = saveEnv ;
	return HandlePrologCallResult(CallProlog(t)) ;
}

static jarray Native_IVarGet(JNIEnv *nenv, jobject self, jstring name)
{
	Unused(self) ;
	AtomPt n = LookupTempAtom(GetJavaStringTextEnv(nenv, name));
	return JavaTermSerialize(IVarGet(n));
}

static void JNICALL Native_IVarSetStr(JNIEnv *nenv, jobject self, jstring name, jstring term)
{
	AtomPt n = LookupTempAtom(GetJavaStringTextEnv(nenv, name));
	Unused(self) ;
	Pt t = ZTermFromStrThruProlog(GetJavaStringTextEnv(nenv, term)) ;
	IVarSet(n, t, false);
}

static void JNICALL Native_IVarSetSerialized(JNIEnv *nenv, jobject self, jstring name, jarray arr)
{	
	AtomPt n = LookupTempAtom(GetJavaStringTextEnv(nenv, name));
	Unused(self) ;
	if( arr == nil ) {
	//	Mesg("Native_IVarSet nil");
		IVarSet(n, MakeAtom("$$_undefined_ivar"), false);
	}
	else
		IVarSet(n, JavaTermUnserialize(arr), false);
}

static jboolean JNICALL Native_CoroutiningWaitingInput(JNIEnv *nenv, jobject self, jstring coroutine)
{
	Unused(self) ;
	return ThreadWaitingInputByAlias(GetJavaStringTextEnv(nenv, coroutine)) ;
}

static void JNICALL Native_CoroutiningInputLine(JNIEnv *nenv, jobject self, jstring coroutine, jstring line)
{
	Unused(self) ;
	ThreadInputLineByAlias(GetJavaStringTextEnv(nenv, coroutine),
							GetJavaStringTextEnv(nenv, line)) ;
}

static jstring JNICALL Native_CoroutiningOutputText(JNIEnv *nenv, jobject self, jstring coroutine)
{
	Unused(self) ;
	Unused(coroutine) ;
	JNIEnv *saveEnv = env ;
	env = nenv ;	
	Str s = ThreadOutputTextByAlias(GetJavaStringTextEnv(nenv, coroutine)) ;
	jstring jstr = NewJString(s) ;
	env = saveEnv ;
	return jstr ;
}

/* Register native method on the Java class cls */
static void RegisterNativeMethodsOnPrologClass(void)
{
	JNINativeMethod *nm, nativeTable[] = {
		{"NotifyEvent", "()V", Native_NotifyEvent},
		{"Info", "(Ljava/lang/String;)V", Native_Info},
		{"Warning",	"(Ljava/lang/String;)V", Native_Warning},
		{"Error", "(Ljava/lang/String;)V", Native_Error},
		{"CallProlog", "(Ljava/lang/String;)Z", Native_CallPrologStr},
		{"CallProlog", "([Ljava/lang/Object;)Z", Native_CallPrologSerialized},
		{"IVarGet", "(Ljava/lang/String;)[Ljava/lang/Object;", Native_IVarGet},
		{"IVarSet", "(Ljava/lang/String;Ljava/lang/String;)V", Native_IVarSetStr},
		{"IVarSet", "(Ljava/lang/String;[Ljava/lang/Object;)V", Native_IVarSetSerialized},
		{"coroutiningWaitingInput", "(Ljava/lang/String;)Z", Native_CoroutiningWaitingInput},
		{"coroutiningInputLine", "(Ljava/lang/String;Ljava/lang/String;)V", Native_CoroutiningInputLine},
		{"coroutiningOutputText", "(Ljava/lang/String;)Ljava/lang/String;", Native_CoroutiningOutputText},
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

void JavaCreateVM(void)
{
	JavaVMInitArgs vm_args ;
	JavaVMOption options[1] ;
	int res ;

	options[0].optionString =
			GStrFormat("-Djava.class.path=%s", JavaClasspath()) ;
	vm_args.version = JNI_VERSION_1_2 ;
	vm_args.options = options ;
	vm_args.nOptions = 1 ;
	vm_args.ignoreUnrecognized = JNI_TRUE ;
	res = JNI_CreateJavaVM(&jvm, (void**)cC99Fix(&env), &vm_args) ;
	InterruptOn() ; /* For some reason needs to be reactivated here */
	if( res != 0 )
		JError("Could not create Java Virtual Machine") ;
}

static void JavaStart(void)
{
	if( jvm == nil ) {
		JavaCreateVM();
	}
	
/* The System class must be loaded first because of the hash code of the objects */
	if( (systemClass = JFindClass("java/lang/System")) == nil )
		JError("Cannot access java class 'java/lang/System'") ;
	systemClass_identityHashCode =
		GetStaticMethodAtBoot(systemClass, "identityHashCode", "(Ljava/lang/Object;)I") ;		
	systemClass = Globalize(systemClass);

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
		
	booleanObjClass_constructor =
		GetMethodAtBoot(booleanObjClass, "<init>", "(Z)V") ;
	byteObjClass_constructor =
		GetMethodAtBoot(byteObjClass, "<init>", "(B)V") ;
	charObjClass_constructor =
		GetMethodAtBoot(charObjClass, "<init>", "(C)V") ;
	shortObjClass_constructor =
		GetMethodAtBoot(shortObjClass, "<init>", "(S)V") ;
	intObjClass_constructor =
		GetMethodAtBoot(intObjClass, "<init>", "(I)V") ;
	longObjClass_constructor =
		GetMethodAtBoot(longObjClass, "<init>", "(J)V") ;
	floatObjClass_constructor =
		GetMethodAtBoot(floatObjClass, "<init>", "(F)V") ;
	doubleObjClass_constructor =
		GetMethodAtBoot(doubleObjClass, "<init>", "(D)V") ;

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

	JExceptionClear() ;
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
	return JavaTermUnserialize(obj) ;
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
			if( JCallMethod(Boolean, target, classClass_isArray) ) {
	/* Yes, it is the array constructor */
				jclass elemClass ;
				jarray arr ;
				PrepareJErrors(ns, "array constructor") ;
				if( !StrEqual(sig, "(I)V") )
					JError("The only array constructor is <init>:(I)V") ;
				elemClass = JCallMethod(Object, target, classClass_getComponentType) ;
				PrologToJavaMultiple(sig, X2, args) ;
				arr = NewJArrayC(elemClass, args[0].i) ;
				MustBe( UnifyWithAtomic(X3, MakeJObj(arr)) ) ;
			}
		/* Give up: issue the apropriate error message */
			ValidateMethodSig(sig) ;
		/* If signature invalid, report now */
			if( sigLast != 'V' )
				JError("Constructors must return void") ;
		/* Otherwise report missing constructor */
			JError("Inexistent constructor in class '%s'", ClassName(target)) ;
		}
/* Create the new object and return it */
	/* Assertion: at this point the field signature is valid */
		PrologToJavaMultiple(sig, X2, args) ;
		obj = JNewObjectA(target, id, args) ;
		if( obj == nil ) CheckException(nil) ;
		MustBe( UnifyWithAtomic(X3, MakeJObj(obj)) ) ;
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
	PrologToJavaMultiple(sig, X2, args) ;
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
			sig = GetResultSig(sig) ;
			jobject obj = JCallMethodA(Object, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( UnifyWithAtomic(X3, JObjectToAtomic(obj)) ) ;
		}
		case '*': { /* polymorphic result */
			jobject obj = JCallMethodA(Object, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( Unify(X3, ConvertPolyObj(obj)) ) ;
		}
		case '@': { /* prolog term */
			jobject obj = JCallMethodA(Object, targetIsClass, target, id, args) ;
			CheckException(nil) ;
			MustBe( Unify(X3, JavaTermUnserialize(obj)) ) ;
		}
		default:
			InternalError("PJavaCall") ;
	}
}

static void PJavaNew()
{
	jobject target ;
	Bool targetIsClass ;
	Str256 sig ;
	Char sigLast ;
	jvalue args[10] ;
	jmethodID id ;
	jobject obj ;

	if( jvm == nil ) JavaStart() ;
/* Pre-validate sig and get method name */
	PrepareJErrors(XTestAtomName(X1), "constructor") ;
	strcpy(sig, "(");
	strcat(sig, ns);
	strcat(sig, ")V");
	sigLast = StrLast(sig) ;
/* Get target */
	target = GetTarget(X0, &targetIsClass) ;
	PrepareJErrors(ns, "constructor") ;
	if( !targetIsClass )
		JError("The target of a constructor must be a class") ;
/* Get the constructor ID */
	id = JGetMethodID(target, "<init>", sig) ;
	if( id == nil ) { /* No matching constructor? */
		JExceptionClear() ;
	/*** Is it the array constructor? */
		if( JCallMethod(Boolean, target, classClass_isArray) ) {
	/* Yes, it is the array constructor */
			jclass elemClass ;
			jarray arr ;
			PrepareJErrors(ns, "array constructor") ;
			if( !StrEqual(sig, "(I)V") )
				JError("The array constructor has a single integer argument") ;
			elemClass = JCallMethod(Object, target, classClass_getComponentType) ;
			PrologToJavaMultiple(sig, X2, args) ;
			arr = NewJArrayC(elemClass, args[0].i) ;
			MustBe( UnifyWithAtomic(X3, MakeJObj(arr)) ) ;
		}
	/* Give up: issue the apropriate error message */
		ValidateMethodSig(sig) ;
	/* If signature invalid, report now */
		if( sigLast != 'V' )
			JError("Constructors must return void") ;
	/* Otherwise report missing constructor */
		JError("Inexistent constructor %s:%s", ClassName(target), sig) ;
	}
/* Create the new object and return it */
/* Assertion: at this point the field signature is valid */
	PrologToJavaMultiple(sig, X2, args) ;
	obj = JNewObjectA(target, id, args) ;
	if( obj == nil ) CheckException(nil) ;
	MustBe( UnifyWithAtomic(X3, MakeJObj(obj)) ) ;
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
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Boolean, targetIsClass, target, id, val.z) ;
			JumpNext() ;
		}
		case 'B': {
			Pt old = MakeByte(JGetField(Byte, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Byte, targetIsClass, target, id, val.b) ;
			JumpNext() ;
		}
		case 'C': {
			Pt old = MakeChar(JGetField(Char, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Char, targetIsClass, target, id, val.c) ;
			JumpNext() ;
		}
		case 'S': {
			Pt old = MakeInt(JGetField(Short, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Short, targetIsClass, target, id, val.s) ;
			JumpNext() ;
		}
		case 'I': {
			Pt old = MakeInt(JGetField(Int, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Int, targetIsClass, target, id, val.i) ;
			JumpNext() ;
		}
		case 'J': {
			Pt old = MakeLLInt(JGetField(Long, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Long, targetIsClass, target, id, val.j) ;
			JumpNext() ;
		}
		case 'F': {
			Pt old = MakeFloat(JGetField(Float, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Float, targetIsClass, target, id, val.f) ;
			JumpNext() ;
		}
		case 'D': {
			Pt old = MakeFloat(JGetField(Double, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
			JSetField(Double, targetIsClass, target, id, val.d) ;
			JumpNext() ;
		}
		case 'L': case '[': {
			Pt old = MakeJObj(JGetField(Object, targetIsClass, target, id)) ;
			Ensure( UnifyWithAtomic(X2, old) ) ;
			if( Drf(X3) == old ) JumpNext() ;
			val = PrologToJava(sig, &dummySig, X3) ;
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

	val = PrologToJava(elemSig, &dummySig, X4) ;
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
			val = PrologToJava(sig, &dummySig, X2) ;
			MustBe( UnifyWithAtomic(t1, MakeJObj(val.l)) ) ;
		}
		else { /* convert java (X1) -> prolog (X2) */
			val.l = XTestJObj(t1) ;
			MustBe( Unify(X2, JavaToProlog(sig, val, true)) ) ; /* Can be a list */
		}
	}
	else {
		if( IsVar(t1) ) { /* convert prolog (X2) -> java (X1) */
			PrologToJava(sig, &dummySig, X2) ; /* only for validation */
			MustBe( UnifyWithAtomic(t1, X2) ) ;
		}
		else { /* convert java (X1) -> prolog (X2) */
			PrologToJava(sig, &dummySig, t1) ; /* only for validation */
			MustBe( UnifyWithAtomic(X2, t1) ) ;
		}
	}
}

static void PJavaObjInfo()
{
	jobject obj = XTestJObj(X0);
	Ensure( obj != nil ) ;
	jclass cls = JGetObjectClass(obj) ;
	Str name = ClassName(cls) ;
	Pt t = JObjectToAtomic(obj) ;
	MustBe( UnifyWithAtomic(X1, MakeTempAtom(name)) && UnifyWithAtomic(X2, t)  ) ;
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

static Str moreJavaBuiltinsStr = "												\
																				\
java_classpath(P) :-															\
	java_call('java/lang/System',												\
				'getProperty:(Ljava/lang/String;)Ljava/lang/String;',			\
				['java.class.path'], P).										\
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
rjava :- '$$_call_prolog_through_java_2'((X is 1, writeln(X))).					\
																				\
'$$_call_prolog_through_java_2'(G) :-	/* For testing reentrant calls 2 */		\
	atom_termq(A, G),															\
	java_call('prolog/Prolog', 'CallProlog:(Ljava/lang/String;)Z', [A], true).	\
" ;

void JavaInit()
{
	jobjType = ExtraTypeNew("JOBJ", JObjSizeFun, nil, JObjBasicGCDelete, 256) ;
	ExtraTypeDoesNotSupportAliasing(jobjType) ;

	PrepareJErrors("", "") ;
	CreateNullStuff() ;
	InstallCBuiltinPred("jobjs", 0, PJObjs) ;
	InstallCBuiltinPred("java_new", 4, PJavaNew) ;
	InstallCBuiltinPred("java_call", 4, PJavaCall) ;
	InstallCBuiltinPred("java_field", 4, PJavaField) ;
	InstallCBuiltinPred("java_array", 5, PJavaArray) ;
	InstallCBuiltinPred("java_convert", 3, PJavaConvert) ;
	InstallCBuiltinPred("java_obj", 3, PJavaObjInfo) ;
	InstallCBuiltinPred("$$_call_prolog_through_java", 1, PCallPrologThroughJava) ;
	InstallCBuiltinPred("$$_call_prolog_through_java_serialized",
								1, PCallPrologThroughJavaSerialized) ;
	InstallCBuiltinPred("java_compile", 1, PJavaCompile) ;
	InstallCBuiltinPred("java_clean", 0, PJavaClean0) ;
	InstallCBuiltinPred("java_clean", 1, PJavaClean1) ;
	ZBasicLoadBuiltinsStr(moreJavaBuiltinsStr) ;

	RegisterForeignInterface("Java", " j") ;
}


/* Stuff for starting from the Java side */

JNIEXPORT void JNICALL Java_prolog_Prolog_touch(JNIEnv *env, jclass class) {
	Unused(env);
	Unused(class);	
	// Nothing
}

JNIEXPORT void JNICALL Java_prolog_Prolog_hello(JNIEnv *env, jclass class) {
	Unused(env);
	Unused(class);	
	Mesg("Hello, from the CxProlog dynamic library!!!");
}

// Setup Prolog called form the Java side
JNIEXPORT void JNICALL Java_prolog_Prolog_SetupProlog(JNIEnv *envDyn, jclass class) {
	Unused(class);
	static Bool started = false ;
	if( started  ) return ;
	started = true ;
	env = envDyn;
	(*env)->GetJavaVM(env, &jvm);
	StartProlog(0, nil, nil);
	JavaStart();
}

#else /* USE_JAVA */

void JavaInit() { /* Nothing */ }

#endif /* USE_JAVA */

/*
gcc  -I/usr/java/j2/include:/usr/java/j2/include/linux -L/usr/java/j2/jre/lib/i386/client/ -ljvm Java.c -o Java
LD_LIBRARY_PATH=/usr/java/j2/jre/lib/i386/client/:/usr/java/j2/jre/lib/i386/ Java
*/
