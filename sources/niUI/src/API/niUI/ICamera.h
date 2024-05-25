#ifndef __ICAMERA_21384900_H__
#define __ICAMERA_21384900_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IFrustum.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Camera move type
enum eCameraMoveType
{
  //! The camera view is set through the position, target & target up directly.
  eCameraMoveType_None = 0,
  //! The camera simulates a flying object.
  //! \remark Moves the target with the position.
  eCameraMoveType_Fly = 1,
  //! The camera orbits around it's target.
  //! \remark Move up will 'orbit up/down', move forward will zoom in/out,
  //!         move sidewards will 'orbit left/right'. The movements should be
  //!         in radians, zoom in/out in the same unit as the regular move
  //!         forward.
  //! \remark The target is never moved directly in orbit mode.
  eCameraMoveType_Orbit = 2,
  //! The camera simulates a flying object with a fixed target.
  eCameraMoveType_FlyFixedTarget = 3,
  //! The camera view is set through SetViewMatrix directly.
  eCameraMoveType_SetMatrix = 4,
  //! \internal
  eCameraMoveType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Camera projection type.
enum eCameraProjectionType
{
  //! Perspective camera.
  eCameraProjectionType_Perspective = 0,
  //! Orthogonal with a free view pov.
  eCameraProjectionType_Orthogonal = 1,
  //! The camera projection is set through SetProjectionMatrix directly.
  eCameraProjectionType_SetMatrix = 2,
  //! \internal
  eCameraProjectionType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Camera interface
struct iCamera : public iUnknown
{
  niDeclareInterfaceUUID(iCamera,0x3769afcf,0x3b13,0x4545,0x84,0x74,0xef,0x46,0x3a,0x1e,0x67,0x55)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Copy the camera.
  virtual void __stdcall Copy(const iCamera* pSrc) = 0;
  //! Clone the camera.
  virtual iCamera* __stdcall Clone() const = 0;
  //! Get the world matrix.
  //! {Property}
  virtual ni::sMatrixf __stdcall GetWorldMatrix() const = 0;
  //! Get the view matrix.
  //! {Property}
  virtual ni::sMatrixf __stdcall GetViewMatrix() const = 0;
  //! Get the projection matrix.
  //! {Property}
  virtual ni::sMatrixf __stdcall GetProjectionMatrix() const = 0;
  //! Get the frustum.
  //! {Property}
  virtual ni::iFrustum* __stdcall GetFrustum() const = 0;
  //! Compute a ray from the given screen position and rectangle.
  //! \return The normalized direction of a ray going starting at
  //!         RayStart and going in the direction of the point in
  //!         the specified rectangle projected in the camera's
  //!         space.
  //! \remark The starting point of the ray should be retrieved with
  //!         iCamera::GetRayStart(), while it usually coincide with
  //!         the camera's position it might not always be the case
  //!         (such as for orthographic projections).
  //! \remark This method is commonly used to build a ray to be used
  //!         by picking in camera space.
  virtual ni::sVec3f __stdcall GetRay(ni::tF32 afX, ni::tF32 afY, const ni::sRectf& aRect) = 0;
  //! Get the starting point of the last ray
  //! {Property}
  virtual sVec3f __stdcall GetRayStart() const = 0;
  //! Get a screen position from the given 3d position.
  virtual ni::sVec3f __stdcall GetScreenPosition(const ni::sVec3f& avPos, const ni::sRectf& aRect) const = 0;

  //! Set the view matrix directly.
  //! \remark This should one be used with eCameraMoveType_None.
  //! {Property}
  virtual void __stdcall SetViewMatrix(const sMatrixf& aViewMatrix) = 0;
  //! Set the projection matrix directly.
  //! \remark This should one be used with eCameraProjectionType_None.
  //! {Property}
  virtual void __stdcall SetProjectionMatrix(const sMatrixf& aProjMatrix) = 0;
  //! Set the camera position, target and up vector from a matrix.
  //! \param aMatrix is the matrix from which the new camera position and target will be derived
  //! \param abSetUp if true the target's up vector will also be derived from the specifed matrix.
  virtual void __stdcall SetFromWorldMatrix(const ni::sMatrixf& aMatrix, tBool abSetUp) = 0;
  //! @}

