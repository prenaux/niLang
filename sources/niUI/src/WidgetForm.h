#ifndef __WIDGETFORM_13670959_H__
#define __WIDGETFORM_13670959_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Widget.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetForm declaration.
class cWidgetForm
    : public ni::ImplRC<ni::iWidgetSink,
                               ni::eImplFlags_Default,
                               ni::iWidgetForm,iWidgetDockable>
{
  niBeginClass(cWidgetForm);

  enum eResizeArea
  {
    eResizeArea_None = 0,
    eResizeArea_TopLeft,
    eResizeArea_TopRight,
    eResizeArea_BottomLeft,
    eResizeArea_BottomRight,
    eResizeArea_Left,
    eResizeArea_Right,
    eResizeArea_Top,
    eResizeArea_Bottom,
    eResizeArea_Title,
  };

 public:
  //! Constructor.
  cWidgetForm(iWidget* apWidget);
  //! Destructor.
  ~cWidgetForm();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  iUnknown* __stdcall QueryInterface(const tUUID& aIID);
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const;

  //// ni::iWidgetForm /////////////////////////
  tBool __stdcall SetTitle(const achar *aVal);
  const achar * __stdcall GetTitle() const;
  tBool __stdcall SetResizeBorderArea(tF32 aVal);
  tF32 __stdcall GetResizeBorderArea() const;

  tU32 __stdcall GetNumTitleWidgets() const;
  iWidget* __stdcall GetTitleWidget(tU32 anIndex) const;
  tU32 __stdcall GetTitleWidgetIndex(iWidget* apWidget) const;
  iWidget* __stdcall GetTitleWidgetFromID(iHString* ahspID) const;
  tBool __stdcall AddTitleWidget(iWidget* apWidget, tF32 afWidth);
  void __stdcall SetFormFrameFlags(tRectFrameFlags aFlags);
  tRectFrameFlags __stdcall GetFormFrameFlags() const;
  //// ni::iWidgetForm /////////////////////////

  //// iWidgetDockable //////////////////////////
  const achar* __stdcall GetDockName() const;
  tBool __stdcall GetIsDocked() const;
  //// iWidgetDockable //////////////////////////

  //// iWidgetSink //////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB);
  //// iWidgetSink //////////////////////////////

  void PushBeforeDock();
  void PopBeforeDock();
  tBool CanDock() const;
  void UpdateStyle();
  void UpdateClientSize();

  eResizeArea FindResizeArea(const sVec2f& aNCMousePos) const;

  void FitInParentRect();
  void _SetDrag(tU32 anDrag);
  void _HandleDragDock(tBool abEnd, const sVec2f* apMousePos = NULL);

 private:
  cWidget*             mpWidget;
  tHStringPtr          mhspTitle;
  tF32                 mfResizeBorderArea;
  tU32                 mnDrag;
  sVec2f            mvPrevMousePos;
  eResizeArea          mResizeArea;
  sVec2f            mvMinSize;
  tBool                mbIsDocked;
  WeakPtr<iWidget>     mpwDockedTo;
  tRectFrameFlags mFormFrameFlags;
  struct sBeforeDock {
    sRectf      mRect;
    tU32             mnStyle;
    eWidgetDockStyle mDockStyle;
    tBool            mbEnabled;
    tBool            mbVisible;
    tBool            mbIgnoreInput;
    eWidgetZOrder    mZOrder;
  } mBeforeDock;
  Ptr<iWidget>         mptrButtonClose;

  struct sSkin {
    sRectf   titleTextFrame;
    sRectf   titleFrame;
    tF32          titleHeight;
    sColor4f      backgroundColor;
    Ptr<iOverlay> normalFrame;
    Ptr<iFont>    normalFont;
    Ptr<iOverlay> inactiveFrame;
    Ptr<iFont>    inactiveFont;
    Ptr<iOverlay> curResizeTL;
    Ptr<iOverlay> curResizeTR;
    Ptr<iOverlay> curResizeHz;
    Ptr<iOverlay> curResizeVt;
  } skin;

  tBool mbActive;
  niEndClass(cWidgetForm);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGETFORM_13670959_H__
