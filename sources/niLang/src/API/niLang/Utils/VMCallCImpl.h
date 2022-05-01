// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_VMAPI
 * @{
 */

//========================================================================
//
// <Use Var>
//
//========================================================================
#ifdef niVMCallUseVar
#pragma niNote("I/VMCall Using Var.")
#include "../Utils/SmartPtr.h"
#include "../ILang.h"

namespace ni {
namespace vmcall {

///
/// BufRead
///
inline void BufRead(const Var& aVar, ni::achar** v) {
  niCAssert(sizeof(achar) == 1 && sizeof(cchar) == sizeof(achar) && sizeof(char) == sizeof(achar));
  *v = const_cast<achar*>(aVar.GetChars());
}
inline void BufRead(const Var& aVar, ni::cString* v) {
  VarToString(*v,aVar);
}
inline void BufRead(const Var& aVar, ni::tI8* v) {
  *v = aVar.GetI8();
}
inline void BufRead(const Var& aVar, ni::tU8* v) {
  *v = aVar.GetU8();
}
inline void BufRead(const Var& aVar, ni::tI16* v) {
  *v = aVar.GetI16();
}
inline void BufRead(const Var& aVar, ni::tU16* v) {
  *v = aVar.GetU16();
}
inline void BufRead(const Var& aVar, ni::tI32* v) {
  *v = aVar.GetI32();
}
inline void BufRead(const Var& aVar, ni::tU32* v) {
  *v = aVar.GetU32();
}
inline void BufRead(const Var& aVar, ni::tI64* v) {
  *v = aVar.GetI64();
}
inline void BufRead(const Var& aVar, ni::tU64* v) {
  *v = aVar.GetU64();
}
#if defined niTypeIntIsOtherType
inline void BufRead(const Var& aVar, ni::tInt* v) {
  *v = aVar.GetInt();
}
#endif
#if defined niTypeIntPtrIsOtherType
inline void BufRead(const Var& aVar, ni::tIntPtr* v) {
  *v = aVar.GetIntPtr();
}
#endif

inline void BufRead(const Var& aVar, ni::tF32* v) {
  *v = aVar.GetF32();
}
inline void BufRead(const Var& aVar, ni::tF64* v) {
  *v = aVar.GetF64();
}
inline void BufRead(const Var& aVar, ni::sVec2f* v) {
  *v = aVar.GetVec2f();
}
inline void BufRead(const Var& aVar, ni::sVec3f* v) {
  *v = aVar.GetVec3f();
}
inline void BufRead(const Var& aVar, ni::sVec4f* v) {
  *v = aVar.GetVec4f();
}
inline void BufRead(const Var& aVar, ni::sVec2i* v) {
  *v = aVar.GetVec2i();
}
inline void BufRead(const Var& aVar, ni::sVec3i* v) {
  *v = aVar.GetVec3i();
}
inline void BufRead(const Var& aVar, ni::sVec4i* v) {
  *v = aVar.GetVec4i();
}

inline void BufRead(const Var& aVar, ni::sColor3ub* v) {
  if (aVar.IsVec3f()) {
    v->x = (tU8)aVar.mV3F[0];
    v->y = (tU8)aVar.mV3F[1];
    v->z = (tU8)aVar.mV3F[2];
  }
  else if (aVar.IsVec3i()) {
    v->x = (tU8)aVar.mV3L[0];
    v->y = (tU8)aVar.mV3L[1];
    v->z = (tU8)aVar.mV3L[2];
  }
  else {
    tU32 c = aVar.GetU32();
    v->x = ULColorGetR(c);
    v->y = ULColorGetG(c);
    v->z = ULColorGetB(c);
  }
}

inline void BufRead(const Var& aVar, ni::sColor4ub* v) {
  if (aVar.IsVec4f()) {
    v->x = (tU8)aVar.mV4F[0];
    v->y = (tU8)aVar.mV4F[1];
    v->z = (tU8)aVar.mV4F[2];
    v->w = (tU8)aVar.mV4F[3];
  }
  else if (aVar.IsVec4i()) {
    v->x = (tU8)aVar.mV4L[0];
    v->y = (tU8)aVar.mV4L[1];
    v->z = (tU8)aVar.mV4L[2];
    v->w = (tU8)aVar.mV4L[3];
  }
  else {
    const tU32 c = aVar.GetU32();
    v->x = ULColorGetR(c);
    v->y = ULColorGetG(c);
    v->z = ULColorGetB(c);
    v->w = ULColorGetA(c);
  }
}

inline void BufRead(const Var& aVar, ni::sMatrixf* v) {
  *v = aVar.GetMatrixf();
}
inline void BufRead(const Var& aVar, ni::Var* v) {
  *v = aVar;
}
inline void BufRead(const Var& aVar, ni::tUUID* v) {
  *v = aVar.GetUUID();
}
#ifdef ni32
inline void BufRead(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU32();
}
inline void BufReadPtr(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU32();
}
#elif defined ni64
inline void BufRead(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU64();
}
inline void BufReadPtr(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU64();
}
#else
#error "Can't determin pointer size !"
#endif

template <typename T>
inline void BufReadIntf(const Var& aVar, T** v) {
  *v = ni::VarQueryInterface<T>(aVar);
}

template <typename T>
inline void BufReadNull(const Var& aVar, T*& v) {
  niAssert(aVar.IsNull());
  v = (T*)NULL;
}

///
/// BufWrite
///
inline void BufWrite(Var& aVar, ni::achar** v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::achar** v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::cString* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, ni::cString** v) {
  if (*v)
    aVar = *v;
  else
    aVar.SetNull();
}

inline void BufWrite(Var& aVar, const ni::tI8* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tU8* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tI16* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tU16* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tI32* v) {
  aVar = *v;
}
#if defined niTypeIntIsOtherType
inline void BufWrite(Var& aVar, const ni::tInt* v) {
  aVar.SetInt(*v);
}
#endif
#if defined niTypeIntPtrIsOtherType
inline void BufWrite(Var& aVar, const ni::tIntPtr* v) {
  aVar = *v;
}
#endif

inline void BufWrite(Var& aVar, const ni::tU32* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tI64* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tU64* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tF32* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tF64* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::sVec2f* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::sVec3f* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::sVec4f* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::sVec2i* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::sVec3i* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::sVec4i* v) {
  aVar = *v;
}

inline void BufWrite(Var& aVar, const ni::sColor3ub* v)
{
  aVar = ULColorBuild(v->x,v->y,v->z);
}
inline void BufWrite(Var& aVar, const ni::sColor4ub* v) {
  aVar = ULColorBuild(v->x,v->y,v->z,v->w);
}

inline void BufWrite(Var& aVar, const ni::sMatrixf* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::Var* v) {
  aVar = *v;
}
inline void BufWrite(Var& aVar, const ni::tUUID* v) {
  aVar = *v;
}
inline void BufWritePtr(Var& aVar, const ni::tPtr* v) {
  tIntPtr vptr = (tIntPtr)*v;
  aVar = vptr;
}
template <typename T>
inline void BufWriteIntf(Var& aVar, T** v) {
  T* p = *v;
#ifdef _DEBUG
  (p && p->IsOK()); // compile time check
#endif
  aVar = (iUnknown*)p;
}
template <typename T>
inline void BufWriteIntf(Var& aVar, const T** v) {
  const T* p = *v;
#ifdef _DEBUG
  (p && p->IsOK()); // compile time check
#endif
  aVar = (iUnknown*)p;
}

template <typename T>
inline void BufWriteIntf(Var& aVar, Ptr<T>* v) {
  aVar = (iUnknown*)v->ptr();
}

} // end of namespace xcall
} // end of namespace ni

///
/// IDLC macros
///
#define IDLC_BEGIN_INTF(NS,NAME)
#define IDLC_END_INTF(NS,NAME)

#ifdef niVMCall_Export
#define IDLC_RET_TYPE niExportFunc(ni::tInt)
#else
#define IDLC_RET_TYPE static ni::tInt
#endif

#ifdef _DEBUG
#define IDLC_DEBUG niDebugFmt
#else
#define IDLC_DEBUG
#endif

#define IDLC_BUF_TO_BASE(TYPE,VAR)                  \
  ni::vmcall::BufRead(*aArgs__++,&VAR);

#define IDLC_BUF_TO_PTR(TYPE,VAR)                                   \
  niCAssert(sizeof(VAR) == sizeof(ni::tPtr));                       \
  ni::vmcall::BufReadPtr(*aArgs__++,(ni::tPtr*)(&VAR));

#define IDLC_BUF_TO_INTF(TYPE,VAR)                            \
  ni::vmcall::BufReadIntf<TYPE>(*aArgs__++,&VAR);

#define IDLC_BUF_TO_ENUM(TYPE,VAR) {                      \
    ni::tU32 tmpVal = 0;                                  \
    ni::vmcall::BufRead(*aArgs__++,&tmpVal);              \
    VAR = (TYPE)tmpVal;                                   \
  }

