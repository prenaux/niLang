#include "stdafx.h"

#include "sqconfig.h"
#include "sqopcodes.h"
#include "sqstate.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqtable.h"
#include "sqarray.h"
#include "sq_hstring.h"

#ifdef NO_GARBAGE_COLLECTOR
#pragma niNote("NO GARBAGE COLLECTOR")
#else
#pragma niNote("USING GARBAGE COLLECTOR")
#endif

#define DEBUG_GC_ROOT(FMT) //niDebugFmt

SQObjectPtr _null_=SQObjectPtr();
SQObjectPtr _notnull_(1);
SQObjectPtr _one_(1);
SQObjectPtr _minusone_(-1);

SQSharedState::SQSharedState() {
  _compilererrorhandler = NULL;
  mbLangDelegatesLocked = eFalse;

  niLet createDelegate = [](SQSharedState *ss,SQRegFunction *funcz) -> SQObjectPtr {
    NN_mut<SQTable> t { SQTable::Create() };
    RegisterSQRegFunctions(t, funcz);
    return t.raw_ptr();
  };

  _refs_table = SQTable::Create();
  _scriptimports_table = SQTable::Create();
  _nativeimports_table = SQTable::Create();
  _table_default_delegate = createDelegate(this,_table_default_delegate_funcz);
  _array_default_delegate = createDelegate(this,_array_default_delegate_funcz);
  _string_default_delegate = createDelegate(this,_string_default_delegate_funcz);
  _number_default_delegate = createDelegate(this,_number_default_delegate_funcz);
  _closure_default_delegate = createDelegate(this,_closure_default_delegate_funcz);
  _idxprop_default_delegate = createDelegate(this,_idxprop_default_delegate_funcz);
  _vec2f_default_delegate = createDelegate(this,_vec2f_default_delegate_funcz);
  _vec3f_default_delegate = createDelegate(this,_vec3f_default_delegate_funcz);
  _vec4f_default_delegate = createDelegate(this,_vec4f_default_delegate_funcz);
  _matrixf_default_delegate = createDelegate(this,_matrixf_default_delegate_funcz);
  _uuid_default_delegate = createDelegate(this,_uuid_default_delegate_funcz);
  _enum_default_delegate = createDelegate(this,_enum_default_delegate_funcz);
  _method_default_delegate = createDelegate(this,_method_default_delegate_funcz);
  _interface_default_delegate = createDelegate(this,_interface_default_delegate_funcz);
  _error_code_default_delegate = createDelegate(this,_error_code_default_delegate_funcz);
  _resolved_type_default_delegate = createDelegate(this,_resolved_type_default_delegate_funcz);

  _typeStr_null = _HC(typestr_null);
  _typeStr_int = _HC(typestr_int);
  _typeStr_float = _HC(typestr_float);
  _typeStr_string = _HC(typestr_string);
  _typeStr_table = _HC(typestr_table);
  _typeStr_array = _HC(typestr_array);
  _typeStr_function = _HC(typestr_function);
  _typeStr_userdata = _HC(typestr_userdata);
  _typeStr_iunknown = _HC(typestr_iunknown);
  _typeStr_closure = _HC(typestr_closure);
}

const SQObjectPtr& SQSharedState::GetTypeNameObj(SQObjectType type) const {
  switch (_RAW_TYPE(type)) {
    case _RT_NULL:
      return _typeStr_null;
    case _RT_INTEGER:
      return _typeStr_int;
    case _RT_FLOAT:
      return _typeStr_float;
    case _RT_STRING:
      return _typeStr_string;
    case _RT_TABLE:
      return _typeStr_table;
    case _RT_ARRAY:
      return _typeStr_array;
    case _RT_CLOSURE:
    case _RT_NATIVECLOSURE:
      return _typeStr_function;
    case _RT_USERDATA:
      return _typeStr_userdata;
    case _RT_FUNCPROTO:
      return _typeStr_function;
    case _RT_IUNKNOWN:
      return _typeStr_iunknown;
    default:
      return _null_;
  }
}

const achar* SQSharedState::GetTypeNameStr(SQObjectType type) const {
  const SQObjectPtr& obj = GetTypeNameObj(type);
  if (obj == _null_) return NULL;
  return _stringval(obj);
}

