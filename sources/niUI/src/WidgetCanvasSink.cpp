// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetCanvasSink.h"
#include "Font.h"
#include "FontTTF.h"
#include "HardCodedSkin.h"
#include "niUI_HString.h"

#define STEP_SIZE_V   mfStepV
#define STEP_SIZE_H   mfStepH
#define SCROLLBAR_SIZE_V  ((ScrollV()&&!HideScrollV()) ? kfScrollBarSize : 0)
#define SCROLLBAR_SIZE_H  ((ScrollH()&&!HideScrollH()) ? kfScrollBarSize : 0)

#pragma niNote("The filling rules for the renderer is that the last pixel is not drawn")
#pragma niTodo("Add a possibility to set a iMetrics in the math expression evaluator, so that it supports parsing those automatically.")
#pragma niTodo("Add a the UI color parsing to the math expression evaluator.")
#pragma niTodo("Add a weight parameter to iFont.")

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetCanvasSink implementation.

///////////////////////////////////////////////
cWidgetCanvasSink::cWidgetCanvasSink(iWidget* apWidget)
{
  ZeroMembers();
  mpWidget = apWidget;
  UpdateStyle();
}

///////////////////////////////////////////////
cWidgetCanvasSink::~cWidgetCanvasSink()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetCanvasSink::ZeroMembers()
{
  mbUpdateScroll = eFalse;
  mfStepH = mfStepV = 20.0f;
  mfMarginH = mfMarginV = 0.0f;
  mvBorder = sVec4f::Zero();
  skin.mnFillColor = 0;
  skin.mnBorderColor = 0;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetCanvasSink::IsOK() const
{
  niClassIsOK(cWidgetCanvasSink);
  return ni::eTrue;
}

///////////////////////////////////////////////
tBool cWidgetCanvasSink::OnWidgetSink(ni::iWidget *apWidget, tU32 nMsg, const Var& aA, const Var& aB)
{
  niGuardObject((iWidgetSink*)this);
  switch (nMsg) {
    case eUIMessage_SkinChanged: {
      InitSkin();
      break;
    }
    case eUIMessage_Layout: {
      if (!IsScrollCanvas()) return eFalse;
      const tU32 autoLayoutFlags = mpWidget->GetAutoLayout();
      tU32 nNumIt = 0;
      do {
        mpWidget->ComputeAutoLayout(autoLayoutFlags&(~eWidgetAutoLayoutFlags_Size));
      } while (UpdateScrolls() && nNumIt++ < 5);
      return eTrue;
    }
    case eUIMessage_StyleChanged: {
      UpdateStyle();
      break;
    }
    case eUIMessage_NCSize: {
      if (!IsScrollCanvas()) return eFalse;
      UpdateScrolls();
      return eTrue; // keep the client size as is...
    }
    case eUIMessage_Size: {
      if (!IsScrollCanvas()) return eFalse;
      UpdateScrolls();
      break;
    }
    case eUIMessage_NCWheel:
    case eUIMessage_Wheel: {
      if (mpwScrollV.IsOK() && mpwScrollV->GetEnabled()) {
        mpwScrollV->SendMessage(nMsg,aA,aB);
        return eTrue;
      }
      else if (mpwScrollH.IsOK() && mpwScrollH->GetEnabled()) {
        mpwScrollH->SendMessage(nMsg,aA,aB);
        return eTrue;
      }
      return eFalse;
    }
    case eUIMessage_Command: {
      if (IsScrollCanvas()) {
        Ptr<iWidgetCommand> cmd = VarQueryInterface<iWidgetCommand>(aA);
        if (cmd.IsOK()) {
          if (cmd->GetSender() == mpwScrollH && cmd->GetID() == eWidgetScrollBarCmd_Scrolled) {
            UpdateClientPos();
          }
          else if (cmd->GetSender() == mpwScrollV && cmd->GetID() == eWidgetScrollBarCmd_Scrolled) {
            UpdateClientPos();
          }
        }
      }
      return eFalse;
    }
    case eUIMessage_NCPaint: {
      const tBool hasFillColor = skin.mnFillColor != 0;
      const tBool hasBorderColor = (skin.mnBorderColor && GetHasBorder());
      if (hasFillColor || hasBorderColor) {
        QPtr<iCanvas> c = aB;
        if (c.IsOK()) {
          sRectf clientRect = mpWidget->GetClientRect();
          if (hasFillColor) {
            c->BlitFillAlpha(clientRect,skin.mnFillColor);
          }
          if (hasBorderColor) {
            const sVec4f padding = mpWidget->GetPadding();
            clientRect.Left() -= padding.Left();
            clientRect.Top() -= padding.Top();
            clientRect.Right() += padding.Right();
            clientRect.Bottom() += padding.Bottom();
            sRectf left(clientRect.GetTopLeft(),clientRect.GetBottomLeft()+Vec2<tF32>(mvBorder.Left(),0));
            sRectf right(clientRect.GetTopRight()-Vec2<tF32>(mvBorder.Right(),0),clientRect.GetBottomRight());
            sRectf top(clientRect.GetTopLeft(),clientRect.GetTopRight()+Vec2<tF32>(0,mvBorder.Top()));
            sRectf bottom(clientRect.GetBottomLeft()-Vec2<tF32>(0,mvBorder.Bottom()),clientRect.GetBottomRight());
            if (mvBorder.Left()) {
              c->BlitFillAlpha(left,skin.mnBorderColor);
              top.Left() += mvBorder.Left();
              bottom.Left() += mvBorder.Left();
            }
            if (mvBorder.Right()) {
              c->BlitFillAlpha(right,skin.mnBorderColor);
              top.Right() -= mvBorder.Right();
              bottom.Right() -= mvBorder.Right();
            }
            if (mvBorder.Top()) {
              c->BlitFillAlpha(top,skin.mnBorderColor);
            }
            if (mvBorder.Bottom()) {
              c->BlitFillAlpha(bottom,skin.mnBorderColor);
            }
          }
        }
      }
      break;
    }
    case eUIMessage_SerializeWidget: {
      QPtr<iDataTable> ptrDT = aA;
      if (ptrDT.IsOK()) {
        const tU32 nFlags = aB.mU32;
        if (nFlags & eWidgetSerializeFlags_Write) {
          ptrDT->SetVec4("border",mvBorder);
        }
        else if (nFlags & eWidgetSerializeFlags_Read) {
          mvBorder = ptrDT->GetVec4Default("border",mvBorder);
        }
      }
      break;
    }
    default:
      return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
void cWidgetCanvasSink::_InitializeScrollBars() {
  if (ScrollH()) {
    if (!mpwScrollH.IsOK()) {
      mpwScrollH = mpWidget->GetUIContext()->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(0,0),eWidgetScrollBarStyle_Horz|eWidgetStyle_NCRelative|eWidgetStyle_DontSerialize,_HC(ID_CanvasScrollH));
      if (mpwScrollH.IsOK()) {
        mpwScrollH->SetStatus(eFalse,eFalse,eTrue);
        mpwScrollH->SetZOrder(eWidgetZOrder_Overlay);
      }
    }
  }
  else {
    if (mpwScrollH.IsOK()) {
      mpwScrollH->Destroy();
      mpwScrollH = NULL;
    }
  }
  if (ScrollV()) {
    if (!mpwScrollV.IsOK()) {
      mpwScrollV = mpWidget->GetUIContext()->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(0,0),eWidgetStyle_NCRelative|eWidgetStyle_DontSerialize,_HC(ID_CanvasScrollV));
      if (mpwScrollV.IsOK()) {
        mpwScrollV->SetStatus(eFalse,eFalse,eTrue);
        mpwScrollV->SetZOrder(eWidgetZOrder_Overlay);
      }
    }
  }
  else {
    if (mpwScrollV.IsOK()) {
      mpwScrollV->Destroy();
      mpwScrollV = NULL;
    }
  }
}

void cWidgetCanvasSink::UpdateStyle()
{
  mbUpdateScroll = eTrue; // updating the scrolling now...
  _InitializeScrollBars();
  mbUpdateScroll = eFalse;
  if (IsScrollCanvas())
    UpdateScrolls();
}

///////////////////////////////////////////////
tBool cWidgetCanvasSink::UpdateScrolls()
{
  tBool bRet = eFalse;
  if (mbUpdateScroll)
    return eTrue;

  mbUpdateScroll = eTrue;

  // make sure the scroll bars are valid, its fairly common to call
  // InvalidateChildren() on a Canvas which will wipe the scroll bars
  _InitializeScrollBars();

  sVec2f size = mpWidget->GetSize();
  sVec2f minSize;
  sVec2f clientSize = mpWidget->GetClientSize();

  if (niFlagIs(mpWidget->GetAutoLayout(),eWidgetAutoLayoutFlags_Size)) {
    sVec2f vNewSize = {0,0};
    for (tU32 i = 0; i < mpWidget->GetNumChildren(); ++i) {
      iWidget* pW = mpWidget->GetChildFromIndex(i);
      if (niFlagIs(pW->GetStyle(),eWidgetStyle_NCRelative))
        continue;
      sRectf rect = pW->GetRect();
      sVec4f vMargin = pW->GetMargin();
      tF32 right = rect.Right()+vMargin.Right();
      if (right > vNewSize.x) vNewSize.x = right;
      tF32 bottom = rect.Bottom()+vMargin.Bottom();
      if (bottom > vNewSize.y) vNewSize.y = bottom;
    }
    if (vNewSize.x > 0) clientSize.x = vNewSize.x;
    if (vNewSize.y > 0) clientSize.y = vNewSize.y;
  }

  VecMinimize(minSize,clientSize,size);
  const tBool bNeedScrollH = ScrollH()&&(clientSize.x > size.x);
  const tBool bNeedScrollV = ScrollV()&&(clientSize.y > size.y);

  if (!ScrollH()) {
    minSize.x = size.x;
    clientSize.x = size.x;
    if (bNeedScrollV)
      clientSize.x -= SCROLLBAR_SIZE_V;
  }
  if (!ScrollV()) {
    minSize.y = size.y;
    clientSize.y = size.y;
    if (bNeedScrollH)
      clientSize.y -= SCROLLBAR_SIZE_H;
  }

  if (ScrollH()) {
    QPtr<iWidgetScrollBar> ptrScroll = mpwScrollH.ptr();
    if (ptrScroll.IsOK()) {
      if (bNeedScrollH) {
        mpwScrollH->SetStatus(!HideScrollH(),eTrue,eFalse);
        mpwScrollH->SetSize(
            Vec2<tF32>(minSize.x-((clientSize.y > size.y)?SCROLLBAR_SIZE_H:0),SCROLLBAR_SIZE_H));
        ptrScroll->SetScrollRange(
            Vec2<tF32>(
                -mfMarginH,
                mfMarginH+((clientSize.x-size.x)+(bNeedScrollV?SCROLLBAR_SIZE_H:0))
                       )/STEP_SIZE_H);
      }
      else {
        // ptrScroll->SetScrollPosition(mfMarginH);
        mpwScrollH->SetStatus(eFalse,eFalse,eTrue);
      }
    }
  }
  if (ScrollV()) {
    QPtr<iWidgetScrollBar> ptrScroll = mpwScrollV.ptr();
    if (ptrScroll.IsOK()) {
      if (bNeedScrollV) {
        mpwScrollV->SetStatus(!HideScrollV(),eTrue,eFalse);
        mpwScrollV->SetSize(
            Vec2<tF32>(SCROLLBAR_SIZE_V,minSize.y/*-((clientSize.x > size.x)?SCROLLBAR_SIZE:0)*/));
        ptrScroll->SetScrollRange(
            Vec2<tF32>(
                -mfMarginV,
                mfMarginV+((clientSize.y-size.y)+(bNeedScrollH?SCROLLBAR_SIZE_V:0))
                       )/STEP_SIZE_V);
      }
      else {
        // ptrScroll->SetScrollPosition(mfMarginV);
        mpwScrollV->SetStatus(eFalse,eFalse,eTrue);
      }
    }
  }
  if (clientSize != mpWidget->GetClientSize()) {
    bRet = eTrue;
    mpWidget->SetClientSize(clientSize);
  }
  UpdateClientPos();
  mbUpdateScroll = eFalse;
  return bRet;
}

///////////////////////////////////////////////
void cWidgetCanvasSink::UpdateClientPos()
{
  sVec2f size = mpWidget->GetSize();
  sVec2f pos = {0,0};
  if (mpwScrollH.IsOK() && mpwScrollH->GetEnabled()) {
    pos.x = -QPtr<iWidgetScrollBar>(mpwScrollH)->GetScrollPosition()*STEP_SIZE_H;
    mpwScrollH->SetPosition(Vec2<tF32>(0,size.y-SCROLLBAR_SIZE_H));
  }
  if (mpwScrollV.IsOK() && mpwScrollV->GetEnabled()) {
    pos.y = -QPtr<iWidgetScrollBar>(mpwScrollV)->GetScrollPosition()*STEP_SIZE_V;
    mpwScrollV->SetPosition(Vec2<tF32>(size.x-SCROLLBAR_SIZE_V,0));
  }
  mpWidget->SetClientPosition(pos);
}

///////////////////////////////////////////////
tBool cWidgetCanvasSink::ScrollV() const {
  return niFlagIs(mpWidget->GetStyle(),eWidgetCanvasStyle_ScrollV);
}
tBool cWidgetCanvasSink::HideScrollV() const {
  return niFlagIs(mpWidget->GetStyle(),eWidgetCanvasStyle_HideScrollV);
}

///////////////////////////////////////////////
tBool cWidgetCanvasSink::ScrollH() const {
  return niFlagIs(mpWidget->GetStyle(),eWidgetCanvasStyle_ScrollH);
}
tBool cWidgetCanvasSink::HideScrollH() const {
  return niFlagIs(mpWidget->GetStyle(),eWidgetCanvasStyle_HideScrollH);
}

///////////////////////////////////////////////
tBool cWidgetCanvasSink::IsScrollCanvas() const
{
  return  ScrollH() || ScrollV();
}

///////////////////////////////////////////////
void cWidgetCanvasSink::InitSkin() {
  skin.mnBorderColor = ULColorBuild(mpWidget->FindSkinColor(Vec4<tF32>(1,1,1,1),NULL,NULL,_H("Border")));
  skin.mnFillColor = ULColorBuild(mpWidget->FindSkinColor(Vec4<tF32>(0,0,0,0),NULL,NULL,_H("Fill")));
}
