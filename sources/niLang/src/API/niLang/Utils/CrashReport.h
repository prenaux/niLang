#ifndef __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
#define __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#if defined niNoCrashReport || !defined __cplusplus

#define niCrashReport_DeclareHandler()

#else

#ifdef niLinux
#define BACKWARD_HAS_DW 1
#endif

#ifdef niWindows
#include <niLang/Platforms/Win32/Win32_Redef.h>
#endif

#include "backward.hpp"

#define niCrashReport_DeclareHandler() backward::SignalHandling _niCrashReport_sh;

#endif

/**@}*/
/**@}*/
#endif // __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
