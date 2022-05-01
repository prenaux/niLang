#include "../API/niUI/IWidgetLabel.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iWidgetLabel **/
IDLC_BEGIN_INTF(ni,iWidgetLabel)
/** ni -> iWidgetLabel::SetFontFormatFlags/1 **/
IDLC_METH_BEGIN(ni,iWidgetLabel,SetFontFormatFlags,1)
	IDLC_DECL_VAR(tFontFormatFlags,aFlags)
	IDLC_BUF_TO_ENUM(tFontFormatFlags,aFlags)
	IDLC_METH_CALL_VOID(ni,iWidgetLabel,SetFontFormatFlags,1,(aFlags))
IDLC_METH_END(ni,iWidgetLabel,SetFontFormatFlags,1)

/** ni -> iWidgetLabel::GetFontFormatFlags/0 **/
IDLC_METH_BEGIN(ni,iWidgetLabel,GetFontFormatFlags,0)
	IDLC_DECL_RETVAR(tFontFormatFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetLabel,GetFontFormatFlags,0,())
	IDLC_RET_FROM_ENUM(tFontFormatFlags,_Ret)
IDLC_METH_END(ni,iWidgetLabel,GetFontFormatFlags,0)

IDLC_END_INTF(ni,iWidgetLabel)

IDLC_END_NAMESPACE()
// EOF //