#define IDLC_BUF_TO_NULL(TYPE,VAR) ni::vmcall::BufReadNull(*aArgs__++,VAR);

#define IDLC_RET_FROM_BASE(TYPE,VAR) if (apRet__) { \
    ni::vmcall::BufWrite(*apRet__,&VAR);            \
  }

#define IDLC_RET_FROM_INTF(TYPE,VAR) if (apRet__) { \
    ni::vmcall::BufWriteIntf<TYPE>(*apRet__,&VAR);  \
  }

#define IDLC_RET_FROM_PTR(TYPE,VAR) if (apRet__) {        \
    niCAssert(sizeof(VAR) == sizeof(ni::tPtr));           \
    ni::vmcall::BufWritePtr(*apRet__,(ni::tPtr*)(&VAR));  \
  }

#define IDLC_RET_FROM_ENUM(TYPE,VAR) if (apRet__) { \
    ni::tU32 tmpVal = VAR;                          \
    ni::vmcall::BufWrite(*apRet__,&tmpVal);         \
  }

#define IDLC_DECL_VAR(TYPE,NAME) TYPE NAME ;
#define IDLC_DECL_RETVAR(TYPE,NAME) TYPE NAME ;
#define IDLC_DECL_RETREFVAR(TYPE,NAME,REFTYPE) REFTYPE NAME ;

