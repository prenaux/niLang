#ifndef __MATHVECTOR3_2363461345_H__
#define __MATHVECTOR3_2363461345_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "MathRect.h"
#include "../Utils/Random.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

///////////////////////////////////////////////
template<typename T>
inline sVec3<T>& VecClamp(sVec3<T>& Out, const sVec3<T>& V, const sVec3<T>& aMin = sVec3<T>::Zero(), const sVec3<T>& aMax = sVec3<T>::One())
{
  niLoop(i,Out.size()) {
    Out[i] = ni::Clamp(V[i],aMin[i],aMax[i]);
  }
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecAdd(sVec3<T>& Out,
                    const sVec3<T>& V1,
                    const sVec3<T>& V2)
{
  Out.x = V1.x + V2.x;
  Out.y = V1.y + V2.y;
  Out.z = V1.z + V2.z;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecSub(sVec3<T>& Out,
                    const sVec3<T>& V1,
                    const sVec3<T>& V2)
{
  Out.x = V1.x - V2.x;
  Out.y = V1.y - V2.y;
  Out.z = V1.z - V2.z;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecMul(sVec3<T>& Out,
                    const sVec3<T>& V1,
                    const sVec3<T>& V2)
{
  Out.x = V1.x * V2.x;
  Out.y = V1.y * V2.y;
  Out.z = V1.z * V2.z;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecDiv(sVec3<T>& Out,
                    const sVec3<T>& V1,
                    const sVec3<T>& V2)
{
  Out.x = V1.x / V2.x;
  Out.y = V1.y / V2.y;
  Out.z = V1.z / V2.z;
  return Out;
}

///////////////////////////////////////////////
// pA + pB*pC
template <class T>
sVec3<T>& VecMAD(sVec3<T>& Out,
                    const sVec3<T>& A,
                    const sVec3<T>& B,
                    const sVec3<T>& C)
{
  Out.x = (A.x * B.x) + C.x;
  Out.y = (A.y * B.y) + C.y;
  Out.z = (A.z * B.z) + C.z;
  return Out;
}

///////////////////////////////////////////////
// pA + pB*scale
template <class T>
sVec3<T>& VecMAD(sVec3<T>& Out,
                    const sVec3<T>& A,
                    const T scale,
                    const sVec3<T>& B)
{
  Out.x = (A.x*scale) + B.x;
  Out.y = (A.y*scale) + B.y;
  Out.z = (A.z*scale) + B.z;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecBaryCentric(sVec3<T>& Out,
                            const sVec3<T>& V1,
                            const sVec3<T>& V2,
                            const sVec3<T>& V3,
                            T f,
                            T g)
{
  Out.x = V1.x + f * (V2.x - V1.x) + g * (V3.x - V1.x);
  Out.y = V1.y + f * (V2.y - V1.y) + g * (V3.y - V1.y);
  Out.z = V1.z + f * (V2.z - V1.z) + g * (V3.z - V1.z);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecCatmullRom(sVec3<T>& Out,
                           const sVec3<T>& V1,
                           const sVec3<T>& V2,
                           const sVec3<T>& V3,
                           const sVec3<T>& V4,
                           T s)
{
  T ss, sss, a, b, c, d;

  ss  = s * s;
  sss = s * ss;

  a = -0.5f * sss + ss - 0.5f * s;
  b =  1.5f * sss - 2.5f * ss + 1.0f;
  c = -1.5f * sss + 2.0f * ss + 0.5f * s;
  d =  0.5f * sss - 0.5f * ss;

  Out.x = a * V1.x + b * V2.x + c * V3.x + d * V4.x;
  Out.y = a * V1.y + b * V2.y + c * V3.y + d * V4.y;
  Out.z = a * V1.z + b * V2.z + c * V3.z + d * V4.z;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecCross(sVec3<T>& Out,
                      const sVec3<T>& V1,
                      const sVec3<T>& V2)
{
  T x = V1.y * V2.z - V1.z * V2.y;
  T y = V1.z * V2.x - V1.x * V2.z;
  T z = V1.x * V2.y - V1.y * V2.x;
  Out.x = x;
  Out.y = y;
  Out.z = z;
  return Out;
}
template <class T>
sVec3<T>* VecCross(sVec3<T>* Out,
                      const sVec3<T>* V1,
                      const sVec3<T>* V2)
{
  return &VecCross(*Out,*V1,*V2);
}

///////////////////////////////////////////////
template <class T>
T VecDot(const sVec3<T>& V1,
         const sVec3<T>& V2)
{
  return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z;
}
template <class T>
T VecDot(const sVec3<T>* V1,
         const sVec3<T>* V2)
{
  return VecDot(*V1,*V2);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecHermite(sVec3<T>& Out,
                        const sVec3<T>& V1,
                        const sVec3<T>& T1,
                        const sVec3<T>& V2,
                        const sVec3<T>& T2,
                        T s)
{
  T ss, sss, a, b, c, d;

  ss  = s * s;
  sss = s * ss;

  a =  2.0f * sss - 3.0f * ss + 1.0f;
  b = -2.0f * sss + 3.0f * ss;
  c =  sss - 2.0f * ss + s;
  d =  sss - ss;

  Out.x = a * V1.x + b * V2.x + c * T1.x + d * T2.x;
  Out.y = a * V1.y + b * V2.y + c * T1.y + d * T2.y;
  Out.z = a * V1.z + b * V2.z + c * T1.z + d * T2.z;

  return Out;
}

///////////////////////////////////////////////
template <class T>
T VecLength(const sVec3<T>& V)
{
  return (T)sqrt(V.x * V.x + V.y * V.y + V.z * V.z);
}

///////////////////////////////////////////////
template <class T>
T VecDistance(const sVec3<T>& A, const sVec3<T>& B)
{
  sVec3<T> v = B-A;
  return (T)VecLength(v);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecSetLength(sVec3<T>& Out, const sVec3<T>& V, T l)
{
  T fac = l/(T)sqrt(V.x*V.x + V.y*V.y + V.z*V.z);
  Out.x = V.x * fac;
  Out.y = V.y * fac;
  Out.z = V.z * fac;
  return Out;
}

///////////////////////////////////////////////
template <class T>
T VecLengthSq(const sVec3<T>& V)
{
  return V.x * V.x + V.y * V.y + V.z * V.z;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecMaximize(sVec3<T>& Out,
                         const sVec3<T>& V1,
                         const sVec3<T>& V2)
{
  Out.x = Max(V1.x, V2.x);
  Out.y = Max(V1.y, V2.y);
  Out.z = Max(V1.z, V2.z);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecMinimize(sVec3<T>& Out,
                         const sVec3<T>& V1,
                         const sVec3<T>& V2)
{
  Out.x = Min(V1.x, V2.x);
  Out.y = Min(V1.y, V2.y);
  Out.z = Min(V1.z, V2.z);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecNormalize(sVec3<T>& Out, const sVec3<T>& V)
{
  T denom = (T)sqrt(V.x * V.x + V.y * V.y + V.z * V.z);
  if (denom == 0) { Out.x = Out.y = Out.z = T(0); return Out; }
  denom = 1.0f / denom;
  Out.x = V.x * denom;
  Out.y = V.y * denom;
  Out.z = V.z * denom;
  return Out;
}
template <class T>
sVec3<T>* VecNormalize(sVec3<T>* Out, const sVec3<T>* V) {
  return &VecNormalize(*Out,*V);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecNormalize(sVec3<T>& Out)
{
  return VecNormalize(Out, Out);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecScale(sVec3<T>& Out,
                      const sVec3<T>& V,
                      T s)
{
  Out.x = V.x * s;
  Out.y = V.y * s;
  Out.z = V.z * s;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecTransform(sVec4<T>& Out,
                          const sVec3<T>& V,
                          const sMatrix<T>& M)
{
  T x, y, z;

  x = V.x; y = V.y; z = V.z;

  Out.x = x * M._11 + y * M._21 + z * M._31 + M._41;
  Out.y = x * M._12 + y * M._22 + z * M._32 + M._42;
  Out.z = x * M._13 + y * M._23 + z * M._33 + M._43;
  Out.w = x * M._14 + y * M._24 + z * M._34 + M._44;

  return Out;
}
template <class T>
sVec4<T>* VecTransform(sVec4<T>* Out,
                          const sVec3<T>* V,
                          const sMatrix<T>* M)
{
  return &VecTransform(*Out,*V,*M);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecTransformCoord(sVec3<T>& Out,
                               const sVec3<T>& V,
                               const sMatrix<T>& M)
{
  T rhw, x, y, z;
  x = V.x; y = V.y; z = V.z;
  rhw = (x * M._14 + y * M._24 + z * M._34 + M._44);
  if (rhw == 0) { Out.x = Out.y = Out.z = T(0); return Out; }
  rhw = 1.0f / rhw;
  Out.x = (x * M._11 + y * M._21 + z * M._31 + M._41) * rhw;
  Out.y = (x * M._12 + y * M._22 + z * M._32 + M._42) * rhw;
  Out.z = (x * M._13 + y * M._23 + z * M._33 + M._43) * rhw;
  return Out;
}
template <class T>
sVec3<T>* VecTransformCoord(sVec3<T>* Out,
                               const sVec3<T>* V,
                               const sMatrix<T>* M)
{
  return &VecTransformCoord(*Out,*V,*M);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecTransformNormal(sVec3<T>& Out,
                                const sVec3<T>& V,
                                const sMatrix<T>& M)
{
  T x, y, z;

  x = V.x; y = V.y; z = V.z;

  Out.x = x * M._11 + y * M._21 + z * M._31;
  Out.y = x * M._12 + y * M._22 + z * M._32;
  Out.z = x * M._13 + y * M._23 + z * M._33;

  return Out;
}
template <class T>
sVec3<T>* VecTransformNormal(sVec3<T>* Out,
                                const sVec3<T>* V,
                                const sMatrix<T>* M)
{
  return &VecTransformNormal(*Out,*V,*M);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecAbs(sVec3<T>& Out, const sVec3<T>& V)
{
  Out.x = (T)Abs(V.x);
  Out.y = (T)Abs(V.y);
  Out.z = (T)Abs(V.z);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecLerp(sVec3<T>& Out,
                     const sVec3<T>& V1,
                     const sVec3<T>& V2,
                     T fac)
{
  Out.x = Lerp(V1.x, V2.x, fac);
  Out.y = Lerp(V1.y, V2.y, fac);
  Out.z = Lerp(V1.z, V2.z, fac);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecLerp(sVec3<T>& Out,
                     const sVec3<T>& V1,
                     const sVec3<T>& V2,
                     const sVec3<T>& V3)
{
  Out.x = Lerp(V1.x, V2.x, V3.x);
  Out.y = Lerp(V1.y, V2.y, V3.y);
  Out.z = Lerp(V1.z, V2.z, V3.z);
  return Out;
}

///////////////////////////////////////////////
template <class T>
tBool VecIsNormal(const sVec3<T>& v, T epsilon = niEpsilon5)
{
  T len = VecLength(v);
  return (len >= 1-epsilon && len <= 1-epsilon);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecInverse(sVec3<T>& Out, const sVec3<T>& V)
{
  Out.x = 1/V.x;
  Out.y = 1/V.y;
  Out.z = 1/V.z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
inline sVec3<T>& VecToViewport(sVec3<T>& aOut, const ni::sVec3<T>& avProjected, const ni::sRect<T>& aRect)
{
  T fVPW2 = (T)aRect.GetWidth()/T(2);
  T fVPH2 = (T)aRect.GetHeight()/T(2);
  T fVPLeft = (T)aRect.GetLeft();
  T fVPBot = (T)aRect.GetBottom();
  T fVPTop = (T)aRect.GetTop();
  aOut.x = (avProjected.x)*fVPW2 + (fVPLeft+fVPW2);
  aOut.y = fVPBot-((avProjected.y)*fVPH2 + (fVPTop+fVPH2));
  aOut.z = avProjected.z;
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
inline sVec3<T>& VecProject(sVec3<T>& aOut, const ni::sVec3<T>& avPos, const ni::sMatrix<T>& amtxWVP, const ni::sRect<T>& aRect)
{
  sVec3<T> projected;
  VecTransformCoord(projected,avPos,amtxWVP);
  T fVPW2 = (T)aRect.GetWidth()/T(2);
  T fVPH2 = (T)aRect.GetHeight()/T(2);
  T fVPLeft = (T)aRect.GetLeft();
  T fVPBot = (T)aRect.GetBottom();
  T fVPTop = (T)aRect.GetTop();
  aOut.x = (projected.x)*fVPW2 + (fVPLeft+fVPW2);
  aOut.y = fVPBot-((projected.y)*fVPH2 + (fVPTop+fVPH2));
  aOut.z = projected.z;
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T>& VecProjectRHW(sVec4<T>& aOut, const ni::sVec3<T>& avPos, const ni::sMatrix<T>& mtxWVP, const ni::sRect<T>& aRect)
{
  sVec4<T> v1;
  ni::VecTransform(v1,avPos,mtxWVP);

  ni::tF32 fVPW2 = aRect.GetWidth()/T(2);
  ni::tF32 fVPH2 = aRect.GetHeight()/T(2);
  ni::tF32 fVPLeft = aRect.GetLeft();
  ni::tF32 fVPBot = aRect.GetBottom();
  ni::tF32 fVPTop = aRect.GetTop();

  ni::tF32 RHW = ni::FInvert(v1.w);
  aOut.x = (v1.x*RHW)*fVPW2 + (fVPLeft+fVPW2);
  aOut.y = fVPBot-((v1.y*RHW)*fVPH2 + (fVPTop+fVPH2));
  aOut.z = v1.z*RHW;
  aOut.w = RHW;

  return aOut;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& VecUnproject(sVec3<T>& avOut, const sVec3<T>& avIn,
                          T afVPX, T afVPY, T afVPW, T afVPH,
                          const sMatrix<T>& amtxViewProj)
{
  // Compute normalized coordinates between -1 and 1.
  sVec4<T> vNormalized;
  vNormalized.x =  ((avIn.x - afVPX)*T(2) / afVPW) - T(1);
  vNormalized.y = -(((avIn.y - afVPY)*T(2) / afVPH) - T(1));
  vNormalized.z = T(2)*avIn.z - T(1);
  vNormalized.w = T(1);

  // Object coordinates.
  sMatrix<T> invVP;
  VecTransform(vNormalized, vNormalized, MatrixInverse(invVP,amtxViewProj));
  if (vNormalized.w == T(0)) {
    avOut.x = avOut.y = avOut.z = 0.0f;
    return avOut;
  }

  avOut.x = vNormalized.x / vNormalized.w;
  avOut.y = vNormalized.y / vNormalized.w;
  avOut.z = vNormalized.z / vNormalized.w;
  return avOut;
}

///////////////////////////////////////////////
template <class T>
inline tBool VecEqual(const sVec3<T> &A, const sVec3<T> &B, T epsilon = niEpsilon5)
{
  return (ni::Abs(A.x-B.x)<=epsilon) &&
      (ni::Abs(A.y-B.y)<=epsilon) &&
      (ni::Abs(A.z-B.z)<=epsilon);
}

///////////////////////////////////////////////
template <class T>
inline sVec3<T>& VecReflect(sVec3<T>& Out, const sVec3<T>& Dir, const sVec3<T>& Normal)
{
  Out = Dir - (Normal*VecDot(Normal,Dir)*T(2));
  return Out;
}

///////////////////////////////////////////////
template <typename T>
T VecAngle(const sVec3<T>& v1, const sVec3<T>& v2) {
  sVec3<T> a; VecNormalize(a,v1);
  sVec3<T> b; VecNormalize(b,v2);
  T dot = VecDot(a,b);
  return ni::ACos(dot);
}

///////////////////////////////////////////////
template <class T>
int VecMaxAxis(const sVec3<T>& V)
{
  int ret = 0;
  T fMaxNorm = (T)ni::Abs(V.x);
  if (fMaxNorm < ni::Abs(V.y)) {
    fMaxNorm = (T)ni::Abs(V.y);
    ret = 1;
  }
  if (fMaxNorm < ni::Abs(V.z)) {
    fMaxNorm = (T)ni::Abs(V.z);
    ret = 2;
  }
  return ret;
}

///////////////////////////////////////////////
template <typename T>
inline ni::sVec3<T> __stdcall GetScreenPosition(const ni::sVec3<T>& avPos, const sMatrix<T>& mtxVP, const ni::sRect<T>& aRect)
{
  ni::sVec4<T> v;
  ni::VecTransform(v,avPos,mtxVP);
  T fVPW2 = aRect.GetWidth()/T(2);
  T fVPH2 = aRect.GetHeight()/T(2);
  T fVPLeft = aRect.GetLeft();
  T fVPBot = aRect.GetBottom();
  T fVPTop = aRect.GetTop();
  T RHW = ni::FInvert(v.w);
  v.x = (v.x*RHW)*fVPW2 + (fVPLeft+fVPW2);
  v.y = fVPBot-((v.y*RHW)*fVPH2 + (fVPTop+fVPH2));
  v.z *= RHW;
  v.w = RHW;  // could put RHW here instead of 1.0
  return v.GetPosition();
}

///////////////////////////////////////////////
//! Compute texture space of a triangle.
template <class T>
void ComputeTangentSpace(const sVec3<T>& PA, const sVec2<T>& TA,
                         const sVec3<T>& PB, const sVec2<T>& TB,
                         const sVec3<T>& PC, const sVec2<T>& TC,
                         sVec3<T>& vTangent0, sVec3<T>& vTangent1)
{
  // get edge vectors of the pixel triangle
  sVec3<T>  dp1 = PB-PA;
  sVec3<T>  dp2 = PC-PA;
  sVec2<T>  duv1 = TB-TA;
  sVec2<T>  duv2 = TC-TA;

  // solve the linear system
  sMatrix<T> M = sMatrix<T>::Identity();
  MatrixSetRight(M,dp1);
  MatrixSetUp(M,dp2);
  sVec3f fwd;
  MatrixSetForward(M,VecCross(fwd,dp1,dp2));
  sMatrix<T> inverseM;
  MatrixInverse(inverseM,M);
  sVec3<T> Ta,Bi;
  VecTransformNormal(Ta,Vec3<T>(duv1.x, duv2.x, 0.0f),inverseM);
  VecTransformNormal(Bi,Vec3<T>(duv1.y, duv2.y, 0.0f),inverseM);

  VecNormalize(vTangent0,Ta);
  VecNormalize(vTangent1,Bi);
}

///////////////////////////////////////////////
// Compute a PIXEL * Scale = WorldSize scale factor
template <typename T>
inline T ComputePixelScaleX(const sVec3<T>& pos, const sMatrix<T>& worldView, const sMatrix<T>& proj, const ni::sRect<T>& aRect)
{
  sVec3<T> ppcam0; VecTransformCoord(ppcam0,pos,worldView);
  sVec3<T> ppcam1 = ppcam0;
  ppcam1.x += T(1.0);
  VecTransformCoord(ppcam0,ppcam0,proj);
  VecTransformCoord(ppcam1,ppcam1,proj);
  VecToViewport(ppcam0,ppcam0,aRect);
  VecToViewport(ppcam1,ppcam1,aRect);
  return ni::FInvert(ni::Abs(ppcam1.x-ppcam0.x));
}

///////////////////////////////////////////////
// Compute a PIXEL * Scale = WorldSize scale factor
template <typename T>
inline T ComputePixelScaleY(const sVec3<T>& pos, const sMatrix<T>& worldView, const sMatrix<T>& proj, const ni::sRect<T>& aRect)
{
  sVec3<T> ppcam0; VecTransformCoord(ppcam0,pos,worldView);
  sVec3<T> ppcam1 = ppcam0;
  ppcam1.y += T(1.0);
  VecTransformCoord(ppcam0,ppcam0,proj);
  VecTransformCoord(ppcam1,ppcam1,proj);
  VecToViewport(ppcam0,ppcam0,aRect);
  VecToViewport(ppcam1,ppcam1,aRect);
  return ni::FInvert(ni::Abs(ppcam1.y-ppcam0.y));
}

///////////////////////////////////////////////
template <typename T>
inline T ComputePixelWorldSizeX(const T afPixelSize, const ni::sVec3<T>& avPos, const ni::sMatrix<T>& mtxWorldView, const ni::sMatrix<T>& mtxProj, const ni::sRect<T>& aRect)
{
  T worldUnitXPixelSize = ComputePixelScaleX(avPos,mtxWorldView,mtxProj,aRect);
  return afPixelSize * worldUnitXPixelSize;
}

///////////////////////////////////////////////
template <typename T>
inline T ComputePixelWorldSizeY(const T afPixelSize, const ni::sVec3<T>& avPos, const ni::sMatrix<T>& mtxWorldView, const ni::sMatrix<T>& mtxProj, const ni::sRect<T>& aRect)
{
  T worldUnitXPixelSize = ComputePixelScaleY(avPos,mtxWorldView,mtxProj,aRect);
  return afPixelSize * worldUnitXPixelSize;
}

///////////////////////////////////////////////
template <typename T>
inline sVec2<T> ComputePixelWorldSizeVec2(const T afPixelSize, const ni::sVec3<T>& avPos, const ni::sMatrix<T>& mtxWorldView, const ni::sMatrix<T>& mtxProj, const ni::sRect<T>& aRect)
{
  return Vec2<T>(
      ComputePixelWorldSizeX(afPixelSize,avPos,mtxWorldView,mtxProj,aRect),
      ComputePixelWorldSizeY(afPixelSize,avPos,mtxWorldView,mtxProj,aRect));
}

///////////////////////////////////////////////
template <typename T>
inline T ComputePixelWorldSize(const T afPixelSize, const ni::sVec3<T>& avPos, const ni::sMatrix<T>& mtxWorldView, const ni::sMatrix<T>& mtxProj, const ni::sRect<T>& aRect)
{
  if (aRect.GetWidth() >= aRect.GetHeight()) {
    return ComputePixelWorldSizeX(afPixelSize,avPos,mtxWorldView,mtxProj,aRect);
  }
  else {
    return ComputePixelWorldSizeY(afPixelSize,avPos,mtxWorldView,mtxProj,aRect);
  }
}

///////////////////////////////////////////////
template<typename T>
inline ni::sVec3<T>& VecSphericalToCartesian(ni::sVec3<T>& aOut, const ni::sVec3<T>& avS) {
  T o = avS.z;
  T co = ni::Cos(o);
  T so = ni::Sin(o);
  aOut.x = avS.x * so * ni::Cos(avS.y);
  aOut.y = avS.x * co;
  aOut.z = avS.x * so * ni::Sin(avS.y);
  return aOut;
}

///////////////////////////////////////////////
template<typename T>
inline ni::sVec3<T>& VecCartesianToSpherical(ni::sVec3<T>& aOut, const ni::sVec3<T>& avC) {
  T d = ni::VecLength(avC);
  aOut.x = d;
  aOut.y = ni::ATan2(avC.z,avC.x);
  aOut.z = ni::ACos(ni::FDiv(avC.y,d));
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
inline ni::sVec3<T>& VecOrbitPitch(ni::sVec3<T>& aOut, const ni::sVec3<T>& aTarget, const ni::sVec3<T>& aPos, T afPitch)
{
  ni::sVec3<T> s;
  VecCartesianToSpherical(s,aPos-aTarget);
  s.z = ni::Clamp<T>(s.z + afPitch,0+(T)niEpsilon3,(T)niPi-(T)niEpsilon3);
  aOut = aTarget + VecSphericalToCartesian(aOut,s);
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
inline ni::sVec3<T>& VecOrbitYaw(ni::sVec3<T>& aOut, const ni::sVec3<T>& aTarget, const ni::sVec3<T>& aPos, T afYaw)
{
  ni::sVec3<T> s;
  VecCartesianToSpherical(s,aPos-aTarget);
  s.y = WrapRad2(s.y + afYaw);
  aOut = aTarget + VecSphericalToCartesian(aOut,s);
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
inline ni::sVec3<T>& VecOrbitYawPitch(ni::sVec3<T>& aOut, const ni::sVec3<T>& aTarget, const ni::sVec3<T>& aPos, T afYaw, T afPitch)
{
  ni::sVec3<T> s;
  VecCartesianToSpherical(s,aPos-aTarget);
  s.y = WrapRad2(s.y + afYaw);
  s.z = ni::Clamp<T>(s.z + afPitch,0+(T)niEpsilon3,(T)niPi-(T)niEpsilon3);
  aOut = aTarget + VecSphericalToCartesian(aOut,s);
  return aOut;
}

///////////////////////////////////////////////
//! Generates a random direction in the upper hemisphere.
template <typename T>
inline sVec3<T>& RandomDirection(sVec3<T>& vDirOut, const sVec3<T>& vN, int4* aPRNG = ni_prng_global())
{
  do {
    const T fRand[2] = { (T)RandFloat(aPRNG), (float)RandFloat(aPRNG) };
    const T fTheta = (T)2 * ni::ACos(ni::Sqrt((T)1 - fRand[0]));
    const T fPhi = (T)2 * (T)niPi * fRand[1];
    vDirOut.x = ni::Sin(fTheta)*ni::Cos(fPhi);
    vDirOut.y = ni::Sin(fTheta)*ni::Sin(fPhi);
    vDirOut.z = ni::Cos(fTheta);
  } while (VecDot(vDirOut,vN) <= T(0));
  return vDirOut;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHVECTOR3_2363461345_H__
