#ifndef _SQVM_H_
#define _SQVM_H_

#include "sqconfig.h"
#include "sqopcodes.h"
#include "sqobject.h"
#include "sqtable.h"
#include "sqclosure.h"
#include "sqfuncproto.h"
#include <niLang/STL/stack.h>

#define MAX_NATIVE_CALLS 256 // out of stack on Windows at around ~300
#define MIN_STACK_OVERHEAD  8

struct sScriptTypeMethodDef;

#define niRTError(v,desc) {                     \
    v->Raise_MsgError(desc);                    \
  }

#define VM_ERRORB_(v,err) { niRTError(v,err); return false;  }
#define VM_ERRORB(err)    { niRTError(this,err); return false; }

#define VM_STACK_TOP() (_stack[_top-1])

//base lib
void sq_base_register(HSQUIRRELVM v);

static __forceinline bool Table_GetMetaMethod(SQObjectPtr& closure, SQTable *mt,SQMetaMethod mm,tBool abOneLevel=eFalse) {
  if (abOneLevel) {
    if (mt->Get(_ss()->_metamethods[mm], closure)) {
      return true;
    }
  }
  else {
    SQTable* ct = mt;
    while (ct)
    {
      if (ct->Get(_ss()->_metamethods[mm], closure)) {
        return true;
      }
      ct = ct->GetDelegate();
    }
  }
  return false;
}

struct SQExceptionTrap{
  SQExceptionTrap() {}
  SQExceptionTrap(int ss, int stackbase,const SQInstruction *ip, int ex_target){ _stacksize = ss; _stackbase = stackbase; _ip = ip; _extarget = ex_target;}
  SQExceptionTrap(const SQExceptionTrap &et) { (*this) = et;  }
  int _stackbase;
  int _stacksize;
  const SQInstruction *_ip;
  int _extarget;
};

SQ_VECTOR_TYPEDEF(SQExceptionTrap,ExceptionsTraps);

struct SQCallInfo {
  SQCallInfo() {}
  const SQInstructionVec *_iv;
  const SQObjectPtrVec *_literals;
  SQObjectPtr _closurePtr;
  int _etraps;
  int _prevstkbase;
  int _prevtop;
  int _target;
  const SQInstruction *_ip;
  int _ncalls;
  bool _root;
};
SQ_VECTOR_TYPEDEF(SQCallInfo,SQCallInfoVec);

struct SQVM : public SQCollectable, public cMemImpl
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

 public:
  SQVM();
  ~SQVM();
  bool Init(bool abInitRootTable);
  bool Execute(const SQObjectPtr &func, int target, int nargs, int stackbase, SQObjectPtr &outres);

  // start a native call return when the NATIVE closure returns
  bool CallNative(SQNativeClosure *nclosure,int nargs,int stackbase,SQObjectPtr &retval);
  bool CallMethodDef(sScriptTypeMethodDef* apMethodDef, int nargs, int stackbase, SQObjectPtr &retval);
  //start a SQUIRREL call in the same "Execution loop"
  bool StartCall(SQClosure *closure, int target, int nargs, int stackbase, bool tailcall);
  //call a generic closure pure SQUIRREL or NATIVE
  bool Call(SQObjectPtr &closure, int nparams, int stackbase, SQObjectPtr &outres);

  void CallDebugHook(int type,int forcedline=0);

  bool DoGet(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest,
             const SQObjectPtr* root, int opExt);
  __forceinline  bool Get(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest,
                          const SQObjectPtr* root, int opExt)
  {
    bool r = DoGet(self,key,dest,root,opExt);
    if (!r) {
      if (opExt & _OPEXT_GET_SAFE) {
        dest = _null_;
        return true;
      }
      return false;
    }
    return true;
  }
  bool DoSet(const SQObjectPtr &self, const SQObjectPtr &key, const SQObjectPtr &val, int opExt);
  __forceinline bool Set(const SQObjectPtr &self, const SQObjectPtr &key, const SQObjectPtr &val,
                         int opExt)
  {
    bool r = DoSet(self,key,val,opExt);
    if (!r && !(opExt & _OPEXT_GET_SAFE)) {
      return false;
    }
    return true;
  }

  bool NewSlot(const SQObjectPtr &self, const SQObjectPtr &key, const SQObjectPtr &val, int opExt);

  bool DeleteSlot(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &res, int opExt);
  bool Clone(const SQObjectPtr &self, SQObjectPtr &target, tSQDeepCloneGuardSet* apDeepClone);
  bool ObjCmp(const SQObjectPtr &o1, const SQObjectPtr &o2, int& res);
  bool ObjEq(const SQObjectPtr &o1, const SQObjectPtr &o2, int& res);
  bool StringCat(const SQObjectPtr &str, const SQObjectPtr &obj, SQObjectPtr &dest);

  void Raise_ObjError(const SQObjectPtr &desc, bool aPrint);
  void Raise_MsgError(const cString& s);
  void Raise_IdxError(const SQObject &o);
  void Raise_CompareError(const SQObject &o1, const SQObject &o2);
  void Raise_ParamTypeError(int nparam,int typemask,int type);

  SQObjectPtr PrintObjVal(const SQObject &o);
  void TypeOf(const SQObjectPtr &obj1, SQObjectPtr &dest);

  __forceinline bool CallMetaMethod(SQObjectPtr& closure, int nparams, SQObjectPtr &outres) {
    bool r = Call(closure, nparams, _top-nparams, outres);
    Pop(nparams);
    return r;
  }

  __forceinline bool CallMetaMethod(SQTable *mt,SQMetaMethod mm,int nparams,SQObjectPtr &outres,tBool abOneLevel = eFalse) {
    SQObjectPtr closure;
    if (!Table_GetMetaMethod(closure,mt,mm,abOneLevel)) {
      return false;
    }
    return CallMetaMethod(closure,nparams,outres);
  }

  bool CallIUnknownMetaMethod(iUnknown* o, SQMetaMethod mm, int nparams, SQObjectPtr &outres);
  bool CallIUnknownMetaMethod(iUnknown* o, SQObjectPtr& closure, int nparams, SQObjectPtr &outres);
  bool GetIUnknownMetaMethod(iUnknown* o, SQMetaMethod mm, SQObjectPtr &closure);
  bool ArithMetaMethod(int op, const SQObjectPtr &o1, const SQObjectPtr &o2, SQObjectPtr &dest);
  bool Modulo(const SQObjectPtr &o1, const SQObjectPtr &o2, SQObjectPtr &dest);
  bool Div(const SQObjectPtr &o1, const SQObjectPtr &o2, SQObjectPtr &dest);
  bool Return(int _arg0, int _arg1, SQObjectPtr &retval);

  bool DerefInc(SQObjectPtr &target, SQObjectPtr &self, SQObjectPtr &key, SQObjectPtr &incr, bool postfix, int opExt);
