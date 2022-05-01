#include "../API/niUI/IWidget.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iWidgetSink **/
IDLC_BEGIN_INTF(ni,iWidgetSink)
/** ni -> iWidgetSink::OnWidgetSink/4 **/
IDLC_METH_BEGIN(ni,iWidgetSink,OnWidgetSink,4)
	IDLC_DECL_VAR(iWidget*,apWidget)
	IDLC_BUF_TO_INTF(iWidget,apWidget)
	IDLC_DECL_VAR(tU32,nMsg)
	IDLC_BUF_TO_BASE(ni::eType_U32,nMsg)
	IDLC_DECL_VAR(ni::Var,varParam0)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,varParam0)
	IDLC_DECL_VAR(ni::Var,varParam1)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,varParam1)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetSink,OnWidgetSink,4,(apWidget,nMsg,varParam0,varParam1))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetSink,OnWidgetSink,4)

IDLC_END_INTF(ni,iWidgetSink)

/** interface : iWidgetCommand **/
IDLC_BEGIN_INTF(ni,iWidgetCommand)
/** ni -> iWidgetCommand::Copy/1 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,Copy,1)
	IDLC_DECL_VAR(iWidgetCommand*,apSrc)
	IDLC_BUF_TO_INTF(iWidgetCommand,apSrc)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCommand,Copy,1,(apSrc))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetCommand,Copy,1)

/** ni -> iWidgetCommand::Clone/0 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,Clone,0)
	IDLC_DECL_RETVAR(iWidgetCommand*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCommand,Clone,0,())
	IDLC_RET_FROM_INTF(iWidgetCommand,_Ret)
IDLC_METH_END(ni,iWidgetCommand,Clone,0)

/** ni -> iWidgetCommand::SetSender/1 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,SetSender,1)
	IDLC_DECL_VAR(iWidget*,apSender)
	IDLC_BUF_TO_INTF(iWidget,apSender)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCommand,SetSender,1,(apSender))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetCommand,SetSender,1)

/** ni -> iWidgetCommand::GetSender/0 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,GetSender,0)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCommand,GetSender,0,())
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetCommand,GetSender,0)

/** ni -> iWidgetCommand::SetID/1 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,SetID,1)
	IDLC_DECL_VAR(tU32,anID)
	IDLC_BUF_TO_BASE(ni::eType_U32,anID)
	IDLC_METH_CALL_VOID(ni,iWidgetCommand,SetID,1,(anID))
IDLC_METH_END(ni,iWidgetCommand,SetID,1)

/** ni -> iWidgetCommand::GetID/0 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,GetID,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCommand,GetID,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetCommand,GetID,0)

/** ni -> iWidgetCommand::SetExtra1/1 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,SetExtra1,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_METH_CALL_VOID(ni,iWidgetCommand,SetExtra1,1,(aVar))
IDLC_METH_END(ni,iWidgetCommand,SetExtra1,1)

/** ni -> iWidgetCommand::GetExtra1/0 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,GetExtra1,0)
	IDLC_DECL_RETREFVAR(Var&,_Ret,Var)
	IDLC_METH_CALL(_Ret,ni,iWidgetCommand,GetExtra1,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iWidgetCommand,GetExtra1,0)

/** ni -> iWidgetCommand::SetExtra2/1 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,SetExtra2,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_METH_CALL_VOID(ni,iWidgetCommand,SetExtra2,1,(aVar))
IDLC_METH_END(ni,iWidgetCommand,SetExtra2,1)

/** ni -> iWidgetCommand::GetExtra2/0 **/
IDLC_METH_BEGIN(ni,iWidgetCommand,GetExtra2,0)
	IDLC_DECL_RETREFVAR(Var&,_Ret,Var)
	IDLC_METH_CALL(_Ret,ni,iWidgetCommand,GetExtra2,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iWidgetCommand,GetExtra2,0)

IDLC_END_INTF(ni,iWidgetCommand)

/** interface : iWidget **/
IDLC_BEGIN_INTF(ni,iWidget)
/** ni -> iWidget::Destroy/0 **/
IDLC_METH_BEGIN(ni,iWidget,Destroy,0)
	IDLC_METH_CALL_VOID(ni,iWidget,Destroy,0,())
IDLC_METH_END(ni,iWidget,Destroy,0)

