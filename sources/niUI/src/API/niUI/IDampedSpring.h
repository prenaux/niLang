#ifndef __IDAMPEDSPRING_19269492_H__
#define __IDAMPEDSPRING_19269492_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! 1D Damped spring interface.
//! \see ni::DampedSpringAcceleration
struct iDampedSpring1 : public iUnknown
{
  niDeclareInterfaceUUID(iDampedSpring1,0x8c5a71f0,0x3e1b,0x41b6,0x84,0xa2,0xf6,0xf8,0xfe,0x08,0x56,0x7a)

  //! Set the Kd constant.
  //! {Property}
  virtual void __stdcall SetKd(tF32 afD) = 0;
  //! Get the Kd constant.
  //! {Property}
  virtual tF32 __stdcall GetKd() const = 0;
  //! Set the Ks constant.
  //! {Property}
  virtual void __stdcall SetKs(tF32 afD) = 0;
  //! Get the Ks constant.
  //! {Property}
  virtual tF32 __stdcall GetKs() const = 0;

  //! Set the value of Kd for the specified Ks that will result in a spring with the specified damping ratio.
  //! \remark Sets Ks with parameter's value, sets Kd in function of Ks.
  virtual void __stdcall SetStiffnessAndDampingRatio(tF32 afKs, tF32 afE) = 0;

  //! Set the spring's damping ratio.
  //! {Property}
  //! \remark Sets Kd in function of the current value of Ks to achieve the specified damping ratio.
  virtual void __stdcall SetDampingRatio(tF32 afE) = 0;
  //! Get the spring's damping ratio.
  //! {Property}
  virtual tF32 __stdcall GetDampingRatio() const = 0;

  //! Set the spring's velocity.
  //! {Property}
  virtual void __stdcall SetVelocity(tF32 avVel) = 0;
  //! Get the spring's velocity.
  //! {Property}
  virtual tF32 __stdcall GetVelocity() const = 0;

  //! Compute the spring's acceleration for the specified displacement.
  virtual tF32 __stdcall ComputeAcceleration(tF32 avD) = 0;
  //! Update the spring's velocity from the specified displacement.
  virtual void __stdcall UpdateVelocity(ni::tF32 afDeltaTime, tF32 avD) = 0;
  //! Update the spring's velocity from the specified acceleration.
  virtual void __stdcall UpdateVelocityWithAcceleration(ni::tF32 afDeltaTime, tF32 avAcc) = 0;
};

//! 2D Damped spring interface.
//! \see ni::DampedSpringAcceleration
struct iDampedSpring2 : public iUnknown
{
  niDeclareInterfaceUUID(iDampedSpring2,0xab391d38,0xff1b,0x4503,0xa0,0x57,0x82,0x42,0xa5,0x11,0x02,0x15)

  //! Set the Kd constant.
  //! {Property}
  virtual void __stdcall SetKd(tF32 afD) = 0;
  //! Get the Kd constant.
  //! {Property}
  virtual tF32 __stdcall GetKd() const = 0;
  //! Set the Ks constant.
  //! {Property}
  virtual void __stdcall SetKs(tF32 afD) = 0;
  //! Get the Ks constant.
  //! {Property}
  virtual tF32 __stdcall GetKs() const = 0;

  //! Set the value of Kd for the specified Ks that will result in a spring with the specified damping ratio.
  //! \remark Sets Ks with parameter's value, sets Kd in function of Ks.
  virtual void __stdcall SetStiffnessAndDampingRatio(tF32 afKs, tF32 afE) = 0;

  //! Set the spring's damping ratio.
  //! {Property}
  //! \remark Sets Kd in function of the current value of Ks to achieve the specified damping ratio.
  virtual void __stdcall SetDampingRatio(tF32 afE) = 0;
  //! Get the spring's damping ratio.
  //! {Property}
  virtual tF32 __stdcall GetDampingRatio() const = 0;

  //! Set the spring's velocity.
  //! {Property}
  virtual void __stdcall SetVelocity(const sVec2f& avVel) = 0;
  //! Get the spring's velocity.
  //! {Property}
  virtual sVec2f __stdcall GetVelocity() const = 0;

  //! Compute the spring's acceleration for the specified displacement.
  virtual sVec2f __stdcall ComputeAcceleration(const sVec2f& avD) = 0;
  //! Update the spring's velocity from the specified displacement.
  virtual void __stdcall UpdateVelocity(ni::tF32 afDeltaTime, const sVec2f& avD) = 0;
  //! Update the spring's velocity from the specified acceleration.
  virtual void __stdcall UpdateVelocityWithAcceleration(ni::tF32 afDeltaTime, const sVec2f& avAcc) = 0;
};

