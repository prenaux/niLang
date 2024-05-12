#ifndef __COLLECTIONIMPL_H_604D0B86_1048_4125_A19E_CCAAE861924F__
#define __COLLECTIONIMPL_H_604D0B86_1048_4125_A19E_CCAAE861924F__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../Var.h"
#include "../ICollection.h"
#include "UnknownImpl.h"
#include "../StringBase.h"
#include "../STL/list.h"
#include "../STL/vector.h"
#include "../STL/map.h"
#include "../STL/utils.h"
#include "../Utils/QPtr.h"
#include "../Utils/Nonnull.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

struct CollectionTraitsI8 {
  enum { eType = eType_I8 };
  typedef ni::tI8 tValueType;
  typedef ni::tI8 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tValueType)(aVar.GetIntValue());
  }
};
struct CollectionTraitsU8 {
  enum { eType = eType_U8 };
  typedef ni::tU8 tValueType;
  typedef ni::tU8 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tU8)aVar.GetIntValue();
  }
};

struct CollectionTraitsI16 {
  enum { eType = eType_I16 };
  typedef ni::tI16 tValueType;
  typedef ni::tI16 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tI16)aVar.GetIntValue();
  }
};
struct CollectionTraitsU16 {
  enum { eType = eType_U16 };
  typedef ni::tU16 tValueType;
  typedef ni::tU16 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tU16)aVar.GetIntValue();
  }
};

struct CollectionTraitsI32 {
  enum { eType = eType_I32 };
  typedef ni::tI32 tValueType;
  typedef ni::tI32 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tValueType)aVar.GetIntValue();
  }
};
struct CollectionTraitsU32 {
  enum { eType = eType_U32 };
  typedef ni::tU32 tValueType;
  typedef ni::tU32 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tValueType)aVar.GetIntValue();
  }
};

struct CollectionTraitsI64 {
  enum { eType = eType_I64 };
  typedef ni::tI64 tValueType;
  typedef ni::tI64 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tValueType)aVar.GetIntValue();
  }
};
struct CollectionTraitsU64 {
  enum { eType = eType_U64 };
  typedef ni::tU64 tValueType;
  typedef ni::tU64 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIntValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tValueType)aVar.GetIntValue();
  }
};

struct CollectionTraitsF32 {
  enum { eType = eType_F32 };
  typedef ni::tF32 tValueType;
  typedef ni::tF32 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsFloatValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tValueType)aVar.GetFloatValue();
  }
};

struct CollectionTraitsF64 {
  enum { eType = eType_F64 };
  typedef ni::tF64 tValueType;
  typedef ni::tF64 tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsFloatValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return (tValueType)aVar.GetFloatValue();
  }
};

struct CollectionTraitsUUID {
  enum { eType = eType_UUID };
  typedef ni::tUUID tValueType;
  typedef ni::tUUID tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsUUID();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetUUID();
  }
};

struct CollectionTraitsString {
  enum { eType = eType_String };
  typedef ni::cString tValueType;
  typedef ni::cString tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return VarIsString(aVar);
  }
  static tValueType GetValue(const Var& aVar) {
    return VarGetString(aVar);
  }
};

struct CollectionTraitsIUnknown {
  enum { eType = eType_IUnknown };
  typedef Ptr<iUnknown> tValueType;
  typedef iUnknown* tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsIUnknownPointer();
  }
  static tValueRetType GetValue(const Var& aVar) {
    return aVar.GetIUnknownPointer();
  }
};

template <typename T>
struct CollectionTraitsInterface {
  enum { eType = eType_IUnknown };
  typedef Ptr<T> tValueType;
  typedef T* tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    if (!aVar.IsIUnknownPointer()) return eFalse;
    return ni::QPtr<T>(aVar.GetIUnknownPointer()).IsOK();
  }
  static tValueRetType GetValue(const Var& aVar) {
    return ni::QueryInterface<T>(aVar.GetIUnknownPointer());
  }
};

struct CollectionTraitsVec2i {
  enum { eType = eType_Vec2i };
  typedef ni::sVec2i tValueType;
  typedef ni::sVec2i tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsVec2iValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetVec2iValue();
  }
};
struct CollectionTraitsVec3i {
  enum { eType = eType_Vec3i };
  typedef ni::sVec3i tValueType;
  typedef ni::sVec3i tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsVec3iValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetVec3iValue();
  }
};
struct CollectionTraitsVec4i {
  enum { eType = eType_Vec4i };
  typedef ni::sVec4i tValueType;
  typedef ni::sVec4i tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsVec4iValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetVec4iValue();
  }
};

struct CollectionTraitsVec2f {
  enum { eType = eType_Vec2f };
  typedef ni::sVec2f tValueType;
  typedef ni::sVec2f tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsVec2fValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetVec2fValue();
  }
};
struct CollectionTraitsVec3f {
  enum { eType = eType_Vec3f };
  typedef ni::sVec3f tValueType;
  typedef ni::sVec3f tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsVec3fValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetVec3fValue();
  }
};
struct CollectionTraitsVec4f {
  enum { eType = eType_Vec4f };
  typedef ni::sVec4f tValueType;
  typedef ni::sVec4f tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsVec4fValue();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetVec4fValue();
  }
};

struct CollectionTraitsMatrixf {
  enum { eType = eType_Matrixf };
  typedef ni::sMatrixf tValueType;
  typedef ni::sMatrixf tValueRetType;
  static tBool IsValueType(const Var& aVar) {
    return aVar.IsMatrixf();
  }
  static tValueType GetValue(const Var& aVar) {
    return aVar.GetMatrixf();
  }
};

