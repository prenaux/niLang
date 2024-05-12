#ifndef __UNKNOWNIMPL_35259980_H__
#define __UNKNOWNIMPL_35259980_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../Utils/Meta.h"
#include "../Utils/SmartPtr.h"
#include "../ObjModel.h"
#include "../ICollection.h"
#include "../Var.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

namespace ImplMeta {
niMetaDeclareIsOfType(cUnknown1);
niMetaDeclareIsOfType(cUnknown2);
niMetaDeclareIsOfType(cUnknown3);
niMetaDeclareIsOfType(cUnknown4);

template <typename BaseImpl> struct sQueryInterfaceNull {
  static iUnknown* _QI(BaseImpl* apThis, const ni::tUUID& aIID) {
    niUnused(apThis); niUnused(aIID);
    return nullptr;
  }
  static tBool _LI(iMutableCollection* apLst) {
    niUnused(apLst);
    return eTrue;
  }
};
template <typename BaseImpl, typename T> struct sQueryInterfaceTpl {
  static inline iUnknown* _QI(BaseImpl* apThis, const ni::tUUID& aIID) {
    if (T::_GetInterfaceUUID() == aIID) return niIUnknownCast2(T,apThis);
    else                return nullptr;
  }
  static inline tBool _LI(iMutableCollection* apLst) {
    apLst->Add(niGetInterfaceUUID(T));
    return ni::eTrue;
  }
};
template <typename BaseImpl, typename T, typename B> struct sQueryInterfaceInheritTpl {
  static inline iUnknown* _QI(BaseImpl* apThis, const ni::tUUID& aIID) {
    if (T::_GetInterfaceUUID() == aIID) return niIUnknownCast2(T,apThis);
    else                return B::_QI(apThis,aIID);
  }
  static inline tBool _LI(iMutableCollection* apLst) {
    apLst->Add(niGetInterfaceUUID(T));
    return B::_LI(apLst);
  }
};

#define niImpl_QueryInterfaceTpl(A)  ni::ImplMeta::sQueryInterfaceTpl<BaseImpl,A>
#define niImpl_QueryInterfaceInheritTpl(A,B) ni::ImplMeta::sQueryInterfaceInheritTpl<BaseImpl,A,B>

#define niImpl_QueryInterface()                                         \
  private:                                                              \
  typedef typename niMetaSelect(ni::ImplMeta::niMetaIsOfType(T4,cUnknown4), \
                                ni::ImplMeta::sQueryInterfaceNull<BaseImpl>, \
                                niImpl_QueryInterfaceTpl(T4)) V4;       \
  typedef typename niMetaSelect(ni::ImplMeta::niMetaIsOfType(T3,cUnknown3), \
                                V4, niImpl_QueryInterfaceInheritTpl(T3,V4)) V3; \
  typedef typename niMetaSelect(ni::ImplMeta::niMetaIsOfType(T2,cUnknown2), \
                                V3, niImpl_QueryInterfaceInheritTpl(T2,V3)) V2; \
  typedef typename niMetaSelect(ni::ImplMeta::niMetaIsOfType(T1,cUnknown1), \
                                V2, niImpl_QueryInterfaceInheritTpl(T1,V2)) V1; \
  typedef niImpl_QueryInterfaceInheritTpl(T0,V1) sQueryInterface;       \
 public:                                                                \
  virtual iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) niOverride {   \
    iUnknown* r = sQueryInterface::_QI(this,aIID);                      \
    if (r) return r;                                                    \
    if (aIID == niGetInterfaceUUID(iUnknown)) return niIUnknownCast2(T0,this); \
    return nullptr;                                                     \
  }                                                                     \
  virtual void __stdcall ListInterfaces(iMutableCollection* apLst, tU32) const niOverride { \
    sQueryInterface::_LI(apLst);                                        \
    apLst->Add(niGetInterfaceUUID(ni::iUnknown));                       \
  }

#define niImpl_Inherit(MEM) Impl_Inherit<ni::Impl_##MEM,T0,FLAGS,T1,T2,T3,T4>

#define niImpl_Base_DeleteThis()                        \
  public:                                               \
  virtual ni::tBool __stdcall IsOK() const niOverride { \
    return ni::eTrue;                                   \
  }                                                     \
  virtual void __stdcall Invalidate() niOverride {      \
  }                                                     \
  virtual void __stdcall DeleteThis() niOverride {      \
    niDelete this;                                      \
  }                                                     \

#define niImpl_Base_EmptyDeleteThis()                   \
  public:                                               \
  virtual ni::tBool __stdcall IsOK() const niOverride { \
    return ni::eTrue;                                   \
  }                                                     \
  virtual void __stdcall Invalidate() niOverride {}     \
  virtual void __stdcall DeleteThis() niOverride {}