//! 3D Damped spring interface.
//! \see ni::DampedSpringAcceleration
struct iDampedSpring3 : public iUnknown
{
  niDeclareInterfaceUUID(iDampedSpring3,0x70fe36f4,0x59c8,0x4590,0xa5,0xa5,0x7d,0xc7,0x37,0xdb,0xf8,0x74)

  //! Set the Kd constant.
  //! {Property}
  virtual void __stdcall SetKd(tF32 afD) = 0;
  //! Get the Kd constant.
  //! {Property}
  virtual tF32 __stdcall GetKd() const = 0;
  //! Set the Ks constant.
  //! {Property}
  virtual void __stdcall SetKs(tF32 afD) = 0;
  //! Get the Ks constant.
  //! {Property}
  virtual tF32 __stdcall GetKs() const = 0;

  //! Set the value of Kd for the specified Ks that will result in a spring with the specified damping ratio.
  //! \remark Sets Ks with parameter's value, sets Kd in function of Ks.
  virtual void __stdcall SetStiffnessAndDampingRatio(tF32 afKs, tF32 afE) = 0;

  //! Set the spring's damping ratio.
  //! {Property}
  //! \remark Sets Kd in function of the current value of Ks to achieve the specified damping ratio.
  virtual void __stdcall SetDampingRatio(tF32 afE) = 0;
  //! Get the spring's damping ratio.
  //! {Property}
  virtual tF32 __stdcall GetDampingRatio() const = 0;

  //! Set the spring's velocity.
  //! {Property}
  virtual void __stdcall SetVelocity(const sVec3f& avVel) = 0;
  //! Get the spring's velocity.
  //! {Property}
  virtual sVec3f __stdcall GetVelocity() const = 0;

  //! Compute the spring's acceleration for the specified displacement.
  virtual sVec3f __stdcall ComputeAcceleration(const sVec3f& avD) = 0;
  //! Update the spring's velocity from the specified displacement.
  virtual void __stdcall UpdateVelocity(ni::tF32 afDeltaTime, const sVec3f& avD) = 0;
  //! Update the spring's velocity from the specified acceleration.
  virtual void __stdcall UpdateVelocityWithAcceleration(ni::tF32 afDeltaTime, const sVec3f& avAcc) = 0;
};

//! 4D Damped spring interface.
//! \see ni::DampedSpringAcceleration
struct iDampedSpring4 : public iUnknown
{
  niDeclareInterfaceUUID(iDampedSpring4,0xc37f3b6d,0x6bb6,0x4036,0xae,0x64,0x80,0xf4,0x3c,0x87,0xc7,0xb2)

  //! Set the Kd constant.
  //! {Property}
  virtual void __stdcall SetKd(tF32 afD) = 0;
  //! Get the Kd constant.
  //! {Property}
  virtual tF32 __stdcall GetKd() const = 0;
  //! Set the Ks constant.
  //! {Property}
  virtual void __stdcall SetKs(tF32 afD) = 0;
  //! Get the Ks constant.
  //! {Property}
  virtual tF32 __stdcall GetKs() const = 0;

  //! Set the value of Kd for the specified Ks that will result in a spring with the specified damping ratio.
  //! \remark Sets Ks with parameter's value, sets Kd in function of Ks.
  virtual void __stdcall SetStiffnessAndDampingRatio(tF32 afKs, tF32 afE) = 0;

  //! Set the spring's damping ratio.
  //! {Property}
  //! \remark Sets Kd in function of the current value of Ks to achieve the specified damping ratio.
  virtual void __stdcall SetDampingRatio(tF32 afE) = 0;
  //! Get the spring's damping ratio.
  //! {Property}
  virtual tF32 __stdcall GetDampingRatio() const = 0;

  //! Set the spring's velocity.
  //! {Property}
  virtual void __stdcall SetVelocity(const sVec4f& avVel) = 0;
  //! Get the spring's velocity.
  //! {Property}
  virtual sVec4f __stdcall GetVelocity() const = 0;

  //! Compute the spring's acceleration for the specified displacement.
  virtual sVec4f __stdcall ComputeAcceleration(const sVec4f& avD) = 0;
  //! Update the spring's velocity from the specified displacement.
  virtual void __stdcall UpdateVelocity(ni::tF32 afDeltaTime, const sVec4f& avD) = 0;
  //! Update the spring's velocity from the specified acceleration.
  virtual void __stdcall UpdateVelocityWithAcceleration(ni::tF32 afDeltaTime, const sVec4f& avAcc) = 0;
};