#define IDLC_STATIC_METH_BEGIN(NS,METH,ARITY)   \
  IDLC_RET_TYPE VMCall_##METH(                  \
      ni::iUnknown* apThis__,                   \
      const ni::Var* aArgs__,                   \
      const ni::tU32 aNumArgs__,                \
      ni::Var* apRet__)                         \
  {                                             \
  if (aNumArgs__ != ARITY)                      \
    return ni::eVMRet_InvalidArgCount;

#define IDLC_STATIC_METH_END(NS,METH,ARITY)     \
  return ni::eVMRet_OK;                         \
  }

#define IDLC_STATIC_METH_CALL(RET,NS,METH,ARITY,PARAMS) \
  RET = METH PARAMS ;
#define IDLC_STATIC_METH_CALL_VOID(RET,NS,METH,ARITY,PARAMS)  \
  METH PARAMS ;                                               \
  if (apRet__) apRet__->SetNull();

#define IDLC_STATIC_METH_CALL_NS(RET,NS,METH,ARITY,PARAMS)  \
  RET = NS :: METH PARAMS ;
#define IDLC_STATIC_METH_CALL_NS_VOID(RET,NS,METH,ARITY,PARAMS) \
  NS :: METH PARAMS ;                                           \
  if (apRet__) apRet__->SetNull();

#define IDLC_METH_BEGIN(NS,INTF,METH,ARITY)     \
  IDLC_RET_TYPE INTF##_##METH(                  \
      ni::iUnknown* apThis__,                   \
      const ni::Var* aArgs__,                   \
      const ni::tU32 aNumArgs__,                \
      ni::Var* apRet__)                         \
  {                                             \
  INTF* _this = niUnsafeCast(INTF*,apThis__);   \
  if (aNumArgs__ != ARITY)                      \
    return ni::eVMRet_InvalidArgCount;

#define IDLC_METH_END(NS,INTF,METH,ARITY)       \
  return ni::eVMRet_OK;                         \
  }

#define IDLC_METH_INVALID_BEGIN(NS,INTF,METH,ARITY)                     \
  IDLC_RET_TYPE INTF##_##METH(ni::iUnknown*, const ni::Var*, ni::tU32, ni::Var*) {

#define IDLC_METH_INVALID_END(NS,INTF,METH,ARITY) \
  return ni::eVMRet_InvalidMethod;                \
  }

#define IDLC_METH_CALL(RET,NS,INTF,METH,ARITY,PARAMS) \
  RET = _this->METH PARAMS ;