const iHString* SQSharedState::GetTypeNameHStr(SQObjectType type) const {
  const SQObjectPtr& obj = GetTypeNameObj(type);
  if (obj == _null_) return NULL;
  return _stringhval(obj);
}

static bool CompileTypemask(SQIntVec &res,const SQChar *typemask)
{
  int i = 0;

  int mask = 0;
  while(typemask[i] != 0) {

    switch(typemask[i]){
      case 'o': mask |= _RT_NULL; break;
      case 'i': mask |= _RT_INTEGER; break;
      case 'f': mask |= _RT_FLOAT; break;
      case 'n': mask |= (_RT_FLOAT | _RT_INTEGER); break;
      case 's': mask |= _RT_STRING; break;
      case 't': mask |= _RT_TABLE; break;
      case 'a': mask |= _RT_ARRAY; break;
      case 'u': mask |= _RT_USERDATA; break;
      case 'c': mask |= (_RT_CLOSURE | _RT_NATIVECLOSURE); break;
      case '.': mask = -1; res.push_back(mask); i++; mask = 0; continue;
      default:
        return false;
    }
    i++;
    if(typemask[i] == '|') {
      i++;
      if(typemask[i] == 0)
        return false;
      continue;
    }
    res.push_back(mask);
    mask = 0;

  }
  return true;
}

Ptr<SQNativeClosure> CreateSQNativeClosure(ain<SQRegFunction> aRegFunc) {
  niCheck(niStringIsOK(aRegFunc.name), nullptr);
  niCheck(aRegFunc.f != nullptr, nullptr);

  Ptr<SQNativeClosure> nc = SQNativeClosure::Create(aRegFunc.f);
  nc->_nparamscheck = aRegFunc.nparamscheck;
  nc->_name = _H(aRegFunc.name);
  if (HStringIsNotEmpty(aRegFunc.rettype)) {
    nc->_returntype = aRegFunc.rettype;
  }
  if (niStringIsOK(aRegFunc.typemask)) {
    if (!CompileTypemask(nc->_typecheck,aRegFunc.typemask)) {
      niError(niFmt("'%s' CompileTypemask '%s' failed.",
                    aRegFunc.name,
                    aRegFunc.typemask));
      return nullptr;
    }
  }
  return nc;
}

tBool RegisterSQRegFunction(ain_nn_mut<SQTable> aTable, ain<SQRegFunction> aRegFunc) {
  Ptr<SQNativeClosure> nc = CreateSQNativeClosure(aRegFunc);
  niCheckIsOK(nc,eFalse);
  aTable->NewSlot(nc->_name, nc.ptr());
  return eTrue;
}

tBool RegisterSQRegFunctions(ain_nn_mut<SQTable> aTable, ain<SQRegFunction*> aRegFuncs) {
  tBool ret = eTrue;
  int i = 0;
  while (niStringIsOK(aRegFuncs[i].name)) {
    niLet& reg = aRegFuncs[i];
    if (!RegisterSQRegFunction(aTable, reg)) {
      niError(niFmt("Can't register function '%s'.", reg.name));
      ret = eFalse;
    }
    ++i;
  }
  return ret;
}

SQSharedState::~SQSharedState()
{
  _table(_refs_table)->Invalidate();
  _refs_table = _null_;

  _table(_scriptimports_table)->Invalidate();
  _scriptimports_table = _null_;

  _table(_nativeimports_table)->Invalidate();
  _nativeimports_table = _null_;

  _table_default_delegate=_null_;
  _array_default_delegate=_null_;
  _string_default_delegate=_null_;
  _number_default_delegate=_null_;
  _closure_default_delegate=_null_;
}

#ifndef NO_GARBAGE_COLLECTOR
void SQGarbageCollector::MarkObject(SQObjectPtr &o,SQCollectable **chain)
{
  switch(_sqtype(o)){
    case OT_TABLE:
      _table(o)->Mark(chain);
      break;
    case OT_ARRAY:
      _array(o)->Mark(chain);
      break;
    case OT_CLOSURE:
      _closure(o)->Mark(chain);
      break;
    case OT_NATIVECLOSURE:
      _nativeclosure(o)->Mark(chain);
      break;
    case OT_IUNKNOWN|eTypeFlags_Collectable:
      _collectable(o)->Mark(chain);
      break;
  }
}

