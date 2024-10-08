#include "stdafx.h"

#include "sqconfig.h"
#include "sqvm.h"
#include "sqobject.h"
#include "sqtable.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqarray.h"
#include "ScriptVM.h"
#include "ScriptVM_Concurrent.h"
#include <niLang/Utils/ThreadImpl.h>
#include "sq_hstring.h"

// Once SQObjectPtr is a Var we can use iIterator
struct SQTableIterator : public ImplRC<iUnknown> {
  SQTableIterator* mpNextIterator;
  SQTable*         mpTable;
  SQTable::tHMapIt mIt;

  SQTableIterator(SQTable* aTable, SQTable::tHMapIt it)
      : mpTable(aTable)
      , mIt(it)
      , mpNextIterator(NULL)
  {
    mpTable->_CollectableAddRef();
    if (aTable->mpIterators != NULL) {
      mpNextIterator = aTable->mpIterators;
    }
    aTable->mpIterators = this;
  }
  ~SQTableIterator() {
    Invalidate();
  }
  void __stdcall Invalidate() {
    if (!mpTable)
      return;
    if (mpTable->mpIterators == this) {
      mpTable->mpIterators = mpNextIterator;
    }
    else {
      SQTableIterator* prev = mpTable->mpIterators;
      niAssert(prev != NULL);
      SQTableIterator* it = prev->mpNextIterator;
      niAssert(it != NULL);
      while (it) {
        if (it == this) {
          // can't be null otherwise it would be the list's sentinel
          niAssert(prev != NULL);
          prev->mpNextIterator = mpNextIterator;
          break;
        }
        prev = it;
        it = it->mpNextIterator;
        niAssert(it != NULL);
      }
    }
    mpTable->_CollectableRelease();
    mpTable = NULL;
  }
  tBool __stdcall HasNext() const {
    return mpTable && mIt != mpTable->mhmap.end();
  }
  const SQObjectPtr& __stdcall Next() {
    if (!HasNext()) {
      return _null_;
    }
    ++mIt;
    return mIt->second;
  }
  const SQObjectPtr& __stdcall Key() const {
    if (!HasNext()) return _null_;
    return mIt->first;
  }
  const SQObjectPtr& __stdcall Value() const {
    if (!HasNext()) return _null_;
    return mIt->second;
  }
};

void SQTable_BeforeWrite(SQTable* t) {
  while (t->mpIterators) {
    t->mpIterators->Invalidate();
  }
}

#define CHECK_HASH_MAP()
/*
#define CHECK_HASH_MAP()  {                                       \
    const achar* tableName = GetDebugName();                      \
    for (tHMapCIt it = mhmap.begin(); it != mhmap.end(); ++it) {  \
      tHMapCIt itNext = it; ++itNext;                             \
      niAssert(it != itNext);                                     \
      if (it == itNext) break;                                    \
    }}
*/

SQTable* SQTable::Create() {
  return niNew SQTable();
}

SQTable::SQTable()
{
  mptrDelegate = _null_;
  mpDispatch = NULL;
  mnFlags = 0;
  mpIterators = NULL;
  INIT_CHAIN();
  ADD_TO_CHAIN(this);
}
SQTable::~SQTable()
{
  niGuardConstructor(SQTable) {
    Invalidate();
    if (mpDispatch) {
#ifdef _DEBUG
      niDebugFmt(("V/Table %p with dispatch %p deleted ...",(void*)this,(void*)mpDispatch));
#endif
      mpDispatch->DeleteThis();
      // dispatch's constructor should have set mpDispatch to NULL
      niAssert(mpDispatch == NULL);
    }
    REMOVE_FROM_CHAIN(this);
  }
}

void SQTable::Remove(const SQObjectPtr &key)
{
  CHECK_HASH_MAP();
  tHMapIt it = mhmap.find(key);
  if (it == mhmap.end())
    return;
  SQTable_BeforeWrite(this);
  mhmap.erase(it);
  CHECK_HASH_MAP();
}


void SQTable::Invalidate()
{
  _CollectableAddRef();
  {
    if (niFlagIsNot(mnFlags,SQ_TABLE_FLAGS_INVALIDATED)) {
      SQTable_BeforeWrite(this);
      niFlagOn(mnFlags,SQ_TABLE_FLAGS_INVALIDATED);
      SQObjectPtr closure;
      if (CanCallMetaMethod() && Table_GetMetaMethod(closure,this,MT_TABLE_INVALIDATE)) {
        SQVM* pVM = concurrent_vm_currentvm();
        if (pVM) {
          pVM->Push(this);
          SQObjectPtr r;
          if (!pVM->CallMetaMethod(closure,1,r)) {
            niWarning("table _invalidate call failed.");
          }
        }
        else {
          niWarning(niFmt("Can't call table's invalidate, no VM for thread '%d'", ni::ThreadGetCurrentThreadID()));
        }
      }
    }
    mhmap.clear();
    SetDelegate(NULL);
  }
  _CollectableRelease();
}

void SQTable::Clear()
{
  SQTable_BeforeWrite(this);
  mhmap.clear();
}

void SQTable::Reserve(int size) {
  // TODO: I'm really tempted to make mhmap a vector map again. They are slow
  // inserts but tables are not meant fo this purpose they are mostly there to
  // hold the language constructs. There's Vector(), Set() and Map() for
  // "high performance" things and of course all the native modules...
#if (SQ_TABLE_MAP_IMPL == SQ_TABLE_MAP_VECTOR_MAP)
  mhmap.reserve(size);
#endif
}