//! 1D Damped spring position interface.
struct iDampedSpringPosition1 : public iDampedSpring1
{
  niDeclareInterfaceUUID(iDampedSpringPosition1,0x58b648fe,0x1cc2,0x4398,0xb9,0x11,0x3c,0x10,0x9f,0x12,0xcc,0xe0)

  //! Set the ideal (target) position of the spring.
  //! {Property}
  virtual void __stdcall SetIdealPosition(tF32 avPos) = 0;
  //! Get the ideal (target) position of the spring.
  //! {Property}
  virtual tF32 __stdcall GetIdealPosition() const = 0;
  //! Set the current position of the spring.
  //! {Property}
  virtual void __stdcall SetCurrentPosition(tF32 avPos) = 0;
  //! Get the current position of the spring.
  //! {Property}
  virtual tF32 __stdcall GetCurrentPosition() const = 0;
  //! Update the current position of the spring.
  //! \return The current position of the spring.
  virtual tF32 __stdcall UpdatePosition(ni::tF32 afDeltaTime) = 0;

  //! Set the maximum update step size.
  //! {Property}
  //! \remark Default is 1.0/50.0
  //! \remark If the step is zero the value is updated at once in UpdatePosition (effectivly disable step update)
  virtual void __stdcall SetStep(tF32 afD) = 0;
  //! Get the update step size.
  //! {Property}
  virtual tF32 __stdcall GetStep() const = 0;
  //! Set the animation speed.
  //! {Property}
  //! \remark This value just multiplies afDeltaTime to ariticially speed up the spring's animation.
  //! \remark Default is 1.0.
  virtual void __stdcall SetSpeed(tF32 afD) = 0;
  //! Get the animation speed.
  //! {Property}
  virtual tF32 __stdcall GetSpeed() const = 0;
  //! Set the end threshold.
  //! {Property}
  //! \remark Default is 1e-3f
  virtual void __stdcall SetEndThreshold(tF32 afD) = 0;
  //! Get the end threshold.
  //! {Property}
  virtual tF32 __stdcall GetEndThreshold() const = 0;
  //! Get the whether the distance between the ideal and the current positions are below the end threshold.
  //! {Property}
  virtual tBool __stdcall GetIsEnded() const = 0;
};

//! 2D Damped spring position interface.
struct iDampedSpringPosition2 : public iDampedSpring2
{
  niDeclareInterfaceUUID(iDampedSpringPosition2,0xa842b5c6,0x3331,0x4a8a,0x81,0xeb,0xf5,0xf1,0x46,0xf5,0xaa,0xc2)

  //! Set the ideal (target) position of the spring.
  //! {Property}
  virtual void __stdcall SetIdealPosition(const sVec2f& avPos) = 0;
  //! Get the ideal (target) position of the spring.
  //! {Property}
  virtual sVec2f __stdcall GetIdealPosition() const = 0;
  //! Set the current position of the spring.
  //! {Property}
  virtual void __stdcall SetCurrentPosition(const sVec2f& avPos) = 0;
  //! Get the current position of the spring.
  //! {Property}
  virtual sVec2f __stdcall GetCurrentPosition() const = 0;
  //! Update the current position of the spring.
  //! \return The current position of the spring.
  virtual sVec2f __stdcall UpdatePosition(ni::tF32 afDeltaTime) = 0;

  //! Set the maximum update step size.
  //! {Property}
  //! \remark Default is 1.0/50.0
  //! \remark If the step is zero the value is updated at once in UpdatePosition (effectivly disable step update)
  virtual void __stdcall SetStep(tF32 afD) = 0;
  //! Get the update step size.
  //! {Property}
  virtual tF32 __stdcall GetStep() const = 0;
  //! Set the animation speed.
  //! {Property}
  //! \remark This value just multiplies afDeltaTime to ariticially speed up the spring's animation.
  //! \remark Default is 1.0.
  virtual void __stdcall SetSpeed(tF32 afD) = 0;
  //! Get the animation speed.
  //! {Property}
  virtual tF32 __stdcall GetSpeed() const = 0;
  //! Set the end threshold.
  //! {Property}
  //! \remark Default is 1e-3f
  virtual void __stdcall SetEndThreshold(tF32 afD) = 0;
  //! Get the end threshold.
  //! {Property}
  virtual tF32 __stdcall GetEndThreshold() const = 0;
  //! Get the whether the distance between the ideal and the current positions are below the end threshold.
  //! {Property}
  virtual tBool __stdcall GetIsEnded() const = 0;
};

