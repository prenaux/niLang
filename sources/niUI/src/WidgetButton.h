#ifndef __BUTTONWIDGET_14185200_H__
#define __BUTTONWIDGET_14185200_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

class cButtonWidget : public ImplRC<iWidgetSink,eImplFlags_Default,iWidgetButton>
{
  niBeginClass(cButtonWidget);

 public:
  cButtonWidget(iWidget *pWidget);
  ~cButtonWidget();

  //// iWidgetButton ////////////////////////////
  void __stdcall SetIcon(iOverlay *apIcon)  { mptrIconNormal = apIcon; }
  iOverlay* __stdcall GetIcon() const { return mptrIconNormal; }
  void __stdcall SetIconPressed(iOverlay *apIcon) { mptrIconPressed = apIcon; }
  iOverlay* __stdcall GetIconPressed() const { return mptrIconPressed; }
  void __stdcall SetIconHover(iOverlay *apIcon) { mptrIconHover = apIcon; }
  iOverlay* __stdcall GetIconHover() const { return mptrIconHover; }
  void __stdcall SetIconSize(const sVec2f& avSize) { mvIconSize = avSize; }
  sVec2f __stdcall GetIconSize() const { return mvIconSize; }
  void __stdcall SetCheck(tBool abCkecked);
  tBool __stdcall GetCheck() const { return mbChecked; }
  tBool __stdcall SetGroupID(iHString* aVal);
  iHString* __stdcall GetGroupID() const;
  void __stdcall SetIconMargin(const sVec4f& avMargin);
  sVec4f __stdcall GetIconMargin() const;
  sRectf __stdcall GetIconDrawRect() const { return mrectIcon; }
  virtual void __stdcall SetDrawFrameFlags(tRectFrameFlags aFlags) {
    mDrawFrameFlags = aFlags;
  }
  virtual tRectFrameFlags __stdcall GetDrawFrameFlags() const {
    return mDrawFrameFlags;
  }
  //// iWidgetButton ////////////////////////////

  //// iWidgetSink //////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);
  //// iWidgetSink //////////////////////////////

  void ProcessClick(const sVec2f& avMousePos);
  void ProcessPush(const sVec2f& avMousePos);
  void ProcessUnPush(const sVec2f& avMousePos);
  void Paint_PushButton(const sRectf& rect, iCanvas* apCanvas);
  void Paint_CheckRadioButton(const sRectf& rect, iCanvas* apCanvas);
  void _GetStatusItems(iOverlay** appFrame, iFont** appFont, iOverlay** appIcon, tBool* apbIsPressed) const;
  const achar* _GetText() const {
    return niHStr(mpWidget->GetLocalizedText());
    // return niHStr(mpWidget->GetText());
  }

 private:
  iWidget *mpWidget;
  Ptr<iOverlay> mptrIconNormal;
  Ptr<iOverlay> mptrIconHover;
  Ptr<iOverlay> mptrIconPressed;
  tBool   mbPressed;
  tBool   mbChecked;
  tHStringPtr mhspGroupID;
  sVec4f mvIconMargin;
  sVec2f mvIconSize;
  sRectf mrectIcon;
  tRectFrameFlags mDrawFrameFlags;

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> normalFrame;
    Ptr<iFont>  normalFont;
    Ptr<iOverlay> pressedFrame;
    Ptr<iFont>  pressedFont;
    Ptr<iOverlay> hoverFrame;
    Ptr<iFont>  hoverFont;
    Ptr<iOverlay> mark;
    tU32  _kcolPressedBack;
    tU32  _kcolBorderColor;
  } skin;
  niEndClass(cButtonWidget);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __BUTTONWIDGET_14185200_H__
