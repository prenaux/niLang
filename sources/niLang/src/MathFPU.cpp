// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Math/MathFPU.h"

using namespace ni;

#if defined niJSCC ||                           \
  defined niAndroid ||                          \
  defined niMingW32 ||                          \
  defined niOSX ||                              \
  defined niIOS ||                              \
  defined niSuperH ||                           \
  defined niARM ||                              \
  defined niX64
#define _GENERIC_FPU
#endif

#if defined _GENERIC_FPU

static ni::tU32 _fpuControl = ni::eFPUPrecision_53|ni::eFPURounding_Near;
niExportFunc(niNamespace(ni,tU32)) FPUMode_Get() {
  return _fpuControl;
}
niExportFunc(void) FPUMode_Set(niNamespace(ni,tU32) aMode) {
  _fpuControl = aMode;
}

#elif defined niLinux && defined niIntelX86

#include <fpu_control.h>

niExportFunc(niNamespace(ni,tU32)) FPUMode_Get() {
  fpu_control_t tmp;
  _FPU_GETCW(tmp);
  return tmp;
}
niExportFunc(void) FPUMode_Set(niNamespace(ni,tU32) aMode) {
  fpu_control_t tmp = aMode;
  _FPU_SETCW(tmp);
}

#elif defined niWindows && defined niX86
niExportFunc(niNamespace(ni,tU32)) FPUMode_Get() {
  unsigned int tmp;
  __asm {
    fwait ;
    fstcw dword ptr [tmp] ;
  }
  return tmp;
}
niExportFunc(void) FPUMode_Set(niNamespace(ni,tU32) aMode) {
  __asm {
    fwait ;
    fldcw dword ptr [aMode] ;
  }
}

#else

#error FPU control not implemented on this platform

#endif