  //########################################################################################
  //! \name Projection setup
  //########################################################################################
  //! @{

  //! Set projection type.
  //! {Property}
  virtual void __stdcall SetProjection(eCameraProjectionType aProjectionType) = 0;
  //! Get orthogonal projection.
  //! {Property}
  virtual eCameraProjectionType __stdcall GetProjection() const = 0;
  //! Set the camera's viewport rectangle.
  //! {Property}
  virtual void __stdcall SetViewport(const ni::sRectf& aRect) = 0;
  //! Get the camera's viewport rectangle.
  //! {Property}
  virtual ni::sRectf __stdcall GetViewport() const = 0;
  //! Set the vertical field of view.
  //! {Property}
  virtual void __stdcall SetFov(ni::tF32 fFOV) = 0;
  //! Get the vertical field of view.
  //! {Property}
  virtual ni::tF32 __stdcall GetFov() const = 0;
  //! Set the aspect ratio.
  //! {Property}
  //! \remark If the aspect ratio is 0, the aspect of the viewport is used.
  virtual void __stdcall SetAspect(ni::tF32 fAspect) = 0;
  //! Get the aspect ratio.
  //! {Property}
  virtual ni::tF32 __stdcall GetAspect() const = 0;
  //! Get the absolute aspect ratio.
  //! {Property}
  //! \remark The absolute aspect ratio is the aspect ratio of the viewport if the aspect is <= 0.
  virtual ni::tF32 __stdcall GetAbsoluteAspect() const = 0;
  //! Set the near clipping plane.
  //! {Property}
  virtual void __stdcall SetNearClipPlane(ni::tF32 afD) = 0;
  //! Get the near clipping plane.
  //! {Property}
  virtual ni::tF32 __stdcall GetNearClipPlane() const = 0;
  //! Set the far clipping plane.
  //! {Property}
  virtual void __stdcall SetFarClipPlane(ni::tF32 afD) = 0;
  //! Get the far clipping plane.
  //! {Property}
  virtual ni::tF32 __stdcall GetFarClipPlane() const = 0;
  //! Set the orthogonal projection size.
  //! {Property}
  virtual void __stdcall SetOrthoSize(tF32 afSize) = 0;
  //! Get the orthogonal projection size.
  //! {Property}
  //! \remark Height of the othogonal projection, Width = OrthoSize * AbsoluteAspect
  virtual ni::tF32 __stdcall GetOrthoSize() const = 0;
  //! @}

  //########################################################################################
  //! \name Position
  //########################################################################################
  //! @{

  //! Set the position.
  //! {Property}
  virtual void __stdcall SetPosition(const ni::sVec3f& vPos) = 0;
  //! Get the position.
  //! {Property}
  virtual ni::sVec3f __stdcall GetPosition() const = 0;
  //! Set the target.
  //! {Property}
  virtual void __stdcall SetTarget(const ni::sVec3f& avTarget) = 0;
  //! Get the target.
  //! {Property}
  virtual ni::sVec3f __stdcall GetTarget() const = 0;
  //! Set the target up vector.
  //! {Property}
  virtual void __stdcall SetTargetUp(const ni::sVec3f& avUp) = 0;
  //! Get the target up vector.
  //! {Property}
  virtual ni::sVec3f __stdcall GetTargetUp() const = 0;
  //! Set the distance between the camera and it's target by moving the target.
  //! {Property}
  virtual void __stdcall SetTargetDistance(ni::tF32 afDist) = 0;
  //! Get the distance between the camera and it's target.
  //! {Property}
  virtual ni::tF32 __stdcall GetTargetDistance() const = 0;
  //! Set the distance between the camera and it's target by moving the position.
  //! {Property}
  virtual void __stdcall SetPositionDistance(ni::tF32 afDist) = 0;
  //! Get the distance between the camera and it's target.
  //! {Property}
  virtual ni::tF32 __stdcall GetPositionDistance() const = 0;
  //! Set the direction of the target.
  //! {Property}
  //! \remark The direction is from the camera to the target.
  //! \remark The direction is a normal vector.
  virtual void __stdcall SetTargetDirection(const ni::sVec3f& avDir) = 0;
  //! Get the direction of the target.
  //! {Property}
  //! \remark The direction is from the camera to the target.
  //! \remark The direction is a normal vector.
  virtual ni::sVec3f __stdcall GetTargetDirection() const = 0;
  //! @}

