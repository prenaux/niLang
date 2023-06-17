// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "Bitmap3D.h"
#include "Bitmap2D.h"

#pragma niTodo("Implement invalidate")

template <typename T>
inline tBool ClipDim(T& p, T& s, const T aP, const T aSz, const T aMaxSz) {
  p = aP; s = aSz==0?aMaxSz:aSz;
  if (p >= aMaxSz) return eFalse; // outsize of size
  if (p < 0) {
    T d = 0-p;
    p = 0;
    s -= d;
  }
  if (p+s > aMaxSz) {
    T d = (p+s)-aMaxSz;
    s -= d;
  }
  if (s <= 0) return eFalse;
  return eTrue;
}

template <typename T>
inline tBool ClipBlitBox(sVec3i& aClPos, sVec3i& aClSz,
                         T* apSrc, const tU32 anLevel,
                         const sVec3i& aPos, const sVec3i& aSz) {
  tI32 nWidth = (tI32)apSrc->GetWidth();
  tI32 nHeight = (tI32)apSrc->GetHeight();
  tI32 nDepth = (tI32)apSrc->GetDepth();
  if (anLevel != eInvalidHandle && anLevel != 0) {
    nWidth >>= anLevel;
    nHeight >>= anLevel;
    nDepth >>= anLevel;
  }

  if (!ClipDim(aClPos.x,aClSz.x,aPos.x,aSz.x,nWidth)) return eFalse;
  if (!ClipDim(aClPos.y,aClSz.y,aPos.y,aSz.y,nHeight)) return eFalse;
  if (!ClipDim(aClPos.z,aClSz.z,aPos.z,aSz.z,nDepth)) return eFalse;

  return eTrue;
}

