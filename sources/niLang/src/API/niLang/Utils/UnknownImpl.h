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

struct  iHString;
struct  iUnknown;

// IUnknown default implementation, meta programs
namespace UnkImplMeta {
niMetaDeclareIsOfType(cUnknown1);
niMetaDeclareIsOfType(cUnknown2);
niMetaDeclareIsOfType(cUnknown3);
niMetaDeclareIsOfType(cUnknown4);

template <typename BaseImpl> struct sQueryInterfaceNULL {
  static iUnknown* _QI(BaseImpl* apThis, const ni::tUUID& aIID) {
    niUnused(apThis); niUnused(aIID);
    return NULL;
  }
  static tBool _LI(iMutableCollection* apLst) {
    niUnused(apLst);
    return eTrue;
  }
};
template <typename BaseImpl, typename T> struct sQueryInterfaceTpl {
  static inline iUnknown* _QI(BaseImpl* apThis, const ni::tUUID& aIID) {
    if (T::_GetInterfaceUUID() == aIID) return niIUnknownCast2(T,apThis);
    else                return NULL;
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

#define MetaQueryInterfaceTpl(A)  UnkImplMeta::sQueryInterfaceTpl<BaseImpl,A>
#define MetaQueryInterfaceInheritTpl(A,B) UnkImplMeta::sQueryInterfaceInheritTpl<BaseImpl,A,B>

#define MetaImplementQueryInterface()                                   \
  typedef typename niMetaSelect(UnkImplMeta::niMetaIsOfType(T4,cUnknown4), \
                                UnkImplMeta::sQueryInterfaceNULL<BaseImpl>, \
                                MetaQueryInterfaceTpl(T4)) V4;          \
  typedef typename niMetaSelect(UnkImplMeta::niMetaIsOfType(T3,cUnknown3), \
                                V4, MetaQueryInterfaceInheritTpl(T3,V4)) V3; \
  typedef typename niMetaSelect(UnkImplMeta::niMetaIsOfType(T2,cUnknown2), \
                                V3, MetaQueryInterfaceInheritTpl(T2,V3)) V2; \
  typedef typename niMetaSelect(UnkImplMeta::niMetaIsOfType(T1,cUnknown1), \
                                V2, MetaQueryInterfaceInheritTpl(T1,V2)) V1; \
  typedef MetaQueryInterfaceInheritTpl(T0,V1) sQueryInterface;

#define MetaImplementInherit(PARENT) UnkImpl_##PARENT<UnkImpl_Inherit<T0,FLAGS,T1,T2,T3,T4>, FLAGS>
};

//{NoAutomation}
enum eIUnknownImplFlags
{
  eIUnknownImplFlags_NoMemoryAllocator = niBit(0),
  eIUnknownImplFlags_NoRefCount = niBit(1),
  eIUnknownImplFlags_Aggregatable = niBit(2),
  eIUnknownImplFlags_DontInherit1 = niBit(3),
  eIUnknownImplFlags_DontInherit2 = niBit(4),
  eIUnknownImplFlags_DontInherit3 = niBit(5),
  eIUnknownImplFlags_DontInherit4 = niBit(6),
  eIUnknownImplFlags_EmptyDeleteThis = niBit(7),
  eIUnknownImplFlags_Default = 0,
  eIUnknownImplFlags_RefCount = 0,
  eIUnknownImplFlags_Local = eIUnknownImplFlags_NoRefCount|eIUnknownImplFlags_NoMemoryAllocator|eIUnknownImplFlags_EmptyDeleteThis,
  eIUnknownImplFlags_ForceDWORD = 0xFFFFFFFF
};

typedef tU32 tIUnknownImplFlags;

//! Implements new/delete for ni memory allocator
class cMemImpl {
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

  // TODO: Make this protected, but we need to get rid of all dynamic C++
  // arrays and update niNew & niDelete to be more function-like.

  // protected:

  // shouldn't be used, use niNew
  inline void*  operator new(size_t anSize) {
    return (void*)ni_object_alloc((ni::tSize)anSize,NULL,-1,NULL);
  }
  // shouldn't be used, use niNew
  inline void* operator new[](size_t anSize) {
    return (void*)ni_object_alloc((ni::tSize)anSize,NULL,-1,NULL);
  }

  // shouldn't be used, use niDelete
  inline void operator delete(void *apMem) {
    ni_object_free(apMem,NULL,-1,NULL);
  }
  // shouldn't be used, use niDelete
  inline void operator delete[](void *apMem) {
    ni_object_free(apMem,NULL,-1,NULL);
  }
};

template <typename BASE>
struct UnkImpl_DeleteThis : public BASE {
  inline void __stdcall DeleteThis() { niDelete this; }
};
template <typename BASE>
struct UnkImpl_EmptyDeleteThis : public BASE {
  inline void __stdcall DeleteThis() {}
};

//! Do the inheritance for IUnknown impl.
template <typename T0, tIUnknownImplFlags FLAGS, typename T1, typename T2, typename T3, typename T4>
class UnkImpl_Inherit :
      public niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_NoMemoryAllocator),cUnknown0,cMemImpl),
      public T0,
      public niMetaSelect(niFlagIsNot(FLAGS,eIUnknownImplFlags_DontInherit1),T1,cUnknown1),
      public niMetaSelect(niFlagIsNot(FLAGS,eIUnknownImplFlags_DontInherit2),T2,cUnknown2),
      public niMetaSelect(niFlagIsNot(FLAGS,eIUnknownImplFlags_DontInherit3),T3,cUnknown3),
      public niMetaSelect(niFlagIsNot(FLAGS,eIUnknownImplFlags_DontInherit4),T4,cUnknown4)
{
public:
  typedef T0 IUnknownBaseType;
protected:
  typedef T0 BaseType;
  typedef typename niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_NoMemoryAllocator),cUnknown0,cMemImpl) tBaseMemAlloc;
};