/** ni -> iWidget::GetGraphics/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetGraphics,0)
	IDLC_DECL_RETVAR(iGraphics*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetGraphics,0,())
	IDLC_RET_FROM_INTF(iGraphics,_Ret)
IDLC_METH_END(ni,iWidget,GetGraphics,0)

/** ni -> iWidget::GetGraphicsContext/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetGraphicsContext,0)
	IDLC_DECL_RETVAR(iGraphicsContext*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetGraphicsContext,0,())
	IDLC_RET_FROM_INTF(iGraphicsContext,_Ret)
IDLC_METH_END(ni,iWidget,GetGraphicsContext,0)

/** ni -> iWidget::GetUIContext/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetUIContext,0)
	IDLC_DECL_RETVAR(iUIContext*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetUIContext,0,())
	IDLC_RET_FROM_INTF(iUIContext,_Ret)
IDLC_METH_END(ni,iWidget,GetUIContext,0)

/** ni -> iWidget::GetClassName/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetClassName,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetClassName,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetClassName,0)

/** ni -> iWidget::SetID/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetID,1)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_METH_CALL_VOID(ni,iWidget,SetID,1,(ahspID))
IDLC_METH_END(ni,iWidget,SetID,1)

/** ni -> iWidget::GetID/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetID,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetID,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetID,0)

/** ni -> iWidget::SetStyle/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetStyle,1)
	IDLC_DECL_VAR(tU32,anStyle)
	IDLC_BUF_TO_BASE(ni::eType_U32,anStyle)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SetStyle,1,(anStyle))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SetStyle,1)

/** ni -> iWidget::GetStyle/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetStyle,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetStyle,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidget,GetStyle,0)

/** ni -> iWidget::SetParent/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetParent,1)
	IDLC_DECL_VAR(iWidget*,apParent)
	IDLC_BUF_TO_INTF(iWidget,apParent)
	IDLC_METH_CALL_VOID(ni,iWidget,SetParent,1,(apParent))
IDLC_METH_END(ni,iWidget,SetParent,1)

/** ni -> iWidget::GetParent/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetParent,0)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetParent,0,())
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidget,GetParent,0)

/** ni -> iWidget::SetZOrder/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetZOrder,1)
	IDLC_DECL_VAR(eWidgetZOrder,aZOrder)
	IDLC_BUF_TO_ENUM(eWidgetZOrder,aZOrder)
	IDLC_METH_CALL_VOID(ni,iWidget,SetZOrder,1,(aZOrder))
IDLC_METH_END(ni,iWidget,SetZOrder,1)

/** ni -> iWidget::GetZOrder/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetZOrder,0)
	IDLC_DECL_RETVAR(eWidgetZOrder,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetZOrder,0,())
	IDLC_RET_FROM_ENUM(eWidgetZOrder,_Ret)
IDLC_METH_END(ni,iWidget,GetZOrder,0)

/** ni -> iWidget::SetZOrderAbove/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetZOrderAbove,1)
	IDLC_DECL_VAR(iWidget*,apWidget)
	IDLC_BUF_TO_INTF(iWidget,apWidget)
	IDLC_METH_CALL_VOID(ni,iWidget,SetZOrderAbove,1,(apWidget))
IDLC_METH_END(ni,iWidget,SetZOrderAbove,1)

/** ni -> iWidget::GetDrawOrder/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetDrawOrder,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetDrawOrder,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidget,GetDrawOrder,0)

/** ni -> iWidget::SetAutoLayout/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetAutoLayout,1)
	IDLC_DECL_VAR(tWidgetAutoLayoutFlags,aFlags)
	IDLC_BUF_TO_ENUM(tWidgetAutoLayoutFlags,aFlags)
	IDLC_METH_CALL_VOID(ni,iWidget,SetAutoLayout,1,(aFlags))
IDLC_METH_END(ni,iWidget,SetAutoLayout,1)

/** ni -> iWidget::GetAutoLayout/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetAutoLayout,0)
	IDLC_DECL_RETVAR(tWidgetAutoLayoutFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetAutoLayout,0,())
	IDLC_RET_FROM_ENUM(tWidgetAutoLayoutFlags,_Ret)
IDLC_METH_END(ni,iWidget,GetAutoLayout,0)

/** ni -> iWidget::ComputeAutoLayout/1 **/
IDLC_METH_BEGIN(ni,iWidget,ComputeAutoLayout,1)
	IDLC_DECL_VAR(tWidgetAutoLayoutFlags,aFlags)
	IDLC_BUF_TO_ENUM(tWidgetAutoLayoutFlags,aFlags)
	IDLC_METH_CALL_VOID(ni,iWidget,ComputeAutoLayout,1,(aFlags))
IDLC_METH_END(ni,iWidget,ComputeAutoLayout,1)

/** ni -> iWidget::SetPosition/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetPosition,1)
	IDLC_DECL_VAR(sVec2f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iWidget,SetPosition,1,(avPos))
IDLC_METH_END(ni,iWidget,SetPosition,1)

/** ni -> iWidget::GetPosition/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetPosition,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetPosition,0)

/** ni -> iWidget::SetSize/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetSize,1)
	IDLC_DECL_VAR(sVec2f,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avSize)
	IDLC_METH_CALL_VOID(ni,iWidget,SetSize,1,(avSize))
IDLC_METH_END(ni,iWidget,SetSize,1)

/** ni -> iWidget::GetSize/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetSize,0)

/** ni -> iWidget::SetMinimumSize/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetMinimumSize,1)
	IDLC_DECL_VAR(sVec2f,avMinSize)
	IDLC_BUF_TO_BASE(ni::eType_Vec2f,avMinSize)
	IDLC_METH_CALL_VOID(ni,iWidget,SetMinimumSize,1,(avMinSize))
IDLC_METH_END(ni,iWidget,SetMinimumSize,1)

/** ni -> iWidget::GetMinimumSize/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetMinimumSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetMinimumSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetMinimumSize,0)

/** ni -> iWidget::SetMaximumSize/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetMaximumSize,1)
	IDLC_DECL_VAR(sVec2f,avMaxSize)
	IDLC_BUF_TO_BASE(ni::eType_Vec2f,avMaxSize)
	IDLC_METH_CALL_VOID(ni,iWidget,SetMaximumSize,1,(avMaxSize))
IDLC_METH_END(ni,iWidget,SetMaximumSize,1)

/** ni -> iWidget::GetMaximumSize/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetMaximumSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetMaximumSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetMaximumSize,0)

/** ni -> iWidget::SetRect/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetRect,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iWidget,SetRect,1,(aRect))
IDLC_METH_END(ni,iWidget,SetRect,1)

/** ni -> iWidget::GetRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetRect,0)

/** ni -> iWidget::GetWidgetRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetWidgetRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetWidgetRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetWidgetRect,0)

/** ni -> iWidget::GetDockFillRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetDockFillRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetDockFillRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetDockFillRect,0)

/** ni -> iWidget::SetClientPosition/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetClientPosition,1)
	IDLC_DECL_VAR(sVec2f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iWidget,SetClientPosition,1,(avPos))
IDLC_METH_END(ni,iWidget,SetClientPosition,1)

/** ni -> iWidget::GetClientPosition/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetClientPosition,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetClientPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetClientPosition,0)

/** ni -> iWidget::SetClientSize/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetClientSize,1)
	IDLC_DECL_VAR(sVec2f,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avSize)
	IDLC_METH_CALL_VOID(ni,iWidget,SetClientSize,1,(avSize))
IDLC_METH_END(ni,iWidget,SetClientSize,1)

/** ni -> iWidget::GetClientSize/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetClientSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetClientSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetClientSize,0)

/** ni -> iWidget::SetClientRect/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetClientRect,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iWidget,SetClientRect,1,(aRect))
IDLC_METH_END(ni,iWidget,SetClientRect,1)

/** ni -> iWidget::GetClientRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetClientRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetClientRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetClientRect,0)

/** ni -> iWidget::ComputeFitRect/1 **/
IDLC_METH_BEGIN(ni,iWidget,ComputeFitRect,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,ComputeFitRect,1,(aRect))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,ComputeFitRect,1)