int SQGarbageCollector::CollectGarbage(SQCollectable** _tchain)
{
  AutoThreadLock chainLock(_gc_chain_mutex);
  _isCollecting = true;

  SQCollectable*& tchain = *_tchain;
  int n = 0;
  for (astl::set<SQObjectPtr>::iterator it = _gc_roots.begin();
       it != _gc_roots.end(); ++it)
  {
    MarkObject(const_cast<SQObjectPtr&>(*it),&tchain);
  }

  {
    SQCollectable *t = _gc_chain_ptr;
    SQCollectable *nx=NULL;
    {
      while(t) {
        niAssert(t->mnRootRefs <= 0);
        SQCOLLECTABLE_ADDREF(t);
        t->Invalidate();
        nx=t->_next;
        if (SQCOLLECTABLE_DECREF(t) == 0) {
          t->DeleteThis();
        }
        t=nx;
        n++;
      }
      t = tchain;
      while(t){
        t->UnMark();
        t=t->_next;
      }
    }
    _gc_chain_ptr = tchain;
  }

  _isCollecting = false;
  return n;
}

int SQGarbageCollector::GetNumRoots() {
  return _gc_roots.size();
}

void SQCollectable::AddToChain(SQCollectable **chain,SQCollectable *c)
{
  c->_prev=NULL;
  c->_next=*chain;
  if(*chain) (*chain)->_prev=c;
  *chain=c;
}

void SQCollectable::RemoveFromChain(SQCollectable **chain,SQCollectable *c)
{
  if(c->_prev) c->_prev->_next=c->_next;
  else *chain=c->_next;
  if(c->_next)
    c->_next->_prev=c->_prev;
  c->_next=NULL;
  c->_prev=NULL;
}

int SQGarbageCollector::AddRoot(const SQObjectPtr& o)
{
  AutoThreadLock lock(_gc_chain_mutex);

  ++_collectable(o)->mnRootRefs;
  if (_collectable(o)->mnRootRefs == 1) {
    DEBUG_GC_ROOT(("SQGarbageCollector::AddRoot: %p",o._var.mIntPtr));
    _gc_roots.insert(o);
  }
  return _collectable(o)->mnRootRefs;
}

int SQGarbageCollector::RemoveRoot(const SQObjectPtr& o)
{
  AutoThreadLock lock(_gc_chain_mutex);

  --_collectable(o)->mnRootRefs;
  if (_collectable(o)->mnRootRefs == 0) {
    DEBUG_GC_ROOT(("SQGarbageCollector::RemoveRoot: %p",o._var.mIntPtr));
    _gc_roots.erase(o);
  }

  return _collectable(o)->mnRootRefs;
}

tI32 Collectable_AddRootRef(SQCollectable* o) {
  SQObjectPtr optr(o,eTrue);
  return _gc()->AddRoot(optr);
}

tI32 Collectable_ReleaseRootRef(SQCollectable* o) {
  SQObjectPtr optr(o,eTrue);
  return _gc()->RemoveRoot(optr);
}

#else

tI32 Collectable_AddRootRef(SQCollectable* o) {
  return o->_CollectableAddRef();
}

tI32 Collectable_ReleaseRootRef(SQCollectable* o) {
  SQObjectPtr optr(o,eTrue);
  return o->_CollectableRelease();
}

#endif

