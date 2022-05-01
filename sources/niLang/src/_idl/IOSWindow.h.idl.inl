#include "../API/niLang/IOSWindow.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iOSGraphicsAPI **/
IDLC_BEGIN_INTF(ni,iOSGraphicsAPI)
/** ni -> iOSGraphicsAPI::GetName/0 **/
IDLC_METH_BEGIN(ni,iOSGraphicsAPI,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSGraphicsAPI,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iOSGraphicsAPI,GetName,0)

IDLC_END_INTF(ni,iOSGraphicsAPI)

/** interface : iOSWindow **/
IDLC_BEGIN_INTF(ni,iOSWindow)
/** ni -> iOSWindow::GetParent/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetParent,0)
	IDLC_DECL_RETVAR(iOSWindow*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetParent,0,())
	IDLC_RET_FROM_INTF(iOSWindow,_Ret)
IDLC_METH_END(ni,iOSWindow,GetParent,0)

/** ni -> iOSWindow::SetClientAreaWindow/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetClientAreaWindow,1)
	IDLC_DECL_VAR(tIntPtr,aHandle)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aHandle)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetClientAreaWindow,1,(aHandle))
IDLC_METH_END(ni,iOSWindow,SetClientAreaWindow,1)

/** ni -> iOSWindow::GetClientAreaWindow/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetClientAreaWindow,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetClientAreaWindow,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iOSWindow,GetClientAreaWindow,0)

/** ni -> iOSWindow::GetPID/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetPID,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetPID,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iOSWindow,GetPID,0)

/** ni -> iOSWindow::GetHandle/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetHandle,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetHandle,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iOSWindow,GetHandle,0)

/** ni -> iOSWindow::GetIsHandleOwned/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetIsHandleOwned,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetIsHandleOwned,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetIsHandleOwned,0)

/** ni -> iOSWindow::ActivateWindow/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,ActivateWindow,0)
	IDLC_METH_CALL_VOID(ni,iOSWindow,ActivateWindow,0,())
IDLC_METH_END(ni,iOSWindow,ActivateWindow,0)

/** ni -> iOSWindow::GetIsActive/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetIsActive,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetIsActive,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetIsActive,0)

/** ni -> iOSWindow::SwitchIn/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SwitchIn,1)
	IDLC_DECL_VAR(tU32,anReason)
	IDLC_BUF_TO_BASE(ni::eType_U32,anReason)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,SwitchIn,1,(anReason))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,SwitchIn,1)

/** ni -> iOSWindow::SwitchOut/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SwitchOut,1)
	IDLC_DECL_VAR(tU32,anReason)
	IDLC_BUF_TO_BASE(ni::eType_U32,anReason)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,SwitchOut,1,(anReason))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,SwitchOut,1)

/** ni -> iOSWindow::SetTitle/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetTitle,1)
	IDLC_DECL_VAR(achar*,aaszTitle)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszTitle)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetTitle,1,(aaszTitle))
IDLC_METH_END(ni,iOSWindow,SetTitle,1)

/** ni -> iOSWindow::GetTitle/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetTitle,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetTitle,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iOSWindow,GetTitle,0)

/** ni -> iOSWindow::SetStyle/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetStyle,1)
	IDLC_DECL_VAR(tOSWindowStyleFlags,aStyle)
	IDLC_BUF_TO_ENUM(tOSWindowStyleFlags,aStyle)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetStyle,1,(aStyle))
IDLC_METH_END(ni,iOSWindow,SetStyle,1)

/** ni -> iOSWindow::GetStyle/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetStyle,0)
	IDLC_DECL_RETVAR(tOSWindowStyleFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetStyle,0,())
	IDLC_RET_FROM_ENUM(tOSWindowStyleFlags,_Ret)
IDLC_METH_END(ni,iOSWindow,GetStyle,0)

/** ni -> iOSWindow::SetShow/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetShow,1)
	IDLC_DECL_VAR(tOSWindowShowFlags,aStyle)
	IDLC_BUF_TO_ENUM(tOSWindowShowFlags,aStyle)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetShow,1,(aStyle))
IDLC_METH_END(ni,iOSWindow,SetShow,1)

/** ni -> iOSWindow::GetShow/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetShow,0)
	IDLC_DECL_RETVAR(tOSWindowShowFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetShow,0,())
	IDLC_RET_FROM_ENUM(tOSWindowShowFlags,_Ret)
IDLC_METH_END(ni,iOSWindow,GetShow,0)

/** ni -> iOSWindow::SetZOrder/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetZOrder,1)
	IDLC_DECL_VAR(eOSWindowZOrder,aZOrder)
	IDLC_BUF_TO_ENUM(eOSWindowZOrder,aZOrder)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetZOrder,1,(aZOrder))
IDLC_METH_END(ni,iOSWindow,SetZOrder,1)

/** ni -> iOSWindow::GetZOrder/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetZOrder,0)
	IDLC_DECL_RETVAR(eOSWindowZOrder,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetZOrder,0,())
	IDLC_RET_FROM_ENUM(eOSWindowZOrder,_Ret)
IDLC_METH_END(ni,iOSWindow,GetZOrder,0)

/** ni -> iOSWindow::SetSize/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetSize,1)
	IDLC_DECL_VAR(sVec2i,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avSize)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetSize,1,(avSize))
IDLC_METH_END(ni,iOSWindow,SetSize,1)

/** ni -> iOSWindow::GetSize/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetSize,0)
	IDLC_DECL_RETVAR(sVec2i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2i,_Ret)
IDLC_METH_END(ni,iOSWindow,GetSize,0)

/** ni -> iOSWindow::SetPosition/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetPosition,1)
	IDLC_DECL_VAR(sVec2i,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetPosition,1,(avPos))
IDLC_METH_END(ni,iOSWindow,SetPosition,1)

/** ni -> iOSWindow::GetPosition/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetPosition,0)
	IDLC_DECL_RETVAR(sVec2i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2i,_Ret)
IDLC_METH_END(ni,iOSWindow,GetPosition,0)

/** ni -> iOSWindow::SetRect/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetRect,1)
	IDLC_DECL_VAR(sRecti,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4i|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetRect,1,(aRect))
IDLC_METH_END(ni,iOSWindow,SetRect,1)

/** ni -> iOSWindow::GetRect/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetRect,0)
	IDLC_DECL_RETVAR(sRecti,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4i,_Ret)
IDLC_METH_END(ni,iOSWindow,GetRect,0)

/** ni -> iOSWindow::SetClientSize/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetClientSize,1)
	IDLC_DECL_VAR(sVec2i,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avSize)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetClientSize,1,(avSize))
IDLC_METH_END(ni,iOSWindow,SetClientSize,1)

/** ni -> iOSWindow::GetClientSize/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetClientSize,0)
	IDLC_DECL_RETVAR(sVec2i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetClientSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2i,_Ret)
IDLC_METH_END(ni,iOSWindow,GetClientSize,0)

/** ni -> iOSWindow::GetContentsScale/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetContentsScale,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetContentsScale,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iOSWindow,GetContentsScale,0)

/** ni -> iOSWindow::GetMessageHandlers/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetMessageHandlers,0)
	IDLC_DECL_RETVAR(tMessageHandlerSinkLst*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetMessageHandlers,0,())
	IDLC_RET_FROM_INTF(tMessageHandlerSinkLst,_Ret)
IDLC_METH_END(ni,iOSWindow,GetMessageHandlers,0)

/** ni -> iOSWindow::UpdateWindow/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,UpdateWindow,1)
	IDLC_DECL_VAR(tBool,abBlockingMessages)
	IDLC_BUF_TO_BASE(ni::eType_I8,abBlockingMessages)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,UpdateWindow,1,(abBlockingMessages))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,UpdateWindow,1)

/** ni -> iOSWindow::CenterWindow/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,CenterWindow,0)
	IDLC_METH_CALL_VOID(ni,iOSWindow,CenterWindow,0,())
IDLC_METH_END(ni,iOSWindow,CenterWindow,0)

/** ni -> iOSWindow::GetRequestedClose/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetRequestedClose,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetRequestedClose,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetRequestedClose,0)

/** ni -> iOSWindow::SetRequestedClose/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetRequestedClose,1)
	IDLC_DECL_VAR(tBool,abRequested)
	IDLC_BUF_TO_BASE(ni::eType_I8,abRequested)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetRequestedClose,1,(abRequested))
IDLC_METH_END(ni,iOSWindow,SetRequestedClose,1)

/** ni -> iOSWindow::SetCursor/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetCursor,1)
	IDLC_DECL_VAR(eOSCursor,aCursor)
	IDLC_BUF_TO_ENUM(eOSCursor,aCursor)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetCursor,1,(aCursor))
IDLC_METH_END(ni,iOSWindow,SetCursor,1)

/** ni -> iOSWindow::GetCursor/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetCursor,0)
	IDLC_DECL_RETVAR(eOSCursor,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetCursor,0,())
	IDLC_RET_FROM_ENUM(eOSCursor,_Ret)
IDLC_METH_END(ni,iOSWindow,GetCursor,0)

/** ni -> iOSWindow::InitCustomCursor/6 **/
IDLC_METH_BEGIN(ni,iOSWindow,InitCustomCursor,6)
	IDLC_DECL_VAR(tIntPtr,aID)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aID)
	IDLC_DECL_VAR(tU32,anWidth)
	IDLC_BUF_TO_BASE(ni::eType_U32,anWidth)
	IDLC_DECL_VAR(tU32,anHeight)
	IDLC_BUF_TO_BASE(ni::eType_U32,anHeight)
	IDLC_DECL_VAR(tU32,anHotSpotX)
	IDLC_BUF_TO_BASE(ni::eType_U32,anHotSpotX)
	IDLC_DECL_VAR(tU32,anHotSpotY)
	IDLC_BUF_TO_BASE(ni::eType_U32,anHotSpotY)
	IDLC_DECL_VAR(tU32*,apData)
	IDLC_BUF_TO_NULL(ni::eTypeFlags_Constant|ni::eType_U32|ni::eTypeFlags_Pointer,apData)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,InitCustomCursor,6,(aID,anWidth,anHeight,anHotSpotX,anHotSpotY,apData))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,InitCustomCursor,6)

