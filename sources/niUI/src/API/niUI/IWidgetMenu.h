#ifndef __IWIDGETMENU_2975466_H__
#define __IWIDGETMENU_2975466_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

struct iWidgetMenu;

/** \addtogroup niUI
 * @{
 */

//! Menu item flags.
enum eWidgetMenuItemFlags
{
  //! No flags.
  eWidgetMenuItemFlags_None = 0,
  //! The item will open a submenu.
  eWidgetMenuItemFlags_SubMenu = niBit(1),
  //! The item is a separator.
  eWidgetMenuItemFlags_Separator = niBit(2),
  //! The item is a check.
  eWidgetMenuItemFlags_Check = niBit(3),
  //! The item is in a group.
  eWidgetMenuItemFlags_Group = niBit(4),
  //! The item is a dummy, aka it isnt clickable.
  eWidgetMenuItemFlags_Dummy = niBit(5),
  //! The item is a title, will be drawn with a different background color.
  eWidgetMenuItemFlags_Title = niBit(6),
  //! The item is disabled, text will be drawn in a different color.
  eWidgetMenuItemFlags_Disabled = niBit(7),
  //! Sort when adding the item.
  eWidgetMenuItemFlags_SortAdd = niBit(8),
  //! \internal
  eWidgetMenuItemFlags_ForceDWORD = 0xFFFFFFFF
};

//! Menu notify message
//! \remark Sent through eUIMessage_Command to the parent widget,
//!     extra1 contains the iWidgetMenuItem of the item concerned.
//! \remark Sub menus are considered part of the parent menu, they
//!     will send messages as if they were the parent menu.
enum eWidgetMenuCmd
{
  //! A check menu item has been checked.
  //! \param A: iWidgetMenuItem
  eWidgetMenuCmd_Checked = 0,
  //! A check menu item has been unchecked.
  //! \param A: iWidgetMenuItem
  eWidgetMenuCmd_UnChecked = 1,
  //! A group menu item has been changed.
  //! \param A: iWidgetMenuItem
  eWidgetMenuCmd_GroupChanged = 2,
  //! A menu item has been clicked.
  //! \param A: iWidgetMenuItem
  eWidgetMenuCmd_Clicked = 3,
  //! Menu has just been opened.
  //! \param A: iWidgetMenu opened
  eWidgetMenuCmd_Opened = 4,
  //! Menu has just been closed.
  //! \param A: iWidgetMenu closed
  eWidgetMenuCmd_Closed = 5,
  //! The menu item focused has changed.
  //! \param A: newly selected iWidgetMenuItem
  //! \param B: previously selected iWidgetMenuItem if any
  eWidgetMenuCmd_FocusedItem = 6,
  //! \internal
  eWidgetMenuCmd_ForceDWORD = 0xFFFFFFFF
};

//! Menu style.
//! \remark The menu style is used only by the parent menu, sub menus will ignore
//!     these flags.
enum eWidgetMenuStyle
{
  //! Dont close the menu on group change.
  eWidgetMenuStyle_NoCloseOnGroupChange = niBit(eWidgetStyle_MaxBit+0),
  //! Dont close the menu on check change.
  eWidgetMenuStyle_NoCloseOnCheckChange = niBit(eWidgetStyle_MaxBit+1),
  //! Dont close the menu on item clicked.
  eWidgetMenuStyle_NoCloseOnItemClicked = niBit(eWidgetStyle_MaxBit+2),
  //! Dont close the menu when clicking outside of the client area.
  eWidgetMenuStyle_NoCloseOnNCClick = niBit(eWidgetStyle_MaxBit+3),
  //! Dont close the menu automatically
  eWidgetMenuStyle_NoClose = eWidgetMenuStyle_NoCloseOnGroupChange|eWidgetMenuStyle_NoCloseOnCheckChange|eWidgetMenuStyle_NoCloseOnItemClicked|eWidgetMenuStyle_NoCloseOnNCClick,
  //! Dont close the menu on group or check change.
  eWidgetMenuStyle_NoCloseOnGroupOrCheckChange = eWidgetMenuStyle_NoCloseOnGroupChange|eWidgetMenuStyle_NoCloseOnCheckChange,
  //! \internal
  eWidgetMenuStyle_ForceDWORD = 0xFFFFFFFF
};

//! Menu item flags type.
typedef tU32 tWidgetMenuItemFlags;

