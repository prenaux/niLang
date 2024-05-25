#ifndef __ITRANSFORM_4187289_H__
#define __ITRANSFORM_4187289_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

struct iTransform;

/** \addtogroup niUI
 * @{
 */

//! Transform flags. Default is eTransformFlags_InheritAll
enum eTransformFlags
{
  //! Inherit the position X axis.
  eTransformFlags_InheritPositionX = niBit(0),
  //! Inherit the position Y axis.
  eTransformFlags_InheritPositionY = niBit(1),
  //! Inherit the position Z axis.
  eTransformFlags_InheritPositionZ = niBit(2),
  //! Inherit the position XYZ axis.
  eTransformFlags_InheritPosition = niBit(0)|niBit(1)|niBit(2),
  //! Inherit the rotation.
  eTransformFlags_InheritRotation = niBit(3),
  //! Parent scalings are also apply on children.
  eTransformFlags_InheritScale  = niBit(4),
  //! Inherit the position and rotation.
  eTransformFlags_InheritPositionRotation = eTransformFlags_InheritPosition|eTransformFlags_InheritRotation,
  //! Inherit all.
  eTransformFlags_InheritAll = eTransformFlags_InheritPositionRotation|eTransformFlags_InheritScale,
  //! \internal
  eTransformFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Transform internal flags.
enum eTransformInternalFlags
{
  //! The transform will have to be recomputed.
  eTransformInternalFlags_Dirty = niBit(8),
  //! The transform uses a scaling factor.
  eTransformInternalFlags_UseScale = niBit(9),
  //! \internal
  eTransformInternalFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Transform description structure.
struct sTransformDesc
{
  Ptr<iTransform> mptrParent;
  sMatrixf  mmtxWorld;
  sMatrixf  mmtxLocal;
  sVec3f mvScale;
  tU16        mnFlags;
  tU16        mnSyncCounter;
};

//! iTransform is the interface for positionning and orienting.
struct iTransform : public iUnknown
{
  niDeclareInterfaceUUID(iTransform,0x2b890822,0x2bff,0x426d,0xa6,0x46,0x51,0x69,0x1c,0x16,0x49,0xb4)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Get the transform description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;

  //! Clone the transform.
  virtual iTransform* __stdcall Clone() const = 0;
  //! Copy another transform in this transform.
  //! \remark This function should be prefered to copying the world matrix
  //!     of the transform. The scale of the transform is stored separatly
  //!     and will be lost if not copied aswell.
  virtual tBool __stdcall Copy(const iTransform* apSrc) = 0;

  //! Mark the transform as "Dirty".
  //! \remark If the transform isn't already marked as dirty it increases the update count.
  //! \return The previous value of the sync counter, garanteed to be different of the current counter.
  virtual tU16 __stdcall SetDirty() = 0;

  //! Set the transform flags.
  //! {Property}
  virtual void __stdcall SetFlags(tU16 anFlags) = 0;
  //! Get the transform flags.
  //! {Property}
  virtual tU16 __stdcall GetFlags() const = 0;

  //! Set the synchronization counter.
  //! {Property}
  virtual void __stdcall SetSyncCounter(tU16 anFlags) = 0;
  //! Get the synchronization counter.
  //! {Property}
  //! \remark The counter can be used to synchronize updates.
  virtual tU16 __stdcall GetSyncCounter() const = 0;
  //! @}

  //########################################################################################
  //! \name Parent
  //########################################################################################
  //! @{

  //! Set the parent transform.
  //! {Property}
  virtual void __stdcall SetParent(iTransform* apParent) = 0;
  //! Get the parent transform.
  //! {Property}
  virtual iTransform* __stdcall GetParent() const = 0;
  //! @}

  //########################################################################################
  //! \name Matrices
  //########################################################################################
  //! @{

