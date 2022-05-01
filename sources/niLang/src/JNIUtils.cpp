#include "API/niLang/Types.h"

#if !defined niJNI
#pragma message("# JNI: NOT Compiling")
#endif // niJNI

#ifdef niJNI
#pragma message("# JNI: Compiling")

#include "API/niLang/StringDef.h"
#include "API/niLang/Utils/JNIUtils.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/STL/hash_map.h"
#include "API/niLang/STL/utils.h"

using namespace ni;

#ifdef _DEBUG
#define REF_LOG(X) //niDebugFmt(X)
#define DEBUG_LOG(X) niDebugFmt(X)
#else
#define REF_LOG(X) //niDebugFmt(X)
#define DEBUG_LOG(X) //niDebugFmt(X)
#endif

static sJVMContext* _JVM = NULL;
static JavaVM*   niJVM_vm = NULL;
static jint      niJVM_ver = NULL;
static jmethodID niJVM_putMemberOffsetMethodID = NULL;
static jmethodID niJVM_initMethodID = NULL;
static jfieldID  niJVM_addressFieldID = NULL;
static jfieldID  niJVM_positionFieldID = NULL;
static jmethodID niJVM_refCountedConstructorMethodID = NULL;
static jfieldID  niJVM_refCountedObjPtrFieldID = NULL;
static jmethodID niJVM_refCounted_cache = NULL;
static jmethodID niJVM_refCounted_obtain = NULL;
static jmethodID niJVM_setProperty = NULL;
static jmethodID niJVM_getProperty = NULL;
static jmethodID niJVM_nioByteBuffer_array = NULL;
static jmethodID niJVM_nioIntBuffer_array = NULL;
static jmethodID niJVM_nioCharBuffer_array = NULL;
static jmethodID niJVM_nioDoubleBuffer_array = NULL;
static jmethodID niJVM_nioFloatBuffer_array = NULL;
static jmethodID niJVM_nioLongBuffer_array = NULL;
static jmethodID niJVM_nioShortBuffer_array = NULL;

jmethodID niJVM_Number_byteValue = NULL;
jmethodID niJVM_Number_shortValue = NULL;
jmethodID niJVM_Number_intValue = NULL;
jmethodID niJVM_Number_longValue = NULL;
jmethodID niJVM_Number_floatValue = NULL;
jmethodID niJVM_Number_doubleValue = NULL;
jmethodID niJVM_Byte_static_valueOf = NULL;
jmethodID niJVM_Short_static_valueOf = NULL;
jmethodID niJVM_Integer_static_valueOf = NULL;
jmethodID niJVM_Long_static_valueOf = NULL;
jmethodID niJVM_Float_static_valueOf = NULL;
jmethodID niJVM_Double_static_valueOf = NULL;

jmethodID niJVM_Boolean_booleanValue = NULL;
jmethodID niJVM_Boolean_static_valueOf = NULL;

jfieldID niJVM_UUID_DataHi = NULL;
jfieldID niJVM_UUID_DataLo = NULL;

jfieldID niJVM_Vec2i_x = NULL;
jfieldID niJVM_Vec2i_y = NULL;
jfieldID niJVM_Vec3i_x = NULL;
jfieldID niJVM_Vec3i_y = NULL;
jfieldID niJVM_Vec3i_z = NULL;
jfieldID niJVM_Vec4i_x = NULL;
jfieldID niJVM_Vec4i_y = NULL;
jfieldID niJVM_Vec4i_z = NULL;
jfieldID niJVM_Vec4i_w = NULL;
jfieldID niJVM_Vec2f_x = NULL;
jfieldID niJVM_Vec2f_y = NULL;
jfieldID niJVM_Vec3f_x = NULL;
jfieldID niJVM_Vec3f_y = NULL;
jfieldID niJVM_Vec3f_z = NULL;
jfieldID niJVM_Vec4f_x = NULL;
jfieldID niJVM_Vec4f_y = NULL;
jfieldID niJVM_Vec4f_z = NULL;
jfieldID niJVM_Vec4f_w = NULL;
jfieldID niJVM_Matrixf_11 = NULL;
jfieldID niJVM_Matrixf_12 = NULL;
jfieldID niJVM_Matrixf_13 = NULL;
jfieldID niJVM_Matrixf_14 = NULL;
jfieldID niJVM_Matrixf_21 = NULL;
jfieldID niJVM_Matrixf_22 = NULL;
jfieldID niJVM_Matrixf_23 = NULL;
jfieldID niJVM_Matrixf_24 = NULL;
jfieldID niJVM_Matrixf_31 = NULL;
jfieldID niJVM_Matrixf_32 = NULL;
jfieldID niJVM_Matrixf_33 = NULL;
jfieldID niJVM_Matrixf_34 = NULL;
jfieldID niJVM_Matrixf_41 = NULL;
jfieldID niJVM_Matrixf_42 = NULL;
jfieldID niJVM_Matrixf_43 = NULL;
jfieldID niJVM_Matrixf_44 = NULL;
jfieldID niJVM_Color3ub_x = NULL;
jfieldID niJVM_Color3ub_y = NULL;
jfieldID niJVM_Color3ub_z = NULL;
jfieldID niJVM_Color4ub_x = NULL;
jfieldID niJVM_Color4ub_y = NULL;
jfieldID niJVM_Color4ub_z = NULL;
jfieldID niJVM_Color4ub_w = NULL;

static const char *niJVM_classNames[] = {
  "java/lang/String",
  "java/lang/Byte",
  "java/lang/Short",
  "java/lang/Integer",
  "java/lang/Long",
  "java/lang/Float",
  "java/lang/Double",
  "java/lang/Number",
  "java/lang/Boolean",
  "java/lang/RuntimeException",
  "java/lang/NullPointerException",
  "ni/types/util/Loader",
  "ni/types/Pointer",
  "ni/types/RefCountedPointer",
  "ni/types/IUnknown",
  "ni/types/UUID",
  "ni/types/Vec2i",
  "ni/types/Vec3i",
  "ni/types/Vec4i",
  "ni/types/Vec2f",
  "ni/types/Vec3f",
  "ni/types/Vec4f",
  "ni/types/Matrixf",
  "ni/types/Color3ub",
  "ni/types/Color4ub",
  "java/nio/Buffer",
  "java/nio/ByteBuffer",
  "java/nio/IntBuffer",
  "java/nio/CharBuffer",
  "java/nio/DoubleBuffer",
  "java/nio/FloatBuffer",
  "java/nio/LongBuffer",
  "java/nio/ShortBuffer",
};
niCAssert(niCountOf(niJVM_classNames) == eJVMStdClass_Last);
static jclass    niJVM_classes[eJVMStdClass_Last] = { NULL };
niCAssert(niCountOf(niJVM_classes) == eJVMStdClass_Last);

