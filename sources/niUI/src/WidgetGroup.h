#ifndef __WIDGETGROUP_29130702_H__
#define __WIDGETGROUP_29130702_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetGroup declaration.
class cWidgetGroup : public ni::cIUnknownImpl<ni::iWidgetSink,ni::eIUnknownImplFlags_Default,ni::iWidgetGroup>
{
  niBeginClass(cWidgetGroup);

 public:
  //! Constructor.
  cWidgetGroup(iWidget *apWidget);
  //! Destructor.
  ~cWidgetGroup();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iWidgetSink /////////////////////////
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 anMsg, const Var& avarA, const Var& avarB);
  //// ni::iWidgetSink /////////////////////////

  //// ni::iWidgetGroup ////////////////////////
  void __stdcall SetFolded(tBool abFolded);
  tBool __stdcall GetFolded() const;
  //// ni::iWidgetGroup ////////////////////////

  void ComputeClientRect();
  void NCPaint(iCanvas* apCanvas);

 private:
  iWidget* mpWidget;
  tBool mbFolded;
  tF32 mfOriginalHeight;

  void _UpdateSkin();
  struct sSkin {
    Ptr<iOverlay> normalFrame;
    Ptr<iOverlay> foldedFrame;
    Ptr<iOverlay> collapsed;
    Ptr<iOverlay> expanded;
    tU32 colEvenGroup;
    tU32 colOddGroup;
  } skin;

  niEndClass(cWidgetGroup);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETGROUP_29130702_H__
