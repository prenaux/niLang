// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IWidgetCanvas.h'.
//
#include "../API/niUI/IWidgetCanvas.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iWidgetCanvas **/
IDLC_BEGIN_INTF(ni,iWidgetCanvas)
/** ni -> iWidgetCanvas::GetScrollV/0 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,GetScrollV,0)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCanvas,GetScrollV,0,())
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetCanvas,GetScrollV,0)

/** ni -> iWidgetCanvas::SetScrollStepV/1 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,SetScrollStepV,1)
	IDLC_DECL_VAR(tF32,afV)
	IDLC_BUF_TO_BASE(ni::eType_F32,afV)
	IDLC_METH_CALL_VOID(ni,iWidgetCanvas,SetScrollStepV,1,(afV))
IDLC_METH_END(ni,iWidgetCanvas,SetScrollStepV,1)

/** ni -> iWidgetCanvas::GetScrollStepV/0 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,GetScrollStepV,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCanvas,GetScrollStepV,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetCanvas,GetScrollStepV,0)

/** ni -> iWidgetCanvas::SetScrollMarginV/1 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,SetScrollMarginV,1)
	IDLC_DECL_VAR(tF32,afV)
	IDLC_BUF_TO_BASE(ni::eType_F32,afV)
	IDLC_METH_CALL_VOID(ni,iWidgetCanvas,SetScrollMarginV,1,(afV))
IDLC_METH_END(ni,iWidgetCanvas,SetScrollMarginV,1)

/** ni -> iWidgetCanvas::GetScrollMarginV/0 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,GetScrollMarginV,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCanvas,GetScrollMarginV,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetCanvas,GetScrollMarginV,0)

/** ni -> iWidgetCanvas::GetScrollH/0 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,GetScrollH,0)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCanvas,GetScrollH,0,())
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetCanvas,GetScrollH,0)

/** ni -> iWidgetCanvas::SetScrollStepH/1 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,SetScrollStepH,1)
	IDLC_DECL_VAR(tF32,afV)
	IDLC_BUF_TO_BASE(ni::eType_F32,afV)
	IDLC_METH_CALL_VOID(ni,iWidgetCanvas,SetScrollStepH,1,(afV))
IDLC_METH_END(ni,iWidgetCanvas,SetScrollStepH,1)

/** ni -> iWidgetCanvas::GetScrollStepH/0 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,GetScrollStepH,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCanvas,GetScrollStepH,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetCanvas,GetScrollStepH,0)

/** ni -> iWidgetCanvas::SetScrollMarginH/1 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,SetScrollMarginH,1)
	IDLC_DECL_VAR(tF32,afV)
	IDLC_BUF_TO_BASE(ni::eType_F32,afV)
	IDLC_METH_CALL_VOID(ni,iWidgetCanvas,SetScrollMarginH,1,(afV))
IDLC_METH_END(ni,iWidgetCanvas,SetScrollMarginH,1)

/** ni -> iWidgetCanvas::GetScrollMarginH/0 **/
IDLC_METH_BEGIN(ni,iWidgetCanvas,GetScrollMarginH,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetCanvas,GetScrollMarginH,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iWidgetCanvas,GetScrollMarginH,0)

IDLC_END_INTF(ni,iWidgetCanvas)

IDLC_END_NAMESPACE()
// EOF //
