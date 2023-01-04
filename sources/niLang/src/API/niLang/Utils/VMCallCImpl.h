// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_VMAPI
 * @{
 */

#include "../Utils/SmartPtr.h"
#include "../ILang.h"

namespace ni {
namespace vmcall {

///
/// BufRead
///
__forceinline void BufRead(const Var& aVar, ni::achar** v) {
  niCAssert(sizeof(achar) == 1 && sizeof(cchar) == sizeof(achar) && sizeof(char) == sizeof(achar));
  *v = const_cast<achar*>(aVar.GetChars());
}
__forceinline void BufRead(const Var& aVar, ni::cString* v) {
  VarToString(*v,aVar);
}
__forceinline void BufRead(const Var& aVar, ni::tI8* v) {
  *v = aVar.GetI8();
}
__forceinline void BufRead(const Var& aVar, ni::tU8* v) {
  *v = aVar.GetU8();
}
__forceinline void BufRead(const Var& aVar, ni::tI16* v) {
  *v = aVar.GetI16();
}
__forceinline void BufRead(const Var& aVar, ni::tU16* v) {
  *v = aVar.GetU16();
}
__forceinline void BufRead(const Var& aVar, ni::tI32* v) {
  *v = aVar.GetI32();
}
__forceinline void BufRead(const Var& aVar, ni::tU32* v) {
  *v = aVar.GetU32();
}
__forceinline void BufRead(const Var& aVar, ni::tI64* v) {
  *v = aVar.GetI64();
}
__forceinline void BufRead(const Var& aVar, ni::tU64* v) {
  *v = aVar.GetU64();
}
#if defined niTypeIntIsOtherType
__forceinline void BufRead(const Var& aVar, ni::tInt* v) {
  *v = aVar.GetInt();
}
#endif
#if defined niTypeIntPtrIsOtherType
__forceinline void BufRead(const Var& aVar, ni::tIntPtr* v) {
  *v = aVar.GetIntPtr();
}
#endif

__forceinline void BufRead(const Var& aVar, ni::tF32* v) {
  *v = aVar.GetF32();
}
__forceinline void BufRead(const Var& aVar, ni::tF64* v) {
  *v = aVar.GetF64();
}
__forceinline void BufRead(const Var& aVar, ni::sVec2f* v) {
  *v = aVar.GetVec2f();
}
__forceinline void BufRead(const Var& aVar, ni::sVec3f* v) {
  *v = aVar.GetVec3f();
}
__forceinline void BufRead(const Var& aVar, ni::sVec4f* v) {
  *v = aVar.GetVec4f();
}
__forceinline void BufRead(const Var& aVar, ni::sVec2i* v) {
  *v = aVar.GetVec2i();
}
__forceinline void BufRead(const Var& aVar, ni::sVec3i* v) {
  *v = aVar.GetVec3i();
}
__forceinline void BufRead(const Var& aVar, ni::sVec4i* v) {
  *v = aVar.GetVec4i();
}

__forceinline void BufRead(const Var& aVar, ni::sColor3ub* v) {
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

__forceinline void BufRead(const Var& aVar, ni::sColor4ub* v) {
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

__forceinline void BufRead(const Var& aVar, ni::sMatrixf* v) {
  *v = aVar.GetMatrixf();
}
__forceinline void BufRead(const Var& aVar, ni::Var* v) {
  *v = aVar;
}
__forceinline void BufRead(const Var& aVar, ni::tUUID* v) {
  *v = aVar.GetUUID();
}
#ifdef ni32
__forceinline void BufRead(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU32();
}
__forceinline void BufReadPtr(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU32();
}
#elif defined ni64
__forceinline void BufRead(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU64();
}
__forceinline void BufReadPtr(const Var& aVar, ni::tPtr* v) {
  *v = (tPtr)aVar.GetU64();
}
#else
#error "Can't determin pointer size !"
#endif

template <typename T>
__forceinline void BufReadIntf(const Var& aVar, T** v) {
  *v = ni::VarQueryInterface<T>(aVar);
}

template <typename T>
__forceinline void BufReadNull(const Var& aVar, T*& v) {
  niAssert(aVar.IsNull());
  v = (T*)NULL;
}

///
/// BufWrite
///
__forceinline void BufWrite(Var& aVar, ni::achar** v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::achar** v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::cString* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, ni::cString** v) {
  if (*v)
    aVar = *v;
  else
    aVar.SetNull();
}

__forceinline void BufWrite(Var& aVar, const ni::tI8* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tU8* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tI16* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tU16* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tI32* v) {
  aVar = *v;
}
#if defined niTypeIntIsOtherType
__forceinline void BufWrite(Var& aVar, const ni::tInt* v) {
  aVar.SetInt(*v);
}
#endif
#if defined niTypeIntPtrIsOtherType
__forceinline void BufWrite(Var& aVar, const ni::tIntPtr* v) {
  aVar = *v;
}
#endif

__forceinline void BufWrite(Var& aVar, const ni::tU32* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tI64* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tU64* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tF32* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tF64* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::sVec2f* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::sVec3f* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::sVec4f* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::sVec2i* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::sVec3i* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::sVec4i* v) {
  aVar = *v;
}

__forceinline void BufWrite(Var& aVar, const ni::sColor3ub* v)
{
  aVar = ULColorBuild(v->x,v->y,v->z);
}
__forceinline void BufWrite(Var& aVar, const ni::sColor4ub* v) {
  aVar = ULColorBuild(v->x,v->y,v->z,v->w);
}

__forceinline void BufWrite(Var& aVar, const ni::sMatrixf* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::Var* v) {
  aVar = *v;
}
__forceinline void BufWrite(Var& aVar, const ni::tUUID* v) {
  aVar = *v;
}
__forceinline void BufWritePtr(Var& aVar, const ni::tPtr* v) {
  tIntPtr vptr = (tIntPtr)*v;
  aVar = vptr;
}
template <typename T>
__forceinline void BufWriteIntf(Var& aVar, T** v) {
  T* p = *v;
#ifdef _DEBUG
  (p && p->IsOK()); // compile time check
#endif
  aVar = (iUnknown*)p;
}
template <typename T>
__forceinline void BufWriteIntf(Var& aVar, const T** v) {
  const T* p = *v;
#ifdef _DEBUG
  (p && p->IsOK()); // compile time check
#endif
  aVar = (iUnknown*)p;
}

template <typename T>
__forceinline void BufWriteIntf(Var& aVar, Ptr<T>* v) {
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

/**@}*/
/**@}*/
