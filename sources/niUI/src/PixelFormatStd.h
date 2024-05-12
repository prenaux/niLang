#ifndef __PIXELFORMATSTD_35460993_H__
#define __PIXELFORMATSTD_35460993_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niUI/IPixelFormat.h"

/** The pixel format description is very flexible, you can specify up to four component
 * per pixel format with up to 32 bits per component for a total of maximum 64 bits
 * (8 bytes) per pixel.<br>
 * The description take this form, with the less significant bits first :<br>
 *  [CompLetter][NumBits][CompLetter][NumBits][CompLetter][NumBits][CompLetter][NumBits]<br>
 * CompLetter is the letter which represent the component which the following number of
 * bits describe.<br>
 * These letters can be:<br>
 * - A, Q, X for the Alpha Component
 * - R, W, L, P for the Red Component
 * - G, V for the Green Component
 * - B, U for the Blue Component
 * Examples:<br>
 * - "B5G6R5" is a 16bits per pixel format with 5bits for the blue, 6bits for the green and 5bits for the red.
 * - "B8G8R8A8" is a 32bits per pixel ARGB format with 8bits for each pixel.
 * - "Q8W8V8U8" is a 32bits format used for normal maps.
 * - "G16R16" is a 32bits format with 16bits per component.
 * Remarks:<br>
 * - BGR is the same order as the one that use DirectX8, because the DirectX formats
 *   are named with the most significant bits first.
 * - Palettized/indexed format is "P8", its not supported anymore, the bitmap loaders should convert it to RGBA32.
 * - Q,W,V,U are signed components.
 */

// The definition rules are :
// - At least two characters.
// - Two non digit letter cannot be together : ex: 'AR8' is invalid.
// - Letters different of  A, Q, X, R, W, L, P, G, V, B, and U are invalid.

#define INFO32_EQUAL()                                                  \
  sInfo32& operator = (const sInfo64& inf)                              \
  {                                                                     \
    ulAMask = (tU32)inf.qAMask; ulALShift = inf.ulALShift; ulARShift = inf.ulARShift; ulASigned = inf.ulASigned; \
    ulRMask = (tU32)inf.qRMask; ulRLShift = inf.ulRLShift; ulRRShift = inf.ulRRShift; ulRSigned = inf.ulRSigned; \
    ulGMask = (tU32)inf.qGMask; ulGLShift = inf.ulGLShift; ulGRShift = inf.ulGRShift; ulGSigned = inf.ulGSigned; \
    ulBMask = (tU32)inf.qBMask; ulBLShift = inf.ulBLShift; ulBRShift = inf.ulBRShift; ulBSigned = inf.ulBSigned; \
    fAMul = inf.fAMul;                                                  \
    fRMul = inf.fRMul;                                                  \
    fGMul = inf.fGMul;                                                  \
    fBMul = inf.fBMul;                                                  \
    return *this;                                                       \
  }

#define INFO64_EQUAL()                                                  \
  sInfo64& operator = (const sInfo64& inf)                              \
  {                                                                     \
    qAMask = inf.qAMask; ulALShift = inf.ulALShift; ulARShift = inf.ulARShift; ulASigned = inf.ulASigned; \
    qRMask = inf.qRMask; ulRLShift = inf.ulRLShift; ulRRShift = inf.ulRRShift; ulRSigned = inf.ulRSigned; \
    qGMask = inf.qGMask; ulGLShift = inf.ulGLShift; ulGRShift = inf.ulGRShift; ulGSigned = inf.ulGSigned; \
    qBMask = inf.qBMask; ulBLShift = inf.ulBLShift; ulBRShift = inf.ulBRShift; ulBSigned = inf.ulBSigned; \
    fAMul = inf.fAMul;                                                  \
    fRMul = inf.fRMul;                                                  \
    fGMul = inf.fGMul;                                                  \
    fBMul = inf.fBMul;                                                  \
    return *this;                                                       \
  }

//////////////////////////////////////////////////////////////////////////////////////////////
// cPixelFormatStd declaration
class cPixelFormatStd : public ImplRC<iPixelFormat>
{
 public:
#pragma pack(push,4)
  struct sInfo64
  {
    struct sComponent {
      tU32 ulRShift;
      tU32 ulLShift;
      tI64 qMask;
      tF32 fMul;
      tU32 ulSigned;
    };

    union {
      struct {
        sComponent Comps[4];
      };
      struct {
        tU32 ulARShift, ulALShift; tI64 qAMask; tF32 fAMul; tU32 ulASigned;
        tU32 ulRRShift, ulRLShift; tI64 qRMask; tF32 fRMul; tU32 ulRSigned;
        tU32 ulGRShift, ulGLShift; tI64 qGMask; tF32 fGMul; tU32 ulGSigned;
        tU32 ulBRShift, ulBLShift; tI64 qBMask; tF32 fBMul; tU32 ulBSigned;
      };
    };
  };

