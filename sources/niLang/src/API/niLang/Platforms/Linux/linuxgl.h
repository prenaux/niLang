#pragma once
#ifndef __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
#define __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#ifdef niLinuxDesktop

// Forward declarations of X11 types with different names to avoid
// conflicts. (Note: Must be declared in the global namespace)
typedef struct _XDisplay* tXWinDisplay;
typedef struct _XVisual* tXWinVisual;
typedef unsigned long tXWinWindow;
typedef struct _XGC* tXWinGC;

namespace ni {

struct iOSWindow;

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Linux
 * @{
 */

struct sOSWindowXWinHandles {
  tXWinDisplay _display;
  tXWinVisual _visual;
  tXWinWindow _window;
  tXWinGC _gc;
  int _screen;
};

niExportFunc(tBool) linuxGetOSWindowXWinHandles(iOSWindow* apWindow, sOSWindowXWinHandles& aOut);
niExportFunc(tBool) linuxglCreateContext(iOSWindow* apWindow);
niExportFunc(tBool) linuxglDestroyContext(iOSWindow* apWindow);
niExportFunc(tBool) linuxglHasContext(iOSWindow* apWindow);
niExportFunc(tBool) linuxglMakeContextCurrent(iOSWindow* apWindow);
niExportFunc(tBool) linuxglSwapBuffers(iOSWindow* apWindow, tBool abDoNotWait);
niExportFunc(void*) linuxglGetProcAddress(const achar* name);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // namespace ni {

#endif // #ifdef niLinuxDesktop

#endif // __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
