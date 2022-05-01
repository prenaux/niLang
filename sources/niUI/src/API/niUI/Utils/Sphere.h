#ifndef __SPHERE_32687073_H__
#define __SPHERE_32687073_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Sphere template class.
template <typename T>
class cSphere
{
 public:
  cSphere(const sVec3<T>& aPos, T aRadius);
  cSphere(const sVec3<T>& aPos);
  cSphere(T aRadius = 1.0f);

  //########################################################################################
  //! \name Properties
  //########################################################################################
  //! @{

  //! Set the sphere radius.
  void SetRadius(T aRadius);
  //! Get the sphere radius.
  T GetRadius() const;
  //! Set the sphere position.
  void SetPosition(const sVec3<T>& aPos);
  //! Get the sphere position.
  const sVec3<T>& GetPosition() const;
  //! Set the sphere center (same as position).
  inline void SetCenter(const sVec3<T>& aPos) { SetPosition(aPos); }
  //! Get the sphere center (same as position).
  inline const sVec3<T>& GetCenter() const { return GetPosition(); }
  //! @}

  //########################################################################################
  //! \name Intersection tests
  //########################################################################################
  //! @{

  //! Intersection with a point.
  tBool Intersect(const sVec3<T>& vPos) const;
  //! Intersection with a ray.
  //! \return the intersection distance between a Ray and a sphere. If no intersection
  //!     return -1.
  T Intersect(const sVec3<T>& avPos, const sVec3<T>& avDir) const;
  //! @}

  void Transform(const sMatrixf& avMatrix);

 protected:
  sVec3<T> mvPosition;
  T     mRadius;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Float Sphere.
typedef cSphere<tF32> cSpheref;
//! Double Sphere.
typedef cSphere<tF64> cSphered;

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
template <typename T>
cSphere<T>::cSphere(const sVec3<T>& aPos, T aRadius)
{
  SetPosition(aPos);
  SetRadius(aRadius);
}

///////////////////////////////////////////////
template <typename T>
cSphere<T>::cSphere(const sVec3<T>& aPos)
{
  SetPosition(aPos);
  SetRadius(1.0f);
}

///////////////////////////////////////////////
template <typename T>
cSphere<T>::cSphere(T aRadius)
{
  SetPosition(sVec3<T>::One());
  SetRadius(aRadius);
}

///////////////////////////////////////////////
//! Set the sphere radius.
template <typename T>
void cSphere<T>::SetRadius(T aRadius)
{
  mRadius = aRadius;
}

///////////////////////////////////////////////
//! Get the sphere radius.
template <typename T>
T cSphere<T>::GetRadius() const
{
  return mRadius;
}

///////////////////////////////////////////////
//! Set the sphere position.
template <typename T>
void cSphere<T>::SetPosition(const sVec3<T>& aPos)
{
  mvPosition = aPos;
}

///////////////////////////////////////////////
//! Get the sphere position.
template <typename T>
const sVec3<T>& cSphere<T>::GetPosition() const
{
  return mvPosition;
}

///////////////////////////////////////////////
//! Intersection with a point.
template <typename T>
tBool cSphere<T>::Intersect(const sVec3<T>& vPos) const
{
  return VecLength(vPos-mvPosition) <= mRadius;
}

///////////////////////////////////////////////
//! Intersection with a ray.
//! \return the first intersection distance with a ray. If no intersection
//!     return -1.
template <typename T>
T cSphere<T>::Intersect(const sVec3<T>& avPos, const sVec3<T>& avDir) const
{
  sVec3<T> Q = mvPosition-avPos;

  T c = VecLength(Q);
  T v = VecDot(Q, VecNormalize(sVec3<T>(),avDir));
  T d = (mRadius*mRadius) - ((c*c) - (v*v));

  // If there was no intersection, return -1
  if (d < 0.0)
    return T(-1.0);

  // Return the distance to the [first] intersecting point
  return T(v - sqrt(d));
}

///////////////////////////////////////////////
//! Transform by the specified matrix.
template <typename T>
void cSphere<T>::Transform(const sMatrixf& aMatrix)
{
  sVec3<T> vec,p;
  VecSetLength(vec,sVec3<T>::XAxis(),GetRadius());
  VecTransformNormal(vec,vec,aMatrix);
  SetRadius(VecLength(vec));
  VecTransformCoord(p,GetCenter(),aMatrix);
  SetPosition(p);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __SPHERE_32687073_H__
