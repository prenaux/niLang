#if defined MATH_IMPL_INCLUDE_DEPS || defined MATH_IMPL_INCLUDE_ONLY
#include <niCC.h>
#include "MathFPU.h"
#include "MathVec2.h"
#include "MathVec3.h"
#include "MathVec4.h"
#include "MathMatrix.h"
#include "MathCurves.h"
#include "MathPlane.h"
#include "MathProb.h"
#include "MathRect.h"
#include "MathUtils.h"
#include "MathTriangle.h"
#include "../Utils/Random.h"
#endif

#if !defined MATH_IMPL_INCLUDE_ONLY
// #define MATH_IMPL_FORWARDS
// #define MATH_IMPL_BEGIN namespace math_impl { using namespace ni;
// #define MATH_IMPL_END }
// #define MATH_IMPL_FN niInline niFn
// #define MATH_IMPL_CONST_OVR //niImpl

#ifndef MATH_IMPL_FORWARDS
#error "MATH_IMPL_FORWARDS must be defined before including MathImpl.h"
#elif ~(~MATH_IMPL_FORWARDS + 0) == 0 && ~(~MATH_IMPL_FORWARDS + 1) == 1
#error "MATH_IMPL_FORWARDS must be defined with a value, not just #define MATH_IMPL_FORWARDS"
#endif

#ifndef MATH_IMPL_BEGIN
#error "MATH_IMPL_BEGIN should be defined before including MathImpl.h"
#endif

#ifndef MATH_IMPL_END
#error "MATH_IMPL_END should be defined before including MathImpl.h"
#endif

#ifndef MATH_IMPL_FN
#error "MATH_IMPL_FN should be defined before including MathImpl.h"
#endif

#ifndef MATH_IMPL_CONST_OVR
#error "MATH_IMPL_CONST_OVR should be defined before including MathImpl.h"
#endif

MATH_IMPL_BEGIN;

