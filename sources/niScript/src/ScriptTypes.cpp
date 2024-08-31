// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(15)

#include <niLang/Math/MathVec2.h>
#include <niLang/Math/MathVec3.h>
#include <niLang/Math/MathVec4.h>
#include <niLang/Math/MathMatrix.h>
#include "API/niScript/IScriptVM.h"
#include "API/niScript_ModuleDef.h"
#include "ScriptVM.h"
#include "ScriptTypes.h"
#include "ScriptAutomation.h"
#include "ScriptObject.h"
#include "ScriptDispatch.h"
#include "sqtable.h"
#include "sqvm.h"
#include "sqarray.h"
#include "sq_hstring.h"

//////////////////////////////////////////////////////////////////////////////////////////////
static int matrixf_constructor(HSQUIRRELVM v);
static int vec2f_constructor(HSQUIRRELVM v);
static int vec3f_constructor(HSQUIRRELVM v);
static int vec4f_constructor(HSQUIRRELVM v);

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
static inline tBool sqa_getVecElementFromChar(HSQUIRRELVM v, achar c, const tF32* apV, tU32 anSize, tF32& afRet)
{
  niAssert(anSize >= 2);
  switch (c) {
    case 'r': case 'x': afRet = apV[0]; return eTrue;
    case 'g': case 'y': afRet = apV[1]; return eTrue;
    case 'b': case 'z': if (anSize >= 3) { afRet = apV[2];  return eTrue; } break;
    case 'a': case 'w': if (anSize >= 4) { afRet = apV[3];  return eTrue; } break;
  }
  return eFalse;
}

///////////////////////////////////////////////
static inline tBool sqa_setVecElementFromChar(HSQUIRRELVM v, achar c, tF32* apV, tU32 anSize, tF32 afVal)
{
  niAssert(anSize >= 2);
  switch (c) {
    case 'r': case 'x': apV[0] = afVal; return eTrue;
    case 'g': case 'y': apV[1] = afVal; return eTrue;
    case 'b': case 'z': if (anSize >= 3) { apV[2] = afVal;  return eTrue; } break;
    case 'a': case 'w': if (anSize >= 4) { apV[3] = afVal;  return eTrue; } break;
  }
  return eFalse;
}

///////////////////////////////////////////////
eScriptType sqa_getscripttype_(SQObjectType anType)
{
  switch (anType) {
    case OT_NULL:     return eScriptType_Null;
    case OT_INTEGER:    return eScriptType_Int;
    case OT_FLOAT:      return eScriptType_Float;
    case OT_STRING:     return eScriptType_String;
    case OT_TABLE:      return eScriptType_Table;
    case OT_ARRAY:      return eScriptType_Array;
    case OT_CLOSURE:    return eScriptType_Closure;
    case OT_NATIVECLOSURE:  return eScriptType_NativeClosure;
    case OT_USERDATA:   return eScriptType_UserData;
    case OT_IUNKNOWN:       return (eScriptType)OT_IUNKNOWN;
  }
  return eScriptType_Invalid;
}

///////////////////////////////////////////////
niExportFunc(eScriptType) sqa_getscriptobjtype(const SQObjectPtr& obj)
{
  switch (_sqtype(obj)) {
    case OT_NULL:     return (eScriptType)OT_NULL;
    case OT_INTEGER:    return (eScriptType)OT_INTEGER;
    case OT_FLOAT:      return (eScriptType)OT_FLOAT;
    case OT_STRING:     return (eScriptType)OT_STRING;
    case OT_TABLE:      return (eScriptType)OT_TABLE;
    case OT_ARRAY:      return (eScriptType)OT_ARRAY;
    case OT_CLOSURE:    return (eScriptType)OT_CLOSURE;
    case OT_NATIVECLOSURE:  return (eScriptType)OT_NATIVECLOSURE;
    case OT_FUNCPROTO:  return (eScriptType)OT_FUNCPROTO;
    case OT_IUNKNOWN:       return (eScriptType)OT_IUNKNOWN;
    case OT_USERDATA: {
      SQUserData* ud = _userdata(obj);
      return (eScriptType)ud->GetType();
    }
  }
  return eScriptType_Invalid;
}
niExportFunc(eScriptType) sqa_getscripttype(HSQUIRRELVM v, int idx) {
  return sqa_getscriptobjtype(stack_get(v,idx));
}

///////////////////////////////////////////////
niExportFunc(eScriptType) sqa_type2scripttype(const tType aType)
{
  switch (niType(aType)) {
    case eType_Null: {
      return eScriptType_Null;
    }
    case eType_I8:
    case eType_U8:
    case eType_I16:
    case eType_U16:
    case eType_I32:
    case eType_U32:
    case eType_I64:
    case eType_U64: {
      return eScriptType_Int;
    }
    case eType_F32:
    case eType_F64: {
      return eScriptType_Float;
    }
    case eType_UUID: {
      return eScriptType_UUID;
    }
    case eType_Vec2f:
    case eType_Vec2i: {
      return eScriptType_Vec2;
    }
    case eType_Vec3f:
    case eType_Vec3i: {
      return eScriptType_Vec3;
    }
    case eType_Vec4f:
    case eType_Vec4i: {
      return eScriptType_Vec4;
    }
    case eType_Matrixf: {
      return eScriptType_Matrix;
    }
    case eType_IUnknown:  {
      return eScriptType_IUnknown;
    }
    case eType_AChar: {
      if (aType == eType_ASZ)
        return eScriptType_String;
      else
        return eScriptType_Invalid;
    }
    case eType_String: {
      return eScriptType_String;
    }
    default: {
      return eScriptType_Invalid;
    }
  }
}

///////////////////////////////////////////////
niExportFunc(iHString*) sqa_getscripttypename(eScriptType aType) {
  switch (aType) {
    case eScriptType_Null: return _HC(typestr_null);
    case eScriptType_Int: return _HC(typestr_int);
    case eScriptType_Float: return _HC(typestr_float);
    case eScriptType_String: return _HC(typestr_string);
    case eScriptType_Table: return _HC(typestr_table);
    case eScriptType_Array: return _HC(typestr_array);
    case eScriptType_UserData: return _HC(typestr_userdata);
    case eScriptType_Closure: return _HC(typestr_closure);
    case eScriptType_NativeClosure: return _HC(typestr_nativeclosure);
    case eScriptType_FunctionProto: return _HC(typestr_functionproto);
    case eScriptType_IUnknown: return _HC(typestr_iunknown);
    case eScriptType_Vec2: return _HC(Vec2);
    case eScriptType_Vec3: return _HC(Vec3);
    case eScriptType_Vec4: return _HC(Vec4);
    case eScriptType_Matrix: return _HC(Matrix);
    case eScriptType_MethodDef: return _HC(method);
    case eScriptType_PropertyDef: return _HC(typestr_property);
    case eScriptType_EnumDef: return _HC(typestr_enum);
    case eScriptType_Iterator: return _HC(typestr_iterator);
    case eScriptType_IndexedProperty: return _HC(typestr_indexed_property);
    case eScriptType_UUID: return _HC(UUID);
    case eScriptType_InterfaceDef: return _HC(interface);
    case eScriptType_ErrorCode: return _HC(error_code);
    case eScriptType_ResolvedType: return _HC(resolved_type);
    case eScriptType_Invalid: return _HC(typestr_invalid);
  }
  return nullptr;
}

///////////////////////////////////////////////
niExportFunc(iHString*) sqa_getscriptobjtypename(const SQObjectPtr& obj) {
  return sqa_getscripttypename(sqa_getscriptobjtype(obj));
}

///////////////////////////////////////////////
niExportFuncCPP(cString) sqa_gettypestring(int type) {
  switch (_RAW_TYPE(type)) {
    case _RT_NULL: return "Null";
    case _RT_INTEGER: return "I32";
    case _RT_FLOAT: return "F64";
    case _RT_STRING: return "STRING";
    case _RT_TABLE: return "TABLE";
    case _RT_ARRAY: return "ARRAY";
    case _RT_USERDATA: return "USERDATA";
    case _RT_CLOSURE: return "CLOSURE";
    case _RT_NATIVECLOSURE: return "NATIVECLOSURE";
    case _RT_FUNCPROTO: return "FUNCPROTO";
    case _RT_IUNKNOWN: return "IUnknownPtr";
    default: return ni::GetTypeString(type);
  }
}

///////////////////////////////////////////////
niExportFunc(void) sqa_registerglobalfunction(HSQUIRRELVM v, const achar* aaszName, SQFUNCTION func)
{
  sq_pushroottable(v);
  sq_pushstring(v,_H(aaszName));
  sq_newclosure(v,func);
  sq_setnativeclosurename(v,-1,aaszName);
  sq_createslot(v,-3);
  sq_pop(v,1);
}

///////////////////////////////////////////////
niExportFunc(void) sqa_setdebugname(HSQUIRRELVM v, int idx, const achar* aaszName)
{
  niAssert(idx<0);
  sq_pushstring(v, _HC(__debug_name));
  sq_pushstring(v, _H(aaszName));
  sq_createslot(v,idx-2);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sInterfaceDef

static int interface_typeof(HSQUIRRELVM v) {
  sq_pushstring(v, _HC(interface));
  return 1;
}

static int interface_Name(HSQUIRRELVM v) {
  sScriptTypeInterfaceDef* pV = sqa_getud<sScriptTypeInterfaceDef>(v,1);
  if (!pV) return SQ_ERROR;
  sq_pushstring(v,_H(pV->pInterfaceDef->maszName));
  return 1;
}

static int interface_interfaceName(HSQUIRRELVM v) {
  sScriptTypeInterfaceDef* pV = sqa_getud<sScriptTypeInterfaceDef>(v,1);
  if (!pV) return SQ_ERROR;
  if (pV->pInterfaceDef) {
    sq_pushstring(v,_H(pV->pInterfaceDef->maszName));
  }
  else {
    sq_pushnull(v);
  }
  return 1;
}

SQRegFunction SQSharedState::_interface_default_delegate_funcz[]={
  {"_typeof", interface_typeof, 0, NULL},
  {"GetName", interface_Name, 0, NULL},
  {"GetInterfaceName", interface_interfaceName, 0, NULL},
  {0,0}
};

niExportFunc(int) sqa_pushInterfaceDef(HSQUIRRELVM v, const sInterfaceDef* apInterfaceDef)
{
  niAssert(apInterfaceDef != NULL);
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  //  niAssert(niIsOK(pVM));
  sq_pushud(*pVM,niNew sScriptTypeInterfaceDef(
    *v->_ss,apInterfaceDef));
  return SQ_OK;
}

niExportFunc(int) sqa_getInterfaceDef(HSQUIRRELVM v, int idx, const sInterfaceDef** appInterfaceDef)
{
  sScriptTypeInterfaceDef* pV = sqa_getud<sScriptTypeInterfaceDef>(v,idx);
  if (!pV)  return SQ_ERROR;
  if (appInterfaceDef) *appInterfaceDef = pV->pInterfaceDef;
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sMethodDef

static int method_typeof(HSQUIRRELVM v) {
  sq_pushstring(v, _HC(method));
  return 1;
}

static int method_Name(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV) return SQ_ERROR;
  sq_pushstring(v,_H(pV->pMethodDef->maszName));
  return 1;
}

static int method_interfaceName(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV) return SQ_ERROR;
  if (pV->pInterfaceDef) {
    sq_pushstring(v,_H(pV->pInterfaceDef->maszName));
  }
  else {
    sq_pushnull(v);
  }
  return 1;
}

static int method_returnType(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV)  return SQ_ERROR;
  sq_pushstring(v,_H(pV->pMethodDef->mReturnTypeName));
  return 1;
}

static int method_returnTypeFlags(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV)  return SQ_ERROR;
  sq_pushint(v,pV->pMethodDef->mReturnType);
  return 1;
}

static int method_numParameters(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV)  return SQ_ERROR;
  sq_pushint(v,pV->pMethodDef->mnNumParameters);
  return 1;
}

