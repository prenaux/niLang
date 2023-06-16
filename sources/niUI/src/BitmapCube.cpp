// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "BitmapCube.h"

#pragma niTodo("Implement invalidate")

//////////////////////////////////////////////////////////////////////////////////////////////
// cBitmapCube implementation

///////////////////////////////////////////////
cBitmapCube::cBitmapCube(cGraphics* pGraphics, iHString* ahspName, tU32 ulW, iPixelFormat* pPixFmt, tBool bAllocFaces)
{
  ZeroMembers();
  mpGraphics = pGraphics;
  mhspName = ahspName;
  mulWidth = ulW;

  if (!niIsOK(pPixFmt)) {
    niError(_A("Invalid pixel format."));
    return;
  }
  mptrPxf = pPixFmt;

  if (bAllocFaces)
  {
    for (tU32 i = 0; i < 6; ++i) {
      mptrFaces[i] = mpGraphics->CreateBitmap2DEx(ulW, ulW, mptrPxf);
      if (!niIsOK(mptrFaces[i])) {
        niError(niFmt(_A("Can't create face %d."), i));
        return;
      }
    }
  }

  mbOK = eTrue;
}

///////////////////////////////////////////////
cBitmapCube::~cBitmapCube()
{
  if (mpGraphics->GetTextureDeviceResourceManager()) {
    mpGraphics->GetTextureDeviceResourceManager()->Unregister(this);
  }
  for (tU32 i = 0; i < 6; ++i)
    mptrFaces[i] = NULL;
  mptrPxf = NULL;
}

///////////////////////////////////////////////
void cBitmapCube::ZeroMembers()
{
  mptrLockedFace = NULL;
  mbOK = eFalse;
  mpGraphics = NULL;
  mulWidth = 0;
}

///////////////////////////////////////////////
tBool cBitmapCube::IsOK() const
{
  return mbOK;
}

///////////////////////////////////////////////
//! Get the pixel format of the texture.
iPixelFormat* cBitmapCube::GetPixelFormat() const
{
  return mptrPxf;
}

///////////////////////////////////////////////
//! Get the width of the bitmap cube.
tU32 cBitmapCube::GetWidth() const
{
  return mulWidth;
}

///////////////////////////////////////////////
iBitmapCube* __stdcall cBitmapCube::CreateResized(tI32 nW) const
{
  Ptr<iPixelFormat> ptrPxfClone = mptrPxf->Clone();

  cBitmapCube* pNew = niNew cBitmapCube(mpGraphics, NULL, nW, ptrPxfClone, eFalse);
  if (!niIsOK(pNew))
  {
    niError(_A("Can't create new cube bitmap."));
    return NULL;
  }

  for (tU32 i = 0; i < 6; ++i) {
    niAssert(niIsOK(mptrFaces[i]));
    pNew->mptrFaces[i] = mptrFaces[i]->CreateResized(nW, nW);
    if (!niIsOK(pNew->mptrFaces[i])) {
      niError(niFmt(_A("Can't created resized face %d."), i));
      return NULL;
    }
  }

  return pNew;
}

///////////////////////////////////////////////
//! Compute the mipmaps.
tBool cBitmapCube::CreateMipMaps(tU32 anNumMipMap, tBool abCompute)
{
  for (tU32 i = 0; i < 6; ++i) {
    niAssert(niIsOK(mptrFaces[i]));
    if (!mptrFaces[i]->CreateMipMaps(anNumMipMap, abCompute)) {
      niError(niFmt(_A("Can't create mip maps of face %d."), i));
      return eFalse;
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
//! Remove all mip maps.
void cBitmapCube::RemoveMipMaps()
{
  for (tU32 i = 0; i < 6; ++i) {
    niAssert(niIsOK(mptrFaces[i]));
    mptrFaces[i]->RemoveMipMaps();
  }
}

///////////////////////////////////////////////
//! Get the number of mip map.
tU32 cBitmapCube::GetNumMipMaps() const
{
  niAssert(niIsOK(mptrFaces[0]));
  return mptrFaces[0]->GetNumMipMaps();
}

///////////////////////////////////////////////
//! Get the content of the indicated face.
iBitmap2D* cBitmapCube::GetFace(eBitmapCubeFace Face) const
{
  niAssert(Face < 6);
  return mptrFaces[Face];
}

///////////////////////////////////////////////
//! Create a copy of the bitmap.
iBitmapBase* cBitmapCube::Clone(ePixelFormatBlit eBlit) const
{
  Ptr<iPixelFormat> ptrPxfClone = mptrPxf->Clone();

  Ptr<cBitmapCube> ptrOut = niNew cBitmapCube(mpGraphics, NULL, mulWidth, ptrPxfClone, eFalse);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the new cube bitmap."));
    return NULL;
  }

  for (tU32 i = 0; i < 6; ++i)
  {
    ptrOut->mptrFaces[i] = niStaticCast(iBitmap2D*,mptrFaces[i]->Clone(eBlit));
    if (!niIsOK(ptrOut->mptrFaces[i])) {
      niError(niFmt(_A("Can't create copy of face %d."), i));
      return NULL;
    }
  }

  return ptrOut.GetRawAndSetNull();
}

///////////////////////////////////////////////
//! Create a copy of the bitmap that use the given format.
iBitmapBase* cBitmapCube::CreateConvertedFormat(const iPixelFormat* pFmt) const
{
  Ptr<cBitmapCube> ptrOut = niNew cBitmapCube(mpGraphics, NULL, mulWidth, pFmt->Clone(), eFalse);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the out cube bitmap."));
    return NULL;
  }

  for (tU32 i = 0; i < 6; ++i)
  {
    ptrOut->mptrFaces[i] = niStaticCast(iBitmap2D*,mptrFaces[i]->CreateConvertedFormat(ptrOut->mptrPxf));
    if (!niIsOK(ptrOut->mptrFaces[i])) {
      niError(niFmt(_A("Can't create out face %d."), i));
      return NULL;
    }
  }

  return ptrOut.GetRawAndSetNull();
}

///////////////////////////////////////////////
//! Create a gamma corrected bitmap.
iBitmapBase* cBitmapCube::CreateGammaCorrected(float factor) const
{
  Ptr<iPixelFormat> ptrPxfClone = mptrPxf->Clone();

  Ptr<cBitmapCube> ptrOut = niNew cBitmapCube(mpGraphics, NULL, mulWidth, ptrPxfClone, eFalse);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the out cube bitmap."));
    return NULL;
  }

  for (tU32 i = 0; i < 6; ++i) {
    ptrOut->mptrFaces[i] = static_cast<iBitmap2D*>(mptrFaces[i]->CreateGammaCorrected(factor));
    if (!niIsOK(ptrOut->mptrFaces[i])) {
      niError(niFmt(_A("Can't create out face %d."), i));
      return NULL;
    }
  }

  return ptrOut.GetRawAndSetNull();
}

///////////////////////////////////////////////
//! Correct gamma of the bitmap.
tBool cBitmapCube::GammaCorrect(float factor)
{
  for (tU32 i = 0; i < 6; ++i)
  {
    niAssert(niIsOK(mptrFaces[i]));
    if (!mptrFaces[i]->GammaCorrect(factor))
      return eFalse;
  }

  return eTrue;
}
