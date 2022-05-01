#ifndef __DRAWOPERATION_842634_H__
#define __DRAWOPERATION_842634_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

struct iDrawOperationSet;

/** \addtogroup niUI
 * @{
 */

//! Draw operation flags type.
typedef tU32 tDrawOperationFlags;

//! Draw operation interface.
struct iDrawOperation : public iUnknown
{
  niDeclareInterfaceUUID(iDrawOperation,0x7f9eaa80,0x03e5,0x4514,0xa1,0x62,0xa3,0x53,0x86,0xcf,0x61,0xea)

  //! Copy another draw operation.
  virtual tBool __stdcall Copy(const iDrawOperation* apDO) = 0;
  //! Clone this draw operation.
  virtual iDrawOperation* __stdcall Clone() const = 0;

  //! Return whether the draw operation is compiled (read-only)
  //! {Property}
  virtual tBool __stdcall GetIsCompiled() const = 0;

  //! Set the priority.
  //! {Property}
  virtual void __stdcall SetPriority(tU32 anPriority) = 0;
  //! Get the priority.
  //! {Property}
  virtual tU32 __stdcall GetPriority() const = 0;

  //! Set the vertex array.
  //! {Property}
  virtual void __stdcall SetVertexArray(iVertexArray* apVertexArray) = 0;
  //! Get the vertex array.
  //! {Property}
  virtual iVertexArray* __stdcall GetVertexArray() const = 0;
  //! Set the primitive type.
  //! \remark The primtive type is relevant only if no index array has been set.
  //! \remark If an index array is set the primitive type is read only, tying to set it will
  //!     have no effect. The primitive type returned will be the one of the index array.
  //! {Property}
  virtual void __stdcall SetPrimitiveType(eGraphicsPrimitiveType aPrim) = 0;
  //! Get the primitive type.
  //! {Property}
  virtual eGraphicsPrimitiveType __stdcall GetPrimitiveType() const = 0;

  //! Set the index array.
  //! {Property}
  virtual void __stdcall SetIndexArray(iIndexArray* apIndexArray) = 0;
  //! Get the index array.
  //! {Property}
  virtual iIndexArray* __stdcall GetIndexArray() const = 0;
  //! Set the first index.
  //! {Property}
  virtual void __stdcall SetFirstIndex(tU32 anIndex) = 0;
  //! Get the first index.
  //! {Property}
  virtual tU32 __stdcall GetFirstIndex() const = 0;
  //! Set the number of indices.
  //! {Property}
  virtual void __stdcall SetNumIndices(tU32 anNumIndices) = 0;
  //! Get the number of indices.
  //! {Property}
  virtual tU32 __stdcall GetNumIndices() const = 0;
  //! Set the base vertex index.
  //! {Property}
  virtual void __stdcall SetBaseVertexIndex(tU32 anBaseVertexIndex) = 0;
  //! Get the base vertex index.
  //! {Property}
  virtual tU32 __stdcall GetBaseVertexIndex() const = 0;

  //! Set the matrix.
  //! {Property}
  virtual void __stdcall SetMatrix(const sMatrixf& apMatrix) = 0;
  //! Get the matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrix() const = 0;

  //! Set the material.
  //! {Property}
  virtual void __stdcall SetMaterial(iMaterial* apMaterial) = 0;
  //! Get the material.
  //! {Property}
  virtual iMaterial* __stdcall GetMaterial() const = 0;

  //! Set the bounding volume.
  //! {Property}
  virtual void __stdcall SetLocalBoundingVolume(iBoundingVolume* apBV) = 0;
  //! Get the bounding volume in local space.
  //! {Property}
  virtual iBoundingVolume* __stdcall GetLocalBoundingVolume() const = 0;
  //! Get the bounding volume in world space.
  //! {Property}
  virtual iBoundingVolume* __stdcall GetBoundingVolume() const = 0;
  //! Get the center position.
  //! {Property}
  //! \remark The center position is the center of the bouding volume multiplied by the matrix.
  virtual sVec3f __stdcall GetCenter() const = 0;

  //! Get the FVF of the draw operation.
  //! {Property}
  //! \remark Try to get the FVF from the material, if zero the FVF of the vertex array is returned.
  virtual tFVF __stdcall GetFVF() const = 0;
};

//! Draw operation smart pointer list.
typedef tInterfaceCVec<iDrawOperation> tDrawOperationCVec;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __DRAWOPERATION_842634_H__
