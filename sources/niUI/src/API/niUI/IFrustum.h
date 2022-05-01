#ifndef __IFRUSTUM_45645949_H__
#define __IFRUSTUM_45645949_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IBoundingVolume.h"

namespace ni {

//! Frustum planes.
enum eFrustumPlane
{
  //! Left frustum plane.
  eFrustumPlane_Left    = 0,
  //! Right frustum plane.
  eFrustumPlane_Right   = 1,
  //! Top frustum plane.
  eFrustumPlane_Top   = 2,
  //! Bottom frustum plane.
  eFrustumPlane_Bottom  = 3,
  //! Near frustum plane.
  eFrustumPlane_Near    = 4,
  //! Far frustum plane.
  eFrustumPlane_Far   = 5,
  //! \internal
  eFrustumPlane_ForceDWORD = 0xFFFFFFFF
};

//! Value return by the culling functions.
enum eCullCode
{
  eCullCode_Out   = 0,    // completely outside the frustum
  eCullCode_In    = 1,    // completely inside the frustum
  eCullCode_Intersect = 2   // partially visible
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Frustum interface
struct iFrustum : public iUnknown
{
  niDeclareInterfaceUUID(iFrustum,0x6cfe2b98,0xb883,0x4344,0xaa,0x1c,0xa7,0xbc,0x56,0x81,0x7d,0x64)

  //! Create a copy of this object.
  virtual iFrustum* __stdcall Clone() const = 0;
  //! Copy the given frustum.
  virtual void __stdcall Copy(const iFrustum* apSrc) = 0;
  //! Get the forward vector of the frustum.
  //! \remark The forward vector of the frustum is the normal of the near plane.
  //! {Property}
  virtual sVec3f __stdcall GetForward() const = 0;
  //! Get the up vector of the frustum.
  //! \remark The up vector of the frustum is the cross product between the left and right frustum planes.
  //! {Property}
  virtual sVec3f __stdcall GetUp() const = 0;
  //! Get the right vector of the frustum.
  //! \remark The right vector of the frustum is the cross product between the top and bottom frustum planes.
  //! {Property}
  virtual sVec3f __stdcall GetRight() const = 0;
  //! Extract the planes from a view projection matrix.
  virtual void __stdcall ExtractPlanes(const sMatrixf& mtxVP) = 0;
  //! Set the number of plane in the frustum.
  //! {Property}
  virtual void __stdcall SetNumPlanes(tU32 ulNumPlane) = 0;
  //! Get the number of planes in the frustum.
  //! {Property}
  virtual tU32 __stdcall GetNumPlanes() const = 0;
  //! Add the given number of planes to the frustum.
  //! {NoAutomation}
  virtual void __stdcall AddPlanes(tU32 aulNumPlane, const sPlanef* apPlanes) = 0;
  //! Add one plane to the frustum.
  virtual void __stdcall AddPlane(const sPlanef& aPlane) = 0;
  //! Set all planes of the frustum.
  //! {NoAutomation}
  virtual void __stdcall SetPlanes(tU32 aulNumPlane, const sPlanef* apPlanes) = 0;
  //! Set the plane of the given index.
  //! {Property}
  virtual void __stdcall SetPlane(tU32 ulIdx, const sPlanef& Plane) = 0;
  //! Get the plane of the given index.
  //! {Property}
  virtual sPlanef __stdcall GetPlane(tU32 ulIdx) const = 0;
  //! Get all planes.
  //! {NoAutomation}
  virtual sPlanef* __stdcall GetPlanes() const = 0;
  //! Cull an AABB.
  virtual eCullCode __stdcall CullAABB(const sVec3f& avMin, const sVec3f& avMax) const = 0;
  //! Check if the given AABB is in the frustum.
  virtual tBool __stdcall IntersectAABB(const sVec3f& avMin, const sVec3f& avMax) const = 0;
  //! Cull a sphere.
  virtual eCullCode __stdcall CullSphere(const sVec3f& avCenter, tF32 afRadius) const = 0;
  //! Check if the given sphere is in the frustum.
  virtual tBool __stdcall IntersectSphere(const sVec3f& avCenter, tF32 afRadius) const = 0;
  //! Transform the frustum by the given matrix.
  virtual tBool __stdcall Transform(const sMatrixf& M) = 0;
  //! Compute the screen bounding box.
  virtual sRectf __stdcall ComputeScreenBoundingBox(const sMatrixf& amtxWVP, const sRectf& aViewport) = 0;
  //! Set the world bounding volume.
  //! {Property}
  virtual tBool __stdcall SetBoundingVolume(iBoundingVolume* apBV) = 0;
  //! Get the world bounding volume.
  //! {Property}
  virtual iBoundingVolume* __stdcall GetBoundingVolume() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
} // End of namespace ni
#endif // __IFRUSTUM_45645949_H__
