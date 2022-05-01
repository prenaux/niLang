#include "stdafx.h"

#include "sqconfig.h"
#include "sqopcodes.h"
#include "sqvm.h"
#include "sqstate.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqtable.h"
#include "sqarray.h"

#ifdef NO_GARBAGE_COLLECTOR
#pragma niNote("NO GARBAGE COLLECTOR")
#else
#pragma niNote("USING GARBAGE COLLECTOR")
#endif

#define DEBUG_VM_ROOT //niDebugFmt

#define newsysstring(s)  _systemstrings.push_back(_H(s));
#define newmetamethod(s) _metamethods.push_back(_H(s));

static __noinline SQTable* _CreateDefaultDelegate(SQSharedState *ss,SQRegFunction *funcz)
{
  int i=0;
  SQTable *t = SQTable::Create();
  while (funcz[i].name!=0) {
    Ptr<SQNativeClosure> nc = SQNativeClosure::Create(funcz[i].f);
    nc->_nparamscheck = funcz[i].nparamscheck;
    nc->_name = _H(funcz[i].name);
    if (funcz[i].typemask && !CompileTypemask(nc->_typecheck,funcz[i].typemask))
      return NULL;
    t->NewSlot(_H(funcz[i].name),nc.ptr());
    i++;
  }
  return t;
}

SQObjectPtr _null_=SQObjectPtr();
SQObjectPtr _notnull_(1);
SQObjectPtr _one_(1);
SQObjectPtr _minusone_(-1);

SQSharedState::SQSharedState() {
  _compilererrorhandler = NULL;
  _debuginfo = false;
#ifndef NO_GARBAGE_COLLECTOR
  _gc_chain_ptr = NULL;
  _gc_chain_sync = 0;
  _gc_chain_lastgc_sync = -1;
  _isCollecting = false;
#endif
  mbLangDelegatesLocked = eFalse;
}

