#ifndef __IWIDGETTOOLBAR_35226412_H__
#define __IWIDGETTOOLBAR_35226412_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! Toolbar group widget interface.
struct iWidgetToolbarGroup : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetToolbarGroup,0xf0ad342b,0x5139,0x4dea,0x98,0x8f,0x1e,0xfa,0x94,0x87,0x16,0x59)

  //! Set the width of the group from the total with of the children.
  virtual void __stdcall SetWidthFromChildren() = 0;

  //! Set the destination message handler for the commands received by the group.
  //! {Property}
  //! \remark Set to eInvalidHandle to disable.
  virtual void __stdcall SetCommandDestination(iMessageHandler* apMT) = 0;
  //! Get the destination message handler for the commands received by the toolbar.
  //! {Property}
  virtual iMessageHandler* __stdcall GetCommandDestination() const = 0;
};

//! Toolbar widget interface.
struct iWidgetToolbar : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetToolbar,0x8e50495c,0x6577,0x4693,0x80,0x59,0x8e,0x9b,0xab,0x98,0xde,0x8e)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Set the toolbar's height in pixels.
  //! {Property}
  //! \remark Default is 120 pixels.
  virtual void __stdcall SetHeight(ni::tF32 afHeight) = 0;
  //! Get the toolbar's height.
  //! {Property}
  virtual ni::tF32 __stdcall GetHeight() const = 0;
  //! Set whether the toolbar should auto-hide.
  //! {Property}
  //! \remark Default is true.
  virtual void __stdcall SetAutoHide(ni::tBool abAutoHide) = 0;
  //! Get whether the toolbar should auto-hide.
  //! {Property}
  virtual ni::tBool __stdcall GetAutoHide() const = 0;
  //! Set whether the toolbar should draw the bottom background when unfolded in auto-hide mode.
  //! {Property}
  //! \remark Default is false.
  virtual void __stdcall SetAutoHideBackground(ni::tBool abAutoHide) = 0;
  //! Get whether the toolbar should draw the bottom background when unfolded in auto-hide mode.
  //! {Property}
  virtual ni::tBool __stdcall GetAutoHideBackground() const = 0;
  //! Set whether the toolbar's active page should be visible.
  //! {Property}
  virtual void __stdcall SetShowPage(ni::tBool abShowPage) = 0;
  //! Get whether the toolbar's active page is visible.
  //! {Property}
  virtual ni::tBool __stdcall GetShowPage() const = 0;

  //! Get the top bar widget.
  //! {Property}
  virtual iWidget* __stdcall GetTopBar() const = 0;
  //! Get the bottom bar widget.
  //! {Property}
  virtual iWidget* __stdcall GetBottomBar() const = 0;

  //! Set the destination message target for the commands received by the toolbar.
  //! {Property}
  //! \remark Set to eInvalidHandle to disable.
  virtual void __stdcall SetCommandDestination(iMessageHandler* apMH) = 0;
  //! Get the destination message for the commands received by the toolbar.
  //! {Property}
  virtual iMessageHandler* __stdcall GetCommandDestination() const = 0;

  //! Updates the toolbar.
  //! \remark Removes all empty or invalid pages, removes all invalid top widgets and recomputes the layout.
  //! \remark Should be called after invalidating any toolbar group.
  virtual void __stdcall UpdateToolbar() = 0;
  //! Hide the toolbar pop-up.
  //! \remark Hides the toolbar pop-up shown in "AutoHide" mode.
  virtual void __stdcall HidePopup() = 0;
  //! @}

  //########################################################################################
  //! \name Pages
  //########################################################################################
  //! @{

  //! Get the number of pages in the toolbar.
  //! {Property}
  virtual ni::tU32 __stdcall GetNumPages() const = 0;
  //! Get the widget of the page at the specified index.
  //! {Property}
  virtual ni::iWidget* __stdcall GetPageWidget(ni::tU32 anIndex) const = 0;
  //! Get the index of the page with the specified ID.
  //! {Property}
  virtual ni::tU32 __stdcall GetPageIndex(ni::iHString* ahspID) const = 0;

  //! Initialize/Add a toolbar with the specified ID and name.
  //! \remark Empty and invalid pages are removed automatically when UpdateToolbar is called.
  virtual iWidget* __stdcall InitPage(iHString* ahspID, iHString* ahspName) = 0;

  //! Set the name/title of the specified page.
  //! {Property}
  virtual ni::tBool __stdcall SetPageName(iHString* ahspID, iHString* ahspName) = 0;
  //! Get the name/title of the specified page.
  //! {Property}
  virtual iHString* __stdcall GetPageName(iHString* ahspID) const = 0;

  //! Set whether the specifed page is enabled.
  //! {Property}
  virtual ni::tBool __stdcall SetPageEnabled(iHString* ahspID, tBool abEnabled) = 0;
  //! Get whether the specifed page is enabled.
  //! {Property}
  virtual ni::tBool __stdcall GetPageEnabled(iHString* ahspID) const = 0;

  //! Set the active's page index.
  //! {Property}
  virtual ni::tBool __stdcall SetActivePageIndex(ni::tU32 anActivePage) = 0;
  //! Get the active's pages index.
  //! {Property}
  virtual ni::tU32 __stdcall GetActivePageIndex() const = 0;
  //! Set the active's page ID.
  //! {Property}
  virtual ni::tBool __stdcall SetActivePageID(ni::iHString* ahspID) = 0;
  //! Get the active's page ID.
  //! {Property}
  virtual ni::iHString* __stdcall GetActivePageID() const = 0;
  //! @}

  //########################################################################################
  //! \name Top bar
  //########################################################################################
  //! @{

  //! Get the number of widgets in the top bar.
  //! {Property}
  virtual tU32 __stdcall GetNumTopWidgets() const = 0;
  //! Get the top bar widget at the specified index.
  //! {Property}
  virtual iWidget* __stdcall GetTopWidget(tU32 anIndex) const = 0;
  //! Get the index of the specified top bar widget.
  //! {Property}
  virtual tU32 __stdcall GetTopWidgetIndex(iWidget* apWidget) const = 0;
  //! Get the top bar widget with the specified ID.
  //! {Property}
  virtual iWidget* __stdcall GetTopWidgetFromID(iHString* ahspID) const = 0;
  //! Set the command destination of the specified top widget.
  //! {Property}
  virtual tBool __stdcall SetTopWidgetCommandDestination(tU32 anIndex, iMessageHandler* apCmdDest) = 0;
  //! Get the command destination of the specified top widget.
  //! {Property}
  virtual iMessageHandler* __stdcall GetTopWidgetCommandDestination(tU32 anIndex) const  = 0;
  //! Add a top bar widget.
  //! \remark Invalid top bar widgets are removed automatically when UpdateToolbar is called.
  virtual tBool __stdcall AddTopWidget(iWidget* apWidget, tF32 afWidth, iMessageHandler* apCmdDest) = 0;
  //! @}

  //########################################################################################
  //! \name Groups
  //########################################################################################
  //! @{

  //! Add a group in the specified page. If the page doesn't already exist it is added.
  //! \remark Calls UpdateToolbar
  virtual iWidget* __stdcall AddGroup(iHString* ahspPageID, iHString* ahspPageName, iHString* ahspGroupID, iHString* ahspGroupName, iMessageHandler* apCmdDest) = 0;
  //! Get the group with the specified ID.
  //! {Property}
  virtual iWidget* __stdcall GetGroup(ni::iHString* ahspID) const = 0;
  //! Remove the specified group.
  //! \remark Calls UpdateToolbar
  virtual tBool __stdcall RemoveGroup(ni::iHString* ahspID) = 0;
  //! @}
};

#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETTOOLBAR_35226412_H__
