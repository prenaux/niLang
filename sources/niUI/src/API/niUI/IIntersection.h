#ifndef __IINTERSECTION_60545992_H__
#define __IINTERSECTION_60545992_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Intersection result.
enum eIntersectionResult
{
  //! No intersection result.
  eIntersectionResult_None = 0,
  //! Inside the bounding volume.
  eIntersectionResult_Inside = 1,
  //! Intersect the bounding volume.
  eIntersectionResult_Intersect = 2,
  //! \internal
  eIntersectionResult_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Intersection result interface.
struct iIntersection : public iUnknown
{
  niDeclareInterfaceUUID(iIntersection,0x64c8b5c4,0x4d70,0x46e4,0xb5,0x34,0x9d,0xa2,0x85,0x4d,0x19,0x28)
  //! Set the intersection result.
  //! {Property}
  virtual void __stdcall SetResult(eIntersectionResult aResult) = 0;
  //! Get the intersection result.
  virtual eIntersectionResult __stdcall GetResult() const = 0;
  //! {Property}
  //! Set the intersection's position.
  //! {Property}
  virtual void __stdcall SetPosition(const sVec3f& avPos) = 0;
  //! Get the intersection's position.
  //! {Property}
  virtual sVec3f __stdcall GetPosition() const = 0;
  //! Set the intersection's barycentric coordinate.
  //! {Property}
  virtual void __stdcall SetBaryCentric(const sVec2f& avBC) = 0;
  //! Get the intersection's barycentric coordinate.
  //! {Property}
  virtual sVec2f __stdcall GetBaryCentric() const = 0;
  //! Set the index of the intersected polygon.
  //! {Property}
  virtual void __stdcall SetPolygonIndex(tU32 anIndex) = 0;
  //! Get the index of the intersected polygon.
  //! {Property}
  virtual tU32 __stdcall GetPolygonIndex() const = 0;
};

niExportFunc(iUnknown*) New_niUI_Intersection(const Var&, const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IINTERSECTION_60545992_H__
