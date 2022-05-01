#ifndef __INUSPLINE_30471746_H__
#define __INUSPLINE_30471746_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include <niLang/ISerializable.h>

namespace ni {
struct iDataTable;

/** \addtogroup niUI
 * @{
 */

//! Non-uniform spline type.
enum eNUSplineType
{
  //! Linear spline. Not a spline simple linear interpolation.
  eNUSplineType_Linear = 0,
  //! Rounded non-uniform spline.
  eNUSplineType_Rounded = 1,
  //! Smooth non-uniform spline.
  eNUSplineType_Smooth = 2,
  //! Timed non-uniform spline.
  eNUSplineType_Timed = 3,
  //! \internal
  eNUSplineType_Last = 4,
  //! \internal
  eNUSplineType_ForceDWORD = 0xFFFFFFFF
};

//! Non-uniform spline interface.
struct iNUSpline : public iUnknown
{
  niDeclareInterfaceUUID(iNUSpline,0x37096dfe,0xb7f7,0x4d34,0xa5,0xbf,0x5e,0x2e,0x2a,0x91,0xa8,0x4a)
  //! Set the spline type.
  //! {Property}
  virtual tBool __stdcall SetType(eNUSplineType aType) = 0;
  //! Get the spline type.
  //! {Property}
  virtual eNUSplineType __stdcall GetType() const = 0;
  //! Clear the spline, remove all nodes.
  virtual void __stdcall Clear() = 0;
  //! Add a node.
  //! \remark The node is a 4D vector, with x,y,z as position and w as time interval.
  //! \remark The time interval is used only by timed non-uniform spline.
  virtual void __stdcall AddNode(const sVec4f& avNode) = 0;
  //! Remove the node at the specified index.
  virtual tBool __stdcall RemoveNode(tU32 anIndex) = 0;
  //! Get the number of nodes.
  //! {Property}
  virtual tU32 __stdcall GetNumNodes() const = 0;
  //! Set the node at the specified index.
  //! {Property}
  virtual tBool __stdcall SetNode(tU32 anIndex, const sVec4f& avNode) = 0;
  //! Get the node at the specified index.
  //! {Property}
  virtual sVec4f __stdcall GetNode(tU32 anIndex) const = 0;
  //! Get the distance of the specified node.
  //! {Property}
  virtual tF32 __stdcall GetNodeDistance(tU32 anIndex) const = 0;
  //! Get the length of the path.
  //! {Property}
  virtual tF32 __stdcall GetLength() const = 0;
  //! Get the position at the specified time.
  //! \remark The spline must have at least three nodes to return a valid position.
  //!     If the spline is invalid a 0,0,0 position will be returned.
  //! {Property}
  virtual sVec3f __stdcall GetPosition(tF32 afTime) = 0;
  //! Get the relative position at the specified time.
  //! \remark The spline must have at least three nodes to return a valid position.
  //!     If the spline is invalid a 0,0,0 position will be returned.
  //! \remark The relative position is relative to the first node's position.
  //! {Property}
  virtual sVec3f __stdcall GetRelativePosition(tF32 afTime) = 0;
  //! Serialize the spline in the specified datatable.
  virtual tBool __stdcall SerializeDataTable(iDataTable* apDT, tSerializeFlags aFlags) = 0;
};

niExportFunc(iUnknown*) New_niUI_NUSpline(const Var&, const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __INUSPLINE_30471746_H__
