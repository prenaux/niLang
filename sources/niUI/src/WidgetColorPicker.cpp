// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"

#if niMinFeatures(20)

#include <niUI/Utils/WidgetSinkImpl.h>
#include <niUI/IWidgetColorPicker.h>
#include <niUI/Utils/ColorUtils.h>

static const tF32 _kfControlsVSpacing = 0.0f;
static const tF32 _kfControlsHSpacing = 0.0f;
static const tF32 _kfRectHRatioHeight = 1.0f;
static const tF32 _kfRectSVRatioHeight = 9.5f;
static const tF32 _kfRectBRatioHeight = 0.5f;
static const tF32 _kfRectSVRatioSize = (1.0f/7.0f)*6.0f;
static const tF32 _kfRectRGBRatioSize = (1.0f/7.0f)*0.5f;
static const tF32 _kfRectARatioSize = (1.0f/7.0f)*0.5f;
static const tU32 _knAlphaSelectionLine = ULColorBuildf(1.0f,0,0,0.65f);
static const tF32 _kfMarkerAlpha = 0.7f;

#define DRAGGING_NO   0
#define DRAGGING_HUE  1
#define DRAGGING_SV   2
#define DRAGGING_ALPHA  3
#define DRAGGING_B    4

//--------------------------------------------------------------------------------------------
//
//  Widget ColorPicker implementation.
//
//--------------------------------------------------------------------------------------------
class cWidgetColorPicker : public ni::cWidgetSinkImpl<ni::iWidgetColorPicker>
{
  niBeginClass(cWidgetColorPicker);

 public:
  //! Constructor.
  cWidgetColorPicker(iWidget *apWidget) {
    ZeroMembers();
  }

  //! Destructor.
  ~cWidgetColorPicker() {
  }

  //! Zeros all the class members.
  void ZeroMembers() {
    mHSVA = Vec4<tF32>(0,0,0,1);
    mnDragging = DRAGGING_NO;
    mfBaseHeight = 14.0f;
    mfBrightness = 1.0f;
    mfMaxBrightness = 10.0f;
  }

