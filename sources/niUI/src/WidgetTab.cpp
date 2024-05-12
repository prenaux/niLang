// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetSplitter.h"
#include "WidgetTab.h"

// In UIContext.cpp
iWidget* _GetFocusActivateWidget(iWidget* apWidget);

///////////////////////////////////////////////
static tF32 _ComputeWidgetButtonWidthFromTextSize(ni::iWidget* apWidget, const sRectf& avMargin) {
  return apWidget->GetFont()->ComputeTextSize(sRectf::Null(),niHStr(apWidget->GetLocalizedText()),0).GetWidth();
}
///////////////////////////////////////////////
static tF32 _UpdateWidgetButtonSize(ni::iWidget* apWidget, const sRectf& avMargin, tF32 afTextSize) {
  iFont* font = apWidget->GetFont();
  tF32 w = afTextSize+avMargin.GetLeft()+avMargin.GetRight();
  tF32 h = font->GetHeight()+avMargin.GetTop()+avMargin.GetBottom();
  apWidget->SetSize(Vec2f(w,h));
  return h;
}

///////////////////////////////////////////////
class cTabPageContainer : public ImplRC<iWidgetSink>
{
 public:
  cTabPageContainer(cTabWidget *tw) {mpParent = tw; }
 private:
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
  {
    niGuardObject((iWidgetSink*)this);
    switch(nMsg)
    {
      case eUIMessage_NCSize: {
        const sRectf rect = apWidget->GetWidgetRect();
        sRectf clientRect = apWidget->GetWidgetRect();
        iOverlay* frame = mpParent->skin.normalFrame;
        if (frame) {
          const sVec4f vPadding = apWidget->GetPadding();
          clientRect = frame->ComputeFrameCenter(rect);
          clientRect.Left() += vPadding.Left();
          clientRect.Right() -= vPadding.Right();
          clientRect.Top() += vPadding.Top();
          clientRect.Bottom() -= vPadding.Bottom();
        }
        apWidget->SetClientRect(clientRect);
        break;
      }
      case eUIMessage_NCPaint: {
        iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
        if (c) {
          if (mpParent->_IsTooSmall()) {
          }
          else {
            sRectf rect = apWidget->GetWidgetRect();
            iOverlay* frame = mpParent->skin.normalFrame;
            tU32 mask = eRectFrameFlags_All & (~(eRectFrameFlags_Center));
            c->BlitOverlayFrame(rect,frame,mask);
            iWidget *selected = mpParent->_GetSelectedButton();
            sRectf selrect = rect;
            if(selected) {
              selrect = selected->GetWidgetRect();
              selrect.Move(selected->GetPosition());
            }
            if(selrect.GetLeft() != 0) {
              // draw left line
              c->BlitOverlayFrame(
                sRectf(0,0,selrect.GetLeft(),frame->GetFrame().y1()),
                frame,
                eRectFrameFlags_TopEdge|eRectFrameFlags_TopLeftCorner);
            }
            if(selrect.GetRight() != rect.GetRight()) {
              // draw right line
              c->BlitOverlayFrame(
                sRectf(selrect.GetRight(),0,rect.GetRight()-selrect.GetRight(),frame->GetFrame().y1()),
                frame,
                eRectFrameFlags_TopEdge|eRectFrameFlags_TopRightCorner);
            }
          }
        }
        break;
      }
      default: {
        return eFalse;
      }
    }
    return eTrue;
  }
  cTabWidget *mpParent;
};

///////////////////////////////////////////////
cTabWidget::cTabWidget(iWidget *pWidget)
{
  mpWidget = pWidget;
  mbNeedsAdjusting = eTrue;
  mbActive = eFalse;
  mbForceActivated = eFalse;
  mpPageContainer = NULL;
  mnMinNumPagesToShowTabs = 1;
}

///////////////////////////////////////////////
cTabWidget::~cTabWidget()
{
}

