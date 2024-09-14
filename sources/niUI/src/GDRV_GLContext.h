#pragma once
#ifndef __GLCONTEXT_H_234535FDSF_FDSF_123213_H__
#define __GLCONTEXT_H_234535FDSF_FDSF_123213_H__

#include <niLang/Types.h>

//===========================================================================
//
//  Config
//
//===========================================================================

//// niWindows ///////////////////////////////////////////////////////////////////////
#if defined niWindows

#pragma comment(lib,"opengl32.lib")
#include <niLang/Platforms/Win32/Win32_Redef.h>
#include <GL/gl.h>
#define TSGL_DESKTOP

//// niOSX ///////////////////////////////////////////////////////////////////////////
#elif defined niOSX

#include <OpenGL/gl.h>
#include <niLang/Platforms/OSX/osxgl.h>
#define TSGL_DESKTOP

//// niLinux /////////////////////////////////////////////////////////////////////////
#elif defined niLinuxDesktop
#include <niLang/Platforms/Linux/linuxgl.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

#define TSGL_DESKTOP
#undef Bool
#undef Status
#undef True
#undef False

//// niLinux /////////////////////////////////////////////////////////////////////////
#else

#  error "Unknown Platform."

#endif

//===========================================================================
//
//  API
//
//===========================================================================
#if !defined TSGL_DESKTOP && !defined TSGL_EMBEDDED
#  error "TSGL_DESKTOP or TSGL_EMBEDDED must be declared."
#endif

#define TSGL_CONTEXT

#define TSGL_OK             0
#define TSGL_ERROR         -1
#define TSGL_ERR_ALLOCCTX  -2
#define TSGL_ERR_LIBEGL    -3
#define TSGL_ERR_LIBGL1    -4
#define TSGL_ERR_LIBGL2    -5
#define TSGL_ERR_LIBUI     -6
#define TSGL_ERR_GETSYM    -7
#define TSGL_ERR_DISPWND   -8
#define TSGL_ERR_EGLINIT   -9
#define TSGL_ERR_CHOOSECFG -10
#define TSGL_ERR_CREATECTX -11
#define TSGL_ERR_CREATEWND -12
#define TSGL_ERR_NOTLOADED -13
#define TSGL_ERR_INVALIDHWND  -14
#define TSGL_ERR_INVALIDHDC   -15
#define TSGL_ERR_INVALIDPXF   -16
#define TSGL_ERR_SETPXF       -17
#define TSGL_ERR_CREATERC     -18

#ifndef TSGL_UNUSED
#  define TSGL_UNUSED(X) (void)(X);
#endif

#ifndef TSGL_RETURN_ERROR
#  define TSGL_RETURN_ERROR(CODE,MSG)                         \
  niError(MSG); return TSGL_ERR_##CODE;
#endif

#ifndef TSGL_MALLOC
#  define TSGL_MALLOC(SIZE)   malloc(SIZE)
#endif

#ifndef TSGL_FREE
#  define TSGL_FREE(PTR,SIZE) free(PTR)
#endif

#ifndef TSGL_ZEROMEMORY
#  define TSGL_ZEROMEMORY(PTR,SIZE) memset((void*)PTR,0,SIZE)
#endif

typedef struct tsglContext_tag {
#if defined TSGL_DESKTOP
  /// Desktop OpenGL ///
#  ifdef _WIN32
  HWND  mhWnd;
  HDC   mhDC;
  HGLRC mhGLRC;
#  elif defined(__APPLE__) || defined niLinux
  ni::tBool mbInitialized;
  ni::iOSWindow* mpWindow;
#  else
#    error "Unsupported platform."
#  endif
#else
  /// Embedded OpenGL ///
  int width;
  int height;
#  ifdef __GLES__
  EGLint majorVersion;
  EGLint minorVersion;
  EGLContext eglContext;
  EGLSurface eglSurface;
  EGLConfig eglConfig;
  EGLDisplay eglDisplay;
#  endif
#  ifdef _WIN32
  HWND displayHWND;
  HDC  displayHDC;
#  endif
#  ifdef ANDROID
  NativeWindowType displayWindow;
#  endif
#  ifdef niSuperH
  fbdev_window fbdevWindow;
#  endif
#endif
} tsglContext;

int tsglCreateContext(
    tsglContext** appCtx,
    ni::iOSWindow* apWindow,
    ni::tU32 anColorBits, ni::tU32 anDepthBits, ni::tU32 anStencilBits,
    ni::tU32 anAA, ni::tU32 anSwapInterval);
void tsglDestroyContext(tsglContext* apCtx);

void tsglMakeCurrent(tsglContext* apCtx);
void tsglSwapBuffers(tsglContext* apCtx, ni::tBool abDoNotWait);

#endif // __GLCONTEXT_H_234535FDSF_FDSF_123213_H__
