#ifndef __IVGPAINT_9917824_H__
#define __IVGPAINT_9917824_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IVGImage.h"
#include "IVGGradientTable.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! VG paint type.
enum eVGPaintType
{
  //! Solid paint type.
  eVGPaintType_Solid = 0,
  //! Image paint type.
  eVGPaintType_Image = 1,
  //! Gradient paint type.
  eVGPaintType_Gradient = 2,
  //! \internal
  eVGPaintType_ForceDWORD = 0xFFFFFFFF
};

//! VG paint units.
enum eVGPaintUnits
{
  //! Units are absolute (default).
  eVGPaintUnits_Absolute = 0,
  //! Units relative to the element's bounding box.
  eVGPaintUnits_ObjectBoundingBox = 1,
  //! Units relative to the current coordinate system.
  eVGPaintUnits_UserSpaceOnUse = 2,
  //! \internal
  eVGPaintUnits_ForceDWORD = 0xFFFFFFFF
};

//! VG paint interface.
struct iVGPaint : public iUnknown
{
  niDeclareInterfaceUUID(iVGPaint,0x70adf57b,0x20e2,0x4734,0x90,0x3c,0xde,0x24,0xe7,0x02,0xef,0xa9)

  //! Copy another paint of the same type.
  virtual tBool __stdcall Copy(const iVGPaint* apSrc) = 0;
  //! Clone this paint.
  virtual iVGPaint* __stdcall Clone() const = 0;
  //! Get the paint type.
  //! {Property}
  virtual eVGPaintType __stdcall GetType() const = 0;
  //! Set the paint color.
  //! {Property}
  //! \remark The paint color is the default color that should be used
  //!     if the rendering context can't draw this paint type.
  //! \remark The color is the modulation color if the paint type is more
  //!     complex than a solid color.
  virtual void __stdcall SetColor(const sColor4f& aColor) = 0;
  //! Get the paint color.
  //! {Property}
  //! \remark The paint color is the default color that should be used
  //!     if the rendering context can't draw this paint type.
  virtual const sColor4f& __stdcall GetColor() const = 0;
};

//! VG paint image interface.
struct iVGPaintImage : public iVGPaint
{
  niDeclareInterfaceUUID(iVGPaintImage,0x83454203,0x03d3,0x43df,0x9f,0xc6,0x06,0xe9,0xe7,0x0a,0x78,0x2f)

  //! Get the painted image.
  //! {Property}
  virtual iVGImage* __stdcall GetImage() const = 0;

  //! Set the image filter type.
  //! {Property}
  virtual void __stdcall SetFilterType(eVGImageFilter aType) = 0;
  //! Get the image filter type.
  //! {Property}
  virtual eVGImageFilter __stdcall GetFilterType() const = 0;

  //! Set the image filter radius.
  //! {Property}
  virtual void __stdcall SetFilterRadius(tF32 afRadius) = 0;
  //! Get the image filter radius.
  //! {Property}
  virtual tF32 __stdcall GetFilterRadius() const = 0;

  //! Set the image filter normalization flag.
  //! {Property}
  virtual void __stdcall SetFilterNormalize(tBool abNormalize) = 0;
  //! Get the image filter normalization flag.
  //! {Property}
  virtual tBool __stdcall GetFilterNormalize() const = 0;

  //! Set the paint wrap type.
  //! {Property}
  virtual void __stdcall SetWrapType(eVGWrapType aWrapType) = 0;
  //! Get the paint wrap type.
  //! {Property}
  virtual eVGWrapType __stdcall GetWrapType() const = 0;

