// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetMenu.h"
#include "WidgetMenuItem.h"
#include <niLang/STL/sort.h>

static const tU32 _knSeparatorHeight = 5;
static const tU32 _knTopMargin = 2;
static const tU32 _knBottomMargin = 2;
static const tU32 _knLeftMarginTextSpace = 4;

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetMenu implementation.

///////////////////////////////////////////////
cWidgetMenu::cWidgetMenu(iWidget* apWidget)
{
  ZeroMembers();
  mpWidget = apWidget;
}

///////////////////////////////////////////////
cWidgetMenu::~cWidgetMenu()
{
  for (tWidgetMenuItemVecIt it = mvItems.begin(); it != mvItems.end(); ++it) {
    (*it)->Invalidate();
  }
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetMenu::ZeroMembers()
{
  mpActiveSubMenu = NULL;
  mnSelectedItem = eInvalidHandle;
  mbKeyInput = eFalse;
  mbIsSubMenu = eFalse;
  mbIsSubSubMenu = eFalse;
  mbClickDown = eFalse;
  mbShouldUpdateSizes = eTrue;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetMenu::IsOK() const
{
  niClassIsOK(cWidgetMenu);
  return ni::eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetMenu::ClearItems()
{
  mbShouldUpdateSizes = eTrue;
  while (!mvItems.empty()) {
    RemoveItem(mvItems[0]);
  }
  return eTrue;
}

///////////////////////////////////////////////
iWidgetMenuItem* __stdcall cWidgetMenu::AddItem(const achar* aaszName, iHString* ahspID, tWidgetMenuItemFlags aFlags)
{
  mbShouldUpdateSizes = eTrue;

  cWidgetMenuItem* pNew = niNew cWidgetMenuItem(mpWidget);
  pNew->SetName(aaszName);
  pNew->SetID(ahspID);
  pNew->SetFlags(aFlags);
  // Sorted add
  if (niFlagIs(aFlags,eWidgetMenuItemFlags_SortAdd)) {
    if (!niIsStringOK(aaszName) || mvItems.empty()) {
      mvItems.push_back(pNew);
    }
    else {
      tU32 nStartIndex = 0;
      niLoopr(ri,(tU32)mvItems.size()) {
        if (niFlagIs(mvItems[ri]->GetFlags(),eWidgetMenuItemFlags_Separator) ||
            niFlagIs(mvItems[ri]->GetFlags(),eWidgetMenuItemFlags_SubMenu) ||
            niFlagIs(mvItems[ri]->GetFlags(),eWidgetMenuItemFlags_Title)) {
          nStartIndex = ri;
          break;
        }
      }
      tBool bInserted = ni::eFalse;
      for (tU32 i = nStartIndex; i < (tU32)mvItems.size(); ++i) {
        if (ni::StrICmp(aaszName,mvItems[i]->GetName()) < 0) {
          mvItems.insert(mvItems.begin()+i,pNew);
          bInserted = ni::eTrue;
          break;
        }
      }
      if (!bInserted) {
        mvItems.push_back(pNew);
      }
    }
  }
  else {
    mvItems.push_back(pNew);
  }
  return pNew;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetMenu::RemoveItem(iWidgetMenuItem* apItem)
{
  mbShouldUpdateSizes = eTrue;
  if (astl::find_erase(mvItems,apItem))
    return eTrue;
  for (tU32 i = 0; i < mpWidget->GetNumChildren(); ++i)
  {
    iWidgetMenu* pSubMenu = ni::QueryInterface<iWidgetMenu>(mpWidget->GetChildFromIndex(i));
    if (pSubMenu && pSubMenu->RemoveItem(apItem))
      return eTrue;
  }
  return eFalse;
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetMenu::GetNumItems() const
{
  return mvItems.size();
}

///////////////////////////////////////////////
iWidgetMenuItem* __stdcall cWidgetMenu::GetItem(tU32 anIndex) const
{
  if (anIndex >= mvItems.size()) return NULL;
  return mvItems[anIndex];
}

///////////////////////////////////////////////
iWidgetMenuItem* __stdcall cWidgetMenu::GetItemFromName(const achar* aaszName) const
{
  for (tWidgetMenuItemVecCIt it = mvItems.begin(); it != mvItems.end(); ++it)
  {
    if (ni::StrEq((*it)->GetName(),aaszName))
      return *it;
  }
  for (tU32 i = 0; i < mpWidget->GetNumChildren(); ++i)
  {
    iWidgetMenu* pSubMenu = ni::QueryInterface<iWidgetMenu>(mpWidget->GetChildFromIndex(i));
    if (pSubMenu)
    {
      iWidgetMenuItem* pItem = pSubMenu->GetItemFromName(aaszName);
      if (pItem)  return pItem;
    }
  }
  return NULL;
}

///////////////////////////////////////////////
iWidgetMenuItem* __stdcall cWidgetMenu::GetItemFromID(iHString* ahspID) const
{
  for (tWidgetMenuItemVecCIt it = mvItems.begin(); it != mvItems.end(); ++it) {
    if ((*it)->GetID() == ahspID)
      return *it;
  }
  for (tU32 i = 0; i < mpWidget->GetNumChildren(); ++i)
  {
    iWidgetMenu* pSubMenu = ni::QueryInterface<iWidgetMenu>(mpWidget->GetChildFromIndex(i));
    if (pSubMenu) {
      iWidgetMenuItem* pItem = pSubMenu->GetItemFromID(ahspID);
      if (pItem)  return pItem;
    }
  }
  return NULL;

}

///////////////////////////////////////////////
tBool __stdcall cWidgetMenu::OnWidgetSink(iWidget *apWidget, tU32 anMsg, const Var& avarA, const Var& avarB)
{
  niGuardObject((iWidgetSink*)this);
  switch (anMsg)
  {
    case eUIMessage_SkinChanged:
      {
        InitSkin();
        return eFalse;
      }
    case eUIMessage_Visible:
      {
        if (avarA.mI32) {
          for (tWidgetMenuItemVecIt it = mvItems.begin(); it != mvItems.end(); ++it) {
            if (niFlagIsNot((*it)->GetFlags(),eWidgetMenuItemFlags_Check) &&
                niFlagIsNot((*it)->GetFlags(),eWidgetMenuItemFlags_Group))
              (*it)->SetSelected(eFalse);
          }
        }
        return eTrue;
      }
    case eUIMessage_Timer:
      {
        if (mbKeyInput)
          mpWidget->SetTimer(0,-1);
        else
          ShowSubMenu();
        return eTrue;
      }
    case eUIMessage_Paint:
      {
        iCanvas* c = VarQueryInterface<iCanvas>(avarB);
        if (!c) return eFalse;
        iFont* pFont = mpWidget->GetFont();

        sRectf rect = mpWidget->GetWidgetRect();

        c->BlitFill(
            sRectf(Vec2<tF32>(tF32(mnLeftMargin-2),0),
                        Vec2<tF32>(rect.GetWidth(),rect.GetHeight())),
            skin._kcolBackNormal);
        c->BlitFill(
            sRectf(Vec2<tF32>(0,0),Vec2<tF32>(tF32(mnLeftMargin-2),rect.GetHeight())),
            skin._kcolLeftMargin);

        // Draw the border
        c->BlitRect(rect,skin._kcolBorder);

        // Draw the items
        tU32 nSelectedItemY = 0, i, x, y;
        for (x = mnLeftMargin, y = _knTopMargin, i = 0; i < mvItems.size(); ++i) {
          if (i == mnSelectedItem) nSelectedItemY = y;
          y += DrawItemRect(c,pFont,i,x,y);
        }

        // Overlays and Text rendering
        for (x = mnLeftMargin, y = _knTopMargin, i = 0; i < mvItems.size(); ++i) {
          if (i == mnSelectedItem) nSelectedItemY = y;
          y += DrawItemOvr(c,pFont,i,x,y);
        }
        for (x = mnLeftMargin+_knLeftMarginTextSpace, y = _knTopMargin, i = 0; i < mvItems.size(); ++i)
        {
          if (i == mnSelectedItem) nSelectedItemY = y;
          y += DrawItemText(c,pFont,i,x,y);
        }

        if (mpActiveSubMenu && !mpActiveSubMenu->GetVisible())
        {
          static_cast<cWidgetMenu*>(ni::QueryInterface<iWidgetMenu>(mpActiveSubMenu))->UpdateSizes();
          mpActiveSubMenu->SetPosition(Vec2<tF32>(tF32(mnWidth),tF32(nSelectedItemY))+mpWidget->GetAbsolutePosition());
          sVec2f newPos = Vec2<tF32>(tF32(mnWidth),tF32(nSelectedItemY));
          sRectf rectAbs = mpActiveSubMenu->GetAbsoluteRect();
          if (rectAbs.GetRight() > mpWidget->GetUIContext()->GetRootWidget()->GetSize().x) {
            newPos.x = -rectAbs.GetWidth();
          }
          if (rectAbs.GetBottom() > mpWidget->GetUIContext()->GetRootWidget()->GetSize().y) {
            newPos.y = tF32(nSelectedItemY)-rectAbs.GetHeight()+tF32(mnItemHeight);
          }
          ni::VecMaximize(newPos,newPos+mpWidget->GetAbsolutePosition(),Vec2f(0.0f,mnItemHeight));
          mpActiveSubMenu->SetPosition(newPos);

          mpActiveSubMenu->SetVisible(eTrue);
          mpActiveSubMenu->SetEnabled(eTrue);
          mpActiveSubMenu->SetZOrder(eWidgetZOrder_TopMost);
          mpWidget->SendCommand(mpWidget->GetParent(),eWidgetMenuCmd_Opened,mpActiveSubMenu);
        }

        return eFalse;
      }
    case eUIMessage_KeyDown:
    case eUIMessage_KeyUp:
      {
        sMouseMessageResult res = ProcessKeyInput(avarA.mU32,avarB.mU32,anMsg==eUIMessage_KeyDown);
        ProcessMouseClickResult(res,mpWidget);
        return eTrue;
      }
    case eUIMessage_NCLeftClickDown: {
      if (_TestMouseIntersect(avarB.GetVec2f())) {
        mbClickDown = eTrue;
        return eTrue;
      }
      // fall-through
    }
    case eUIMessage_NCRightClickDown: {
      this->Close();
      mpWidget->GetUIContext()->SendWindowMessage(
          eOSWindowMessage_MouseButtonDown,
          (anMsg == eUIMessage_NCRightClickDown) ? ePointerButton_Right : ePointerButton_Left,
          niVarNull);
      return eTrue;
    }
    case eUIMessage_RightClickDown:
    case eUIMessage_LeftClickDown:
      {
        mbClickDown = eTrue;
        return eTrue;
      }
    case eUIMessage_NCRightClickUp:
    case eUIMessage_NCLeftClickUp:
    case eUIMessage_RightClickUp:
    case eUIMessage_LeftClickUp:
      {
        if (mbClickDown) {
          mbKeyInput = eFalse;
          sMouseMessageResult res = ProcessMouseMessage(anMsg,avarB.GetVec2f());
          ProcessMouseClickResult(res,mpWidget);
        }
        mbClickDown = eFalse;
        return eTrue;
      }
    case eUIMessage_NCMouseMove:
    case eUIMessage_MouseMove:
      {
        ProcessMouseMessage(anMsg,avarA.GetVec2f());
        return eTrue;
      }
    case eUIMessage_NCWheel:
    case eUIMessage_Wheel: {
      const tF32 fWheelDelta = avarA.GetFloatValue();
      QPtr<iWidgetMenu> wMenu;
      if (mpActiveSubMenu && mpActiveSubMenu->GetVisible()) {
        const sRectf rootAbsRect = mpWidget->GetUIContext()->GetRootWidget()->GetAbsoluteRect();
        const sRectf rectAbs = mpActiveSubMenu->GetAbsoluteRect();
        // can scroll only if the menu is larger than the root widget's height (usually that'd be the app's window)
        if (rectAbs.GetHeight() > (rootAbsRect.GetHeight()-mnItemHeight)) {
          const tF32 fMinY =
              rootAbsRect.GetBottom() -
              (tF32)mnItemHeight -
              rectAbs.GetHeight();
          const tF32 fMaxY =
              rootAbsRect.GetTop() +
              (tF32)mnItemHeight;
          const sVec2f relPos = mpActiveSubMenu->GetPosition()-mpWidget->GetAbsolutePosition();
          mpActiveSubMenu->SetPosition(Vec2f(
              mpWidget->GetAbsolutePosition().x + relPos.x,
              ni::Clamp(mpWidget->GetAbsolutePosition().y + relPos.y + (fWheelDelta * mnItemHeight), fMinY, fMaxY)));
        }
      }
      else {
        // Can't scroll the root menu atm...
      }
      return eTrue;
    }
  }
  return eFalse;
}

///////////////////////////////////////////////
void cWidgetMenu::UpdateSizes()
{
  if (!mbShouldUpdateSizes || !mpWidget)
    return;

  mbShouldUpdateSizes = ni::eFalse;

  mnLeftMargin = 10;
  mnRightMargin = 10;
  mnItemWidth = 0;
  mnItemHeight = 0;

  tU32 nIconWidth = (tU32)ni::Max(skin.radio->GetSize().x,
                                  skin.check->GetSize().x);

  iFont* pFont = mpWidget->GetFont();
  tWidgetMenuItemVecIt it;
  for (it = mvItems.begin(); it != mvItems.end(); ++it)
  {
    Ptr<iWidgetMenuItem> ptrItem = *it;
    tWidgetMenuItemFlags flags = ptrItem->GetFlags();
    if (niFlagIs(flags,eWidgetMenuItemFlags_Check) ||
        niFlagIs(flags,eWidgetMenuItemFlags_Group) ||
        ptrItem->GetSelected())
    {
      mnLeftMargin = ni::Max(nIconWidth+4,mnLeftMargin);
    }
    if (niFlagIs(flags,eWidgetMenuItemFlags_SubMenu))
    {
      mnRightMargin = tU32(skin.arrowRight->GetSize().x)+14;
    }
    if (niFlagIs(flags,eWidgetMenuItemFlags_Separator) && niFlagIsNot(flags,eWidgetMenuItemFlags_Title))
    {
      mnHeight += _knSeparatorHeight;
    }
    else
    {
      if (ptrItem->GetIcon())
      {
        sVec2f size = ptrItem->GetIcon()->GetSize();
        mnLeftMargin = ni::Max(mnLeftMargin,size.x+6);
        mnItemHeight = ni::Max(mnItemHeight,size.y+4);
      }
      sRectf rect = pFont->ComputeTextSize(sRectf(0,0),ptrItem->GetName(),0);
      mnItemWidth = ni::Max(mnItemWidth,rect.GetWidth()+4);
      mnItemHeight = ni::Max(mnItemHeight,rect.GetHeight()+4);
    }
  }
  mnHeight = _knTopMargin;
  for (it = mvItems.begin(); it != mvItems.end(); ++it)
  {
    if (niFlagIs((*it)->GetFlags(),eWidgetMenuItemFlags_Separator) && niFlagIsNot((*it)->GetFlags(),eWidgetMenuItemFlags_Title))
      mnHeight += _knSeparatorHeight;
    else
      mnHeight += mnItemHeight;
  }
  mnHeight += _knBottomMargin;
  mnWidth = mnLeftMargin+_knLeftMarginTextSpace+mnItemWidth+mnRightMargin;

  // update the size of the client rect
  mpWidget->SetSize(Vec2<tF32>(tF32(mnWidth),tF32(mnHeight)));
}

///////////////////////////////////////////////
tU32 cWidgetMenu::DrawItemRect(iCanvas* c, iFont* apFont, tU32 anIndex, tU32 aX, tU32 aY)
{
  Ptr<iWidgetMenuItem> ptrItem = (anIndex < mvItems.size()) ? mvItems[anIndex].ptr() : NULL;
  if (!ptrItem.IsOK())
    return mnItemHeight;

  if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Title)) {
    sRectf rect = sRectf(1,tF32(aY),tF32(mnWidth-2),tF32(mnItemHeight));
    c->BlitFill(rect,skin._kcolBackTitle);
    c->BlitRect(rect,skin._kcolBorderTitle);
    return mnItemHeight;
  }
  else if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Separator))
  {
    tF32 y = tF32(aY+(_knSeparatorHeight/2));
    c->BlitFill(
        sRectf(Vec2<tF32>(tF32(aX),y),
                    Vec2<tF32>(tF32(aX+mnItemWidth+mnRightMargin-1),y+1)),
        skin._kcolSeparator);
    return _knSeparatorHeight;
  }
  else
  {
    sRectf rect = sRectf(2,tF32(aY),tF32(mnWidth-4),tF32(mnItemHeight));
    if (anIndex == mnSelectedItem) {
      c->BlitFill(rect,skin._kcolBackSelected);
      c->BlitRect(rect,skin._kcolBorderSelected);
    }
    return mnItemHeight;
  }
}

///////////////////////////////////////////////
tU32 cWidgetMenu::DrawItemOvr(iCanvas* c, iFont* apFont, tU32 anIndex, tU32 aX, tU32 aY)
{
  Ptr<iWidgetMenuItem> ptrItem = (anIndex < mvItems.size()) ? mvItems[anIndex].ptr() : NULL;
  if (!ptrItem.IsOK())
    return mnItemHeight;

  if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Title)) {
    return mnItemHeight;
  }
  else if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Separator))
  {
    return _knSeparatorHeight;
  }
  else
  {
    sRectf rect = sRectf(2,tF32(aY),tF32(mnWidth-4),tF32(mnItemHeight));
    // Draw the check
    if (ptrItem->GetSelected())
    {
      iOverlay* pOvr;
      if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Check))
        pOvr = skin.check;
      else
        pOvr = skin.radio;
      sVec2f pos = { tF32(aX-pOvr->GetSize().x-3),tF32(aY+(mnItemHeight>>1)-(pOvr->GetSize().y/2)) };
      c->BlitOverlay(
          sRectf(pos.x,pos.y,
                      pOvr->GetSize().x,pOvr->GetSize().y),
          pOvr);
    }
    else if (ptrItem->GetIcon())
    {
      iOverlay* pOvr = ptrItem->GetIcon();
      // Draw the icon
      sVec2f pos = {tF32(aX-pOvr->GetSize().x-3),tF32(aY+(mnItemHeight>>1)-(pOvr->GetSize().y/2))};
      c->BlitOverlay(
          sRectf(pos.x,pos.y,
                      pOvr->GetSize().x,pOvr->GetSize().y),
          pOvr);
    }
    // Draw the sub-menu arrow
    if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_SubMenu))
    {
      iOverlay* pOvr = skin.arrowRight;
      sVec2f pos = {tF32(mnWidth-pOvr->GetSize().x-2),tF32(aY+(mnItemHeight>>1)-(pOvr->GetSize().y/2))};
      c->BlitOverlay(
          sRectf(pos.x,pos.y,
                      pOvr->GetSize().x,pOvr->GetSize().y),
          pOvr);
    }
    return mnItemHeight;
  }
}

