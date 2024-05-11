// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/IMath.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/Random.h"
#include "API/niLang/Math/MathFPU.h"
#include "API/niLang/Math/MathVec2.h"
#include "API/niLang/Math/MathVec3.h"
#include "API/niLang/Math/MathVec4.h"
#include "API/niLang/Math/MathMatrix.h"
#include "API/niLang/Math/MathCurves.h"
#include "API/niLang/Math/MathPlane.h"
#include "API/niLang/Math/MathProb.h"
#include "API/niLang/Math/MathRect.h"
#include "API/niLang/Math/MathUtils.h"
#include "API/niLang/Math/MathTriangle.h"
#include "API/niLang/ILang.h"

#define IMPL_TRANSFORM(TYPE,VEC,TRAN)               \
  niAssert(t##TYPE##CVec::IsSameType(VEC));         \
  niCheckSilent(t##TYPE##CVec::IsSameType(VEC),;);  \
  s##TYPE* pData = apVecs->_Data();                 \
  niLoop(i,pData->size()) {                          \
    ni::TRAN(*pData,*pData,aMatrix);                \
    ++pData;                                        \
  }

using namespace ni;

//////////////////////////////////////////////////////////////////////////////////////////////
// cMath declaration.
class cMath : public ImplLocal<iMath>
{
  niBeginClass(cMath);

 public:
  //! Constructor.
  cMath();
  //! Destructor.
  ~cMath();

  //// iMath ////////////////////////////////////
#if niMinFeatures(15)
  tF64 __stdcall Sqrt(tF64 v) const;
  tF64 __stdcall Sin(tF64 v) const;
  tF64 __stdcall Cos(tF64 v) const;
  tF64 __stdcall ASin(tF64 v) const;
  tF64 __stdcall ACos(tF64 v) const;
  tF64 __stdcall Tan(tF64 v) const;
  tF64 __stdcall ATan(tF64 v) const;
  tF64 __stdcall ATan2(tF64 x, tF64 y) const;
  tF64 __stdcall LogX(tF64 v, tF64 base) const;
  tF64 __stdcall LogE(tF64 v) const;
  tF64 __stdcall Log2(tF64 v) const;
  tF64 __stdcall Log10(tF64 v) const;
  tF64 __stdcall Pow(tF64 v, tF64 e) const;
  tF64 __stdcall Floor(tF64 v) const;
  tF64 __stdcall Ceil(tF64 v) const;
  tF64 __stdcall Exp(tF64 v) const;
  tF64 __stdcall Abs(tF64 v) const;
  tF64 __stdcall ToRad(tF64 afDeg) const;
  tF64 __stdcall ToDeg(tF64 afRad) const;
  tF64 __stdcall Min(tF64 a, tF64 b) const;
  tF64 __stdcall Min3(tF64 a, tF64 b, tF64 c) const;
  tF64 __stdcall Max(tF64 a, tF64 b) const;
  tF64 __stdcall Max3(tF64 a, tF64 b, tF64 c) const;
  tF64 __stdcall Clamp(tF64 v, tF64 afMin, tF64 afMax) const;
  tF64 __stdcall ClampZeroOne(tF64 a) const;
  tF64 __stdcall Lerp(tF64 a, tF64 b, tF64 f) const;
  tF64 __stdcall BlendIntoAccumulator(tF64 accumulator, tF64 newValue, tF64 smoothRate) const;

  void __stdcall RandSeed(tU32 ulSeed) const;
  tI32 __stdcall RandInt() const;
  tI32 __stdcall RandIntRange(tI32 aMin, tI32 aMax) const;
  tF32 __stdcall RandFloat() const;
  tF32 __stdcall RandFloatRange(tF32 afMin, tF32 afMax) const;
  tF32 __stdcall RandNormal(tF32 sigma) const;
  sVec3f __stdcall RandomDirection(const sVec3f& vN) const;
  sVec3f __stdcall RandomDirectionEx(const sVec3f& vN, tF32 fDeviAngle) const;
  sVec3f __stdcall RotateRay(const sVec3f& avIn, const sVec3f& avN) const;
  tU32 __stdcall RandColorA(tU8 aA) const;
  tU32 __stdcall RandColorAf(tF32 aA) const;
  tU32 __stdcall RandColor() const;

  sVec2f __stdcall Vec2Add(const sVec2f &aLeft, const sVec2f &aRight) const;
  sVec2f __stdcall Vec2Sub(const sVec2f &aLeft, const sVec2f &aRight) const;
  sVec2f __stdcall Vec2Mul(const sVec2f &aLeft, const sVec2f &aRight) const;
  sVec2f __stdcall Vec2Div(const sVec2f &aLeft, const sVec2f &aRight) const;
  sVec2f __stdcall Vec2Scale(const sVec2f &aLeft, tF32 afRight) const;
  tI32 __stdcall Vec2Compare(const sVec2f &aLeft, const sVec2f &aRight) const;
  tF32 __stdcall Vec2Length(const sVec2f &aV) const;
  tF32 __stdcall Vec2LengthSq(const sVec2f &aV) const;
  sVec2f __stdcall Vec2Normalize(const sVec2f &aV) const;
  tBool __stdcall Vec2IsNormal(const sVec2f &aV) const;
  tF32 __stdcall Vec2Dot(const sVec2f &aLeft, const sVec2f &aRight) const;
  sVec2f __stdcall Vec2Lerp(const sVec2f &aLeft, const sVec2f &aRight, tF32 afF) const;
  sVec2f __stdcall Vec2BlendIntoAccumulator(const sVec2f &accumulator, const sVec2f &newValue, tF32 smoothRate) const;
  sVec2f __stdcall Vec2Lerp2(const sVec2f &aLeft, const sVec2f &aRight, const sVec2f &aF) const;
  sVec2f __stdcall Vec2Abs(const sVec2f &aV) const;
  sVec2f __stdcall Vec2Min(const sVec2f &aLeft, const sVec2f &aRight) const;
  sVec2f __stdcall Vec2Max(const sVec2f &aLeft, const sVec2f &aRight) const;
  sVec2f __stdcall Vec2CatmullRom(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, const sVec2f &V4, tF32 s) const;
  sVec2f __stdcall Vec2Hermite(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, const sVec2f &V4, tF32 s) const;
  sVec2f __stdcall Vec2BaryCentric(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, tF32 u, tF32 v) const;
  sVec4f __stdcall Vec2Transform(const sVec2f &aVec, const sMatrixf &aMatrix) const;
  sVec2f __stdcall Vec2TransformCoord(const sVec2f &aVec, const sMatrixf &aMatrix) const;
  sVec2f __stdcall Vec2TransformNormal(const sVec2f &aVec, const sMatrixf &aMatrix) const;
  void __stdcall Vec2TransformCoordArray(tVec2fCVec *apVecs, const sMatrixf &aMatrix) const;
  void __stdcall Vec2TransformNormalArray(tVec2fCVec *apVecs, const sMatrixf &aMatrix) const;

  sVec3f __stdcall Vec3Add(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3Sub(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3Mul(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3Div(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3Scale(const sVec3f &aLeft, tF32 afRight) const;
  tI32 __stdcall Vec3Compare(const sVec3f &aLeft, const sVec3f &aRight) const;
  tF32 __stdcall Vec3Length(const sVec3f &aV) const;
  tF32 __stdcall Vec3LengthSq(const sVec3f &aV) const;
  sVec3f __stdcall Vec3Normalize(const sVec3f &aV) const;
  tBool __stdcall Vec3IsNormal(const sVec3f &aV) const;
  tF32 __stdcall Vec3Dot(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3Cross(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3Lerp(const sVec3f &aLeft, const sVec3f &aRight, tF32 afF) const;
  sVec3f __stdcall Vec3Lerp2(const sVec3f &aLeft, const sVec3f &aRight, const sVec3f &aF) const;
  sVec3f __stdcall Vec3BlendIntoAccumulator(const sVec3f &accumulator, const sVec3f &newValue, tF32 smoothRate) const;
  sVec3f __stdcall Vec3Abs(const sVec3f &aV) const;
  sVec3f __stdcall Vec3Min(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3Max(const sVec3f &aLeft, const sVec3f &aRight) const;
  sVec3f __stdcall Vec3CatmullRom(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, const sVec3f &V4, tF32 s) const;
  sVec3f __stdcall Vec3Hermite(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, const sVec3f &V4, tF32 s) const;
  sVec3f __stdcall Vec3BaryCentric(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, tF32 u, tF32 v) const;
  sVec4f __stdcall Vec3Transform(const sVec3f &aVec, const sMatrixf &aMatrix) const;
  sVec3f __stdcall Vec3TransformCoord(const sVec3f &aVec, const sMatrixf &aMatrix) const;
  sVec3f __stdcall Vec3TransformNormal(const sVec3f &aVec, const sMatrixf &aMatrix) const;
  void __stdcall Vec3TransformCoordArray(tVec3fCVec *apVecs, const sMatrixf &aMatrix) const;
  void __stdcall Vec3TransformNormalArray(tVec3fCVec *apVecs, const sMatrixf &aMatrix) const;
  sVec3f __stdcall Vec3Unproject(const sVec3f &avIn, const sRectf &aVP, const sMatrixf &amtxViewProj) const;
  sVec3f __stdcall Vec3Project(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) const;
  sVec4f __stdcall Vec3ProjectRHW(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) const;
  sVec3f __stdcall Vec3Reflect(const sVec3f& Dir, const sVec3f& Normal) const;

  sVec4f __stdcall Vec4Add(const sVec4f &aLeft, const sVec4f &aRight) const;
  sVec4f __stdcall Vec4Sub(const sVec4f &aLeft, const sVec4f &aRight) const;
  sVec4f __stdcall Vec4Mul(const sVec4f &aLeft, const sVec4f &aRight) const;
  sVec4f __stdcall Vec4Div(const sVec4f &aLeft, const sVec4f &aRight) const;
  sVec4f __stdcall Vec4Scale(const sVec4f &aLeft, tF32 afRight) const;
  tI32 __stdcall Vec4Compare(const sVec4f &aLeft, const sVec4f &aRight) const;
  tF32 __stdcall Vec4Length(const sVec4f &aV) const;
  tF32 __stdcall Vec4LengthSq(const sVec4f &aV) const;
  sVec4f __stdcall Vec4Normalize(const sVec4f &aV) const;
  tBool __stdcall Vec4IsNormal(const sVec4f &aV) const;
  tF32 __stdcall Vec4Dot(const sVec4f &aLeft, const sVec4f &aRight) const;
  sVec4f __stdcall Vec4Cross(const sVec4f& aA, const sVec4f& aB, const sVec4f& aC) const;
  sVec4f __stdcall Vec4Lerp(const sVec4f &aLeft, const sVec4f &aRight, tF32 afF) const;
  sVec4f __stdcall Vec4Lerp2(const sVec4f &aLeft, const sVec4f &aRight, const sVec4f &aF) const;
  sVec4f __stdcall Vec4BlendIntoAccumulator(const sVec4f &accumulator, const sVec4f &newValue, tF32 smoothRate) const;
  sVec4f __stdcall Vec4Abs(const sVec4f &aV) const;
  sVec4f __stdcall Vec4Min(const sVec4f &aLeft, const sVec4f &aRight) const;
  sVec4f __stdcall Vec4Max(const sVec4f &aLeft, const sVec4f &aRight) const;
  sVec4f __stdcall Vec4CatmullRom(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, const sVec4f &V4, tF32 s) const;
  sVec4f __stdcall Vec4Hermite(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, const sVec4f &V4, tF32 s) const;
  sVec4f __stdcall Vec4BaryCentric(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, tF32 u, tF32 v) const;
  sVec4f __stdcall Vec4Transform(const sVec4f &aVec, const sMatrixf &aMatrix) const;
  void __stdcall Vec4TransformArray(tVec4fCVec *apVecs, const sMatrixf &aMatrix) const;

  sMatrixf __stdcall MatrixIdentity() const;
  tBool __stdcall MatrixIsIdentity(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixAdd(const sMatrixf &aLeft, const sMatrixf &aRight) const;
  sMatrixf __stdcall MatrixSub(const sMatrixf &aLeft, const sMatrixf &aRight) const;
  sMatrixf __stdcall MatrixScale(const sMatrixf &aLeft, tF32 afRight) const;
  sMatrixf __stdcall MatrixLerp(const sMatrixf &A, const sMatrixf &B, tF32 fFac) const;
  tI32 __stdcall MatrixCompare(const sMatrixf &aLeft, const sMatrixf &aRight) const;
  sMatrixf __stdcall MatrixMultiply(const sMatrixf &M1, const sMatrixf &M2) const;
  tF32 __stdcall MatrixDeterminant2(const sMatrixf &M) const;
  tF32 __stdcall MatrixDeterminant3(const sMatrixf &M) const;
  tF32 __stdcall MatrixDeterminant(const sMatrixf &M) const;
  tF32 __stdcall MatrixGetHandeness(const sMatrixf& M) const;
  sMatrixf __stdcall MatrixInverse(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixTransformInverse(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixLookAtRH(const sVec3f &Eye, const sVec3f &At, const sVec3f &Up) const;
  sMatrixf __stdcall MatrixLookAtLH(const sVec3f &Eye, const sVec3f &At, const sVec3f &Up) const;
  sMatrixf __stdcall MatrixOrthoRH(tF32 w, tF32 h, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixOrthoLH(tF32 w, tF32 h, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixOrthoOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixOrthoOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixPerspectiveRH(tF32 w, tF32 h, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixPerspectiveLH(tF32 w, tF32 h, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixPerspectiveFovRH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixPerspectiveFovLH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixPerspectiveOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixPerspectiveOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const;
  sMatrixf __stdcall MatrixReflect(const sPlanef &Plane) const;
  sMatrixf __stdcall MatrixRotationAxis(const sVec3f &V, tF32 angle) const;
  sMatrixf __stdcall MatrixRotationQuat(const sQuatf &Q) const;
  sMatrixf __stdcall MatrixRotationX(tF32 angle) const;
  sMatrixf __stdcall MatrixRotationY(tF32 angle) const;
  sMatrixf __stdcall MatrixRotationZ(tF32 angle) const;
  sMatrixf __stdcall MatrixRotationYawPitchRoll(tF32 yaw, tF32 pitch, tF32 roll) const;
  sVec3f __stdcall MatrixDecomposeYawPitchRoll(const sMatrixf& M) const;
  sMatrixf __stdcall MatrixScaling(const sVec3f &V) const;
  sMatrixf __stdcall MatrixVecScale(const sMatrixf &M, const sVec3f &V) const;
  sMatrixf __stdcall MatrixShadow(const sVec4f &vLight, const sPlanef &Plane) const;
  sMatrixf __stdcall MatrixTranslation(const sVec3f &V) const;
  sMatrixf __stdcall MatrixTranspose(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixTranspose3x3(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixTextureOffset(tF32 fBias, tI32 nTexW, tI32 nTexH);
  sMatrixf __stdcall MatrixTextureOffset2(tF32 fBias, tI32 nTexW, tI32 nTexH, tI32 nDepthBits);
  sVec3f __stdcall MatrixGetForward(const sMatrixf &M) const;
  sVec3f __stdcall MatrixGetUp(const sMatrixf &M) const;
  sVec3f __stdcall MatrixGetRight(const sMatrixf &M) const;
  sVec3f __stdcall MatrixGetTranslation(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixSetForward(const sMatrixf &M, const sVec3f &V) const;
  sMatrixf __stdcall MatrixSetUp(const sMatrixf &M, const sVec3f &V) const;
  sMatrixf __stdcall MatrixSetRight(const sMatrixf &M, const sVec3f &V) const;
  sMatrixf __stdcall MatrixSetTranslation(const sMatrixf &M, const sVec3f &V) const;
  tBool __stdcall MatrixIsNormal(const sMatrixf &M) const;
  tBool __stdcall MatrixIsOrthogonal(const sMatrixf &M) const;
  tBool __stdcall MatrixIsOrthoNormal(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixSetRotation(const sMatrixf &In, const sMatrixf &RotM) const;
  sVec3f __stdcall MatrixToEuler(const sMatrixf &M) const;
  sMatrixf __stdcall MatrixToCoordinateSystem(const sVec3f &avRight, const sVec3f &avUp, const sVec3f &avFwd, const sVec3f &avOrg) const;
  sVec3f __stdcall MatrixGetProjectedTranslation(const sMatrixf& In) const;
  sMatrixf __stdcall MatrixSetProjectedTranslation(const sMatrixf& In, const sVec3f& avT) const;
  sMatrixf __stdcall MatrixSetNotRotation(const sMatrixf& In, const sMatrixf& M) const;
  sMatrixf __stdcall MatrixRotate(const sMatrixf& M1, const sMatrixf& M2) const;
  sMatrixf __stdcall MatrixViewport(const sVec4f& aVP, tF32 afMinZ, tF32 afMaxZ);
  sMatrixf __stdcall MatrixAdjustViewport(const sVec4f& aContextVP, const sVec4f& aVirtualVP, tF32 afMinZ, tF32 afMaxZ);

  sVec3f __stdcall MatrixDecomposeGetTranslation(const sMatrixf& aMatrix);
  sVec3f __stdcall MatrixDecomposeGetZYX(const sMatrixf& aMatrix);
  sQuatf __stdcall MatrixDecomposeGetQuat(const sMatrixf& aMatrix);
  sVec4f __stdcall MatrixDecomposeGetScale(const sMatrixf& aMatrix);
  sMatrixf __stdcall MatrixCompose(const sVec3f& aT, const sVec3f& aZYX, const sVec4f& aS);
  sMatrixf __stdcall MatrixComposeQ(const sVec3f& aT, const sQuatf& aQ, const sVec4f& aS);

  sQuatf __stdcall QuatIdentity() const;
  tBool __stdcall QuatIsIdentity(const sQuatf &Q) const;
  sQuatf __stdcall QuatAdd(const sQuatf &aLeft, const sQuatf &aRight) const;
  sQuatf __stdcall QuatSub(const sQuatf &aLeft, const sQuatf &aRight) const;
  sQuatf __stdcall QuatScale(const sQuatf &aLeft, tF32 afRight) const;
  tI32 __stdcall QuatCompare(const sQuatf &aLeft, const sQuatf &aRight) const;
  sQuatf __stdcall QuatBaryCentric(const sQuatf &Q1, const sQuatf &Q2, const sQuatf &Q3, tF32 f, tF32 g) const;
  sQuatf __stdcall QuatConjugate(const sQuatf &Q) const;
  tF32 __stdcall QuatDot(const sQuatf &Q1, const sQuatf &Q2) const;
  sQuatf __stdcall QuatExp(const sQuatf &Q) const;
  sQuatf __stdcall QuatInverse(const sQuatf &Q) const;
  tF32 __stdcall QuatLength(const sQuatf &Q) const;
  tF32 __stdcall QuatLengthSq(const sQuatf &Q) const;
  sQuatf __stdcall QuatLn(const sQuatf &Q) const;
  sQuatf __stdcall QuatMultiply(const sQuatf &Q1, const sQuatf &Q2) const;
  sQuatf __stdcall QuatNormalize(const sQuatf &Q) const;
  sQuatf __stdcall QuatRotationAxis(const sVec3f &V, tF32 angle) const;
  sQuatf __stdcall QuatRotationX(tF32 angle) const;
  sQuatf __stdcall QuatRotationY(tF32 angle) const;
  sQuatf __stdcall QuatRotationZ(tF32 angle) const;
  sQuatf __stdcall QuatRotationXYZ(const sVec3f &V) const;
  sQuatf __stdcall QuatRotationMatrix(const sMatrixf &M) const;
  sQuatf __stdcall QuatRotationYawPitchRoll(const tF32 yaw, const tF32 pitch, const tF32 roll) const;
  sQuatf __stdcall QuatRotationVector(const sVec3f &vFrom, const sVec3f &vTo) const;
  sQuatf __stdcall QuatSlerp(const sQuatf &Q1, const sQuatf &Q2, tF32 t, eQuatSlerp mode) const;
  sQuatf __stdcall QuatSquad(const sQuatf &Q1, const sQuatf &Q2, const sQuatf &Q3, const sQuatf &Q4, tF32 t) const;
  sVec4f __stdcall QuatToAxisAngle(const sQuatf &Q) const;
  sVec3f __stdcall QuatToEuler(const sQuatf &Q) const;
  tF32 __stdcall WrapFloat(tF32 aX, tF32 aMin, tF32 aMax) const;
  tI32 __stdcall WrapInt(tI32 aX, tI32 aMin, tI32 aMax) const;
  tF32 __stdcall WrapRad(tF32 aX) const;
  tF32 __stdcall WrapRad2(tF32 aX) const;
  tF32 __stdcall GetAngleFromPoints2D(const sVec2f& aStartPos, const sVec2f&  aGoalPos) const;
  sVec2f __stdcall GetAngleFromPoints3D(const sVec3f& aStartPos, const sVec3f& aGoalPos) const;
  eClassify __stdcall ClassifyPoint(const sPlanef& plane, const sVec3f& point) const;
  tBool __stdcall IsPointInsideEdge(const sVec3f& A, const sVec3f& B, const sVec3f& P) const;
  sVec3f __stdcall ClosestPointOnLine(const sVec3f& A, const sVec3f& B, const sVec3f& P) const;
  sVec3f __stdcall ClosestPointOnTriangle(const sVec3f& A, const sVec3f& B, const sVec3f& C,const sVec3f& P) const;
#endif // #if niMinFeatures(15)

#if niMinFeatures(15)

  tF32 __stdcall GetAngularDifference(tF32 afA, tF32 afB) const;
  tF32 __stdcall PlaneDot(const sPlanef& P, const sVec4f& V) const;
  tF32 __stdcall PlaneDotCoord(const sPlanef& P, const sVec3f& V) const;
  tF32 __stdcall PlaneDotNormal(const sPlanef& P, const sVec3f& V) const;
  tF32 __stdcall PlaneDistance(const sPlanef& P, const sVec3f& V) const;
  sPlanef __stdcall PlaneFromPointNormal(const sVec3f& vPoint, const sVec3f& vNormal) const;
  sPlanef __stdcall PlaneFromPoints(const sVec3f& V1, const sVec3f& V2, const sVec3f& V3) const;
  sVec4f __stdcall PlaneIntersectLine(const sPlanef& P, const sVec3f& V1, const sVec3f& V2) const;
  sVec4f __stdcall PlaneIntersectRay(const sPlanef& P, const sVec3f& avRayPos, const sVec3f& avRayDir) const;
  sPlanef __stdcall PlaneNormalize(const sPlanef& P) const;
  sPlanef __stdcall PlaneTransform(const sPlanef& P, const sMatrixf& M) const;
  sPlanef __stdcall PlaneTransformInversedMatrix(const sPlanef& P, const sMatrixf& M) const;
  sVec3f __stdcall PlaneIntersection(const sPlanef& PA, const sPlanef& PB, const sPlanef& PC) const;
  sVec3f __stdcall PlaneClosest(const sPlanef& P, const sVec3f& A) const;
  ePlaneType __stdcall PlaneType(const sPlanef& Plane) const;
  ePlaneType __stdcall PlaneMaxType(const sPlanef& Plane) const;
  sMatrixf __stdcall PlaneExtractCoordinateSystem(const sPlanef& Plane, const sVec3f& avFwdDir) const;
  tBool __stdcall PlaneIntersectSphere(const sPlanef& plane, const sVec3f& avCenter, const tF32 afRadius) const;
  sVec3f __stdcall GetScreenPosition(const sVec3f& avPos, const sMatrixf& mtxVP, const sRectf& aRect) const;
  sVec3f __stdcall Vec3SphericalToCartesian(const sVec3f& avS) const;
  sVec3f __stdcall Vec3CartesianToSpherical(const sVec3f& avC) const;
  sVec3f __stdcall Vec3OrbitPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afPitch) const;
  sVec3f __stdcall Vec3OrbitYaw(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw) const;
  sVec3f __stdcall Vec3OrbitYawPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw, tF32 afPitch) const;
  tF32 __stdcall ComputePixelScaleX(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const;
  tF32 __stdcall ComputePixelScaleY(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const;
  tF32 __stdcall ComputePixelWorldSizeX(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const;
  tF32 __stdcall ComputePixelWorldSizeY(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const;
  sVec2f __stdcall ComputePixelWorldSizeVec2(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const;
  tF32 __stdcall ComputePixelWorldSize(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const;
  tF32 __stdcall Det2x2f(const tF32 a, const tF32 b, const tF32 c, const tF32 d) const;
  tF32 __stdcall Det3x3f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8) const;
  tF32 __stdcall Det4x4f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8, const tF32 a9, const tF32 a10, const tF32 a11, const tF32 a12, const tF32 a13, const tF32 a14, const tF32 a15) const;
  tF64 __stdcall Det2x2d(const tF64 a, const tF64 b, const tF64 c, const tF64 d) const;
  tF64 __stdcall Det3x3d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8) const;
  tF64 __stdcall Det4x4d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8, const tF64 a9, const tF64 a10, const tF64 a11, const tF64 a12, const tF64 a13, const tF64 a14, const tF64 a15) const;
  tF32 __stdcall FovHzToVt(tF32 afHFov, tF32 afWbyHAspect) const;
  tF32 __stdcall FovVtToHz(tF32 afVFov, tF32 afWbyHAspect) const;

  sVec4f __stdcall TriangleAreaNormal(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const;
  tBool __stdcall TriangleIsDegenerate(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const;
  tBool __stdcall TriangleIntersectTriangle(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& U0, const sVec3f& U1, const sVec3f& U2) const;
  sVec3f __stdcall TriangleIntersectRay(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const;
  sVec3f __stdcall TriangleIntersectRayCull(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const;
  tBool __stdcall TriangleIntersectAABB(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& avMin, const sVec3f& avMax) const;
  sVec3f __stdcall TriangleBaryCentric(const sVec3f& A, const sVec3f& B, const sVec3f& C, const sVec3f& P) const;

  tF64 __stdcall ClampT(tF64 aX, tF64 aMin, tF64 aMax) const;
  tF64 __stdcall RepeatT(tF64 aX, tF64 aMin, tF64 aMax) const;
  tF64 __stdcall MirrorT(tF64 aX, tF64 aMin, tF64 aMax) const;
  tF64 __stdcall CycleT(tF64 aX, tF64 aMin, tF64 aMax) const;
  tF64 __stdcall CurveStep(tF64 a, tF64 b, tF64 t) const;
  tF64 __stdcall CurveLinear(tF64 V1, tF64 V2, tF64 s) const;
  tF64 __stdcall CurveCos(tF64 a, tF64 b, tF64 t) const;
  tF64 __stdcall CurveHermite(tF64 V1, tF64 T1, tF64 V2, tF64 T2, tF64 s) const;
  tF64 __stdcall CurveCatmullRom(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s) const;
  tF64 __stdcall CurveCardinal(tF64 V1, tF64 V2, tF64 s, tF64 a) const;
  tF64 __stdcall CurveCardinal4(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s, tF64 a) const;
  tF64 __stdcall CycleLinear(tF64 aX, tF64 aMin, tF64 aMax) const;
  tF64 __stdcall CycleCos(tF64 aX, tF64 aMin, tF64 aMax) const;
  tF64 __stdcall CycleCardinal(tF64 aX, tF64 aMin, tF64 aMax, tF64 a) const;
#endif

  tF64 __stdcall ProbSum(tF64CVec* apProbs) niImpl;
  tBool __stdcall ProbNormalize(tF64CVec* apProbs) niImpl;
  tBool __stdcall ProbSampleBuildAliasMethodArrays(const tF64CVec* apProbs, tF64CVec* apAMQ, tU32CVec* apAMA) niImpl;
  tBool __stdcall ProbSampleAliasMethod(tU32CVec* apResults, const tF64CVec* apAMQ, const tU32CVec* apAMA) niImpl;
  //// iMath ////////////////////////////////////

 private:
  niEndClass(cMath);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cMath implementation.

///////////////////////////////////////////////
cMath::cMath()
{
  FPUMode::SetPrecision(eFPUPrecision_64);
}

///////////////////////////////////////////////
cMath::~cMath()
{
}

#if niMinFeatures(15)
///////////////////////////////////////////////
tF64 __stdcall cMath::Sqrt(tF64 v) const
{
  return ni::Sqrt(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Sin(tF64 v) const
{
  return ni::Sin(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Cos(tF64 v) const
{
  return ni::Cos(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ASin(tF64 v) const
{
  return ni::ASin(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ACos(tF64 v) const
{
  return ni::ACos(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Tan(tF64 v) const
{
  return ni::Tan(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ATan(tF64 v) const
{
  return ni::ATan(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ATan2(tF64 x, tF64 y) const
{
  return ni::ATan2(x,y);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::LogX(tF64 v, tF64 base) const
{
  return ni::LogX(v,base);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Log2(tF64 v) const
{
  return ni::Log2(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::LogE(tF64 v) const
{
  return ni::LogE(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Log10(tF64 v) const
{
  return ni::Log10(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Pow(tF64 v, tF64 e) const
{
  return ni::Pow(v,e);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Floor(tF64 v) const
{
  return ni::Floor(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Ceil(tF64 v) const
{
  return ni::Ceil(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Exp(tF64 v) const
{
  return ni::Exp(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Abs(tF64 v) const
{
  return ni::Abs(v);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ToRad(tF64 afDeg) const
{
  return niRadf(afDeg);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ToDeg(tF64 afRad) const
{
  return niDegf(afRad);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Min(tF64 a, tF64 b) const
{
  return ni::Min(a,b);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Min3(tF64 a, tF64 b, tF64 c) const
{
  return ni::Min(a,b,c);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Max(tF64 a, tF64 b) const
{
  return ni::Max(a,b);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Max3(tF64 a, tF64 b, tF64 c) const
{
  return ni::Max(a,b,c);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Clamp(tF64 v, tF64 afMin, tF64 afMax) const
{
  return ni::Clamp(v,afMin,afMax);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ClampZeroOne(tF64 a) const
{
  return ni::ClampZeroOne(a);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Lerp(tF64 a, tF64 b, tF64 f) const
{
  return ni::Lerp(a,b,f);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::BlendIntoAccumulator(tF64 accumulator, tF64 newValue, tF64 smoothRate) const
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
void __stdcall cMath::RandSeed(tU32 ulSeed) const
{
  ni::RandSeed(ulSeed);
}

///////////////////////////////////////////////
tI32 __stdcall cMath::RandInt() const
{
  return ni::RandInt();
}

///////////////////////////////////////////////
tI32 __stdcall cMath::RandIntRange(tI32 aMin, tI32 aMax) const
{
  return ni::RandIntRange(aMin,aMax);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::RandFloat() const
{
  return ni::RandFloat();
}

///////////////////////////////////////////////
tF32 __stdcall cMath::RandFloatRange(tF32 afMin, tF32 afMax) const
{
  return (tF32)ni::RandFloatRange((tF32)afMin,(tF32)afMax);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::RandNormal(tF32 sigma) const
{
  return ni::RandN(sigma);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::RandomDirection(const sVec3f& vN) const
{
  sVec3f ret;
  return ni::RandomDirection(ret,vN);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::RandomDirectionEx(const sVec3f& vN, tF32 fDeviAngle) const
{
  sVec3f ret;
  return ni::RandomDirectionEx(ret,vN,fDeviAngle);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::RotateRay(const sVec3f& avIn, const sVec3f& avN) const
{
  sVec3f vOut = avIn;
  return ni::RotateRay(vOut,avN);
}

///////////////////////////////////////////////
tU32 __stdcall cMath::RandColorA(tU8 aA) const {
  return ni::RandColorA(aA);
}

///////////////////////////////////////////////
tU32 __stdcall cMath::RandColorAf(tF32 aA) const {
  return ni::RandColorAf(aA);
}

///////////////////////////////////////////////
tU32 __stdcall cMath::RandColor() const {
  return ni::RandColor();
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Add(const sVec2f &aLeft, const sVec2f &aRight) const
{
  sVec2f ret;
  return ni::VecAdd(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Sub(const sVec2f &aLeft, const sVec2f &aRight) const
{
  sVec2f ret;
  return ni::VecSub(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Mul(const sVec2f &aLeft, const sVec2f &aRight) const
{
  sVec2f ret;
  return ni::VecMul(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Div(const sVec2f &aLeft, const sVec2f &aRight) const
{
  sVec2f ret;
  return ni::VecDiv(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Scale(const sVec2f &aLeft, tF32 afRight) const
{
  sVec2f ret;
  return ni::VecScale(ret,aLeft,afRight);
}

///////////////////////////////////////////////
tI32 cMath::Vec2Compare(const sVec2f &aLeft, const sVec2f &aRight) const
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sVec2f));
}

///////////////////////////////////////////////
tF32 cMath::Vec2Length(const sVec2f &aV) const
{
  return ni::VecLength(aV);
}

///////////////////////////////////////////////
tF32 cMath::Vec2LengthSq(const sVec2f &aV) const
{
  return ni::VecLengthSq(aV);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Normalize(const sVec2f &aV) const
{
  sVec2f ret;
  return ni::VecNormalize(ret,aV);
}

///////////////////////////////////////////////
tBool cMath::Vec2IsNormal(const sVec2f &aV) const
{
  return ni::VecIsNormal(aV);
}

///////////////////////////////////////////////
tF32 cMath::Vec2Dot(const sVec2f &aLeft, const sVec2f &aRight) const
{
  return ni::VecDot(aLeft,aRight);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Lerp(const sVec2f &aLeft, const sVec2f &aRight, tF32 afF) const
{
  sVec2f ret;
  return ni::VecLerp(ret,aLeft,aRight,afF);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Lerp2(const sVec2f &aLeft, const sVec2f &aRight, const sVec2f& aF) const
{
  sVec2f ret;
  return ni::VecLerp(ret,aLeft,aRight,aF);
}

///////////////////////////////////////////////
sVec2f __stdcall cMath::Vec2BlendIntoAccumulator(const sVec2f &accumulator, const sVec2f &newValue, tF32 smoothRate) const
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Abs(const sVec2f &aV) const
{
  sVec2f ret;
  return ni::VecAbs(ret,aV);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Min(const sVec2f &aLeft, const sVec2f &aRight) const
{
  sVec2f ret;
  return ni::VecMinimize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Max(const sVec2f &aLeft, const sVec2f &aRight) const
{
  sVec2f ret;
  return ni::VecMaximize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2CatmullRom(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, const sVec2f &V4, tF32 s) const
{
  sVec2f ret;
  return ni::VecCatmullRom(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2Hermite(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, const sVec2f &V4, tF32 s) const
{
  sVec2f ret;
  return ni::VecHermite(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2BaryCentric(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, tF32 u, tF32 v) const
{
  sVec2f ret;
  return ni::VecBaryCentric(ret,V1,V2,V3,u,v);
}

///////////////////////////////////////////////
sVec4f cMath::Vec2Transform(const sVec2f &aVec, const sMatrixf &aMatrix) const
{
  sVec4f ret;
  return ni::VecTransform(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2TransformCoord(const sVec2f &aVec, const sMatrixf &aMatrix) const
{
  sVec2f ret;
  return ni::VecTransformCoord(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
sVec2f cMath::Vec2TransformNormal(const sVec2f &aVec, const sMatrixf &aMatrix) const
{
  sVec2f ret;
  return ni::VecTransformNormal(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
void cMath::Vec2TransformCoordArray(tVec2fCVec *apVecs, const sMatrixf &aMatrix) const
{
  IMPL_TRANSFORM(Vec2f,apVecs,VecTransformCoord);
}

///////////////////////////////////////////////
void cMath::Vec2TransformNormalArray(tVec2fCVec *apVecs, const sMatrixf &aMatrix) const
{
  IMPL_TRANSFORM(Vec2f,apVecs,VecTransformNormal);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Add(const sVec3f &aLeft, const sVec3f &aRight) const
{
  sVec3f ret;
  return ni::VecAdd(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Sub(const sVec3f &aLeft, const sVec3f &aRight) const
{
  sVec3f ret;
  return ni::VecSub(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Mul(const sVec3f &aLeft, const sVec3f &aRight) const
{
  sVec3f ret;
  return ni::VecMul(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Div(const sVec3f &aLeft, const sVec3f &aRight) const
{
  sVec3f ret;
  return ni::VecDiv(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Scale(const sVec3f &aLeft, tF32 afRight) const
{
  sVec3f ret;
  return ni::VecScale(ret,aLeft,afRight);
}

///////////////////////////////////////////////
tI32 cMath::Vec3Compare(const sVec3f &aLeft, const sVec3f &aRight) const
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sVec3f));
}

///////////////////////////////////////////////
tF32 cMath::Vec3Length(const sVec3f &aV) const
{
  return ni::VecLength(aV);
}

///////////////////////////////////////////////
tF32 cMath::Vec3LengthSq(const sVec3f &aV) const
{
  return ni::VecLengthSq(aV);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Normalize(const sVec3f &aV) const
{
  sVec3f ret;
  return ni::VecNormalize(ret,aV);
}

///////////////////////////////////////////////
tBool cMath::Vec3IsNormal(const sVec3f &aV) const
{
  return ni::VecIsNormal(aV);
}

///////////////////////////////////////////////
tF32 cMath::Vec3Dot(const sVec3f &aLeft, const sVec3f &aRight) const
{
  return ni::VecDot(aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Cross(const sVec3f &aLeft, const sVec3f &aRight) const
{
  sVec3f ret;
  return ni::VecCross(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Lerp(const sVec3f &aLeft, const sVec3f &aRight, tF32 afF) const
{
  sVec3f ret;
  return ni::VecLerp(ret,aLeft,aRight,afF);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Lerp2(const sVec3f &aLeft, const sVec3f &aRight, const sVec3f &aF) const
{
  sVec3f ret;
  return ni::VecLerp(ret,aLeft,aRight,aF);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::Vec3BlendIntoAccumulator(const sVec3f &accumulator, const sVec3f &newValue, tF32 smoothRate) const
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Abs(const sVec3f &aV) const
{
  sVec3f ret;
  return ni::VecAbs(ret,aV);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Min(const sVec3f &aLeft, const sVec3f &aRight) const
{
  sVec3f ret;
  return ni::VecMinimize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Max(const sVec3f &aLeft, const sVec3f &aRight) const
{
  sVec3f ret;
  return ni::VecMaximize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3CatmullRom(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, const sVec3f &V4, tF32 s) const
{
  sVec3f ret;
  return ni::VecCatmullRom(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Hermite(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, const sVec3f &V4, tF32 s) const
{
  sVec3f ret;
  return ni::VecHermite(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3BaryCentric(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, tF32 u, tF32 v) const
{
  sVec3f ret;
  return ni::VecBaryCentric(ret,V1,V2,V3,u,v);
}

///////////////////////////////////////////////
sVec4f cMath::Vec3Transform(const sVec3f &aVec, const sMatrixf &aMatrix) const
{
  sVec4f ret;
  return ni::VecTransform(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3TransformCoord(const sVec3f &aVec, const sMatrixf &aMatrix) const
{
  sVec3f ret;
  return ni::VecTransformCoord(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3TransformNormal(const sVec3f &aVec, const sMatrixf &aMatrix) const
{
  sVec3f ret;
  return ni::VecTransformNormal(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
void cMath::Vec3TransformCoordArray(tVec3fCVec *apVecs, const sMatrixf &aMatrix) const
{
  IMPL_TRANSFORM(Vec3f,apVecs,VecTransformCoord);
}

///////////////////////////////////////////////
void cMath::Vec3TransformNormalArray(tVec3fCVec *apVecs, const sMatrixf &aMatrix) const
{
  IMPL_TRANSFORM(Vec3f,apVecs,VecTransformNormal);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Unproject(const sVec3f &avIn, const sRectf &aVP, const sMatrixf &amtxViewProj) const
{
  sVec3f ret;
  return ni::VecUnproject(ret,avIn,aVP.GetLeft(),aVP.GetTop(),aVP.GetWidth(),aVP.GetHeight(),amtxViewProj);
}

///////////////////////////////////////////////
sVec3f cMath::Vec3Project(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) const
{
  sVec3f v;
  return ni::VecProject(v,avPos,amtxWVP,aRect);
}

///////////////////////////////////////////////
sVec4f cMath::Vec3ProjectRHW(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) const
{
  sVec4f v;
  return ni::VecProjectRHW(v,avPos,amtxWVP,aRect);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::Vec3Reflect(const sVec3f& Dir, const sVec3f& Normal) const {
  sVec3f v;
  return ni::VecReflect(v,Dir,Normal);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Add(const sVec4f &aLeft, const sVec4f &aRight) const
{
  sVec4f ret;
  return ni::VecAdd(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Sub(const sVec4f &aLeft, const sVec4f &aRight) const
{
  sVec4f ret;
  return ni::VecSub(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Mul(const sVec4f &aLeft, const sVec4f &aRight) const
{
  sVec4f ret;
  return ni::VecMul(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Div(const sVec4f &aLeft, const sVec4f &aRight) const
{
  sVec4f ret;
  return ni::VecDiv(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Scale(const sVec4f &aLeft, tF32 afRight) const
{
  sVec4f ret;
  return ni::VecScale(ret,aLeft,afRight);
}

///////////////////////////////////////////////
tI32 cMath::Vec4Compare(const sVec4f &aLeft, const sVec4f &aRight) const
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sVec4f));
}

///////////////////////////////////////////////
tF32 cMath::Vec4Length(const sVec4f &aV) const
{
  return ni::VecLength(aV);
}

///////////////////////////////////////////////
tF32 cMath::Vec4LengthSq(const sVec4f &aV) const
{
  return ni::VecLengthSq(aV);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Normalize(const sVec4f &aV) const
{
  sVec4f ret;
  return ni::VecNormalize(ret,aV);
}

///////////////////////////////////////////////
tBool cMath::Vec4IsNormal(const sVec4f &aV) const
{
  return ni::VecIsNormal(aV);
}

///////////////////////////////////////////////
tF32 cMath::Vec4Dot(const sVec4f &aLeft, const sVec4f &aRight) const
{
  return ni::VecDot(aLeft,aRight);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Cross(const sVec4f& aA, const sVec4f& aB, const sVec4f& aC) const
{
  sVec4f ret;
  return ni::VecCross(ret,aA,aB,aC);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Lerp(const sVec4f &aLeft, const sVec4f &aRight, tF32 afF) const
{
  sVec4f ret;
  return ni::VecLerp(ret,aLeft,aRight,afF);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Lerp2(const sVec4f &aLeft, const sVec4f &aRight, const sVec4f &aF) const
{
  sVec4f ret;
  return ni::VecLerp(ret,aLeft,aRight,aF);
}

///////////////////////////////////////////////
sVec4f __stdcall cMath::Vec4BlendIntoAccumulator(const sVec4f &accumulator, const sVec4f &newValue, tF32 smoothRate) const
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Abs(const sVec4f &aV) const
{
  sVec4f ret;
  return ni::VecAbs(ret,aV);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Min(const sVec4f &aLeft, const sVec4f &aRight) const
{
  sVec4f ret;
  return ni::VecMinimize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Max(const sVec4f &aLeft, const sVec4f &aRight) const
{
  sVec4f ret;
  return ni::VecMaximize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4CatmullRom(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, const sVec4f &V4, tF32 s) const
{
  sVec4f ret;
  return ni::VecCatmullRom(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Hermite(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, const sVec4f &V4, tF32 s) const
{
  sVec4f ret;
  return ni::VecHermite(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4BaryCentric(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, tF32 u, tF32 v) const
{
  sVec4f ret;
  return ni::VecBaryCentric(ret,V1,V2,V3,u,v);
}

///////////////////////////////////////////////
sVec4f cMath::Vec4Transform(const sVec4f &aVec, const sMatrixf &aMatrix) const
{
  sVec4f ret;
  return ni::VecTransform(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
void cMath::Vec4TransformArray(tVec4fCVec *apVecs, const sMatrixf &aMatrix) const
{
  IMPL_TRANSFORM(Vec4f,apVecs,VecTransform);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixIdentity() const
{
  return (sMatrixf::Identity());
}

///////////////////////////////////////////////
tBool cMath::MatrixIsIdentity(const sMatrixf &M) const
{
  return ni::MatrixIsIdentity(M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixAdd(const sMatrixf &aLeft, const sMatrixf &aRight) const
{
  return aLeft+aRight;
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixSub(const sMatrixf &aLeft, const sMatrixf &aRight) const
{
  return aLeft-aRight;
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixScale(const sMatrixf &aLeft, tF32 afRight) const
{
  return aLeft*afRight;
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixLerp(const sMatrixf &A, const sMatrixf &B, tF32 fFac) const
{
  sMatrixf ret;
  return ni::MatrixLerp(ret,A,B,fFac);
}

///////////////////////////////////////////////
tI32 cMath::MatrixCompare(const sMatrixf &aLeft, const sMatrixf &aRight) const
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sMatrixf));
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixMultiply(const sMatrixf &M1, const sMatrixf &M2) const
{
  sMatrixf ret;
  return ni::MatrixMultiply(ret,M1,M2);
}

///////////////////////////////////////////////
tF32 cMath::MatrixDeterminant2(const sMatrixf &M) const
{
  return ni::MatrixDeterminant2(M);
}

///////////////////////////////////////////////
tF32 cMath::MatrixDeterminant3(const sMatrixf &M) const
{
  return ni::MatrixDeterminant3(M);
}

///////////////////////////////////////////////
tF32 cMath::MatrixDeterminant(const sMatrixf &M) const
{
  return ni::MatrixDeterminant(M);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::MatrixGetHandeness(const sMatrixf& M) const
{
  return ni::MatrixGetHandeness(M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixInverse(const sMatrixf &M) const
{
  sMatrixf ret;
  return ni::MatrixInverse(ret,M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixTransformInverse(const sMatrixf &M) const
{
  sMatrixf ret;
  return ni::MatrixTransformInverse(ret,M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixLookAtRH(const sVec3f &Eye, const sVec3f &At, const sVec3f &Up) const
{
  sMatrixf ret;
  return ni::MatrixLookAtRH(ret,Eye,At,Up);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixLookAtLH(const sVec3f &Eye, const sVec3f &At, const sVec3f &Up) const
{
  sMatrixf ret;
  return ni::MatrixLookAtLH(ret,Eye,At,Up);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixOrthoRH(tF32 w, tF32 h, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixOrthoRH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixOrthoLH(tF32 w, tF32 h, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixOrthoLH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixOrthoOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixOrthoOffCenterRH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixOrthoOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixOrthoOffCenterLH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixPerspectiveRH(tF32 w, tF32 h, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixPerspectiveRH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixPerspectiveLH(tF32 w, tF32 h, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixPerspectiveLH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixPerspectiveFovRH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixPerspectiveFovRH(ret,fovy,aspect,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixPerspectiveFovLH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixPerspectiveFovLH(ret,fovy,aspect,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixPerspectiveOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixPerspectiveOffCenterRH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixPerspectiveOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) const
{
  sMatrixf ret;
  return ni::MatrixPerspectiveOffCenterLH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixReflect(const sPlanef &Plane) const
{
  sMatrixf ret;
  return ni::MatrixReflect(ret,Plane);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixRotationAxis(const sVec3f &V, tF32 angle) const
{
  sMatrixf ret;
  return ni::MatrixRotationAxis(ret,V,angle);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixRotationQuat(const sQuatf &Q) const
{
  sMatrixf ret;
  return ni::MatrixRotationQuat(ret,Q);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixRotationX(tF32 angle) const
{
  sMatrixf ret;
  return ni::MatrixRotationX(ret,angle);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixRotationY(tF32 angle) const
{
  sMatrixf ret;
  return ni::MatrixRotationY(ret,angle);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixRotationZ(tF32 angle) const
{
  sMatrixf ret;
  return ni::MatrixRotationZ(ret,angle);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixRotationYawPitchRoll(tF32 yaw, tF32 pitch, tF32 roll) const
{
  sMatrixf ret;
  return ni::MatrixRotationYawPitchRoll(ret,yaw,pitch,roll);
}

///////////////////////////////////////////////
sVec3f cMath::MatrixDecomposeYawPitchRoll(const sMatrixf& M) const
{
  return ni::MatrixDecomposeYawPitchRoll(M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixScaling(const sVec3f &V) const
{
  sMatrixf ret;
  return ni::MatrixScaling(ret,V);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixVecScale(const sMatrixf &M, const sVec3f &V) const
{
  sMatrixf ret;
  return ni::MatrixScale(ret,M,V);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixShadow(const sVec4f &vLight, const sPlanef &Plane) const
{
  sMatrixf ret;
  return ni::MatrixShadow(ret,vLight,Plane);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixTranslation(const sVec3f &V) const
{
  sMatrixf ret;
  return ni::MatrixTranslation(ret,V);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixTranspose(const sMatrixf &M) const
{
  sMatrixf ret;
  return ni::MatrixTranspose(ret,M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixTranspose3x3(const sMatrixf &M) const
{
  sMatrixf ret;
  return ni::MatrixTranspose3x3(ret,M);
}

///////////////////////////////////////////////
sMatrixf __stdcall cMath::MatrixTextureOffset(tF32 fBias, tI32 nTexW, tI32 nTexH)
{
  sMatrixf ret;
  return ni::MatrixTextureOffset(ret,fBias,nTexW,nTexH);
}

///////////////////////////////////////////////
sMatrixf __stdcall cMath::MatrixTextureOffset2(tF32 fBias, tI32 nTexW, tI32 nTexH, tI32 nDepthBits)
{
  sMatrixf ret;
  return ni::MatrixTextureOffset2(ret,fBias,nTexW,nTexH,nDepthBits);
}

///////////////////////////////////////////////
sVec3f cMath::MatrixGetForward(const sMatrixf &M) const
{
  sVec3f ret;
  return ni::MatrixGetForward(ret,M);
}

///////////////////////////////////////////////
sVec3f cMath::MatrixGetUp(const sMatrixf &M) const
{
  sVec3f ret;
  return ni::MatrixGetUp(ret,M);
}

///////////////////////////////////////////////
sVec3f cMath::MatrixGetRight(const sMatrixf &M) const
{
  sVec3f ret;
  return ni::MatrixGetRight(ret,M);
}

///////////////////////////////////////////////
sVec3f cMath::MatrixGetTranslation(const sMatrixf &M) const
{
  sVec3f ret;
  return ni::MatrixGetTranslation(ret,M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixSetForward(const sMatrixf &M, const sVec3f &V) const
{
  sMatrixf v = M;
  return ni::MatrixSetForward(v,V);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixSetUp(const sMatrixf &M, const sVec3f &V) const
{
  sMatrixf v = M;
  return ni::MatrixSetUp(v,V);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixSetRight(const sMatrixf &M, const sVec3f &V) const
{
  sMatrixf v = M;
  return ni::MatrixSetRight(v,V);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixSetTranslation(const sMatrixf &M, const sVec3f &V) const
{
  sMatrixf v = M;
  return ni::MatrixSetTranslation(v,V);
}

///////////////////////////////////////////////
tBool cMath::MatrixIsNormal(const sMatrixf &M) const
{
  return ni::MatrixIsNormal(M);
}

///////////////////////////////////////////////
tBool cMath::MatrixIsOrthogonal(const sMatrixf &M) const
{
  return ni::MatrixIsOrthogonal(M);
}

///////////////////////////////////////////////
tBool cMath::MatrixIsOrthoNormal(const sMatrixf &M) const
{
  return ni::MatrixIsOrthoNormal(M);
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixSetRotation(const sMatrixf &In, const sMatrixf &RotM) const
{
  sMatrixf v = In;
  return ni::MatrixSetRotation(v,RotM);
}

///////////////////////////////////////////////
sVec3f cMath::MatrixToEuler(const sMatrixf &M) const
{
  return QuatToEuler(QuatNormalize(QuatRotationMatrix(M)));
  //return ni::MatrixToEuler(sMatrixf(),sVec3f());
}

///////////////////////////////////////////////
sMatrixf cMath::MatrixToCoordinateSystem(const sVec3f &avRight, const sVec3f &avUp, const sVec3f &avFwd, const sVec3f &avOrg) const
{
  sMatrixf ret;
  return ni::MatrixToCoordinateSystem(ret,avRight,avUp,avFwd,avOrg);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::MatrixGetProjectedTranslation(const sMatrixf& In) const
{
  sVec3f o;
  return ni::MatrixGetProjectedTranslation(o,In);
}

///////////////////////////////////////////////
sMatrixf __stdcall cMath::MatrixSetProjectedTranslation(const sMatrixf& In, const sVec3f& avT) const
{
  sMatrixf v = In;
  return ni::MatrixSetProjectedTranslation(v,avT);
}

///////////////////////////////////////////////
sMatrixf __stdcall cMath::MatrixSetNotRotation(const sMatrixf& In, const sMatrixf& M) const
{
  sMatrixf v = In;
  return ni::MatrixSetNotRotation(v,M);
}

///////////////////////////////////////////////
sMatrixf __stdcall cMath::MatrixRotate(const sMatrixf& M1, const sMatrixf& M2) const
{
  sMatrixf ret;
  return ni::MatrixRotate(ret,M1,M2);
}

///////////////////////////////////////////////
sMatrixf __stdcall cMath::MatrixViewport(const sVec4f& aVP, tF32 afMinZ, tF32 afMaxZ) {
  sMatrixf ret;
  return ni::MatrixViewport(ret,aVP,afMinZ,afMaxZ);
}

sMatrixf __stdcall cMath::MatrixAdjustViewport(const sVec4f& aContextVP, const sVec4f& aVirtualVP, tF32 afMinZ, tF32 afMaxZ) {
  sMatrixf ret;
  return ni::MatrixAdjustViewport(ret,aContextVP,aVirtualVP,afMinZ,afMaxZ);
}

///////////////////////////////////////////////
sQuatf cMath::QuatIdentity() const
{
  return (sQuatf::Identity());
}

///////////////////////////////////////////////
tBool cMath::QuatIsIdentity(const sQuatf &Q) const
{
  return ni::QuatIsIdentity(Q);
}

///////////////////////////////////////////////
sQuatf cMath::QuatAdd(const sQuatf &aLeft, const sQuatf &aRight) const
{
  return aLeft+aRight;
}

///////////////////////////////////////////////
sQuatf cMath::QuatSub(const sQuatf &aLeft, const sQuatf &aRight) const
{
  return aLeft+aRight;
}

///////////////////////////////////////////////
sQuatf cMath::QuatScale(const sQuatf &aLeft, tF32 afRight) const
{
  return aLeft*afRight;
}

///////////////////////////////////////////////
tI32 cMath::QuatCompare(const sQuatf &aLeft, const sQuatf &aRight) const
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sQuatf));
}

///////////////////////////////////////////////
sQuatf cMath::QuatBaryCentric(const sQuatf &Q1, const sQuatf &Q2, const sQuatf &Q3, tF32 f, tF32 g) const
{
  sQuatf ret;
  return ni::QuatBaryCentric(ret,Q1,Q2,Q3,f,g);
}

///////////////////////////////////////////////
sQuatf cMath::QuatConjugate(const sQuatf &Q) const
{
  sQuatf ret;
  return ni::QuatConjugate(ret,Q);
}

///////////////////////////////////////////////
tF32 cMath::QuatDot(const sQuatf &Q1, const sQuatf &Q2) const
{
  return ni::QuatDot(Q1,Q2);
}

///////////////////////////////////////////////
sQuatf cMath::QuatExp(const sQuatf &Q) const
{
  sQuatf ret;
  return ni::QuatExp(ret,Q);
}

///////////////////////////////////////////////
sQuatf cMath::QuatInverse(const sQuatf &Q) const
{
  sQuatf ret;
  return ni::QuatInverse(ret,Q);
}

///////////////////////////////////////////////
tF32 cMath::QuatLength(const sQuatf &Q) const
{
  return ni::QuatLength(Q);
}

///////////////////////////////////////////////
tF32 cMath::QuatLengthSq(const sQuatf &Q) const
{
  return ni::QuatLengthSq(Q);
}

///////////////////////////////////////////////
sQuatf cMath::QuatLn(const sQuatf &Q) const
{
  sQuatf ret;
  return ni::QuatLn(ret,Q);
}

///////////////////////////////////////////////
sQuatf cMath::QuatMultiply(const sQuatf &Q1, const sQuatf &Q2) const
{
  sQuatf ret;
  return ni::QuatMultiply(ret,Q1,Q2);
}

///////////////////////////////////////////////
sQuatf cMath::QuatNormalize(const sQuatf &Q) const
{
  sQuatf ret;
  return ni::QuatNormalize(ret,Q);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationAxis(const sVec3f &V, tF32 angle) const
{
  sQuatf ret;
  return ni::QuatRotationAxis(ret,V,angle);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationX(tF32 angle) const
{
  sQuatf ret;
  return ni::QuatRotationX(ret,angle);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationY(tF32 angle) const
{
  sQuatf ret;
  return ni::QuatRotationY(ret,angle);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationZ(tF32 angle) const
{
  sQuatf ret;
  return ni::QuatRotationZ(ret,angle);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationXYZ(const sVec3f &V) const
{
  sQuatf ret;
  return ni::QuatRotationXYZ(ret,V);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationMatrix(const sMatrixf &M) const
{
  sQuatf ret;
  return ni::QuatRotationMatrix(ret,M);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationYawPitchRoll(const tF32 yaw, const tF32 pitch, const tF32 roll) const
{
  sQuatf ret;
  return ni::QuatRotationYawPitchRoll(ret,yaw,pitch,roll);
}

///////////////////////////////////////////////
sQuatf cMath::QuatRotationVector(const sVec3f &vFrom, const sVec3f &vTo) const
{
  sQuatf ret;
  return ni::QuatRotationVector(ret,vFrom,vTo);
}

///////////////////////////////////////////////
sQuatf cMath::QuatSlerp(const sQuatf &Q1, const sQuatf &Q2, tF32 t, eQuatSlerp mode) const
{
  sQuatf ret;
  return ni::QuatSlerp(ret,Q1,Q2,t,mode);
}

///////////////////////////////////////////////
sQuatf cMath::QuatSquad(const sQuatf &Q1, const sQuatf &Q2, const sQuatf &Q3, const sQuatf &Q4, tF32 t) const
{
  sQuatf ret;
  return ni::QuatSquad(ret,Q1,Q2,Q3,Q4,t);
}

///////////////////////////////////////////////
sVec4f cMath::QuatToAxisAngle(const sQuatf &Q) const
{
  sVec4f v;
  ni::QuatToAxisAngle(Q,(sVec3f&)v,v.w);
  return v;
}

///////////////////////////////////////////////
sVec3f cMath::QuatToEuler(const sQuatf &Q) const
{
  sVec3f ret;
  return ni::QuatToEuler(Q,ret);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::WrapFloat(tF32 aX, tF32 aMin, tF32 aMax) const
{
  return ni::WrapFloat(aX,aMin,aMax);
}

tI32 __stdcall cMath::WrapInt(tI32 aX, tI32 aMin, tI32 aMax) const
{
  return ni::WrapInt(aX,aMin,aMax);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::WrapRad(tF32 aX) const
{
  return ni::WrapRad(aX);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::WrapRad2(tF32 aX) const
{
  return ni::WrapRad2(aX);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::GetAngleFromPoints2D(const sVec2f& aStartPos, const sVec2f&  aGoalPos) const
{
  return ni::GetAngleFromPoints2D(aStartPos,aGoalPos);
}

///////////////////////////////////////////////
sVec2f __stdcall cMath::GetAngleFromPoints3D(const sVec3f& aStartPos, const sVec3f& aGoalPos) const
{
  return ni::GetAngleFromPoints3D(aStartPos,aGoalPos);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::GetAngularDifference(tF32 afA, tF32 afB) const
{
  return ni::GetAngularDifference(afA,afB);
}

///////////////////////////////////////////////
eClassify __stdcall cMath::ClassifyPoint(const sPlanef& plane, const sVec3f& point) const
{
  return ni::ClassifyPoint(plane,point);
}

///////////////////////////////////////////////
tBool __stdcall cMath::IsPointInsideEdge(const sVec3f& A, const sVec3f& B, const sVec3f& P) const
{
  return ni::IsPointInsideEdge(A,B,P);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::ClosestPointOnLine(const sVec3f& A, const sVec3f& B, const sVec3f& P) const
{
  sVec3f ret;
  return ni::ClosestPointOnLine(ret,A,B,P);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::ClosestPointOnTriangle(const sVec3f& A, const sVec3f& B, const sVec3f& C,const sVec3f& P) const
{
  sVec3f ret;
  return ni::ClosestPointOnTriangle(ret,A,B,C,P);
}

//////////////////////////////////////////////////////////////////////////////////////////////
#endif // #if niMinFeatures(15)

#if niMinFeatures(15)

///////////////////////////////////////////////
tF32 __stdcall cMath::PlaneDot(const sPlanef& P, const sVec4f& V) const
{
  return ni::PlaneDot(P,V);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::PlaneDotCoord(const sPlanef& P, const sVec3f& V) const
{
  return ni::PlaneDotCoord(P,V);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::PlaneDotNormal(const sPlanef& P, const sVec3f& V) const
{
  return ni::PlaneDotNormal(P,V);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::PlaneDistance(const sPlanef& P, const sVec3f& V) const
{
  return ni::PlaneDistance(P,V);
}

///////////////////////////////////////////////
sPlanef __stdcall cMath::PlaneFromPointNormal(const sVec3f& vPoint, const sVec3f& vNormal) const
{
  sPlanef ret;
  return ni::PlaneFromPointNormal(ret,vPoint,vNormal);
}

///////////////////////////////////////////////
sPlanef __stdcall cMath::PlaneFromPoints(const sVec3f& V1, const sVec3f& V2, const sVec3f& V3) const
{
  sPlanef ret;
  return ni::PlaneFromPoints(ret,V1,V2,V3);
}

///////////////////////////////////////////////
sVec4f __stdcall cMath::PlaneIntersectLine(const sPlanef& P, const sVec3f& V1, const sVec3f& V2) const
{
  sVec4f v;
  if (ni::PlaneIntersectLine(P,V1,V2,(sVec3f*)&v,&v.w) == NULL)
    return (sVec4f::Zero());
  return v;
}

///////////////////////////////////////////////
sVec4f __stdcall cMath::PlaneIntersectRay(const sPlanef& P, const sVec3f& avRayPos, const sVec3f& avRayDir) const
{
  sVec4f v;
  if (ni::PlaneIntersectRay(P,avRayPos,avRayDir,(sVec3f*)&v,&v.w) == NULL)
    return (sVec4f::Zero());
  return v;
}

///////////////////////////////////////////////
sPlanef __stdcall cMath::PlaneNormalize(const sPlanef& P) const
{
  sPlanef p;
  if (ni::PlaneNormalize(p,P) == NULL)
    return sPlanef::Zero();
  return p;
}

///////////////////////////////////////////////
sPlanef __stdcall cMath::PlaneTransform(const sPlanef& P, const sMatrixf& M) const
{
  sPlanef ret;
  return ni::PlaneTransform(ret,P,M);
}

///////////////////////////////////////////////
sPlanef __stdcall cMath::PlaneTransformInversedMatrix(const sPlanef& P, const sMatrixf& M) const
{
  sPlanef ret;
  return ni::PlaneTransformInversedMatrix(ret,P,M);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::PlaneIntersection(const sPlanef& PA, const sPlanef& PB, const sPlanef& PC) const
{
  sVec3f v;
  if (ni::PlaneIntersection(v,PA,PB,PC) == NULL)
    return (sVec3f::Zero());
  return v;
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::PlaneClosest(const sPlanef& P, const sVec3f& A) const
{
  sVec3f ret;
  return ni::PlaneClosest(ret,P,A);
}

///////////////////////////////////////////////
ePlaneType __stdcall cMath::PlaneType(const sPlanef& Plane) const
{
  return ni::PlaneType(Plane);
}

///////////////////////////////////////////////
ePlaneType __stdcall cMath::PlaneMaxType(const sPlanef& Plane) const
{
  return ni::PlaneMaxType(Plane);
}

///////////////////////////////////////////////
sMatrixf __stdcall cMath::PlaneExtractCoordinateSystem(const sPlanef& Plane, const sVec3f& avFwdDir) const
{
  sMatrixf ret;
  return ni::PlaneExtractCoordinateSystem<tF32>(ret,Plane,
                                                eFalse,
                                                (avFwdDir==sVec3f::Zero())?NULL:&avFwdDir,
                                                NULL);
}

///////////////////////////////////////////////
tBool __stdcall cMath::PlaneIntersectSphere(const sPlanef& plane, const sVec3f& avCenter, const tF32 afRadius) const
{
  return ni::PlaneIntersectSphere(plane,avCenter,afRadius);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::GetScreenPosition(const sVec3f& avPos, const sMatrixf& mtxVP, const sRectf& aRect) const {
  return ni::GetScreenPosition(avPos,mtxVP,aRect);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::Vec3SphericalToCartesian(const sVec3f& avS) const
{
  sVec3f ret;
  return ni::VecSphericalToCartesian(ret,avS);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::Vec3CartesianToSpherical(const sVec3f& avC) const
{
  sVec3f ret;
  return ni::VecCartesianToSpherical(ret,avC);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::Vec3OrbitPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afPitch) const
{
  sVec3f ret;
  return ni::VecOrbitPitch(ret,aTarget,aPos,afPitch);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::Vec3OrbitYaw(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw) const
{
  sVec3f ret;
  return ni::VecOrbitYaw(ret,aTarget,aPos,afYaw);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::Vec3OrbitYawPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw, tF32 afPitch) const
{
  sVec3f ret;
  return ni::VecOrbitYawPitch(ret,aTarget,aPos,afYaw,afPitch);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::ComputePixelScaleX(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const
{
  return ni::ComputePixelScaleX(aPos,amtxWorldView,amtxProj,aVP);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::ComputePixelScaleY(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const
{
  return ni::ComputePixelScaleY(aPos,amtxWorldView,amtxProj,aVP);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::ComputePixelWorldSizeX(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const {
  return ni::ComputePixelWorldSizeX(afPixelSize,avPos,mtxWorldView,mtxProj,aVP);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::ComputePixelWorldSizeY(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const {
  return ni::ComputePixelWorldSizeY(afPixelSize,avPos,mtxWorldView,mtxProj,aVP);
}

///////////////////////////////////////////////
sVec2f __stdcall cMath::ComputePixelWorldSizeVec2(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) const {
  return ni::ComputePixelWorldSizeVec2(afPixelSize,avPos,mtxWorldView,mtxProj,aVP);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::ComputePixelWorldSize(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) const {
  return ni::ComputePixelWorldSize(afPixelSize,avPos,amtxWorldView,amtxProj,aVP);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::Det2x2f(const tF32 a, const tF32 b, const tF32 c, const tF32 d) const
{
  return ni::Det2x2<tF32>(a,b,c,d);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::Det3x3f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8) const
{
  return ni::Det3x3<tF32>(a0,a1,a2,a3,a4,a5,a6,a7,a8);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::Det4x4f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8, const tF32 a9, const tF32 a10, const tF32 a11, const tF32 a12, const tF32 a13, const tF32 a14, const tF32 a15) const
{
  return ni::Det4x4<tF32>(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Det2x2d(const tF64 a, const tF64 b, const tF64 c, const tF64 d) const
{
  return ni::Det2x2<tF64>(a,b,c,d);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Det3x3d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8) const
{
  return ni::Det3x3<tF64>(a0,a1,a2,a3,a4,a5,a6,a7,a8);
}

///////////////////////////////////////////////
tF64 __stdcall cMath::Det4x4d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8, const tF64 a9, const tF64 a10, const tF64 a11, const tF64 a12, const tF64 a13, const tF64 a14, const tF64 a15) const
{
  return ni::Det4x4<tF64>(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::FovHzToVt(tF32 afHFov, tF32 afWbyHAspect) const
{
  return ni::FovHzToVt(afHFov,afWbyHAspect);
}

///////////////////////////////////////////////
tF32 __stdcall cMath::FovVtToHz(tF32 afVFov, tF32 afWbyHAspect) const
{
  return ni::FovVtToHz(afVFov,afWbyHAspect);
}

///////////////////////////////////////////////
sVec4f __stdcall cMath::TriangleAreaNormal(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const {
  sVec3f normal;
  tF32 area = ni::TriangleAreaAndNormal(normal,V0,V1,V2);
  return Vec4(normal.x,normal.y,normal.z,area);
}

///////////////////////////////////////////////
tBool __stdcall cMath::TriangleIsDegenerate(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const {
  return ni::TriangleIsDegenerate(V0,V1,V2);
}

///////////////////////////////////////////////
tBool __stdcall cMath::TriangleIntersectTriangle(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& U0, const sVec3f& U1, const sVec3f& U2) const {
  return ni::TriangleIntersectTriangle(
      V0.ptr(),V1.ptr(),V2.ptr(),
      U0.ptr(),U1.ptr(),U2.ptr());
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::TriangleIntersectRay(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const {
  sVec3f I = {niMaxF32,niMaxF32,niMaxF32};
  ni::TriangleIntersectRay(avOrigin.ptr(),avDir.ptr(),
                           V0.ptr(),V1.ptr(),V2.ptr(),
                           &I.x,&I.y,&I.z);
  return I;
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::TriangleIntersectRayCull(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) const {
  sVec3f I = {niMaxF32,niMaxF32,niMaxF32};
  ni::TriangleIntersectRayCull(avOrigin.ptr(),avDir.ptr(),
                               V0.ptr(),V1.ptr(),V2.ptr(),
                               &I.x,&I.y,&I.z);
  return I;
}

///////////////////////////////////////////////
tBool __stdcall cMath::TriangleIntersectAABB(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& avMin, const sVec3f& avMax) const {
  return ni::TriangleIntersectAABB(avMin,avMax,V0,V1,V2);
}

///////////////////////////////////////////////
sVec3f __stdcall cMath::TriangleBaryCentric(const sVec3f& A, const sVec3f& B, const sVec3f& C, const sVec3f& P) const {
  sVec3f I = {niMaxF32,niMaxF32,niMaxF32};
  ni::TriangleBaryCentric(I,A,B,C,P);
  return I;
}

///////////////////////////////////////////////
tF64 __stdcall cMath::ClampT(tF64 aX, tF64 aMin, tF64 aMax) const {
  return ni::ClampT(aX,aMin,aMax);
}
tF64 __stdcall cMath::RepeatT(tF64 aX, tF64 aMin, tF64 aMax) const {
  return ni::RepeatT(aX,aMin,aMax);
}
tF64 __stdcall cMath::MirrorT(tF64 aX, tF64 aMin, tF64 aMax) const {
  return ni::MirrorT(aX,aMin,aMax);
}
tF64 __stdcall cMath::CycleT(tF64 aX, tF64 aMin, tF64 aMax) const {
  return ni::CycleT(aX,aMin,aMax);
}
tF64 __stdcall cMath::CurveStep(tF64 a, tF64 b, const tF64 t) const {
  return ni::CurveStep(a,b,t);
}
tF64 __stdcall cMath::CurveLinear(tF64 V1, tF64 V2, tF64 s) const {
  return ni::CurveLinear(V1,V2,s);
}
tF64 __stdcall cMath::CurveCos(tF64 a, tF64 b, const tF64 t) const {
  return ni::CurveCos(a,b,t);
}
tF64 __stdcall cMath::CurveHermite(tF64 V1, tF64 T1, tF64 V2, tF64 T2, tF64 s) const {
  return ni::CurveHermite(V1,T1,V2,T2,s);
}
tF64 __stdcall cMath::CurveCatmullRom(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s) const {
  return ni::CurveCatmullRom(V1,V2,V3,V4,s);
}
tF64 __stdcall cMath::CurveCardinal(tF64 V1, tF64 V2, tF64 s, tF64 a) const {
  return ni::CurveCardinal(V1,V2,s,a);
}
tF64 __stdcall cMath::CurveCardinal4(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s, tF64 a) const {
  return ni::CurveCardinal(V1,V2,V3,V4,s,a);
}
tF64 __stdcall cMath::CycleLinear(tF64 aX, tF64 aMin, tF64 aMax) const {
  return ni::CycleLinear(aX,aMin,aMax);
}
tF64 __stdcall cMath::CycleCos(tF64 aX, tF64 aMin, tF64 aMax) const {
  return ni::CycleCos(aX,aMin,aMax);
}
tF64 __stdcall cMath::CycleCardinal(tF64 aX, tF64 aMin, tF64 aMax, tF64 a) const {
  return ni::CycleCardinal(aX,aMin,aMax,a);
}

sVec3f __stdcall cMath::MatrixDecomposeGetTranslation(const sMatrixf& aMatrix) {
  sVec3f T;
  return ni::MatrixDecomposeGetTranslation(T,aMatrix);
}
sVec3f __stdcall cMath::MatrixDecomposeGetZYX(const sMatrixf& aMatrix) {
  sVec3f R;
  return ni::MatrixDecomposeGetZYX(R,aMatrix);
}
sQuatf __stdcall cMath::MatrixDecomposeGetQuat(const sMatrixf& aMatrix) {
  sQuatf Q;
  return ni::MatrixDecomposeGetQuat(Q,aMatrix);
}
sVec4f __stdcall cMath::MatrixDecomposeGetScale(const sMatrixf& aMatrix) {
  sVec4f S;
  return ni::MatrixDecomposeGetScale(S,aMatrix);
}
sMatrixf __stdcall cMath::MatrixCompose(const sVec3f& aT, const sVec3f& aZYX, const sVec4f& aS) {
  sMatrixf mtx;
  return ni::MatrixCompose(mtx,aT,aZYX,aS);
}
sMatrixf __stdcall cMath::MatrixComposeQ(const sVec3f& aT, const sQuatf& aQ, const sVec4f& aS) {
  sMatrixf mtx;
  return ni::MatrixComposeQ(mtx,aT,aQ,aS);
}

tF64 cMath::ProbSum(tF64CVec* apProbs) {
  if (!apProbs || apProbs->size() < 1)
    return 0.0;
  return ni::ProbSum(apProbs->data(),apProbs->size());
}

tBool cMath::ProbNormalize(tF64CVec* apProbs) {
  if (!apProbs || apProbs->size() < 1)
    return eFalse;
  return ni::ProbNormalize(apProbs->data(),apProbs->size());
}

tBool cMath::ProbSampleBuildAliasMethodArrays(const tF64CVec* apProbs, tF64CVec* apAMQ, tU32CVec* apAMA)
{
  niCheck(apProbs && apProbs->size() >= 1, eFalse);
  niCheck(apAMQ, eFalse);
  niCheck(apAMA, eFalse);
  const tU32 n = apProbs->size();
  apAMQ->resize(n);
  apAMA->resize(n*2);
  ni::ProbSampleBuildAliasMethodArrays(
    apProbs->data(), n, apAMQ->data(), apAMA->data());
  return eTrue;
}

tBool cMath::ProbSampleAliasMethod(tU32CVec* apResults, const tF64CVec* apAMQ, const tU32CVec* apAMA)
{
  niCheck(apResults, eFalse);
  const tU32 nres = apResults->size();
  niCheck(nres >= 1, eFalse);
  niCheck(apAMQ, eFalse);
  const tU32 n = apAMQ->size();
  niCheck(n >= 1, eFalse);
  niCheck(apAMA, eFalse);
  niCheck(apAMA->size() == (n*2), eFalse);
  return ni::ProbSampleAliasMethod(
    apResults->data(), nres,
    apAMQ->data(), apAMA->data(), n,
    []() { return ni::RandFloat(); });
}

#endif

namespace ni {

niExportFunc(ni::iMath*) GetMath() {
  static cMath _math;
  return &_math;
}

niExportFunc(ni::iUnknown*) New_niLang_Math(const ni::Var&,const ni::Var&) {
  return GetMath();
}

}
