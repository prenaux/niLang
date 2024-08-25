#include "stdafx.h"

#include "sqvm.h"
#include "sqtable.h"
#include "sqarray.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqfuncstate.h"
#include "sqcompiler.h"
#include "sqobject.h"

#include "ScriptVM.h"
#include "ScriptTypes.h"

niExportFunc(bool) sq_aux_gettypedarg_(const ni::achar* section, HSQUIRRELVM v,int idx,SQObjectType type,SQObjectPtr **o)
{
  *o = &stack_get(v,idx);
  if(_sqtype(**o) != type){
    SQObjectPtr oval = v->PrintObjVal(**o);
    niRTError(v,niFmt(_A("[%s] wrong argument type, expected '%s' got '%.50s'"),
                      section,sq_typeof(v,idx),_stringval(oval)));
    return false;
  }
  return true;
}

niExportFunc(bool) sq_aux_gettypedarg(HSQUIRRELVM v,int idx,SQObjectType type,SQObjectPtr **o)
{
  return sq_aux_gettypedarg_(_A("unknown"),v,idx,type,o);
}

#define _GETSAFE_OBJ(ZONE,v,idx,type,o) { if(!sq_aux_gettypedarg_(ZONE,v,idx,type,&o)) return SQ_ERROR; }
#define sq_aux_paramscheck(v,count)  {  if(sq_gettop(v) < count){ niRTError(v,_A("not enough params in the stack")); return SQ_ERROR; } }

niExportFunc(int) sq_aux_invalidtype(HSQUIRRELVM v,SQObjectType type)
{
  return sq_throwerror(v, niFmt(_A("unexpected type %s"), v->_ss->GetTypeNameStr(type)));
}

niExportFunc(void) sq_seterrorhandler(HSQUIRRELVM v)
{
  SQObjectPtr o = stack_get(v, -1);
  if(sq_isclosure(o) || sq_isnativeclosure(o) || sq_isnull(o)) {
    v->_errorhandler = o;
    v->Pop();
  }
}

niExportFunc(void) sq_setdebughook(HSQUIRRELVM v)
{
  SQObjectPtr o = stack_get(v,-1);
  if(sq_isclosure(o) || sq_isnativeclosure(o) || sq_isnull(o)) {
    v->_debughook = o;
    v->Pop();
  }
}

niExportFunc(SQRESULT) sq_compile(HSQUIRRELVM v,SQLEXREADFUNC read,ni::tPtr p,const SQChar *sourcename,int raiseerror)
{
  SQObjectPtr o;
  if (CompileScript(v, read, p, sourcename, o, raiseerror>0?true:false, v->_ss->_debuginfo)) {
    o = SQClosure::Create(_funcproto(o),_table(v->_roottable));
    v->Push(o);
    return SQ_OK;
  }
  return SQ_ERROR;
}

niExportFunc(void) sq_enabledebuginfos(HSQUIRRELVM v, int debuginfo)
{
  v->_ss->_debuginfo = debuginfo!=0?true:false;
}

niExportFunc(bool) sq_aredebuginfosenabled(HSQUIRRELVM v)
{
  return v->_ss->_debuginfo;
}

niExportFunc(int) sq_addref(SQSharedState& aSS,HSQOBJECT *po)
{
  if (aSS._refs_table == _null_)
    return -1;
  SQObjectPtr refs;
  if (!ISREFCOUNTED(_sqtype(*po)) && _sqtype(*po) != OT_IUNKNOWN)
    return -1;
  if (_table(aSS._refs_table)->Get(*po, refs)) {
    refs = (SQInt)(_int(refs) + 1);
  }
  else{
    refs = 1;
  }
  _table(aSS._refs_table)->NewSlot(*po, refs);
  return _int(refs);
}

