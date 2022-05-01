#ifndef __MATHPLANE_9071702_H__
#define __MATHPLANE_9071702_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "MathVec3.h"
#include "MathVec4.h"
#include "MathMatrix.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

///////////////////////////////////////////////
template <class T>
T PlaneDot(const sVec4<T>& P, const sVec4<T>& V)
{
  return P.A() * V.x + P.B() * V.y + P.C() * V.z + P.D() * V.w;
}

///////////////////////////////////////////////
template <class T>
T PlaneDotCoord(const sVec4<T>& P, const sVec3<T>& V)
{
  return P.A() * V.x + P.B() * V.y + P.C() * V.z + P.D();
}

///////////////////////////////////////////////
template <class T>
T PlaneDistance(const sVec4<T>& P, const sVec3<T>& V)
{
  return P.A() * V.x + P.B() * V.y + P.C() * V.z + P.D();
}

///////////////////////////////////////////////
template <class T>
T PlaneDotNormal(const sVec4<T>& P, const sVec3<T>& V)
{
  return P.A() * V.x + P.B() * V.y + P.C() * V.z;
}


///////////////////////////////////////////////
template <class T>
sVec4<T>& PlaneFromPointNormal(sVec4<T>& Out, const sVec3<T>& vPoint, const sVec3<T>& vNormal)
{
  Out.A() = vNormal.x;
  Out.B() = vNormal.y;
  Out.C() = vNormal.z;
  Out.D() = -VecDot(vNormal,vPoint);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& PlaneFromPoints(sVec4<T>& Out,
                             const sVec3<T>& V1,
                             const sVec3<T>& V2,
                             const sVec3<T>& V3)
{
  /*
    sVec3f vCOM = (V1+V2+V3)/3.0f;
    const sVec3f *vi, *vj;

    Out.Normal().Set(0.0f);

    vi = &V1; vj = &V2;
    Out.Normal().x += tF32(vi->y - vj->y) * tF32(vi->z + vj->z);
    Out.Normal().y += tF32(vi->z - vj->z) * tF32(vi->x + vj->x);
    Out.Normal().z += tF32(vi->x - vj->x) * tF32(vi->y + vj->y);

    vi = &V2; vj = &V3;
    Out.Normal().x += tF32(vi->y - vj->y) * tF32(vi->z + vj->z);
    Out.Normal().y += tF32(vi->z - vj->z) * tF32(vi->x + vj->x);
    Out.Normal().z += tF32(vi->x - vj->x) * tF32(vi->y + vj->y);

    vi = &V3; vj = &V1;
    Out.Normal().x += tF32(vi->y - vj->y) * tF32(vi->z + vj->z);
    Out.Normal().y += tF32(vi->z - vj->z) * tF32(vi->x + vj->x);
    Out.Normal().z += tF32(vi->x - vj->x) * tF32(vi->y + vj->y);

    VecNormalize(Out.Normal());
    Out.D() = -PlaneDotNormal(Out, vCOM);

    return Out;
  */
  sVec3<T> e0, e1;
  VecSub(e0, V2, V1);
  VecSub(e1, V3, V1);
  sVec3<T> n;
  Out.SetNormal(VecCross(n, e0, e1));
  Out.SetDist(-PlaneDotNormal(Out, V1));
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>* PlaneIntersectLine(const sVec4<T>& P,
                                const sVec3<T>& V1,
                                const sVec3<T>& V2,
                                sVec3<T>* pOut = NULL,
                                T *percent = NULL)
{
  sVec3<T> line(V2-V1), lineN;
  sVec4<T> pi(P);
  T     denom, t;

  VecNormalize(lineN,line);
  PlaneNormalize(pi);

  denom = PlaneDotNormal(pi,lineN);
  if (denom == 0)
    return NULL;

  t = -PlaneDotCoord(pi,V1) / denom;

  if (pOut)
    VecMAD(*pOut, lineN, t, V1);

  if (percent) *percent = t/VecLength(line);

  return pOut;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T> *PlaneIntersectRay(const sVec4<T>& P,
                               const sVec3<T>& avRayPos,
                               const sVec3<T>& avRayDir,
                               sVec3<T> *pOut = NULL,
                               T *dist = NULL)
{
  sVec3<T> line(avRayDir), lineN;
  sVec4<T> pi(P);
  T     denom, t;

  VecNormalize(lineN,line);
  PlaneNormalize(pi);

  denom = PlaneDotNormal(pi,lineN);
  if (denom == 0)
    return NULL;

  t = -PlaneDotCoord(pi,avRayPos) / denom;

  if (pOut)
    VecMAD(*pOut, lineN, t, avRayPos);

  if(dist) *dist = t;

  return pOut;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>* PlaneNormalize(sVec4<T>& Out, const sVec4<T>& P)
{
  T denom;

  denom = (T)sqrt(P.A() * P.A() + P.B() * P.B() + P.C() * P.C());
  if (denom == 0)
    return NULL;

  denom = 1.0f / denom;

  Out.A() = P.A() * denom;
  Out.B() = P.B() * denom;
  Out.C() = P.C() * denom;
  Out.D() = P.D() * denom;

  return &Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>* PlaneNormalize(sVec4<T>& Out)
{
  return PlaneNormalize(Out,Out);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& PlaneTransform(sVec4<T>& Out,
                            const sVec4<T>& P,
                            const sMatrix<T>& M)
{
  sMatrix<T>  m;
  T     a, b, c, d;

  MatrixInverse(m,M);

  a = P.A(); b = P.B(); c = P.C(); d = P.D();

  Out.A() = a * m._11 + b * m._12 + c * m._13 + d * m._14;
  Out.B() = a * m._21 + b * m._22 + c * m._23 + d * m._24;
  Out.C() = a * m._31 + b * m._32 + c * m._33 + d * m._34;
  Out.D() = a * m._41 + b * m._42 + c * m._43 + d * m._44;

  return *PlaneNormalize(Out);
}

///////////////////////////////////////////////
// Same as PlaneTransform but assume the matrix passed is already inversed.
template <class T>
sVec4<T>& PlaneTransformInversedMatrix(sVec4<T>& Out, const sVec4<T>& P, const sMatrix<T>& M)
{
  T a, b, c, d;
  a = P.A(); b = P.B(); c = P.C(); d = P.D();

  Out.A() = a * M._11 + b * M._12 + c * M._13 + d * M._14;
  Out.B() = a * M._21 + b * M._22 + c * M._23 + d * M._24;
  Out.C() = a * M._31 + b * M._32 + c * M._33 + d * M._34;
  Out.D() = a * M._41 + b * M._42 + c * M._43 + d * M._44;

  return *PlaneNormalize(Out);
}

///////////////////////////////////////////////
template <class T>
sVec3<T>* PlaneIntersection(sVec3<T>& Out, const sVec4<T>& PA, const sVec4<T>& PB, const sVec4<T>& PC)
{
  sVec3<T> BcC, CcA, AcB;

  T denom = VecDot(PA.GetNormal(), VecCross(BcC,PB.GetNormal(),PC.GetNormal()));
  if (denom == 0)
    return NULL;
  denom = 1/denom;

  VecCross(CcA, PC.GetNormal(), PA.GetNormal());
  VecCross(AcB, PA.GetNormal(), PB.GetNormal());

  Out.x = (-PA.D()*BcC.x - PB.D()*CcA.x - PC.D()*AcB.x) * denom;
  Out.y = (-PA.D()*BcC.y - PB.D()*CcA.y - PC.D()*AcB.y) * denom;
  Out.z = (-PA.D()*BcC.z - PB.D()*CcA.z - PC.D()*AcB.z) * denom;

  return &Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& PlaneClosest(sVec3<T>& Out, const sVec4<T>& P, const sVec3<T>& A)
{
  T dist = PlaneDotCoord(P, A);
  Out.x = A.x - P.x*dist;
  Out.y = A.y - P.y*dist;
  Out.z = A.z - P.z*dist;
  return Out;
}

///////////////////////////////////////////////
template <class T>
ePlaneType PlaneType(const sVec4<T>& Plane)
{
  if (Plane.x == 1.0 || Plane.x == -1.0)
    return ePlaneType_X;

  if (Plane.y == 1.0 || Plane.y == -1.0)
    return ePlaneType_Y;

  if (Plane.z == 1.0 || Plane.z == -1.0)
    return ePlaneType_Z;

  return ePlaneType_NonAxial;
}

///////////////////////////////////////////////
template <class T>
ePlaneType PlaneMaxType(const sVec4<T>& Plane)
{
  ePlaneType ret = ePlaneType_X;
  T fMaxNorm = (T)fabs(Plane.x);

  if(fMaxNorm < fabs(Plane.y))
  {
    fMaxNorm = (T)fabs(Plane.y);
    ret = ePlaneType_Y;
  }

  if(fMaxNorm < fabs(Plane.z))
  {
    fMaxNorm = (T)fabs(Plane.z);
    ret = ePlaneType_Z;
  }

  return ret;
}

///////////////////////////////////////////////
//! The output matrix will contain the coordinate system that corespond to the plane.
//! Up vector is the plane normal.
//! Right and Fwd lies on the plane.
//! If not NULL apFwdDir is the direction toward which the forward vector should be the closest.
//! By example if you want the forward vector to be the one that points down you'd pass sVec3f::OpYAxis() here.
template <class T>
sMatrix<T>& PlaneExtractCoordinateSystem(sMatrix<T>& Out, const sVec4<T>& Plane, tBool abTranspose, const sVec3<T>* apFwdDir, const sVec3<T>* apOrigin)
{
  sVec3<T> vOrg, vRight, vFwd;

  // find the major axis
  sVec3<T> vA = { 0, 0, 0 };
  if (ni::Abs(Plane.y) > ni::Abs(Plane.z)) {
    if (ni::Abs(Plane.z) < ni::Abs(Plane.x))  vA.z = 1.0f;
    else                      vA.x = 1.0f;
  }
  else {
    if (ni::Abs(Plane.y) <= ni::Abs(Plane.x)) vA.y = 1.0f;
    else                      vA.x = 1.0f;
  }

  VecNormalize(VecCross(vRight,   vA,  Plane.GetNormal()));
  VecNormalize(VecCross(vFwd,   vRight, Plane.GetNormal()));
  vOrg = -Plane.GetDist() * Plane.GetNormal();

  if (apFwdDir)
  {
    sVec3<T> vFwdDir = -*apFwdDir;
    T fRYCos = VecDot(vRight,vFwdDir);
    T fFYCos = VecDot(vFwd,vFwdDir);
    T fIRYCos = VecDot(-vRight,vFwdDir);
    T fIFYCos = VecDot(-vFwd,vFwdDir);

    T fSmallest = 1.0f;
    sVec3<T> newRight(vRight), newFwd(vFwd);
    if (fRYCos < fFYCos) {
      newFwd = vRight;
      newRight = vFwd;
      fSmallest = fRYCos;
    }
    else {
      fSmallest = fFYCos;
    }

    if (fIRYCos < fSmallest) {
      newFwd = -vRight;
      newRight = vFwd;
      fSmallest = fIRYCos;
    }
    if (fIFYCos < fSmallest) {
      newFwd = -vFwd;
      newRight = vRight;
      fSmallest = fIRYCos;
    }

    vFwd = newFwd;
    vRight = newRight;
  }

  const sVec3<T> planeNormal = Plane.GetNormal();
  if (abTranspose) {
    Out._11 = vRight.x; Out._21 = planeNormal.x;  Out._31 = vFwd.x;
    Out._12 = vRight.y; Out._22 = planeNormal.y;  Out._32 = vFwd.y;
    Out._13 = vRight.z; Out._23 = planeNormal.z;  Out._33 = vFwd.z;
  }
  else {
    Out._11 = vRight.x; Out._12 = planeNormal.x;  Out._13 = vFwd.x;
    Out._21 = vRight.y; Out._22 = planeNormal.y;  Out._23 = vFwd.y;
    Out._31 = vRight.z; Out._32 = planeNormal.z;  Out._33 = vFwd.z;
  }

  Out._14 = 0.0f;
  Out._24 = 0.0f;
  Out._34 = 0.0f;
  Out._44 = 1.0f;
  if (apOrigin) {
    Out._41 = apOrigin->x;
    Out._42 = apOrigin->y;
    Out._43 = apOrigin->z;
  }
  else {
    Out._41 = vOrg.x;
    Out._42 = vOrg.y;
    Out._43 = vOrg.z;
  }
  return Out;
}

///////////////////////////////////////////////
template <class T>
tBool PlaneIntersectSphere(const sVec4<T>& plane, const sVec3<T>& pos, T radius)
{
  tF32 d = PlaneDotCoord(plane, pos);
  return fabs(d) < radius;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHPLANE_9071702_H__