niExportFunc(jclass) niJVM_GetStdClass(JNIEnv *e, eJVMStdClass i) {
  if (niJVM_classes[i] == NULL) {
    jclass c = e->FindClass(niJVM_classNames[i]);
    if (c == NULL || e->ExceptionCheck()) {
      niError(niFmt("Error loading class %s.", niJVM_classNames[i]));
      return NULL;
    }
    niJVM_classes[i] = (jclass)e->NewGlobalRef(c);
    if (niJVM_classes[i] == NULL || e->ExceptionCheck()) {
      niError(niFmt("Error creating global reference of class %s.", niJVM_classNames[i]));
      return NULL;
    }
    DEBUG_LOG(("# Loaded JVM class %s.", niJVM_classNames[i]));
  }
  return niJVM_classes[i];
}

niExportFunc(void)    niJVM_ThrowRuntime(JNIEnv *e, const achar* aaszMsg) {
  e->ThrowNew(niJVM_Class(e,RuntimeException), aaszMsg);
}
niExportFunc(void)    niJVM_ThrowNullPtr(JNIEnv *e, const achar* aaszMsg) {
  e->ThrowNew(niJVM_Class(e,NullPointerException), aaszMsg);
}

static int _loadCount = 0;

#define LOAD_STATIC_ID(CLASS,VARNAME,NAME,TYPE)                         \
  DEBUG_LOG(("### LOAD_STATIC_ID(%s,%s,%s,%s)\n",#CLASS,#VARNAME,#NAME,TYPE)); \
  niJVM_##VARNAME = e->GetStaticMethodID(niJVM_GetStdClass(e,eJVMStdClass_##CLASS), NAME, TYPE); \
  if (niJVM_##VARNAME == NULL || e->ExceptionCheck()) {                 \
    DEBUG_LOG(("Error getting '" NAME "' static method ID of " #CLASS " class.")); \
    return 0;                                                           \
  }

#define LOAD_FUN_ID(CLASS,VARNAME,NAME,TYPE)                            \
  DEBUG_LOG(("### LOAD_FUN_ID(%s,%s,%s,%s)\n",#CLASS,#VARNAME,#NAME,TYPE)); \
  niJVM_##VARNAME = e->GetMethodID(niJVM_GetStdClass(e,eJVMStdClass_##CLASS), NAME, TYPE); \
  if (niJVM_##VARNAME == NULL || e->ExceptionCheck()) {                 \
    DEBUG_LOG(("Error getting '" NAME "' method ID of " #CLASS " class.")); \
    return 0;                                                           \
  }

#define LOAD_VAR_ID(CLASS,VARNAME,NAME,TYPE)                            \
  DEBUG_LOG(("### LOAD_VAR_ID(%s,%s,%s,%s)\n",#CLASS,#VARNAME,#NAME,TYPE)); \
  niJVM_##VARNAME = e->GetFieldID(niJVM_GetStdClass(e,eJVMStdClass_##CLASS), NAME, TYPE); \
  if (niJVM_##VARNAME == NULL || e->ExceptionCheck()) {                 \
    DEBUG_LOG(("Error getting '" NAME "' field ID of " #CLASS " class.")); \
    return 0;                                                           \
  }

static sJVMContext _jvmContext;
niExportFunc(sJVMContext*) niJVM_GetContext() {
  return &_jvmContext;
}

niExportFunc(jint) niJVM_OnLoad(JavaVM* vm, void** appEnv, jint aJNIVer) {
  if (vm->GetEnv(appEnv, aJNIVer) != JNI_OK) {
    DEBUG_LOG(("Could not get JNIEnv for JNI_VERSION_1_4 inside JNI_OnLoad())."));
    return 0;
  }

  niJVM_vm = vm;
  niJVM_ver = aJNIVer;
  JNIEnv* e = (JNIEnv*)*appEnv;

  if (_loadCount == 0) {
    // Loader
    LOAD_STATIC_ID(Loader, putMemberOffsetMethodID, "putMemberOffset", "(Ljava/lang/String;Ljava/lang/String;I)V");
    LOAD_STATIC_ID(Loader, setProperty, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    LOAD_STATIC_ID(Loader, getProperty, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");

    // NIO Buffer
    // REM: Not using java.nio.Buffer.array (the base abstract
    //      class) because Android 2.2 doesn't support it, also
    //      perf of array access that way is crap anyway... should
    //      use Direct Buffer for perf cirtical things.
    LOAD_FUN_ID(NIOByteBuffer, nioByteBuffer_array, "array", "()[B");
    LOAD_FUN_ID(NIOIntBuffer, nioIntBuffer_array, "array", "()[I");
    LOAD_FUN_ID(NIOCharBuffer, nioCharBuffer_array, "array", "()[C");
    LOAD_FUN_ID(NIODoubleBuffer, nioDoubleBuffer_array, "array", "()[D");
    LOAD_FUN_ID(NIOFloatBuffer, nioFloatBuffer_array, "array", "()[F");
    LOAD_FUN_ID(NIOLongBuffer, nioLongBuffer_array, "array", "()[J");
    LOAD_FUN_ID(NIOShortBuffer, nioShortBuffer_array, "array", "()[S");

    // Pointer
    LOAD_FUN_ID(Pointer, initMethodID,    "init",     "(JJ)V");
    LOAD_VAR_ID(Pointer, addressFieldID,  "address",  "J");
    LOAD_VAR_ID(Pointer, positionFieldID, "position", "I");

    // RefCountedPointer
    LOAD_FUN_ID(RefCountedPointer, refCountedConstructorMethodID, "<init>", "(JJ)V");
    LOAD_VAR_ID(RefCountedPointer, refCountedObjPtrFieldID,       "objPtr", "J");
    LOAD_STATIC_ID(RefCountedPointer, refCounted_cache, "cache", "(JJLjava/lang/Object;)V");
    LOAD_STATIC_ID(RefCountedPointer, refCounted_obtain, "obtain", "(JJ)Ljava/lang/Object;");

    // Number
    LOAD_FUN_ID(Number, Number_byteValue,   "byteValue",   "()B");
    LOAD_FUN_ID(Number, Number_shortValue,  "shortValue",  "()S");
    LOAD_FUN_ID(Number, Number_intValue,    "intValue",    "()I");
    LOAD_FUN_ID(Number, Number_longValue,   "longValue",   "()J");
    LOAD_FUN_ID(Number, Number_floatValue,  "floatValue",  "()F");
    LOAD_FUN_ID(Number, Number_doubleValue, "doubleValue", "()D");

    LOAD_STATIC_ID(Byte,    Byte_static_valueOf,    "valueOf", "(B)Ljava/lang/Byte;");
    LOAD_STATIC_ID(Short,   Short_static_valueOf,   "valueOf", "(S)Ljava/lang/Short;");
    LOAD_STATIC_ID(Integer, Integer_static_valueOf, "valueOf", "(I)Ljava/lang/Integer;");
    LOAD_STATIC_ID(Long,    Long_static_valueOf,    "valueOf", "(J)Ljava/lang/Long;");
    LOAD_STATIC_ID(Float,   Float_static_valueOf,   "valueOf", "(F)Ljava/lang/Float;");
    LOAD_STATIC_ID(Double,  Double_static_valueOf,  "valueOf", "(D)Ljava/lang/Double;");

    // Boolean
    LOAD_FUN_ID   (Boolean, Boolean_booleanValue,   "booleanValue", "()Z");
    LOAD_STATIC_ID(Boolean, Boolean_static_valueOf, "valueOf",      "(Z)Ljava/lang/Boolean;");

    // UUID
    LOAD_VAR_ID(UUID, UUID_DataHi, "dataHi", "J");
    LOAD_VAR_ID(UUID, UUID_DataLo, "dataLo", "J");

    // Vec2i
    LOAD_VAR_ID(Vec2i, Vec2i_x, "x", "I");
    LOAD_VAR_ID(Vec2i, Vec2i_y, "y", "I");

    // Vec3i
    LOAD_VAR_ID(Vec3i, Vec3i_x, "x", "I");
    LOAD_VAR_ID(Vec3i, Vec3i_y, "y", "I");
    LOAD_VAR_ID(Vec3i, Vec3i_z, "z", "I");

    // Vec4i
    LOAD_VAR_ID(Vec4i, Vec4i_x, "x", "I");
    LOAD_VAR_ID(Vec4i, Vec4i_y, "y", "I");
    LOAD_VAR_ID(Vec4i, Vec4i_z, "z", "I");
    LOAD_VAR_ID(Vec4i, Vec4i_w, "w", "I");

    // Vec2f
    LOAD_VAR_ID(Vec2f, Vec2f_x, "x", "F");
    LOAD_VAR_ID(Vec2f, Vec2f_y, "y", "F");

    // Vec3f
    LOAD_VAR_ID(Vec3f, Vec3f_x, "x", "F");
    LOAD_VAR_ID(Vec3f, Vec3f_y, "y", "F");
    LOAD_VAR_ID(Vec3f, Vec3f_z, "z", "F");

    // Vec4f
    LOAD_VAR_ID(Vec4f, Vec4f_x, "x", "F");
    LOAD_VAR_ID(Vec4f, Vec4f_y, "y", "F");
    LOAD_VAR_ID(Vec4f, Vec4f_z, "z", "F");
    LOAD_VAR_ID(Vec4f, Vec4f_w, "w", "F");

    // Matrixf
    LOAD_VAR_ID(Matrixf, Matrixf_11, "_11", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_12, "_12", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_13, "_13", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_14, "_14", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_21, "_21", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_22, "_22", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_23, "_23", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_24, "_24", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_31, "_31", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_32, "_32", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_33, "_33", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_34, "_34", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_41, "_41", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_42, "_42", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_43, "_43", "F");
    LOAD_VAR_ID(Matrixf, Matrixf_44, "_44", "F");

    // Color3ub
    LOAD_VAR_ID(Color3ub, Color3ub_x, "x", "B");
    LOAD_VAR_ID(Color3ub, Color3ub_y, "y", "B");
    LOAD_VAR_ID(Color3ub, Color3ub_z, "z", "B");

    // Color4ub
    LOAD_VAR_ID(Color4ub, Color4ub_x, "x", "B");
    LOAD_VAR_ID(Color4ub, Color4ub_y, "y", "B");
    LOAD_VAR_ID(Color4ub, Color4ub_z, "z", "B");
    LOAD_VAR_ID(Color4ub, Color4ub_w, "w", "B");

    // Initialize the context
    _JVM = &_jvmContext;
    memset(_JVM,0,sizeof(sJVMContext));
    _JVM->vm = niJVM_vm;
    _JVM->ver = niJVM_ver;
    _JVM->initMethodID = niJVM_initMethodID;
    _JVM->addressFieldID = niJVM_addressFieldID;
    _JVM->positionFieldID = niJVM_positionFieldID;
    _JVM->refCountedConstructorMethodID = niJVM_refCountedConstructorMethodID;
    _JVM->refCountedObjPtrFieldID = niJVM_refCountedObjPtrFieldID;

    // Load the base classes
    if (niJVM_OnLoad_Base(e) == 0) {
      DEBUG_LOG(("Error niJVM_OnLoad_Base()) failed."));
      return 0;
    }
  }

  ++_loadCount;
  return e->GetVersion();
}

niExportFunc(jint) niJVM_OnUnload(JavaVM* vm, void** appEnv, jint aJNIVer) {
  if (vm->GetEnv(appEnv, aJNIVer) != JNI_OK) {
    DEBUG_LOG(("Could not get JNIEnv for JNI_VERSION_1_4 inside JNI_OnUnLoad())."));
    return -1;
  }
  if (_loadCount == 0)
    return JNI_OK;
  --_loadCount;
  if (_loadCount != 0)
    return JNI_OK;
  JNIEnv* e = (JNIEnv*)*appEnv;
  niJVM_OnUnload_Base(e);
  niLoop(i,eJVMStdClass_Last) {
    e->DeleteGlobalRef(niJVM_classes[i]);
  }
  return JNI_OK;
}

niExportFunc(void) niJVM_JString2String(JNIEnv* apEnv, ni::cString& astrOut, jstring aJString) {
  jboolean iscopy;
  const char* utfJString = apEnv->GetStringUTFChars(aJString, &iscopy);
  astrOut = utfJString;
  apEnv->ReleaseStringUTFChars(aJString, utfJString);
}

niExportFunc(void) niJVM_SetUUID(JNIEnv* e, jobject o, const tUUID& v) {
  niAssert(niJVM_InstanceOf(e,o,UUID));
  e->SetLongField(o,niJVM_UUID_DataHi,((tUUID64&)v).nDataHi);
  e->SetLongField(o,niJVM_UUID_DataLo,((tUUID64&)v).nDataLo);
}
niExportFunc(tUUID) niJVM_GetUUID(JNIEnv* e, jobject o) {
  tUUID v;
  if (!o) {
    v = kuuidZero;
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,UUID));
    ((tUUID64&)v).nDataHi = e->GetLongField(o,niJVM_UUID_DataHi);
    ((tUUID64&)v).nDataLo = e->GetLongField(o,niJVM_UUID_DataLo);
  }
  return v;
}

niExportFunc(void) niJVM_SetVec2i(JNIEnv* e, jobject o, const ni::sVec2i& v) {
  niAssert(niJVM_InstanceOf(e,o,Vec2i));
  e->SetIntField(o,niJVM_Vec2i_x,v[0]);
  e->SetIntField(o,niJVM_Vec2i_y,v[1]);
}
niExportFunc(ni::sVec2i) niJVM_GetVec2i(JNIEnv* e, jobject o) {
  ni::sVec2i v;
  if (!o) {
    v = ni::sVec2i::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Vec2i));
    v[0] = e->GetIntField(o,niJVM_Vec2i_x);
    v[1] = e->GetIntField(o,niJVM_Vec2i_y);
  }
  return v;
}

niExportFunc(void) niJVM_SetVec3i(JNIEnv* e, jobject o, const ni::sVec3i& v) {
  niAssert(niJVM_InstanceOf(e,o,Vec3i));
  e->SetIntField(o,niJVM_Vec3i_x,v[0]);
  e->SetIntField(o,niJVM_Vec3i_y,v[1]);
  e->SetIntField(o,niJVM_Vec3i_z,v[2]);
}
niExportFunc(ni::sVec3i) niJVM_GetVec3i(JNIEnv* e, jobject o) {
  ni::sVec3i v;
  if (!o) {
    v = ni::sVec3i::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Vec3i));
    v[0] = e->GetIntField(o,niJVM_Vec3i_x);
    v[1] = e->GetIntField(o,niJVM_Vec3i_y);
    v[2] = e->GetIntField(o,niJVM_Vec3i_z);
  }
  return v;
}

niExportFunc(void) niJVM_SetVec4i(JNIEnv* e, jobject o, const ni::sVec4i& v) {
  niAssert(niJVM_InstanceOf(e,o,Vec4i));
  e->SetIntField(o,niJVM_Vec4i_x,v[0]);
  e->SetIntField(o,niJVM_Vec4i_y,v[1]);
  e->SetIntField(o,niJVM_Vec4i_z,v[2]);
  e->SetIntField(o,niJVM_Vec4i_w,v[3]);
}
niExportFunc(ni::sVec4i) niJVM_GetVec4i(JNIEnv* e, jobject o) {
  ni::sVec4i v;
  if (!o) {
    v = ni::sVec4i::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Vec4i));
    v[0] = e->GetIntField(o,niJVM_Vec4i_x);
    v[1] = e->GetIntField(o,niJVM_Vec4i_y);
    v[2] = e->GetIntField(o,niJVM_Vec4i_z);
    v[3] = e->GetIntField(o,niJVM_Vec4i_w);
  }
  return v;
}

niExportFunc(void) niJVM_SetVec2f(JNIEnv* e, jobject o, const ni::sVec2f& v) {
  niAssert(niJVM_InstanceOf(e,o,Vec2f));
  e->SetFloatField(o,niJVM_Vec2f_x,v[0]);
  e->SetFloatField(o,niJVM_Vec2f_y,v[1]);
}
niExportFunc(ni::sVec2f) niJVM_GetVec2f(JNIEnv* e, jobject o) {
  ni::sVec2f v;
  if (!o) {
    v = ni::sVec2f::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Vec2f));
    v[0] = e->GetFloatField(o,niJVM_Vec2f_x);
    v[1] = e->GetFloatField(o,niJVM_Vec2f_y);
  }
  return v;
}

niExportFunc(void) niJVM_SetVec3f(JNIEnv* e, jobject o, const ni::sVec3f& v) {
  niAssert(niJVM_InstanceOf(e,o,Vec3f));
  e->SetFloatField(o,niJVM_Vec3f_x,v[0]);
  e->SetFloatField(o,niJVM_Vec3f_y,v[1]);
  e->SetFloatField(o,niJVM_Vec3f_z,v[2]);
}
niExportFunc(ni::sVec3f) niJVM_GetVec3f(JNIEnv* e, jobject o) {
  ni::sVec3f v;
  if (!o) {
    v = ni::sVec3f::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Vec3f));
    v[0] = e->GetFloatField(o,niJVM_Vec3f_x);
    v[1] = e->GetFloatField(o,niJVM_Vec3f_y);
    v[2] = e->GetFloatField(o,niJVM_Vec3f_z);
  }
  return v;
}

niExportFunc(void) niJVM_SetVec4f(JNIEnv* e, jobject o, const ni::sVec4f& v) {
  niAssert(niJVM_InstanceOf(e,o,Vec4f));
  e->SetFloatField(o,niJVM_Vec4f_x,v[0]);
  e->SetFloatField(o,niJVM_Vec4f_y,v[1]);
  e->SetFloatField(o,niJVM_Vec4f_z,v[2]);
  e->SetFloatField(o,niJVM_Vec4f_w,v[3]);
}
niExportFunc(ni::sVec4f) niJVM_GetVec4f(JNIEnv* e, jobject o) {
  ni::sVec4f v;
  if (!o) {
    v = ni::sVec4f::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Vec4f));
    v[0] = e->GetFloatField(o,niJVM_Vec4f_x);
    v[1] = e->GetFloatField(o,niJVM_Vec4f_y);
    v[2] = e->GetFloatField(o,niJVM_Vec4f_z);
    v[3] = e->GetFloatField(o,niJVM_Vec4f_w);
  }
  return v;
}