niExportFunc(int) sq_release(SQSharedState& aSS,HSQOBJECT *po)
{
  if (aSS._refs_table == _null_)
    return -1;
  SQObjectPtr refs;
  if(!ISREFCOUNTED(_sqtype(*po)) && _sqtype(*po) != OT_IUNKNOWN)
    return -1;
  if (_table(aSS._refs_table)->Get(*po, refs)) {
    int n = _int(refs) - 1;
    if (n <= 0) {
      _table(aSS._refs_table)->Remove(*po);
      sq_resetobject(po);
    }
    else {
      refs = n;
      _table(aSS._refs_table)->Set(*po, refs);
    }
  }
  return _int(refs);
}

niExportFunc(void) sq_pushnull(HSQUIRRELVM v)
{
  v->Push(_null_);
}

niExportFunc(void) sq_pushstring(HSQUIRRELVM v, const iHString* hstr)
{
  if (!HStringIsEmpty(hstr)) {
    v->Push(SQObjectPtr(hstr));
  }
  else {
    v->Push(v->GetEmptyString());
  }
}

niExportFunc(void) sq_pushint(HSQUIRRELVM v,SQInt n)
{
  v->Push(n);
}

niExportFunc(void) sq_pushf32(HSQUIRRELVM v,tF32 n)
{
  v->Push((SQFloat)n);
}

niExportFunc(void) sq_pushf64(HSQUIRRELVM v,tF64 n)
{
  v->Push((SQFloat)n);
}

niExportFunc(void) sq_pushud(HSQUIRRELVM v,SQUserData* ud)
{
  v->Push(ud);
}

niExportFunc(void) sq_newtable(HSQUIRRELVM v)
{
  v->Push(SQTable::Create());
}

niExportFunc(void) sq_newarray(HSQUIRRELVM v,int size)
{
  v->Push(SQArray::Create(size));
}