const SQObjectPtr& SQSharedState::GetLangDelegate(const achar* delID) {
  if (mbLangDelegatesLocked)
    return _null_;
  if (StrEq(delID,"table")) {
    return _table_default_delegate;
  }
  else if (StrEq(delID,"array")) {
    return _array_default_delegate;
  }
  else if (StrEq(delID,"string")) {
    return _string_default_delegate;
  }
  else if (StrEq(delID,"number")) {
    return _number_default_delegate;
  }
  else if (StrEq(delID,"closure")) {
    return _closure_default_delegate;
  }
  else if (StrEq(delID,"vec2f")) {
    return _vec2f_default_delegate;
  }
  else if (StrEq(delID,"vec3f")) {
    return _vec3f_default_delegate;
  }
  else if (StrEq(delID,"vec4f")) {
    return _vec4f_default_delegate;
  }
  else if (StrEq(delID,"matrixf")) {
    return _matrixf_default_delegate;
  }
  else if (StrEq(delID,"uuid")) {
    return _uuid_default_delegate;
  }
  else if (StrEq(delID,"enum")) {
    return _enum_default_delegate;
  }
  else if (StrEq(delID,"method")) {
    return _method_default_delegate;
  }
  else if (StrEq(delID,"interface")) {
    return _interface_default_delegate;
  }
  else if (StrStartsWith(delID,"enum:")) {
    cString enumID = delID + 5;
    const sEnumDef* pEnumDef = ni::GetLang()->GetEnumDef(enumID.Chars());
    if (pEnumDef)
      return GetEnumDefTable(pEnumDef);
  }
  else if (StrStartsWith(delID,"iunknown:")) {
    cString intfID = delID + 9;
    tHStringPtr hsp = _H(intfID.Chars());
    const sInterfaceDef* pInterfaceDef = ni::GetLang()->GetInterfaceDefFromUUID(
        ni::GetLang()->GetInterfaceUUID(hsp));
    if (pInterfaceDef) {
      return GetInterfaceDelegate(*pInterfaceDef->mUUID);
    }
  }
  return _null_;
}
void SQSharedState::LockLangDelegates() {
  mbLangDelegatesLocked = eTrue;
}

#ifndef NO_GARBAGE_COLLECTOR
SQGarbageCollector::SQGarbageCollector() {
  _gc_chain_ptr = NULL;
  _gc_chain_sync = 0;
  _gc_chain_lastgc_sync = -1;
  _isCollecting = false;
}
#endif

SQObjectPtr _sq_metamethods[MT__LAST];
SQGarbageCollector* _gGC = NULL;

tBool _InitializeSQGCAndGlobals() {
  if (_sq_metamethods[0].IsNull()) {
    // meta methods, have to be declared in the same order as SQMetaMethod
    _sq_metamethods[MT_STD_TYPEOF] = _H(MM_STD_TYPEOF);
    _sq_metamethods[MT_STD_TOSTRING] = _H(MM_STD_TOSTRING);
    _sq_metamethods[MT_STD_CMP] = _H(MM_STD_CMP);
    _sq_metamethods[MT_STD_CALL] = _H(MM_STD_CALL);
    _sq_metamethods[MT_STD_NEXTI] = _H(MM_STD_NEXTI);
    _sq_metamethods[MT_ARITH_ADD] = _H(MM_ARITH_ADD);
    _sq_metamethods[MT_ARITH_SUB] = _H(MM_ARITH_SUB);
    _sq_metamethods[MT_ARITH_MUL] = _H(MM_ARITH_MUL);
    _sq_metamethods[MT_ARITH_DIV] = _H(MM_ARITH_DIV);
    _sq_metamethods[MT_ARITH_UNM] = _H(MM_ARITH_UNM);
    _sq_metamethods[MT_ARITH_MODULO] = _H(MM_ARITH_MODULO);
    _sq_metamethods[MT_TABLE_INVALIDATE] = _H(MM_TABLE_INVALIDATE);
    _sq_metamethods[MT_USERDATA_SET] = _H(MM_USERDATA_SET);
    _sq_metamethods[MT_USERDATA_GET] = _H(MM_USERDATA_GET);
    _sq_metamethods[MT_USERDATA_NEWSLOT] = _H(MM_USERDATA_NEWSLOT);
    _sq_metamethods[MT_USERDATA_DELSLOT] = _H(MM_USERDATA_DELSLOT);
    _sq_metamethods[MT_IUNKNOWN_CLONE] = _H(MM_IUNKNOWN_CLONE);
  }

#ifndef NO_GARBAGE_COLLECTOR
  if (_gGC == NULL) {
    _gGC = new SQGarbageCollector();
  }
#endif
  return eTrue;
}
