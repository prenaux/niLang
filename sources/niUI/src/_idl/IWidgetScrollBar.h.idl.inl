#include "../API/niUI/IWidgetScrollBar.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iWidgetScrollBar **/
IDLC_BEGIN_INTF(ni,iWidgetScrollBar)
/** ni -> iWidgetScrollBar::SetScrollRange/1 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,SetScrollRange,1)
	IDLC_DECL_VAR(sVec2f,avRange)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Vec2f|ni::eTypeFlags_Pointer,avRange)
	IDLC_METH_CALL_VOID(ni,iWidgetScrollBar,SetScrollRange,1,(avRange))
IDLC_METH_END(ni,iWidgetScrollBar,SetScrollRange,1)

/** ni -> iWidgetScrollBar::GetScrollRange/0 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,GetScrollRange,0)
	IDLC_DECL_RETVAR(sVec2f,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetScrollBar,GetScrollRange,0,())
	IDLC_RET_FROM_BASE(ni::eType_Vec2f,_Ret)
IDLC_METH_END(ni,iWidgetScrollBar,GetScrollRange,0)

/** ni -> iWidgetScrollBar::SetScrollPosition/1 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,SetScrollPosition,1)
	IDLC_DECL_VAR(tF32,afScrollPos)
	IDLC_BUF_TO_BASE(ni::eType_F32,afScrollPos)
	IDLC_METH_CALL_VOID(ni,iWidgetScrollBar,SetScrollPosition,1,(afScrollPos))
IDLC_METH_END(ni,iWidgetScrollBar,SetScrollPosition,1)

/** ni -> iWidgetScrollBar::GetScrollPosition/0 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,GetScrollPosition,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetScrollBar,GetScrollPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetScrollBar,GetScrollPosition,0)

/** ni -> iWidgetScrollBar::SetNormalizedScrollPosition/1 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,SetNormalizedScrollPosition,1)
	IDLC_DECL_VAR(tF32,afScrollPos)
	IDLC_BUF_TO_BASE(ni::eType_F32,afScrollPos)
	IDLC_METH_CALL_VOID(ni,iWidgetScrollBar,SetNormalizedScrollPosition,1,(afScrollPos))
IDLC_METH_END(ni,iWidgetScrollBar,SetNormalizedScrollPosition,1)

/** ni -> iWidgetScrollBar::GetNormalizedScrollPosition/0 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,GetNormalizedScrollPosition,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetScrollBar,GetNormalizedScrollPosition,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetScrollBar,GetNormalizedScrollPosition,0)

/** ni -> iWidgetScrollBar::SetPageSize/1 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,SetPageSize,1)
	IDLC_DECL_VAR(tF32,afPageSize)
	IDLC_BUF_TO_BASE(ni::eType_F32,afPageSize)
	IDLC_METH_CALL_VOID(ni,iWidgetScrollBar,SetPageSize,1,(afPageSize))
IDLC_METH_END(ni,iWidgetScrollBar,SetPageSize,1)

/** ni -> iWidgetScrollBar::GetPageSize/0 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,GetPageSize,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetScrollBar,GetPageSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetScrollBar,GetPageSize,0)

/** ni -> iWidgetScrollBar::ComputeRoundedPosition/1 **/
IDLC_METH_BEGIN(ni,iWidgetScrollBar,ComputeRoundedPosition,1)
	IDLC_DECL_VAR(tF32,afNewPos)
	IDLC_BUF_TO_BASE(ni::eType_F32,afNewPos)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetScrollBar,ComputeRoundedPosition,1,(afNewPos))
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetScrollBar,ComputeRoundedPosition,1)

IDLC_END_INTF(ni,iWidgetScrollBar)

IDLC_END_NAMESPACE()
// EOF //
