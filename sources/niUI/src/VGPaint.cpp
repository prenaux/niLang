// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "niLang/Types.h"
#include "stdafx.h"
#if niMinFeatures(20)

#include "Graphics.h"
#include "AGG.h"
#include "VG.h"
#include <niLang/Utils/TimedLerp.h>

typedef sTimedLerp<tF32CVec,tF32,sTimedLerp_GetKeyTimeStaticCast<tF32,tF32> > tTimedLerp;

//--------------------------------------------------------------------------------------------
//
//  Gradient Table
//
//--------------------------------------------------------------------------------------------
template<class GradientF>
class gradient_reflect_polymorphic_wrapper : public ImplRC<iAGGGradientPolymorphicWrapperBase>
{
 public:
  gradient_reflect_polymorphic_wrapper() : m_adaptor(m_gradient) {}
  virtual int calculate(int x, int y, int d) const
  {
    return m_adaptor.calculate(x, y, d);
  }
  GradientF m_gradient;
  agg::gradient_reflect_adaptor<GradientF> m_adaptor;
};
template<class GradientF>
class gradient_repeat_polymorphic_wrapper : public ImplRC<iAGGGradientPolymorphicWrapperBase>
{
 public:
  gradient_repeat_polymorphic_wrapper() : m_adaptor(m_gradient) {}
  virtual int calculate(int x, int y, int d) const
  {
    return m_adaptor.calculate(x, y, d);
  }
  GradientF m_gradient;
  agg::gradient_repeat_adaptor<GradientF> m_adaptor;
};
template<class GradientF>
class gradient_clamp_polymorphic_wrapper : public ImplRC<iAGGGradientPolymorphicWrapperBase>
{
 public:
  gradient_clamp_polymorphic_wrapper() : m_adaptor(m_gradient) {}
  virtual int calculate(int x, int y, int d) const
  {
    return m_adaptor.calculate(x, y, d);
  }
  GradientF m_gradient;
  agg::gradient_clamp_adaptor<GradientF> m_adaptor;
};

iAGGGradientPolymorphicWrapperBase* GetNewGradientWrapperBase(eVGGradientType aType, eVGWrapType aWrap) {
#define SELWRAP(TYPE)                                           \
  if (aWrap == eVGWrapType_Repeat) {                            \
    return niNew gradient_repeat_polymorphic_wrapper<TYPE>();   \
  }                                                             \
  else if (aWrap == eVGWrapType_Mirror) {                       \
    return niNew gradient_reflect_polymorphic_wrapper<TYPE>();  \
  }                                                             \
  else /*if (aWrap == eVGWrapType_Clamp)*/ {                    \
    return niNew gradient_clamp_polymorphic_wrapper<TYPE>();    \
  }

  switch (aType) {
    default:
    case eVGGradientType_Linear:
      SELWRAP(agg::gradient_x);
      break;
    case eVGGradientType_Radial:
      SELWRAP(agg::gradient_radial);
      break;
    case eVGGradientType_Cross:
      SELWRAP(agg::gradient_xy);
      break;
    case eVGGradientType_SqrtCross:
      SELWRAP(agg::gradient_sqrt_xy);
      break;
    case eVGGradientType_Conic:
      SELWRAP(agg::gradient_conic);
      break;
    case eVGGradientType_Diamond:
      SELWRAP(agg::gradient_diamond);
      break;
  }

#undef SELWRAP
}

class cVGGradientTable : public ImplRC<iVGGradientTable>
{
 public:
  cVGGradientTable(iGraphics* apGraphics, tU32 anSize)
  {
    mptrGraphics = apGraphics;
    mnTableSize = anSize;
    mTable = niTMalloc(tAGGColorType,mnTableSize);
  }
  ~cVGGradientTable() {
    niSafeFree(mTable);
  }