///////////////////////////////////////////////
tU32 cWidgetMenu::DrawItemText(iCanvas* c, iFont* apFont, tU32 anIndex, tU32 aX, tU32 aY)
{
  niAssert(anIndex < mvItems.size());
  Ptr<cWidgetMenuItem> ptrItem = (cWidgetMenuItem*)mvItems[anIndex].ptr();
  if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Title)) {
    sRectf rect = sRectf(2,tF32(aY),tF32(mnWidth-4),tF32(mnItemHeight));
    rect.MoveTo(Vec2<tF32>(tF32(mnLeftMargin),tF32(aY)));
    c->BlitText(
        apFont,
        rect,
        eFontFormatFlags_CenterV,
        ptrItem->GetLocalizedName());
    return mnItemHeight;
  }
  else if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Separator))
  {
    return _knSeparatorHeight;
  }
  else
  {
    tU32 col = apFont->GetColor();
    if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Disabled)) {
      apFont->SetColor(skin._kcolTextDisabled);
    }
    sRectf rect = sRectf(tF32(aX),tF32(aY),tF32(mnWidth-4),tF32(mnItemHeight));
    // rect.MoveTo(Vec2<tF32>(tF32(mnLeftMargin),tF32(aY)));
    c->BlitText(
        apFont,
        rect,
        eFontFormatFlags_CenterV,
        ptrItem->GetLocalizedName());
    if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Disabled)) {
      apFont->SetColor(col);
    }
    return mnItemHeight;
  }
}