  struct sInfo32
  {
    struct sComponent {
      tU32 ulRShift;
      tU32 ulLShift;
      tU32 ulMask;
      tF32 fMul;
      tU32 ulSigned;
    };
    union {
      struct {
        sComponent Comps[4];
      };
      struct {
        tU32 ulARShift, ulALShift, ulAMask; tF32 fAMul; tU32 ulASigned;
        tU32 ulRRShift, ulRLShift, ulRMask; tF32 fRMul; tU32 ulRSigned;
        tU32 ulGRShift, ulGLShift, ulGMask; tF32 fGMul; tU32 ulGSigned;
        tU32 ulBRShift, ulBLShift, ulBMask; tF32 fBMul; tU32 ulBSigned;
      };
    };
  };
#pragma pack(pop)

  struct sBase : public ni::Impl_HeapAlloc
  {
    virtual ~sBase() {}
    virtual tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a = 0) const = 0;
    virtual tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a = 0.0f) const = 0;
    virtual void UnpackPixelf(tPtr pColor, sColor4f& col) const = 0;
    virtual void UnpackPixelul(tPtr pColor, sColor4ul& col) const = 0;
    virtual void CopyOne(tPtr pDst, tPtr pSrc) const = 0;
  };

  struct s8 : public sBase, public sInfo32
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO32_EQUAL()
  };

  struct s16 : public sBase, public sInfo32
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO32_EQUAL()
  };

  struct s24 : public sBase, public sInfo32
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO32_EQUAL()
  };

  struct s32 : public sBase, public sInfo32
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO32_EQUAL()
  };

  struct s40 : public sBase, public sInfo64
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO64_EQUAL()
  };

  struct s48 : public sBase, public sInfo64
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO64_EQUAL()
  };

  struct s56 : public sBase, public sInfo64
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO64_EQUAL()
  };

  struct s64 : public sBase, public sInfo64
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    INFO64_EQUAL()
  };

#define PXFFLOAT_GETINDEX(FLAGS)  ((FLAGS>>4)&0xF)
#define PXFFLOAT_BUILDINDEX(IDX)  (((IDX)&0xF)<<4)
#define PXFFLOAT_HASCHANNEL     niBit(0)
#define PXFFLOAT_16BITS       niBit(1)

  struct sFloat : public sBase
  {
    tPtr BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a) const;
    tPtr BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a) const;
    void UnpackPixelf(tPtr pColor, sColor4f& col) const;
    void UnpackPixelul(tPtr pColor, sColor4ul& col) const;
    void CopyOne(tPtr pDst, tPtr pSrc) const;
    sFloat()
    {
      RFlags = GFlags = BFlags = AFlags = 0;
    }
    sFloat& operator = (const sFloat& inf)
    {
      RFlags = inf.RFlags;
      GFlags = inf.GFlags;
      BFlags = inf.BFlags;
      AFlags = inf.AFlags;
      return *this;
    }
    tU8 RFlags;
    tU8 GFlags;
    tU8 BFlags;
    tU8 AFlags;
  };

 public:
  cPixelFormatStd(const achar* cszFormat);
  ~cPixelFormatStd();

  void ZeroMembers();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  tU32 __stdcall GetNumParameters() const { return 0; }
  tBool __stdcall SetParameter(tU32 ulParameter, tU32 ulValue) { return eFalse; }
  tU32 __stdcall GetParameter(tU32 ulParameter) const { return eInvalidHandle; }

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

  tPtr __stdcall BuildPixelub(tPtr pOut, tU8 r, tU8 g, tU8 b, tU8 a = 0) const    { return BuildPixelul(pOut,r<<24,g<<24,b<<24,a<<24); }
  tPtr __stdcall BuildPixelus(tPtr pOut, tU16 r, tU16 g, tU16 b, tU16 a = 0) const  { return BuildPixelul(pOut,r<<16,g<<16,b<<16,a<<16); }
  tPtr __stdcall BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a = 0) const;
  tPtr __stdcall BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a = 0.0f) const;

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
  tU32 mCaps;
  tU32 mulBitsPerPixel;
  tU32 mulBytesPerPixel;
  tU32 mulNumRBits, mulNumGBits, mulNumBBits, mulNumABits;
  tU32 mulComp;
  sBase* mpBase;
  cString mstrFormat;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __PIXELFORMATSTD_35460993_H__
