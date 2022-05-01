#ifndef __UTILS_25387638_H__
#define __UTILS_25387638_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../IMath.h"
#include "../Utils/Random.h"

namespace ni {

///////////////////////////////////////////////
//! Counts number of 1 bits in i
inline tU32 BitCount(tU32 i) {
  i = i - ((i >> 1) & 0x55555555);
  i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
  i = (i + (i >> 4)) & 0x0F0F0F0F;
  i = i + (i >> 8);
  i = i + (i >> 16);
  return i & 0x0000003F;
}

///////////////////////////////////////////////
//! Return integer facorial of n
inline int Factorial(int n) {
  int prod = 1;
  for (int i = 1; i <= n; ++i) {
    prod *= i;
  }
  return prod;
}

///////////////////////////////////////////////
//! Check if the given size is power of 2.
inline tBool IsPow2(tU32 x)
{
  return ((x != 0) && !(x & (x - 1)));
}

///////////////////////////////////////////////
//! Find the value nearest to n which is also a power of two.
inline tU32 Round2(tU32 n)
{
  tU32 m;

  for(m = 1; m < n; m <<= 1) {}

  if(m-n <= n-(m>>1))
    return m;
  else
    return m>>1;
}

///////////////////////////////////////////////
//! Resize a not pow2 in pow2 size.
inline tU32 GetNearestPow2(tU32 size, tBool bGetLowerOnly = eFalse)
{
  if (IsPow2(size))
    return size;

  int ri;
  tU8 base = 1;

  ri = int(floor(log((float)(size))/log(2.0)));

  if((!bGetLowerOnly) && (size > 32) && (size < 256))
    base = 2;

  return base<<ri;
}

///////////////////////////////////////////////
//! Resize a not pow2 in pow2 size.
inline tU32 GetNextPow2(tU32 size)
{
  if (IsPow2(size)) return size;
  int ri = int(floor(log((float)(size))/log(2.0)));
  return 2<<ri;
}

///////////////////////////////////////////////
inline tU32 ComputeNumPow2Levels(tU32 w, tU32 h = 0, tU32 d = 0) {
  w = ni::Max(w,h,d);
  if (w <= 0) return 0;
  return 1+int(floor(log((float)(w))/log(2.0)));
}

///////////////////////////////////////////////
template <typename T>
T Det2x2(const T* apIn) {
  return apIn[0]*apIn[3] - apIn[1]*apIn[2];
}
template <typename T>
T Det2x2(const T a, const T b, const T c, const T d) {
  const T v[4] = { a, b, c, d };
  return Det2x2(v);
}

///////////////////////////////////////////////
template <typename T>
T Det3x3(const T* apIn) {
  T fA = apIn[4]*apIn[8] - apIn[5]*apIn[7];
  T fB = apIn[5]*apIn[6] - apIn[3]*apIn[8];
  T fC = apIn[3]*apIn[7] - apIn[4]*apIn[6];
  return apIn[0]*fA + apIn[1]*fB + apIn[2]*fC;
}
template <typename T>
T Det3x3(const T a0, const T a1, const T a2, const T a3, const T a4, const T a5, const T a6, const T a7, const T a8) {
  const T v[9] = { a0, a1, a2, a3, a4, a5, a6, a7, a8 };
  return Det3x3(v);
}

///////////////////////////////////////////////
template <typename T>
T Det4x4(const T* apIn) {
  T fA0 = apIn[ 0]*apIn[ 5] - apIn[ 1]*apIn[ 4];
  T fA1 = apIn[ 0]*apIn[ 6] - apIn[ 2]*apIn[ 4];
  T fA2 = apIn[ 0]*apIn[ 7] - apIn[ 3]*apIn[ 4];
  T fA3 = apIn[ 1]*apIn[ 6] - apIn[ 2]*apIn[ 5];
  T fA4 = apIn[ 1]*apIn[ 7] - apIn[ 3]*apIn[ 5];
  T fA5 = apIn[ 2]*apIn[ 7] - apIn[ 3]*apIn[ 6];
  T fB0 = apIn[ 8]*apIn[13] - apIn[ 9]*apIn[12];
  T fB1 = apIn[ 8]*apIn[14] - apIn[10]*apIn[12];
  T fB2 = apIn[ 8]*apIn[15] - apIn[11]*apIn[12];
  T fB3 = apIn[ 9]*apIn[14] - apIn[10]*apIn[13];
  T fB4 = apIn[ 9]*apIn[15] - apIn[11]*apIn[13];
  T fB5 = apIn[10]*apIn[15] - apIn[11]*apIn[14];
  return fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
}
template <typename T>
T Det4x4(const T a0, const T a1, const T a2, const T a3, const T a4, const T a5, const T a6, const T a7, const T a8, const T a9, const T a10, const T a11, const T a12, const T a13, const T a14, const T a15) {
  const T v[16] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 };
  return Det4x4(v);
}

