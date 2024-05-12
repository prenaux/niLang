// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "Image.h"

#define IMAGEFLAGS_BMPCREATED niBit(0)
#define IMAGEFLAGS_BMPDIRTY   niBit(1)
#define IMAGEFLAGS_TEXCREATED niBit(2)
#define IMAGEFLAGS_TEXDIRTY   niBit(3)
#define IMAGEFLAGS_TEXRT      niBit(4)
#define IMAGEFLAGS_DSCREATED  niBit(5)
#define IMAGEFLAGS_RECOMPUTE_BITMAP_MIPMAPS_BEFORE_COPY_TO_TEXTURE niBit(6)

#define IMAGE_NAMED
#ifdef IMAGE_NAMED
#define IMAGE_NEWTEXNAME _H(niFmt(_A("Image_Tex_%p"),this))
#define IMAGE_NEWDSNAME  _H(niFmt(_A("Image_DS_%p"),this))
#else
#define IMAGE_NEWTEXNAME NULL
#define IMAGE_NEWDSNAME  NULL
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
class cImage : public ImplRC<iImage>
{
 public:
  cImage(iGraphics* apGraphics, tU32 anWidth, tU32 anHeight, iBitmap2D* apBmp, iTexture* apTex, iTexture* apDepthStencil) {
    _Init(apGraphics,anWidth,anHeight,apBmp,apTex,apDepthStencil);
  }

  void _Init(iGraphics* apGraphics, tU32 anWidth, tU32 anHeight, iBitmap2D* apBmp, iTexture* apTex, iTexture* apDepthStencil)
  {
    mnFlags = 0;
    mnWidth = anWidth;
    mnHeight = anHeight;
    mrectBmpDirtyRect = sRecti::Null();
    mrectTexDirtyRect = sRecti::Null();

    mptrGraphics = apGraphics;

    if (niIsOK(apBmp)) {
      mptrBitmap = apBmp;
      mnFlags |= IMAGEFLAGS_BMPCREATED|IMAGEFLAGS_BMPDIRTY;
      if (!mnWidth || !mnHeight) {
        mnWidth = apBmp->GetWidth();
        mnHeight = apBmp->GetHeight();
      }
    }

    if (niIsOK(apTex)) {
      mptrTexture = apTex;
      mnFlags |= IMAGEFLAGS_TEXCREATED|IMAGEFLAGS_TEXDIRTY;
      if (mptrTexture->GetFlags() & eTextureFlags_RenderTarget) {
        mnFlags |= IMAGEFLAGS_TEXRT;
      }
      if (!mnWidth || !mnHeight) {
        mnWidth = apTex->GetWidth();
        mnHeight = apTex->GetHeight();
      }
      else if (apTex->GetWidth() != mnWidth || apTex->GetHeight() != mnHeight) {
          niError(niFmt("Image invalid texture size, expected %dx%d but is %dx%d.",
                        mnWidth,mnHeight,apTex->GetWidth(),apTex->GetHeight()));
      }
    }

    if (niIsOK(apBmp) && niIsOK(apTex)) {
      // assume the bitmap & texture are in sync when both are specified
      mnFlags &= ~IMAGEFLAGS_BMPDIRTY;
      mnFlags &= ~IMAGEFLAGS_TEXDIRTY;
    }

    if (niIsOK(apDepthStencil)) {
      mptrDepthStencil = apDepthStencil;
      mnFlags |= IMAGEFLAGS_DSCREATED;
      if (!mnWidth || !mnHeight) {
        mnWidth = apDepthStencil->GetWidth();
        mnHeight = apDepthStencil->GetHeight();
      }
      else if (apDepthStencil->GetWidth() != mnWidth || apDepthStencil->GetHeight() != mnHeight) {
        niError(niFmt("Image invalid depth stencil size, expected %dx%d but is %dx%d.",
                      mnWidth,mnHeight,apDepthStencil->GetWidth(),apDepthStencil->GetHeight()));
      }
    }

    _AddDirtyRect(mrectTexDirtyRect,sRecti::Null(),eFalse);

    if (mptrBitmap.IsOK()) {
      mptrPxf = mptrBitmap->GetPixelFormat()->Clone();
    }
    else if (mptrTexture.IsOK()) {
      mptrPxf = mptrTexture->GetPixelFormat()->Clone();
    }
    else if (mptrDepthStencil.IsOK()) {
      mptrPxf = mptrDepthStencil->GetPixelFormat()->Clone();
    }
    else {
      mptrPxf = mptrGraphics->CreatePixelFormat(IMAGE_PIXELFORMAT);
    }
  }

