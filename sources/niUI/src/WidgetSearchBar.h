// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifndef WIDGETSEARCHBAR_H_
#define WIDGETSEARCHBAR_H_

#include "API/niUI/IWidgetSearchBar.h"

typedef astl::pair<tU32, cString> tSearchItem;

// TODO: Expose that in IWidgetSearchBar?
struct iSearchEngine : public iUnknown
{
  niDeclareInterfaceUUID(iSearchEngine, 0xbe57611e, 0x58a2, 0x4f10, 0xa6, 0x33, 0x2b, 0xb2, 0x70, 0x93, 0xb2, 0xbe)
  virtual void __stdcall SetCorpus(const astl::vector<tSearchItem>& avCorpus) = 0;
  virtual void __stdcall Search(const achar* aSearchTerm, astl::vector<tSearchItem>& avResult) = 0;
};

class cWidgetSearchBar : public cIUnknownImpl<iWidgetSink, eIUnknownImplFlags_Default, ni::iWidgetSearchBar>
{
  niBeginClass(cWidgetSearchBar);

 public:
  cWidgetSearchBar(iWidget* pWidget);
  ~cWidgetSearchBar();

  iUnknown* __stdcall QueryInterface(const tUUID& aIID);
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const;

  virtual void __stdcall SetSearchable(iSearchable* aS);
  virtual iSearchable* __stdcall GetSearchable() const {
    return mptrSearchable;
  }

  // iWidgetSink interface
  virtual tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);

  // internals
  void _InitLayout();
  void _AdjustChildrenLayout(tF32 w, tF32 h);
  void _MoveFocus(tBool abToPrevious);
  void _LostFocus();
  void _UpdateEditBoxText();
  void _OnKeyDown(tU32 key, iUnknown* apOriginalWidget);
  void _Suicide();

 private:
  iWidget*          mpWidget;
  Ptr<iWidget>      mptrInputBox; //QPtr<iWidgetComboBox>
  Ptr<iWidget>      mptrShowBox;  //QPtr<iWidgetListBox>

  astl::vector<tSearchItem> mvResult;   //result of each search
  Ptr<iSearchable>    mptrSearchable; //things to be searched
  Ptr<iSearchEngine>  mptrSE;     //search service provider

  niEndClass(cWidgetSearchBar);
};

#endif /* WIDGETSEARCHBAR_H_ */
