#include "../API/niUI/IWidgetComboBox.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iWidgetComboBox **/
IDLC_BEGIN_INTF(ni,iWidgetComboBox)
/** ni -> iWidgetComboBox::SetNumLines/1 **/
IDLC_METH_BEGIN(ni,iWidgetComboBox,SetNumLines,1)
	IDLC_DECL_VAR(tU32,anNum)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNum)
	IDLC_METH_CALL_VOID(ni,iWidgetComboBox,SetNumLines,1,(anNum))
IDLC_METH_END(ni,iWidgetComboBox,SetNumLines,1)

/** ni -> iWidgetComboBox::GetNumLines/0 **/
IDLC_METH_BEGIN(ni,iWidgetComboBox,GetNumLines,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetComboBox,GetNumLines,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetComboBox,GetNumLines,0)

/** ni -> iWidgetComboBox::SetDroppedWidget/1 **/
IDLC_METH_BEGIN(ni,iWidgetComboBox,SetDroppedWidget,1)
	IDLC_DECL_VAR(iWidget*,apWidget)
	IDLC_BUF_TO_INTF(iWidget,apWidget)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetComboBox,SetDroppedWidget,1,(apWidget))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetComboBox,SetDroppedWidget,1)

/** ni -> iWidgetComboBox::GetDroppedWidget/0 **/
IDLC_METH_BEGIN(ni,iWidgetComboBox,GetDroppedWidget,0)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetComboBox,GetDroppedWidget,0,())
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetComboBox,GetDroppedWidget,0)

IDLC_END_INTF(ni,iWidgetComboBox)

IDLC_END_NAMESPACE()
// EOF //