#ifdef _DEBUG_DUMP
  void dumpstack(int stackbase=-1, bool dumpall = false);
  bool _execDumpStack;
#endif

#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable **chain);
#endif
  virtual void __stdcall Invalidate();

  ////////////////////////////////////////////////////////////////////////////
  //stack functions for the api
  __forceinline void Pop() {
    _stack[--_top] = _null_;
  }
  __forceinline void Pop(int n) {
    for(int i = 0; i < n; i++){
      _stack[--_top] = _null_;
    }
  }
  __forceinline void Remove(int n) {
    n = (n >= 0)?n + _stackbase - 1:_top + n;
    niAssert(_top <= (int)_stack.size());
    for(int i = n; i < _top; i++){
      _stack[i] = _stack[i+1];
    }
    _stack[_top] = _null_;
    _top--;
  }

  __forceinline void Push(const SQObjectPtr &o) { niAssert(_top+1 < (int)_stack.size()); _stack[_top++] = o; }
  __forceinline SQObjectPtr &Top() { niAssert(_top-1 < (int)_stack.size()); return _stack[_top-1]; }
  __forceinline SQObjectPtr &PopGet() { niAssert(_top-1 < (int)_stack.size()); return _stack[--_top]; }
  __forceinline SQObjectPtr &GetUp(int n) { niAssert(_top+n < (int)_stack.size()); return _stack[_top+n]; }
  __forceinline SQObjectPtr &GetAt(int n) { niAssert(n < (int)_stack.size()); return _stack[n]; }
  __forceinline int GetCallStackBase(int numParams) { return _top-_stackbase-numParams+1; }
  SQObjectPtr &GetEmptyString();

  SQObjectPtrVec _stack;
  int _top;
  int _stackbase;
  SQObjectPtr _roottable;

  SQObjectPtr _lasterror;
  void CallErrorHandler(const SQObjectPtr& error);

  SQObjectPtr _errorhandler;
  SQObjectPtr _debughook;
  SQObjectPtr _emptystring;
  tU32 _raiseErrorMode;
  tU32 _logRaiseError;

  SQCallInfoVec _callsstack;
  ExceptionsTraps _etraps;
  SQCallInfo *_ci;
  void *_foreignptr;
  int _nnativecalls;
};

struct AutoDec {
  AutoDec() = delete;
  AutoDec(int *n) { _n = n; }
  ~AutoDec() { (*_n)--; }
  int *_n;
};

__forceinline SQObjectPtr& stack_get(HSQUIRRELVM v, int idx)
{
  return ((idx>=0) ? (v->GetAt(idx + v->_stackbase - 1)) : (v->GetUp(idx)));
}
__forceinline void stack_set(HSQUIRRELVM v, int idx, SQObjectPtr& o)
{
  if (idx >= 0) {
    v->GetAt(idx + v->_stackbase - 1) = o;
  }
  else {
    v->GetUp(idx) = o;
  }
}

__forceinline void push_callinfo(HSQUIRRELVM v, const SQCallInfo& nci) {
  v->_callsstack.push_back(nci);
  v->_ci = &v->_callsstack.back();
}

__forceinline void pop_callinfo(HSQUIRRELVM v) {
  v->_callsstack.pop_back();
  if(v->_callsstack.size())  v->_ci = &v->_callsstack.back();
  else                       v->_ci = NULL;
}

__forceinline cString& SQVM_CatFloatToString(cString& out, const SQFloat val) {
  if (ni::Floor(val) == val) {
    out.CatFormat("%.1f", val);
  }
  else {
    out.CatFormat("%.14g", val);
  }
  return out;
}

#endif //_SQVM_H_