///////////////////////////////////////////////
tU32 cWidgetMenu::FindItemFromPos(const sVec2f& avPos)
{
  tU32 y = _knTopMargin;
  for (tWidgetMenuItemVecCIt it = mvItems.begin(); it != mvItems.end(); ++it)
  {
    if (niFlagIs((*it)->GetFlags(),eWidgetMenuItemFlags_Title) || niFlagIs((*it)->GetFlags(),eWidgetMenuItemFlags_Disabled)) {
      y += mnItemHeight;
    }
    else if (niFlagIs((*it)->GetFlags(),eWidgetMenuItemFlags_Separator))
    {
      y += _knSeparatorHeight;
    }
    else
    {
      sRectf rect = sRectf(0,tF32(y),tF32(mnWidth),tF32(mnItemHeight));
      if (rect.Intersect(avPos))
        return astl::const_iterator_index(mvItems,it);
      y += mnItemHeight;
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
void cWidgetMenu::SelectItem(tU32 anNewSel)
{
  Ptr<iWidgetMenuItem> ptrWasSelectedItem = (mnSelectedItem < mvItems.size()) ? mvItems[mnSelectedItem].ptr() : NULL;

  if (anNewSel >= GetNumItems())
    anNewSel = eInvalidHandle;

  if (mnSelectedItem != anNewSel) {
    if (mnSelectedItem != eInvalidHandle && niFlagIs(mvItems[mnSelectedItem]->GetFlags(),eWidgetMenuItemFlags_SubMenu)) {
      CloseSubMenu();
    }
    if (anNewSel != eInvalidHandle && niFlagIs(mvItems[anNewSel]->GetFlags(),eWidgetMenuItemFlags_SubMenu))
      mpWidget->SetTimer(0,0.3f);
  }

  mnSelectedItem = anNewSel;

  Ptr<iWidgetMenuItem> ptrSelectedItem = (mnSelectedItem < mvItems.size()) ? mvItems[mnSelectedItem].ptr() : NULL;
  if (ptrWasSelectedItem != ptrSelectedItem) {
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetMenuCmd_FocusedItem,ptrSelectedItem.ptr(),ptrWasSelectedItem.ptr());
  }
}

///////////////////////////////////////////////
cWidgetMenu::sMouseMessageResult cWidgetMenu::ProcessKeyInput(tU32 aKey, tU32 anKeyMod, tBool abIsDown)
{
  sMouseMessageResult res;
  res.bIntersect = eTrue;

  mbKeyInput = eTrue;
  if (abIsDown)
    return res;

  if (mpActiveSubMenu)
  {
    cWidgetMenu* pMenu = static_cast<cWidgetMenu*>(ni::QueryInterface<iWidgetMenu>(mpActiveSubMenu));
    res = pMenu->ProcessKeyInput(aKey,anKeyMod,abIsDown);
    if (res.bIntersect == eFalse) // bIntersect to false means the menu
      // has been closed by the key input
    {
      if (mpActiveSubMenu)
      {
        mbKeyInput = eTrue;
        CloseSubMenu();
        res.bIntersect = eTrue;
      }
      else
      {
        if (mbKeyInput)
          res.bIntersect = eTrue;
      }
    }
  }
  else
  {
    switch (aKey)
    {
      case eKey_Home:
        {
          SelectFirstItem();
          break;
        }
      case eKey_End:
        {
          SelectLastItem();
          break;
        }
      case eKey_Up:
        {
          if (niFlagIs(anKeyMod,eKeyMod_Control) ||
              niFlagIs(anKeyMod,eKeyMod_Shift))
            SelectPrevSeparator();
          else
            SelectPrevItem();
          break;
        }
      case eKey_Down:
        {
          if (niFlagIs(anKeyMod,eKeyMod_Control) ||
              niFlagIs(anKeyMod,eKeyMod_Shift))
            SelectNextSeparator();
          else
            SelectNextItem();
          break;
        }
      case eKey_PgUp:
        {
          SelectPrevSeparator();
          break;
        }
      case eKey_PgDn:
        {
          SelectNextSeparator();
          break;
        }
      case eKey_Left:
        {
          // signal that this submenu should be closed
          res.bIntersect = eFalse;
          break;
        }
      case eKey_Right:
        {
          ShowSubMenu();
          cWidgetMenu* pMenu = static_cast<cWidgetMenu*>(ni::QueryInterface<iWidgetMenu>(mpActiveSubMenu));
          if (pMenu)  pMenu->SelectItem(0);
          break;
        }
      case eKey_Space:
      case eKey_Enter:
      case eKey_NumPadEnter:
        {
          ProcessClick(res);
          cWidgetMenu* pMenu = static_cast<cWidgetMenu*>(ni::QueryInterface<iWidgetMenu>(mpActiveSubMenu));
          if (pMenu)  pMenu->SelectItem(0);
          break;
        }
      case eKey_Escape:
        {
          mbKeyInput = eFalse;
          res.bIntersect = eFalse;
          break;
        }
      default:
        {
          for (tU32 i = 0; i < mvItems.size(); ++i) {
            if (mvItems[i]->GetKey() == (eKey)aKey) {
              SelectItem(i);
              ProcessClick(res);
              cWidgetMenu* pMenu = static_cast<cWidgetMenu*>(
                  ni::QueryInterface<iWidgetMenu>(mpActiveSubMenu));
              if (pMenu)  pMenu->SelectItem(0);
              break;
            }
          }
          break;
        }
    }
  }

  return res;
}

///////////////////////////////////////////////
tBool cWidgetMenu::_TestMouseIntersect(const sVec2f& avMousePos) {
  sVec2f vAbsMousePos = mpWidget->GetAbsolutePosition()+avMousePos;
  if (mpWidget->GetAbsoluteRect().Intersect(vAbsMousePos)) {
    return eTrue;
  }

  if (mpActiveSubMenu) {
    cWidgetMenu* pMenu = static_cast<cWidgetMenu*>(ni::QueryInterface<iWidgetMenu>(mpActiveSubMenu));
    return pMenu->_TestMouseIntersect(vAbsMousePos-mpActiveSubMenu->GetAbsolutePosition());
  }

  return eFalse;
}

///////////////////////////////////////////////
cWidgetMenu::sMouseMessageResult cWidgetMenu::ProcessMouseMessage(tU32 anMsg, const sVec2f& avMousePos)
{
  sMouseMessageResult res;
  sVec2f vAbsMousePos = mpWidget->GetAbsolutePosition()+avMousePos;
  tBool bIntersect = mpWidget->GetAbsoluteRect().Intersect(vAbsMousePos);
  res.bIntersect = bIntersect;

  if (mpActiveSubMenu)
  {
    cWidgetMenu* pMenu = static_cast<cWidgetMenu*>(ni::QueryInterface<iWidgetMenu>(mpActiveSubMenu));
    res = pMenu->ProcessMouseMessage(anMsg,vAbsMousePos-mpActiveSubMenu->GetAbsolutePosition());
  }

  switch (anMsg)
  {
    case eUIMessage_NCMouseMove:
    case eUIMessage_MouseMove:
      {
        if (!mpActiveSubMenu || (!res.bIntersect && bIntersect))
        {
          res.bIntersect = bIntersect;
          mbKeyInput = eFalse;
          SelectItem(FindItemFromPos(avMousePos));
        }
        break;
      }
    case eUIMessage_NCLeftClickUp:
    case eUIMessage_LeftClickUp:
      {
        if (!mpActiveSubMenu || (!res.bIntersect && bIntersect))
        {
          res.bIntersect = bIntersect;
          ProcessClick(res);
        }
        break;
      }
    default:
      if (!mpActiveSubMenu || (!res.bIntersect && bIntersect))
        res.bIntersect = bIntersect;
      break;
  }

  return res;
}

///////////////////////////////////////////////
void cWidgetMenu::ProcessMouseClickResult(const sMouseMessageResult& aRes, iWidget* apRoot)
{
  if (!aRes.bIntersect) {
    Close();
  }
  else {
    tBool bClose = eFalse;
    tU32 nStyle = apRoot->GetStyle();
    switch (aRes.Msg)
    {
      case eWidgetMenuCmd_Checked:
      case eWidgetMenuCmd_UnChecked:
        {
          mpWidget->SendCommand(mpWidget->GetParent(),aRes.Msg,aRes.ptrItem.ptr());
          if (niFlagIsNot(nStyle,eWidgetMenuStyle_NoCloseOnCheckChange)) {
            bClose = eTrue;
          }
          break;
        }
      case eWidgetMenuCmd_GroupChanged:
        {
          mpWidget->SendCommand(mpWidget->GetParent(),aRes.Msg,aRes.ptrItem.ptr());
          if (niFlagIsNot(nStyle,eWidgetMenuStyle_NoCloseOnGroupChange)) {
            bClose = eTrue;
          }
          break;
        }
      case eWidgetMenuCmd_Clicked:
        {
          mpWidget->SendCommand(mpWidget->GetParent(),aRes.Msg,aRes.ptrItem.ptr());
          if (niFlagIsNot(nStyle,eWidgetMenuStyle_NoCloseOnItemClicked)) {
            if (niFlagIs(aRes.ptrItem.ptr()->GetFlags(), eWidgetMenuItemFlags_SubMenu)) {
              ShowSubMenu();
              break;
            }
            bClose = eTrue;
          }
          break;
        }
      default:
        {
          // click out of the client area
          if (!mbKeyInput && niFlagIsNot(nStyle,eWidgetMenuStyle_NoCloseOnNCClick)) {
            bClose = eTrue;
          }
          break;
        }
    }
    if (bClose)
      Close();
  }
}

///////////////////////////////////////////////
void cWidgetMenu::ShowSubMenu()
{
  if (mpActiveSubMenu ||
      mnSelectedItem == eInvalidHandle ||
      niFlagIsNot(mvItems[mnSelectedItem]->GetFlags(),eWidgetMenuItemFlags_SubMenu))
    return;
  niAssert(mnSelectedItem != eInvalidHandle);
  niAssert(niFlagIs(mvItems[mnSelectedItem]->GetFlags(),eWidgetMenuItemFlags_SubMenu));
  Ptr<iWidgetMenuItem> ptrItem = mvItems[mnSelectedItem];
  mpActiveSubMenu = mpWidget->GetChildFromID(ptrItem->GetSubmenu());
  if (!QPtr<iWidgetMenu>(mpActiveSubMenu).IsOK()) {
    mpActiveSubMenu = NULL;
  }
  mpWidget->SetTimer(0,-1);
}

///////////////////////////////////////////////
void cWidgetMenu::CloseSubMenu()
{
  mpWidget->SetTimer(0,-1);
  if (mpActiveSubMenu)
  {
    /*mpActiveSubMenu->SetVisible(eFalse);
      mpActiveSubMenu->SetEnabled(eFalse);*/
    QPtr<iWidgetMenu>(mpActiveSubMenu)->Close();
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetMenuCmd_Closed,mpActiveSubMenu);
    mpActiveSubMenu = NULL;
  }
}

///////////////////////////////////////////////
void cWidgetMenu::ProcessClick(sMouseMessageResult& aRes)
{
  Ptr<iWidgetMenuItem> ptrItem = (mnSelectedItem < mvItems.size()) ? mvItems[mnSelectedItem].ptr() : NULL;
  if (!ptrItem.IsOK())
    return;

  if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_SubMenu))
  {
    aRes.Msg = eWidgetMenuCmd_Clicked;
    aRes.ptrItem = ptrItem;
  }
  else if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Check))
  {
    tBool bNewState = ptrItem->GetSelected()?eFalse:eTrue;
    ptrItem->SetSelected(bNewState);
    // set the result
    aRes.Msg = bNewState?eWidgetMenuCmd_Checked:eWidgetMenuCmd_UnChecked;
    aRes.ptrItem = ptrItem;
  }
  else if (niFlagIs(ptrItem->GetFlags(),eWidgetMenuItemFlags_Group))
  {
    if (!ptrItem->GetSelected())
      ptrItem->SetSelected(eTrue);
    // set the result
    aRes.Msg = eWidgetMenuCmd_GroupChanged;
    aRes.ptrItem = ptrItem;
  }
  else if (niFlagIsNot(ptrItem->GetFlags(),eWidgetMenuItemFlags_SubMenu) &&
           niFlagIsNot(ptrItem->GetFlags(),eWidgetMenuItemFlags_Separator) &&
           niFlagIsNot(ptrItem->GetFlags(),eWidgetMenuItemFlags_Disabled))
  {
    // set the result
    aRes.Msg = eWidgetMenuCmd_Clicked;
    aRes.ptrItem = ptrItem;
  }
}

