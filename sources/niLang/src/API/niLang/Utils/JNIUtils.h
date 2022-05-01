#ifndef __JNIUTILS_H_11BF51A8_6017_486F_BBDC_026439B09444__
#define __JNIUTILS_H_11BF51A8_6017_486F_BBDC_026439B09444__

#include "JNIHeaders.h"
#include <stdio.h>

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_JNI
 * @{
 */

#ifdef niJNI

#if defined _DEBUG && (HAS_EXCEPTIONS == 1)
#define NI_JVM_USE_EXCEPTIONS
#endif

enum eJVMStdClass
{
  eJVMStdClass_String,
  eJVMStdClass_Byte,
  eJVMStdClass_Short,
  eJVMStdClass_Integer,
  eJVMStdClass_Long,
  eJVMStdClass_Float,
  eJVMStdClass_Double,
  eJVMStdClass_Number,
  eJVMStdClass_Boolean,
  eJVMStdClass_RuntimeException,
  eJVMStdClass_NullPointerException,
  eJVMStdClass_Loader,
  eJVMStdClass_Pointer,
  eJVMStdClass_RefCountedPointer,
  eJVMStdClass_IUnknown,
  eJVMStdClass_UUID,
  eJVMStdClass_Vec2i,
  eJVMStdClass_Vec3i,
  eJVMStdClass_Vec4i,
  eJVMStdClass_Vec2f,
  eJVMStdClass_Vec3f,
  eJVMStdClass_Vec4f,
  eJVMStdClass_Matrixf,
  eJVMStdClass_Color3ub,
  eJVMStdClass_Color4ub,
  eJVMStdClass_NIOBuffer,
  eJVMStdClass_NIOByteBuffer,
  eJVMStdClass_NIOIntBuffer,
  eJVMStdClass_NIOCharBuffer,
  eJVMStdClass_NIODoubleBuffer,
  eJVMStdClass_NIOFloatBuffer,
  eJVMStdClass_NIOLongBuffer,
  eJVMStdClass_NIOShortBuffer,
  //! \internal
  eJVMStdClass_Last
};

#define niJVM_Class(E,CLASS)                    \
  niJVM_GetStdClass((E),eJVMStdClass_##CLASS)

#define niJVM_InstanceOf(E,O,CLASS)                                   \
  (E)->IsInstanceOf((O),niJVM_GetStdClass((E),eJVMStdClass_##CLASS))

#define niJVM_CallStaticObject(E,CLASS,METH,ARGS)                       \
  (E)->CallStaticObjectMethodA(niJVM_GetStdClass((E),eJVMStdClass_##CLASS),niJVM_##CLASS##_static_##METH,ARGS)

#define niJVM_CallMethod(E,O,TYPE,CLASS,METH,ARGS)          \
  (E)->Call##TYPE##MethodA((O),niJVM_##CLASS##_##METH,ARGS)

#define niJVM_CheckException(E,RET)             \
  if ((E)->ExceptionCheck()) {                  \
    (E)->ExceptionDescribe();                   \
    niError("Unknown JVM Exception.");         \
    return RET;                                 \
  }

#define niJVM_GetIUnknown(E,O,TYPE)                                     \
  ((O)==NULL)?(TYPE*)NULL:(TYPE*)jlong_to_ptr((E)->GetLongField((O), _JVM->refCountedObjPtrFieldID))

#define NI_JVM_XPRIM(TYPE,NAME)                                         \
  niExportFunc(void) niJVM_Set##NAME(JNIEnv* e, jobject o, const TYPE& v); \
  niExportFunc(TYPE) niJVM_Get##NAME(JNIEnv* e, jobject o);             \
  __forceinline jobject niJVM_New##NAME(JNIEnv* e, const TYPE& v) {     \
    jobject o = e->AllocObject(niJVM_Class(e,NAME));                    \
    niJVM_Set##NAME(e,o,v);                                             \
    return o;                                                           \
  }

#define NI_JVM_WRAPPER_TRY_GET_EXISTING(NS,INTF,E,O)

#define NI_JVM_WRAPPER_REGISTER(E,O,THIS)
#define NI_JVM_WRAPPER_UNREGISTER(E,O,THIS)

#define NI_JVM_WRAPPER_METH_ENTER(NS,INTF,METH,JMETH,SIG,RET) \
  if (mObject == (jobject)NULL) {                             \
    niError("Calling invalid object.");                      \
    return RET;                                               \
  }                                                           \
  JNIEnv* e = niJVM_GetCurrentEnv();                          \
  niAssert(e != NULL);                                        \
  if (!mm##METH) {                                            \
    if (e) {                                                  \
      mm##METH = e->GetMethodID(mClass, #JMETH, SIG);         \
      if (mm##METH == NULL)                                   \
        mm##METH = e->GetMethodID(mClass, #METH, SIG);        \
    }                                                         \
    if (mm##METH == NULL || e->ExceptionCheck()) {            \
      niError("Can't get method ID.");                       \
      return RET;                                             \
    }                                                         \
  }

#define NI_JVM_WRAPPER_METH_LEAVE(NS,INTF,METH,JMETH,RET) \
  if (e->ExceptionCheck()) {                              \
    niError("Exception in method call.");                \
    return RET;                                           \
  }

struct sJVMContext {
  JavaVM*   vm;
  jint      ver;
  jmethodID initMethodID;
  jfieldID  addressFieldID;
  jfieldID  positionFieldID;
  jmethodID refCountedConstructorMethodID;
  jfieldID  refCountedObjPtrFieldID;
};

niExportFunc(ni::tBool) niJVM_HasVM();
niExportFunc(jclass)  niJVM_GetStdClass(JNIEnv *e, eJVMStdClass i);
niExportFunc(JNIEnv*) niJVM_GetCurrentEnv();
niExportFunc(void) niJVM_Service(JNIEnv* apEnv);

niExportFunc(jint)    niJVM_PutMemberOffset(JNIEnv* e, const char* className, const char* member, int offset);

niExportFunc(sJVMContext*) niJVM_GetContext();
niExportFunc(jint)    niJVM_OnLoad(JavaVM* vm, void** appEnv, jint aJNIVer);
niExportFunc(jint)    niJVM_OnUnload(JavaVM* vm, void** appEnv, jint aJNIVer);
niExportFunc(void)    niJVM_JString2String(JNIEnv* apEnv, ni::cString& astrOut, jstring aJString);
niExportFunc(jobject)  niJVM_IUnknownNew(JNIEnv* apEnv, jclass aClass, const ni::tUUID* apIID, const ni::iUnknown* apIUnknown);
niExportFunc(jint)    niJVM_IUnknownDispose(JNIEnv* apEnv, ni::tIntPtr anIID, ni::tIntPtr anIUnknown);

niExportFunc(void)    niJVM_ThrowRuntime(JNIEnv *e, const ni::achar* aaszMsg);
niExportFunc(void)    niJVM_ThrowNullPtr(JNIEnv *e, const ni::achar* aaszMsg);

niExportFunc(jobject) niJVM_NewVar(JNIEnv* e, const ni::Var& v);
niExportFuncCPP(ni::Var) niJVM_GetVar(JNIEnv* e, jobject o);

NI_JVM_XPRIM(ni::tUUID,UUID);
NI_JVM_XPRIM(ni::sVec2i,Vec2i);
NI_JVM_XPRIM(ni::sVec3i,Vec3i);
NI_JVM_XPRIM(ni::sVec4i,Vec4i);
NI_JVM_XPRIM(ni::sVec2f,Vec2f);
NI_JVM_XPRIM(ni::sVec3f,Vec3f);
NI_JVM_XPRIM(ni::sVec4f,Vec4f);
NI_JVM_XPRIM(ni::sMatrixf,Matrixf);
NI_JVM_XPRIM(ni::sColor3ub,Color3ub);
NI_JVM_XPRIM(ni::sColor4ub,Color4ub);

niExportFunc(jstring) niJVM_EmptyString(JNIEnv* e);

__forceinline jstring niJVM_NewString(JNIEnv* e, const char* aChars) {
  return niStringIsOK(aChars) ? e->NewStringUTF(aChars) : niJVM_EmptyString(e);
}
__forceinline jstring niJVM_NewString(JNIEnv* e, const ni::cString& str) {
  return niJVM_NewString(e, str.Chars());
}
__forceinline jstring niJVM_NewString(JNIEnv* e, const ni::cString* str) {
  return niJVM_NewString(e, str ? str->Chars() : NULL);
}

/**
 * Set a property using java.lang.System.setProperty, ret contains the previous value of the property
 * @return eFalse a Java exception was thrown
 */
niExportFunc(ni::tBool) niJVM_SetProperty(JNIEnv* e, ni::cString& ret, const ni::achar* aProperty, const ni::achar* aaszValue);
/**
 * Get a property using java.lang.System.getProperty, ret contains the current value of the property
 * @return eFalse if the property can't be found or a Java exception was thrown
 */
niExportFunc(ni::tBool) niJVM_GetProperty(JNIEnv* e, ni::cString& ret, const ni::achar* aProperty);

niExportFunc(const ni::achar*) niJVM_GetWrapperRetChars(JNIEnv* e, jstring aStr);

template<typename T>
struct LocalJRef {
  JNIEnv* e;
  T o;
  LocalJRef(JNIEnv* aE, T aO) : e(aE), o(aO) {
  }
  ~LocalJRef() {
    if (o) {
      e->DeleteLocalRef(o);
    }
  }
};

struct LocalJString : public LocalJRef<jstring> {
  LocalJString(JNIEnv* e, const ni::achar* s) :
      LocalJRef<jstring>(e,(s) ? e->NewStringUTF(s) : NULL)
  {
  }
};

niExportFunc(void*) niJVM_LockNIOBuffer(JNIEnv* e, jobject jBuffer, jarray* outBufferArray);
niExportFunc(void) niJVM_UnlockNIOBuffer(JNIEnv* e, jobject jBuffer, void* dataPtr, jarray jBufferArray);

struct LockJBuffer {
  JNIEnv* e;
  jobject buf;
  jarray  bufArray;
  void*   dataPtr;
  LockJBuffer(JNIEnv* aE, jobject jBuffer) : e(aE), buf(jBuffer), bufArray(NULL) {
    dataPtr = niJVM_LockNIOBuffer(e,buf,&bufArray);
  }
  ~LockJBuffer() {
    niJVM_UnlockNIOBuffer(e,buf,dataPtr,bufArray);
  }
};

static inline int niJVM_CheckNull(JNIEnv *e, uintptr_t pointer) {
  if (pointer == 0) {
    niJVM_ThrowNullPtr(e,"Pointer address is NULL.");
    return 1;
  }
  return 0;
}

niExportFunc(void) niJVM_HandleException(JNIEnv *e);

template <typename T>
T* niJVM_QueryInterface(const ni::Var& v) {
  return ni::VarQueryInterface<T>(v);
}

template <typename T>
T* niJVM_QueryInterface(const ni::iUnknown* v) {
  return ni::QueryInterface<T>(v);
}

#ifdef NI_JVM_USE_EXCEPTIONS
#define NI_JVM_CPP_TRY try
#define NI_JVM_CPP_CATCH_AND_HANDLE_EXCEPTION(e) catch (...) { niJVM_HandleException(e); }
#else
#define NI_JVM_CPP_TRY
#define NI_JVM_CPP_CATCH_AND_HANDLE_EXCEPTION(e)
#endif

#define jlong_to_ptr(a) ((void*)(uintptr_t)(a))
#define ptr_to_jlong(a) ((jlong)(uintptr_t)(a))

///// Autogenerated in _niLang_JNIBase.cpp /////
niExportFunc(jint) niJVM_OnLoad_Base(JNIEnv* e);
niExportFunc(void) niJVM_OnUnload_Base(JNIEnv* e);
JNIEXPORT jint JNICALL Java_ni_types_RefCountedPointer_IUnknownDispose(JNIEnv *e, jclass c, jlong p0, jlong p1);
JNIEXPORT void JNICALL Java_ni_types_Pointer_00024ReferenceDeallocator_deallocate(JNIEnv *e, jobject o, jlong p0, jlong p1);
JNIEXPORT jobject JNICALL Java_ni_types_Pointer_asDirectBuffer(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jbyte JNICALL Java_ni_types_BytePointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_BytePointer_put(JNIEnv *e, jobject o, jbyte p0);
JNIEXPORT void JNICALL Java_ni_types_BytePointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jshort JNICALL Java_ni_types_ShortPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_ShortPointer_put(JNIEnv *e, jobject o, jshort p0);
JNIEXPORT void JNICALL Java_ni_types_ShortPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jint JNICALL Java_ni_types_IntPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_IntPointer_put(JNIEnv *e, jobject o, jint p0);
JNIEXPORT void JNICALL Java_ni_types_IntPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jlong JNICALL Java_ni_types_LongPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_LongPointer_put(JNIEnv *e, jobject o, jlong p0);
JNIEXPORT void JNICALL Java_ni_types_LongPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jfloat JNICALL Java_ni_types_FloatPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_FloatPointer_put(JNIEnv *e, jobject o, jfloat p0);
JNIEXPORT void JNICALL Java_ni_types_FloatPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jdouble JNICALL Java_ni_types_DoublePointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_DoublePointer_put(JNIEnv *e, jobject o, jdouble p0);
JNIEXPORT void JNICALL Java_ni_types_DoublePointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jchar JNICALL Java_ni_types_CharPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_CharPointer_put(JNIEnv *e, jobject o, jchar p0);
JNIEXPORT void JNICALL Java_ni_types_CharPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jobject JNICALL Java_ni_types_PointerPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_PointerPointer_put(JNIEnv *e, jobject o, jobject p0);
JNIEXPORT void JNICALL Java_ni_types_PointerPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jlong JNICALL Java_ni_types_CLongPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_CLongPointer_put(JNIEnv *e, jobject o, jlong p0);
JNIEXPORT void JNICALL Java_ni_types_CLongPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jlong JNICALL Java_ni_types_SizeTPointer_get(JNIEnv *e, jobject o);
JNIEXPORT jobject JNICALL Java_ni_types_SizeTPointer_put(JNIEnv *e, jobject o, jlong p0);
JNIEXPORT void JNICALL Java_ni_types_SizeTPointer_allocateArray(JNIEnv *e, jobject o, jint p0);
JNIEXPORT jobject JNICALL Java_ni_types_IUnknown_query__Ljava_lang_Object_2(JNIEnv *e, jclass c, jobject p0);
JNIEXPORT jobject JNICALL Java_ni_types_IUnknown_query__Lni_types_IUnknown_2(JNIEnv *e, jclass c, jobject p0);
JNIEXPORT jint JNICALL Java_ni_types_IUnknown_getNumRefs(JNIEnv *e, jobject o);
JNIEXPORT void JNICALL Java_ni_types_IUnknown_invalidate(JNIEnv *e, jobject o);

#endif // #ifdef niJNI

/**@}*/
/**@}*/
#endif // __JNIUTILS_H_11BF51A8_6017_486F_BBDC_026439B09444__