static int method_parameterName(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV)  return SQ_ERROR;
  const sMethodDef* methodDef = pV->pMethodDef;

  SQInt idx;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&idx)))
    return sq_throwerror(v, niFmt(_A("method_parameterName, index to lookup in the method parameters '%s' has to be an integer."),idx,methodDef->maszName,methodDef->mnNumParameters));

  if ((tU32)idx > methodDef->mnNumParameters)
    return sq_throwerror(v, niFmt(_A("method_parameterName, index '%d' to get in the method parameters '%s' is out of range (size:%d)."),idx,methodDef->maszName,methodDef->mnNumParameters));

  if (methodDef->mpParameters) {
    sq_pushstring(v,_H(methodDef->mpParameters[idx].maszName));
  }
  else {
    sq_pushnull(v);
  }
  return 1;
}

static int method_parameterTypeFlags(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV)  return SQ_ERROR;
  const sMethodDef* methodDef = pV->pMethodDef;

  SQInt idx;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&idx)))
    return sq_throwerror(v, niFmt(_A("method_parameterValue, index to lookup in the method parameters '%s' has to be an integer."),idx,methodDef->maszName,methodDef->mnNumParameters));

  if ((tU32)idx > methodDef->mnNumParameters)
    return sq_throwerror(v, niFmt(_A("method_parameterValue, index '%d' to get in the method parameters '%s' is out of range (size:%d)."),idx,methodDef->maszName,methodDef->mnNumParameters));

  if (methodDef->mpParameters) {
    sq_pushint(v,methodDef->mpParameters[idx].mType);
  }
  else {
    sq_pushnull(v);
  }
  return 1;
}

static int method_parameterType(HSQUIRRELVM v) {
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,1);
  if (!pV)  return SQ_ERROR;
  const sMethodDef* methodDef = pV->pMethodDef;

  SQInt idx;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&idx)))
    return sq_throwerror(v, niFmt(_A("method_parameterValue, index to lookup in the method parameters '%s' has to be an integer."),idx,methodDef->maszName,methodDef->mnNumParameters));

  if ((tU32)idx > methodDef->mnNumParameters)
    return sq_throwerror(v, niFmt(_A("method_parameterValue, index '%d' to get in the method parameters '%s' is out of range (size:%d)."),idx,methodDef->maszName,methodDef->mnNumParameters));

  if (methodDef->mpParameters) {
    sq_pushstring(v,_H(methodDef->mpParameters[idx].mTypeName));
  }
  else {
    sq_pushnull(v);
  }
  return 1;
}

SQRegFunction SQSharedState::_method_default_delegate_funcz[]={
  {"_typeof", method_typeof, 0, NULL},
  {"GetName", method_Name, 0, NULL},
  {"GetInterfaceName", method_interfaceName, 0, NULL},
  {"GetReturnTypeFlags", method_returnTypeFlags, 0, NULL},
  {"GetReturnType", method_returnType, 0, NULL},
  {"GetNumParams", method_numParameters, 0, NULL},
  {"GetParamName", method_parameterName, 0, NULL},
  {"GetParamType", method_parameterType, 0, NULL},
  {"GetParamTypeFlags", method_parameterTypeFlags, 0, NULL},
  {0,0}
};

niExportFunc(int) sqa_pushMethodDef(HSQUIRRELVM v, const sInterfaceDef* apInterfaceDef, const sMethodDef* apMethDef)
{
  niAssert(apMethDef != NULL);
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  //  niAssert(niIsOK(pVM));
  sq_pushud(*pVM,niNew sScriptTypeMethodDef(
    *v->_ss,apInterfaceDef,apMethDef));
  return SQ_OK;
}

niExportFunc(int) sqa_getMethodDef(HSQUIRRELVM v, int idx, const sInterfaceDef** appInterfaceDef, const sMethodDef** appMethDef)
{
  sScriptTypeMethodDef* pV = sqa_getud<sScriptTypeMethodDef>(v,idx);
  if (!pV)  return SQ_ERROR;
  if (appMethDef) *appMethDef = pV->pMethodDef;
  if (appInterfaceDef) *appInterfaceDef = pV->pInterfaceDef;
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

int sqa_pushPropertyDef(HSQUIRRELVM v, const sInterfaceDef* apInterfaceDef, const sMethodDef* apSetMethDef, const sMethodDef* apGetMethDef)
{
  niAssert((apSetMethDef != NULL) || (apGetMethDef != NULL));

  //  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  //  niAssert(niIsOK(pVM));

  sScriptTypePropertyDef* ud = niNew sScriptTypePropertyDef(
      apInterfaceDef,apSetMethDef,apGetMethDef);
  sq_pushud(v,ud);
  return SQ_OK;
}

int sqa_getPropertyDef(HSQUIRRELVM v, int idx, const sInterfaceDef** appInterfaceDef, const sMethodDef** appSetMethDef, const sMethodDef** appGetMethDef)
{
  sScriptTypePropertyDef* pV = sqa_getud<sScriptTypePropertyDef>(v,idx);
  if (!pV)  return SQ_ERROR;
  if (appSetMethDef) *appSetMethDef = pV->pSetMethodDef;
  if (appGetMethDef) *appGetMethDef = pV->pGetMethodDef;
  if (appInterfaceDef) *appInterfaceDef = pV->pInterfaceDef;
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Indexed property

///////////////////////////////////////////////
int sqa_pushIndexedProperty(HSQUIRRELVM v, iUnknown* apObject, const sScriptTypePropertyDef* apProp)
{
  niAssert(niIsOK(apObject));
  sScriptTypeIndexedProperty* ud = niNew sScriptTypeIndexedProperty(
    *v->_ss,apObject,apProp);
  sq_pushud(v,ud);
  return SQ_OK;
}

///////////////////////////////////////////////
int sqa_getIndexedProperty(HSQUIRRELVM v, int idx, iUnknown** appObject, const sScriptTypePropertyDef** appProp)
{
  sScriptTypeIndexedProperty* pV = sqa_getud<sScriptTypeIndexedProperty>(v,idx);
  if (!pV)  return SQ_ERROR;
  if (appObject) *appObject = pV->pObject;
  if (appProp) *appProp = pV->pProp;
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sErrorCode

static int error_code_typeof(HSQUIRRELVM v) {
  sq_pushstring(v, _HC(error_code));
  return 1;
}

static int error_code_GetKind(HSQUIRRELVM v) {
  sScriptTypeErrorCode* pV = sqa_getud<sScriptTypeErrorCode>(v,1);
  if (!pV) return SQ_ERROR;
  sq_pushstring(v,pV->_hspKind);
  return 1;
}

static int error_code_GetDesc(HSQUIRRELVM v) {
  sScriptTypeErrorCode* pV = sqa_getud<sScriptTypeErrorCode>(v,1);
  if (!pV) return SQ_ERROR;
  sq_pushstring(v,pV->_hspKind);
  return 1;
}

SQRegFunction SQSharedState::_error_code_default_delegate_funcz[]={
  {"_typeof", interface_typeof, 0, NULL},
  {"GetKind", error_code_GetKind, 0, NULL},
  {"GetDesc", error_code_GetDesc, 0, NULL},
  {0,0}
};

niExportFuncCPP(int) sqa_pushErrorCode(HSQUIRRELVM v, ain_nn<iHString> ahspKind, ain<tChars> aDesc)
{
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  //  niAssert(niIsOK(pVM));
  sq_pushud(*pVM,niNew sScriptTypeErrorCode(*v->_ss,ahspKind,aDesc));
  return SQ_OK;
}

niExportFuncCPP(int) sqa_getErrorCode(HSQUIRRELVM v, int idx, aout<NN<sScriptTypeErrorCode>> aOut)
{
  sScriptTypeErrorCode* pV = sqa_getud<sScriptTypeErrorCode>(v,idx);
  if (!pV)  return SQ_ERROR;
  aOut = AsNN(pV);
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// sResolvedType

cString __stdcall sScriptTypeResolvedType::GetTypeString() const {
  cString ret = "resolved_type<";
  ret << niHStr(sqa_getscripttypename(_scriptType));
  if (_opcode != __OP_LAST) {
    ret << ",";
    ret << _GetOpDesc(_opcode);
  }
  ret << ">";
  return ret;
}

static int resolved_type_typeof(HSQUIRRELVM v) {
  sq_pushstring(v, _HC(resolved_type));
  return 1;
}

static int resolved_type_GetResolvedTypeName(HSQUIRRELVM v) {
  sScriptTypeResolvedType* pV = sqa_getud<sScriptTypeResolvedType>(v,1);
  if (!pV) return SQ_ERROR;
  sq_pushstring(v,_H(pV->GetTypeString()));
  return 1;
}

SQRegFunction SQSharedState::_resolved_type_default_delegate_funcz[]={
  {"_typeof", interface_typeof, 0, NULL},
  {"GetResolvedTypeName", resolved_type_GetResolvedTypeName, 0, NULL},
  {0,0}
};

niExportFuncCPP(int) sqa_pushResolvedType(HSQUIRRELVM v, ain<eScriptType> aType)
{
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  //  niAssert(niIsOK(pVM));
  sq_pushud(*pVM,niNew sScriptTypeResolvedType(*v->_ss,aType));
  return SQ_OK;
}

niExportFuncCPP(int) sqa_getResolvedType(HSQUIRRELVM v, int idx, aout<eScriptType> aType)
{
  sScriptTypeResolvedType* pV = sqa_getud<sScriptTypeResolvedType>(v,idx);
  if (!pV)  return SQ_ERROR;
  aType = pV->_scriptType;
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
static int enum_typeof(HSQUIRRELVM v)
{
  const sEnumDef* pV;
  if (!SQ_SUCCEEDED(sqa_getEnumDef(v,1,&pV))) return sq_throwerror(v,_A("Can't get enum definition."));
  sq_pushstring(v, _H(niFmt(_A("enum[%s]"),pV->maszName)));
  return 1;
}

///////////////////////////////////////////////
static int enum_newslot(HSQUIRRELVM v)
{
  const sEnumDef* pV;
  if (!SQ_SUCCEEDED(sqa_getEnumDef(v,1,&pV))) return sq_throwerror(v,_A("Can't get enum definition."));
  return sq_throwerror(v,niFmt(_A("Can't call _newslot on enumeration table '%s'."),pV->maszName));
}

///////////////////////////////////////////////
static int enum_delslot(HSQUIRRELVM v)
{
  const sEnumDef* pV;
  if (!SQ_SUCCEEDED(sqa_getEnumDef(v,1,&pV))) return sq_throwerror(v,_A("Can't get enum definition."));
  return sq_throwerror(v,niFmt(_A("Can't call _delslot on enumeration table '%s'."),pV->maszName));
}

///////////////////////////////////////////////
static int enum_set(HSQUIRRELVM v)
{
  const sEnumDef* pV;
  if (!SQ_SUCCEEDED(sqa_getEnumDef(v,1,&pV))) return sq_throwerror(v,_A("Can't get enum definition."));
  return sq_throwerror(v,niFmt(_A("Can't call _set on enumeration table '%s'."),pV->maszName));
}

///////////////////////////////////////////////
static int enum_get(HSQUIRRELVM v)
{
  sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,1);
  if (!pV)  return SQ_ERROR;

  HSQOBJECT obj = stack_get(v,2);
  if (sq_type(obj) == OT_STRING) {
    SQTable* valTable = pV->_GetTable(*v->_ss);
    SQObjectPtr val;
    if (!valTable->Get(obj,val)) {
      return sq_throwerror(v, niFmt(_A("Can't find key '%s' in enum '%s'."), _stringhval(obj), pV->pEnumDef->maszName));
    }
    v->Push(val);
  }
  else if (sq_type(obj) == OT_INTEGER || sq_type(obj) == OT_FLOAT) {
    SQTable* valTable = pV->_GetTable(*v->_ss);
    int val = 0;
    if (sq_type(obj) == OT_INTEGER) {
      val = _int(obj);
    }
    else if (sq_type(obj) == OT_FLOAT) {
      val = _int(obj);
    }
    SQObjectPtr key;
    if (!valTable->GetKey(val,key)) {
      return sq_throwerror(v, niFmt(_A("Can't find key '%d' in enum '%s'."),val,pV->pEnumDef->maszName));
    }
    v->Push(key);
  }
  else {
    return sq_throwerror(
        v, niFmt(_A("Lookup in the enumeration table '%s' has to be a string or a number."),
                 pV->pEnumDef->maszName));
  }

  return 1;
}

///////////////////////////////////////////////
static int enum_numElements(HSQUIRRELVM v)
{
  //  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,1);
  if (!pV)  return SQ_ERROR;
  sq_pushint(v,pV->pEnumDef->mnNumValues);
  return 1;
}

///////////////////////////////////////////////
static int enum_enumName(HSQUIRRELVM v)
{
  //  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,1);
  if (!pV)  return SQ_ERROR;
  sq_pushstring(v,_H(pV->pEnumDef->maszName));
  return 1;
}

///////////////////////////////////////////////
static int enum_elementName(HSQUIRRELVM v)
{
  //  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,1);
  if (!pV)  return SQ_ERROR;
  const sEnumDef* enumDef = pV->pEnumDef;

  SQInt idx;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&idx)))
    return sq_throwerror(v, niFmt(_A("enum_elementName, index to lookup in the enumeration table '%s' has to be an integer."),idx,enumDef->maszName,enumDef->mnNumValues));

  if ((tU32)idx > enumDef->mnNumValues)
    return sq_throwerror(v, niFmt(_A("enum_elementName, index '%d' to get in the enumeration table '%s' is out of range (size:%d)."),idx,enumDef->maszName,enumDef->mnNumValues));

  sq_pushstring(v,_H(enumDef->mpValues[idx].maszName));
  return 1;
}

///////////////////////////////////////////////
static int enum_elementValue(HSQUIRRELVM v)
{
  //  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,1);
  if (!pV)  return SQ_ERROR;
  const sEnumDef* enumDef = pV->pEnumDef;

  SQInt idx;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&idx)))
    return sq_throwerror(v, niFmt(_A("enum_elementValue, index to lookup in the enumeration table '%s' has to be an integer."),idx,enumDef->maszName,enumDef->mnNumValues));

  if ((tU32)idx > enumDef->mnNumValues)
    return sq_throwerror(v, niFmt(_A("enum_elementValue, index '%d' to get in the enumeration table '%s' is out of range (size:%d)."),idx,enumDef->maszName,enumDef->mnNumValues));

  sq_pushint(v,enumDef->mpValues[idx].mnValue);
  return 1;
}