#define NI_UNKIMPL_ADDREF() {                     \
    return SYNC_INCREMENT(&mprotected_nNumRefs);  \
  }

#define NI_UNKIMPL_RELEASE() {                                \
    if (SYNC_DECREMENT(&mprotected_nNumRefs) == 0) {          \
      this->DeleteThis();                                     \
      return -1;                                              \
    }                                                         \
    /* you have a double free if this assert is triggered, */ \
    /* you should figure out why and fix it */                \
    niAssert(mprotected_nNumRefs > 0);                        \
    return SYNC_READ(&mprotected_nNumRefs);                   \
  }

#define NI_UNKIMPL_SETNUMREFS(anNumRefs) {      \
    SYNC_WRITE(&mprotected_nNumRefs,anNumRefs); \
    return SYNC_READ(&mprotected_nNumRefs);     \
  }

#define NI_UNKIMPL_GETNUMREFS() {               \
    return SYNC_READ(&mprotected_nNumRefs);     \
  }

//! Implements ref counting policy.
template <typename BASE, tIUnknownImplFlags FLAGS>
class UnkImpl_RefCount : public niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_EmptyDeleteThis),
                                             UnkImpl_EmptyDeleteThis<BASE>,
                                             UnkImpl_DeleteThis<BASE>)
{
public:
  UnkImpl_RefCount() {}

  virtual tI32 __stdcall AddRef() {
    NI_UNKIMPL_ADDREF();
  }
  virtual tI32 __stdcall Release() {
    NI_UNKIMPL_RELEASE();
  }
  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) {
    NI_UNKIMPL_SETNUMREFS(anNumRefs);
  }
  virtual tI32 __stdcall GetNumRefs() const {
    NI_UNKIMPL_GETNUMREFS();
  }

  SYNC_INT_TYPE mprotected_nNumRefs;
};

