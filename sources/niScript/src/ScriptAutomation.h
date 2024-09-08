#ifndef __SCRIPTAUTOMATION_4169328_H__
#define __SCRIPTAUTOMATION_4169328_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if niMinFeatures(15)

#include <niLang/STL/hash_set.h>
#include <niLang/STL/bimap.h>
#include <niLang/ILang.h>
#include "ScriptDispatch.h"
#include "ScriptTypes.h"

// One of the three should be defined
#define GETIUNKNOWN_INDEX_VTABLE
// #define GETIUNKNOWN_OBJECT
// #define GETIUNKNOWN_SAFE

#define NI_VM(v) niUnsafeCast(cScriptVM*,sq_getforeignptr(v))
#define NI_VMA(v) niUnsafeCast(cScriptVM*,sq_getforeignptr(v))->GetAutomation()

#ifdef GETIUNKNOWN_INDEX_VTABLE
static const tU32 knUnkTableMaxSize = 64;
#elif defined GETIUNKNOWN_OBJECT
static const tU32 knUnkTableMaxSize = 1024;
#endif

///////////////////////////////////////////////
template<typename T>
inline SQFloat _toSQFloat(T a) {
  return (SQFloat)a;
}

// int iunknown_set(HSQUIRRELVM v);
// int iunknown_get(HSQUIRRELVM v);
int indexedproperty_set(HSQUIRRELVM v);
int indexedproperty_get(HSQUIRRELVM v);

void iunknown_gettype_concat(cString& astrOut, HSQUIRRELVM v, const iUnknown* apI);

//////////////////////////////////////////////////////////////////////////////////////////////
// cScriptAutomation declaration.

ASTL_RAW_ALLOCATOR_IMPL(ScriptDispatch);

//! Script automation system.
class cScriptAutomation : public ImplRC<iUnknown>
{
  niBeginClass(cScriptAutomation);

  typedef astl::hash_map<cString,Ptr<iModuleDef> >   tModuleMap;
  typedef tModuleMap::iterator        tModuleMapIt;
  typedef tModuleMap::const_iterator      tModuleMapCIt;

  typedef astl::bimap<SQTable*,cScriptDispatch*> tDispatchMap;

#if 0
  struct sIUnknownHash {
    size_t operator()(const iUnknown* __x) const {
      return (size_t)__x;
    }
  };
  struct sScriptTypeIUnknownHash {
    size_t operator()(const sScriptTypeIUnknown* __x) const {
      return (size_t)__x;
    }
  };
  typedef astl::hash_set<sScriptTypeIUnknown*,sScriptTypeIUnknownHash>  tScriptIUnknownSet;
  typedef tScriptIUnknownSet::iterator      tScriptIUnknownSetIt;

  typedef astl::list<sScriptTypeIUnknown*,
                     ASTL_ALLOCATOR(sScriptTypeIUnknown*,ScriptTypeIUnknown) >
  tScriptIUnknownLst;
#endif

 public:
#if 0
  struct sIUnknown {
    iUnknown*     pObj;
    tScriptObjectPtrLst lstInterfaces;
    tScriptObjectPtr  ptrBaseTable;
    tU32        nNumRefs;
    tScriptIUnknownLst  lstReferenced;
    sIUnknown() {
      pObj = NULL;
      nNumRefs = 0;
    }
    tBool IsReferenced() const {
      return !lstReferenced.empty();
    }
  };
  typedef astl::hash_map<iUnknown*,sIUnknown,sIUnknownHash> tIUnknownHMap;
  typedef tIUnknownHMap::iterator               tIUnknownHMapIt;
  typedef tIUnknownHMap::const_iterator           tIUnknownHMapCIt;
#endif

 public:
  //! Constructor.
  cScriptAutomation();
  //! Destructor.
  virtual ~cScriptAutomation();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;
  //! Invalidate.
  void __stdcall Invalidate();

  tBool Import(HSQUIRRELVM vm, const achar* aszMod);

  int PushIUnknown(HSQUIRRELVM vm, iUnknown* apClass);
  int GetIUnknown(HSQUIRRELVM vm, int idx, iUnknown** appIUnknown, const tUUID& aIID);

#if 0
  sScriptTypeIUnknown* NewIUnknown(SQVM* v, iUnknown* apIUnknown, sIUnknown*& apIU);
  void ReleaseIUnknownRef(sScriptTypeIUnknown* apScriptIUnknown);
  sIUnknown* GetIUnknown(iUnknown* apIUnknown);
#endif

  tBool IsValid() const { return mbIsValid; }

  bool Get(HSQUIRRELVM vm, iUnknown* apObj, const SQObjectPtr &key, SQObjectPtr &dest, int opExt);
  bool Set(HSQUIRRELVM vm, iUnknown* apObj,const SQObjectPtr &key,const SQObjectPtr &val, int opExt);

