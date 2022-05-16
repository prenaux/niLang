#include "stdafx.h"

#include <niLang/Types.h>
#include <niLang/IConcurrent.h>
#include <niLang/Utils/Sync.h>
#include <niLang/Utils/ConcurrentImpl.h>
#include "API/niScript/IScriptVM.h"
#include "ScriptVM.h"
#include "sqvm.h"

#include "ScriptVM_Concurrent.h"
// #include "Concurrent.h"

using namespace ni;

//===========================================================================
//
//  VMPool
//
//===========================================================================
struct VMPool : public cIUnknownImpl<iUnknown>
{
  __sync_mutex();

  Ptr<iScriptVM> _root;

  typedef astl::hash_map<tU64,Ptr<iScriptVM> > tMap;
  tMap _mapVM;

  VMPool(iScriptVM* apRootVM) {
    niAssert(niIsOK(apRootVM));
    _root = apRootVM;
  }
  ~VMPool() {
    while (!_mapVM.empty()) {
      InvalidateThreadVM(_mapVM.begin()->first);
    }
    _root = NULL;
  }

  tBool InvalidateThreadVM(tU64 aThreadID) {
    __sync_lock();
    tMap::iterator it = _mapVM.find(aThreadID);
    niAssert(it != _mapVM.end());
    if (it == _mapVM.end()) {
      return eFalse;
    }
#ifdef _DEBUG
    niDebugFmt(("D/ScriptVM[%p], detached from thread '%d'",(void*)it->second.ptr(),aThreadID));
#endif
    _mapVM.erase(aThreadID);
    return eTrue;
  }

  iScriptVM* GetThreadVM(tU64 aThreadID, tBool abAutoCreate) {
    __sync_lock();
    tMap::iterator it = _mapVM.find(aThreadID);
    if (it == _mapVM.end()) {
      if (!abAutoCreate) {
        niAssert(it != _mapVM.end());
        return NULL;
      }

      Ptr<iScriptVM> newVM = CreateChildScriptVM(_root);
      it = astl::upsert(_mapVM,aThreadID,newVM);
#ifdef _DEBUG
      niDebugFmt(("D/ScriptVM[%p], attached to thread '%d'",(void*)newVM.ptr(),aThreadID));
#endif
    }
    return it->second;
  }

  iScriptVM* GetCurrentVM(tBool abAutoCreate) {
    return GetThreadVM(ni::ThreadGetCurrentThreadID(),abAutoCreate);
  }
};
static Ptr<VMPool> _vmPool;
static tU64 _mainThreadID = 0;

#if !defined niNoThreads
static Ptr<iConcurrent> _vmConcurrent;

static tBool concurrent_vm_initthread(tU64 aThreadID) {
  niAssert(concurrent_vm_is_started());
  niAssert(aThreadID != _mainThreadID);
  if (!concurrent_vm_is_started()) return eFalse;
  if (_mainThreadID == aThreadID) return eTrue;
  return _vmPool->GetThreadVM(aThreadID,eTrue) != NULL;
}

static tBool concurrent_vm_invalidatethread(tU64 aThreadID) {
  niAssert(concurrent_vm_is_started());
  niAssert(aThreadID != 0);
  niAssert(aThreadID != _mainThreadID);
  if (!concurrent_vm_is_started()) return eFalse;
  if (_mainThreadID == aThreadID) return eFalse;
  return _vmPool->InvalidateThreadVM(aThreadID);
}

static void _ConcurrentCallbackThreadStart(tU64 threadID) {
  if (concurrent_vm_is_started()) {
    concurrent_vm_initthread(threadID);
  }
}

static void _ConcurrentCallbackThreadEnd(tU64 threadID) {
  if (concurrent_vm_is_started()) {
    concurrent_vm_initthread(threadID);
  }
}
#endif

tBool concurrent_vm_startup(HSQUIRRELVM rootVM, iConcurrent* apConcurrent) {
  if (_mainThreadID != 0) {
    niError("VM MultiThreading already initialized.");
    return eFalse;
  }

#if !defined niNoThreads
  ni::ConcurrentSetThreadStartEndCallbacks(
      _ConcurrentCallbackThreadStart,
      _ConcurrentCallbackThreadEnd);
#endif

  _mainThreadID = ni::ThreadGetCurrentThreadID();
  iScriptVM* vm = (iScriptVM*)sq_getforeignptr(rootVM);
  if (vm->GetParentVM() != NULL) {
    niError("RootVM for MultiThreading can't be a child VM.");
    return eFalse;
  }
  _vmPool = niNew VMPool(vm);
#if !defined niNoThreads
  _vmConcurrent = apConcurrent;
#endif
  return eTrue;
}

void concurrent_vm_shutdown() {
  if (_vmPool.IsOK()) {
    _vmPool->Invalidate();
    _vmPool = NULL;
#if !defined niNoThreads
    _vmConcurrent = NULL;
#endif
    _mainThreadID = 0;
  }
}

tBool concurrent_vm_is_started() {
  return _vmPool.IsOK();
}

tU64 concurrent_vm_mainthreadid() {
  return _mainThreadID;
}

HSQUIRRELVM concurrent_vm_mainvm() {
  niAssert(concurrent_vm_is_started());
  if (!concurrent_vm_is_started())
    return NULL;
  return (HSQUIRRELVM)_vmPool->_root->GetHandle();
}

