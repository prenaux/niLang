#ifndef __IVGIMAGE_4087974_H__
#define __IVGIMAGE_4087974_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! VG wrap type.
enum eVGWrapType
{
  //! Clamp, the edge color is used when outside of the source rectangle.
  eVGWrapType_Clamp = 0,
  //! Repeat, the source is repeated at each source rectangle boundary.
  eVGWrapType_Repeat = 1,
  //! Mirror, the source is mirror at each source rectangle boundary.
  eVGWrapType_Mirror = 2,
  //! Padding, the background color is used when outside of the source rectangle.
  //! \remark Not supported by gradients.
  eVGWrapType_Pad = 3,
  //! \internal
  eVGWrapType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! VG Image filter.
enum eVGImageFilter
{
  //! No filter.
  eVGImageFilter_Point = 0,
  //! Bilinear filter.
  eVGImageFilter_Bilinear = 1,
  //! Bicubic filter.
  eVGImageFilter_Bicubic = 2,
  //! Spline16 filter.
  eVGImageFilter_Spline16 = 3,
  //! Spline36 filter.
  eVGImageFilter_Spline36 = 4,
  //! Hanning filter.
  eVGImageFilter_Hanning = 5,
  //! Hamming filter.
  eVGImageFilter_Hamming = 6,
  //! Hermite filter.
  eVGImageFilter_Hermite = 7,
  //! Kaiser filter.
  eVGImageFilter_Kaiser = 8,
  //! Quadric filter.
  eVGImageFilter_Quadric = 9,
  //! Catrom filter.
  eVGImageFilter_Catrom = 10,
  //! Gaussian filter.
  eVGImageFilter_Gaussian = 11,
  //! Bessel filter.
  eVGImageFilter_Bessel = 12,
  //! Mitchell filter.
  eVGImageFilter_Mitchell = 13,
  //! Sinc filter.
  eVGImageFilter_Sinc = 14,
  //! Lanczos filter.
  eVGImageFilter_Lanczos = 15,
  //! Blackman filter.
  eVGImageFilter_Blackman = 16,
  //! \internal
  eVGImageFilter_Last niMaybeUnused = 17,
  //! \internal
  eVGImageFilter_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! VGImage usage.
enum eVGImageUsage
{
  //! The usage is read-only to be used a source for rendering.
  //! \remark No dirty region is added.
  eVGImageUsage_Source = 0,
  //! The usage is write-only to be used as a render target.
  //! \remark For texture this will force the creation of a render target
  //!         texture. Once a render target has been created the image
  //!         will always remain a render target.
  eVGImageUsage_Target = 1,
  //! For GetTexture only, grabs the depth stencil for rendering.
  eVGImageUsage_DepthStencil = 2,
  //! \internal
  eVGImageUsage_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! VGImage interface.
struct iVGImage : public iUnknown
{
  niDeclareInterfaceUUID(iVGImage,0x848a96e9,0x4b5c,0x43cf,0x9c,0x61,0xa2,0x6d,0xf5,0x4c,0x7f,0x07)

  //! Copy the specified image.
  //! \remark Copy the bitmap's content.
  virtual tBool __stdcall Copy(iVGImage* apImage) = 0;
  //! Clone this image.
  //! \remark Clone the bitmap's content.
  virtual Ptr<iVGImage> __stdcall Clone() const = 0;

  //! Get whether a bitmap is already initialized in the image.
  //! {Property}
  virtual tBool __stdcall GetHasBitmap() const = 0;
  //! Grab the image as a bitmap to be used for some other operations.
  //! \remark This function only adds a dirty rectangles and manage the bitmap's update from the texture.
  //! \remark Pass a Null rectangle to identity the whole image size.
  virtual iBitmap2D* __stdcall GrabBitmap(eVGImageUsage aLock, const sRecti& aDirtyRect) = 0;
  //! Get whether a texture is already initialized in the image.
  //! {Property}
  virtual tBool __stdcall GetHasTexture() const = 0;
  //! Get whether a depth stencil is already initialized in the image.
  //! {Property}
  virtual tBool __stdcall GetHasDepthStencil() const = 0;
  //! Grab the image as a texture to be used for some other operations.
  //! \remark This function only adds a dirty rectangles and manage the texture's update from the bitmap.
  //! \remark Pass a Null rectangle to identity the whole image size.
  virtual iTexture* __stdcall GrabTexture(eVGImageUsage aLock, const sRecti& aDirtyRect) = 0;

  //! Get the image's width.
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Get the image's height.
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! Get the image's size in a vector2f.
  //! {Property}
  virtual sVec2f __stdcall GetSize() const = 0;
  //! Resize the image.
  //! \remark This will clear the image's content.
  //! \remark Resize will fail if the size is smaller than one.
  //! \remark When resize fail the previous image's state is preserved.
  virtual tBool __stdcall Resize(tU32 anNewWidth, tU32 anNewHeight) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVGIMAGE_4087974_H__