/** ni -> iWidget::SetFitRect/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetFitRect,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iWidget,SetFitRect,1,(aRect))
IDLC_METH_END(ni,iWidget,SetFitRect,1)

/** ni -> iWidget::SetFitSize/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetFitSize,1)
	IDLC_DECL_VAR(sVec2f,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f,avSize)
	IDLC_METH_CALL_VOID(ni,iWidget,SetFitSize,1,(avSize))
IDLC_METH_END(ni,iWidget,SetFitSize,1)

/** ni -> iWidget::SetAbsolutePosition/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetAbsolutePosition,1)
	IDLC_DECL_VAR(sVec2f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iWidget,SetAbsolutePosition,1,(avPos))
IDLC_METH_END(ni,iWidget,SetAbsolutePosition,1)

/** ni -> iWidget::GetAbsolutePosition/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetAbsolutePosition,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetAbsolutePosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetAbsolutePosition,0)

/** ni -> iWidget::SetAbsoluteRect/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetAbsoluteRect,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iWidget,SetAbsoluteRect,1,(aRect))
IDLC_METH_END(ni,iWidget,SetAbsoluteRect,1)

/** ni -> iWidget::GetAbsoluteRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetAbsoluteRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetAbsoluteRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetAbsoluteRect,0)

/** ni -> iWidget::GetClippedRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetClippedRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetClippedRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetClippedRect,0)

/** ni -> iWidget::GetAbsoluteClippedRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetAbsoluteClippedRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetAbsoluteClippedRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetAbsoluteClippedRect,0)

/** ni -> iWidget::GetClippedClientRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetClippedClientRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetClippedClientRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetClippedClientRect,0)

/** ni -> iWidget::GetAbsoluteClippedClientRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetAbsoluteClippedClientRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetAbsoluteClippedClientRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetAbsoluteClippedClientRect,0)

/** ni -> iWidget::SetRelativePosition/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetRelativePosition,1)
	IDLC_DECL_VAR(sVec2f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPos)
	IDLC_METH_CALL_VOID(ni,iWidget,SetRelativePosition,1,(avPos))
IDLC_METH_END(ni,iWidget,SetRelativePosition,1)

/** ni -> iWidget::GetRelativePosition/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetRelativePosition,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetRelativePosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetRelativePosition,0)

/** ni -> iWidget::SetRelativeSize/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetRelativeSize,1)
	IDLC_DECL_VAR(sVec2f,avSize)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avSize)
	IDLC_METH_CALL_VOID(ni,iWidget,SetRelativeSize,1,(avSize))
IDLC_METH_END(ni,iWidget,SetRelativeSize,1)

/** ni -> iWidget::GetRelativeSize/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetRelativeSize,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetRelativeSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidget,GetRelativeSize,0)

/** ni -> iWidget::SetRelativeRect/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetRelativeRect,1)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iWidget,SetRelativeRect,1,(aRect))
IDLC_METH_END(ni,iWidget,SetRelativeRect,1)

/** ni -> iWidget::GetRelativeRect/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetRelativeRect,0)
	IDLC_DECL_RETVAR(sRectf,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetRelativeRect,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetRelativeRect,0)

/** ni -> iWidget::SetPadding/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetPadding,1)
	IDLC_DECL_VAR(sVec4f,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iWidget,SetPadding,1,(aRect))
IDLC_METH_END(ni,iWidget,SetPadding,1)

/** ni -> iWidget::GetPadding/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetPadding,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetPadding,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetPadding,0)

/** ni -> iWidget::GetHasPadding/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetHasPadding,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetHasPadding,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetHasPadding,0)

/** ni -> iWidget::SetMargin/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetMargin,1)
	IDLC_DECL_VAR(sVec4f,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_METH_CALL_VOID(ni,iWidget,SetMargin,1,(aRect))
IDLC_METH_END(ni,iWidget,SetMargin,1)

/** ni -> iWidget::GetMargin/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetMargin,0)
	IDLC_DECL_RETVAR(sVec4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetMargin,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,GetMargin,0)

/** ni -> iWidget::GetHasMargin/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetHasMargin,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetHasMargin,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetHasMargin,0)

/** ni -> iWidget::SetFocus/0 **/
IDLC_METH_BEGIN(ni,iWidget,SetFocus,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SetFocus,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SetFocus,0)

