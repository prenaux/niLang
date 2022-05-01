// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetMenuItem.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetMenuItem implementation.

///////////////////////////////////////////////
cWidgetMenuItem::cWidgetMenuItem(iWidget* apwMenu)
{
  niAssert(niIsOK(apwMenu));
  ZeroMembers();
  mpwMenu = apwMenu;
}

///////////////////////////////////////////////
cWidgetMenuItem::~cWidgetMenuItem()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetMenuItem::ZeroMembers()
{
  mhspID = _H(AZEROSTR);
  mnFlags = 0;
  mhspSubmenu = _H(AZEROSTR);
  mbSelected = eFalse;
  mhspGroupID = _H(AZEROSTR);
}

///////////////////////////////////////////////
ni::tBool __stdcall cWidgetMenuItem::IsOK() const
{
  niClassIsOK(cWidgetMenuItem);
  return ni::eTrue;
}

///////////////////////////////////////////////
void __stdcall cWidgetMenuItem::Invalidate() {
  mpwMenu = NULL;
}

///////////////////////////////////////////////
iWidgetMenu* __stdcall cWidgetMenuItem::GetMenu() const
{
  return NULL;
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::SetName(const achar *aVal)
{
  mhspName = _H(aVal);
  if (mpwMenu)
    mhspLocalizedName = mpwMenu->FindLocalized(mhspName);
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetMenuItem::_UpdateLocale() {
  if (mpwMenu)
    mhspLocalizedName = mpwMenu->FindLocalized(mhspName);
}

///////////////////////////////////////////////
const achar * cWidgetMenuItem::GetName() const
{
  return niHStr(mhspName);
}
const achar* __stdcall cWidgetMenuItem::GetLocalizedName() const {
  return niHStr(mhspLocalizedName);
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::SetID(iHString* aVal)
{
  mhspID = aVal;
  return eTrue;
}

///////////////////////////////////////////////
iHString* cWidgetMenuItem::GetID() const
{
  return mhspID;
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::SetFlags(tWidgetMenuItemFlags aVal)
{
  mnFlags = aVal;
  return eTrue;
}

///////////////////////////////////////////////
tWidgetMenuItemFlags cWidgetMenuItem::GetFlags() const
{
  return mnFlags;
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::SetIcon(iOverlay *aVal)
{
  mptrIcon = aVal;
  return eTrue;
}

///////////////////////////////////////////////
iOverlay * cWidgetMenuItem::GetIcon() const
{
  return mptrIcon;
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::SetSubmenu(iHString* aVal)
{
  mhspSubmenu = aVal;
  return eTrue;
}

///////////////////////////////////////////////
iHString* cWidgetMenuItem::GetSubmenu() const
{
  return mhspSubmenu;
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::SetSelected(tBool aVal)
{
  QPtr<iWidgetMenu> wMenu = mpwMenu;
  if (wMenu.IsOK()) {
    if (aVal && niFlagIs(mnFlags,eWidgetMenuItemFlags_Group))
    {
      for (tU32 i = 0; i < wMenu->GetNumItems(); ++i)
      {
        iWidgetMenuItem* pItem = wMenu->GetItem(i);
        if (!niFlagIs(pItem->GetFlags(),eWidgetMenuItemFlags_Group))
          continue;
        if (pItem->GetGroupID() == mhspGroupID)
          static_cast<cWidgetMenuItem*>(pItem)->mbSelected = eFalse;
      }
    }
    mbSelected = aVal;
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::GetSelected() const
{
  return mbSelected;
}

///////////////////////////////////////////////
tBool cWidgetMenuItem::SetGroupID(iHString* aVal)
{
  mhspGroupID = aVal;
  return eTrue;
}

///////////////////////////////////////////////
iHString* cWidgetMenuItem::GetGroupID() const
{
  return mhspGroupID;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetMenuItem::SetKey(eKey aVal)
{
  mKey = aVal;
  return eTrue;
}

///////////////////////////////////////////////
eKey __stdcall cWidgetMenuItem::GetKey() const
{
  return mKey;
}
