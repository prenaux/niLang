// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "AGG.h"

#define VGIMAGEFLAGS_BMPCREATED niBit(0)
#define VGIMAGEFLAGS_BMPDIRTY   niBit(1)
#define VGIMAGEFLAGS_TEXCREATED niBit(2)
#define VGIMAGEFLAGS_TEXDIRTY   niBit(3)
#define VGIMAGEFLAGS_TEXRT      niBit(4)
#define VGIMAGEFLAGS_DSCREATED  niBit(5)

#define VGIMAGE_NAMED
#ifdef VGIMAGE_NAMED
#define VGIMAGE_NEWBMPNAME _H(niFmt(_A("VGImage_Bmp_%p"),(tIntPtr)this))
#define VGIMAGE_NEWTEXNAME _H(niFmt(_A("VGImage_Tex_%p"),(tIntPtr)this))
#define VGIMAGE_NEWDSNAME  _H(niFmt(_A("VGImage_DS_%p"),(tIntPtr)this))
#else
#define VGIMAGE_NEWBMPNAME NULL
#define VGIMAGE_NEWTEXNAME NULL
#define VGIMAGE_NEWDSNAME  NULL
#endif

#define IS_COMPATIBLE_PXF(PXF) true //(PXF->GetBytesPerPixel() == 4 || PXF->GetBytesPerPixel() == 1)

//////////////////////////////////////////////////////////////////////////////////////////////
class cVGImage : public cIUnknownImpl<iVGImage>
{
 public:
  cVGImage(iGraphics* apGraphics, iBitmap2D* apBmp)
      : mnWidth(apBmp->GetWidth())
      , mnHeight(apBmp->GetHeight())
  {
    ZeroMembers();
    mptrGraphics = apGraphics;
    niAssert(IS_COMPATIBLE_PXF(apBmp->GetPixelFormat()));
    mptrBitmap = apBmp;
    mnFlags |= VGIMAGEFLAGS_BMPCREATED|VGIMAGEFLAGS_BMPDIRTY;
    _AddDirtyRect(mrectBmpDirtyRect,sRecti::Null(),eFalse);
    _InitPixelFormat();
  }
  cVGImage(iGraphics* apGraphics, iTexture* apTex)
      : mnWidth(apTex->GetWidth())
      , mnHeight(apTex->GetHeight())
  {
    ZeroMembers();
    mptrGraphics = apGraphics;
    niAssert(IS_COMPATIBLE_PXF(apTex->GetPixelFormat()));
    mptrTexture = apTex;
    mnFlags |= VGIMAGEFLAGS_TEXCREATED|VGIMAGEFLAGS_TEXDIRTY;
    _AddDirtyRect(mrectTexDirtyRect,sRecti::Null(),eFalse);
    _InitPixelFormat();
  }
  cVGImage(iGraphics* apGraphics, iBitmap2D* apBmp, iTexture* apTex)
      : mnWidth(apBmp->GetWidth())
      , mnHeight(apBmp->GetHeight())
  {
    ZeroMembers();
    mptrGraphics = apGraphics;
    mptrBitmap = apBmp;
    mptrTexture = apTex;
    niAssert(IS_COMPATIBLE_PXF(apBmp->GetPixelFormat()));
    niAssert(mptrBitmap->GetWidth() == mptrTexture->GetWidth() &&
             mptrBitmap->GetHeight() == mptrTexture->GetHeight());
    mnFlags |= VGIMAGEFLAGS_BMPCREATED;
    mnFlags |= VGIMAGEFLAGS_TEXCREATED;
    _InitPixelFormat();
  }
  cVGImage(iGraphics* apGraphics, tU32 anWidth, tU32 anHeight)
      : mnWidth(anWidth)
      , mnHeight(anHeight)
  {
    ZeroMembers();
    mptrGraphics = apGraphics;
    _InitPixelFormat();
  }
  ~cVGImage() {
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
  }

  void ZeroMembers() {
    mnFlags = 0;
    mrectBmpDirtyRect = sRecti::Null();
    mrectTexDirtyRect = sRecti::Null();
  }

  tBool __stdcall Resize(tU32 anWidth, tU32 anHeight) {
    if (anWidth < 1 || anHeight < 1 || anWidth > 0xFFFF || anHeight > 0xFFFF)
      return eFalse;
    if (anWidth == mnWidth && anHeight == mnHeight)
      return eTrue;
    mnFlags = 0;
    mnWidth = anWidth;
    mnHeight = anHeight;
    mptrBitmap = NULL;
    mptrTexture = NULL;
    mptrDepthStencil = NULL;
    return eTrue;
  }

