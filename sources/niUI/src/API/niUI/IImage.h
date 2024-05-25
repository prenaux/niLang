#pragma once
#ifndef __IIMAGE_H_61C48F5A_1349_4733_AD7C_BB05BCD9F997__
#define __IIMAGE_H_61C48F5A_1349_4733_AD7C_BB05BCD9F997__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Image usage.
enum eImageUsage
{
  //! The usage is read-only to be used a source for rendering.
  //! \remark No dirty region is added.
  eImageUsage_Source = 0,
  //! The usage is write-only to be used as a render target. Previous content is preserved.
  eImageUsage_Target = 1,
  //! For GetTexture only, grabs the depth stencil for rendering.
  eImageUsage_DepthStencil = 2,
  //! The usage is write-only to be used as a render target. The whole content
  //! of the target is assumed to be overwritten and so no effort to keep the
  //! previous content is made.
  eImageUsage_TargetDiscard = 3,
  //! \internal
  eImageUsage_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Image interface.
struct iImage : public iUnknown
{
  niDeclareInterfaceUUID(iImage,0x848a96e9,0x4b5c,0x43cf,0x9c,0x61,0xa2,0x6d,0xf5,0x4c,0x7f,0x07)

  //! Copy the specified image.
  //! \remark Copy the bitmap's content.
  virtual tBool __stdcall Copy(iImage* apImage) = 0;
  //! Clone this image.
  //! \remark Clone the bitmap's content.
  virtual iImage* __stdcall Clone() const = 0;

  //! Get whether a bitmap is already initialized in the image.
  //! {Property}
  virtual tBool __stdcall GetHasBitmap() const = 0;
  //! Grab the image as a bitmap to be used for some other operations.
  //! \remark This function only adds a dirty rectangles and manage the bitmap's update from the texture.
  //! \remark Pass a Null rectangle to identity the whole image size.
  virtual iBitmap2D* __stdcall GrabBitmap(eImageUsage aLock, const sRecti& aDirtyRect) = 0;
  //! Get whether a texture is already initialized in the image.
  //! {Property}
  virtual tBool __stdcall GetHasTexture() const = 0;
  //! Get whether a depth stencil is already initialized in the image.
  //! {Property}
  virtual tBool __stdcall GetHasDepthStencil() const = 0;
  //! Grab the image as a texture to be used for some other operations.
  //! \remark This function only adds a dirty rectangles and manage the texture's update from the bitmap.
  //! \remark Pass a Null rectangle to identity the whole image size.
  virtual iTexture* __stdcall GrabTexture(eImageUsage aLock, const sRecti& aDirtyRect) = 0;

  //! Get the image's width.
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Get the image's height.
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! Get the image's size in a vec2f.
  //! {Property}
  virtual sVec2f __stdcall GetSize() const = 0;

  //! Indicate that the bitmap's mipmaps should be recomputed the next time
  //! the bitmap is copied to the texture.
  virtual void __stdcall RecomputeBitmapMipmapsBeforeCopyToTexture() = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IIMAGE_H_61C48F5A_1349_4733_AD7C_BB05BCD9F997__