//! Menu item interface.
struct iWidgetMenuItem : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetMenuItem,0x5c063f02,0x4c0e,0x4828,0x9d,0x0a,0xec,0xca,0xc5,0xf3,0x92,0xb0)
  //! Get the item's menu.
  //! {Property}
  virtual iWidgetMenu* __stdcall GetMenu() const = 0;
  //! Set the item's name.
  //! {Property}
  virtual tBool __stdcall SetName(const achar* aVal) = 0;
  //! Get the item's name.
  //! {Property}
  virtual const achar* __stdcall GetName() const = 0;
  //! Set the item's ID.
  //! {Property}
  virtual tBool __stdcall SetID(iHString* aVal) = 0;
  //! Get the item's ID.
  //! {Property}
  virtual iHString* __stdcall GetID() const = 0;
  //! Set the item's flags.
  //! {Property}
  virtual tBool __stdcall SetFlags(tWidgetMenuItemFlags aVal) = 0;
  //! Get the item's flags.
  //! {Property}
  virtual tWidgetMenuItemFlags __stdcall GetFlags() const = 0;
  //! Set the item's icon
  //! {Property}
  virtual tBool __stdcall SetIcon(iOverlay* aVal) = 0;
  //! Get the item's icon
  //! {Property}
  virtual iOverlay* __stdcall GetIcon() const = 0;
  //! Set the item's submenu ID.
  //! {Property}
  virtual tBool __stdcall SetSubmenu(iHString* aVal) = 0;
  //! Get the item's submenu ID.
  //! {Property}
  virtual iHString* __stdcall GetSubmenu() const = 0;
  //! Set the item's selected status.
  //! {Property}
  virtual tBool __stdcall SetSelected(tBool aVal) = 0;
  //! Get the item's selected status.
  //! {Property}
  virtual tBool __stdcall GetSelected() const = 0;
  //! Set the item's group id.
  //! {Property}
  virtual tBool __stdcall SetGroupID(iHString* aVal) = 0;
  //! Get the item's group id.
  //! {Property}
  virtual iHString* __stdcall GetGroupID() const = 0;
  //! Set the item's key shortcut.
  //! {Property}
  virtual tBool __stdcall SetKey(eKey aVal) = 0;
  //! Get the item's key shortcut.
  //! {Property}
  virtual eKey __stdcall GetKey() const = 0;
};

//! Menu interface.
//! \remark The keyboard input for the menu is :<br>
//!     Escape : Close the current menu.<br>
//!     Home/End : Select the first/last menu item.<br>
//!     Up/Down : Select the previous/next menu item.<br>
//!     Ctrl/Shit+Up/Down or PgUp/PgDn : Select the next menu item before/after a separator.<br>
//!     Left : Close the current sub-menu. Wont close the main menu.<br>
//!     Right : Open the current sub-menu.<br>
//!     Enter/Space : Select/Click the current menu item.<br>
//! \remark If the NoCloseOnNCClick style is set the main menu will not close when escape is pressed.
struct iWidgetMenu : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetMenu,0x1bbb53a4,0xf078,0x4d46,0x87,0xd4,0xdd,0x86,0xde,0xad,0x1b,0x95)

  //! Clear/removes all the items in the menu.
  virtual tBool __stdcall ClearItems() = 0;
  //! Add a menu item.
  //! \param  aaszName is the name and text of the item. It can contain a & that will notify
  //!         the shortcut key to use.
  //! \param  ahspID is the ID of the item.
  //! \param  aFlags are the flags of the item.
  //! \return The index of the item, or eInvalidHandle if the item can't be added.
  virtual iWidgetMenuItem* __stdcall AddItem(const achar* aaszName, iHString* ahspID, tWidgetMenuItemFlags aFlags) = 0;
  //! Remove the item at the specified index.
  //! \param  apItem is the item to remove.
  //! \return eFalse if the index is invalid, eTrue if the item has been removed.
  //! \remark This function will search the item in the sub menus.
  virtual tBool __stdcall RemoveItem(iWidgetMenuItem* apItem) = 0;
  //! Get the number of items.
  //! \remark This function doesn't include the sub menus.
  //! {Property}
  virtual tU32 __stdcall GetNumItems() const = 0;
  //! Get the item at the specified index.
  //! \remark This function doesn't include the sub menus.
  //! {Property}
  virtual iWidgetMenuItem* __stdcall GetItem(tU32 anIndex) const = 0;
  //! Get the index of the first item with the specified name.
  //! \remark This function will search the item in the sub menus.
  //! {Property}
  virtual iWidgetMenuItem* __stdcall GetItemFromName(const achar* aaszName) const = 0;
  //! Get the index of the first item with the specified id.
  //! \remark This function will search the item in the sub menus.
  //! {Property}
  virtual iWidgetMenuItem* __stdcall GetItemFromID(iHString* ahspID) const = 0;
  //! Open the menu.
  virtual tBool __stdcall Open() = 0;
  //! Close the menu.
  virtual tBool __stdcall Close() = 0;
  //! Sort the items.
  virtual tBool __stdcall SortItems() = 0;

  //! Get the menu's width in pixels.
  //! {Property}
  virtual tU32 __stdcall GetMenuWidth() const = 0;
  //! Get the menu's height in pixels.
  //! {Property}
  virtual tU32 __stdcall GetMenuHeight() const = 0;
  //! Get the item's width in pixels.
  //! {Property}
  virtual tU32 __stdcall GetItemWidth() const = 0;
  //! Get the item's height in pixels.
  //! {Property}
  virtual tU32 __stdcall GetItemHeight() const = 0;

  //! Update the menu's sizes.
  //! \remark If the item list changed since the last call this is done
  //!         automatically when Open, GetMenuWidth/Height or
  //!         GetItemWidth/Height is called.
  virtual void __stdcall UpdateSizes() = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETMENU_2975466_H__
