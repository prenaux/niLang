#ifndef __SCROLLBARWIDGET_2190979_H__
#define __SCROLLBARWIDGET_2190979_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#define SCROLL_DOWN niBit(0)
#define SCROLL_UP   niBit(1)
#define SCROLL_PAGE niBit(2)

class cScrollBarWidget : public cIUnknownImpl<iWidgetSink,eIUnknownImplFlags_Default,iWidgetScrollBar>
{
  niBeginClass(cScrollBarWidget);
 public:
  cScrollBarWidget(iWidget *pWidget);
  ~cScrollBarWidget();

  //// iWidgetScrollBar /////////////////////////
  void __stdcall SetScrollRange(const sVec2f& avRange) niImpl;
  sVec2f __stdcall GetScrollRange() const niImpl { return Vec2<tF32>(mfMin,mfMax); }
  void __stdcall SetScrollPosition(tF32 afScrollPos) niImpl;
  tF32 __stdcall GetScrollPosition() const niImpl;
  void __stdcall SetNormalizedScrollPosition(tF32 afScrollPos) niImpl;
  tF32 __stdcall GetNormalizedScrollPosition() const niImpl;
  void __stdcall SetPageSize(tF32 afPageSize) niImpl;
  tF32 __stdcall GetPageSize() const niImpl {return mfPageSize; }
  tF32 __stdcall ComputeRoundedPosition(tF32 afNewPos) const niImpl;
  //// iWidgetScrollBar /////////////////////////

  //internals
  void ThumbMove(const sVec2f &pos);

  //iWidgetSink interface
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);
  //internal stuff
  void PaintWidget();
  void AdjustChildrenLayout(tF32 w,tF32 h);
  void ComputeClientRect(tF32 w,tF32 h);
  void UpdateStyle();
  void ThumbMoveMouse(const sVec2f& avAbsPos, tBool abSlideMove);

  void _DoScroll(tU32 anFlags);
  void _StartScroll(tU32 anFlags, tF32 afStopAt = niMaxF32);
  void _StopScroll();

  void __stdcall _SetScrollPosition(tF32 afScrollPos, tBool abWheel);

 public:
  iWidget *mpWidget;
  Ptr<iWidget> mptrThumb;
  tF32 mfMin;
  tF32 mfMax;
  tF32 mfPageSize;
  tF32 mfScrollPos;
  tF32 mfNormalizedScrollPos;
  tBool mbDragging;
  tU32 mnScrollTimerID;
  tF32 mfScrollStopAt;

  struct sButtons {
    Ptr<iWidget> up;
    Ptr<iWidget> down;
    ~sButtons() { Clear(); }
    void Clear() {
      if (up.IsOK()) {
        up->Destroy();
        up = NULL;
      }
      if (down.IsOK()) {
        down->Destroy();
        down = NULL;
      }
    }
    tBool HasButtons() const {
      return up.IsOK();
    }
  } mButtons;

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> back;
  } skin;

  niEndClass(cScrollBarWidget);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SCROLLBARWIDGET_2190979_H__