///////////////////////////////////////////////
template <typename T>
T FovHzToVt(T afHFov, T afWbyHAspect) {
  return T(2.0)*ni::ATan(ni::Tan(afHFov/T(2.0))/afWbyHAspect);
}
///////////////////////////////////////////////
template <typename T>
T FovVtToHz(T afVFov, T afWbyHAspect) {
  return T(2.0)*ni::ATan(ni::Tan(afVFov/T(2.0))*afWbyHAspect);
}

///////////////////////////////////////////////
template <class A, class B>
A& Vec2Conv(A& Out, const B& In)
{
  Out.x = In.x;
  Out.y = In.y;
  return Out;
}

///////////////////////////////////////////////
template <class A, class B>
A& Vec3Conv(A& Out, const B& In)
{
  Out.x = In.x;
  Out.y = In.y;
  Out.z = In.z;
  return Out;
}

///////////////////////////////////////////////
template <class A, class B>
A& Vec4Conv(A& Out, const B& In)
{
  Out.x = In.x;
  Out.y = In.y;
  Out.z = In.z;
  Out.w = In.w;
  return Out;
}

///////////////////////////////////////////////
// Classify a point around the given plane.
template <class T>
eClassify ClassifyPoint(const sVec4<T>& plane, const sVec3<T>& point, const T epsilon = niEpsilon5)
{
  T result = PlaneDotCoord(plane, point);

  if(result < -epsilon)
    return eClassify_Back;

  if(result > epsilon)
    return eClassify_Front;

  return eClassify_Coplanar;
}



///////////////////////////////////////////////
// Returns eTrue/eFalse if point 'p' is between 'a' and 'b'.
// This routine assumes that all three points are linear.
template <class T>
tBool IsPointInsideEdge(const sVec3<T>& A, const sVec3<T>& B, const sVec3<T>& P)
{
  return VecDot(A-P, B-P) <= 0;
}

///////////////////////////////////////////////
// Return the closest point on a line defined by A and B.
// A-B is the line and P is the point in question.
template <class T>
sVec3<T>& ClosestPointOnLine(sVec3<T>& Out,
                                const sVec3<T>& A,
                                const sVec3<T>& B,
                                const sVec3<T>& P)
{
  // Determine t (the length of the vector from a to p)
  sVec3<T> c = P-A;
  sVec3<T> V = B-A;
  VecNormalize(V);

  T t = VecDot(V,c);

  // Return the point between a and b
  // set length of V to t. V is normalized so this it's easy.
  return VecMAD(Out, V, t, A);
}

///////////////////////////////////////////////
// Returns the closest point to 'p' on the line-segment defined by a & b.
// Sets 'pbEdge' to eTrue if result is one of the endpoints
template <class T>
sVec3<T>& ClosestPointOnLineSegment(sVec3<T>& Out,
                                       const sVec3<T>& A,
                                       const sVec3<T>& B,
                                       const sVec3<T>& P,
                                       tBool* pbEdge = NULL)
{
  // Determine t (the length of the vector from 'a' to 'p')
  sVec3<T> c = P-A;
  sVec3<T> V = B-A;
  VecNormalize(V);

  if (pbEdge) *pbEdge = eFalse;
  T t = VecDot(V,c);
  // Check to see if 't' is beyond the extents of the line segment
  if (t < 0.0f)
  {
    Out = A;
    return Out;
  }

  T d = VecDistance(B, A);
  if (t > d)
  {
    Out = B;
    return Out;
  }

  // Return the point between 'a' and 'b'
  // set length of V to t. V is normalized so this it's easy.
  if (pbEdge) *pbEdge = eTrue;
  return VecMAD(Out, V, t, A);
}

