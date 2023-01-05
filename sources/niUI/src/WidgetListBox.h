#ifndef __WIDGETLISTBOX_44874688_H__
#define __WIDGETLISTBOX_44874688_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/STLVectorTypes.h>

static const tU32 knWidgetListBoxMinDefaultColumnWidth = 100;

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetListBox declaration.
class cWidgetListBox : public ni::cIUnknownImpl<ni::iWidgetSink,ni::eIUnknownImplFlags_Default,ni::iWidgetListBox>
{
  niBeginClass(cWidgetListBox);

  typedef astl::vector<tHStringPtr> tHStringPtrVec;
  typedef tHStringPtrVec::iterator  tHStringPtrVecIt;
  typedef tHStringPtrVec::const_iterator  tHStringPtrVecCIt;

  struct sColumn {
    tHStringPtr hspName;
    tU32        nSetSize;
  };

  struct sItemColumnData {
    cString      strText;
    Ptr<iWidget> ptrWidget;
    tU32         nTextColor;
    sItemColumnData(const achar* aaszText = AZEROSTR) {
      strText = aaszText;
      nTextColor = 0;
    }
  };

  struct sItem {
    astl::vector<sItemColumnData> vData;
    Ptr<iOverlay>                 ptrIcon;
    tF32                          fWidgetsHeight;
    Ptr<iUnknown>                 ptrUserData;

    void Invalidate() {
      niLoop(i,vData.size()) {
        InvalidateColumn(i);
      }
    }
    void InvalidateColumn(tU32 anCol) {
      niAssert(anCol < vData.size());
      SetWidget(anCol,NULL);
    }
    tBool SetWidget(tU32 anCol, iWidget* apWidget) {
      niAssert(anCol < vData.size());
      Ptr<iWidget>& ptrWidget = vData[anCol].ptrWidget;
      if (ptrWidget == apWidget)
        return eTrue;
      if (ptrWidget.IsOK() && niFlagIs(ptrWidget->GetStyle(),eWidgetStyle_ItemOwned)) {
        ptrWidget->Destroy();
        ptrWidget = NULL;
      }
      ptrWidget = apWidget;
      if (niIsOK(apWidget)) {
        // hide the widget, it'll be shown when it's placed
        apWidget->SetVisible(eFalse);
      }
      return eTrue;
    }
  };

 public:
  //! Constructor.
  cWidgetListBox(iWidget *apWidget);
  //! Destructor.
  ~cWidgetListBox();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetListBox //////////////////////
  bool __stdcall SortItem(const sItem& a,const sItem& b);
  void __stdcall ClearColumns();
  void __stdcall AddColumn(const achar *aaszName, tU32 anSize);
  tBool __stdcall SetColumn(tU32 anColumn, const achar* aaszName, tU32 anSize);
  tBool __stdcall RemoveColumn(tU32 anColumn);
  tU32 __stdcall GetNumColumns() const;
  tBool __stdcall SetColumnName(tU32 anColumn, const achar *aaszName);
  const achar * __stdcall GetColumnName(tU32 anColumn) const;
  tBool __stdcall SetColumnWidth(tU32 anColumn, tU32 anWidth);
  tU32 __stdcall GetColumnWidth(tU32 anColumn) const;
  tU32 __stdcall GetNumItems() const;
  void __stdcall ClearItems();
  tU32 __stdcall AddItem(const achar *aaszText);
  tBool __stdcall RemoveItem(tU32 anItem);
  tBool __stdcall SetItemText(tU32 anColumn, tU32 anItem, const achar *aaszText);
  const achar * __stdcall GetItemText(tU32 anColumn, tU32 anItem) const;
  tBool __stdcall SetItemWidget(tU32 anColumn, tU32 anItem, iWidget* apWidget);
  iWidget* __stdcall GetItemWidget(tU32 anColumn, tU32 anItem) const;
  tBool __stdcall SetItemIcon(tU32 anItem, iOverlay* apIcon);
  iOverlay* __stdcall GetItemIcon(tU32 anItem) const;
  tBool __stdcall SetItemData(tU32 anItem, iUnknown *apData);
  iUnknown * __stdcall GetItemData(tU32 anItem) const;
  tBool __stdcall SetSortKey(tU32 anKeyColumn);
  tU32 __stdcall GetSortKey() const;
  void __stdcall SetSortAscendant(tBool abAscendant);
  tBool __stdcall GetSortAscendant() const;
  tU32 __stdcall GetItemFromText(tU32 anColumn, const achar* aaszText) const;
  void __stdcall ClearSelection();
  tBool __stdcall AddSelection(tU32 anItem);
  tBool __stdcall RemoveSelection(tU32 anItem);
  tU32 __stdcall GetNumSelections() const;
  tU32 __stdcall GetSelection(tU32 anIndex) const;
  tBool __stdcall SetSelected(tU32 anSelection);
  tU32 __stdcall GetSelected() const;
  tBool __stdcall GetIsItemSelected(tU32 anItem) const;
  tBool __stdcall AddSelectedItem(tU32 anCol, const achar* aaszText);
  tBool __stdcall SetSelectedItem(tU32 anCol, const achar* aaszText);
  const achar* __stdcall GetSelectedItem(tU32 anCol) const;
  const achar* __stdcall GetSelectedItemText(tU32 anCol, tU32 anIndex) const;
  iWidget* __stdcall GetSelectedItemWidget(tU32 anCol, tU32 anIndex) const;
  tBool __stdcall SetItemHeight(tF32 afHeight);
  tF32 __stdcall GetItemHeight() const;
  tBool __stdcall SelectItemByPos(const sVec2f& avAbsPos);
  void __stdcall AutoScroll();
  tBool __stdcall SetItemTextColor(tU32 anColumn, tU32 anItem, tU32 anTextColor);
  tU32 __stdcall GetItemTextColor(tU32 anColumn, tU32 anItem) const;
  void __stdcall SetMaxNumItems(tU32 anMaxItems);
  tU32 __stdcall GetMaxNumItems() const;
  //// ni::iWidgetListBox //////////////////////

