// clang-format off
//
// Autogenerated IDL definition for 'API/niScript/IScriptObject.h'.
//
#include "../API/niScript/IScriptObject.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iScriptObject **/
/** iScriptObject -> MIN FEATURES '15' **/
#if niMinFeatures(15)
IDLC_BEGIN_INTF(ni,iScriptObject)
/** ni -> iScriptObject::GetVM/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetVM/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetVM,0)
	IDLC_DECL_RETVAR(iScriptVM*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetVM,0,())
	IDLC_RET_FROM_INTF(iScriptVM,_Ret)
IDLC_METH_END(ni,iScriptObject,GetVM,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetType/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetType/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetType,0)
	IDLC_DECL_RETVAR(eScriptObjectType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetType,0,())
	IDLC_RET_FROM_ENUM(eScriptObjectType,_Ret)
IDLC_METH_END(ni,iScriptObject,GetType,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::Clone/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::Clone/1 **/
IDLC_METH_BEGIN(ni,iScriptObject,Clone,1)
	IDLC_DECL_VAR(tBool,abHoldRef)
	IDLC_BUF_TO_BASE(ni::eType_I8,abHoldRef)
	IDLC_DECL_RETVAR(iScriptObject*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,Clone,1,(abHoldRef))
	IDLC_RET_FROM_INTF(iScriptObject,_Ret)
IDLC_METH_END(ni,iScriptObject,Clone,1)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::EnumObjects/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::EnumObjects/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,EnumObjects,0)
	IDLC_DECL_RETVAR(tScriptObjectPtrCVec*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,EnumObjects,0,())
	IDLC_RET_FROM_INTF(tScriptObjectPtrCVec,_Ret)
IDLC_METH_END(ni,iScriptObject,EnumObjects,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetInt/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetInt/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetInt,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetInt,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iScriptObject,GetInt,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetFloat/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetFloat/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetFloat,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetFloat,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iScriptObject,GetFloat,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetString/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetString/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetString,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetString,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iScriptObject,GetString,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetHString/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetHString/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetHString,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetHString,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iScriptObject,GetHString,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetIUnknown/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetIUnknown/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetIUnknown,0)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetIUnknown,0,())
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iScriptObject,GetIUnknown,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetIUnknownEx/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetIUnknownEx/1 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetIUnknownEx,1)
	IDLC_DECL_VAR(tUUID,aIID)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_UUID|ni::eTypeFlags_Pointer,aIID)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetIUnknownEx,1,(aIID))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iScriptObject,GetIUnknownEx,1)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetVec2/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetVec2/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetVec2,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetVec2,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iScriptObject,GetVec2,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetVec3/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetVec3/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetVec3,0)
	IDLC_DECL_RETVAR(sVec3f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetVec3,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec3f,_Ret)
IDLC_METH_END(ni,iScriptObject,GetVec3,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetVec4/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetVec4/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetVec4,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetVec4,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iScriptObject,GetVec4,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetMatrix/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetMatrix/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetMatrix,0)
	IDLC_DECL_RETVAR(sMatrixf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetMatrix,0,())
	IDLC_RET_FROM_BASE(ni::eType_Matrixf,_Ret)
IDLC_METH_END(ni,iScriptObject,GetMatrix,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetUUID/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetUUID/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetUUID,0)
	IDLC_DECL_RETREFVAR(tUUID&,_Ret,tUUID)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetUUID,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_UUID|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iScriptObject,GetUUID,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetNumParameters/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetNumParameters/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetNumParameters,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetNumParameters,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iScriptObject,GetNumParameters,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetNumFreeVars/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetNumFreeVars/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetNumFreeVars,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetNumFreeVars,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iScriptObject,GetNumFreeVars,0)
#endif // niMinFeatures(15)

/** ni -> iScriptObject::GetNumCallParameters/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iScriptObject::GetNumCallParameters/0 **/
IDLC_METH_BEGIN(ni,iScriptObject,GetNumCallParameters,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iScriptObject,GetNumCallParameters,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iScriptObject,GetNumCallParameters,0)
#endif // niMinFeatures(15)

IDLC_END_INTF(ni,iScriptObject)

#endif // if niMinFeatures(15)
IDLC_END_NAMESPACE()
// EOF //
