#ifndef __TIMEDLERP_47937924_H__
#define __TIMEDLERP_47937924_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

template <typename TimeT, typename ItemT>
struct sTimedLerp_GetKeyTimeStaticCast {
  static TimeT GetKeyTime(const ItemT& e) {
    return static_cast<const TimeT&>(e);
  }
};

template <typename ContainerT, typename TimeT, typename GetKeyTimeT>
struct sTimedLerp
{
  static inline void GetIndices(const ContainerT& avKeys, TimeT afTime, tU32& aA, tU32& aB, TimeT& afFactor)
  {
    const tU32 ulNumKey = (tU32)avKeys.size();
    if (ulNumKey < 2) {
      aA = aB = 0; afFactor = 0.0f;
      return;
    }

    TimeT fFrameTime = GetKeyTimeT::GetKeyTime(avKeys[1])-GetKeyTimeT::GetKeyTime(avKeys[0]);
    TimeT fCount = afTime;
    tU32 i;
    for(i = 0; i <= ulNumKey; ++i) {
      fCount -= fabsf(GetKeyTimeT::GetKeyTime(avKeys[(i+1)%ulNumKey])-GetKeyTimeT::GetKeyTime(avKeys[i%ulNumKey]));
      if (fCount <= 0.0f)
        break;
    }

    aA = i;
    if (aA >= ulNumKey)
      aA = ulNumKey-1;

    aB = (aA+1);
    if (aB >= ulNumKey)
      aB = ulNumKey-1;

    if (aA != aB) {
      fFrameTime = GetKeyTimeT::GetKeyTime(avKeys[aB]) - GetKeyTimeT::GetKeyTime(avKeys[aA]);
      afFactor = (fFrameTime+fCount)/fFrameTime;
    }
    else {
      afFactor = 0.0f;
    }
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __TIMEDLERP_47937924_H__
