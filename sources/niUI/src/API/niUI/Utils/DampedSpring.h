#ifndef __DAMPEDSPRING_19269492_H__
#define __DAMPEDSPRING_19269492_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../IDampedSpring.h"
#include <niLang/Math/MathVec2.h>
#include <niLang/Math/MathVec3.h>
#include <niLang/Math/MathVec4.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

static const tF32 kfDampedSpringUpdateDefaultTimeStep = 1.0f/50.0f;

//! Computes the acceleration of a damped spring system.
//!
//! The spring acceleration is applied in the direction opposite from its
//! displacement from a rest position, and a damping acceleration is applied in
//! the direction opposite from its velocity.
//! The damping ratio is : E = Kd/(2*sqrt(Ks)).
//! - When E=1, the spring system is critically damped, and the system will return
//!   to its rest position in the minimal possibile time for the given value of
//!   Ks.
//! - When E < 1, the spring system is underdamped, and it will oscillate before
//!   returning to rest.
//! - When E > 1, then the system is overdamped and it will take longer than it is
//!   necessary to reach equilibrum.
//!
//! \param D is the displacement of the spring/object
//! \param V is the current velocity
//! \param Ks is the spring acceleration constant, control the spring
//!        component of the acceleration - the stiffness of the spring (the
//!        higher the value the faster the spring will reach its target).
//! \param Kd is the damping acceleration constant, control the damping
//!        component of the acceleration.
template <typename T, typename TF>
T DampedSpringAcceleration(const T& D, const T& V, const TF Ks, const TF Kd) {
  return (D * -Ks) - (V * Kd);
}

//! Get the damping ratio of a spring set with the specified Ks/Kd constants.
template <typename T>
T DampedSpringGetDampingRatio(const T Ks, const T Kd) {
  return Kd / (T(2.0)*ni::Sqrt(Ks));
}

//! Compute the value of Kd for the specified Ks that will result in a spring with the specified damping ratio.
template <typename T>
T DampedSpringComputeKdFromDampingRatio(const T Ks, const T E) {
  return (T(2.0)*ni::Sqrt(Ks))*E;
}

//! Damped spring helper template.
template <typename T, typename TF, typename BASE = ni::cUnknown, typename TREF=const T&>
struct sDampedSpring : public BASE
{
  T mvVel;
  TF  mKs, mKd;

  sDampedSpring() {
    ni::MemZero((tPtr)&mvVel,sizeof(mvVel));
    SetStiffnessAndDampingRatio(10.0f,1.0f);
  }

  void __stdcall SetKd(TF aKd) {
    mKd = aKd;
  }
  TF __stdcall GetKd() const {
    return mKd;
  }
  void __stdcall SetKs(TF aKs) {
    mKs = aKs;
  }
  TF __stdcall GetKs() const {
    return mKs;
  }

  void __stdcall SetStiffnessAndDampingRatio(ni::tF32 aKs, ni::tF32 E) {
    SetKs(aKs);
    SetKd(DampedSpringComputeKdFromDampingRatio(aKs,E));
  }

  void __stdcall SetDampingRatio(ni::tF32 E) {
    SetStiffnessAndDampingRatio(mKs,E);
  }
  TF __stdcall GetDampingRatio() const {
    return DampedSpringGetDampingRatio(mKs,mKd);
  }

  void __stdcall SetVelocity(TREF avVel) {
    mvVel = avVel;
  }
  T __stdcall GetVelocity() const {
    return mvVel;
  }

  T __stdcall ComputeAcceleration(TREF avD) {
    return DampedSpringAcceleration(avD, mvVel, mKs, mKd);
  }

