#ifndef __IBITMAP3D_2711532_H__
#define __IBITMAP3D_2711532_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
struct iBitmap2D;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! 3D Bitmap interface.
/**
 * <pre>
 *              *============*
 *             /||||||||||||/| <== Slice Pitch
 *            /||||||||||||/||     (Num of bytes to go
 *           /||||||||||||/|/|      from the top left
 *          *===+========*|/ |      corner of one slice
 *          ||||||||||||||/  |      to the other)
 *          |============|   |
 * Depth    |   *========+===*
 * (R coo)  |  /||||||||||||/ <=== Row Pitch
 * (Slice)  | /==========|=/       Height
 *          |/           |/        (T Coo)
 *          *============*
 *          Width (S coo)
 * </pre>
 */
struct iBitmap3D : public iBitmapBase
{
  niDeclareInterfaceUUID(iBitmap3D,0x5523e61d,0x74f6,0x4af8,0xbf,0x42,0xdc,0x76,0x0a,0x61,0x44,0xec)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Set the memory address.
  //! \param  apAddr is the address of the bitmap's data.
  //! \param abFreeData: if eTrue the data will be freed automatically when the bitmap is released.
  //! \param anRowPitch: if != eInvalidHandle will adjust the bitmap's row pitch.
  //! \param anSlicePitch: if != eInvalidHandle will adjust the bitmap's slice pitch.
  //! \return eFalse if apAddre is NULL, else eTrue.
  //! \remark If abFreeData the memory must have been allocated with niNew or niMalloc.
  // {NoAutomation}
  virtual tBool __stdcall SetMemoryAddress(tPtr apAddr, tBool abFreeData, tU32 anRowPitch = eInvalidHandle, tU32 anSlicePitch = eInvalidHandle) = 0;

  //! Get the number of bytes per row
  //! {Property}
  virtual tU32  __stdcall GetRowPitch() const = 0;
  //! Get the number of bytes per slice
  //! {Property}
  virtual tU32  __stdcall GetSlicePitch() const = 0;
  //! Get the size in bytes.
  //! {Property}
  virtual tU32  __stdcall GetSize() const = 0;
  //! Get the data pointer.
  //! {Property}{NoAutomation}
  virtual tPtr  __stdcall GetData() const = 0;
  //! Get the slice data pointer.
  //! {Property}{NoAutomation}
  virtual tPtr  __stdcall GetSlicePtr(tU32 anSlice) const = 0;
  //! Create a 2d bitmap which points to the memory of the specified slice.
  //! {NoAutomation}
  //! \remark Creates a bitmap that points to the slice data,
  //!         that bitmap DOES NOT own the memory, if the
  //!         3d bitmap's memory is deleted that new bitmaps
  //!         become invalid.
  virtual iBitmap2D*  __stdcall CreateSliceBitmap(tU32 anSlice) const = 0;
  //! @}

  //########################################################################################
  //! \name Derived bitmaps creation
  //########################################################################################
  //! @{

  //! Create a resized copy of this bitmap.
  virtual iBitmap3D* __stdcall CreateResized(tU32 anW, tU32 anH, tU32 anD) const = 0;
  //! @}

  //########################################################################################
  //! \name Mip maps
  //########################################################################################
  //! @{

  //! Get the indicated mip map.
  //! {Property}
  virtual iBitmap3D* __stdcall GetMipMap(tU32 ulIdx) const = 0;
  //! Get the bitmap at the specified level.
  //! \remark 0 is the main surface, 1 is the first mip map, and so on.
  //! {Property}
  virtual iBitmap3D* __stdcall GetLevel(tU32 anIndex) const = 0;
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
  //! \name Blitting
  //########################################################################################
  //! @{

  //! Blit a bitmap in this bitmap.
  //! \remark Clipping and pixel format conversion are automatically performed.
  virtual tBool __stdcall Blit(const iBitmap3D* apSrc, const sVec3i& avSrcMin = sVec3i::Zero(), const sVec3i& avDestMin = sVec3i::Zero(), const sVec3i& avSize = sVec3i::Zero()) = 0;
  //! @}

  //########################################################################################
  //! \name Primitives
  //########################################################################################
  //! @{

  //! Put a pixel at the specified position.
  //! {NoAutomation}
  virtual void __stdcall PutPixel(const sVec3i& avPos, tPtr col) = 0;
  //! Get a pixel at the specified position.
  //! {NoAutomation}
  virtual tPtr __stdcall GetPixel(const sVec3i& avPos, tPtr pOut) const = 0;
  //! Clear the bitmap
  //! {NoAutomation}
  virtual void __stdcall Clear(tPtr pColor = NULL) = 0;
  //! Clear the bitmap
  //! \remark The right, bottom and back edges are not filled.
  //! {NoAutomation}
  virtual void __stdcall ClearBox(const sVec3i& avMin, const sVec3i& avMax, tPtr pColor) = 0;
  //! Put a pixel at the specified position.
  virtual void __stdcall PutPixelf(const sVec3i& avPos, const sColor4f& avCol) = 0;
  //! Get a pixel at the specified position.
  virtual sColor4f __stdcall GetPixelf(const sVec3i& avPos) const = 0;
  //! Clear the bitmap.
  virtual void __stdcall Clearf(const sColor4f& avCol) = 0;
  //! Clear the bitmap.
  //! \remark The right and bottom edges are not filled.
  virtual void __stdcall ClearBoxf(const sVec3i& avMin, const sVec3i& avMax, const sColor4f& avCol) = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IBITMAP3D_2711532_H__