  tBool __stdcall Copy(const iVGGradientTable* apSrc) {
    if (!niIsOK(apSrc)) return eFalse;
    mnTableSize = apSrc->GetSize();
    niSafeFree(mTable);
    mTable = niTMalloc(tAGGColorType,mnTableSize);
    for (tU32 i = 0; i < mnTableSize; ++i) {
      SetColor(i,apSrc->GetColor(i));
    }
    return eTrue;
  }

  iVGGradientTable* __stdcall Clone() const {
    cVGGradientTable* pNew = niNew cVGGradientTable(mptrGraphics,mnTableSize);
    memcpy(pNew->mTable,mTable,sizeof(mTable[0])*mnTableSize);
    return pNew;
  }

  tU32 __stdcall GetSize() const {
    return mnTableSize;
  }

  tBool __stdcall SetColor(tU32 anIndex, const sColor4f& avColor) {
    if (anIndex >= mnTableSize) return eFalse;
    mTable[anIndex] = AGGColorFromColor4f(avColor);
    return eTrue;
  }
  sColor4f __stdcall GetColor(tU32 anIndex) const {
    if (anIndex >= mnTableSize) return sColor4f::Zero();
    return AGGColorToColor4f(mTable[anIndex]);
  }

  // Set a range of color (start and end inclusive)
  tBool __stdcall SetColorRange(tU32 anStart, tU32 anEnd, const sColor4f& avColor) {
    if (anStart >= mnTableSize) return eFalse;
    if (anEnd >= mnTableSize) anEnd = mnTableSize-1;
    tAGGColorType col = AGGColorFromColor4f(avColor);
    for (tU32 i = anStart; i <= anEnd; ++i)
      mTable[i] = col;
    return eTrue;
  }

  void __stdcall GenerateTwoColors(const sColor4f& aStartColor, const sColor4f& aEndColor)
  {
    sColor4f colorT;
    for (tU32 i = 0; i < mnTableSize; ++i) {
      tF32 f = tF32(i)/tF32(mnTableSize);
      mTable[i] = AGGColorFromColor4f(ni::VecLerp(colorT,aStartColor,aEndColor,f));
    }
  }

  tBool __stdcall GenerateStops(const tF32CVec* apOffsets, const tVec4fCVec* apColors)
  {
    niCheck(niIsOK(apOffsets),ni::eFalse);
    niCheck(niIsOK(apColors),ni::eFalse);
    niCheck(!apOffsets->IsEmpty(),ni::eFalse);
    niCheck(apOffsets->GetSize() == apColors->GetSize(),ni::eFalse);
    if (apOffsets->GetSize() == 1) {
      SetColorRange(0,mnTableSize-1,apColors->front());
    }
    else {
      sColor4f colorT;
      for (tU32 i = 0; i < mnTableSize; ++i) {
        tF32 f = tF32(i)/tF32(mnTableSize);
        tU32 a = 0, b = 0;
        tF32 t = 0.0f;
        tTimedLerp::GetIndices(*apOffsets,f,a,b,t);
        mTable[i] = AGGColorFromColor4f(ni::VecLerp(colorT,(*apColors)[a],(*apColors)[b],t));
      }
    }
    return ni::eTrue;
  }

