#ifndef _SQREFCOUNTED_H_
#define _SQREFCOUNTED_H_

#include "sqalloc.h"
#include <niLang/Utils/Sync.h>

struct SQCollectable;

namespace ni {
struct iCollectable : public iUnknown
{
  niDeclareInterfaceUUID(iCollectable,0x9ac9f3b1,0xb738,0x419a,0xaf,0x44,0xc5,0x4a,0x9c,0x13,0xc6,0x3a);
  virtual SQCollectable* __stdcall GetCollectableStructPtr() = 0;
#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable **chain) = 0;
#endif
};
}

#if 1
#define SQREF_INT_TYPE SYNC_INT_TYPE
#define SQREF_READ SYNC_READ
#define SQREF_WRITE SYNC_WRITE
#define SQREF_INCREMENT SYNC_INCREMENT
#define SQREF_DECREMENT SYNC_DECREMENT
#else
#define SQREF_INT_TYPE ni::tI32
#define SQREF_READ(addr) (*(addr))
#define SQREF_WRITE(addr, value) (*(addr)) = value
#define SQREF_INCREMENT(addr) (++(*(addr)))
#define SQREF_DECREMENT(addr) (--(*(addr)))
#endif

#ifndef NO_GARBAGE_COLLECTOR

#define MARK_FLAG 0x08000000

#define SQCOLLECTABLE_ADDREF(O) SQREF_INCREMENT(&(O)->mprotected_nNumRefs)
#define SQCOLLECTABLE_DECREF(O) SQREF_DECREMENT(&(O)->mprotected_nNumRefs)

tI32 Collectable_AddRootRef(SQCollectable* o);
tI32 Collectable_ReleaseRootRef(SQCollectable* o);

struct SQCollectable : public ni::iCollectable {
  SQCollectable *_next;
  SQCollectable *_prev;
  SQREF_INT_TYPE mprotected_nNumRefs;
  SQREF_INT_TYPE mnRootRefs;
  SQCollectable() : mnRootRefs(0), mprotected_nNumRefs(0), _next(NULL), _prev(NULL) {
  }
  void UnMark();
  static void AddToChain(SQCollectable **chain,SQCollectable *c);
  static void RemoveFromChain(SQCollectable **chain,SQCollectable *c);
  inline ni::iUnknown* __stdcall _DoQueryInterface(iUnknown* apBase, const ni::tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(ni::iCollectable)) return apBase;
    if (aIID == niGetInterfaceUUID(ni::iUnknown)) return apBase;
    return NULL;
  }
  virtual void __stdcall ListInterfaces(ni::iMutableCollection* apLst, tU32) const {
    apLst->Add(niGetInterfaceUUID(ni::iCollectable));
    apLst->Add(niGetInterfaceUUID(ni::iUnknown));
  }
  virtual SQCollectable* __stdcall GetCollectableStructPtr() {
    return this;
  }
  __forceinline ni::tI32 __stdcall _CollectableAddRef() {
    return SQREF_INCREMENT(&mprotected_nNumRefs);
  }
  __forceinline ni::tI32 __stdcall _CollectableRelease()
  {
    if (SQREF_DECREMENT(&mprotected_nNumRefs) <= 0) {
      this->DeleteThis();
      return -1;
    }
    return SQREF_READ(&mprotected_nNumRefs);
  }
  virtual ni::tI32 __stdcall AddRef() {
    return _CollectableAddRef();
  }
  virtual ni::tI32 __stdcall Release() {
    return _CollectableRelease();
  }
  virtual ni::tI32 __stdcall SetNumRefs(ni::tI32 anNumRefs) {
    SQREF_WRITE(&mprotected_nNumRefs,anNumRefs);
    return SQREF_READ(&mprotected_nNumRefs);
  }
  virtual ni::tI32 __stdcall GetNumRefs() const {
    return mprotected_nNumRefs;
  }
  virtual void __stdcall DeleteThis() {
    niDelete this;
  }
  virtual tBool __stdcall IsOK() const {
    return eTrue;
  }
};