#define niImpl_DefineBaseTypes(CLASS)                     \
  public:                                                 \
  typedef CLASS<T0,FLAGS,T1,T2,T3,T4> BaseImpl;           \
  typedef T0 Base0;                                       \
  typedef T1 Base1;                                       \
  typedef T2 Base2;                                       \
  typedef T3 Base3;                                       \
  typedef T4 Base4;

#define niImpl_AddRef() {                         \
    return SYNC_INCREMENT(&mprotected_nNumRefs);  \
  }

#define niImpl_Release() {                                    \
    if (SYNC_DECREMENT(&mprotected_nNumRefs) == 0) {          \
      this->DeleteThis();                                     \
      return -1;                                              \
    }                                                         \
    /* you have a double free if this assert is triggered, */ \
    /* you should figure out why and fix it */                \
    niAssert(mprotected_nNumRefs > 0);                        \
    return SYNC_READ(&mprotected_nNumRefs);                   \
  }

#define niImpl_SetNumRefs(anNumRefs) {          \
    SYNC_WRITE(&mprotected_nNumRefs,anNumRefs); \
    return SYNC_READ(&mprotected_nNumRefs);     \
  }

#define niImpl_GetNumRefs() {                   \
    return SYNC_READ(&mprotected_nNumRefs);     \
  }

#define niImpl_RefCount()                                               \
  SYNC_INT_TYPE mprotected_nNumRefs;                                    \
  virtual ni::tI32 __stdcall AddRef() niOverride {                      \
    niImpl_AddRef();                                                    \
  }                                                                     \
  virtual ni::tI32 __stdcall Release() niOverride {                     \
    niImpl_Release();                                                   \
  }                                                                     \
  virtual ni::tI32 __stdcall SetNumRefs(ni::tI32 anNumRefs) niOverride { \
    niImpl_SetNumRefs(anNumRefs);                                       \
  }                                                                     \
  virtual ni::tI32 __stdcall GetNumRefs() const niOverride {            \
    niImpl_GetNumRefs();                                                \
  }

#define niImpl_NoRefCount()                                       \
  virtual ni::tI32 __stdcall AddRef() niOverride {                \
    return ni::eInvalidHandle;                                    \
  }                                                               \
  virtual tI32 __stdcall Release() niOverride {                   \
    return ni::eInvalidHandle;                                    \
  }                                                               \
  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) niOverride {  \
    niUnused(anNumRefs);                                          \
    return ni::eInvalidHandle;                                    \
  }                                                               \
  virtual tI32 __stdcall GetNumRefs() const niOverride {          \
    return ni::eInvalidHandle;                                    \
  }

#define niImpl_Aggregate()                                              \
  ni::iUnknown* mprotected_pAggregateParent;                            \
  virtual ni::tI32 __stdcall AddRef() niOverride {                      \
    niAssert(mprotected_pAggregateParent != nullptr);                   \
    return mprotected_pAggregateParent->AddRef();                       \
  }                                                                     \
  virtual ni::tI32 __stdcall Release() niOverride {                     \
    niAssert(mprotected_pAggregateParent != nullptr);                   \
    return mprotected_pAggregateParent->Release();                      \
  }                                                                     \
                                                                        \
  virtual ni::tI32 __stdcall SetNumRefs(ni::tI32 anNumRefs) niOverride { \
    niAssert(mprotected_pAggregateParent != nullptr);                   \
    return mprotected_pAggregateParent->SetNumRefs(anNumRefs);          \
  }                                                                     \
  virtual ni::tI32 __stdcall GetNumRefs() const niOverride {            \
    niAssert(mprotected_pAggregateParent != nullptr);                   \
    return mprotected_pAggregateParent->GetNumRefs();                   \
  }

};

//{NoAutomation}
enum eImplFlags {
  eImplFlags_DontInherit1 = niBit(3),
  eImplFlags_DontInherit2 = niBit(4),
  eImplFlags_DontInherit3 = niBit(5),
  eImplFlags_DontInherit4 = niBit(6),
  eImplFlags_Default = 0,
  eImplFlags_ForceDWORD = 0xFFFFFFFF
};

typedef tU32 tImplFlags;

//! Implements new/delete for ni memory allocator
class Impl_HeapAlloc {
public:
  inline void* operator new(size_t anSize, const achar* file, int line, const achar* fun) {
    return (void*)ni_object_alloc(anSize,file,line,fun);
  }
  inline void* operator new[](size_t anSize, const achar* file, int line, const achar* fun) {
    return (void*)ni_object_alloc(anSize,file,line,fun);
  }
  inline void   operator delete(void *apMem, const achar* file, int line, const achar* fun) {
    ni_object_free(apMem,file,line,fun);
  }
  inline void   operator delete[](void *apMem, const achar* file, int line, const achar* fun) {
    ni_object_free(apMem,file,line,fun);
  }

