#ifndef __IVGSTYLE_8767038_H__
#define __IVGSTYLE_8767038_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
struct iVGPaint;

/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! VG Line Cap type.
enum eVGLineCap
{
  eVGLineCap_Butt = 0,
  eVGLineCap_Square = 1,
  eVGLineCap_Round = 2,
  //! \internal
  eVGLineCap_ForceDWORD = 0xFFFFFFFF
};

//! VG Line join.
enum eVGLineJoin
{
  eVGLineJoin_Miter = 0,
  eVGLineJoin_MiterRevert = 1,
  eVGLineJoin_MiterRound = 2,
  eVGLineJoin_Round = 3,
  eVGLineJoin_Bevel = 4,
  //! \internal
  eVGLineJoin_ForceDWORD = 0xFFFFFFFF
};

//! VG Inner join.
enum eVGInnerJoin
{
  eVGInnerJoin_Bevel = 0,
  eVGInnerJoin_Miter = 1,
  eVGInnerJoin_Jag = 2,
  eVGInnerJoin_Round = 3,
  //! \internal
  eVGInnerJoin_ForceDWORD = 0xFFFFFFFF
};

//! VGStyle inteface.
struct iVGStyle : public iUnknown
{
  niDeclareInterfaceUUID(iVGStyle,0xecc6259b,0x41cc,0x4d37,0xb1,0xfc,0x55,0xf6,0x48,0x5d,0x95,0x73)

  //########################################################################################
  //! \name Management
  //########################################################################################
  //! @{

  //! Create a copy of the style.
  //! \remark Only the top style will be copied, not the style's stack.
  virtual tBool __stdcall Copy(const iVGStyle* apStyle) = 0;
  //! Clone the style.
  virtual iVGStyle* __stdcall Clone() const = 0;
  //! Push the style states.
  virtual tBool __stdcall Push() = 0;
  //! Pop the style states.
  virtual tBool __stdcall Pop() = 0;
  //! Set the default states.
  virtual void __stdcall SetDefault() = 0;
  //! @}

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Set the global opacity.
  //! {Property}
  virtual void __stdcall SetOpacity(tF32 afOpacity) = 0;
  //! Get the global opacity.
  //! {Property}
  virtual tF32 __stdcall GetOpacity() const = 0;
  //! Set the line's cap.
  //! {Property}
  virtual void __stdcall SetLineCap(eVGLineCap aCap) = 0;
  //! Get the line's cap
  //! {Property}
  virtual eVGLineCap __stdcall GetLineCap() const = 0;
  //! Set the line's join.
  //! {Property}
  virtual void __stdcall SetLineJoin(eVGLineJoin aJoin) = 0;
  //! Get the line's join.
  //! {Property}
  virtual eVGLineJoin __stdcall GetLineJoin() const = 0;
  //! Set the inner join type.
  //! {Property}
  virtual void __stdcall SetInnerJoin(eVGInnerJoin aInnerJoin) = 0;
  //! Get the inner join type.
  //! {Property}
  virtual eVGInnerJoin __stdcall GetInnerJoin() const = 0;
  //! Set the miter limit.
  //! {Property}
  virtual void __stdcall SetMiterLimit(tF32 afMiterLimit) = 0;
  //! Get the miter limit.
  //! {Property}
  virtual tF32 __stdcall GetMiterLimit() const = 0;
  //! Set the smoothing.
  //! {Property}
  virtual void __stdcall SetSmooth(tF32 afSmooth) = 0;
  //! Get the smoothing.
  //! {Property}
  virtual tF32 __stdcall GetSmooth() const = 0;
  //! Set the current style color.
  //! {Property}
  virtual void __stdcall SetCurrentColor(const sColor4f& avColor) = 0;
  //! Get the current style color.
  //! {Property}
  virtual const sColor4f& __stdcall GetCurrentColor() const = 0;
  //! @}

  //########################################################################################
  //! \name Quality
  //########################################################################################
  //! @{

  //! Set the antialiasing.
  //! {Property}
  virtual void __stdcall SetAntiAliasing(tBool abAntiAliasing) = 0;
  //! Get whether antialiasing is enabled.
  //! {Property}
  virtual tBool __stdcall GetAntiAliasing() const = 0;
  //! Set the rasterizer's approximation scale.
  //! {Property}
  virtual void __stdcall SetRasterizerApproximationScale(tF32 afRasterizerApproximationScale) = 0;
  //! Get the rasterizer's approximation scale.
  //! {Property}
  virtual tF32 __stdcall GetRasterizerApproximationScale() const = 0;
  //! Set the tesselator's approximation scale.
  //! {Property}
  virtual void __stdcall SetTesselatorApproximationScale(tF32 afTesselatorApproximationScale) = 0;
  //! Get the tesselator's approximation scale.
  //! {Property}
  virtual tF32 __stdcall GetTesselatorApproximationScale() const = 0;

  //! @}

  //########################################################################################
  //! \name Stroke
  //########################################################################################
  //! @{