  //! Sanity check.
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cWidgetColorPicker);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnSetText(tBool abInSerialize) {
    mpWidget->SetText(GetExpression());
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnTextChanged() {
    SetExpression(mpWidget->GetText());
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnSetCursor(const sVec2f& avMousePos, const sVec2f& avNCMousePos) {
    mpWidget->GetUIContext()->SetCursor(skin.curCross);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnSinkAttached() {
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual void PickColor(const sVec2f& avPos) {
    switch (mnDragging) {
      case DRAGGING_HUE:
        {
          sColor3f hsv = GetHSV();
          hsv.x = ni::ClampZeroOne((avPos.x-mrectH.GetLeft())/mrectH.GetWidth());
          _SetHSV(hsv,mnDragging);
          break;
        }
      case DRAGGING_SV:
        {
          sColor3f hsv = GetHSV();
          hsv.y = ni::ClampZeroOne((avPos.x-mrectSV.GetLeft())/mrectSV.GetWidth());
          hsv.z = 1.0f-ni::ClampZeroOne((avPos.y-mrectSV.GetTop())/mrectSV.GetHeight());
          // The clamping is a hack so that the Hue isn't reset to 0 when the
          // Saturation/Luminance is set to zero. TODO: We should do the color
          // picker better, it should only store and handle HSV and let the
          // user take care of the conversions.
          static const tF32 clampEpsilon = 1e-5f;
          if (hsv.y == 0.0f) hsv.y = clampEpsilon;
          else if (hsv.y == 1.0f) hsv.y = 1.0f - clampEpsilon;
          if (hsv.z == 0.0f) hsv.z = clampEpsilon;
          else if (hsv.z == 1.0f) hsv.z = 1.0f - clampEpsilon;
          _SetHSV(hsv,mnDragging);
          break;
        }
      case DRAGGING_ALPHA:
        {
          sColor4f rgba = GetRGBA();
          rgba.w = 1.0f-ni::ClampZeroOne((avPos.y-mrectA.GetTop())/mrectA.GetHeight());
          _SetRGBA(rgba,mnDragging);
          break;
        }
      case DRAGGING_B:
        {
          tF32 b = mfMaxBrightness * ni::ClampZeroOne((avPos.x-mrectB.GetLeft())/mrectB.GetWidth());
          _SetBrightness(b,mnDragging);
          break;
        }
    };
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnMouseMove(const sVec2f& avPos, const sVec2f& avNCPos) {
    PickColor(avPos);
    return eFalse;
  }
  virtual tBool __stdcall OnNCMouseMove(const sVec2f& avPos, const sVec2f& avNCPos) {
    return OnMouseMove(avPos,avNCPos);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnNCLeftClickDown(const sVec2f& avPos, const sVec2f& avNCPos) {
    return OnLeftClickDown(avPos,avNCPos);
  }
  virtual tBool __stdcall OnLeftClickDown(const sVec2f& avPos, const sVec2f& avNCPos) {
    mnDragging = DRAGGING_NO;
    if (mrectH.Intersect(avPos)) {
      mnDragging = DRAGGING_HUE;
    }
    else if (mrectSV.Intersect(avPos)) {
      mnDragging = DRAGGING_SV;
    }
    else if (mrectA.Intersect(avPos)) {
      mnDragging = DRAGGING_ALPHA;
    }
    else if (mrectB.Intersect(avPos)) {
      mnDragging = DRAGGING_B;
    }
    PickColor(avPos);
    if (mnDragging) {
      mpWidget->SetCapture(eTrue);
    }
    return eFalse;
  }
  virtual tBool __stdcall OnLeftClickUp(const sVec2f& avPos, const sVec2f& avNCPos) {
    if (mnDragging) {
      mpWidget->SetCapture(eFalse);
    }
    mnDragging = DRAGGING_NO;
    return eFalse;
  }
  virtual tBool __stdcall OnNCLeftClickUp(const sVec2f& avPos, const sVec2f& avNCPos) {
    return OnLeftClickUp(avPos,avNCPos);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnNCSize(const sVec2f& avNewSize) {
    _UpdateLayout();
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnNCPaint(const sVec2f& avMousePos, iCanvas* apCanvas) {
    //    skin.frame->DrawFrameEx(NULL,
    //                eRectFrameFlags_All,
    //                sVec2f::Zero(),
    //                mpWidget->GetSize());
    apCanvas->BlitOverlayFrame(
        mpWidget->GetWidgetRect(),
        skin.frame,
        eRectFrameFlags_All);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) {
    sRectf rect;
    sColor4f colHSV = mHSVA;
    sColor4f colBrightRGBA = ColorConvert_HSV_RGB(Vec4<tF32>(colHSV.x,1,1,1));
    sColor4f colRGBA = GetRGBA();

    // draw the hue
    apCanvas->Flush();
    apCanvas->SetMaterial(NULL);
    apCanvas->GetMaterial()->SetChannelTexture(eMaterialChannel_Base,GetTextureHue(apCanvas->GetGraphicsContext()->GetGraphics()));
    //      apCanvas->Rect(mrectH.GetTopLeft(),
    //              mrectH.GetBottomRight(),
    //            0.0f);
    apCanvas->Rect(sVec2f::Zero(),
                  mpWidget->GetClientRect().GetSize(),
                  0.0f);
    apCanvas->Flush();
    apCanvas->GetMaterial()->SetChannelTexture(eMaterialChannel_Base,NULL);

    // draw the saturation/brightness
    {
      sColor4f colTL, colTR, colBL, colBR;
      colTL = sColor4f::White();
      colTR = colBrightRGBA;
      colBL = sColor4f::Black();
      colBR = sColor4f::Black();
      apCanvas->RectTA2(
          mrectSV.GetTopLeft(),mrectSV.GetBottomRight(),
          sVec2f::Zero(),sVec2f::Zero(),0.0f,
          ULColorBuild(colTL),
          ULColorBuild(colTR),
          ULColorBuild(colBR),
          ULColorBuild(colBL));
    }

    // RGB rectangle
    /*apCanvas->RectA(mrectRGB.GetTopLeft(),
      mrectRGB.GetBottomRight(),
      0.0f,
      0xFF777777);
      rect = sRectf(mrectRGB.GetTopLeft()+sVec2f::One(),mrectRGB.GetBottomRight()-sVec2f::One());*/
    rect = mrectRGB;
    apCanvas->RectA(rect.GetTopLeft(),
               rect.GetBottomRight(),
               0.0f,
               ULColorBuildf(colRGBA.x,colRGBA.y,colRGBA.z,1.0f));

    // alpha rectangle
    /*apCanvas->RectA(mrectA.GetTopLeft(),
      mrectA.GetBottomRight(),
      0.0f,
      0xFF777777);
      rect = sRectf(mrectA.GetTopLeft()+sVec2f::One(),mrectA.GetBottomRight()-sVec2f::One());*/
    rect = mrectA;
    apCanvas->RectTA2(rect.GetTopLeft(),rect.GetBottomRight(),
                 sVec2f::Zero(),sVec2f::Zero(),0.0f,
                 ULColorBuild(sColor4f::White()),
                 ULColorBuild(sColor4f::White()),
                 ULColorBuild(sColor4f::Black()),
                 ULColorBuild(sColor4f::Black()));

    // brightness rectangle
    if (niFlagIs(mpWidget->GetStyle(),eWidgetColorPickerStyle_Brightness)) {
      /*apCanvas->RectA(mrectB.GetTopLeft(),
        mrectB.GetBottomRight(),
        0.0f,
        0xFF777777);
        rect = sRectf(mrectB.GetTopLeft()+sVec2f::One(),mrectB.GetBottomRight()-sVec2f::One());*/
      rect = mrectB;
      sRectf rectGradA = sRectf(rect.GetLeft(),rect.GetTop(),rect.GetWidth()*(1.0f/mfMaxBrightness),rect.GetHeight());
      sRectf rectGradB = sRectf(rectGradA.GetTopRight(),rect.GetBottomRight());
      apCanvas->RectTA2(rectGradA.GetTopLeft(),rectGradA.GetBottomRight(),
                   sVec2f::Zero(),sVec2f::Zero(),0.0f,
                   ULColorBuild(sColor4f::Black()),
                   ULColorBuild(colRGBA),
                   ULColorBuild(colRGBA),
                   ULColorBuild(sColor4f::Black()));
      apCanvas->RectTA2(rectGradB.GetTopLeft(),rectGradB.GetBottomRight(),
                   sVec2f::Zero(),sVec2f::Zero(),0.0f,
                   ULColorBuild(colRGBA),
                   ULColorBuild(sColor4f::White()),
                   ULColorBuild(sColor4f::White()),
                   ULColorBuild(colRGBA));
    }

    // setup the target line drawing
    apCanvas->Flush();
    apCanvas->GetMaterial()->SetBlendMode(eBlendMode_Translucent);

    // draw the hue line
    tF32 xH = ni::Lerp(mrectH.GetLeft(),mrectH.GetRight(),colHSV.x);
    apCanvas->SetColorA(ULColorBuild((sColor3f::One()-(sColor3f&)colBrightRGBA),_kfMarkerAlpha));
    apCanvas->Rect(Vec2<tF32>(xH-1,mrectH.GetTop()),Vec2<tF32>(xH+1,mrectH.GetBottom()),0.0f);

    // draw the selection cross
    tF32 xS = ni::Lerp(mrectSV.GetLeft(),mrectSV.GetRight(),colHSV.y);
    tF32 xV = ni::Lerp(mrectSV.GetBottom(),mrectSV.GetTop(),colHSV.z);
    apCanvas->SetColorA(ULColorBuild((sColor3f::One()-(sColor3f&)colBrightRGBA),_kfMarkerAlpha));
    apCanvas->Rect(Vec2<tF32>(xS-1,mrectSV.GetTop()),Vec2<tF32>(xS+1,mrectSV.GetBottom()),0.0f);
    apCanvas->Rect(Vec2<tF32>(mrectSV.GetLeft(),xV-1),Vec2<tF32>(mrectSV.GetRight(),xV+1),0.0f);

    // draw the alpha line
    apCanvas->SetColorA(_knAlphaSelectionLine);
    tF32 xA = ni::Lerp(mrectA.GetBottom(),mrectA.GetTop(),mHSVA.w);
    apCanvas->Rect(Vec2<tF32>(mrectA.GetLeft(),xA-1),Vec2<tF32>(mrectA.GetRight(),xA+1),0.0f);

    if (niFlagIs(mpWidget->GetStyle(),eWidgetColorPickerStyle_Brightness)) {
      // draw the brightness line
      apCanvas->SetColorA(_knAlphaSelectionLine);
      tF32 xB = ni::Lerp(mrectB.GetLeft(),mrectB.GetRight(),mfBrightness/mfMaxBrightness);
      apCanvas->Rect(Vec2<tF32>(xB-1,mrectB.GetTop()),Vec2<tF32>(xB+1,mrectB.GetBottom()),0.0f);
    }

    apCanvas->Flush();
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall OnSkinChanged() {
    skin.frame = mpWidget->FindSkinElement(NULL,NULL,_H("Frame"));
    skin.curCross = mpWidget->FindSkinCursor(NULL,NULL,_H("Cross"));
    _UpdateLayout();
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual ni::iTexture* GetTextureHue(iGraphics* apGraphics) {
    if (mptrTextureHue.IsOK())
      return mptrTextureHue;

    mptrTextureHue = apGraphics->GetTextureFromName(_H("ColorPicker_Hue"));
    if (mptrTextureHue.IsOK())
      return mptrTextureHue;

    Ptr<iBitmap2D> ptrBmp = apGraphics->CreateBitmap2D(512,2,_A("R8G8B8"));
    tF32 fInvWidth = ni::FInvert(tF32(ptrBmp->GetWidth()));
    for (tU32 x = 0; x < ptrBmp->GetWidth(); ++x) {
      sColor4f col = ColorConvert_HSV_RGB(Vec4<tF32>(tF32(x)*fInvWidth,1,1,1));
      ptrBmp->PutPixelf(x,0,col);
      ptrBmp->PutPixelf(x,1,col);
    }

    mptrTextureHue = apGraphics->CreateTextureFromBitmap(_H("ColorPicker_Hue"),ptrBmp,eTextureFlags_Default);
    return mptrTextureHue;
  }

  ///////////////////////////////////////////////
  void _UpdateLayout() {
    sRectf clientRect = skin.frame->ComputeFrameCenter(mpWidget->GetWidgetRect());
    mpWidget->SetClientRect(clientRect);
    tF32 clW = clientRect.GetWidth();
    tF32 clH = clientRect.GetHeight();

    mfBaseHeight = clH;
    tF32 fTotalRatio = _kfRectHRatioHeight+_kfRectSVRatioHeight;
    mfBaseHeight -= _kfControlsVSpacing;
    if (niFlagIs(mpWidget->GetStyle(),eWidgetColorPickerStyle_Brightness)) {
      fTotalRatio += _kfRectBRatioHeight;
      mfBaseHeight -= _kfControlsVSpacing;
    }
    mfBaseHeight = (mfBaseHeight/fTotalRatio);

    tF32 y = 0.0f;
    tF32 x = 0.0f;

#define NewLine(H) x = 0.0f; y += H;

    tF32 hueH = _kfRectHRatioHeight*mfBaseHeight;
    mrectH = sRectf(x,y,clW,hueH);
    NewLine(mrectH.GetHeight()+_kfControlsVSpacing);

    tF32 svH = _kfRectSVRatioHeight*mfBaseHeight;
    mrectSV = sRectf(x,y,(clW*_kfRectSVRatioSize)-_kfControlsHSpacing,svH);
    x += mrectSV.GetWidth()+_kfControlsHSpacing;
    mrectA = sRectf(x,y,(clW*_kfRectARatioSize)-_kfControlsHSpacing,svH);
    x += mrectA.GetWidth()+_kfControlsHSpacing;
    mrectRGB = sRectf(x,y,(clW*_kfRectRGBRatioSize),svH);
    NewLine(mrectSV.GetHeight()+_kfControlsVSpacing);

    if (niFlagIs(mpWidget->GetStyle(),eWidgetColorPickerStyle_Brightness)) {
      tF32 bH = _kfRectBRatioHeight*mfBaseHeight;
      mrectB = sRectf(x,y,clW,bH);
      NewLine(mrectB.GetHeight() + _kfControlsVSpacing);
    }

#undef NewLine

    // Clamp the width and heights of the controls
#define ClampRect(R)                                                \
    (R).SetRight(ni::Min(clientRect.GetRight(),(R).GetRight()));    \
    (R).SetBottom(ni::Min(clientRect.GetBottom(),(R).GetBottom()));

    ClampRect(mrectH);
    ClampRect(mrectSV);
    ClampRect(mrectA);
    ClampRect(mrectRGB);
    ClampRect(mrectB);

#undef ClampRect
  }

  ///////////////////////////////////////////////
  void _NotifyColorChanged(tU32 anColorPickerState) {
    OnSetText(eFalse);
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetColorPickerCmd_ColorChanged,anColorPickerState);
  }

  ///////////////////////////////////////////////
  void __stdcall _SetHSV(const sColor3f& avHSV, tU32 anColorPickerState) {
    mHSVA.x = avHSV.x;
    mHSVA.y = avHSV.y;
    mHSVA.z = avHSV.z;
    //    sColor3f hsv;
    //    hsv.x = avHSV.x;
    //    hsv.y = avHSV.y; //ni::Clamp(avHSV.y,niEpsilon5,1.0f-niEpsilon5);
    //    hsv.z = ni::Clamp(avHSV.z,niEpsilon3,1.0f);
    //    ((sColor3f&)mRGBA) = (sColor3f&)ColorConvert_HSV_RGB(Vec4(hsv));
    _NotifyColorChanged(anColorPickerState);
  }
  virtual void __stdcall SetHSV(const sColor3f& avHSV) {
    _SetHSV(avHSV,0);
  }
  virtual sColor3f __stdcall GetHSV() const {
    return (sColor3f&)mHSVA;
    //    return Vec3(ColorConvert_RGB_HSV(mRGBA));
  }

  ///////////////////////////////////////////////
  void __stdcall _SetRGBA(const sColor4f& avRGBA, tU32 anColorPickerState) {
    sColor4f colRGBA;
    ni::VecMaximize((sColor4f&)colRGBA, (sColor4f&)sColor4f::Zero(),
                    ni::VecMinimize((sColor4f&)colRGBA,(sColor4f&)sColor4f::One(),avRGBA));
    sColor4f colHSV = ColorConvert_RGB_HSV(colRGBA);
    mHSVA.x = colHSV.x;
    mHSVA.y = colHSV.y;
    mHSVA.z = colHSV.z;
    mHSVA.w = colRGBA.w;
    _NotifyColorChanged(anColorPickerState);
  }
  void __stdcall SetRGBA(const sColor4f& avRGBA) {
    _SetRGBA(avRGBA,0);
  }
  virtual sColor4f __stdcall GetRGBA() const {
    sColor4f colRGBA;
    colRGBA = ColorConvert_HSV_RGB(mHSVA);
    colRGBA.w = mHSVA.w;
    return colRGBA;
  }

  ///////////////////////////////////////////////
  void __stdcall _SetAlpha(tF32 afAlpha, tU32 anColorPickerState) {
    mHSVA.w = afAlpha;
    _NotifyColorChanged(anColorPickerState);
  }
  virtual void __stdcall SetAlpha(tF32 afAlpha) {
    _SetAlpha(afAlpha, 0);
  }
  virtual tF32 __stdcall GetAlpha() const {
    return mHSVA.w;
  }

  ///////////////////////////////////////////////
  void __stdcall _SetRGB(const sColor3f& avRGB, tU32 anColorPickerState) {
    sColor4f colRGBA = {avRGB.x,avRGB.y,avRGB.z,mHSVA.w};
    _SetRGBA(colRGBA,anColorPickerState);
  }
  virtual void __stdcall SetRGB(const sColor3f& avRGB) {
    _SetRGB(avRGB,0);
  }
  virtual sColor3f __stdcall GetRGB() const {
    return Vec3<tF32>(GetRGBA().ptr());
  }

  ///////////////////////////////////////////////
  virtual void __stdcall _SetBrightness(const tF32 afBrightness, tU32 anColorPickerState) {
    mfBrightness = ni::Min(afBrightness,mfMaxBrightness);
    _NotifyColorChanged(anColorPickerState);
  }
  virtual void __stdcall SetBrightness(const tF32 afBrightness) {
    _SetBrightness(afBrightness, 0);
  }
  virtual tF32 __stdcall GetBrightness() const {
    return mfBrightness;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetMaxBrightness(const tF32 afMaxBrightness) {
    mfMaxBrightness = ni::Max(1.0f,afMaxBrightness);
    SetBrightness(mfBrightness);
  }
  virtual tF32 __stdcall GetMaxBrightness() const {
    return mfMaxBrightness;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetColorName(iHString* ahspName) {
    sColor4f colRGBA = mpWidget->GetGraphics()->GetColor4FromName(ahspName);
    SetRGBA(colRGBA);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual const achar* __stdcall GetColorName() const {
    return mpWidget->GetGraphics()->FindColorName(GetRGBA());
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetExpression(iHString* ahspExpr) {
    if (HStringIsEmpty(ahspExpr)) return eFalse;
    mhspExpr = ahspExpr;
    Ptr<iExpressionVariable> var = ni::GetLang()->Eval(niHStr(ahspExpr));
    if (var.IsOK()) {
      sVec4f v;
      switch (var->GetType()) {
        case eExpressionVariableType_Vec3:
          {
            v = Vec4(var->GetVec3());
            SetRGB(var->GetVec3());
            break;
          }
        default: // vec4, etc...
          {
            v = var->GetVec4();
            SetRGBA(v);
            break;
          }
      }
      return eTrue;
    }
    else {
      return eFalse;
    }
  }
  virtual iHString* __stdcall GetExpression() const {
    sColor4f colRGBA = GetRGBA();
    niThis(cWidgetColorPicker)->mhspExpr = _H(niFmt(_A(":RGBA(%g,%g,%g,%g)"),
                                                     ni::ClampZeroOne(colRGBA.x),
                                                     ni::ClampZeroOne(colRGBA.y),
                                                     ni::ClampZeroOne(colRGBA.z),
                                                     ni::ClampZeroOne(colRGBA.w)));
    return mhspExpr;
  }

 private:
  tF32        mfBaseHeight;

  // color
  //  sVec4f     mRGBA;
  sVec4f           mHSVA;
  tF32        mfBrightness;
  tF32        mfMaxBrightness;

  // rgba color
  sRectf     mrectRGB;
  sRectf     mrectA;

  // hue
  sRectf     mrectH;
  Ptr<iTexture> mptrTextureHue;

  // saturation/brightness
  sRectf     mrectSV;

  // brightness slider
  sRectf     mrectB;

  // skin
  struct sSkin {
    Ptr<iOverlay> frame;
    Ptr<iOverlay> curCross;
  } skin;

  // dragging
  tU32 mnDragging;

  // math expression
  tHStringPtr mhspExpr;

  niEndClass(cWidgetColorPicker);
};

ni::iWidgetSink* __stdcall New_WidgetColorPicker(iWidget* apWidget) {
  return niNew cWidgetColorPicker(apWidget);
}
#endif
