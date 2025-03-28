#ifndef __BUTTONWIDGET_14185200_H__
#define __BUTTONWIDGET_14185200_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

class cButtonWidget
    : public ImplRC<iWidgetSink, eImplFlags_Default, iWidgetButton> {
  niBeginClass(cButtonWidget);

 public:
  cButtonWidget(iWidget* pWidget);
  ~cButtonWidget();

  //// iWidgetButton ////////////////////////////
  void __stdcall SetIcon(iOverlay* apIcon);
  iOverlay* __stdcall GetIcon() const;
  void __stdcall SetIconPressed(iOverlay* apIcon);
  iOverlay* __stdcall GetIconPressed() const;
  void __stdcall SetIconHover(iOverlay* apIcon);
  iOverlay* __stdcall GetIconHover() const;
  void __stdcall SetIconSize(const sVec2f& avSize);
  sVec2f __stdcall GetIconSize() const;
  void __stdcall SetCheck(tBool abCkecked);
  tBool __stdcall GetCheck() const;
  tBool __stdcall SetGroupID(iHString* aVal);
  iHString* __stdcall GetGroupID() const;
  void __stdcall SetIconMargin(const sVec4f& avMargin);
  sVec4f __stdcall GetIconMargin() const;
  sRectf __stdcall GetIconDrawRect() const;
  void __stdcall SetDrawFrameFlags(tRectFrameFlags aFlags);
  tRectFrameFlags __stdcall GetDrawFrameFlags() const;
  //// iWidgetButton ////////////////////////////

  //// iWidgetSink //////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget* apWidget, tU32 nMsg,
                               const Var& varParam0, const Var& varParam1);
  //// iWidgetSink //////////////////////////////

  void ProcessClick(const sVec2f& avMousePos);
  void ProcessPush(const sVec2f& avMousePos);
  void ProcessUnPush(const sVec2f& avMousePos);
  void Measure_PushButton(const sRectf& aWidgetRect);
  void Paint_PushButton(const sRectf& aWidgetRect,
                        ain_nn<iCanvas> apCanvas) const;
  void Measure_CheckRadioButton(const sRectf& aWidgetRect);
  void Paint_CheckRadioButton(const sRectf& aWidgetRect,
                              ain_nn<iCanvas> apCanvas) const;
  void _GetStatusItems(iOverlay** appFrame, iFont** appFont, iOverlay** appIcon,
                       tBool* apbIsPressed) const;
  const achar* _GetText() const;
  void _MeasureAndPaint(iCanvas* apCanvas);

 private:
  iWidget* mpWidget;
  Ptr<iOverlay> mptrIconNormal;
  Ptr<iOverlay> mptrIconHover;
  Ptr<iOverlay> mptrIconPressed;
  tHStringPtr mhspGroupID;
  sRectf mrectText;
  sRectf mrectIcon;
  sVec4f mvIconMargin;
  sVec2f mvIconSize;
  tRectFrameFlags mDrawFrameFlags;
  tBool mbPressed;
  tBool mbChecked;

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> normalFrame;
    Ptr<iFont> normalFont;
    Ptr<iOverlay> pressedFrame;
    Ptr<iFont> pressedFont;
    Ptr<iOverlay> hoverFrame;
    Ptr<iFont> hoverFont;
    Ptr<iOverlay> mark;
    tU32 _kcolPressedBack;
    tU32 _kcolBorderColor;
  } skin;
  niEndClass(cButtonWidget);
};

/// EOF /////////////////////////////////////////////////////////////////////////////////////////
#endif // __BUTTONWIDGET_14185200_H__
