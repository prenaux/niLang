#ifndef __AABB_17223519_H__
#define __AABB_17223519_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../IBoundingVolume.h"
#include "../IFrustum.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Axis aligned bounding box template class interface
/*!
  <pre>
vertex[x]
    \                                    Y+    Z+
     7 <===  size  ===> 6 (max)          |    /
      *===============*                  |   /
     /|              /|                  |  /
    / |             / |                  | /
 4 /  |          5 /  |                  |/
  *===+===========*   |        ==========+========== X+
  |   |           |   |                 /|
  |   |           |   |                / |
  | 3 *===========+===* 2             /  |
  |  /            |  /               /   |
  | /             | /               /    |
  |/              |/
  *===============*
 0 (min)        1

  Vertices :
  0 = bottom left front (min)
  1 = bottom right front
  2 = bottom right back
  3 = bottom left back
  4 = top left front
  5 = top right front
  6 = top right back
  7 = top left back

  Faces (CW, facing outside) - so that texcoo(0,0 - 1,1) are mapped directly (for a cube) :
  PositiveX = 5,6,2,1
  NegativeX = 7,3,0,4
  PositiveY = 7,6,5,4
  NegativeY = 0,1,2,3
  PositiveZ = 6,7,3,2
  NegativeZ = 4,5,1,0

  Faces (CCW, facing inside) - so that texcoo(0,0 - 1,1) are mapped directly (for a cubemap) :
  PositiveX = 6,5,1,2
  NegativeX = 4,7,3,0
  PositiveY = 4,5,6,7
  NegativeY = 3,2,1,0
  PositiveZ = 7,6,2,3
  NegativeZ = 5,4,0,1

  </pre>
*/
template <class T>
class cAABB
{
 public:
  cAABB() {
    ZeroMembers();
  }

  cAABB(const sVec3<T>& min, const sVec3<T>& max) {
    ZeroMembers();
    Set(min, max);
  }

  cAABB(const sVec3<T>& center, T afRadius) {
    ZeroMembers();
    SetCenterExtents(center, afRadius);
  }

  ~cAABB() {
  }

  void ZeroMembers() {
    mvMin.Set( 99999,  99999,  99999);
    mvMax.Set(-99999, -99999, -99999);
  }

  const sVec3<T>& GetMin() const {
    return mvMin;
  }
  const sVec3<T>& GetMax() const {
    return mvMax;
  }

  const T GetXSize() const {
    return mvMax.x-mvMin.x;
  }
  const T GetYSize() const {
    return mvMax.y-mvMin.y;
  }
  const T GetZSize() const {
    return mvMax.z-mvMin.z;
  }
  const T GetMaxSize() const {
    return Max(GetXSize(), Max(GetYSize(), GetZSize()));
  }
  const T GetRadius() const {
    return GetMaxSize()/(T)2;
  }

  sVec3<T> GetSize() const {
    return mvMax - mvMin;
  }
  sVec3<T> GetExtents() const {
    return (mvMax - mvMin)/T(2);
  }
  sVec3<T> GetCenter() const {
    return (mvMax - mvMin)/T(2) + mvMin;
  }
  T GetAverageExtent() const {
    return (GetXSize()+GetYSize()+GetZSize())/3;
  }

  void Set(const sVec3<T>& min, const sVec3<T>& max) {
    mvMin = min;
    mvMax = max;
  }
  void SetMin(const sVec3<T>& min) {
    mvMin = min;
  }
  void SetMax(const sVec3<T>& max) {
    mvMax = max;
  }
  void SetCenterSize(const sVec3<T>& center, T size) {
    SetCenterExtents(center, size/T(2));
  }
  void SetCenterSize(const sVec3<T>& center, const sVec3<T>& size) {
    SetCenterExtents(center, size/T(2));
  }
  void SetCenterExtents(const sVec3<T>& center, T size) {
    mvMin.x = center.x - size;
    mvMin.y = center.y - size;
    mvMin.z = center.z - size;
    mvMax.x = center.x + size;
    mvMax.y = center.y + size;
    mvMax.z = center.z + size;
  }
  void SetCenterExtents(const sVec3<T>& center, const sVec3<T>& size) {
    mvMin = center - size;
    mvMax = center + size;
  }

