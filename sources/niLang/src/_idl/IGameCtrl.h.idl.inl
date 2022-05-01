#include "../API/niLang/IGameCtrl.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iGameCtrl **/
IDLC_BEGIN_INTF(ni,iGameCtrl)
/** ni -> iGameCtrl::Update/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,Update,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,Update,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGameCtrl,Update,0)

/** ni -> iGameCtrl::GetIsConnected/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetIsConnected,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetIsConnected,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetIsConnected,0)

/** ni -> iGameCtrl::GetName/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetName,0)

/** ni -> iGameCtrl::GetIndex/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetIndex,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetIndex,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetIndex,0)

/** ni -> iGameCtrl::GetNumButtons/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetNumButtons,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetNumButtons,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetNumButtons,0)

/** ni -> iGameCtrl::GetButton/1 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetButton,1)
	IDLC_DECL_VAR(tU32,ulButton)
	IDLC_BUF_TO_BASE(ni::eType_U32,ulButton)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetButton,1,(ulButton))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetButton,1)

/** ni -> iGameCtrl::GetNumAxis/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetNumAxis,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetNumAxis,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetNumAxis,0)

/** ni -> iGameCtrl::GetAxis/1 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetAxis,1)
	IDLC_DECL_VAR(eGameCtrlAxis,axis)
	IDLC_BUF_TO_ENUM(eGameCtrlAxis,axis)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetAxis,1,(axis))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetAxis,1)

/** ni -> iGameCtrl::GetCanVibrate/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetCanVibrate,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetCanVibrate,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetCanVibrate,0)

/** ni -> iGameCtrl::Vibrate/1 **/
IDLC_METH_BEGIN(ni,iGameCtrl,Vibrate,1)
	IDLC_DECL_VAR(sVec2f,aSpeed)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,aSpeed)
	IDLC_METH_CALL_VOID(ni,iGameCtrl,Vibrate,1,(aSpeed))
IDLC_METH_END(ni,iGameCtrl,Vibrate,1)

/** ni -> iGameCtrl::GetHasBattery/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetHasBattery,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetHasBattery,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetHasBattery,0)

/** ni -> iGameCtrl::GetBatteryLevel/0 **/
IDLC_METH_BEGIN(ni,iGameCtrl,GetBatteryLevel,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iGameCtrl,GetBatteryLevel,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iGameCtrl,GetBatteryLevel,0)

IDLC_END_INTF(ni,iGameCtrl)

IDLC_END_NAMESPACE()
// EOF //