template <typename BASE>
struct CollectionTraitsVector : public astl::vector<typename BASE::tValueType>, public BASE {
  enum {
    eKeyType = eType_Null,
    eValType = BASE::eType,
    eModifyContainerInvalidatesAllIterators = ni::eTrue
  };
  typedef BASE tKeyTraits;
  typedef BASE tValTraits;
  typedef typename BASE::tValueType  tKeyType;
  typedef typename BASE::tValueType  tValType;
  typedef astl::vector<tValType>     tContainer;

  CollectionTraitsVector() {}
  CollectionTraitsVector(const tContainer& c) : tContainer(c) {}

  const tValType& at(tU32 anIndex) const { return static_cast<const tContainer*>(this)->at(anIndex); }
  tValType& operator[] (tU32 anIndex) { return static_cast<tContainer*>(this)->at(anIndex); }
  const tValType& operator[] (tU32 anIndex) const { return static_cast<const tContainer*>(this)->at(anIndex); }
  tValType& front() { return static_cast<tContainer*>(this)->front(); }
  const tValType& front() const { return static_cast<const tContainer*>(this)->front(); }
  tValType& back() { return static_cast<tContainer*>(this)->back(); }
  const tValType& back() const { return static_cast<const tContainer*>(this)->back(); }

  static typename tContainer::iterator Find(const tContainer& aContainer, const tValType& aVal) {
    const typename tContainer::const_iterator it = astl::find(aContainer,aVal);
    return (typename tContainer::iterator&)it;
  }
  static tBool Reserve(tContainer& aContainer, tU32 anNumElements) {
    aContainer.reserve(anNumElements);
    return eTrue;
  }
  static tBool Resize(tContainer& aContainer, tU32 anNumElements) {
    aContainer.resize(anNumElements);
    return eTrue;
  }
  static tBool __stdcall CanPut(const Var& aKey, const Var& aValue) {
    if (!BASE::IsValueType(aValue))
      return eFalse;
    if (!CollectionTraitsU32::IsValueType(aKey))
      return eFalse;
    return eTrue;
  }
  static tBool __stdcall Put(tContainer& aContainer, const Var& aKey, const Var& aValue) {
    if (!CanPut(aKey,aValue))
      return eFalse;
    const tU32 index = CollectionTraitsU32::GetValue(aKey);
    if (index >= aContainer.size())
      return eFalse;
    aContainer[index] = BASE::GetValue(aValue);
    return eTrue;
  }
  static Var __stdcall Get(const tContainer& aContainer, const Var& aKey) {
    if (!CollectionTraitsU32::IsValueType(aKey))
      return niVarNull;
    const tU32 index = CollectionTraitsU32::GetValue(aKey);
    if (index >= aContainer.size())
      return niVarNull;
    return Var(aContainer[index]);
  }
  static Var __stdcall GetFirst(const tContainer& aContainer) {
    if (aContainer.empty()) return niVarNull;
    return Var(aContainer.front());
  }
  static tBool __stdcall SetFirst(tContainer& aContainer, const Var& aValue) {
    if (aContainer.empty()) return eFalse;
    aContainer.front() = BASE::GetValue(aValue);
    return eTrue;
  }
  static Var __stdcall GetLast(const tContainer& aContainer) {
    if (aContainer.empty()) return niVarNull;
    return Var(aContainer.back());
  }
  static tBool __stdcall SetLast(tContainer& aContainer, const Var& aValue) {
    if (aContainer.empty()) return eFalse;
    aContainer.back() = BASE::GetValue(aValue);
    return eTrue;
  }
  static void __stdcall AddValue(tContainer& aContainer, const tValType& aV) {
    aContainer.push_back(aV);
  }
  static void __stdcall Add(tContainer& aContainer, const Var& aVar) {
    aContainer.push_back(BASE::GetValue(aVar));
  }
  static tBool __stdcall RemoveFirst(tContainer& aContainer) {
    if (aContainer.empty())
      return eFalse;
    aContainer.erase(aContainer.begin());
    return eTrue;
  }
  static tBool __stdcall RemoveLast(tContainer& aContainer) {
    if (aContainer.empty())
      return eFalse;
    aContainer.pop_back();
    return eTrue;
  }
  static tPtr __stdcall GetDataPtr(const tContainer& aContainer) {
    if (aContainer.empty()) return NULL;
    return (tPtr)aContainer.data();
  }
  static tSize __stdcall GetDataSize(const tContainer& aContainer) {
    return (ni::tSize)(aContainer.size() * sizeof(tValType));
  }
  static void GetIterator(const tContainer& aContainer,
                          const typename tContainer::const_iterator& aIt,
                          Var& aKey, Var& aVal)
  {
    aKey = astl::const_iterator_index(aContainer,aIt);
    aVal = *aIt;
  }
};

template <typename KEYT, typename VALT>
struct CollectionTraitsMap : public astl::map<typename KEYT::tValueType,typename VALT::tValueType>, public KEYT {
  enum {
    eKeyType = KEYT::eType,
    eValType = VALT::eType,
    eModifyContainerInvalidatesAllIterators = ni::eFalse
  };
  typedef KEYT tKeyTraits;
  typedef VALT tValTraits;
  typedef typename KEYT::tValueType  tKeyType;
  typedef typename VALT::tValueType  tValType;
  typedef astl::map<tKeyType,tValType> tContainer;

