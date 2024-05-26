// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetDockingManager.h"

//#define OVERLAY_DEBUG

#define DOCKBUTTON_WIDTH  50
#define DOCKBUTTON_HEIGHT 40

#define SPLITTERID_PREFIX _A("ID_DockSplitter")
#define DOCKTAB_PREFIX    _A("DockTab")
#define DOCKAREA_PREFIX   _A("DockArea")

#define TAB_STYLE (eWidgetStyle_FocusActivate|eWidgetStyle_HoldFocus)
#define TOOLBAR_STYLE (eWidgetStyle_FocusActivate|eWidgetStyle_HoldFocus)

niDefConstHString(TabMenu_Undock);
niDefConstHString(TabMenu_MoveLeft);
niDefConstHString(TabMenu_MoveRight);

const tF32 _kfNewDockWidth = 0.3f;
const tF32 _kfNewDockHeight = 0.3f;
const tF32 _kfNewLocalDockWidth = 0.3f;
const tF32 _kfNewLocalDockHeight = 0.5f;
const tU32 _kcolButtonBorder = ULColorBuildf(0,0,0,1);
const tU32 _kcolButtonBorderSelected = ULColorBuildf(0,0,0,1);
const tU32 _kcolButtonSelected = ULColorBuild(60,122,241,255);
const tU32 _kcolGlobalButton = ULColorBuild(220,220,134,255);
const tU32 _kcolLocalButton = ULColorBuild(134,172,246,255);
const tU32 _kcolOverlay = ULColorBuild(82,92,120,80);
const tU32 _kcolOverlayBorder = ULColorBuild(127,127,127,127);
const tF32 _kfSplitterBorderSize = 5.0f;

static iHString* GetUniqueID(iWidget* apRoot, const achar* aaszBaseName) {
  tHStringPtr hsp;
  iWidget* w;
  tU32 nCount = 0;
  do {
    hsp = _H(niFmt(_A("%s%d"),aaszBaseName,nCount++));
    w = apRoot->FindWidget(hsp);
  } while (w);
  return hsp.GetRawAndSetNull();
}

static iHString* GetNewSplitterID(iWidget* apRoot) {
  return GetUniqueID(apRoot,SPLITTERID_PREFIX);
}
static iHString* GetNewTabID(iWidget* apRoot) {
  return GetUniqueID(apRoot,DOCKTAB_PREFIX);
}
static iHString* GetNewDockAreaID(iWidget* apRoot) {
  return GetUniqueID(apRoot,DOCKAREA_PREFIX);
}

///////////////////////////////////////////////
static iWidget* CreateVerticalDockArea(tWidgetDockingManagerFlags aDockingFlags, iWidget* apRoot, iWidget* apParent, sRectf aRect, tBool abLeft, tBool abResizable, tBool abDock, iWidget* apwTab = NULL)
{
  // vertical splitter
  iWidget* pNew = apParent->GetUIContext()->CreateWidget(
      _A("Splitter"),apParent,aRect,
      eWidgetSplitterStyle_Empty,
      GetNewSplitterID(apRoot));
  QPtr<iWidgetSplitter> ptrSplitter = pNew;
  if (abResizable) {
    ptrSplitter->SetSplitterBorderSize(_kfSplitterBorderSize);
    ptrSplitter->SetSplitterResizableBorders(abLeft?eRectEdges_Right:eRectEdges_Left);
  }
  if (abDock) {
    pNew->SetDockStyle(abLeft?eWidgetDockStyle_DockLeft:eWidgetDockStyle_DockRight);
  }
  else {
    pNew->SetDockStyle(eWidgetDockStyle_DockFill);
  }

  if (apwTab) {
    apwTab->SetParent(ptrSplitter->GetSplitterWidget(0));
    apwTab->SetDockStyle(eWidgetDockStyle_DockFill);
  }
  else {
    iWidget* pTab = apParent->GetUIContext()->CreateWidget(_A("Tab"),ptrSplitter->GetSplitterWidget(0),sRectf(),TAB_STYLE,GetNewTabID(apRoot));
    pTab->SetDockStyle(eWidgetDockStyle_DockFill);
    if (niFlagIs(aDockingFlags,eWidgetDockingManagerFlags_HideTabIfOnePage)) {
      QPtr<iWidgetTab>(pTab)->SetMinNumPagesToShowTabs(2);
    }
  }

  pNew->SetZOrder(eWidgetZOrder_Background);
  return pNew;
}