  //! Set the transform to identity.
  virtual void __stdcall Identity() = 0;
  //! Set the world matrix.
  //! {Property}
  virtual void __stdcall SetWorldMatrix(const sMatrixf& aMatrix) = 0;
  //! Get the world matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetWorldMatrix() const = 0;
  //! Set the local matrix.
  //! {Property}
  virtual void __stdcall SetLocalMatrix(const sMatrixf& aMatrix) = 0;
  //! Get the local matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetLocalMatrix() const = 0;
  //! Multiply the world matrix.
  virtual void __stdcall MultiplyWorldMatrix(const sMatrixf& aMatrix) = 0;
  //! Pre-Multiply the world matrix.
  virtual void __stdcall PreMultiplyWorldMatrix(const sMatrixf& aMatrix) = 0;
  //! Multiply the local matrix.
  virtual void __stdcall MultiplyLocalMatrix(const sMatrixf& aMatrix) = 0;
  //! Pre-Multiply the local matrix.
  virtual void __stdcall PreMultiplyLocalMatrix(const sMatrixf& aMatrix) = 0;
  //! Set the transform's orientation that the forward vector points
  //! at a specified world position.
  virtual void __stdcall LookAt(const sVec3f& avLookAt, const sVec3f& avUp) = 0;
  //! @}

  //########################################################################################
  //! \name Position
  //########################################################################################
  //! @{

  //! Set the world position  of the transform.
  //! {Property}
  virtual void __stdcall SetWorldPosition(const sVec3f& v) = 0;
  //! Get the world position of the transform.
  //! {Property}
  virtual sVec3f __stdcall GetWorldPosition() const = 0;
  //! Set the local position of the transform.
  //! {Property}
  virtual void __stdcall SetLocalPosition(const sVec3f& v) = 0;
  //! Get the local position of the transform.
  //! {Property}
  virtual sVec3f __stdcall GetLocalPosition() const = 0;
  //! Translate the local position of the transform.
  virtual void __stdcall Translate(const sVec3f& v) = 0;
  //! PreTranslate the local position of the transform.
  virtual void __stdcall PreTranslate(const sVec3f& v) = 0;
  //! @}

  //########################################################################################
  //! \name Rotations
  //########################################################################################
  //! @{

  //! Set the world rotation of the transform.
  //! {Property}
  virtual void __stdcall SetWorldRotation(const sMatrixf& aMatrix) = 0;
  //! Set the local rotation of the transform.
  //! {Property}
  virtual void __stdcall SetLocalRotation(const sMatrixf& aMatrix) = 0;
  //! Rotate the local rotation of the transform.
  virtual void __stdcall Rotate(const sMatrixf& aMatrix) = 0;
  //! PreRotate the local rotation of the transform.
  virtual void __stdcall PreRotate(const sMatrixf& aMatrix) = 0;
  //! @}

  //########################################################################################
  //! \name Scale
  //########################################################################################
  //! @{

  //! Set the scale of the transform.
  //! {Property}
  virtual void __stdcall SetScale(const sVec3f& aScale) = 0;
  //! Return the scale of the transform.
  //! {Property}
  virtual sVec3f __stdcall GetScale() const = 0;
  //! @}

  //########################################################################################
  //! \name Offset transforms
  //########################################################################################
  //! @{

  //! Create a pre offset transform.
  //! \remark The created transform will be the new parent of this transform. The current
  //!     parent of this transform becoming the parent of the offset transform.
  virtual iTransform* __stdcall CreatePreOffsetTransform() = 0;
  //! Create a post offset transform.
  //! \remark The created transform will have this transform as parent.
  virtual iTransform* __stdcall CreatePostOffsetTransform() = 0;
  //! @}

  //########################################################################################
  //! \name Axis vectors
  //########################################################################################
  //! @{

  //! Get the right (X) vector.
  //! {Property}
  virtual sVec3f __stdcall GetRight() const = 0;
  //! Get the up (Y) vector.
  //! {Property}
  virtual sVec3f __stdcall GetUp() const = 0;
  //! Get the forward (Z) vector.
  //! {Property}
  virtual sVec3f __stdcall GetForward() const = 0;
  //! Get the inverse transform right (X) vector.
  //! {Property}
  virtual sVec3f __stdcall GetInvRight() const = 0;
  //! Get the inverse transform up (Y) vector.
  //! {Property}
  virtual sVec3f __stdcall GetInvUp() const = 0;
  //! Get the inverse transform forward (Z) vector.
  //! {Property}
  virtual sVec3f __stdcall GetInvForward() const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ITRANSFORM_4187289_H__