/** ni -> iOSWindow::GetCustomCursorID/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetCustomCursorID,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetCustomCursorID,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iOSWindow,GetCustomCursorID,0)

/** ni -> iOSWindow::SetCursorPosition/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetCursorPosition,1)
	IDLC_DECL_VAR(sVec2i,avCursorPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2i|ni::eTypeFlags_Pointer,avCursorPos)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetCursorPosition,1,(avCursorPos))
IDLC_METH_END(ni,iOSWindow,SetCursorPosition,1)

/** ni -> iOSWindow::GetCursorPosition/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetCursorPosition,0)
	IDLC_DECL_RETVAR(sVec2i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetCursorPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2i,_Ret)
IDLC_METH_END(ni,iOSWindow,GetCursorPosition,0)

/** ni -> iOSWindow::SetCursorCapture/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetCursorCapture,1)
	IDLC_DECL_VAR(tBool,abCapture)
	IDLC_BUF_TO_BASE(ni::eType_I8,abCapture)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetCursorCapture,1,(abCapture))
IDLC_METH_END(ni,iOSWindow,SetCursorCapture,1)

/** ni -> iOSWindow::GetCursorCapture/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetCursorCapture,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetCursorCapture,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetCursorCapture,0)

/** ni -> iOSWindow::GetIsCursorOverClient/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetIsCursorOverClient,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetIsCursorOverClient,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetIsCursorOverClient,0)

/** ni -> iOSWindow::TryClose/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,TryClose,0)
	IDLC_METH_CALL_VOID(ni,iOSWindow,TryClose,0,())
IDLC_METH_END(ni,iOSWindow,TryClose,0)

/** ni -> iOSWindow::SetDropTarget/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetDropTarget,1)
	IDLC_DECL_VAR(tBool,abDropTarget)
	IDLC_BUF_TO_BASE(ni::eType_I8,abDropTarget)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetDropTarget,1,(abDropTarget))
IDLC_METH_END(ni,iOSWindow,SetDropTarget,1)

/** ni -> iOSWindow::GetDropTarget/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetDropTarget,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetDropTarget,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetDropTarget,0)

/** ni -> iOSWindow::Clear/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,Clear,0)
	IDLC_METH_CALL_VOID(ni,iOSWindow,Clear,0,())
IDLC_METH_END(ni,iOSWindow,Clear,0)

/** ni -> iOSWindow::RedrawWindow/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,RedrawWindow,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,RedrawWindow,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,RedrawWindow,0)

/** ni -> iOSWindow::GetMonitor/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetMonitor,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetMonitor,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iOSWindow,GetMonitor,0)

/** ni -> iOSWindow::SetFullScreen/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetFullScreen,1)
	IDLC_DECL_VAR(tU32,anMonitor)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMonitor)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,SetFullScreen,1,(anMonitor))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,SetFullScreen,1)

/** ni -> iOSWindow::GetFullScreen/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetFullScreen,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetFullScreen,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iOSWindow,GetFullScreen,0)

/** ni -> iOSWindow::GetIsMinimized/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetIsMinimized,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetIsMinimized,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetIsMinimized,0)

/** ni -> iOSWindow::GetIsMaximized/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetIsMaximized,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetIsMaximized,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetIsMaximized,0)

/** ni -> iOSWindow::SetFocus/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetFocus,0)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetFocus,0,())
IDLC_METH_END(ni,iOSWindow,SetFocus,0)

/** ni -> iOSWindow::GetHasFocus/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetHasFocus,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetHasFocus,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,GetHasFocus,0)

/** ni -> iOSWindow::SetRefreshTimer/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,SetRefreshTimer,1)
	IDLC_DECL_VAR(tF32,afTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afTime)
	IDLC_METH_CALL_VOID(ni,iOSWindow,SetRefreshTimer,1,(afTime))
IDLC_METH_END(ni,iOSWindow,SetRefreshTimer,1)

/** ni -> iOSWindow::GetRefreshTimer/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetRefreshTimer,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetRefreshTimer,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iOSWindow,GetRefreshTimer,0)

/** ni -> iOSWindow::GetParentHandle/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetParentHandle,0)
	IDLC_DECL_RETVAR(tIntPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetParentHandle,0,())
	IDLC_RET_FROM_BASE(ni::eType_IntPtr,_Ret)
IDLC_METH_END(ni,iOSWindow,GetParentHandle,0)

/** ni -> iOSWindow::IsParentWindow/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,IsParentWindow,1)
	IDLC_DECL_VAR(tIntPtr,aHandle)
	IDLC_BUF_TO_BASE(ni::eType_IntPtr,aHandle)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,IsParentWindow,1,(aHandle))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iOSWindow,IsParentWindow,1)

/** ni -> iOSWindow::AttachGraphicsAPI/1 **/
IDLC_METH_BEGIN(ni,iOSWindow,AttachGraphicsAPI,1)
	IDLC_DECL_VAR(iOSGraphicsAPI*,apAPI)
	IDLC_BUF_TO_INTF(iOSGraphicsAPI,apAPI)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,AttachGraphicsAPI,1,(apAPI))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iOSWindow,AttachGraphicsAPI,1)