  virtual tBool __stdcall Copy(iVGImage* apImage) {
    niCheck(niIsOK(apImage),eFalse);
    if (apImage->GetHasBitmap() || apImage->GetHasTexture()) {
      iBitmap2D* dst = this->GrabBitmap(eVGImageUsage_Target,sRecti::Null());
      niAssert(niIsOK(dst));
      iBitmap2D* src = apImage->GrabBitmap(eVGImageUsage_Source,sRecti::Null());
      niAssert(niIsOK(src));
      dst->Blit(src,0,0,0,0,src->GetWidth(),src->GetHeight());
    }
    return eTrue;
  }

  virtual Ptr<iVGImage> __stdcall Clone() const {
    Ptr<iVGImage> newImg = niNew cVGImage(mptrGraphics,mnWidth,mnHeight);
    VGImage_SetDebugName(newImg,niFmt(_A("CLONE%p"),this));
    newImg->Copy(niThis(cVGImage));
    return newImg;
  }

  tU32 __stdcall GetWidth() const {
    return mnWidth;
  }
  tU32 __stdcall GetHeight() const {
    return mnHeight;
  }

  sVec2f __stdcall GetSize() const {
    return Vec2f((tF32)GetWidth(),(tF32)GetHeight());
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
  iBitmap2D* __stdcall GrabBitmap(eVGImageUsage aLock, const sRecti& aDirtyRect) {
    niCheck(aLock != eVGImageUsage_DepthStencil,NULL);
    if (!(mnFlags&VGIMAGEFLAGS_BMPCREATED)) {
      mnFlags |= VGIMAGEFLAGS_BMPCREATED;
      mptrBitmap = mptrGraphics->CreateBitmap2DEx(mnWidth,mnHeight,
                                                  _GetPixelFormat());
      niCheck(mptrBitmap.IsOK(),NULL);
      mptrBitmap->Clear();
    }
    if (mptrBitmap.IsOK()) {
      if (aLock == eVGImageUsage_Target) {
        // add dirty rect
        _AddDirtyRect(mrectBmpDirtyRect,aDirtyRect,
                      mnFlags&VGIMAGEFLAGS_BMPDIRTY);
        mnFlags |= VGIMAGEFLAGS_BMPDIRTY;
      }
      else {
        // copy texture to bitmap
        if (mnFlags&VGIMAGEFLAGS_TEXDIRTY) {
          niAssert(mptrTexture.IsOK());
          if (!mptrGraphics->BlitTextureToBitmap(
                  mptrTexture,eInvalidHandle,
                  mptrBitmap,
                  mrectTexDirtyRect,
                  mrectTexDirtyRect)) {
            niWarning(_A("Couldn't copy texture to bitmap !"));
          }
          mrectTexDirtyRect = sRecti::Null();
          mnFlags &= ~VGIMAGEFLAGS_TEXDIRTY;
        }
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
  iTexture* __stdcall GrabTexture(eVGImageUsage aLock, const sRecti& aDirtyRect) {
    if (aLock == eVGImageUsage_DepthStencil) {
      if (!(mnFlags&VGIMAGEFLAGS_DSCREATED)) {
        if (mptrTexture.IsOK()) {
          mptrTexture->Invalidate();
        }
        mptrTexture = mptrGraphics->CreateTexture(
            VGIMAGE_NEWDSNAME,eBitmapType_2D,_A("D24S8"),
            0,mnWidth,mnHeight,0,
            eTextureFlags_DepthStencil);
        mnFlags |= VGIMAGEFLAGS_DSCREATED;
      }
      return mptrDepthStencil;
    }
    else {
      // create texture
      if (!(mnFlags&VGIMAGEFLAGS_TEXCREATED)
          || (aLock == eVGImageUsage_Target &&
              !(mnFlags&VGIMAGEFLAGS_TEXRT)))
      {
        mnFlags |= VGIMAGEFLAGS_TEXCREATED;
        if (aLock == eVGImageUsage_Target)
          mnFlags |= VGIMAGEFLAGS_TEXRT;

        if (mptrTexture.IsOK()) {
          mptrTexture->Invalidate();
        }
        mptrTexture = mptrGraphics->CreateTexture(
            VGIMAGE_NEWTEXNAME,eBitmapType_2D,
            _GetPixelFormat()->GetFormat(),
            0,mnWidth,mnHeight,0,
            eTextureFlags_Overlay |
            ((aLock == eVGImageUsage_Target) ?
             eTextureFlags_RenderTarget :
             eTextureFlags_Dynamic));
        niCheck(mptrTexture.IsOK(),NULL);
      }
      if (mptrTexture.IsOK()) {
        if (aLock == eVGImageUsage_Target) {
          // add dirty rect
          _AddDirtyRect(mrectBmpDirtyRect,aDirtyRect,
                        mnFlags&VGIMAGEFLAGS_TEXDIRTY);
          mnFlags |= VGIMAGEFLAGS_TEXDIRTY;
        }
        else {
          // copy texture to bitmap
          if (mptrTexture->HasDeviceResourceBeenReset(eTrue) &&
              mptrBitmap.IsOK())
          {
            _AddDirtyRect(mrectBmpDirtyRect,sRecti(0,0),0);
            mnFlags |= VGIMAGEFLAGS_BMPDIRTY;
          }
          if (mnFlags&VGIMAGEFLAGS_BMPDIRTY) {
            niAssert(mptrTexture.IsOK());
            if (!mptrGraphics->BlitBitmapToTexture(
                    mptrBitmap,
                    mptrTexture,eInvalidHandle,
                    mrectBmpDirtyRect,
                    mrectBmpDirtyRect)) {
              niWarning(_A("Couldn't copy bitmap to texture !"));
            }
            mrectBmpDirtyRect = sRecti::Null();
            mnFlags &= ~VGIMAGEFLAGS_BMPDIRTY;
          }
        }
      }
      return mptrTexture;
    }
  }

  void _InitPixelFormat() {
    if (mptrBitmap.IsOK()) {
      mptrPxf = mptrBitmap->GetPixelFormat()->Clone();
    }
    else if (mptrTexture.IsOK()) {
      mptrPxf = mptrTexture->GetPixelFormat()->Clone();
    }
    else {
      mptrPxf = mptrGraphics->CreatePixelFormat(AGG_PIXELFORMAT);
    }
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
  Ptr<iGraphics>  mptrGraphics;
  tU32            mnWidth;
  tU32                    mnHeight;
  tU32                    mnFlags;
  sRecti       mrectBmpDirtyRect;
  sRecti       mrectTexDirtyRect;
  Ptr<iBitmap2D>  mptrBitmap;
  Ptr<iTexture>   mptrTexture;
  Ptr<iTexture>   mptrDepthStencil;
};

#ifdef _DEBUG
void __VGImage_SetDebugName(iVGImage* apImg, const achar* aaszDebugName) {
  ((cVGImage*)apImg)->_SetDebugName(aaszDebugName);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
Ptr<iVGImage> __stdcall cGraphics::CreateVGImage(tU32 anWidth, tU32 anHeight) {
  Ptr<iVGImage> img = niNew cVGImage(QPtr<iGraphics>(this),anWidth,anHeight);
  VGImage_SetDebugName(img,niFmt(_A("B%dx%d"),anWidth,anHeight));
  return img;
}

///////////////////////////////////////////////
Ptr<iVGImage> __stdcall cGraphics::CreateVGImageFromBitmap(iBitmap2D* apBitmap) {
  niCheckIsOK(apBitmap,NULL);
  niCheck(IS_COMPATIBLE_PXF(apBitmap->GetPixelFormat()),NULL);
  Ptr<iVGImage> img = niNew cVGImage(QPtr<iGraphics>(this),apBitmap);
  VGImage_SetDebugName(img,niFmt(_A("FBMP%p"),apBitmap));
  return img;
}

///////////////////////////////////////////////
Ptr<iVGImage> __stdcall cGraphics::CreateVGImageFromTexture(iTexture* apTexture) {
  niCheckIsOK(apTexture,NULL);
  niCheck(IS_COMPATIBLE_PXF(apTexture->GetPixelFormat()),NULL);
  Ptr<iVGImage> img = niNew cVGImage(QPtr<iGraphics>(this),apTexture);
  VGImage_SetDebugName(img,niFmt(_A("FTEX%p"),apTexture));
  return img;
}

///////////////////////////////////////////////
Ptr<iVGImage> __stdcall cGraphics::CreateVGImageFromBitmapAndTexture(iBitmap2D* apBitmap, iTexture* apTexture) {
  niCheckIsOK(apBitmap,NULL);
  niCheck(IS_COMPATIBLE_PXF(apBitmap->GetPixelFormat()),NULL);
  niCheckIsOK(apTexture,NULL);
  niCheck(apBitmap->GetWidth() == apTexture->GetWidth(),NULL);
  niCheck(apBitmap->GetHeight() == apTexture->GetHeight(),NULL);

  Ptr<iVGImage> img = niNew cVGImage(QPtr<iGraphics>(this),apBitmap,apTexture);
  VGImage_SetDebugName(img,niFmt(_A("FBMPnTEX_%p_%p"),apBitmap,apTexture));
  return img;
}

///////////////////////////////////////////////
Ptr<iVGImage> __stdcall cGraphics::CreateVGImageFromFile(iFile* apFile) {
  Ptr<iBitmapBase> ptrBmp = this->LoadBitmap(apFile);
  if (!niIsOK(ptrBmp)) {
    niError(_A("Can't load the bitmap !"));
    return NULL;
  }
  if (ptrBmp->GetType() != eBitmapType_2D) {
    niError(_A("Not a 2d bitmap !"));
    return NULL;
  }

  if (!IS_COMPATIBLE_PXF(ptrBmp->GetPixelFormat())) {
    Ptr<iPixelFormat> pxf = this->CreatePixelFormat(AGG_PIXELFORMAT);
    ptrBmp = ptrBmp->CreateConvertedFormat(pxf);
    if (!ptrBmp.IsOK()) {
      niError(_A("Can't convert bitmap to VG image format !"));
      return NULL;
    }
  }

  Ptr<iVGImage> img = niNew cVGImage(this,QPtr<iBitmap2D>(ptrBmp));
  VGImage_SetDebugName(img,niFmt(_A("FP_%s"),apFile->GetSourcePath()));
  return img;
}

///////////////////////////////////////////////
Ptr<iVGImage> cGraphics::CreateVGImageFromResource(iHString* ahspRes) {
  Ptr<iFile> ptrFile = this->OpenBitmapFile(niHStr(ahspRes));
  if (!niIsOK(ptrFile)) {
    niError(niFmt(_A("Can't open resource '%s' !"),niHStr(ahspRes)));
    return NULL;
  }

  Ptr<iBitmapBase> ptrBmp = this->LoadBitmap(ptrFile);
  if (!niIsOK(ptrBmp)) {
    niError(niFmt(_A("Can't load the bitmap from resource '%s' !"),niHStr(ahspRes)));
    return NULL;
  }

  if (ptrBmp->GetType() != eBitmapType_2D) {
    niError(niFmt(_A("Resource '%s' is not a 2d bitmap !"),niHStr(ahspRes)));
    return NULL;
  }

  if (!ni::StrEq(ptrBmp->GetPixelFormat()->GetFormat(),AGG_PIXELFORMAT)) {
    Ptr<iPixelFormat> pxf = this->CreatePixelFormat(AGG_PIXELFORMAT);
    ptrBmp = ptrBmp->CreateConvertedFormat(pxf);
    if (!ptrBmp.IsOK()) {
      niError(_A("Can't convert bitmap to VG image format !"));
      return NULL;
    }
  }

  Ptr<iVGImage> img = niNew cVGImage(QPtr<iGraphics>(this),QPtr<iBitmap2D>(ptrBmp));
  VGImage_SetDebugName(img,niFmt(_A("RES_%s"),niHStr(ahspRes)));
  return img;
}

///////////////////////////////////////////////
Ptr<iVGImage> cGraphics::CreateVGImageFromAnyBitmap(iBitmap2D* apBmp) {
  niCheckIsOK(apBmp,NULL);
  if (apBmp->GetType() != eBitmapType_2D) {
    niError(_A("Not a 2D bitmap !"));
    return NULL;
  }

  Ptr<iBitmap2D> bmp = apBmp;
  if (!ni::StrEq(apBmp->GetPixelFormat()->GetFormat(),AGG_PIXELFORMAT)) {
    bmp = (iBitmap2D*)bmp->CreateConvertedFormat(this->CreatePixelFormat(AGG_PIXELFORMAT));
  }

  Ptr<iVGImage> img = CreateVGImageFromBitmap(bmp);
  if (!img.IsOK()) {
    niError(_A("Can't create image from bitmap !"));
    return NULL;
  }

  VGImage_SetDebugName(img,niFmt(_A("ANYBMP%p"),apBmp));
  return img;
}