///////////////////////////////////////////////
tBool __stdcall cWidgetMenu::Open()
{
  if (!GetNumItems())
    return eFalse;

  tHStringPtr hspLocale = mpWidget->GetActiveLocale();
  if (hspLocale != mhspPrevOpenLocale) {
    for (tU32 i = 0; i < mvItems.size(); ++i) {
      ((cWidgetMenuItem*)mvItems[i].ptr())->_UpdateLocale();
    }
    mhspPrevOpenLocale = hspLocale;
    mbShouldUpdateSizes = eTrue;
  }

  {
    UpdateSizes();
    sVec2f deskSize = mpWidget->GetUIContext()->GetRootWidget()->GetSize();
    sRectf rectAbs = mpWidget->GetAbsoluteRect();
    sVec2f newPos = rectAbs.GetTopLeft();
    if (rectAbs.GetRight() > deskSize.x)
    {
      newPos.x -= rectAbs.GetRight()-deskSize.x;
    }
    if (rectAbs.GetBottom() > deskSize.y)
    {
      newPos.y -= rectAbs.GetHeight();
    }
    ni::VecMaximize(newPos,newPos,sVec2f::Zero());
    if (newPos != rectAbs.GetTopLeft())
    {
      rectAbs.MoveTo(newPos);
      mpWidget->SetAbsoluteRect(rectAbs);
    }
  }
  mbClickDown = eFalse;
  mpWidget->SetVisible(eTrue);
  mpWidget->SetEnabled(eTrue);
  mpWidget->SetZOrder(eWidgetZOrder_TopMost);
  mpWidget->SetCapture(eTrue);
  mpWidget->SetFocus();
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetMenuCmd_Opened,mpWidget);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetMenu::Close()
{
  CloseSubMenu();
  mpWidget->SetCapture(eFalse);
  mpWidget->SetVisible(eFalse);
  mpWidget->SetEnabled(eFalse);
  Ptr<iWidgetMenuItem> ptrWasSelectedItem = (mnSelectedItem < mvItems.size()) ? mvItems[mnSelectedItem].ptr() : NULL;
  if (ptrWasSelectedItem.IsOK()) {
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetMenuCmd_FocusedItem,NULL,ptrWasSelectedItem.ptr());
  }
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetMenuCmd_Closed,mpWidget);
  SelectItem(eInvalidHandle);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetMenu::SortItems()
{
  // Return whether first element is greater than the second
  struct _Local {
    static int compare(const Ptr<iWidgetMenuItem>& aLeft, const Ptr<iWidgetMenuItem>& aRight) {
      return ni::StrCmp(aLeft->GetName(),aRight->GetName()) < 0;
    }
  };
  astl::stable_sort(mvItems.begin(),mvItems.end(),_Local::compare);
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetMenu::SelectFirstItem()
{
  tU32 i;
  for (i = 0; i < mvItems.size(); ++i)
  {
    if (niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Separator) &&
        niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Disabled))
      break;
  }
  if (i == mvItems.size())  SelectItem(eInvalidHandle);
  else            SelectItem(i);
}

