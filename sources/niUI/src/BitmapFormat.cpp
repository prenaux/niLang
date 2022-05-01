// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include <niUI/IBitmapBase.h>

static Ptr<iPixelFormat> _defaultPxf;

class cBitmapFormat : public cIUnknownImpl<iBitmapFormat>
{
  niBeginClass(cBitmapFormat);
 public:
  cBitmapFormat(iGraphics* apGraphics) {
    if (!_defaultPxf.IsOK() && apGraphics) {
      _defaultPxf = apGraphics->CreatePixelFormat(_A("R8G8B8A8"));
    }
    Reset();
  }
  ~cBitmapFormat() {
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Reset() {
    mType = eBitmapType_2D;
    mnWidth = mnHeight = 16;
    mnDepth = 0;
    mnNumMipMaps = 0;
    mptrPxf = _defaultPxf;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Copy(const iBitmapFormat* apFormat) {
    SetType(apFormat->GetType());
    SetWidth(apFormat->GetWidth());
    SetHeight(apFormat->GetHeight());
    SetDepth(apFormat->GetDepth());
    SetNumMipMaps(apFormat->GetNumMipMaps());
    SetPixelFormat(niIsOK(apFormat->GetPixelFormat())?
                   apFormat->GetPixelFormat()->Clone():
                   _defaultPxf.ptr());
    return eTrue;
  }
  virtual iBitmapFormat* __stdcall Clone() const {
    cBitmapFormat* pNewFmt = niNew cBitmapFormat(NULL);
    pNewFmt->mType = this->mType;
    pNewFmt->mnWidth = this->mnWidth;
    pNewFmt->mnHeight = this->mnHeight;
    pNewFmt->mnDepth = this->mnDepth;
    pNewFmt->mnNumMipMaps = this->mnNumMipMaps;
    pNewFmt->mptrPxf = this->mptrPxf;
    return pNewFmt;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetType(eBitmapType aV) {
    mType = aV;
  }
  virtual eBitmapType __stdcall GetType() const {
    return mType;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetWidth(tU32 aV) {
    mnWidth = aV;
  }
  virtual tU32 __stdcall GetWidth() const {
    return mnWidth;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetHeight(tU32 aV) {
    mnHeight = aV;
  }
  virtual tU32 __stdcall GetHeight() const {
    return mnHeight;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDepth(tU32 aV) {
    mnDepth = aV;
  }
  virtual tU32 __stdcall GetDepth() const {
    return mnDepth;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetNumMipMaps(tU32 aV) {
    mnNumMipMaps = aV;
  }
  virtual tU32 __stdcall GetNumMipMaps() const {
    return mnNumMipMaps;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetPixelFormat(iPixelFormat* aV) {
    mptrPxf = niGetIfOK(aV);
    if (!mptrPxf.IsOK())
      mptrPxf = _defaultPxf;
  }
  virtual iPixelFormat* __stdcall GetPixelFormat() const {
    return mptrPxf;
  }

 private:
  eBitmapType mType;
  tU32 mnWidth, mnHeight, mnDepth;
  tU32 mnNumMipMaps;
  Ptr<iPixelFormat> mptrPxf;

  niEndClass(cBitmapFormat);
};

iBitmapFormat* __stdcall New_BitmapFormat(iGraphics* apGraphics) {
  return niNew cBitmapFormat(apGraphics);
}