/** ni -> iWidget::MoveFocus/1 **/
IDLC_METH_BEGIN(ni,iWidget,MoveFocus,1)
	IDLC_DECL_VAR(tBool,abToPrevious)
	IDLC_BUF_TO_BASE(ni::eType_I8,abToPrevious)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,MoveFocus,1,(abToPrevious))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,MoveFocus,1)

/** ni -> iWidget::SetCapture/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetCapture,1)
	IDLC_DECL_VAR(tBool,abEnable)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEnable)
	IDLC_METH_CALL_VOID(ni,iWidget,SetCapture,1,(abEnable))
IDLC_METH_END(ni,iWidget,SetCapture,1)

/** ni -> iWidget::GetCapture/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetCapture,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetCapture,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetCapture,0)

/** ni -> iWidget::SetExclusive/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetExclusive,1)
	IDLC_DECL_VAR(tBool,abEnable)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEnable)
	IDLC_METH_CALL_VOID(ni,iWidget,SetExclusive,1,(abEnable))
IDLC_METH_END(ni,iWidget,SetExclusive,1)

/** ni -> iWidget::GetExclusive/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetExclusive,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetExclusive,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetExclusive,0)

/** ni -> iWidget::SetInputSubmitFlags/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetInputSubmitFlags,1)
	IDLC_DECL_VAR(tU32,aSubmitFlags)
	IDLC_BUF_TO_BASE(ni::eType_U32,aSubmitFlags)
	IDLC_METH_CALL_VOID(ni,iWidget,SetInputSubmitFlags,1,(aSubmitFlags))
IDLC_METH_END(ni,iWidget,SetInputSubmitFlags,1)

/** ni -> iWidget::GetInputSubmitFlags/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetInputSubmitFlags,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetInputSubmitFlags,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidget,GetInputSubmitFlags,0)

/** ni -> iWidget::SetVisible/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetVisible,1)
	IDLC_DECL_VAR(tBool,abVisible)
	IDLC_BUF_TO_BASE(ni::eType_I8,abVisible)
	IDLC_METH_CALL_VOID(ni,iWidget,SetVisible,1,(abVisible))
IDLC_METH_END(ni,iWidget,SetVisible,1)

/** ni -> iWidget::GetVisible/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetVisible,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetVisible,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetVisible,0)

/** ni -> iWidget::SetEnabled/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetEnabled,1)
	IDLC_DECL_VAR(tBool,abEnabled)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEnabled)
	IDLC_METH_CALL_VOID(ni,iWidget,SetEnabled,1,(abEnabled))
IDLC_METH_END(ni,iWidget,SetEnabled,1)

/** ni -> iWidget::GetEnabled/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetEnabled,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetEnabled,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetEnabled,0)

/** ni -> iWidget::SetIgnoreInput/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetIgnoreInput,1)
	IDLC_DECL_VAR(tBool,abIgnoreInput)
	IDLC_BUF_TO_BASE(ni::eType_I8,abIgnoreInput)
	IDLC_METH_CALL_VOID(ni,iWidget,SetIgnoreInput,1,(abIgnoreInput))
IDLC_METH_END(ni,iWidget,SetIgnoreInput,1)

/** ni -> iWidget::GetIgnoreInput/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetIgnoreInput,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetIgnoreInput,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetIgnoreInput,0)

/** ni -> iWidget::SetStatus/3 **/
IDLC_METH_BEGIN(ni,iWidget,SetStatus,3)
	IDLC_DECL_VAR(tBool,abVisible)
	IDLC_BUF_TO_BASE(ni::eType_I8,abVisible)
	IDLC_DECL_VAR(tBool,abEnabled)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEnabled)
	IDLC_DECL_VAR(tBool,abIgnoreInput)
	IDLC_BUF_TO_BASE(ni::eType_I8,abIgnoreInput)
	IDLC_METH_CALL_VOID(ni,iWidget,SetStatus,3,(abVisible,abEnabled,abIgnoreInput))
IDLC_METH_END(ni,iWidget,SetStatus,3)

/** ni -> iWidget::SetHideChildren/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetHideChildren,1)
	IDLC_DECL_VAR(tBool,abHideChildren)
	IDLC_BUF_TO_BASE(ni::eType_I8,abHideChildren)
	IDLC_METH_CALL_VOID(ni,iWidget,SetHideChildren,1,(abHideChildren))
IDLC_METH_END(ni,iWidget,SetHideChildren,1)

/** ni -> iWidget::GetHideChildren/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetHideChildren,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetHideChildren,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetHideChildren,0)

/** ni -> iWidget::GetIsMouseOver/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetIsMouseOver,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetIsMouseOver,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetIsMouseOver,0)

/** ni -> iWidget::GetIsPressed/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetIsPressed,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetIsPressed,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetIsPressed,0)

/** ni -> iWidget::GetIsNcMouseOver/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetIsNcMouseOver,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetIsNcMouseOver,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetIsNcMouseOver,0)

/** ni -> iWidget::GetIsNcPressed/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetIsNcPressed,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetIsNcPressed,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetIsNcPressed,0)

/** ni -> iWidget::GetHasFocus/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetHasFocus,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetHasFocus,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetHasFocus,0)

/** ni -> iWidget::GetDraggingSource/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetDraggingSource,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetDraggingSource,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetDraggingSource,0)

/** ni -> iWidget::GetDragging/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetDragging,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetDragging,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetDragging,0)

/** ni -> iWidget::Redraw/0 **/
IDLC_METH_BEGIN(ni,iWidget,Redraw,0)
	IDLC_METH_CALL_VOID(ni,iWidget,Redraw,0,())
