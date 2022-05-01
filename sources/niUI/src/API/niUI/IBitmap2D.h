#ifndef __IBITMAP2D_11143201_H__
#define __IBITMAP2D_11143201_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IBitmapBase.h"

namespace ni {

struct iBitmapCube;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! 2D Bitmap interface.
struct iBitmap2D : public iBitmapBase
{
  niDeclareInterfaceUUID(iBitmap2D,0x30011ccb,0xe4b7,0x45f9,0x95,0xbc,0x7c,0x36,0x5a,0x52,0x27,0x95)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Set the memory address.
  //! \param  apAddr is the address of the bitmap's data.
  //! \param  abFreeData: if eTrue the data will be freed automatically when the bitmap is released.
  //! \param  anPitch: if != eInvalidHandle will adjust the bitmap's pitch.
  //! \return eFalse if apAddre is NULL, else eTrue.
  //! \remark If abFreeData the memory must have been allocated with niNew or niMalloc.
  // {NoAutomation}
  virtual tBool __stdcall SetMemoryAddress(tPtr apAddr, tBool abFreeData, tU32 anPitch = eInvalidHandle) = 0;
  //! Get the number of bytes per line.
  //! {Property}
  virtual tU32  __stdcall GetPitch() const = 0;
  //! Get the data pointer.
  //! {Property}
  virtual tPtr  __stdcall GetData() const = 0;
  //! Get the size in bytes.
  //! {Property}
  virtual tU32  __stdcall GetSize() const = 0;
  //! @}

  //########################################################################################
  //! \name Derived bitmaps creation
  //########################################################################################
  //! @{

  //! Create a resized copy of this bitmap.
  virtual iBitmap2D* __stdcall CreateResized(tI32 nW, tI32 nH) const = 0;
  //! Create a resized copy of this bitmap.
  //! \remark This variant of CreateResized as an additional
  //!         parameter that allows to specify whether the mipmaps
  //!         should be resized as-well or just dropped in the
  //!         resized version.
  virtual iBitmap2D* __stdcall CreateResizedEx(tI32 nW, tI32 nH, tBool abPreserveMipMaps) const = 0;
  //! Create a cube bitmap from the specified 6 rectangles.
  virtual iBitmapCube* __stdcall CreateCubeBitmap(tU32 anWidth,
                                                  const sVec2i& avPX, ePixelFormatBlit aBlitPX,
                                                  const sVec2i& avNX, ePixelFormatBlit aBlitNX,
                                                  const sVec2i& avPY, ePixelFormatBlit aBlitPY,
                                                  const sVec2i& avNY, ePixelFormatBlit aBlitNY,
                                                  const sVec2i& avPZ, ePixelFormatBlit aBlitPZ,
                                                  const sVec2i& avNZ, ePixelFormatBlit aBlitNZ) const = 0;
  //! Create a cube bitmap from a vertical or horizontal cross unfolded cube.
  //! \remark Wheter a vertical or horizontal cross is contained in the bitmap is determined
  //!     from the ratio between the width and height of the bitmap. If the width is greater
  //!     an horizontal cross is assumed, else if the height is greater then a vertical
  //!     cross is assumed.
  //! \remark The cross is assumed to be on the 'top'/'left' of the bitmap.
  //! \remark Horizontal cross :
  //!     <pre>
  //!     |------------------------
  //!     |     | PY  |           |
  //!     |     |     |           |
  //!     |------------------------
  //!     | NX  | PZ  | PX  | NZ  |
  //!     |     |     |     |     |
  //!     |------------------------
  //!     |     | NY  |           |
  //!     |     |     |           |
  //!     |------------------------
  //!     </pre>
  //! \remark Vertical cross :
  //!     <pre>
  //!     |-----------------|
  //!     |     | NX  |     |
  //!     |     |     |     |
  //!     |-----|-----|-----|
  //!     | NY  | PZ  | PY  |
  //!     |     |     |     |
  //!     |-----|-----|-----|
  //!     |     | PX  |     |
  //!     |     |     |     |
  //!     |     |-----|     |
  //!     |     | NZ  |     |
  //!     |     |     |     |
  //!     |-----|-----|-----|
  //!     </pre>
  virtual iBitmapCube* __stdcall CreateCubeBitmapCross() const = 0;
  //! @}