  iVGImage* __stdcall CreateImage(eVGGradientType aType, eVGWrapType aWrapType, const iVGTransform* apTransform, tU32 anWidth, tU32 anHeight, tI32 anD1, tI32 anD2)
  {
    Ptr<iAGGGradientPolymorphicWrapperBase> ptrGrad = GetNewGradientWrapperBase(aType,aWrapType);
    if (!niIsOK(ptrGrad)) {
      niError(_A("Can't get the specified gradient type !"));
      return NULL;
    }

    Ptr<iVGImage> ptrImage = mptrGraphics->CreateVGImage(anWidth,anHeight);
    if (!niIsOK(ptrImage)) {
      niError(_A("Can't create the output image !"));
      return NULL;
    }
    VGImage_SetDebugName(ptrImage,niFmt(_A("VGPAINT_%p"),this));

    Ptr<iBitmap2D> ptrBmp = ptrImage->GrabBitmap(eVGImageUsage_Target,sRecti::Null());
    niAssert(ptrBmp.IsOK());

    agg::rendering_buffer buf(
        ptrBmp->GetData(),
        ptrBmp->GetWidth(),
        ptrBmp->GetHeight(),
        ptrBmp->GetPitch(),
        ptrBmp->GetPixelFormat()->GetBytesPerPixel()
                              );
    tAGGPixFmt          pixf(buf);
    tAGGRendererBase      rb(pixf);
    tAGGGradientSpanAlloc   span_alloc;
    agg::trans_affine trans;
    if (apTransform) {
      trans = AGGGetTransform(niConstCast(iVGTransform*,apTransform));
      trans.invert();
    }
    tAGGLinearInterpolator  inter(trans);
    tAGGGradientSpanGen   span_gen(inter, *ptrGrad, AGGGetGradientTable(this), agg_real(anD1), agg_real(anD2));
    tAGGRendererBinGradient r1(rb, span_alloc, span_gen);

    tAGGRasterizerScanline pf;
    tAGGScanline sl;

    agg::rounded_rect rect(0,0,agg_real(anWidth),agg_real(anHeight),0);
    pf.add_path(rect);
    agg::render_scanlines(pf, sl, r1);

    return ptrImage.GetRawAndSetNull();
  }

  unsigned int size() const           { return mnTableSize; }
  tAGGColorType operator [] (unsigned v) const  { return mTable[v]; }

 public:
  tU32           mnTableSize;
  Ptr<iGraphics> mptrGraphics;
  tAGGColorType* mTable;
};

sAGGGradientTable AGGGetGradientTable(const iVGGradientTable* apTable)
{
  return sAGGGradientTable(niStaticCast(const cVGGradientTable*,apTable)->mnTableSize,
                           niStaticCast(const cVGGradientTable*,apTable)->mTable);
}

//--------------------------------------------------------------------------------------------
//
//  Paint Solid
//
//--------------------------------------------------------------------------------------------
class cVGPaintSolid : public ImplRC<iVGPaint>
{
 public:
  cVGPaintSolid(const sColor4f& avColor = sColor4f::White()) {
    mColor = avColor;
  }
  ~cVGPaintSolid() {
  }

  tBool __stdcall Copy(const iVGPaint* apPaint) {
    if (apPaint->GetType() != eVGPaintType_Solid) return eFalse;
    const cVGPaintSolid* pPaint = niStaticCast(const cVGPaintSolid*,apPaint);
    SetColor(pPaint->GetColor());
    return eTrue;
  }

  iVGPaint* __stdcall Clone() const {
    cVGPaintSolid* pPaint = niNew cVGPaintSolid(sColor4f::Zero());
    pPaint->Copy(this);
    return pPaint;
  }

  eVGPaintType __stdcall GetType() const {
    return eVGPaintType_Solid;
  }

  void __stdcall SetColor(const sColor4f& acolColor) {
    mColor = acolColor;
  }
  const sColor4f& __stdcall GetColor() const {
    return mColor;
  }

 private:
  sColor4f  mColor;
};

//--------------------------------------------------------------------------------------------
//
//  Paint Image
//
//--------------------------------------------------------------------------------------------
class cVGPaintImage : public ImplRC<iVGPaintImage,eImplFlags_DontInherit1,iVGPaint>
{
 public:
  cVGPaintImage(iGraphics* apGraphics, iVGImage* apImage = NULL, const sColor4f& avColor = sColor4f::White()) {
    mptrGraphics = apGraphics;
    mColor = avColor;
    mptrImage = apImage ? Ptr<iVGImage>(apImage) : apGraphics->CreateVGImage(4,4);
    mRect = Rectf(0,0,(tF32)mptrImage->GetWidth(),(tF32)mptrImage->GetHeight());
    mUnits = eVGPaintUnits_ObjectBoundingBox;
    mSourceUnits = eVGPaintUnits_ObjectBoundingBox;
    mFilterType = eVGImageFilter_Point;
    mfFilterRadius = 2.0f;
    mbFilterNormalize = eTrue;
    mWrapType = eVGWrapType_Pad;
  }
  ~cVGPaintImage() {
  }

