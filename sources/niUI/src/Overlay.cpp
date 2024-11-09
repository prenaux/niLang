// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "ClipRect.h"
#include <niLang/Math/MathMatrix.h>

#define SS_NOT_FILTERED eCompiledStates_SS_PointClamp
#define SS_FILTERED     eCompiledStates_SS_SmoothClamp

static Ptr<iMaterial> _CreateOverlayMaterial(iGraphics* apGraphics, iTexture* apTexture) {
  Ptr<iMaterial> ptrMaterial = apGraphics->CreateMaterial();
  ptrMaterial->SetRasterizerStates(eCompiledStates_RS_NoCullingFilled);
  ptrMaterial->SetDepthStencilStates(eCompiledStates_DS_NoDepthTest);
  ptrMaterial->SetFlags(ptrMaterial->GetFlags()|eMaterialFlags_NoLighting|eMaterialFlags_DoubleSided|eMaterialFlags_Vertex);
  ptrMaterial->SetChannelTexture(eMaterialChannel_Base,apTexture);
  ptrMaterial->SetChannelSamplerStates(eMaterialChannel_Base,SS_NOT_FILTERED);
  return ptrMaterial;
}

//////////////////////////////////////////////////////////////////////////////////////////////
class cOverlay : public ImplRC<iOverlay>
{
 public:
  cOverlay(iMaterial* apMaterial);
  ~cOverlay();

  tBool __stdcall IsOK() const;

  iOverlay* __stdcall Clone() const;

  __forceinline void _ApplyStatesToMaterial();
  iMaterial* __stdcall GetMaterial() const;

  iImage* __stdcall GetImage() const {
    return mptrImage;
  }

  sVec2f __stdcall GetBaseSize() const;

  void __stdcall SetPosition(sVec2f avPos);
  sVec2f __stdcall GetPosition() const;

  void __stdcall SetPivot(const sVec2f& avPivot) {
    mvPivot = avPivot;
  }
  sVec2f __stdcall GetPivot() const {
    return mvPivot;
  }

  void __stdcall SetSize(sVec2f avSize);
  sVec2f __stdcall GetSize() const;

  void __stdcall SetBlendMode(eBlendMode aVal);
  eBlendMode __stdcall GetBlendMode() const;

  void __stdcall SetFiltering(tBool abEnabled);
  tBool __stdcall GetFiltering() const;

  void __stdcall SetColor(const sColor4f& aColor);
  sColor4f __stdcall GetColor() const;
  void __stdcall SetCornerColor(eRectCorners aCorner, const sColor4f& aColor);
  sColor4f __stdcall GetCornerColor(eRectCorners aCorner) const;

  void __stdcall SetMapping(const sRectf& aRect);
  sRectf __stdcall GetMapping() const;

  void __stdcall SetFrame(const sVec4f& aFrameBorder);
  sVec4f __stdcall GetFrame() const;
  tBool __stdcall GetIsFrame() const;
  sRectf __stdcall ComputeFrameCenter(const sRectf& aDest) const;

  tBool DoDraw(iCanvas* apCanvas, const sVec2f& aPos, const sVec2f& aSize, tBool abFrame, tRectFrameFlags aFrame);
  tBool __stdcall Draw(iCanvas* apCanvas, const sVec2f& aPos, const sVec2f& aSize);
  tBool __stdcall DrawFrame(iCanvas* apCanvas, tRectFrameFlags aFrame, const sVec2f& aPos, const sVec2f& aSize);

  tF32 GetWidth() const;
  tF32 GetHeight() const;

 public:
  Ptr<iMaterial> mptrMaterial;
  Ptr<iImage>    mptrImage;
  sVec2f         mvSize;
  sVec2f         mvPivot;
  tU32           mcolTopLeft;
  tU32           mcolTopRight;
  tU32           mcolBottomRight;
  tU32           mcolBottomLeft;
  sRectf         mrectMapping;
  sVec4f         mvFrame;
};

///////////////////////////////////////////////
cOverlay::cOverlay(iMaterial* apMaterial)
{
  mcolTopLeft =
      mcolTopRight =
      mcolBottomRight =
      mcolBottomLeft = 0xFFFFFFFF;
  mrectMapping = sRectf(0,0,1,1);
  mvFrame = sVec4f::Zero();
  mvPivot = sVec2f::Zero();
  mvSize = Vec2<tF32>(0,0);
  mptrMaterial = apMaterial;
  mvSize = GetBaseSize();
}

///////////////////////////////////////////////
cOverlay::~cOverlay()
{
  mptrMaterial = NULL;
}