  iFileSystem* _GetFS();

 private:
  Ptr<iFileSystem> mptrFS;
  tBool           mbIsValid;

  tModuleMap mmapModules;

  typedef astl::list<SQTable*> tIntfDelegateLst;
  void InitIntfDelegateLst(
      HSQUIRRELVM vm,
      tIntfDelegateLst& alstInterfaces,
      iUnknown* apObj
#ifdef GETIUNKNOWN_INDEX_VTABLE
      , tBool abDynamicOnly
#endif
                           );

#ifdef GETIUNKNOWN_INDEX_VTABLE
  typedef astl::hash_map<tIntPtr,tIntfDelegateLst> tVTableMap;
  tIntfDelegateLst* GetIUnknownVTable(iUnknown* apIUnknown);
  tVTableMap mmapVTable;
#endif

#ifndef GETIUNKNOWN_SAFE
  struct sIUnknownEntry {
    tIntPtr     pUnknown;
    tIntPtr     pVTable;
    tIntfDelegateLst  lstInterfaces;
  };

  typedef astl::list<sIUnknownEntry*> tUEntryLst;
  typedef astl::hash_map<tIntPtr,tUEntryLst::iterator> tUEntryMap;
  sIUnknownEntry* NewIUnknownEntry(iUnknown* apIUnknown);
  sIUnknownEntry* GetIUnknownEntry(iUnknown* apIUnknown);
  sIUnknownEntry mvUnk[knUnkTableMaxSize];
  tUEntryLst mlstUnk;
  tUEntryMap mmapUnk;
#endif

  __forceinline bool GetIUnknownMethod(HSQUIRRELVM vm, iUnknown* apObj, const SQObjectPtr &aKey, SQObjectPtr& aMethod) {
#ifdef GETIUNKNOWN_INDEX_VTABLE
    {
      tIntfDelegateLst& lstInterfaces = *this->GetIUnknownVTable(apObj);
      if (lstInterfaces.empty()) {
        InitIntfDelegateLst(vm,lstInterfaces,apObj,eFalse);
      }
      niLoopit(astl::list<SQTable*>::iterator,it,lstInterfaces) {
        if ((*it)->Get(aKey,aMethod)) {
          return true;
        }
      }
    }

    // Didn't find the method in the VTable intf list, look in the
    // instance specific cache...  note that using QueryInterface
    // explicitly when necessary skips this step completly this is
    // kept for compatibility with old code and also because its
    // really handy to program UI, etc... (which are not really
    // pref critical anyhow...)
    //
    // See the implementation of InitIntfDelegateLst for more
    // details...
    {
#ifdef _DEBUG
      // niDebugFmt(("### MISSED METHOD '%s' FOR OBJECT '%p'",_stringhval(aKey),(tIntPtr)apObj));
#endif
      // sIUnknownEntry* e = this->GetIUnknownEntry(apObj);
      // tIntfDelegateLst& lstInterfaces = e->lstInterfaces;
      // if (lstInterfaces.empty()) {
      // #ifdef _DEBUG
      // niDebugFmt(("!!! FETCHED NEW INTF DELEGATES"));
      // #endif
      // InitIntfDelegateLst(lstInterfaces,apObj,eTrue);
      // }
      tIntfDelegateLst lstInterfaces;
      if (lstInterfaces.empty()) {
        InitIntfDelegateLst(vm,lstInterfaces,apObj,eTrue);
      }
      niLoopit(astl::list<SQTable*>::iterator,it,lstInterfaces) {
        if ((*it)->Get(aKey,aMethod)) {
          return true;
        }
      }
    }

#elif defined GETIUNKNOWN_OBJECT
    sIUnknownEntry* e = this->GetIUnknownEntry(apObj);
    tIntfDelegateLst& lstInterfaces = e->lstInterfaces;
    if (lstInterfaces.empty()) {
      InitIntfDelegateLst(vm,lstInterfaces,apObj);
    }
    niLoopit(astl::list<SQTable*>::iterator,it,lstInterfaces) {
      if ((*it)->Get(aKey,aMethod)) {
        return true;
      }
    }
#elif defined GETIUNKNOWN_SAFE
    tIntfDelegateLst lstInterfaces;
    if (lstInterfaces.empty()) {
      InitIntfDelegateLst(lstInterfaces,apObj);
    }
    niLoopit(astl::list<SQTable*>::iterator,it,lstInterfaces) {
      if ((*it)->Get(aKey,aMethod)) {
        return true;
      }
    }
#else
#error "No GETIUNKNOWN defined."
#endif
    return false;
  }

  niEndClass(cScriptAutomation);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif
#endif // __SCRIPTAUTOMATION_4169328_H__
