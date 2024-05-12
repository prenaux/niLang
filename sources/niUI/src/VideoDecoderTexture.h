#ifndef __VIDEODECODERTEXTURE_56899112_H__
#define __VIDEODECODERTEXTURE_56899112_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niUI/IVideoDecoder.h"

struct sVideoDecoderTexture : public ImplAggregate<iTexture,
                                                   eImplFlags_DontInherit1,
                                                   ni::iDeviceResource>
{
  iGraphics*  mpGraphics;
  Ptr<iTexture> mptrTexture;
  Ptr<iHString> mhspName;

  iVideoDecoder* _GetDecoder() {
    return (iVideoDecoder*)mprotected_pAggregateParent;
  }

  sVideoDecoderTexture(iGraphics* apGraphics, iHString* ahspName, iVideoDecoder* apDecoder, iTexture* apTexture) {
    mpGraphics = apGraphics;
    mhspName = ahspName;
    mptrTexture = apTexture;
    mprotected_pAggregateParent = apDecoder;
    apGraphics->GetTextureDeviceResourceManager()->Register(this);
  }
  ~sVideoDecoderTexture() {
    Invalidate();
  }

  void __stdcall Invalidate() {
    if (mpGraphics) {
      mpGraphics->GetTextureDeviceResourceManager()->Unregister(this);
      mhspName = NULL;
      mpGraphics = NULL;
    }
  }

  iUnknown* __stdcall QueryInterface(const tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(iVideoDecoder))
      return _GetDecoder();
    if (aIID == niGetInterfaceUUID(iGLTexture))
      return mptrTexture.IsOK() ? mptrTexture->QueryInterface(aIID) : mptrTexture.ptr();
    return BaseImpl::QueryInterface(aIID);
  }
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const {
    apLst->Add(niGetInterfaceUUID(iVideoDecoder));
    BaseImpl::ListInterfaces(apLst,anFlags);
  }

  virtual iHString* __stdcall GetDeviceResourceName() const {
    return mhspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eTrue;
  }
  virtual eBitmapType __stdcall GetType() const {
    return eBitmapType_2D;
  }
  virtual tU32 __stdcall GetWidth() const {
    return mptrTexture.IsOK() ? mptrTexture->GetWidth() : 256;
  }
  virtual tU32 __stdcall GetHeight() const {
    return mptrTexture.IsOK() ? mptrTexture->GetHeight() : 256;
  }
  virtual tU32 __stdcall GetDepth() const {
    return mptrTexture.IsOK() ? mptrTexture->GetDepth() : 0;
  }
  virtual iPixelFormat* __stdcall GetPixelFormat() const {
    return mptrTexture.IsOK() ? mptrTexture->GetPixelFormat() : NULL;
  }
  virtual tU32 __stdcall GetNumMipMaps() const {
    return mptrTexture.IsOK() ? mptrTexture->GetNumMipMaps() : 0;
  }
  virtual tTextureFlags __stdcall GetFlags() const {
    return eTextureFlags_Virtual;
  }
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    return NULL;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) {
    if (!mpGraphics || !mptrTexture.IsOK())
      return NULL;
    ni::iVideoDecoder* d = _GetDecoder();
    d->Update(ni::eTrue,d->GetUpdateOnBind() ? ni::GetLang()->GetFrameTime() : 0);
    return mptrTexture->Bind(apDevice);
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __VIDEODECODERTEXTURE_56899112_H__