///////////////////////////////////////////////
static int enum_gettable(HSQUIRRELVM v)
{
  sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,1);
  if (!pV)  return SQ_ERROR;
  v->Push(pV->_GetTable(*v->_ss));
  return 1;
}

///////////////////////////////////////////////
int sqa_pushEnumDef(HSQUIRRELVM v, const sEnumDef* apEnumDef)
{
  niAssert(apEnumDef != NULL);
  sScriptTypeEnumDef* ud = niNew sScriptTypeEnumDef(
    *v->_ss,apEnumDef);
  sq_pushud(v,ud);
  return SQ_OK;
}

///////////////////////////////////////////////
int sqa_getEnumDef(HSQUIRRELVM v, int idx, const sEnumDef** appEnumDef)
{
  sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,1);
  if (!pV)  return SQ_ERROR;
  if (appEnumDef) *appEnumDef = pV->pEnumDef;
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// iUnknown

///////////////////////////////////////////////
niExportFunc(int) sqa_pushIUnknown(HSQUIRRELVM v, iUnknown* apClass)
{
  return reinterpret_cast<cScriptVM*>(sq_getforeignptr(v))->
      GetAutomation()->PushIUnknown(v,apClass);
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getIUnknown(HSQUIRRELVM v, int idx, iUnknown** appIUnknown, const tUUID& aIID)
{
  return reinterpret_cast<cScriptVM*>(sq_getforeignptr(v))->GetAutomation()->
      GetIUnknown(v,idx,appIUnknown,aIID);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Vectors

struct mathop_Add {
  static tF32 Op(tF32 x, tF32 y) { return x+y; }
};
struct mathop_Sub {
  static tF32 Op(tF32 x, tF32 y) { return x-y; }
};
struct mathop_Mul {
  static tF32 Op(tF32 x, tF32 y) { return x*y; }
};
struct mathop_Div {
  static tF32 Op(tF32 x, tF32 y) { return x/y; }
};
struct mathop_Mod {
  static tF32 Op(tF32 x, tF32 y) { return ::fmodf(x,y); }
};
struct mathop_Neg {
  static tF32 Op(tF32 x) { return -x; }
};

#define GET_MATHUD(NAME,INDEX)                                          \
  SQUserData* ud##NAME;                                                 \
  if (!SQ_SUCCEEDED(sq_getuserdata(v,INDEX,&ud##NAME))) return SQ_ERROR; \
  int type##NAME = udThis->GetType(); niUnused(type##NAME);             \
  float* p##NAME = ((sScriptTypeMathFloats*)ud##NAME)->GetData();

///////////////////////////////////////////////
template<typename OPERATION>
int __cdecl mathop_binary(HSQUIRRELVM v)
{
  GET_MATHUD(This,1);

  int typeOp = sqa_getscripttype(v,2);
  if (typeOp != eScriptType_Int && typeOp != eScriptType_Float && (typeThis != typeOp)) {
    return sq_throwerror(
        v,niFmt(_A("mathop_binary: This '%s' and Op '%s' are not compatible types."),
                v->_ss->GetTypeNameStr(typeThis),
                v->_ss->GetTypeNameStr(typeOp)));
  }

  tU32 dimThis;
  switch (typeThis) {
    case eScriptType_Vec2: dimThis = 2; break;
    case eScriptType_Vec3: dimThis = 3; break;
    case eScriptType_Vec4: dimThis = 4; break;
    case eScriptType_Matrix:  dimThis = 16; break;
    default:
      return sq_throwerror(v,niFmt(_A("mathop_binary: Invalid type '%s'."),
                                   v->_ss->GetTypeNameStr(typeThis)));
  }

  float ret[16];

  if (typeOp == eScriptType_Int || typeOp == eScriptType_Float) {
    tF32 f;
    sq_getf32(v,2,&f);
    for (tU32 i = 0; i < dimThis; ++i)
      ret[i] = OPERATION::Op(pThis[i],f);
  }
  else {
    GET_MATHUD(TheOp,2);
    for (tU32 i = 0; i < dimThis; ++i)
      ret[i] = OPERATION::Op(pThis[i],pTheOp[i]);
  }

  switch (typeThis) {
    case eScriptType_Vec2: sqa_pushvec2f(v,Vec2<tF32>(ret)); break;
    case eScriptType_Vec3: sqa_pushvec3f(v,Vec3<tF32>(ret)); break;
    case eScriptType_Vec4: sqa_pushvec4f(v,Vec4<tF32>(ret)); break;
    case eScriptType_Matrix:  sqa_pushmatrixf(v,Matrix<tF32>(ret)); break;
    default:
      niAssertUnreachable("Unreachable code.");
      break;
  }

  return 1;
}

///////////////////////////////////////////////
template<typename OPERATION>
int __cdecl mathop_unary(HSQUIRRELVM v)
{
  GET_MATHUD(This,1);

  tU32 dimThis;
  switch (typeThis) {
    case eScriptType_Vec2: dimThis = 2; break;
    case eScriptType_Vec3: dimThis = 3; break;
    case eScriptType_Vec4: dimThis = 4; break;
    case eScriptType_Matrix:  dimThis = 16; break;
    default:
      return sq_throwerror(v,niFmt(_A("mathop_unary: Invalid this type '%s'."),
                                   v->_ss->GetTypeNameStr(typeThis)));
  }

  float ret[16];
  tU32 i;
  for (i = 0; i < dimThis; ++i)
    ret[i] = OPERATION::Op(pThis[i]);

  switch (typeThis) {
    case eScriptType_Vec2: sqa_pushvec2f(v,Vec2<tF32>(ret)); break;
    case eScriptType_Vec3: sqa_pushvec3f(v,Vec3<tF32>(ret)); break;
    case eScriptType_Vec4: sqa_pushvec4f(v,Vec4<tF32>(ret)); break;
    case eScriptType_Matrix:  sqa_pushmatrixf(v,Matrix<tF32>(ret)); break;
    default:
      niAssertUnreachable("Unreachable code.");
      break;
  }

  return 1;
}

#if 0
int __cdecl mathop_cmp(HSQUIRRELVM v)
{
  float* pThis; int typeThis;
  if (!SQ_SUCCEEDED(sq_getuserdata(v,1,(ni::tPtr*)&pThis, &typeThis)))  return SQ_ERROR;
  float* pOp; int typeOp;
  if (!SQ_SUCCEEDED(sq_getuserdata(v,2,(ni::tPtr*)&pOp, &typeOp)))  return SQ_ERROR;

  if (typeThis != typeOp)
  {
    return sq_throwerror(
        v,niFmt(_A("mathop_cmp: This '%s' and Op '%s' are not of the same type."),
                v->_ss->GetTypeNameStr(typeThis),
                v->_ss->GetTypeNameStr(typeOp)));
  }

  tU32 dimThis;
  switch (typeThis)
  {
    case eScriptType_Vec2: dimThis = 2; break;
    case eScriptType_Vec3: dimThis = 3; break;
    case eScriptType_Vec4: dimThis = 4; break;
    case eScriptType_Matrix:  dimThis = 16; break;
    default:
      return sq_throwerror(v,niFmt(_A("mathop_cmp: Invalid type '%s'."),
                                   v->_ss->GetTypeNameStr(typeThis)));
  }

  sq_getuserdata(v,2,(ni::tPtr*)&pOp, &typeOp);
  for (tU32 i = 0; i < dimThis; ++i)
  {
    if (pThis[i] < pOp[i])
    {
      sq_pushint(v,-1);
      return 1;
    }
    else if (pThis[i] > pOp[i])
    {
      sq_pushint(v,1);
      return 1;
    }
  }

  sq_pushint(v,0);
  return 1;
}
#endif

#define GET_MATHUD_TYPE(T,NAME,INDEX)                                   \
  SQUserData* ud##NAME;                                                 \
  if (!SQ_SUCCEEDED(sq_getuserdata(v,INDEX,&ud##NAME))) return SQ_ERROR; \
  int type##NAME = udThis->GetType(); niUnused(type##NAME);             \
  T* p##NAME = (T*)((sScriptTypeMathFloats*)ud##NAME)->GetData();

#define GET_MATHUD_VECOP(NAME,INDEX)            \
  GET_MATHUD_TYPE(T,NAME,INDEX)

#define VECOP_VARS1(FUNCNAME)                   \
  GET_MATHUD_VECOP(This,1);

#define VECOP_VARS(FUNCNAME)                                            \
  GET_MATHUD_VECOP(This,1);                                             \
  GET_MATHUD_VECOP(Op,2);                                               \
  if (typeThis != typeOp)  {                                            \
    return sq_throwerror(                                               \
        v,niFmt(_A(#FUNCNAME) _A(": This '%s' and Op '%s' are not compatible types."), \
                v->_ss->GetTypeNameStr(typeThis),                        \
                v->_ss->GetTypeNameStr(typeOp)));                        \
  }                                                                     \

#define VECOP_DO(CALL)                                                  \
  switch (sizeof(T)) {                                                  \
    case sizeof(sVec2f): sqa_pushvec2f(v,(sVec2f&)(CALL)); break; \
    case sizeof(sVec3f): sqa_pushvec3f(v,(sVec3f&)(CALL)); break; \
    case sizeof(sVec4f): sqa_pushvec4f(v,(sVec4f&)(CALL)); break; \
  }

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_add(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS(vecop_add);
  VECOP_DO(VecAdd(vec,*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_sub(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS(vecop_sub);
  VECOP_DO(VecSub(vec,*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_mul(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS(vecop_mul);
  VECOP_DO(VecMul(vec,*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_div(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS(vecop_div);
  VECOP_DO(VecDiv(vec,*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_length(HSQUIRRELVM v)
{
  VECOP_VARS1(vecop_length);
  sq_pushf32(v,VecLength(*pThis));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_lengthsq(HSQUIRRELVM v)
{
  VECOP_VARS1(vecop_lengthsq);
  sq_pushf32(v,VecLengthSq(*pThis));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_dot(HSQUIRRELVM v)
{
  VECOP_VARS(vecop_dot);
  sq_pushf32(v,VecDot(*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_cross(HSQUIRRELVM v)
{
  VECOP_VARS(vecop_cross);

  sVec3f vec;
  switch (sizeof(T)) {
    case sizeof(sVec2f): sq_pushf32(v,(pThis->x*pOp->y)-(pThis->y*pOp->x)); break;
    case sizeof(sVec3f): sqa_pushvec3f(v,(sVec3f&)VecCross(vec,(sVec3f&)*pThis,(sVec3f&)*pOp)); break;
    case sizeof(sVec4f): sqa_pushvec3f(v,(sVec3f&)VecCross(vec,(sVec3f&)*pThis,(sVec3f&)*pOp)); break;
  }

  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_normalize(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS1(vecop_normalize);
  VECOP_DO(VecNormalize(vec,*pThis));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_isnormal(HSQUIRRELVM v)
{
  VECOP_VARS1(vecop_normalize);
  sq_pushint(v,VecIsNormal(*pThis));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_abs(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS1(vecop_abs);
  VECOP_DO(VecAbs(vec,*pThis));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_min(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS(vecop_min);
  VECOP_DO(VecMinimize(vec,*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_max(HSQUIRRELVM v)
{
  T vec;
  VECOP_VARS(vecop_max);
  VECOP_DO(VecMaximize(vec,*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_transform(HSQUIRRELVM v)
{
  GET_MATHUD_VECOP(This,1);
  GET_MATHUD_TYPE(sMatrixf,Op,2);
  if (typeOp != eScriptType_Matrix)
    return sq_throwerror(v,niFmt(_A("vecop_transform: The operand must be a matrix.")));

  sVec4f vec;
  sqa_pushvec4f(v,VecTransform(vec,*pThis,*pOp));
  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_transform_coord(HSQUIRRELVM v)
{
  GET_MATHUD_VECOP(This,1);
  GET_MATHUD_TYPE(sMatrixf,Op,2);
  if (typeOp != eScriptType_Matrix)
    return sq_throwerror(v,niFmt(_A("vecop_transform_coord: The operand must be a matrix.")));

  switch (sizeof(T)) {
    case sizeof(sVec2f): {
      sVec2f vec;
      sqa_pushvec2f(v,VecTransformCoord(vec,(sVec2f&)*pThis,*pOp));
      break;
    }
    case sizeof(sVec3f): {
      sVec3f vec;
      sqa_pushvec3f(v,VecTransformCoord(vec,(sVec3f&)*pThis,*pOp));
      break;
    }
    case sizeof(sVec4f): {
      sVec4f vec;
      sqa_pushvec4f(v,VecTransform(vec,(sVec4f&)*pThis,*pOp));
      break;
    }
  }

  return 1;
}

///////////////////////////////////////////////
template <typename T>
int __cdecl vecop_transform_normal(HSQUIRRELVM v)
{
  GET_MATHUD_VECOP(This,1);
  GET_MATHUD_TYPE(sMatrixf,Op,2);
  if (typeOp != eScriptType_Matrix)
    return sq_throwerror(v,niFmt(_A("vecop_transform_normal: The operand must be a matrix.")));

  switch (sizeof(T)) {
    case sizeof(sVec2f): {
      sVec2f vec;
      sqa_pushvec2f(v,VecTransformNormal(vec,(sVec2f&)*pThis,*pOp));
      break;
    }
    case sizeof(sVec3f): {
      sVec3f vec;
      sqa_pushvec3f(v,VecTransformNormal(vec,(sVec3f&)*pThis,*pOp));
      break;
    }
    case sizeof(sVec4f): {
      sVec4f vec;
      sqa_pushvec4f(v,VecTransform(vec,(sVec4f&)*pThis,*pOp));
      break;
    }
  }

  return 1;
}

///////////////////////////////////////////////
int __cdecl rect_intersect(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pThis = sqa_getud<sScriptTypeVec4f>(v,1);
  if (!pThis) return SQ_ERROR;
  const sRectf& thisRect = (sRectf&)pThis->_val;

  SQUserData* pOp;
  if (!SQ_SUCCEEDED(sq_getuserdata(v,2,&pOp)))  return SQ_ERROR;

  switch (pOp->GetType()) {
    case eScriptType_Vec2:
      sq_pushint(v,thisRect.Intersect(((sScriptTypeVec2f*)pOp)->_val));
      break;
    case eScriptType_Vec4:
      sq_pushint(v,thisRect.IntersectRect(((sScriptTypeVec4f*)pOp)->_val));
      break;
    default:
      return sq_throwerror(v,niFmt(_A("rect_intersect: The operand must be a vector2 or vector4 or a rectangle.")));
  }

  return 1;
}

///////////////////////////////////////////////
int __cdecl rect_frameCenter(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pThis = sqa_getud<sScriptTypeVec4f>(v,1);
  if (!pThis) return SQ_ERROR;
  const sRectf& thisRect = (sRectf&)pThis->_val;

  tF32 l,r,t,b;
  if (!SQ_SUCCEEDED(sq_getf32(v,2,&l))) return sq_throwerror(v,_A("rect_frameCenter: left parameter must be a number."));
  if (!SQ_SUCCEEDED(sq_getf32(v,3,&r))) return sq_throwerror(v,_A("rect_frameCenter: right parameter must be a number."));
  if (!SQ_SUCCEEDED(sq_getf32(v,4,&t))) return sq_throwerror(v,_A("rect_frameCenter: top parameter must be a number."));
  if (!SQ_SUCCEEDED(sq_getf32(v,5,&b))) return sq_throwerror(v,_A("rect_frameCenter: bottom parameter must be a number."));

  sqa_pushvec4f(v,thisRect.ComputeFrameCenter(l,r,t,b));
  return 1;
}

///////////////////////////////////////////////
int __cdecl rect_frameLeftBorder(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pThis = sqa_getud<sScriptTypeVec4f>(v,1);
  if (!pThis) return SQ_ERROR;
  const sRectf& thisRect = (sRectf&)pThis->_val;
  sScriptTypeVec4f* pFrame = sqa_getud<sScriptTypeVec4f>(v,2);
  if (!pFrame)  return sq_throwerror(v,niFmt(_A("rect_frameLeftBorder: The parameter must be a rectangle/vec4.")));
  const sRectf& frameRect = (sRectf&)pFrame->_val;
  sq_pushf32(v,thisRect.GetFrameLeftBorder(frameRect));
  return 1;
}

///////////////////////////////////////////////
int __cdecl rect_frameRightBorder(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pThis = sqa_getud<sScriptTypeVec4f>(v,1);
  if (!pThis) return SQ_ERROR;
  const sRectf& thisRect = (sRectf&)pThis->_val;
  sScriptTypeVec4f* pFrame = sqa_getud<sScriptTypeVec4f>(v,2);
  if (!pFrame)  return sq_throwerror(v,niFmt(_A("rect_frameRightBorder: The parameter must be a rectangle/vec4.")));
  const sRectf& frameRect = (sRectf&)pFrame->_val;
  sq_pushf32(v,thisRect.GetFrameRightBorder(frameRect));
  return 1;
}

///////////////////////////////////////////////
int __cdecl rect_frameTopBorder(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pThis = sqa_getud<sScriptTypeVec4f>(v,1);
  if (!pThis) return SQ_ERROR;
  const sRectf& thisRect = (sRectf&)pThis->_val;
  sScriptTypeVec4f* pFrame = sqa_getud<sScriptTypeVec4f>(v,2);
  if (!pFrame)  return sq_throwerror(v,niFmt(_A("rect_frameTopBorder: The parameter must be a rectangle/vec4.")));
  const sRectf& frameRect = (sRectf&)pFrame->_val;
  sq_pushf32(v,thisRect.GetFrameTopBorder(frameRect));
  return 1;
}

///////////////////////////////////////////////
int __cdecl rect_frameBottomBorder(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pThis = sqa_getud<sScriptTypeVec4f>(v,1);
  if (!pThis) return SQ_ERROR;
  const sRectf& thisRect = (sRectf&)pThis->_val;
  sScriptTypeVec4f* pFrame = sqa_getud<sScriptTypeVec4f>(v,2);
  if (!pFrame)  return sq_throwerror(v,niFmt(_A("rect_frameBottomBorder: The parameter must be a rectangle/vec4.")));
  const sRectf& frameRect = (sRectf&)pFrame->_val;
  sq_pushf32(v,thisRect.GetFrameBottomBorder(frameRect));
  return 1;
}

///////////////////////////////////////////////
int __cdecl rect_frameBorder(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pThis = sqa_getud<sScriptTypeVec4f>(v,1);
  if (!pThis) return SQ_ERROR;
  const sRectf& thisRect = (sRectf&)pThis->_val;
  sScriptTypeVec4f* pFrame = sqa_getud<sScriptTypeVec4f>(v,2);
  if (!pFrame)  return sq_throwerror(v,niFmt(_A("rect_frameLeftBorder: The parameter must be a rectangle/vec4.")));
  const sRectf& frameRect = (sRectf&)pFrame->_val;
  sqa_pushvec4f(v,thisRect.GetFrameBorder(frameRect));
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Vec2

///////////////////////////////////////////////
static int vec2f_tostring(HSQUIRRELVM v)
{
  sScriptTypeVec2f* pV = sqa_getud<sScriptTypeVec2f>(v,-1);
  if (!pV) return SQ_ERROR;
  cString str(pV->_val);
  sq_pushstring(v,_H(str));
  return 1;
}

///////////////////////////////////////////////
static int vec2f_typeof(HSQUIRRELVM v)
{
  sq_pushstring(v, _HC(Vec2));
  return 1;
}

///////////////////////////////////////////////
static int vec2f_set(HSQUIRRELVM v)
{
  sScriptTypeVec2f* pV = sqa_getud<sScriptTypeVec2f>(v,-3);
  if (!pV) return SQ_ERROR;
  tF32* vals = pV->_val.ptr();

  int type = sq_gettype(v,-2);
  if (type == OT_INTEGER) {
    SQInt idx;
    sq_getint(v,-2,&idx);
    if (idx < 0 || idx > 1) return sq_throwerror(v,_A("vec2 set, index out of range."));
    sq_getf32(v,-1,&vals[idx]);
  }
  else if (type == OT_STRING) {
    SQInt idx;
    const achar* aaszStr;
    sq_getstring(v,-2,&aaszStr);
    switch (*aaszStr)
    {
      case 'x': case 'r': idx = 0; break;
      case 'y': case 'g': idx = 1; break;
      default: return sq_throwerror(v,_A("vec2 set, unknown member."));
    }
    sq_getf32(v,-1,&vals[idx]);
  }
  else {
    return sq_throwerror(v,_A("vec2 set, can index 2d vector only with string and integer."));
  }
  return 0;
}

///////////////////////////////////////////////
static int vec2f_get(HSQUIRRELVM v)
{
  sScriptTypeVec2f* pV = sqa_getud<sScriptTypeVec2f>(v,-2);
  if (!pV) return SQ_ERROR;
  const tF32* vals = pV->_val.ptr();

  int type = sq_gettype(v,-1);
  if (type == OT_INTEGER)
  {
    SQInt idx;
    sq_getint(v,-1,&idx);
    if (idx < 0 || idx > 1) return sq_throwerror(v,_A("vec2f_get, index out of range."));
    sq_pushf32(v,vals[idx]);
    return 1;
  }
  else if (type == OT_STRING)
  {
    const achar* aaszStr;
    sq_getstring(v,-1,&aaszStr);
    int len = ni::StrLen(aaszStr);
    switch (len)
    {
      case 1:
        {
          tF32 r;
          if (sqa_getVecElementFromChar(v,aaszStr[0],vals,2,r)) {
            sq_pushf32(v,r);
            return 1;
          }
          break;
        }
      case 2:
        {
          sVec2f r;
          if (sqa_getVecElementFromChar(v,aaszStr[0],vals,2,r[0])) {
            if (sqa_getVecElementFromChar(v,aaszStr[1],vals,2,r[1])) {
              sqa_pushvec2f(v,r);
              return 1;
            }
          }
          break;
        }
    }

    return sq_throwerror(v,niFmt(_A("vec2f_get, invalid member '%s'."),aaszStr));
  }

  return sq_throwerror(v,_A("vec2f_get, can index 2d vector only with string and integer."));
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushvec2f(HSQUIRRELVM v, const sVec2f& aV)
{
  sScriptTypeVec2f* ud = niNew sScriptTypeVec2f(*v->_ss,aV);
  sq_pushud(v,ud);
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getvec2f(HSQUIRRELVM v, int idx, sVec2f* aV)
{
  sScriptTypeVec2f* pV = sqa_getud<sScriptTypeVec2f>(v,idx);
  if (!pV) return SQ_ERROR;
  *aV = pV->_val;
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushvec2i(HSQUIRRELVM v, const sVec2i& aV)
{
  sVec2f vf = {
    (float)aV.x,
    (float)aV.y
  };
  return sqa_pushvec2f(v,vf);
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getvec2i(HSQUIRRELVM v, int idx, sVec2i* aV)
{
  sVec2f vf;
  if (!SQ_SUCCEEDED(sqa_getvec2f(v,idx,&vf)))  return SQ_ERROR;
  aV->x = (long)vf.x;
  aV->y = (long)vf.y;
  return SQ_OK;
}

///////////////////////////////////////////////
static int vec2f_constructor(HSQUIRRELVM v)
{
  sVec2f val;
  int top = sq_gettop(v)-1;
  switch (top)
  {
    case 0:
      {
        val = sVec2f::Zero();
        break;
      }
    case 1:
      {
        eScriptType type = sqa_getscripttype(v,-1);
        switch (type)
        {
          case eScriptType_Int:
          case eScriptType_Float:
            {
              sq_getf32(v,-1,&val.x);
              val.y = val.x;
              break;
            }
          case eScriptType_Vec2:
            {
              sqa_getvec2f(v,-1,&val);
              break;
            }
          case eScriptType_Vec3:
            {
              sVec3f tmpV;
              sqa_getvec3f(v,-1,&tmpV);
              val.x = tmpV.x; val.y = tmpV.y;
              break;
            }
          case eScriptType_Vec4:
            {
              sVec4f tmpV;
              sqa_getvec4f(v,-1,&tmpV);
              val.x = tmpV.x; val.y = tmpV.y;
              break;
            }
          default:
            {
              return sq_throwerror(v,_A("vec2 constructor error, copy constructor parameter invalid type."));
            }
        }
        break;
      }
    case 2:
      {
        sq_getf32(v,-2,&val.x);
        sq_getf32(v,-1,&val.y);
        break;
      }
    default:
      return sq_throwerror(v,niFmt(_A("vec2 constructor error, invalid number of parameters '%d'."),top));
  }
  sqa_pushvec2f(v,val);
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Vec3

///////////////////////////////////////////////
static int vec3f_tostring(HSQUIRRELVM v)
{
  sScriptTypeVec3f* pV = sqa_getud<sScriptTypeVec3f>(v,-1);
  if (!pV) return SQ_ERROR;
  cString str(pV->_val);
  sq_pushstring(v,_H(str));
  return 1;
}

///////////////////////////////////////////////
static int vec3f_toint(HSQUIRRELVM v)
{
  sScriptTypeVec3f* pV = sqa_getud<sScriptTypeVec3f>(v,-1);
  if (!pV) return SQ_ERROR;
  tF32* vals = pV->_val.ptr();
  sq_pushint(v,ULColorBuildf(vals[0],vals[1],vals[2]));
  return 1;
}

///////////////////////////////////////////////
static int vec3f_typeof(HSQUIRRELVM v)
{
  sq_pushstring(v, _HC(Vec3));
  return 1;
}

///////////////////////////////////////////////
static int vec3f_set(HSQUIRRELVM v)
{
  sScriptTypeVec3f* pV = sqa_getud<sScriptTypeVec3f>(v,-3);
  if (!pV) return SQ_ERROR;
  tF32* vals = pV->_val.ptr();

  int type = sq_gettype(v,-2);
  if (type == OT_INTEGER)
  {
    SQInt idx;
    sq_getint(v,-2,&idx);
    if (idx < 0 || idx > 2) return sq_throwerror(v,_A("vec3 set, index out of range."));
    sq_getf32(v,-1,&vals[idx]);
  }
  else if (type == OT_STRING)
  {
    SQInt idx;
    const achar* aaszStr;
    sq_getstring(v,-2,&aaszStr);
    switch (*aaszStr)
    {
      case 'x': case 'r': idx = 0; break;
      case 'y': case 'g': idx = 1; break;
      case 'z': case 'b': idx = 2; break;
      default: return sq_throwerror(v,_A("vec3 set, unknown member."));
    }

    sq_getf32(v,-1,&vals[idx]);
  }
  else
  {
    return sq_throwerror(v,_A("vec3 set, can index 3d vector only with string and integer."));
  }
  return 0;
}

///////////////////////////////////////////////
static int vec3f_get(HSQUIRRELVM v)
{
  sScriptTypeVec3f* pV = sqa_getud<sScriptTypeVec3f>(v,-2);
  if (!pV) return SQ_ERROR;
  const tF32* vals = pV->_val.ptr();

  int type = sq_gettype(v,-1);
  if (type == OT_INTEGER)
  {
    SQInt idx;
    sq_getint(v,-1,&idx);
    if (idx < 0 || idx > 2) return sq_throwerror(v,_A("vec3f_get, index out of range."));
    sq_pushf32(v,vals[idx]);
    return 1;
  }
  else if (type == OT_STRING)
  {
    const achar* aaszStr;
    sq_getstring(v,-1,&aaszStr);
    int len = ni::StrLen(aaszStr);
    switch (len)
    {
      case 1:
        {
          tF32 r;
          if (sqa_getVecElementFromChar(v,aaszStr[0],vals,3,r)) {
            sq_pushf32(v,r);
            return 1;
          }
          break;
        }
      case 2:
        {
          sVec2f r;
          if (sqa_getVecElementFromChar(v,aaszStr[0],vals,3,r[0])) {
            if (sqa_getVecElementFromChar(v,aaszStr[1],vals,3,r[1])) {
              sqa_pushvec2f(v,r);
              return 1;
            }
          }
          break;
        }
      case 3:
        {
          sVec3f r;
          if (sqa_getVecElementFromChar(v,aaszStr[0],vals,3,r[0])) {
            if (sqa_getVecElementFromChar(v,aaszStr[1],vals,3,r[1])) {
              if (sqa_getVecElementFromChar(v,aaszStr[2],vals,3,r[2])) {
                sqa_pushvec3f(v,r);
                return 1;
              }
            }
          }
          break;
        }
    }

    return sq_throwerror(v,niFmt(_A("vec3f_get, invalid member '%s'."),aaszStr));
  }
  else
  {
    return sq_throwerror(v,_A("vec3f_get, can index 3d vector only with string and integer."));
  }
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushvec3f(HSQUIRRELVM v, const sVec3f& aV)
{
  sScriptTypeVec3f* ud = niNew sScriptTypeVec3f(*v->_ss,aV);
  sq_pushud(v,ud);
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getvec3f(HSQUIRRELVM v, int idx, sVec3f* aV)
{
  sScriptTypeVec3f* pV = sqa_getud<sScriptTypeVec3f>(v,idx);
  if (!pV) return SQ_ERROR;
  *aV = pV->_val;
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushvec3i(HSQUIRRELVM v, const sVec3i& aV)
{
  sVec3f vf = { (float)aV.x, (float)aV.y, (float)aV.z };
  return sqa_pushvec3f(v,vf);
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getvec3i(HSQUIRRELVM v, int idx, sVec3i* aV)
{
  sVec3f vf;
  if (!SQ_SUCCEEDED(sqa_getvec3f(v,idx,&vf)))  return SQ_ERROR;
  aV->x = (long)vf.x;
  aV->y = (long)vf.y;
  aV->z = (long)vf.z;
  return SQ_OK;
}

///////////////////////////////////////////////
static int vec3f_constructor(HSQUIRRELVM v)
{
  sVec3f val;
  int top = sq_gettop(v)-1;
  switch (top)
  {
    case 0:
      {
        val = sVec3f::Zero();
        break;
      }
    case 1:
      {
        eScriptType type = sqa_getscripttype(v,-1);
        switch (type)
        {
          case eScriptType_Int:
            {
              tU32 vi;
              sq_getint(v,-1,(SQInt*)&vi);
              val.x = ULColorGetRf(vi);
              val.y = ULColorGetGf(vi);
              val.z = ULColorGetBf(vi);
              break;
            }
          case eScriptType_Float:
            {
              sq_getf32(v,-1,&val.x);
              val.y = val.z = val.x;
              break;
            }
          case eScriptType_Vec2:
            {
              sqa_getvec3f(v,-1,&val);
              val.z = 0.0f;
              break;
            }
          case eScriptType_Vec3:
            {
              sqa_getvec3f(v,-1,&val);
              break;
            }
          case eScriptType_Vec4:
            {
              sVec4f tmpV;
              sqa_getvec4f(v,-1,&tmpV);
              val.x = tmpV.x; val.y = tmpV.y; val.z = tmpV.z;
              break;
            }
          default:
            {
              return sq_throwerror(v,_A("vec3 constructor error, copy constructor parameter invalid type."));
            }
        }
        break;
      }
    case 2:
      {
        sq_getf32(v,-2,&val.x);
        sq_getf32(v,-1,&val.y);
        val.z = 0.0f;
        break;
      }
    case 3:
      {
        sq_getf32(v,-3,&val.x);
        sq_getf32(v,-2,&val.y);
        sq_getf32(v,-1,&val.z);
        break;
      }
    default:
      return sq_throwerror(v,niFmt(_A("vec3 constructor error, invalid number of parameters '%d'."),top));
  }
  sqa_pushvec3f(v,val);
  return 1;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Vec4

///////////////////////////////////////////////
static int vec4f_tostring(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pV = sqa_getud<sScriptTypeVec4f>(v,-1);
  if (!pV) return SQ_ERROR;
  cString str(pV->_val);
  sq_pushstring(v,_H(str));
  return 1;
}

///////////////////////////////////////////////
static int vec4f_toint(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pV = sqa_getud<sScriptTypeVec4f>(v,-1);
  if (!pV) return SQ_ERROR;
  tF32* vals = pV->_val.ptr();
  sq_pushint(v,ULColorBuildf(vals[0],vals[1],vals[2],vals[3]));
  return 1;
}

///////////////////////////////////////////////
static int vec4f_typeof(HSQUIRRELVM v)
{
  sq_pushstring(v, _HC(Vec4));
  return 1;
}

///////////////////////////////////////////////
static int vec4f_set(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pV = sqa_getud<sScriptTypeVec4f>(v,-3);
  if (!pV) return SQ_ERROR;
  sVec4f& vals = pV->_val;

  int type = sq_gettype(v,-2);
  if (type == OT_INTEGER) {
    SQInt idx;
    sq_getint(v,-2,&idx);
    if (idx < 0 || idx > 3) return sq_throwerror(v,_A("vec4 set, index out of range."));
    sq_getf32(v,-1,&vals[idx]);
  }
  else if (type == OT_STRING) {
    SQInt idx;
    const achar* aaszStr;
    sq_getstring(v,-2,&aaszStr);
    if (aaszStr[1] == 0)
    {
      switch (*aaszStr)
      {
        case 'x': case 'r': idx = 0; break;
        case 'y': case 'g': idx = 1; break;
        case 'z': case 'b': idx = 2; break;
        case 'w': case 'a': idx = 3; break;
        default: return sq_throwerror(v,_A("vec4 set, unknown member."));
      }
    }
    else
    {
      if (ni::StrEq(aaszStr,_A("width")))
      {
        tF32 w;
        sq_getf32(v,-1,&w);
        vals.Right() = vals.Left()+w;
        return 0;
      }
      else if (ni::StrEq(aaszStr,_A("height")))
      {
        tF32 h;
        sq_getf32(v,-1,&h);
        vals.Bottom() = vals.Top()+h;
        return 0;
      }
      else if (ni::StrEq(aaszStr,_A("size"))) {
        sVec2f vec;
        if (SQ_FAILED(sqa_getvec2f(v,-1,&vec)))
          return sq_throwerror(v,niFmt(_A("vec4 set, size is a vec2."), aaszStr));
        static_cast<sRectf&>(vals).SetSize(vec.x,vec.y);
        return 0;
      }
      else if (ni::StrEq(aaszStr,_A("top_left"))) {
        sVec2f vec;
        if (SQ_FAILED(sqa_getvec2f(v,-1,&vec)))
          return sq_throwerror(v,niFmt(_A("vec4 set, top_left is a vec2."), aaszStr));
        static_cast<sRectf&>(vals).SetCorner(eRectCorners_TopLeft,vec);
        return 0;
      }
      else if (ni::StrEq(aaszStr,_A("top_right"))) {
        sVec2f vec;
        if (SQ_FAILED(sqa_getvec2f(v,-1,&vec)))
          return sq_throwerror(v,niFmt(_A("vec4 set, top_right is a vec2."), aaszStr));
        static_cast<sRectf&>(vals).SetCorner(eRectCorners_TopRight,vec);
        return 0;
      }
      else if (ni::StrEq(aaszStr,_A("bottom_left"))) {
        sVec2f vec;
        if (SQ_FAILED(sqa_getvec2f(v,-1,&vec)))
          return sq_throwerror(v,niFmt(_A("vec4 set, bottom_left is a vec2."), aaszStr));
        static_cast<sRectf&>(vals).SetCorner(eRectCorners_BottomLeft,vec);
        return 0;
      }
      else if (ni::StrEq(aaszStr,_A("bottom_right"))) {
        sVec2f vec;
        if (SQ_FAILED(sqa_getvec2f(v,-1,&vec)))
          return sq_throwerror(v,niFmt(_A("vec4 set, bottom_right is a vec2."), aaszStr));
        static_cast<sRectf&>(vals).SetCorner(eRectCorners_BottomRight,vec);
        return 0;
      }
      else if (ni::StrEq(aaszStr,_A("left")))
        idx = 0;
      else if (ni::StrEq(aaszStr,_A("top")))
        idx = 1;
      else if (ni::StrEq(aaszStr,_A("right")))
        idx = 2;
      else if (ni::StrEq(aaszStr,_A("bottom")))
        idx = 3;
      else
        return sq_throwerror(v,niFmt(_A("vec4 set, unknown member '%s'."), aaszStr));
    }
    sq_getf32(v,-1,&vals[idx]);
  }
  else
  {
    return sq_throwerror(v,_A("vec4 set, can index 3d vector only with string and integer."));
  }
  return 0;
}

///////////////////////////////////////////////
static int vec4f_get(HSQUIRRELVM v)
{
  sScriptTypeVec4f* pV = sqa_getud<sScriptTypeVec4f>(v,-2);
  if (!pV) return SQ_ERROR;
  const sVec4f& vals = pV->_val;

  int type = sq_gettype(v,-1);
  if (type == OT_INTEGER) {
    SQInt idx;
    sq_getint(v,-1,&idx);
    if (idx < 0 || idx > 3) return sq_throwerror(v,_A("vec4f_get, index out of range."));
    sq_pushf32(v,vals[idx]);
    return 1;
  }
  else if (type == OT_STRING) {
    const achar* aaszStr;
    sq_getstring(v,-1,&aaszStr);
    int len = ni::StrLen(aaszStr);
    switch (len)
    {
      case 1: {
        tF32 r;
        if (sqa_getVecElementFromChar(v,aaszStr[0],vals.ptr(),4,r)) {
          sq_pushf32(v,r);
          return 1;
        }
        break;
      }
      case 2: {
        sVec2f r;
        if (sqa_getVecElementFromChar(v,aaszStr[0],vals.ptr(),4,r[0])) {
          if (sqa_getVecElementFromChar(v,aaszStr[1],vals.ptr(),4,r[1])) {
            sqa_pushvec2f(v,r);
            return 1;
          }
        }
        break;
      }
      case 3:
        {
          sVec3f r;
          if (sqa_getVecElementFromChar(v,aaszStr[0],vals.ptr(),4,r[0])) {
            if (sqa_getVecElementFromChar(v,aaszStr[1],vals.ptr(),4,r[1])) {
              if (sqa_getVecElementFromChar(v,aaszStr[2],vals.ptr(),4,r[2])) {
                sqa_pushvec3f(v,r);
                return 1;
              }
            }
          }
          break;
        }
      case 4:
        {
          sVec4f r;
          if (sqa_getVecElementFromChar(v,aaszStr[0],vals.ptr(),4,r[0])) {
            if (sqa_getVecElementFromChar(v,aaszStr[1],vals.ptr(),4,r[1])) {
              if (sqa_getVecElementFromChar(v,aaszStr[2],vals.ptr(),4,r[2])) {
                if (sqa_getVecElementFromChar(v,aaszStr[3],vals.ptr(),4,r[3])) {
                  sqa_pushvec4f(v,r);
                  return 1;
                }
              }
            }
          }
          break;
        }
    }

    if (ni::StrEq(aaszStr,_A("width"))) {
      sq_pushf32(v,vals.Right()-vals.Left());
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("height"))) {
      sq_pushf32(v,vals.Bottom()-vals.Top());
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("size"))) {
      sqa_pushvec2f(v,static_cast<const sRectf&>(vals).GetSize());
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("center"))) {
      sqa_pushvec2f(v,static_cast<const sRectf&>(vals).GetCenter());
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("top_left"))) {
      sqa_pushvec2f(v,static_cast<const sRectf&>(vals).GetCorner(eRectCorners_TopLeft));
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("top_right"))) {
      sqa_pushvec2f(v,static_cast<const sRectf&>(vals).GetCorner(eRectCorners_TopRight));
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("bottom_left"))) {
      sqa_pushvec2f(v,static_cast<const sRectf&>(vals).GetCorner(eRectCorners_BottomLeft));
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("bottom_right"))) {
      sqa_pushvec2f(v,static_cast<const sRectf&>(vals).GetCorner(eRectCorners_BottomRight));
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("left"))) {
      sq_pushf32(v,vals.Left());
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("top"))) {
      sq_pushf32(v,vals.Top());
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("right"))) {
      sq_pushf32(v,vals.Right());
      return 1;
    }
    else if (ni::StrEq(aaszStr,_A("bottom"))) {
      sq_pushf32(v,vals.Bottom());
      return 1;
    }

    return sq_throwerror(v,niFmt(_A("vec4f_get, invalid member '%s'."),aaszStr));
  }

  return sq_throwerror(v,_A("vec4f_get, can index 4d vector only with string and integer."));
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushvec4f(HSQUIRRELVM v, const sVec4f& aV)
{
  sScriptTypeVec4f* ud = niNew sScriptTypeVec4f(*v->_ss,aV);
  sq_pushud(v,ud);
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getvec4f(HSQUIRRELVM v, int idx, sVec4f* aV)
{
  sScriptTypeVec4f* pV = sqa_getud<sScriptTypeVec4f>(v,idx);
  if (!pV) return SQ_ERROR;
  const sVec4f& vals = pV->_val;
  *aV = vals;
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushvec4i(HSQUIRRELVM v, const sVec4i& aV)
{
  sVec4f vf = { (float)aV.x,(float)aV.y,(float)aV.z,(float)aV.w };
  sqa_pushvec4f(v,vf);
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getvec4i(HSQUIRRELVM v, int idx, sVec4i* aV)
{
  sVec4f vf;
  if (!SQ_SUCCEEDED(sqa_getvec4f(v,idx,&vf)))  return SQ_ERROR;
  aV->x = (long)vf.x;
  aV->y = (long)vf.y;
  aV->z = (long)vf.z;
  aV->w = (long)vf.w;
  return SQ_OK;
}

///////////////////////////////////////////////
static int vector4base_constructor(HSQUIRRELVM v, sVec4f& val)
{
  int top = sq_gettop(v)-1;
  switch (top)
  {
    case 0:
      {
        val = sVec4f::Zero();
        break;
      }
    case 1:
      {
        eScriptType type = sqa_getscripttype(v,-1);
        switch (type)
        {
          case eScriptType_Int:
            {
              tU32 vi;
              sq_getint(v,-1,(SQInt*)&vi);
              val.x = ULColorGetRf(vi);
              val.y = ULColorGetGf(vi);
              val.z = ULColorGetBf(vi);
              val.w = ULColorGetAf(vi);
              break;
            }
          case eScriptType_Float:
            {
              sq_getf32(v,-1,&val.x);
              val.y = val.z = val.w = val.x;
              break;
            }
          case eScriptType_Vec2:
            {
              sqa_getvec2f(v,-1,(sVec2f*)&val);
              val.z = 0.0f; val.w = 0.0f;
              break;
            }
          case eScriptType_Vec3:
            {
              sqa_getvec3f(v,-1,(sVec3f*)&val);
              val.w = 0.0f;
              break;
            }
          case eScriptType_Vec4:
            {
              sqa_getvec4f(v,-1,&val);
              break;
            }
          default:
            {
              return sq_throwerror(v,_A("vector4base_constructor, copy constructor parameter invalid type."));
            }
        }
        break;
      }
    case 2:
      {
        sq_getf32(v,-2,&val.x);
        sq_getf32(v,-1,&val.y);
        val.z = 0.0f; val.w = 0.0f;
        break;
      }
    case 3:
      {
        sq_getf32(v,-3,&val.x);
        sq_getf32(v,-2,&val.y);
        sq_getf32(v,-1,&val.z);
        val.w = 0.0f;
        break;
      }
    case 4:
      {
        sq_getf32(v,-4,&val.x);
        sq_getf32(v,-3,&val.y);
        sq_getf32(v,-2,&val.z);
        sq_getf32(v,-1,&val.w);
        break;
      }
    default:
      return sq_throwerror(v,niFmt(_A("vector4base_constructor, invalid number of parameters '%d'."),top));
  }
  return 1;
}

///////////////////////////////////////////////
static int vec4f_constructor(HSQUIRRELVM v)
{
  sVec4f val;
  int r = vector4base_constructor(v,val);
  if (SQ_FAILED(r)) return r;
  sqa_pushvec4f(v,val);
  return 1;
}

///////////////////////////////////////////////
static int rect_constructor(HSQUIRRELVM v)
{
  sVec4f val;
  int r = vector4base_constructor(v,val);
  if (SQ_FAILED(r)) return r;
  sqa_pushvec4f(v,Vec4<tF32>(val.x,val.y,val.x+val.z,val.y+val.w));
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
static int matrixf_tostring(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  cString str(vals);
  sq_pushstring(v, _H(str));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_typeof(HSQUIRRELVM v)
{
  sq_pushstring(v, _HC(Matrix));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_set(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-3);
  if (!pV) return SQ_ERROR;
  sMatrixf& vals = pV->_val;

  int type = sq_gettype(v,-2);
  if (type == OT_STRING)
  {
    SQInt ridx,cidx;
    const achar* aaszStr;
    sq_getstring(v,-2,&aaszStr);
    const achar* p = aaszStr;
    if (*p++ != _A('_'))
    {
      sVec3f val;
      if (ni::StrEq(aaszStr,_A("right"))) {
        if (!SQ_SUCCEEDED(sqa_getvec3f(v,-1,&val))) return sq_throwerror(v,_A("matrixf_set, invalid type, right is a vector3."));
        MatrixSetRight(vals,val);
      }
      else if (ni::StrEq(aaszStr,_A("up"))) {
        if (!SQ_SUCCEEDED(sqa_getvec3f(v,-1,&val))) return sq_throwerror(v,_A("matrixf_set, invalid type, up is a vector3."));
        MatrixSetUp(vals,val);
      }
      else if (ni::StrEq(aaszStr,_A("forward"))) {
        if (!SQ_SUCCEEDED(sqa_getvec3f(v,-1,&val))) return sq_throwerror(v,_A("matrixf_set, invalid type, forward is a vector3."));
        MatrixSetForward(vals,val);
      }
      else if (ni::StrEq(aaszStr,_A("translation"))) {
        if (!SQ_SUCCEEDED(sqa_getvec3f(v,-1,&val))) return sq_throwerror(v,_A("matrixf_set, invalid type, translation is a vector3."));
        MatrixSetTranslation(vals,val);
      }
      else
        return sq_throwerror(v,niFmt(_A("matrixf_set, unknown member '%s'."), aaszStr));
    }
    else
    {
      switch (*p++) {
        case '1': ridx = 0; break;
        case '2': ridx = 1; break;
        case '3': ridx = 2; break;
        case '4': ridx = 3; break;
        default: return sq_throwerror(v,niFmt(_A("matrix set, invalid row in member '%s'."), aaszStr));
      }
      switch (*p) {
        case '1': cidx = 0; break;
        case '2': cidx = 1; break;
        case '3': cidx = 2; break;
        case '4': cidx = 3; break;
        default: return sq_throwerror(v,niFmt(_A("matrix set, invalid column in member '%s'."), aaszStr));
      }
      sq_getf32(v,-1,&vals(ridx,cidx));
    }
  }
  else if (type == OT_INTEGER)
  {
    SQInt idx;
    sq_getint(v,-2,&idx);
    if (idx < 0 || idx > 15) return sq_throwerror(v,_A("matrix get, index out of range."));
    sq_getf32(v,-1,&vals.ptr()[idx]);
  }
  else
  {
    return sq_throwerror(v,_A("vec4 set, can index matrix only with string."));
  }
  return 0;
}

///////////////////////////////////////////////
static int matrixf_get(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-2);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;

  int type = sq_gettype(v,-1);
  if (type == OT_STRING)
  {
    SQInt ridx, cidx;
    const achar* aaszStr;
    sq_getstring(v,-1,&aaszStr);
    const achar* p = aaszStr;
    if (*p++ != _A('_'))
    {
      if (ni::StrEq(aaszStr,_A("right"))) {
        sqa_pushvec3f(v,Vec3<tF32>(vals._11,vals._21,vals._31));
      }
      else if (ni::StrEq(aaszStr,_A("up"))) {
        sqa_pushvec3f(v,Vec3<tF32>(vals._12,vals._22,vals._32));
      }
      else if (ni::StrEq(aaszStr,_A("forward"))) {
        sqa_pushvec3f(v,Vec3<tF32>(vals._13,vals._23,vals._33));
      }
      else if (ni::StrEq(aaszStr,_A("translation"))) {
        sqa_pushvec3f(v,Vec3<tF32>(vals._41,vals._42,vals._43));
      }
      else
        return sq_throwerror(v,niFmt(_A("matrix set, unknown member '%s'."), aaszStr));
    }
    else
    {
      switch (*p++) {
        case '1': ridx = 0; break;
        case '2': ridx = 1; break;
        case '3': ridx = 2; break;
        case '4': ridx = 3; break;
        default: return sq_throwerror(v,niFmt(_A("matrix set, invalid row in member '%s'."), aaszStr));
      }
      switch (*p) {
        case '1': cidx = 0; break;
        case '2': cidx = 1; break;
        case '3': cidx = 2; break;
        case '4': cidx = 3; break;
        default: return sq_throwerror(v,niFmt(_A("matrix set, invalid column in member '%s'."), aaszStr));
      }
      sq_pushf32(v,vals(ridx,cidx));
    }
  }
  else if (type == OT_INTEGER)
  {
    SQInt idx;
    sq_getint(v,-1,&idx);
    if (idx < 0 || idx > 15) return sq_throwerror(v,_A("matrix get, index out of range."));
    sq_pushf32(v,vals.ptr()[idx]);
  }
  else
  {
    return sq_throwerror(v,_A("matrix get, can index matrix only with string."));
  }
  return 1;
}

///////////////////////////////////////////////
static int matrixf_isidentity(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sq_pushint(v,MatrixIsIdentity(vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_isorthogonal(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sq_pushint(v,MatrixIsOrthogonal(vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_isorthonormal(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sq_pushint(v,MatrixIsOrthoNormal(vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_isnormal(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sq_pushint(v,MatrixIsNormal(vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_determinant(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sq_pushf32(v,MatrixDeterminant(vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_inverse(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sMatrixf mtx;
  sqa_pushmatrixf(v,MatrixInverse(mtx,vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_transform_inverse(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sMatrixf mtx;
  sqa_pushmatrixf(v,MatrixTransformInverse(mtx,vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_transpose(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sMatrixf mtx;
  sqa_pushmatrixf(v,MatrixTranspose(mtx,vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_transposerot(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sMatrixf mtx;
  sqa_pushmatrixf(v,MatrixTranspose3x3(mtx,vals));
  return 1;
}

///////////////////////////////////////////////
static int matrixf_multiply(HSQUIRRELVM v)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,-2);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  sScriptTypeMatrixf* pOp = sqa_getud<sScriptTypeMatrixf>(v,-1);
  if (!pOp) return sq_throwerror(v,_A("matrixf_multiply, right operand can be only a matrix."));
  const sMatrixf& valsOp = pOp->_val;
  sMatrixf mtx;
  sqa_pushmatrixf(v,MatrixMultiply(mtx,vals,valsOp));
  return 1;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushmatrixf(HSQUIRRELVM v, const sMatrixf& aV)
{
  sScriptTypeMatrixf* ud = niNew sScriptTypeMatrixf(*v->_ss,aV);
  sq_pushud(v,ud);
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getmatrixf(HSQUIRRELVM v, int idx, sMatrixf* aV)
{
  sScriptTypeMatrixf* pV = sqa_getud<sScriptTypeMatrixf>(v,idx);
  if (!pV) return SQ_ERROR;
  const sMatrixf& vals = pV->_val;
  *aV = vals;
  return SQ_OK;
}

///////////////////////////////////////////////
static int matrixf_constructor(HSQUIRRELVM v)
{
  sMatrixf val;
  int top = sq_gettop(v)-1;
  switch (top)
  {
    case 0: {
      val = sMatrixf::Identity();
      break;
    }
    case 16: {
      for (tI32 i = 0; i < 16; ++i) {
        sq_getf32(v,-(i+1),&val[15-i]);
      }
      break;
    }
    default:
      return sq_throwerror(v,niFmt(_A("matrix constructor error, invalid number of parameters '%d'."),top));
  }
  sqa_pushmatrixf(v,val);
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
static int uuid_tostring(HSQUIRRELVM v)
{
  sScriptTypeUUID* pV = sqa_getud<sScriptTypeUUID>(v,-1);
  if (!pV) return SQ_ERROR;
  sq_pushstring(v, _H(pV->mUUID.ToString()));
  return 1;
}

///////////////////////////////////////////////
static int uuid_typeof(HSQUIRRELVM v)
{
  sq_pushstring(v, _HC(UUID));
  return 1;
}

///////////////////////////////////////////////
static int uuid_constructor(HSQUIRRELVM v)
{
  niCAssert(sizeof(tUUID) == sizeof(sUUID));
  tUUID val = kuuidZero;
  int top = sq_gettop(v)-1;
  switch (top)
  {
    case 0:
      {
        break;
      }
    case 1:
      {
        eScriptType type = sqa_getscripttype(v,-1);
        switch (type)
        {
          case eScriptType_UUID:
            {
              sqa_getUUID(v,-1,&val);
              break;
            }
          case eScriptType_String:
            {
              const achar* str;
              sq_getstring(v,-1,&str);
              if (ni::StrICmp(str,_A("local")) == 0) {
                val = ni::GetLang()->CreateLocalUUID();
              }
              else if (ni::StrICmp(str,_A("global")) == 0) {
                val = ni::GetLang()->CreateGlobalUUID();
              }
              else {
                sUUID uuidFromStr;
                uuidFromStr.FromString(str);
                val = uuidFromStr.nUUID;
              }
              break;
            }
          default:
            {
              return sq_throwerror(v,_A("uuid_constructor, copy constructor parameter invalid type."));
            }
        }
        break;
      }
    default:
      return sq_throwerror(v,niFmt(_A("uuid_constructor, invalid number of parameters '%d'."),top));
  }
  sqa_pushUUID(v,val);
  return 1;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_pushUUID(HSQUIRRELVM v, const tUUID& aV)
{
  sScriptTypeUUID* ud = niNew sScriptTypeUUID(*v->_ss,aV);
  sq_pushud(v,ud);
  return SQ_OK;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_getUUID(HSQUIRRELVM v, int idx, tUUID* aV)
{
  sScriptTypeUUID* pV = sqa_getud<sScriptTypeUUID>(v,idx);
  if (!pV) return SQ_ERROR;
  *((sUUID*)aV) = pV->mUUID;
  return SQ_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
SQRegFunction SQSharedState::_idxprop_default_delegate_funcz[]={
  {"_set",              indexedproperty_set,                0, NULL},
  {"_get",              indexedproperty_get,                0, NULL},
  {0,0}
};

SQRegFunction SQSharedState::_enum_default_delegate_funcz[]={
  {"_typeof", enum_typeof, 0, NULL},
  {"_set", enum_set, 0, NULL},
  {"_get", enum_get, 0, NULL},
  {"_delslot", enum_delslot, 0, NULL},
  {"_newslot", enum_newslot, 0, NULL},
  {"gettable", enum_gettable, 0, NULL},
  {"enumName", enum_enumName, 0, NULL},
  {"numElements", enum_numElements, 0, NULL},
  {"elementName", enum_elementName, 0, NULL},
  {"elementValue", enum_elementValue, 0, NULL},
  {0,0}
};

SQRegFunction SQSharedState::_vec2f_default_delegate_funcz[]={
  {"_tostring",         vec2f_tostring,                  1, NULL},
  {"_typeof",           vec2f_typeof,                    1, NULL},
  {"_set",              vec2f_set,                       3, NULL},
  {"_get",              vec2f_get,                       2, NULL},
  {"_add",              mathop_binary<mathop_Add>,       2, NULL},
  {"_sub",              mathop_binary<mathop_Sub>,       2, NULL},
  {"_mul",              mathop_binary<mathop_Mul>,       2, NULL},
  {"_div",              mathop_binary<mathop_Div>,       2, NULL},
  {"normalize",         vecop_normalize<sVec2f>,         1, NULL},
  {"Normalize",         vecop_normalize<sVec2f>,         1, NULL},
  {"isnormal",          vecop_isnormal<sVec2f>,          1, NULL},
  {"IsNormal",          vecop_isnormal<sVec2f>,          1, NULL},
  {"dot",               vecop_dot<sVec2f>,               2, NULL},
  {"Dot",               vecop_dot<sVec2f>,               2, NULL},
  {"length",            vecop_length<sVec2f>,            1, NULL},
  {"Length",            vecop_length<sVec2f>,            1, NULL},
  {"lengthsq",          vecop_lengthsq<sVec2f>,          1, NULL},
  {"LengthSq",          vecop_lengthsq<sVec2f>,          1, NULL},
  {"cross",             vecop_cross<sVec2f>,             2, NULL},
  {"Cross",             vecop_cross<sVec2f>,             2, NULL},
  {"abs",               vecop_abs<sVec2f>,               1, NULL},
  {"Abs",               vecop_abs<sVec2f>,               1, NULL},
  {"min",               vecop_min<sVec2f>,               2, NULL},
  {"Min",               vecop_min<sVec2f>,               2, NULL},
  {"max",               vecop_max<sVec2f>,               2, NULL},
  {"Max",               vecop_max<sVec2f>,               2, NULL},
  {"transform",         vecop_transform<sVec2f>,         2, NULL},
  {"Transform",         vecop_transform<sVec2f>,         2, NULL},
  {"transform_coord",   vecop_transform_coord<sVec2f>,   2, NULL},
  {"TransformCoord",    vecop_transform_coord<sVec2f>,   2, NULL},
  {"transform_normal",  vecop_transform_normal<sVec2f>,  2, NULL},
  {"TransformNormal",   vecop_transform_normal<sVec2f>,  2, NULL},
  {"_modulo",           mathop_binary<mathop_Mod>,       1, NULL},
  {"_unm",              mathop_unary<mathop_Neg>,        1, NULL},
  {0,0}
};

SQRegFunction SQSharedState::_vec3f_default_delegate_funcz[]={
  {"_tostring",         vec3f_tostring,                  1, NULL},
  {"_typeof",           vec3f_typeof,                    1, NULL},
  {"_set",              vec3f_set,                       3, NULL},
  {"_get",              vec3f_get,                       2, NULL},
  {"_add",              mathop_binary<mathop_Add>,       2, NULL},
  {"_sub",              mathop_binary<mathop_Sub>,       2, NULL},
  {"_mul",              mathop_binary<mathop_Mul>,       2, NULL},
  {"_div",              mathop_binary<mathop_Div>,       2, NULL},
  {"normalize",         vecop_normalize<sVec3f>,         1, NULL},
  {"Normalize",         vecop_normalize<sVec3f>,         1, NULL},
  {"isnormal",          vecop_isnormal<sVec3f>,          1, NULL},
  {"IsNormal",          vecop_isnormal<sVec3f>,          1, NULL},
  {"dot",               vecop_dot<sVec3f>,               2, NULL},
  {"Dot",               vecop_dot<sVec3f>,               2, NULL},
  {"length",            vecop_length<sVec3f>,            1, NULL},
  {"Length",            vecop_length<sVec3f>,            1, NULL},
  {"lengthsq",          vecop_lengthsq<sVec3f>,          1, NULL},
  {"LengthSq",          vecop_lengthsq<sVec3f>,          1, NULL},
  {"cross",             vecop_cross<sVec3f>,             2, NULL},
  {"Cross",             vecop_cross<sVec3f>,             2, NULL},
  {"abs",               vecop_abs<sVec3f>,               1, NULL},
  {"Abs",               vecop_abs<sVec3f>,               1, NULL},
  {"min",               vecop_min<sVec3f>,               2, NULL},
  {"Min",               vecop_min<sVec3f>,               2, NULL},
  {"max",               vecop_max<sVec3f>,               2, NULL},
  {"Max",               vecop_max<sVec3f>,               2, NULL},
  {"transform",         vecop_transform<sVec3f>,         2, NULL},
  {"Transform",         vecop_transform<sVec3f>,         2, NULL},
  {"transform_coord",   vecop_transform_coord<sVec3f>,   2, NULL},
  {"TransformCoord",    vecop_transform_coord<sVec3f>,   2, NULL},
  {"transform_normal",  vecop_transform_normal<sVec3f>,  2, NULL},
  {"TransformNormal",   vecop_transform_normal<sVec3f>,  2, NULL},
  {"_modulo",           mathop_binary<mathop_Mod>,       1, NULL},
  {"_unm",              mathop_unary<mathop_Neg>,        1, NULL},
  {"toint",             vec3f_toint,                     1, NULL},
  {"ToInt",             vec3f_toint,                     1, NULL},
  {0,0}
};

SQRegFunction SQSharedState::_vec4f_default_delegate_funcz[]={
  {"_tostring",            vec4f_tostring,                  1,  NULL},
  {"_typeof",              vec4f_typeof,                    1,  NULL},
  {"_set",                 vec4f_set,                       3,  NULL},
  {"_get",                 vec4f_get,                       2,  NULL},
  {"_add",                 mathop_binary<mathop_Add>,       2,  NULL},
  {"_sub",                 mathop_binary<mathop_Sub>,       2,  NULL},
  {"_mul",                 mathop_binary<mathop_Mul>,       2,  NULL},
  {"_div",                 mathop_binary<mathop_Div>,       2,  NULL},
  {"Normalize",            vecop_normalize<sVec4f>,         1,  NULL},
  {"normalize",            vecop_normalize<sVec4f>,         1,  NULL},
  {"isnormal",             vecop_isnormal<sVec4f>,          1,  NULL},
  {"IsNormal",             vecop_isnormal<sVec4f>,          1,  NULL},
  {"dot",                  vecop_dot<sVec4f>,               2,  NULL},
  {"Dot",                  vecop_dot<sVec4f>,               2,  NULL},
  {"length",               vecop_length<sVec4f>,            1,  NULL},
  {"Length",               vecop_length<sVec4f>,            1,  NULL},
  {"lengthsq",             vecop_lengthsq<sVec4f>,          1,  NULL},
  {"LengthSq",             vecop_lengthsq<sVec4f>,          1,  NULL},
  {"cross",                vecop_cross<sVec4f>,             2,  NULL},
  {"Cross",                vecop_cross<sVec4f>,             2,  NULL},
  {"abs",                  vecop_abs<sVec4f>,               1,  NULL},
  {"Abs",                  vecop_abs<sVec4f>,               1,  NULL},
  {"min",                  vecop_min<sVec4f>,               2,  NULL},
  {"Min",                  vecop_min<sVec4f>,               2,  NULL},
  {"max",                  vecop_max<sVec4f>,               2,  NULL},
  {"Max",                  vecop_max<sVec4f>,               2,  NULL},
  {"transform",            vecop_transform<sVec4f>,         2,  NULL},
  {"Transform",            vecop_transform<sVec4f>,         2,  NULL},
  {"transform_coord",      vecop_transform_coord<sVec4f>,   2,  NULL},
  {"TransformCoord",       vecop_transform_coord<sVec4f>,   2,  NULL},
  {"transform_normal",     vecop_transform_normal<sVec4f>,  2,  NULL},
  {"TransformNormal",      vecop_transform_normal<sVec4f>,  2,  NULL},
  {"_modulo",              mathop_binary<mathop_Mod>,       1,  NULL},
  {"_unm",                 mathop_unary<mathop_Neg>,        1,  NULL},
  {"toint",                vec4f_toint,                     1,  NULL},
  {"ToInt",                vec4f_toint,                     1,  NULL},
  {"intersect",            rect_intersect,                  2,  NULL},
  {"Intersect",            rect_intersect,                  2,  NULL},
  {"frame_center",         rect_frameCenter,                5,  NULL},
  {"ComputeFrameCenter",   rect_frameCenter,                5,  NULL},
  {"frame_left_border",    rect_frameLeftBorder,            2,  NULL},
  {"GetFrameLeftBorder",   rect_frameLeftBorder,            2,  NULL},
  {"frame_right_border",   rect_frameRightBorder,           2,  NULL},
  {"GetFrameRightBorder",  rect_frameRightBorder,           2,  NULL},
  {"frame_top_border",     rect_frameTopBorder,             2,  NULL},
  {"GetFrameTopBorder",    rect_frameTopBorder,             2,  NULL},
  {"frame_bottom_border",  rect_frameBottomBorder,          2,  NULL},
  {"GetFrameBottomBorder", rect_frameBottomBorder,          2,  NULL},
  {"frame_border",         rect_frameBorder,                2,  NULL},
  {"GetFrameBorder",       rect_frameBorder,                2,  NULL},
  {0,0}
};

SQRegFunction SQSharedState::_matrixf_default_delegate_funcz[] = {
  {"_tostring",          matrixf_tostring,            1,  NULL},
  {"_typeof",            matrixf_typeof,              1,  NULL},
  {"_set",               matrixf_set,                 3,  NULL},
  {"_get",               matrixf_get,                 2,  NULL},
  {"_add",               mathop_binary<mathop_Add>,   2,  NULL},
  {"_sub",               mathop_binary<mathop_Sub>,   2,  NULL},
  {"_mul",               matrixf_multiply,            2,  NULL},
  {"_modulo",            mathop_binary<mathop_Mod>,   2,  NULL},
  {"isidentity",         matrixf_isidentity,          1,  NULL},
  {"IsIdentity",         matrixf_isidentity,          1,  NULL},
  {"isnormal",           matrixf_isnormal,            1,  NULL},
  {"IsNormal",           matrixf_isnormal,            1,  NULL},
  {"isorthonormal",      matrixf_isorthonormal,       1,  NULL},
  {"IsOrthonormal",      matrixf_isorthonormal,       1,  NULL},
  {"isorthogonal",       matrixf_isorthogonal,        1,  NULL},
  {"IsOrthogonal",       matrixf_isorthogonal,        1,  NULL},
  {"determinant",        matrixf_determinant,         1,  NULL},
  {"Determinant",        matrixf_determinant,         1,  NULL},
  {"inverse",            matrixf_inverse,             1,  NULL},
  {"Inverse",            matrixf_inverse,             1,  NULL},
  {"transform_inverse",  matrixf_transform_inverse,   1,  NULL},
  {"TransformInverse",   matrixf_transform_inverse,   1,  NULL},
  {"transpose",          matrixf_transpose,           1,  NULL},
  {"Transpose",          matrixf_transpose,           1,  NULL},
  {"transposerot",       matrixf_transposerot,        1,  NULL},
  {"TransposeRot",       matrixf_transposerot,        1,  NULL},
  {"multiply",           matrixf_multiply,            2,  NULL},
  {"Multiply",           matrixf_multiply,            2,  NULL},
  {"_unm",               mathop_unary<mathop_Neg>,    1,  NULL},
  {0,0}
};

SQRegFunction SQSharedState::_uuid_default_delegate_funcz[]={
  {"_tostring", uuid_tostring, 1, NULL},
  {"_typeof", uuid_typeof, 1, NULL},
  {0,0}
};

///////////////////////////////////////////////
niExportFunc(void*) sqa_getud(HSQUIRRELVM v, int idx, int type) {
  SQUserData* ud;
  if (!SQ_SUCCEEDED(sq_getuserdata(v,idx,&ud))) return NULL;
  if (ud->GetType() != type) return NULL;
  return (void*)ud;
}

///////////////////////////////////////////////
niExportFunc(int) sqa_registernewtypes(HSQUIRRELVM v)
{
  sqa_registerglobalfunction(v, _A("Vec2"),   vec2f_constructor);
  sqa_registerglobalfunction(v, _A("Vec3"),   vec3f_constructor);
  sqa_registerglobalfunction(v, _A("Vec4"),   vec4f_constructor);
  sqa_registerglobalfunction(v, _A("RGB"),    vec3f_constructor);
  sqa_registerglobalfunction(v, _A("RGBA"),   vec4f_constructor);
  sqa_registerglobalfunction(v, _A("Quat"),   vec4f_constructor);
  sqa_registerglobalfunction(v, _A("Plane"),  vec4f_constructor);
  sqa_registerglobalfunction(v, _A("Rect"),   rect_constructor);
  sqa_registerglobalfunction(v, _A("Matrix"), matrixf_constructor);
  sqa_registerglobalfunction(v, _A("UUID"),   uuid_constructor);
  return SQ_OK;
}

#endif // niMinFeatures
