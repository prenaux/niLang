#ifndef __ITABWIDGET_3249727_H__
#define __ITABWIDGET_3249727_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Tab widget notify message.
enum eWidgetTabCmd
{
  //! A page has been activated.
  eWidgetTabCmd_ActivatePage = 0,
  //! A page has been added.
  eWidgetTabCmd_AddPage = 1,
  //! A page has been removed.
  eWidgetTabCmd_RemovePage = 2,
  //! \internal
  eWidgetTabCmd_ForceDWORD = 0xFFFFFFFF
};

//! Tab widget.
struct iWidgetTab : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetTab,0xd7908465,0xa722,0x4bd3,0xbd,0xe1,0x27,0xe7,0xc2,0xc9,0x58,0x19)

  //########################################################################################
  //! \name Page management
  //########################################################################################
  //! @{

  //! Add a page.
  virtual void __stdcall AddPage(iHString* ahspName, iWidget *apPage) = 0;
  //! Remove a page.
  virtual tBool __stdcall RemovePage(iWidget* apPage) = 0;

  //! Check if the specified page is in the tab.
  virtual tBool __stdcall HasPage(iWidget* apPage) const = 0;

  //! Get the number of pages.
  //! {Property}
  virtual tU32 __stdcall GetNumPages() const = 0;
  //! Get the page at the specified index.
  //! {Property}
  virtual iWidget* __stdcall GetPage(tU32 anIndex) const = 0;
  //! Get the button associated with the page at the specified index.
  //! {Property}
  virtual iWidget* __stdcall GetPageButton(tU32 anIndex) const = 0;
  //! Get the page with the specified name.
  //! {Property}
  virtual iWidget* __stdcall GetPageFromName(iHString* ahspName) const = 0;
  //! Get the page with the specified ID.
  //! {Property}
  virtual iWidget* __stdcall GetPageFromID(iHString* ahspID) const = 0;

  //! Set the name of the specified page.
  //! {Property}
  virtual tBool __stdcall SetPageName(iWidget* apPage, iHString* ahspName) = 0;
  //! Get the name of the specified page.
  //! {Property}
  virtual iHString* __stdcall GetPageName(iWidget* apPage) const = 0;

  //! Set the index of the specified page.
  //! {Property}
  //! \remark Will swap with the page at the specified index.
  virtual tBool __stdcall SetPageIndex(iWidget* apPage, tU32 anIndex) = 0;
  //! Get the index of the specified page.
  //! {Property}
  virtual tU32 __stdcall GetPageIndex(iWidget* apPage) const = 0;

  //! Move the specified type to the left.
  virtual tBool __stdcall MovePageLeft(iWidget* apPage, tU32 anLeft) = 0;
  //! Move the specified type to the right.
  virtual tBool __stdcall MovePageRight(iWidget* apPage, tU32 anCount) = 0;

  //! Set the number of pages necessary to show the tabs.
  //! {Property}
  //! \remark Default is two. If set to <= 1 when any page is added the tabs are displayed, otherwise can be set to 2 for example to show the tab page only if they are at least two pages visible.
  virtual void __stdcall SetMinNumPagesToShowTabs(tU32 anMinNumPages) = 0;
  //! Get the number of pages necessary to show the tabs.
  //! {Property}
  virtual tU32 __stdcall GetMinNumPagesToShowTabs() const = 0;
  //! @}

  //########################################################################################
  //! \name Active page
  //########################################################################################
  //! @{

  //! Activate the specified page.
  //! {Property}
  virtual tBool __stdcall SetActivePage(iWidget* apPage) = 0;
  //! Get the active page.
  //! {Property}
  virtual iWidget* __stdcall GetActivePage() const = 0;
  //! Activate the page with the specified name.
  //! {Property}
  virtual tBool __stdcall SetActivePageName(iHString* ahspName) = 0;
  //! Get the name of the active page.
  //! {Property}
  virtual iHString* __stdcall GetActivePageName() const = 0;
  //! Activate the page with the specified index.
  //! {Property}
  virtual tBool __stdcall SetActivePageIndex(tU32 anIndex) = 0;
  //! Get the index of the active page.
  //! {Property}
  virtual tU32 __stdcall GetActivePageIndex() const = 0;
  //! Activate the page with the specified id.
  //! {Property}
  virtual tBool __stdcall SetActivePageID(iHString* ahspName) = 0;
  //! Get the id of the active page.
  //! {Property}
  virtual iHString* __stdcall GetActivePageID() const = 0;

  //! Set the forced activated state.
  //! {Property}
  virtual void __stdcall SetForceActivated(tBool abForce) = 0;
  //! Get the forced activated state.
  //! {Property}
  virtual tBool __stdcall GetForceActivated(tBool abForce) const = 0;
  //! @}

};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ITABWIDGET_3249727_H__