  tBool __stdcall Copy(const iVGPaint* apPaint) {
    if (apPaint->GetType() != eVGPaintType_Image) return eFalse;
    const cVGPaintImage* pSrc = niStaticCast(const cVGPaintImage*,apPaint);
    SetColor(pSrc->GetColor());
    SetFilterType(pSrc->GetFilterType());
    SetFilterRadius(pSrc->GetFilterRadius());
    SetFilterNormalize(pSrc->GetFilterNormalize());
    SetUnits(pSrc->GetUnits());
    SetRectangle(pSrc->GetRectangle());
    SetSource(pSrc->GetSource());
    SetSourceUnits(pSrc->GetSourceUnits());
    GetImage()->Copy(pSrc->GetImage());
    SetWrapType(pSrc->GetWrapType());
    return eTrue;
  }

  iVGPaint* __stdcall Clone() const {
    cVGPaintImage* pPaint = niNew cVGPaintImage(mptrGraphics);
    pPaint->Copy(this);
    return pPaint;
  }

  eVGPaintType __stdcall GetType() const {
    return eVGPaintType_Image;
  }

  void __stdcall SetColor(const sColor4f& acolColor) {
    mColor = acolColor;
  }
  const sColor4f& __stdcall GetColor() const {
    return mColor;
  }

  iVGImage* __stdcall GetImage() const {
    return mptrImage;
  }

  void __stdcall SetFilterType(eVGImageFilter aType) {
    mFilterType = aType;
  }
  eVGImageFilter __stdcall GetFilterType() const {
    return mFilterType;
  }

  void __stdcall SetFilterRadius(tF32 afRadius) {
    mfFilterRadius = afRadius;
  }
  tF32 __stdcall GetFilterRadius() const {
    return mfFilterRadius;
  }

  void __stdcall SetFilterNormalize(tBool abNormalize) {
    mbFilterNormalize = abNormalize;
  }
  tBool __stdcall GetFilterNormalize() const {
    return mbFilterNormalize;
  }

  void __stdcall SetUnits(eVGPaintUnits aUnits) {
    mUnits = aUnits;
  }
  eVGPaintUnits __stdcall GetUnits() const {
    return mUnits;
  }

  void __stdcall SetWrapType(eVGWrapType aWrapType) {
    mWrapType = aWrapType;
  }
  eVGWrapType __stdcall GetWrapType() const {
    return mWrapType;
  }

  void __stdcall SetSource(iUnknown* apSource) {
    mptrSource = apSource;
  }
  iUnknown* __stdcall GetSource() const {
    return mptrSource;
  }
  void __stdcall SetSourceUnits(eVGPaintUnits aUnits) {
    mSourceUnits = aUnits;
  }
  eVGPaintUnits __stdcall GetSourceUnits() const {
    return mSourceUnits;
  }

  ///////////////////////////////////////////////
  void __stdcall SetRectangle(const sRectf& aRect) {
    mRect = aRect;
  }
  sRectf __stdcall GetRectangle() const {
    return mRect;
  }

 private:
  Ptr<iGraphics> mptrGraphics;
  Ptr<iVGImage>  mptrImage;
  Ptr<iUnknown>  mptrSource;
  eVGPaintUnits  mSourceUnits;
  eVGPaintUnits  mUnits;
  sRectf         mRect;
  sColor4f       mColor;
  eVGImageFilter mFilterType;
  tF32           mfFilterRadius;
  tBool          mbFilterNormalize;
  eVGWrapType    mWrapType;
};

