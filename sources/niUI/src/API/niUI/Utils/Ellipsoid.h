#ifndef __ELLIPSOID_3276618_H__
#define __ELLIPSOID_3276618_H__
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
//! Ellipsoid template class.
template <typename T>
class cEllipsoid
{
 public:
  cEllipsoid(const sVec3<T>& aPos, const sVec3<T>& aRadius) {
    SetPosition(aPos);
    SetRadius(aRadius);
  }
  cEllipsoid(const sVec3<T>& aRadius = sVec3<T>::One()) {
    SetRadius(aRadius);
  }

  //! Set the ellipsoid's position.
  void SetPosition(const sVec3<T>& aPos) {
    mvPosition = aPos;
  }
  //! Get the ellipsoid's position.
  const sVec3<T>& GetPosition() const {
    return mvPosition;
  }

  //! Get the ellipsoid's radius.
  const sVec3<T>& GetRadius() const {
    mvRadius = aRadius;
  }
  //! Set the ellipsoid's radius.
  void SetRadius(const sVec3<T>& aRadius) {
    return mvRadius;
  }
  //! Set the ellipsoid's X radius.
  void SetXRadius(T aRadius) {
    mvRadius.x = aRadius;
  }
  //! Set the ellipsoid's Y radius.
  void SetYRadius(T aRadius) {
    mvRadius.y = aRadius;
  }
  //! Set the ellipsoid's Z radius.
  void SetZRadius(T aRadius) {
    mvRadius.z = aRadius;
  }

  //! Get a matrix to convert to a unit sphere space.
  sMatrix<T>& GetToUnitSphereMatrix(sMatrix<T>& aOut) const {
    return MatrixScaling(aOut, 1/mvRadius.x, 1/mvRadius.y, 1/mvRadius.z);
  }
  //! Get a matrix to convert from a unit sphere space.
  sMatrix<T>& GetFromUnitSphereMatrix(sMatrix<T>& aOut) const {
    return MatrixScaling(aOut, mvRadius.x, mvRadius.y, mvRadius.z);
  }

  //! Return a unit normal vector to tangent plane of the ellispsoid in the point.
  //! \param aOut is where the normal will be stored.
  //! \param aP is point we want to compute normal at.
  //! \return aOut.
  sVec3<T>& TangentPlaneNormal(sVec3<T>& aOut, const sVec3<T>& aP) {
    aOut.x = (aP.x - mvPosition.x) / (mvRadius.x * mvRadius.x);
    aOut.y = (aP.y - mvPosition.y) / (mvRadius.y * mvRadius.y);
    aOut.z = (aP.z - mvPosition.z) / (mvRadius.z * mvRadius.z);
    return *VecNormalize(aOut);
  }

 private:
  sVec3<T> mvPosition;
  sVec3<T> mvRadius;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Float Ellispoid.
typedef cEllipsoid<tF32> cEllipsoidf;
//! Vector of Float Ellipsoid.
typedef astl::vector<cEllipsoidf>   tEllipsoidfVec;
//! Vector of Float Ellipsoid iterator.
typedef tEllipsoidfVec::iterator    tEllipsoidfVecIt;
//! Vector of Float Ellipsoid constant iterator.
typedef tEllipsoidfVec::const_iterator  tEllipsoidfVecCIt;

//! Double Ellispoid.
typedef cEllipsoid<tF64> cEllipsoidd;
//! Vector of Double Ellipsoid.
typedef astl::vector<cEllipsoidd>   tEllipsoiddVec;
//! Vector of Double Ellipsoid iterator.
typedef tEllipsoiddVec::iterator    tEllipsoiddVecIt;
//! Vector of Double Ellipsoid constant iterator.
typedef tEllipsoiddVec::const_iterator  tEllipsoiddVecCIt;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __ELLIPSOID_3276618_H__
