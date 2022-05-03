// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "Bitmap2D.h"
#include "BitmapCube.h"
#include "BlitClip.h"
#include "API/niUI/Utils/ULColorBlend.h"

void BmpUtils_BlitPaletteTo32Bits(tPtr apDest, const tPtr apSrc, const tU32 anNumSrcPixels, const tU32* apPalette) {
  tU32* d = (tU32*)apDest;
  const tU8* s = (const tU8*)apSrc;
  niLoop(i,anNumSrcPixels) {
    *d++ = apPalette[*s++];
  }
}

#define TRACE_BLIT_STRETCH(X) //niDebugFmt(X)

static tBool __stdcall _BlitResample(iGraphics* apGraphics, iBitmap2D* apDst, const sRecti& aDestRect, const iBitmap2D* apSrc, const sRecti& aSrcRect);

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
cBitmap2D::cBitmap2D(cGraphics* pGraphics, iHString* ahspName, tU32 nW, tU32 nH, iPixelFormat* pPixFmt, tU32 anPitch, tPtr ptrAddr, tBool bFreeAddr)
{
  mulWidth = 0;
  mulHeight = 0;
  mulPitch = 0;
  mulSize = 0;
  mpData = NULL;
  mpOldData = NULL;
  mFlags = 0;

  mpwGraphics.Swap(pGraphics);
  mhspName = ahspName;

  if(!niIsOK(pPixFmt))
  {
    niError(_A("Invalid pixel format."));
    return;
  }

  mptrPixFmt = pPixFmt->Clone();
  if (!_Setup(nW, nH, anPitch, ptrAddr, bFreeAddr)) {
    niError(_A("Can't setup bitmap."));
    return;
  }
}

///////////////////////////////////////////////
cBitmap2D::~cBitmap2D()
{
  EndUnpackPixels();
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  if (ptrGraphics.IsOK()) {
    if (ptrGraphics->GetTextureDeviceResourceManager()) {
      ptrGraphics->GetTextureDeviceResourceManager()->Unregister(this);
    }
  }
  RemoveMipMaps();
  if (niFlagIs(mFlags,BMPFLAGS_FREEDATA)) {
    niFree(mpData);
  }
}

///////////////////////////////////////////////
tBool cBitmap2D::IsOK() const
{
  return (mpData != NULL);
}

///////////////////////////////////////////////
tTextureFlags __stdcall cBitmap2D::GetFlags() const
{
  return eTextureFlags_SystemMemory|
      eTextureFlags_Dynamic|
      (GetNumMipMaps()?eTextureFlags_MipMaps:0);
}

///////////////////////////////////////////////
iBitmap2D* __stdcall cBitmap2D::Lock(enum eLock aLock, tU32 anLevel, tU32 anFaceSlice)
{
  mFlags |= BMPFLAGS_LOCKED;
  return this;
}

///////////////////////////////////////////////
void __stdcall cBitmap2D::Unlock()
{
  niFlagOff(mFlags,BMPFLAGS_LOCKED);
}

///////////////////////////////////////////////
tBool __stdcall cBitmap2D::GetIsLocked() const
{
  return niFlagIs(mFlags,BMPFLAGS_LOCKED);
}

///////////////////////////////////////////////
tU32 cBitmap2D::GetWidth() const
{
  return mulWidth;
}

///////////////////////////////////////////////
tU32 cBitmap2D::GetHeight() const
{
  return mulHeight;
}

///////////////////////////////////////////////
tU32 cBitmap2D::GetDepth() const
{
  return 1;
}

///////////////////////////////////////////////
tU32 cBitmap2D::GetPitch() const
{
  return mulPitch;
}

///////////////////////////////////////////////
tPtr cBitmap2D::GetData() const
{
  return tPtr(mpData);
}

///////////////////////////////////////////////
tU32    cBitmap2D::GetSize() const
{
  return mulSize;
}

///////////////////////////////////////////////
iPixelFormat* cBitmap2D::GetPixelFormat() const
{
  return mptrPixFmt;
}

///////////////////////////////////////////////
tBool cBitmap2D::BeginUnpackPixels()
{
  if (mpOldData)
    return eFalse;

  tU8* pNewData = (tU8*)mptrPixFmt->BeginUnpackPixels(tPtr(mpData), mulPitch, 0, 0, mulWidth, mulHeight);
  if (!pNewData)
    return eFalse;

  mpOldData = mpData;
  mpData = pNewData;

  return eTrue;
}

///////////////////////////////////////////////
void  cBitmap2D::EndUnpackPixels()
{
  if (mpOldData)
  {
    mpData = mpOldData;
    mptrPixFmt->EndUnpackPixels();
    mpOldData = NULL;
  }
}

///////////////////////////////////////////////
iBitmapBase* cBitmap2D::Clone(ePixelFormatBlit eBlit) const
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheckIsOK(ptrGraphics,NULL);

  Ptr<cBitmap2D> ptrBmp = niNew cBitmap2D(ptrGraphics, NULL, mulWidth, mulHeight, GetPixelFormat());
  if (!ptrBmp.IsOK()) {
    niError(_A("Can't create return bitmap."));
    return NULL;
  }

  if (!ptrBmp->Blit(this, 0, 0, 0, 0, mulWidth, mulHeight, eBlit)) {
    niError(niFmt(_A("Can't blit, pixel format %s."), ptrBmp->GetPixelFormat()->GetFormat()));
    return NULL;
  }

  if (GetNumMipMaps()) {
    ptrBmp->_ResizeMipMapsVector(GetNumMipMaps());
    for (tU32 i = 0; i < GetNumMipMaps(); ++i) {
      ptrBmp->mvMipMaps[i] = niStaticCast(iBitmap2D*,GetMipMap(i)->Clone(eBlit));
      if (!niIsOK(ptrBmp->mvMipMaps[i])) {
        niError(niFmt(_A("Can't create copy of the mipmap %d."), i));
        return NULL;
      }
    }
  }

  return ptrBmp.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmap2D* cBitmap2D::CreateResizedEx(tI32 nW, tI32 nH, tBool abMipMaps) const
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheckIsOK(ptrGraphics,NULL);

  Ptr<cBitmap2D> ptrBmp = niNew cBitmap2D(ptrGraphics, NULL, nW, nH, GetPixelFormat());
  if (!ptrBmp.IsOK()) {
    niError(_A("Can't create return bitmap."));
    return NULL;
  }

  if (!ptrBmp->BlitStretch(this, 0, 0, 0, 0, mulWidth, mulHeight, ptrBmp->GetWidth(), ptrBmp->GetHeight())) {
    niError(niFmt(_A("Can't blit, pixel format %s."), ptrBmp->GetPixelFormat()->GetFormat()));
    return NULL;
  }

  if (abMipMaps && GetNumMipMaps()) {
    if (!ptrBmp->CreateMipMaps(0, eTrue)) {
      niError(_A("Can't compute mipmaps."));
      return NULL;
    }
  }

  return ptrBmp.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmap2D* cBitmap2D::CreateResized(tI32 nW, tI32 nH) const
{
  return CreateResizedEx(nW,nH,eTrue);
}

