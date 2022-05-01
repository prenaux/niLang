#ifndef __TABWIDGET_31676544_H__
#define __TABWIDGET_31676544_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

struct TabPage {
  Ptr<iWidget> pPage;
  Ptr<iWidget> pButton;
  void Reset() {
    pPage = NULL;
    pButton = NULL;
  }
  tBool IsNull() const {
    return !pPage.IsOK();
  }
};

typedef astl::vector<TabPage> tTabPageVec;
typedef astl::vector<tTabPageVec> tTabPageVecVec;

class cTabWidget : public cIUnknownImpl<iWidgetSink,eIUnknownImplFlags_Default,iWidgetTab>
{
  niBeginClass(cTabWidget);
 public:
  cTabWidget(iWidget *pWidget);
  ~cTabWidget();

  //// iWidgetTab //////////////////////////////////
  void __stdcall AddPage(iHString* ahspName, iWidget *apPage);
  tBool __stdcall RemovePage(iWidget* apPage);
  tBool __stdcall HasPage(iWidget* apPage) const;
  tU32 __stdcall GetNumPages() const;
  iWidget* __stdcall GetPage(tU32 anIndex) const;
  iWidget* __stdcall GetPageButton(tU32 anIndex) const;
  iWidget* __stdcall GetPageFromName(iHString* ahspName) const;
  iWidget* __stdcall GetPageFromID(iHString* ahspID) const;
  tBool __stdcall SetPageName(iWidget* apPage, iHString* ahspName);
  iHString* __stdcall GetPageName(iWidget* apPage) const;
  tBool __stdcall SetPageIndex(iWidget* apPage, tU32 anIndex);
  tU32 __stdcall GetPageIndex(iWidget* apPage) const;
  tBool __stdcall SetActivePage(iWidget* apPage);
  iWidget* __stdcall GetActivePage() const;
  tBool __stdcall SetActivePageName(iHString* ahspName);
  iHString* __stdcall GetActivePageName() const;
  tBool __stdcall SetActivePageID(iHString* ahspID);
  iHString* __stdcall GetActivePageID() const;
  tBool __stdcall SetActivePageIndex(tU32 anIndex);
  tU32 __stdcall GetActivePageIndex() const;
  tBool __stdcall MovePageLeft(iWidget* apPage, tU32 anLeft);
  tBool __stdcall MovePageRight(iWidget* apPage, tU32 anCount);
  void __stdcall SetMinNumPagesToShowTabs(tU32 anMinNumPages);
  tU32 __stdcall GetMinNumPagesToShowTabs() const;
  void __stdcall SetForceActivated(tBool abForce);
  tBool __stdcall GetForceActivated(tBool abForce) const;
  //// iWidgetTab //////////////////////////////////

  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);

  void _AdjustChildrenLayout();
  void _ActivateTab(iWidget *button);
  iWidget* _GetSelectedButton() const { return mSelectedPage.pButton; }
  void _NeedsAdjusting();
  tBool _IsActivatedStyle() const;
  tBool _IsTooSmall() const;
  //     void _UpdateStyle();

 public:
  iWidget*    mpWidget;
  iWidget*    mpPageContainer;
  tTabPageVec mvecPages;
  TabPage     mSelectedPage;
  tBool       mbNeedsAdjusting;
  tBool       mbActive;
  tBool       mbForceActivated;
  tU32        mnMinNumPagesToShowTabs;

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> focusNormalFrame;
    Ptr<iOverlay> focusTabButtonFrame;
    Ptr<iOverlay> regularNormalFrame;
    Ptr<iOverlay> regularTabButtonFrame;
    Ptr<iOverlay> normalFrame;
    Ptr<iOverlay> tabButtonFrame;
  } skin;

  niEndClass(cTabWidget);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __TABWIDGET_31676544_H__