///////////////////////////////////////////////
// Return the closest point on line triangle edge.
// A-B-C the triangle and P the point in question.
template <class T>
sVec3<T>& ClosestPointOnTriangle(sVec3<T>& Out,
                                    const sVec3<T>& A,
                                    const sVec3<T>& B,
                                    const sVec3<T>& C,
                                    const sVec3<T>& P)
{
  sVec3<T> Rab, Rbc, Rca;
  ClosestPointOnLine(Rab, A, B, P);
  ClosestPointOnLine(Rbc, B, C, P);
  ClosestPointOnLine(Rca, C, A, P);

  T dAB = VecLength(P-Rab);
  T dBC = VecLength(P-Rbc);
  T dCA = VecLength(P-Rca);

  T min = dAB;
  Out = Rab;

  if(dBC < min)
  {
    min = dBC;
    Out = Rbc;
  }

  if(dCA < min)
  {
    Out = Rca;
  }

  return Out;
}

///////////////////////////////////////////////
//! Interpolate three components using the given barycenter
template <typename T, typename TB>
TB InterpolateBaryCentric(const sVec3<T>& avBaryCenter, const TB& A, const TB& B, const TB& C)
{
  return (avBaryCenter.x*A) + (avBaryCenter.y*B) + (avBaryCenter.z*C);
}

///////////////////////////////////////////////
//! Generates a random direction in the upper hemisphere.
//! fDeviAngle represents the spreading range of the ray, 180deg (pi rad) will allow the ray to be spread
//! away from -90, to 90 degrees, 90deg (pi/2 rad) will allow the ray to be spread away from -45 to 45
//! degrees, and so on...
template <typename T>
inline sVec3<T>& RandomDirectionEx(sVec3<T>& vDirOut, const sVec3<T>& vN, tF32 fDeviAngle)
{
  // Random cosine weighted direction
  const float fRand[2] = { (T)ni::RandFloatRange(0,fDeviAngle/ni::kPi), (T)(1.0f-ni::RandFloatRange(0,fDeviAngle/ni::kPi)) };
  const float fInvSq = sqrtf(1.0f - fRand[1]);
  vDirOut.x = cosf(2.0f * ni::kPi * fRand[0]) * fInvSq;
  vDirOut.y = sinf(2.0f * ni::kPi * fRand[0]) * fInvSq;
  vDirOut.z = sqrtf(fRand[1]);
  niAssert(VecDot(vDirOut,sVec3<T>::ZAxis()) > -0.01f);

  // Generate basis along normal so we can rotate the direction
  sVec3<T> vXAxis, vYAxis, vZAxis;
  vZAxis = vN; // Z axis points along the normal
  VecNormalize(vZAxis);
  sVec3<T> vUp = sVec3<T>::YAxis(); // Up vector
  if (fabsf(VecDot(vUp,vZAxis)) > 0.9f)
    vUp = sVec3<T>::XAxis();
  VecCross(vXAxis,vUp,vZAxis); // X Axis perpendicular to Up and Z
  VecNormalize(vXAxis);
  VecCross(vYAxis,vZAxis,vXAxis); // Y Axis perpendicular to Z and X
  VecNormalize(vYAxis);

  // Rotate sample
  sVec3<T> vDirTmp = vDirOut;
  vDirOut.x = vDirTmp.x * vXAxis.x + vDirTmp.y * vYAxis.x + vDirTmp.z * vZAxis.x;
  vDirOut.y = vDirTmp.x * vXAxis.y + vDirTmp.y * vYAxis.y + vDirTmp.z * vZAxis.y;
  vDirOut.z = vDirTmp.x * vXAxis.z + vDirTmp.y * vYAxis.z + vDirTmp.z * vZAxis.z;

  return vDirOut;
};

