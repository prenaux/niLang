#ifndef _SQSTATE_H_
#define _SQSTATE_H_

#include "sqvector.h"
#include "sqobject.h"
#include <niLang/IHString.h>
#include <niLang/Utils/Hash.h> // for HashUUID
#include <niLang/Utils/ThreadImpl.h>

struct SQTable;
struct SQObjectPtr;

struct SQRegFunction {
  const SQChar *name = AZEROSTR;
  SQFUNCTION f = nullptr;
  int nparamscheck = 0;
  const SQChar *typemask = AZEROSTR;
  tHStringPtr rettype;
};

struct SQSharedState : public ni::ImplRC<ni::iUnknown>
{
 public:
  SQSharedState();
  ~SQSharedState();
#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable **chain);
#endif

  SQObjectPtr _refs_table;
  SQObjectPtr _scriptimports_table;
  SQObjectPtr _nativeimports_table;

  static SQRegFunction _base_funcs[];
  static SQRegFunction _automation_funcs[]; // in ScriptAutomation.cpp
  static SQRegFunction _lint_funcs[]; // in sqlinter.cpp
  static SQRegFunction _concurrent_funcs[]; // in ScriptVM_Concurrent.cpp

  SQObjectPtr _table_default_delegate;
  static SQRegFunction _table_default_delegate_funcz[];
  SQObjectPtr _array_default_delegate;
  static SQRegFunction _array_default_delegate_funcz[];
  SQObjectPtr _string_default_delegate;
  static SQRegFunction _string_default_delegate_funcz[];
  SQObjectPtr _number_default_delegate;
  static SQRegFunction _number_default_delegate_funcz[];
  SQObjectPtr _closure_default_delegate;
  static SQRegFunction _closure_default_delegate_funcz[];

  SQObjectPtr _idxprop_default_delegate;
  static SQRegFunction _idxprop_default_delegate_funcz[];
  SQObjectPtr _vec2f_default_delegate;
  static SQRegFunction _vec2f_default_delegate_funcz[];
  SQObjectPtr _vec3f_default_delegate;
  static SQRegFunction _vec3f_default_delegate_funcz[];
  SQObjectPtr _vec4f_default_delegate;
  static SQRegFunction _vec4f_default_delegate_funcz[];
  SQObjectPtr _matrixf_default_delegate;
  static SQRegFunction _matrixf_default_delegate_funcz[];
  SQObjectPtr _uuid_default_delegate;
  static SQRegFunction _uuid_default_delegate_funcz[];
  SQObjectPtr _enum_default_delegate;
  static SQRegFunction _enum_default_delegate_funcz[];
  SQObjectPtr _method_default_delegate;
  static SQRegFunction _method_default_delegate_funcz[];
  SQObjectPtr _interface_default_delegate;
  static SQRegFunction _interface_default_delegate_funcz[];
  SQObjectPtr _error_code_default_delegate;
  static SQRegFunction _error_code_default_delegate_funcz[];
  SQObjectPtr _resolved_type_default_delegate;
  static SQRegFunction _resolved_type_default_delegate_funcz[];

  SQCOMPILERERROR _compilererrorhandler;

  const SQObjectPtr& GetInterfaceDelegate(const tUUID& aID);
  typedef astl::hash_map<tUUID,SQObjectPtr> tDelegateMap;
  tDelegateMap  mmapDelegates;

  const SQObjectPtr& GetEnumDefTable(const sEnumDef* apEnumDef);
  typedef astl::hash_map<tIntPtr,SQObjectPtr> tEnumDefMap;
  tEnumDefMap mmapEnumDefs;

  SQObjectPtr _typeStr_null;
  SQObjectPtr _typeStr_int;
  SQObjectPtr _typeStr_float;
  SQObjectPtr _typeStr_string;
  SQObjectPtr _typeStr_table;
  SQObjectPtr _typeStr_array;
  SQObjectPtr _typeStr_function;
  SQObjectPtr _typeStr_userdata;
  SQObjectPtr _typeStr_iunknown;
  SQObjectPtr _typeStr_closure;
  const SQObjectPtr& GetTypeNameObj(SQObjectType type) const;
  const iHString* GetTypeNameHStr(SQObjectType type) const;
  const achar* GetTypeNameStr(SQObjectType type) const;
  const SQObjectPtr& GetTypeNameObj(const SQObject& o) const {
    return GetTypeNameObj(_sqtype(o));
  }
  const achar* GetTypeNameStr(const SQObject& o) const {
    return GetTypeNameStr(_sqtype(o));
  }

  tBool mbLangDelegatesLocked;
  const SQObjectPtr& GetLangDelegate(const achar* delID);
  void LockLangDelegates();
};

#define _ddel(ss,basename) _table((ss)._##basename##_default_delegate)

#ifdef SQUNICODE //rsl REAL STRING LEN
#define rsl(l) ((l)<<1)
#else
#define rsl(l) (l)
#endif

Ptr<SQNativeClosure> CreateSQNativeClosure(ain<SQRegFunction> aRegFunc);
tBool RegisterSQRegFunction(ain_nn_mut<SQTable> aTable, ain<SQRegFunction> aRegFunc);
tBool RegisterSQRegFunctions(ain_nn_mut<SQTable> aTable, ain<SQRegFunction*> aRegFuncs);

void *sq_vm_malloc(unsigned int size);
void *sq_vm_realloc(void *p,unsigned int oldsize,unsigned int size);
void sq_vm_free(void *p,unsigned int size);

#ifndef NO_GARBAGE_COLLECTOR
struct SQGarbageCollector {
  SQGarbageCollector();
  int CollectGarbage(SQCollectable** tchain);
  static void MarkObject(SQObjectPtr &o,SQCollectable **chain);
  bool _isCollecting;
  SQCollectable* _gc_chain_ptr;
  ThreadMutex _gc_chain_mutex;
  int _gc_chain_sync;
  int _gc_chain_lastgc_sync;
  astl::set<SQObjectPtr,SQObjectPtrSortByPtr> _gc_roots;
  int GetNumRoots();
  int AddRoot(const SQObjectPtr& o);
  int RemoveRoot(const SQObjectPtr& o);
};

extern SQGarbageCollector* _gGC;
#define _gc() _gGC
#endif

extern SQObjectPtr _sq_metamethods[MT__LAST];

tBool _InitializeSQGCAndGlobals();

#endif //_SQSTATE_H_
