#include "../API/niUI/IWidgetGroup.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iWidgetGroup **/
IDLC_BEGIN_INTF(ni,iWidgetGroup)
/** ni -> iWidgetGroup::SetFolded/1 **/
IDLC_METH_BEGIN(ni,iWidgetGroup,SetFolded,1)
	IDLC_DECL_VAR(tBool,abFolded)
	IDLC_BUF_TO_BASE(ni::eType_I8,abFolded)
	IDLC_METH_CALL_VOID(ni,iWidgetGroup,SetFolded,1,(abFolded))
IDLC_METH_END(ni,iWidgetGroup,SetFolded,1)

/** ni -> iWidgetGroup::GetFolded/0 **/
IDLC_METH_BEGIN(ni,iWidgetGroup,GetFolded,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetGroup,GetFolded,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetGroup,GetFolded,0)

IDLC_END_INTF(ni,iWidgetGroup)

IDLC_END_NAMESPACE()
// EOF //
