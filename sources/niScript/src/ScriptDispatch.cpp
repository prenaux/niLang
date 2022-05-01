// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(15)

#include "ScriptVM.h"
#include "ScriptObject.h"
#include "ScriptTypes.h"
#include "ScriptDispatch.h"
#include "ScriptAutomation.h"
#include "ScriptVM_Concurrent.h"
#include "sqtable.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cScriptDispatch implementation.

static tBool _FindMethod(SQTable* apTable, const SQObjectPtr& aKey, SQObjectPtr& aFunction) {
  niAssert(apTable != NULL);
  tBool bRet = eFalse;

  if (apTable->GetDelegate()) {
    // Fill with the delegate first so that the table override its delegate methods
    bRet = _FindMethod(apTable->GetDelegate(),aKey,aFunction);
  }

  SQObjectPtr theFunction;
  if (apTable->Get(aKey,theFunction)) {
    if (sq_isclosure(theFunction) || sq_isnativeclosure(theFunction)) {
      aFunction = theFunction;
      return eTrue;
    }
  }

  return bRet;
}

///////////////////////////////////////////////
cScriptDispatch::cScriptDispatch(SQTable* apTable)
{

  mprotected_pAggregateParent = apTable;

  niAssert(apTable);
  niAssert(apTable->mpDispatch == NULL);
  apTable->mpDispatch = this;
#ifdef _DEBUG
  niDebugFmt(("V/Dispatch %p for table %p created ...",(void*)this,(void*)apTable));
#endif
}

