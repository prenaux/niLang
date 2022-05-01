#ifndef __IVGGRADIENTTABLE_27968128_H__
#define __IVGGRADIENTTABLE_27968128_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IVGTransform.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! Gradient type.
enum eVGGradientType
{
  //! Linear gradient.
  eVGGradientType_Linear = 0,
  //! Radial/circle gradient.
  eVGGradientType_Radial = 1,
  //! Cross gradient.
  eVGGradientType_Cross = 2,
  //! Square root cross gradient.
  eVGGradientType_SqrtCross = 3,
  //! Conic gradient.
  eVGGradientType_Conic = 4,
  //! Diamond gradient.
  eVGGradientType_Diamond = 5,
  //! \internal
  eVGGradientType_ForceDWORD = 0xFFFFFFFF
};

//! Gradient table.
struct iVGGradientTable : public iUnknown
{
  niDeclareInterfaceUUID(iVGGradientTable,0xd508082d,0x9992,0x4eeb,0xa8,0xa7,0x0a,0xde,0xe6,0xe2,0x66,0x48)
  //! Copy another table.
  virtual tBool __stdcall Copy(const iVGGradientTable* apSrc) = 0;
  //! Clone this table.
  virtual iVGGradientTable* __stdcall Clone() const = 0;
  //! Get the size of the table.
  //! {Property}
  virtual tU32 __stdcall GetSize() const = 0;
  //! Set the color of the gradient at the specified index.
  //! {Property}
  virtual tBool __stdcall SetColor(tU32 anIndex, const sColor4f& aColor) = 0;
  //! Get the color of the gradient at the specified index.
  //! {Property}
  virtual sColor4f __stdcall GetColor(tU32 anIndex) const = 0;
  //! Set a range of color.
  //! {Property}
  virtual tBool __stdcall SetColorRange(tU32 anStart, tU32 anEnd, const sColor4f& avColor) = 0;
  //! Generate a gradient between two colors.
  virtual void __stdcall GenerateTwoColors(const sColor4f& aStartColor, const sColor4f& aEndColor) = 0;
  //! Generate a gradient between specified stops.
  virtual tBool __stdcall GenerateStops(const tF32CVec* apOffsets, const tVec4fCVec* apColors) = 0;
  //! Create an image containing the gradient.
  virtual iVGImage* __stdcall CreateImage(eVGGradientType aType, eVGWrapType aWrapType, const iVGTransform* apTransform, tU32 anWidth, tU32 anHeight, tI32 anD1, tI32 anD2) = 0;
};

#endif // niMinFeatures

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVGGRADIENTTABLE_27968128_H__