///////////////////////////////////////////////
iBitmapCube* __stdcall cBitmap2D::CreateCubeBitmap(tU32 anWidth,
                                                   const sVec2i& avPX, ePixelFormatBlit aBlitPX,
                                                   const sVec2i& avNX, ePixelFormatBlit aBlitNX,
                                                   const sVec2i& avPY, ePixelFormatBlit aBlitPY,
                                                   const sVec2i& avNY, ePixelFormatBlit aBlitNY,
                                                   const sVec2i& avPZ, ePixelFormatBlit aBlitPZ,
                                                   const sVec2i& avNZ, ePixelFormatBlit aBlitNZ) const
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheckIsOK(ptrGraphics,NULL);

  Ptr<iBitmapCube> ptrCube = niNew cBitmapCube(ptrGraphics,NULL,anWidth,GetPixelFormat()->Clone());
  niCheck(ptrCube.IsOK(),NULL);

  iBitmap2D* pDst = NULL;

  // Positive X
  pDst = ptrCube->GetFace(eBitmapCubeFace_PositiveX);
  niAssert(niIsOK(pDst));
  pDst->Blit(this,avPX.x,avPX.y,0,0,anWidth,anWidth,aBlitPX);
  // Negative X
  pDst = ptrCube->GetFace(eBitmapCubeFace_NegativeX);
  niAssert(niIsOK(pDst));
  pDst->Blit(this,avNX.x,avNX.y,0,0,anWidth,anWidth,aBlitNX);
  // Positive Y
  pDst = ptrCube->GetFace(eBitmapCubeFace_PositiveY);
  niAssert(niIsOK(pDst));
  pDst->Blit(this,avPY.x,avPY.y,0,0,anWidth,anWidth,aBlitPY);
  // Negative Y
  pDst = ptrCube->GetFace(eBitmapCubeFace_NegativeY);
  niAssert(niIsOK(pDst));
  pDst->Blit(this,avNY.x,avNY.y,0,0,anWidth,anWidth,aBlitNY);
  // Positive Z
  pDst = ptrCube->GetFace(eBitmapCubeFace_PositiveZ);
  niAssert(niIsOK(pDst));
  pDst->Blit(this,avPZ.x,avPZ.y,0,0,anWidth,anWidth,aBlitPZ);
  // Negative Z
  pDst = ptrCube->GetFace(eBitmapCubeFace_NegativeZ);
  niAssert(niIsOK(pDst));
  pDst->Blit(this,avNZ.x,avNZ.y,0,0,anWidth,anWidth,aBlitNZ);

  return ptrCube.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmapCube* __stdcall cBitmap2D::CreateCubeBitmapCross() const
{
  if (GetWidth() >= GetHeight()) {
    // Horizontal
    tU32 width = GetHeight()/3;
    return CreateCubeBitmap(width,
                            Vec2<tI32>(2*width,1*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(0*width,1*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(1*width,0*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(1*width,2*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(1*width,1*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(3*width,1*width),ePixelFormatBlit_Normal);
  }
  else {
    // Vertical
    tU32 width = GetWidth()/3;
    return CreateCubeBitmap(width,
                            Vec2<tI32>(2*width,1*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(0*width,1*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(1*width,0*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(1*width,2*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(1*width,1*width),ePixelFormatBlit_Normal,
                            Vec2<tI32>(1*width,3*width),ePixelFormatBlit_MirrorDiagonal);
  }
}

///////////////////////////////////////////////
iBitmapBase* cBitmap2D::CreateConvertedFormat(const iPixelFormat* pFmt) const
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheckIsOK(ptrGraphics,NULL);

  cBitmap2D* pBmp = niNew cBitmap2D(ptrGraphics, NULL, mulWidth, mulHeight, pFmt->Clone());
  if (!niIsOK(pBmp))
  {
    niSafeRelease(pBmp);
    niError(_A("Can't create return bitmap."));
    return NULL;
  }

  if (!pBmp->Blit(this, 0, 0, 0, 0, mulWidth, mulHeight, ePixelFormatBlit_Normal))
  {
    niError(niFmt(_A("Can't blit, pixel format %s."), pBmp->GetPixelFormat()->GetFormat()));
    niSafeRelease(pBmp);
    return NULL;
  }

  if (GetNumMipMaps()) {
    pBmp->_ResizeMipMapsVector(GetNumMipMaps());
    for (tU32 i = 0; i < GetNumMipMaps(); ++i) {
      pBmp->mvMipMaps[i] = niStaticCast(iBitmap2D*,GetMipMap(i)->CreateConvertedFormat(pFmt));
      if (!niIsOK(pBmp->mvMipMaps[i])) {
        niSafeRelease(pBmp);
        niError(niFmt(_A("Can't create copy of the mipmap %d."), i));
        return NULL;
      }
    }
  }

  return pBmp;
}

///////////////////////////////////////////////
iBitmapBase* cBitmap2D::CreateGammaCorrected(float factor) const
{
  iBitmap2D* pBmp = static_cast<iBitmap2D*>(Clone());
  if (!niIsOK(pBmp))
  {
    niError(_A("Can't create return bitmap."));
    return NULL;
  }

  pBmp->GammaCorrect(factor);
  for (tU32 i = 0; i < pBmp->GetNumMipMaps(); ++i)
    pBmp->GetMipMap(i)->GammaCorrect(factor);

  return pBmp;
}

///////////////////////////////////////////////
tBool cBitmap2D::GammaCorrect(float factor)
{
  if (!(mptrPixFmt->GetCaps()&ePixelFormatCaps_BuildPixel) ||
      !(mptrPixFmt->GetCaps()&ePixelFormatCaps_UnpackPixel))
    return eFalse;

  if (BeginUnpackPixels())
  {
    niDeclareTempPixel();
    tPtr pCol;
    sColor4f col;
    for (tU32 y = 0; y < mulHeight; ++y)
    {
      for (tU32 x = 0; x < mulWidth; ++x)
      {
        pCol = GetPixel(x+1,y+1,niTempPixelPtr());
        col = mptrPixFmt->UnpackPixelf(pCol);
        ColorGammaCorrect((sColor3f&)col, factor);
        PutPixel(x, y, mptrPixFmt->BuildPixelf(pCol, col.x, col.y, col.z, col.w));
      }
    }
    EndUnpackPixels();
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool cBitmap2D::CreateMipMaps(tU32 anNumMipMaps, tBool abCompute)
{
  if (abCompute &&
      (!(mptrPixFmt->GetCaps()&ePixelFormatCaps_BlitStretch)) &&
      (!IsPow2(mulWidth) || !IsPow2(mulHeight)))
  {
    niWarning(niFmt(_A("Pixel format '%s' can't compute mip maps of npot size: %dx%d."),
                    mptrPixFmt->GetFormat(),
                    mulWidth, mulHeight));
    return eFalse;
  }

  const tU32 ulNumMipMaps = anNumMipMaps ? anNumMipMaps : ni::ComputeNumPow2Levels(GetWidth()>>1,GetHeight()>>1);
  if (!ulNumMipMaps) {
    niWarning(niFmt(_A("No mip map required for resolution %dx%d."),GetWidth(),GetHeight()));
    return eTrue;
  }

  const tBool bAllocMipmaps = (ulNumMipMaps != GetNumMipMaps());
  if (bAllocMipmaps) {
    // niDebugFmt(("... Allocating mipmaps."));

    QPtr<cGraphics> ptrGraphics = mpwGraphics;
    niCheckIsOK(ptrGraphics,eFalse);

    _ResizeMipMapsVector(ulNumMipMaps);

    Ptr<iBitmap2D> prevMip = this;
    niLoop(i,GetNumMipMaps()) {
      const tU32 newW = ni::Max(1,prevMip->GetWidth()/2);
      const tU32 newH = ni::Max(1,prevMip->GetHeight()/2);
      mvMipMaps[i] = ptrGraphics->CreateBitmap2DEx(newW, newH, this->GetPixelFormat());
      if (!niIsOK(mvMipMaps[i])) {
        RemoveMipMaps();
        niWarning(niFmt(_A("Can't create mipmap '%d' (%dx%d)."), i, newW, newH));
        return eFalse;
      }
      prevMip = mvMipMaps[i];
    }
  }

  if (abCompute) {
    // if (!bAllocMipmaps) {
    // niDebugFmt(("... Updating mipmaps without recreating them."));
    // }
    Ptr<iBitmap2D> prevMip = this;
    niLoop(i,GetNumMipMaps()) {
      Ptr<iBitmap2D> mip = GetMipMap(i);
      mip->BlitStretch(
        prevMip,
        0, 0,
        0, 0,
        prevMip->GetWidth(), prevMip->GetHeight(),
        mip->GetWidth(), mip->GetHeight());
      prevMip = mip;
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
void cBitmap2D::RemoveMipMaps()
{
  mvMipMaps.clear();
}

///////////////////////////////////////////////
tU32 cBitmap2D::GetNumMipMaps() const
{
  return (tU32)mvMipMaps.size();
}

///////////////////////////////////////////////
iBitmap2D* cBitmap2D::GetMipMap(tU32 ulIdx) const
{
  niCheckSilent(ulIdx < GetNumMipMaps(),NULL);
  return mvMipMaps[ulIdx];
}

///////////////////////////////////////////////
iBitmap2D* __stdcall cBitmap2D::GetLevel(tU32 anIndex) const
{
  if (anIndex == 0) return niConstCast(cBitmap2D*,this);
  return GetMipMap(anIndex-1);
}

///////////////////////////////////////////////
tBool cBitmap2D::Blit(const iBitmap2D* src,
                      tI32 sx, tI32 sy,
                      tI32 dx, tI32 dy,
                      tI32 w, tI32 h,
                      ePixelFormatBlit blitFlags)
{
#pragma niTodo("Emulate if not supported using get/putpixel")
  if (!niIsOK(src))
    return eFalse;

  ePixelFormatCaps caps = mptrPixFmt->GetCaps();
  if ((blitFlags & ePixelFormatBlit_MirrorLeftRight) && !(caps & ePixelFormatCaps_BlitMirrorLeftRight))
    return eFalse;

  if (w == 0) {
    w = src->GetWidth();
  }
  if (h == 0) {
    h = src->GetHeight();
  }

  if ((blitFlags & ePixelFormatBlit_MirrorUpDown) && !(caps & ePixelFormatCaps_BlitMirrorUpDown))
    return eFalse;

  if((sx >= (tI32)src->GetWidth()) || (sy >= (tI32)src->GetHeight()) ||
     (dx >= (tI32)GetWidth()) || (dy >= (tI32)GetHeight()))
    return eFalse;

  if(sx < 0)
  {
    w += sx;
    dx -= sx;
    sx = 0;
  }

  if(sy < 0)
  {
    h += sy;
    dy -= sy;
    sy = 0;
  }

  if(sx+w > (tI32)src->GetWidth())
    w = src->GetWidth() - sx;

  if(sy+h > (tI32)src->GetHeight())
    h = src->GetHeight() - sy;

  if(dx < 0)
  {
    w += dx;
    sx -= dx;
    dx = 0;
  }

  if(dy < 0)
  {
    h += dy;
    sy -= dy;
    dy = 0;
  }

  if(dx+w > (tI32)GetWidth())
    w = GetWidth() - dx;

  if(dy+h > (tI32)GetHeight())
    h = GetHeight() - dy;

  if((w <= 0) || (h <= 0))
    return eFalse;

  return mptrPixFmt->Blit(tPtr(mpData),mulPitch,dx,dy,
                          src->GetData(),src->GetPitch(),
                          src->GetPixelFormat(),
                          sx,sy,w,h,blitFlags);
}

///////////////////////////////////////////////
tBool cBitmap2D::BlitStretch(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd, tI32 yd, tI32 ws, tI32 hs, tI32 wd, tI32 hd)
{
  if (ws == wd && hs == hd) {
    TRACE_BLIT_STRETCH(("... BlitStretch, same size, fallback to Blit"));
    return Blit(src,xs,ys,xd,yd,ws,hs,ePixelFormatBlit_Normal);
  }
  else {
    if (!niIsOK(src))
      return eFalse;

    if (wd == 0 || wd > (tI32)GetWidth()) {
      wd = GetWidth();
    }
    if (hd == 0 || hd > (tI32)GetHeight()) {
      hd = GetHeight();
    }
    if (ws == 0 || ws > (tI32)src->GetWidth()) {
      ws = src->GetWidth();
    }
    if (hs == 0 || hs > (tI32)src->GetHeight()) {
      hs = src->GetHeight();
    }

    if (((wd == ws/2) && (hd == hs/2)) && (mptrPixFmt->GetCaps()&ePixelFormatCaps_BlitStretchHalf)) {
      TRACE_BLIT_STRETCH(("... BlitStretch, Mipmap blit"));
      // "mip map" blit, downsized by half optimized and filtered
      return mptrPixFmt->BlitStretch(tPtr(mpData),mulPitch,mulWidth,mulHeight,xd,yd,wd,hd,
                                     src->GetData(),src->GetPitch(),src->GetPixelFormat(),xs,ys,ws,hs);
    }

    QPtr<cGraphics> ptrGraphics = mpwGraphics;
    niCheckIsOK(ptrGraphics,eFalse);

    // Use _BlitResample highest quality resizing if possible
    if (_BlitResample(ptrGraphics, this, Recti(xd,yd,wd,hd), src, Recti(xs,ys,ws,hs))) {
      TRACE_BLIT_STRETCH(("... BlitStretch, Resample"));
      return eTrue;
    }

    // Can't use _BlitResample fallback to the standard 'fast' BlitStretch
    TRACE_BLIT_STRETCH(("... BlitStretch, can't resample fallback to point filtered blit stretch"));
    if (!(mptrPixFmt->GetCaps() & ePixelFormatCaps_BlitStretch)) {
      TRACE_BLIT_STRETCH(("... BlitStretch, pixel format can't BlitStretch."));
      return eFalse;
    }

    return mptrPixFmt->BlitStretch(
      tPtr(mpData),mulPitch,mulWidth,mulHeight,xd,yd,wd,hd,
      src->GetData(),src->GetPitch(),src->GetPixelFormat(),xs,ys,ws,hs);
  }
}

///////////////////////////////////////////////
tBool __stdcall cBitmap2D::BlitAlpha(const iBitmap2D* src, tI32 sx, tI32 sy, tI32 dx, tI32 dy, tI32 w, tI32 h, ePixelFormatBlit blitFlags,
                                     const sColor4f& acolSource, const sColor4f& acolDest, eBlendMode aBlendMode)
{
  if (!niIsOK(src)) return eFalse;

#pragma niTodo("Emulate if not supported using get/putpixel")
#pragma niTodo("Implement")

  if (w == 0) {
    w = src->GetWidth();
  }
  if (h == 0) {
    h = src->GetHeight();
  }

  ePixelFormatCaps caps = mptrPixFmt->GetCaps();
  if ((blitFlags & ePixelFormatBlit_MirrorUpDown) && !(caps & ePixelFormatCaps_BlitMirrorUpDown))
    return eFalse;

  if((sx >= (tI32)src->GetWidth()) || (sy >= (tI32)src->GetHeight()) ||
     (dx >= (tI32)GetWidth()) || (dy >= (tI32)GetHeight()))
    return eFalse;

  if(sx < 0)
  {
    w += sx;
    dx -= sx;
    sx = 0;
  }

  if(sy < 0)
  {
    h += sy;
    dy -= sy;
    sy = 0;
  }

  if(sx+w > (tI32)src->GetWidth())
    w = src->GetWidth() - sx;

  if(sy+h > (tI32)src->GetHeight())
    h = src->GetHeight() - sy;

  if(dx < 0)
  {
    w += dx;
    sx -= dx;
    dx = 0;
  }

  if(dy < 0)
  {
    h += dy;
    sy -= dy;
    dy = 0;
  }

  if(dx+w > (tI32)GetWidth())
    w = GetWidth() - dx;

  if(dy+h > (tI32)GetHeight())
    h = GetHeight() - dy;

  if((w <= 0) || (h <= 0))
    return eFalse;

  switch (aBlendMode) {
    case eBlendMode_Translucent: {
      if ((acolSource == sColor4f::One() && acolDest == sColor4f::One()) &&
          ((ni::StrEq(src->GetPixelFormat()->GetFormat(),_A("R8G8B8A8")) &&
            ni::StrEq(this->GetPixelFormat()->GetFormat(),_A("R8G8B8A8"))) ||
           (ni::StrEq(src->GetPixelFormat()->GetFormat(),_A("B8G8R8A8")) &&
            ni::StrEq(this->GetPixelFormat()->GetFormat(),_A("B8G8R8A8")))))
      {
        const tU32 sbpp = 4;
        const tU32 dbpp = 4;
        const tU32 spitch = src->GetPitch();
        const tPtr sdata = src->GetData();
        const tU32 dpitch = this->GetPitch();
        const tPtr ddata = this->GetData();
        for (tU32 y = 0; y < (tU32)h; ++y) {
          tU32* pSrc = (tU32*)(sdata+(spitch*(sy+y))+(sbpp*sx));
          tU32* pDst = (tU32*)(ddata+(dpitch*(dy+y))+(dbpp*dx));
          for (tU32 x = 0; x < (tU32)w; ++x)  {
            const tU32 a = *pSrc;
            const tU32 b = *pDst;
            const tU32 n = ULColorGetA(a);
            *pDst = ULColorBlend24(a,b,n);
            ++pSrc;
            ++pDst;
          }
        }
      }
      else if (ni::StrEq(src->GetPixelFormat()->GetFormat(),_A("A8")) &&
               ni::StrEq(this->GetPixelFormat()->GetFormat(),_A("R8G8B8A8")))
      {
        const tU32 dbpp = 4;
        const tU32 spitch = src->GetPitch();
        const tPtr sdata = src->GetData();
        const tU32 dpitch = this->GetPitch();
        const tPtr ddata = this->GetData();
        const tU32 sourceColor = ULColorBuildf(
          acolSource.z,acolSource.y,acolSource.x);
        for (tU32 y = 0; y < (tU32)h; ++y) {
          tU8* pSrc = (tU8*)(sdata+(spitch*(sy+y))+(sx));
          tU32* pDst = (tU32*)(ddata+(dpitch*(dy+y))+(dbpp*dx));
          for (tU32 x = 0; x < (tU32)w; ++x)  {
            const tU32 b = *pDst;
            const tU32 n = *pSrc;
            *pDst = ULColorBlend24(sourceColor,b,n);
            ++pSrc;
            ++pDst;
          }
        }
      }
      else
      {
        // Unoptimized path
        const tU32 bipp = src->GetPixelFormat()->GetBitsPerPixel();
        const tU32 abits = src->GetPixelFormat()->GetNumABits();
        const tBool srcHasRGB = !!(bipp-abits);
        // const tBool srcHasAlpha = !!abits;
        if (((iBitmap2D*)src)->BeginUnpackPixels()) {
          if (acolSource == sColor4f::One() && acolDest == sColor4f::One()) {
            for (tU32 y = 0; y < (tU32)h; ++y) {
              for (tU32 x = 0; x < (tU32)w; ++x) {
                const sColor4f s = src->GetPixelf(x+sx,y+sy);
                const sColor4f d = this->GetPixelf(x+dx,y+dy);
                this->PutPixelf(x+dx,y+dy, ni::Lerp(d,s,s.w));
              }
            }
          }
          else {
            sColor4f colSrc = acolSource;
            for (tU32 y = 0; y < (tU32)h; ++y) {
              for (tU32 x = 0; x < (tU32)w; ++x) {
                const sColor4f srcPixel = src->GetPixelf(x+sx,y+sy);
                if (srcHasRGB) {
                  ((sVec3f&)colSrc) = ((sVec3f&)srcPixel)*
                      ((sVec3f&)acolSource);
                }
                colSrc.w = srcPixel.w*acolSource.w;
                const sColor4f colDest = this->GetPixelf(x+dx,y+dy)*acolDest;
                this->PutPixelf(x+dx,y+dy, ni::Lerp(colDest,colSrc,colSrc.w));
              }
            }
          }
          ((iBitmap2D*)src)->EndUnpackPixels();
        }
      }
      return eTrue;
    }

    default: {
      break;
    }
  }

  return eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cBitmap2D::BlitAlphaStretch(
  const iBitmap2D* src, tI32 sx, tI32 sy, tI32 dx, tI32 dy, tI32 sw, tI32 sh, tI32 dw, tI32 dh,
  const sColor4f& acolSource, const sColor4f& acolDest, eBlendMode aBlendMode)
{
  if (sw == dw && sh == dh) {
    return BlitAlpha(src,sx,sy,dx,dy,sw,sh,ePixelFormatBlit_Normal,acolSource,acolDest,aBlendMode);
  }
  else {
    if (!niIsOK(src))
      return eFalse;

    if (dw == 0 || dw > (tI32)GetWidth()) {
      dw = GetWidth();
    }
    if (dh == 0 || dh > (tI32)GetHeight()) {
      dh = GetHeight();
    }
    if (sw == 0 || sw > (tI32)src->GetWidth()) {
      sw = src->GetWidth();
    }
    if (sh == 0 || sh > (tI32)src->GetHeight()) {
      sh = src->GetHeight();
    }

    iPixelFormat* pSrcFmt = src->GetPixelFormat();
    if (!(pSrcFmt->GetCaps() & ePixelFormatCaps_UnpackPixel))
      return eFalse;

    const iPixelFormat* pDstFmt = this->GetPixelFormat();
    const tU32 ulDestWidth = this->GetWidth();
    const tU32 ulDestHeight = this->GetHeight();
    const tU32 ulSrcBypp = pSrcFmt->GetBytesPerPixel();
    const tU32 ulDestBypp = pDstFmt->GetBytesPerPixel();

    int x, y;
    int i1, i2, dd;
    int xinc, yinc;
    int dxbeg, dxend;
    int dybeg, dyend;
    int si_sx, si_dx;
    int si_i1, si_i2;
    int si_dd, si_dw;
    tU8 *s, *d, *dend;

    if((sw <= 0) || (sh <= 0) || (dw <= 0) || (dh <= 0))
      return eFalse;

    dybeg = (dy > 0) ? dy : 0;
    dyend = ((dy + dh) < (int)ulDestHeight) ? (dy + dh) : (int)ulDestHeight;
    if(dybeg >= dyend)
      return eFalse;

    dxbeg = (dx > 0) ? dx : 0;
    dxend = ((dx + dw) < (int)ulDestWidth) ? (dx + dw) : (int)ulDestWidth;
    if(dxbeg >= dxend)
      return eFalse;

    --sw; --sh;
    --dw; --dh;

    if (dw == 0) {
      xinc = 0;
    }
    else {
      xinc = sw / dw;
      sw %= dw;
    }
    if (dh == 0) {
      yinc = 0;
    }
    else {
      yinc = sh / dh;
      sh %= dh;
    }

    si_dw = dxend - dxbeg;
    si_i1 = 2 * sw;
    si_dd = si_i1 - si_dw;
    si_i2 = si_dd - si_dw;
    for(si_dx = dx, si_sx = sx; si_dx < dxbeg; si_dx++, si_sx += xinc)
    {
      if(si_dd >= 0)
      {
        si_sx++;
        si_dd += si_i2;
      }
      else
      {
        si_dd += si_i1;
      }
    }
    si_dx *= ulDestBypp;
    si_sx *= ulSrcBypp;
    si_dw *= ulDestBypp;
    xinc *= ulSrcBypp;

    i2 = (dd = (i1 = 2 * sh) - dh) - dh;
    for(x = dy, y = sy; x < dybeg; x++, y += yinc)
    {
      if(dd >= 0)
        y++, dd += i2;
      else
        dd += i1;
    }

    const tU32 ulSrcPitch = src->GetWidth()*pSrcFmt->GetBytesPerPixel();
    const tU32 ulDestPitch = this->GetWidth()*pDstFmt->GetBytesPerPixel();
    tPtr pSrc = src->GetData();
    tPtr pDst = this->GetData();
    {
      pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, sw, sh);
      if (pSrc)
      {
        for (; x < dyend; ++x, y += yinc) {
          // Do stretch line
          s = ((tU8*)(pSrc)) + (y*ulSrcPitch) + si_sx;
          d = ((tU8*)(pDst)) + (x*ulDestPitch) + si_dx;
          dend = d + si_dw;
          for (; d < dend; d += ulDestBypp, s += xinc) {

            const sVec4f srcColor = pSrcFmt->UnpackPixelf(tPtr(s));
            const tF32 fAlpha = srcColor.w;
            const sVec4f dstColor = ni::Lerp(
              pDstFmt->UnpackPixelf(tPtr(d)), srcColor, fAlpha);
            pDstFmt->BuildPixelf(
              tPtr(d),
              dstColor.x, dstColor.y, dstColor.z, dstColor.w);
            if (si_dd >= 0) {
              s += pSrcFmt->GetBytesPerPixel();
              si_dd += si_i2;
            }
            else {
              si_dd += si_i1;
            }
          }
          if (dd >= 0) {
            ++y;
            dd += i2;
          }
          else {
            dd += i1;
          }
        }
        pSrcFmt->EndUnpackPixels();
      }
    }

    return eTrue;
  }
}

///////////////////////////////////////////////
tBool cBitmap2D::_Setup(int anW, int anH, tU32 anPitch, tPtr aptrAddr, tBool abFreeAddr)
{
  mulWidth = ni::Clamp(anW,1,65535);
  mulHeight = ni::Clamp(anH,1,65535);
  mulPitch = anPitch ? anPitch : mulWidth * mptrPixFmt->GetBytesPerPixel();
  mulSize = mptrPixFmt->GetSize(mulWidth,mulHeight,0);

  if (!aptrAddr)
  {
    if (!SetMemoryAddress((tPtr)niMalloc(mulSize), eTrue, eInvalidHandle))
    {
      niError(_A("Can't alloc data memory."));
      return eFalse;
    }
  }
  else
  {
    SetMemoryAddress(aptrAddr, abFreeAddr, eInvalidHandle);
  }

  return eTrue;
}

///////////////////////////////////////////////
void cBitmap2D::PutPixel(tI32 x, tI32 y, tPtr col)
{
  if (x < 0 || x >= (tI32)mulWidth || y < 0 || y >= (tI32)mulHeight)
    return;

  tU32 i;
  tU8* pPtr = (tU8*)col;
  tU32 bypp = mptrPixFmt->GetBytesPerPixel();
  tU8* pDest = mpData + (y*mulPitch) + (x*bypp);
  for (i = 0; i < bypp; ++i)
    *pDest++ = *pPtr++;
}

///////////////////////////////////////////////
tPtr cBitmap2D::GetPixel(tI32 x, tI32 y, tPtr pOut) const
{
  if (!pOut || x < 0 || x >= (tI32)mulWidth || y < 0 || y >= (tI32)mulHeight)
    return pOut;

  tU32 i;
  tU32 bypp = mptrPixFmt->GetBytesPerPixel();
  tU8* pPtr = mpData + (y*mulPitch) + (x*bypp);
  tU8* pDest = (tU8*)pOut;
  for (i = 0; i < bypp; ++i)
    *pDest++ = *pPtr++;

  return pOut;
}

///////////////////////////////////////////////
void cBitmap2D::Clear(tPtr pColor)
{
  this->ClearRect(Recti(0,0,GetWidth(),GetHeight()),pColor);
}

///////////////////////////////////////////////
void __stdcall cBitmap2D::ClearRect(const sRecti& aRect, tPtr pColor)
{
  // clip the rectangle to fit in the bitmap
  sRecti rect = aRect;
  if (rect.GetWidth() == 0) rect.SetWidth(GetWidth());
  if (rect.GetHeight() == 0) rect.SetHeight(GetHeight());
  if (rect.Left() >= (tI32)GetWidth())    return;
  if (rect.Left() < 0)    rect.Left() = 0;
  if (rect.Top() >= (tI32)GetHeight())    return;
  if (rect.Top() < 0) rect.Top() = 0;
  if (rect.Right()  > (tI32)GetWidth()) rect.Right() = GetWidth();
  if (rect.Bottom() > (tI32)GetHeight()) rect.Bottom() = GetHeight();

  if (niFlagIs(mptrPixFmt->GetCaps(),ePixelFormatCaps_Clear)) {
    mptrPixFmt->Clear(mpData, mulPitch, rect.Left(), rect.Top(), rect.GetWidth(), rect.GetHeight(), pColor);
    return;
  }

  // if we clear the whole surface with 0
  if ((!pColor) &&
      rect.Left() == 0 &&
      rect.Top() == 0 &&
      rect.GetWidth() == (tI32)this->GetWidth() &&
      rect.GetHeight() == (tI32)this->GetHeight())
  {
    memset(mpData,0,mulSize);
    return;
  }

  if (niFlagIs(mptrPixFmt->GetCaps(),ePixelFormatCaps_BlockCompressed)) {
    // We can't clear a block compressed 'per byte'
    return;
  }

  tI32 i, x, y;
  tI32 bypp = mptrPixFmt->GetBytesPerPixel();
  switch (bypp) {
    case 4:
      {
        if (pColor) {
          tU32 nColor = *(tU32*)(pColor);
          for (y = rect.Top(); y < rect.Bottom(); ++y) {
            tU32* pDest = (tU32*)(mpData + (y*mulPitch) + (rect.Left()*4));
            for (x = rect.Left(); x < rect.Right(); ++x) {
              *pDest++ = nColor;
            }
          }
        }
        else {
          for (y = rect.Top(); y < rect.Bottom(); ++y) {
            tU8* pDest = mpData + (y*mulPitch) + (rect.Left()*4);
            memset(pDest,0,((rect.Right()-rect.Left())-1)*4);
          }
        }
        break;
      }
    default:
      {
        if (pColor) {
          for (y = rect.Top(); y < rect.Bottom(); ++y) {
            tU8* pDest = mpData + (y*mulPitch) + (rect.Left()*bypp);
            for (x = rect.Left(); x < rect.Right(); ++x) {
              for (i = 0; i < bypp; ++i)
                *pDest++ = ((tU8*)(pColor))[i];
            }
          }
        }
        else {
          for (y = rect.Top(); y < rect.Bottom(); ++y) {
            tU8* pDest = mpData + (y*mulPitch) + (rect.Left()*bypp);
            for (x = rect.Left(); x < rect.Right(); ++x) {
              for (i = 0; i < bypp; ++i)
                *pDest++ = 0;
            }
          }
        }
        break;
      }
  }
}

///////////////////////////////////////////////
void __stdcall cBitmap2D::PutPixelf(tI32 x, tI32 y, const sColor4f& col)
{
  niDeclareTempPixel();
  mptrPixFmt->BuildPixelf(niTempPixelPtr(),col.x,col.y,col.z,col.w);
  PutPixel(x,y,niTempPixelPtr());
}

///////////////////////////////////////////////
sColor4f __stdcall cBitmap2D::GetPixelf(tI32 x, tI32 y) const
{
  niDeclareTempPixel();
  return mptrPixFmt->UnpackPixelf(GetPixel(x,y,niTempPixelPtr()));
}

///////////////////////////////////////////////
void __stdcall cBitmap2D::Clearf(const sColor4f& col)
{
  if (col == sColor4f::Zero()) {
    // This is important so that pixel format that dont support a colored
    // clear (such as most block compressed format) work correctly.
    Clear(NULL);
  }
  else {
    niDeclareTempPixel();
    mptrPixFmt->BuildPixelf(niTempPixelPtr(),col.x,col.y,col.z,col.w);
    Clear(niTempPixelPtr());
  }
}

///////////////////////////////////////////////
void __stdcall cBitmap2D::ClearRectf(const sRecti& aRect, const sColor4f& col)
{
  if (col == sColor4f::Zero()) {
    // This is important so that pixel format that dont support a colored
    // clear (such as most block compressed format) work correctly.
    ClearRect(aRect,NULL);
  }
  else {
    niDeclareTempPixel();
    mptrPixFmt->BuildPixelf(niTempPixelPtr(),col.x,col.y,col.z,col.w);
    ClearRect(aRect,niTempPixelPtr());
  }
}

///////////////////////////////////////////////
void cBitmap2D::_ResizeMipMapsVector(tU32 aulNumMipMaps)
{
  RemoveMipMaps();
  mvMipMaps.clear();
  mvMipMaps.resize(aulNumMipMaps);
}

///////////////////////////////////////////////
//! Set the memory address.
tBool __stdcall cBitmap2D::SetMemoryAddress(tPtr apAddr, tBool abFreeAddr, tU32 anPitch)
{
  mpData = (tU8*)apAddr;
  niFlagOnIf(mFlags,BMPFLAGS_FREEDATA,abFreeAddr);
  if (anPitch != eInvalidHandle) mulPitch = anPitch;
  return mpData != NULL;
}

///////////////////////////////////////////////
iTexture* __stdcall cBitmap2D::GetSubTexture(tU32 anIndex) const
{
  return NULL;
}

//--------------------------------------------------------------------------------------------
//
// _BlitResample
//
//--------------------------------------------------------------------------------------------
#define RESAMPLE_PXF "R8G8B8A8"

// The algorithm requires the pixel format to be 32 bits wide;
static inline tBool _CanResampleLossless(const iBitmap2D* bmp) {
  const iPixelFormat* pxfmt = bmp->GetPixelFormat();
  const char* cszPxf = pxfmt->GetFormat();
  return
      // can't be a float pixel format
      (cszPxf[0] != 'F') &&
      // max 8 bits per channel
      pxfmt->GetNumRBits() <= 8 &&
      pxfmt->GetNumGBits() <= 8 &&
      pxfmt->GetNumBBits() <= 8 &&
      pxfmt->GetNumABits() <= 8;
}

// The algorithm requires the pixel format to be 32 bits wide;
static inline tBool _CanResamplePixelFormat(const iBitmap2D* bmp) {
  const iPixelFormat* pxfmt = bmp->GetPixelFormat();
  const char* cszPxf = pxfmt->GetFormat();
  return
      // should be b8g8r8x8 or r8g8b8x8
      (cszPxf[0] == 'r' || cszPxf[0] == 'R' || cszPxf[0] == 'b' || cszPxf[0] == 'B') &&
      (cszPxf[4] == 'r' || cszPxf[4] == 'R' || cszPxf[4] == 'b' || cszPxf[4] == 'B') &&
      // need four 8 bit channels
      pxfmt->GetNumRBits() == 8 &&
      pxfmt->GetNumGBits() == 8 &&
      pxfmt->GetNumBBits() == 8 &&
      pxfmt->GetNumABits() == 8;
}

static inline tBool _ShouldSwapRB(const achar* aaszPxfmtA, const achar* aaszPxfmtB) {
  return
      ((aaszPxfmtA[0] == 'r' || aaszPxfmtA[0] == 'R')  &&
       (aaszPxfmtB[0] == 'b' || aaszPxfmtB[0] == 'B')) ||
      ((aaszPxfmtA[0] == 'b' || aaszPxfmtA[0] == 'B')  &&
       (aaszPxfmtB[0] == 'r' || aaszPxfmtB[0] == 'R')) ;
}

// c implementation
static __forceinline int _ResampleShrinkX_C(
  ni::tU8 *dstpix, const ni::tU8 *srcpix,
  int height, long dstpitch, long srcpitch, int dstwidth, int srcwidth,
  void *workmem)
{
  ni::tI32 srcdiff = srcpitch - (srcwidth * 4);
  ni::tI32 dstdiff = dstpitch - (dstwidth * 4);
  ni::tI32 x, y;
  ni::tI32 xspace = 0x10000 * srcwidth / dstwidth;
  ni::tI32 xrecip = 0;
  ni::tI64 zrecip = 1;

  zrecip <<= 32;
  xrecip = (int)(zrecip / xspace);

  for (y = 0; y < height; y++) {
    ni::tU32 accumulate[4] = { 0, 0, 0, 0 };
    int xcounter = xspace;
    for (x = 0; x < srcwidth; x++) {
      if (xcounter > 0x10000) {
        accumulate[0] += (ni::tU32) *srcpix++;
        accumulate[1] += (ni::tU32) *srcpix++;
        accumulate[2] += (ni::tU32) *srcpix++;
        accumulate[3] += (ni::tU32) *srcpix++;
        xcounter -= 0x10000;
      } else {
        int xfrac = 0x10000 - xcounter;
#define ismooth_putpix_x(n) {                                           \
          *dstpix++ = (ni::tU8)(((accumulate[n] + ((srcpix[n]           \
                                                    * xcounter) >> 16)) * xrecip) >> 16); \
        }
        ismooth_putpix_x(0);
        ismooth_putpix_x(1);
        ismooth_putpix_x(2);
        ismooth_putpix_x(3);
#undef ismooth_putpix_x
        accumulate[0] = (ni::tU32)((*srcpix++ * xfrac) >> 16);
        accumulate[1] = (ni::tU32)((*srcpix++ * xfrac) >> 16);
        accumulate[2] = (ni::tU32)((*srcpix++ * xfrac) >> 16);
        accumulate[3] = (ni::tU32)((*srcpix++ * xfrac) >> 16);
        xcounter = xspace - xfrac;
      }
    }
    srcpix += srcdiff;
    dstpix += dstdiff;
  }
  return 0;
}

// c implementation
static __forceinline int _ResampleShrinkY_C(
  ni::tU8 *dstpix, const ni::tU8 *srcpix,
  int width, long dstpitch, long srcpitch, int dstheight, int srcheight,
  void *workmem)
{
  TRACE_BLIT_STRETCH(("... _ResampleShrinkY_C"));

  ni::tI32 srcdiff = srcpitch - (width * 4);
  ni::tI32 dstdiff = dstpitch - (width * 4);
  ni::tI32 x, y;
  ni::tI32 yspace = 0x10000 * srcheight / dstheight;
  ni::tI32 yrecip = 0;
  ni::tI32 ycounter = yspace;
  ni::tU32 *templine;

  ni::tI64 zrecip = 1;
  zrecip <<= 32;
  yrecip = (ni::tI32)(zrecip / yspace);

  // size = width * 4 * 4
  templine = (ni::tU32*)workmem;
  if (templine == NULL) return -1;

  memset(templine, 0, width * 4 * 4);

  for (y = 0; y < srcheight; y++) {
    ni::tU32 *accumulate = templine;
    if (ycounter > 0x10000) {
      for (x = 0; x < width; srcpix += 4, accumulate += 4, x++) {
        accumulate[0] += (ni::tU32)srcpix[0];
        accumulate[1] += (ni::tU32)srcpix[1];
        accumulate[2] += (ni::tU32)srcpix[2];
        accumulate[3] += (ni::tU32)srcpix[3];
      }
      ycounter -= 0x10000;
    } else {
      ni::tI32 yfrac = 0x10000 - ycounter;
      ni::tI32 yc = ycounter;
      ni::tI32 yr = yrecip;
      for (x = 0; x < width; dstpix += 4, srcpix += 4, accumulate += 4, x++) {
        dstpix[0] = (ni::tU8)(((accumulate[0] + ((srcpix[0] * yc) >> 16)) * yr) >> 16);
        dstpix[1] = (ni::tU8)(((accumulate[1] + ((srcpix[1] * yc) >> 16)) * yr) >> 16);
        dstpix[2] = (ni::tU8)(((accumulate[2] + ((srcpix[2] * yc) >> 16)) * yr) >> 16);
        dstpix[3] = (ni::tU8)(((accumulate[3] + ((srcpix[3] * yc) >> 16)) * yr) >> 16);
      }
      dstpix += dstdiff;
      accumulate = templine;
      srcpix -= 4 * width;
      for (x = 0; x < width; accumulate += 4, srcpix += 4, x++) {
        accumulate[0] = (ni::tU32)((srcpix[0] * yfrac) >> 16);
        accumulate[1] = (ni::tU32)((srcpix[1] * yfrac) >> 16);
        accumulate[2] = (ni::tU32)((srcpix[2] * yfrac) >> 16);
        accumulate[3] = (ni::tU32)((srcpix[3] * yfrac) >> 16);
      }
      ycounter = yspace - yfrac;
    }
    srcpix += srcdiff;
  }

  return 0;
}

// c implementation
static __forceinline int _ResampleExpandX_C(ni::tU8 *dstpix, const ni::tU8 *srcpix,
                                            int height, long dstpitch, long srcpitch, int dstwidth, int srcwidth,
                                            void *workmem)
{
  ni::tI32 dstdiff = dstpitch - (dstwidth * 4);
  ni::tI32 *xidx0, *xmult0, *xmult1;
  ni::tI32 x, y;

  if (workmem == NULL) return -1;

  xidx0 = (ni::tI32*)workmem;   // size = 3 * dstwidth * 4
  xmult0 = xidx0 + dstwidth;
  xmult1 = xidx0 + dstwidth * 2;

  for (x = 0; x < dstwidth; x++) {
    xidx0[x] = x * (srcwidth - 1) / dstwidth;
    xmult1[x] = 0x10000 * ((x * (srcwidth - 1)) % dstwidth) / dstwidth;
    xmult0[x] = 0x10000 - xmult1[x];
  }

  for (y = 0; y < height; y++) {
    const ni::tU8 *srcrow0 = srcpix + y * srcpitch;
    for (x = 0; x < dstwidth; x++) {
      const ni::tU8 *src = srcrow0 + xidx0[x] * 4;
      ni::tI32 xm0 = xmult0[x];
      ni::tI32 xm1 = xmult1[x];
      *dstpix++ = (ni::tU8)(((src[0] * xm0) + (src[4] * xm1)) >> 16);
      *dstpix++ = (ni::tU8)(((src[1] * xm0) + (src[5] * xm1)) >> 16);
      *dstpix++ = (ni::tU8)(((src[2] * xm0) + (src[6] * xm1)) >> 16);
      *dstpix++ = (ni::tU8)(((src[3] * xm0) + (src[7] * xm1)) >> 16);
    }
    dstpix += dstdiff;
  }

  return 0;
}

// c implementation
static __forceinline int _ResampleExpandY_C(ni::tU8 *dstpix, const ni::tU8 *srcpix,
                                            int width, long dstpitch, long srcpitch, int dstheight, int srcheight,
                                            void *workmem)
{
  ni::tI32 x, y;
  for (y = 0; y < dstheight; y++) {
    int yidx0 = y * (srcheight - 1) / dstheight;
    const ni::tU8 *s0 = srcpix + yidx0 * srcpitch;
    const ni::tU8 *s1 = s0 + srcpitch;
    int ym1 = 0x10000 * ((y * (srcheight - 1)) % dstheight) / dstheight;
    int ym0 = 0x10000 - ym1;
    for (x = 0; x < width; x++) {
      *dstpix++ = (ni::tU8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
      *dstpix++ = (ni::tU8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
      *dstpix++ = (ni::tU8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
      *dstpix++ = (ni::tU8)(((*s0++ * ym0) + (*s1++ * ym1)) >> 16);
    }
    dstpix += dstpitch - 4 * width;
  }

  return 0;
}

//---------------------------------------------------------------------
// ResampleSmooth
//---------------------------------------------------------------------
static __forceinline int _ResampleSmooth(
  ni::tU8 *dstpix, const ni::tU8 *srcpix, int dstwidth,
  int srcwidth, int dstheight, int srcheight, long dstpitch, long srcpitch)
{
  if (srcwidth == dstwidth && srcheight == dstheight) {
    long size = srcwidth * 4;
    for (int y = 0; y < dstheight; y++) {
      ni::MemCopy((ni::tPtr)dstpix + y * dstpitch, (ni::tPtr)srcpix + y * srcpitch, size);
    }
    return 0;
  }

  long needsrc = (srcwidth > srcheight)? srcwidth : srcheight;
  long needdst = (dstwidth > dstheight)? dstwidth : dstheight;
  long worksize = ((needsrc > needdst)? needsrc : needdst) * 32;
  long imagesize = ((long)srcwidth) * dstheight * 4;

  ni::tU8 *temp = new ni::tU8[imagesize + worksize];
  if (temp == NULL) return -1;

  ni::tU8 *workmem = temp + imagesize;

  if (dstwidth == srcwidth) {
    if (dstheight < srcheight) {
      if (_ResampleShrinkY_C(dstpix, srcpix, srcwidth, dstpitch,
                             srcpitch, dstheight, srcheight, workmem) != 0) {
        delete []temp;
        return -2;
      }
    }
    else if (dstheight > srcheight) {
      if (_ResampleExpandY_C(dstpix, srcpix, srcwidth, dstpitch,
                             srcpitch, dstheight, srcheight, workmem) != 0) {
        delete []temp;
        return -3;
      }
    }
    else {
      niAssert(0);
    }
    delete []temp;
    return 0;
  }

  if (dstheight < srcheight) {
    if (_ResampleShrinkY_C(temp, srcpix, srcwidth, srcwidth * 4,
                           srcpitch, dstheight, srcheight, workmem) != 0) {
      delete []temp;
      return -4;
    }
  }
  else if (dstheight > srcheight) {
    if (_ResampleExpandY_C(temp, srcpix, srcwidth, srcwidth * 4,
                           srcpitch, dstheight, srcheight, workmem) != 0) {
      delete []temp;
      return -5;
    }
  }
  else {
    if (dstwidth < srcwidth) {
      if (_ResampleShrinkX_C(dstpix, srcpix, dstheight, dstpitch,
                             srcpitch, dstwidth, srcwidth, workmem) != 0) {
        delete []temp;
        return -6;
      }
    }
    else if (dstwidth > srcwidth) {
      if (_ResampleExpandX_C(dstpix, srcpix, dstheight, dstpitch,
                             srcpitch, dstwidth, srcwidth, workmem) != 0) {
        delete []temp;
        return -7;
      }
    }
    else {
      niAssert(0);
    }
    delete []temp;
    return 0;
  }

  if (dstwidth < srcwidth) {
    if (_ResampleShrinkX_C(dstpix, temp, dstheight, dstpitch,
                           srcwidth * 4, dstwidth, srcwidth, workmem) != 0) {
      delete []temp;
      return -8;
    }
  }
  else if (dstwidth > srcwidth) {
    if (_ResampleExpandX_C(dstpix, temp, dstheight, dstpitch,
                           srcwidth * 4, dstwidth, srcwidth, workmem) != 0) {
      delete []temp;
      return -9;
    }
  }
  else {
    long size = srcwidth * 4;
    for (int y = 0; y < dstheight; y++) {
      ni::MemCopy((ni::tPtr)dstpix + y * dstpitch, (ni::tPtr)temp + y * size, size);
    }
  }

  delete[] temp;
  return 0;
}

///////////////////////////////////////////////
static tBool __stdcall _BlitResample(iGraphics* apGraphics, iBitmap2D* apDst, const sRecti& aDestRect, const iBitmap2D* apSrc, const sRecti& aSrcRect)
{
  niCheck(niIsOK(apSrc),eFalse);
  niCheck(niIsOK(apDst),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_2D,eFalse);
  if (!_CanResampleLossless(apSrc))
    return eFalse;

  sRecti srcRect;
  if (!ClipBlitRect(srcRect,apSrc,0,aSrcRect))
    return eTrue;

  sRecti dstRect;
  if (!ClipBlitRect(dstRect,apDst,0,aDestRect))
    return eTrue;

  tI32 sx = srcRect.x, sy = srcRect.y;
  const tI32 sw = srcRect.GetWidth(), sh = srcRect.GetHeight();
  tI32 dx = dstRect.x, dy = dstRect.y;
  const tI32 dw = dstRect.GetWidth(), dh = dstRect.GetHeight();

  // No resampling if the src area is too small
  if (sw < 2 || sh < 2)
    return eFalse;

  Ptr<iBitmap2D> src = apSrc;
  if (!_CanResamplePixelFormat(apSrc)) {
    src = apGraphics->CreateBitmap2D(srcRect.GetWidth(),srcRect.GetHeight(),RESAMPLE_PXF);
    src->Blit(apSrc,srcRect.x,srcRect.y,0,0,srcRect.GetWidth(),srcRect.GetHeight());
    sx = 0; sy = 0;
    TRACE_BLIT_STRETCH(("... _BlitResample::Converting Src %s -> %s",
                        apSrc->GetPixelFormat()->GetFormat(),
                        src->GetPixelFormat()->GetFormat()));
  }

  Ptr<iBitmap2D> dst = apDst;
  if (!_CanResamplePixelFormat(apDst)) {
    dst = apGraphics->CreateBitmap2D(dstRect.GetWidth(),dstRect.GetHeight(),src->GetPixelFormat()->GetFormat());
    dx = 0; dy = 0;
    TRACE_BLIT_STRETCH(("... _BlitResample::Converted Dst %s -> %s",
                        apDst->GetPixelFormat()->GetFormat(),
                        dst->GetPixelFormat()->GetFormat()));
  }

  const tBool shouldSwapRB = _ShouldSwapRB(src->GetPixelFormat()->GetFormat(),dst->GetPixelFormat()->GetFormat());
  TRACE_BLIT_STRETCH(("... Blitting from %s %s (%dx%d) to %s %s (%dx%d), swapRB: %d.",
                      apSrc->GetPixelFormat()->GetFormat(), srcRect, apSrc->GetWidth(), apSrc->GetHeight(),
                      apDst->GetPixelFormat()->GetFormat(), dstRect, apDst->GetWidth(), apDst->GetHeight(),
                      shouldSwapRB));

  const tI32 spitch = src->GetPitch();
  const tPtr srcData = src->GetData() + (sx*4) + (sy*spitch);
  const tI32 dpitch = dst->GetPitch();
  tPtr dstData = dst->GetData() + (dx*4) + (dy*dpitch);
  const int r = _ResampleSmooth(dstData, srcData,
                                dw, sw,
                                dh, sh,
                                dpitch, spitch);
  if (r < 0) {
    niError(niFmt("Resampling failed: %d", r));
    return eFalse;
  }

  if (shouldSwapRB) {
    tPtr dline = dstData;
    for (tU32 y = dy; y < dh; ++y) {
      tPtr d = dline;
      for (tU32 x = 0; x < dw; ++x) {
        ni::Swap(d[0],d[2]);
        d += 4;
      }
      dline += dpitch;
    }
  }

  // If dst is no apDst we blit it back in apDst...
  if (apDst != dst) {
    apDst->Blit(dst,0,0,dstRect.x,dstRect.y,dstRect.GetWidth(),dstRect.GetHeight(),ePixelFormatBlit_Normal);
  }

  return eTrue;
}