IDLC_METH_END(ni,iWidget,Redraw,0)

/** ni -> iWidget::SetTimer/2 **/
IDLC_METH_BEGIN(ni,iWidget,SetTimer,2)
	IDLC_DECL_VAR(tU32,anID)
	IDLC_BUF_TO_BASE(ni::eType_U32,anID)
	IDLC_DECL_VAR(tF32,afTime)
	IDLC_BUF_TO_BASE(ni::eType_F32,afTime)
	IDLC_METH_CALL_VOID(ni,iWidget,SetTimer,2,(anID,afTime))
IDLC_METH_END(ni,iWidget,SetTimer,2)

/** ni -> iWidget::GetTimer/1 **/
IDLC_METH_BEGIN(ni,iWidget,GetTimer,1)
	IDLC_DECL_VAR(tU32,anID)
	IDLC_BUF_TO_BASE(ni::eType_U32,anID)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetTimer,1,(anID))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidget,GetTimer,1)

/** ni -> iWidget::AddSink/1 **/
IDLC_METH_BEGIN(ni,iWidget,AddSink,1)
	IDLC_DECL_VAR(iWidgetSink*,apSink)
	IDLC_BUF_TO_INTF(iWidgetSink,apSink)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,AddSink,1,(apSink))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,AddSink,1)

/** ni -> iWidget::AddPostSink/1 **/
IDLC_METH_BEGIN(ni,iWidget,AddPostSink,1)
	IDLC_DECL_VAR(iWidgetSink*,apSink)
	IDLC_BUF_TO_INTF(iWidgetSink,apSink)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,AddPostSink,1,(apSink))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,AddPostSink,1)

/** ni -> iWidget::AddClassSink/1 **/
IDLC_METH_BEGIN(ni,iWidget,AddClassSink,1)
	IDLC_DECL_VAR(achar*,aaszClassName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszClassName)
	IDLC_DECL_RETVAR(iWidgetSink*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,AddClassSink,1,(aaszClassName))
	IDLC_RET_FROM_INTF(iWidgetSink,_Ret)
IDLC_METH_END(ni,iWidget,AddClassSink,1)

/** ni -> iWidget::AddClassPostSink/1 **/
IDLC_METH_BEGIN(ni,iWidget,AddClassPostSink,1)
	IDLC_DECL_VAR(achar*,aaszClassName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszClassName)
	IDLC_DECL_RETVAR(iWidgetSink*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,AddClassPostSink,1,(aaszClassName))
	IDLC_RET_FROM_INTF(iWidgetSink,_Ret)
IDLC_METH_END(ni,iWidget,AddClassPostSink,1)

/** ni -> iWidget::RemoveSink/1 **/
IDLC_METH_BEGIN(ni,iWidget,RemoveSink,1)
	IDLC_DECL_VAR(iWidgetSink*,apSink)
	IDLC_BUF_TO_INTF(iWidgetSink,apSink)
	IDLC_METH_CALL_VOID(ni,iWidget,RemoveSink,1,(apSink))
IDLC_METH_END(ni,iWidget,RemoveSink,1)

/** ni -> iWidget::InvalidateChildren/0 **/
IDLC_METH_BEGIN(ni,iWidget,InvalidateChildren,0)
	IDLC_METH_CALL_VOID(ni,iWidget,InvalidateChildren,0,())
IDLC_METH_END(ni,iWidget,InvalidateChildren,0)

/** ni -> iWidget::GetNumChildren/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetNumChildren,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetNumChildren,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidget,GetNumChildren,0)

/** ni -> iWidget::GetChildIndex/1 **/
IDLC_METH_BEGIN(ni,iWidget,GetChildIndex,1)
	IDLC_DECL_VAR(iWidget*,apWidget)
	IDLC_BUF_TO_INTF(iWidget,apWidget)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetChildIndex,1,(apWidget))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidget,GetChildIndex,1)

/** ni -> iWidget::GetChildFromIndex/1 **/
IDLC_METH_BEGIN(ni,iWidget,GetChildFromIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetChildFromIndex,1,(anIndex))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidget,GetChildFromIndex,1)

/** ni -> iWidget::GetChildFromID/1 **/
IDLC_METH_BEGIN(ni,iWidget,GetChildFromID,1)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetChildFromID,1,(ahspID))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidget,GetChildFromID,1)

/** ni -> iWidget::GetChildFromDrawOrder/1 **/
IDLC_METH_BEGIN(ni,iWidget,GetChildFromDrawOrder,1)
	IDLC_DECL_VAR(tU32,anDrawOrder)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDrawOrder)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetChildFromDrawOrder,1,(anDrawOrder))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidget,GetChildFromDrawOrder,1)

/** ni -> iWidget::FindWidget/1 **/
IDLC_METH_BEGIN(ni,iWidget,FindWidget,1)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,FindWidget,1,(ahspID))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidget,FindWidget,1)

/** ni -> iWidget::FindWidgetByPos/1 **/
IDLC_METH_BEGIN(ni,iWidget,FindWidgetByPos,1)
	IDLC_DECL_VAR(sVec2f,avPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avPos)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,FindWidgetByPos,1,(avPos))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidget,FindWidgetByPos,1)

