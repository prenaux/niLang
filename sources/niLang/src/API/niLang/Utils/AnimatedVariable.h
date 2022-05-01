#ifndef __ANIMATEDVARIABLE_7008758_H__
#define __ANIMATEDVARIABLE_7008758_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

template <typename T, typename TIMET = ni::tF32>
class sAnimatedVariable
{
 public:
  sAnimatedVariable()
  {
    mfVar = 0;
    mfTime = 0;
    mfTimeTarget = 0;
  }

  void SetTarget(T fTarget, TIMET fTime)
  {
    mfBase = mfVar;
    mfTime = 0.0f;
    mfTarget = fTarget;
    mfDelta = mfTarget - mfVar;
    mfTimeTarget = mfTime + fTime;
  }

  void SetValue(T fVal)
  {
    mfVar = fVal;
    mfTarget = fVal;
    mfTime = TIMET(0.0);
    mfTimeTarget = TIMET(0.0f);
  }

  const T& GetValue() const
  {
    return mfVar;
  }

  void Update(TIMET fDeltaTime)
  {
    mfTime += fDeltaTime;
    mfVar = mfBase + (mfTime / mfTimeTarget) * mfDelta;
    if (GetHasReachedTarget()) {
      mfVar = mfTarget;
    }
  }

  ni::tBool GetHasReachedTarget() const
  {
    return (mfTime >= mfTimeTarget);
  }

 private:
  T mfBase;
  T   mfVar;
  T   mfTarget;
  T   mfDelta;
  TIMET   mfTime;
  TIMET   mfTimeTarget;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __ANIMATEDVARIABLE_7008758_H__
