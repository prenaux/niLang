// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"

#if niMinFeatures(20)

#include <niUI/Utils/WidgetSinkImpl.h>
#include <niUI/IWidgetToolbar.h>
#include "niUI_HString.h"

#define SKIN_AUTOHIDE       niBit(0)
#define SKIN_DRAWBACKFRAME  niBit(1)

struct sToolbarSkin : public cIUnknownImpl<ni::iUnknown>
{
  tU32          flags;
  sColor4f      backgroundColor;
  Ptr<iOverlay> menuFrame;
  Ptr<iOverlay> frame;
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
struct sWidgetToolbarButton : public ni::cWidgetSinkImpl<>
{
  iWidget* __stdcall GetParentToolbar() const {
    return mpWidget->GetParent()->GetParent()->GetParent();
  }

  ni::tBool __stdcall OnMouseEnter(const ni::sVec2f&,const ni::sVec2f&) {
    QPtr<iWidgetButton> b = mpWidget;
    if (b.IsOK()) {
      b->SetCheck(ni::eTrue);
      QPtr<iWidgetToolbar> tb = GetParentToolbar();
      if (tb->GetAutoHide()) {
        if (tb->GetShowPage()) {
          tb->SetActivePageID(mpWidget->GetID());
        }
      }
    }
    return ni::eTrue;
  }
  ni::tBool __stdcall OnMouseLeave(const ni::sVec2f&,const ni::sVec2f&) {
    QPtr<iWidgetButton> b = mpWidget;
    if (b.IsOK())
      b->SetCheck(ni::eFalse);
    return ni::eTrue;
  }
  ni::tBool __stdcall OnLeftClickDown(const ni::sVec2f& a,const ni::sVec2f& b) {
    QPtr<iWidgetToolbar> tb = GetParentToolbar();
    if (tb.IsOK()) {
      if (tb->GetAutoHide()) {
        if (mpWidget->GetID() == tb->GetActivePageID()) {
          tb->SetShowPage(!tb->GetShowPage());
        }
        else {
          tb->SetActivePageID(mpWidget->GetID());
          tb->SetShowPage(ni::eTrue);
        }
      }
      else {
        tb->SetActivePageID(mpWidget->GetID());
        tb->SetShowPage(ni::eTrue);
      }
    }
    return ni::eTrue;
  }
  ni::tBool __stdcall OnNCLeftDoubleClick(const ni::sVec2f& a,const ni::sVec2f& b) {
    return OnLeftDoubleClick(a,b);
  }
  ni::tBool __stdcall OnLeftDoubleClick(const ni::sVec2f& a,const ni::sVec2f& b) {
    QPtr<iWidgetToolbar> tb = GetParentToolbar();
    if (tb.IsOK()) {
      tb->SetAutoHide(!tb->GetAutoHide());
    }
    return ni::eTrue;
  }
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
struct sWidgetToolbarBars : public ni::cWidgetSinkImpl<>
{
  sWidgetToolbarBars(sToolbarSkin* apSkin) {
  }
  iWidget* __stdcall GetParentToolbar() const {
    return mpWidget->GetParent();
  }

  void _HidePages(const tBool abIsRightClick) {
    if (mpWidget->GetExclusive()) {
      QPtr<iWidgetToolbar> tb = GetParentToolbar();
      if (tb.IsOK()) {
        tb->SetShowPage(ni::eFalse);
      }

      if (mpWidget && mpWidget->GetUIContext()) {
        mpWidget->GetUIContext()->SendWindowMessage(
            eOSWindowMessage_MouseButtonDown,
            abIsRightClick ? ePointerButton_Right : ePointerButton_Left,
            niVarNull);
      }
    }
  }

  virtual tBool __stdcall OnNCLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) {
    _HidePages(eFalse);
    return eFalse;
  }
  virtual tBool __stdcall OnNCRightClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) {
    _HidePages(eTrue);
    return eFalse;
  }
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
struct sWidgetToolbarTopBar : public ni::cWidgetSinkImpl<>
{
  Ptr<sToolbarSkin> skin;

  sWidgetToolbarTopBar(sToolbarSkin* apSkin) : skin(apSkin) {
  }
  iWidget* __stdcall GetParentToolbar() const {
    return mpWidget->GetParent()->GetParent();
  }