///////////////////////////////////////////////
void cTabWidget::_AdjustChildrenLayout()
{
  if (!mbNeedsAdjusting) return;
  if (mvecPages.empty()) return;
  mbNeedsAdjusting = eFalse;

  const tBool bIsActivated = _IsActivatedStyle();
  if (bIsActivated) {
    skin.normalFrame = skin.focusNormalFrame;
    skin.tabButtonFrame = skin.focusTabButtonFrame;
  }
  else {
    skin.normalFrame = skin.regularNormalFrame;
    skin.tabButtonFrame = skin.regularTabButtonFrame;
  }

  const tF32 w = mpWidget->GetSize().x;
  const tF32 h = mpWidget->GetSize().y;
  const tBool isTooSmall = _IsTooSmall();

  tF32 maxBtTextSize = 0.0f;
  tF32 maxBtHeight = 0.0f;

  // get the max button width
  for (tTabPageVec::const_iterator itr = mvecPages.begin(); itr != mvecPages.end(); ++itr) {
    maxBtTextSize = ni::Max(
      maxBtTextSize,
      _ComputeWidgetButtonWidthFromTextSize(itr->pButton,skin.tabButtonFrame->GetFrame()));
  }
  // apply the max button width to all buttons
  for (tTabPageVec::const_iterator itr = mvecPages.begin(); itr != mvecPages.end(); ++itr) {
    maxBtHeight = ni::Max(
      maxBtHeight,
      _UpdateWidgetButtonSize(itr->pButton,skin.tabButtonFrame->GetFrame(),maxBtTextSize));
  }

  tF32 ypos = 0;
  if (mvecPages.size() < mnMinNumPagesToShowTabs) {
    //         mvecLines.clear();
    //         mvecLines.push_back();
    //         mvecLines.back().push_back(*mvecPages.begin());
    mvecPages[0].pButton->SetPosition(sVec2f::Zero());
    for (tTabPageVec::const_iterator itr = mvecPages.begin(); itr != mvecPages.end(); ++itr) {
      itr->pButton->SetStatus(eFalse,eFalse,eFalse);
    }
  }
  else if (!mvecPages.empty()) {
    tTabPageVecVec mvecLines;
    const tF32 lineXStart = 0;
    const tBool isActivated = _IsActivatedStyle();

    // right to left, to define the primary order and build the lines
    {
      tU32 curNumLines = 1;
      niLoop(i,2) {
        mvecLines.clear();
        tTabPageVec *curline = &astl::push_back(mvecLines);

        const tU32 numLines = curNumLines;
        tF32 linex = lineXStart;
        if (numLines > 1 && mSelectedPage.pButton.IsOK()) {
          curline->push_back(mSelectedPage);
          linex += mSelectedPage.pButton->GetSize().x;
        }
        curNumLines = 1;
        niLoopr(ri,mvecPages.size()) {
          TabPage& p = mvecPages[ri];
          if (numLines > 1 && p.pButton == mSelectedPage.pButton)
            continue;
          tF32 buttonw = p.pButton->GetSize().x;
          if (linex+buttonw > w && (buttonw < w || linex != lineXStart)) {
            linex = lineXStart;
            curline = &astl::push_back(mvecLines);
            ++curNumLines;
          }
          curline->push_back(p);
          linex += buttonw;
        }
      }
    }

    // layout the buttons
    {
      tF32 liney = 0;
      niLoopr(ri,mvecLines.size()) {
        tTabPageVec& line = mvecLines[ri];
        tF32 linex = lineXStart;
        niLoopr(rj,line.size()) {
          TabPage& p = line[rj];
          tF32 buttonw = p.pButton->GetSize().x;
          if (isActivated) {
            p.pButton->SetSkinClass(_H("FocusTabButton"));
          }
          else {
            p.pButton->SetSkinClass(_H("TabButton"));
          }
          p.pButton->SetPosition(Vec2(linex,liney));
          p.pButton->SetStatus(eTrue,eTrue,isTooSmall);
          QPtr<iWidgetButton> bt = p.pButton.ptr();
          if (bt.IsOK()) {
            if (isTooSmall) {
              bt->SetCheck(eFalse);
              bt->SetDrawFrameFlags(eRectFrameFlags_Center);
            }
            else {
              bt->SetCheck(p.pButton == mSelectedPage.pButton);
              if (ri == 0) {
                bt->SetDrawFrameFlags(eRectFrameFlags_All);
              }
              else {
                bt->SetDrawFrameFlags(
                  eRectFrameFlags_All&(~eRectFrameFlags_BottomSide));
              }
            }
          }
          linex += buttonw;
        }
        liney += maxBtHeight;
      }
      ypos = liney; //-skin.tabButtonFrame->GetFrame().w;
    }
  }

  sRectf rectPage = sRectf(0,ypos,w,h-ypos);
  mpPageContainer->SetRect(rectPage);

  if (isTooSmall) {
    niLoopr(ri,mvecPages.size()) {
      TabPage& p = mvecPages[ri];
      if (!p.pPage.IsOK())
        continue;
      p.pPage->SetStatus(eFalse,eFalse,eTrue);
    }
  }
  else {
    niLoopr(ri,mvecPages.size()) {
      TabPage& p = mvecPages[ri];
      if (!p.pPage.IsOK())
        continue;
      if (p.pPage == mSelectedPage.pPage) {
        p.pPage->SetStatus(eTrue,eTrue,eFalse);
        p.pPage->SetRect(mpPageContainer->GetClientRect());
        p.pPage->SetZOrder(eWidgetZOrder_Bottom);
      }
      else {
        p.pPage->SetStatus(eFalse,eFalse,eTrue);
      }
    }
  }
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch(nMsg) {
    case eUIMessage_ChildAdded:
      {
#pragma niTodo("This is kinda a hack, investigate a better solution")
        Ptr<iWidget> ptrNew = ni::VarQueryInterface<iWidget>(varParam0);
        if (mpPageContainer != NULL && ptrNew.IsOK() &&
            !mpPageContainer->FindWidget(ptrNew->GetID()) &&
            (ptrNew->GetClassName() == _H("Dummy")))
        {
          AddPage(ptrNew->GetID(), ptrNew);
        }
        break;
      }
    case eUIMessage_SinkAttached:
      mpPageContainer = mpWidget->GetUIContext()->CreateWidget(_A("Canvas"),mpWidget,sRectf(),eWidgetStyle_DontSerialize);
      mpPageContainer->AddSink(niNew cTabPageContainer(this));
      mpPageContainer->SetZOrder(eWidgetZOrder_Bottom);
      InitSkin();
      return eFalse; //so other adapters can receive this message
    case eUIMessage_NCSize:
    case eUIMessage_Size:
      _NeedsAdjusting();
      _AdjustChildrenLayout();
      return eFalse;
    case eUIMessage_Paint:
      _AdjustChildrenLayout();
      return eTrue;
    case eUIMessage_NCPaint:
      _AdjustChildrenLayout();
      return eTrue;
    case eUIMessage_StyleChanged:
      _NeedsAdjusting();
      return eFalse;
    case eUIMessage_Activate:
      mbActive = eTrue;
      _NeedsAdjusting();
      break;
    case eUIMessage_Deactivate:
      mbActive = eFalse;
      _NeedsAdjusting();
      break;
    case eUIMessage_Notify:
      {
        QPtr<iWidget> ptrWidget = varParam0.mpIUnknown;
        QPtr<iMessageDesc> ptrMsg = varParam1;
        if (ptrWidget.IsOK() && ptrMsg.IsOK()) {
          if (ptrMsg->GetID() == eUIMessage_LeftClickDown || ptrMsg->GetID() == eUIMessage_RightClickDown) {
            _ActivateTab(ptrWidget);
          }
          else if (ptrMsg->GetID() == eUIMessage_RightClickUp) {
            if (ptrWidget.ptr() == mSelectedPage.pButton) {
              Ptr<iWidget> pwMenu = mpWidget->GetContextMenu();
              if (pwMenu.IsOK()) {
                QPtr<iWidgetMenu> ptrMenu = pwMenu.ptr();
                pwMenu->SetAbsolutePosition(ptrMsg->GetB().GetVec2fValue()+ptrWidget->GetAbsolutePosition());
                ptrMenu->Open();
              }
            }
          }
        }
        break;
      }
    case eUIMessage_SkinChanged:
      InitSkin();
      return eFalse;
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
void cTabWidget::_ActivateTab(iWidget *button)
{
  if (!button) {
    mSelectedPage.Reset();
    return;
  }
  else {
    _NeedsAdjusting();
    _AdjustChildrenLayout();
    tBool bFound = eFalse;
    iWidget *page = NULL;
    niLoop(i,mvecPages.size()) {
      TabPage& p = mvecPages[i];
      if (p.pButton == button) {
        page = p.pPage;
        bFound = eTrue;
        break;
      }
    }
    if (!bFound)
      return;
    if (mSelectedPage.pButton != button) {
      mSelectedPage.pButton = button;
      mSelectedPage.pPage = page;
      _NeedsAdjusting();
      _AdjustChildrenLayout();
    }
  }
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetTabCmd_ActivatePage,mpWidget);
}

///////////////////////////////////////////////
void __stdcall cTabWidget::AddPage(iHString* ahspName, iWidget *apPage) {
  if (!niIsOK(mpWidget)) return;
  TabPage page;
  page.pButton = mpWidget->GetUIContext()->CreateWidget(
      _A("Button"),mpWidget,
      sRectf(0,0,10,10),
      eWidgetStyle_NotifyParent|eWidgetStyle_DontSerialize);
  if (_IsActivatedStyle()) {
    page.pButton->SetSkinClass(_H("FocusTabButton"));
  }
  else {
    page.pButton->SetSkinClass(_H("TabButton"));
  }
  page.pButton->SetText(ahspName);
  QPtr<iWidgetButton>(page.pButton)->SetGroupID(_H("TabButtons"));
  page.pPage = apPage;
  if (page.pPage.IsOK()) {
    page.pPage->SetVisible(eFalse);
    page.pPage->SetParent(mpPageContainer);
  }
  mvecPages.push_back(page);
  _NeedsAdjusting();
  _ActivateTab(page.pButton);
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetTabCmd_AddPage);
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::RemovePage(iWidget* apPage) {
  tU32 i;
  for (i = 0; i < mvecPages.size(); ++i) {
    if (mvecPages[i].pPage == apPage) {
      break;
    }
  }
  if (i == mvecPages.size()) {
    return eFalse;
  }
  mvecPages[i].pButton->SetParent(NULL);
  mvecPages.erase(mvecPages.begin()+i);
  _NeedsAdjusting();
  if (GetNumPages())
    SetActivePage(GetPage((i+1)%GetNumPages()));
  else
    SetActivePage(NULL);
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetTabCmd_RemovePage);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::HasPage(iWidget* apPage) const {
  for (tTabPageVec::const_iterator it = mvecPages.begin(); it != mvecPages.end(); ++it) {
    if (it->pPage == apPage) {
      return eTrue;
    }
  }
  return eFalse;
}

///////////////////////////////////////////////
tU32 __stdcall cTabWidget::GetNumPages() const {
  return mvecPages.size();
}

///////////////////////////////////////////////
iWidget* __stdcall cTabWidget::GetPage(tU32 anIndex) const {
  if (anIndex >= GetNumPages()) return NULL;
  return mvecPages[anIndex].pPage;
}


///////////////////////////////////////////////
iWidget* __stdcall cTabWidget::GetPageButton(tU32 anIndex) const {
  if (anIndex >= GetNumPages()) return NULL;
  return mvecPages[anIndex].pButton;
}

///////////////////////////////////////////////
iWidget* __stdcall cTabWidget::GetPageFromName(iHString* ahspName) const {
  for (tTabPageVec::const_iterator it = mvecPages.begin(); it != mvecPages.end(); ++it) {
    if (it->pButton->GetText() == ahspName) {
      return it->pPage;
    }
  }
  return NULL;
}

///////////////////////////////////////////////
iWidget* __stdcall cTabWidget::GetPageFromID(iHString* ahspID) const
{
  for (tTabPageVec::const_iterator it = mvecPages.begin(); it != mvecPages.end(); ++it) {
    if (it->pPage->GetID() == ahspID) {
      return it->pPage;
    }
  }
  return NULL;
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::SetPageName(iWidget* apPage, iHString* ahspName) {
  for (tTabPageVec::const_iterator it = mvecPages.begin(); it != mvecPages.end(); ++it) {
    if (it->pPage == apPage) {
      it->pButton->SetText(ahspName);
      _NeedsAdjusting();
      return eTrue;
    }
  }
  return eFalse;
}

///////////////////////////////////////////////
iHString* __stdcall cTabWidget::GetPageName(iWidget* apPage) const {
  for (tTabPageVec::const_iterator it = mvecPages.begin(); it != mvecPages.end(); ++it) {
    if (it->pPage == apPage) {
      return it->pButton->GetText();
    }
  }
  return NULL;
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::SetPageIndex(iWidget* apPage, tU32 anIndex) {
  niCheck(anIndex < GetNumPages(),eFalse);

  tU32 nPageIndex = GetPageIndex(apPage);
  niCheck(nPageIndex != eInvalidHandle,eFalse);
  iWidget* pActivePage = GetActivePage();
  if (anIndex != nPageIndex) {
    ni::Swap(mvecPages[anIndex],mvecPages[nPageIndex]);
    _NeedsAdjusting();
  }
  SetActivePage(pActivePage);
  return eTrue;
}

///////////////////////////////////////////////
tU32 __stdcall cTabWidget::GetPageIndex(iWidget* apPage) const {
  tU32 nCount = 0;
  for (tTabPageVec::const_iterator it = mvecPages.begin(); it != mvecPages.end(); ++it, ++nCount) {
    if (it->pPage == apPage) {
      return nCount;
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::SetActivePage(iWidget* apPage) {
  TabPage page;
  if (apPage) {
    for (tTabPageVec::iterator it = mvecPages.begin(); it != mvecPages.end(); ++it) {
      if (it->pPage == apPage) {
        page = *it;
        break;
      }
    }
  }
  _ActivateTab(page.pButton);
  return page.IsNull()?eFalse:eTrue;
}

///////////////////////////////////////////////
iWidget* __stdcall cTabWidget::GetActivePage() const {
  return mSelectedPage.pPage;
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::SetActivePageName(iHString* ahspName) {
  return SetActivePage(GetPageFromName(ahspName));
}

///////////////////////////////////////////////
iHString* __stdcall cTabWidget::GetActivePageName() const {
  if (mSelectedPage.IsNull()) return NULL;
  return mSelectedPage.pButton->GetText();
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::SetActivePageID(iHString* ahspID)
{
  return SetActivePage(GetPageFromID(ahspID));
}

///////////////////////////////////////////////
iHString* __stdcall cTabWidget::GetActivePageID() const
{
  if (mSelectedPage.IsNull()) return NULL;
  return mSelectedPage.pPage->GetID();
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::SetActivePageIndex(tU32 anIndex) {
  anIndex = ni::Clamp<tU32>(anIndex,0,GetNumPages()-1);
  return SetActivePage(GetPage(anIndex));
}

///////////////////////////////////////////////
tU32 __stdcall cTabWidget::GetActivePageIndex() const {
  if (mSelectedPage.IsNull()) return eInvalidHandle;
  return GetPageIndex(mSelectedPage.pPage);
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::MovePageLeft(iWidget* apPage, tU32 anLeft) {
  tU32 index = GetPageIndex(apPage);
  if (index == eInvalidHandle)
    return eFalse;
  if (anLeft >= index)
    SetPageIndex(apPage,0);
  else
    SetPageIndex(apPage,index-anLeft);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cTabWidget::MovePageRight(iWidget* apPage, tU32 anRight) {
  tU32 index = GetPageIndex(apPage);
  if (index == eInvalidHandle)
    return eFalse;
  if (anRight+index >= GetNumPages())
    SetPageIndex(apPage,GetNumPages()-1);
  else
    SetPageIndex(apPage,anRight+index);
  return eTrue;
}

///////////////////////////////////////////////
void cTabWidget::InitSkin() {
  skin.regularNormalFrame = mpWidget->FindSkinElement(_H("TabPage"),NULL,_H("Frame"));
  skin.regularTabButtonFrame = mpWidget->FindSkinElement(_H("TabButton"),NULL,_H("Frame"));
  skin.focusNormalFrame = mpWidget->FindSkinElement(_H("FocusTabPage"),NULL,_H("Frame"));
  skin.focusTabButtonFrame = mpWidget->FindSkinElement(_H("FocusTabButton"),NULL,_H("Frame"));
  //     _UpdateStyle();
  _NeedsAdjusting();
}

///////////////////////////////////////////////
void cTabWidget::_NeedsAdjusting() {
  mbNeedsAdjusting = eTrue;
}

///////////////////////////////////////////////
tBool cTabWidget::_IsActivatedStyle() const {
  return mbForceActivated || mbActive;
}
void __stdcall cTabWidget::SetForceActivated(tBool abForce) {
  tBool bUpdateStyle = (mbForceActivated != abForce);
  mbForceActivated = abForce;
  if (bUpdateStyle) {
    //         _UpdateStyle();
    _NeedsAdjusting();
  }
}
tBool __stdcall cTabWidget::GetForceActivated(tBool abForce) const {
  return mbForceActivated;
}

///////////////////////////////////////////////
void __stdcall cTabWidget::SetMinNumPagesToShowTabs(tU32 anMinNumPages) {
  mnMinNumPagesToShowTabs = anMinNumPages;
  _NeedsAdjusting();
}
tU32 __stdcall cTabWidget::GetMinNumPagesToShowTabs() const {
  return mnMinNumPagesToShowTabs;
}

///////////////////////////////////////////////
tBool cTabWidget::_IsTooSmall() const {
  const sVec2f containerSize = mpWidget->GetSize();
  return containerSize.x < kSplitterFoldedSize ||
      containerSize.y < kSplitterFoldedSize;
}
