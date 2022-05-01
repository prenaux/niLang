#ifndef _SQCLOSURE_H_
#define _SQCLOSURE_H_

#include "sqstate.h"
#include "sqtable.h"

struct SQFunctionProto;
struct SQClosure : public SQCollectable, public SQ_ALLOCATOR(SQClosure)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

private:
  SQClosure(SQFunctionProto *func) {
    _function=func;
    INIT_CHAIN();
    ADD_TO_CHAIN(this);
  }

public:
  static SQClosure *Create(SQFunctionProto *func, SQTable* root) {
    SQClosure* c = niNew SQClosure(func);
    niAssert(root != NULL);
    c->_root.Swap(root);
    return c;
  }
  ~SQClosure() {
    REMOVE_FROM_CHAIN(this);
  }
#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable **chain);
#endif
  virtual void __stdcall Invalidate() {_outervalues.resize(0); }
  SQObjectPtr _function;
  SQObjectPtrVec _outervalues;
  WeakPtr<SQTable> _root;
};

struct SQNativeClosure : public SQCollectable, SQ_ALLOCATOR(SQNativeClosure)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

private:
  SQNativeClosure(SQFUNCTION func) {
    _function = func;
    INIT_CHAIN();
    ADD_TO_CHAIN(this);
  }
public:
  static SQNativeClosure *Create(SQFUNCTION func) {
    return niNew SQNativeClosure(func);
  }
  ~SQNativeClosure()
  {
    REMOVE_FROM_CHAIN(this);
  }
#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable **chain);
#endif
  virtual void __stdcall Invalidate(){_outervalues.resize(0);}
  SQFUNCTION _function;
  SQObjectPtr _name;
  SQObjectPtrVec _outervalues;
  SQIntVec _typecheck;
  int _nparamscheck;
};

#endif //_SQCLOSURE_H_