///////////////////////////////////////////////
template <typename T>
inline sVec3<T>& RotateRay(sVec3<T>& avOut, const sVec3<T>& avN)
{
  // Generate basis along normal so we can rotate the direction
  sVec3<T> vXAxis, vYAxis, vZAxis;
  vZAxis = avN; // Z axis points along the normal
  VecNormalize(vZAxis);
  sVec3<T> vUp = sVec3<T>::YAxis(); // Up vector
  if (fabsf(VecDot(vUp,vZAxis)) > 0.9f)
    vUp = sVec3<T>::XAxis();
  VecCross(vXAxis,vUp,vZAxis); // X Axis perpendicular to Up and Z
  VecNormalize(vXAxis);
  VecCross(vYAxis,vZAxis,vXAxis); // Y Axis perpendicular to Z and X
  VecNormalize(vYAxis);

  // Rotate sample
  sVec3<T> vDirTmp = avOut;
  avOut.x = vDirTmp.x * vXAxis.x + vDirTmp.y * vYAxis.x + vDirTmp.z * vZAxis.x;
  avOut.y = vDirTmp.x * vXAxis.y + vDirTmp.y * vYAxis.y + vDirTmp.z * vZAxis.y;
  avOut.z = vDirTmp.x * vXAxis.z + vDirTmp.y * vYAxis.z + vDirTmp.z * vZAxis.z;
  return avOut;
}

///////////////////////////////////////////////
template <typename T>
inline T GetAngleFromPoints2D(const sVec2<T>& aStartPos, const sVec2<T>&  aGoalPos)
{
  T Angle = 0.0f;
  T Dx, Dy;

  Dx = (aGoalPos.x - aStartPos.x);
  Dy = (aGoalPos.y - aStartPos.y);

  if (Dy == 0) Dy = niEpsilon5;
  if (Dx == 0) Dx = niEpsilon5;

  if (Dy >= 0 && Dx >= 0) {
    Angle = ni::ATan(Dx/Dy);
  }
  else if (Dy < 0 && Dx >= 0) {
    Angle = ni::ATan((-Dy)/Dx) + ni::kPi2;
  }
  else if (Dy < 0 && Dx < 0) {
    Angle = ni::ATan(Dx/Dy) + ni::kPi;
  }
  else if (Dy >= 0 && Dx < 0) {
    Angle = ni::ATan(Dy/(-Dx)) + ni::kPi+ni::kPi2;
  }

  return Angle;
}

///////////////////////////////////////////////
template <typename T>
inline sVec2<T> GetAngleFromPoints3D(const sVec3<T>& aStartPos, const sVec3<T>& aGoalPos)
{
  sVec2<T> Angle = sVec2<T>::Zero();
  Angle.y = WrapRad(GetAngleFromPoints2D(Vec2<T>(aStartPos.x,aStartPos.z), Vec2<T>(aGoalPos.x,aGoalPos.z)));
  sVec3<T> RotPos = aGoalPos - aStartPos;
  sMatrixf MtxRot, MtxGoal;
  MatrixRotationY(MtxRot,-Angle.y);
  MatrixMultiply(MtxGoal,MatrixTranslation(MtxGoal,RotPos),MtxRot);
  sVec3<T> Goal;
  MatrixGetTranslation(Goal,MtxGoal);
  Angle.x = WrapRad(GetAngleFromPoints2D(Vec2<T>(0,0),Vec2<T>(Goal.z,Goal.y)));
  return Angle;
}

///////////////////////////////////////////////
template <typename T>
inline T GetAngularDifference(T afAngle1, T afAngle2)
{
  const T kfMaxAngle = T(ni::k2Pi);
  afAngle1 = WrapFloat(afAngle1,T(0),kfMaxAngle);
  afAngle2 = WrapFloat(afAngle2,T(0),kfMaxAngle);

  if(afAngle1==afAngle2)
  {
    return 0;
  }
  else
  {
    T fDist1 = afAngle2 - afAngle1;
    T fDist2 = kfMaxAngle - ni::Abs(fDist1);

    if (fDist1 > 0)
      fDist2 = -fDist2;

    if (ni::Abs(fDist1) < ni::Abs(fDist2))
      return fDist1;
    else
      return fDist2;
  }
}

