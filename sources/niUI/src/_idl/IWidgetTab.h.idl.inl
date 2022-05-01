#include "../API/niUI/IWidgetTab.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iWidgetTab **/
IDLC_BEGIN_INTF(ni,iWidgetTab)
/** ni -> iWidgetTab::AddPage/2 **/
IDLC_METH_BEGIN(ni,iWidgetTab,AddPage,2)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_METH_CALL_VOID(ni,iWidgetTab,AddPage,2,(ahspName,apPage))
IDLC_METH_END(ni,iWidgetTab,AddPage,2)

/** ni -> iWidgetTab::RemovePage/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,RemovePage,1)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,RemovePage,1,(apPage))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,RemovePage,1)

/** ni -> iWidgetTab::HasPage/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,HasPage,1)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,HasPage,1,(apPage))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,HasPage,1)

/** ni -> iWidgetTab::GetNumPages/0 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetNumPages,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetNumPages,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetNumPages,0)

/** ni -> iWidgetTab::GetPage/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetPage,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetPage,1,(anIndex))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetPage,1)

/** ni -> iWidgetTab::GetPageButton/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetPageButton,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetPageButton,1,(anIndex))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetPageButton,1)

/** ni -> iWidgetTab::GetPageFromName/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetPageFromName,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetPageFromName,1,(ahspName))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetPageFromName,1)

/** ni -> iWidgetTab::GetPageFromID/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetPageFromID,1)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetPageFromID,1,(ahspID))
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetPageFromID,1)

/** ni -> iWidgetTab::SetPageName/2 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetPageName,2)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,SetPageName,2,(apPage,ahspName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,SetPageName,2)

/** ni -> iWidgetTab::GetPageName/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetPageName,1)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetPageName,1,(apPage))
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetPageName,1)

/** ni -> iWidgetTab::SetPageIndex/2 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetPageIndex,2)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,SetPageIndex,2,(apPage,anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,SetPageIndex,2)

/** ni -> iWidgetTab::GetPageIndex/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetPageIndex,1)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetPageIndex,1,(apPage))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetPageIndex,1)

/** ni -> iWidgetTab::MovePageLeft/2 **/
IDLC_METH_BEGIN(ni,iWidgetTab,MovePageLeft,2)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_VAR(tU32,anLeft)
	IDLC_BUF_TO_BASE(ni::eType_U32,anLeft)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,MovePageLeft,2,(apPage,anLeft))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,MovePageLeft,2)

/** ni -> iWidgetTab::MovePageRight/2 **/
IDLC_METH_BEGIN(ni,iWidgetTab,MovePageRight,2)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_VAR(tU32,anCount)
	IDLC_BUF_TO_BASE(ni::eType_U32,anCount)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,MovePageRight,2,(apPage,anCount))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,MovePageRight,2)

/** ni -> iWidgetTab::SetMinNumPagesToShowTabs/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetMinNumPagesToShowTabs,1)
	IDLC_DECL_VAR(tU32,anMinNumPages)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMinNumPages)
	IDLC_METH_CALL_VOID(ni,iWidgetTab,SetMinNumPagesToShowTabs,1,(anMinNumPages))
IDLC_METH_END(ni,iWidgetTab,SetMinNumPagesToShowTabs,1)

/** ni -> iWidgetTab::GetMinNumPagesToShowTabs/0 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetMinNumPagesToShowTabs,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetMinNumPagesToShowTabs,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetMinNumPagesToShowTabs,0)

/** ni -> iWidgetTab::SetActivePage/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetActivePage,1)
	IDLC_DECL_VAR(iWidget*,apPage)
	IDLC_BUF_TO_INTF(iWidget,apPage)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,SetActivePage,1,(apPage))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,SetActivePage,1)

/** ni -> iWidgetTab::GetActivePage/0 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetActivePage,0)
	IDLC_DECL_RETVAR(iWidget*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetActivePage,0,())
	IDLC_RET_FROM_INTF(iWidget,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetActivePage,0)

/** ni -> iWidgetTab::SetActivePageName/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetActivePageName,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,SetActivePageName,1,(ahspName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,SetActivePageName,1)

/** ni -> iWidgetTab::GetActivePageName/0 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetActivePageName,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetActivePageName,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetActivePageName,0)

/** ni -> iWidgetTab::SetActivePageIndex/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetActivePageIndex,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,SetActivePageIndex,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,SetActivePageIndex,1)

/** ni -> iWidgetTab::GetActivePageIndex/0 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetActivePageIndex,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetActivePageIndex,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetActivePageIndex,0)

/** ni -> iWidgetTab::SetActivePageID/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetActivePageID,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,SetActivePageID,1,(ahspName))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,SetActivePageID,1)

/** ni -> iWidgetTab::GetActivePageID/0 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetActivePageID,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetActivePageID,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetActivePageID,0)

/** ni -> iWidgetTab::SetForceActivated/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,SetForceActivated,1)
	IDLC_DECL_VAR(tBool,abForce)
	IDLC_BUF_TO_BASE(ni::eType_I8,abForce)
	IDLC_METH_CALL_VOID(ni,iWidgetTab,SetForceActivated,1,(abForce))
IDLC_METH_END(ni,iWidgetTab,SetForceActivated,1)

/** ni -> iWidgetTab::GetForceActivated/1 **/
IDLC_METH_BEGIN(ni,iWidgetTab,GetForceActivated,1)
	IDLC_DECL_VAR(tBool,abForce)
	IDLC_BUF_TO_BASE(ni::eType_I8,abForce)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetTab,GetForceActivated,1,(abForce))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetTab,GetForceActivated,1)

IDLC_END_INTF(ni,iWidgetTab)

IDLC_END_NAMESPACE()
// EOF //