niExportFunc(void) niJVM_SetMatrixf(JNIEnv* e, jobject o, const sMatrixf& v) {
  niAssert(niJVM_InstanceOf(e,o,Matrixf));
  e->SetFloatField(o,niJVM_Matrixf_11,v._11);
  e->SetFloatField(o,niJVM_Matrixf_12,v._12);
  e->SetFloatField(o,niJVM_Matrixf_13,v._13);
  e->SetFloatField(o,niJVM_Matrixf_14,v._14);
  e->SetFloatField(o,niJVM_Matrixf_21,v._21);
  e->SetFloatField(o,niJVM_Matrixf_22,v._22);
  e->SetFloatField(o,niJVM_Matrixf_23,v._23);
  e->SetFloatField(o,niJVM_Matrixf_24,v._24);
  e->SetFloatField(o,niJVM_Matrixf_31,v._31);
  e->SetFloatField(o,niJVM_Matrixf_32,v._32);
  e->SetFloatField(o,niJVM_Matrixf_33,v._33);
  e->SetFloatField(o,niJVM_Matrixf_34,v._34);
  e->SetFloatField(o,niJVM_Matrixf_41,v._41);
  e->SetFloatField(o,niJVM_Matrixf_42,v._42);
  e->SetFloatField(o,niJVM_Matrixf_43,v._43);
  e->SetFloatField(o,niJVM_Matrixf_44,v._44);
}
niExportFunc(ni::sMatrixf) niJVM_GetMatrixf(JNIEnv* e, jobject o) {
  ni::sMatrixf v;
  if (!o) {
    v = ni::sMatrixf::Identity();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Matrixf));
    v._11 = e->GetFloatField(o,niJVM_Matrixf_11);
    v._12 = e->GetFloatField(o,niJVM_Matrixf_12);
    v._13 = e->GetFloatField(o,niJVM_Matrixf_13);
    v._14 = e->GetFloatField(o,niJVM_Matrixf_14);
    v._21 = e->GetFloatField(o,niJVM_Matrixf_21);
    v._22 = e->GetFloatField(o,niJVM_Matrixf_22);
    v._23 = e->GetFloatField(o,niJVM_Matrixf_23);
    v._24 = e->GetFloatField(o,niJVM_Matrixf_24);
    v._31 = e->GetFloatField(o,niJVM_Matrixf_31);
    v._32 = e->GetFloatField(o,niJVM_Matrixf_32);
    v._33 = e->GetFloatField(o,niJVM_Matrixf_33);
    v._34 = e->GetFloatField(o,niJVM_Matrixf_34);
    v._41 = e->GetFloatField(o,niJVM_Matrixf_41);
    v._42 = e->GetFloatField(o,niJVM_Matrixf_42);
    v._43 = e->GetFloatField(o,niJVM_Matrixf_43);
    v._44 = e->GetFloatField(o,niJVM_Matrixf_44);
  }
  return v;
}

