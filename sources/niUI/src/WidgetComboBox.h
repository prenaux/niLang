#ifndef __COMBOBOXWIDGET_6759518_H__
#define __COMBOBOXWIDGET_6759518_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

class cComboBoxWidget : public ImplRC<iWidgetSink,eImplFlags_Default,iWidgetComboBox>
{
  friend class cComboListBox;
  niBeginClass(cComboBoxWidget);
 public:
  cComboBoxWidget(iWidget *pWidget);
  ~cComboBoxWidget();

  iUnknown* __stdcall QueryInterface(const tUUID& aIID);
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const;

  // iWidgetComboBox
  iWidget* __stdcall GetDroppedWidget() const;
  tBool __stdcall SetDroppedWidget(iWidget* apWidget);
  void __stdcall SetNumLines(tU32 anNum);
  tU32 __stdcall GetNumLines() const;

  // iWidgetSink interface
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);

  // internals
  void _MoveFocus(tBool abToPrevious);
  void _DropList();
  void _HideList();
  void _AdjustChildrenLayout(tF32 w,tF32 h);
  void _LostFocus();
  void _UpdateStyle();
  void _UpdateEditBoxText();
  void _UpdateDroppedText();
  void _UpdateHover();
  tBool _SetDropped(iWidget* apDB, tBool abAdd);

 public:
  iWidget *mpWidget;
  Ptr<iWidget> mpwEditBox;
  Ptr<iWidget> mpwButton;
  Ptr<iWidget> mpwDroppedBox;
  Ptr<iWidgetSink> mptrDroppedSink;
  tU32  mnNumLines;
  tU32  mnStatus;
  niEndClass(cComboBoxWidget);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __COMBOBOXWIDGET_6759518_H__