HSQUIRRELVM concurrent_vm_threadvm(tU64 aThreadID) {
  niAssert(aThreadID != 0);
  niAssert(concurrent_vm_is_started());
  if (!concurrent_vm_is_started())
    return NULL;
  iScriptVM* pVM;
  if (aThreadID == _mainThreadID) {
    pVM = _vmPool->_root;
  }
  else {
    pVM = _vmPool->GetThreadVM(aThreadID,eFalse);
  }
  niAssert(pVM != NULL);
  if (!pVM) return NULL;
  return (HSQUIRRELVM)pVM->GetHandle();
}

HSQUIRRELVM concurrent_vm_currentvm() {
  return concurrent_vm_threadvm(ni::ThreadGetCurrentThreadID());
}

//===========================================================================
//
//  Mutithreaded API for the ScriptVM
//
//===========================================================================
static int mt_GetMainThreadID(HSQUIRRELVM v) {
  sq_pushint(v, _mainThreadID);
  return 1;
}

static int mt_GetCurrentThreadID(HSQUIRRELVM v) {
  sq_pushint(v, ni::ThreadGetCurrentThreadID());
  return 1;
}

#if !defined niNoThreads
static int mt_GetConcurrent(HSQUIRRELVM v) {
  sqa_pushIUnknown(v, _vmConcurrent);
  return 1;
}
#endif

struct RunnableScript : public cIUnknownImpl<iRunnable> {
  WeakPtr<SQSharedState> mSS;
  HSQOBJECT mClosure;

  RunnableScript(SQSharedState* apSS,HSQOBJECT closure)
      : mSS(apSS)
      , mClosure(closure)
  {
    sq_addref(*apSS,&mClosure);
  }
  ~RunnableScript() {
    QPtr<SQSharedState> ss = mSS;
    if (ss.IsOK()) {
      sq_release(*ss,&mClosure);
    }
  }

  Var __stdcall Run() {
    if (!concurrent_vm_is_started()) {
      niError("ConcurrentVM is not initialized.");
      return niVarNull;
    }

    HSQUIRRELVM v = concurrent_vm_currentvm();
    if (!v) {
      niError(niFmt("ConcurrentVM can't get VM for current thread '%d'.",
                    ni::ThreadGetCurrentThreadID()));
      return niVarNull;
    }

    niSqGuardSimple(v);

    Var r;

    sq_pushobject(v, mClosure);
    sq_newtable(v);
    if (SQ_SUCCEEDED(sq_call(v,1,1))) {
      sqa_getvar(v,-1,&r);
      sq_pop(v,1); // pop the return value
    }
    else {
      niError("ConcurrentVM call to script runnable failed.");
    }
    sq_pop(v,1); // pop the closure

    niSqUnGuardSimple(v);
    return r;
  }
};

static int mt_RunnableFromFunction(HSQUIRRELVM v) {
  HSQOBJECT oClosure;
  sq_resetobject(&oClosure);
  sq_getstackobj(v,2,&oClosure);
  Ptr<iRunnable> runnable = niNew RunnableScript(v->_ss,oClosure);
  sqa_pushIUnknown(v,runnable);
  return 1;
}

struct CallbackScript : public cIUnknownImpl<iCallback,eIUnknownImplFlags_DontInherit1,iRunnable> {
  WeakPtr<SQSharedState> mSS;
  HSQOBJECT mClosure;

  CallbackScript(SQSharedState* aSS, HSQOBJECT closure)
      : mSS(aSS)
      , mClosure(closure) {
    sq_addref(*aSS,&mClosure);
  }
  ~CallbackScript() {
    QPtr<SQSharedState> ss = mSS;
    if (ss.IsOK()) {
      sq_release(*ss,&mClosure);
    }
  }

  Var __stdcall RunCallback(const Var& avarA, const Var& avarB) {
    if (!concurrent_vm_is_started()) {
      niError("ConcurrentVM is not initialized.");
      return niVarNull;
    }

    HSQUIRRELVM v = concurrent_vm_currentvm();
    if (!v) {
      niError(niFmt("ConcurrentVM can't get VM for current thread '%d'.",
                    ni::ThreadGetCurrentThreadID()));
      return niVarNull;
    }

    niSqGuardSimple(v);

    Var r;

    sq_pushobject(v, mClosure);
    sq_newtable(v);
    sqa_pushvar(v, avarA);
    sqa_pushvar(v, avarB);
    if (SQ_SUCCEEDED(sq_call(v,2+1,1))) {
      sqa_getvar(v,-1,&r);
      sq_pop(v,1); // pop the return value
    }
    else {
      niError("ConcurrentVM call to script callback failed.");
    }
    sq_pop(v,1); // pop the closure

    niSqUnGuardSimple(v);
    return r;
  }

  Var __stdcall Run() {
    return RunCallback(niVarNull,niVarNull);
  }
};

static int mt_CallbackFromFunction(HSQUIRRELVM v) {
  HSQOBJECT oClosure;
  sq_resetobject(&oClosure);
  sq_getstackobj(v,2,&oClosure);
  Ptr<iCallback> callback = niNew CallbackScript(v->_ss,oClosure);
  sqa_pushIUnknown(v,callback);
  return 1;
}

SQRegFunction SQSharedState::_concurrent_funcs[] = {
#if !defined niNoThreads
  { "Concurrent_Get", mt_GetConcurrent, 1, "t" },
#endif
  { "Concurrent_GetMainThreadID", mt_GetMainThreadID, 1, "t" },
  { "Concurrent_GetCurrentThreadID", mt_GetCurrentThreadID, 1, "t" },
  { "Concurrent_RunnableFromFunction", mt_RunnableFromFunction, 2, "tc" },
  { "Concurrent_CallbackFromFunction", mt_CallbackFromFunction, 2, "tc" },
  { 0 }
};