  CollectionTraitsMap() {}
  CollectionTraitsMap(const tContainer& c) : tContainer(c) {}

  tValType& operator[] (const tKeyType& aKey)       { return (*static_cast<tContainer*>(this))[aKey]; }
  const tValType& operator[] (const tKeyType& aKey) const { return (*static_cast<const tContainer*>(this))[aKey]; }

  static typename tContainer::iterator Find(const tContainer& aContainer, const tKeyType& aVal) {
    const typename tContainer::const_iterator it = aContainer.find(aVal);
    return (typename tContainer::iterator&)it;
  }
  static tBool Reserve(tContainer& aContainer, tU32 anNumElements) {
    return eFalse;
  }
  static tBool Resize(tContainer& aContainer, tU32 anNumElements) {
    return eFalse;
  }
  static tBool __stdcall CanPut(const Var& aKey, const Var& aValue) {
    if (!KEYT::IsValueType(aKey)) return eFalse;
    if (!VALT::IsValueType(aValue)) return eFalse;
    return eTrue;
  }
  static tBool __stdcall Put(tContainer& aContainer, const Var& aKey, const Var& aValue) {
    if (!CanPut(aKey,aValue)) return eFalse;
    astl::upsert(aContainer,KEYT::GetValue(aKey),VALT::GetValue(aValue));
    return eTrue;
  }
  static Var __stdcall Get(const tContainer& aContainer, const Var& aKey) {
    typename tContainer::iterator it = Find(aContainer,KEYT::GetValue(aKey));
    if (it == aContainer.end()) return niVarNull;
    return Var(it->second);
  }
  static Var __stdcall GetFirst(const tContainer& aContainer) {
    if (aContainer.empty()) return niVarNull;
    return Var(aContainer.begin()->second);
  }
  static tBool __stdcall SetFirst(tContainer& aContainer, const Var& aValue) {
    if (aContainer.empty()) return eFalse;
    aContainer.begin()->second = VALT::GetValue(aValue);
    return eTrue;
  }
  static Var __stdcall GetLast(const tContainer& aContainer) {
    if (aContainer.empty()) return niVarNull;
    return Var(aContainer.rbegin()->second);
  }
  static tBool __stdcall SetLast(tContainer& aContainer, const Var& aValue) {
    if (aContainer.empty()) return eFalse;
    aContainer.rbegin()->second = VALT::GetValue(aValue);
    return eTrue;
  }
  static void __stdcall AddValue(tContainer& aContainer, const tValType& aV) {
    astl::pair<tKeyType,tValType> myPair;
    memset(&myPair,0,sizeof(myPair));
    myPair.first = aV;
    aContainer.insert(myPair);
  }
  static void __stdcall Add(tContainer& aContainer, const Var& aVar) {
    astl::pair<tKeyType,tValType> myPair;
    memset(&myPair,0,sizeof(myPair));
    myPair.first = KEYT::GetValue(aVar);
    aContainer.insert(myPair);
  }
  static tBool __stdcall RemoveFirst(tContainer& aContainer) {
    if (aContainer.empty())
      return eFalse;
    aContainer.erase(aContainer.begin());
    return eTrue;
  }
  static tBool __stdcall RemoveLast(tContainer& aContainer) {
    if (aContainer.empty())
      return eFalse;
    aContainer.erase(aContainer.rbegin()->first);
    return eTrue;
  }
  static void GetIterator(const tContainer& aContainer,
                          const typename tContainer::const_iterator& aIt,
                          Var& aKey, Var& aVal)
  {
    if (aIt == aContainer.end()) return;
    aKey = aIt->first;
    aVal = aIt->second;
  }
  static tPtr __stdcall GetDataPtr(const tContainer& aContainer) {
    return NULL;
  }
  static tSize __stdcall GetDataSize(const tContainer& aContainer) {
    return 0;
  }
};

template <typename TRAITS, typename BASEIMPL=ImplRC<iCollection> >
class cCollectionImpl : public TRAITS, public BASEIMPL {
  niClassNoCopyAssign(cCollectionImpl);

 public:
  typedef cCollectionImpl<TRAITS,ImplRC<iCollection> > tImmutable;
  typedef cCollectionImpl<TRAITS,BASEIMPL>                    tThisImmutable;
  typedef typename TRAITS::tKeyTraits                   tKeyTraits;
  typedef typename TRAITS::tValTraits                   tValTraits;
  typedef typename TRAITS::tValueType                   tKeyType;
  typedef typename TRAITS::tValueType                   tValType;
  typedef typename TRAITS::tContainer                   tContainer;
  typedef typename tContainer::const_iterator           const_iterator;
  typedef typename tContainer::const_reverse_iterator   const_reverse_iterator;

  struct sIterator : public ImplRC<iIterator> {
    Ptr<tThisImmutable> mptrContainer;
    const_iterator   mIt;
    Var              mKey, mVal;
    tBool            mbAtNext;

    sIterator(tThisImmutable* apContainer, const_iterator aIt) {
      niAssert(niIsOK(apContainer));
      mptrContainer = apContainer;
      mIt = aIt;
      mKey = mVal = niVarNull;
      mbAtNext = eFalse;
      if (mIt != _GetUncheckedContainerBase().end()) {
        TRAITS::GetIterator(_GetUncheckedContainerBase(),mIt,mKey,mVal);
      }
    }
    ~sIterator() {
      Invalidate();
    }