  tBool Intersect(const sVec3<T>& p, T epsilon = niEpsilon5) const {
    if (p.x < mvMin.x-epsilon)  return eFalse;
    if (p.y < mvMin.y-epsilon)  return eFalse;
    if (p.z < mvMin.z-epsilon)  return eFalse;
    if (p.x > mvMax.x+epsilon) return eFalse;
    if (p.y > mvMax.y+epsilon) return eFalse;
    if (p.z > mvMax.z+epsilon) return eFalse;
    return eTrue;
  }
  tBool Intersect(const cAABB<T>& aabb, T epsilon = niEpsilon5) const {
    return Intersect(GetMin(),GetMax(),aabb.GetMin(),aabb.GetMax(),epsilon);
  }
  tBool Intersect(const sVec3<T>& min, const sVec3<T>& max, T epsilon = niEpsilon5) const {
    return Intersect(GetMin(),GetMax(),min,max,epsilon);
  }
  static tBool Intersect(const sVec3<T>& mina, const sVec3<T>& maxa,
                         const sVec3<T>& minb, const sVec3<T>& maxb,
                         T epsilon = niEpsilon5) {
    if (maxa.x+epsilon < minb.x-epsilon || mina.x-epsilon > maxb.x+epsilon)
      return eFalse;

    if (maxa.y+epsilon < minb.y-epsilon || mina.y-epsilon > maxb.y+epsilon)
      return eFalse;

    if (maxa.z+epsilon < minb.z-epsilon || mina.z-epsilon > maxb.z+epsilon)
      return eFalse;

    return eTrue;
  }
  tBool IntersectRay(const sVec3<T>& avRayPos, const sVec3<T>& avRayDir, sVec3<T>* pOut = NULL, T epsilon = niEpsilon5) const {
    tBool bInside = eTrue;
    sVec3<T> vMaxT = Vec3<T>(T(-1),T(-1),T(-1));
    sVec3<T> vOut = sVec3<T>::Zero();
    tU32 i;

    // Find candidate planes.
    for (i = 0; i < 3; ++i)
    {
      T fPos = avRayPos[i];
      T fDir = avRayDir[i];
      if (fPos < mvMin[i])
      {
        vOut[i] = mvMin[i];
        bInside = eFalse;

        // Calculate T distances to candidate planes
        if (fDir != T(0))
          vMaxT[i] = (mvMin[i] - fPos) / fDir;
      }
      else if (fPos > mvMax[i])
      {
        vOut[i] = mvMax[i];
        bInside = eFalse;

        // Calculate T distances to candidate planes
        if (fDir != T(0))
          vMaxT[i] = (mvMax[i] - fPos) / fDir;
      }
    }

    // Ray origin inside bounding box
    if (bInside)
    {
      if (pOut) *pOut = avRayPos;
      return eTrue;
    }

    // Get largest of the maxT's for final choice of intersection
    tU32 WhichPlane = 0;
    if (vMaxT.y > vMaxT[WhichPlane])  WhichPlane = 1;
    if (vMaxT.z > vMaxT[WhichPlane])  WhichPlane = 2;

    // Check final candidate actually inside box
    if (vMaxT[WhichPlane] < T(0))
      return eFalse;

    for (i = 0; i < 3; ++i)
    {
      if (i != WhichPlane)
      {
        vOut[i] = avRayPos[i] + vMaxT[WhichPlane] * avRayDir[i];
        if (vOut[i] < mvMin[i] - epsilon ||
            vOut[i] > mvMax[i] + epsilon)
          return eFalse;
      }
    }

    if (pOut) *pOut = vOut;
    return eTrue; // ray hits box
  }

