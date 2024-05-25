#ifndef __IVGTRANSFORM_39683212_H__
#define __IVGTRANSFORM_39683212_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! VG transform type.
enum eVGTransform
{
  //! Path to surface transform.
  eVGTransform_Path = 0,
  //! Image to surface transform.
  eVGTransform_Image = 1,
  //! Fill paint to user transform.
  eVGTransform_FillPaint = 2,
  //! Stroke paint to user transform.
  eVGTransform_StrokePaint = 3,
  //! \internal
  eVGTransform_Last niMaybeUnused = 4,
  //! \internal
  eVGTransform_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! VG transform value.
enum eVGTransformValue
{
  //! ScaleX (m0) component.
  eVGTransformValue_ScaleX = 0,
  //! RotationSkew0 (m1) component.
  eVGTransformValue_RotationSkew0 = 1,
  //! RotationSkew1 (m2) component.
  eVGTransformValue_RotationSkew1 = 2,
  //! RotationSkew1 (m3) component.
  eVGTransformValue_ScaleY = 3,
  //! RotationSkew1 (m4) component.
  eVGTransformValue_TranslateX = 4,
  //! RotationSkew1 (m5) component.
  eVGTransformValue_TranslateY = 5,
  //! \internal
  eVGTransformValue_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! VG transform interface.
struct iVGTransform : public iUnknown
{
  niDeclareInterfaceUUID(iVGTransform,0x399c48d6,0xe2ca,0x4560,0x82,0x66,0xc4,0x82,0x48,0x4e,0xbd,0x5d)

  //! Copy another transform.
  virtual tBool __stdcall Copy(const iVGTransform* apTransform) = 0;
  //! Clone this transform.
  virtual iVGTransform* __stdcall Clone() const = 0;

  //! Push the transform states on the stack.
  virtual tBool __stdcall Push() = 0;
  //! Pop the transfrom states from the stack.
  virtual tBool __stdcall Pop() = 0;

  //! Set the 3x2 transform matrix.
  //! {Property}
  virtual void __stdcall SetMatrix(const sMatrixf& aMatrix) = 0;
  //! Get the 3x2 transform matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrix() const = 0;

  //! Set a value of the transform.
  //! {Property}
  virtual void __stdcall SetValue(eVGTransformValue aType, tF32 afValue) = 0;
  //! Get a value of the transform.
  //! {Property}
  virtual tF32 __stdcall GetValue(eVGTransformValue aType) const = 0;

  //! Set all values.
  virtual void __stdcall SetValues(tF32 m0, tF32 m1, tF32 m2, tF32 m3, tF32 m4, tF32 m5) = 0;

  //! Set the transform to identity.
  virtual void __stdcall Identity() = 0;
  //! Invert the transform.
  virtual void __stdcall Invert() = 0;

  //! Check whether the transform is identity.
  //! {Property}
  virtual tBool __stdcall GetIsIdentity() const = 0;
  //! Check whether the transform is equal the passed transform.
  virtual tBool __stdcall IsEqual(const iVGTransform* apTransform) = 0;

  //! Get the transform's determinant.
  virtual tF32 __stdcall GetDeterminant() const = 0;

  //! Flip the transform's X-axis.
  virtual void __stdcall FlipX() = 0;
  //! Flip the transform's Y-axis.
  virtual void __stdcall FlipY() = 0;

  //! Transform a vertex.
  virtual sVec2f __stdcall Transform(const sVec2f& avV) = 0;
  //! Transform a vertex with the rotational part (2x2) of the transform only.
  virtual sVec2f __stdcall TransformRotate(const sVec2f& avV) = 0;

  //! Multiply the transform.
  virtual void __stdcall Multiply(const iVGTransform* apTransform) = 0;
  //! PreMultiply the transform.
  virtual void __stdcall PreMultiply(const iVGTransform* apTransform) = 0;

  //! Multiply the transform by a matrix.
  virtual void __stdcall MultiplyMatrix(const sMatrixf& aMatrix) = 0;
  //! PreMultiply the transform by a matrix.
  virtual void __stdcall PreMultiplyMatrix(const sMatrixf& aMatrix) = 0;

  //! Multiply the transform by values.
  virtual void __stdcall MultiplyValues(tF32 m0, tF32 m1, tF32 m2, tF32 m3, tF32 m4, tF32 m5) = 0;
  //! PreMultiply the transform by values.
  virtual void __stdcall PreMultiplyValues(tF32 m0, tF32 m1, tF32 m2, tF32 m3, tF32 m4, tF32 m5) = 0;

  //! Rotate the transform.
  virtual void __stdcall Rotate(tF32 afRadians) = 0;
  //! PreRotate the transform.
  virtual void __stdcall PreRotate(tF32 afRadians) = 0;

  //! Rotate the transform around the specified position.
  virtual void __stdcall RotateAround(const sVec2f& aV, tF32 afRadians) = 0;
  //! PreRotate the transform around the specified position.
  virtual void __stdcall PreRotateAround(const sVec2f& aV, tF32 afRadians) = 0;

  //! Translate the transform.
  virtual void __stdcall Translate(const sVec2f& aV) = 0;
  //! PreTranslate the transform.
  virtual void __stdcall PreTranslate(const sVec2f& aV) = 0;

  //! Scale the transform.
  virtual void __stdcall Scaling(const sVec2f& aV) = 0;
  //! PreScale the transform.
  virtual void __stdcall PreScaling(const sVec2f& aV) = 0;

  //! Skew the transform.
  virtual void __stdcall Skew(const sVec2f& aV) = 0;
  //! PreSkew the transform.
  virtual void __stdcall PreSkew(const sVec2f& aV) = 0;

  //! Generates a transform that point in the direction of a line segment.
  //! \remark If afDist > 0.0f it will be used to scale in function of the line's length.
  virtual void __stdcall LineSegment(const sVec2f& aStart, const sVec2f& aEnd, tF32 afDist) = 0;
  //! Generates a transform that point in the direction of a line segment.
  //! \see ni::iVGTransform::LineSegment
  virtual void __stdcall PreLineSegment(const sVec2f& aStart, const sVec2f& aEnd, tF32 afDist) = 0;

  //! Get the transform's rotation.
  //! {Property}
  //! \remark Could be inaccurate with degenerate transforms.
  virtual tF32 __stdcall GetRotation() const = 0;
  //! Get the transform's scale.
  //! {Property}
  //! \remark Could be inaccurate with degenerate transforms.
  virtual tF32 __stdcall GetScale() const = 0;
  //! Get the transform's translation.
  //! {Property}
  //! \remark Could be inaccurate with degenerate transforms.
  virtual sVec2f __stdcall GetTranslation() const = 0;
  //! Get the transform's scaling.
  //! {Property}
  //! \remark Could be inaccurate with degenerate transforms.
  virtual sVec2f __stdcall GetScaling() const = 0;
};

#endif // niMinFeatures

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVGTRANSFORM_39683212_H__
