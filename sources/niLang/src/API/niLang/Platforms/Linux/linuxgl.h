#pragma once
#ifndef __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
#define __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#ifdef niLinuxDesktop

namespace ni {

struct iOSWindow;

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_OSX
 * @{
 */

niExportFunc(tBool) linuxglCreateContext(iOSWindow* apWindow);
niExportFunc(tBool) linuxglDestroyContext(iOSWindow* apWindow);
niExportFunc(tBool) linuxglHasContext(iOSWindow* apWindow);
niExportFunc(tBool) linuxglMakeContextCurrent(iOSWindow* apWindow);
niExportFunc(tBool) linuxglSwapBuffers(iOSWindow* apWindow, tBool abDoNotWait);
niExportFunc(void*) linuxglGetProcAddress(const achar* name);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // namespace ni { namespace niOSX {

#endif // #ifdef niLinuxDesktop

#endif // __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