  void __stdcall Invalidate() {
    if (mptrBitmap.IsOK()) {
      mptrBitmap->Invalidate();
      mptrBitmap = NULL;
    }
    if (mptrTexture.IsOK()) {
      mptrTexture->Invalidate();
      mptrTexture = NULL;
    }
    if (mptrDepthStencil.IsOK()) {
      mptrDepthStencil->Invalidate();
      mptrDepthStencil = NULL;
    }
  }

  tBool __stdcall IsOK() const niImpl {
    return eTrue;
  }

  virtual tBool __stdcall Copy(iImage* apImage) {
    niCheck(niIsOK(apImage),eFalse);
    if (apImage->GetHasBitmap() || apImage->GetHasTexture()) {
      iBitmap2D* dst = this->GrabBitmap(eImageUsage_Target,sRecti::Null());
      niAssert(niIsOK(dst));
      iBitmap2D* src = apImage->GrabBitmap(eImageUsage_Source,sRecti::Null());
      niAssert(niIsOK(src));
      dst->Blit(src,0,0,0,0,src->GetWidth(),src->GetHeight());
    }
    return eTrue;
  }

  virtual iImage* __stdcall Clone() const {
    Ptr<iImage> newImg = niNew cImage(mptrGraphics,mnWidth,mnHeight,NULL,NULL,NULL);
    Image_SetDebugName(newImg,niFmt(_A("CLONE%p"),this));
    newImg->Copy(niThis(cImage));
    return newImg.GetRawAndSetNull();
  }

  tU32 __stdcall GetWidth() const {
    return mnWidth;
  }
  tU32 __stdcall GetHeight() const {
    return mnHeight;
  }

  sVec2f __stdcall GetSize() const {
    return Vec2<tF32>((tF32)GetWidth(),(tF32)GetHeight());
  }