  tBool __stdcall OnNCPaint(const sVec2f& avNCMousePos, iCanvas* apCanvas) {
    sRectf rectBottom = mpWidget->GetWidgetRect();
    QPtr<iWidgetToolbar> tb = GetParentToolbar();
    if (tb->GetAutoHide() || tb->GetNumPages() == 0) {
      apCanvas->BlitOverlayFrame(
          rectBottom,
          skin->menuFrame,
          eRectFrameFlags_All);
      //      skin->menuFrame->DrawFrameEx(pIM,eRectFrameFlags_All,rectBottom.GetTopLeft(),rectBottom.GetBottomRight());
    }
    return ni::eTrue;
  }
  ni::tBool __stdcall OnNCLeftDoubleClick(const ni::sVec2f& a,const ni::sVec2f& b) {
    return OnLeftDoubleClick(a,b);
  }
  ni::tBool __stdcall OnLeftDoubleClick(const ni::sVec2f& a,const ni::sVec2f& b) {
    QPtr<iWidgetToolbar> tb = GetParentToolbar();
    if (tb.IsOK()) {
      tb->SetAutoHide(!tb->GetAutoHide());
    }
    return ni::eTrue;
  }
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
struct sWidgetToolbarBotBar : public ni::cWidgetSinkImpl<>
{
  Ptr<sToolbarSkin> skin;

  sWidgetToolbarBotBar(sToolbarSkin* apSkin) : skin(apSkin) {
  }

