#ifndef __BITMAP2D_H__
#define __BITMAP2D_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Graphics.h"

#define BMPFLAGS_FREEDATA niBit(0)
#define BMPFLAGS_LOCKED niBit(1)

//////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
namespace ni {
class cGraphics;

//////////////////////////////////////////////////////////////////////////////////////////////
class cBitmap2D
    : public ImplRC<iBitmap2D, eImplFlags_DontInherit1, iBitmapBase> {
 public:
  cBitmap2D(tU32 nW, tU32 nH, iPixelFormat* pPixFmt, tU32 anPitch = 0,
            tPtr ptrAddr = NULL, tBool bFreeAddr = eFalse);
  virtual ~cBitmap2D();

  tBool __stdcall IsOK() const;

  tTextureFlags __stdcall GetFlags() const;
  iBitmap2D* __stdcall Lock(eLock aLock, tU32 anLevel, tU32 anFaceSlice);
  void __stdcall Unlock();
  tBool __stdcall GetIsLocked() const;

  //! Return the bitmap type.
  eBitmapType __stdcall GetType() const
  {
    return eBitmapType_2D;
  }

  // Infos functions1
  tU32 __stdcall GetWidth() const;
  tU32 __stdcall GetHeight() const;
  tU32 __stdcall GetDepth() const;
  tU32 __stdcall GetPitch() const;
  tPtr __stdcall GetData() const;
  tU32 __stdcall GetSize() const;

  // Conversion functions
  iBitmapBase* __stdcall Clone(
    ePixelFormatBlit eBlit = ePixelFormatBlit_Normal) const;
  iBitmapBase* __stdcall CreateConvertedFormat(const iPixelFormat* pFmt) const;
  iBitmapBase* __stdcall CreateGammaCorrected(float factor) const;
  tBool __stdcall GammaCorrect(float factor);
  iBitmap2D* __stdcall CreateResized(tI32 nW, tI32 nH) const;
  iBitmap2D* __stdcall CreateResizedEx(tI32 nW, tI32 nH, tBool abMipMaps) const;
  iBitmapCube* __stdcall CreateCubeBitmap(
    tU32 anWidth, const sVec2i& avPX, ePixelFormatBlit aBlitPX,
    const sVec2i& avNX, ePixelFormatBlit aBlitNX, const sVec2i& avPY,
    ePixelFormatBlit aBlitPY, const sVec2i& avNY, ePixelFormatBlit aBlitNY,
    const sVec2i& avPZ, ePixelFormatBlit aBlitPZ, const sVec2i& avNZ,
    ePixelFormatBlit aBlitNZ) const;
  iBitmapCube* __stdcall CreateCubeBitmapCross() const;

  // Pixel format
  iPixelFormat* __stdcall GetPixelFormat() const;
  tBool __stdcall BeginUnpackPixels();
  void __stdcall EndUnpackPixels();

  // Mip maps
  tBool __stdcall CreateMipMaps(tU32 anNumMipMaps, tBool abCompute);
  void __stdcall RemoveMipMaps();
  tU32 __stdcall GetNumMipMaps() const;
  iBitmap2D* __stdcall GetMipMap(tU32 ulIdx) const;
  iBitmap2D* __stdcall GetLevel(tU32 anIndex) const;

  // Blitting
  tBool __stdcall Blit(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd, tI32 yd,
                       tI32 w, tI32 h, ePixelFormatBlit eBlit);
  tBool __stdcall BlitStretch(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd,
                              tI32 yd, tI32 ws, tI32 hs, tI32 wd, tI32 hd);
  tBool __stdcall BlitAlpha(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd,
                            tI32 yd, tI32 w, tI32 h, ePixelFormatBlit eBlit,
                            const sColor4f& acolSource,
                            const sColor4f& acolDest, eBlendMode aBlendMode);
  tBool __stdcall BlitAlphaStretch(const iBitmap2D* src, tI32 xs, tI32 ys,
                                   tI32 xd, tI32 yd, tI32 ws, tI32 hs, tI32 wd,
                                   tI32 hd, const sColor4f& acolSource,
                                   const sColor4f& acolDest,
                                   eBlendMode aBlendMode);

  // Primitives
  void __stdcall PutPixel(tI32 x, tI32 y, tPtr col);
  tPtr __stdcall GetPixel(tI32 x, tI32 y, tPtr pOut) const;
  void __stdcall Clear(tPtr pColor);
  void __stdcall ClearRect(const sRecti& aRect, tPtr pColor);

  void __stdcall PutPixelf(tI32 x, tI32 y, const sColor4f& col);
  sColor4f __stdcall GetPixelf(tI32 x, tI32 y) const;
  void __stdcall Clearf(const sColor4f& col);
  void __stdcall ClearRectf(const sRecti& aRect, const sColor4f& col);

  //! Set the memory address.
  tBool __stdcall SetMemoryAddress(tPtr apAddr, tBool abFreeAddr, tU32 anPitch);

 protected:
  tBool _Setup(int nW, int nH, tU32 anPitch, tPtr ptrAddr = NULL,
               tBool bFreeAddr = eFalse);
  void _ResizeMipMapsVector(tU32 aulNumMipMaps);

 protected:
  Ptr<iPixelFormat> mptrPixFmt;
  astl::vector<Ptr<iBitmap2D>> mvMipMaps;
  tU32 mulWidth;
  tU32 mulHeight;
  tU32 mulPitch;
  tU32 mulSize;
  tU8* mpData;
  tU8* mpOldData;
  tU8 mFlags;
};
} // end of namespace ni

void BmpUtils_BlitPaletteTo24Bits(tPtr apDest, const tPtr apSrc,
                                  const tU32 anNumSrcPixels,
                                  const tU32* apPalette);
void BmpUtils_BlitPaletteTo32Bits(tPtr apDest, const tPtr apSrc,
                                  const tU32 anNumSrcPixels,
                                  const tU32* apPalette);

#endif // __BITMAP_H__