  //! Set stroke drawing.
  //! {Property}
  virtual void __stdcall SetStroke(tBool abStroke) = 0;
  //! Get whether stroke drawing is enabled.
  //! {Property}
  virtual tBool __stdcall GetStroke() const = 0;
  //! Set the stroke's width.
  //! {Property}
  virtual void __stdcall SetStrokeWidth(tF32 afWidth) = 0;
  //! Get the stroke's width.
  //! {Property}
  virtual tF32 __stdcall GetStrokeWidth() const = 0;
  //! Set whether the transformed path should be stroked.
  //! {Property}
  //! \remark If true, the path will be first transformed, and that new transformed path will
  //!     be stroked. So the size of the stroke (line) will be the same no matter what is
  //!     the scaling factor of the transform.
  virtual void __stdcall SetStrokeTransformed(tBool abStrokeTransformed) = 0;
  //! Get whether the transformed path should be stroked.
  //! {Property}
  virtual tBool __stdcall GetStrokeTransformed() const = 0;
  //! Set the stroke's paint.
  //! {Property}
  virtual tBool __stdcall SetStrokePaint(iVGPaint* apPaint) = 0;
  //! Get the stroke's paint.
  //! {Property}
  virtual iVGPaint* __stdcall GetStrokePaint() const = 0;
  //! Set the stroke's paint color.
  //! \remark Conveniance for GetStrokePaint()->SetColor()
  //! {Property}
  virtual void __stdcall SetStrokeColor(const sColor3f& avColor) = 0;
  //! Get the stroke's paint color.
  //! \remark Conveniance for GetStrokePaint()->GetColor()
  //! {Property}
  virtual sColor3f __stdcall GetStrokeColor() const = 0;
  //! Set the stroke's paint color.
  //! \remark Conveniance for GetStrokePaint()->SetColor()
  //! {Property}
  virtual void __stdcall SetStrokeColor4(const sColor4f& avColor) = 0;
  //! Get the stroke's paint color.
  //! \remark Conveniance for GetStrokePaint()->GetColor()
  //! {Property}
  virtual sColor4f __stdcall GetStrokeColor4() const = 0;
  //! Set the stroke opacity.
  //! {Property}
  //! \remark Conveniance for GetStrokePaint()->SetColor()
  virtual void __stdcall SetStrokeOpacity(tF32 afOpacity) = 0;
  //! Get the stroke opacity.
  //! {Property}
  //! \remark Conveniance for GetStrokePaint()->GetColor()
  virtual tF32 __stdcall GetStrokeOpacity() const = 0;
  //! Get the number of dashes.
  //! {Property}
  virtual tU32 __stdcall GetNumDashes() const = 0;
  //! Remove all dashes.
  virtual void __stdcall ClearDashes() = 0;
  //! Add a dash.
  //! \remark Vector2f(Position,Length)
  virtual void __stdcall AddDash(const sVec2f& aV) = 0;
  //! Remove a dash.
  virtual tBool __stdcall RemoveDash(tU32 anIndex) = 0;
  //! Get the dash at the specified index.
  //! {Property}
  virtual sVec2f __stdcall GetDash(tU32 anIndex) const = 0;
  //! Set the dash start.
  //! {Property}
  virtual void __stdcall SetDashStart(tF32 afDashStart) = 0;
  //! Get the dash start.
  //! {Property}
  virtual tF32 __stdcall GetDashStart() const = 0;

  //! @}

  //########################################################################################
  //! \name Filling
  //########################################################################################
  //! @{

  //! Set fill drawing.
  //! {Property}
  virtual void __stdcall SetFill(tBool abFill) = 0;
  //! Get whether fill drawing is enabled.
  //! {Property}
  virtual tBool __stdcall GetFill() const = 0;
  //! Set fill odd even.
  //! {Property}
  virtual void __stdcall SetFillEvenOdd(tBool abFillEvenOdd) = 0;
  //! Get whether the fill uses the odd even or non zero rules to fill polygons.
  //! {Property}
  virtual tBool __stdcall GetFillEvenOdd() const = 0;
  //! Set the filling expand.
  //! {Property}
  virtual void __stdcall SetFillExpand(tF32 afExpand) = 0;
  //! Get the filling expand.
  //! {Property}
  virtual tF32 __stdcall GetFillExpand() const = 0;
  //! Set the fill paint.
  //! {Property}
  virtual tBool __stdcall SetFillPaint(iVGPaint* apPaint) = 0;
  //! Get the fill paint.
  //! {Property}
  virtual iVGPaint* __stdcall GetFillPaint() const = 0;
  //! Set the fill color.
  //! \remark Conveniance for GetFillPaint()->SetColor()
  //! {Property}
  virtual void __stdcall SetFillColor(const sColor3f& avColor) = 0;
  //! Get the fill color.
  //! \remark Conveniance for GetFillPaint()->GetColor()
  //! {Property}
  virtual sColor3f __stdcall GetFillColor() const = 0;
  //! Set the fill color.
  //! \remark Conveniance for GetFillPaint()->SetColor()
  //! {Property}
  virtual void __stdcall SetFillColor4(const sColor4f& avColor) = 0;
  //! Get the fill color.
  //! \remark Conveniance for GetFillPaint()->GetColor()
  //! {Property}
  virtual sColor4f __stdcall GetFillColor4() const = 0;
  //! Set the fill opacity.
  //! {Property}
  //! \remark Conveniance for GetFillPaint()->SetColor()
  virtual void __stdcall SetFillOpacity(tF32 afOpacity) = 0;
  //! Get the fill opacity.
  //! {Property}
  //! \remark Conveniance for GetFillPaint()->GetColor()
  virtual tF32 __stdcall GetFillOpacity() const = 0;

  //! @}
};

#endif // niMinFeatures

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVGSTYLE_8767038_H__