  //! Bounding box computing helper
  void Reset() {
    ZeroMembers();
  }
  void SetPoint(const sVec3<T>& p) {
    if (p.x < mvMin.x)  mvMin.x = p.x;
    if (p.x > mvMax.x)  mvMax.x = p.x;

    if (p.y < mvMin.y)  mvMin.y = p.y;
    if (p.y > mvMax.y)  mvMax.y = p.y;

    if (p.z < mvMin.z)  mvMin.z = p.z;
    if (p.z > mvMax.z)  mvMax.z = p.z;
  }

  //! Inflate the bounding box of the given percent (1.0 == 100%).
  void Inflate(T percent) {
    T fSzX = GetXSize();
    T fSzY = GetYSize();
    T fSzZ = GetZSize();

    mvMin.x = mvMin.x - (fSzX*percent);
    mvMin.y = mvMin.y - (fSzY*percent);
    mvMin.z = mvMin.z - (fSzZ*percent);

    mvMax.x = mvMax.x + (fSzX*percent);
    mvMax.y = mvMax.y + (fSzY*percent);
    mvMax.z = mvMax.z + (fSzZ*percent);
  }

  //! Extend the bounding box of the given vector.
  void Extend(const sVec3<T>& v) {
    mvMin -= v;
    mvMax += v;
  }

  //! Translate the AABB.
  void Translate(const sVec3<T>& vVec) {
    mvMin += vVec;
    mvMax += vVec;
  }

  //! Rotate/Scale the AABB.
  void Rotate(const sMatrix<T>& aMtx) {
    sVec3<T> verts[8];
    GetVertices((tPtr)verts,0,sizeof(verts[0]));
    Reset();
    niLoop(i,niCountOf(verts)) {
      VecTransformNormal(verts[i],verts[i],aMtx);
      SetPoint(verts[i]);
    }

#if 0
    // The three edges transformed: you can efficiently transform an X-only vector
    // by just getting the "X" column of the matrix
    sVec3<T> vx = *((sVec3<T>*)&aMtx._11) * (mvMax.x - mvMin.x);
    sVec3<T> vy = *((sVec3<T>*)&aMtx._21) * (mvMax.y - mvMin.y);
    sVec3<T> vz = *((sVec3<T>*)&aMtx._31) * (mvMax.z - mvMin.z);

    // Transform the min point
    VecTransformNormal(mvMin, mvMin, aMtx);
    mvMax = mvMin;

    // Take the transformed min & axes and find new extents
    // Using CPU code in the right place is faster...
    if (vx.x < T(0))  mvMin.x += vx.x; else mvMax.x += vx.x;
    if (vx.y < T(0))  mvMin.y += vx.y; else mvMax.y += vx.y;
    if (vx.z < T(0))  mvMin.z += vx.z; else mvMax.z += vx.z;
    if (vy.x < T(0))  mvMin.x += vy.x; else mvMax.x += vy.x;
    if (vy.y < T(0))  mvMin.y += vy.y; else mvMax.y += vy.y;
    if (vy.z < T(0))  mvMin.z += vy.z; else mvMax.z += vy.z;
    if (vz.x < T(0))  mvMin.x += vz.x; else mvMax.x += vz.x;
    if (vz.y < T(0))  mvMin.y += vz.y; else mvMax.y += vz.y;
    if (vz.z < T(0))  mvMin.z += vz.z; else mvMax.z += vz.z;
#endif
  }

  //! Transform the AABB.
  void Transform(const sMatrix<T>& aMtx) {
    // sVec3<T> temp;
    // Rotate(aMtx);
    // Translate(MatrixGetTranslation(temp,aMtx));
    sVec3<T> verts[8];
    GetVertices((tPtr)verts,0,sizeof(verts[0]));
    Reset();
    niLoop(i,niCountOf(verts)) {
      VecTransformCoord(verts[i],verts[i],aMtx);
      SetPoint(verts[i]);
    }
  }

