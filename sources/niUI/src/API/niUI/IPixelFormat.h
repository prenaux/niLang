#ifndef __IPIXELFORMAT_32337024_H__
#define __IPIXELFORMAT_32337024_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "GraphicsEnum.h"

namespace ni {

/** \addtogroup niUI
 * @{
 */

//! Declare a temporary pixel that will be used for per-pixel operations.
#define niDeclareTempPixel()  tU32 __tempPixel[4]
//! Pointer to the previously declared temporary pixel.
#define niTempPixelPtr()    tPtr(__tempPixel)

//! Declare a temporary pixel that will be used for per-pixel operations.
#define niDeclareTempPixel_(x)  tU32 __tempPixel_##x[4]
//! Pointer to the previously declared temporary pixel.
#define niTempPixelPtr_(x)    tPtr(__tempPixel_##x)

//////////////////////////////////////////////////////////////////////////////////////////////
//! Pixel Format Blitting flags.
enum ePixelFormatBlit
{
  //! Normal blitting.
  ePixelFormatBlit_Normal     = 0x00000000,
  //! Mirrored left right blitting.
  ePixelFormatBlit_MirrorLeftRight= 0x00000001,
  //! Mirrored up down blitting.
  ePixelFormatBlit_MirrorUpDown = 0x00000002,
  //! Diagonal mirror. Equivalent to a Pi rad rotation.
  ePixelFormatBlit_MirrorDiagonal = ePixelFormatBlit_MirrorLeftRight|ePixelFormatBlit_MirrorUpDown,
  //! \internal
  ePixelFormatBlit_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Pixel format capabilities.
enum ePixelFormatCaps
{
  //! Support the build pixel methods.
  ePixelFormatCaps_BuildPixel = niBit(0),
  //! Support pixel unpacking.
  ePixelFormatCaps_UnpackPixel = niBit(1),
  //! Support standard blitting.
  ePixelFormatCaps_Blit = niBit(2),
  //! Support mirrored left right blitting.
  ePixelFormatCaps_BlitMirrorLeftRight = niBit(3),
  //! Support mirrored up down blitting.
  ePixelFormatCaps_BlitMirrorUpDown = niBit(4),
  //! Support stretched blitting.
  ePixelFormatCaps_BlitStretch = niBit(5),
  //! Support stretched blitting of half size only (for mipmaps generation).
  ePixelFormatCaps_BlitStretchHalf = niBit(6),
  //! Is a signed pixel format.
  ePixelFormatCaps_Signed = niBit(7),
  //! Support standard alpha blended blitting.
  ePixelFormatCaps_BlitAlpha = niBit(8),
  //! Support mirrored left right alpha blended blitting.
  ePixelFormatCaps_BlitAlphaMirrorLeftRight = niBit(9),
  //! Support mirrored up down alpha blended blitting.
  ePixelFormatCaps_BlitAlphaMirrorUpDown = niBit(10),
  //! Support stretched alpha blended blitting.
  ePixelFormatCaps_BlitAlphaStretch = niBit(11),
  //! Support stretched alpha blended blitting of half size only (for mipmaps generation).
  ePixelFormatCaps_BlitAlphaStretchHalf = niBit(12),
  //! Support a specialize clear method.
  ePixelFormatCaps_Clear = niBit(13),
  //! Block compressed format.
  //! \remark This includes format such as DXT formats and ASTC.
  ePixelFormatCaps_BlockCompressed = niBit(14),
  //! \internal
  ePixelFormatCaps_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Pixel Format interface.
struct iPixelFormat : public iUnknown
{
  niDeclareInterfaceUUID(iPixelFormat,0xdc0b6ce3,0xfd1c,0x4283,0xb6,0xb8,0x76,0x8e,0x85,0xa2,0x72,0xa4)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Test if the given pixel format is the same as this pixel format.
  virtual tBool __stdcall IsSamePixelFormat(const iPixelFormat* pPixFmt) const = 0;

  //! Get the number of parameters of the pixel format.
  //! {Property}
  virtual tU32 __stdcall GetNumParameters() const = 0;
  //! Set a parameter of the pixel format.
  //! {Property}
  virtual tBool __stdcall SetParameter(tU32 ulParameter, tU32 ulValue) = 0;
  //! Get a parameter of the pixel format.
  //! {Property}
  virtual tU32 __stdcall GetParameter(tU32 ulParameter) const = 0;

  //! Clone the pixel format.
  virtual iPixelFormat* __stdcall Clone() const = 0;

  //! Get the pixel format, format string.
  //! {Property}
  virtual const achar* __stdcall GetFormat() const = 0;
  //! Get the pixel format capabilities.
  //! {Property}
  virtual ePixelFormatCaps __stdcall GetCaps() const = 0;

