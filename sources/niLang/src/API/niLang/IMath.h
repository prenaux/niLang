#pragma once
#ifndef __IMATH_82899656_H__
#define __IMATH_82899656_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"
#include "Utils/CollectionImpl.h"
#include "Math/MathRect.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Values returned by the classification functions
//! \remark Classification values can be combined as flags.
enum eClassify
{
  eClassify_Front   = niBit(0),
  eClassify_Back    = niBit(1),
  eClassify_Coplanar  = niBit(2),
  eClassify_Spanned = niBit(3)
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Math interface.
//! \remark The simple operations, +, -, *, /, ==, etc. are not implemented in this interface,
//!     they are expected to be implemented natively into the target language.
struct iMath : public iUnknown
{
  niDeclareInterfaceUUID(iMath,0x4b3c51e1,0x7b6d,0x4875,0x99,0x61,0x1d,0x43,0xd2,0xcb,0x5b,0x88)

  //########################################################################################
  //! \name Standard.
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Square root.
  virtual tF64 __stdcall Sqrt(tF64 v) const = 0;
  //! Sinus.
  virtual tF64 __stdcall Sin(tF64 v) const = 0;
  //! Cosinus.
  virtual tF64 __stdcall Cos(tF64 v) const = 0;
  //! Arc Sinus.
  virtual tF64 __stdcall ASin(tF64 v) const = 0;
  //! Arc Cosinus.
  virtual tF64 __stdcall ACos(tF64 v) const = 0;
  //! Tan.
  virtual tF64 __stdcall Tan(tF64 v) const = 0;
  //! Arc Tangent.
  virtual tF64 __stdcall ATan(tF64 v) const = 0;
  //! Arc Tangent 2.
  virtual tF64 __stdcall ATan2(tF64 x, tF64 y) const = 0;
  //! Log base N.
  virtual tF64 __stdcall LogX(tF64 v, tF64 n) const = 0;
  //! Log base E.
  virtual tF64 __stdcall LogE(tF64 v) const = 0;
  //! Log base 2.
  virtual tF64 __stdcall Log2(tF64 v) const = 0;
  //! Log base 10.
  virtual tF64 __stdcall Log10(tF64 v) const = 0;
  //! Power.
  virtual tF64 __stdcall Pow(tF64 v, tF64 e) const = 0;
  //! Floor.
  virtual tF64 __stdcall Floor(tF64 v) const = 0;
  //! Ceil.
  virtual tF64 __stdcall Ceil(tF64 v) const = 0;
  //! Exponential.
  virtual tF64 __stdcall Exp(tF64 v) const = 0;
  //! Absolute value.
  virtual tF64 __stdcall Abs(tF64 v) const = 0;
  //! Convert degree to radian.
  virtual tF64 __stdcall ToRad(tF64 afDeg) const = 0;
  //! Convert radian to degree.
  virtual tF64 __stdcall ToDeg(tF64 afRad) const = 0;
  //! Minimum between a and b.
  virtual tF64 __stdcall Min(tF64 a, tF64 b) const = 0;
  //! Minimum between a, b and c.
  virtual tF64 __stdcall Min3(tF64 a, tF64 b, tF64 c) const = 0;
  //! Maximum between a and b.
  virtual tF64 __stdcall Max(tF64 a, tF64 b) const = 0;
  //! Maximum between a, b and c.
  virtual tF64 __stdcall Max3(tF64 a, tF64 b, tF64 c) const = 0;
  //! Clamp the specified number between min and max.
  virtual tF64 __stdcall Clamp(tF64 v, tF64 afMin, tF64 afMax) const = 0;
  //! Clamp the specified number between zero and one.
  virtual tF64 __stdcall ClampZeroOne(tF64 a) const = 0;
  //! Linear interpolation between a and b.
  //! \remark The formula used is a + ((b-a)*f).
  virtual tF64 __stdcall Lerp(tF64 a, tF64 b, tF64 f) const = 0;
  //! Blends new values into an accumulator to produce a smoothed time series.
  //!
  //! \param accumulator is the current value of the accumulator
  //! \param newValue is the new value to accumulate
  //! \param smoothRate typically made proportional to "frameTime". If
  //!            smoothRate is 0 the accumulator will not change, if smoothRate
  //!            is 1 the accumulator is set to the new value (no smoothing),
  //!            useful values are "near zero".
  //! \return the new smoothed value of the accumulator
  virtual tF64 __stdcall BlendIntoAccumulator(tF64 accumulator, tF64 newValue, tF64 smoothRate) const = 0;

#endif
  //! @}

  //########################################################################################
  //! \name Random numbers
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Randomize.
  virtual void __stdcall RandSeed(tU32 ulSeed) const = 0;
  //! Compute a random integer number.
  virtual tI32 __stdcall RandInt() const = 0;
  //! Compute a random integer number between a specified range.
  virtual tI32 __stdcall RandIntRange(tI32 aMin, tI32 aMax) const = 0;
  //! Compute a random float number.
  virtual tF32 __stdcall RandFloat() const = 0;
  //! Compute a random float number between a specified range.
  virtual tF32 __stdcall RandFloatRange(tF32 afMin, tF32 afMax) const = 0;
  //! Compute a random number with a normal distribution.
  virtual tF32 __stdcall RandNormal(tF32 sigma) const = 0;
  //! Generates a random direction in the upper hemisphere.
  //! \remark Where is the upper hemisphere is defined by the specified normal.
  virtual sVec3f __stdcall RandomDirection(const sVec3f& vN) const = 0;
  //! Generates a random direction in the upper hemisphere.
  //! \param  avN indicates the basis/direction of the hemisphere.
  //! \param  afDeviAngle represents the spreading range of the ray, 180deg
  //!     (pi rad) will allow the ray to be spread away from -90, to 90
  //!     degrees, 90deg (pi/2 rad) will allow the ray to be spread away
  //!     from -45 to 45 degrees, and so on...
  //! \remark Where is the upper hemisphere is defined by the specified normal.
  virtual sVec3f __stdcall RandomDirectionEx(const sVec3f& avN, tF32 afDeviAngle) const = 0;
  //! Rotates a ray to have avN as basis.
  virtual sVec3f __stdcall RotateRay(const sVec3f& avIn, const sVec3f& avN) const = 0;
  //! Return a random color with a specified alpha.
  virtual tU32 __stdcall RandColorA(tU8 aA) const = 0;
  //! Return a random 32 bit color with specified alpha.
  virtual tU32 __stdcall RandColorAf(tF32 aA) const = 0;
  //! Return a random 32 bit color.
  virtual tU32 __stdcall RandColor() const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Vector 2D operations.
  //########################################################################################
  //! @{
#if niMinFeatures(15)

