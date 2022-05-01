#ifndef __IOVERLAY_608540_H__
#define __IOVERLAY_608540_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

struct iImage;
struct iCanvas;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! iOverlay interface
//!
struct iOverlay : public iUnknown
{
  niDeclareInterfaceUUID(iOverlay,0xfc8d5e7e,0x931f,0x4388,0xb6,0x70,0x1e,0x30,0xb9,0xb1,0xa2,0x1f)

  //! Clone the overlay.
  virtual iOverlay* __stdcall Clone() const = 0;

  //! Get the overlay's material.
  //! {Property}
  virtual iMaterial* __stdcall GetMaterial() const = 0;

  //! Get the overlay's base image if there's one.
  //! {Property}
  virtual iImage* __stdcall GetImage() const = 0;

  //! Get the overlay's base size.
  //! \remark The base size is the size of the overlay's material.
  //! {Property}
  virtual sVec2f __stdcall GetBaseSize() const = 0;

  //! Set the overlay's pivot position.
  //! \remark The pivot position, is the offset at which the overaly is going to be drawn.
  //! \remark If < 1.0 the center position is considered to be relative to the overlay's size, else it's assumed to be 'absolute'.
  //! \remark Default is (0,0), the position is relative the the top left corner of the overlay.
  //! {Property}
  virtual void __stdcall SetPivot(const sVec2f& avPivot) = 0;
  //! Get the overlay's pivot position.
  //! {Property}
  virtual sVec2f __stdcall GetPivot() const = 0;

  //! Set the overlay size.
  //! {Property}
  virtual void __stdcall SetSize(sVec2f avSize) = 0;
  //! Get the overlay position and size.
  //! {Property}
  virtual sVec2f __stdcall GetSize() const = 0;

  //! Set the rasterizer blend mode.
  //! \remark This changes the blend mode in the material.
  //! {Property}
  virtual void __stdcall SetBlendMode(eBlendMode aBlendMode) = 0;
  //! Get the rasterizer blend mode.
  //! {Property}
  virtual eBlendMode __stdcall GetBlendMode() const = 0;

  //! Set the filtering mode.
  //! \remark This changes the filtering type in the material.
  //! {Property}
  virtual void __stdcall SetFiltering(tBool abEnabled) = 0;
  //! Get the filtering mode.
  //! {Property}
  virtual tBool __stdcall GetFiltering() const = 0;

  //! Set the color of the overlay.
  //! {Property}
  virtual void __stdcall SetColor(const sColor4f& aColor) = 0;
  //! Get the color of the overlay.
  //! {Property}
  virtual sColor4f __stdcall GetColor() const = 0;

  //! Set the color of the overlay.
  //! \param  aCorner specify the corner(s) on which the color apply.
  //! \param  aColor is the new color of the corner.
  //! {Property}
  virtual void __stdcall SetCornerColor(eRectCorners aCorner, const sColor4f& aColor) = 0;
  //! Get the color of the overlay.
  //! \param  aCorner is the corner to get the color of.
  //! \return the color of the specified corner of the overlay.
  //! {Property}
  virtual sColor4f __stdcall GetCornerColor(eRectCorners aCorner) const = 0;

  //! Set the overlay's mapping coordinates.
  //! {Property}
  virtual void __stdcall SetMapping(const sRectf& aRect) = 0;
  //! Get the overlay's mapping coordinates.
  //! {Property}
  virtual sRectf __stdcall GetMapping() const = 0;

  //! Set the overlay's frame.
  //! {Property}
  virtual void __stdcall SetFrame(const sVec4f& aFrameBorder) = 0;
  //! Get the overlay's frame.
  //! {Property}
  virtual sVec4f __stdcall GetFrame() const = 0;
  //! Get whether a frame is activated.
  //! {Property}
  //! \remark A frame is defined if the frame is != Vec4::Zero
  virtual tBool __stdcall GetIsFrame() const = 0;
  //! Compute the center of the frame from the provided destination rectangle.
  virtual sRectf __stdcall ComputeFrameCenter(const sRectf& aDest) const = 0;

  virtual tBool __stdcall Draw(iCanvas* apCanvas, const sVec2f& aPos, const sVec2f& aSize) = 0;
  virtual tBool __stdcall DrawFrame(iCanvas* apCanvas, tRectFrameFlags aFrame, const sVec2f& aPos, const sVec2f& aSize) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IOVERLAY_608540_H__