/** ni -> iOSWindow::GetGraphicsAPI/0 **/
IDLC_METH_BEGIN(ni,iOSWindow,GetGraphicsAPI,0)
	IDLC_DECL_RETVAR(iOSGraphicsAPI*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iOSWindow,GetGraphicsAPI,0,())
	IDLC_RET_FROM_INTF(iOSGraphicsAPI,_Ret)
IDLC_METH_END(ni,iOSWindow,GetGraphicsAPI,0)

IDLC_END_INTF(ni,iOSWindow)

/** interface : iOSWindowGeneric **/
IDLC_BEGIN_INTF(ni,iOSWindowGeneric)
/** ni -> iOSWindowGeneric::GenericInputString/1 **/
IDLC_METH_BEGIN(ni,iOSWindowGeneric,GenericInputString,1)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_METH_CALL_VOID(ni,iOSWindowGeneric,GenericInputString,1,(aaszString))
IDLC_METH_END(ni,iOSWindowGeneric,GenericInputString,1)

/** ni -> iOSWindowGeneric::GenericSendMessage/3 **/
IDLC_METH_BEGIN(ni,iOSWindowGeneric,GenericSendMessage,3)
	IDLC_DECL_VAR(tU32,anMsg)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMsg)
	IDLC_DECL_VAR(Var,avarA)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarA)
	IDLC_DECL_VAR(Var,avarB)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarB)
	IDLC_METH_CALL_VOID(ni,iOSWindowGeneric,GenericSendMessage,3,(anMsg,avarA,avarB))