//--------------------------------------------------------------------------------------------
//
//  Paint Gradient
//
//--------------------------------------------------------------------------------------------
class cVGPaintGradient : public ImplRC<iVGPaintGradient,eImplFlags_DontInherit1,iVGPaint>
{
 public:
  cVGPaintGradient(iGraphics* apGraphics, const sColor4f& avColor = sColor4f::White(), iVGGradientTable* apTable = NULL) {
    mptrGraphics = apGraphics;
    mColor = avColor;
    mGradientType = eVGGradientType_Linear;
    mptrTable = apTable ? apTable : niNew cVGGradientTable(mptrGraphics,256);
    if (!apTable) {
      mptrTable->GenerateTwoColors(sColor4f::White(),sColor4f::Black());
    }
    mnD1 = 0;
    mnD2 = mptrTable->GetSize();
    mA = Vec2f(0.0f,0.0f);
    mB = Vec2f(1.0f,0.0f);
    mR = 1.0f;
    mWrapType = eVGWrapType_Repeat;
    mUnits = eVGPaintUnits_ObjectBoundingBox;
  }
  ~cVGPaintGradient() {
  }

  tBool __stdcall Copy(const iVGPaint* apPaint) {
    if (apPaint->GetType() != eVGPaintType_Gradient) return eFalse;
    const cVGPaintGradient* pSrc = niStaticCast(const cVGPaintGradient*,apPaint);
    SetGradientType(pSrc->GetGradientType());
    SetColor(pSrc->GetColor());
    GetGradientTable()->Copy(pSrc->GetGradientTable());
    SetD1(pSrc->GetD1());
    SetD2(pSrc->GetD2());
    SetA(pSrc->GetA());
    SetB(pSrc->GetB());
    SetR(pSrc->GetR());
    SetUnits(pSrc->GetUnits());
    SetWrapType(pSrc->GetWrapType());
    return eTrue;
  }

  iVGPaint* __stdcall Clone() const {
    cVGPaintGradient* pPaint = niNew cVGPaintGradient(mptrGraphics);
    pPaint->Copy(this);
    return pPaint;
  }

  eVGPaintType __stdcall GetType() const {
    return eVGPaintType_Gradient;
  }

  void __stdcall SetColor(const sColor4f& acolColor) {
    mColor = acolColor;
  }
  const sColor4f& __stdcall GetColor() const {
    return mColor;
  }

  void __stdcall SetGradientType(eVGGradientType aType) {
    mGradientType = aType;
  }
  eVGGradientType __stdcall GetGradientType() const {
    return mGradientType;
  }