  //! Get the AABB vertices.
  //! \remark The array needs to have room for 8 elements.
  void GetVertices(tPtr pVerts, tU32 aulPosOffset, tU32 aulStride) const {
    sVec3<T>* pV;
    // 0: bottom left front
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*0));
    pV->x = mvMin.x;
    pV->y = mvMin.y;
    pV->z = mvMin.z;
    // 1: bottom right front
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*1));
    pV->x = mvMax.x;
    pV->y = mvMin.y;
    pV->z = mvMin.z;
    // 2: bottom right back
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*2));
    pV->x = mvMax.x;
    pV->y = mvMin.y;
    pV->z = mvMax.z;
    // 3: bottom left back
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*3));
    pV->x = mvMin.x;
    pV->y = mvMin.y;
    pV->z = mvMax.z;
    // 4: top left front
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*4));
    pV->x = mvMin.x;
    pV->y = mvMax.y;
    pV->z = mvMin.z;
    // 5: top right front
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*5));
    pV->x = mvMax.x;
    pV->y = mvMax.y;
    pV->z = mvMin.z;
    // 6: top right back
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*6));
    pV->x = mvMax.x;
    pV->y = mvMax.y;
    pV->z = mvMax.z;
    // 7: top left back
    pV = (sVec3<T>*)(pVerts+aulPosOffset+(aulStride*7));
    pV->x = mvMin.x;
    pV->y = mvMax.y;
    pV->z = mvMax.z;
  }

  //! Get triangles indices.
  //! \remark The array needs to have room for 36 elements.
  static void GetTrianglesIndices(tU32* pIndices, bool bCW) {
    //
    // Note: The MSVC 19 optimizer used to deadlockdeadlocks when compiling
    // the previous version of GetTrianglesIndices that was using *pIndices++
    // instead of the explicit array positions.
    //
    // See: https://godbolt.org/z/o8rWsWsbb &
    // https://gist.github.com/prenaux/0fd3157abbf15e4cad6db733d0516bac
    //
    if (bCW) {
      pIndices[0] = 5; pIndices[1] = 6; pIndices[2] = 1;
      pIndices[3] = 6; pIndices[4] = 2; pIndices[5] = 1;
      pIndices[6] = 7; pIndices[7] = 4; pIndices[8] = 3;
      pIndices[9] = 3; pIndices[10] = 4; pIndices[11] = 0;
      pIndices[12] = 7; pIndices[13] = 6; pIndices[14] = 4;
      pIndices[15] = 6; pIndices[16] = 5; pIndices[17] = 4;
      pIndices[18] = 0; pIndices[19] = 1; pIndices[20] = 3;
      pIndices[21] = 1; pIndices[22] = 2; pIndices[23] = 3;
      pIndices[24] = 6; pIndices[25] = 7; pIndices[26] = 2;
      pIndices[27] = 7; pIndices[28] = 3; pIndices[29] = 2;
      pIndices[30] = 4; pIndices[31] = 5; pIndices[32] = 0;
      pIndices[33] = 5; pIndices[34] = 1; pIndices[35] = 0;
    }
    else {
      pIndices[0] = 5; pIndices[1] = 1; pIndices[2] = 6;
      pIndices[3] = 6; pIndices[4] = 1; pIndices[5] = 2;
      pIndices[6] = 7; pIndices[7] = 3; pIndices[8] = 4;
      pIndices[9] = 3; pIndices[10] = 0; pIndices[11] = 4;
      pIndices[12] = 7; pIndices[13] = 4; pIndices[14] = 6;
      pIndices[15] = 6; pIndices[16] = 4; pIndices[17] = 5;
      pIndices[18] = 0; pIndices[19] = 3; pIndices[20] = 1;
      pIndices[21] = 1; pIndices[22] = 3; pIndices[23] = 2;
      pIndices[24] = 6; pIndices[25] = 2; pIndices[26] = 7;
      pIndices[27] = 7; pIndices[28] = 2; pIndices[29] = 3;
      pIndices[30] = 4; pIndices[31] = 0; pIndices[32] = 5;
      pIndices[33] = 5; pIndices[34] = 0; pIndices[35] = 1;
    }
  }

  //! Get lines indices. To draw the AABB borders by example.
  //! \remark The array needs to have room for 24 elements.
  static void GetLinesIndices(tU32* pIndices) {
    *pIndices++ = 0; *pIndices++ = 1;
    *pIndices++ = 1; *pIndices++ = 2;
    *pIndices++ = 2; *pIndices++ = 3;
    *pIndices++ = 3; *pIndices++ = 0;

    *pIndices++ = 0; *pIndices++ = 4;
    *pIndices++ = 1; *pIndices++ = 5;
    *pIndices++ = 2; *pIndices++ = 6;
    *pIndices++ = 3; *pIndices++ = 7;

    *pIndices++ = 4; *pIndices++ = 5;
    *pIndices++ = 5; *pIndices++ = 6;
    *pIndices++ = 6; *pIndices++ = 7;
    *pIndices++ = 7; *pIndices++ = 4;
  }

  //! Get the AABB faces planes.
  //! \remark The array needs to have room for 6 elements.
  void GetFacesPlanes(sVec4<T>* pPlanes, bool bOutward) const {
    sVec3<T> vVerts[8];
    GetVertices(tPtr(vVerts),0,sizeof(vVerts[0]));

    if (bOutward) {
      PlaneFromPoints(pPlanes[0], vVerts[1], vVerts[5], vVerts[6]);
      PlaneFromPoints(pPlanes[1], vVerts[0], vVerts[3], vVerts[7]);
      PlaneFromPoints(pPlanes[2], vVerts[5], vVerts[4], vVerts[7]);
      PlaneFromPoints(pPlanes[3], vVerts[0], vVerts[1], vVerts[2]);
      PlaneFromPoints(pPlanes[4], vVerts[2], vVerts[6], vVerts[7]);
      PlaneFromPoints(pPlanes[5], vVerts[0], vVerts[4], vVerts[5]);
    }
    else {
      PlaneFromPoints(pPlanes[0], vVerts[2], vVerts[6], vVerts[5]);
      PlaneFromPoints(pPlanes[1], vVerts[4], vVerts[7], vVerts[3]);
      PlaneFromPoints(pPlanes[2], vVerts[6], vVerts[7], vVerts[4]);
      PlaneFromPoints(pPlanes[3], vVerts[3], vVerts[2], vVerts[1]);
      PlaneFromPoints(pPlanes[4], vVerts[3], vVerts[7], vVerts[6]);
      PlaneFromPoints(pPlanes[5], vVerts[1], vVerts[5], vVerts[4]);
    }

    for (tU32 i = 0; i < 6; ++i)
      PlaneNormalize(pPlanes[i]);
  }

 protected:
  sVec3<T> mvMin;
  sVec3<T> mvMax;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Float AABB.
