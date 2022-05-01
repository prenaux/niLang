#ifndef __MODULEUTILS_876945646_H__
#define __MODULEUTILS_876945646_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../StringBase.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#if defined niDebug
#define niModuleFileNameBuildType "da"
#else
#define niModuleFileNameBuildType "ra"
#endif

///////////////////////////////////////////////
#ifndef niNoDLL
inline cString __stdcall GetModuleFileName(const achar* aaszName, const achar* aaszExt = niDLLSuffix, const achar* aBuildType = niModuleFileNameBuildType) {
  cString ret;

  // Prefix
#if !defined niWindows
  ret += "lib";
#endif

  // Module name
  ret += aaszName;

  // Build type
  ret += _A("_");
  ret += aBuildType;

  // Extension
  if (niStringIsOK(aaszExt)) {
    ret += aaszExt;
  }

  return ret;
}
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __CORELIBSYSTEMEMUIMPL_27946974_H__
