#ifndef __IBOUNDINGVOLUME_2803289_H__
#define __IBOUNDINGVOLUME_2803289_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IIntersection.h"

namespace ni {

template <class T> class cAABB;
template <class T> class cFrustum;
typedef cAABB<tF32>     cAABBf;

struct iFrustum;
struct iBoundingVolume;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Bounding volume type.
enum eBoundingVolumeType
{
  //! AABB volume type.
  eBoundingVolumeType_AABB = 0,
  //! Sphere volume type.
  eBoundingVolumeType_Sphere = 1,
  //! Convex hull volume type.
  eBoundingVolumeType_ConvexHull = 2,
  //! User volume type
  eBoundingVolumeType_User = 2,
  //! \internal
  eBoundingVolumeType_ForceDWORD = 0xFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Bounding volume interface.
struct iBoundingVolume : public iUnknown
{
  niDeclareInterfaceUUID(iBoundingVolume,0x489ebd21,0x5b90,0x46db,0xa8,0x57,0x5c,0x6e,0x8d,0x3b,0x89,0xba)

  //! Copy another bounding volume in this volume.
  virtual tBool __stdcall Copy(iBoundingVolume* apSrc) = 0;
  //! Create a copy of the bounding volume.
  virtual iBoundingVolume* __stdcall Clone() const = 0;

  //! Get the bounding volume type.
  //! {Property}
  virtual eBoundingVolumeType __stdcall GetType() const = 0;

  //########################################################################################
  //! \name Computation
  //########################################################################################
  //! @{

  //! Begin the computation of the bounding volume.
  virtual tBool __stdcall Begin(tBool abReset) = 0;
  //! End the computation of the bounding volume.
  virtual tBool __stdcall End() = 0;
  //! Add a point inside the bounding volume.
  virtual tBool __stdcall AddPoint(const sVec3f& avPoint) = 0;
  //! @}

  //########################################################################################
  //! \name Transformations
  //########################################################################################
  //! @{

  //! Set the center.
  //! {Property}
  virtual tBool __stdcall SetCenter(const sVec3f& avPos) = 0;
  //! Get the center.
  //! {Property}
  virtual sVec3f __stdcall GetCenter() const = 0;
  //! Set the radius.
  //! {Property}
  virtual void __stdcall SetRadius(tF32 afRadius) = 0;
  //! Get the radius.
  //! {Property}
  virtual tF32 __stdcall GetRadius() const = 0;
  //! Set the extents.
  //! {Property}
  virtual void __stdcall SetExtents(const sVec3f& avExtends) = 0;
  //! Get the extents.
  //! {Property}
  virtual sVec3f __stdcall GetExtents() const = 0;
  //! Set the size.
  //! {Property}
  virtual void __stdcall SetSize(const sVec3f& avSize) = 0;
  //! Get the size.
  //! {Property}
  virtual sVec3f __stdcall GetSize() const = 0;
  //! Set the minimum value.
  //! {Property}
  virtual void __stdcall SetMin(const sVec3f& avMin) = 0;
  //! Get the minimum value.
  //! {Property}
  virtual sVec3f __stdcall GetMin() const = 0;
  //! Set the maximum value.
  //! {Property}
  virtual void __stdcall SetMax(const sVec3f& avMax) = 0;
  //! Get the maximum value.
  //! {Property}
  virtual sVec3f __stdcall GetMax() const = 0;
  //! Translate the bounding volume.
  virtual tBool __stdcall Translate(const sVec3f& avV) = 0;
  //! Rotate the bounding volume.
  virtual tBool __stdcall Rotate(const sMatrixf& amtxRotation) = 0;
  //! Transform by the specified matrix.
  virtual tBool __stdcall Transform(const sMatrixf& aMatrix) = 0;
  //! Inflate the bounding volume by the given percentage.
  virtual void __stdcall Inflate(tF32 afPercent) = 0;
  //! @}

  //########################################################################################
  //! \name Intersection tests
  //########################################################################################
  //! @{

  //! Intersect with an AABB.
  virtual eIntersectionResult __stdcall IntersectAABB(iIntersection* apResult, const sVec3f& avMin, const sVec3f& avMax) const = 0;
  //! Intersect with a ray.
  virtual eIntersectionResult __stdcall IntersectRay(iIntersection* apResult, const sVec3f& avOrg, const sVec3f& avDir) const = 0;
  //! Intersect with a point.
  virtual eIntersectionResult __stdcall IntersectPoint(iIntersection* apResult, const sVec3f& aPosition) const = 0;
  //! Intersect with a frustum.
  virtual eIntersectionResult __stdcall IntersectFrustum(iIntersection* apResult, const iFrustum* apFrustum) const = 0;
  //! @}
};

//! Bounding volume smart pointer.
typedef Ptr<iBoundingVolume>  tBoundingVolumePtr;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IBOUNDINGVOLUME_2803289_H__