///////////////////////////////////////////////
void cWidgetMenu::SelectLastItem()
{
  tI32 i;
  for (i = mvItems.size()-1; i >= 0; --i)
  {
    if (niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Separator) &&
        niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Disabled))
      break;
  }
  if (i == -1)  SelectItem(eInvalidHandle);
  else      SelectItem(i);
}

///////////////////////////////////////////////
void cWidgetMenu::SelectNextItem()
{
  if (mnSelectedItem == eInvalidHandle)
  {
    SelectFirstItem();
  }
  else
  {
    tU32 i;
    for (i = mnSelectedItem+1; i < mvItems.size(); ++i)
    {
      if (niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Separator) &&
          niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Disabled))
        break;
    }
    if (i == mvItems.size())  SelectFirstItem();
    else            SelectItem(i);
  }
}

///////////////////////////////////////////////
void cWidgetMenu::SelectPrevItem()
{
  if (mnSelectedItem == eInvalidHandle)
  {
    SelectLastItem();
  }
  else
  {
    tI32 i;
    for (i = mnSelectedItem-1; i >= 0; --i)
    {
      if (niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Separator) &&
          niFlagIsNot(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Disabled))
        break;
    }
    if (i == -1)  SelectLastItem();
    else      SelectItem(i);
  }
}

