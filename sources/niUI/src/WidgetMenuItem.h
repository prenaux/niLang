#ifndef __WIDGETMENUITEM_14781300_H__
#define __WIDGETMENUITEM_14781300_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetMenuItem declaration.
class cWidgetMenuItem : public ni::cIUnknownImpl<ni::iWidgetMenuItem,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cWidgetMenuItem);

 public:
  //! Constructor.
  cWidgetMenuItem(iWidget* apwMenu);
  //! Destructor.
  ~cWidgetMenuItem();

  void ZeroMembers();
  ni::tBool __stdcall IsOK() const;
  void __stdcall Invalidate();

  //// ni::iWidgetMenuItem /////////////////////
  iWidgetMenu* __stdcall GetMenu() const;
  tBool __stdcall SetName(const achar *aVal);
  const achar* __stdcall GetName() const;
  const achar* __stdcall GetLocalizedName() const;
  tBool __stdcall SetID(iHString* aVal);
  iHString* __stdcall GetID() const;
  tBool __stdcall SetFlags(tWidgetMenuItemFlags aVal);
  tWidgetMenuItemFlags __stdcall GetFlags() const;
  tBool __stdcall SetIcon(iOverlay *aVal);
  iOverlay * __stdcall GetIcon() const;
  tBool __stdcall SetSubmenu(iHString* aVal);
  iHString* __stdcall GetSubmenu() const;
  tBool __stdcall SetSelected(tBool aVal);
  tBool __stdcall GetSelected() const;
  tBool __stdcall SetGroupID(iHString* aVal);
  iHString* __stdcall GetGroupID() const;
  tBool __stdcall SetKey(eKey aVal);
  eKey __stdcall GetKey() const;
  //// ni::iWidgetMenuItem /////////////////////

  void _UpdateLocale();

 private:
  iWidget*        mpwMenu;
  tHStringPtr     mhspName;
  tHStringPtr     mhspLocalizedName;
  tHStringPtr     mhspID;
  tWidgetMenuItemFlags    mnFlags;
  Ptr<iOverlay> mptrIcon;
  tHStringPtr     mhspSubmenu;
  tBool       mbSelected;
  tHStringPtr     mhspGroupID;
  eKey        mKey;
  niEndClass(cWidgetMenuItem);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETMENUITEM_14781300_H__
