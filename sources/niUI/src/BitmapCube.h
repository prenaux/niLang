#ifndef __BITMAPCUBE_11704955_H__
#define __BITMAPCUBE_11704955_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cBitmapCube declaration
class cBitmapCube : public ImplRC<iBitmapCube,eImplFlags_DontInherit1,iBitmapBase>
{
 public:
  cBitmapCube(tU32 ulW, iPixelFormat* pPixFmt, tBool bAllocFaces = eTrue);
  ~cBitmapCube();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  //// iBitmapCube //////////////////////////////
  eBitmapType __stdcall GetType() const { return eBitmapType_Cube; }
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
  //// iBitmapCube //////////////////////////////

 private:
  tU32            mulWidth;
  Ptr<iPixelFormat> mptrPxf;
  Ptr<iBitmap2D>  mptrFaces[6];
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __BITMAPCUBE_11704955_H__