  //########################################################################################
  //! \name Pixel format
  //########################################################################################
  //! @{

  //! Begin unpacking pixels.
  virtual tBool __stdcall BeginUnpackPixels() = 0;
  //! End unpacking pixels.
  virtual void  __stdcall EndUnpackPixels() = 0;
  //! @}

  //########################################################################################
  //! \name Mip maps
  //########################################################################################
  //! @{

  //! Get the mipmap at the specified index.
  //! \remark Mip map zero is not the main surface but the first mip map.
  //! {Property}
  virtual iBitmap2D* __stdcall GetMipMap(tU32 ulIdx) const = 0;
  //! Get the bitmap at the specified level.
  //! \remark 0 is the main surface, 1 is the first mip map, and so on.
  //! {Property}
  virtual iBitmap2D* __stdcall GetLevel(tU32 anIndex) const = 0;
  //! @}

  //########################################################################################
  //! \name Blitting
  //########################################################################################
  //! @{

  //! Blit a bitmap in this bitmap.
  //! \remark Clipping and pixel format conversion are automatically performed.
  virtual tBool __stdcall Blit(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd, tI32 yd, tI32 w, tI32 h, ePixelFormatBlit eBlit = ePixelFormatBlit_Normal) = 0;
  //! Blit stretch a bitmap in this bitmap.
  //! \remark Clipping and pixel format conversion are automatically performed.
  virtual tBool __stdcall BlitStretch(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd, tI32 yd, tI32 ws, tI32 hs, tI32 wd, tI32 hd) = 0;
  //! Blit, with alpha blending, a bitmap in this bitmap.
  //! \remark The source and destination color are used to replace missing components in the source/destination format.
  //! \remark Clipping and pixel format conversion are automatically performed.
  virtual tBool __stdcall BlitAlpha(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd, tI32 yd, tI32 w, tI32 h, ePixelFormatBlit eBlit,
                                    const sColor4f& acolSource, const sColor4f& acolDest, eBlendMode aBlendMode) = 0;
  //! Blit stretch, with alpha blending, a bitmap in this bitmap.
  //! \remark The source and destination color are used to replace missing components in the source/destination format.
  //! \remark Clipping and pixel format conversion are automatically performed.
  virtual tBool __stdcall BlitAlphaStretch(const iBitmap2D* src, tI32 xs, tI32 ys, tI32 xd, tI32 yd, tI32 ws, tI32 hs, tI32 wd, tI32 hd,
                                           const sColor4f& acolSource, const sColor4f& acolDest, eBlendMode aBlendMode) = 0;
  //! @}

  //########################################################################################
  //! \name Primitives
  //########################################################################################
  //! @{

  //! Put a pixel at the specified position.
  //! {NoAutomation}
  virtual void __stdcall PutPixel(tI32 x, tI32 y, tPtr col) = 0;
  //! Get a pixel at the specified position.
  //! {NoAutomation}
  virtual tPtr __stdcall GetPixel(tI32 x, tI32 y, tPtr pOut) const = 0;
  //! Clear the bitmap
  //! {NoAutomation}
  virtual void __stdcall Clear(tPtr pColor = NULL) = 0;
  //! Clear the bitmap
  //! \remark The right and bottom edges are not filled.
  //! {NoAutomation}
  virtual void __stdcall ClearRect(const sRecti& aRect, tPtr pColor) = 0;
  //! Put a pixel at the specified position.
  virtual void __stdcall PutPixelf(tI32 x, tI32 y, const sColor4f& avCol) = 0;
  //! Get a pixel at the specified position.
  virtual sColor4f __stdcall GetPixelf(tI32 x, tI32 y) const = 0;
  //! Clear the bitmap.
  virtual void __stdcall Clearf(const sColor4f& avCol) = 0;
  //! Clear the bitmap.
  //! \remark The right and bottom edges are not filled.
  virtual void __stdcall ClearRectf(const sRecti& aRect, const sColor4f& avCol) = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IBITMAP2D_11143201_H__