///////////////////////////////////////////////
template <typename T>
inline void ExtractFrustumPlanes(sVec4<T> vPlanes[6], const sMatrix<T>& mtxVP, tBool abNormalize = eTrue, tBool abOutward = eFalse) {
  // Near clipping plane
  vPlanes[4].x = (mtxVP._14 + mtxVP._13);
  vPlanes[4].y = (mtxVP._24 + mtxVP._23);
  vPlanes[4].z = (mtxVP._34 + mtxVP._33);
  vPlanes[4].w = (mtxVP._44 + mtxVP._43);

  // Far clipping plane
  vPlanes[5].x = (mtxVP._14 - mtxVP._13);
  vPlanes[5].y = (mtxVP._24 - mtxVP._23);
  vPlanes[5].z = (mtxVP._34 - mtxVP._33);
  vPlanes[5].w = (mtxVP._44 - mtxVP._43);

  // Left clipping plane
  vPlanes[0].x = (mtxVP._14 + mtxVP._11);
  vPlanes[0].y = (mtxVP._24 + mtxVP._21);
  vPlanes[0].z = (mtxVP._34 + mtxVP._31);
  vPlanes[0].w = (mtxVP._44 + mtxVP._41);

  // Right clipping plane
  vPlanes[1].x = (mtxVP._14 - mtxVP._11);
  vPlanes[1].y = (mtxVP._24 - mtxVP._21);
  vPlanes[1].z = (mtxVP._34 - mtxVP._31);
  vPlanes[1].w = (mtxVP._44 - mtxVP._41);

  // Top clipping plane
  vPlanes[2].x = (mtxVP._14 - mtxVP._12);
  vPlanes[2].y = (mtxVP._24 - mtxVP._22);
  vPlanes[2].z = (mtxVP._34 - mtxVP._32);
  vPlanes[2].w = (mtxVP._44 - mtxVP._42);

  // Bottom clipping plane
  vPlanes[3].x = (mtxVP._14 + mtxVP._12);
  vPlanes[3].y = (mtxVP._24 + mtxVP._22);
  vPlanes[3].z = (mtxVP._34 + mtxVP._32);
  vPlanes[3].w = (mtxVP._44 + mtxVP._42);

  if (abOutward) {
    for (tU32 i = 0; i < 6; ++i) {
      vPlanes[i] = -vPlanes[i];
    }
  }

  if (abNormalize) {
    for (tU32 i = 0; i < 6; ++i) {
      PlaneNormalize(vPlanes[i]);
    }
  }
}

///////////////////////////////////////////////
template <typename T>
inline T ZeroToEpsilon(T aValue) {
  if (aValue == (T)0) {
    return ni::TypeEpsilon<T>() * ni::Sign(aValue);
  }
  return aValue;
}
template<>
inline tF32 ZeroToEpsilon(tF32 aValue) {
  if (niFloatIsZero(aValue)) {
    return ni::TypeEpsilon<tF32>() * ni::Sign(aValue);
  }
  return aValue;
}
template<typename T>
inline sVec2<T> VecZeroToEpsilon(sVec2<T> aValue) {
  sVec2<T> r = {
    ZeroToEpsilon(aValue.x),
    ZeroToEpsilon(aValue.y)
  };
  return r;
}
template<typename T>
inline sVec3<T> VecZeroToEpsilon(sVec3<T> aValue) {
  sVec3<T> r = {
    ZeroToEpsilon(aValue.x),
    ZeroToEpsilon(aValue.y),
    ZeroToEpsilon(aValue.z)
  };
  return r;
}
template<typename T>
inline sVec4<T> VecZeroToEpsilon(sVec4<T> aValue) {
  sVec4<T> r = {
    ZeroToEpsilon(aValue.x),
    ZeroToEpsilon(aValue.y),
    ZeroToEpsilon(aValue.z),
    ZeroToEpsilon(aValue.w)
  };
  return r;
}

///////////////////////////////////////////////
inline sVec3f RainbowColor(tF64 step, tF64 numOfSteps) {
  // This function generates vibrant, "evenly spaced" colours (i.e. no
  // clustering). This is ideal for creating easily distinguishable vibrant
  // markers in Google Maps and other apps.
  tF32 h = ni::FMod(step,numOfSteps) / numOfSteps;
  tI32 i = (tI32)(h * 6);
  tF32 f = (h * 6) - (tF32)i;
  tF32 q = 1 - f;
  float r = 0.0, g = 0.0, b = 0.0;
  switch(i % 6) {
  case 0: r = 1; g = f; b = 0; break;
  case 1: r = q; g = 1; b = 0; break;
  case 2: r = 0; g = 1; b = f; break;
  case 3: r = 0; g = q; b = 1; break;
  case 4: r = f; g = 0; b = 1; break;
  case 5: r = 1; g = 0; b = q; break;
  }
  return Vec3f(r,g,b);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
}; // End of ni
#endif // __UTILS_25387638_H__