  ///////////////////////////////////////////////
  inline void _AddDirtyRect(
      sRecti& dirtyRect, const sRecti& aRect, const tBool abHasDirtyRect)
  {
    if (aRect.GetWidth() == 0 || aRect.GetHeight() == 0) {
      dirtyRect.Set(0,0,mnWidth,mnHeight);
    }
    else if (abHasDirtyRect) {
      dirtyRect.Left()  = ni::Min(dirtyRect.Left(),aRect.Left());
      dirtyRect.Top()   = ni::Min(dirtyRect.Top(),aRect.Top());
      dirtyRect.Right() = ni::Max(dirtyRect.Right(),aRect.Right());
      dirtyRect.Bottom()  = ni::Max(dirtyRect.Bottom(),aRect.Bottom());
    }
    else {
      dirtyRect = aRect;
    }
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetHasBitmap() const {
    return mptrBitmap.IsOK();
  }
  iBitmap2D* __stdcall GrabBitmap(eImageUsage aLock, const sRecti& aDirtyRect) {
    niCheck(aLock != eImageUsage_DepthStencil,NULL);

    // create bitmap
    if (!(mnFlags&IMAGEFLAGS_BMPCREATED)) {
      mnFlags |= IMAGEFLAGS_BMPCREATED;
      mptrBitmap = mptrGraphics->CreateBitmap2DEx(mnWidth,mnHeight,
                                                  _GetPixelFormat());
      niCheck(mptrBitmap.IsOK(),NULL);
      mptrBitmap->Clear();
    }

    if (mptrBitmap.IsOK()) {
      // copy texture to bitmap
      if ((mptrTexture.IsOK()) && (mnFlags&IMAGEFLAGS_TEXDIRTY))
      {
        if (aLock != eImageUsage_TargetDiscard) { // don't blit if we discard the current content
          if (!mptrGraphics->BlitTextureToBitmap(
                  mptrTexture,eInvalidHandle,
                  mptrBitmap,
                  mrectTexDirtyRect,
                  mrectTexDirtyRect)) {
            niWarning(_A("Couldn't copy texture to bitmap."));
          }
        }
        mrectTexDirtyRect = sRecti::Null();
        mnFlags &= ~IMAGEFLAGS_TEXDIRTY;
      }
      // add target dirty rect
      const tBool isTarget = (aLock == eImageUsage_Target || aLock == eImageUsage_TargetDiscard);
      if (isTarget) {
        _AddDirtyRect(mrectBmpDirtyRect,aDirtyRect,
                      mnFlags&IMAGEFLAGS_BMPDIRTY);
        mnFlags |= IMAGEFLAGS_BMPDIRTY;
      }
    }

    return mptrBitmap;
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetHasTexture() const {
    return mptrTexture.IsOK();
  }
  tBool __stdcall GetHasDepthStencil() const {
    return mptrDepthStencil.IsOK();
  }
  iTexture* __stdcall GrabTexture(eImageUsage aLock, const sRecti& aDirtyRect) {
    if (aLock == eImageUsage_DepthStencil) {
      if (!(mnFlags&IMAGEFLAGS_DSCREATED)) {
        if (mptrDepthStencil.IsOK()) {
          mptrDepthStencil->Invalidate();
        }
        mptrDepthStencil = mptrGraphics->CreateTexture(
            IMAGE_NEWDSNAME,eBitmapType_2D,_A("D24S8"),
            0,mnWidth,mnHeight,0,
            eTextureFlags_DepthStencil);
        mnFlags |= IMAGEFLAGS_DSCREATED;
      }
      return mptrDepthStencil;
    }
    else {
      tBool textureWasCreated = eFalse;
      const tBool isTarget = (aLock == eImageUsage_Target || aLock == eImageUsage_TargetDiscard);
      // create texture
      if (!(mnFlags&IMAGEFLAGS_TEXCREATED) || (isTarget && !(mnFlags&IMAGEFLAGS_TEXRT))) {
        textureWasCreated = eTrue;
        mnFlags |= IMAGEFLAGS_TEXCREATED;
        if (isTarget) {
          mnFlags |= IMAGEFLAGS_TEXRT;
        }
        if (mptrTexture.IsOK()) {
          mptrTexture->Invalidate();
        }
        mptrTexture = mptrGraphics->CreateTexture(
            IMAGE_NEWTEXNAME,eBitmapType_2D,
            _GetPixelFormat()->GetFormat(),
            0,mnWidth,mnHeight,0,
            eTextureFlags_Overlay |
            (isTarget ? eTextureFlags_RenderTarget : eTextureFlags_Dynamic));
        niCheck(mptrTexture.IsOK(),NULL);
      }

      if (mptrTexture.IsOK()) {
        // copy bitmap to texture
        if (mptrBitmap.IsOK()) {
          // if the texture resource has been reset or just created we force a
          // full blit of the whole bitmap regardless of whether its been
          // marked as dirty
          if (mptrTexture->HasDeviceResourceBeenReset(eTrue) || textureWasCreated) {
            _AddDirtyRect(mrectBmpDirtyRect,sRecti(0,0),0);
            mnFlags |= IMAGEFLAGS_BMPDIRTY;
          }
          // blit the bitmap to the texture if its dirty
          if (mnFlags&IMAGEFLAGS_BMPDIRTY) {
            if (aLock != eImageUsage_TargetDiscard) { // don't blit if we discard the current content
              if ((mnFlags&IMAGEFLAGS_RECOMPUTE_BITMAP_MIPMAPS_BEFORE_COPY_TO_TEXTURE)
                  && mptrBitmap->GetNumMipMaps())
              {
                mptrBitmap->CreateMipMaps(mptrBitmap->GetNumMipMaps(),eTrue);
              }
              if (!mptrGraphics->BlitBitmapToTexture(
                      mptrBitmap,
                      mptrTexture,eInvalidHandle,
                      mrectBmpDirtyRect,
                      mrectBmpDirtyRect)) {
                niWarning(_A("Couldn't copy bitmap to texture."));
              }
            }
            mrectBmpDirtyRect = sRecti::Null();
            mnFlags &= ~IMAGEFLAGS_BMPDIRTY;
          }
        }
        // add target dirty rect
        if (isTarget) {
          _AddDirtyRect(mrectBmpDirtyRect,aDirtyRect,
                        mnFlags&IMAGEFLAGS_TEXDIRTY);
          mnFlags |= IMAGEFLAGS_TEXDIRTY;
        }
      }

      return mptrTexture;
    }
  }

  void __stdcall RecomputeBitmapMipmapsBeforeCopyToTexture() {
    mnFlags |= IMAGEFLAGS_RECOMPUTE_BITMAP_MIPMAPS_BEFORE_COPY_TO_TEXTURE;
  }

  iPixelFormat* _GetPixelFormat() const {
    return mptrPxf;
  }

#ifdef _DEBUG
  cString _debugName;
  void _SetDebugName(const achar* aaszDebugName) {
    if (!_debugName.empty())
      _debugName << _A("_");
    _debugName << aaszDebugName;
  }
#endif

 private:
  Ptr<iPixelFormat> mptrPxf;
  Ptr<iGraphics>    mptrGraphics;
  tU32              mnWidth;
  tU32              mnHeight;
  tU32              mnFlags;
  sRecti            mrectBmpDirtyRect;
  sRecti            mrectTexDirtyRect;
  Ptr<iBitmap2D>    mptrBitmap;
  Ptr<iTexture>     mptrTexture;
  Ptr<iTexture>     mptrDepthStencil;
};

#ifdef _DEBUG
void __Image_SetDebugName(iImage* apImg, const achar* aaszDebugName) {
  ((cImage*)apImg)->_SetDebugName(aaszDebugName);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
iImage* __stdcall cGraphics::CreateImage(tU32 anWidth, tU32 anHeight) {
  iImage* img = niNew cImage(this,anWidth,anHeight,NULL,NULL,NULL);
  Image_SetDebugName(img,niFmt(_A("B%dx%d"),anWidth,anHeight));
  return img;
}

///////////////////////////////////////////////
iImage* __stdcall cGraphics::CreateImageEx(tU32 anWidth, tU32 anHeight, iBitmap2D* apBmp, iTexture* apTex, iTexture* apDepthStencil) {
  iImage* img = niNew cImage(this,anWidth,anHeight,apBmp,apTex,apDepthStencil);
  Image_SetDebugName(img,niFmt(_A("B%dx%d"),anWidth,anHeight));
  return img;
}

///////////////////////////////////////////////
iImage* __stdcall cGraphics::CreateImageFromBitmap(iBitmap2D* apBitmap) {
  niCheckIsOK(apBitmap,NULL);
  iImage* img = niNew cImage(this,0,0,apBitmap,NULL,NULL);
  Image_SetDebugName(img,niFmt(_A("FBMP%p"),apBitmap));
  return img;
}

///////////////////////////////////////////////
iImage* __stdcall cGraphics::CreateImageFromTexture(iTexture* apTexture) {
  niCheckIsOK(apTexture,NULL);
  iImage* img = niNew cImage(this,0,0,NULL,apTexture,NULL);
  Image_SetDebugName(img,niFmt(_A("FTEX%p"),apTexture));
  return img;
}

///////////////////////////////////////////////
iImage* __stdcall cGraphics::CreateImageFromBitmapAndTexture(
    iBitmap2D* apBitmap, iTexture* apTexture)
{
  niCheckIsOK(apBitmap,NULL);
  niCheckIsOK(apTexture,NULL);
  niCheck(apBitmap->GetWidth() == apTexture->GetWidth(),NULL);
  niCheck(apBitmap->GetHeight() == apTexture->GetHeight(),NULL);

  iImage* img = niNew cImage(this,0,0,apBitmap,apTexture,NULL);
  Image_SetDebugName(img,niFmt(_A("FBMPnTEX_%p_%p"),apBitmap,apTexture));
  return img;
}

///////////////////////////////////////////////
iImage* __stdcall cGraphics::CreateImageFromFile(iFile* apFile) {
  Ptr<iBitmapBase> ptrBmp = this->LoadBitmap(apFile);
  if (!niIsOK(ptrBmp)) {
    niError(_A("Can't load the bitmap."));
    return NULL;
  }
  if (ptrBmp->GetType() != eBitmapType_2D) {
    niError(_A("Not a 2d bitmap."));
    return NULL;
  }

  iImage* img = niNew cImage(this,0,0,QPtr<iBitmap2D>(ptrBmp),NULL,NULL);
  Image_SetDebugName(img,niFmt(_A("FP_%s"),apFile->GetSourcePath()));
  return img;
}

///////////////////////////////////////////////
iImage* __stdcall cGraphics::CreateImageFromResource(iHString* ahspRes) {
  Ptr<iFile> ptrFile = this->OpenBitmapFile(niHStr(ahspRes));
  if (!niIsOK(ptrFile)) {
    niError(niFmt(_A("Can't open resource '%s'."),niHStr(ahspRes)));
    return NULL;
  }
  Ptr<iBitmapBase> ptrBmp = this->LoadBitmap(ptrFile);
  if (!niIsOK(ptrBmp)) {
    niError(niFmt(_A("Can't load the bitmap from resource '%s'."),niHStr(ahspRes)));
    return NULL;
  }
  if (ptrBmp->GetType() != eBitmapType_2D) {
    niError(niFmt(_A("Resource '%s' is not a 2d bitmap."),niHStr(ahspRes)));
    return NULL;
  }

  iImage* img = niNew cImage(this,0,0,QPtr<iBitmap2D>(ptrBmp),NULL,NULL);
  Image_SetDebugName(img,niFmt(_A("RES_%s"),niHStr(ahspRes)));
  return img;
}

//////////////////////////////////////////////////////////////////////////////////////////////
struct cImageGraphicsContext : public ni::ImplRC<iGraphicsContext>
{
  Ptr<iImage> mptrImage;
  Ptr<iGraphicsContext> mptrGraphicsContext;

  cImageGraphicsContext(iImage* apImage, iGraphics* apGraphics) {
    mptrImage = apImage;
    Ptr<iTexture> rt = mptrImage->GrabTexture(eImageUsage_Target, sRecti::Null());
    Ptr<iTexture> ds = mptrImage->GrabTexture(eImageUsage_DepthStencil, sRecti::Null());
    mptrGraphicsContext = apGraphics->CreateContextForRenderTargets(rt,NULL,NULL,NULL,ds);
    const tF32 fOrthoProjectionOffset = ultof(mptrGraphicsContext->GetDriver()->GetCaps(eGraphicsCaps_OrthoProjectionOffset));
    mptrGraphicsContext->GetFixedStates()->SetCameraViewMatrix(sMatrixf::Identity());
    mptrGraphicsContext->GetFixedStates()->SetCameraProjectionMatrix(MatrixProjection2D(
        fOrthoProjectionOffset,Rectf(0,0,rt->GetWidth(),rt->GetHeight()),0.0f));
  }

  virtual tBool __stdcall IsOK() const {
    return mptrGraphicsContext.IsOK();
  }

  virtual void __stdcall Invalidate() {
  }

  ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) niImpl {
    if (aIID == niGetInterfaceUUID(ni::iImage))
      return mptrImage;
    return BaseImpl::QueryInterface(aIID);
  }
  void __stdcall ListInterfaces(ni::iMutableCollection* apLst, ni::tU32 anFlags) const niImpl {
    apLst->Add(niGetInterfaceUUID(ni::iImage));
    BaseImpl::ListInterfaces(apLst,anFlags);
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return mptrGraphicsContext->GetGraphics();
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mptrGraphicsContext->GetDriver();
  }
  virtual tU32 __stdcall GetWidth() const {
    return mptrImage->GetWidth();
  }
  virtual tU32 __stdcall GetHeight() const {
    return mptrImage->GetHeight();
  }
  virtual iOSWindow* __stdcall GetWindow() const {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    return mptrGraphicsContext->DrawOperation(apDrawOp);
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return mptrImage->GrabBitmap(eImageUsage_Source,sRecti::Null());
  }
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    return mptrGraphicsContext->Display(aFlags, aRect);
  }

  /////////////////////////////////////////////
  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    return mptrGraphicsContext->ClearBuffers(clearBuffer,anColor,afDepth,anStencil);
  }
  virtual iTexture* __stdcall GetRenderTarget(tU32 anIndex) const {
    return mptrImage->GrabTexture(eImageUsage_Target,sRecti::Null());
  }
  virtual iTexture* __stdcall GetDepthStencil() const {
    if (mptrImage->GetHasDepthStencil()) {
      return mptrImage->GrabTexture(eImageUsage_DepthStencil,sRecti::Null());
    }
    return NULL;
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetViewport(const sRecti& aVal) {
    mptrGraphicsContext->SetViewport(aVal);
  }
  virtual sRecti __stdcall GetViewport() const {
    return mptrGraphicsContext->GetViewport();
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetScissorRect(const sRecti& aVal) {
    mptrGraphicsContext->SetScissorRect(aVal);
  }
  virtual sRecti __stdcall GetScissorRect() const {
    return mptrGraphicsContext->GetScissorRect();
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall SetFixedStates(iFixedStates* apStates) {
    return mptrGraphicsContext->SetFixedStates(apStates);
  }
  virtual iFixedStates* __stdcall GetFixedStates() const {
    return mptrGraphicsContext->GetFixedStates();
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall SetMaterial(iMaterial* apMat) {
    return mptrGraphicsContext->SetMaterial(apMat);
  }
  virtual const iMaterial* __stdcall GetMaterial() const {
    return mptrGraphicsContext->GetMaterial();
  }
};

iGraphicsContext* cGraphics::CreateImageGraphicsContext(iImage* apImage) {
  niCheckIsOK(apImage,NULL);
  return niNew cImageGraphicsContext(apImage, this);

}
