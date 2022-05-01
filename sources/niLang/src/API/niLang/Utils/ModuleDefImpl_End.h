#ifndef __MODULEDEFIMPL_END_329018490_H__
#define __MODULEDEFIMPL_END_329018490_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#ifdef niMSVC
#include "../Platforms/Win32/Win32_DelayLoadImpl.h"
#endif

#ifndef _DEBUG

#ifdef niConfig_MinModuleDefSize
#ifdef niMSVC
// restore default optimizations
#pragma optimize( "", on )
#endif
#endif

#endif

#ifdef ANDROID
// From Android's doc:
// If your project is linked with "-nostdlib -Wl,--no-undefined", you need to
// provide your own __dso_handle because crtbegin_so.o isn't linked.  The
// content of __dso_handle doesn't matter.
//
// ... why not ^^ ...
extern void *__dso_handle __attribute__((__visibility__ ("hidden")));
void* __dso_handle;
#endif

/**@}*/
/**@}*/

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __MODULEDEFIMPL_END_329018490_H__