IDLC_METH_END(ni,iOSWindowGeneric,GenericSendMessage,3)

/** ni -> iOSWindowGeneric::SetContentsScale/1 **/
IDLC_METH_BEGIN(ni,iOSWindowGeneric,SetContentsScale,1)
	IDLC_DECL_VAR(tF32,afContentsScale)
	IDLC_BUF_TO_BASE(ni::eType_F32,afContentsScale)
	IDLC_METH_CALL_VOID(ni,iOSWindowGeneric,SetContentsScale,1,(afContentsScale))
IDLC_METH_END(ni,iOSWindowGeneric,SetContentsScale,1)

IDLC_END_INTF(ni,iOSWindowGeneric)

/** interface : iOSWindowWindowsSink **/
/** iOSWindowWindowsSink -> NO AUTOMATION **/
/** ni -> iOSWindowWindowsSink::OnOSWindowWindowsSink_WndProc/4 -> NO AUTOMATION **/
/** interface : iOSWindowWindows **/
/** iOSWindowWindows -> NO AUTOMATION **/
/** ni -> iOSWindowWindows::GetWindowsWindowSinkList/0 -> NO AUTOMATION **/
/** ni -> iOSWindowWindows::WindowsTranslateKey/3 -> NO AUTOMATION **/
/** ni -> iOSWindowWindows::WndProc/4 -> NO AUTOMATION **/
/** interface : iOSWindowOSX **/
/** iOSWindowOSX -> NO AUTOMATION **/
/** ni -> iOSWindowOSX::GetNSWindow/0 -> NO AUTOMATION **/
/** interface : iOSWindowQNX **/
/** iOSWindowQNX -> NO AUTOMATION **/
/** ni -> iOSWindowQNX::GetScreenHandle/0 -> NO AUTOMATION **/
IDLC_END_NAMESPACE()
// EOF //
