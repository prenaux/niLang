#include "../API/niUI/IDrawOperationSet.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iDrawOperationSet **/
IDLC_BEGIN_INTF(ni,iDrawOperationSet)
/** ni -> iDrawOperationSet::Clear/0 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,Clear,0)
	IDLC_METH_CALL_VOID(ni,iDrawOperationSet,Clear,0,())
IDLC_METH_END(ni,iDrawOperationSet,Clear,0)

/** ni -> iDrawOperationSet::Insert/1 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,Insert,1)
	IDLC_DECL_VAR(iDrawOperation*,apDO)
	IDLC_BUF_TO_INTF(iDrawOperation,apDO)
	IDLC_DECL_RETVAR(iDrawOperation*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,Insert,1,(apDO))
	IDLC_RET_FROM_INTF(iDrawOperation,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,Insert,1)

/** ni -> iDrawOperationSet::InsertSet/1 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,InsertSet,1)
	IDLC_DECL_VAR(iDrawOperationSet*,apSet)
	IDLC_BUF_TO_INTF(iDrawOperationSet,apSet)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,InsertSet,1,(apSet))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,InsertSet,1)

/** ni -> iDrawOperationSet::GetNumDrawOperations/0 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,GetNumDrawOperations,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,GetNumDrawOperations,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,GetNumDrawOperations,0)

/** ni -> iDrawOperationSet::Begin/0 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,Begin,0)
	IDLC_DECL_RETVAR(iDrawOperation*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,Begin,0,())
	IDLC_RET_FROM_INTF(iDrawOperation,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,Begin,0)

/** ni -> iDrawOperationSet::Next/0 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,Next,0)
	IDLC_DECL_RETVAR(iDrawOperation*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,Next,0,())
	IDLC_RET_FROM_INTF(iDrawOperation,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,Next,0)

/** ni -> iDrawOperationSet::IsEnd/0 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,IsEnd,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,IsEnd,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,IsEnd,0)

/** ni -> iDrawOperationSet::GetCurrent/0 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,GetCurrent,0)
	IDLC_DECL_RETVAR(iDrawOperation*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,GetCurrent,0,())
	IDLC_RET_FROM_INTF(iDrawOperation,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,GetCurrent,0)

/** ni -> iDrawOperationSet::GetIsEmpty/0 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,GetIsEmpty,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,GetIsEmpty,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,GetIsEmpty,0)

/** ni -> iDrawOperationSet::Draw/2 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,Draw,2)
	IDLC_DECL_VAR(iGraphicsContext*,apContext)
	IDLC_BUF_TO_INTF(iGraphicsContext,apContext)
	IDLC_DECL_VAR(iFrustum*,apFrustum)
	IDLC_BUF_TO_INTF(iFrustum,apFrustum)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,Draw,2,(apContext,apFrustum))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,Draw,2)

/** ni -> iDrawOperationSet::XDraw/3 **/
IDLC_METH_BEGIN(ni,iDrawOperationSet,XDraw,3)
	IDLC_DECL_VAR(sMatrixf,aMatrix)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Matrixf|ni::eTypeFlags_Pointer,aMatrix)
	IDLC_DECL_VAR(iGraphicsContext*,apContext)
	IDLC_BUF_TO_INTF(iGraphicsContext,apContext)
	IDLC_DECL_VAR(iFrustum*,apFrustum)
	IDLC_BUF_TO_INTF(iFrustum,apFrustum)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iDrawOperationSet,XDraw,3,(aMatrix,apContext,apFrustum))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iDrawOperationSet,XDraw,3)

IDLC_END_INTF(ni,iDrawOperationSet)

IDLC_END_NAMESPACE()
// EOF //
