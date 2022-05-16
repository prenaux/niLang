#ifndef _SQTABLE_H_
#define _SQTABLE_H_

#include "sqobject.h"

#define hashptr(p)  (((unsigned long)(p)) >> 3)

// don't start at zero because the preprocessor will use 0 by default if the value isn't defined...
#define SQ_TABLE_MAP_HASH_MAP    1
#define SQ_TABLE_MAP_MAP         2
#define SQ_TABLE_MAP_VECTOR_MAP  3

//
// Use a map by default atm, as it provides a stable and sorted ordering of
// the keys. The advantage of having stable & logically sorted keys does
// justify the cost for a scripting language IMO.
//
#define SQ_TABLE_MAP_IMPL SQ_TABLE_MAP_MAP

ASTL_RAW_ALLOCATOR_IMPL(sqtable);
#define SQ_TABLE_ALLOCATOR(A,B) ASTL_PAIR_ALLOCATOR(const A,B,sqtable)

#define SQ_TABLE_FLAGS_CAN_CALL_METATABLE niBit(0)
#define SQ_TABLE_FLAGS_INVALIDATED        niBit(1)
#define SQ_TABLE_FLAGS_WRITTING           niBit(2)

class cScriptDispatch;
struct SQTableIterator;

struct SQTable : public SQCollectable, public SQ_ALLOCATOR(SQTable)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID);

#if (SQ_TABLE_MAP_IMPL == SQ_TABLE_MAP_HASH_MAP)
  //
  // Not sorted and unstable (aka the keys won't be in the same order on
  // different runs).
  //
  // Still fastest in of most cases, but it does suffer from larger
  // performance deviations, sometimes performances are worst than map on
  // different runs (this is a side effect of the unstable sorting). Also use
  // the more memory.
  //
  // Performance is to 2-5% better than when using a map.
  //
  struct sHash {
    __forceinline size_t operator()(const SQObjectPtr &key) const {
      switch(_sqtype(key)) {
        case OT_STRING:   return (size_t)_stringhval(key);
        case OT_FLOAT:    return (size_t)*((SQInt*)(&_float(key)));
        case OT_INTEGER:  return (size_t)_int(key);
        case OT_USERDATA: return (size_t)_userdata(key)->Hash();
        default:          return hashptr(_collectable(key));
      }
    }
  };
  struct sHash_equal_to {
    __forceinline bool operator()(const SQObjectPtr& aLeft, const SQObjectPtr& aRight) const {
      return aLeft == aRight;
    }
  };
  typedef astl::hash_map<SQObjectPtr,SQObjectPtr,
      sHash,sHash_equal_to,
      SQ_TABLE_ALLOCATOR(SQObjectPtr,SQObjectPtr)
      > tHMap;
#elif (SQ_TABLE_MAP_IMPL == SQ_TABLE_MAP_MAP)
  // Use less memory than HASH_MAP, provides stable sorting of keys.
  typedef astl::map<SQObjectPtr,SQObjectPtr,SQObjectPtrSortLogical,SQ_TABLE_ALLOCATOR(SQObjectPtr,SQObjectPtr)> tHMap;
#elif (SQ_TABLE_MAP_IMPL == SQ_TABLE_MAP_VECTOR_MAP)
  // Use less memory than MAP, in theory slower insert/delete time, in
  // practise seems the same as a regular map (benchmarked with the nil compiler).
  typedef astl::vector_map<SQObjectPtr,SQObjectPtr,SQObjectPtrSortLogical,SQ_TABLE_ALLOCATOR(SQObjectPtr,SQObjectPtr)> tHMap;
#endif
  typedef tHMap::iterator     tHMapIt;
  typedef tHMap::const_iterator tHMapCIt;

  tHMap       mhmap;
  SQObjectPtr mptrDelegate;
  SQTable*    mpParent;
  iUnknown*   mpDispatch;
  tU32        mnFlags;
  SQTableIterator* mpIterators;

private:
  SQTable();
  ~SQTable();

public:
  virtual ni::tI32 __stdcall AddRef();
  virtual ni::tI32 __stdcall Release();

  static SQTable* Create();
  virtual void __stdcall Invalidate();
  SQTable* Clone(SQVM* apVM, tSQDeepCloneGuardSet* apDeepClone);
#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable **chain);
#endif
  bool Get(const SQObjectPtr &key,SQObjectPtr &val) const;
  bool GetKey(const SQObjectPtr &val,SQObjectPtr &key) const;
  void Remove(const SQObjectPtr &key);
  void Clear();
  bool Set(const SQObjectPtr &key, const SQObjectPtr &val);
  //returns true if a new slot has been created, false if it was already present
  bool NewSlot(const SQObjectPtr &key,const SQObjectPtr &val);
  tBool Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval, SQObjectPtr &itr);
  int CountUsed();
  bool SetDelegate(SQTable *mt);
  SQTable* GetDelegate() const;
  void SetParent(SQTable* apParent);
  SQTable* GetParent() const;
  void __stdcall SetDebugName(const achar* aaszName);
  void __stdcall SetDebugName(iHString* hspName) { SetDebugName(niHStr(hspName)); }
  const iHString* __stdcall GetDebugHName() const;
  const achar* __stdcall GetDebugName() const;
  __forceinline tHMap& __stdcall GetHMap() { return mhmap; }
  __forceinline const tHMap& __stdcall GetHMap() const { return mhmap; }
  __forceinline void __stdcall SetCanCallMetaMethod(tBool abEnabled) {
    niFlagOnIf(mnFlags,SQ_TABLE_FLAGS_CAN_CALL_METATABLE,abEnabled);
  }
  __forceinline tBool __stdcall CanCallMetaMethod() const {
    return niFlagIs(mnFlags,SQ_TABLE_FLAGS_CAN_CALL_METATABLE);
  }
  __forceinline tBool __stdcall SerializeWriteLock() {
    if (niFlagIs(mnFlags,SQ_TABLE_FLAGS_WRITTING))
      return ni::eFalse;
    niFlagOn(mnFlags,SQ_TABLE_FLAGS_WRITTING);
    return ni::eTrue;
  }
  __forceinline tBool __stdcall SerializeWriteUnlock() {
    if (niFlagIsNot(mnFlags,SQ_TABLE_FLAGS_WRITTING))
      return ni::eFalse;
    niFlagOff(mnFlags,SQ_TABLE_FLAGS_WRITTING);
    return ni::eTrue;
  }
};

#endif //_SQTABLE_H_