/** ni -> iWidget::HasChild/2 **/
IDLC_METH_BEGIN(ni,iWidget,HasChild,2)
	IDLC_DECL_VAR(iWidget*,apW)
	IDLC_BUF_TO_INTF(iWidget,apW)
	IDLC_DECL_VAR(tBool,abRecursive)
	IDLC_BUF_TO_BASE(ni::eType_I8,abRecursive)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,HasChild,2,(apW,abRecursive))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,HasChild,2)

/** ni -> iWidget::HasParent/1 **/
IDLC_METH_BEGIN(ni,iWidget,HasParent,1)
	IDLC_DECL_VAR(iWidget*,apW)
	IDLC_BUF_TO_INTF(iWidget,apW)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,HasParent,1,(apW))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,HasParent,1)

/** ni -> iWidget::SetText/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetText,1)
	IDLC_DECL_VAR(iHString*,ahspText)
	IDLC_BUF_TO_INTF(iHString,ahspText)
	IDLC_METH_CALL_VOID(ni,iWidget,SetText,1,(ahspText))
IDLC_METH_END(ni,iWidget,SetText,1)

/** ni -> iWidget::GetText/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetText,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetText,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetText,0)

/** ni -> iWidget::GetLocalizedText/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetLocalizedText,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetLocalizedText,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetLocalizedText,0)

/** ni -> iWidget::SetHoverText/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetHoverText,1)
	IDLC_DECL_VAR(iHString*,ahspText)
	IDLC_BUF_TO_INTF(iHString,ahspText)
	IDLC_METH_CALL_VOID(ni,iWidget,SetHoverText,1,(ahspText))
IDLC_METH_END(ni,iWidget,SetHoverText,1)

/** ni -> iWidget::GetHoverText/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetHoverText,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetHoverText,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetHoverText,0)

/** ni -> iWidget::CreateDefaultHoverWidget/1 **/
IDLC_METH_BEGIN(ni,iWidget,CreateDefaultHoverWidget,1)
	IDLC_DECL_VAR(ni::iHString*,ahspHoverText)
	IDLC_BUF_TO_INTF(ni::iHString,ahspHoverText)
	IDLC_DECL_RETVAR(ni::iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,CreateDefaultHoverWidget,1,(ahspHoverText))
	IDLC_RET_FROM_INTF(ni::iWidget,_Ret)
IDLC_METH_END(ni,iWidget,CreateDefaultHoverWidget,1)

/** ni -> iWidget::ShowHoverWidget/2 **/
IDLC_METH_BEGIN(ni,iWidget,ShowHoverWidget,2)
	IDLC_DECL_VAR(ni::iWidget*,apWidget)
	IDLC_BUF_TO_INTF(ni::iWidget,apWidget)
	IDLC_DECL_VAR(sVec2f,avAbsPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avAbsPos)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,ShowHoverWidget,2,(apWidget,avAbsPos))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,ShowHoverWidget,2)

/** ni -> iWidget::ResetHoverWidget/1 **/
IDLC_METH_BEGIN(ni,iWidget,ResetHoverWidget,1)
	IDLC_DECL_VAR(tBool,abRestart)
	IDLC_BUF_TO_BASE(ni::eType_I8,abRestart)
	IDLC_METH_CALL_VOID(ni,iWidget,ResetHoverWidget,1,(abRestart))
IDLC_METH_END(ni,iWidget,ResetHoverWidget,1)

/** ni -> iWidget::GetHoverWidget/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetHoverWidget,0)
	IDLC_DECL_RETVAR(ni::iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetHoverWidget,0,())
	IDLC_RET_FROM_INTF(ni::iWidget,_Ret)
IDLC_METH_END(ni,iWidget,GetHoverWidget,0)

/** ni -> iWidget::SetFont/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetFont,1)
	IDLC_DECL_VAR(iFont*,apFont)
	IDLC_BUF_TO_INTF(iFont,apFont)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SetFont,1,(apFont))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SetFont,1)

/** ni -> iWidget::GetFont/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetFont,0)
	IDLC_DECL_RETVAR(iFont*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetFont,0,())
	IDLC_RET_FROM_INTF(iFont,_Ret)
IDLC_METH_END(ni,iWidget,GetFont,0)

/** ni -> iWidget::SetDockStyle/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetDockStyle,1)
	IDLC_DECL_VAR(eWidgetDockStyle,aStyle)
	IDLC_BUF_TO_ENUM(eWidgetDockStyle,aStyle)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SetDockStyle,1,(aStyle))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SetDockStyle,1)

/** ni -> iWidget::GetDockStyle/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetDockStyle,0)
	IDLC_DECL_RETVAR(eWidgetDockStyle,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetDockStyle,0,())
	IDLC_RET_FROM_ENUM(eWidgetDockStyle,_Ret)
IDLC_METH_END(ni,iWidget,GetDockStyle,0)

/** ni -> iWidget::SetSkin/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetSkin,1)
	IDLC_DECL_VAR(iHString*,ahspSkin)
	IDLC_BUF_TO_INTF(iHString,ahspSkin)
	IDLC_METH_CALL_VOID(ni,iWidget,SetSkin,1,(ahspSkin))
IDLC_METH_END(ni,iWidget,SetSkin,1)

/** ni -> iWidget::GetSkin/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetSkin,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetSkin,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetSkin,0)

/** ni -> iWidget::SetSkinClass/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetSkinClass,1)
	IDLC_DECL_VAR(iHString*,ahspSkinClass)
	IDLC_BUF_TO_INTF(iHString,ahspSkinClass)
	IDLC_METH_CALL_VOID(ni,iWidget,SetSkinClass,1,(ahspSkinClass))
IDLC_METH_END(ni,iWidget,SetSkinClass,1)

/** ni -> iWidget::GetSkinClass/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetSkinClass,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetSkinClass,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetSkinClass,0)

/** ni -> iWidget::SetLocale/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetLocale,1)
	IDLC_DECL_VAR(iHString*,ahspLocale)
	IDLC_BUF_TO_INTF(iHString,ahspLocale)
	IDLC_METH_CALL_VOID(ni,iWidget,SetLocale,1,(ahspLocale))
IDLC_METH_END(ni,iWidget,SetLocale,1)

/** ni -> iWidget::GetLocale/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetLocale,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetLocale,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetLocale,0)

/** ni -> iWidget::GetActiveLocale/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetActiveLocale,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetActiveLocale,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,GetActiveLocale,0)

/** ni -> iWidget::FindSkinFont/3 **/
IDLC_METH_BEGIN(ni,iWidget,FindSkinFont,3)
	IDLC_DECL_VAR(iHString*,ahspSkinClass)
	IDLC_BUF_TO_INTF(iHString,ahspSkinClass)
	IDLC_DECL_VAR(iHString*,ahspState)
	IDLC_BUF_TO_INTF(iHString,ahspState)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(iFont*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,FindSkinFont,3,(ahspSkinClass,ahspState,ahspName))
	IDLC_RET_FROM_INTF(iFont,_Ret)
