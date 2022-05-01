// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#if niMinFeatures(20)

#include "Graphics.h"
#include "AGG.h"
#include "VG.h"

#define SET(PARAM)      //top().mSetFlags |= eVGStyleSetFlags_##PARAM
#define GET(PARAM,V)    (top().V)
#define GETEX(PARAM,T,V)  ((T)top().V)

class cVGStyle : public cIUnknownImpl<iVGStyle>
{
 public:
  cVGStyle() {
    mStack.push(sStyle());
  }
  ~cVGStyle() {
  }

  tBool __stdcall Copy(const iVGStyle* apStyle) {
    top().mbAA = apStyle->GetAntiAliasing();
    top().mbStroke = apStyle->GetStroke();
    top().mStrokePaint = apStyle->GetStrokePaint()->Clone();
    top().mbStrokeTransformed = apStyle->GetStrokeTransformed();
    top().mbFill = apStyle->GetFill();
    top().mbFillEvenOdd = apStyle->GetFillEvenOdd();
    top().mfFillExpand = apStyle->GetFillExpand();
    top().mFillPaint = apStyle->GetFillPaint()->Clone();
    top().mfStrokeWidth = apStyle->GetStrokeWidth();
    top().mLineCap = apStyle->GetLineCap();
    top().mLineJoin = apStyle->GetLineJoin();
    top().mInnerJoin = apStyle->GetInnerJoin();
    top().mfMiterLimit = apStyle->GetMiterLimit();
    top().mfDashStart = apStyle->GetDashStart();
    ClearDashes();
    for (tU32 i = 0; i < apStyle->GetNumDashes(); ++i) {
      AddDash(apStyle->GetDash(i));
    }
    top().mfOpacity = apStyle->GetOpacity();
    top().mfRasterizerApproximationScale = apStyle->GetRasterizerApproximationScale();
    top().mfTesselatorApproximationScale = apStyle->GetTesselatorApproximationScale();
    top().mfSmooth = apStyle->GetSmooth();
    top().mCurrentColor = apStyle->GetCurrentColor();
    return eTrue;
  }

  iVGStyle* __stdcall Clone() const {
    cVGStyle* pStyle = niNew cVGStyle();
    pStyle->Copy(this);
    return pStyle;
  }

  tBool __stdcall Push() {
    mStack.push(mStack.top());
    SetFillPaint(GetFillPaint()->Clone());
    SetStrokePaint(GetStrokePaint()->Clone());
    return eTrue;
  }

  tBool __stdcall Pop() {
    if (mStack.size() == 1)
      return eFalse;
    mStack.pop();
    return eTrue;
  }

  void __stdcall SetDefault() {
    top() = sStyle();
  }

  void __stdcall SetAntiAliasing(tBool abAntiAliasing) {
    SET(AntiAliasing);
    top().mbAA = abAntiAliasing;
  }
  tBool __stdcall GetAntiAliasing() const {
    return GET(AntiAliasing,mbAA);
  }

  void __stdcall SetStroke(tBool abStroke) {
    SET(Stroke);
    top().mbStroke = abStroke;
  }
  tBool __stdcall GetStroke() const {
    return GET(Stroke,mbStroke);
  }

  virtual void __stdcall SetStrokeTransformed(tBool abStrokeTransformed) {
    SET(StrokeTransformed);
    top().mbStrokeTransformed = abStrokeTransformed;
  }
  virtual tBool __stdcall GetStrokeTransformed() const {
    return top().mbStrokeTransformed;
  }

  tBool __stdcall SetStrokePaint(iVGPaint* apPaint) {
    SET(StrokePaint);
    tBool bRet = eTrue;
    top().mStrokePaint = apPaint;
    if (!top().mStrokePaint.IsOK())  {
      top().mStrokePaint = CreateVGPaintSolid(sColor4f::Black());
      bRet = eFalse;
    }
    return bRet;
  }
  iVGPaint* __stdcall GetStrokePaint() const {
    return GET(StrokePaint,mStrokePaint);
  }

  void __stdcall SetStrokeColor(const sColor3f& avColor) {
    SET(StrokeColor);
    top().mStrokePaint->SetColor(Vec4f(avColor.x,avColor.y,avColor.z,GetStrokeOpacity()));
  }
  sColor3f __stdcall GetStrokeColor() const {
    return GETEX(StrokeColor,sColor3f&,mStrokePaint->GetColor());
  }

  virtual void __stdcall SetStrokeOpacity(tF32 afOpacity) {
    SET(StrokeOpacity);
    sColor4f col = top().mStrokePaint->GetColor();
    col.w = afOpacity;
    top().mStrokePaint->SetColor(col);
  }