  iWidget* __stdcall GetParentToolbar() const {
    return mpWidget->GetParent()->GetParent();
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall OnSize(const ni::sVec2f& avNewSize) {
    if (skin->frame.IsOK()) {
      mpWidget->SetClientRect(skin->frame->ComputeFrameCenter(mpWidget->GetWidgetRect()));
    }
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall OnNCPaint(const sVec2f& avNCMousePos, iCanvas* apCanvas) {
    if (niFlagIs(skin->flags,SKIN_DRAWBACKFRAME)) {
      sRectf rectBottom = mpWidget->GetWidgetRect();
      apCanvas->BlitOverlayFrame(
          rectBottom,
          skin->frame,
          eRectFrameFlags_All);
    }
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnRightClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) {
    return OnLeftClickDown(avMousePos,avNCMousePos);
  }
  virtual tBool __stdcall OnLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) {
    // In auto-hide close the toolbar if clicking outside of any group
    QPtr<iWidgetToolbar> tb = GetParentToolbar();
    if (tb.IsOK() && tb->GetAutoHide()) {
      tb->SetShowPage(ni::eFalse);
    }
    return eFalse;
  }
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
class cWidgetToolbar : public ni::cWidgetSinkImpl<ni::iWidgetToolbar>
{
  niBeginClass(cWidgetToolbar);

 public:
  //! Constructor.
  cWidgetToolbar() : skin(niNew sToolbarSkin())  {
    ZeroMembers();
  }

  //! Destructor.
  ~cWidgetToolbar() {
  }

  //! Zeros all the class members.
  void ZeroMembers() {
    mfHeight = 128.0f;
    mbAutoHide = ni::eTrue;
    mbAutoHideBackground = ni::eFalse;
    mnActivePage = 0;
  }

  //! Sanity check.
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cWidgetToolbar);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnSinkAttached() {

    // Create the bars container
    {
      mwBars = mpWidget->GetUIContext()->CreateWidget(_A("Dummy"),mpWidget,ni::sRectf(0,0,80,20),eWidgetStyle_NCRelative,_HC(ID_Toolbar_Bars));
      mwBars->SetDockStyle(eWidgetDockStyle_DockFill);
      mwBars->AddSink(niNew sWidgetToolbarBars(skin));
    }

    // Create the top bar
    {
      mwTopBar = mpWidget->GetUIContext()->CreateWidget(_A("Canvas"),mwBars,ni::sRectf(0,0,80,20),eWidgetStyle_NCRelative,_HC(ID_Toolbar_TopBar));
      mwTopBar->SetDockStyle(eWidgetDockStyle_DockTop);
      mwTopBar->SetSkinClass(_H("Toolbar"));
      mwTopBar->AddSink(niNew sWidgetToolbarTopBar(skin));
    }

    // Create the bottom bar
    {
      mwBotBar = mpWidget->GetUIContext()->CreateWidget(_A("Canvas"),mwBars,ni::sRectf(0,0,80,20),eWidgetStyle_NCRelative,_HC(ID_Toolbar_BotBar));
      mwBotBar->SetSkinClass(_H("Toolbar"));
      mwBotBar->AddSink(niNew sWidgetToolbarBotBar(skin));
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnNCSize(const sVec2f& avNewSize) {
    _ComputeLayout();
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnNCPaint(const sVec2f& avMousePos, iCanvas* apCanvas) {
    _UpdateSkinFlags();

    apCanvas->BlitFill(mwTopBar->GetRect(),
                       ULColorBuild(skin->backgroundColor));

    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall OnSkinChanged() {
    skin->backgroundColor = mpWidget->FindSkinColor(sColor4f::Zero(),NULL,NULL,_H("Background"));
    skin->menuFrame = mpWidget->FindSkinElement(NULL,NULL,_H("MenuFrame"));
    skin->frame = mpWidget->FindSkinElement(NULL,NULL,_H("Frame"));
    _ComputeLayout();
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnVisible(ni::tBool abVisible) {
    UpdateToolbar();
    return ni::eFalse;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall OnCommand(ni::iWidgetCommand* apCmd) {
    tU32 nTopWidgetIndex = GetTopWidgetIndex(apCmd->GetSender());
    if (nTopWidgetIndex != eInvalidHandle) {
      const sTopWidget& tw = mvTopWidgets[nTopWidgetIndex];
      if (tw.cmdDest.IsOK()) {
        ni::SendMessage(tw.cmdDest,eUIMessage_Command,apCmd);
        return ni::eTrue;
      }
    }
    if (mCommandDestination.IsOK() && mCommandDestination.ptr() != mpWidget) {
      ni::SendMessage(mCommandDestination,eUIMessage_Command,apCmd,niVarNull);
      return ni::eTrue;
    }
    return ni::eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall UpdateToolbar() {
    ni::tBool bErased;
    do {
      bErased = ni::eFalse;
      niLoop(i,mvTopWidgets.size()) {
        if (!niIsOK(mvTopWidgets[i].wWidget)) {
          mvTopWidgets.erase(mvTopWidgets.begin()+i);
          bErased = ni::eTrue;
          break;
        }
      }
    } while(bErased);
    do {
      bErased = ni::eFalse;
      niLoop(i,mvPages.size()) {
        sPage& p = mvPages[i];
        if (!niIsOK(p.wPage) || (p.wPage->GetNumChildren() == 0)) {
          p.wPage->Invalidate();
          p.wButton->Invalidate();
          mvPages.erase(mvPages.begin()+i);
          bErased = ni::eTrue;
          break;
        }
      }
    } while(bErased);
    _ActivatePage(mnActivePage);
    _UpdatePagesStatus();
    _ComputeSize();
    _ComputeLayout();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall HidePopup() {
    UpdateToolbar(); // has the same effect...
  }

  ///////////////////////////////////////////////
  void __stdcall _ComputeSize() {
    if (mbAutoHide || mvPages.empty()) {
      mpWidget->SetSize(Vec2(mpWidget->GetSize().x,GetMenuHeight()));
    }
    else {
      mpWidget->SetSize(Vec2(mpWidget->GetSize().x,mfHeight));
    }
  }
  void __stdcall _ComputeLayout() {
    if (!skin->menuFrame.IsOK()) return;
    mpWidget->SetZOrder(mpWidget->GetZOrder());
    mpWidget->SetDockStyle(eWidgetDockStyle_DockTop);
    mwTopBar->SetSize(Vec2(10.0f,skin->menuFrame->GetSize().y));
    niLoop(i,mwTopBar->GetNumChildren()) {
      iWidget* w = mwTopBar->GetChildFromIndex(i);
      _UpdateWidgetButtonWidth(w);
    }
    niLoop(i,mvTopWidgets.size()) {
      tBool bLeft = mvTopWidgets[i].fWidth < 0.0f;
      tF32 fWidth = ni::Abs(mvTopWidgets[i].fWidth);
      if (fWidth != 0.0f) {
        mvTopWidgets[i].wWidget->SetSize(Vec2(fWidth,mwTopBar->GetSize().y));
        mvTopWidgets[i].wWidget->SetPadding(Vec4<tF32>(0.0f,0.0f,0.0f,1.0f+skin->menuFrame->GetFrame().Bottom()));
        mvTopWidgets[i].wWidget->SetDockStyle(bLeft?eWidgetDockStyle_DockLeft:eWidgetDockStyle_DockRight);
      }
    }
    SetShowPage(!mbAutoHide);
  }

  ///////////////////////////////////////////////
  virtual iWidget* __stdcall InitPage(iHString* ahspID, iHString* ahspText) {
    UpdateToolbar();
    {
      tU32 nPageIndex = GetPageIndex(ahspID);
      if (nPageIndex != eInvalidHandle) {
        mvPages[nPageIndex].wButton->SetText(ahspText);
        return mvPages[nPageIndex].wPage;
      }
    }
    iWidget* w = mpWidget->GetUIContext()->CreateWidget(
        _A("Button"),mwTopBar,sRectf(0,0,0,20));
    w->SetDockStyle(eWidgetDockStyle_DockLeft);
    w->SetSkinClass(_H("ToolbarMenuButtonNormal"));
    w->SetText(ahspText);
    w->SetID(ahspID);
    _UpdateWidgetButtonWidth(w);
    w->AddSink(niNew sWidgetToolbarButton());
    sPage p;
    p.wButton = w;
    p.wPage = mpWidget->GetUIContext()->CreateWidget(
        _A("Dummy"),mwBotBar,sRectf::Null(),0,ahspID);
    p.wPage->SetDockStyle(eWidgetDockStyle_DockFill);
    mvPages.push_back(p);
    _ActivatePage(mnActivePage);
    return p.wPage;
  }
  virtual ni::tBool __stdcall SetPageName(iHString* ahspID, iHString* ahspText) {
    tU32 nPageIndex = GetPageIndex(ahspID);
    if (nPageIndex != eInvalidHandle) {
      mvPages[nPageIndex].wButton->SetText(ahspText);
      return ni::eTrue;
    }
    return ni::eFalse;
  }
  virtual iHString* __stdcall GetPageName(iHString* ahspID) const {
    tU32 nPageIndex = GetPageIndex(ahspID);
    if (nPageIndex != eInvalidHandle) {
      return mvPages[nPageIndex].wButton->GetText();
    }
    return NULL;
  }
  virtual ni::tBool __stdcall SetPageEnabled(iHString* ahspID, ni::tBool abEnabled) {
    tU32 nPageIndex = GetPageIndex(ahspID);
    if (nPageIndex != eInvalidHandle) {
      if (mvPages[nPageIndex].IsEnabled() == abEnabled)
        return ni::eTrue;
      mvPages[nPageIndex].SetEnabled(abEnabled);
      if (!abEnabled && GetActivePageIndex() == nPageIndex) {
        _ActivatePage(nPageIndex);
        if (mbAutoHide) UpdateToolbar();
      }
      else if (!GetActivePageID()) {
        _ActivatePage(0);
        if (mbAutoHide) UpdateToolbar();
      }
      return ni::eTrue;
    }
    return ni::eFalse;
  }
  virtual ni::tBool __stdcall GetPageEnabled(iHString* ahspID) const {
    tU32 nPageIndex = GetPageIndex(ahspID);
    if (nPageIndex != eInvalidHandle) {
      return mvPages[nPageIndex].IsEnabled();
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  tF32 __stdcall _UpdateWidgetButtonWidth(iWidget* apButton) {
    if (!skin->menuFrame.IsOK() || !apButton)
      return 0.0f;
    sRectf rectText = apButton->GetFont()->ComputeTextSize(sRectf::Null(),niHStr(apButton->GetText()),0);
    const tF32 w = rectText.GetWidth()+skin->menuFrame->GetFrame().Left()+skin->menuFrame->GetFrame().Right();
    apButton->SetSize(Vec2(
        w,
        rectText.GetHeight()+skin->menuFrame->GetFrame().Top()+skin->menuFrame->GetFrame().Bottom()));
    return w;
  }
  void __stdcall _UpdatePagesStatus() {
    niLoop(i,mvPages.size()) {
      if (!mvPages[i].IsEnabled()) {
        continue;
      }
      // Selected page
      if (i == mnActivePage) {
        if (mvPages[i].wButton.IsOK()) {
          if (mbAutoHide && !mwBars->GetExclusive()) {
            mvPages[i].wButton->SetSkinClass(_H("ToolbarMenuButtonNormal"));
          }
          else {
            mvPages[i].wButton->SetSkinClass(_H("ToolbarMenuButtonSelected"));
          }
        }
        if (mvPages[i].wPage.IsOK()) {
          mvPages[i].wPage->SetDockStyle(eWidgetDockStyle_DockFill);
          mvPages[i].wPage->SetStatus(eTrue,eTrue,eFalse);
        }
      }
      // Other pages
      else {
        if (mvPages[i].wButton.IsOK()) {
          mvPages[i].wButton->SetSkinClass(_H("ToolbarMenuButtonNormal"));
        }
        if (mvPages[i].wPage.IsOK()) {
          mvPages[i].wPage->SetDockStyle(eWidgetDockStyle_DockFill);
          mvPages[i].wPage->SetStatus(eFalse,eFalse,eTrue);
        }
      }
    }
  }
  ni::tBool __stdcall _ActivatePage(ni::tU32 anNewIndex) {
    if (mvPages.empty())
      return ni::eFalse;
    if (anNewIndex >= mvPages.size()) {
      anNewIndex = 0;
    }
    if (!mvPages[anNewIndex].IsEnabled()) {
      // the new index can't be enabled, try to find the first page that can be...
      niLoop(i,mvPages.size()) {
        anNewIndex = i;
        if (mvPages[anNewIndex].IsEnabled())
          break;
      }
      if (!mvPages[anNewIndex].IsEnabled())
        return ni::eFalse; // can't activate any page...
    }
    if (anNewIndex == mnActivePage) {
      if (!mbAutoHide || mpWidget->GetExclusive())
        return ni::eTrue;
    }
    mnActivePage = anNewIndex;
    if (GetShowPage()) {
      SetShowPage(ni::eTrue);
    }
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual ni::tU32 __stdcall GetNumPages() const {
    return (ni::tU32)mvPages.size();
  }
  virtual ni::iWidget* __stdcall GetPageWidget(ni::tU32 anIndex) const {
    niCheckSilent(anIndex < mvPages.size(),NULL);
    return mvPages[anIndex].wPage;
  }
  virtual ni::tU32 __stdcall GetPageIndex(ni::iHString* ahspID) const {
    niLoop(i,mvPages.size()) {
      if (mvPages[i].wButton->GetID() == ahspID)
        return (tU32)i;
    }
    return ni::eInvalidHandle;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall SetActivePageIndex(ni::tU32 anActivePage) {
    return _ActivatePage(anActivePage);
  }
  virtual ni::tU32 __stdcall GetActivePageIndex() const {
    return mnActivePage;
  }
  virtual ni::tBool __stdcall SetActivePageID(ni::iHString* ahspID) {
    return _ActivatePage(GetPageIndex(ahspID));
  }
  virtual ni::iHString* __stdcall GetActivePageID() const {
    niCheckSilent(mnActivePage < mvPages.size(),NULL);
    if (!mvPages[mnActivePage].IsEnabled()) return NULL;
    return mvPages[mnActivePage].wButton->GetID();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetAutoHide(ni::tBool abAutoHide) {
    mbAutoHide = abAutoHide;
    _ComputeSize();
    SetShowPage(!mbAutoHide);
  }
  virtual ni::tBool __stdcall GetAutoHide() const {
    return mbAutoHide;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetAutoHideBackground(ni::tBool abAutoHide) {
    mbAutoHideBackground = abAutoHide;
  }
  virtual ni::tBool __stdcall GetAutoHideBackground() const {
    return mbAutoHideBackground;
  }

  ///////////////////////////////////////////////
  virtual ni::tF32 __stdcall GetMenuHeight() const {
    return mwTopBar->GetSize().y;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetShowPage(ni::tBool abVisible) {
    if (!skin->menuFrame.IsOK())
      return;
    _UpdateSkinFlags();
    if (!mvPages.empty()) {
      sRectf rectBottom;
      rectBottom.SetTopLeft(mbAutoHide ? mpWidget->GetAbsolutePosition() : mpWidget->GetPosition());
      tF32 fStickToTop = skin->menuFrame->GetFrame().Bottom();
      rectBottom.SetSize(Vec2(mpWidget->GetSize().x,mfHeight-GetMenuHeight()+fStickToTop));
      rectBottom.Move(Vec2(0.0f,GetMenuHeight()-fStickToTop));
      if (!niFlagIs(skin->flags,SKIN_DRAWBACKFRAME)) {
        rectBottom.Move(Vec2(0.0f,-2.0f));
      }
      mwBotBar->SetRect(rectBottom);
      mwBotBar->SetStatus(abVisible,abVisible,!abVisible);
      if (mbAutoHide) {
        mwBotBar->SetStyle(mwBotBar->GetStyle()|eWidgetStyle_Free);
        mwBotBar->SetZOrder(eWidgetZOrder_OverlayBottom);
        mwBars->SetExclusive(abVisible);
        if (!mbAutoHideBackground &&
            mnActivePage < mvPages.size() &&
            mvPages[mnActivePage].IsEnabled())
        {
          const sPage& page = mvPages[mnActivePage];
          sVec2f pos = mwBotBar->GetPosition();
          pos.x = page.wButton->GetAbsolutePosition().x-
              ((mnActivePage == 0) ? 16.0f : 32.0f);
          if (pos.x > 0)
          {
            const tF32 fPageWidth = page.GetPageContentWidth()+8;
            const tF32 fRight = mpWidget->GetAbsoluteRect().GetRight();
            if (pos.x+fPageWidth > fRight) {
              pos.x -= (fPageWidth+pos.x)-fRight;
              pos.x = ni::Max(0.0f,pos.x); // still make sure it's above zero
              rectBottom.MoveTo(pos);
            }
            else {
              rectBottom.MoveTo(pos);
            }

            mwBotBar->SetRect(rectBottom);
          }
        }
      }
      else {
        mwBotBar->SetStyle(mwBotBar->GetStyle()&(~eWidgetStyle_Free));
        mwBotBar->SetZOrder(eWidgetZOrder_Background);
        mwBars->SetExclusive(ni::eFalse);
      }
    }
    else {
      mwBotBar->SetStatus(eFalse,eFalse,eTrue);
    }
    _UpdatePagesStatus();
    _ComputeSize();
  }
  virtual ni::tBool __stdcall GetShowPage() const {
    return mwBotBar->GetVisible();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetHeight(ni::tF32 afHeight) {
    mfHeight = ni::Clamp(afHeight,20.0f,256.0f);
  }
  virtual ni::tF32 __stdcall GetHeight() const {
    return mfHeight;
  }

  ///////////////////////////////////////////////
  virtual iWidget* __stdcall GetTopBar() const {
    return mwTopBar;
  }
  ///////////////////////////////////////////////
  virtual iWidget* __stdcall GetBottomBar() const {
    return mwBotBar;
  }

  ///////////////////////////////////////////////
  virtual ni::tU32 __stdcall GetNumTopWidgets() const {
    return (tU32)mvTopWidgets.size();
  }
  virtual ni::iWidget* __stdcall GetTopWidget(ni::tU32 anIndex) const {
    niCheckSilent(anIndex < mvTopWidgets.size(),NULL);
    return mvTopWidgets[anIndex].wWidget;
  }
  virtual ni::tU32 __stdcall GetTopWidgetIndex(iWidget* apWidget) const {
    niLoop(i,mvTopWidgets.size()) {
      if (mvTopWidgets[i].wWidget == apWidget)
        return (tU32)i;
    }
    return eInvalidHandle;
  }
  virtual iWidget* __stdcall GetTopWidgetFromID(iHString* ahspID) const {
    niLoop(i,mvTopWidgets.size()) {
      if (!mvTopWidgets[i].wWidget.IsOK())
        continue;
      if (mvTopWidgets[i].wWidget->GetID() == ahspID)
        return mvTopWidgets[i].wWidget;
    }
    return NULL;
  }
  virtual tBool __stdcall SetTopWidgetCommandDestination(tU32 anIndex, iMessageHandler* aCmdDest) {
    niCheckSilent(anIndex < mvTopWidgets.size(),eFalse);
    mvTopWidgets[anIndex].cmdDest = aCmdDest;
    return eTrue;
  }
  virtual iMessageHandler* __stdcall GetTopWidgetCommandDestination(tU32 anIndex) const  {
    niCheckSilent(anIndex < mvTopWidgets.size(),NULL);
    return mvTopWidgets[anIndex].cmdDest;
  }

  virtual tBool __stdcall AddTopWidget(iWidget* apWidget, ni::tF32 afWidth, iMessageHandler* aCmdDest) {
    niCheckIsOK(apWidget,ni::eFalse);
    sTopWidget w;
    w.wWidget = apWidget; // keep a ref of the widget first
    w.cmdDest = aCmdDest;
    w.fWidth = afWidth; // negative width to dock on the left instead of the right
    apWidget->SetParent(mwTopBar);
    mvTopWidgets.push_back(w);
    UpdateToolbar();
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual iWidget* __stdcall AddGroup(iHString* ahspPageID, iHString* ahspPageName, iHString* ahspGroupID, iHString* ahspGroupName, iMessageHandler* aCmdDest) {
    niCheck(niIsOK(mpWidget),NULL);

    iWidget* pPage = GetPageWidget(GetPageIndex(ahspPageID));
    if (!niIsOK(pPage)) {
      pPage = InitPage(ahspPageID,ahspPageName);
      if (!niIsOK(pPage)) {
        niError(niFmt(_A("Can't find page '%s' to add group '%s'."),niHStr(ahspPageID),niHStr(ahspGroupID)));
        return NULL;
      }
    }
    niCheck(niIsOK(pPage),NULL);

    Ptr<iWidget> w = mpWidget->GetUIContext()->CreateWidget(
        _A("ToolbarGroup"),pPage,
        sRectf::Null(),
        eWidgetStyle_MoveFocusGroup,ahspGroupID);
    niCheck(w.IsOK(),NULL);
    w->SetText(ahspGroupName);
    w->SetDockStyle(eWidgetDockStyle_DockLeft);
    QPtr<iWidgetToolbarGroup> g = w.ptr();
    if (!g.IsOK()) {
      niError(niFmt(_A("Can't get toolbar group '%s'."),niHStr(ahspGroupID)));
      return NULL;
    }
    g->SetCommandDestination(aCmdDest);
    g->SetWidthFromChildren();

    UpdateToolbar();
    return w.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  virtual iWidget* __stdcall GetGroup(ni::iHString* ahspID) const {
    niLoop(i,mvPages.size()) {
      iWidget* w = mvPages[i].wPage->FindWidget(ahspID);
      if (w) return w;
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall RemoveGroup(ni::iHString* ahspID) {
    iWidget* w = GetGroup(ahspID);
    if (w) {
      w->Invalidate();
      UpdateToolbar();
      return ni::eTrue;
    }
    return ni::eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCommandDestination(iMessageHandler* aCommandDestination) {
    if (mpWidget == aCommandDestination) {
      niWarning(_A("Trying to set itself as command destination."));
      return;
    }
    mCommandDestination = aCommandDestination;
  }
  virtual iMessageHandler* __stdcall GetCommandDestination() const {
    return mCommandDestination;
  }

 private:

  ni::tBool mbAutoHide;
  ni::tBool mbAutoHideBackground;
  ni::tF32  mfHeight;

  Ptr<iWidget>  mwBars;
  Ptr<iWidget>  mwTopBar;
  Ptr<iWidget>  mwBotBar;

  // selection
  struct sPage {
    Ptr<iWidget>  wButton;
    Ptr<iWidget>  wPage;

    void Invalidate() {
      if (wButton.IsOK()) {
        wButton->Invalidate();
        wButton = NULL;
      }
      if (wPage.IsOK()) {
        wPage->Invalidate();
        wPage = NULL;
      }
    }

    tBool IsEnabled() const{
      return (wButton.IsOK()) && (wButton->GetEnabled());
    }

    void SetEnabled(tBool abEnabled) {
      if (!wButton.IsOK()) return;
      if (abEnabled == IsEnabled()) return;
      if (abEnabled) {
        wButton->SetStatus(eTrue,eTrue,eFalse);
      }
      else {
        wButton->SetSkinClass(_H("ToolbarMenuButtonNormal"));
        wButton->SetStatus(eFalse,eFalse,eFalse);
        wPage->SetStatus(eFalse,eFalse,eFalse);
      }
    }

    tF32 GetPageContentWidth() const {
      tF32 fRight = 0.0f;
      niLoop(i,wPage->GetNumChildren()) {
        iWidget* c = wPage->GetChildFromIndex(i);
        fRight = ni::Max(fRight,c->GetRect().GetRight());
      }
      return fRight;
    }
  };
  typedef astl::vector<sPage> tPageVec;
  tPageVec        mvPages;
  tU32          mnActivePage;

  struct sTopWidget {
    Ptr<iWidget> wWidget;
    tF32         fWidth;
    Ptr<iMessageHandler> cmdDest;
  };
  typedef astl::vector<sTopWidget>  tTopWidgetVec;
  tTopWidgetVec     mvTopWidgets;

  // skin
  MemberPointer<sToolbarSkin> skin;
  void _UpdateSkinFlags() {
    niFlagOnIf(skin->flags,SKIN_AUTOHIDE,
               mbAutoHide);
    niFlagOnIf(skin->flags,SKIN_DRAWBACKFRAME,
               mbAutoHideBackground||!mbAutoHide);
  }

  Ptr<iMessageHandler> mCommandDestination;

  niEndClass(cWidgetToolbar);
};

ni::iWidgetSink* __stdcall New_WidgetToolbar() {
  return niNew cWidgetToolbar();
}

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------
class cWidgetToolbarGroup : public ni::cWidgetSinkImpl<ni::iWidgetToolbarGroup>
{
  niBeginClass(cWidgetToolbarGroup);

 public:
  //! Constructor.
  cWidgetToolbarGroup()  {
    mptrChildWidgetSink = niNew sChildWidgetSink(this);
  }

  //! Destructor.
  ~cWidgetToolbarGroup() {
    mptrChildWidgetSink->_this = NULL;
  }

  //! Sanity check.
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cWidgetToolbarGroup);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnSinkAttached() {
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnNCSize(const sVec2f& avNewSize) {
    if (skin.frameNormal.IsOK()) {
      mpWidget->SetClientRect(skin.frameNormal->ComputeFrameCenter(mpWidget->GetWidgetRect()));
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnNCPaint(const sVec2f& avMousePos, iCanvas* apCanvas) {
    const sRectf& rect = mpWidget->GetWidgetRect();
    iOverlay* frame = mpWidget->GetIsNcMouseOver() ? skin.frameHover : skin.frameNormal;
    sRectf rectText = rect;
    rectText.MoveTo(Vec2(0.0f,mpWidget->GetClientRect().Bottom()+1));
    rectText.SetHeight(frame->GetFrame().Bottom());

    apCanvas->BlitOverlayFrame(rect,frame,eRectFrameFlags_All);
    apCanvas->BlitText(
        mpWidget->GetFont(),rectText,
        eFontFormatFlags_CenterH|eFontFormatFlags_CenterV,
        niHStr(mpWidget->GetLocalizedText()));

    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall OnSkinChanged() {
    skin.frameNormal = mpWidget->FindSkinElement(NULL,_H("Normal"),_H("Frame"));
    skin.frameHover = mpWidget->FindSkinElement(NULL,_H("Hover"),_H("Frame"));
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall OnCommand(ni::iWidgetCommand* apCmd) {
    if (mCommandDestination.IsOK() && mCommandDestination != mpWidget) {
      ni::SendMessage(mCommandDestination,eUIMessage_Command,apCmd,niVarNull);
      return ni::eTrue;
    }
    return ni::eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetWidthFromChildren() {
    mpWidget->SetSize(Vec2(1000.0f,mpWidget->GetSize().y));
    tF32 fMaxX = mpWidget->GetFont()->ComputeTextSize(sRectf::Null(),niHStr(mpWidget->GetText()),eFontFormatFlags_CenterH).GetWidth();
    niLoop(i,mpWidget->GetNumChildren()) {
      iWidget* w = mpWidget->GetChildFromIndex(i);
      if (niIsOK(w) && !niFlagIs(w->GetStyle(),eWidgetStyle_Free) && !niFlagIs(w->GetStyle(),eWidgetStyle_NCRelative)) {
        fMaxX = ni::Max(fMaxX,w->GetRect().Right());
      }
    }
    fMaxX += skin.frameNormal->GetFrame().Left()+skin.frameNormal->GetFrame().Right();
    mpWidget->SetSize(Vec2(fMaxX,mpWidget->GetSize().y));
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCommandDestination(iMessageHandler* apMT) {
    if (mpWidget == apMT) {
      niWarning(_A("Trying to set itself as command destination."));
      return;
    }
    mCommandDestination = apMT;
  }
  virtual iMessageHandler* __stdcall GetCommandDestination() const {
    return mCommandDestination;
  }

 private:
  struct sSkin {
    Ptr<iOverlay> frameNormal;
    Ptr<iOverlay> frameHover;
  } skin;

  Ptr<iMessageHandler> mCommandDestination;

  struct sChildWidgetSink : public cWidgetSinkImpl<> {
    cWidgetToolbarGroup* _this;
    sChildWidgetSink(cWidgetToolbarGroup* apThis) : _this(apThis) {
    }
    virtual tBool __stdcall OnNCMouseEnter(const sVec2f& avMP, const sVec2f& avNC) {
      if (!_this) return ni::eFalse;
      return _this->OnNCMouseEnter(avMP,avNC);
    }
    virtual tBool __stdcall OnMouseEnter(const sVec2f& avMP, const sVec2f& avNC) {
      if (!_this) return ni::eFalse;
      return _this->OnMouseEnter(avMP,avNC);
    }
    virtual tBool __stdcall OnNCMouseLeave(const sVec2f& avMP, const sVec2f& avNC) {
      if (!_this) return ni::eFalse;
      return _this->OnNCMouseLeave(avMP,avNC);
    }
    virtual tBool __stdcall OnMouseLeave(const sVec2f& avMP, const sVec2f& avNC) {
      if (!_this) return ni::eFalse;
      return _this->OnMouseLeave(avMP,avNC);
    }
  };
  ni::Ptr<sChildWidgetSink> mptrChildWidgetSink;

  niEndClass(cWidgetToolbarGroup);
};

ni::iWidgetSink* __stdcall New_WidgetToolbarGroup() {
  return niNew cWidgetToolbarGroup();
}
#endif