IDLC_METH_END(ni,iWidget,FindSkinFont,3)

/** ni -> iWidget::FindSkinCursor/3 **/
IDLC_METH_BEGIN(ni,iWidget,FindSkinCursor,3)
	IDLC_DECL_VAR(iHString*,ahspSkinClass)
	IDLC_BUF_TO_INTF(iHString,ahspSkinClass)
	IDLC_DECL_VAR(iHString*,ahspState)
	IDLC_BUF_TO_INTF(iHString,ahspState)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,FindSkinCursor,3,(ahspSkinClass,ahspState,ahspName))
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iWidget,FindSkinCursor,3)

/** ni -> iWidget::FindSkinElement/3 **/
IDLC_METH_BEGIN(ni,iWidget,FindSkinElement,3)
	IDLC_DECL_VAR(iHString*,ahspSkinClass)
	IDLC_BUF_TO_INTF(iHString,ahspSkinClass)
	IDLC_DECL_VAR(iHString*,ahspState)
	IDLC_BUF_TO_INTF(iHString,ahspState)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,FindSkinElement,3,(ahspSkinClass,ahspState,ahspName))
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iWidget,FindSkinElement,3)

/** ni -> iWidget::FindSkinColor/4 **/
IDLC_METH_BEGIN(ni,iWidget,FindSkinColor,4)
	IDLC_DECL_VAR(sColor4f,aDefault)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aDefault)
	IDLC_DECL_VAR(iHString*,ahspSkinClass)
	IDLC_BUF_TO_INTF(iHString,ahspSkinClass)
	IDLC_DECL_VAR(iHString*,ahspState)
	IDLC_BUF_TO_INTF(iHString,ahspState)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(sColor4f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,FindSkinColor,4,(aDefault,ahspSkinClass,ahspState,ahspName))
	IDLC_RET_FROM_BASE(ni::eType_Vec4f,_Ret)
IDLC_METH_END(ni,iWidget,FindSkinColor,4)

/** ni -> iWidget::FindLocalized/1 **/
IDLC_METH_BEGIN(ni,iWidget,FindLocalized,1)
	IDLC_DECL_VAR(iHString*,ahspText)
	IDLC_BUF_TO_INTF(iHString,ahspText)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,FindLocalized,1,(ahspText))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidget,FindLocalized,1)

/** ni -> iWidget::SetContextMenu/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetContextMenu,1)
	IDLC_DECL_VAR(iWidget*,apMenu)
	IDLC_BUF_TO_INTF(iWidget,apMenu)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SetContextMenu,1,(apMenu))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SetContextMenu,1)

/** ni -> iWidget::GetContextMenu/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetContextMenu,0)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetContextMenu,0,())
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidget,GetContextMenu,0)

/** ni -> iWidget::SendMessage/3 **/
IDLC_METH_BEGIN(ni,iWidget,SendMessage,3)
	IDLC_DECL_VAR(tU32,anMsg)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMsg)
	IDLC_DECL_VAR(Var,avarA)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarA)
	IDLC_DECL_VAR(Var,avarB)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarB)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SendMessage,3,(anMsg,avarA,avarB))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SendMessage,3)

/** ni -> iWidget::SendCommand/4 **/
IDLC_METH_BEGIN(ni,iWidget,SendCommand,4)
	IDLC_DECL_VAR(iWidget*,apDest)
	IDLC_BUF_TO_INTF(iWidget,apDest)
	IDLC_DECL_VAR(tU32,anCommand)
	IDLC_BUF_TO_BASE(ni::eType_U32,anCommand)
	IDLC_DECL_VAR(Var,avarExtra1)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarExtra1)
	IDLC_DECL_VAR(Var,avarExtra2)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarExtra2)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SendCommand,4,(apDest,anCommand,avarExtra1,avarExtra2))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SendCommand,4)

/** ni -> iWidget::BroadcastMessage/3 **/
IDLC_METH_BEGIN(ni,iWidget,BroadcastMessage,3)
	IDLC_DECL_VAR(tU32,anMsg)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMsg)
	IDLC_DECL_VAR(Var,avarA)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarA)
	IDLC_DECL_VAR(Var,avarB)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,avarB)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,BroadcastMessage,3,(anMsg,avarA,avarB))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,BroadcastMessage,3)

