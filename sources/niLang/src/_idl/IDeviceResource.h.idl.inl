// clang-format off
//
// Autogenerated IDL definition for 'API/niLang/IDeviceResource.h'.
//
#include "../API/niLang/IDeviceResource.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iDeviceResource **/
IDLC_BEGIN_INTF(ni,iDeviceResource)
/** ni -> iDeviceResource::GetDeviceResourceName/0 **/
IDLC_METH_BEGIN(ni,iDeviceResource,GetDeviceResourceName,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResource,GetDeviceResourceName,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iDeviceResource,GetDeviceResourceName,0)

/** ni -> iDeviceResource::HasDeviceResourceBeenReset/1 **/
IDLC_METH_BEGIN(ni,iDeviceResource,HasDeviceResourceBeenReset,1)
	IDLC_DECL_VAR(tBool,abClearFlag)
	IDLC_BUF_TO_BASE(ni::eType_I8,abClearFlag)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResource,HasDeviceResourceBeenReset,1,(abClearFlag))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDeviceResource,HasDeviceResourceBeenReset,1)

/** ni -> iDeviceResource::ResetDeviceResource/0 **/
IDLC_METH_BEGIN(ni,iDeviceResource,ResetDeviceResource,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResource,ResetDeviceResource,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDeviceResource,ResetDeviceResource,0)

/** ni -> iDeviceResource::Bind/1 **/
IDLC_METH_BEGIN(ni,iDeviceResource,Bind,1)
	IDLC_DECL_VAR(iUnknown*,apDevice)
	IDLC_BUF_TO_INTF(iUnknown,apDevice)
	IDLC_DECL_RETVAR(iDeviceResource*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResource,Bind,1,(apDevice))
	IDLC_RET_FROM_INTF(iDeviceResource,_Ret)
IDLC_METH_END(ni,iDeviceResource,Bind,1)

IDLC_END_INTF(ni,iDeviceResource)

/** interface : iDeviceResourceManager **/
IDLC_BEGIN_INTF(ni,iDeviceResourceManager)
/** ni -> iDeviceResourceManager::GetType/0 **/
IDLC_METH_BEGIN(ni,iDeviceResourceManager,GetType,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResourceManager,GetType,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iDeviceResourceManager,GetType,0)

/** ni -> iDeviceResourceManager::Clear/0 **/
IDLC_METH_BEGIN(ni,iDeviceResourceManager,Clear,0)
	IDLC_METH_CALL_VOID(ni,iDeviceResourceManager,Clear,0,())
IDLC_METH_END(ni,iDeviceResourceManager,Clear,0)

/** ni -> iDeviceResourceManager::GetSize/0 **/
IDLC_METH_BEGIN(ni,iDeviceResourceManager,GetSize,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResourceManager,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDeviceResourceManager,GetSize,0)

/** ni -> iDeviceResourceManager::GetFromName/1 **/
IDLC_METH_BEGIN(ni,iDeviceResourceManager,GetFromName,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(iDeviceResource*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResourceManager,GetFromName,1,(ahspName))
	IDLC_RET_FROM_INTF(iDeviceResource,_Ret)
IDLC_METH_END(ni,iDeviceResourceManager,GetFromName,1)

/** ni -> iDeviceResourceManager::GetFromIndex/1 **/
IDLC_METH_BEGIN(ni,iDeviceResourceManager,GetFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iDeviceResource*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResourceManager,GetFromIndex,1,(anIndex))
	IDLC_RET_FROM_INTF(iDeviceResource,_Ret)
IDLC_METH_END(ni,iDeviceResourceManager,GetFromIndex,1)

/** ni -> iDeviceResourceManager::Register/1 **/
IDLC_METH_BEGIN(ni,iDeviceResourceManager,Register,1)
	IDLC_DECL_VAR(iDeviceResource*,apRes)
	IDLC_BUF_TO_INTF(iDeviceResource,apRes)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResourceManager,Register,1,(apRes))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDeviceResourceManager,Register,1)

/** ni -> iDeviceResourceManager::Unregister/1 **/
IDLC_METH_BEGIN(ni,iDeviceResourceManager,Unregister,1)
	IDLC_DECL_VAR(iDeviceResource*,apRes)
	IDLC_BUF_TO_INTF(iDeviceResource,apRes)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDeviceResourceManager,Unregister,1,(apRes))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDeviceResourceManager,Unregister,1)

IDLC_END_INTF(ni,iDeviceResourceManager)

IDLC_END_NAMESPACE()
// EOF //
