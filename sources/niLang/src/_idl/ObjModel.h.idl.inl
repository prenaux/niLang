#include "../API/niLang/ObjModel.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iObjectTypeDef **/
IDLC_BEGIN_INTF(ni,iObjectTypeDef)
/** ni -> iObjectTypeDef::GetName/0 **/
IDLC_METH_BEGIN(ni,iObjectTypeDef,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iObjectTypeDef,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iObjectTypeDef,GetName,0)

/** ni -> iObjectTypeDef::CreateInstance/2 **/
IDLC_METH_BEGIN(ni,iObjectTypeDef,CreateInstance,2)
	IDLC_DECL_VAR(Var,aVarA)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVarA)
	IDLC_DECL_VAR(Var,aVarB)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVarB)
	IDLC_DECL_RETVAR(iUnknown*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iObjectTypeDef,CreateInstance,2,(aVarA,aVarB))
	IDLC_RET_FROM_INTF(iUnknown,_Ret)
IDLC_METH_END(ni,iObjectTypeDef,CreateInstance,2)

IDLC_END_INTF(ni,iObjectTypeDef)

/** interface : iModuleDef **/
IDLC_BEGIN_INTF(ni,iModuleDef)
/** ni -> iModuleDef::GetName/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iModuleDef,GetName,0)

/** ni -> iModuleDef::GetVersion/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetVersion,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetVersion,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iModuleDef,GetVersion,0)

/** ni -> iModuleDef::GetDesc/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetDesc,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetDesc,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iModuleDef,GetDesc,0)

/** ni -> iModuleDef::GetAuthor/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetAuthor,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetAuthor,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iModuleDef,GetAuthor,0)

/** ni -> iModuleDef::GetCopyright/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetCopyright,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetCopyright,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iModuleDef,GetCopyright,0)

/** ni -> iModuleDef::GetNumDependencies/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetNumDependencies,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetNumDependencies,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_U32,_Ret)
IDLC_METH_END(ni,iModuleDef,GetNumDependencies,0)

/** ni -> iModuleDef::GetDependency/1 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetDependency,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetDependency,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iModuleDef,GetDependency,1)

/** ni -> iModuleDef::GetNumInterfaces/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetNumInterfaces,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetNumInterfaces,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_U32,_Ret)
IDLC_METH_END(ni,iModuleDef,GetNumInterfaces,0)

/** ni -> iModuleDef::GetInterface/1 -> NO AUTOMATION **/
/** ni -> iModuleDef::GetNumEnums/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetNumEnums,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetNumEnums,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_U32,_Ret)
IDLC_METH_END(ni,iModuleDef,GetNumEnums,0)

/** ni -> iModuleDef::GetEnum/1 -> NO AUTOMATION **/
/** ni -> iModuleDef::GetNumConstants/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetNumConstants,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetNumConstants,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_U32,_Ret)
IDLC_METH_END(ni,iModuleDef,GetNumConstants,0)

/** ni -> iModuleDef::GetConstant/1 -> NO AUTOMATION **/
/** ni -> iModuleDef::GetNumObjectTypes/0 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetNumObjectTypes,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetNumObjectTypes,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_U32,_Ret)
IDLC_METH_END(ni,iModuleDef,GetNumObjectTypes,0)

/** ni -> iModuleDef::GetObjectType/1 **/
IDLC_METH_BEGIN(ni,iModuleDef,GetObjectType,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(const iObjectTypeDef*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iModuleDef,GetObjectType,1,(anIndex))
	IDLC_RET_FROM_INTF(const iObjectTypeDef,_Ret)
IDLC_METH_END(ni,iModuleDef,GetObjectType,1)

IDLC_END_INTF(ni,iModuleDef)

IDLC_END_NAMESPACE()
// EOF //
