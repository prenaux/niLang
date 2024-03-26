#pragma once
#ifndef __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
#define __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef niLinuxDesktop
#define GL_GLEXT_PROTOTYPES
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glext.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_OSX
 * @{
 */

struct sXWindowDesc {

  Display* mpDisplay;
  Window   mpWindow;
  GLXContext mpGLX;

  XVisualInfo* mXVI;
  XSetWindowAttributes mXSWA;

  void Create(sVec2i aSize) {
    mpDisplay = XOpenDisplay(NULL);
    if (mpDisplay == NULL) {
      printf("\n\tcannot connect to X server\n\n");
      return;
    }

    // XVisualInfo
    GLint attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    mXVI = glXChooseVisual(mpDisplay, 0, attr);
    if (mXVI == NULL) {
      printf("\n\tno appropriate visual found\n\n");
      return;
    }
    else {
      printf("\n\tvisual %p selected\n", (void *)mXVI->visualid); /* %p creates hexadecimal output like in glxinfo */
    }


    Window rootWindow = DefaultRootWindow(mpDisplay);

    // XWindowAttributes
    mXSWA.colormap = XCreateColormap(mpDisplay, rootWindow, mXVI->visual, AllocNone);
    mXSWA.event_mask = ExposureMask | KeyPressMask;

    mpWindow = XCreateWindow(mpDisplay, rootWindow, 0, 0, aSize.x, aSize.y, 0, mXVI->depth, InputOutput, mXVI->visual, CWColormap | CWEventMask, &mXSWA);

    XMapWindow(mpDisplay, mpWindow);
    XStoreName(mpDisplay, mpWindow, "VERY SIMPLE APPLICATION");

    mpGLX = glXCreateContext(mpDisplay, mXVI, NULL, GL_TRUE);
    glXMakeCurrent(mpDisplay, mpWindow, mpGLX);
  }

  tBool IsOK() const {
    return mpDisplay != NULL;
  }

  void Destory() {
    glXMakeCurrent(mpDisplay, None, NULL);
    glXDestroyContext(mpDisplay, mpGLX);
    XDestroyWindow(mpDisplay, mpWindow);
    XCloseDisplay(mpDisplay);
  }

  // iOSWindow* share;
};

// niExportFunc(tBool) osxglIsStarted();
// niExportFunc(tBool) osxglStartup(void);
// niExportFunc(void) osxglShutdown(void);
// niExportFunc(tBool) osxglCreateContext(iOSWindow* apWindow, sXWindowDesc* nsglConfig);
// niExportFunc(void) osxglDestroyContext(iOSWindow* apWindow);
// niExportFunc(tBool) osxglHasContext(iOSWindow* apWindow);
// niExportFunc(void) osxglMakeContextCurrent(iOSWindow* apWindow);
// niExportFunc(iOSWindow*) osxglGetCurrentContext(void);
// niExportFunc(void) osxglSwapBuffers(iOSWindow* apWindow, tBool abDoNotWait);
// niExportFunc(tPtr) osxglGetProcAddress(const char* procname);
// niExportFunc(tIntPtr) osxglGetNSOpenGLContext(iOSWindow* apWindow);
// niExportFunc(void) osxglUpdateDisplayLinkDisplay(iOSWindow* apWindow);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // namespace ni { namespace niOSX {

#undef Bool

#endif

#endif // __LINUXGL_H_6A10DB64_06C2_C441_A70B_1FB42BA690F4__