  virtual tF32 __stdcall GetStrokeOpacity() const {
    return GET(StrokeOpacity,mStrokePaint->GetColor().w);
  }

  void __stdcall SetStrokeColor4(const sColor4f& avColor) {
    SetStrokeColor((const sColor3f&)avColor);
    SetStrokeOpacity(avColor.w);
  }
  sColor4f __stdcall GetStrokeColor4() const {
    sColor4f col;
    niUnsafeCast(sColor3f&,col) = GetStrokeColor();
    col.w = GetStrokeOpacity();
    return col;
  }

  void __stdcall SetFill(tBool abFill) {
    SET(Fill);
    top().mbFill = abFill;
  }
  tBool __stdcall GetFill() const {
    return GET(Fill,mbFill);
  }

  void __stdcall SetFillEvenOdd(tBool abFillEvenOdd) {
    SET(FillEvenOdd);
    top().mbFillEvenOdd = abFillEvenOdd;
  }
  tBool __stdcall GetFillEvenOdd() const {
    return GET(FillEvenOdd,mbFillEvenOdd);
  }

  virtual void __stdcall SetFillExpand(tF32 afExpand) {
    SET(FillExpand);
    top().mfFillExpand = afExpand;
  }
  virtual tF32 __stdcall GetFillExpand() const {
    return GET(FillExpand,mfFillExpand);
  }

  tBool __stdcall SetFillPaint(iVGPaint* apPaint) {
    SET(FillPaint);
    tBool bRet = eTrue;
    top().mFillPaint = apPaint;
    if (!top().mFillPaint.IsOK())  {
      top().mFillPaint = CreateVGPaintSolid(sColor4f::White());
      bRet = eFalse;
    }
    return bRet;
  }
  iVGPaint* __stdcall GetFillPaint() const {
    return GET(FillPaint,mFillPaint);
  }

  void __stdcall SetFillColor(const sColor3f& avColor) {
    SET(FillColor);
    top().mFillPaint->SetColor(Vec4f(avColor.x,avColor.y,avColor.z,GetFillOpacity()));
  }
  sColor3f __stdcall GetFillColor() const {
    return GETEX(FillColor,sColor3f&,mFillPaint->GetColor());
  }

  virtual void __stdcall SetFillOpacity(tF32 afOpacity) {
    SET(FillOpacity);
    sColor4f col = top().mFillPaint->GetColor();
    col.w = afOpacity;
    top().mFillPaint->SetColor(col);
  }
  virtual tF32 __stdcall GetFillOpacity() const {
    return GET(FillOpacity,mFillPaint->GetColor().w);
  }

  void __stdcall SetFillColor4(const sColor4f& avColor) {
    SetFillColor((const sColor3f&)avColor);
    SetFillOpacity(avColor.w);
  }
  sColor4f __stdcall GetFillColor4() const {
    sColor4f col;
    niUnsafeCast(sColor3f&,col) = GetFillColor();
    col.w = GetFillOpacity();
    return col;
  }

  void __stdcall SetOpacity(tF32 afOpacity) {
    SET(Opacity);
    top().mfOpacity = afOpacity;
  }
  tF32 __stdcall GetOpacity() const {
    return GET(Opacity,mfOpacity);
  }

  void __stdcall SetRasterizerApproximationScale(tF32 afRasterizerApproximationScale) {
    SET(RasterizerApproximationScale);
    top().mfRasterizerApproximationScale = afRasterizerApproximationScale;
  }
  tF32 __stdcall GetRasterizerApproximationScale() const {
    return GET(RasterizerApproximationScale,mfRasterizerApproximationScale);
  }

  void __stdcall SetTesselatorApproximationScale(tF32 afTesselatorApproximationScale) {
    SET(TesselatorApproximationScale);
    top().mfTesselatorApproximationScale = afTesselatorApproximationScale;
  }
  tF32 __stdcall GetTesselatorApproximationScale() const {
    return GET(TesselatorApproximationScale,mfTesselatorApproximationScale);
  }

  void __stdcall SetStrokeWidth(tF32 afWidth) {
    SET(StrokeWidth);
    top().mfStrokeWidth = afWidth;
  }
  tF32 __stdcall GetStrokeWidth() const {
    return GET(StrokeWidth,mfStrokeWidth);
  }

  void __stdcall SetLineCap(eVGLineCap aCap) {
    SET(LineCap);
    top().mLineCap = aCap;
  }
  eVGLineCap __stdcall GetLineCap() const {
    return GET(LineCap,mLineCap);
  }