  //########################################################################################
  //! \name Movements
  //########################################################################################
  //! @{

  //! Set the camera move mode.
  //! {Property}
  virtual void __stdcall SetMoveType(eCameraMoveType aMode) = 0;
  //! Get the camera move mode.
  //! {Property}
  virtual eCameraMoveType __stdcall GetMoveType() const = 0;
  //! Move the camera sidewards relatively to itself. (X axis)
  virtual void __stdcall MoveSidewards(ni::tF32 delta) = 0;
  //! Move the camera upward relatively to itself. (Y axis)
  virtual void __stdcall MoveUp(ni::tF32 delta) = 0;
  //! Move the camera forward relatively to itself. (Y axis)
  virtual void __stdcall MoveForward(ni::tF32 delta) = 0;
  //! Set the relative movement vector. (x+ sidewards, y+ upwards and z+ forward)
  //! {Property}
  virtual void __stdcall SetMove(const ni::sVec3f& avMove) = 0;
  //! Get the relative movement vector.
  //! {Property}
  virtual ni::sVec3f __stdcall GetMove() const = 0;
  //! @}

  //########################################################################################
  //! \name Orientation
  //########################################################################################
  //! @{

  //! Rotates the target around the specified axis.
  virtual void __stdcall RotateTarget(ni::sVec3f avAxis, ni::tF32 afAngle) = 0;
  //! Rotates the target up around the specified axis.
  virtual void __stdcall RotateTargetUp(ni::sVec3f avAxis, ni::tF32 afAngle) = 0;
  //! Rotates the target up around the specified axis.
  //! Add yaw.
  //! \remark Rotates the target around the Y axis.
  virtual void __stdcall AddYaw(ni::tF32 a) = 0;
  //! Add pitch.
  //! \remark Rotates the target around the right axis projected on the XZ plane.
  virtual void __stdcall AddPitch(ni::tF32 a) = 0;
  //! Add roll.
  //! \remark Rotates the target up around the forward axis projected on the XZ plane.
  virtual void __stdcall AddRoll(ni::tF32 a) = 0;
  //! Orbit up/down the position of the camera around the current target.
  //! \remark Rotates the position around the target about it's right axis.
  //! \remark The rotation is clamped to the the pole of the orbit sphere, so
  //!     that trying to orbit up when the camera is already at the north/top
  //!     pole will not have any effect, and reciprocicaly at the south/bottom
  //!     pole.
  virtual void __stdcall OrbitUp(ni::tF32 afA) = 0;
  //! Orbit left/right the position of the camera around the current target.
  //! \remark Rotates the position around the target about the y axis.
  virtual void __stdcall OrbitSidewards(ni::tF32 afA) = 0;
  //! Get the forward vector.
  //! {Property}
  virtual ni::sVec3f __stdcall GetForward() const = 0;
  //! Get the right vector.
  //! {Property}
  virtual ni::sVec3f __stdcall GetRight() const = 0;
  //! Get the up vector.
  //! {Property}
  virtual ni::sVec3f __stdcall GetUp() const = 0;
  //! @}
};

niExportFunc(iUnknown*) New_niUI_Camera(const Var&, const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ICAMERA_21384900_H__
