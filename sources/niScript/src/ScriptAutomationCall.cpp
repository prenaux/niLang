// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(15)

#include <niLang/Utils/ObjectInterfaceCast.h>
#include <niLang_ModuleDef.h>
#include "ScriptVM.h"
#include "ScriptObject.h"
#include "ScriptDispatch.h"
#include "ScriptAutomation.h"
#include "ScriptTypes.h"
#include "sqvm.h"
#include "sqtable.h"
#include "sqvm.h"
#include "sq_hstring.h"

#define niSQThrowError(ERR)                                           \
  return sq_throwerror(v, niFmt(_A("%s: %s"), __FUNCTION__, (ERR)));

// #define USE_XCALL_SET_PUSH_RET

//======================================================================
//=
//= Push/Pop Variants
//=
//======================================================================

///////////////////////////////////////////////
niExportFunc(int) sqa_pushvar(HSQUIRRELVM v, const Var& aVar)
{
  switch (niType(aVar.GetType())) {
    case eType_Null: {  v->Push(_null_); return SQ_OK; }
    case eType_I8:  { v->Push((SQInt)aVar.GetI8()); return SQ_OK; }
    case eType_U8:  { v->Push((SQInt)aVar.GetU8()); return SQ_OK; }
    case eType_I16: { v->Push((SQInt)aVar.GetI16());  return SQ_OK; }
    case eType_U16: { v->Push((SQInt)aVar.GetU16());  return SQ_OK; }
    case eType_I32: { v->Push((SQInt)aVar.GetI32());  return SQ_OK; }
    case eType_U32: { v->Push((SQInt)aVar.GetU32());  return SQ_OK; }
    case eType_I64: { v->Push((SQInt)(aVar.GetI64()&0xFFFFFFFF)); return SQ_OK; }
    case eType_U64: { v->Push((SQInt)(aVar.GetU64()&0xFFFFFFFF)); return SQ_OK; }
    case eType_F32: { v->Push((SQFloat)aVar.GetF32());  return SQ_OK; }
    case eType_F64: { v->Push((SQFloat)aVar.GetF64()); return SQ_OK;  }
    case eType_UUID: {
      sqa_pushUUID(v, aVar.GetUUID());
      return SQ_OK;
    }
    case eType_Vec2f:  {
      return sqa_pushvec2f(v,aVar.GetVec2f());
    }
    case eType_Vec2i:  {
      return sqa_pushvec2i(v,aVar.GetVec2i());
    }
    case eType_Vec3f:  {
      return sqa_pushvec3f(v,aVar.GetVec3f());
    }
    case eType_Vec3i:  {
      return sqa_pushvec3i(v,aVar.GetVec3i());
    }
    case eType_Vec4f:  {
      return sqa_pushvec4f(v,aVar.GetVec4f());
    }
    case eType_Vec4i:  {
      return sqa_pushvec4i(v,aVar.GetVec4i());
    }
    case eType_Matrixf:   {
      return sqa_pushmatrixf(v,aVar.GetMatrixf());
    }
    case eType_IUnknown:  {
      if (!aVar.IsIUnknownPointer()) {
        niSQThrowError(_A("IUnknown variant not a pointer."));
      }
      sqa_pushIUnknown(v,aVar.GetIUnknownPointer());
      return SQ_OK;
    }
    case eType_AChar:   {
      if (!aVar.IsACharConstPointer()) {
        niSQThrowError(_A("AChar not a constant pointer."));
      }
      sq_pushstring(v,_H(aVar.GetACharConstPointer()?aVar.GetACharConstPointer():cString::ConstSharpNull()));
      return SQ_OK;
    }
    case eType_String:  {
      sq_pushstring(v,_H(aVar.GetString()));
      return SQ_OK;
    }
    default: {
      niSQThrowError(niFmt(_A("Variant type '%d' is not handled."),niType(aVar.GetType())));
    }
  }
}

#define GET_UNSAFEUD(VM,IDX,TYPE,V)                 \
  SQObjectPtr& o##V = stack_get(VM,IDX);            \
  TYPE* p##V = static_cast<TYPE*>(_userdata(o##V));