    const tContainer& _GetUncheckedContainerBase() const {
      niAssert(mptrContainer.IsOK());
      return mptrContainer.raw_ptr()->_Base();
    }

    virtual void __stdcall Invalidate() {
      if (mptrContainer.IsOK()) {
        mIt = _GetUncheckedContainerBase().end();
        mKey = mVal = niVarNull;
        mptrContainer = NULL;
      }
    }

    void __stdcall _WeakInvalidate() {
      if (mIt != _GetUncheckedContainerBase().end()) {
        ++mIt;
        mbAtNext = eTrue;
      }
    }

    virtual tBool __stdcall IsOK() const {
      return mptrContainer.IsOK();
    }

    virtual iCollection* __stdcall GetCollection() const {
      return mptrContainer.raw_ptr();
    }

    virtual tBool __stdcall HasNext() const {
      return (mptrContainer.IsOK() && mIt != _GetUncheckedContainerBase().end());
    }

    virtual const Var& __stdcall Next() {
      niAssert(HasNext()); // properly behaved iteration should not call
      // next if there is no next value...
      if (!HasNext()) {
        mKey = mVal = niVarNull;
      }
      else {
        if (mbAtNext) {
          mbAtNext = eFalse;
        }
        else {
          ++mIt;
        }
        if (!HasNext()) {
          mKey = mVal = niVarNull;
        }
        else {
          TRAITS::GetIterator(_GetUncheckedContainerBase(),mIt,mKey,mVal);
        }
      }
      return mVal;
    }
    virtual const Var& __stdcall Key() const {
      return mKey;
    }
    virtual const Var& __stdcall Value() const {
      return mVal;
    }

    tKeyType _Key() const {
      niAssert(HasNext());
      return tKeyTraits::GetValue(mKey);
    }
    tValType _Value() const {
      niAssert(HasNext());
      return tValTraits::GetValue(mVal);
    }
    tValType _Next() {
      niAssert(HasNext());
      return tKeyTraits::GetValue(Next());
    }
  };

 protected:
  cCollectionImpl() {}
  cCollectionImpl(const tContainer& aContainer) : TRAITS(aContainer) {
  }

 public:
  static tImmutable* Create() {
    return niNew tImmutable();
  }
  static tImmutable* Create(const tContainer& aContainer) {
    return niNew tImmutable(aContainer);
  }
  static tImmutable* __stdcall Add(tImmutable* apColl, const Var& avVar) {
    if (!TRAITS::IsValueType(avVar))
      return apColl;
    Ptr<tImmutable> newColl = (apColl == NULL) ? Create() : Create(*apColl);
    TRAITS::Add(*newColl.raw_ptr(),avVar);
    return newColl.GetRawAndSetNull();
  }
  static tImmutable* __stdcall Remove(tImmutable* apColl, const Var& avVar, tBool* apDidContain = NULL) {
    if (!apColl)
      return NULL;
    // Check whether the value is in the collection, if not return the collection
    tBool contains = apColl->Contains(avVar);
    if (apDidContain)
      *apDidContain = contains;
    if (!contains)
      return apColl;
    // Create a copy of the collection and remove the object from it
    Ptr<tImmutable> newColl = Create(*apColl);
    tKeyType v = TRAITS::GetValue(avVar);
    typename tContainer::iterator it = TRAITS::Find(newColl.raw_ptr()->_Base(),v);
    niAssert(it != newColl.raw_ptr()->_Base().end());
    newColl.raw_ptr()->erase(it);
    // If the new collection is empty, return NULL
    if (newColl.raw_ptr()->empty())
      return NULL;
    return newColl.GetRawAndSetNull();
  }
  static tImmutable* __stdcall Put(tImmutable* apColl, const Var& aKey, const Var& aVal) {
    if (!TRAITS::CanPut(aKey,aVal))
      return apColl;
    Ptr<tImmutable> newColl = (apColl == NULL) ? Create() : Create(*apColl);
    TRAITS::Put(*newColl,aKey,aVal);
    return newColl.GetRawAndSetNull();
  }

  tType __stdcall GetKeyType() const {
    return TRAITS::eKeyType;
  }
  tType __stdcall GetValueType() const {
    return TRAITS::eValType;
  }

  tBool __stdcall IsEmpty() const {
    return !!this->_Base().empty();
  }

  tU32 __stdcall GetSize() const {
    return (tU32)this->_Base().size();
  }

  tBool __stdcall ContainsAll(const iCollection* apCollection) const {
    if (!niIsOK(apCollection)) return eFalse;
    Nonnull<iIterator> it{apCollection->Iterator()};
    while (it->HasNext()) {
      if (!this->Contains(it->Next()))
        return eFalse;
    }
    return eTrue;
  }

  sIterator* __stdcall _ImmutableIterator(const_iterator it) const {
    return niNew sIterator(niThis(tThisImmutable),it);
  }
  sIterator* __stdcall _ImmutableIterator() const {
    return niNew sIterator(niThis(tThisImmutable),niThis(tThisImmutable)->_Base().begin());
  }
  virtual iIterator* __stdcall Iterator(const_iterator it) const {
    return this->_ImmutableIterator(it);
  }
  virtual iIterator* __stdcall Iterator() const {
    return this->_ImmutableIterator(this->_Base().begin());
  }

  tBool __stdcall Contains(const Var& avVar) const {
    tKeyType v = TRAITS::GetValue(avVar);
    typename tContainer::const_iterator it = TRAITS::Find(this->_Base(),v);
    return !!(it != this->_Base().end());
  }

