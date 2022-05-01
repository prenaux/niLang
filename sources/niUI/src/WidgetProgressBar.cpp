// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetProgressBar.h"

cProgressBarWidget::cProgressBarWidget(iWidget *pWidget)
{
  mpWidget = pWidget;
  mfMin = 0;
  mfMax = 100;
  mfPos = 0;
}

cProgressBarWidget::~cProgressBarWidget()
{
}

void cProgressBarWidget::ComputeClientRect(tF32 w,tF32 h)
{
  mpWidget->SetClientRect(skin.normalFrame->ComputeFrameCenter(sRectf(0,0,w,h)));
}

tBool cProgressBarWidget::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch(nMsg)
  {
    case eUIMessage_SkinChanged:
      InitSkin();
      return eFalse;
    case eUIMessage_NCSize:
      ComputeClientRect(varParam0.mV2F[0],varParam0.mV2F[1]);
      break;
    case eUIMessage_NCPaint:
      {
        iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
        if (c) {
          //                 skin.normalFrame->DrawFrameEx(NULL,eRectFrameFlags_All,
          //                                               mpWidget->GetWidgetRect().GetTopLeft(),
          //                                               mpWidget->GetWidgetRect().GetSize());
          c->BlitOverlayFrame(mpWidget->GetWidgetRect(),
                              skin.normalFrame,
                              eRectFrameFlags_All);
        }
      }
      break;
    case eUIMessage_Paint:
      {
        iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
        if (c) {
          sRectf clirect = sRectf(0,0,mpWidget->GetClientSize().x,mpWidget->GetClientSize().y);
          tF32 scale = mfPos / (mfMax - mfMin);
          if (mpWidget->GetStyle()& eWidgetProgressBarStyle_Vert) {
            tF32 clih = clirect.GetHeight();
            tF32 y = clih-(clih*scale);
            tF32 h = clih - y;
            //                     skin.fill->DrawEx(0,Vec2<tF32>(0,y),Vec2<tF32>(clirect.GetWidth(),h));
            c->BlitOverlay(sRectf(0,y,clirect.GetWidth(),h),skin.fill);
          }
          else {
            //                     skin.fill->DrawEx(0,sVec2f::Zero(),Vec2<tF32>(clirect.GetWidth()*scale,clirect.GetHeight()));
            c->BlitOverlay(
                sRectf(0,0,clirect.GetWidth()*scale,clirect.GetHeight()),
                skin.fill);
          }
          {
            c->BlitText(
                mpWidget->GetFont(),
                clirect,
                eFontFormatFlags_CenterH|eFontFormatFlags_CenterV,
                niFmt(_A("%.0f%%"),scale*100.0f));
          }
        }
      }
      break;
    case eUIMessage_SerializeWidget:
      {
        Ptr<iDataTable> ptrDT = ni::VarQueryInterface<iDataTable>(varParam0);
        tU32 nFlags = varParam1.mU32;
        if (niFlagIs(nFlags,eWidgetSerializeFlags_Read)) {
          tU32 nIndex;
          nIndex = ptrDT->GetPropertyIndex(_A("progress_min"));
          if (nIndex != eInvalidHandle) {
            mfMin = ptrDT->GetFloatFromIndex(nIndex);
          }
          nIndex = ptrDT->GetPropertyIndex(_A("progress_max"));
          if (nIndex != eInvalidHandle) {
            mfMax = ptrDT->GetFloatFromIndex(nIndex);
          }
          SetRange(mfMin,mfMax);
          nIndex = ptrDT->GetPropertyIndex(_A("progress"));
          if (nIndex != eInvalidHandle) {
            SetProgress(ptrDT->GetFloatFromIndex(nIndex));
          }
        }
        else {
          ptrDT->SetFloat(_A("progress"),GetProgress());
          ptrDT->SetFloat(_A("progress_min"),mfMin);
          ptrDT->SetFloat(_A("progress_max"),mfMax);
        }
        break;
      }
    default:
      return eFalse;
  }
  return eTrue;
}

void cProgressBarWidget::SetRange(tF32 afMin,tF32 afMax)
{
  if(afMin>afMax)
  {
    mfMax = afMin;
    mfMin = afMax;
  }
  else
  {
    mfMax = afMax;
    mfMin = afMin;
  }
}

void cProgressBarWidget::SetProgress(tF32 afPos)
{
  mfPos = afPos>mfMax ? mfMax:(afPos<mfMin ? mfMin:afPos);
}

tF32 cProgressBarWidget::GetProgress() const
{
  return mfPos;
}

void cProgressBarWidget::InitSkin()
{
  skin.normalFrame = mpWidget->FindSkinElement(NULL,NULL,_H("Frame"));
  skin.fill = mpWidget->FindSkinElement(NULL,NULL,_H("Fill"));
}
