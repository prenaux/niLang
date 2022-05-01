#ifndef __BITMAPCUBE_11704955_H__
#define __BITMAPCUBE_11704955_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cBitmapCube declaration
class cBitmapCube : public cIUnknownImpl<iBitmapCube,
                                         eIUnknownImplFlags_DontInherit1|
                                         eIUnknownImplFlags_DontInherit3,
                                         iBitmapBase,iTexture,iDeviceResource>
{
 public:
  cBitmapCube(cGraphics* pGraphics, iHString* ahspName, tU32 ulW, iPixelFormat* pPixFmt, tBool bAllocFaces = eTrue);
  ~cBitmapCube();

  void ZeroMembers();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  //// iBitmapCube //////////////////////////////
  eBitmapType __stdcall GetType() const { return eBitmapType_Cube; }
  iHString* __stdcall GetDeviceResourceName() const {
    return ni::HStringIsEmpty(mhspName)?(iHString*)NULL:mhspName.ptr();
  }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }
  iPixelFormat* __stdcall GetPixelFormat() const;
  tU32 __stdcall GetWidth() const;
  tU32 __stdcall GetHeight() const { return GetWidth(); }
  tU32 __stdcall GetDepth() const { return GetWidth(); }
  iBitmapCube* __stdcall CreateResized(tI32 nW) const;
  tBool __stdcall CreateMipMaps(tU32 anNumMipMap, tBool abCompute);
  void __stdcall RemoveMipMaps();
  tU32 __stdcall GetNumMipMaps() const;
  iBitmap2D* __stdcall GetFace(eBitmapCubeFace Face) const;
  iBitmapBase* __stdcall Clone(ePixelFormatBlit eBlit) const;
  iBitmapBase* __stdcall CreateConvertedFormat(const iPixelFormat* pFmt) const;
  iBitmapBase* __stdcall CreateGammaCorrected(float factor) const;
  tBool __stdcall GammaCorrect(float factor);

  tTextureFlags __stdcall GetFlags() const {
    return eTextureFlags_SystemMemory|
        eTextureFlags_Dynamic|
        (GetNumMipMaps()?eTextureFlags_MipMaps:0);
  }

  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    if (anIndex >= 6) return NULL;
    return ni::QueryInterface<iTexture>(mptrFaces[anIndex]);
  }

  virtual iDeviceResource* __stdcall Bind(iUnknown*) { return this; }
  //// iBitmapCube //////////////////////////////

 private:
  tBool   mbOK;
  tHStringPtr mhspName;
  cGraphics*  mpGraphics;
  tU32    mulWidth;
  Ptr<iPixelFormat> mptrPxf;
  Ptr<iBitmap2D>  mptrFaces[6];
  Ptr<iTexture>   mptrLockedFace;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __BITMAPCUBE_11704955_H__
