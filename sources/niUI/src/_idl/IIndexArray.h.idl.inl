#include "../API/niUI/IIndexArray.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iIndexArray **/
IDLC_BEGIN_INTF(ni,iIndexArray)
/** ni -> iIndexArray::GetPrimitiveType/0 **/
IDLC_METH_BEGIN(ni,iIndexArray,GetPrimitiveType,0)
	IDLC_DECL_RETVAR(eGraphicsPrimitiveType,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIndexArray,GetPrimitiveType,0,())
	IDLC_RET_FROM_ENUM(eGraphicsPrimitiveType,_Ret)
IDLC_METH_END(ni,iIndexArray,GetPrimitiveType,0)

/** ni -> iIndexArray::GetNumIndices/0 **/
IDLC_METH_BEGIN(ni,iIndexArray,GetNumIndices,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIndexArray,GetNumIndices,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iIndexArray,GetNumIndices,0)

/** ni -> iIndexArray::GetMaxVertexIndex/0 **/
IDLC_METH_BEGIN(ni,iIndexArray,GetMaxVertexIndex,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIndexArray,GetMaxVertexIndex,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iIndexArray,GetMaxVertexIndex,0)

/** ni -> iIndexArray::GetUsage/0 **/
IDLC_METH_BEGIN(ni,iIndexArray,GetUsage,0)
	IDLC_DECL_RETVAR(eArrayUsage,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIndexArray,GetUsage,0,())
	IDLC_RET_FROM_ENUM(eArrayUsage,_Ret)
IDLC_METH_END(ni,iIndexArray,GetUsage,0)

/** ni -> iIndexArray::Lock/3 **/
IDLC_METH_BEGIN(ni,iIndexArray,Lock,3)
	IDLC_DECL_VAR(tU32,ulFirstIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,ulFirstIndex)
	IDLC_DECL_VAR(tU32,ulNumIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,ulNumIndex)
	IDLC_DECL_VAR(eLock,aLock)
	IDLC_BUF_TO_ENUM(eLock,aLock)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIndexArray,Lock,3,(ulFirstIndex,ulNumIndex,aLock))
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iIndexArray,Lock,3)

/** ni -> iIndexArray::Unlock/0 **/
IDLC_METH_BEGIN(ni,iIndexArray,Unlock,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIndexArray,Unlock,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iIndexArray,Unlock,0)

/** ni -> iIndexArray::GetIsLocked/0 **/
IDLC_METH_BEGIN(ni,iIndexArray,GetIsLocked,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iIndexArray,GetIsLocked,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iIndexArray,GetIsLocked,0)

IDLC_END_INTF(ni,iIndexArray)

IDLC_END_NAMESPACE()
// EOF //