  //! Get the number of bits per pixel.
  //! {Property}
  virtual tU32 __stdcall GetBitsPerPixel() const = 0;
  //! Get the number of bytes per pixel.
  //! {Property}
  virtual tU32 __stdcall GetBytesPerPixel() const = 0;
  //! Get the size in bytes of a surface using this pixel format.
  virtual tU32 __stdcall GetSize(tU32 ulW, tU32 ulH, tU32 ulD) const = 0;
  //! Get the number of components of the pixel format.
  //! {Property}
  virtual tU32 __stdcall GetNumComponents() const  = 0;
  //! Get the number of red bits.
  //! {Property}
  virtual tU32 __stdcall GetNumRBits() const = 0;
  //! Get the number of green bits.
  //! {Property}
  virtual tU32 __stdcall GetNumGBits() const = 0;
  //! Get the number of blue bits.
  //! {Property}
  virtual tU32 __stdcall GetNumBBits() const = 0;
  //! Get the number of alpha bits.
  //! {Property}
  virtual tU32 __stdcall GetNumABits() const = 0;
  //! @}

  //########################################################################################
  //! \name Pixel packing
  //########################################################################################
  //! @{

  //! Build a pixel using unsigned byte values.
  virtual tPtr __stdcall BuildPixelub(tPtr pOut, tU8 r, tU8 g, tU8 b, tU8 a = 0) const = 0;
  //! Build a pixel using unsigned short values.
  virtual tPtr __stdcall BuildPixelus(tPtr pOut, tU16 r, tU16 g, tU16 b, tU16 a = 0) const = 0;
  //! Build a pixel using unsigned short values.
  virtual tPtr __stdcall BuildPixelul(tPtr pOut, tU32 r, tU32 g, tU32 b, tU32 a = 0) const = 0;
  //! Build a pixel using floating point values.
  virtual tPtr __stdcall BuildPixelf(tPtr pOut, tF32 r, tF32 g, tF32 b, tF32 a = 0.0f) const = 0;
  //! @}

  //########################################################################################
  //! \name Pixels unpacking
  //########################################################################################
  //! @{

  //! Begin pixel unpacking.
  virtual tPtr __stdcall BeginUnpackPixels(tPtr pSurface, tU32 ulPitch, tU32 ulX, tU32 ulY, tU32 ulW, tU32 ulH) = 0;
  //! End pixel unpacking.
  virtual void __stdcall EndUnpackPixels() = 0;
  //! Unpack a pixel in an unsigned byte color.
  virtual sColor4ub __stdcall UnpackPixelub(tPtr pColor) const = 0;
  //! Unpack a pixel in an unsigned short color.
  //! \remark Return a Vec4i in with member in the 0-0xFFFF range.
  virtual sVec4i __stdcall UnpackPixelus(tPtr pColor) const = 0;
  //! Unpack a pixel in an unsigned long color.
  virtual sVec4i __stdcall UnpackPixelul(tPtr pColor) const = 0;
  //! Unpack a pixel in a floating point color.
  virtual sColor4f __stdcall UnpackPixelf(tPtr pColor) const = 0;
  //! @}

  //########################################################################################
  //! \name Blitting
  //########################################################################################
  //! @{

  //! Clear a surface with a specified color.
  virtual tBool __stdcall Clear(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy, tU32 w, tU32 h, tPtr apClearColor) = 0;

  //! Blit a surface of any pixel format to a surface of this pixel format.
  virtual tBool __stdcall Blit(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy,
                               tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy,
                               tU32 w, tU32 h, ePixelFormatBlit blitFlags) const = 0;

  //! Blit stretch a surface of any pixel format to a surface of this pixel format.
  virtual tBool __stdcall BlitStretch(tPtr pDst, tU32 ulDestPitch, tU32 ulDestWidth, tU32 ulDestHeight, tU32 dx, tU32 dy, tU32 dw, tU32 dh,
                                      tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy, tU32 sw, tU32 sh) const = 0;

  //! Blit a surface of any pixel format to a surface of this pixel format.
  //! \remark The source and destination color are used to replace missing components in the source/destination format.
  //! \remark If the passed blending mode is not supported the method will return false to be handled by the reference implementation.
  virtual tBool __stdcall BlitAlpha(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy,
                                    tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy,
                                    tU32 w, tU32 h, ePixelFormatBlit blitFlags,
                                    const sColor4f& aSourceColor, const sColor4f& aDestColor, eBlendMode aBlendMode) const = 0;

  //! Blit stretch a surface of any pixel format to a surface of this pixel format.
  //! \remark The source and destination color are used to replace missing components in the source/destination format.
  //! \remark If the passed blending mode is not supported the method will return false to be handled by the reference implementation.
  virtual tBool __stdcall BlitAlphaStretch(tPtr pDst, tU32 ulDestPitch, tU32 ulDestWidth, tU32 ulDestHeight, tU32 dx, tU32 dy, tU32 dw, tU32 dh,
                                           tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy, tU32 sw, tU32 sh,
                                           const sColor4f& aSourceColor, const sColor4f& aDestColor, eBlendMode aBlendMode) const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IPIXELFORMAT_32337024_H__