///////////////////////////////////////////////
iOverlay* __stdcall cOverlay::Clone() const {
  niCheck(IsOK(),NULL);
  Ptr<iMaterial> ptrNewMaterial = mptrMaterial->Clone();
  Ptr<cOverlay> ptrNewOverlay = niNew cOverlay(ptrNewMaterial);
  *ptrNewOverlay = *this;
  ptrNewOverlay->mptrMaterial = ptrNewMaterial;
  return ptrNewOverlay.GetRawAndSetNull();
}

///////////////////////////////////////////////
tBool cOverlay::IsOK() const {
  return niIsOK(mptrMaterial);
}

///////////////////////////////////////////////
iMaterial* __stdcall cOverlay::GetMaterial() const {
  niThis(cOverlay)->_ApplyStatesToMaterial();
  return mptrMaterial;
}

void cOverlay::_ApplyStatesToMaterial() {
  if (mptrImage.IsOK()) {
    niThis(cOverlay)->mptrMaterial->SetChannelTexture(
        eMaterialChannel_Base,mptrImage->GrabTexture(eImageUsage_Source,sRecti::Null()));
  }
}

///////////////////////////////////////////////
sVec2f __stdcall cOverlay::GetBaseSize() const
{
  return Vec2<tF32>(GetWidth(),GetHeight());
}

///////////////////////////////////////////////
void __stdcall cOverlay::SetBlendMode(eBlendMode aVal) {
  mptrMaterial->SetBlendMode(aVal);
}
eBlendMode __stdcall cOverlay::GetBlendMode() const {
  return mptrMaterial->GetBlendMode();
}

///////////////////////////////////////////////
void cOverlay::SetSize(sVec2f avSize) {
  if (avSize.x <= 0) {
    avSize.x = (tF32)GetWidth();
  }
  if (avSize.y <= 0) {
    avSize.y = (tF32)GetHeight();
  }
  mvSize.Set(avSize.x, avSize.y);
}
sVec2f cOverlay::GetSize() const {
  return mvSize;
}

///////////////////////////////////////////////
void __stdcall cOverlay::SetFiltering(tBool abEnabled) {
  mptrMaterial->SetChannelSamplerStates(
      eMaterialChannel_Base,
      abEnabled ? SS_FILTERED : SS_NOT_FILTERED);
}
tBool __stdcall cOverlay::GetFiltering() const
{
  return mptrMaterial->GetChannelSamplerStates(eMaterialChannel_Base) == SS_FILTERED;
}

///////////////////////////////////////////////
void cOverlay::SetColor(const sColor4f& col) {
  SetCornerColor(eRectCorners_All,col);
}
sColor4f cOverlay::GetColor() const {
  return ULColorToVec4f(mcolTopLeft);
}

///////////////////////////////////////////////
void cOverlay::SetCornerColor(eRectCorners aCorners, const sColor4f& col)
{
  const tU32 nColor = ULColorBuild(col);
  if (niFlagTest(aCorners,eRectCorners_TopLeft)) {
    mcolTopLeft = nColor;
  }
  if (niFlagTest(aCorners,eRectCorners_TopRight)) {
    mcolTopRight = nColor;
  }
  if (niFlagTest(aCorners,eRectCorners_BottomRight)) {
    mcolBottomRight = nColor;
  }
  if (niFlagTest(aCorners,eRectCorners_BottomLeft)) {
    mcolBottomLeft = nColor;
  }
}
sColor4f cOverlay::GetCornerColor(eRectCorners aCorners) const
{
  if (niFlagTest(aCorners,eRectCorners_TopRight)) {
    return ULColorToVec4f(mcolTopRight);
  }
  if (niFlagTest(aCorners,eRectCorners_BottomRight)) {
    return ULColorToVec4f(mcolBottomRight);
  }
  if (niFlagTest(aCorners,eRectCorners_BottomLeft)) {
    return ULColorToVec4f(mcolBottomLeft);
  }
  return ULColorToVec4f(mcolTopLeft);
}

///////////////////////////////////////////////
void __stdcall cOverlay::SetMapping(const sRectf& aRect) {
  mrectMapping = aRect;
}
sRectf __stdcall cOverlay::GetMapping() const {
  return mrectMapping;
}

///////////////////////////////////////////////
void __stdcall cOverlay::SetFrame(const sVec4f& aFrameBorder) {
  mvFrame = aFrameBorder;
}
sVec4f __stdcall cOverlay::GetFrame() const {
  return mvFrame;
}
tBool __stdcall cOverlay::GetIsFrame() const {
  return mvFrame != sVec4f::Zero();
}
sRectf __stdcall cOverlay::ComputeFrameCenter(const sRectf& aDest) const {
  return aDest.ComputeFrameCenter(mvFrame);
}