niExportFunc(void) niJVM_SetColor3ub(JNIEnv* e, jobject o, const ni::sColor3ub& v) {
  niAssert(niJVM_InstanceOf(e,o,Color3ub));
  e->SetByteField(o,niJVM_Color3ub_x,v[0]);
  e->SetByteField(o,niJVM_Color3ub_y,v[1]);
  e->SetByteField(o,niJVM_Color3ub_z,v[2]);
}
niExportFunc(ni::sColor3ub) niJVM_GetColor3ub(JNIEnv* e, jobject o) {
  ni::sColor3ub v;
  if (!o) {
    v = ni::sColor3ub::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Color3ub));
    v[0] = e->GetByteField(o,niJVM_Color3ub_x);
    v[1] = e->GetByteField(o,niJVM_Color3ub_y);
    v[2] = e->GetByteField(o,niJVM_Color3ub_z);
  }
  return v;
}

niExportFunc(void) niJVM_SetColor4ub(JNIEnv* e, jobject o, const ni::sColor4ub& v) {
  niAssert(niJVM_InstanceOf(e,o,Color4ub));
  e->SetByteField(o,niJVM_Color4ub_x,v[0]);
  e->SetByteField(o,niJVM_Color4ub_y,v[1]);
  e->SetByteField(o,niJVM_Color4ub_z,v[2]);
  e->SetByteField(o,niJVM_Color4ub_w,v[3]);
}
niExportFunc(ni::sColor4ub) niJVM_GetColor4ub(JNIEnv* e, jobject o) {
  ni::sColor4ub v;
  if (!o) {
    v = ni::sColor4ub::Zero();
  }
  else {
    niAssert(niJVM_InstanceOf(e,o,Color4ub));
    v[0] = e->GetByteField(o,niJVM_Color4ub_x);
    v[1] = e->GetByteField(o,niJVM_Color4ub_y);
    v[2] = e->GetByteField(o,niJVM_Color4ub_z);
    v[3] = e->GetByteField(o,niJVM_Color4ub_w);
  }
  return v;
}