  void __stdcall SetD1(tI32 anD1) {
    mnD1 = anD1;
  }
  tI32 __stdcall GetD1() const {
    return mnD1;
  }
  void __stdcall SetD2(tI32 anD2) {
    mnD2 = anD2;
  }
  tI32 __stdcall GetD2() const {
    return mnD2;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetWrapType(eVGWrapType aWrapType) {
    mWrapType = aWrapType;
  }
  virtual eVGWrapType __stdcall GetWrapType() const {
    return mWrapType;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetA(const sVec2f& aV) {
    mA = aV;
  }
  virtual sVec2f __stdcall GetA() const {
    return mA;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetB(const sVec2f& aV) {
    mB = aV;
  }
  virtual sVec2f __stdcall GetB() const {
    return mB;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetR(tF32 afV) {
    mR = afV;
  }
  virtual tF32 __stdcall GetR() const {
    return mR;
  }

  ///////////////////////////////////////////////
  void __stdcall SetUnits(eVGPaintUnits aUnits) {
    mUnits = aUnits;
  }
  eVGPaintUnits __stdcall GetUnits() const {
    return mUnits;
  }

  ///////////////////////////////////////////////
  iVGGradientTable* __stdcall GetGradientTable() const {
    return mptrTable;
  }

 private:
  eVGGradientType       mGradientType;
  eVGWrapType           mWrapType;
  Ptr<iGraphics>        mptrGraphics;
  Ptr<iVGGradientTable> mptrTable;
  sColor4f              mColor;
  tI32                  mnD1,mnD2;
  sVec2f             mA,mB;
  tF32                  mR;
  eVGPaintUnits         mUnits;
};

//--------------------------------------------------------------------------------------------
//
//  UI
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
Ptr<iVGGradientTable> __stdcall cGraphics::CreateVGGradientTable(tU32 anSize) {
  return niNew cVGGradientTable(QPtr<iGraphics>(this),anSize);
}

///////////////////////////////////////////////
Ptr<iVGGradientTable> __stdcall cGraphics::CreateVGGradientTableTwoColors(const sColor4f& acolStart, const sColor4f& acolEnd) {
  Ptr<iVGGradientTable> ptrTable = CreateVGGradientTable(64);
  ptrTable->GenerateTwoColors(acolStart,acolEnd);
  return ptrTable.GetRawAndSetNull();
}

///////////////////////////////////////////////
Ptr<iVGGradientTable> __stdcall cGraphics::CreateVGGradientTableStops(const tF32CVec* apOffsets, const tVec4fCVec* apColors) {
  Ptr<iVGGradientTable> ptrTable = CreateVGGradientTable(256);
  ptrTable->GenerateStops(apOffsets,apColors);
  return ptrTable.GetRawAndSetNull();
}

///////////////////////////////////////////////
Ptr<iVGPaint> __stdcall cGraphics::CreateVGPaint(eVGPaintType aType) {
  switch (aType) {
    case eVGPaintType_Solid:
      return niNew cVGPaintSolid();
    case eVGPaintType_Gradient:
      return niNew cVGPaintGradient(QPtr<iGraphics>(this));
    case eVGPaintType_Image:
      return niNew cVGPaintImage(QPtr<iGraphics>(this));
  }
  return NULL;
}

///////////////////////////////////////////////
Ptr<iVGPaint> __stdcall cGraphics::CreateVGPaintSolid(const sColor4f& aColor) {
  return niNew cVGPaintSolid(aColor);
}

///////////////////////////////////////////////
Ptr<iVGPaintImage> __stdcall cGraphics::CreateVGPaintImage(iVGImage* apImage, eVGImageFilter aFilter, eVGWrapType aWrap) {
  Ptr<iVGPaintImage> ptrPaint = niNew cVGPaintImage(QPtr<iGraphics>(this),apImage);
  ptrPaint->SetFilterType(aFilter);
  ptrPaint->SetWrapType(aWrap);
  return ptrPaint.GetRawAndSetNull();
}

///////////////////////////////////////////////
Ptr<iVGPaintGradient> __stdcall cGraphics::CreateVGPaintGradient(eVGGradientType aType, iVGGradientTable* apGradientTable) {
  Ptr<iVGPaintGradient> ptrPaint = niNew cVGPaintGradient(QPtr<iGraphics>(this),sColor4f::White(),apGradientTable);
  ptrPaint->SetGradientType(aType);
  return ptrPaint.GetRawAndSetNull();
}

///////////////////////////////////////////////
Ptr<iVGPaintGradient> __stdcall cGraphics::CreateVGPaintGradientTwoColors(eVGGradientType aType, const sColor4f& acolStart, const sColor4f& acolEnd) {
  Ptr<iVGPaintGradient> ptrPaint = niNew cVGPaintGradient(QPtr<iGraphics>(this));
  ptrPaint->SetGradientType(aType);
  ptrPaint->GetGradientTable()->GenerateTwoColors(acolStart,acolEnd);
  return ptrPaint.GetRawAndSetNull();
}

///////////////////////////////////////////////
Ptr<iVGPaintGradient> __stdcall cGraphics::CreateVGPaintGradientStops(eVGGradientType aType, const tF32CVec* apOffsets, const tVec4fCVec* apColors) {
  Ptr<iVGPaintGradient> ptrPaint = niNew cVGPaintGradient(QPtr<iGraphics>(this));
  ptrPaint->SetGradientType(aType);
  ptrPaint->GetGradientTable()->GenerateStops(apOffsets,apColors);
  return ptrPaint.GetRawAndSetNull();
}

///////////////////////////////////////////////
iVGPaint* CreateVGPaintSolid(const sColor4f& aColor) {
  return niNew cVGPaintSolid(aColor);
}

#endif
