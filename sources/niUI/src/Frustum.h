#ifndef __FRUSTUM_11925949_H__
#define __FRUSTUM_11925949_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

template <typename T>
void RectBBReset(sRect<T>& aRect, T aLimit) {
  aRect.Left() = aLimit;
  aRect.Right() = -aLimit;
  aRect.Top() = aLimit;
  aRect.Bottom() = -aLimit;
}

template <typename T>
void RectBBAddPoint(sRect<T>& aRect, const sVec2<T>& v) {
  if (v.x < aRect.Left()) {
    aRect.Left() = v.x;
  }
  else if (v.x > aRect.Right()) {
    aRect.Right() = v.x;
  }
  if (v.y < aRect.Top()) {
    aRect.Top() = v.y;
  }
  else if (v.y > aRect.Bottom()) {
    aRect.Bottom() = v.y;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cFrustum declaration
template <typename T, typename BASE = cUnknown0>
class cFrustumImpl : public BASE
{
  typedef astl::vector< sVec4<T> > tPlaneVec;
  typedef niTypename tPlaneVec::iterator      tPlaneVecIt;
  typedef niTypename tPlaneVec::const_iterator  tPlaneVecCIt;

 public:
  cFrustumImpl();
  ~cFrustumImpl();

  void ZeroMembers();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  iFrustum* __stdcall Clone() const;
  void __stdcall Copy(const iFrustum* apSrc);

  sVec3<T> __stdcall GetForward() const;
  sVec3<T> __stdcall GetUp() const;
  sVec3<T> __stdcall GetRight() const;

  void __stdcall ExtractPlanes(const sMatrix<T>& mtxVP);

  //! Set the number of plane in the frustum.
  void __stdcall SetNumPlanes(tU32 ulNumPlane);
  //! Get the number of planes in the frustum.
  tU32 __stdcall GetNumPlanes() const;
  //! Add the given number of planes to the frustum.
  void __stdcall AddPlanes(tU32 aulNumPlane, const sVec4<T>* apPlanes);
  //! Add one plane to the frustum.
  void __stdcall AddPlane(const sVec4<T>& aPlane);
  //! Set all planes of the frustum.
  void __stdcall SetPlanes(tU32 aulNumPlane, const sVec4<T>* apPlanes);
  //! Set the plane of the given index.
  void __stdcall SetPlane(tU32 ulIdx, const sVec4<T>& Plane);
  //! Get the plane of the given index.
  sVec4<T> __stdcall GetPlane(tU32 ulIdx) const;
  //! Get all planes.
  sVec4<T>* __stdcall GetPlanes() const;
  //! Cull an AABB.
  eCullCode __stdcall CullAABB(const sVec3f& avMin, const sVec3f& avMax) const;
  //! Check if the given AABB is in the frustum.
  tBool __stdcall IntersectAABB(const sVec3f& avMin, const sVec3f& avMax) const;
  //! Cull a sphere.
  eCullCode __stdcall CullSphere(const sVec3f& avCenter, tF32 afRadius) const;
  //! Check if the given sphere is in the frustum.
  tBool __stdcall IntersectSphere(const sVec3f& avCenter, tF32 afRadius) const;
  //! Transform the frustum by the given matrix.
  tBool __stdcall Transform(const sMatrix<T>& M);

  sRectf __stdcall ComputeScreenBoundingBox(const sMatrixf& amtxWVP, const sRectf& aViewport) {
    sVec3f verts[8];
    sRectf rect = sRectf::Null();
    if (ExtractVertices(verts)) {
      RectBBReset(rect,1000000.0f);
      sVec4f v;
      niLoop(i,8) {
        VecProjectRHW(v,verts[i],amtxWVP,aViewport);
        RectBBAddPoint(rect,Vec2(v.x,v.y));
      }
    }
    return rect;
  }

  tBool __stdcall SetBoundingVolume(iBoundingVolume* apBV) {
    mptrBV = niGetIfOK(apBV);
    UpdateBoundingVolume();
    return mptrBV.IsOK();
  }
  iBoundingVolume* __stdcall GetBoundingVolume() const {
    return mptrBV;
  }

  tBool ExtractVertices(sVec3f verts[8]) {
    if (mvPlanes.size() < 6) return eFalse;
    PlaneIntersection(verts[0], mvPlanes[eFrustumPlane_Near], mvPlanes[eFrustumPlane_Left], mvPlanes[eFrustumPlane_Top]);
    PlaneIntersection(verts[1], mvPlanes[eFrustumPlane_Near], mvPlanes[eFrustumPlane_Right], mvPlanes[eFrustumPlane_Top]);
    PlaneIntersection(verts[2], mvPlanes[eFrustumPlane_Near], mvPlanes[eFrustumPlane_Right], mvPlanes[eFrustumPlane_Bottom]);
    PlaneIntersection(verts[3], mvPlanes[eFrustumPlane_Near], mvPlanes[eFrustumPlane_Left], mvPlanes[eFrustumPlane_Bottom]);
    PlaneIntersection(verts[4], mvPlanes[eFrustumPlane_Far], mvPlanes[eFrustumPlane_Left], mvPlanes[eFrustumPlane_Top]);
    PlaneIntersection(verts[5], mvPlanes[eFrustumPlane_Far], mvPlanes[eFrustumPlane_Right], mvPlanes[eFrustumPlane_Top]);
    PlaneIntersection(verts[6], mvPlanes[eFrustumPlane_Far], mvPlanes[eFrustumPlane_Right], mvPlanes[eFrustumPlane_Bottom]);
    PlaneIntersection(verts[7], mvPlanes[eFrustumPlane_Far], mvPlanes[eFrustumPlane_Left], mvPlanes[eFrustumPlane_Bottom]);
    return eTrue;
  }

  void UpdateBoundingVolume() {
    if (!mptrBV.IsOK()) return;
    sVec3f verts[8];
    ExtractVertices(verts);
    mptrBV->Begin(eTrue);
    niLoop(i,8) {
      mptrBV->AddPoint(verts[i]);
    }
    mptrBV->End();
  }

 private:
  astl::vector<sVec4<T> >  mvPlanes;
  Ptr<iBoundingVolume>  mptrBV;
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Float frustum.
typedef cFrustumImpl<tF32,ImplRC<iFrustum> > cFrustumf;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
} // End of namespace ni
#endif // __FRUSTUM_11925949_H__
