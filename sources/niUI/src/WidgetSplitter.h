#ifndef __WIDGETSPLITTER_47772312_H__
#define __WIDGETSPLITTER_47772312_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

static const tF32 kSplitterFoldHandleSize = 64.0f;
static const tF32 kSplitterFoldedSize = 32.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetSplitter declaration.
class cWidgetSplitter : public ni::cIUnknownImpl<ni::iWidgetSink,ni::eIUnknownImplFlags_Default,ni::iWidgetSplitter>
{
  niBeginClass(cWidgetSplitter);

 public:
  //! Constructor.
  cWidgetSplitter(iWidget* apWidget);
  //! Destructor.
  ~cWidgetSplitter();

  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetSplitter /////////////////////
  tBool __stdcall AddSplitterFront(tF32 afPos) niImpl;
  tBool __stdcall AddSplitterBack(tF32 afPos) niImpl;
  tBool __stdcall RemoveSplitter(tU32 anIndex) niImpl;
  tU32 __stdcall GetNumSplitters() const niImpl;
  tU32 __stdcall GetNumSplitterWidgets() const niImpl;
  iWidget* __stdcall GetSplitterWidget(tU32 anIndex) const niImpl;
  tU32 __stdcall GetSplitterWidgetIndex(iWidget* apWidget) const niImpl;
  tBool __stdcall RemoveSplitterWidget(tU32 anIndex) niImpl;
  tBool __stdcall AddSplitterBefore(tU32 anWidget, tF32 afPos) niImpl;
  tBool __stdcall AddSplitterAfter(tU32 anWidget, tF32 afPos) niImpl;
  tBool __stdcall SetSplitterPosition(tU32 anIndex, tF32 aVal) niImpl;
  tF32 __stdcall GetSplitterPosition(tU32 anIndex) const niImpl;
  tBool __stdcall SetSplitterMinPosition(tF32 aVal) niImpl;
  tF32 __stdcall GetSplitterMinPosition() const niImpl;
  tBool __stdcall SetSplitterSize(tF32 afSize) niImpl;
  tF32 __stdcall GetSplitterSize() const niImpl;
  tBool __stdcall SwapSplitterWidget(tU32 anA, tU32 anB) niImpl;
  void __stdcall SetSplitterBorderSize(tF32 afSize) niImpl;
  tF32 __stdcall GetSplitterBorderSize() const niImpl;
  void __stdcall SetSplitterResizableBorders(tU32 aEdges) niImpl;
  tU32 __stdcall GetSplitterResizableBorders() const niImpl;
  void __stdcall SetSplitterParentDockRectMinimumSize(sVec2f avMinSize) niImpl;
  sVec2f __stdcall GetSplitterParentDockRectMinimumSize() const niImpl;
  void __stdcall SetSplitterFillerIndex(ni::tU32 anIndex) niImpl;
  tU32 __stdcall GetSplitterFillerIndex() const niImpl;
  void __stdcall SetSplitterFoldMode(eWidgetSplitterFoldMode aFoldMode) niImpl {
    mFoldMode = aFoldMode;
  }
  eWidgetSplitterFoldMode __stdcall GetSplitterFoldMode() const niImpl {
    return mFoldMode;
  }
  //// ni::iWidgetSplitter /////////////////////

  tBool __stdcall OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB);
  void UpdateSplitterRects(tU32 anPivotSplitter);
  void UpdateClientRect();
  void PushBorder(iCanvas* c, tU32 anZone, tBool abBorder);
  void ClearSplitters();
  void FoldSplitter(tU32 anID, tBool abLeftTop);
  void FoldClear(tU32 anID);
  tBool FoldIsFolded(tU32 anID) const;

  tBool TestRect(const sVec2f& avMousePos, tU32 anZone, tU32& anFold) const;
  tBool TestIntersection(const sVec2f& avMousePos, tU32& anIntersect, tU32& anFold) const;
  sRectf GetZoneRect(tU32 anZone, tU32 anFold) const;
  tBool GetIsZoneHorizontal(tU32 anZone) const;

  __forceinline tBool _SplittersEnabled() const {
    return !(mnFillerIndex < mvSplitters.size());
  }

  __forceinline tBool _CanFold() const {
    return _CanFoldLeftTop() || _CanFoldRightBottom();
  }
  __forceinline tBool _CanFoldOnlyOne() const {
    return !_CanFoldLeftTop() || !_CanFoldRightBottom();
  }
  __forceinline tBool _CanFoldLeftTop() const {
    const eWidgetSplitterFoldMode foldMode = this->GetSplitterFoldMode();
    if (foldMode == eWidgetSplitterFoldMode_Auto) {
      return niFlagIs(mnResizableBorders,eRectEdges_Right) || niFlagIs(mnResizableBorders,eRectEdges_Bottom);
    }
    else {
      return (foldMode == eWidgetSplitterFoldMode_All);
    }
  }
  __forceinline tBool _CanFoldRightBottom() const {
    const eWidgetSplitterFoldMode foldMode = this->GetSplitterFoldMode();
    if (foldMode == eWidgetSplitterFoldMode_Auto) {
      return niFlagIs(mnResizableBorders,eRectEdges_Left) || niFlagIs(mnResizableBorders,eRectEdges_Top);
    }
    else {
      return (foldMode == eWidgetSplitterFoldMode_All);
    }
  }

 private:
  iWidget* mpWidget;
  tF32     mfMinPos;
  tF32     mfSize;
  tU32     mnDragging;
  tF32     mfBorderSize;
  tU32     mnResizableBorders;
  sVec2f   mvParentDockRectMinSize;
  tU32     mnFillerIndex;

  typedef astl::hash_map<tU32,astl::pair<tF32,tBool> >  tFoldMap;
  tFoldMap mmapFoldMap;

  struct sSplitter {
    tF32  mfPosition;
    sRectf mRect;
    sSplitter() {
      mfPosition = 0;
      mRect = sRectf(0,0,0,0);
    }
  };
  typedef astl::vector<sSplitter> tSplitterVec;
  tSplitterVec  mvSplitters;

  typedef astl::vector<Ptr<iWidget> > tWidgetVec;
  tWidgetVec    mvWidgets;

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> curResizeHz;
    Ptr<iOverlay> curResizeVt;
    Ptr<iOverlay> curArrowBiHz;
    Ptr<iOverlay> curArrowBiVt;
    Ptr<iOverlay> curArrowUp;
    Ptr<iOverlay> curArrowDown;
    Ptr<iOverlay> curArrowLeft;
    Ptr<iOverlay> curArrowRight;

    tU32 colBorder;
    tU32 colFill;
    tU32 colFoldLeftTop;
    tU32 colFoldRightBottom;
    tU32 colFoldUnfold;
  } skin;

  eWidgetSplitterFoldMode mFoldMode;

  niEndClass(cWidgetSplitter);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETSPLITTER_47772312_H__