typedef cAABB<tF32> cAABBf;
//! Double AABB.
typedef cAABB<tF64> cAABBd;

//////////////////////////////////////////////////////////////////////////////////////////////
class cBoundingVolumeAABB : public cIUnknownImpl<iBoundingVolume>, public cAABBf
{
 public:
  cBoundingVolumeAABB() {
  }
  ~cBoundingVolumeAABB() {
  }

  //// iBoundingVolume //////////////////////////
  tBool __stdcall Copy(iBoundingVolume* apSrc) {
    SetMin(apSrc->GetMin());
    SetMax(apSrc->GetMax());
    return eTrue;
  }
  iBoundingVolume* __stdcall Clone() const {
    cBoundingVolumeAABB* pNew = niNew cBoundingVolumeAABB();
    pNew->mvMin = this->mvMin;
    pNew->mvMax = this->mvMax;
    return pNew;
  }
  eBoundingVolumeType __stdcall GetType() const {
    return eBoundingVolumeType_AABB;
  }
  tBool __stdcall Begin(tBool abReset) {
    if (abReset) cAABBf::Reset();
    return eTrue;
  }
  tBool __stdcall End() {
    return eTrue;
  }
  tBool __stdcall AddPoint(const sVec3f& avPoint) {
    cAABBf::SetPoint(avPoint);
    return eTrue;
  }
  tBool __stdcall SetCenter(const sVec3f& avPos) {
    sVec3f vExtents = GetExtents();
    cAABBf::SetCenterExtents(avPos,vExtents);
    return eTrue;
  }
  sVec3f __stdcall GetCenter() const {
    return cAABBf::GetCenter();
  }
  void __stdcall SetRadius(tF32 afRadius) {
    cAABBf::SetCenterExtents(GetCenter(),afRadius);
  }
  tF32 __stdcall GetRadius() const {
    return cAABBf::GetMaxSize()*0.5f;
  }
  void __stdcall SetExtents(const sVec3f& avExtends) {
    cAABBf::SetCenterExtents(GetCenter(),avExtends);
  }
  sVec3f __stdcall GetExtents() const {
    return cAABBf::GetExtents();
  }
  void __stdcall SetSize(const sVec3f& avSize) {
    cAABBf::SetCenterSize(GetCenter(),avSize);
  }
  sVec3f __stdcall GetSize() const {
    return cAABBf::GetSize();
  }
  void __stdcall SetMin(const sVec3f& avMin) {
    cAABBf::SetMin(avMin);
  }
  sVec3f __stdcall GetMin() const {
    return cAABBf::GetMin();
  }
  void __stdcall SetMax(const sVec3f& avMax) {
    cAABBf::SetMax(avMax);
  }
  sVec3f __stdcall GetMax() const {
    return cAABBf::GetMax();
  }
  tBool __stdcall Translate(const sVec3f& avV) {
    cAABBf::Translate(avV);
    return eTrue;
  }
  tBool __stdcall Rotate(const sMatrixf& amtxRotation) {
    cAABBf::Rotate(amtxRotation);
    return eTrue;
  }
  tBool __stdcall Transform(const sMatrixf& aMatrix) {
    cAABBf::Transform(aMatrix);
    return eTrue;
  }
  void __stdcall Inflate(tF32 afPercent) {
    cAABBf::Inflate(afPercent);
  }
  eIntersectionResult __stdcall IntersectAABB(iIntersection* apResult, const sVec3f& avMin, const sVec3f& avMax) const {
    return cAABBf::Intersect(avMin,avMax)?eIntersectionResult_Intersect:eIntersectionResult_None;
  }
  eIntersectionResult __stdcall IntersectRay(iIntersection* apResult, const sVec3f& avPos, const sVec3f& avDir) const {
    eIntersectionResult res = eIntersectionResult_None;
    sVec3f vPos;
    if (cAABBf::IntersectRay(avPos,avDir,&vPos)) {
      res = eIntersectionResult_Intersect;
    }
    if (apResult) {
      apResult->SetResult(res);
      apResult->SetPosition(vPos);
    }
    return res;
  }
  eIntersectionResult __stdcall IntersectPoint(iIntersection* apResult, const sVec3f& aPosition) const {
    eIntersectionResult res = eIntersectionResult_None;
    if (cAABBf::Intersect(aPosition))
    {
      res = eIntersectionResult_Intersect;
    }
    if (apResult) {
      apResult->SetResult(res);
      apResult->SetPosition(aPosition);
    }
    return res;
  }
  eIntersectionResult __stdcall IntersectFrustum(iIntersection* apResult, const iFrustum* apFrustum) const {
    eIntersectionResult res = apFrustum ?
        (eIntersectionResult)apFrustum->CullAABB(mvMin,mvMax) :
        eIntersectionResult_Inside; // no frustum, set as 'inside'
    if (apResult) {
      apResult->SetResult(res);
    }
    return res;
  }
  //// iBoundingVolume //////////////////////////
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __AABB_17223519_H__