  void __stdcall SetLineJoin(eVGLineJoin aJoin) {
    SET(LineJoin);
    top().mLineJoin = aJoin;
  }
  eVGLineJoin __stdcall GetLineJoin() const {
    return GET(LineJoin,mLineJoin);
  }

  void __stdcall SetInnerJoin(eVGInnerJoin aInnerJoin) {
    SET(InnerJoin);
    top().mInnerJoin = aInnerJoin;
  }
  eVGInnerJoin __stdcall GetInnerJoin() const {
    return GET(InnerJoin,mInnerJoin);
  }

  void __stdcall SetMiterLimit(tF32 afMiterLimit) {
    SET(MiterLimit);
    top().mfMiterLimit = afMiterLimit;
  }
  tF32 __stdcall GetMiterLimit() const {
    return GET(MiterLimit,mfMiterLimit);
  }

  virtual void __stdcall SetCurrentColor(const sColor4f& avColor) {
    SET(CurrentColor);
    top().mCurrentColor = avColor;
  }
  virtual const sColor4f& __stdcall GetCurrentColor() const {
    return GET(CurrentColor,mCurrentColor);
  }

  tU32 __stdcall GetNumDashes() const {
    return top().mvDashes.size();
  }
  void __stdcall ClearDashes() {
    top().mvDashes.clear();
  }
  void __stdcall AddDash(const sVec2f& aV) {
    SET(Dashes);
    // x: dash len
    // y: gap len
    top().mvDashes.push_back(aV);
  }
  tBool __stdcall RemoveDash(tU32 anIndex) {
    if (anIndex >= top().mvDashes.size()) return eFalse;
    top().mvDashes.erase(top().mvDashes.begin()+anIndex);
    return eTrue;
  }
  sVec2f __stdcall GetDash(tU32 anIndex) const {
    if (anIndex >= top().mvDashes.size())
      return sVec2f::Zero();
    return top().mvDashes[anIndex];
  }
  void __stdcall SetDashStart(tF32 afDashStart) {
    SET(DashStart);
    top().mfDashStart = afDashStart;
  }
  tF32 __stdcall GetDashStart() const {
    return GET(DashStart,mfDashStart);
  }

  void __stdcall SetSmooth(tF32 afSmooth) {
    SET(Smooth);
    top().mfSmooth = afSmooth;
  }
  tF32 __stdcall GetSmooth() const {
    return GET(Smooth,mfSmooth);
  }

 private:
  Ptr<iVGStyle>   mptrParent;
  struct sStyle {
    tBool       mbAA;
    tBool       mbStroke;
    tBool       mbStrokeTransformed;
    Ptr<iVGPaint> mStrokePaint;
    tBool       mbFill;
    tBool       mbFillEvenOdd;
    tF32        mfFillExpand;
    Ptr<iVGPaint> mFillPaint;
    tF32      mfStrokeWidth;
    eVGLineCap    mLineCap;
    eVGLineJoin   mLineJoin;
    eVGInnerJoin  mInnerJoin;
    tF32      mfMiterLimit;
    astl::vector<sVec2f> mvDashes;
    tF32      mfDashStart;
    tF32      mfOpacity;
    tF32      mfRasterizerApproximationScale;
    tF32      mfTesselatorApproximationScale;
    tF32      mfSmooth;
    sColor4f    mCurrentColor;
    sStyle() {
      mbAA = eTrue;
      mbStroke = eTrue;
      mbStrokeTransformed = eTrue;
      mStrokePaint = CreateVGPaintSolid(sColor4f::Black());
      mbFill = eTrue;
      mbFillEvenOdd = eFalse;
      mfFillExpand = 0.0f;
      mFillPaint = CreateVGPaintSolid(sColor4f::White());
      mfStrokeWidth = 1.0f;
      mLineCap = eVGLineCap_Round;
      mLineJoin = eVGLineJoin_Miter;
      mInnerJoin = eVGInnerJoin_Round;
      mfMiterLimit = 4.0f;
      mfDashStart = 0.0f;
      mfOpacity = 1.0f;
      mfRasterizerApproximationScale = 1.0f;
      mfTesselatorApproximationScale = 0.5f;
      mfSmooth = 0.0f;
      mCurrentColor = sColor4f::Black();
      mvDashes.clear();
    }
  };
  astl::stack<sStyle> mStack;

  inline sStyle& top() { return mStack.top(); }
  inline const sStyle& top() const { return mStack.top(); }
};

Ptr<iVGStyle> __stdcall cGraphics::CreateVGStyle() {
  return niNew cVGStyle();
}

///////////////////////////////////////////////
niExportFunc(iVGStyle*) CreateVGStyle() {
  return niNew cVGStyle();
}
#endif // niMinFeatures
