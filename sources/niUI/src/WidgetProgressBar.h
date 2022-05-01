#ifndef __PROGRESSBARWIDGET_24000048_H__
#define __PROGRESSBARWIDGET_24000048_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

class cProgressBarWidget : public cIUnknownImpl<iWidgetSink,eIUnknownImplFlags_Default,iWidgetProgressBar>
{
  niBeginClass(cProgressBarWidget);
 public:
  cProgressBarWidget(iWidget *pWidget);
  ~cProgressBarWidget();
  //iWidgetSink interface
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);
  //iWidgetProgressBar
  void __stdcall SetRange(tF32 afMin,tF32 afMax);
  void __stdcall SetProgress(tF32 afPos);
  tF32 __stdcall GetProgress() const;
  //internals
  void ComputeClientRect(tF32 w,tF32 h);
 private:
  iWidget *mpWidget;
  tF32 mfMin;
  tF32 mfMax;
  tF32 mfPos;

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> normalFrame;
    Ptr<iOverlay> fill;
  } skin;

  niEndClass(cProgressBarWidget);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __PROGRESSBARWIDGET_24000048_H__