  //! Add.
  virtual sVec2f __stdcall Vec2Add(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! Subtract.
  virtual sVec2f __stdcall Vec2Sub(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! Multiply.
  virtual sVec2f __stdcall Vec2Mul(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! Divide.
  virtual sVec2f __stdcall Vec2Div(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! Scale.
  virtual sVec2f __stdcall Vec2Scale(const sVec2f& aLeft, tF32 afRight) const = 0;
  //! Compare.
  virtual tI32 __stdcall Vec2Compare(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! Length.
  virtual tF32 __stdcall Vec2Length(const sVec2f& aV) const = 0;
  //! Length squared.
  virtual tF32 __stdcall Vec2LengthSq(const sVec2f& aV) const = 0;
  //! Normalize.
  virtual sVec2f __stdcall Vec2Normalize(const sVec2f& aV) const = 0;
  //! Return eTrue if it's a normal vector.
  virtual tBool __stdcall Vec2IsNormal(const sVec2f& aV) const = 0;
  //! Dot product.
  virtual tF32 __stdcall Vec2Dot(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! Lerp.
  virtual sVec2f __stdcall Vec2Lerp(const sVec2f& aLeft, const sVec2f& aRight, tF32 afF) const = 0;
  //! Lerp.
  virtual sVec2f __stdcall Vec2Lerp2(const sVec2f& aLeft, const sVec2f& aRight, const sVec2f& aF) const = 0;
  //! Blends new values into an accumulator to produce a smoothed time series.
  //!
  //! \param accumulator is the current value of the accumulator
  //! \param newValue is the new value to accumulate
  //! \param smoothRate typically made proportional to "frameTime". If
  //!            smoothRate is 0 the accumulator will not change, if smoothRate
  //!            is 1 the accumulator is set to the new value (no smoothing),
  //!            useful values are "near zero".
  //! \return the new smoothed value of the accumulator
  virtual sVec2f __stdcall Vec2BlendIntoAccumulator(const sVec2f &accumulator, const sVec2f &newValue, tF32 smoothRate) const = 0;
  //! Abs.
  virtual sVec2f __stdcall Vec2Abs(const sVec2f& aV) const = 0;
  //! Min.
  virtual sVec2f __stdcall Vec2Min(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! Max.
  virtual sVec2f __stdcall Vec2Max(const sVec2f& aLeft, const sVec2f& aRight) const = 0;
  //! CatmullRom spline.
  virtual sVec2f __stdcall Vec2CatmullRom(const sVec2f& V1,
                                             const sVec2f& V2,
                                             const sVec2f& V3,
                                             const sVec2f& V4,
                                             tF32 s) const = 0;
  //! Hermite spline.
  virtual sVec2f __stdcall Vec2Hermite(const sVec2f& V1,
                                          const sVec2f& V2,
                                          const sVec2f& V3,
                                          const sVec2f& V4,
                                          tF32 s) const = 0;
  //! BaryCentric.
  virtual sVec2f __stdcall Vec2BaryCentric(const sVec2f& V1,
                                              const sVec2f& V2,
                                              const sVec2f& V3,
                                              tF32 u, tF32 v) const = 0;
  //! Transform by the specified matrix.
  virtual sVec4f __stdcall Vec2Transform(const sVec2f& aVec, const sMatrixf& aMatrix) const = 0;
  //! Transform by the specified matrix.
  virtual sVec2f __stdcall Vec2TransformCoord(const sVec2f& aVec, const sMatrixf& aMatrix) const = 0;
  //! Transform by the specified matrix.
  virtual sVec2f __stdcall Vec2TransformNormal(const sVec2f& aVec, const sMatrixf& aMatrix) const = 0;
  //! Transform an array by the specified matrix.
  virtual void __stdcall Vec2TransformCoordArray(tVec2fCVec* apVecs, const sMatrixf& aMatrix) const = 0;
  //! Transform an array by the specified matrix.
  virtual void __stdcall Vec2TransformNormalArray(tVec2fCVec* apVecs, const sMatrixf& aMatrix) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Vector 3D operations.
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Add.
  virtual sVec3f __stdcall Vec3Add(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Subtract.
  virtual sVec3f __stdcall Vec3Sub(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Multiply.
  virtual sVec3f __stdcall Vec3Mul(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Divide.
  virtual sVec3f __stdcall Vec3Div(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Scale.
  virtual sVec3f __stdcall Vec3Scale(const sVec3f& aLeft, tF32 afRight) const = 0;
  //! Compare.
  virtual tI32 __stdcall Vec3Compare(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Length.
  virtual tF32 __stdcall Vec3Length(const sVec3f& aV) const = 0;
  //! Length squared.
  virtual tF32 __stdcall Vec3LengthSq(const sVec3f& aV) const = 0;
  //! Normalize.
  virtual sVec3f __stdcall Vec3Normalize(const sVec3f& aV) const = 0;
  //! Return eTrue if it's a normal vector.
  virtual tBool __stdcall Vec3IsNormal(const sVec3f& aV) const = 0;
  //! Dot product.
  virtual tF32 __stdcall Vec3Dot(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Cross product.
  virtual sVec3f __stdcall Vec3Cross(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Lerp.
  virtual sVec3f __stdcall Vec3Lerp(const sVec3f& aLeft, const sVec3f& aRight, tF32 afF) const = 0;
  //! Lerp.
  virtual sVec3f __stdcall Vec3Lerp2(const sVec3f& aLeft, const sVec3f& aRight, const sVec3f& aF) const = 0;
  //! Blends new values into an accumulator to produce a smoothed time series.
  //!
  //! \param accumulator is the current value of the accumulator
  //! \param newValue is the new value to accumulate
  //! \param smoothRate typically made proportional to "frameTime". If
  //!            smoothRate is 0 the accumulator will not change, if smoothRate
  //!            is 1 the accumulator is set to the new value (no smoothing),
  //!            useful values are "near zero".
  //! \return the new smoothed value of the accumulator
  virtual sVec3f __stdcall Vec3BlendIntoAccumulator(const sVec3f &accumulator, const sVec3f &newValue, tF32 smoothRate) const = 0;
  //! Abs.
  virtual sVec3f __stdcall Vec3Abs(const sVec3f& aV) const = 0;
  //! Min.
  virtual sVec3f __stdcall Vec3Min(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! Max.
  virtual sVec3f __stdcall Vec3Max(const sVec3f& aLeft, const sVec3f& aRight) const = 0;
  //! CatmullRom spline.
  virtual sVec3f __stdcall Vec3CatmullRom(const sVec3f& V1,
                                             const sVec3f& V2,
                                             const sVec3f& V3,
                                             const sVec3f& V4,
                                             tF32 s) const = 0;
  //! Hermite spline.
  virtual sVec3f __stdcall Vec3Hermite(const sVec3f& V1,
                                          const sVec3f& V2,
                                          const sVec3f& V3,
                                          const sVec3f& V4,
                                          tF32 s) const = 0;
  //! BaryCentric.
  virtual sVec3f __stdcall Vec3BaryCentric(const sVec3f& V1,
                                              const sVec3f& V2,
                                              const sVec3f& V3,
                                              tF32 u, tF32 v) const = 0;
  //! Transform by the specified matrix.
  virtual sVec4f __stdcall Vec3Transform(const sVec3f& aVec, const sMatrixf& aMatrix) const = 0;
  //! Transform by the specified matrix.
  virtual sVec3f __stdcall Vec3TransformCoord(const sVec3f& aVec, const sMatrixf& aMatrix) const = 0;
  //! Transform by the specified matrix.
  virtual sVec3f __stdcall Vec3TransformNormal(const sVec3f& aVec, const sMatrixf& aMatrix) const = 0;
  //! Transform an array by the specified matrix.
  virtual void __stdcall Vec3TransformCoordArray(tVec3fCVec* apVecs, const sMatrixf& aMatrix) const = 0;
  //! Transform an array by the specified matrix.
  virtual void __stdcall Vec3TransformNormalArray(tVec3fCVec* apVecs, const sMatrixf& aMatrix) const = 0;
  //! Unproject a vector.
  virtual sVec3f __stdcall Vec3Unproject(const sVec3f& avIn, const sRectf& aVP, const sMatrixf& amtxViewProj) const = 0;
  //! Project a vector.
  virtual sVec3f __stdcall Vec3Project(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) const = 0;
  //! Project a vector.
  virtual sVec4f __stdcall Vec3ProjectRHW(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) const = 0;
  //! Returns a vector reflected about the plane indicated by the specified normal.
  //! \remark The formula is : R = (Dir - (Normal*Normal.dot(Dir)*2.0)
  //! \remark The input is assumed to have been normalized
  virtual sVec3f __stdcall Vec3Reflect(const sVec3f& Dir, const sVec3f& Normal) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Vector 4D operations.
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Add.
  virtual sVec4f __stdcall Vec4Add(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! Subtract.
  virtual sVec4f __stdcall Vec4Sub(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! Multiply.
  virtual sVec4f __stdcall Vec4Mul(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! Divide.
  virtual sVec4f __stdcall Vec4Div(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! Scale.
  virtual sVec4f __stdcall Vec4Scale(const sVec4f& aLeft, tF32 afRight) const = 0;
  //! Compare.
  virtual tI32 __stdcall Vec4Compare(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! Length.
  virtual tF32 __stdcall Vec4Length(const sVec4f& aV) const = 0;
  //! Length squared.
  virtual tF32 __stdcall Vec4LengthSq(const sVec4f& aV) const = 0;
  //! Normalize.
  virtual sVec4f __stdcall Vec4Normalize(const sVec4f& aV) const = 0;
  //! Return eTrue if it's a normal vector.
  virtual tBool __stdcall Vec4IsNormal(const sVec4f& aV) const = 0;
  //! Dot product.
  virtual tF32 __stdcall Vec4Dot(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! Cross product.
  virtual sVec4f __stdcall Vec4Cross(const sVec4f& aA, const sVec4f& aB, const sVec4f& aC) const = 0;
  //! Lerp.
  virtual sVec4f __stdcall Vec4Lerp(const sVec4f& aLeft, const sVec4f& aRight, tF32 afF) const = 0;
  //! Lerp.
  virtual sVec4f __stdcall Vec4Lerp2(const sVec4f& aLeft, const sVec4f& aRight, const sVec4f& aF) const = 0;
  //! Blends new values into an accumulator to produce a smoothed time series.
  //!
  //! \param accumulator is the current value of the accumulator
  //! \param newValue is the new value to accumulate
  //! \param smoothRate typically made proportional to "frameTime". If
  //!            smoothRate is 0 the accumulator will not change, if smoothRate
  //!            is 1 the accumulator is set to the new value (no smoothing),
  //!            useful values are "near zero".
  //! \return the new smoothed value of the accumulator
  virtual sVec4f __stdcall Vec4BlendIntoAccumulator(const sVec4f &accumulator, const sVec4f &newValue, tF32 smoothRate) const = 0;
  //! Abs.
  virtual sVec4f __stdcall Vec4Abs(const sVec4f& aV) const = 0;
  //! Min.
  virtual sVec4f __stdcall Vec4Min(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! Max.
  virtual sVec4f __stdcall Vec4Max(const sVec4f& aLeft, const sVec4f& aRight) const = 0;
  //! CatmullRom spline.
  virtual sVec4f __stdcall Vec4CatmullRom(const sVec4f& V1,
                                             const sVec4f& V2,
                                             const sVec4f& V3,
                                             const sVec4f& V4,
                                             tF32 s) const = 0;
  //! Hermite spline.
  virtual sVec4f __stdcall Vec4Hermite(const sVec4f& V1,
                                          const sVec4f& V2,
                                          const sVec4f& V3,
                                          const sVec4f& V4,
                                          tF32 s) const = 0;
  //! BaryCentric.
  virtual sVec4f __stdcall Vec4BaryCentric(const sVec4f& V1,
                                              const sVec4f& V2,
                                              const sVec4f& V3,
                                              tF32 u, tF32 v) const = 0;
  //! Transform by the specified matrix.
  virtual sVec4f __stdcall Vec4Transform(const sVec4f& aVec, const sMatrixf& aMatrix) const = 0;
  //! Transform an array by the specified matrix.
  virtual void __stdcall Vec4TransformArray(tVec4fCVec* apVecs, const sMatrixf& aMatrix) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Matrix operations.
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Get the identity matrix.
  virtual sMatrixf __stdcall MatrixIdentity() const = 0;
  //! Check if a matrix is identity.
  virtual tBool __stdcall MatrixIsIdentity(const sMatrixf& M) const = 0;
  //! Add.
  virtual sMatrixf __stdcall MatrixAdd(const sMatrixf& aLeft, const sMatrixf& aRight) const = 0;
  //! Subtract.
  virtual sMatrixf __stdcall MatrixSub(const sMatrixf& aLeft, const sMatrixf& aRight) const = 0;
  //! Scale.
  virtual sMatrixf __stdcall MatrixScale(const sMatrixf& aLeft, tF32 afRight) const = 0;
  //! Lerp.
  virtual sMatrixf __stdcall MatrixLerp(const sMatrixf& A, const sMatrixf& B, tF32 fFac) const = 0;
  //! Compare.
  virtual tI32 __stdcall MatrixCompare(const sMatrixf& aLeft, const sMatrixf& aRight) const = 0;
  //! Multiply two matrix.
  virtual sMatrixf __stdcall MatrixMultiply(const sMatrixf& M1, const sMatrixf& M2) const = 0;
  //! Compute the top left 2x2 matrix determinant.
  virtual tF32 __stdcall MatrixDeterminant2(const sMatrixf& M) const = 0;
  //! Compute the top left 3x3 matrix determinant.
  virtual tF32 __stdcall MatrixDeterminant3(const sMatrixf& M) const = 0;
  //! Compute matrix determinant.
  virtual tF32 __stdcall MatrixDeterminant(const sMatrixf& M) const = 0;
  //! Compute matrix handeness.
  //! \return a number > +1 if left handed and -1 if right handed
  virtual tF32 __stdcall MatrixGetHandeness(const sMatrixf& M) const = 0;
  //! Inverse the given matrix.
  virtual sMatrixf __stdcall MatrixInverse(const sMatrixf& M) const = 0;
  //! Inverse the transformation part. Means transpose the rotation and negate the translation.
  virtual sMatrixf __stdcall MatrixTransformInverse(const sMatrixf& M) const = 0;
  //! Make a right handed look at matrix.
  virtual sMatrixf __stdcall MatrixLookAtRH(const sVec3f& Eye, const sVec3f& At, const sVec3f& Up) const = 0;
  //! Make a left handed look at matrix.
  virtual sMatrixf __stdcall MatrixLookAtLH(const sVec3f& Eye, const sVec3f& At, const sVec3f& Up) const = 0;
  //! Make a right handed orthogonal projection matrix.
  virtual sMatrixf __stdcall MatrixOrthoRH(tF32 w, tF32 h, tF32 zn, tF32 zf) const = 0;
  //! Make a left handed orthogonal projection matrix.
  virtual sMatrixf __stdcall MatrixOrthoLH(tF32 w, tF32 h, tF32 zn, tF32 zf) const = 0;
  //! Make a right handed orthogonal projection matrix.
  virtual sMatrixf __stdcall MatrixOrthoOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const = 0;
  //! Make a left handed orthogonal projection matrix.
  virtual sMatrixf __stdcall MatrixOrthoOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const = 0;
  //! Make a right handed perspective projection matrix.
  virtual sMatrixf __stdcall MatrixPerspectiveRH(tF32 w, tF32 h, tF32 zn, tF32 zf) const = 0;
  //! Make a left handed perspective projection matrix.
  virtual sMatrixf __stdcall MatrixPerspectiveLH(tF32 w, tF32 h, tF32 zn, tF32 zf) const = 0;
  //! Make a right handed perspective projection matrix.
  virtual sMatrixf __stdcall MatrixPerspectiveFovRH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) const = 0;
  //! Make a left handed perspective projection matrix.
  virtual sMatrixf __stdcall MatrixPerspectiveFovLH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) const = 0;
  //! Make a right handed perspective projection matrix.
  virtual sMatrixf __stdcall MatrixPerspectiveOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const = 0;
  //! Make a left handed perspective projection matrix.
  virtual sMatrixf __stdcall MatrixPerspectiveOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const = 0;
  //! Make a reflection matrix.
  virtual sMatrixf __stdcall MatrixReflect(const sPlanef& Plane) const = 0;
  //! Make a rotation matrix.
  virtual sMatrixf __stdcall MatrixRotationAxis(const sVec3f& V, tF32 angle) const = 0;
  //! Make a rotation matrix.
  virtual sMatrixf __stdcall MatrixRotationQuat(const sQuatf& Q) const = 0;
  //! Make a rotation matrix.
  virtual sMatrixf __stdcall MatrixRotationX(tF32 angle) const = 0;
  //! Make a rotation matrix.
  virtual sMatrixf __stdcall MatrixRotationY(tF32 angle) const = 0;
  //! Make a rotation matrix.
  virtual sMatrixf __stdcall MatrixRotationZ(tF32 angle) const = 0;
  //! Make a rotation matrix.
  virtual sMatrixf __stdcall MatrixRotationYawPitchRoll(tF32 yaw, tF32 pitch, tF32 roll) const = 0;
  //! Extract the rotation contained in the provided matrix as yaw/heading/Y,
  //! pitch/X and roll/bank/Z in radians.
  //! \see MatrixRotationYawPitchRoll
  virtual sVec3f __stdcall MatrixDecomposeYawPitchRoll(const sMatrixf& M) const = 0;
  //! Make a scaling matrix.
  virtual sMatrixf __stdcall MatrixScaling(const sVec3f& V) const = 0;
  //! Scale a matrix.
  virtual sMatrixf __stdcall MatrixVecScale(const sMatrixf& M, const sVec3f& V) const = 0;
  //! Make a shadow projection matrix.
  virtual sMatrixf __stdcall MatrixShadow(const sVec4f& vLight, const sPlanef& Plane) const = 0;
  //! Make a translation matrix.
  virtual sMatrixf __stdcall MatrixTranslation(const sVec3f& V) const = 0;
  //! Transpose a matrix.
  virtual sMatrixf __stdcall MatrixTranspose(const sMatrixf& M) const = 0;
  //! Transpose a matrix.
  virtual sMatrixf __stdcall MatrixTranspose3x3(const sMatrixf& M) const = 0;
  //! Texture offset matrix.
  virtual sMatrixf __stdcall MatrixTextureOffset(tF32 fBias, tI32 nTexW, tI32 nTexH) = 0;
  //! Texture offset matrix that takes in account a bit depth.
  virtual sMatrixf __stdcall MatrixTextureOffset2(tF32 fBias, tI32 nTexW, tI32 nTexH, tI32 nDepthBits) = 0;
  //! Get the forward vector of a matrix.
  virtual sVec3f __stdcall MatrixGetForward(const sMatrixf& M) const = 0;
  //! Get the up vector of a matrix.
  virtual sVec3f __stdcall MatrixGetUp(const sMatrixf& M) const = 0;
  //! Get the right vector of a matrix.
  virtual sVec3f __stdcall MatrixGetRight(const sMatrixf& M) const = 0;
  //! Get the translation vector of a matrix.
  virtual sVec3f __stdcall MatrixGetTranslation(const sMatrixf& M) const = 0;
  //! Set the forward vector of a matrix.
  virtual sMatrixf __stdcall MatrixSetForward(const sMatrixf& M, const sVec3f& V) const = 0;
  //! Set the up vector of a matrix.
  virtual sMatrixf __stdcall MatrixSetUp(const sMatrixf& M, const sVec3f& V) const = 0;
  //! Set the right vector of a matrix.
  virtual sMatrixf __stdcall MatrixSetRight(const sMatrixf& M, const sVec3f& V) const = 0;
  //! Set the translation vector of a matrix.
  virtual sMatrixf __stdcall MatrixSetTranslation(const sMatrixf& M, const sVec3f& V) const = 0;
  //! Check if a matrix is normalized.
  virtual tBool __stdcall MatrixIsNormal(const sMatrixf& M) const = 0;
  //! Check if a matrix is orthogonal.
  virtual tBool __stdcall MatrixIsOrthogonal(const sMatrixf& M) const = 0;
  //! Check if a matrix is orthonormal.
  virtual tBool __stdcall MatrixIsOrthoNormal(const sMatrixf& M) const = 0;
  //! Set the rotation part of a matrix.
  virtual sMatrixf __stdcall MatrixSetRotation(const sMatrixf& In, const sMatrixf& RotM) const = 0;
  //! Get euler angles from a rotation matrix.
  virtual sVec3f __stdcall MatrixToEuler(const sMatrixf& M) const = 0;
  //! Make a coordinate system conversion matrix.
  virtual sMatrixf __stdcall MatrixToCoordinateSystem(const sVec3f& avRight,
                                                      const sVec3f& avUp,
                                                      const sVec3f& avFwd,
                                                      const sVec3f& avOrg) const = 0;
  //! Get the translation by 'unprojecting' the translation from the matrix's axis (translation of inverse matrix).
  //! \remark This can be used to get the world position from a view matrix.
  virtual sVec3f __stdcall MatrixGetProjectedTranslation(const sMatrixf& In) const = 0;
  //! Set the translation by projecting it onto the matrix's axis.
  //! \remark This can be used to set a world position into a view matrix.
  virtual sMatrixf __stdcall MatrixSetProjectedTranslation(const sMatrixf& In, const sVec3f& avT) const = 0;
  //! Set the non-rotation part of the matrix.
  virtual sMatrixf __stdcall MatrixSetNotRotation(const sMatrixf& In, const sMatrixf& M) const = 0;
  //! Rotate the matrix, this affects only the 3x3 top-left rotation part of the matrix.
  virtual sMatrixf __stdcall MatrixRotate(const sMatrixf& M1, const sMatrixf& M2) const = 0;
  //! Compute a viewport matrix.
  //! \remark a Viewport matrix converts from project coordinates to viewport/window/screen coordinates.
  //! \remark This is used to scale the projection to fit into a
  //!         viewport which is different of the current hardware
  //!         viewport. FinalProj = MatrixProjection
  //!                               * inverse(MatrixContextVP)
  //!                               * MatrixVirtualVP
  //! \remark MinZ/MaxZ is usually 0/1
  virtual sMatrixf __stdcall MatrixViewport(const sVec4f& aVP, tF32 afMinZ, tF32 afMaxZ) = 0;

  //! Compute a matrix that will compensate the projection matrix to fit into a viewport which is different of the context/physical viewport.
  //! \remark FinalProj = Proj * MatrixAdjustViewport(...)
  //! \remark MinZ/MaxZ is usually 0/1
  virtual sMatrixf __stdcall MatrixAdjustViewport(const sVec4f& aContextVP, const sVec4f& aVirtualVP, tF32 afMinZ, tF32 afMaxZ) = 0;

  //! Get the translation part of a matrix.
  virtual sVec3f __stdcall MatrixDecomposeGetTranslation(const sMatrixf& aMatrix) = 0;
  //! Get the rotation part of a matrix as euler angle in the Z*Y*X rotation order.
  virtual sVec3f __stdcall MatrixDecomposeGetZYX(const sMatrixf& aMatrix) = 0;
  //! Get the rotation part of a matrix as a quaternion.
  virtual sQuatf __stdcall MatrixDecomposeGetQuat(const sMatrixf& aMatrix) = 0;
  //! Get the local scaling part of a matrix (x,y,z,handness)
  virtual sVec4f __stdcall MatrixDecomposeGetScale(const sMatrixf& aMatrix) = 0;
  //! Build a matrix from a translation, zyx euler rotation and scale.
  virtual sMatrixf __stdcall MatrixCompose(const sVec3f& aT, const sVec3f& aZYX, const sVec4f& aS) = 0;
  //! Build a matrix from a translation, rotation quaternion and scale.
  virtual sMatrixf __stdcall MatrixComposeQ(const sVec3f& aT, const sQuatf& aQ, const sVec4f& aS) = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Quat operations.
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Get the identity quaternion.
  virtual sQuatf __stdcall QuatIdentity() const = 0;
  //! Check if the quaternion is identity.
  virtual tBool __stdcall QuatIsIdentity(const sQuatf& Q) const = 0;
  //! Add.
  virtual sQuatf __stdcall QuatAdd(const sQuatf& aLeft, const sQuatf& aRight) const = 0;
  //! Subtract.
  virtual sQuatf __stdcall QuatSub(const sQuatf& aLeft, const sQuatf& aRight) const = 0;
  //! Scale.
  virtual sQuatf __stdcall QuatScale(const sQuatf& aLeft, tF32 afRight) const = 0;
  //! Compare.
  virtual tI32 __stdcall QuatCompare(const sQuatf& aLeft, const sQuatf& aRight) const = 0;
  //! BaryCenter.
  virtual sQuatf __stdcall QuatBaryCentric(const sQuatf& Q1, const sQuatf& Q2, const sQuatf& Q3, tF32 f, tF32 g) const = 0;
  //! Quat conjugate.
  virtual sQuatf __stdcall QuatConjugate(const sQuatf& Q) const = 0;
  //! Quat dot.
  virtual tF32 __stdcall QuatDot(const sQuatf& Q1, const sQuatf& Q2) const = 0;
  //! Quat exponentials.
  virtual sQuatf __stdcall QuatExp(const sQuatf& Q) const = 0;
  //! Quat inverse.
  virtual sQuatf __stdcall QuatInverse(const sQuatf& Q) const = 0;
  //! Quat length.
  virtual tF32 __stdcall QuatLength(const sQuatf& Q) const = 0;
  //! Quat length squared.
  virtual tF32 __stdcall QuatLengthSq(const sQuatf& Q) const = 0;
  //! Calculates the natural logarithm.
  virtual sQuatf __stdcall QuatLn(const sQuatf& Q) const = 0;
  //! Quat multiply.
  virtual sQuatf __stdcall QuatMultiply(const sQuatf& Q1, const sQuatf& Q2) const = 0;
  //! Quat normalize.
  virtual sQuatf __stdcall QuatNormalize(const sQuatf& Q) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationAxis(const sVec3f& V, tF32 angle) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationX(tF32 angle) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationY(tF32 angle) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationZ(tF32 angle) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationXYZ(const sVec3f& V) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationMatrix(const sMatrixf& M) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationYawPitchRoll(const tF32 yaw, const tF32 pitch, const tF32 roll) const = 0;
  //! Rotation quaternion.
  virtual sQuatf __stdcall QuatRotationVector(const sVec3f& vFrom, const sVec3f& vTo) const = 0;
  //! Quat slerp.
  virtual sQuatf __stdcall QuatSlerp(const sQuatf& Q1, const sQuatf& Q2, tF32 t, eQuatSlerp mode) const = 0;
  //! Quat squad.
  virtual sQuatf __stdcall QuatSquad(const sQuatf& Q1, const sQuatf& Q2, const sQuatf& Q3, const sQuatf& Q4, tF32 t) const = 0;
  //! Quat to axis angle.
  virtual sVec4f __stdcall QuatToAxisAngle(const sQuatf& Q) const = 0;
  //! Quat to euler.
  virtual sVec3f __stdcall QuatToEuler(const sQuatf& Q) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Plane
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! 4D dot product between a plane and a 4d vector.
  virtual tF32 __stdcall PlaneDot(const sPlanef& P, const sVec4f& V) const = 0;
  //! 4D dot product between a plane and a 3d vector (w is assumed 1).
  virtual tF32 __stdcall PlaneDotCoord(const sPlanef& P, const sVec3f& V) const = 0;
  //! 3D dot product between a plane and a 3d vector.
  virtual tF32 __stdcall PlaneDotNormal(const sPlanef& P, const sVec3f& V) const = 0;
  //! Distance between a plane and the specified point.
  virtual tF32 __stdcall PlaneDistance(const sPlanef& P, const sVec3f& V) const = 0;
  //! Construct a plane from a point and a normal.
  virtual sPlanef __stdcall PlaneFromPointNormal(const sVec3f& vPoint, const sVec3f& vNormal) const = 0;
  //! Construct a plane from three points.
  virtual sPlanef __stdcall PlaneFromPoints(const sVec3f& V1, const sVec3f& V2, const sVec3f& V3) const = 0;
  //! Compute the intersection between a plane and a line.
  //! \return the intersection point (x,y,z), t is stored in the w component
  //! \remark returns a null vector if the line is parralel to the plane.
  virtual sVec4f __stdcall PlaneIntersectLine(const sPlanef& P, const sVec3f& V1, const sVec3f& V2) const = 0;
  //! Compute the intersection between a plane and a ray.
  //! \return the intersection point (x,y,z), t is stored in the w component
  //! \remark returns a null vector if the line is parralel to the plane.
  virtual sVec4f __stdcall PlaneIntersectRay(const sPlanef& P, const sVec3f& avRayPos, const sVec3f& avRayDir) const = 0;
  //! Normalize the specified plane.
  virtual sPlanef __stdcall PlaneNormalize(const sPlanef& P) const = 0;
  //! Transform the plane by the specified matrix.
  virtual sPlanef __stdcall PlaneTransform(const sPlanef& P, const sMatrixf& M) const = 0;
  //! Transform the plane by the inverse of the specified matrix.
  virtual sPlanef __stdcall PlaneTransformInversedMatrix(const sPlanef& P, const sMatrixf& M) const = 0;
  //! Get the intersection point between 3d planes.
  virtual sVec3f __stdcall PlaneIntersection(const sPlanef& PA, const sPlanef& PB, const sPlanef& PC) const = 0;
  //! Get the closest point on the plane to the specified vector.
  virtual sVec3f __stdcall PlaneClosest(const sPlanef& P, const sVec3f& A) const = 0;
  //! Get the type of plane.
  virtual ePlaneType __stdcall PlaneType(const sPlanef& Plane) const = 0;
  //! Get the type of plane's maximum component.
  virtual ePlaneType __stdcall PlaneMaxType(const sPlanef& Plane) const = 0;
  //! Extract a coordinate system from the plane.
  //! \param Plane
  //! \param avFwdDir: if not zero is the direction toward which the forward vector should be the closest.
  //!     For example if you want the forward vector to be the one that points down you'd pass sVec3f::OpYAxis() here.
  //! \remark Up vector (Y) is the plane normal.
  //! \remark Right (X) and Fwd (Z) lies on the plane.
  virtual sMatrixf __stdcall PlaneExtractCoordinateSystem(const sPlanef& Plane, const sVec3f& avFwdDir) const = 0;
  //! Check if the specified sphere intersect the specified plane.
  virtual tBool __stdcall PlaneIntersectSphere(const sPlanef& plane, const sVec3f& avCenter, const tF32 afRadius) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Triangles
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Compute the normal and area of a triangle.
  //! \return The normal is in xyz and the area in w.
  virtual sVec4f __stdcall TriangleAreaNormal(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const = 0;
  //! Check whether the triangle is degenerated.
  virtual tBool __stdcall TriangleIsDegenerate(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const = 0;
  //! Check triangle-triangle intersection.
  virtual tBool __stdcall TriangleIntersectTriangle(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& U0, const sVec3f& U1, const sVec3f& U2) const = 0;
  //! Check triangle-ray intersection.
  virtual sVec3f __stdcall TriangleIntersectRay(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const = 0;
  //! Check triangle-ray intersection with culling.
  virtual sVec3f __stdcall TriangleIntersectRayCull(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const = 0;
  //! Check triangle-aabb intersection.
  virtual tBool __stdcall TriangleIntersectAABB(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& avMin, const sVec3f& avMax) const = 0;
  //! Compute the barycentric coordinates of the specified point.
  virtual sVec3f __stdcall TriangleBaryCentric(const sVec3f& A, const sVec3f& B, const sVec3f& C, const sVec3f& P) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Utils
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Wrap a floating value between the specified min and max values.
  virtual tF32 __stdcall WrapFloat(tF32 aX, tF32 aMin, tF32 aMax) const = 0;
  //! Wrap an integer value between the specified min and max values.
  virtual tI32 __stdcall WrapInt(tI32 aX, tI32 aMin, tI32 aMax) const = 0;
  //! Wrap a radian between 0 and 2pi.
  virtual tF32 __stdcall WrapRad(tF32 aX) const = 0;
  //! Wrap a radian between -pi and pi.
  virtual tF32 __stdcall WrapRad2(tF32 aX) const = 0;
  //! Get the angle between two 2d vectors, returns an angle in [0;2pi]
  virtual tF32 __stdcall GetAngleFromPoints2D(const sVec2f& aStartPos, const sVec2f&  aGoalPos) const = 0;
  //! Get the angles between two 3d vectors.
  //! \remark The angles returned are on the sphere that can be visualized with StartPos as center and Radius as Len(aGoalPos-aStartPos).
  //!     <ul>
  //!     <li>Y is the yaw/left-right/latitude angle in radians, its domain is [0;2pi]</li>
  //!     <li>X is the pitch/up-down/longitude angle in radians, its domain is [0;pi], 0 is the bottom of the sphere, and pi the top of the sphere.</li>
  //!     </ul>
  virtual sVec2f __stdcall GetAngleFromPoints3D(const sVec3f& aStartPos, const sVec3f& aGoalPos) const = 0;
  //! Check whether a point is in-front, behind or on a plane.
  virtual eClassify __stdcall ClassifyPoint(const sPlanef& plane, const sVec3f& point) const = 0;
  //! Check whether a point is on the specified edge.
  virtual tBool __stdcall IsPointInsideEdge(const sVec3f& A, const sVec3f& B, const sVec3f& P) const = 0;
  //! Return the closest point to the specified vector P on the specified AB line.
  virtual sVec3f __stdcall ClosestPointOnLine(const sVec3f& A, const sVec3f& B, const sVec3f& P) const = 0;
  //! Return the closest point to the specified vector P on the specified ABC triangle.
  virtual sVec3f __stdcall ClosestPointOnTriangle(const sVec3f& A, const sVec3f& B, const sVec3f& C,const sVec3f& P) const = 0;
  //! Get the shortest 'distance' (angular difference) between two angles, no matter if the angles are in the [0;pi][-0;-pi] range or the [0;2pi] range.
  virtual tF32 __stdcall GetAngularDifference(tF32 afA, tF32 afB) const = 0;
  //! Get the screen position of the specified 3d world position.
  virtual sVec3f __stdcall GetScreenPosition(const sVec3f& avPos, const sMatrixf& mtxVP, const sRectf& aRect) const = 0;
  //! Convert a 3d spherical coordinate to a 3d cartesian coordinate.
  virtual sVec3f __stdcall Vec3SphericalToCartesian(const sVec3f& avS) const = 0;
  //! Convert a 3d cartesian coordinate to a 3d spherical coordinate.
  virtual sVec3f __stdcall Vec3CartesianToSpherical(const sVec3f& avC) const = 0;
  //! Orbit/rotate a 3d position of the specified yaw (Y-axis) angle around the specified target.
  virtual sVec3f __stdcall Vec3OrbitPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afPitch) const = 0;
  //! Orbit/rotate a 3d position of the specified pitch (X-axis) angle around the specified target.
  virtual sVec3f __stdcall Vec3OrbitYaw(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw) const = 0;
  //! Orbit/rotate a 3d position of the specified yaw (Y-axis) and then pitch (X-axis) angle around the specified target.
  virtual sVec3f __stdcall Vec3OrbitYawPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw, tF32 afPitch) const = 0;
  //! Compute a pixel to world unit scale value, based on the X-axis delta.
  virtual tF32 __stdcall ComputePixelScaleX(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const = 0;
  //! Compute a pixel to world unit scale value, based on the Y-axis delta.
  virtual tF32 __stdcall ComputePixelScaleY(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const = 0;
  //! Compute the world size of the given pixel size, scale based on the X-axis delta.
  virtual tF32 __stdcall ComputePixelWorldSizeX(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const = 0;
  //! Compute the world size of the given pixel size, scale based on the Y-axis delta.
  virtual tF32 __stdcall ComputePixelWorldSizeY(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const = 0;
  //! Compute the world size of the given pixel size, scale based on the X and Y axis delta.
  virtual sVec2f __stdcall ComputePixelWorldSizeVec2(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const = 0;
  //! Compute the world size of the give pixel size, scale based on the axis with the highest resolution.
  virtual tF32 __stdcall ComputePixelWorldSize(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const = 0;
  //! Computes a 2x2 matrix determinant.
  virtual tF32 __stdcall Det2x2f(const tF32 a, const tF32 b, const tF32 c, const tF32 d) const = 0;
  //! Computes a 3x3 matrix determinant.
  virtual tF32 __stdcall Det3x3f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8) const = 0;
  //! Computes a 4x4 matrix determinant.
  virtual tF32 __stdcall Det4x4f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8, const tF32 a9, const tF32 a10, const tF32 a11, const tF32 a12, const tF32 a13, const tF32 a14, const tF32 a15) const = 0;
  //! Computes a 2x2 matrix determinant.
  virtual tF64 __stdcall Det2x2d(const tF64 a, const tF64 b, const tF64 c, const tF64 d) const = 0;
  //! Computes a 3x3 matrix determinant.
  virtual tF64 __stdcall Det3x3d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8) const = 0;
  //! Computes a 4x4 matrix determinant.
  virtual tF64 __stdcall Det4x4d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8, const tF64 a9, const tF64 a10, const tF64 a11, const tF64 a12, const tF64 a13, const tF64 a14, const tF64 a15) const = 0;
  //! Convert an horizontal fov to a vertical fov.
  virtual tF32 __stdcall FovHzToVt(tF32 afHFov, tF32 afWbyHAspect) const = 0;
  //! Convert a vertical fov to an horizontal fov.
  virtual tF32 __stdcall FovVtToHz(tF32 afVFov, tF32 afWbyHAspect) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Curves
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  virtual tF64 __stdcall ClampT(tF64 aX, tF64 aMin, tF64 aMax) const = 0;
  virtual tF64 __stdcall RepeatT(tF64 aX, tF64 aMin, tF64 aMax) const = 0;
  virtual tF64 __stdcall MirrorT(tF64 aX, tF64 aMin, tF64 aMax) const = 0;
  virtual tF64 __stdcall CycleT(tF64 aX, tF64 aMin, tF64 aMax) const = 0;
  virtual tF64 __stdcall CurveStep(tF64 a, tF64 b, tF64 t) const = 0;
  virtual tF64 __stdcall CurveLinear(tF64 V1, tF64 V2, tF64 s) const = 0;
  virtual tF64 __stdcall CurveCos(tF64 a, tF64 b, tF64 t) const = 0;
  virtual tF64 __stdcall CurveHermite(tF64 V1, tF64 T1, tF64 V2, tF64 T2, tF64 s) const = 0;
  virtual tF64 __stdcall CurveCatmullRom(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s) const = 0;
  virtual tF64 __stdcall CurveCardinal(tF64 V1, tF64 V2, tF64 s, tF64 a) const = 0;
  virtual tF64 __stdcall CurveCardinal4(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s, tF64 a) const = 0;
  virtual tF64 __stdcall CycleLinear(tF64 aX, tF64 aMin, tF64 aMax) const = 0;
  virtual tF64 __stdcall CycleCos(tF64 aX, tF64 aMin, tF64 aMax) const = 0;
  virtual tF64 __stdcall CycleCardinal(tF64 aX, tF64 aMin, tF64 aMax, tF64 a) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Probabilities
  //########################################################################################
  //! @{

  virtual tF64 __stdcall ProbSum(tF64CVec* apProbs) = 0;
  virtual tBool __stdcall ProbNormalize(tF64CVec* apProbs) = 0;
  virtual tBool __stdcall ProbSampleBuildAliasMethodArrays(const tF64CVec* apProbs, tF64CVec* apAMQ, tU32CVec* apAMA) = 0;
  virtual tBool __stdcall ProbSampleAliasMethod(tU32CVec* apResults, const tF64CVec* apAMQ, const tU32CVec* apAMA) = 0;
  //! @}
};

niExportFunc(ni::iMath*) GetMath();
niExportFunc(ni::iUnknown*) New_niLang_Math(const ni::Var&,const ni::Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IMATH_82899656_H__