#define IDLC_METH_CALL_VOID(NS,INTF,METH,ARITY,PARAMS)  \
  _this->METH PARAMS ;                                  \
  if (apRet__) apRet__->SetNull();

//========================================================================
//
// <Use VMAPI>
//
//========================================================================
#else
#pragma niNote("I/VMCall Using VMAPI.")
#include "../Utils/VMAPI.h"
#include "../Utils/SmartPtr.h"
#include "../ILang.h"

namespace ni {
namespace vmcall {

///
/// BufRead
///
template <typename T>
static __forceinline tInt _BufReadInt(HSQUIRRELVM vm, int idx, T* v) {
  SQInt iv;
  if (SQ_FAILED(sq_getint(vm,idx,&iv)))
    return eVMRet_InvalidArg;
  *v = (T)iv;
  return eVMRet_OK;
}

static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::achar** v) {
  niCAssert(sizeof(achar) == 1 && sizeof(cchar) == sizeof(achar) && sizeof(char) == sizeof(achar));
  eScriptType scriptType = sqa_getscripttype(vm,idx);
  switch (scriptType) {
    case eScriptType_Null:
      *v = NULL;
      break;
    case eScriptType_String: {
      iHString* hstr;
      if (SQ_FAILED(sq_gethstring(vm,idx,&hstr)))
        return eVMRet_InvalidArg;
      *v = const_cast<achar*>(niHStr(hstr));
      break;
    }
    default: {
      return eVMRet_InvalidArg;
    }
  }
  return eVMRet_OK;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::cString* v) {
  iHString* hstr;
  if (SQ_FAILED(sq_gethstring(vm,idx,&hstr)))
    return eVMRet_InvalidArg;
  *v = niHStr(hstr);
  return eVMRet_OK;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tI8* v) {
  return _BufReadInt<tI8>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tU8* v) {
  return _BufReadInt<tU8>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tI16* v) {
  return _BufReadInt<tI16>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tU16* v) {
  return _BufReadInt<tU16>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tI32* v) {
  return _BufReadInt<tI32>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tU32* v) {
  return _BufReadInt<tU32>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tI64* v) {
  return _BufReadInt<tI64>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tU64* v) {
  return _BufReadInt<tU64>(vm,idx,v);
}
#if defined niTypeIntIsOtherType
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tInt* v) {
  return _BufReadInt<tInt>(vm,idx,v);
}
#endif
#if defined niTypeIntPtrIsOtherType
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tIntPtr* v) {
  return _BufReadInt<tIntPtr>(vm,idx,v);
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tUIntPtr* v) {
  return _BufReadInt<tUIntPtr>(vm,idx,v);
}
#endif

