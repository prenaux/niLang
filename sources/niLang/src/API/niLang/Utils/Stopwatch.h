#pragma once
#ifndef __STOPWATCH_H_B513D9A2_BFC8_354D_854E_B34E715D58C4__
#define __STOPWATCH_H_B513D9A2_BFC8_354D_854E_B34E715D58C4__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

struct sStopwatch {
  sStopwatch() {
    Start();
  }
  sStopwatch(tF32 afStart) : mfStart(afStart) {
  }
  void Start() {
    mfStart = ni::TimerInSeconds();
  }
  tF64 GetStart() const {
    return mfStart;
  }
  tF64 GetSeconds() const {
    return ni::TimerInSeconds() - mfStart;
  }
  tI64 GetMs() const {
    return (tI64)(GetSeconds() * 1000.0);
  }

private:
  tF64 mfStart;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __STOPWATCH_H_B513D9A2_BFC8_354D_854E_B34E715D58C4__
