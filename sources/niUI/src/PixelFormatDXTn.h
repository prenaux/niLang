#ifndef __PIXELFORMATDXTN_35460993_H__
#define __PIXELFORMATDXTN_35460993_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niUI/IPixelFormat.h"

#if defined niEmbedded
// #define PIXELFORMAT_NO_DXT
#define PIXELFORMAT_NO_DXT_COMPRESSION
#endif

#if !defined PIXELFORMAT_NO_DXT

//////////////////////////////////////////////////////////////////////////////////////////////
// Flags
#define DXTFLAG_1   0x80000000
#define DXTFLAG_3   0x40000000
#define DXTFLAG_5   0x20000000
#define DXTFLAG_ALPHA1  0x08000000

//////////////////////////////////////////////////////////////////////////////////////////////
// cPixelFormatDXTn declaration
class cPixelFormatDXTn : public cIUnknownImpl<iPixelFormat>
{
 public:
  cPixelFormatDXTn(const achar* cszFormat);
  ~cPixelFormatDXTn();

  void ZeroMembers();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  tU32 __stdcall GetNumParameters() const;
  tBool __stdcall SetParameter(tU32 ulParameter, tU32 ulValue);
  tU32 __stdcall GetParameter(tU32 ulParameter) const;

  tBool __stdcall IsSamePixelFormat(const iPixelFormat* pPixFmt) const;

  iPixelFormat* __stdcall Clone() const;

  const achar* __stdcall GetFormat() const;
  ePixelFormatCaps __stdcall GetCaps() const;

  tU32 __stdcall GetBitsPerPixel() const;
  tU32 __stdcall GetBytesPerPixel() const;
  tU32 __stdcall GetSize(tU32 ulW, tU32 ulH, tU32 ulD) const;
  tU32 __stdcall GetNumComponents() const;
  tU32 __stdcall GetNumRBits() const;
  tU32 __stdcall GetNumGBits() const;
  tU32 __stdcall GetNumBBits() const;
  tU32 __stdcall GetNumABits() const;

  tPtr __stdcall BuildPixelub(tPtr pOut, tU8 r, tU8 g, tU8 b, tU8 a = 0) const;
  tPtr __stdcall BuildPixelus(tPtr pOut, tU16 r, tU16 g, tU16 b, tU16 a = 0) const  { return BuildPixelub(pOut,tU8(r>>8),   tU8(g>>8),    tU8(b>>8),    tU8(a>>8)); }
  tPtr __stdcall BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a = 0) const  { return BuildPixelub(pOut,tU8(r>>24),    tU8(g>>24),   tU8(b>>24),   tU8(a>>24)); }
  tPtr __stdcall BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a = 0.0f) const {  return BuildPixelub(pOut,tU8(r*255.0f), tU8(g*255.0f),  tU8(b*255.0f),  tU8(a*255.0f)); }

  tPtr __stdcall BeginUnpackPixels(tPtr pSurface, tU32 ulPitch, tU32 ulX, tU32 ulY, tU32 ulW, tU32 ulH);
  void __stdcall EndUnpackPixels();
  sColor4ub __stdcall UnpackPixelub(tPtr pColor) const;
  sVec4i __stdcall UnpackPixelus(tPtr pColor) const;
  sVec4i __stdcall UnpackPixelul(tPtr pColor) const;
  sColor4f  __stdcall UnpackPixelf(tPtr pColor) const;

  tBool __stdcall Clear(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy, tU32 w, tU32 h, tPtr apClearColor) {
    return eFalse;
  }

  tBool __stdcall Blit(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy,
                       tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy,
                       tU32 w, tU32 h, ePixelFormatBlit blitFlags) const;

  tBool __stdcall BlitStretch(tPtr pDst, tU32 ulDestPitch, tU32 ulDestWidth, tU32 ulDestHeight, tU32 dx, tU32 dy, tU32 dw, tU32 dh,
                              tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy, tU32 sw, tU32 sh) const;

  tBool __stdcall BlitAlpha(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy,
                            tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy,
                            tU32 w, tU32 h, ePixelFormatBlit blitFlags,
                            const sColor4f& aSourceColor, const sColor4f& aDestColor, eBlendMode aBlendMode) const {
    return eFalse;
  }

  tBool __stdcall BlitAlphaStretch(tPtr pDst, tU32 ulDestPitch, tU32 ulDestWidth, tU32 ulDestHeight, tU32 dx, tU32 dy, tU32 dw, tU32 dh,
                                   tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy, tU32 sw, tU32 sh,
                                   const sColor4f& aSourceColor, const sColor4f& aDestColor, eBlendMode aBlendMode) const {
    return eFalse;
  }

 private:
  tU32   mulFlags;
  tPtr   mpUnpackBuffer;
};

#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __PIXELFORMATDXTN_35460993_H__
