#ifndef __WIDGETCANVASSINK_1335907_H__
#define __WIDGETCANVASSINK_1335907_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetCanvasSink declaration.
class cWidgetCanvasSink : public ni::ImplRC<ni::iWidgetSink,ni::eImplFlags_Default,ni::iWidgetCanvas>
{
  niBeginClass(cWidgetCanvasSink);

 public:
  //! Constructor.
  cWidgetCanvasSink(iWidget* apWidget);
  //! Destructor.
  ~cWidgetCanvasSink();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetSink /////////////////////////
  tBool __stdcall OnWidgetSink(ni::iWidget *apWidget, tU32 nMsg, const Var &varParam0, const Var &varParam1);
  //// ni::iWidgetSink /////////////////////////

  virtual iWidget* __stdcall GetScrollV() const {
    return mpwScrollV;
  }
  virtual void __stdcall SetScrollStepV(tF32 afV) {
    mfStepV = ni::Max(niEpsilon4,afV);
  }
  virtual tF32 __stdcall GetScrollStepV() const {
    return mfStepV;
  }
  virtual void __stdcall SetScrollMarginV(tF32 afV) {
    mfMarginV = afV;
  }
  virtual tF32 __stdcall GetScrollMarginV() const {
    return mfMarginV;
  }

  virtual iWidget* __stdcall GetScrollH() const {
    return mpwScrollH;
  }
  virtual void __stdcall SetScrollStepH(tF32 afV) {
    mfStepH = ni::Max(niEpsilon4,afV);
  }
  virtual tF32 __stdcall GetScrollStepH() const {
    return mfStepH;
  }
  virtual void __stdcall SetScrollMarginH(tF32 afV) {
    mfMarginH = afV;
  }
  virtual tF32 __stdcall GetScrollMarginH() const {
    return mfMarginH;
  }

  void __stdcall SetBorder(const sVec4f& aRect) {
    mvBorder = aRect;
  }
  sVec4f __stdcall GetBorder() const {
    return mvBorder;
  }
  tBool __stdcall GetHasBorder() const {
    return mvBorder != sVec4f::Zero();
  }

  void UpdateStyle();
  tBool IsScrollCanvas() const;
  tBool ScrollV() const;
  tBool ScrollH() const;
  tBool HideScrollV() const;
  tBool HideScrollH() const;
  tBool UpdateScrolls();
  void UpdateClientRect();
  void UpdateClientPos();
  void InitSkin();
  void _InitializeScrollBars();

 private:
  iWidget*     mpWidget;
  Ptr<iWidget> mpwScrollV;
  Ptr<iWidget> mpwScrollH;
  tBool        mbUpdateScroll;
  tF32         mfStepV, mfMarginV;
  tF32         mfStepH, mfMarginH;
  sVec4f       mvBorder;
  struct {
    tU32 mnFillColor;
    tU32 mnBorderColor;
  } skin;

  niEndClass(cWidgetCanvasSink);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETCANVASSINK_1335907_H__