  void __stdcall UpdateVelocity(ni::tF32 afDeltaTime, TREF avD) {
    TF time = afDeltaTime;
    while (time > kfDampedSpringUpdateDefaultTimeStep) {
      T vSpringAcc = DampedSpringAcceleration(avD, mvVel, mKs, mKd);
      mvVel += vSpringAcc * kfDampedSpringUpdateDefaultTimeStep;
      time -= kfDampedSpringUpdateDefaultTimeStep;
    }
    if (time > 0.0f) {
      T vSpringAcc = DampedSpringAcceleration(avD, mvVel, mKs, mKd);
      mvVel += vSpringAcc * time;
    }
  }
  void __stdcall UpdateVelocityWithAcceleration(ni::tF32 afDeltaTime, TREF avAcc) {
    TF time = afDeltaTime;
    while (time > kfDampedSpringUpdateDefaultTimeStep) {
      mvVel += avAcc * kfDampedSpringUpdateDefaultTimeStep;
      time -= kfDampedSpringUpdateDefaultTimeStep;
    }
    if (time > 0.0f) {
      mvVel += avAcc * time;
    }
  }
};

//! 1D spring.
typedef sDampedSpring<tF32,tF32,cUnknown,tF32>              sDampedSpring1f;
//! 1D spring, implements the ni::iDampedSpring1 interface.
typedef sDampedSpring<tF32,tF32,ImplRC<iDampedSpring1>,tF32 >  sDampedSpring1fImpl;
//! 2D spring.
typedef sDampedSpring<sVec2f,tF32>                 sDampedSpring2f;
//! 2D spring, implements the ni::iDampedSpring2 interface.
typedef sDampedSpring<sVec2f,tF32,ImplRC<iDampedSpring2> >  sDampedSpring2fImpl;
//! 3D spring.
typedef sDampedSpring<sVec3f,tF32>                 sDampedSpring3f;
//! 3D spring, implements the ni::iDampedSpring3 interface.
typedef sDampedSpring<sVec3f,tF32,ImplRC<iDampedSpring3> >  sDampedSpring3fImpl;
//! 4D spring.
typedef sDampedSpring<sVec4f,tF32>                 sDampedSpring4f;
//! 4D spring, implements the ni::iDampedSpring4 interface.
typedef sDampedSpring<sVec4f,tF32,ImplRC<iDampedSpring4> >  sDampedSpring4fImpl;

//! Integrate one frame of the spring simulation using a simple euler method.
template <typename T, typename TF>
void DampedSpringIntegrateFrameEuler(T& aP, T& aV, const T& I, const TF Ks, const TF Kd, const TF dT) {
  const T acc = ni::DampedSpringAcceleration(aP-I,aV,Ks,Kd);
  aV += acc * dT;
  aP += aV * dT;
}

template <typename T>
inline tF32 _DampedSpringDistance(const T& a, const T&b) { niCAssert(-1); return -1; /*error*/ }

template <>
inline tF32 _DampedSpringDistance<tF32>(const tF32& a, const tF32& b) {
  return ni::Abs(a-b);
}
template <>
inline tF32 _DampedSpringDistance<sVec2f>(const sVec2f& a, const sVec2f& b) {
  return ni::VecLength(a-b);
}
template <>
inline tF32 _DampedSpringDistance<sVec3f>(const sVec3f& a, const sVec3f& b) {
  return ni::VecLength(a-b);
}
template <>
inline tF32 _DampedSpringDistance<sVec4f>(const sVec4f& a, const sVec4f& b) {
  return ni::VecLength(a-b);
}

//! Damped spring with position helper template.
template <typename T, typename TF, typename BASE = ni::cUnknown, typename TREF = const T&>
struct sDampedSpringPosition : public sDampedSpring<T,TF,BASE,TREF> {
  T   mvIdealPos;
  T   mvCurrentPos;
  tF32  mfEndThreshold;
  tF32  mfStep;
  tF32  mfSpeed;

  sDampedSpringPosition() {
    ni::MemZero((tPtr)&mvIdealPos,sizeof(mvIdealPos));
    ni::MemZero((tPtr)&mvCurrentPos,sizeof(mvCurrentPos));
    mfEndThreshold = niEpsilon3;
    mfStep = kfDampedSpringUpdateDefaultTimeStep;
    mfSpeed = 1.0f;
  }