//! 3D Damped spring position interface.
struct iDampedSpringPosition3 : public iDampedSpring3
{
  niDeclareInterfaceUUID(iDampedSpringPosition3,0x1fb0297e,0x3d7d,0x4f26,0xa6,0xd3,0x07,0x83,0x13,0x10,0x46,0x1d)

  //! Set the ideal (target) position of the spring.
  //! {Property}
  virtual void __stdcall SetIdealPosition(const sVec3f& avPos) = 0;
  //! Get the ideal (target) position of the spring.
  //! {Property}
  virtual sVec3f __stdcall GetIdealPosition() const = 0;
  //! Set the current position of the spring.
  //! {Property}
  virtual void __stdcall SetCurrentPosition(const sVec3f& avPos) = 0;
  //! Get the current position of the spring.
  //! {Property}
  virtual sVec3f __stdcall GetCurrentPosition() const = 0;
  //! Update the current position of the spring.
  //! \return The current position of the spring.
  virtual sVec3f __stdcall UpdatePosition(ni::tF32 afDeltaTime) = 0;

  //! Set the maximum update step size.
  //! {Property}
  //! \remark Default is 1.0/50.0
  //! \remark If the step is zero the value is updated at once in UpdatePosition (effectivly disable step update)
  virtual void __stdcall SetStep(tF32 afD) = 0;
  //! Get the update step size.
  //! {Property}
  virtual tF32 __stdcall GetStep() const = 0;
  //! Set the animation speed.
  //! {Property}
  //! \remark This value just multiplies afDeltaTime to ariticially speed up the spring's animation.
  //! \remark Default is 1.0.
  virtual void __stdcall SetSpeed(tF32 afD) = 0;
  //! Get the animation speed.
  //! {Property}
  virtual tF32 __stdcall GetSpeed() const = 0;
  //! Set the end threshold.
  //! {Property}
  //! \remark Default is 1e-3f
  virtual void __stdcall SetEndThreshold(tF32 afD) = 0;
  //! Get the end threshold.
  //! {Property}
  virtual tF32 __stdcall GetEndThreshold() const = 0;
  //! Get the whether the distance between the ideal and the current positions are below the end threshold.
  //! {Property}
  virtual tBool __stdcall GetIsEnded() const = 0;
};

//! 4D Damped spring position interface.
struct iDampedSpringPosition4 : public iDampedSpring4
{
  niDeclareInterfaceUUID(iDampedSpringPosition4,0x466c694d,0x0e04,0x47ee,0x8a,0x39,0x2e,0xd3,0x1f,0x0f,0xc3,0x8c)

  //! Set the ideal (target) position of the spring.
  //! {Property}
  virtual void __stdcall SetIdealPosition(const sVec4f& avPos) = 0;
  //! Get the ideal (target) position of the spring.
  //! {Property}
  virtual sVec4f __stdcall GetIdealPosition() const = 0;
  //! Set the current position of the spring.
  //! {Property}
  virtual void __stdcall SetCurrentPosition(const sVec4f& avPos) = 0;
  //! Get the current position of the spring.
  //! {Property}
  virtual sVec4f __stdcall GetCurrentPosition() const = 0;
  //! Update the current position of the spring.
  //! \return The current position of the spring.
  virtual sVec4f __stdcall UpdatePosition(ni::tF32 afDeltaTime) = 0;

  //! Set the maximum update step size.
  //! {Property}
  //! \remark Default is 1.0/50.0
  //! \remark If the step is zero the value is updated at once in UpdatePosition (effectivly disable step update)
  virtual void __stdcall SetStep(tF32 afD) = 0;
  //! Get the maximum update step size.
  //! {Property}
  virtual tF32 __stdcall GetStep() const = 0;
  //! Set the animation speed.
  //! {Property}
  //! \remark This value just multiplies afDeltaTime to ariticially speed up the spring's animation.
  //! \remark Default is 1.0.
  virtual void __stdcall SetSpeed(tF32 afD) = 0;
  //! Get the animation speed.
  //! {Property}
  virtual tF32 __stdcall GetSpeed() const = 0;
  //! Set the end threshold.
  //! {Property}
  //! \remark Default is 1e-3f
  virtual void __stdcall SetEndThreshold(tF32 afD) = 0;
  //! Get the end threshold.
  //! {Property}
  virtual tF32 __stdcall GetEndThreshold() const = 0;
  //! Get the whether the distance between the ideal and the current positions are below the end threshold.
  //! {Property}
  virtual tBool __stdcall GetIsEnded() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IDAMPEDSPRING_19269492_H__