// assumes that _gc_chain_mutex is already locked, which should be
// done by the garbage collection function
#define START_MARK()                                                    \
  if (!(SQREF_READ(&mprotected_nNumRefs)&MARK_FLAG)) {                  \
  SQREF_WRITE(&mprotected_nNumRefs, SQREF_READ(&mprotected_nNumRefs)|MARK_FLAG);

#define END_MARK(chain)                         \
  RemoveFromChain(&_gc()->_gc_chain_ptr, this); \
  AddToChain(chain, this);                      \
  }

#define UNMARK()  SQREF_WRITE(&mprotected_nNumRefs, SQREF_READ(&mprotected_nNumRefs) & (~MARK_FLAG));

#define ADD_TO_CHAIN(obj) {                           \
    AutoThreadLock chainLock(_gc()->_gc_chain_mutex); \
    ++_gc()->_gc_chain_sync;                          \
    AddToChain(&_gc()->_gc_chain_ptr,obj);            \
  }

#define REMOVE_FROM_CHAIN(obj) {                          \
    if (!(SQREF_READ(&mprotected_nNumRefs)&MARK_FLAG)) {  \
      AutoThreadLock chainLock(_gc()->_gc_chain_mutex);   \
      RemoveFromChain(&_gc()->_gc_chain_ptr,obj);         \
    }                                                     \
  }

#define INIT_CHAIN() {_next=NULL;_prev=NULL;}

#else
///////////////////////////////////////////////////////////////////////////
// No garabage collector

#define ADD_TO_CHAIN(obj) ((void)0)
#define REMOVE_FROM_CHAIN(obj) ((void)0)
#define INIT_CHAIN() ((void)0)

struct SQCollectable : public ni::iCollectable {
  SQREF_INT_TYPE mprotected_nNumRefs;
  SQCollectable() : mprotected_nNumRefs(0) {
  }
  void UnMark();
  static void AddToChain(SQCollectable **chain,SQCollectable *c);
  static void RemoveFromChain(SQCollectable **chain,SQCollectable *c);
  inline ni::iUnknown* __stdcall _DoQueryInterface(iUnknown* apBase, const ni::tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(ni::iCollectable)) return apBase;
    if (aIID == niGetInterfaceUUID(ni::iUnknown)) return apBase;
    return NULL;
  }
  virtual void __stdcall ListInterfaces(ni::iMutableCollection* apLst, tU32) const {
    apLst->Add(niGetInterfaceUUID(ni::iCollectable));
    apLst->Add(niGetInterfaceUUID(ni::iUnknown));
  }
  virtual SQCollectable* __stdcall GetCollectableStructPtr() {
    return this;
  }
  virtual ni::tI32 __stdcall AddRef() {
    return SQREF_INCREMENT(&mprotected_nNumRefs);
  }
  virtual ni::tI32 __stdcall Release() {
    if (SQREF_READ(&mprotected_nNumRefs) < 0)
      return -1;
    if (SQREF_DECREMENT(&mprotected_nNumRefs) <= 0) {
      this->DeleteThis();
      return -1;
    }
    return SQREF_READ(&mprotected_nNumRefs);
  }
  virtual ni::tI32 __stdcall SetNumRefs(ni::tI32 anNumRefs) {
    SQREF_WRITE(&mprotected_nNumRefs,anNumRefs);
    return SQREF_READ(&mprotected_nNumRefs);
  }
  virtual ni::tI32 __stdcall GetNumRefs() const {
    return mprotected_nNumRefs;
  }
  virtual void __stdcall DeleteThis() {
    niDelete this;
  }
  virtual tBool __stdcall IsOK() const {
    return eTrue;
  }
};

#endif

struct SQObjectPtr;

#endif