///////////////////////////////////////////////
cScriptDispatch::~cScriptDispatch()
{
#ifdef _DEBUG
  niDebugFmt(("V/Dispatch %p for table %p deleted ...",(void*)this,(void*)_GetTable()));
#endif

  niAssert(_GetTable()->mpDispatch == this);
  _GetTable()->mpDispatch = NULL;

  // Delete the dispatch wrapper's memory here. This destructor won't be called
  // until the last dispatch wrapper it reference is cleared since the dispatch
  // wrapper aggregates the iDispatch object.
  for (tInterfaceMap::iterator it = mmapInterfaces.begin(); it != mmapInterfaces.end(); ++it) {
    iUnknown* wrapper = it->second;
    wrapper->DeleteThis();
  }
  mmapInterfaces.clear();
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cScriptDispatch::IsOK() const
{
  niClassIsOK(cScriptDispatch);
  return mprotected_pAggregateParent->IsOK();
}

///////////////////////////////////////////////
//! Query an interface.
iUnknown* __stdcall cScriptDispatch::QueryInterface(const tUUID& aIID)
{
  if (aIID == niGetInterfaceUUID(iScriptObject)) {
    if (niIsNullPtr(mptrObj)) {
      HSQUIRRELVM vm = concurrent_vm_currentvm();
      if (!vm) {
        niWarning("Can't get currentvm.");
        return NULL;
      }
      vm->Push(_GetTable());
      mptrObj = niNew cScriptObject((cScriptVM*)vm->_foreignptr,-1,1,eTrue);
    }
    return mptrObj;
  }

  // already got a dispatch wrapper ?
  {
    __sync_lock();
    tInterfaceMap::const_iterator itI = mmapInterfaces.find(aIID);
    if (itI != mmapInterfaces.end())
      return itI->second;
  }

  // check if looking for the base interfaces...
  iUnknown* pRet = BaseImpl::QueryInterface(aIID);
  if (pRet)
    return pRet;

  // create the dispatch wrapper...
  const sInterfaceDef* pIDef = ni::GetLang()->GetInterfaceDefFromUUID(aIID);
  if (pIDef && (pIDef->mpfnCreateDispatchWrapper != NULL))
    return _CreateInterfaceDispatchWrapper(pIDef);
  else
    return NULL;
}

///////////////////////////////////////////////
void __stdcall cScriptDispatch::ListInterfaces(iMutableCollection* apLst, tU32) const
{
  apLst->Add(niGetInterfaceUUID(iDispatch));
  apLst->Add(niGetInterfaceUUID(iUnknown));
  {
    __sync_lock();
    for (tInterfaceMap::const_iterator it = mmapInterfaces.begin(); it != mmapInterfaces.end(); ++it) {
      apLst->Add(it->first);
    }
  }
}

///////////////////////////////////////////////
tBool __stdcall cScriptDispatch::InitializeMethods(const sMethodDef* const* apMethods, ni::tU32 anNumMethods) {
  __sync_lock();
  niLoop(i, anNumMethods) {
    const sMethodDef* meth = apMethods[i];
    SQObjectPtr objFunction;
    SQObjectPtr key = _H(meth->maszName);
    if (_FindMethod(_GetTable(),key,objFunction)) {
      astl::upsert(mmapMethods,meth,objFunction);
    }
    else if (!niFlagIs(meth->mReturnType, eTypeFlags_MethodOptional))  {
      niError(niFmt("Can't find required method '%s'.",meth->maszName));
      return eFalse;
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cScriptDispatch::CallMethod(const sMethodDef* const apMethodDef, ni::tU32 anMethodIndex, const Var* apParameters, tU32 anNumParameters, Var* apRet)
{
  HSQUIRRELVM vm = concurrent_vm_currentvm();
  if (!vm) {
    niWarning("Can't get currentvm.");
    return eFalse;
  }

  SQObjectPtr objMeth;
  {
    __sync_lock();
    tMethodMap::const_iterator itMeth = mmapMethods.find(apMethodDef);
    if (itMeth == mmapMethods.end()) {
      niError(niFmt("Can't find method '%s'.",apMethodDef->maszName));
      return eFalse;
    }
    objMeth = itMeth->second;
    if (!mOwnerVM) {
      mOwnerVM = vm;
    }
    else if (mOwnerVM != vm && !mbDidPrintMultiThreadedWarning) {
      mbDidPrintMultiThreadedWarning = eTrue;
      cString strFunc = "NA", strSource = "NA";
      tU32 nLine = 0;
      if (_sqtype(objMeth) == OT_CLOSURE) {
        SQFunctionProto *func = _funcproto(_closure(objMeth)->_function);
        if (_sqtype(func->_name) == OT_STRING) {
          strFunc = _stringval(func->_name);
        }
        if (_sqtype(func->_sourcename) == OT_STRING) {
          strSource = _stringval(func->_sourcename);
        }
        nLine = func->GetLine(func->_instructions.data());
      }
      niWarning(niFmt("ScriptDispatch %x (%s:%s:%d): Called from multiple threads, the calls will be serialized, thus very likely rendering the multi-threading useless and creating potential deadlocks.", (tIntPtr)this, strFunc, strSource, nLine));
    }
  }

  niSqGuard(vm);
  {
    vm->Push(objMeth);
    vm->Push(_GetTable());
    for (tU32 i = 0; i < anNumParameters; ++i) {
      // we cast to prevent the unnecessary construction of a variant
      sqa_pushvar(vm,(Var&)apParameters[i]);
    }

    niGuardObject(this);
    {
      __sync_lock();
      if (!SQ_SUCCEEDED(sq_call(vm, anNumParameters+1, apRet?1:0))) {
        vm->Pop(1); // pop the closure
        niSqUnGuard(vm);
        niError(niFmt("Call to method '%s' failed.",apMethodDef->maszName));
        return eFalse;
      }
    }

    if (apRet) {
      if (!SQ_SUCCEEDED(sqa_getvar_astype(vm,-1,apRet,apMethodDef->mReturnType))) {
        vm->Pop(2); // pop the return value and the closure
        niError(niFmt("Method '%s', can't get the return value.",apMethodDef->maszName));
        niSqUnGuard(vm);
        return eFalse;
      }
      vm->Pop(1); // pop the return value
    }
    vm->Pop(1); // pop the closure
  }
  niSqUnGuard(vm);
  return eTrue;
}

///////////////////////////////////////////////
iUnknown* __stdcall cScriptDispatch::_CreateInterfaceDispatchWrapper(const sInterfaceDef* apDef) {
#ifdef _DEBUG
  {
    __sync_lock();
    niAssert(apDef != NULL);
    niAssert(mmapInterfaces.find(*apDef->mUUID) == mmapInterfaces.end());
  }
#endif

  if (!apDef->mpfnCreateDispatchWrapper) {
    niError(niFmt(_A("The interface '%s' doesnt have a dispatch wrapper."),apDef->maszName));
    return NULL;
  }

  iUnknown* pWrapper = apDef->mpfnCreateDispatchWrapper(this);
  if (!pWrapper) {
    niError(niFmt(_A("Can't create the dispatch wrapper of interface '%s'."),apDef->maszName));
    return NULL;
  }

  {
    __sync_lock();
    astl::upsert(mmapInterfaces,*apDef->mUUID,pWrapper);
  }

  return pWrapper;
}

tI32 __stdcall cScriptDispatch::AddRef() {
  return BaseImpl::AddRef();
}
tI32 __stdcall cScriptDispatch::Release() {
  return BaseImpl::Release();
}

tI32 __stdcall cScriptDispatch::SetNumRefs(tI32 anNumRefs) {
  return BaseImpl::SetNumRefs(anNumRefs);
}
tI32 __stdcall cScriptDispatch::GetNumRefs() const {
  return BaseImpl::GetNumRefs();
}

#endif // niMinFeatures