/** ni -> iWidget::SetCanvas/1 **/
IDLC_METH_BEGIN(ni,iWidget,SetCanvas,1)
	IDLC_DECL_VAR(iCanvas*,apCanvas)
	IDLC_BUF_TO_INTF(iCanvas,apCanvas)
	IDLC_METH_CALL_VOID(ni,iWidget,SetCanvas,1,(apCanvas))
IDLC_METH_END(ni,iWidget,SetCanvas,1)

/** ni -> iWidget::GetCanvas/0 **/
IDLC_METH_BEGIN(ni,iWidget,GetCanvas,0)
	IDLC_DECL_RETVAR(iCanvas*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetCanvas,0,())
	IDLC_RET_FROM_INTF(iCanvas,_Ret)
IDLC_METH_END(ni,iWidget,GetCanvas,0)

/** ni -> iWidget::SerializeLayout/2 **/
IDLC_METH_BEGIN(ni,iWidget,SerializeLayout,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(tWidgetSerializeFlags,anFlags)
	IDLC_BUF_TO_ENUM(tWidgetSerializeFlags,anFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SerializeLayout,2,(apDT,anFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SerializeLayout,2)

/** ni -> iWidget::SerializeChildren/2 **/
IDLC_METH_BEGIN(ni,iWidget,SerializeChildren,2)
	IDLC_DECL_VAR(iDataTable*,apDT)
	IDLC_BUF_TO_INTF(iDataTable,apDT)
	IDLC_DECL_VAR(tWidgetSerializeFlags,anFlags)
	IDLC_BUF_TO_ENUM(tWidgetSerializeFlags,anFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SerializeChildren,2,(apDT,anFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SerializeChildren,2)

/** ni -> iWidget::ApplyDockStyle/1 **/
IDLC_METH_BEGIN(ni,iWidget,ApplyDockStyle,1)
	IDLC_DECL_VAR(eWidgetDockStyle,aStyle)
	IDLC_BUF_TO_ENUM(eWidgetDockStyle,aStyle)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,ApplyDockStyle,1,(aStyle))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,ApplyDockStyle,1)

/** ni -> iWidget::Place/3 **/
IDLC_METH_BEGIN(ni,iWidget,Place,3)
	IDLC_DECL_VAR(sRectf,aRect)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,aRect)
	IDLC_DECL_VAR(eWidgetDockStyle,aStyle)
	IDLC_BUF_TO_ENUM(eWidgetDockStyle,aStyle)
	IDLC_DECL_VAR(sVec4f,avMargin)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec4f|ni::eTypeFlags_Pointer,avMargin)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,Place,3,(aRect,aStyle,avMargin))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,Place,3)

/** ni -> iWidget::SnapInside/2 **/
IDLC_METH_BEGIN(ni,iWidget,SnapInside,2)
	IDLC_DECL_VAR(iWidget*,apContainer)
	IDLC_BUF_TO_INTF(iWidget,apContainer)
	IDLC_DECL_VAR(tF32,afSnapMargin)
	IDLC_BUF_TO_BASE(ni::eType_F32,afSnapMargin)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,SnapInside,2,(apContainer,afSnapMargin))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,SnapInside,2)

/** ni -> iWidget::PopAt/3 **/
IDLC_METH_BEGIN(ni,iWidget,PopAt,3)
	IDLC_DECL_VAR(iWidget*,apContainer)
	IDLC_BUF_TO_INTF(iWidget,apContainer)
	IDLC_DECL_VAR(sVec2f,avAbsPos)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avAbsPos)
	IDLC_DECL_VAR(tF32,afSnapMargin)
	IDLC_BUF_TO_BASE(ni::eType_F32,afSnapMargin)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,PopAt,3,(apContainer,avAbsPos,afSnapMargin))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,PopAt,3)

/** ni -> iWidget::Layout/1 **/
IDLC_METH_BEGIN(ni,iWidget,Layout,1)
	IDLC_DECL_VAR(tBool,abChildren)
	IDLC_BUF_TO_BASE(ni::eType_I8,abChildren)
	IDLC_METH_CALL_VOID(ni,iWidget,Layout,1,(abChildren))
IDLC_METH_END(ni,iWidget,Layout,1)

/** ni -> iWidget::SetFingerCapture/2 **/
IDLC_METH_BEGIN(ni,iWidget,SetFingerCapture,2)
	IDLC_DECL_VAR(tU32,anFinger)
	IDLC_BUF_TO_BASE(ni::eType_U32,anFinger)
	IDLC_DECL_VAR(tBool,abEnable)
	IDLC_BUF_TO_BASE(ni::eType_I8,abEnable)
	IDLC_METH_CALL_VOID(ni,iWidget,SetFingerCapture,2,(anFinger,abEnable))
IDLC_METH_END(ni,iWidget,SetFingerCapture,2)

/** ni -> iWidget::GetFingerCapture/1 **/
IDLC_METH_BEGIN(ni,iWidget,GetFingerCapture,1)
	IDLC_DECL_VAR(tU32,anFinger)
	IDLC_BUF_TO_BASE(ni::eType_U32,anFinger)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidget,GetFingerCapture,1,(anFinger))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidget,GetFingerCapture,1)

IDLC_END_INTF(ni,iWidget)

IDLC_END_NAMESPACE()
// EOF //
