// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetGroup.h"
#include "Widget.h"
#include "UIContext.h"
#include <niLang/Utils/UnitSnap.h>

static const tU32 _knFontHeightMarginExpanded = 8;
static const tU32 _knFontHeightMarginCollapsed = 4;

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetGroup implementation.

///////////////////////////////////////////////
cWidgetGroup::cWidgetGroup(iWidget *apWidget)
{
  ZeroMembers();
  mpWidget = apWidget;
  mbFolded = eFalse;
}

///////////////////////////////////////////////
cWidgetGroup::~cWidgetGroup()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetGroup::ZeroMembers()
{
  mpWidget = NULL;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetGroup::IsOK() const
{
  niClassIsOK(cWidgetGroup);
  return ni::eTrue;
}

///////////////////////////////////////////////
void cWidgetGroup::ComputeClientRect()
{
  iOverlay* frame = mbFolded ? skin.foldedFrame : skin.normalFrame;
  sRectf rectClient = frame->ComputeFrameCenter(mpWidget->GetWidgetRect());
  if (HStringIsNotEmpty(mpWidget->GetText())) {
    tF32 fonth = mpWidget->GetFont()->GetHeight()+_knFontHeightMarginExpanded;
    rectClient.SetTop(fonth);
  }
  sVec4f vPadding = mpWidget->GetPadding();
  rectClient.Left() += vPadding.Left();
  rectClient.Right() -= vPadding.Right();
  rectClient.Top() += vPadding.Top();
  rectClient.Bottom() -= vPadding.Bottom();
  mpWidget->SetClientRect(rectClient);
}

///////////////////////////////////////////////
void cWidgetGroup::NCPaint(iCanvas* apCanvas)
{
  iOverlay* frame = mbFolded ? skin.foldedFrame : skin.normalFrame;
  sRectf rect = mpWidget->GetWidgetRect();

  if (mpWidget->GetDrawOrder()%2) {
    if (ULColorGetA(skin.colOddGroup)) {
      apCanvas->BlitFill(rect,skin.colOddGroup);
    }
  }
  else {
    if (ULColorGetA(skin.colEvenGroup)) {
      apCanvas->BlitFill(rect,skin.colEvenGroup);
    }
  }

  if (HStringIsNotEmpty(mpWidget->GetText())) {
    iFont *font = mpWidget->GetFont();
    tF32 fonth = font->GetHeight()+_knFontHeightMarginExpanded;
    rect.SetTop(ni::UnitSnapf(fonth*0.4f));

    tBool didDrawFoldButton = eFalse;
    if (niFlagIs(mpWidget->GetStyle(),eWidgetGroupStyle_Fold)) {
      iOverlay* folded = GetFolded() ? skin.collapsed : skin.expanded;
      if (folded) {
        sVec2f size = folded->GetSize();
        apCanvas->BlitOverlay(
            sRectf(2 + rect.Left(), 6, size.x,size.y),
            folded);
        rect.SetLeft(rect.GetLeft() + size.x + 4);
        didDrawFoldButton = eTrue;
      }
    }
    else {
      rect.SetLeft(rect.GetLeft() + 4);
    }

    {
      const sVec4f frameSize = frame->GetFrame();
      const tF32 textoff = rect.GetLeft();
      const sRectf txtrect = apCanvas->BlitText(
          font,sRectf(textoff,4),
          0,niHStr(mpWidget->GetLocalizedText()));
      if (false) {
        const tF32 toplen = rect.GetWidth() - (textoff + (didDrawFoldButton?-4:7) + txtrect.GetWidth());
        apCanvas->BlitOverlayFrame(
          sRectf(textoff+txtrect.GetWidth()+6,rect.y+2,
                 toplen,frameSize.y1()),
          frame,
          eRectFrameFlags_TopEdge);
      }
    }
  }
  else {
    apCanvas->BlitOverlayFrame(
        rect,
        frame,
        eRectFrameFlags_All);
  }
}

///////////////////////////////////////////////
tBool __stdcall cWidgetGroup::OnWidgetSink(iWidget *apWidget, tU32 anMsg, const Var& avarA, const Var& avarB)
{
  niGuardObject((iWidgetSink*)this);
  switch (anMsg)
  {
    case eUIMessage_SkinChanged:
      _UpdateSkin();
      break;
    case eUIMessage_NCPaint: {
      iCanvas* c = VarQueryInterface<iCanvas>(avarB);
      if (c) {
        NCPaint(c);
      }
      break;
    }
    case eUIMessage_NCSize:
    case eUIMessage_Padding:
      ComputeClientRect();
      return eTrue;
    case eUIMessage_NCLeftDoubleClick: {
      if (niFlagIs(mpWidget->GetStyle(),eWidgetGroupStyle_Fold)) {
        SetFolded(!GetFolded());
      }
      break;
    }
    case eUIMessage_NCLeftClickDown: {
      if (niFlagIs(mpWidget->GetStyle(),eWidgetGroupStyle_Fold) &&
          HStringIsNotEmpty(mpWidget->GetText()))
      {
        sRectf rect = mpWidget->GetWidgetRect();

        iFont *font = mpWidget->GetFont();
        tF32 fonth = font->GetHeight()+_knFontHeightMarginExpanded;
        rect.SetTop(ni::UnitSnapf(fonth*0.4f));

        iOverlay* folded = GetFolded() ? skin.collapsed : skin.expanded;
        if (folded) {
          sVec2f size = folded->GetSize();
          sRectf rectPos(2 + rect.Left(), 4, size.x + 2, size.y + 2);
          if (rectPos.Intersect(avarB.GetVec2fValue())) {
            SetFolded(!GetFolded());
          }
        }
      }
      break;
    }
    case eUIMessage_SerializeWidget: {
      Ptr<iDataTable> ptrDT = ni::VarQueryInterface<iDataTable>(avarA);
      const tU32 nFlags = avarB.mU32;
      if (niFlagIs(nFlags,eWidgetSerializeFlags_Read)) {
        tU32 nIndex;
        nIndex = ptrDT->GetPropertyIndex(_A("folded"));
        if (nIndex != eInvalidHandle) {
          SetFolded(ptrDT->GetBoolFromIndex(nIndex));
        }
      }
      else {
        ptrDT->SetBool(_A("folded"),GetFolded());
      }
      break;
    }
  }
  return eFalse;
}

///////////////////////////////////////////////
void cWidgetGroup::_UpdateSkin() {
  const sColor4f color = sColor4f::Zero();
  skin.normalFrame = mpWidget->FindSkinElement(NULL,NULL,_H("Frame"));
  skin.foldedFrame = mpWidget->FindSkinElement(NULL,_H("Folded"),_H("Frame"));
  skin.collapsed = mpWidget->FindSkinElement(NULL,NULL,_H("Collapsed"));
  skin.expanded = mpWidget->FindSkinElement(NULL,NULL,_H("Expanded"));
  skin.colEvenGroup = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("EvenGroup")));
  skin.colOddGroup = ULColorBuild(mpWidget->FindSkinColor(color,NULL,NULL,_H("OddGroup")));
}