  iIterator* __stdcall Find(const Var& avVar) const {
    tKeyType v = TRAITS::GetValue(avVar);
    typename tContainer::const_iterator it = TRAITS::Find(this->_Base(),v);
    if (it == this->_Base().end())
      return NULL;
    return niThis(tThisImmutable)->Iterator(it);
  }

  Var __stdcall Get(const Var& aKey) const {
    return TRAITS::Get(this->_Base(),aKey);
  }

  Var __stdcall GetFirst() const {
    return TRAITS::GetFirst(this->_Base());
  }
  Var __stdcall GetLast() const {
    return TRAITS::GetLast(this->_Base());
  }

  static tBool __stdcall IsSameType(const iCollection* apCollection) {
    niAssert(niIsOK(apCollection));
    if (!apCollection)
      return eFalse;
    return (apCollection->GetKeyType() == TRAITS::eKeyType) &&
        (apCollection->GetValueType() == TRAITS::eValType);
  }

  bool empty() const { return this->_Base().empty(); }
  tU32 size() const { return (tU32)this->_Base().size(); }
  const_iterator find(const tValType& aVal) const {
    return TRAITS::Find(this->_Base(),aVal);
  }

  const_iterator begin() const { return this->_Base().begin(); }
  const_iterator end() const { return this->_Base().end(); }
  const_reverse_iterator rbegin() const { return this->_Base().rbegin(); }
  const_reverse_iterator rend() const { return this->_Base().rend(); }

  //! \remark Assumes that the underlying container is read only and won't be modified.
  __forceinline const tContainer& GetUnderlyingContainer() const {
    return *this;
  }

 protected:
  template <typename S, typename T>
  friend class cMutableCollectionImpl;

  __forceinline const tContainer& _Base() const { return *this; }
};

template <typename TRAITS,
          typename IMMIMPL=cCollectionImpl<
            TRAITS,ImplRC<iMutableCollection,eImplFlags_DontInherit1,iCollection> > >
class cMutableCollectionImpl : public IMMIMPL {
  niClassNoCopyAssign(cMutableCollectionImpl);

 public:
  typedef cMutableCollectionImpl<TRAITS,IMMIMPL>        tMutable;
  typedef typename IMMIMPL::tImmutable                  tImmutable;
  typedef typename IMMIMPL::tKeyTraits                  tKeyTraits;
  typedef typename IMMIMPL::tValTraits                  tValTraits;
  typedef typename IMMIMPL::tKeyType                    tKeyType;
  typedef typename IMMIMPL::tValType                    tValType;
  typedef typename IMMIMPL::tContainer                  tContainer;
  typedef typename IMMIMPL::const_iterator              const_iterator;
  typedef typename IMMIMPL::const_reverse_iterator      const_reverse_iterator;
  typedef typename tContainer::iterator                 iterator;
  typedef typename tContainer::reverse_iterator         reverse_iterator;

  struct sIterator : public ImplRC<iIterator> {
    Ptr<tMutable> mptrContainer;
    sIterator* mpNext;
    iterator mIt;
    Var mKey, mVal;
    tBool mbAtNext;

    sIterator(tMutable* apContainer, iterator aIt) {
      niAssert(niIsOK(apContainer));
      mptrContainer = apContainer;
      mptrContainer.raw_ptr()->_RegisterIterator(this);
      mIt = aIt;
      mKey = mVal = niVarNull;
      mbAtNext = eFalse;
      if (mIt != _GetUncheckedContainerBase().end()) {
        TRAITS::GetIterator(_GetUncheckedContainerBase(),mIt,mKey,mVal);
      }
    }
    ~sIterator() {
      Invalidate();
    }

    tContainer& _GetUncheckedContainerBase() {
      niAssert(mptrContainer.IsOK());
      return mptrContainer.raw_ptr()->_Base();
    }
    const tContainer& _GetUncheckedContainerBase() const {
      niAssert(mptrContainer.IsOK());
      return mptrContainer.raw_ptr()->_Base();
    }

    virtual void __stdcall Invalidate() {
      if (mptrContainer.IsOK()) {
        mIt = _GetUncheckedContainerBase().end();
        mKey = mVal = niVarNull;
        mptrContainer.raw_ptr()->_UnregisterIterator(this);
        mptrContainer = NULL;
      }
    }

    void __stdcall _WeakInvalidate() {
      if (mIt != _GetUncheckedContainerBase().end()) {
        ++mIt;
        mbAtNext = eTrue;
      }
    }

    virtual tBool __stdcall IsOK() const {
      return mptrContainer.IsOK();
    }

    virtual iCollection* __stdcall GetCollection() const {
      return mptrContainer.raw_ptr();
    }

    virtual tBool __stdcall HasNext() const {
      return (mptrContainer.IsOK() && mIt != _GetUncheckedContainerBase().end());
    }

    virtual const Var& __stdcall Next() {
      if (!HasNext()) {
        mKey = mVal = niVarNull;
      }
      else {
        if (mbAtNext) {
          mbAtNext = eFalse;
        }
        else {
          ++mIt;
        }
        if (!HasNext()) {
          mKey = mVal = niVarNull;
        }
        else {
          TRAITS::GetIterator(_GetUncheckedContainerBase(),mIt,mKey,mVal);
        }
      }
      return mVal;
    }
    virtual const Var& __stdcall Key() const {
      return mKey;
    }
    virtual const Var& __stdcall Value() const {
      return mVal;
    }