  void __stdcall SetSpeed(tF32 afSpeed) {
    mfSpeed = afSpeed;
  }
  tF32 __stdcall GetSpeed() const {
    return mfSpeed;
  }

  void __stdcall SetStep(tF32 afStep) {
    mfStep = afStep;
  }
  tF32 __stdcall GetStep() const {
    return mfStep;
  }

  void __stdcall SetIdealPosition(TREF avPos) {
    mvIdealPos = avPos;
  }
  T __stdcall GetIdealPosition() const {
    return mvIdealPos;
  }
  void __stdcall SetCurrentPosition(TREF avPos) {
    mvCurrentPos = avPos;
  }
  T __stdcall GetCurrentPosition() const {
    return mvCurrentPos;
  }

  void __stdcall SetEndThreshold(tF32 afEndThreshold) {
    mfEndThreshold = afEndThreshold;
  }
  tF32 __stdcall GetEndThreshold() const {
    return mfEndThreshold;
  }

  tBool __stdcall GetIsEnded() const {
    tF32 fD = _DampedSpringDistance(mvIdealPos,mvCurrentPos);
    return (fD < mfEndThreshold);
  }

  T __stdcall UpdatePosition(ni::tF32 afDeltaTime) {
    if (this->mKs == 0.0f || this->mKd == 0.0f) {
      mvCurrentPos = mvIdealPos;
    }
    else {
      tF32 fUpdateTime = afDeltaTime*mfSpeed;
      if (!niFloatIsZero(mfStep)) {
        while (fUpdateTime > mfStep) {
          DampedSpringIntegrateFrameEuler(mvCurrentPos,this->mvVel,mvIdealPos,
                                          this->mKs,this->mKd,mfStep);
          fUpdateTime -= mfStep;
        }
      }
      if (fUpdateTime > 0.0) {
        DampedSpringIntegrateFrameEuler(mvCurrentPos,this->mvVel,mvIdealPos,
                                        this->mKs,this->mKd,fUpdateTime);
      }
    }
    return mvCurrentPos;
  }
};

//! 1D spring position.
typedef sDampedSpringPosition<tF32,tF32,cUnknown,tF32>              sDampedSpringPosition1f;
//! 1D spring position, implements the ni::iDampedSpringPosition1 interface.
typedef sDampedSpringPosition<tF32,tF32,ImplRC<iDampedSpringPosition1,eImplFlags_DontInherit1,iDampedSpring1>,tF32 > sDampedSpringPosition1fImpl;
//! 2D spring position.
typedef sDampedSpringPosition<sVec2f,tF32>                 sDampedSpringPosition2f;
//! 2D spring position, implements the ni::iDampedSpringPosition2 interface.
typedef sDampedSpringPosition<sVec2f,tF32,ImplRC<iDampedSpringPosition2,eImplFlags_DontInherit1,iDampedSpring2> > sDampedSpringPosition2fImpl;
//! 3D spring position.
typedef sDampedSpringPosition<sVec3f,tF32>                 sDampedSpringPosition3f;
//! 3D spring position, implements the ni::iDampedSpringPosition3 interface.
typedef sDampedSpringPosition<sVec3f,tF32,ImplRC<iDampedSpringPosition3,eImplFlags_DontInherit1,iDampedSpring3> > sDampedSpringPosition3fImpl;
//! 4D spring position.
typedef sDampedSpringPosition<sVec4f,tF32>                 sDampedSpringPosition4f;
//! 4D spring position, implements the ni::iDampedSpringPosition4 interface.
typedef sDampedSpringPosition<sVec4f,tF32,ImplRC<iDampedSpringPosition4,eImplFlags_DontInherit1,iDampedSpring4> > sDampedSpringPosition4fImpl;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __DAMPEDSPRING_19269492_H__