///////////////////////////////////////////////
void __stdcall cWidgetGroup::SetFolded(tBool abFolded) {
  if (abFolded == mbFolded) return;
  mbFolded = abFolded;
  niLoop(i,mpWidget->GetNumChildren()) {
    iWidget* c = mpWidget->GetChildFromIndex(i);
    c->SetVisible(!mbFolded);
  }
  if (mbFolded) {
    iOverlay* frame = skin.foldedFrame;
    mfOriginalHeight = mpWidget->GetSize().y;
    const tF32 newHeight =
        frame->GetFrame().y + frame->GetFrame().w +
        mpWidget->GetFont()->GetHeight() + _knFontHeightMarginCollapsed;
    mpWidget->SetMaximumSize(Vec2f(_kvMaxSize.x,newHeight));
    mpWidget->SetSize(Vec2f(mpWidget->GetSize().x,newHeight));
  }
  else {
    mpWidget->SetMaximumSize(_kvMaxSize);
    mpWidget->SetSize(Vec2f(mpWidget->GetSize().x,mfOriginalHeight));
  }
  ((cWidget*)mpWidget)->SetHideChildren(mbFolded);
  ((cWidget*)mpWidget)->UpdateParentAutoLayout("SetFolded");
}
tBool __stdcall cWidgetGroup::GetFolded() const {
  return mbFolded;
}