tBool SQSharedState::Init() {
  newsysstring(_A("null"));
  newsysstring(_A("table"));
  newsysstring(_A("array"));
  newsysstring(_A("closure"));
  newsysstring(_A("string"));
  newsysstring(_A("userdata"));
  newsysstring(_A("int"));
  newsysstring(_A("float"));
  newsysstring(_A("function"));

  // meta methods, have to be declared in the same order as SQMetaMethod
  newmetamethod(MM_STD_TYPEOF);
  newmetamethod(MM_STD_TOSTRING);
  newmetamethod(MM_STD_CMP);
  newmetamethod(MM_STD_CALL);
  newmetamethod(MM_STD_NEXTI);
  newmetamethod(MM_ARITH_ADD);
  newmetamethod(MM_ARITH_SUB);
  newmetamethod(MM_ARITH_MUL);
  newmetamethod(MM_ARITH_DIV);
  newmetamethod(MM_ARITH_UNM);
  newmetamethod(MM_ARITH_MODULO);
  newmetamethod(MM_TABLE_INVALIDATE);
  newmetamethod(MM_USERDATA_SET);
  newmetamethod(MM_USERDATA_GET);
  newmetamethod(MM_USERDATA_NEWSLOT);
  newmetamethod(MM_USERDATA_DELSLOT);
  newmetamethod(MM_IUNKNOWN_CLONE);

  _refs_table = SQTable::Create();
  _table_default_delegate=_CreateDefaultDelegate(this,_table_default_delegate_funcz);
  _array_default_delegate=_CreateDefaultDelegate(this,_array_default_delegate_funcz);
  _string_default_delegate=_CreateDefaultDelegate(this,_string_default_delegate_funcz);
  _number_default_delegate=_CreateDefaultDelegate(this,_number_default_delegate_funcz);
  _closure_default_delegate=_CreateDefaultDelegate(this,_closure_default_delegate_funcz);
  _idxprop_default_delegate=_CreateDefaultDelegate(this,_idxprop_default_delegate_funcz);
  _vec2f_default_delegate=_CreateDefaultDelegate(this,_vec2f_default_delegate_funcz);
  _vec3f_default_delegate=_CreateDefaultDelegate(this,_vec3f_default_delegate_funcz);
  _vec4f_default_delegate=_CreateDefaultDelegate(this,_vec4f_default_delegate_funcz);
  _matrixf_default_delegate=_CreateDefaultDelegate(this,_matrixf_default_delegate_funcz);
  _uuid_default_delegate=_CreateDefaultDelegate(this,_uuid_default_delegate_funcz);
  _enum_default_delegate=_CreateDefaultDelegate(this,_enum_default_delegate_funcz);
  _method_default_delegate=_CreateDefaultDelegate(this,_method_default_delegate_funcz);

  _typeStr_null = _H("null");
  _typeStr_int = _H("int");
  _typeStr_float = _H("float");
  _typeStr_string = _H("string");
  _typeStr_table = _H("table");
  _typeStr_array = _H("array");
  _typeStr_function = _H("function");
  _typeStr_userdata = _H("userdata");
  _typeStr_vm = _H("vm");
  _typeStr_iunknown = _H("iunknown");
  return eTrue;
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

bool CompileTypemask(SQIntVec &res,const SQChar *typemask)
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

SQSharedState::~SQSharedState()
{
  _table(_refs_table)->Invalidate();
  _refs_table = _null_;

  while (!_systemstrings.empty()){
    _systemstrings.back()=_null_;
    _systemstrings.pop_back();
  }

  _table_default_delegate=_null_;
  _array_default_delegate=_null_;
  _string_default_delegate=_null_;
  _number_default_delegate=_null_;
  _closure_default_delegate=_null_;
  _metamethods.clear();
  _systemstrings.clear();
}

#ifndef NO_GARBAGE_COLLECTOR
void SQSharedState::MarkObject(SQObjectPtr &o,SQCollectable **chain)
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

int SQSharedState::CollectGarbage(SQCollectable** _tchain)
{
  AutoThreadLock chainLock(_gc_chain_mutex);
  _isCollecting = true;

  SQCollectable*& tchain = *_tchain;
  int n = 0;
  MarkObject(_refs_table,&tchain);
  MarkObject(_table_default_delegate,&tchain);
  MarkObject(_array_default_delegate,&tchain);
  MarkObject(_string_default_delegate,&tchain);
  MarkObject(_number_default_delegate,&tchain);
  MarkObject(_closure_default_delegate,&tchain);
  MarkObject(_idxprop_default_delegate,&tchain);
  MarkObject(_vec2f_default_delegate,&tchain);
  MarkObject(_vec3f_default_delegate,&tchain);
  MarkObject(_vec4f_default_delegate,&tchain);
  MarkObject(_matrixf_default_delegate,&tchain);
  MarkObject(_uuid_default_delegate,&tchain);
  MarkObject(_enum_default_delegate,&tchain);
  MarkObject(_method_default_delegate,&tchain);
  for (tDelegateMap::iterator it = mmapDelegates.begin(); it != mmapDelegates.end(); ++it) {
    MarkObject(it->second,&tchain);
  }
  for (tEnumDefMap::iterator it = mmapEnumDefs.begin(); it != mmapEnumDefs.end(); ++it) {
    MarkObject(it->second,&tchain);
  }
  for (astl::set<SQObjectPtr>::iterator it = _gc_roots.begin(); it != _gc_roots.end(); ++it) {
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
        else {
          niAssertUnreachable("Invalidated object still referenced, failed to break cycle.");
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

int SQSharedState::GetNumRoots() {
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

int SQSharedState::AddRoot(const SQObjectPtr& o)
{
  AutoThreadLock lock(_gc_chain_mutex);
  niAssert(_refs_table != _null_);

  ++_collectable(o)->mnRootRefs;
  if (_collectable(o)->mnRootRefs == 1) {
    DEBUG_VM_ROOT(("V/vm_addRoot: %p",o._var.mIntPtr));
    _gc_roots.insert(o);
  }
  return _collectable(o)->mnRootRefs;
}

int SQSharedState::RemoveRoot(const SQObjectPtr& o)
{
  AutoThreadLock lock(_gc_chain_mutex);
  niAssert(_refs_table != _null_);

  --_collectable(o)->mnRootRefs;
  if (_collectable(o)->mnRootRefs == 0) {
    DEBUG_VM_ROOT(("V/vm_removeRoot: %p",o._var.mIntPtr));
    _gc_roots.erase(o);
  }

  return _collectable(o)->mnRootRefs;
}

tI32 Collectable_AddRootRef(SQCollectable* o) {
  SQObjectPtr optr(o,eTrue);
  return _ss()->AddRoot(optr);
}

tI32 Collectable_ReleaseRootRef(SQCollectable* o) {
  SQObjectPtr optr(o,eTrue);
  return _ss()->RemoveRoot(optr);
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

const SQObjectPtr& SQSharedState::GetLangDelegate(HSQUIRRELVM v, const achar* delID) {
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
      return GetInterfaceDelegate(v,*pInterfaceDef->mUUID);
    }
  }
  return _null_;
}
void SQSharedState::LockLangDelegates() {
  mbLangDelegatesLocked = eTrue;
}

SQSharedState* _gSS = NULL;
tBool SQSharedState::_Initialize() {
  if (_gSS != NULL) {
    return eTrue;
  }
  _gSS = new SQSharedState();
  _gSS->Init();
  return eTrue;
}