niExportFuncCPP(ni::Var) niJVM_GetVar(JNIEnv* e, jobject o) {
  ni::Var v;
  if (!o) {
    v.SetNull();
  }
  else {
    if (niJVM_InstanceOf(e,o,IUnknown)) {
      v.SetIUnknownPointer(niJVM_GetIUnknown(e,o,ni::iUnknown));
    }
    else if (niJVM_InstanceOf(e,o,Integer)) {
      v.SetI32(niJVM_CallMethod(e,o,Int,Number,intValue,NULL));
    }
    else if (niJVM_InstanceOf(e,o,Float)) {
      v.SetF32(niJVM_CallMethod(e,o,Float,Number,floatValue,NULL));
    }
    else if (niJVM_InstanceOf(e,o,Vec2f)) {
      v.mType = eType_Vec2f;
      static_cast<sVec2f&>(v.mVec2f) = niJVM_GetVec2f(e,o);
    }
    else if (niJVM_InstanceOf(e,o,Vec3f)) {
      v.mType = eType_Vec3f;
      static_cast<sVec3f&>(v.mVec3f) = niJVM_GetVec3f(e,o);
    }
    else if (niJVM_InstanceOf(e,o,Vec4f)) {
      v.mType = eType_Vec4f;
      static_cast<sVec4f&>(v.mVec4f) = niJVM_GetVec4f(e,o);
    }
    else if (niJVM_InstanceOf(e,o,Vec2i)) {
      v.mType = eType_Vec2i;
      static_cast<sVec2i&>(v.mVec2i) = niJVM_GetVec2i(e,o);
    }
    else if (niJVM_InstanceOf(e,o,Vec3i)) {
      v.mType = eType_Vec3i;
      static_cast<sVec3i&>(v.mVec3i) = niJVM_GetVec3i(e,o);
    }
    else if (niJVM_InstanceOf(e,o,Vec4i)) {
      v.mType = eType_Vec4i;
      static_cast<sVec4i&>(v.mVec4i) = niJVM_GetVec4i(e,o);
    }
    else if (niJVM_InstanceOf(e,o,Matrixf)) {
      v.SetMatrixf(niJVM_GetMatrixf(e,o));
    }
    else if (niJVM_InstanceOf(e,o,Double)) {
      v.SetF64(niJVM_CallMethod(e,o,Double,Number,doubleValue,NULL));
    }
    else if (niJVM_InstanceOf(e,o,Byte)) {
      v.SetI8(niJVM_CallMethod(e,o,Byte,Number,byteValue,NULL));
    }
    else if (niJVM_InstanceOf(e,o,Short)) {
      v.SetI16(niJVM_CallMethod(e,o,Short,Number,shortValue,NULL));
    }
    else if (niJVM_InstanceOf(e,o,Long)) {
      v.SetI64(niJVM_CallMethod(e,o,Long,Number,longValue,NULL));
    }
    else if (niJVM_InstanceOf(e,o,String)) {
      jboolean isCopy;
      const char* utfJString = e->GetStringUTFChars(static_cast<jstring>(o), &isCopy);
      v.SetString(utfJString);
      e->ReleaseStringUTFChars(static_cast<jstring>(o), utfJString);
    }
    else {
      v.SetNull();
      niWarning("Object can't be convert to Variant, returning niVarNull");
    }
  }
  niJVM_CheckException(e,(v=niVarNull,v));
  return v;
}