//! Implements aggregatable ref counting policy.
template <typename BASE, tIUnknownImplFlags FLAGS>
class UnkImpl_Aggregatable : public niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_EmptyDeleteThis),
                                                 UnkImpl_EmptyDeleteThis<BASE>,
                                                 UnkImpl_DeleteThis<BASE>)
{
public:
  UnkImpl_Aggregatable()
      : mprotected_pAggregateParent(NULL) {}

  virtual tI32 __stdcall AddRef() {
    if (mprotected_pAggregateParent) {
      return mprotected_pAggregateParent->AddRef();
    }
    else {
      NI_UNKIMPL_ADDREF();
    }
  }

  virtual tI32 __stdcall Release() {
    if (mprotected_pAggregateParent) {
      return mprotected_pAggregateParent->Release();
    }
    else {
      NI_UNKIMPL_RELEASE();
    }
  }

  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) {
    if (mprotected_pAggregateParent != NULL) {
      return mprotected_pAggregateParent->SetNumRefs(anNumRefs);
    }
    else {
      NI_UNKIMPL_SETNUMREFS(anNumRefs);
    }
  }
  virtual tI32 __stdcall GetNumRefs() const {
    if (mprotected_pAggregateParent) {
      return mprotected_pAggregateParent->GetNumRefs();
    }
    else {
      NI_UNKIMPL_GETNUMREFS();
    }
  }

  iUnknown* mprotected_pAggregateParent;
  SYNC_INT_TYPE mprotected_nNumRefs;
};

//! Implements aggregate ref counting only policy.
template <typename BASE, tIUnknownImplFlags FLAGS>
class UnkImpl_AggregateOnly : public niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_EmptyDeleteThis),
                                                  UnkImpl_EmptyDeleteThis<BASE>,
                                                  UnkImpl_DeleteThis<BASE>)
{
public:
  UnkImpl_AggregateOnly()
      : mprotected_pAggregateParent(NULL) {}
  virtual tI32 __stdcall AddRef()
  {
    niAssert(mprotected_pAggregateParent != NULL);
    return mprotected_pAggregateParent->AddRef();
  }
  virtual tI32 __stdcall Release()
  {
    niAssert(mprotected_pAggregateParent != NULL);
    return mprotected_pAggregateParent->Release();
  }

  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) {
    niAssert(mprotected_pAggregateParent != NULL);
    return mprotected_pAggregateParent->SetNumRefs(anNumRefs);
  }
  virtual tI32 __stdcall GetNumRefs() const {
    niAssert(mprotected_pAggregateParent != NULL);
    return mprotected_pAggregateParent->GetNumRefs();
  }

  iUnknown* mprotected_pAggregateParent;
};

//! Implements not ref counted policy.
template <typename BASE, tIUnknownImplFlags FLAGS>
class UnkImpl_NoRefCount : public niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_EmptyDeleteThis),
                                               UnkImpl_EmptyDeleteThis<BASE>,
                                               UnkImpl_DeleteThis<BASE>)
{
public:
  virtual tI32 __stdcall AddRef()   { return eInvalidHandle; }
  virtual tI32 __stdcall Release()  { return eInvalidHandle; }
  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) { niUnused(anNumRefs); return eInvalidHandle; }
  virtual tI32 __stdcall GetNumRefs() const { return eInvalidHandle; }
};

//! RefCounted iUnknown implementation.
template <typename T0,
          tIUnknownImplFlags FLAGS = eIUnknownImplFlags_Default,
          typename T1 = cUnknown1,
          typename T2 = cUnknown2,
          typename T3 = cUnknown3,
          typename T4 = cUnknown4>
class cIUnknownImpl : public niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_NoRefCount),
                                          typename niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_Aggregatable),
                                                                MetaImplementInherit(AggregateOnly),
                                                                MetaImplementInherit(NoRefCount)),
                                          typename niMetaSelect(niFlagIs(FLAGS,eIUnknownImplFlags_Aggregatable),
                                                                MetaImplementInherit(Aggregatable),
                                                                MetaImplementInherit(RefCount)))
{
public:
  typedef cIUnknownImpl<T0,FLAGS,T1,T2,T3,T4> BaseImpl;
  typedef T0  Base0;
  typedef T1  Base1;
  typedef T2  Base2;
  typedef T3  Base3;
  typedef T4  Base4;

private:
  MetaImplementQueryInterface();

public:
  cIUnknownImpl();

  //// iUnkown //////////////////////////////////
  virtual tBool __stdcall IsOK() const;
  virtual void __stdcall Invalidate();
  virtual iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID);
  virtual void __stdcall ListInterfaces(iMutableCollection* apLst, tU32) const;
  //// iUnkown //////////////////////////////////
};

#undef MetaQueryInterfaceInheritTpl
#undef MetaQueryInterfaceTpl
#undef MetaImplementQueryInterface
#undef MetaImplementInherit

