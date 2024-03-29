// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IWidgetMenu.h'.
//
#include "../API/niUI/IWidgetMenu.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iWidgetMenuItem **/
IDLC_BEGIN_INTF(ni,iWidgetMenuItem)
/** ni -> iWidgetMenuItem::GetMenu/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetMenu,0)
	IDLC_DECL_RETVAR(iWidgetMenu*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetMenu,0,())
	IDLC_RET_FROM_INTF(iWidgetMenu,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetMenu,0)

/** ni -> iWidgetMenuItem::SetName/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetName,1)
	IDLC_DECL_VAR(achar*,aVal)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetName,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetName,1)

/** ni -> iWidgetMenuItem::GetName/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetName,0)

/** ni -> iWidgetMenuItem::SetID/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetID,1)
	IDLC_DECL_VAR(iHString*,aVal)
	IDLC_BUF_TO_INTF(iHString,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetID,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetID,1)

/** ni -> iWidgetMenuItem::GetID/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetID,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetID,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetID,0)

/** ni -> iWidgetMenuItem::SetFlags/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetFlags,1)
	IDLC_DECL_VAR(tWidgetMenuItemFlags,aVal)
	IDLC_BUF_TO_ENUM(tWidgetMenuItemFlags,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetFlags,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetFlags,1)

/** ni -> iWidgetMenuItem::GetFlags/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetFlags,0)
	IDLC_DECL_RETVAR(tWidgetMenuItemFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetFlags,0,())
	IDLC_RET_FROM_ENUM(tWidgetMenuItemFlags,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetFlags,0)

/** ni -> iWidgetMenuItem::SetIcon/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetIcon,1)
	IDLC_DECL_VAR(iOverlay*,aVal)
	IDLC_BUF_TO_INTF(iOverlay,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetIcon,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetIcon,1)

/** ni -> iWidgetMenuItem::GetIcon/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetIcon,0)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetIcon,0,())
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetIcon,0)

/** ni -> iWidgetMenuItem::SetSubmenu/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetSubmenu,1)
	IDLC_DECL_VAR(iHString*,aVal)
	IDLC_BUF_TO_INTF(iHString,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetSubmenu,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetSubmenu,1)

/** ni -> iWidgetMenuItem::GetSubmenu/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetSubmenu,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetSubmenu,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetSubmenu,0)

/** ni -> iWidgetMenuItem::SetSelected/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetSelected,1)
	IDLC_DECL_VAR(tBool,aVal)
	IDLC_BUF_TO_BASE(ni::eType_I8,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetSelected,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetSelected,1)

/** ni -> iWidgetMenuItem::GetSelected/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetSelected,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetSelected,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetSelected,0)

/** ni -> iWidgetMenuItem::SetGroupID/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetGroupID,1)
	IDLC_DECL_VAR(iHString*,aVal)
	IDLC_BUF_TO_INTF(iHString,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetGroupID,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetGroupID,1)

/** ni -> iWidgetMenuItem::GetGroupID/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetGroupID,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetGroupID,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetGroupID,0)

/** ni -> iWidgetMenuItem::SetKey/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,SetKey,1)
	IDLC_DECL_VAR(eKey,aVal)
	IDLC_BUF_TO_ENUM(eKey,aVal)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,SetKey,1,(aVal))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,SetKey,1)

/** ni -> iWidgetMenuItem::GetKey/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenuItem,GetKey,0)
	IDLC_DECL_RETVAR(eKey,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenuItem,GetKey,0,())
	IDLC_RET_FROM_ENUM(eKey,_Ret)
IDLC_METH_END(ni,iWidgetMenuItem,GetKey,0)

IDLC_END_INTF(ni,iWidgetMenuItem)

/** interface : iWidgetMenu **/
IDLC_BEGIN_INTF(ni,iWidgetMenu)
/** ni -> iWidgetMenu::ClearItems/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,ClearItems,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,ClearItems,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenu,ClearItems,0)

/** ni -> iWidgetMenu::AddItem/3 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,AddItem,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_DECL_VAR(tWidgetMenuItemFlags,aFlags)
	IDLC_BUF_TO_ENUM(tWidgetMenuItemFlags,aFlags)
	IDLC_DECL_RETVAR(iWidgetMenuItem*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,AddItem,3,(aaszName,ahspID,aFlags))
	IDLC_RET_FROM_INTF(iWidgetMenuItem,_Ret)
IDLC_METH_END(ni,iWidgetMenu,AddItem,3)

/** ni -> iWidgetMenu::RemoveItem/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,RemoveItem,1)
	IDLC_DECL_VAR(iWidgetMenuItem*,apItem)
	IDLC_BUF_TO_INTF(iWidgetMenuItem,apItem)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,RemoveItem,1,(apItem))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenu,RemoveItem,1)

/** ni -> iWidgetMenu::GetNumItems/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetNumItems,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetNumItems,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetNumItems,0)

/** ni -> iWidgetMenu::GetItem/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetItem,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iWidgetMenuItem*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetItem,1,(anIndex))
	IDLC_RET_FROM_INTF(iWidgetMenuItem,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetItem,1)

/** ni -> iWidgetMenu::GetItemFromName/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetItemFromName,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(iWidgetMenuItem*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetItemFromName,1,(aaszName))
	IDLC_RET_FROM_INTF(iWidgetMenuItem,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetItemFromName,1)

/** ni -> iWidgetMenu::GetItemFromID/1 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetItemFromID,1)
	IDLC_DECL_VAR(iHString*,ahspID)
	IDLC_BUF_TO_INTF(iHString,ahspID)
	IDLC_DECL_RETVAR(iWidgetMenuItem*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetItemFromID,1,(ahspID))
	IDLC_RET_FROM_INTF(iWidgetMenuItem,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetItemFromID,1)

/** ni -> iWidgetMenu::Open/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,Open,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,Open,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenu,Open,0)

/** ni -> iWidgetMenu::Close/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,Close,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,Close,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenu,Close,0)

/** ni -> iWidgetMenu::SortItems/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,SortItems,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,SortItems,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iWidgetMenu,SortItems,0)

/** ni -> iWidgetMenu::GetMenuWidth/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetMenuWidth,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetMenuWidth,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetMenuWidth,0)

/** ni -> iWidgetMenu::GetMenuHeight/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetMenuHeight,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetMenuHeight,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetMenuHeight,0)

/** ni -> iWidgetMenu::GetItemWidth/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetItemWidth,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetItemWidth,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetItemWidth,0)

/** ni -> iWidgetMenu::GetItemHeight/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,GetItemHeight,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iWidgetMenu,GetItemHeight,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iWidgetMenu,GetItemHeight,0)

/** ni -> iWidgetMenu::UpdateSizes/0 **/
IDLC_METH_BEGIN(ni,iWidgetMenu,UpdateSizes,0)
	IDLC_METH_CALL_VOID(ni,iWidgetMenu,UpdateSizes,0,())
IDLC_METH_END(ni,iWidgetMenu,UpdateSizes,0)

IDLC_END_INTF(ni,iWidgetMenu)

IDLC_END_NAMESPACE()
// EOF //