///////////////////////////////////////////////
static iWidget* CreateHorizontalDockArea(tWidgetDockingManagerFlags aDockingFlags, iWidget* apRoot, iWidget* apParent, sRectf aRect, tBool abTop, tBool abResizable, tBool abDock)
{
  iWidget* pNew = apParent->GetUIContext()->CreateWidget(
      _A("Splitter"),apParent,aRect,
      eWidgetSplitterStyle_Empty|eWidgetSplitterStyle_Horizontal,
      GetNewSplitterID(apRoot));
  QPtr<iWidgetSplitter> ptrSplitter = pNew;
  if (abResizable) {
    ptrSplitter->SetSplitterBorderSize(_kfSplitterBorderSize);
    ptrSplitter->SetSplitterResizableBorders(abTop?eRectEdges_Bottom:eRectEdges_Top);
  }
  if (abDock) {
    pNew->SetDockStyle(abTop?eWidgetDockStyle_DockTop:eWidgetDockStyle_DockBottom);
  }
  else {
    pNew->SetDockStyle(eWidgetDockStyle_DockFill);
  }

  iWidget* pTab = apParent->GetUIContext()->CreateWidget(_A("Tab"),ptrSplitter->GetSplitterWidget(0),sRectf(),TAB_STYLE,GetNewTabID(apRoot));
  pTab->SetDockStyle(eWidgetDockStyle_DockFill);
  if (niFlagIs(aDockingFlags,eWidgetDockingManagerFlags_HideTabIfOnePage)) {
    QPtr<iWidgetTab>(pTab)->SetMinNumPagesToShowTabs(2);
  }

  pNew->SetZOrder(eWidgetZOrder_Background);
  return pNew;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Overlay widget

tBool __stdcall cWidgetDockingManager::sOverlayWidgetSink::OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB)
{
  niGuardObject((iWidgetSink*)this);
  switch (anMsg) {
    case eUIMessage_Paint:
      {
        iCanvas* c = VarQueryInterface<iCanvas>(aB);
        if (c) {
          for (tI32 i = 0; i < cWidgetDockingManager::eDockButton_Last; ++i) {
            cWidgetDockingManager::sDockButton& button = mpDockManager->mButtons[i];
            if (!button.mbHover || !button.mbActive) {
              continue;
            }
            if (button.mPosition != 0) {
              button.mTabRect = mpDockManager->GetDockPositionRect(button.mPosition,button.mpDockArea,mpDockManager->mpWidget->GetAbsolutePosition(),i<=eDockButton_LocalTab);
              sVec2f tl = button.mTabRect.GetTopLeft();
              sVec2f br = button.mTabRect.GetBottomRight();
              // left
              c->BlitFillAlpha(sRectf(tl,Vec2<tF32>(tl.x+2,br.y)),_kcolOverlayBorder);
              // right
              c->BlitFillAlpha(sRectf(Vec2<tF32>(br.x-2,tl.y),br),_kcolOverlayBorder);
              // top
              c->BlitFillAlpha(sRectf(Vec2<tF32>(tl.x+2,tl.y),Vec2<tF32>(br.x-2,tl.y+2)),_kcolOverlayBorder);
              // bottom
              c->BlitFillAlpha(sRectf(Vec2<tF32>(tl.x+2,br.y-2),Vec2<tF32>(br.x-2,br.y)),_kcolOverlayBorder);
              // center
              c->BlitFillAlpha(sRectf(tl+Vec2<tF32>(2,2),br-Vec2<tF32>(2,2)),_kcolOverlay);
            }
            break;
          }

          for (tU32 i = 0; i < cWidgetDockingManager::eDockButton_Last; ++i) {
            cWidgetDockingManager::sDockButton& button = mpDockManager->mButtons[i];
            if (!button.mbActive) continue;
            sVec2f tl = button.mRect.GetTopLeft();
            sVec2f br = button.mRect.GetBottomRight();
            c->BlitFillAlpha(sRectf(tl-sVec2f::One(),br+sVec2f::One()),
                             button.mbHover ?
                             _kcolButtonBorderSelected :
                             _kcolButtonBorder);
            c->BlitFillAlpha(sRectf(tl,br),
                             button.mbHover ?
                             _kcolButtonBorder :
                             button.mnColor);
          }

#ifdef OVERLAY_DEBUG
          cString strText = niFmt(_A("NumDockAreas:%d, NumWidgets:%d\n"),mpDockManager->GetNumDockAreas(),apWidget->GetUIContext()->GetNumWidgets());
          for (tU32 i = 0; i < mpDockManager->GetNumDockAreas(); ++i) {
            sRectf rect = mpDockManager->GetDockArea(i)->GetAbsoluteRect();
            strText += niFmt(_A("DockArea[%02d]:%s,(%.2f,%.2f,%.2f,%.2f)\n"),
                             i,mpDockManager->GetDockArea(i)->GetID()->GetString(),
                             rect.GetLeft(),rect.GetTop(),rect.GetWidth(),rect.GetHeight());
          }
          apWidget->GetFont()->DrawText(5,5,strText);
#endif
        }
        break;
      }
    default:
      return eFalse;
  }
  return eTrue;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetDockingManager implementation.

///////////////////////////////////////////////
cWidgetDockingManager::cWidgetDockingManager(ni::iWidget *apWidget)
{
  ZeroMembers();
  mpWidget = apWidget;
  mnFlags = eWidgetDockingManagerFlags_Default;

  // Create the overlay widget
  mpwOverlay = apWidget->GetUIContext()->CreateWidget(_A("Canvas"),mpWidget,mpWidget->GetRect(),eWidgetStyle_Free|eWidgetStyle_NoClip,_H("ID_Overlay"));
  mpwOverlay->AddSink(niNew sOverlayWidgetSink(this));
  mpwOverlay->SetZOrder(eWidgetZOrder_Overlay);
  mpwOverlay->SetStatus(eFalse,eFalse,eTrue);

  // Create the document tab
  mpwTab = apWidget->GetUIContext()->CreateWidget(_A("Tab"),mpWidget,sRectf(),TAB_STYLE,_H("ID_RootDockTab"));
  mpwTab->SetZOrder(eWidgetZOrder_Background);
  mpwTab->SetStatus(eFalse,eFalse,eFalse);
  mpwTab->SetDockStyle(eWidgetDockStyle_DockFill);

  // Initialize the dock buttons
  mButtons[eDockButton_Left].mPosition = eRectEdges_Left;
  mButtons[eDockButton_Left].mnColor = _kcolGlobalButton;
  mButtons[eDockButton_Right].mPosition = eRectEdges_Right;
  mButtons[eDockButton_Right].mnColor = _kcolGlobalButton;
  mButtons[eDockButton_Top].mPosition = eRectEdges_Top;
  mButtons[eDockButton_Top].mnColor = _kcolGlobalButton;
  mButtons[eDockButton_Bottom].mPosition = eRectEdges_Bottom;
  mButtons[eDockButton_Bottom].mnColor = _kcolGlobalButton;
  mButtons[eDockButton_LocalLeft].mPosition = eRectEdges_Left;
  mButtons[eDockButton_LocalLeft].mnColor = _kcolLocalButton;
  mButtons[eDockButton_LocalRight].mPosition = eRectEdges_Right;
  mButtons[eDockButton_LocalRight].mnColor = _kcolLocalButton;
  mButtons[eDockButton_LocalTop].mPosition = eRectEdges_Top;
  mButtons[eDockButton_LocalTop].mnColor = _kcolLocalButton;
  mButtons[eDockButton_LocalBottom].mPosition = eRectEdges_Bottom;
  mButtons[eDockButton_LocalBottom].mnColor = _kcolLocalButton;
  mButtons[eDockButton_LocalTab].mPosition = 0;
  mButtons[eDockButton_LocalTab].mnColor = _kcolLocalButton;
  for (tU32 i = 0; i < eDockButton_Last; ++i) {
    mButtons[i].mpDockArea = mpWidget;
  }

  SetDefaultDockAreaTabMenu();

  SetFlags(mnFlags);
}

///////////////////////////////////////////////
cWidgetDockingManager::~cWidgetDockingManager()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetDockingManager::ZeroMembers()
{
  mpWidget = NULL;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetDockingManager::IsOK() const
{
  niClassIsOK(cWidgetDockingManager);
  return ni::eTrue;
}

///////////////////////////////////////////////
tBool cWidgetDockingManager::SetFlags(tWidgetDockingManagerFlags aFlags)
{
  if (mnFlags != aFlags) {
    mnFlags = aFlags;
    _ApplyFlags();
  }
  return eTrue;
}
void cWidgetDockingManager::_ApplyFlags() {
  // Apply eWidgetDockingManagerFlags_HideTabIfOnePage
  niLoop(i,GetNumDockAreas()) {
    iWidget* pDock = GetDockArea(i);
    Ptr<iWidget> pwTab;
    QPtr<iWidgetTab> ptrTab;
    if (pDock == mpWidget) {
      pwTab = mpwTab;
      ptrTab = mpwTab;
    }
    else {
      Ptr<iWidget>    pwSplitter = pDock->GetParent();
      QPtr<iWidgetSplitter> ptrSplitter = pwSplitter.ptr();
      tU32 k = 0;
      if (ptrSplitter.IsOK()) {
        for (k = 0; k < ptrSplitter->GetNumSplitterWidgets(); ++k) {
          if (ptrSplitter->GetSplitterWidget(k) == pDock) {
            break;
          }
        }
        k = (k == ptrSplitter->GetNumSplitterWidgets())?0:k;
      }
      pwTab = pDock->GetChildFromIndex(0);
      ptrTab = pwTab;
    }
    if (ptrTab.IsOK()) {
      if (niFlagIs(mnFlags,eWidgetDockingManagerFlags_HideTabIfOnePage)) {
        ptrTab->SetMinNumPagesToShowTabs(2);
      }
      else {
        ptrTab->SetMinNumPagesToShowTabs(1);
      }
    }
  }
}

///////////////////////////////////////////////
tWidgetDockingManagerFlags cWidgetDockingManager::GetFlags() const
{
  return mnFlags;
}

///////////////////////////////////////////////
tU32 cWidgetDockingManager::GetDockAreaFromPageName(const achar *aaszName) const
{
  for (tU32 i = 0; i < GetNumDockAreas(); ++i) {
    QPtr<iWidgetTab> ptrTab = GetDockAreaTab(i);
    if (ptrTab.IsOK()) {
      for (tU32 j = 0; j < ptrTab->GetNumPages(); ++j) {
        if (ni::StrEq(niHStr(ptrTab->GetPageName(ptrTab->GetPage(j))),aaszName))
          return i;
      }
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetDockingManager::GetDockAreaFromWidget(iWidget* apWidget) const
{
  for (tU32 i = 0; i < GetNumDockAreas(); ++i) {
    QPtr<iWidgetTab> ptrTab = (i==0)?mpwTab.ptr():GetDockArea(i)->GetChildFromIndex(0);
    if (ptrTab.IsOK()) {
      for (tU32 j = 0; j < ptrTab->GetNumPages(); ++j) {
        if (apWidget == ptrTab->GetPage(j))
          return i;
      }
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetDockingManager::GetDockAreaFromDockAreaWidget(iWidget* apWidget) const
{
  for (tU32 i = 0; i < GetNumDockAreas(); ++i) {
    if (GetDockArea(i) == apWidget) {
      return i;
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tU32 cWidgetDockingManager::GetDockAreaHovered(sVec2f avPos) const
{
  for (tI32 i = GetNumDockAreas()-1; i >= 0; --i) {
    iWidget* pDockArea = GetDockArea(i);
    if (pDockArea->GetAbsoluteRect().Intersect(avPos))
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tBool cWidgetDockingManager::DockWidget(tU32 anDock, iWidget *apWidget)
{
  QPtr<iWidgetDockable> ptrDockable = apWidget;
  if (!ptrDockable.IsOK()) {
    niError(_A("Invalid widget, doesnt implement iWidgetDockable."));
    return eFalse;
  }

  iWidget* pDock = GetDockArea(anDock);
  if (!pDock) {
    niWarning(niFmt(_A("Invalid dock area '%d'."),anDock));
    return eFalse;
  }

  Ptr<iWidget> pwTab;
  QPtr<iWidgetTab> ptrTab;
  if (pDock == mpWidget) {
    pwTab = mpwTab;
    ptrTab = mpwTab;
  }
  else {
    pwTab = pDock->GetChildFromIndex(0);
    ptrTab = pwTab;
  }

  if (!ptrTab.IsOK()) {
    niError(_A("The dock area is invalid, doesnt contain a tab widget."));
    return eFalse;
  }

  apWidget->SendMessage(eUIMessage_BeforeDocked,mpWidget);
  ptrTab->AddPage(_H(ptrDockable->GetDockName()),apWidget);
  apWidget->SendMessage(eUIMessage_AfterDocked,pwTab.ptr());
  apWidget->SetDockStyle(eWidgetDockStyle_DockFill);
  apWidget->SetZOrder(eWidgetZOrder_Background);
  pwTab->SetStatus(eTrue,eTrue,eFalse);
  mvDocked.push_back(apWidget);
  return eFalse;
}

///////////////////////////////////////////////
tBool cWidgetDockingManager::UndockWidget(iWidget *apWidget)
{
  if (!niIsOK(apWidget)) {
    niError(_A("Invalid widget."));
    return eFalse;
  }

  tU32 nDockArea = GetDockAreaFromWidget(apWidget);
  if (nDockArea == eInvalidHandle) {
    niError(niFmt(_A("Widget (ID:%s) not docked."),HStringGetStringEmpty(apWidget->GetID())));
    return eFalse;
  }

  Ptr<iWidget> ptrDockArea = GetDockArea(nDockArea);
  Ptr<iWidget> pwTab;
  QPtr<iWidgetTab> ptrTab;
  if (ptrDockArea == mpWidget) {
    pwTab = mpwTab;
    ptrTab = mpwTab;
  }
  else {
    pwTab = ptrDockArea->GetChildFromIndex(0);
    ptrTab = pwTab;
    if (!ptrTab.IsOK()) {
      niError(_A("The dock area is invalid, doesnt contain one tab widget."));
      return eFalse;
    }

  }

  ptrTab->RemovePage(apWidget);
  apWidget->SetParent(mpWidget->GetUIContext()->GetRootWidget());
  astl::find_erase(mvDocked,apWidget);

  QPtr<iWidgetDockable> ptrDockable = apWidget;
  if (ptrDockable.IsOK()) {
    apWidget->SendMessage(eUIMessage_Undocked);
  }

  if (ptrTab->GetNumPages() == 0) {
    if (ptrDockArea == mpWidget) {
      pwTab->SetStatus(eFalse,eFalse,eFalse);
    }
    else {
      // Get the tab's splitter
      Ptr<iWidget>    pwSplitter = ptrDockArea->GetParent();
      QPtr<iWidgetSplitter> ptrSplitter = pwSplitter.ptr();
      niAssert(ptrSplitter.IsOK());
      if (ptrSplitter->GetNumSplitters() >= 1) {
        tBool bRes = ptrSplitter->RemoveSplitterWidget(ptrSplitter->GetSplitterWidgetIndex(ptrDockArea));
        niUnused(bRes);
        niAssert(bRes);
      }
      else {
        RemoveSplitter(pwSplitter);
      }
      CleanSplitters();
      CleanDockAreas();
    }
  }

  if (mpwTab.IsOK()) {
    // makes sure the root dock tab is where it should be
    mpwTab->SetZOrder(eWidgetZOrder_Background);
  }
  return eTrue;
}

///////////////////////////////////////////////
tU32 cWidgetDockingManager::GetNumDockedWidgets() const
{
  return (tU32)mvDocked.size();
}

///////////////////////////////////////////////
iWidget * cWidgetDockingManager::GetDockedWidget(tU32 anIndex) const
{
  if (anIndex >= GetNumDockedWidgets()) return NULL;
  return mvDocked[anIndex];
}

///////////////////////////////////////////////
tU32 cWidgetDockingManager::GetNumDockAreas() const
{
  return (tU32)mvDockAreas.size()+1;
}

///////////////////////////////////////////////
iWidget * cWidgetDockingManager::GetDockArea(tU32 anIndex) const
{
  if (anIndex == 0) {
    return mpWidget;
  }
  else {
    if (anIndex >= GetNumDockAreas()) {
      return NULL;
    }
    return mvDockAreas[anIndex-1];
  }
}

///////////////////////////////////////////////
tBool __stdcall cWidgetDockingManager::OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB)
{
  niGuardObject((iWidgetSink*)this);
  switch (anMsg) {
    case eUIMessage_Destroy:
      {
        ClearDockAreas();
        break;
      }
    case eUIMessage_Size:
      {
        Resize(aA.mV2F[0],aA.mV2F[1],
               aB.mV2F[0],aB.mV2F[1]);
        break;
      }
    case eUIMessage_StyleChanged:
      {
        break;
      };
    case eUIMessage_SkinChanged: {
      mSkin.mnClearColor = ULColorBuild(mpWidget->FindSkinColor(sColor4f::Zero(),NULL,NULL,_H("Background")));
      break;
    }
    case eUIMessage_Paint: {
      if (ULColorGetA(mSkin.mnClearColor)) {
        iCanvas* c = VarQueryInterface<iCanvas>(aB);
        if (c) {
          c->BlitFill(mpWidget->GetWidgetRect(), mSkin.mnClearColor);
        }
      }
      return ni::eTrue;
    }
    case eUIMessage_Command:
      {
        Ptr<iWidgetCommand> ptrCmd = ni::VarQueryInterface<iWidgetCommand>(aA);
        niAssert(ptrCmd.IsOK());
        if (ptrCmd->GetSender() == mpwDockAreaTabContextMenu)
        {
          QPtr<iWidgetMenu> ptrMenu = ptrCmd->GetSender();
          if (ptrCmd->GetID() == eWidgetMenuCmd_Clicked)
          {
            Ptr<iWidgetMenuItem> ptrItem = ni::VarQueryInterface<iWidgetMenuItem>(ptrCmd->GetExtra1());
            niAssert(ptrItem.IsOK());
            if (ptrItem->GetID() == niGetConstHString(TabMenu_Undock))
            {
              QPtr<iWidgetTab> ptrTab = mpWidget->GetUIContext()->GetActiveWidget();
              if (ptrTab.IsOK()) {
                Ptr<iWidget> ptrDocked = ptrTab->GetPage(ptrTab->GetActivePageIndex());
                if (ptrDocked.IsOK()) {
                  UndockWidget(ptrDocked);
                }
              }
            }
            else if (ptrItem->GetID() == niGetConstHString(TabMenu_MoveLeft))
            {
              QPtr<iWidgetTab> ptrTab = mpWidget->GetUIContext()->GetActiveWidget();
              if (ptrTab.IsOK()) {
                ptrTab->MovePageLeft(ptrTab->GetActivePage(),1);
              }
            }
            else if (ptrItem->GetID() == niGetConstHString(TabMenu_MoveRight))
            {
              QPtr<iWidgetTab> ptrTab = mpWidget->GetUIContext()->GetActiveWidget();
              if (ptrTab.IsOK()) {
                ptrTab->MovePageRight(ptrTab->GetActivePage(),1);
              }
            }
          }
        }
        break;
      }
    case eWidgetDockingManagerMessage_BeginMove:
      {
        mpwOverlay->SetStatus(eTrue,eTrue,eTrue);
        for (tU32 i = 0; i < eDockButton_Last; ++i) {
          sDockButton& button = mButtons[i];
          button.mbHover = eFalse;
        }
        break;
      }
    case eWidgetDockingManagerMessage_EndMove:
      {
        Ptr<iWidget> pwWidget = ni::VarQueryInterface<iWidget>(aA);
        if (pwWidget.IsOK()) {
          if (!QPtr<iWidgetDockable>(pwWidget).IsOK()) {
            niWarning(_A("EndMove: Widget not dockable."));
          }
          else {
            tU32 nNewDock = eInvalidHandle;
            for (tU32 i = 0; i < eDockButton_Last; ++i) {
              sDockButton& button = mButtons[i];
              if (!button.mbHover) continue;
              if (button.mPosition == 0) {
                nNewDock = GetDockAreaIndex(button.mpDockArea);
              }
              else {
                nNewDock = AddDockArea(GetDockAreaIndex(button.mpDockArea),
                                       (eRectEdges)button.mPosition,
                                       button.mTabRect,
                                       i<=eDockButton_LocalTab);
              }
              break;
            }
            if (nNewDock != eInvalidHandle) {
              DockWidget(nNewDock,pwWidget);
            }
          }
        }
        mpwOverlay->SetStatus(eFalse,eFalse,eTrue);
        break;
      }
    case eWidgetDockingManagerMessage_Move:
      {
        sVec2f mousePos = Vec2<tF32>(aA.mV2F);

        iWidget* pLocalDockArea = GetDockArea(GetDockAreaHovered(mousePos));
        if (pLocalDockArea) {
          UpdateLocalDockButtons(pLocalDockArea);
        }

        tI32 i;
        for (i = 0; i < eDockButton_Last; ++i) {
          sDockButton& button = mButtons[i];
          button.mbHover = eFalse;
        }

        for (i = eDockButton_Last-1; i >= 0; --i) {
          if (mButtons[i].mRect.Intersect(mousePos-mpWidget->GetAbsolutePosition())) {
            mButtons[i].mbHover = eTrue;
            break;
          }
          else {
            mButtons[i].mbHover = eFalse;
          }
        }
        break;
      }
    case eUIMessage_SerializeWidget:
      {
        Ptr<iDataTable> ptrDT = ni::VarQueryInterface<iDataTable>(aA);
        tU32 nFlags = aB.mU32;
        if (niFlagIs(nFlags,eWidgetSerializeFlags_Read)) {
          SerializeRead(ptrDT,nFlags);
        }
        else {
          SerializeWrite(ptrDT,nFlags);
        }
        break;
      }
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetDockingManager::Resize(tF32 afWidth, tF32 afHeight,
                                   tF32 afPrevWidth, tF32 afPrevHeight)
{
  //     niDebugFmt((_A("[%g,%g] -> [%g,%g]"),
  //               afWidth,afHeight,
  //               afPrevWidth,afPrevHeight));
  {
    astl::vector<Ptr<iWidget> > vSplitters;
    niLoop(i,mpWidget->GetNumChildren()) {
      iWidget* pWidget = mpWidget->GetChildFromIndex(i);
      if (cString(HStringGetStringEmpty(pWidget->GetID())).StartsWith(SPLITTERID_PREFIX)) {
        vSplitters.push_back(pWidget);
      }
    }
    const tF32 xScale = ni::FDiv(afWidth,afPrevWidth);
    const tF32 yScale = ni::FDiv(afHeight,afPrevHeight);
    niLoop(i,vSplitters.size()) {
      Ptr<iWidget> da = vSplitters[i];
      if (!da.IsOK()) continue;
      sRectf r = da->GetRect();
      r.x *= xScale;
      r.z *= xScale;
      r.y *= yScale;
      r.w *= yScale;
      da->SetRect(r);
    }
  }

  mpwOverlay->SetAbsoluteRect(mpWidget->GetAbsoluteRect());

  //  sVec2f vDiff = mpwOverlay->GetSize()-Vec2<tF32>(afWidth,afHeight);
  //  mpwOverlay->SetPosition(sVec2f::Zero());
  //  mpwOverlay->SetSize(Vec2<tF32>(afWidth,afHeight));

  // Set the dock buttons positions
  mButtons[eDockButton_Left].mRect = sRectf(10,afHeight/2-DOCKBUTTON_WIDTH/2,
                                                 DOCKBUTTON_HEIGHT,DOCKBUTTON_WIDTH);
  mButtons[eDockButton_Right].mRect = sRectf(afWidth-10-DOCKBUTTON_HEIGHT,afHeight/2-DOCKBUTTON_WIDTH/2,
                                                  DOCKBUTTON_HEIGHT,DOCKBUTTON_WIDTH);
  mButtons[eDockButton_Top].mRect = sRectf(afWidth/2-DOCKBUTTON_WIDTH/2,10,
                                                DOCKBUTTON_WIDTH,DOCKBUTTON_HEIGHT);
  mButtons[eDockButton_Bottom].mRect = sRectf(afWidth/2-DOCKBUTTON_WIDTH/2,afHeight-10-DOCKBUTTON_HEIGHT,
                                                   DOCKBUTTON_WIDTH,DOCKBUTTON_HEIGHT);
  UpdateLocalDockButtons(mpWidget);
}

///////////////////////////////////////////////
void cWidgetDockingManager::UpdateLocalDockButtons(iWidget* apArea)
{
  sRectf rectArea = (apArea==mpWidget)?apArea->GetDockFillRect():(apArea->GetAbsoluteRect()-mpWidget->GetAbsolutePosition());
  sVec2f offset = rectArea.GetTopLeft();
  sVec2f size = rectArea.GetSize();

  sRectf rectCenter = sRectf(size.x/2-DOCKBUTTON_WIDTH/2,size.y/2-DOCKBUTTON_WIDTH/2,
                                       DOCKBUTTON_WIDTH,DOCKBUTTON_WIDTH);

  mButtons[eDockButton_LocalTab].mRect = rectCenter;
  mButtons[eDockButton_LocalTab].mRect.Move(offset);
  mButtons[eDockButton_LocalTab].mpDockArea = apArea;

  mButtons[eDockButton_LocalLeft].mRect = rectCenter;
  mButtons[eDockButton_LocalLeft].mRect.Move(Vec2<tF32>(-(DOCKBUTTON_HEIGHT+5),0));
  mButtons[eDockButton_LocalLeft].mRect.SetSize(DOCKBUTTON_HEIGHT,DOCKBUTTON_WIDTH);
  mButtons[eDockButton_LocalLeft].mRect.Move(offset);
  mButtons[eDockButton_LocalLeft].mpDockArea = apArea;

  mButtons[eDockButton_LocalRight].mRect = rectCenter;
  mButtons[eDockButton_LocalRight].mRect.Move(Vec2<tF32>(DOCKBUTTON_WIDTH+5,0));
  mButtons[eDockButton_LocalRight].mRect.SetSize(DOCKBUTTON_HEIGHT,DOCKBUTTON_WIDTH);
  mButtons[eDockButton_LocalRight].mRect.Move(offset);
  mButtons[eDockButton_LocalRight].mpDockArea = apArea;

  mButtons[eDockButton_LocalTop].mRect = rectCenter;
  mButtons[eDockButton_LocalTop].mRect.Move(Vec2<tF32>(0,-(DOCKBUTTON_HEIGHT+5)));
  mButtons[eDockButton_LocalTop].mRect.SetSize(DOCKBUTTON_WIDTH,DOCKBUTTON_HEIGHT);
  mButtons[eDockButton_LocalTop].mRect.Move(offset);
  mButtons[eDockButton_LocalTop].mpDockArea = apArea;

  mButtons[eDockButton_LocalBottom].mRect = rectCenter;
  mButtons[eDockButton_LocalBottom].mRect.Move(Vec2<tF32>(0,DOCKBUTTON_WIDTH+5));
  mButtons[eDockButton_LocalBottom].mRect.SetSize(DOCKBUTTON_WIDTH,DOCKBUTTON_HEIGHT);
  mButtons[eDockButton_LocalBottom].mRect.Move(offset);
  mButtons[eDockButton_LocalBottom].mpDockArea = apArea;
}

///////////////////////////////////////////////
tU32 cWidgetDockingManager::GetDockAreaIndex(iWidget* apDockArea) const
{
  for (tU32 i = 0; i < GetNumDockAreas(); ++i) {
    if (GetDockArea(i) == apDockArea)
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tU32 cWidgetDockingManager::GetDockAreaParent(iWidget* apDockArea) const
{
  if (apDockArea == mpWidget) {
    return eInvalidHandle;
  }
  iWidget* pThis = apDockArea->GetParent();
  while (1) {
    tU32 nDockArea = GetDockAreaIndex(pThis);
    if (nDockArea != eInvalidHandle)
      return nDockArea;
    pThis = pThis->GetParent();
    niAssert(pThis != NULL);
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tU32 cWidgetDockingManager::AddDockArea(tU32 anParent, tU32 aPos, sRectf aRect, tBool abLocal)
{
  if (aPos != eRectEdges_Left && aPos != eRectEdges_Right &&
      aPos != eRectEdges_Top && aPos != eRectEdges_Bottom)
    return eInvalidHandle;

  tU32 nRet = eInvalidHandle;
  if (anParent == 0) {
    // Create the new dock area in the root
    tWidgetPtr pwNew;
    switch (aPos) {
      case eRectEdges_Left:
        pwNew = CreateVerticalDockArea(mnFlags,mpWidget,mpWidget,aRect,eTrue,eTrue,eTrue);
        break;
      case eRectEdges_Right:
        pwNew = CreateVerticalDockArea(mnFlags,mpWidget,mpWidget,aRect,eFalse,eTrue,eTrue);
        break;
      case eRectEdges_Top:
        pwNew = CreateHorizontalDockArea(mnFlags,mpWidget,mpWidget,aRect,eTrue,eTrue,eTrue);
        break;
      case eRectEdges_Bottom:
        pwNew = CreateHorizontalDockArea(mnFlags,mpWidget,mpWidget,aRect,eFalse,eTrue,eTrue);
        break;
    }
    pwNew->SetZOrder(abLocal?eWidgetZOrder_Background:eWidgetZOrder_BackgroundBottom);
    PushBackDockArea(pwNew->GetChildFromIndex(0));
    nRet = (tU32)mvDockAreas.size();
  }
  else {
    iWidget* pInsertWidget = GetDockArea(anParent);
    if (!pInsertWidget) return eInvalidHandle;
    iWidget* pSplitterWidget = pInsertWidget->GetParent();
    if (!pSplitterWidget) return eInvalidHandle;
    QPtr<iWidgetSplitter> pwSplitter = pSplitterWidget;
    niAssert(pwSplitter.IsOK());

    tBool bNeedHorizontal = (aPos == eRectEdges_Top || aPos == eRectEdges_Bottom);
    if (pwSplitter->GetNumSplitters() == 0)
    {
      if (pSplitterWidget->GetParent() != mpWidget) {
        niAssert(pSplitterWidget->GetParent() != NULL);
        iWidget* pParentSplitterChildWidget = pSplitterWidget->GetParent();
        iWidget* pParentSplitter = pParentSplitterChildWidget->GetParent();
        QPtr<iWidgetSplitter> pwParentSplitter = pParentSplitter;
        niAssert(pwParentSplitter.IsOK());
        if (bNeedHorizontal == niFlagIs(pParentSplitter->GetStyle(),eWidgetSplitterStyle_Horizontal)) {
          pInsertWidget = pParentSplitterChildWidget;
          pSplitterWidget = pParentSplitter;
          pwSplitter = pwParentSplitter;
        }
      }
    }
    else if (bNeedHorizontal != niFlagIs(pSplitterWidget->GetStyle(),eWidgetSplitterStyle_Horizontal))
    {
      niAssert(pInsertWidget->GetNumChildren() == 1);
      tWidgetPtr pwTab = pInsertWidget->GetChildFromIndex(0);
      QPtr<iWidgetTab> ptrTab = pwTab.ptr();
      niAssert(QPtr<iWidgetTab>(ptrTab).IsOK());
      tWidgetPtr pwInt = CreateVerticalDockArea(mnFlags,mpWidget,pInsertWidget,aRect,eFalse,eFalse,eFalse,pwTab);
      pSplitterWidget = pwInt;
      pwSplitter = pwInt;
      pInsertWidget = pwSplitter->GetSplitterWidget(0);
      PushBackDockArea(pwTab->GetParent());
    }

    tU32 nInsertPoint = 0;
    for (tU32 i = 0; i < pwSplitter->GetNumSplitterWidgets(); ++i) {
      if (pwSplitter->GetSplitterWidget(i) == pInsertWidget) {
        nInsertPoint = i;
        break;
      }
    }

    iWidget* pNewSplitterParent = NULL;
    switch (aPos) {
      case eRectEdges_Left:
        pSplitterWidget->SetStyle(pSplitterWidget->GetStyle()&(~eWidgetSplitterStyle_Horizontal));
        pwSplitter->AddSplitterBefore(nInsertPoint,aRect.GetWidth()/pInsertWidget->GetSize().x);
        pNewSplitterParent = pwSplitter->GetSplitterWidget(nInsertPoint);
        break;
      case eRectEdges_Right:
        pSplitterWidget->SetStyle(pSplitterWidget->GetStyle()&(~eWidgetSplitterStyle_Horizontal));
        pwSplitter->AddSplitterAfter(nInsertPoint,aRect.GetWidth()/pInsertWidget->GetSize().x);
        pNewSplitterParent = pwSplitter->GetSplitterWidget(nInsertPoint+1);
        break;
      case eRectEdges_Top:
        pSplitterWidget->SetStyle(pSplitterWidget->GetStyle()|(eWidgetSplitterStyle_Horizontal));
        pwSplitter->AddSplitterBefore(nInsertPoint,aRect.GetHeight()/pInsertWidget->GetSize().y);
        pNewSplitterParent = pwSplitter->GetSplitterWidget(nInsertPoint);
        break;
      case eRectEdges_Bottom:
        pSplitterWidget->SetStyle(pSplitterWidget->GetStyle()|(eWidgetSplitterStyle_Horizontal));
        pwSplitter->AddSplitterAfter(nInsertPoint,aRect.GetHeight()/pInsertWidget->GetSize().y);
        pNewSplitterParent = pwSplitter->GetSplitterWidget(nInsertPoint+1);
        break;
    }

    tWidgetPtr pwNew = CreateVerticalDockArea(mnFlags,mpWidget,pNewSplitterParent,aRect,eFalse,eFalse,eFalse);
    pwNew->SetZOrder(eWidgetZOrder_Background);
    PushBackDockArea(pwNew->GetChildFromIndex(0));
    nRet = (tU32)mvDockAreas.size();
  }

  mpwTab->SetZOrder(eWidgetZOrder_Background);
  // Updates the context menu
  SetDockAreaTabContextMenu(GetDockAreaTabContextMenu());
  return nRet;
}

///////////////////////////////////////////////
sRectf cWidgetDockingManager::GetDockPositionRect(tRectEdgesFlags aDockPos, iWidget* apArea, sVec2f avRootPos, tBool abLocal)
{
  tBool bIsRoot = (apArea==mpWidget);
  tF32 fNewDockWidth = (bIsRoot)?_kfNewDockWidth:_kfNewLocalDockWidth;
  tF32 fNewDockHeight = (bIsRoot)?_kfNewDockHeight:_kfNewLocalDockHeight;
  sRectf rect;
  if (bIsRoot && abLocal) {
    rect = apArea->GetDockFillRect()+apArea->GetAbsolutePosition()+apArea->GetClientPosition();
  }
  else {
    rect = apArea->GetAbsoluteRect();
  }
  rect -= avRootPos;
  sRectf outRect = rect;
  switch (aDockPos) {
    case eRectEdges_Left:
      {
        outRect.SetLeft(rect.GetLeft());
        outRect.SetRight(rect.GetLeft()+(rect.GetWidth()*fNewDockWidth));
        outRect.SetTop(rect.GetTop());
        outRect.SetBottom(rect.GetBottom());
        break;
      }
    case eRectEdges_Right:
      {
        outRect.SetLeft(rect.GetRight()-(rect.GetWidth()*fNewDockWidth));
        outRect.SetRight(rect.GetRight());
        outRect.SetTop(rect.GetTop());
        outRect.SetBottom(rect.GetBottom());
        break;
      }
    case eRectEdges_Top:
      {
        outRect.SetLeft(rect.GetLeft());
        outRect.SetRight(rect.GetRight());
        outRect.SetTop(rect.GetTop());
        outRect.SetBottom(rect.GetTop()+(rect.GetHeight()*fNewDockHeight));
        break;
      }
    case eRectEdges_Bottom:
      {
        outRect.SetLeft(rect.GetLeft());
        outRect.SetRight(rect.GetRight());
        outRect.SetTop(rect.GetBottom()-(rect.GetHeight()*fNewDockHeight));
        outRect.SetBottom(rect.GetBottom());
        break;
      }
  }
  return outRect;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetDockingManager::SetDockAreaTabContextMenu(iWidget* apMenu)
{
  if (!QPtr<iWidgetMenu>(apMenu).IsOK())
    mpwDockAreaTabContextMenu = NULL;
  else
    mpwDockAreaTabContextMenu = apMenu;

  mpwTab->SetContextMenu(mpwDockAreaTabContextMenu);
  for (tU32 i = 0; i < GetNumDockAreas(); ++i) {
    iWidget* pDock = GetDockArea(i);
    iWidget* pwTab = pDock->GetChildFromIndex(0);
    if (QPtr<iWidgetTab>(pwTab).IsOK()) {
      pwTab->SetContextMenu(mpwDockAreaTabContextMenu);
    }
  }
  return mpwDockAreaTabContextMenu.IsOK();
}

///////////////////////////////////////////////
iWidget* __stdcall cWidgetDockingManager::GetDockAreaTabContextMenu() const
{
  return mpwDockAreaTabContextMenu;
}

///////////////////////////////////////////////
void cWidgetDockingManager::SetDefaultDockAreaTabMenu()
{
  Ptr<iWidget> pwMenu = mpWidget->GetUIContext()->CreateWidget(_A("Menu"),mpWidget,sRectf(),eWidgetStyle_Free|eWidgetStyle_HoldFocus,_H("ID_DockDefaultTabMenu"));
  QPtr<iWidgetMenu>  ptrMenu = pwMenu.ptr();
  pwMenu->SetStatus(eFalse,eFalse,eFalse);
  ptrMenu->AddItem(_A("Undock"),niGetConstHString(TabMenu_Undock),0);
  ptrMenu->AddItem(_A("Move Left"),niGetConstHString(TabMenu_MoveLeft),0);
  ptrMenu->AddItem(_A("Move Right"),niGetConstHString(TabMenu_MoveRight),0);
  SetDockAreaTabContextMenu(pwMenu);
}

///////////////////////////////////////////////
tBool cWidgetDockingManager::PushBackDockArea(iWidget* apWidget)
{
  mvDockAreas.push_back(apWidget);
  mvDockAreas.back()->SetID(GetNewDockAreaID(mpWidget));
  return eTrue;
}

///////////////////////////////////////////////
tBool cWidgetDockingManager::RemoveSplitter(iWidget* apSplitter)
{
  Ptr<iWidget> pwSplitter = apSplitter;
  Ptr<iWidget> pwCanvas = pwSplitter->GetParent();
  if (pwCanvas != mpWidget) {
    Ptr<iWidget> pwParentSplitter = pwCanvas->GetParent();
    QPtr<iWidgetSplitter> ptrParentSplitter = pwParentSplitter.ptr();
    if (ptrParentSplitter.IsOK()) {
      if (ptrParentSplitter->GetNumSplitters() == 0) {
        RemoveSplitter(pwParentSplitter);
      }
      else {
        tBool bRes = ptrParentSplitter->RemoveSplitterWidget(ptrParentSplitter->GetSplitterWidgetIndex(pwCanvas));
        niUnused(bRes);
        niAssert(bRes);
      }
    }
  }
  pwSplitter->SetParent(NULL);
  return pwCanvas != mpWidget;
}

///////////////////////////////////////////////
tBool cWidgetDockingManager::DoCleanSplitters(iWidget* apSplitter)
{
  Ptr<iWidget> pwSplitter = apSplitter;
  QPtr<iWidgetSplitter> ptrSplitter = pwSplitter.ptr();
  if (!ptrSplitter.IsOK()) {
    return eFalse;
  }

  Ptr<iWidget> pwCanvas = pwSplitter->GetParent();
  if (pwCanvas != mpWidget) {
    Ptr<iWidget> pwParentSplitter = pwCanvas->GetParent();
    QPtr<iWidgetSplitter> ptrParentSplitter = pwParentSplitter.ptr();
    if (ptrSplitter->GetNumSplitters() == 0 && ptrParentSplitter->GetNumSplitters() == 0) {
      iWidget* pParentParent = pwParentSplitter->GetParent();
      sRectf rectParent = pwParentSplitter->GetRect();
      eWidgetDockStyle dockStyleParent = pwParentSplitter->GetDockStyle();
      tU32 resizableBordersParent = QPtr<iWidgetSplitter>(pwParentSplitter)->GetSplitterResizableBorders();
      tF32 borderSizeParent = QPtr<iWidgetSplitter>(pwParentSplitter)->GetSplitterBorderSize();

      pwParentSplitter->SetParent(NULL);

      pwSplitter->SetParent(pParentParent);
      pwSplitter->SetDockStyle(dockStyleParent);
      pwSplitter->SetRect(rectParent);
      QPtr<iWidgetSplitter>(pwSplitter)->SetSplitterResizableBorders(resizableBordersParent);
      QPtr<iWidgetSplitter>(pwSplitter)->SetSplitterBorderSize(_kfSplitterBorderSize);
    }
  }

  for (tU32 i = 0; i < ptrSplitter->GetNumSplitterWidgets(); ++i) {
    DoCleanSplitters(ptrSplitter->GetSplitterWidget(i)->GetChildFromIndex(0));
  }

  return eTrue;
}

///////////////////////////////////////////////
void cWidgetDockingManager::CleanSplitters()
{
  for (tU32 i = 0; i < mpWidget->GetNumChildren(); ++i) {
    iWidget* pW = mpWidget->GetChildFromIndex(i);
    if (cString(niHStr(pW->GetID())).StartsWith(SPLITTERID_PREFIX)) {
      DoCleanSplitters(pW);
    }
  }
}

///////////////////////////////////////////////
tBool cWidgetDockingManager::RemoveDockArea(tU32 anIndex)
{
  if (anIndex < 1) {
    niAssertUnreachable("Invalid index");
    return eFalse;
  }
  --anIndex;
  if (anIndex >= mvDockAreas.size()) {
    niAssertUnreachable("Invalid index");
    return eFalse;
  }
  mvDockAreas.erase(mvDockAreas.begin()+anIndex);
  return eTrue;
}

///////////////////////////////////////////////
static tU32 __stdcall _GetNumPages(iWidget* apDockArea)
{
  QPtr<iWidgetTab> ptrTab = apDockArea->GetChildFromIndex(0);
  if (ptrTab.IsOK()) {
    return ptrTab->GetNumPages();
  }
  else {
    return 0;
  }
}

///////////////////////////////////////////////
tU32 _GetEmptySplitters(iWidget* apWidget, astl::vector<Ptr<iWidget> >& aLst) {
  tU32 nNumEmpty = 0;
  niLoop(i,apWidget->GetNumChildren()) {
    Ptr<iWidget> c = apWidget->GetChildFromIndex(i);
    if (c.IsOK()) {
      cString id = niHStr(c->GetID());
      if (id.StartsWith(SPLITTERID_PREFIX) || id.StartsWith(DOCKTAB_PREFIX) || id.StartsWith(DOCKAREA_PREFIX))
      {
        nNumEmpty += _GetEmptySplitters(c,aLst);

        QPtr<iWidgetSplitter> splitter = c.ptr();
        if (splitter.IsOK()) {
          niLoop(i,splitter->GetNumSplitters()) {
            iWidget* s = splitter->GetSplitterWidget(i);
            if (s->GetNumChildren() == 0) {
              nNumEmpty++;
              aLst.push_back(c);
            }
          }
        }
        else
        {
          QPtr<iWidgetTab> tab = c.ptr();
          if (tab.IsOK() && tab->GetNumPages() == 0) {
            nNumEmpty++;
            aLst.push_back(c);
          }
        }
      }
    }
  }
  return nNumEmpty;
}

///////////////////////////////////////////////
void cWidgetDockingManager::CleanDockAreas()
{
  tBool bEmptyDockArea = eTrue;
  while (bEmptyDockArea) {
    bEmptyDockArea = eFalse;
    for (tU32 i = 1; i < GetNumDockAreas(); ++i) {
      iWidget* pDockArea = GetDockArea(i);
      tU32 nNumPages = _GetNumPages(pDockArea);
      if (!nNumPages) {
        RemoveDockArea(i);
        bEmptyDockArea = eTrue;
        break;
      }
    }
  }
  astl::vector<Ptr<iWidget> > emptySpitters;
  if (_GetEmptySplitters(mpWidget,emptySpitters)) {
    niLoop(i,emptySpitters.size()) {
      Ptr<iWidget> c = emptySpitters[i]->GetParent();
      if (c.IsOK()) {
        Ptr<iWidget> s = c->GetParent();
        QPtr<iWidgetSplitter> sq = s.ptr();
        if (sq.IsOK()) {
          tU32 index = sq->GetSplitterWidgetIndex(c);
          tBool r = sq->RemoveSplitterWidget(index);
          niUnused(r);
          if (sq->GetNumSplitters() == 0) {
            s->Destroy();
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////
void cWidgetDockingManager::ClearDockAreas()
{
  CleanDockAreas();

  tWidgetVec vDocked = mvDocked;
  for (tU32 i = 0; i < vDocked.size(); ++i) {
    UndockWidget(vDocked[i]);
  }

  astl::vector<Ptr<iWidget> > toDestroy;
  niLoop(i,mpWidget->GetNumChildren()) {
    iWidget* c = mpWidget->GetChildFromIndex(i);
    if (niIsOK(c)) {
      cString id = niHStr(c->GetID());
      if (id.StartsWith(SPLITTERID_PREFIX) ||
          id.StartsWith(DOCKTAB_PREFIX) ||
          id.StartsWith(DOCKAREA_PREFIX))
      {
        toDestroy.push_back(c);
      }
    }
  }

  niLoop(i,toDestroy.size()) {
    iWidget* d = toDestroy[i];
    if (niIsOK(d))
      d->Destroy();
  }

  CleanDockAreas();
}

///////////////////////////////////////////////
void cWidgetDockingManager::SerializeRead(iDataTable* apDT, tWidgetSerializeFlags aFlags)
{
  ClearDockAreas();
  Ptr<iDataTable> dtFormsLayout = apDT->GetChild(_A("FormsLayout"));
  if (!dtFormsLayout.IsOK()) {
    niWarning(_A("No FormsLayout data table."));
    return;
  }

  tU32 nNumDockAreas = dtFormsLayout->GetInt(_A("num_dock_areas"));
  tU32 nNumDockedWidget = dtFormsLayout->GetInt(_A("num_docked_widgets"));
  tU32 nNumForms = dtFormsLayout->GetInt(_A("num_forms"));
  Ptr<iDataTable> dtDockAreas;
  if (nNumForms) {
    Ptr<iDataTable> dtForms = dtFormsLayout->GetChild(_A("Forms"));
    if (dtForms.IsOK()) {
      for (tU32 i = 0; i < dtForms->GetNumChildren(); ++i) {
        Ptr<iDataTable> dtWidget = dtForms->GetChildFromIndex(i);
        tHStringPtr hspFormID = _H(dtWidget->GetString(_A("id")));
        Ptr<iWidget> pwForm = mpWidget->GetUIContext()->GetRootWidget()->FindWidget(hspFormID);
        if (pwForm.IsOK()) {
          if (!mpWidget->GetUIContext()->SerializeWidget(pwForm,dtWidget,eWidgetSerializeFlags_Read,NULL)) {
            niWarning(niFmt(_A("Can't read form '%d'."),i));
            continue;
          }
        }
      }
    }
  }
  if (nNumDockAreas) {
    Ptr<iDataTable> dtSplitters = dtFormsLayout->GetChild(_A("Splitters"));
    if (dtSplitters.IsOK()) {
      for (tU32 i = 0; i < dtSplitters->GetNumChildren(); ++i) {
        Ptr<iDataTable> dtWidget = dtSplitters->GetChildFromIndex(i);
        Ptr<iWidget> pwSplitter = mpWidget->GetUIContext()->CreateWidget(_A("Splitter"),mpWidget,sRectf(0,0),eWidgetSplitterStyle_Empty,NULL);
        if (!mpWidget->GetUIContext()->SerializeWidget(pwSplitter,dtWidget,eWidgetSerializeFlags_Read|eWidgetSerializeFlags_Children,NULL)) {
          niWarning(niFmt(_A("Can't read splitter '%d'."),i));
          continue;
        }
      }
    }
    dtDockAreas = dtFormsLayout->GetChild(_A("DockAreas"));
    if (dtDockAreas.IsOK()) {
      // the dock area 0 is the desktop
      for (tU32 i = 1; i < dtDockAreas->GetNumChildren(); ++i) {
        Ptr<iDataTable> dtDockArea = dtDockAreas->GetChildFromIndex(i);
        tHStringPtr hspID = _H(dtDockArea->GetString(_A("dock_id")));
        Ptr<iWidget> ptrDock = mpWidget->GetUIContext()->GetRootWidget()->FindWidget(hspID);
        if (!ptrDock.IsOK()) {
          niWarning(niFmt(_A("Can't find dock area widget '%s'."),hspID));
          continue;
        }
        mvDockAreas.push_back(ptrDock);
      }
    }
  }
  if (nNumDockedWidget) {
    Ptr<iDataTable> dtDocked = dtFormsLayout->GetChild(_A("DockedWidgets"));
    for (tU32 i = 0; i < dtDocked->GetNumProperties(); ++i) {
      tHStringPtr hspID = _H(dtDocked->GetPropertyName(i));
      tU32 nDockArea = dtDocked->GetIntFromIndex(i);
      Ptr<iWidget> pwWidget = mpWidget->GetUIContext()->GetRootWidget()->FindWidget(hspID);
      if (pwWidget.IsOK()) {
        DockWidget(nDockArea,pwWidget);
      }
    }
  }
  if (dtDockAreas.IsOK()) {
    // restore the active pages
    for (tU32 i = 0; i < dtDockAreas->GetNumChildren(); ++i) {
      Ptr<iDataTableReadStack> dt = ni::CreateDataTableReadStack(dtDockAreas->GetChildFromIndex(i));

      QPtr<iWidgetTab> ptrTab = GetDockAreaTab(i);
      if (ptrTab.IsOK()) {
        if (dt->PushFail(_A("Tab"))) {
          for (tU32 n = 0; n < dt->GetNumChildren(); ++n) {
            dt->PushChild(n);
            iWidget* pPage = ptrTab->GetPageFromID(dt->GetHStringDefault(_A("id"),NULL));
            if (pPage) {
              ptrTab->SetPageName(pPage,dt->GetHStringDefault(_A("name"),ptrTab->GetPageName(pPage)));
              ptrTab->SetPageIndex(pPage,dt->GetIntDefault(_A("index"),ptrTab->GetPageIndex(pPage)));
            }
            dt->Pop();
          }
          ptrTab->SetActivePageID(dt->GetHStringDefault(_A("active_page"),ptrTab->GetActivePageID()));
          dt->Pop();
        }
      }
    }
  }

  mpwTab->SetZOrder(eWidgetZOrder_Background);
  CleanDockAreas();
  SetDockAreaTabContextMenu(GetDockAreaTabContextMenu());
  _ApplyFlags();
}

///////////////////////////////////////////////
void cWidgetDockingManager::SerializeWrite(iDataTable* apDT, tWidgetSerializeFlags aFlags)
{
  Ptr<iDataTable> ptrDT = ni::CreateDataTable(_A("FormsLayout"));

  ptrDT->SetInt(_A("num_dock_areas"),GetNumDockAreas());
  ptrDT->SetInt(_A("num_docked_widgets"),GetNumDockedWidgets());
  {
    astl::list<iWidget*> lstForms;
#pragma niNote("This is not what should be called a clean solution... but well, since we pretty much always have a desktop widget as child of the root widget... it'll do for now")
    iWidget* pParent = mpWidget->GetParent();
    if (pParent == mpWidget->GetUIContext()->GetRootWidget()) {
      for (tU32 i = 0; i < pParent->GetNumChildren(); ++i) {
        iWidget* pW = pParent->GetChildFromIndex(i);
        if ((pW->GetClassName() == _H("Form")) && niFlagIsNot(pW->GetStyle(),eWidgetStyle_DontSerialize)) {
          lstForms.push_back(pW);
        }
      }
    }
    for (tU32 i = 0; i < mpWidget->GetNumChildren(); ++i) {
      iWidget* pW = mpWidget->GetChildFromIndex(i);
      if ((pW->GetClassName() == _H("Form")) && niFlagIsNot(pW->GetStyle(),eWidgetStyle_DontSerialize)) {
        lstForms.push_back(pW);
      }
    }
    if (!lstForms.empty()) {
      Ptr<iDataTable> dtForms = ni::CreateDataTable(_A("Forms"));
      for (astl::list<iWidget*>::iterator itF = lstForms.begin(); itF != lstForms.end(); ++itF) {
        Ptr<iDataTable> dtWidget = ni::CreateDataTable(AZEROSTR);
        mpWidget->GetUIContext()->SerializeWidget(*itF,dtWidget,eWidgetSerializeFlags_Write,NULL);
        dtForms->AddChild(dtWidget);
      }
      ptrDT->AddChild(dtForms);
    }
    ptrDT->SetInt(_A("num_forms"),(tU32)lstForms.size());
  }
  if (GetNumDockedWidgets())
  {
    Ptr<iDataTable> ptrDockedDT = ni::CreateDataTable(_A("DockedWidgets"));
    for (tU32 i = 0; i < GetNumDockedWidgets(); ++i) {
      iWidget* pDocked = GetDockedWidget(i);
      tU32 nDockArea = GetDockAreaFromWidget(pDocked);
      if (HStringIsNotEmpty(pDocked->GetID())) {
        ptrDockedDT->SetInt(niHStr(pDocked->GetID()),nDockArea);
      }
    }
    ptrDT->AddChild(ptrDockedDT);
  }
  if (GetNumDockAreas()) {
    tU32 i;
    Ptr<iDataTable> ptrSplittersDT = ni::CreateDataTable(_A("Splitters"));
    Ptr<iRegex> ptrFilter = ni::CreateFilePatternRegex(_A("Splitter|Canvas|Tab"));
    for (i = 0; i < mpWidget->GetNumChildren(); ++i) {
      iWidget* pWidget = mpWidget->GetChildFromIndex(i);
      if (cString(HStringGetStringEmpty(pWidget->GetID())).StartsWith(SPLITTERID_PREFIX)) {
        Ptr<iDataTable> ptrNewDT = ni::CreateDataTable(AZEROSTR);
        mpWidget->GetUIContext()->SerializeWidget(pWidget,ptrNewDT,eWidgetSerializeFlags_Write|eWidgetSerializeFlags_Children,ptrFilter);
        ptrSplittersDT->AddChild(ptrNewDT);
      }
    }
    ptrDT->AddChild(ptrSplittersDT);

    Ptr<iDataTable> ptrDockAreasDT = ni::CreateDataTable(_A("DockAreas"));
    for (i = 0; i < GetNumDockAreas(); ++i) {
      Ptr<iDataTable> ptrDockAreaDT = ni::CreateDataTable(_A("DockArea"));

      iWidget* pDock = GetDockArea(i);
      if (!pDock) continue;
      Ptr<iWidget> pwTab;
      QPtr<iWidgetTab> ptrTab;
      if (pDock == mpWidget) {
        pwTab = mpwTab;
        ptrTab = mpwTab;
      }
      else {
        Ptr<iWidget>    pwSplitter = pDock->GetParent();
        if (!pwSplitter.IsOK()) continue;
        QPtr<iWidgetSplitter> ptrSplitter = pwSplitter.ptr();
        if (!ptrSplitter.IsOK()) continue;
        tU32 k = 0;
        if (ptrSplitter.IsOK()) {
          for (k = 0; k < ptrSplitter->GetNumSplitterWidgets(); ++k) {
            if (ptrSplitter->GetSplitterWidget(k) == pDock) {
              break;
            }
          }
          k = (k == ptrSplitter->GetNumSplitterWidgets())?0:k;
        }
        ptrDockAreaDT->SetString(_A("splitter"),niHStr(pwSplitter->GetID()));
        ptrDockAreaDT->SetString(_A("dock_id"),niHStr(pDock->GetID()));
        ptrDockAreaDT->SetInt(_A("splitter_child_widget"),k);
        pwTab = pDock->GetChildFromIndex(0);
        ptrTab = pwTab;
      }

      if (!pwTab.IsOK() || !ptrTab.IsOK())
        continue;

      Ptr<iDataTable> dtTab = ni::CreateDataTable(_A("Tab"));
      dtTab->SetString(_A("active_page"),niHStr(ptrTab->GetActivePageID()));
      for (tU32 n = 0; n < ptrTab->GetNumPages(); ++n) {
        Ptr<iDataTable> dtPage = ni::CreateDataTable(_A("Page"));
        iWidget* pPage = ptrTab->GetPage(n);
        dtPage->SetString(_A("id"),niHStr(pPage->GetID()));
        dtPage->SetString(_A("name"),niHStr(ptrTab->GetPageName(pPage)));
        dtPage->SetInt(_A("index"),ptrTab->GetPageIndex(pPage));
        dtTab->AddChild(dtPage);
      }
      ptrDockAreaDT->AddChild(dtTab);

      ptrDockAreasDT->AddChild(ptrDockAreaDT);
    }
    ptrDT->AddChild(ptrDockAreasDT);
  }
  apDT->AddChild(ptrDT);
}

///////////////////////////////////////////////
iWidget* cWidgetDockingManager::GetDockAreaTab(tU32 anIndex) const
{
  if (anIndex==0) return mpwTab;
  iWidget* pWidget = GetDockArea(anIndex);
  if (!pWidget) return NULL;
  return pWidget->GetChildFromIndex(0);
}