    tKeyType _Key() const {
      niAssert(HasNext());
      return tKeyTraits::GetValue(mKey);
    }
    tValType _Value() const {
      niAssert(HasNext());
      return tValTraits::GetValue(mVal);
    }
    tValType _Next() {
      niAssert(HasNext());
      return tKeyTraits::GetValue(Next());
    }
  };

 protected:
  cMutableCollectionImpl() {}
  cMutableCollectionImpl(const tContainer& aContainer) {
    this->_Base() = aContainer;
  }

 public:
  static cMutableCollectionImpl* Create() {
    return niNew tMutable();
  }
  static cMutableCollectionImpl* Create(const tContainer& aContainer) {
    return niNew tMutable(aContainer);
  }
  static cMutableCollectionImpl* Create(const tImmutable* apCollection) {
    niCAssert(sizeof(typename tImmutable::tContainer) == sizeof(tContainer));
    return niNew tMutable(apCollection->_Base());
  }

  tBool __stdcall Reserve(tU32 anSize) {
    return TRAITS::Reserve(this->_Base(),anSize);
  }

  tBool __stdcall Resize(tU32 anSize) {
    return TRAITS::Resize(this->_Base(),anSize);
  }

  tBool __stdcall Clear() {
    _InvalidateAllIterators();
    this->_Base().clear();
    return eTrue;
  }

  tBool __stdcall Copy(const iCollection* apCollection) {
    if (!niIsOK(apCollection))
      return eFalse;
    if (!tMutable::IsSameType(apCollection))
      return eFalse;
    this->Clear();
    this->_Base() = ((tMutable*)(apCollection))->_Base();
    return eTrue;
  }

  tBool __stdcall Add(const Var& aVar) {
    if (!TRAITS::IsValueType(aVar)) return eFalse;
    if (TRAITS::eModifyContainerInvalidatesAllIterators) {
      _InvalidateAllIterators();
    }
    TRAITS::Add(this->_Base(),aVar);
    return eTrue;
  }

  tBool __stdcall AddAll(const iCollection* apCollection) {
    if (!niIsOK(apCollection)) return eFalse;
    Nonnull<iIterator> it{apCollection->Iterator()};
    while (it->HasNext()) {
      if (!this->Add(it->Next()))
        return eFalse;
    }
    return eTrue;
  }

  tBool __stdcall Remove(const Var& aVar) {
    if (!TRAITS::IsValueType(aVar)) return eFalse;
    tKeyType v = TRAITS::GetValue(aVar);
    iterator it = TRAITS::Find(this->_Base(),v);
    if (it == this->_Base().end())
      return eFalse;
    _InvalidateIterator(it,eFalse);
    this->_Base().erase(it);
    return eTrue;
  }

  tBool __stdcall RemoveAll(const iCollection* apCollection) {
    if (!niIsOK(apCollection)) return eFalse;
    Nonnull<iIterator> it{apCollection->Iterator()};
    while (it->HasNext()) {
      this->Remove(it->Next());
    }
    return eTrue;
  }

  tBool __stdcall RemoveIterator(iIterator* apIt) {
    if (!niIsOK(apIt))
      return eFalse;
    Ptr<iIterator> ptrIt = apIt;
    niAssert(apIt->GetCollection() == this);
    if (apIt->GetCollection() != this)
      return eFalse;
    iterator it = ((sIterator*)apIt)->mIt;
    _InvalidateIterator(it,eTrue);
    this->_Base().erase(it);
    return eTrue;
  }

  tBool __stdcall RemoveFirst() {
    return TRAITS::RemoveFirst(this->_Base());
  }

  tBool __stdcall RemoveLast() {
    return TRAITS::RemoveLast(this->_Base());
  }

  sIterator* __stdcall _MutableIterator(const_iterator it) const {
    return niNew sIterator(niThis(tMutable),(iterator&)it);
  }
  sIterator* __stdcall _MutableIterator() const {
    return niNew sIterator(niThis(tMutable),niThis(tMutable)->_Base().begin());
  }
  virtual iIterator* __stdcall Iterator(const_iterator it) const {
    return this->_MutableIterator(it);
  }
  virtual iIterator* __stdcall Iterator() const {
    return this->_MutableIterator(this->_Base().begin());
  }

  virtual tBool __stdcall Put(const Var& aKey, const Var& aValue) {
    return TRAITS::Put(this->_Base(),aKey,aValue);
  }

  virtual tBool __stdcall SetFirst(const Var& aValue) {
    return TRAITS::SetFirst(this->_Base(),aValue);
  }

  virtual tBool __stdcall SetLast(const Var& aValue) {
    return TRAITS::SetLast(this->_Base(),aValue);
  }

  tPtr __stdcall GetDataPtr() const {
    return TRAITS::GetDataPtr(this->_Base());
  }
  tSize __stdcall GetDataSize() const {
    return TRAITS::GetDataSize(this->_Base());
  }
  const tValType* __stdcall _Data() const {
    niAssert(GetDataPtr() != NULL);
    return (tValType*)GetDataPtr();
  }
  tValType* __stdcall _Data() {
    niAssert(GetDataPtr() != NULL);
    return (tValType*)GetDataPtr();
  }