  //! Set the image's display rectangle.
  //! {Property}
  //! \remark User side parameter, the user should setup the fill/stroke paint transform according to this parameter before rendering.
  virtual void __stdcall SetRectangle(const sRectf& aRect) = 0;
  //! Get the image's display rectangle.
  //! {Property}
  virtual sRectf __stdcall GetRectangle() const = 0;
  //! Set the paint units.
  //! {Property}
  //! \remark User side parameter, the user should setup the fill/stroke paint transform according to this parameter before rendering.
  virtual void __stdcall SetUnits(eVGPaintUnits aUnits) = 0;
  //! Get the paint units.
  //! {Property}
  virtual eVGPaintUnits __stdcall GetUnits() const = 0;
  //! Set the source user pointer to be used to generate the image's content.
  //! {Property}
  //! \remark User side parameter, the user should update the fill/stroke image transform according to this parameter before rendering.
  virtual void __stdcall SetSource(iUnknown* apSource) = 0;
  //! Get the source user pointer to be used to generate the image's content.
  //! {Property}
  virtual iUnknown* __stdcall GetSource() const = 0;
  //! Set the content units.
  //! {Property}
  //! \remark User side parameter, the user should setup the fill/stroke paint transform according to this parameter before rendering.
  //! \remark Valid only if generated from a source.
  virtual void __stdcall SetSourceUnits(eVGPaintUnits aUnits) = 0;
  //! Get the content units.
  //! {Property}
  virtual eVGPaintUnits __stdcall GetSourceUnits() const = 0;
};

//! VG paint gradient interface.
struct iVGPaintGradient : public iVGPaint
{
  niDeclareInterfaceUUID(iVGPaintGradient,0x2da65d45,0x7b4b,0x4885,0x9b,0x68,0x09,0xe5,0x86,0xde,0xf1,0x1b)

  //! Set the type of gradient.
  //! {Property}
  virtual void __stdcall SetGradientType(eVGGradientType aType) = 0;
  //! Get the type of gradient.
  //! {Property}
  virtual eVGGradientType __stdcall GetGradientType() const = 0;
  //! Set the D1 parameter.
  //! {Property}
  virtual void __stdcall SetD1(tI32 anD1) = 0;
  //! Get the D1 parameter.
  //! {Property}
  virtual tI32 __stdcall GetD1() const = 0;
  //! Set the D2 parameter.
  //! {Property}
  virtual void __stdcall SetD2(tI32 anD2) = 0;
  //! Get the D2 parameter.
  //! {Property}
  virtual tI32 __stdcall GetD2() const = 0;
  //! Set the paint wrap type.
  //! {Property}
  virtual void __stdcall SetWrapType(eVGWrapType aWrapType) = 0;
  //! Get the paint wrap type.
  //! {Property}
  virtual eVGWrapType __stdcall GetWrapType() const = 0;

  //! Set the direction start for linear gradient, or center for radial gradients.
  //! {Property}
  //! \remark User side parameter, the user should setup the fill/stroke paint transform according to this parameter before rendering.
  virtual void __stdcall SetA(const sVec2f& aV) = 0;
  //! Get the direction start for linear gradient, or center for radial gradients.
  //! {Property}
  virtual sVec2f __stdcall GetA() const = 0;
  //! Set the direction end for linear gradient, or focal for radial gradients.
  //! {Property}
  //! \remark User side parameter, the user should setup the fill/stroke paint transform according to this parameter before rendering.
  virtual void __stdcall SetB(const sVec2f& aV) = 0;
  //! Get the direction end for linear gradient, or focal for radial gradients.
  //! {Property}
  virtual sVec2f __stdcall GetB() const = 0;
  //! Set the distance scale for linear gradient, or radius for radial gradients.
  //! {Property}
  //! \remark User side parameter, the user should setup the fill/stroke paint transform according to this parameter before rendering.
  virtual void __stdcall SetR(tF32 afV) = 0;
  //! Get the distance scale for linear gradient, or radius for radial gradients.
  //! {Property}
  virtual tF32 __stdcall GetR() const = 0;

  //! Set the paint units.
  //! {Property}
  //! \remark User side parameter, the user should setup the fill/stroke paint transform according to this parameter before rendering.
  virtual void __stdcall SetUnits(eVGPaintUnits aUnits) = 0;
  //! Get the paint units.
  //! {Property}
  virtual eVGPaintUnits __stdcall GetUnits() const = 0;

  //! Get the gradient table.
  //! {Property}
  virtual iVGGradientTable* __stdcall GetGradientTable() const = 0;
};

#endif // niMinFeatures

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVGPAINT_9917824_H__