niExportFunc(jobject) niJVM_NewVar(JNIEnv* e, const ni::Var& v) {
  jvalue args[1];
  switch (niType(v.mType)) {
    case eType_Null:
      return NULL;
    case eType_I8:
    case eType_U8: {
      args[0].b = v.mI8;
      auto ret = niJVM_CallStaticObject(e,Byte,valueOf,args);
      niJVM_CheckException(e, NULL);
      return ret;
    }
    case eType_I16:
    case eType_U16: {
      args[0].s = v.mI16;
      auto ret = niJVM_CallStaticObject(e,Short,valueOf,args);
      niJVM_CheckException(e, NULL);
      return ret;
    }
    case eType_I32:
    case eType_U32: {
      args[0].i = v.mI32;
      auto ret = niJVM_CallStaticObject(e,Integer,valueOf,args);
      niJVM_CheckException(e, NULL);
      return ret;
    }
    case eType_I64:
    case eType_U64: {
      args[0].j = v.mI64;
      auto ret = niJVM_CallStaticObject(e,Long,valueOf,args);
      niJVM_CheckException(e, NULL);
      return ret;
    }
    case eType_F32: {
      args[0].f = v.mF32;
      auto ret = niJVM_CallStaticObject(e,Float,valueOf,args);
      niJVM_CheckException(e, NULL);
      return ret;
    }
    case eType_F64: {
      args[0].d = v.mF64;
      auto ret = niJVM_CallStaticObject(e,Double,valueOf,args);
      niJVM_CheckException(e, NULL);
      return ret;
    }
    case eType_Vec2i: {
      return niJVM_NewVec2i(e,v.mVec2i);
    }
    case eType_Vec3i: {
      return niJVM_NewVec3i(e,v.mVec3i);
    }
    case eType_Vec4i: {
      return niJVM_NewVec4i(e,v.mVec4i);
    }
    case eType_Vec2f: {
      return niJVM_NewVec2f(e,v.mVec2f);
    }
    case eType_Vec3f: {
      return niJVM_NewVec3f(e,v.mVec3f);
    }
    case eType_Vec4f: {
      return niJVM_NewVec4f(e,v.mVec4f);
    }
    case eType_Matrixf: {
      return niJVM_NewMatrixf(e,*v.mpMatrixf);
    }
    case eType_IUnknown: {
      if (!niIsOK(v.mpIUnknown))
        return NULL;
      else
        return niJVM_IUnknownNew(e,niJVM_Class(e,IUnknown),&niGetInterfaceUUID(ni::iUnknown),v.mpIUnknown);
    }
    default:
      if (VarIsString(v)) {
        return niJVM_NewString(e,VarGetString(v));
      }
      else {
        niWarning(niFmt("Variant[%s] can't be convert to Object, returning null",GetTypeString(v.mType)));
        break;
      }
  }
  niJVM_CheckException(e,NULL);
  return NULL;
}

/**
 * This shouldn't be used, it seems that the JNI GlobalWeak references
 * are weak when returned from the JNI, which cause them to be
 * collected during the program's run-time at any point even if the
 * object is a parameter, or a local variable... Seems GlobalWeak is
 * Weak forever...
 *
 * I didn't figure out a way to make them "Strong" reference when
 * returned... (NewLocalRef is not enought it seems).
 *
 * sJvmRefTableObtain is another attempt at the same idea, it uses
 * a Map held by the JVM to do the caching, obviously less efficient
 * but works as it should...
 */
struct sJvmRefTableWeakRef {
  sJvmRefTableWeakRef() {
  }
  ~sJvmRefTableWeakRef() {
  }

  typedef astl::pair<tIntPtr,tIntPtr> tPair;
  struct hash_pair {
    inline size_t operator()(const tPair& aPair) const {
      return (size_t)((tIntPtr)aPair.first+(tIntPtr)aPair.second);
    };
  };
  typedef astl::hash_map<tPair,jweak,hash_pair> tObjectMap;
  tObjectMap _objects;

  jobject New(JNIEnv* apEnv, jclass aClass, const tUUID* apIID, iUnknown* apIUnknown) {
    if (!apIUnknown) return NULL;
    tPair key((tIntPtr)apIID,(tIntPtr)apIUnknown);
    tObjectMap::iterator it = _objects.find(key);
    if (it != _objects.end()) {
      REF_LOG(("sJvmRefTable::New(%p,%p,%p) - Found: %p [numRefs:%d]\n",(void*)apIID,(void*)apIUnknown,(void*)aClass,(void*)it->second,apIUnknown->GetNumRefs()));

      jobject localRef = apEnv->NewLocalRef(it->second);
      if (apEnv->IsSameObject(localRef,NULL)) {
        DEBUG_LOG(("W/ --> Was Null <--"));
      }
      else {
        return localRef;
      }
    }

    // Couldn't find the object or it was null
    apIUnknown->AddRef();
    jobject objectRef = apEnv->NewObject(
        aClass, niJVM_refCountedConstructorMethodID,
        (jlong)apIID, (jlong)apIUnknown);
    REF_LOG(("sJvmRefTable::New(%p,%p,%p) - New: %p [numRefs:%d]\n",
             (void*)apIID,(void*)apIUnknown,(void*)aClass,(void*)objectRef,apIUnknown->GetNumRefs()));
    jweak weakRef = apEnv->NewWeakGlobalRef(objectRef);
    astl::upsert(_objects,key,weakRef);
    return objectRef;
  }
  jint Dispose(JNIEnv* apEnv, tIntPtr anIID, tIntPtr anIUnknown) {
    niAssert(anIUnknown != 0);
    tPair key(anIID,anIUnknown);
    tObjectMap::iterator it = _objects.find(key);
    if (it == _objects.end()) {
      DEBUG_LOG(("sJvmRefTable::Dispose(%p,%p) - Not Found !\n",anIID,anIUnknown));
      return ~0;
    }
    else {
      iUnknown* pIUnknown = ((ni::iUnknown*)anIUnknown);
      REF_LOG(("sJvmRefTable::Dispose(%p,%p) - Release: %p [numRefs:%d]\n",
               anIID,anIUnknown,(void*)it->second,pIUnknown->GetNumRefs()));
      apEnv->DeleteWeakGlobalRef(it->second);
      _objects.erase(it);
      return pIUnknown->Release();
    }
  }
};

struct sJvmRefTableNew {
  sJvmRefTableNew() {
  }
  ~sJvmRefTableNew() {
  }

  jobject New(JNIEnv* apEnv, jclass aClass, const tUUID* apIID, iUnknown* apIUnknown) {
    if (!apIUnknown) return NULL;
    REF_LOG(("sJvmRefTable::New(%p,%p,%p) [numRefs:%d]\n",(void*)apIID,(void*)apIUnknown,(void*)aClass,apIUnknown->GetNumRefs()));
    apIUnknown->AddRef();
    jobject objectRef = apEnv->NewObject(aClass, niJVM_refCountedConstructorMethodID, (jlong)apIID, (jlong)apIUnknown);
    return objectRef;
  }