#define MATH_IMPL_TRANSFORM(TYPE,VEC,TRAN)          \
  niAssert(t##TYPE##CVec::IsSameType(VEC));         \
  niCheckSilent(t##TYPE##CVec::IsSameType(VEC),;);  \
  s##TYPE* pData = apVecs->_Data();                 \
  niLoop(i,pData->size()) {                         \
    ni::TRAN(*pData,*pData,aMatrix);                \
    ++pData;                                        \
  }

#if MATH_IMPL_FORWARDS == 1
///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Sqrt(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Sqrt(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Sin(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Sin(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Cos(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Cos(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ASin(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::ASin(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ACos(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::ACos(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Tan(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Tan(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ATan(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::ATan(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ATan2(tF64 x, tF64 y) MATH_IMPL_CONST_OVR
{
  return ni::ATan2(x,y);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) LogX(tF64 v, tF64 base) MATH_IMPL_CONST_OVR
{
  return ni::LogX(v,base);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Log2(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Log2(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) LogE(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::LogE(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Log10(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Log10(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Pow(tF64 v, tF64 e) MATH_IMPL_CONST_OVR
{
  return ni::Pow(v,e);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Floor(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Floor(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Ceil(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Ceil(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Exp(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Exp(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Abs(tF64 v) MATH_IMPL_CONST_OVR
{
  return ni::Abs(v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ToRad(tF64 afDeg) MATH_IMPL_CONST_OVR
{
  return niRadf(afDeg);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ToDeg(tF64 afRad) MATH_IMPL_CONST_OVR
{
  return niDegf(afRad);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Min(tF64 a, tF64 b) MATH_IMPL_CONST_OVR
{
  return ni::Min(a,b);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Min3(tF64 a, tF64 b, tF64 c) MATH_IMPL_CONST_OVR
{
  return ni::Min(a,b,c);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Max(tF64 a, tF64 b) MATH_IMPL_CONST_OVR
{
  return ni::Max(a,b);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Max3(tF64 a, tF64 b, tF64 c) MATH_IMPL_CONST_OVR
{
  return ni::Max(a,b,c);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Clamp(tF64 v, tF64 afMin, tF64 afMax) MATH_IMPL_CONST_OVR
{
  return ni::Clamp(v,afMin,afMax);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ClampZeroOne(tF64 a) MATH_IMPL_CONST_OVR
{
  return ni::ClampZeroOne(a);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Lerp(tF64 a, tF64 b, tF64 f) MATH_IMPL_CONST_OVR
{
  return ni::Lerp(a,b,f);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) BlendIntoAccumulator(tF64 accumulator, tF64 newValue, tF64 smoothRate) MATH_IMPL_CONST_OVR
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
MATH_IMPL_FN(void) RandSeed(tU32 ulSeed) MATH_IMPL_CONST_OVR
{
  ni::RandSeed(ulSeed);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tI32) RandInt() MATH_IMPL_CONST_OVR
{
  return ni::RandInt();
}

///////////////////////////////////////////////
MATH_IMPL_FN(tI32) RandIntRange(tI32 aMin, tI32 aMax) MATH_IMPL_CONST_OVR
{
  return ni::RandIntRange(aMin,aMax);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) RandFloat() MATH_IMPL_CONST_OVR
{
  return ni::RandFloat();
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) RandFloatRange(tF32 afMin, tF32 afMax) MATH_IMPL_CONST_OVR
{
  return (tF32)ni::RandFloatRange((tF32)afMin,(tF32)afMax);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) RandNormal(tF32 sigma) MATH_IMPL_CONST_OVR
{
  return ni::RandNormal(sigma);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tU32) RandColorA(tU8 aA) MATH_IMPL_CONST_OVR {
  return ni::RandColorA(aA);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tU32) RandColorAf(tF32 aA) MATH_IMPL_CONST_OVR {
  return ni::RandColorAf(aA);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tU32) RandColor() MATH_IMPL_CONST_OVR {
  return ni::RandColor();
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) ComputePixelScaleX(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) MATH_IMPL_CONST_OVR
{
  return ni::ComputePixelScaleX(aPos,amtxWorldView,amtxProj,aVP);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) ComputePixelScaleY(const sVec3f& aPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) MATH_IMPL_CONST_OVR
{
  return ni::ComputePixelScaleY(aPos,amtxWorldView,amtxProj,aVP);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) ComputePixelWorldSizeX(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) MATH_IMPL_CONST_OVR {
  return ni::ComputePixelWorldSizeX(afPixelSize,avPos,mtxWorldView,mtxProj,aVP);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) ComputePixelWorldSizeY(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) MATH_IMPL_CONST_OVR {
  return ni::ComputePixelWorldSizeY(afPixelSize,avPos,mtxWorldView,mtxProj,aVP);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) ComputePixelWorldSizeVec2(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& mtxWorldView, const sMatrixf& mtxProj, const sRectf& aVP) MATH_IMPL_CONST_OVR {
  return ni::ComputePixelWorldSizeVec2(afPixelSize,avPos,mtxWorldView,mtxProj,aVP);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) ComputePixelWorldSize(const tF32 afPixelSize, const sVec3f& avPos, const sMatrixf& amtxWorldView, const sMatrixf& amtxProj, const sRectf& aVP) MATH_IMPL_CONST_OVR {
  return ni::ComputePixelWorldSize(afPixelSize,avPos,amtxWorldView,amtxProj,aVP);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) FovHzToVt(tF32 afHFov, tF32 afWbyHAspect) MATH_IMPL_CONST_OVR
{
  return ni::FovHzToVt(afHFov,afWbyHAspect);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) FovVtToHz(tF32 afVFov, tF32 afWbyHAspect) MATH_IMPL_CONST_OVR
{
  return ni::FovVtToHz(afVFov,afWbyHAspect);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) TriangleIsDegenerate(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) MATH_IMPL_CONST_OVR {
  return ni::TriangleIsDegenerate(V0,V1,V2);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) TriangleIntersectAABB(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& avMin, const sVec3f& avMax) MATH_IMPL_CONST_OVR {
  return ni::TriangleIntersectAABB(V0,V1,V2,avMin,avMax);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ClampT(tF64 aX, tF64 aMin, tF64 aMax) MATH_IMPL_CONST_OVR {
  return ni::ClampT(aX,aMin,aMax);
}
MATH_IMPL_FN(tF64) RepeatT(tF64 aX, tF64 aMin, tF64 aMax) MATH_IMPL_CONST_OVR {
  return ni::RepeatT(aX,aMin,aMax);
}
MATH_IMPL_FN(tF64) MirrorT(tF64 aX, tF64 aMin, tF64 aMax) MATH_IMPL_CONST_OVR {
  return ni::MirrorT(aX,aMin,aMax);
}
MATH_IMPL_FN(tF64) CycleT(tF64 aX, tF64 aMin, tF64 aMax) MATH_IMPL_CONST_OVR {
  return ni::CycleT(aX,aMin,aMax);
}
MATH_IMPL_FN(tF64) CurveStep(tF64 a, tF64 b, const tF64 t) MATH_IMPL_CONST_OVR {
  return ni::CurveStep(a,b,t);
}
MATH_IMPL_FN(tF64) CurveLinear(tF64 V1, tF64 V2, tF64 s) MATH_IMPL_CONST_OVR {
  return ni::CurveLinear(V1,V2,s);
}
MATH_IMPL_FN(tF64) CurveCos(tF64 a, tF64 b, const tF64 t) MATH_IMPL_CONST_OVR {
  return ni::CurveCos(a,b,t);
}
MATH_IMPL_FN(tF64) CurveHermite(tF64 V1, tF64 T1, tF64 V2, tF64 T2, tF64 s) MATH_IMPL_CONST_OVR {
  return ni::CurveHermite(V1,T1,V2,T2,s);
}
MATH_IMPL_FN(tF64) CurveCatmullRom(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s) MATH_IMPL_CONST_OVR {
  return ni::CurveCatmullRom(V1,V2,V3,V4,s);
}
MATH_IMPL_FN(tF64) CurveCardinal(tF64 V1, tF64 V2, tF64 s, tF64 a) MATH_IMPL_CONST_OVR {
  return ni::CurveCardinal(V1,V2,s,a);
}
MATH_IMPL_FN(tF64) CurveCardinal4(tF64 V1, tF64 V2, tF64 V3, tF64 V4, tF64 s, tF64 a) MATH_IMPL_CONST_OVR {
  return ni::CurveCardinal(V1,V2,V3,V4,s,a);
}
MATH_IMPL_FN(tF64) CycleLinear(tF64 aX, tF64 aMin, tF64 aMax) MATH_IMPL_CONST_OVR {
  return ni::CycleLinear(aX,aMin,aMax);
}
MATH_IMPL_FN(tF64) CycleCos(tF64 aX, tF64 aMin, tF64 aMax) MATH_IMPL_CONST_OVR {
  return ni::CycleCos(aX,aMin,aMax);
}
MATH_IMPL_FN(tF64) CycleCardinal(tF64 aX, tF64 aMin, tF64 aMax, tF64 a) MATH_IMPL_CONST_OVR {
  return ni::CycleCardinal(aX,aMin,aMax,a);
}
#endif // #if MATH_IMPL_FORWARDS == 1

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) RandomDirection(const sVec3f& vN) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::RandomDirection(ret,vN);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) RandomDirectionEx(const sVec3f& vN, tF32 fDeviAngle) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::RandomDirectionEx(ret,vN,fDeviAngle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) RotateRay(const sVec3f& avIn, const sVec3f& avN) MATH_IMPL_CONST_OVR
{
  sVec3f vOut = avIn;
  return ni::RotateRay(vOut,avN);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Add(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecAdd(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Sub(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecSub(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Mul(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecMul(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Div(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecDiv(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Scale(const sVec2f &aLeft, tF32 afRight) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecScale(ret,aLeft,afRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tI32) Vec2Compare(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sVec2f));
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec2Length(const sVec2f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecLength(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec2LengthSq(const sVec2f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecLengthSq(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Normalize(const sVec2f &aV) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecNormalize(ret,aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) Vec2IsNormal(const sVec2f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecIsNormal(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec2Dot(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  return ni::VecDot(aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Lerp(const sVec2f &aLeft, const sVec2f &aRight, tF32 afF) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecLerp(ret,aLeft,aRight,afF);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Lerp2(const sVec2f &aLeft, const sVec2f &aRight, const sVec2f& aF) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecLerp(ret,aLeft,aRight,aF);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2BlendIntoAccumulator(const sVec2f &accumulator, const sVec2f &newValue, tF32 smoothRate) MATH_IMPL_CONST_OVR
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Abs(const sVec2f &aV) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecAbs(ret,aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Min(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecMinimize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Max(const sVec2f &aLeft, const sVec2f &aRight) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecMaximize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2CatmullRom(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, const sVec2f &V4, tF32 s) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecCatmullRom(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2Hermite(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, const sVec2f &V4, tF32 s) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecHermite(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2BaryCentric(const sVec2f &V1, const sVec2f &V2, const sVec2f &V3, tF32 u, tF32 v) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecBaryCentric(ret,V1,V2,V3,u,v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec2Transform(const sVec2f &aVec, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecTransform(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2TransformCoord(const sVec2f &aVec, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecTransformCoord(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) Vec2TransformNormal(const sVec2f &aVec, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  sVec2f ret;
  return ni::VecTransformNormal(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
MATH_IMPL_FN(void) Vec2TransformCoordArray(tVec2fCVec *apVecs, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  MATH_IMPL_TRANSFORM(Vec2f,apVecs,VecTransformCoord);
}

///////////////////////////////////////////////
MATH_IMPL_FN(void) Vec2TransformNormalArray(tVec2fCVec *apVecs, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  MATH_IMPL_TRANSFORM(Vec2f,apVecs,VecTransformNormal);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Add(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecAdd(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Sub(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecSub(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Mul(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecMul(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Div(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecDiv(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Scale(const sVec3f &aLeft, tF32 afRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecScale(ret,aLeft,afRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tI32) Vec3Compare(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sVec3f));
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec3Length(const sVec3f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecLength(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec3LengthSq(const sVec3f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecLengthSq(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Normalize(const sVec3f &aV) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecNormalize(ret,aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) Vec3IsNormal(const sVec3f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecIsNormal(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec3Dot(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  return ni::VecDot(aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Cross(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecCross(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Lerp(const sVec3f &aLeft, const sVec3f &aRight, tF32 afF) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecLerp(ret,aLeft,aRight,afF);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Lerp2(const sVec3f &aLeft, const sVec3f &aRight, const sVec3f &aF) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecLerp(ret,aLeft,aRight,aF);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3BlendIntoAccumulator(const sVec3f &accumulator, const sVec3f &newValue, tF32 smoothRate) MATH_IMPL_CONST_OVR
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Abs(const sVec3f &aV) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecAbs(ret,aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Min(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecMinimize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Max(const sVec3f &aLeft, const sVec3f &aRight) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecMaximize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3CatmullRom(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, const sVec3f &V4, tF32 s) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecCatmullRom(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Hermite(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, const sVec3f &V4, tF32 s) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecHermite(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3BaryCentric(const sVec3f &V1, const sVec3f &V2, const sVec3f &V3, tF32 u, tF32 v) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecBaryCentric(ret,V1,V2,V3,u,v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec3Transform(const sVec3f &aVec, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecTransform(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3TransformCoord(const sVec3f &aVec, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecTransformCoord(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3TransformNormal(const sVec3f &aVec, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecTransformNormal(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
MATH_IMPL_FN(void) Vec3TransformCoordArray(tVec3fCVec *apVecs, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  MATH_IMPL_TRANSFORM(Vec3f,apVecs,VecTransformCoord);
}

///////////////////////////////////////////////
MATH_IMPL_FN(void) Vec3TransformNormalArray(tVec3fCVec *apVecs, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  MATH_IMPL_TRANSFORM(Vec3f,apVecs,VecTransformNormal);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Unproject(const sVec3f &avIn, const sRectf &aVP, const sMatrixf &amtxViewProj) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecUnproject(ret,avIn,aVP.GetLeft(),aVP.GetTop(),aVP.GetWidth(),aVP.GetHeight(),amtxViewProj);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Project(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) MATH_IMPL_CONST_OVR
{
  sVec3f v;
  return ni::VecProject(v,avPos,amtxWVP,aRect);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec3ProjectRHW(const sVec3f& avPos, const sMatrixf& amtxWVP, const sRectf& aRect) MATH_IMPL_CONST_OVR
{
  sVec4f v;
  return ni::VecProjectRHW(v,avPos,amtxWVP,aRect);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3Reflect(const sVec3f& Dir, const sVec3f& Normal) MATH_IMPL_CONST_OVR {
  sVec3f v;
  return ni::VecReflect(v,Dir,Normal);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Add(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecAdd(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Sub(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecSub(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Mul(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecMul(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Div(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecDiv(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Scale(const sVec4f &aLeft, tF32 afRight) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecScale(ret,aLeft,afRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tI32) Vec4Compare(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sVec4f));
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec4Length(const sVec4f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecLength(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec4LengthSq(const sVec4f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecLengthSq(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Normalize(const sVec4f &aV) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecNormalize(ret,aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) Vec4IsNormal(const sVec4f &aV) MATH_IMPL_CONST_OVR
{
  return ni::VecIsNormal(aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Vec4Dot(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  return ni::VecDot(aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Cross(const sVec4f& aA, const sVec4f& aB, const sVec4f& aC) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecCross(ret,aA,aB,aC);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Lerp(const sVec4f &aLeft, const sVec4f &aRight, tF32 afF) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecLerp(ret,aLeft,aRight,afF);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Lerp2(const sVec4f &aLeft, const sVec4f &aRight, const sVec4f &aF) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecLerp(ret,aLeft,aRight,aF);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4BlendIntoAccumulator(const sVec4f &accumulator, const sVec4f &newValue, tF32 smoothRate) MATH_IMPL_CONST_OVR
{
  return ni::BlendIntoAccumulator(accumulator,newValue,smoothRate);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Abs(const sVec4f &aV) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecAbs(ret,aV);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Min(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecMinimize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Max(const sVec4f &aLeft, const sVec4f &aRight) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecMaximize(ret,aLeft,aRight);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4CatmullRom(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, const sVec4f &V4, tF32 s) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecCatmullRom(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Hermite(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, const sVec4f &V4, tF32 s) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecHermite(ret,V1,V2,V3,V4,s);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4BaryCentric(const sVec4f &V1, const sVec4f &V2, const sVec4f &V3, tF32 u, tF32 v) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecBaryCentric(ret,V1,V2,V3,u,v);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) Vec4Transform(const sVec4f &aVec, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  sVec4f ret;
  return ni::VecTransform(ret,aVec,aMatrix);
}

///////////////////////////////////////////////
MATH_IMPL_FN(void) Vec4TransformArray(tVec4fCVec *apVecs, const sMatrixf &aMatrix) MATH_IMPL_CONST_OVR
{
  MATH_IMPL_TRANSFORM(Vec4f,apVecs,VecTransform);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixIdentity() MATH_IMPL_CONST_OVR
{
  return sMatrixf::Identity();
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) MatrixIsIdentity(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixIsIdentity(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixAdd(const sMatrixf &aLeft, const sMatrixf &aRight) MATH_IMPL_CONST_OVR
{
  return aLeft+aRight;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSub(const sMatrixf &aLeft, const sMatrixf &aRight) MATH_IMPL_CONST_OVR
{
  return aLeft-aRight;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixScale(const sMatrixf &aLeft, tF32 afRight) MATH_IMPL_CONST_OVR
{
  return aLeft*afRight;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixLerp(const sMatrixf &A, const sMatrixf &B, tF32 fFac) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixLerp(ret,A,B,fFac);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tI32) MatrixCompare(const sMatrixf &aLeft, const sMatrixf &aRight) MATH_IMPL_CONST_OVR
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sMatrixf));
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixMultiply(const sMatrixf &M1, const sMatrixf &M2) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixMultiply(ret,M1,M2);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) MatrixDeterminant2(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixDeterminant2(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) MatrixDeterminant3(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixDeterminant3(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) MatrixDeterminant(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixDeterminant(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) MatrixGetHandeness(const sMatrixf& M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixGetHandeness(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixInverse(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixInverse(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixTransformInverse(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixTransformInverse(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixLookAtRH(const sVec3f &Eye, const sVec3f &At, const sVec3f &Up) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixLookAtRH(ret,Eye,At,Up);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixLookAtLH(const sVec3f &Eye, const sVec3f &At, const sVec3f &Up) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixLookAtLH(ret,Eye,At,Up);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixOrthoRH(tF32 w, tF32 h, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixOrthoRH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixOrthoLH(tF32 w, tF32 h, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixOrthoLH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixOrthoOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixOrthoOffCenterRH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixOrthoOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixOrthoOffCenterLH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixPerspectiveRH(tF32 w, tF32 h, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixPerspectiveRH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixPerspectiveLH(tF32 w, tF32 h, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixPerspectiveLH(ret,w,h,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixPerspectiveFovRH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixPerspectiveFovRH(ret,fovy,aspect,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixPerspectiveFovLH(tF32 fovy, tF32 aspect, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixPerspectiveFovLH(ret,fovy,aspect,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixPerspectiveOffCenterRH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixPerspectiveOffCenterRH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixPerspectiveOffCenterLH(tF32 l, tF32 r, tF32 t, tF32 b, tF32 zn, tF32 zf) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixPerspectiveOffCenterLH(ret,l,r,t,b,zn,zf);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixReflect(const sPlanef &Plane) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixReflect(ret,Plane);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixRotationAxis(const sVec3f &V, tF32 angle) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixRotationAxis(ret,V,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixRotationQuat(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixRotationQuat(ret,Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixRotationX(tF32 angle) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixRotationX(ret,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixRotationY(tF32 angle) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixRotationY(ret,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixRotationZ(tF32 angle) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixRotationZ(ret,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixRotationYawPitchRoll(tF32 yaw, tF32 pitch, tF32 roll) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixRotationYawPitchRoll(ret,yaw,pitch,roll);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixDecomposeYawPitchRoll(const sMatrixf& M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixDecomposeYawPitchRoll(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixScaling(const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixScaling(ret,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixVecScale(const sMatrixf &M, const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixScale(ret,M,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixShadow(const sVec4f &vLight, const sPlanef &Plane) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixShadow(ret,vLight,Plane);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixTranslation(const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixTranslation(ret,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixTranspose(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixTranspose(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixTranspose3x3(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixTranspose3x3(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixTextureOffset(tF32 fBias, tI32 nTexW, tI32 nTexH) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixTextureOffset(ret,fBias,nTexW,nTexH);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixTextureOffset2(tF32 fBias, tI32 nTexW, tI32 nTexH, tI32 nDepthBits) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixTextureOffset2(ret,fBias,nTexW,nTexH,nDepthBits);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixGetForward(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::MatrixGetForward(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixGetUp(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::MatrixGetUp(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixGetRight(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::MatrixGetRight(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixGetTranslation(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::MatrixGetTranslation(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSetForward(const sMatrixf &M, const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sMatrixf v = M;
  return ni::MatrixSetForward(v,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSetUp(const sMatrixf &M, const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sMatrixf v = M;
  return ni::MatrixSetUp(v,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSetRight(const sMatrixf &M, const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sMatrixf v = M;
  return ni::MatrixSetRight(v,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSetTranslation(const sMatrixf &M, const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sMatrixf v = M;
  return ni::MatrixSetTranslation(v,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) MatrixIsNormal(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixIsNormal(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) MatrixIsOrthogonal(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixIsOrthogonal(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) MatrixIsOrthoNormal(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  return ni::MatrixIsOrthoNormal(M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSetRotation(const sMatrixf &In, const sMatrixf &RotM) MATH_IMPL_CONST_OVR
{
  sMatrixf v = In;
  return ni::MatrixSetRotation(v,RotM);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixToEuler(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sVec3f euler;
  return ni::MatrixToEuler(sMatrixf(),euler);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixToCoordinateSystem(const sVec3f &avRight, const sVec3f &avUp, const sVec3f &avFwd, const sVec3f &avOrg) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixToCoordinateSystem(ret,avRight,avUp,avFwd,avOrg);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixGetProjectedTranslation(const sMatrixf& In) MATH_IMPL_CONST_OVR
{
  sVec3f o;
  return ni::MatrixGetProjectedTranslation(o,In);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSetProjectedTranslation(const sMatrixf& In, const sVec3f& avT) MATH_IMPL_CONST_OVR
{
  sMatrixf v = In;
  return ni::MatrixSetProjectedTranslation(v,avT);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixSetNotRotation(const sMatrixf& In, const sMatrixf& M) MATH_IMPL_CONST_OVR
{
  sMatrixf v = In;
  return ni::MatrixSetNotRotation(v,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixRotate(const sMatrixf& M1, const sMatrixf& M2) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::MatrixRotate(ret,M1,M2);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) MatrixViewport(const sVec4f& aVP, tF32 afMinZ, tF32 afMaxZ) MATH_IMPL_CONST_OVR {
  sMatrixf ret;
  return ni::MatrixViewport(ret,aVP,afMinZ,afMaxZ);
}

MATH_IMPL_FN(sMatrixf) MatrixAdjustViewport(const sVec4f& aContextVP, const sVec4f& aVirtualVP, tF32 afMinZ, tF32 afMaxZ) MATH_IMPL_CONST_OVR {
  sMatrixf ret;
  return ni::MatrixAdjustViewport(ret,aContextVP,aVirtualVP,afMinZ,afMaxZ);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatIdentity() MATH_IMPL_CONST_OVR
{
  return (sQuatf::Identity());
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) QuatIsIdentity(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  return ni::QuatIsIdentity(Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatAdd(const sQuatf &aLeft, const sQuatf &aRight) MATH_IMPL_CONST_OVR
{
  return aLeft+aRight;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatSub(const sQuatf &aLeft, const sQuatf &aRight) MATH_IMPL_CONST_OVR
{
  return aLeft+aRight;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatScale(const sQuatf &aLeft, tF32 afRight) MATH_IMPL_CONST_OVR
{
  return aLeft*afRight;
}

///////////////////////////////////////////////
MATH_IMPL_FN(tI32) QuatCompare(const sQuatf &aLeft, const sQuatf &aRight) MATH_IMPL_CONST_OVR
{
  return ::memcmp(aLeft.ptr(),aRight.ptr(),sizeof(sQuatf));
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatBaryCentric(const sQuatf &Q1, const sQuatf &Q2, const sQuatf &Q3, tF32 f, tF32 g) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatBaryCentric(ret,Q1,Q2,Q3,f,g);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatConjugate(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatConjugate(ret,Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) QuatDot(const sQuatf &Q1, const sQuatf &Q2) MATH_IMPL_CONST_OVR
{
  return ni::QuatDot(Q1,Q2);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatExp(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatExp(ret,Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatInverse(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatInverse(ret,Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) QuatLength(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  return ni::QuatLength(Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) QuatLengthSq(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  return ni::QuatLengthSq(Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatLn(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatLn(ret,Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatMultiply(const sQuatf &Q1, const sQuatf &Q2) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatMultiply(ret,Q1,Q2);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatNormalize(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatNormalize(ret,Q);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationAxis(const sVec3f &V, tF32 angle) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationAxis(ret,V,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationX(tF32 angle) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationX(ret,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationY(tF32 angle) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationY(ret,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationZ(tF32 angle) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationZ(ret,angle);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationXYZ(const sVec3f &V) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationXYZ(ret,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationMatrix(const sMatrixf &M) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationMatrix(ret,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationYawPitchRoll(const tF32 yaw, const tF32 pitch, const tF32 roll) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationYawPitchRoll(ret,yaw,pitch,roll);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatRotationVector(const sVec3f &vFrom, const sVec3f &vTo) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatRotationVector(ret,vFrom,vTo);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatSlerp(const sQuatf &Q1, const sQuatf &Q2, tF32 t, eQuatSlerp mode) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatSlerp(ret,Q1,Q2,t,mode);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sQuatf) QuatSquad(const sQuatf &Q1, const sQuatf &Q2, const sQuatf &Q3, const sQuatf &Q4, tF32 t) MATH_IMPL_CONST_OVR
{
  sQuatf ret;
  return ni::QuatSquad(ret,Q1,Q2,Q3,Q4,t);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) QuatToAxisAngle(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sVec4f v;
  ni::QuatToAxisAngle(Q,(sVec3f&)v,v.w);
  return v;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) QuatToEuler(const sQuatf &Q) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::QuatToEuler(Q,ret);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) WrapFloat(tF32 aX, tF32 aMin, tF32 aMax) MATH_IMPL_CONST_OVR
{
  return ni::WrapFloat(aX,aMin,aMax);
}

MATH_IMPL_FN(tI32) WrapInt(tI32 aX, tI32 aMin, tI32 aMax) MATH_IMPL_CONST_OVR
{
  return ni::WrapInt(aX,aMin,aMax);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) WrapRad(tF32 aX) MATH_IMPL_CONST_OVR
{
  return ni::WrapRad(aX);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) WrapRad2(tF32 aX) MATH_IMPL_CONST_OVR
{
  return ni::WrapRad2(aX);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) GetAngleFromPoints2D(const sVec2f& aStartPos, const sVec2f&  aGoalPos) MATH_IMPL_CONST_OVR
{
  return ni::GetAngleFromPoints2D(aStartPos,aGoalPos);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec2f) GetAngleFromPoints3D(const sVec3f& aStartPos, const sVec3f& aGoalPos) MATH_IMPL_CONST_OVR
{
  return ni::GetAngleFromPoints3D(aStartPos,aGoalPos);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) GetAngularDifference(tF32 afA, tF32 afB) MATH_IMPL_CONST_OVR
{
  return ni::GetAngularDifference(afA,afB);
}

///////////////////////////////////////////////
MATH_IMPL_FN(eClassify) ClassifyPoint(const sPlanef& plane, const sVec3f& point) MATH_IMPL_CONST_OVR
{
  return ni::ClassifyPoint(plane,point);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) IsPointInsideEdge(const sVec3f& A, const sVec3f& B, const sVec3f& P) MATH_IMPL_CONST_OVR
{
  return ni::IsPointInsideEdge(A,B,P);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) ClosestPointOnLine(const sVec3f& A, const sVec3f& B, const sVec3f& P) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::ClosestPointOnLine(ret,A,B,P);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) ClosestPointOnTriangle(const sVec3f& A, const sVec3f& B, const sVec3f& C,const sVec3f& P) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::ClosestPointOnTriangle(ret,A,B,C,P);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) PlaneDot(const sPlanef& P, const sVec4f& V) MATH_IMPL_CONST_OVR
{
  return ni::PlaneDot(P,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) PlaneDotCoord(const sPlanef& P, const sVec3f& V) MATH_IMPL_CONST_OVR
{
  return ni::PlaneDotCoord(P,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) PlaneDotNormal(const sPlanef& P, const sVec3f& V) MATH_IMPL_CONST_OVR
{
  return ni::PlaneDotNormal(P,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) PlaneDistance(const sPlanef& P, const sVec3f& V) MATH_IMPL_CONST_OVR
{
  return ni::PlaneDistance(P,V);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sPlanef) PlaneFromPointNormal(const sVec3f& vPoint, const sVec3f& vNormal) MATH_IMPL_CONST_OVR
{
  sPlanef ret;
  return ni::PlaneFromPointNormal(ret,vPoint,vNormal);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sPlanef) PlaneFromPoints(const sVec3f& V1, const sVec3f& V2, const sVec3f& V3) MATH_IMPL_CONST_OVR
{
  sPlanef ret;
  return ni::PlaneFromPoints(ret,V1,V2,V3);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) PlaneIntersectLine(const sPlanef& P, const sVec3f& V1, const sVec3f& V2) MATH_IMPL_CONST_OVR
{
  sVec4f v;
  if (ni::PlaneIntersectLine(P,V1,V2,(sVec3f*)&v,&v.w) == NULL)
    return (sVec4f::Zero());
  return v;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) PlaneIntersectRay(const sPlanef& P, const sVec3f& avRayPos, const sVec3f& avRayDir) MATH_IMPL_CONST_OVR
{
  sVec4f v;
  if (ni::PlaneIntersectRay(P,avRayPos,avRayDir,(sVec3f*)&v,&v.w) == NULL)
    return (sVec4f::Zero());
  return v;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sPlanef) PlaneNormalize(const sPlanef& P) MATH_IMPL_CONST_OVR
{
  sPlanef p;
  if (ni::PlaneNormalize(p,P) == NULL)
    return sPlanef::Zero();
  return p;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sPlanef) PlaneTransform(const sPlanef& P, const sMatrixf& M) MATH_IMPL_CONST_OVR
{
  sPlanef ret;
  return ni::PlaneTransform(ret,P,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sPlanef) PlaneTransformInversedMatrix(const sPlanef& P, const sMatrixf& M) MATH_IMPL_CONST_OVR
{
  sPlanef ret;
  return ni::PlaneTransformInversedMatrix(ret,P,M);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) PlaneIntersection(const sPlanef& PA, const sPlanef& PB, const sPlanef& PC) MATH_IMPL_CONST_OVR
{
  sVec3f v;
  if (ni::PlaneIntersection(v,PA,PB,PC) == NULL)
    return (sVec3f::Zero());
  return v;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) PlaneClosest(const sPlanef& P, const sVec3f& A) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::PlaneClosest(ret,P,A);
}

///////////////////////////////////////////////
MATH_IMPL_FN(ePlaneType) PlaneType(const sPlanef& Plane) MATH_IMPL_CONST_OVR
{
  return ni::PlaneType(Plane);
}

///////////////////////////////////////////////
MATH_IMPL_FN(ePlaneType) PlaneMaxType(const sPlanef& Plane) MATH_IMPL_CONST_OVR
{
  return ni::PlaneMaxType(Plane);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sMatrixf) PlaneExtractCoordinateSystem(const sPlanef& Plane, const sVec3f& avFwdDir) MATH_IMPL_CONST_OVR
{
  sMatrixf ret;
  return ni::PlaneExtractCoordinateSystem<tF32>(ret,Plane,
                                                eFalse,
                                                (avFwdDir==sVec3f::Zero())?NULL:&avFwdDir,
                                                NULL);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) PlaneIntersectSphere(const sPlanef& plane, const sVec3f& avCenter, const tF32 afRadius) MATH_IMPL_CONST_OVR
{
  return ni::PlaneIntersectSphere(plane,avCenter,afRadius);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) GetScreenPosition(const sVec3f& avPos, const sMatrixf& mtxVP, const sRectf& aRect) MATH_IMPL_CONST_OVR {
  return ni::GetScreenPosition(avPos,mtxVP,aRect);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3SphericalToCartesian(const sVec3f& avS) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecSphericalToCartesian(ret,avS);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3CartesianToSpherical(const sVec3f& avC) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecCartesianToSpherical(ret,avC);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3OrbitPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afPitch) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecOrbitPitch(ret,aTarget,aPos,afPitch);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3OrbitYaw(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecOrbitYaw(ret,aTarget,aPos,afYaw);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) Vec3OrbitYawPitch(const sVec3f& aTarget, const sVec3f& aPos, tF32 afYaw, tF32 afPitch) MATH_IMPL_CONST_OVR
{
  sVec3f ret;
  return ni::VecOrbitYawPitch(ret,aTarget,aPos,afYaw,afPitch);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Det2x2f(const tF32 a, const tF32 b, const tF32 c, const tF32 d) MATH_IMPL_CONST_OVR
{
  return ni::Det2x2<tF32>(a,b,c,d);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Det3x3f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8) MATH_IMPL_CONST_OVR
{
  return ni::Det3x3<tF32>(a0,a1,a2,a3,a4,a5,a6,a7,a8);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF32) Det4x4f(const tF32 a0, const tF32 a1, const tF32 a2, const tF32 a3, const tF32 a4, const tF32 a5, const tF32 a6, const tF32 a7, const tF32 a8, const tF32 a9, const tF32 a10, const tF32 a11, const tF32 a12, const tF32 a13, const tF32 a14, const tF32 a15) MATH_IMPL_CONST_OVR
{
  return ni::Det4x4<tF32>(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Det2x2d(const tF64 a, const tF64 b, const tF64 c, const tF64 d) MATH_IMPL_CONST_OVR
{
  return ni::Det2x2<tF64>(a,b,c,d);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Det3x3d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8) MATH_IMPL_CONST_OVR
{
  return ni::Det3x3<tF64>(a0,a1,a2,a3,a4,a5,a6,a7,a8);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) Det4x4d(const tF64 a0, const tF64 a1, const tF64 a2, const tF64 a3, const tF64 a4, const tF64 a5, const tF64 a6, const tF64 a7, const tF64 a8, const tF64 a9, const tF64 a10, const tF64 a11, const tF64 a12, const tF64 a13, const tF64 a14, const tF64 a15) MATH_IMPL_CONST_OVR
{
  return ni::Det4x4<tF64>(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec4f) TriangleAreaNormal(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) MATH_IMPL_CONST_OVR {
  sVec3f normal;
  tF32 area = ni::TriangleAreaAndNormal(normal,V0,V1,V2);
  return Vec4(normal.x,normal.y,normal.z,area);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tBool) TriangleIntersectTriangle(const sVec3f& V0, const sVec3f& V1, const sVec3f& V2, const sVec3f& U0, const sVec3f& U1, const sVec3f& U2) MATH_IMPL_CONST_OVR {
  return ni::TriangleIntersectTriangle(
      V0.ptr(),V1.ptr(),V2.ptr(),
      U0.ptr(),U1.ptr(),U2.ptr());
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) TriangleIntersectRay(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) MATH_IMPL_CONST_OVR {
  sVec3f I = {niMaxF32,niMaxF32,niMaxF32};
  ni::TriangleIntersectRay(avOrigin.ptr(),avDir.ptr(),
                           V0.ptr(),V1.ptr(),V2.ptr(),
                           &I.x,&I.y,&I.z);
  return I;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) TriangleIntersectRayCull(const sVec3f& avOrigin, const sVec3f& avDir, const sVec3f& V0, const sVec3f& V1, const sVec3f& V2) MATH_IMPL_CONST_OVR {
  sVec3f I = {niMaxF32,niMaxF32,niMaxF32};
  ni::TriangleIntersectRayCull(avOrigin.ptr(),avDir.ptr(),
                               V0.ptr(),V1.ptr(),V2.ptr(),
                               &I.x,&I.y,&I.z);
  return I;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) TriangleBaryCentric(const sVec3f& A, const sVec3f& B, const sVec3f& C, const sVec3f& P) MATH_IMPL_CONST_OVR {
  sVec3f I = {niMaxF32,niMaxF32,niMaxF32};
  ni::TriangleBaryCentric(I,A,B,C,P);
  return I;
}

///////////////////////////////////////////////
MATH_IMPL_FN(sVec3f) MatrixDecomposeGetTranslation(const sMatrixf& aMatrix) MATH_IMPL_CONST_OVR {
  sVec3f T;
  return ni::MatrixDecomposeGetTranslation(T,aMatrix);
}
MATH_IMPL_FN(sVec3f) MatrixDecomposeGetZYX(const sMatrixf& aMatrix) MATH_IMPL_CONST_OVR {
  sVec3f R;
  return ni::MatrixDecomposeGetZYX(R,aMatrix);
}
MATH_IMPL_FN(sQuatf) MatrixDecomposeGetQuat(const sMatrixf& aMatrix) MATH_IMPL_CONST_OVR {
  sQuatf Q;
  return ni::MatrixDecomposeGetQuat(Q,aMatrix);
}
MATH_IMPL_FN(sVec4f) MatrixDecomposeGetScale(const sMatrixf& aMatrix) MATH_IMPL_CONST_OVR {
  sVec4f S;
  return ni::MatrixDecomposeGetScale(S,aMatrix);
}
MATH_IMPL_FN(sMatrixf) MatrixCompose(const sVec3f& aT, const sVec3f& aZYX, const sVec4f& aS) MATH_IMPL_CONST_OVR {
  sMatrixf mtx;
  return ni::MatrixCompose(mtx,aT,aZYX,aS);
}
MATH_IMPL_FN(sMatrixf) MatrixComposeQ(const sVec3f& aT, const sQuatf& aQ, const sVec4f& aS) MATH_IMPL_CONST_OVR {
  sMatrixf mtx;
  return ni::MatrixComposeQ(mtx,aT,aQ,aS);
}

///////////////////////////////////////////////
MATH_IMPL_FN(tF64) ProbSum(tF64CVec* apProbs) MATH_IMPL_CONST_OVR {
  if (!apProbs || apProbs->size() < 1)
    return 0.0;
  return ni::ProbSum(apProbs->data(),apProbs->size());
}

MATH_IMPL_FN(tBool) ProbNormalize(tF64CVec* apProbs) MATH_IMPL_CONST_OVR {
  if (!apProbs || apProbs->size() < 1)
    return eFalse;
  return ni::ProbNormalize(apProbs->data(),apProbs->size());
}

MATH_IMPL_FN(tBool) ProbSampleBuildAliasMethodArrays(const tF64CVec* apProbs, tF64CVec* apAMQ, tU32CVec* apAMA) MATH_IMPL_CONST_OVR
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

MATH_IMPL_FN(tBool) ProbSampleAliasMethod(tU32CVec* apResults, const tF64CVec* apAMQ, const tU32CVec* apAMA) MATH_IMPL_CONST_OVR
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

MATH_IMPL_END;
#endif // #if !defined MATH_IMPL_INCLUDE_ONLY

#undef MATH_IMPL_TRANSFORM
#undef MATH_IMPL_FORWARDS
#undef MATH_IMPL_BEGIN
#undef MATH_IMPL_END
#undef MATH_IMPL_FN
#undef MATH_IMPL_CONST_OVR
#undef MATH_IMPL_INCLUDE_DEPS
#undef MATH_IMPL_INCLUDE_ONLY