//////////////////////////////////////////////////////////////////////////////////////////////
// NoRefCount template
template <typename T>
struct NoRefCount : public T
{
  inline tI32 __stdcall AddRef() { return eInvalidHandle; }
  inline tI32 __stdcall Release() { return eInvalidHandle; }
  inline T* __stdcall GetNoRefCountPointer() const { return static_cast<T*>(const_cast<NoRefCount*>(this)); }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Aggregate template
template <typename T>
struct Aggregate : public T
{
  inline Aggregate(iUnknown* apParent) : mpParent(apParent) {}
  inline tI32 __stdcall AddRef() { return mpParent->AddRef(); }
  inline tI32 __stdcall Release() { return mpParent->Release(); }
  inline iUnknown* __stdcall GetAggregateParent() { return mpParent; }
  inline T* __stdcall GetAggregatePointer() const { return static_cast<T*>(const_cast<Aggregate*>(this)); }
 private:
  iUnknown* mpParent;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cIUnknownImpl class implementation.

///////////////////////////////////////////////
template <typename T0, tIUnknownImplFlags FLAGS, typename T1, typename T2, typename T3, typename T4>
inline cIUnknownImpl<T0,FLAGS,T1,T2,T3,T4>::cIUnknownImpl()
{
}

///////////////////////////////////////////////
template <typename T0, tIUnknownImplFlags FLAGS, typename T1, typename T2, typename T3, typename T4>
inline tBool __stdcall cIUnknownImpl<T0,FLAGS,T1,T2,T3,T4>::IsOK() const
{
  return eTrue;
}

///////////////////////////////////////////////
template <typename T0, tIUnknownImplFlags FLAGS, typename T1, typename T2, typename T3, typename T4>
inline void __stdcall cIUnknownImpl<T0,FLAGS,T1,T2,T3,T4>::Invalidate()
{
}

///////////////////////////////////////////////
template <typename T0, tIUnknownImplFlags FLAGS, typename T1, typename T2, typename T3, typename T4>
iUnknown* __stdcall cIUnknownImpl<T0,FLAGS,T1,T2,T3,T4>::QueryInterface(const tUUID& aIID)
{
  iUnknown* r = sQueryInterface::_QI(this,aIID);
  return r ? r :
      (aIID == niGetInterfaceUUID(iUnknown)) ? niIUnknownCast2(T0,this):
      NULL;
}

///////////////////////////////////////////////
template <typename T0, tIUnknownImplFlags FLAGS, typename T1, typename T2, typename T3, typename T4>
void __stdcall cIUnknownImpl<T0,FLAGS,T1,T2,T3,T4>::ListInterfaces(iMutableCollection* apLst, tU32) const
{
  sQueryInterface::_LI(apLst);
  apLst->Add(niGetInterfaceUUID(ni::iUnknown)); // always add iUnknown...
}

} // end of ni

//////////////////////////////////////////////////////////////////////////////////////////////
// Macro-based IUnknown implementation.
namespace ni {

//! Local, no ref count, no delete this
#define niIUnknownImpl_Local(I0)                                        \
  virtual tBool __stdcall IsOK() const niImpl { return ni::eTrue; }            \
  virtual void __stdcall Invalidate() niImpl {}                         \
  virtual tI32 __stdcall AddRef()  niImpl { return 0; }                \
  virtual tI32 __stdcall Release() niImpl { return 0; }                \
  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) niImpl { niUnused(anNumRefs); return 0; } \
  virtual tI32 __stdcall GetNumRefs() const niImpl { return 0; }        \
  virtual void __stdcall DeleteThis() niImpl {}                         \
  virtual void __stdcall ListInterfaces(ni::iMutableCollection* apLst, ni::tU32) const niImpl { \
    apLst->Add(niGetInterfaceUUID(I0));                                 \
    apLst->Add(niGetInterfaceUUID(ni::iUnknown));                       \
  }                                                                     \
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) niImpl { \
    if (aIID == niGetInterfaceUUID(I0)) return static_cast<I0*>(this);  \
    if (aIID == niGetInterfaceUUID(ni::iUnknown)) return this;          \
    return NULL;                                                        \
  }

} // end of ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
#endif // __UNKNOWNIMPL_35259980_H__