  jint Dispose(JNIEnv* apEnv, tIntPtr anIID, tIntPtr anIUnknown) {
    niAssert(anIUnknown != 0);
    iUnknown* pIUnknown = ((ni::iUnknown*)anIUnknown);
    REF_LOG(("sJvmRefTable::Dispose(%p,%p) [numRefs:%d]\n",
             anIID,anIUnknown,pIUnknown->GetNumRefs()));
    return pIUnknown->Release();
  }
};

struct sJvmRefTableObtain {
  sJvmRefTableObtain() {
  }
  ~sJvmRefTableObtain() {
  }

  jobject New(JNIEnv* apEnv, jclass aClass, const tUUID* apIID, iUnknown* apIUnknown) {
    if (!apIUnknown) return NULL;
    jobject objectRef;

    jvalue args[3];
    args[0].j = (jlong)apIID;
    args[1].j = (jlong)apIUnknown;

    objectRef = apEnv->CallStaticObjectMethodA(
        niJVM_Class(apEnv,RefCountedPointer),
        niJVM_refCounted_obtain,
        args);
    niJVM_CheckException(apEnv, NULL)
    if (objectRef != NULL) {
      REF_LOG(("sJvmRefTable::Obtained(%p,%p,%p) [numRefs:%d]\n",(void*)apIID,(void*)apIUnknown,(void*)aClass,apIUnknown->GetNumRefs()));
      return objectRef;
    }

    REF_LOG(("sJvmRefTable::New(%p,%p,%p) [numRefs:%d]\n",(void*)apIID,(void*)apIUnknown,(void*)aClass,apIUnknown->GetNumRefs()));
    apIUnknown->AddRef();

    objectRef = apEnv->NewObject(aClass, niJVM_refCountedConstructorMethodID, (jlong)apIID, (jlong)apIUnknown);

    args[0].j = (jlong)apIID;
    args[1].j = (jlong)apIUnknown;
    args[2].l = objectRef;
    apEnv->CallStaticVoidMethodA(
        niJVM_Class(apEnv,RefCountedPointer),
        niJVM_refCounted_cache,
        args);
    niJVM_CheckException(apEnv,NULL);
    return objectRef;
  }

  jint Dispose(JNIEnv* apEnv, tIntPtr anIID, tIntPtr anIUnknown) {
    niAssert(anIUnknown != 0);
    iUnknown* pIUnknown = ((ni::iUnknown*)anIUnknown);
    REF_LOG(("sJvmRefTable::Dispose(%p,%p) [numRefs:%d]\n",
             anIID,anIUnknown,pIUnknown->GetNumRefs()));
    return pIUnknown->Release();
  }
};

// niSingleton(sJvmRefTableWeakRef,refTable,());
niSingleton(sJvmRefTableObtain,refTable,());
// niSingleton(sJvmRefTableNew,refTable,());

// #define JVM_DISPOSE_IN_SERVICE

struct sJvmIUnknownTable {
  __sync_mutex();
#ifdef JVM_DISPOSE_IN_SERVICE
  astl::vector<astl::pair<tIntPtr,tIntPtr> > _toDispose;
#endif

  jobject New(JNIEnv* apEnv, jclass aClass, const tUUID* apIID, const iUnknown* apIUnknown) {
    __sync_lock();
    return GetSingleton_refTable()->New(apEnv,aClass,apIID,const_cast<ni::iUnknown*>(apIUnknown));
  }
  jint Dispose(JNIEnv* apEnv, tIntPtr anIID, tIntPtr anIUnknown) {
#ifdef JVM_DISPOSE_IN_SERVICE
    __sync_lock();
    _toDispose.push_back(eastl::make_pair(anIID,anIUnknown));
    return -1;
#else
    return _DoDispose(apEnv,anIID,anIUnknown);
#endif
  }
  jint _DoDispose(JNIEnv* apEnv, tIntPtr anIID, tIntPtr anIUnknown) {
    __sync_lock();
    return GetSingleton_refTable()->Dispose(apEnv,anIID,anIUnknown);
  }
  void Service(JNIEnv* apEnv) {
#ifdef JVM_DISPOSE_IN_SERVICE
#pragma message("# JNI: Dispose in service")
    __sync_lock();
    int numToDispose = _toDispose.size();
    if (numToDispose > 0) {
#ifdef _DEBUG
      niDebugFmt(("I/JVM Dispose '%d' objects.", numToDispose));
#endif
      niLoop(i,numToDispose) {
        _DoDispose(apEnv,_toDispose[i].first,_toDispose[i].second);
      }
      _toDispose.clear();
    }
#else
#pragma message("# JNI: Dispose immediatly")
#endif
  }
};
static sJvmIUnknownTable _iunknownTable;

niExportFunc(jobject) niJVM_IUnknownNew(JNIEnv* apEnv, jclass aClass, const tUUID* apIID, const iUnknown* apIUnknown) {
  return _iunknownTable.New(apEnv,aClass,apIID,apIUnknown);
}
niExportFunc(jint) niJVM_IUnknownDispose(JNIEnv* apEnv, tIntPtr anIID, tIntPtr anIUnknown) {
  // niDebugFmt(("niJVM_IUnknownDispose: %p, %p",anIID,anIUnknown));
  return _iunknownTable.Dispose(apEnv,anIID,anIUnknown);
}
niExportFunc(void) niJVM_Service(JNIEnv* apEnv) {
  _iunknownTable.Service(apEnv);
}

niExportFunc(tBool)   niJVM_HasVM() {
  return niJVM_vm != NULL;
}

niExportFunc(JNIEnv*) niJVM_GetCurrentEnv() {
  niAssert(niJVM_vm != NULL);
  JNIEnv *e;
#if defined(niMSVC) || defined(niOSX)
#define CAST_ENV(e) (void**)&e
#else
#define CAST_ENV(e) &e
#endif
  if (niJVM_vm->AttachCurrentThread(CAST_ENV(e), NULL) != JNI_OK) {
    DEBUG_LOG(("Could not attach the JavaVM to the current thread"));
    return 0;
  }
#undef CAST_ENV
  return e;
}

niExportFunc(jint) niJVM_PutMemberOffset(JNIEnv* e, const char* className, const char* member, int offset) {
  niAssert(niJVM_putMemberOffsetMethodID != NULL);
  jvalue args[3];
  args[0].l = e->NewStringUTF(className);
  args[1].l = e->NewStringUTF(member);
  args[2].i = offset;
  e->CallStaticVoidMethodA(niJVM_Class(e,Loader), niJVM_putMemberOffsetMethodID, args);
  e->DeleteLocalRef(args[0].l);
  e->DeleteLocalRef(args[1].l);
  if (e->ExceptionCheck()) {
    niError(niFmt("Error in putMemberOffset ID for (%s,%s,%d).",
                  className, member, offset));
    return 0;
  }
  return 1;
}