///////////////////////////////////////////////
niExportFunc(int) sqa_getvar_astype(HSQUIRRELVM v, int idx, Var* apVar, const tType type)
{
  switch (niType(type)) {
    case eType_Null: {
      if (sqa_getscripttype(v,idx) != eScriptType_Null)
        niSQThrowError(niFmt(_A("Stack position '%d' isn't a null object."),idx));
      apVar->SetNull();
      return SQ_OK;
    }
    case eType_I8: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get I8 at stack position '%d'."),idx));
      *apVar = (tI8)val;
      return SQ_OK;
    }
    case eType_U8: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get U8 at stack position '%d'."),idx));
      *apVar = (tU8)val;
      return SQ_OK;
    }
    case eType_I16: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        return sq_throwerror(v,niFmt(_A("Can't get I16 at stack position '%d'."),idx));
      *apVar = (tI16)val;
      return SQ_OK;
    }
    case eType_U16: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get U16 at stack position '%d'."),idx));
      *apVar = (tU16)val;
      return SQ_OK;
    }
    case eType_I32: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get I32 at stack position '%d'."),idx));
      *apVar = (tI32)val;
      return SQ_OK;
    }
    case eType_U32: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get U32 at stack position '%d'."),idx));
      *apVar = (tU32)val;
      return SQ_OK;
    }
    case eType_I64: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get I64 at stack position '%d'."),idx));
      *apVar = (tI64)val;
      return SQ_OK;
    }
    case eType_U64: {
      int val = 0;
      if (!SQ_SUCCEEDED(sq_getint(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get U64 at stack position '%d'."),idx));
      *apVar = (tU64)val;
      return SQ_OK;
    }
    case eType_F32: {
      tF32 val = 0;
      if (!SQ_SUCCEEDED(sq_getf32(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get F32 at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_F64: {
      tF64 val = 0;
      if (!SQ_SUCCEEDED(sq_getf64(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get F64 at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_Vec2i: {
      sVec2i val;
      if (!SQ_SUCCEEDED(sqa_getvec2i(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get vec2i at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_Vec2f: {
      sVec2f val;
      if (!SQ_SUCCEEDED(sqa_getvec2f(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get vec2f at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_Vec3i: {
      sVec3i val;
      if (!SQ_SUCCEEDED(sqa_getvec3i(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get vec3i at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_Vec3f: {
      sVec3f val;
      if (!SQ_SUCCEEDED(sqa_getvec3f(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get vec3f at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_Vec4i: {
      sVec4i val;
      if (!SQ_SUCCEEDED(sqa_getvec4i(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get vec4i at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_Vec4f: {
      sVec4f val;
      if (!SQ_SUCCEEDED(sqa_getvec4f(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get vec4f at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_Matrixf: {
      sMatrixf val;
      if (!SQ_SUCCEEDED(sqa_getmatrixf(v, idx, &val)))
        niSQThrowError(niFmt(_A("Can't get matrixf at stack position '%d'."),idx));
      *apVar = val;
      return SQ_OK;
    }
    case eType_String:  {
      const achar* aszStr;
      if (!SQ_SUCCEEDED(sq_getstring(v,idx,&aszStr)))
        niSQThrowError(niFmt(_A("Can't get CString at stack position '%d'."),idx));
      *apVar = cString(aszStr);
      return SQ_OK;
    }
    case eType_AChar: {
      if (!niFlagIs(type,((tU32)(eTypeFlags_Constant|eTypeFlags_Pointer)))) {
        if (niFlagIs(type,eTypeFlags_Pointer))
          niSQThrowError(_A("Only constant pointer cchar accepted."));
        SQInt n;
        sq_getint(v,idx,&n);
        *apVar = (tI32)n;
        return SQ_OK;
      }
      else {
        const achar* aszStr;
        if (!SQ_SUCCEEDED(sq_getstring(v,idx,&aszStr)))
          niSQThrowError(niFmt(_A("Can't get const cchar* at stack position '%d'."),idx));
        *apVar = aszStr;
        return SQ_OK;
      }
    }
    case eType_IUnknown: {
      if (!niFlagIs(type,eTypeFlags_Pointer))
        niSQThrowError(_A("Only iUnknown pointer accepted."));
      iUnknown* pIUnk;
      if (!SQ_SUCCEEDED(sqa_getIUnknown(v,idx,&pIUnk,kuuidZero)))
        niSQThrowError(niFmt(_A("Can't get IUnknown at stack position '%d'."),idx));
      //apVar->SetIUnknownPointer(pIUnk);
      *apVar = pIUnk;
      return SQ_OK;
    }
    case eType_Variant: {
      if (!SQ_SUCCEEDED(sqa_getvar(v,idx,apVar)))
        niSQThrowError(niFmt(_A("Can't get variant at stack position '%d'."),idx));
      return SQ_OK;
    }
    case eType_UUID: {
      tUUID uuid;
      if (!SQ_SUCCEEDED(sqa_getUUID(v,idx,&uuid)))
        niSQThrowError(niFmt(_A("Can't get UUID at stack position '%d'."),idx));
      *apVar = uuid;
      return SQ_OK;
    }
  }

  niSQThrowError(niFmt(_A("Unhandled type %s."),sqa_gettypestring(type)));
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getvar(HSQUIRRELVM v, int idx, Var* apVar)
{
  eScriptType type = sqa_getscripttype(v,idx);
  switch (type) {
    default:
    case eScriptType_Invalid:
    case eScriptType_Table:
    case eScriptType_Array:
    case eScriptType_Closure:
    case eScriptType_NativeClosure:
    case eScriptType_UserData:
      niSQThrowError(_A("Invalid script type."));
    case eScriptType_Null: {
      apVar->SetNull();
      return SQ_OK;
    }
    case eScriptType_Int: {
      int val;
      sq_getint(v,idx,&val);
      *apVar = (tI32)val;
      return SQ_OK;
    }
    case eScriptType_Float: {
      SQFloat val;
      sq_getf64(v,idx,&val);
      *apVar = val;
      return SQ_OK;
    }
    case eScriptType_String: {
      const achar* aszVal;
      sq_getstring(v,idx,&aszVal);
      *apVar = cString(aszVal);
      return SQ_OK;
    }
    case eScriptType_Vec2: {
      GET_UNSAFEUD(v,idx,sScriptTypeVec2f,V);
      *apVar = pV->_val;
      return SQ_OK;
    }
    case eScriptType_Vec3: {
      GET_UNSAFEUD(v,idx,sScriptTypeVec3f,V);
      *apVar = pV->_val;
      return SQ_OK;
    }
    case eScriptType_Vec4: {
      GET_UNSAFEUD(v,idx,sScriptTypeVec4f,V);
      *apVar = pV->_val;
      return SQ_OK;
    }
    case eScriptType_Matrix: {
      GET_UNSAFEUD(v,idx,sScriptTypeMatrixf,V);
      *apVar = pV->_val;
      return SQ_OK;
    }
    case eScriptType_IUnknown: {
      iUnknown* p = NULL;
      sq_getiunknown(v,idx,&p);
      *apVar = p;
      return SQ_OK;
    }
    case eScriptType_UUID: {
      GET_UNSAFEUD(v,idx,sScriptTypeUUID,V);
      *apVar = pV->mUUID;
      return SQ_OK;
    }
  }
}

//======================================================================
//=
//= VMCall Utils
//=
//======================================================================
struct sVMCallWrapper
{
  const ni::achar*    _caller;
  const sMethodDef*    _meth;
  const sInterfaceDef* _intf;
  HSQUIRRELVM          _vm;
  tpfnVMCall           _vmcall;
  cString              _err;

  inline tBool Init(HSQUIRRELVM vm,
                    const ni::achar* aaszCallerName,
                    const sMethodDef* apMethodDef,
                    const sInterfaceDef* apInterfaceDef)
  {
    _vm = vm;
    _caller = aaszCallerName;
    _meth = apMethodDef;
    _intf = apInterfaceDef;
    _vmcall = (tpfnVMCall)_meth->mpVMCall;
    if (_vmcall == NULL) return WriteError(_A("method has no xcall wrapper."));
    return eTrue;
  }

  inline tU32 GetNumExpectedParams() const {
    return _meth->mnNumParameters;
  }

  inline tBool DoCall(iUnknown* apInst, tInt aStackBase, tU32 numPassed) {
    ni::Var params[16];
    ni::Var ret;
    if (_meth->mnNumParameters >= 16) {
      return eVMRet_OutOfMemory;
    }
    if (numPassed != _meth->mnNumParameters) {
      return WriteError(niFmt("Invalid number of parameters, expected '%s' but got '%s'.",
                              _meth->mnNumParameters,
                              numPassed));
    }
    const ni::tU32 numParams = ni::Min(numPassed,_meth->mnNumParameters);
    if (_meth->mpParameters) {
      niLoop(i,numParams) {
        if (!SQ_SUCCEEDED(sqa_getvar_astype(_vm,i+aStackBase,&params[i],_meth->mpParameters[i].mType))) {
          return WriteError(niFmt("Can't get param '%d', expected '%s' but got '%s'.", i,
                                  ni::GetTypeString(_meth->mpParameters[i].mType),
                                  ni::GetTypeString(sq_gettype(_vm,i+aStackBase))));
        }
      }
    }
    else {
      niLoop(i,numParams) {
        if (!SQ_SUCCEEDED(sqa_getvar(_vm,i+aStackBase,&params[i]))) {
          return WriteError(niFmt("Can't get param '%d'.", i));
        }
      }
    }
    const tInt callRet = _vmcall(apInst,params,_meth->mnNumParameters,&ret);
    if (callRet != eVMRet_OK) {
      const achar* aszReason = _A("Unknown");
      switch (callRet) {
        case eVMRet_InvalidArgCount: aszReason = _A("InvalidArgCount"); break;
        case eVMRet_InvalidMethod: aszReason = _A("InvalidMethod"); break;
        case eVMRet_InvalidArg: aszReason = _A("InvalidArg"); break;
        case eVMRet_InvalidRet: aszReason = _A("InvalidRet"); break;
        case eVMRet_OutOfMemory: aszReason = _A("OutOfMemory"); break;
      }
      return WriteError(aszReason);
    }
    sqa_pushvar(_vm,ret);
    return eTrue;
  }

  inline tBool WriteError(const achar* aaszErr) {
    _err = niFmt(_A("%s: %s::%s/%d: "),
                 _caller,
                 _intf?_intf->maszName:_A(""),
                 _meth->maszName,
                 _meth->mnNumParameters);
    if (aaszErr)
      _err << aaszErr;
    return eFalse;
  }

  inline int ThrowError(const achar* aaszErr) {
    WriteError(aaszErr);
    return ThrowError();
  }

  inline int ThrowError() {
    int r = sq_throwerror(_vm,_err.Chars());
    _err = AZEROSTR;
    return r;
  }
};

#define GET_XCALL(XCALL) sVMCallWrapper XCALL;

//======================================================================
//=
//= Indexed Property Set/Get
//=
//======================================================================

#define GET_IPUD(NAME,INDEX)                                            \
  Ptr<sScriptTypeIndexedProperty> ptr##NAME = sqa_getud<sScriptTypeIndexedProperty>(v,INDEX); \
  if (!ptr##NAME.IsOK()) return SQ_ERROR;

///////////////////////////////////////////////
int indexedproperty_set(HSQUIRRELVM v)
{
  GET_IPUD(V,1);
  const sMethodDef* pMethodDef = ptrV->pProp->pSetMethodDef;
  const sInterfaceDef* pInterfaceDef = ptrV->pProp->pInterfaceDef;
  if (pMethodDef == NULL)
    return sq_throwerror(v,niFmt(_A("indexedproperty_set, %s::%s, no setter."),
                                 pInterfaceDef?pInterfaceDef->maszName:_A(""),
                                 ptrV->pProp->GetName().Chars()));

  GET_XCALL(xcall);
  if (!xcall.Init(v,__FUNCTION__,pMethodDef,pInterfaceDef))
    return xcall.ThrowError();

  if (!xcall.DoCall(ptrV->pObject,2,xcall.GetNumExpectedParams()))
    return xcall.ThrowError();

#ifdef USE_XCALL_SET_PUSH_RET
  if (!xcall->PushRetVal(eFalse))
    return xcall->ThrowError();
  return 1;
#endif

  return 0;
}

///////////////////////////////////////////////
int indexedproperty_get(HSQUIRRELVM v)
{
  GET_IPUD(V,1);
  const sMethodDef* pMethodDef = ptrV->pProp->pGetMethodDef;
  const sInterfaceDef* pInterfaceDef = ptrV->pProp->pInterfaceDef;
  if (pMethodDef == NULL)
    return sq_throwerror(v,niFmt(_A("indexedproperty_get, %s::%s, no getter."),
                                 pInterfaceDef?pInterfaceDef->maszName:_A(""),
                                 ptrV->pProp->GetName().Chars()));

  GET_XCALL(xcall);
  if (!xcall.Init(v,__FUNCTION__,pMethodDef,pInterfaceDef))
    return xcall.ThrowError();

  if (!xcall.DoCall(ptrV->pObject,2,xcall.GetNumExpectedParams()))
    return xcall.ThrowError();

  return 1;
}

///////////////////////////////////////////////
// #define UNSAFE_DIRECT_THIS // Used for benchmarking only

bool SQVM::CallMethodDef(sScriptTypeMethodDef* meth, int nargs, int stackbase, SQObjectPtr &retval)
{
  niAssert(nargs > 0);
  if (_nnativecalls + 1 > MAX_NATIVE_CALLS) {
    VM_ERRORB(_A("Native stack overflow"));
  }

  const sMethodDef* pMethodDef = meth->pMethodDef;
  const sInterfaceDef* pInterfaceDef = meth->pInterfaceDef;

  SQVM* v = this;
  GET_XCALL(xcall);
  if (!xcall.Init(v,__FUNCTION__,pMethodDef,pInterfaceDef)) {
    VM_ERRORB(niFmt(_A("Call to %s::%s/%d failed. No VMCall Wrapper."),
                    pInterfaceDef?pInterfaceDef->maszName:_A(""),
                    pMethodDef->maszName,
                    pMethodDef->mnNumParameters));
  }

  const tU32 numPassed = (tU32)(nargs-1);
  {
    const tU32 needed = pMethodDef->mnNumParameters;
    if (numPassed > needed) {
      VM_ERRORB(niFmt(_A("Call to %s::%s/%d failed. Too many parameters %d."),
                      pInterfaceDef?pInterfaceDef->maszName:_A(""),
                      pMethodDef->maszName,
                      pMethodDef->mnNumParameters,
                      (nargs-1)));
    }
  }

  _nnativecalls++;
  if ((_top + MIN_STACK_OVERHEAD) > (int)_stack.size()) {
    _stack.resize(_stack.size() + (MIN_STACK_OVERHEAD<<1));
  }
  int oldtop = _top;
  int oldstackbase = _stackbase;
  _top = stackbase + nargs;
  push_callinfo(this, SQCallInfo());
  _ci->_etraps = 0;
  _ci->_closurePtr = meth;
  _ci->_prevstkbase = stackbase - _stackbase;
  _ci->_ncalls = 1;
  _stackbase = stackbase;
  _ci->_prevtop = (oldtop - oldstackbase);

  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  cScriptAutomation* pAutomation = pVM->GetAutomation();
#ifdef UNSAFE_DIRECT_THIS
  SQObjectPtr objThis;
#else
  iUnknown* pStackInst = NULL;
  Ptr<iUnknown> ptrInst;
#endif
  if (pInterfaceDef) {
#ifdef UNSAFE_DIRECT_THIS
    objThis = stack_get(v,1);
#else
    if (!SQ_SUCCEEDED(pAutomation->GetIUnknown(this,1,&pStackInst,*pInterfaceDef->mUUID))
        || !pStackInst)
    {
      VM_ERRORB(niFmt(_A("Call to %s::%s/%d failed. Can't get this instance."),
                      pInterfaceDef?pInterfaceDef->maszName:_A(""),
                      pMethodDef->maszName,
                      pMethodDef->mnNumParameters));
    }
    ptrInst = pStackInst;
#endif
  }
  else {
    if (!(pMethodDef->mReturnType&eTypeFlags_MethodStatic)) {
#ifdef UNSAFE_DIRECT_THIS
      objThis = stack_get(v,1);
#else
      if (!SQ_SUCCEEDED(pAutomation->GetIUnknown(this,1,&pStackInst,kuuidZero))
          || !pStackInst)
      {
        VM_ERRORB(niFmt(_A("Call to ?::%s/%d failed. Can't get this instance."),
                        pMethodDef->maszName,
                        pMethodDef->mnNumParameters));
      }
      ptrInst = pStackInst;
#endif
    }
  }

  tBool ret =
#ifdef UNSAFE_DIRECT_THIS
      xcall.DoCall(_sqtype(objThis)==OT_IUNKNOWN?_iunknown(objThis):NULL,2,numPassed)
#else
      xcall.DoCall(ptrInst,2,numPassed)
#endif
      ;
  _nnativecalls--;

  // call succeeded, copy the return value
  if (ret) {
    retval = VM_STACK_TOP();
  }

  // restore the stack & call info
  _stackbase = oldstackbase;
  _top = oldtop;
  pop_callinfo(this);

  // call failed, raise an error.
  if (!ret) {
    Raise_MsgError(xcall._err.Chars());
    return false;
  }

  return true;
}

///////////////////////////////////////////////
bool cScriptAutomation::Get(HSQUIRRELVM v, iUnknown* apObj, const SQObjectPtr &key, SQObjectPtr &dest, int opExt) {
  niAssert(v != NULL);
  niSqGuard(v);

  // If integer key we try to index a vector
  if (sq_type(key) == OT_INTEGER) {
    Ptr<iCollection> pCollection = ni::QueryInterface<iCollection>(apObj);
    if (!niIsOK(pCollection)) {
      if (!(opExt & _OPEXT_GET_SAFE)) {
        niWarning("Get[int], object is not a iCollection.");
      }
      niSqUnGuard(v);
      return false;
    }

    SQInt index = _int(key);
    if (index < 0 || index >= (SQInt)pCollection->GetSize()) {
      if (!(opExt & _OPEXT_GET_SAFE)) {
        niWarning(niFmt("Get[int], index %d out of range [0-%d].",index,pCollection->GetSize()-1));
      }
      niSqUnGuard(v);
      return false;
    }
    if (SQ_FAILED(sqa_pushvar(v,pCollection->Get(index)))) {
      if (!(opExt & _OPEXT_GET_SAFE)) {
        niWarning(niFmt("Get[int], can't push the item %d.",index));
      }
      niSqUnGuard(v);
      return false;
    }

    dest = stack_get(v,-1);
    v->Pop(1);

    niSqUnGuard(v);
    return true;
  }

  // If not an integer it should be a string
  if (sq_type(key) != OT_STRING)  {
    niSqUnGuard(v);
    if (!(opExt & _OPEXT_GET_SAFE)) {
      niWarning("Key must be a string or an integer.");
    }
    return false;
  }

  SQObjectPtr objFound;
  if (!GetIUnknownMethod(v,apObj,key,objFound)) {
    if (!(opExt & _OPEXT_GET_SAFE)) {
      niWarning(niFmt("Can't get method/property %s.",_stringhval(key)));
    }
    niSqUnGuard(v);
    return false;
  }

  switch (sq_type(objFound)) {
    case OT_USERDATA: {
      // Found userdata, property or method
      SQUserData* ud = _userdata(objFound);
      switch (ud->GetType()) {
        case eScriptType_PropertyDef: {
          sScriptTypePropertyDef* propDef = (sScriptTypePropertyDef*)ud;
          const sInterfaceDef* pInterfaceDef = propDef->pInterfaceDef;
          const sMethodDef* pMethodDef = propDef->pGetMethodDef;
          if (pMethodDef == NULL || pMethodDef->mnNumParameters > 0) {
            /// Indexed property
            iUnknown* pObject = apObj->QueryInterface(*pInterfaceDef->mUUID);
            if (!pObject) {
              if (!(opExt & _OPEXT_GET_SAFE)) {
                niWarning(niFmt("Can't query interface '%s' of '%p' when getting property '%s'.",
                                pInterfaceDef->maszName,(tIntPtr)apObj,_stringhval(key)));
              }
              niSqUnGuard(v);
              return false;
            }
            dest = niNew sScriptTypeIndexedProperty(*v->_ss,pObject,propDef);
            niSqUnGuard(v);
            return true;
          }
          else {
            /// Property
            GET_XCALL(xcall);
            if (!xcall.Init(v,__FUNCTION__,pMethodDef,pInterfaceDef)) {
              if (!(opExt & _OPEXT_GET_SAFE)) {
                niWarning(niFmt("Can't init XCall for %s::%s: %s",
                                pInterfaceDef?pInterfaceDef->maszName:_A(""),
                                _stringhval(key),
                                xcall._err));
              }
              niSqUnGuard(v);
              return false;
            }

            Ptr<iUnknown> ptrInst = apObj->QueryInterface(*pInterfaceDef->mUUID);
            if (niIsNullPtr(ptrInst)) {
              if (!(opExt & _OPEXT_GET_SAFE)) {
                niWarning(niFmt("Can't query interface '%s' of '%p' when getting property '%s'.",
                                pInterfaceDef->maszName,(tIntPtr)apObj,_stringhval(key)));
              }
              niSqUnGuard(v);
              return false;
            }

            if (!xcall.DoCall(ptrInst,0,xcall.GetNumExpectedParams())) {
              if (!(opExt & _OPEXT_GET_SAFE)) {
                niWarning(niFmt("Call to %s::%s failed: %s",
                                pInterfaceDef?pInterfaceDef->maszName:_A(""),
                                _stringhval(key),
                                xcall._err));
              }
              niSqUnGuard(v);
              return false;
            }

            dest = stack_get(v,-1);
            v->Pop(1); // pop return value...
            niSqUnGuard(v);
            return true;
          }
          break;
        }
        case eScriptType_MethodDef: {
          // Method definition
          dest = objFound;
          niSqUnGuard(v);
          return true;
        }
      }
      if (!(opExt & _OPEXT_GET_SAFE)) {
        niWarning(niFmt("'%s': Not a method or property getter.",_stringhval(key)));
      }
      niSqUnGuard(v);
      return false;
    }
    case OT_NULL:
      // NULL, can't find property
      if (!(opExt & _OPEXT_GET_SAFE)) {
        niWarning(niFmt("Can't get method/property %s.",_stringhval(key)));
      }
      niSqUnGuard(v);
      return false;
    default:
      // Found a function or other native type
      dest = objFound;
      niSqUnGuard(v);
      return true;
  }
}

///////////////////////////////////////////////
bool cScriptAutomation::Set(HSQUIRRELVM v, iUnknown* apObj,const SQObjectPtr &key,const SQObjectPtr &val, int opExt) {
  niAssert(v != NULL);

  niSqGuard(v);
  if (sq_type(key) != OT_STRING)  {
    niSqUnGuard(v);
    niWarning("Get, key must be a string.");
    return false;
  }

  SQObjectPtr objFound;
  if (!GetIUnknownMethod(v,apObj,key,objFound)) {
    if (!(opExt & _OPEXT_GET_SAFE)) {
      niWarning(niFmt("Can't get method/property %s.",_stringhval(key)));
    }
    niSqUnGuard(v);
    return false;
  }

  SQUserData* ud = _userdata(objFound);
  if (ud->GetType() == eScriptType_PropertyDef) {
    sScriptTypePropertyDef* propDef = (sScriptTypePropertyDef*)ud;
    const sInterfaceDef* pInterfaceDef = propDef->pInterfaceDef;
    const sMethodDef* pMethodDef = propDef->pSetMethodDef;
    if (pMethodDef != NULL && pMethodDef->mnNumParameters == 1) {
      /// Property
      GET_XCALL(xcall);
      if (!xcall.Init(v,__FUNCTION__,pMethodDef,pInterfaceDef)) {
        if (!(opExt & _OPEXT_GET_SAFE)) {
          niWarning(niFmt("Can't init XCall for %s::%s: %s",
                          pInterfaceDef?pInterfaceDef->maszName:_A(""),
                          _stringhval(key),
                          xcall._err));
        }
        niSqUnGuard(v);
        return false;
      }

      Ptr<iUnknown> ptrInst = apObj->QueryInterface(*pInterfaceDef->mUUID);
      if (niIsNullPtr(ptrInst)) {
        if (!(opExt & _OPEXT_GET_SAFE)) {
          niWarning(niFmt("Can't query interface '%s' of '%p' when getting property '%s'.",
                          pInterfaceDef->maszName,(tIntPtr)apObj,_stringhval(key)));
        }
        niSqUnGuard(v);
        return false;
      }

      v->Push(val);
      if (!xcall.DoCall(ptrInst,v->GetCallStackBase(1),xcall.GetNumExpectedParams())) {
        if (!(opExt & _OPEXT_GET_SAFE)) {
          niWarning(niFmt("Call to %s::%s failed: %s",
                          pInterfaceDef?pInterfaceDef->maszName:_A(""),
                          _stringhval(key),
                          xcall._err));
        }
        v->Pop(1);
        niSqUnGuard(v);
        return false;
      }

      v->Pop(2);
      niSqUnGuard(v);
      return true;
    }
  }

  if (!(opExt & _OPEXT_GET_SAFE)) {
    niWarning(niFmt("'%s': Not a property setter.",_stringhval(key)));
  }
  niSqUnGuard(v);
  return false;
}
#endif