///////////////////////////////////////////////
tF32 cOverlay::GetWidth() const
{
  if (mptrImage.IsOK()) {
    return (tF32)mptrImage->GetWidth();
  }
  else {
    return (tF32)mptrMaterial->GetWidth();
  }
}
tF32 cOverlay::GetHeight() const
{
  if (mptrImage.IsOK()) {
    return (tF32)mptrImage->GetHeight();
  }
  else {
    return (tF32)mptrMaterial->GetHeight();
  }
}

///////////////////////////////////////////////
tBool cOverlay::DoDraw(iCanvas* apCanvas, const sVec2f& aPos, const sVec2f& aSize, tBool abFrame, tRectFrameFlags aFrame)
{
  if (!this->IsOK()) return eFalse;
  if (!niIsOK(apCanvas)) return eFalse;

  const sVec2f pos = aPos-mvPivot;
  const sVec2f vSize = (aSize == sVec2f::Zero()) ? mvSize : aSize;

  sVec2f
      vTL = mrectMapping.GetTopLeft(),
      vBR = mrectMapping.GetBottomRight();
  if (vTL.x > 1.0f) vTL.x /= GetWidth();
  if (vTL.y > 1.0f) vTL.y /= GetHeight();
  if (vBR.x > 1.0f) vBR.x /= GetWidth();
  if (vBR.y > 1.0f) vBR.y /= GetHeight();

  _ApplyStatesToMaterial();
  apCanvas->SetMaterial(mptrMaterial);
  if (!abFrame) {
    apCanvas->RectTA2(pos,pos+vSize,vTL,vBR,0,
                      mcolTopLeft,
                      mcolTopRight,
                      mcolBottomRight,
                      mcolBottomLeft);
  }
  else {
    apCanvas->FrameTA2(aFrame,mvFrame,pos,pos+vSize,vTL,vBR,0,
                       mcolTopLeft,
                       mcolTopRight,
                       mcolBottomRight,
                       mcolBottomLeft);
  }

  return eTrue;
}

tBool __stdcall cOverlay::Draw(iCanvas* apCanvas, const sVec2f& aPos, const sVec2f& aSize) {
  return DoDraw(apCanvas,aPos,aSize,eFalse,0);
}
tBool __stdcall cOverlay::DrawFrame(iCanvas* apCanvas, tRectFrameFlags aFrame, const sVec2f& aPos, const sVec2f& aSize) {
  return DoDraw(apCanvas,aPos,aSize,eTrue,aFrame);
}

///////////////////////////////////////////////
iOverlay* __stdcall cGraphics::CreateOverlayResource(iHString* ahspRes) {
  Ptr<iTexture> ptrTex = this->CreateTextureFromRes(ahspRes,NULL,eTextureFlags_Overlay);
  niCheck(ptrTex.IsOK(),NULL);
  Ptr<cOverlay> ptrOvr = niNew cOverlay(_CreateOverlayMaterial(this,ptrTex));
  niCheck(ptrOvr.IsOK(),NULL);
  return ptrOvr.GetRawAndSetNull();
}

iOverlay* __stdcall cGraphics::CreateOverlayTexture(iTexture* apTexture) {
  niCheckIsOK(apTexture,NULL);
  Ptr<cOverlay> ptrOvr = niNew cOverlay(_CreateOverlayMaterial(this,apTexture));
  niCheck(ptrOvr.IsOK(),NULL);
  return ptrOvr.GetRawAndSetNull();
}

iOverlay* __stdcall cGraphics::CreateOverlayColor(const sColor4f& aColor) {
  Ptr<cOverlay> ptrOvr = niNew cOverlay(_CreateOverlayMaterial(this,NULL));
  niCheck(ptrOvr.IsOK(),NULL);
  ptrOvr->SetColor(aColor);
  return ptrOvr.GetRawAndSetNull();
}

iOverlay* __stdcall cGraphics::CreateOverlayImage(iImage* apImage) {
  niCheckIsOK(apImage,NULL);
  Ptr<cOverlay> ptrOvr = niNew cOverlay(_CreateOverlayMaterial(this,apImage->GrabTexture(eImageUsage_Source,sRecti::Null())));
  niCheck(ptrOvr.IsOK(),NULL);
  ptrOvr->mptrImage = apImage;
  return ptrOvr.GetRawAndSetNull();
}

iOverlay* __stdcall cGraphics::CreateOverlayMaterial(iMaterial* apMaterial) {
  Ptr<cOverlay> ptrOvr = niNew cOverlay(apMaterial);
  niCheck(ptrOvr.IsOK(),NULL);
  return ptrOvr.GetRawAndSetNull();
}