static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tF32* v) {
  return SQ_SUCCEEDED(sq_getf32(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tF64* v) {
  return SQ_SUCCEEDED(sq_getf64(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sVec2f* v) {
  return SQ_SUCCEEDED(sqa_getvec2f(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sVec3f* v) {
  return SQ_SUCCEEDED(sqa_getvec3f(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sVec4f* v) {
  return SQ_SUCCEEDED(sqa_getvec4f(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sVec2i* v) {
  return SQ_SUCCEEDED(sqa_getvec2i(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sVec3i* v) {
  return SQ_SUCCEEDED(sqa_getvec3i(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sVec4i* v) {
  return SQ_SUCCEEDED(sqa_getvec4i(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}

static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sColor3ub* v) {
  tInt r = eVMRet_InvalidArg;
  switch (sqa_getscripttype(vm,idx)) {
    case eScriptType_Null: {
      v->x = v->y = v->z = 0;
      break;
    }
    case eScriptType_Float:
    case eScriptType_Int: {
      SQInt s = 0;
      r = SQ_SUCCEEDED(sq_getint(vm,idx,&s)) ? eVMRet_OK : eVMRet_InvalidArg;
      if (r == eVMRet_OK) {
        v->x = ULColorGetR((tU32)s);
        v->y = ULColorGetG((tU32)s);
        v->z = ULColorGetB((tU32)s);
      }
      break;
    }
    case eScriptType_Vec3: {
      sVec3i s = {0,0,0};
      r = SQ_SUCCEEDED(sqa_getvec3i(vm,idx,&s)) ? eVMRet_OK : eVMRet_InvalidArg;
      if (r == eVMRet_OK) {
        v->x = (tU8)s.x;
        v->y = (tU8)s.y;
        v->z = (tU8)s.z;
      }
      break;
    }
  }
  return r;
}

static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sColor4ub* v) {
  tInt r = eVMRet_InvalidArg;
  switch (sqa_getscripttype(vm,idx)) {
    case eScriptType_Null: {
      v->x = v->y = v->z = v->w = 0;
      break;
    }
    case eScriptType_Float:
    case eScriptType_Int: {
      SQInt s = 0;
      r = SQ_SUCCEEDED(sq_getint(vm,idx,&s)) ? eVMRet_OK : eVMRet_InvalidArg;
      if (r == eVMRet_OK) {
        v->x = ULColorGetR((tU32)s);
        v->y = ULColorGetG((tU32)s);
        v->z = ULColorGetB((tU32)s);
        v->w = ULColorGetA((tU32)s);
      }
      break;
    }
    case eScriptType_Vec4: {
      sVec4i s = {0,0,0,0};
      r = SQ_SUCCEEDED(sqa_getvec4i(vm,idx,&s)) ? eVMRet_OK : eVMRet_InvalidArg;
      if (r == eVMRet_OK) {
        v->x = (tU8)s.x;
        v->y = (tU8)s.y;
        v->z = (tU8)s.z;
        v->w = (tU8)s.w;
      }
      break;
    }
  }
  return r;
}

static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::sMatrixf* v) {
  return SQ_SUCCEEDED(sqa_getmatrixf(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::Var* v) {
  return SQ_SUCCEEDED(sqa_getvar(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tUUID* v) {
  return SQ_SUCCEEDED(sqa_getUUID(vm,idx,v)) ? eVMRet_OK : eVMRet_InvalidArg;
}
static __forceinline tInt BufRead(HSQUIRRELVM vm, int idx, ni::tPtr* v) {
  return _BufReadInt<tPtr>(vm,idx,v);
}
static __forceinline tInt BufReadPtr(HSQUIRRELVM vm, int idx, ni::tPtr* v) {
  return _BufReadInt<tPtr>(vm,idx,v);
}

template <typename T>
static __forceinline tInt BufReadIntf(HSQUIRRELVM vm, int idx, T** v) {
  return SQ_SUCCEEDED(sqa_getIUnknown(vm,idx,(iUnknown**)v,niGetInterfaceUUID(T))) ?
      eVMRet_OK : eVMRet_InvalidArg;
}

template <typename T>
static __forceinline tInt BufReadNull(HSQUIRRELVM vm, int idx, T*& v) {
  SQInt nullV;
  if (SQ_FAILED(sq_getint(vm,idx,&nullV)))
    return eVMRet_InvalidNullArg;
  niAssert(nullV == 0);
  if (nullV != 0)
    return eVMRet_InvalidNullArg;
  v = (T*)NULL;
  return eVMRet_OK;
}

///
/// BufWrite
///
static __forceinline tInt BufWrite(HSQUIRRELVM vm, ni::cchar** v) {
  sq_pushhstring(vm,_H(*v));
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, ni::gchar** v) {
  sq_pushhstring(vm,_H(*v));
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, ni::xchar** v) {
  sq_pushhstring(vm,_H(*v));
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::cchar** v) {
  sq_pushhstring(vm,_H(*v));
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::gchar** v) {
  sq_pushhstring(vm,_H(*v));
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::xchar** v) {
  sq_pushhstring(vm,_H(*v));
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::cString* v) {
  sq_pushhstring(vm,_H(v->Chars()));
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, ni::cString** v) {
  (*v) ?
      sq_pushhstring(vm,_H((*v)->Chars())) :
      sq_pushhstring(vm,NULL);
  return eVMRet_OK;
}

static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tI8* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tU8* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tI16* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tU16* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tI32* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
#if defined niTypeIntIsOtherType
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tInt* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
#endif
#if defined niTypeIntPtrIsOtherType
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tIntPtr* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tUIntPtr* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
#endif

static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tU32* v) {
  sq_pushint(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tI64* v) {
  sq_pushint(vm,(SQInt)*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tU64* v) {
  sq_pushint(vm,(SQInt)*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tF32* v) {
  sq_pushf32(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tF64* v) {
  sq_pushf64(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sVec2f* v) {
  sqa_pushvec2f(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sVec3f* v) {
  sqa_pushvec3f(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sVec4f* v) {
  sqa_pushvec4f(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sVec2i* v) {
  sqa_pushvec2i(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sVec3i* v) {
  sqa_pushvec3i(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sVec4i* v) {
  sqa_pushvec4i(vm,*v);
  return eVMRet_OK;
}

static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sColor3ub* v) {
  sVec3i s = {
    (tI32)v->x,
    (tI32)v->y,
    (tI32)v->z
  };
  sqa_pushvec3i(vm,s);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sColor4ub* v) {
  sVec4i s = {
    (tI32)v->x,
    (tI32)v->y,
    (tI32)v->z,
    (tI32)v->w
  };
  sqa_pushvec4i(vm,s);
  return eVMRet_OK;
}

static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::sMatrixf* v) {
  sqa_pushmatrixf(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::Var* v) {
  sqa_pushvar(vm,(Var&)*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWrite(HSQUIRRELVM vm, const ni::tUUID* v) {
  sqa_pushUUID(vm,*v);
  return eVMRet_OK;
}
static __forceinline tInt BufWritePtr(HSQUIRRELVM vm, const ni::tPtr* v) {
  tIntPtr vptr = (tIntPtr)*v;
  sq_pushint(vm,(SQInt)vptr);
  return eVMRet_OK;
}
template <typename T>
static __forceinline tInt BufWriteIntf(HSQUIRRELVM vm, T** v) {
  T* p = *v;
#ifdef _DEBUG
  (p && p->IsOK()); // compile time check
#endif
  sqa_pushIUnknown(vm,(iUnknown*)p);
  return eVMRet_OK;
}
template <typename T>
static __forceinline tInt BufWriteIntf(HSQUIRRELVM vm, const T** v) {
  const T* p = *v;
#ifdef _DEBUG
  (p && p->IsOK()); // compile time check
#endif
  sqa_pushIUnknown(vm,(iUnknown*)p);
  return eVMRet_OK;
}

template <typename T>
static __forceinline tInt BufWriteIntf(HSQUIRRELVM vm, Ptr<T>* v) {
  Ptr<T>& p = *v;
  sqa_pushIUnknown(vm,(iUnknown*)p.ptr());
  return eVMRet_OK;
}

} // end of namespace xcall
} // end of namespace ni

///
/// IDLC macros
///
#define IDLC_BEGIN_INTF(NS,NAME)
#define IDLC_END_INTF(NS,NAME)

#ifdef niVMCall_Export
#define IDLC_RET_TYPE niExportFunc(ni::tInt)
#else
#define IDLC_RET_TYPE static ni::tInt
#endif

#ifdef _DEBUG
#define IDLC_DEBUG niDebugFmt
#else
#define IDLC_DEBUG
#endif

#define VMSTACK_NEXTARG   (argIndex++)

#define IDLC_BUF_TO_BASE(TYPE,VAR)                                    \
  if (ni::vmcall::BufRead(_vm,VMSTACK_NEXTARG,&VAR) != ni::eVMRet_OK) \
    return ni::eVMRet_InvalidArg;
#define IDLC_BUF_TO_PTR(TYPE,VAR)                                       \
  niCAssert(sizeof(VAR) == sizeof(ni::tPtr));                           \
  if (ni::vmcall::BufReadPtr(_vm,VMSTACK_NEXTARG,(ni::tPtr*)(&VAR)) != ni::eVMRet_OK) \
    return ni::eVMRet_InvalidArg;
#define IDLC_BUF_TO_INTF(TYPE,VAR)                                      \
  if (ni::vmcall::BufReadIntf<TYPE>(_vm,VMSTACK_NEXTARG,&VAR) != ni::eVMRet_OK) \
    return ni::eVMRet_InvalidArg;
#define IDLC_BUF_TO_ENUM(TYPE,VAR) {                                    \
    ni::tU32 tmpVal = 0;                                                \
    if (ni::vmcall::BufRead(_vm,VMSTACK_NEXTARG,&tmpVal) != ni::eVMRet_OK) \
      return ni::eVMRet_InvalidArg;                                     \
    VAR = (TYPE)tmpVal;                                                 \
  }
#define IDLC_BUF_TO_NULL(TYPE,VAR)                                      \
  if (ni::vmcall::BufReadNull(_vm,VMSTACK_NEXTARG,VAR) != ni::eVMRet_OK) \
    return ni::eVMRet_InvalidArg;

#define IDLC_RET_FROM_BASE(TYPE,VAR)                    \
  if (ni::vmcall::BufWrite(_vm,&VAR) != ni::eVMRet_OK)  \
    return ni::eVMRet_InvalidRet;
#define IDLC_RET_FROM_INTF(TYPE,VAR)                              \
  if (ni::vmcall::BufWriteIntf<TYPE>(_vm,&VAR) != ni::eVMRet_OK)  \
    return ni::eVMRet_InvalidRet;
#define IDLC_RET_FROM_PTR(TYPE,VAR)                                     \
  niCAssert(sizeof(VAR) == sizeof(ni::tPtr));                           \
  if (ni::vmcall::BufWritePtr(_vm,(ni::tPtr*)(&VAR)) != ni::eVMRet_OK)  \
    return ni::eVMRet_InvalidRet;
#define IDLC_RET_FROM_ENUM(TYPE,VAR) {                      \
    ni::tU32 tmpVal = VAR;                                  \
    if (ni::vmcall::BufWrite(_vm,&tmpVal) != ni::eVMRet_OK) \
      return ni::eVMRet_InvalidRet;                         \
  }

#define IDLC_DECL_VAR(TYPE,NAME) TYPE NAME ;
#define IDLC_DECL_RETVAR(TYPE,NAME) TYPE NAME ;
#define IDLC_DECL_RETREFVAR(TYPE,NAME,REFTYPE) REFTYPE NAME ;

#define IDLC_STATIC_METH_BEGIN(NS,METH,ARITY)         \
  IDLC_RET_TYPE VMCall_##METH(                        \
      ni::iUnknown* apThis__,                         \
      ni::tIntPtr apVM__,                             \
      ni::tInt aArgBase__)                            \
  {                                                   \
  HSQUIRRELVM _vm = (HSQUIRRELVM)apVM__;              \
  ni::tU32 argIndex = aArgBase__; niUnused(argIndex);

#define IDLC_STATIC_METH_END(NS,METH,ARITY)     \
  return ni::eVMRet_OK;                         \
  }

#define IDLC_STATIC_METH_CALL(RET,NS,METH,ARITY,PARAMS) \
  RET = METH PARAMS ;
#define IDLC_STATIC_METH_CALL_VOID(RET,NS,METH,ARITY,PARAMS)  \
  METH PARAMS ;                                               \
  sq_pushnull(_vm);

#define IDLC_STATIC_METH_CALL_NS(RET,NS,METH,ARITY,PARAMS)  \
  RET = NS :: METH PARAMS ;
#define IDLC_STATIC_METH_CALL_NS_VOID(RET,NS,METH,ARITY,PARAMS) \
  NS :: METH PARAMS ;                                           \
  sq_pushnull(_vm);

#define IDLC_METH_BEGIN(NS,INTF,METH,ARITY)           \
  IDLC_RET_TYPE INTF##_##METH(                        \
      ni::iUnknown* apThis__,                         \
      ni::tIntPtr apVM__,                             \
      ni::tInt aArgBase__)                            \
  {                                                   \
  INTF* _this = niUnsafeCast(INTF*,apThis__);         \
  HSQUIRRELVM _vm = (HSQUIRRELVM)apVM__;              \
  ni::tU32 argIndex = aArgBase__; niUnused(argIndex);

#define IDLC_METH_END(NS,INTF,METH,ARITY)       \
  return ni::eVMRet_OK;                         \
  }

#define IDLC_METH_INVALID_BEGIN(NS,INTF,METH,ARITY)                   \
  IDLC_RET_TYPE INTF##_##METH(ni::iUnknown*, ni::tIntPtr, ni::tInt) {

#define IDLC_METH_INVALID_END(NS,INTF,METH,ARITY) \
  return ni::eVMRet_InvalidMethod;                \
  }

#define IDLC_METH_CALL(RET,NS,INTF,METH,ARITY,PARAMS) \
  RET = _this->METH PARAMS ;

#define IDLC_METH_CALL_VOID(NS,INTF,METH,ARITY,PARAMS)  \
  _this->METH PARAMS ;                                  \
  sq_pushnull(_vm);

#endif

/**@}*/
/**@}*/
