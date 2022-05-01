#ifndef __OSXGL_895464_NARF_123_H__
#define __OSXGL_895464_NARF_123_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../../IOSWindow.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_OSX
 * @{
 */

struct sOSXGLConfig {
  sOSXGLConfig() {
    ni::MemZero((tPtr)this,sizeof(sOSXGLConfig));
    colorBits = 24;
    alphaBits = 8;
    depthBits = 24;
    stencilBits = 8;
    glMajor = 2;
    glMinor = 0;
    swapInterval = 1;
  }

  int colorBits;
  int alphaBits;
  int depthBits;
  int stencilBits;
  int accumBits;
  int auxBuffers;
  int aaSamples;
  int stereo;
  int glMajor;
  int glMinor;
  tU32 swapInterval;
  iOSWindow* share;
};

niExportFunc(tBool) osxglIsStarted();
niExportFunc(tBool) osxglStartup(void);
niExportFunc(void) osxglShutdown(void);
niExportFunc(tBool) osxglCreateContext(iOSWindow* apWindow, sOSXGLConfig* nsglConfig);
niExportFunc(void) osxglDestroyContext(iOSWindow* apWindow);
niExportFunc(tBool) osxglHasContext(iOSWindow* apWindow);
niExportFunc(void) osxglMakeContextCurrent(iOSWindow* apWindow);
niExportFunc(iOSWindow*) osxglGetCurrentContext(void);
niExportFunc(void) osxglSwapBuffers(iOSWindow* apWindow, tBool abDoNotWait);
niExportFunc(tPtr) osxglGetProcAddress(const char* procname);
niExportFunc(tIntPtr) osxglGetNSOpenGLContext(iOSWindow* apWindow);
niExportFunc(void) osxglUpdateDisplayLinkDisplay(iOSWindow* apWindow);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // namespace ni { namespace niOSX {
#endif // __OSXGL_895464_NARF_123_H__