///////////////////////////////////////////////
// Return the number of mipmaps to create.
inline tU32 GetNumMipMaps3D(tU32 w, tU32 h, tU32 d, tU32 minw, tU32 minh, tU32 mind)
{
  if(minw == 0) minw = 1;
  if(minh == 0) minh = 1;
  if(mind == 0) mind = 1;
  tU32 i = 0;
  while (1) {
    if(w <= minw && h <= minh && d <= mind)
      return 0;
    w >>= 1;
    h >>= 1;
    d >>= 1;
    ++i;
  }
  return i;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cBitmap3D implementation

///////////////////////////////////////////////
cBitmap3D::cBitmap3D(cGraphics* pGraphics, iHString* ahspName, tU32 ulW, tU32 ulH, tU32 ulD, iPixelFormat* pPixFmt, tBool abAllocateData)
{
  ZeroMembers();
  mhspName = ahspName;
  mpGraphics = pGraphics;
  mulWidth = ulW;
  mulHeight = ulH;
  mulDepth = ulD;

  if (!niIsOK(pPixFmt)) {
    niError(_A("Invalid pixel format."));
    return;
  }
  mptrPxf = pPixFmt;

  mnRowPitch = mulWidth * mptrPxf->GetBytesPerPixel();
  mnSlicePitch = mnRowPitch * mulHeight;
  mnSize = mptrPxf->GetSize(mulWidth,mulHeight,mulDepth);

  if (abAllocateData) {
    mptrData = (tPtr)niMalloc(mnSize);
    if (!mptrData) {
      niError(_A("Can't allocate data."));
      return;
    }
  }

  mbOK = eTrue;
}

///////////////////////////////////////////////
cBitmap3D::~cBitmap3D()
{
  if (mbFreeData && mptrData) {
    niFree(mptrData);
    mptrData = NULL;
  }
  mptrPxf = NULL;
}

///////////////////////////////////////////////
void cBitmap3D::ZeroMembers()
{
  mbOK = eFalse;
  mpGraphics = NULL;
  mulWidth = 0;
  mulHeight = 0;
  mulDepth = 0;
  mptrData = NULL;
  mbFreeData = eFalse;
  mnRowPitch = 0;
  mnSlicePitch = 0;
  mnSize = 0;
}

///////////////////////////////////////////////
tBool cBitmap3D::IsOK() const
{
  return mbOK;
}

///////////////////////////////////////////////
iPixelFormat* cBitmap3D::GetPixelFormat() const
{
  return mptrPxf;
}

///////////////////////////////////////////////
tU32 cBitmap3D::GetWidth() const
{
  return mulWidth;
}

///////////////////////////////////////////////
tU32 cBitmap3D::GetHeight() const
{
  return mulHeight;
}

///////////////////////////////////////////////
tU32 cBitmap3D::GetDepth() const
{
  return mulDepth;
}

///////////////////////////////////////////////
tBool cBitmap3D::CreateMipMaps(tU32 anNumMipMaps, tBool abCompute)
{
  RemoveMipMaps();

  if (abCompute && (!IsPow2(mulWidth) || !IsPow2(mulHeight) || !IsPow2(mulDepth))) {
    niWarning(niFmt("Can't compute mip maps of npot 3d bitmaps: %dx%dx%d.",
                    mulWidth, mulHeight, mulDepth));
    return eFalse;
  }

  tU32 ulNumMipMaps = ni::Min(
      GetNumMipMaps3D(GetWidth()>>1,GetHeight()>>1,GetDepth()>>1,1,1,1),
      (anNumMipMaps ?
       anNumMipMaps :
       GetNumMipMaps3D(GetWidth()>>1,GetHeight()>>1,GetDepth()>>1,4,4,4)));
  if (!ulNumMipMaps)
  {
    niWarning(niFmt(_A("No mip map required for resolution %dx%dx%d."),GetWidth(),GetHeight(),GetDepth()));
    return eTrue;
  }


  if (abCompute)
  {
    // create the first mipmap
    iBitmap3D* pMip0 = CreateResized(GetWidth()>>1,
                                     GetHeight()>>1,
                                     GetDepth()>>1);
    if (!niIsOK(pMip0)) {
      RemoveMipMaps();
      niWarning(_A("Can't compute mipmap 0."));
      return eFalse;
    }

    _ResizeMipMapsVector(ulNumMipMaps);
    mvMipMaps[0] = pMip0;
    for (tU32 i = 1; i < GetNumMipMaps(); ++i)
    {
      iBitmap3D* srcMip = GetMipMap(i-1);
      mvMipMaps[i] = srcMip->CreateResized(
          srcMip->GetWidth()>>1,
          srcMip->GetHeight()>>1,
          srcMip->GetDepth()>>1);
      if (!niIsOK(mvMipMaps[i]))  {
        RemoveMipMaps();
        niError(niFmt(_A("Can't compute mipmap %d."), i));
        return eFalse;
      }
    }
  }
  else
  {
    _ResizeMipMapsVector(ulNumMipMaps);

    // create the first mipmap
    mvMipMaps[0] = mpGraphics->CreateBitmap3DMemoryEx(
        GetWidth()>>1,GetHeight()>>1,GetDepth()>>1,
        GetPixelFormat()->Clone(), 0, 0,
        NULL, eTrue);
    if (!niIsOK(mvMipMaps[0]))
    {
      RemoveMipMaps();
      niWarning(_A("Can't create mipmap 0."));
      return eFalse;
    }

    for (tU32 i = 1; i < GetNumMipMaps(); ++i)
    {
      mvMipMaps[i] = mpGraphics->CreateBitmap3DMemoryEx(
          mvMipMaps[i-1]->GetWidth()>>1,
          mvMipMaps[i-1]->GetHeight()>>1,
          mvMipMaps[i-1]->GetHeight()>>1,
          GetPixelFormat()->Clone(), 0, 0,
          NULL, eTrue);
      if (!niIsOK(mvMipMaps[i]))
      {
        RemoveMipMaps();
        niWarning(niFmt(_A("Can't create mipmap %d."), i));
        return eFalse;
      }
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
void cBitmap3D::RemoveMipMaps()
{
  mvMipMaps.clear();
}

///////////////////////////////////////////////
tU32 cBitmap3D::GetNumMipMaps() const
{
  return (tU32)mvMipMaps.size();
}

///////////////////////////////////////////////
//! Create a copy of the bitmap.
iBitmapBase* cBitmap3D::Clone(ePixelFormatBlit aBlitMode) const
{
  Ptr<iPixelFormat> ptrPxfClone = mptrPxf->Clone();

  Ptr<cBitmap3D> ptrOut = niNew cBitmap3D(mpGraphics, NULL,
                                          mulWidth, mulHeight, mulDepth,
                                          ptrPxfClone, eTrue);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the out cube bitmap."));
    return NULL;
  }

  if (!((iBitmap3D*)ptrOut.ptr())->Blit(this)) {
    niError(_A("Blitting failed."));
    return NULL;
  }

  if (GetNumMipMaps()) {
    ptrOut->_ResizeMipMapsVector(GetNumMipMaps());
    niLoop(i,GetNumMipMaps()) {
      ptrOut->mvMipMaps[i] = (iBitmap3D*)this->mvMipMaps[i]->Clone(aBlitMode);
      if (!ptrOut->mvMipMaps[i].IsOK()) {
        niError(niFmt(_A("Can't process mipmap '%d'."),i));
        return NULL;
      }
    }
  }

  return ptrOut.GetRawAndSetNull();
}

///////////////////////////////////////////////
//! Create a copy of the bitmap that use the given format.
iBitmapBase* cBitmap3D::CreateConvertedFormat(const iPixelFormat* apFmt) const
{
  niCheckIsOK(apFmt,NULL);
  Ptr<iPixelFormat> ptrPxfClone = apFmt->Clone();

  Ptr<cBitmap3D> ptrOut = niNew cBitmap3D(mpGraphics, NULL,
                                          mulWidth, mulHeight, mulDepth,
                                          ptrPxfClone, eTrue);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the out cube bitmap."));
    return NULL;
  }

  if (!((iBitmap3D*)ptrOut.ptr())->Blit(this)) {
    niError(_A("Blitting failed."));
    return NULL;
  }

  if (GetNumMipMaps()) {
    ptrOut->_ResizeMipMapsVector(GetNumMipMaps());
    niLoop(i,GetNumMipMaps()) {
      ptrOut->mvMipMaps[i] = (iBitmap3D*)this->mvMipMaps[i]->CreateConvertedFormat(apFmt);
      if (!ptrOut->mvMipMaps[i].IsOK()) {
        niError(niFmt(_A("Can't process mipmap '%d'."),i));
        return NULL;
      }
    }
  }

  return ptrOut.GetRawAndSetNull();
}

///////////////////////////////////////////////
//! Create a gamma corrected bitmap.
iBitmapBase* cBitmap3D::CreateGammaCorrected(tF32 factor) const
{
  Ptr<iPixelFormat> ptrPxfClone = mptrPxf->Clone();

  Ptr<cBitmap3D> ptrOut = niNew cBitmap3D(mpGraphics, NULL,
                                          mulWidth, mulHeight, mulDepth,
                                          ptrPxfClone, eFalse);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the out cube bitmap."));
    return NULL;
  }

  if (!ptrOut->GammaCorrect(factor)) {
    niError(_A("Gamme correction failed."));
    return NULL;
  }


  if (GetNumMipMaps()) {
    ptrOut->_ResizeMipMapsVector(GetNumMipMaps());
    niLoop(i,GetNumMipMaps()) {
      ptrOut->mvMipMaps[i] = (iBitmap3D*)this->mvMipMaps[i]->CreateGammaCorrected(factor);
      if (!ptrOut->mvMipMaps[i].IsOK()) {
        niError(niFmt(_A("Can't process mipmap '%d'."),i));
        return NULL;
      }
    }
  }

  return ptrOut.GetRawAndSetNull();
}

///////////////////////////////////////////////
//! Correct gamma of the bitmap.
tBool cBitmap3D::GammaCorrect(float factor)
{
  niLoop(i,mulDepth) {
    ni::Ptr<iBitmap2D> bmp = _GetSliceBmp(i);
    if (!bmp.IsOK() || !bmp->GammaCorrect(factor))
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool cBitmap3D::BeginUnpackPixels()
{
  return eTrue;
}

///////////////////////////////////////////////
void  cBitmap3D::EndUnpackPixels()
{
}

///////////////////////////////////////////////
iBitmap3D* cBitmap3D::GetMipMap(tU32 ulIdx) const
{
  niCheckSilent(ulIdx < GetNumMipMaps(),NULL);
  return mvMipMaps[ulIdx];
}

///////////////////////////////////////////////
iBitmap3D* __stdcall cBitmap3D::GetLevel(tU32 anIndex) const
{
  if (anIndex == 0) return niConstCast(cBitmap3D*,this);
  return GetMipMap(anIndex-1);
}

///////////////////////////////////////////////
void cBitmap3D::_ResizeMipMapsVector(tU32 aulNumMipMaps)
{
  RemoveMipMaps();
  mvMipMaps.resize(aulNumMipMaps);
}

///////////////////////////////////////////////
tPtr        cBitmap3D::_GetSlicePtr(tU32 anSlice) const
{
  return mptrData + anSlice * mnSlicePitch;
}

///////////////////////////////////////////////
iBitmap2D*  cBitmap3D::_GetSliceBmp(tU32 anSlice) const
{
  return niNew cBitmap2D(mpGraphics,NULL,mulWidth,mulHeight,mptrPxf,
                         mnRowPitch, _GetSlicePtr(anSlice), eFalse);
}

///////////////////////////////////////////////
tBool __stdcall cBitmap3D::SetMemoryAddress(tPtr apAddr, tBool abFreeAddr, tU32 anRowPitch, tU32 anSlicePitch)
{
  if (mptrData == apAddr) {
    // do nothing...
  }
  else {
    if (mbFreeData && mptrData) {
      niFree(mptrData);
      mptrData = NULL;
    }
    mptrData = apAddr;
    mbFreeData = abFreeAddr;
  }
  if (anRowPitch != eInvalidHandle)
    mnRowPitch = anRowPitch;
  if (anSlicePitch != eInvalidHandle)
    mnSlicePitch = anSlicePitch;
  return eTrue;
}

///////////////////////////////////////////////
tU32  __stdcall cBitmap3D::GetRowPitch() const
{
  return mnRowPitch;
}

///////////////////////////////////////////////
tU32  __stdcall cBitmap3D::GetSlicePitch() const
{
  return mnSlicePitch;
}

///////////////////////////////////////////////
tPtr  __stdcall cBitmap3D::GetData() const
{
  return mptrData;
}

///////////////////////////////////////////////
tU32  __stdcall cBitmap3D::GetSize() const
{
  return mnSize;
}

///////////////////////////////////////////////
iBitmap3D* __stdcall cBitmap3D::CreateResized(tU32 anW, tU32 anH, tU32 anD) const
{
#pragma niTodo("IMPLEMENT")
  return NULL;
}

///////////////////////////////////////////////
tBool __stdcall cBitmap3D::Blit(const iBitmap3D* src, const sVec3i& avSrcMin, const sVec3i& avDestMin, const sVec3i& avSize)
{
  niCheckSilent(niIsOK(src),eFalse);
  if (mptrPxf->GetCaps()&ePixelFormatCaps_BlockCompressed)
    return eFalse; // not supported...

  sVec3i srcP, dstP, srcS, dstS;
  if (!ClipBlitBox(srcP,srcS,src,0,avSrcMin,avSize)) return eFalse;
  if (!ClipBlitBox(dstP,dstS,src,0,avSrcMin,avSize)) return eFalse;
  sVec3i sz;
  ni::VecMinimize(sz,dstS,srcS);

  iPixelFormat* srcPxf = src->GetPixelFormat();
  tPtr srcData = src->GetData();
  tU32 srcRowPitch = src->GetRowPitch();
  tU32 srcSlPitch = src->GetSlicePitch();
  niLoop(k,sz.z) {
    tPtr srcSl = srcData  + (k+srcP.z)*srcSlPitch;
    tPtr dstSl = mptrData + (k+dstP.z)*mnSlicePitch;
    mptrPxf->Blit(dstSl,mnRowPitch,dstP.x,dstP.y,
                  srcSl,srcRowPitch,srcPxf,srcP.x,srcP.y,
                  sz.x,sz.y,ePixelFormatBlit_Normal);
  }

  return eTrue;
}

///////////////////////////////////////////////
void __stdcall cBitmap3D::PutPixel(const sVec3i& avPos, tPtr col)
{
  if (mptrPxf->GetCaps()&ePixelFormatCaps_BlockCompressed)
    return; // not supported...
  if (avPos.x < 0 || avPos.x >= (tI32)mulWidth ||
      avPos.y < 0 || avPos.y >= (tI32)mulHeight ||
      avPos.z < 0 || avPos.z >= (tI32)mulDepth)
    return;

  tU32 i;
  tU8* pPtr = (tU8*)col;
  tU32 bypp = mptrPxf->GetBytesPerPixel();
  tU8* pDest = mptrData + (avPos.z*mnSlicePitch) + (avPos.y*mnRowPitch) + (avPos.x*bypp);
  for (i = 0; i < bypp; ++i)
    *pDest++ = *pPtr++;
}

///////////////////////////////////////////////
tPtr __stdcall cBitmap3D::GetPixel(const sVec3i& avPos, tPtr pOut) const
{
  if (!pOut || mptrPxf->GetCaps()&ePixelFormatCaps_BlockCompressed)
    return pOut; // not supported...
  if (avPos.x < 0 || avPos.x >= (tI32)mulWidth ||
      avPos.y < 0 || avPos.y >= (tI32)mulHeight ||
      avPos.z < 0 || avPos.z >= (tI32)mulDepth)
    return pOut;

  tU32 i;
  tU32 bypp = mptrPxf->GetBytesPerPixel();
  tU8* pSrc = mptrData + (avPos.z*mnSlicePitch) + (avPos.y*mnRowPitch) + (avPos.x*bypp);
  for (i = 0; i < bypp; ++i)
    *pOut++ = *pSrc++;

  return pOut;
}

///////////////////////////////////////////////
void __stdcall cBitmap3D::Clear(tPtr pColor)
{
  if (!pColor) {
    if (mptrPxf->GetCaps()&ePixelFormatCaps_BlockCompressed) {
      memset(mptrData,0,mnSize);
    }
    else {
      niLoop(k,mulDepth) {
        tPtr ptrSl = _GetSlicePtr(k);
        niLoop(j,mulHeight) {
          tPtr ptrRow = ptrSl + j*mnRowPitch;
          memset(ptrRow,0,mnRowPitch);
        }
      }
    }
  }
  else {
    if (mptrPxf->GetCaps()&ePixelFormatCaps_BlockCompressed)
      return;
    tU32 bypp = mptrPxf->GetBytesPerPixel();
    niLoop(k,mulDepth) {
      tPtr ptrSl = _GetSlicePtr(k);
      niLoop(j,mulHeight) {
        tPtr ptrRow = ptrSl + j*mnRowPitch;
        niLoop(i,bypp) {
          *ptrRow++ = ((tU8*)(pColor))[i];
        }
      }
    }
  }
}

///////////////////////////////////////////////
void __stdcall cBitmap3D::ClearBox(const sVec3i& avMin, const sVec3i& avSize, tPtr pColor)
{
  sVec3i dstP, dstS;
  if (!ClipBlitBox(dstP,dstS,this,0,avMin,avSize))
    return;

  if (mptrPxf->GetCaps()&ePixelFormatCaps_BlockCompressed)
    return; // not supported...

  if (pColor) {
    for (tI32 k = dstP.z; k < dstP.z+dstS.z; ++k) {
      tPtr ptrSl = _GetSlicePtr(k);
      for (tI32 j = dstP.y; j < dstP.y+dstS.y; ++j) {
        tPtr ptrRow = ptrSl + j*mnRowPitch + dstP.x;
        niLoop(i,dstS.x) {
          *ptrRow++ = ((tU8*)(pColor))[i];
        }
      }
    }
  }
  else {
    tU32 bypp = mptrPxf->GetBytesPerPixel();
    for (tI32 k = dstP.z; k < dstP.z+dstS.z; ++k) {
      tPtr ptrSl = _GetSlicePtr(k);
      for (tI32 j = dstP.y; j < dstP.y+dstS.y; ++j) {
        tPtr ptrRow = ptrSl + j*mnRowPitch + dstP.x;
        memset(ptrRow,0,bypp*dstS.x);
      }
    }
  }
}

///////////////////////////////////////////////
void __stdcall cBitmap3D::PutPixelf(const sVec3i& avPos, const sColor4f& col)
{
  niDeclareTempPixel();
  mptrPxf->BuildPixelf(niTempPixelPtr(),col.x,col.y,col.z,col.w);
  PutPixel(avPos,niTempPixelPtr());
}

///////////////////////////////////////////////
sColor4f __stdcall cBitmap3D::GetPixelf(const sVec3i& avPos) const
{
  niDeclareTempPixel();
  tPtr pix = GetPixel(avPos,niTempPixelPtr());
  if (!pix) return sColor4f::Zero();
  return mptrPxf->UnpackPixelf(pix);
}

///////////////////////////////////////////////
void __stdcall cBitmap3D::Clearf(const sColor4f& col)
{
  if (col == sColor4f::Zero()) {
    Clear(NULL);
  }
  else {
    niDeclareTempPixel();
    mptrPxf->BuildPixelf(niTempPixelPtr(),col.x,col.y,col.z,col.w);
    Clear(niTempPixelPtr());
  }
}

///////////////////////////////////////////////
void __stdcall cBitmap3D::ClearBoxf(const sVec3i& avMin, const sVec3i& avSize, const sColor4f& col)
{
  niDeclareTempPixel();
  mptrPxf->BuildPixelf(niTempPixelPtr(),col.x,col.y,col.z,col.w);
  ClearBox(avMin,avSize,niTempPixelPtr());
}