/**
 * Set a property using java.lang.System.setProperty, ret contains the previous value of the property
 */
niExportFunc(ni::tBool) niJVM_SetProperty(JNIEnv* e, ni::cString& ret, const ni::achar* aProperty, const ni::achar* aaszValue) {
  niAssert(e != NULL);
  jvalue args[2];
  args[0].l = e->NewStringUTF(aProperty ? aProperty : AZEROSTR);
  args[1].l = e->NewStringUTF(aaszValue ? aaszValue : AZEROSTR);
  jstring retJava = (jstring)e->CallStaticObjectMethodA(
      niJVM_Class(e,Loader), niJVM_setProperty, args);
  e->DeleteLocalRef(args[0].l);
  e->DeleteLocalRef(args[1].l);
  niJVM_CheckException(e, NULL);
  if (retJava) {
    niJVM_JString2String(e,ret,retJava);
    e->DeleteLocalRef(retJava);
  }
  else {
    ret = "";
  }
  if (e->ExceptionCheck()) {
    niError(niFmt("Exception in setProperty('%s','%s').", aProperty, aaszValue));
    return ni::eFalse;
  }
  return ni::eTrue;
}

/**
 * Get a property using java.lang.System.getProperty, ret contains the current value of the property
 */
niExportFunc(ni::tBool) niJVM_GetProperty(JNIEnv* e, ni::cString& ret, const ni::achar* aProperty) {
  niAssert(e != NULL);
  jvalue args[1];
  args[0].l = e->NewStringUTF(aProperty ? aProperty : AZEROSTR);
  jstring retJava = (jstring)e->CallStaticObjectMethodA(
      niJVM_Class(e,Loader), niJVM_getProperty, args);
  e->DeleteLocalRef(args[0].l);
  niJVM_CheckException(e, NULL);
  if (retJava) {
    niJVM_JString2String(e,ret,retJava);
    e->DeleteLocalRef(retJava);
    return ni::eTrue;
  }
  if (e->ExceptionCheck()) {
    ret = "";
    niError(niFmt("Exception in getProperty('%s').", aProperty));
    return ni::eFalse;
  }
  else {
    ret = "";
    return ni::eFalse;
  }
}

static jstring _emptyString = NULL;

niExportFunc(jstring) niJVM_EmptyString(JNIEnv* e) {
  if (!_emptyString) {
    _emptyString = (jstring)e->NewGlobalRef(e->NewStringUTF(""));
  }
  return _emptyString;
}

niExportFunc(void*) niJVM_LockNIOBuffer(JNIEnv* e, jobject jBuffer, jarray* outBufferArray) {
  niAssert(outBufferArray != NULL);

  if (!jBuffer)
    return NULL;

  void* dataPtr = e->GetDirectBufferAddress(jBuffer);
  if (!dataPtr) {
    if (niJVM_nioByteBuffer_array && niJVM_InstanceOf(e,jBuffer,NIOByteBuffer)) {
      *outBufferArray = (jarray)niJVM_CallMethod(e,jBuffer,Object,nioByteBuffer,array,NULL);
    }
    else if (niJVM_nioIntBuffer_array && niJVM_InstanceOf(e,jBuffer,NIOIntBuffer)) {
      *outBufferArray = (jarray)niJVM_CallMethod(e,jBuffer,Object,nioIntBuffer,array,NULL);
    }
    else if (niJVM_nioCharBuffer_array && niJVM_InstanceOf(e,jBuffer,NIOCharBuffer)) {
      *outBufferArray = (jarray)niJVM_CallMethod(e,jBuffer,Object,nioCharBuffer,array,NULL);
    }
    else if (niJVM_nioDoubleBuffer_array && niJVM_InstanceOf(e,jBuffer,NIODoubleBuffer)) {
      *outBufferArray = (jarray)niJVM_CallMethod(e,jBuffer,Object,nioDoubleBuffer,array,NULL);
    }
    else if (niJVM_nioFloatBuffer_array && niJVM_InstanceOf(e,jBuffer,NIOFloatBuffer)) {
      *outBufferArray = (jarray)niJVM_CallMethod(e,jBuffer,Object,nioFloatBuffer,array,NULL);
    }
    else if (niJVM_nioLongBuffer_array && niJVM_InstanceOf(e,jBuffer,NIOLongBuffer)) {
      *outBufferArray = (jarray)niJVM_CallMethod(e,jBuffer,Object,nioLongBuffer,array,NULL);
    }
    else if (niJVM_nioShortBuffer_array && niJVM_InstanceOf(e,jBuffer,NIOShortBuffer)) {
      *outBufferArray = (jarray)niJVM_CallMethod(e,jBuffer,Object,nioShortBuffer,array,NULL);
    }
    else {
      niError(niFmt("E/ Can't cast NIOBuffer to a valid buffer type."));
      return NULL;
    }
    if (!*outBufferArray) {
      niError(niFmt("E/ Can't get array object from NIOBuffer."));
      return NULL;
    }
    dataPtr = e->GetPrimitiveArrayCritical(*outBufferArray,0);
  }

  return dataPtr;
}

niExportFunc(void) niJVM_UnlockNIOBuffer(JNIEnv* e, jobject jBuffer, void* dataPtr, jarray jBufferArray) {
  if (!dataPtr || jBufferArray == NULL)
    return;
  e->ReleasePrimitiveArrayCritical(jBufferArray,dataPtr,0);
}

#ifdef NI_JVM_USE_EXCEPTIONS
#include <exception>
niExportFunc(void) niJVM_HandleException(JNIEnv *e) {
  try {
    throw;
  } catch (std::exception& ex) {
    niJVM_ThrowRuntime(e,ex.what());
  } catch (...) {
    niJVM_ThrowRuntime(e,"Unknown Exception");
  }
}
#else
niExportFunc(void) niJVM_HandleException(JNIEnv *e) {
  niAssertUnreachable("JVM Exception");
}
#endif

niExportFunc(const ni::achar*) niJVM_GetWrapperRetChars(JNIEnv* e, jstring aStr) {
  if (aStr == (jstring)NULL)
    return AZEROSTR;

  static niThreadLocal cString* _tempString = NULL;
  if (!_tempString) {
    _tempString = new cString();
  }

  const char* utfChars = e->GetStringUTFChars(aStr, NULL);
  const tI32 len = StrSize(utfChars);
  _tempString->reserve(len);
  _tempString->Set(utfChars,len);
  e->ReleaseStringUTFChars(aStr, utfChars);
  return _tempString->Chars();
}

#endif