int SQTable::CountUsed()
{
  return mhmap.size();
}

SQTable *SQTable::Clone(tSQDeepCloneGuardSet* apDeepClone)
{
  SQTable* nt = niNew SQTable();
  nt->mhmap = mhmap;
  if (!mptrDelegate.IsNull()) {
    nt->SetDelegate(_table(mptrDelegate));
  }
  nt->SetCanCallMetaMethod(this->CanCallMetaMethod());
  return nt;
}

bool SQTable::Get(const SQObjectPtr &key,SQObjectPtr &val) const
{
  if (mhmap.empty())
    return false;
  CHECK_HASH_MAP();
  tHMapCIt it = mhmap.find(key);
  if (it == mhmap.end())
    return false;
  val = it->second;
  CHECK_HASH_MAP();
  return true;
}

bool SQTable::GetKey(const SQObjectPtr &val,SQObjectPtr &key) const
{
  CHECK_HASH_MAP();
  for (tHMapCIt it = mhmap.begin(); it != mhmap.end(); ++it)
  {
    if (it->second == val)
    {
      key = it->first;
      return true;
    }
  }
  return false;
}

bool SQTable::Set(const SQObjectPtr &key, const SQObjectPtr &val)
{
  CHECK_HASH_MAP();
  tHMapIt it = mhmap.find(key);
  if (it == mhmap.end())
    return false;
  it->second = val;
  CHECK_HASH_MAP();
  return true;
}

//returns true if a new slot has been created, false if it was already present
bool SQTable::NewSlot(const SQObjectPtr &key,const SQObjectPtr &val)
{
  CHECK_HASH_MAP();
  tHMapIt it = mhmap.find(key);
  if (it == mhmap.end()) {
    SQTable_BeforeWrite(this);
    astl::upsert(mhmap,key,val);
    CHECK_HASH_MAP();
    return true;
  }
  else {
    it->second = val;
    CHECK_HASH_MAP();
    return false;
  }
}

tBool SQTable::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval, SQObjectPtr& itr)
{
  Ptr<SQTableIterator> it;
  if (_sqtype(refpos) == OT_NULL) {
    if (mhmap.empty())
      return eFalse;
    if (niFlagIs(mnFlags,SQ_TABLE_FLAGS_INVALIDATED)) {
      outkey = outval = _null_;
      itr = _H("Table invalidated.");
      return eFalse;
    }
    it = niNew SQTableIterator(this,mhmap.begin());
    niAssert(it->HasNext());
  }
  else {
    it = down_cast<SQTableIterator*>(_iunknown(refpos));
    if (!it->mpTable) {
      outkey = outval = _null_;
      itr = _H("Table modified while iterating.");
      return eFalse;
    }
    niAssert(it->HasNext());
    it->Next();
    if (!it->HasNext()) {
      return eFalse;
    }
  }
  outkey = it->mIt->first;
  outval = it->mIt->second;
  itr = it.ptr();
  return eTrue;
}

bool SQTable::SetDelegate(SQTable *mt)
{
  CHECK_HASH_MAP();
  SQTable *temp = mt;
  while (temp) {
    niAssert(_table(temp->mptrDelegate) != this);
    if (_table(temp->mptrDelegate) == this)
      return false; //cycle detected
    temp = _table(temp->mptrDelegate);
  }
  mptrDelegate = mt ? mt : _null_;
  CHECK_HASH_MAP();
  return true;
}

SQTable* SQTable::GetDelegate() const
{
#ifdef _DEBUG
  if (mptrDelegate.IsNull()) {
    niAssert(_table(mptrDelegate) == NULL);
  }
#endif
  return _table(mptrDelegate);
}

void SQTable::SetDebugName(const achar* aaszName)
{
  SQObjectPtr key = _HC(__debug_name);
  SQObjectPtr val = _H(aaszName);
  NewSlot(key,val);
}

const iHString* __stdcall SQTable::GetDebugHName() const
{
  SQObjectPtr obj = _HC(__debug_name);
  tHMapCIt it = mhmap.find(obj);
  if (it == mhmap.end())
    return NULL;
  if (_sqtype(it->second) == OT_STRING)
    return _stringhval(it->second);
  else
    return NULL;
}

const achar* __stdcall SQTable::GetDebugName() const
{
  const iHString* hspName = GetDebugHName();
  if (!hspName) hspName = _HC(no_debug_name);
  return niHStr(hspName);
}

#ifndef NO_GARBAGE_COLLECTOR
void SQTable::Mark(SQCollectable **chain)
{
  START_MARK();
  CHECK_HASH_MAP();
  if (!mptrDelegate.IsNull()) {
    _table(mptrDelegate)->Mark(chain);
  }
  CHECK_HASH_MAP();
  for (tHMapIt it = mhmap.begin(); it != mhmap.end(); ++it) {
    SQGarbageCollector::MarkObject((SQObjectPtr&)it->first,chain);
    SQGarbageCollector::MarkObject((SQObjectPtr&)it->second,chain);
  }
  END_MARK(chain);
}
#endif

ni::iUnknown* __stdcall SQTable::QueryInterface(const ni::tUUID& aIID) {
  return this->_DoQueryInterface(this,aIID);
}

ni::tI32 __stdcall SQTable::AddRef() {
  return Collectable_AddRootRef(this);
}
ni::tI32 __stdcall SQTable::Release() {
  return Collectable_ReleaseRootRef(this);
}