  inline void __stdcall reserve(size_t newSize) {
    this->Reserve((tU32)newSize);
  }
  inline void __stdcall resize(size_t newSize) {
    this->Resize((tU32)newSize);
  }
  inline void __stdcall clear() {
    this->Clear();
  }
  inline void __stdcall push_back(const tValType& aV) {
    if (TRAITS::eModifyContainerInvalidatesAllIterators) {
      _InvalidateAllIterators();
    }
    TRAITS::AddValue(this->_Base(),aV);
  }

  bool empty() const { return this->_Base().empty(); }
  tU32 size() const { return (tU32)this->_Base().size(); }

  iterator begin() { return this->_Base().begin(); }
  iterator end() { return this->_Base().end(); }
  reverse_iterator rbegin() { return this->_Base().rbegin(); }
  reverse_iterator rend() { return this->_Base().rend(); }

  const_iterator begin() const { return this->_Base().begin(); }
  const_iterator end() const { return this->_Base().end(); }
  const_reverse_iterator rbegin() const { return this->_Base().rbegin(); }
  const_reverse_iterator rend() const { return this->_Base().rend(); }

  iterator find(const tValType& aVal) {
    return TRAITS::Find(this->_Base(),aVal);
  }
  const_iterator find(const tValType& aVal) const {
    return TRAITS::Find(this->_Base(),aVal);
  }

  //! \remark Assumes that the underlying container will be modified, so
  //!         invalidates the iterators accordingly.
  __forceinline tContainer& GetUnderlyingContainer() {
    if (TRAITS::eModifyContainerInvalidatesAllIterators) {
      _InvalidateAllIterators();
    }
    return *this;
  }

 private:
  __forceinline tContainer& _Base() {
    return *this;
  }
  __forceinline const tContainer& _Base() const {
    return *this;
  }

  void _InvalidateIterator(iterator aIt, tBool abAssert) {
    if (TRAITS::eModifyContainerInvalidatesAllIterators) {
      this->_InvalidateAllIterators();
    }
    else {
      tU32 numInvalidate = 0;
      sIterator* it = mpFirstIterator;
      while (it) {
        if (it->IsOK()) {
          if (aIt == it->mIt) {
            it->_WeakInvalidate();
            ++numInvalidate;
          }
        }
        it = it->mpNext;
      }
      if (abAssert) {
        niUnused(numInvalidate);
        niAssert(numInvalidate != 0);
      }
    }
  }
  void _InvalidateIterator(const_iterator aIt, tBool abAssert) {
    _InvalidateIterator((iterator&)aIt,abAssert);
  }

  void _InvalidateAllIterators() {
    while (mpFirstIterator) {
      mpFirstIterator->Invalidate();
    }
  }

  void _RegisterIterator(sIterator* apIt) {
    // niDebugFmt(("_RegisterIterator: %p\n",(void*)apIt));
    if (mpFirstIterator) {
      mpFirstIterator->mpNext = apIt;
    }
    else {
      mpFirstIterator = apIt;
    }
  }
  void _UnregisterIterator(sIterator* apIt) {
    // niDebugFmt(("_UnregisterIterator: %p\n",(void*)apIt));
    niAssert(mpFirstIterator != NULL);
    if (!mpFirstIterator) return;
    sIterator* prev = NULL;
    sIterator* it = mpFirstIterator;
    while (it != NULL) {
      if (it == apIt) {
        if (prev) {
          prev->mpNext = it->mpNext;
        }
        else {
          mpFirstIterator = it->mpNext;
        }
        return;
      }
      prev = it;
      it = it->mpNext;
    }
    niAssertUnreachable("Removing iterator from another collection.");
  }

  sIterator* mpFirstIterator;
};

// Standard Vector Collections
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsI8> > tI8CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsU8> > tU8CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsI16> > tI16CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsU16> > tU16CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsI32> > tI32CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsU32> > tU32CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsF32> > tF32CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsF64> > tF64CVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsVec2i> > tVec2iCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsVec2f> > tVec2fCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsVec3i> > tVec3iCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsVec3f> > tVec3fCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsVec4i> > tVec4iCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsVec4f> > tVec4fCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsMatrixf> > tMatrixfCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsIUnknown> > tIUnknownCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsString> > tStringCVec;
typedef ni::cMutableCollectionImpl<ni::CollectionTraitsVector<ni::CollectionTraitsUUID> > tUUIDCVec;

//! Create a collection to store the specified type of elements
static inline ni::iMutableCollection* __stdcall CreateCollectionVector(tType aType) {
  switch (niType(aType)) {
    case eType_I8: return tI8CVec::Create();
    case eType_U8: return tU8CVec::Create();
    case eType_I16: return tI16CVec::Create();
    case eType_U16: return tU16CVec::Create();
    case eType_I32: return tI32CVec::Create();
    case eType_U32: return tU32CVec::Create();
    case eType_F32: return tF32CVec::Create();
    case eType_F64: return tF64CVec::Create();
    case eType_Vec2i: return tVec2iCVec::Create();
    case eType_Vec2f: return tVec2fCVec::Create();
    case eType_Vec3i: return tVec3iCVec::Create();
    case eType_Vec3f: return tVec3fCVec::Create();
    case eType_Vec4i: return tVec4iCVec::Create();
    case eType_Vec4f: return tVec4fCVec::Create();
    case eType_Matrixf: return tMatrixfCVec::Create();
    case eType_IUnknown: return tIUnknownCVec::Create();
    case eType_String: return tStringCVec::Create();
    case eType_UUID: return tUUIDCVec::Create();
    default:
      return NULL;
  }
}