///////////////////////////////////////////////
void cWidgetMenu::SelectNextSeparator()
{
  if (mnSelectedItem == eInvalidHandle)
  {
    SelectFirstItem();
  }
  else
  {
    tU32 i;
    for (i = mnSelectedItem+1; i < mvItems.size(); ++i)
    {
      if (niFlagIs(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Separator))
        break;
    }
    if (i == mvItems.size()) {
      SelectFirstItem();
    }
    else {
      SelectItem(i);
      SelectNextItem();
    }
  }
}

///////////////////////////////////////////////
void cWidgetMenu::SelectPrevSeparator()
{
  if (mnSelectedItem == eInvalidHandle)
  {
    SelectLastItem();
  }
  else
  {
    tI32 i;
    for (i = mnSelectedItem-1; i >= 0; --i)
    {
      if (niFlagIs(mvItems[i]->GetFlags(),eWidgetMenuItemFlags_Separator))
        break;
    }
    if (i == -1) {
      SelectLastItem();
    }
    else {
      SelectItem(i);
      SelectPrevItem();
    }
  }
}

///////////////////////////////////////////////
void cWidgetMenu::InitSkin()
{
  mbShouldUpdateSizes = eTrue;

  skin.radio = mpWidget->FindSkinElement(NULL,NULL,_H("Radio"));
  skin.check = mpWidget->FindSkinElement(NULL,NULL,_H("Check"));
  skin.arrowRight = mpWidget->FindSkinElement(NULL,NULL,_H("ArrowRight"));

  // EDMOND _newUI_
  const sColor4f color = sColor4f::White();
  skin._kcolBorder = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("Border")));
  skin._kcolBorderSelected = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("BorderSelected")));
  skin._kcolSeparator = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("Separator")));
  skin._kcolBackNormal = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("BackNormal")));
  skin._kcolBackSelected = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("BackSelected")));
  skin._kcolLeftMargin = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("LeftMargin")));
  skin._kcolBackTitle = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("BackTitle")));
  skin._kcolBorderTitle = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("BorderTitle")));
  skin._kcolTextDisabled = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("TextDisabled")));
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetMenu::GetMenuWidth() const {
  niThis(cWidgetMenu)->UpdateSizes();
  return mnWidth;
}
tU32 __stdcall cWidgetMenu::GetMenuHeight() const {
  niThis(cWidgetMenu)->UpdateSizes();
  return mnHeight;
}
tU32 __stdcall cWidgetMenu::GetItemWidth() const {
  niThis(cWidgetMenu)->UpdateSizes();
  return mnItemWidth;
}
tU32 __stdcall cWidgetMenu::GetItemHeight() const {
  niThis(cWidgetMenu)->UpdateSizes();
  return mnItemHeight;
}