niExportFunc(SQRESULT) sq_arrayappend(HSQUIRRELVM v,int idx)
{
  sq_aux_paramscheck(v,2);
  SQObjectPtr *arr;
  _GETSAFE_OBJ(_A("sq_arrayappend"), v, idx, OT_ARRAY,arr);
  _array(*arr)->Append(v->GetUp(-1));
  v->Pop(1);
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_arraypop(HSQUIRRELVM v,int idx, int pushval)
{
  sq_aux_paramscheck(v, 1);
  SQObjectPtr *arr;
  _GETSAFE_OBJ(_A("sq_arraypop"), v, idx, OT_ARRAY,arr);
  if(_array(*arr)->Size() > 0) {
    if(pushval != 0){ v->Push(_array(*arr)->Top()); }
    _array(*arr)->Pop();
    return SQ_OK;
  }
  return sq_throwerror(v, _A("empty array"));
}

niExportFunc(SQRESULT) sq_arrayresize(HSQUIRRELVM v,int idx,int newsize)
{
  sq_aux_paramscheck(v,1);
  SQObjectPtr *arr;
  _GETSAFE_OBJ(_A("sq_arrayresize"), v, idx, OT_ARRAY,arr);
  if(_array(*arr)->Size() > 0) {
    _array(*arr)->Resize(newsize);
    return SQ_OK;
  }
  return sq_throwerror(v, _A("empty array"));
}

niExportFunc(SQRESULT) sq_arrayreverse(HSQUIRRELVM v,int idx)
{
  sq_aux_paramscheck(v, 1);
  SQObjectPtr *o;
  _GETSAFE_OBJ(_A("sq_arrayreverse"), v, idx, OT_ARRAY,o);
  SQArray *arr = _array(*o);
  if(arr->Size() > 0) {
    SQObjectPtr t;
    int size = arr->Size();
    int n = size >> 1; size -= 1;
    for(int i = 0; i < n; i++) {
      t = arr->_values[i];
      arr->_values[i] = arr->_values[size-i];
      arr->_values[size-i] = t;
    }
  }
  return SQ_OK;
}

niExportFunc(void) sq_newclosure(HSQUIRRELVM v,SQFUNCTION func)
{
  SQNativeClosure *nc = SQNativeClosure::Create(func);
  nc->_nparamscheck = 0;
  v->Push(SQObjectPtr(nc));
}

niExportFunc(SQRESULT) sq_getclosureinfo(HSQUIRRELVM v,int idx,ni::tU32* nparams,ni::tU32* nfreevars)
{
  SQObjectPtr o = stack_get(v, idx);
  if(sq_isclosure(o)) {
    SQClosure *c = _closure(o);
    SQFunctionProto *proto = _funcproto(c->_function);
    *nparams = (tU32)proto->_parameters.size();
    *nfreevars = (tU32)c->_outervalues.size();
    return SQ_OK;
  }
  return sq_throwerror(v,_A("the object is not a closure"));
}

niExportFunc(SQRESULT) sq_setnativeclosurename(HSQUIRRELVM v,int idx,const SQChar *name)
{
  SQObjectPtr o = stack_get(v, idx);
  if(sq_isnativeclosure(o)) {
    SQNativeClosure *nc = _nativeclosure(o);
    nc->_name = _H(name);
    return SQ_OK;
  }
  return sq_throwerror(v,_A("the object is not a nativeclosure"));
}

niExportFunc(void) sq_pushroottable(HSQUIRRELVM v)
{
  v->Push(v->_roottable);
}

niExportFunc(SQRESULT) sq_setroottable(HSQUIRRELVM v)
{
  SQObjectPtr o = stack_get(v, -1);
  if(sq_istable(o) || sq_isnull(o)) {
    v->_roottable = o;
    v->Pop();
    return SQ_OK;
  }
  return sq_throwerror(v, _A("ivalid type"));
}

niExportFunc(void) sq_setforeignptr(HSQUIRRELVM v,ni::tPtr p)
{
  v->_foreignptr = p;
}

niExportFunc(ni::tPtr) sq_getforeignptr(HSQUIRRELVM v)
{
  return (tPtr)v->_foreignptr;
}

niExportFunc(void) sq_push(HSQUIRRELVM v,int idx)
{
  v->Push(stack_get(v, idx));
}

niExportFunc(SQObjectType) sq_gettype(HSQUIRRELVM v,int idx)
{
  return _sqtype(stack_get(v, idx));
}

cString sq_typeof(HSQUIRRELVM v,int idx)
{
  SQObjectPtr obj = stack_get(v, idx);
  SQObjectPtr typeName = _null_;
  v->TypeOf(obj,typeName);
  if (!sq_isstring(typeName))
    typeName = _H("<?>");
  return _stringval(typeName);
}

niExportFunc(SQRESULT) sq_getiunknown(HSQUIRRELVM v,int idx,iUnknown** i)
{
  SQObjectPtr &o = stack_get(v, idx);
  if (sq_isnull(o)) {
    *i = NULL;
    return SQ_OK;
  }
  else if (_sqtype(o) == OT_IUNKNOWN) {
    *i = _iunknown(o);
    return SQ_OK;
  }
  return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_getint(HSQUIRRELVM v,int idx,SQInt *i)
{
  SQObjectPtr &o = stack_get(v, idx);
  if (sq_isnull(o)) {
    *i = 0;
    return SQ_OK;
  }
  else if (sq_isnumeric(o)) {
    *i = toint(o);
    return SQ_OK;
  }
  return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_getf32(HSQUIRRELVM v,int idx,ni::tF32 *f)
{
  SQObjectPtr &o = stack_get(v, idx);
  if (sq_isnull(o)) {
    *f = 0.0f;
    return SQ_OK;
  }
  else if(sq_isnumeric(o)) {
    *f = (ni::tF32)tofloat(o);
    return SQ_OK;
  }
  return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_getf64(HSQUIRRELVM v,int idx,ni::tF64 *f)
{
  SQObjectPtr &o = stack_get(v, idx);
  if (sq_isnull(o)) {
    *f = 0.0;
    return SQ_OK;
  }
  else if(sq_isnumeric(o)) {
    *f = (ni::tF64)tofloat(o);
    return SQ_OK;
  }
  return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_getstring(HSQUIRRELVM v,int idx,const SQChar **c)
{
  if (sq_gettype(v,idx) == OT_NULL) {
    *c = AZEROSTR;
    return SQ_OK;
  }
  SQObjectPtr *o = NULL;
  _GETSAFE_OBJ(_A("sq_getstring"), v, idx, OT_STRING,o);
  *c = _stringval(*o);
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_gethstring(HSQUIRRELVM v,int idx,iHString **c)
{
  if (sq_gettype(v,idx) == OT_NULL) {
    *c = NULL;
    return SQ_OK;
  }
  SQObjectPtr *o = NULL;
  _GETSAFE_OBJ(_A("sq_gethstring"), v, idx, OT_STRING,o);
  *c = _stringhval(*o);
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_deep_clone(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &o = stack_get(v,idx);
  v->Push(_null_);
  tSQDeepCloneGuardSet deepCloneGuardSet;
  if(!v->Clone(o, stack_get(v, -1), &deepCloneGuardSet)){
    v->Pop();
    return sq_aux_invalidtype(v, _sqtype(o));
  }
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_shallow_clone(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &o = stack_get(v,idx);
  v->Push(_null_);
  if(!v->Clone(o, stack_get(v, -1), NULL)){
    v->Pop();
    return sq_aux_invalidtype(v, _sqtype(o));
  }
  return SQ_OK;
}

niExportFunc(SQInt) sq_getsize(HSQUIRRELVM v, int idx)
{
  SQObjectPtr &o = stack_get(v, idx);
  SQObjectType type = _sqtype(o);
  switch(type) {
    case OT_STRING:   return _stringlen(o);
    case OT_TABLE:    return _table(o)->CountUsed();
    case OT_ARRAY:    return _array(o)->Size();
    case OT_USERDATA: return _userdata(o)->GetSize();
    default:
      return sq_aux_invalidtype(v, type);
  }
}

niExportFunc(SQRESULT) sq_getuserdata(HSQUIRRELVM v,int idx,SQUserData** p)
{
  SQObjectPtr *o = NULL;
  _GETSAFE_OBJ(_A("sq_getuserdata"), v, idx, OT_USERDATA,o);
  (*p) = _userdata(*o);
  return SQ_OK;
}

niExportFunc(int) sq_gettop(HSQUIRRELVM v)
{
  return (v->_top) - v->_stackbase;
}

niExportFunc(void) sq_settop(HSQUIRRELVM v, int newtop)
{
  int top = sq_gettop(v);
  if(top > newtop)
    sq_pop(v, top - newtop);
  else
    while(top < newtop) sq_pushnull(v);
}

niExportFunc(void) sq_pop(HSQUIRRELVM v, int nelemstopop)
{
  niAssert(v->_top >= nelemstopop);
  v->Pop(nelemstopop);
}

niExportFunc(void) sq_remove(HSQUIRRELVM v, int idx)
{
  v->Remove(idx);
}

niExportFunc(int) sq_cmp(HSQUIRRELVM v)
{
  int res = eInvalidHandle;
  v->ObjCmp(stack_get(v, -1), stack_get(v, -2),res);
  return res;
}

niExportFunc(SQRESULT) sq_createslot(HSQUIRRELVM v, int idx)
{
  sq_aux_paramscheck(v, 3);
  SQObjectPtr &self = stack_get(v, idx);
  if(_sqtype(self) == OT_TABLE) {
    SQObjectPtr &key = v->GetUp(-2);
    if(_sqtype(key) == OT_NULL) return sq_throwerror(v, _A("null is not a valid key"));
    v->NewSlot(self, key, v->GetUp(-1), 0);
    v->Pop(2);
  }
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_deleteslot(HSQUIRRELVM v,int idx,int pushval)
{
  sq_aux_paramscheck(v, 2);
  SQObjectPtr *self;
  _GETSAFE_OBJ(_A("sq_deleteslot"), v, idx, OT_TABLE,self);
  SQObjectPtr &key = v->GetUp(-1);
  if(_sqtype(key) == OT_NULL) return sq_throwerror(v, _A("null is not a valid key"));
  SQObjectPtr res;
  if(!v->DeleteSlot(*self, key, res, 0)){
    return SQ_ERROR;
  }
  if(pushval) v->GetUp(-1) = res;
  else v->Pop(1);
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_set(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &self = stack_get(v, idx);
  if(v->Set(self, v->GetUp(-2), v->GetUp(-1), 0)) {
    v->Pop(2);
    return SQ_OK;
  }
  v->Raise_IdxError(v->GetUp(-2));return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_rawset(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &self = stack_get(v, idx);
  if (_sqtype(v->GetUp(-2)) == OT_NULL)
    return sq_throwerror(v, _A("null key"));
  switch(_sqtype(self)) {
    case OT_TABLE:
      _table(self)->NewSlot(v->GetUp(-2), v->GetUp(-1));
      v->Pop(2);
      return SQ_OK;
      break;
    case OT_ARRAY:
      if(v->Set(self, v->GetUp(-2), v->GetUp(-1), 0)) {
        v->Pop(2);
        return SQ_OK;
      }
      break;
    default:
      v->Pop(2);
      return sq_throwerror(v, _A("rawset works only on tables & arrays."));
  }
  v->Raise_IdxError(v->GetUp(-2));return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_setdelegate(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &self = stack_get(v, idx);
  SQObjectPtr &mt = v->GetUp(-1);
  SQObjectType type = _sqtype(self);
  switch(type) {
    case OT_TABLE:
      if (_sqtype(mt) == OT_TABLE) {
        if (!_table(self)->SetDelegate(_table(mt)))
          return sq_throwerror(v, _A("delagate cycle")); v->Pop();
      }
      else if(_sqtype(mt)==OT_NULL) {
        _table(self)->SetDelegate(NULL);
        v->Pop();
      }
      else return sq_aux_invalidtype(v,type);
      break;
    case OT_USERDATA:
      if (_sqtype(mt)==OT_TABLE) {
        _userdata(self)->SetDelegate(_table(mt));
        v->Pop();
      }
      else if(_sqtype(mt)==OT_NULL) {
        _userdata(self)->SetDelegate(NULL);
        v->Pop();
      }
      else return sq_aux_invalidtype(v, type);
      break;
    default:
      return sq_aux_invalidtype(v, type);
      break;
  }
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_rawdeleteslot(HSQUIRRELVM v,int idx,int pushval)
{
  sq_aux_paramscheck(v, 2);
  SQObjectPtr *self;
  _GETSAFE_OBJ(_A("sq_rawdeleteslot"), v, idx, OT_TABLE,self);
  SQObjectPtr &key = v->GetUp(-1);
  SQObjectPtr t;
  if(_table(*self)->Get(key,t)) {
    _table(*self)->Remove(key);
  }
  if(pushval != 0) {
    if(pushval) v->GetUp(-1) = t;
    else
      v->Pop(1);
  }
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_getdelegate(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &self=stack_get(v,idx);
  switch(_sqtype(self)){
    case OT_TABLE:
      if(!_table(self)->GetDelegate())break;
      v->Push(SQObjectPtr(_table(self)->GetDelegate()));
      return SQ_OK;
    case OT_USERDATA:
      if(!_userdata(self)->GetDelegate())break;
      v->Push(SQObjectPtr(_userdata(self)->GetDelegate()));
      return SQ_OK;
  }
  return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_getparent(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &self=stack_get(v,idx);
  switch(_sqtype(self)){
    case OT_TABLE:
      if (!_table(self)->GetParent()) break;
      v->Push(SQObjectPtr(_table(self)->GetParent()));
      return SQ_OK;
  }
  return SQ_ERROR;
}

static cString GetObjectString(const SQSharedState& aSS,
                               const SQObjectPtr &obj) {
  cString val;
  switch (_sqtype(obj)) {
    case OT_STRING: val = _stringval(obj); break;
    case OT_TABLE:  val = _table(obj)->GetDebugName(); break;
  }
  return niFmt(_A("type:%s:%s"),aSS.GetTypeNameStr(obj),val.Chars());
}

niExportFunc(SQRESULT) sq_get(HSQUIRRELVM v,int idx)
{
  SQObjectPtr &self=stack_get(v,idx);
  if (v->Get(self,v->GetUp(-1),v->GetUp(-1),nullptr,0)) {
    return SQ_OK;
  }
  v->Pop(1);
  //return sq_throwerror(v,niFmt(_A("sq_get (%d), the index '%s' doesn't exist"),counter++,GetObjectString(self).Chars()));
  return SQ_ERROR;
}

niExportFunc(SQRESULT) sq_rawget(HSQUIRRELVM v,int idx)
{
  static int counter = 0;
  SQObjectPtr &self=stack_get(v,idx);
  switch(_sqtype(self)) {
    case OT_TABLE:
      if(_table(self)->Get(v->GetUp(-1),v->GetUp(-1)))
        return SQ_OK;
      break;
    case OT_ARRAY:
      if (v->Get(self,v->GetUp(-1),v->GetUp(-1),NULL,_OPEXT_GET_RAW))
        return SQ_OK;
      break;
    default:
      v->Pop(1);
      return sq_throwerror(v,_A("rawget works only on tables & arrays."));
  }
  v->Pop(1);
  return sq_throwerror(v,niFmt(_A("sq_rawget (%d), the index '%s' doesn't exist"),counter++,GetObjectString(*v->_ss,self).Chars()));
}

niExportFunc(SQRESULT) sq_getstackobj(HSQUIRRELVM v,int idx,HSQOBJECT *po)
{
  *po=stack_get(v,idx);
  return SQ_OK;
}

niExportFunc(const SQChar*) sq_getlocal(HSQUIRRELVM v,unsigned int level,unsigned int idx)
{
  tU32 cstksize = (tU32)v->_callsstack.size();
  tU32 lvl = (cstksize-level)-1;
  int stackbase=v->_stackbase;
  if(lvl<cstksize){
    for(unsigned int i=0;i<level;i++){
      SQCallInfo &ci=v->_callsstack[(cstksize-i)-1];
      stackbase-=ci._prevstkbase;
    }
    SQCallInfo &ci=v->_callsstack[lvl];
    if (_sqtype(ci._closurePtr) != OT_CLOSURE)
      return NULL;
    SQClosure* c = _closure(ci._closurePtr);
    SQFunctionProto *func=_funcproto(c->_function);
    return func->GetLocal(v,stackbase,idx,(ci._ip-(&func->_instructions[0]))-1);
  }
  return NULL;
}

niExportFunc(void) sq_pushobject(HSQUIRRELVM v,HSQOBJECT obj)
{
  v->Push(SQObjectPtr(obj));
}

niExportFunc(void) sq_resetobject(HSQOBJECT *po)
{
  po->_var.mType = OT_NULL;
}

niExportFunc(int) sq_throwerror(HSQUIRRELVM v,const SQChar *err)
{
  v->Raise_MsgError(err);
  return -1;
}

niExportFunc(void) sq_getlasterror(HSQUIRRELVM v)
{
  v->Push(v->_lasterror);
}

niExportFunc(void) sq_reservestack(HSQUIRRELVM v,int nsize)
{
  if (((unsigned int)v->_top + nsize) > v->_stack.size()) {
    v->_stack.resize(v->_stack.size() + ((v->_top + nsize) - v->_stack.size()));
  }
}

niExportFunc(SQRESULT) sq_call(HSQUIRRELVM v,int params,int retval)
{
  SQObjectPtr res = _null_;
  if (v->Call(v->GetUp(-(params+1)),params,v->_top-params,res)){
    v->Pop(params); //pop closure and args
    if (retval) {
      v->Push(res);
      return SQ_OK;
    }
    return SQ_OK;
  }
  else {
    v->Pop(params);
    return SQ_ERROR;
  }
}

niExportFunc(void) sq_setcompilererrorhandler(HSQUIRRELVM v,SQCOMPILERERROR f)
{
  v->_ss->_compilererrorhandler=f;
}

niExportFunc(SQRESULT) sq_writeclosure(HSQUIRRELVM v,SQWRITEFUNC w,ni::tPtr up)
{
  SQObjectPtr *o = NULL;
  _GETSAFE_OBJ(_A("sq_writeclosure"), v, -1, OT_CLOSURE,o);
  if (!WriteSQClosure(_closure(*o),v,up,w))
    return SQ_ERROR;
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_readclosure(HSQUIRRELVM v,SQREADFUNC r,ni::tPtr up)
{
  SQObjectPtr func=SQFunctionProto::Create();
  SQObjectPtr closure=SQClosure::Create(_funcproto(func),_table(v->_roottable));
  if(!ReadSQClosure(_closure(closure),v,up,r))
    return SQ_ERROR;
  v->Push(closure);
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_setfreevariable(HSQUIRRELVM v,int idx,unsigned int nval)
{
  SQObjectPtr &self=stack_get(v,idx);
  switch(_sqtype(self))
  {
    case OT_CLOSURE:
      if(_closure(self)->_outervalues.size()>nval){
        _closure(self)->_outervalues[nval]=stack_get(v,-1);
      }
      else return sq_throwerror(v,_A("invalid free var index"));
      break;
    default:
      return sq_aux_invalidtype(v,_sqtype(self));
  }
  v->Pop(1);
  return SQ_OK;
}

niExportFunc(SQRESULT) sq_next(HSQUIRRELVM v,int idx)
{
  SQObjectPtr o=stack_get(v,idx),key=stack_get(v,-1),realkey,val;
  switch(_sqtype(stack_get(v,idx))) {
    case OT_TABLE: {
      SQObjectPtr itr;
      if (!_table(o)->Next(key,realkey,val,itr))
        return SQ_ERROR;
      stack_get(v,-1) = itr;
      break;
    }
    case OT_ARRAY: {
      int nrefidx;
      if ((nrefidx=_array(o)->Next(key,realkey,val))==-1)
        return SQ_ERROR;
      stack_get(v,-1) = (SQInt)nrefidx;
      break;
    }
    case OT_STRING: {
      SQObjectPtr itr;
      if (!vm_string_nexti(_stringobj(o),key,realkey,val,itr))
        return SQ_ERROR;
      stack_get(v,-1) = itr;
      break;
    }
    default:
      return sq_aux_invalidtype(v,_sqtype(o));
  }
  v->Push(realkey);
  v->Push(val);
  return SQ_OK;
}

struct BufState{
  const SQChar *buf;
  int ptr;
  int size;
};

SQInt buf_lexfeed(ni::tPtr file)
{
  BufState *buf=(BufState*)file;
  if(buf->size<(buf->ptr+1))
    return 0;
  return buf->buf[buf->ptr++];
}

niExportFunc(SQRESULT) sq_compilebuffer(HSQUIRRELVM v,const SQChar *s,int size,const SQChar *sourcename,int raiseerror) {
  BufState buf;
  buf.buf = s;
  buf.size = size;
  buf.ptr = 0;
  return sq_compile(v, buf_lexfeed, (tPtr)&buf, sourcename, raiseerror);
}

niExportFunc(void) sq_move(HSQUIRRELVM dest,HSQUIRRELVM src,int idx)
{
  dest->Push(stack_get(src,idx));
}
