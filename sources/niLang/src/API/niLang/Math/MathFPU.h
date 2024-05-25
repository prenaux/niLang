#ifndef __MATHFPU_44024344_H__
#define __MATHFPU_44024344_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

niExportFunc(niNamespace(ni,tU32)) FPUMode_Get();
niExportFunc(void) FPUMode_Set(niNamespace(ni,tU32) aMode);

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

//! FPU precision
enum eFPUPrecision {
  eFPUPrecision_24 = 0,
  eFPUPrecision_53 = 512,
  eFPUPrecision_64 = 512|256,
  eFPUPrecision_Mask = 512|256,
  eFPUPrecision_Lowest = eFPUPrecision_24,
  eFPUPrecision_Highest = eFPUPrecision_53,
  eFPUPrecision_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! FPU rounding
enum eFPURounding {
  eFPURounding_Near = 0,
  eFPURounding_Down = 1024,
  eFPURounding_Up = 2048,
  eFPURounding_Truncate = 2048|1024,
  eFPURounding_Mask = 2048|1024,
  eFPURounding_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Utility call to control the x87 FPU modes
struct FPUMode
{
 public:
  FPUMode(eFPUPrecision p) : mnPrevMode(Get()), mbChanged(eFalse) {
    ni::tU32 tmp = (mnPrevMode & ~eFPUPrecision_Mask) | (ni::tU32)(p);
    if (tmp != mnPrevMode) {
      Set(tmp);
      mbChanged = eTrue;
    }
  }
  FPUMode(eFPURounding r) : mnPrevMode(Get()), mbChanged(eFalse) {
    ni::tU32 tmp = (mnPrevMode & ~eFPURounding_Mask) | (ni::tU32)(r);
    if (tmp != mnPrevMode) {
      Set(tmp);
      mbChanged = eTrue;
    }
  }
  FPUMode(eFPUPrecision p, eFPURounding r) : mnPrevMode(Get()), mbChanged(eFalse) {
    ni::tU32 tmp = (mnPrevMode & ~(eFPUPrecision_Mask|eFPURounding_Mask)) | (ni::tU32)(r) | (ni::tU32)(p);
    if (tmp != mnPrevMode) {
      Set(tmp);
      mbChanged = eTrue;
    }
  }
  ~FPUMode() {
    if (mbChanged) Set(mnPrevMode);
  }

  static ni::tU32  Get() {
    return FPUMode_Get();
  }
  static void        Set(ni::tU32 aMode) {
    FPUMode_Set(aMode);
  }

  static void SetRounding(eFPURounding r) {
    Set((Get() & ~eFPURounding_Mask) | (ni::tU32)(r));
  }
  static eFPURounding  GetRounding() {
    return (eFPURounding)(Get() & eFPURounding_Mask);
  }
  static void SetPrecision(eFPUPrecision p) {
    Set((Get() & ~eFPUPrecision_Mask) | (ni::tU32)(p));
  }
  static eFPUPrecision GetPrecision() {
    return (eFPUPrecision)(Get() & eFPUPrecision_Mask);
  }

 private:
  FPUMode   (const FPUMode&);
  FPUMode&          operator= (const FPUMode&);

  ni::tU32          mnPrevMode;
  ni::tBool         mbChanged;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHFPU_44024344_H__