// Standard Map Collections
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsI32,ni::CollectionTraitsI32> >
tI32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsI32,ni::CollectionTraitsU32> >
tI32U32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsI32,ni::CollectionTraitsIUnknown> >
tI32IUnknownCMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsI32,ni::CollectionTraitsString> >
tI32StringCMap;

typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsU32,ni::CollectionTraitsI32> >
tU32I32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsU32,ni::CollectionTraitsU32> >
tU32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsU32,ni::CollectionTraitsIUnknown> >
tU32IUnknownCMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsU32,ni::CollectionTraitsString> >
tU32StringCMap;

typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsIUnknown,ni::CollectionTraitsI32> >
tIUnknownI32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsIUnknown,ni::CollectionTraitsU32> >
tIUnknownU32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsIUnknown,ni::CollectionTraitsIUnknown> >
tIUnknownCMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsIUnknown,ni::CollectionTraitsString> >
tIUnknownStringCMap;

typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsString,ni::CollectionTraitsI32> >
tStringI32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsString,ni::CollectionTraitsU32> >
tStringU32CMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsString,ni::CollectionTraitsIUnknown> >
tStringIUnknownCMap;
typedef ni::cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsString,ni::CollectionTraitsString> >
tStringCMap;

//! Create a collection to store the specified type of elements
static inline ni::iMutableCollection* __stdcall CreateCollectionMap(tType aKeyType, tType aValType) {
  switch (niType(aKeyType)|(niType(aValType)<<16)) {
    case (eType_I32|(eType_I32<<16)):      return tI32CMap::Create();
    case (eType_I32|(eType_U32<<16)):      return tI32U32CMap::Create();
    case (eType_I32|(eType_IUnknown<<16)): return tI32IUnknownCMap::Create();
    case (eType_I32|(eType_String<<16)):   return tI32StringCMap::Create();
    case (eType_U32|(eType_I32<<16)):      return tU32I32CMap::Create();
    case (eType_U32|(eType_U32<<16)):      return tU32CMap::Create();
    case (eType_U32|(eType_IUnknown<<16)): return tU32IUnknownCMap::Create();
    case (eType_U32|(eType_String<<16)):   return tU32StringCMap::Create();
    case (eType_IUnknown|(eType_I32<<16)):      return tIUnknownI32CMap::Create();
    case (eType_IUnknown|(eType_U32<<16)):      return tIUnknownU32CMap::Create();
    case (eType_IUnknown|(eType_IUnknown<<16)): return tIUnknownCMap::Create();
    case (eType_IUnknown|(eType_String<<16)):   return tIUnknownStringCMap::Create();
    case (eType_String|(eType_I32<<16)):      return tStringI32CMap::Create();
    case (eType_String|(eType_U32<<16)):      return tStringU32CMap::Create();
    case (eType_String|(eType_IUnknown<<16)): return tStringIUnknownCMap::Create();
    case (eType_String|(eType_String<<16)):   return tStringCMap::Create();
  }
  return NULL;
};

// Interface CVec template
template <typename T>
struct tInterfaceCVec : public cMutableCollectionImpl<
  ni::CollectionTraitsVector<ni::CollectionTraitsInterface<T> > >
{
  niClassNoCopyAssign(tInterfaceCVec);

 private:
  tInterfaceCVec() {}

 public:
  static tInterfaceCVec<T>* Create() {
    return niNew tInterfaceCVec<T>();
  }
};

// Interface CMap template
template <typename T>
struct tStringInterfaceCMap : public cMutableCollectionImpl<
  ni::CollectionTraitsMap<ni::CollectionTraitsString,ni::CollectionTraitsInterface<T> > >
{
  niClassNoCopyAssign(tStringInterfaceCMap);

 private:
  tStringInterfaceCMap() {}

 public:
  static tStringInterfaceCMap<T>* Create() {
    return niNew tStringInterfaceCMap<T>();
  }
};

template <typename TTo, typename TFrom>
TTo ptr_cast(Ptr<TFrom>& v) {
  return static_cast<TTo>(v.ptr());
}
template <typename TTo, typename TFrom>
TTo ptr_cast(TFrom v) {
  return static_cast<TTo>(v);
}

#define niLoopIterator(NAME,C)                        \
  if (!niIsOK(C) || (C)->IsEmpty()) {} else           \
    for (ni::Nonnull<ni::iIterator> NAME{C->Iterator()}; \
         NAME->HasNext(); NAME->Next())

#define niLoopImmutableIterator(TYPE,NAME,C)                      \
  if (!niIsOK(C) || ni::ptr_cast<TYPE*>(C)->empty()) {} else      \
    for (ni::Nonnull<TYPE::sIterator> NAME{C->_ImmutableIterator()}; \
         NAME->HasNext(); NAME->Next())

#define niLoopMutableIterator(TYPE,NAME,C)                      \
  if (!niIsOK(C) || ni::ptr_cast<TYPE*>(C)->empty()) {} else    \
    for (ni::Nonnull<TYPE::sIterator> NAME{C->_MutableIterator()}; \
         NAME->HasNext(); NAME->Next())

template <typename T>
T* QueryCollection(const iUnknown* apObj) {
  ni::iCollection* col = ni::QueryInterface<ni::iCollection>(apObj);
  if (!col)
    return NULL;
  if (!T::IsSameType(col))
    return NULL;
  return (T*)col;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __COLLECTIONIMPL_H_604D0B86_1048_4125_A19E_CCAAE861924F__