  //// iWidgetSink //////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);
  //// iWidgetSink //////////////////////////////

  void _RemoveOverflowingItems();
  __forceinline tU32 _ComputeDefaultColumnWidth() const;
  __forceinline tU32 _ComputeColumnWidth(const tU32 anCol, const tU32 anDefaultColumnWidth) const;
  __forceinline tU32 _ComputeItemsPerPage() const;

  tU32 RecomputeWidestItem();
  void ComputeClientRect(tF32 w,tF32 h);
  void UpdateWidgetScrollBars(tF32 w, tF32 h);
  void UpdateLayout();
  void DoUpdateLayout(tBool abForce);
  void Paint_Items(iCanvas* apCanvas);
  void Paint_Header(iCanvas* apCanvas, const sRectf &rect);
  void OnKeyDown(tU32 key);
  void NotifySelChange();
  tU32 GetSelectedColumnHandle(const sVec2f &mpos);
  tU32 GetModSelMode() const;
  void SetSelectionFromText();
  void SetTextFromSelection();
  void UpdateSelectOnMove();

 private:
  iWidget*      mpWidget;
  Ptr<iWidget>  mptrHzScroll;
  Ptr<iWidget>  mptrVtScroll;

  astl::vector<sColumn> mvColumns;
  astl::vector<astl::shared_ptr<sItem> > mvItems;
  tU32  mnSortKey;
  tBool mbSortType;
  tU32Vec mvSelection;

  tF32 mfWidestItem;
  tF32 mfRealW;
  tF32 mfRealH;
  tU32 mnColumns;
  tI32 mnHeaderHandlePressed;
  tU32 mnBaseSize;
  sVec2f mvHeaderPivot;
  sRectf mrectHeader;
  tF32 mfItemHeight;
  tU32 mnInputSelected;

  void InitSkin();
  struct sSkin {
    Ptr<iFont>    headerFont;
    Ptr<iOverlay> normalFrame;
    Ptr<iOverlay> focusedFrame;
    Ptr<iOverlay> header;
    Ptr<iOverlay> curResizeHz;
    Ptr<iOverlay> sel;
    Ptr<iOverlay> selNF;
    Ptr<iOverlay> arrow;
    Ptr<iOverlay> arrowUp;
    Ptr<iOverlay> arrowDown;
    tU32          ulcolBg;
  } skin;

  tF32 _GetVScrollPos() const;
  tF32 _GetHScrollPos() const;
  tF32 _GetScrolledBaseX() const;

  tU32 mnMaxNumItems;

  struct {
    tBool bShouldUpdateLayout : 1;
    tBool bShouldSortItems : 1;
    tBool bShouldAutoScroll : 1;
    tBool bInNotifySelChanged : 1;
  } mInternalFlags;

  niEndClass(cWidgetListBox);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETLISTBOX_44874688_H__