  // placement new
  inline void* operator new(size_t anSize, void* apMem)  {
    niUnused(anSize); return apMem;
  }
  // placement delete
  inline void operator delete(void* apMem, void* apMemDum) {
    niUnused(apMem); niUnused(apMemDum);
    /*nothing, this is for the placed new which should handle the memory given by itself*/
  }

  // shouldn't be used, use niNew
  inline void*  operator new(size_t anSize) {
    return (void*)ni_object_alloc((ni::tSize)anSize,nullptr,-1,nullptr);
  }
  // shouldn't be used, use niNew
  inline void* operator new[](size_t anSize) {
    return (void*)ni_object_alloc((ni::tSize)anSize,nullptr,-1,nullptr);
  }

  // shouldn't be used, use niDelete
  inline void operator delete(void *apMem) {
    ni_object_free(apMem,nullptr,-1,nullptr);
  }
  // shouldn't be used, use niDelete
  inline void operator delete[](void *apMem) {
    ni_object_free(apMem,nullptr,-1,nullptr);
  }
};

//! Dont allow heap allocation
class Impl_NoHeapAlloc {
protected:
  void* operator new(size_t anSize, const achar* file, int line, const achar* fun);
  void* operator new[](size_t anSize, const achar* file, int line, const achar* fun);
  void* operator new(size_t anSize);
  void* operator new[](size_t anSize);
  void* operator new(size_t anSize, void* apMem);
};

//! Do the inheritance for IZupnown impl.
template <typename TMEM, typename T0, tImplFlags FLAGS, typename T1, typename T2, typename T3, typename T4>
class Impl_Inherit :
      public TMEM,
      public T0,
      public niMetaSelect(niFlagIsNot(FLAGS,eImplFlags_DontInherit1),T1,cUnknown1),
      public niMetaSelect(niFlagIsNot(FLAGS,eImplFlags_DontInherit2),T2,cUnknown2),
      public niMetaSelect(niFlagIsNot(FLAGS,eImplFlags_DontInherit3),T3,cUnknown3),
      public niMetaSelect(niFlagIsNot(FLAGS,eImplFlags_DontInherit4),T4,cUnknown4)
{
public:
  typedef T0 IUnknownBaseType;
protected:
  typedef T0 BaseType;
};

#if niPragmaPack
#pragma niPackPush(4)
#endif

//! Implements ref counting policy.
template <typename T0,
          tImplFlags FLAGS = eImplFlags_Default,
          typename T1 = cUnknown1,
          typename T2 = cUnknown2,
          typename T3 = cUnknown3,
          typename T4 = cUnknown4>
struct niEmptyBases ImplRC : public niImpl_Inherit(HeapAlloc)
{
  niImpl_DefineBaseTypes(ImplRC);
  niImpl_QueryInterface();
  niImpl_Base_DeleteThis();
  niImpl_RefCount();
} niAligned(4) niPacked(4);
static_assert(sizeof(ni::ImplRC<iUnknown>) == sizeof(void*)+sizeof(SYNC_INT_TYPE));

//! Implements aggregation policy.
template <typename T0,
          tImplFlags FLAGS = eImplFlags_Default,
          typename T1 = cUnknown1,
          typename T2 = cUnknown2,
          typename T3 = cUnknown3,
          typename T4 = cUnknown4>
struct niEmptyBases ImplAggregate : public niImpl_Inherit(HeapAlloc)
{
  niImpl_DefineBaseTypes(ImplAggregate);
  niImpl_QueryInterface();
  niImpl_Base_DeleteThis();
  niImpl_Aggregate();
} niAligned(4) niPacked(4);
static_assert(sizeof(ni::ImplAggregate<iUnknown>) == sizeof(void*)+sizeof(ni::iUnknown*));

//! Implements a local/member/stack only policy.
template <typename T0,
          tImplFlags FLAGS = eImplFlags_Default,
          typename T1 = cUnknown1,
          typename T2 = cUnknown2,
          typename T3 = cUnknown3,
          typename T4 = cUnknown4>
struct niEmptyBases ImplLocal : public niImpl_Inherit(NoHeapAlloc)
{
  niImpl_DefineBaseTypes(ImplLocal);
  niImpl_QueryInterface();
  niImpl_Base_EmptyDeleteThis();
  niImpl_NoRefCount();
} niAligned(4) niPacked(4);
static_assert(sizeof(ni::ImplLocal<iUnknown>) == sizeof(void*));

#if niPragmaPack
#pragma niPackPop()
#endif

/**@}*/
/**@}*/
} // end of ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __UNKNOWNIMPL_35259980_H__
