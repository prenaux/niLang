#ifndef __SINKLIST_H_BED500D1_D4C7_467E_A9CA_CA601C0F11BA__
#define __SINKLIST_H_BED500D1_D4C7_467E_A9CA_CA601C0F11BA__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../ISinkList.h"
#include "../Utils/CollectionImpl.h"
#include "Nonnull.h"
#include "Sync.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

template <typename T>
struct SinkList : public ImplRC<iSinkList,eImplFlags_DontInherit1,iCollection>
{
  niClassNoCopyAssign(SinkList);

 private:
  SinkList()
      : mbMute(eFalse)
      , mCollection(EMPTY())
  {}

 public:
  typedef SinkList<T> tThis;
  typedef typename cCollectionImpl<
    ni::CollectionTraitsVector<ni::CollectionTraitsInterface<T> > >::tImmutable
  tImmutableCollection;
  typedef typename tImmutableCollection::sIterator              sIterator;
  typedef typename tImmutableCollection::const_iterator         const_iterator;
  typedef typename tImmutableCollection::const_reverse_iterator const_reverse_iterator;

  static SinkList<T>* Create() {
    return niNew SinkList<T>();
  }

  const tUUID& __stdcall GetSinkUUID() const niOverride {
    return niGetInterfaceUUID(T);
  }

  void __stdcall Clear() niOverride {
    __sync_lock();
    mCollection = EMPTY();
  }

  tBool __stdcall HasSink(iUnknown* apSink) niImpl {
    __sync_lock();
    Ptr<T> ptr = (T*)apSink; // we can use an unsafe case because we're not
                             // going to do anything beside what is in iUnknown
    return (mCollection->find(ptr) != mCollection->end());
  }

  tBool __stdcall AddSink(T* apSink) {
    niCheckSilent(niIsOK(apSink),eFalse);
    __sync_lock();
    Ptr<T> sink = apSink;
    // already in the list ?
    if (mCollection->find(apSink) != mCollection->end())
      return eFalse;
    // add to the back of the list
    mCollection = ni::Nonnull<tImmutableCollection>{
        tImmutableCollection::Add(mCollection, apSink)};
    return eTrue;
  }
  tBool __stdcall AddSink(iUnknown* apSink) niOverride {
    Ptr<T> pISink = ni::QueryInterface<T>(apSink);
    return AddSink(pISink.raw_ptr());
  }

  tBool __stdcall AddFrontSink(T* apSink) {
    niCheckSilent(niIsOK(apSink),eFalse);
    __sync_lock();
    Ptr<T> sink = apSink;
    // already in the list ?
    if (mCollection->find(apSink) != mCollection->end())
      return eFalse;
    // add to the front of the list
    typename tImmutableCollection::tContainer newContainer = mCollection->GetUnderlyingContainer();
    newContainer.insert(newContainer.begin(),apSink);
    mCollection = ni::Nonnull<tImmutableCollection>{
        tImmutableCollection::Create(newContainer)};
    return eTrue;
  }
  tBool __stdcall AddFrontSink(iUnknown* apSink) niImpl {
    Ptr<T> pISink = ni::QueryInterface<T>(apSink);
    return AddFrontSink(pISink.raw_ptr());
  }

  tBool __stdcall RemoveSink(T* apSink) {
    if (IsEmpty())
      return eFalse;
    tBool bContains = HasSink(apSink);
    if (bContains) {
      __sync_lock();
      Ptr<T> sinkRef = apSink;
      auto newColl = tImmutableCollection::Remove(mCollection,apSink);
      mCollection = ni::IsOK(newColl)
                        ? ni::Nonnull<tImmutableCollection>{newColl}
                        : EMPTY();
    }
    return bContains;
  }
  tBool __stdcall RemoveSink(iUnknown* apSink) niImpl {
    Ptr<T> pISink = ni::QueryInterface<T>(apSink);
    return RemoveSink(pISink.raw_ptr());
  }

  void __stdcall SetMute(tBool abMute) niImpl {
    mbMute = abMute;
  }
  tBool __stdcall GetMute() const niImpl {
    return mbMute;
  }

  virtual tType __stdcall GetKeyType() const niImpl {
    return eType_Null;
  }
  virtual tType __stdcall GetValueType() const niImpl {
    return eType_IUnknown;
  }

  virtual tBool __stdcall IsEmpty() const niImpl {
    __sync_lock();
    return mCollection->IsEmpty();
  }

  virtual tU32 __stdcall GetSize() const niImpl {
    __sync_lock();
    return mCollection->GetSize();
  }

  virtual tBool __stdcall Contains(const Var& aVar) const niImpl {
    __sync_lock();
    return mCollection->Contains(aVar);
  }
  virtual tBool __stdcall ContainsAll(const iCollection* apCollection) const niImpl {
    __sync_lock();
    return mCollection->ContainsAll(apCollection);
  }

  virtual iIterator* __stdcall Iterator() const niImpl {
    __sync_lock();
    return mCollection->Iterator();
  }
  virtual iIterator* __stdcall Find(const Var& aVar) const niImpl {
    __sync_lock();
    return mCollection->Find(aVar);
  }

  virtual Var __stdcall Get(const Var& aKey) const niImpl {
    __sync_lock();
    return mCollection->Get(aKey);
  }

  virtual Var __stdcall GetFirst() const niImpl {
    __sync_lock();
    return mCollection->GetFirst();
  }

  virtual Var __stdcall GetLast() const niImpl {
    __sync_lock();
    return mCollection->GetLast();
  }

  sIterator* __stdcall _ImmutableIterator() const {
    __sync_lock();
    return mCollection->_ImmutableIterator();
  }

  Ptr<tImmutableCollection> __stdcall _ImmutableCollection() const {
    __sync_lock();
    return mCollection;
  }

  bool empty() const { return this->IsEmpty(); }
  size_t size() const { return this->GetSize(); }

 private:
  __sync_mutex();

  static Nonnull<tImmutableCollection>& EMPTY() {
    static Nonnull<tImmutableCollection> _EMPTY{tImmutableCollection::Create()};
    return _EMPTY;
  }

  Nonnull<tImmutableCollection> mCollection;
  tBool                         mbMute;
};

#define niLoopSink(TYPE,NAME,C)                                         \
  if ((!niIsOK(C)) || (C)->IsEmpty() || (C)->GetMute()) {} else         \
    for (ni::Nonnull<ni::SinkList<TYPE>::sIterator> NAME((C)->_ImmutableIterator()); \
         NAME->HasNext(); NAME->Next())

#define niCallSinkVoid_(NAME,SINK,METH,PARAMS)  \
  niLoopSink(i##NAME,__sinkIt,SINK) {           \
    i##NAME* pSink = __sinkIt->_Value();        \
    pSink->On##NAME##_##METH PARAMS;            \
  }

#define niCallSinkRetTest_(NAME,SINK,METH,PARAMS,RET,RETTEST) \
  niLoopSink(i##NAME,__sinkIt,SINK) {                         \
    i##NAME* pSink = __sinkIt->_Value();                      \
    (RET) = pSink->On##NAME##_##METH PARAMS;                  \
    if (!(RETTEST)) break;                                    \
  }

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __SINKLIST_H_BED500D1_D4C7_467E_A9CA_CA601C0F11BA__
