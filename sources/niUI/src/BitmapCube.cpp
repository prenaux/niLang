// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "BitmapCube.h"
#include "Bitmap2D.h"

#pragma niTodo("Implement invalidate")

//////////////////////////////////////////////////////////////////////////////////////////////
// cBitmapCube implementation

///////////////////////////////////////////////
cBitmapCube::cBitmapCube(tU32 ulW, iPixelFormat* pPixFmt, tBool bAllocFaces)
{
  niPanicAssert(niIsOK(pPixFmt));
  mulWidth = ulW;
  mptrPxf = pPixFmt;

  if (bAllocFaces) {
    for (tU32 i = 0; i < 6; ++i) {
      mptrFaces[i] = niNew cBitmap2D(ulW, ulW, mptrPxf);
      niPanicAssert(mptrFaces[i].IsOK());
    }
  }
}

///////////////////////////////////////////////
cBitmapCube::~cBitmapCube()
{
  for (tU32 i = 0; i < 6; ++i)
    mptrFaces[i] = NULL;
  mptrPxf = NULL;
}

///////////////////////////////////////////////
tBool cBitmapCube::IsOK() const
{
  return eTrue;
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

  cBitmapCube* pNew = niNew cBitmapCube(nW, ptrPxfClone, eFalse);
  if (!niIsOK(pNew)) {
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
iBitmapBase* cBitmapCube::Clone(ePixelFormatBlit aBlitMode) const
{
  Ptr<iPixelFormat> ptrPxfClone = mptrPxf->Clone();

  Ptr<cBitmapCube> ptrOut = niNew cBitmapCube(mulWidth, ptrPxfClone, eFalse);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the new cube bitmap."));
    return NULL;
  }

  for (tU32 i = 0; i < 6; ++i) {
    ptrOut->mptrFaces[i] =
      niStaticCast(iBitmap2D*, mptrFaces[i]->Clone(aBlitMode));
    if (!niIsOK(ptrOut->mptrFaces[i])) {
      niError(niFmt(_A("Can't create copy of face %d."), i));
      return NULL;
    }
  }

  return ptrOut.GetRawAndSetNull();
}

///////////////////////////////////////////////
//! Create a copy of the bitmap that use the given format.
iBitmapBase* cBitmapCube::CreateConvertedFormat(const iPixelFormat* apFmt) const
{
  niCheckIsOK(apFmt, NULL);
  Ptr<iPixelFormat> ptrPxfClone = apFmt->Clone();

  Ptr<cBitmapCube> ptrOut = niNew cBitmapCube(mulWidth, ptrPxfClone, eFalse);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the out cube bitmap."));
    return NULL;
  }

  for (tU32 i = 0; i < 6; ++i) {
    ptrOut->mptrFaces[i] = niStaticCast(
      iBitmap2D*, mptrFaces[i]->CreateConvertedFormat(ptrOut->mptrPxf));
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

  Ptr<cBitmapCube> ptrOut = niNew cBitmapCube(mulWidth, ptrPxfClone, eFalse);
  if (!niIsOK(ptrOut)) {
    niError(_A("Can't allocate the out cube bitmap."));
    return NULL;
  }

  for (tU32 i = 0; i < 6; ++i) {
    ptrOut->mptrFaces[i] =
      static_cast<iBitmap2D*>(mptrFaces[i]->CreateGammaCorrected(factor));
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
  for (tU32 i = 0; i < 6; ++i) {
    niAssert(niIsOK(mptrFaces[i]));
    if (!mptrFaces[i]->GammaCorrect(factor))
      return eFalse;
  }

  return eTrue;
}
