#pragma once
#ifndef __GLCONTEXT_H_234535FDSF_FDSF_123213_H__
#define __GLCONTEXT_H_234535FDSF_FDSF_123213_H__

#include <niLang/Types.h>

//===========================================================================
//
//  Config
//
//===========================================================================

//// Embedded OpenGL /////////////////////////////////////////////////////////////////
#if defined __JSCC__ || defined niAndroid || defined niIOS || defined niQNX
#  define TSGL_EMBEDDED
#  define __TSGL_KIND_ES2__
#  define __TSGL_NOCONTEXT__

#  if defined __JSCC__
#    include <GLES2/gl2.h>
#    include <GLES2/gl2ext.h>

#    ifndef GL_HALF_FLOAT_OES
#      error "Emscripten GLES OES extensions not included"
#    endif

#  elif defined niAndroid
#    include <GLES2/gl2.h>
#    include <GLES2/gl2ext.h>
// Use separated alpha blending to composite the alpha channel additively so
// that translucent backgrounds work correctly.
#    define GLDRV_CORRECT_ALPHA_COMPOSITING

#  elif defined niIOS

#    ifdef USE_GLES3
#      import <OpenGLES/ES3/gl.h>
#      import <OpenGLES/ES3/glext.h>
#    else
#      import <OpenGLES/ES2/gl.h>
#      import <OpenGLES/ES2/glext.h>
#    endif

#  elif defined niQNX
#    include "GLES2/gl2.h"

#  else
#    error "Unknown embedded OpenGL platform."
#  endif

//// niWindows ///////////////////////////////////////////////////////////////////////
#elif defined niWindows

#  include <niLang/Platforms/Win32/Win32_Redef.h>
#  include <GL/gl.h>
#  include "GL/ni_glext.h"
#  pragma comment(lib,"opengl32.lib")
#  define TSGL_DESKTOP
#  define __TSGL_STATIC_CORE__
#  define __TSGL_KIND_DESKTOP__
using GLhandle = void*;

#  define GL_APIENTRY APIENTRY
#  define USE_OQ

#  define GL_INVALID_FRAMEBUFFER_OPERATION GL_INVALID_FRAMEBUFFER_OPERATION_EXT
#  define GL_MAX_RENDERBUFFER_SIZE GL_MAX_RENDERBUFFER_SIZE_EXT
#  define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_EXT
#  define GL_RENDERBUFFER_BINDING GL_RENDERBUFFER_BINDING_EXT
#  define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT
#  define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT
#  define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT
#  define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT
#  define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT
#  define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
#  define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT
#  define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT
#  define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
#  define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#  define GL_FRAMEBUFFER_INCOMPLETE_FORMATS GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT
#  define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT
#  define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT
#  define GL_FRAMEBUFFER_UNSUPPORTED GL_FRAMEBUFFER_UNSUPPORTED_EXT
#  define GL_MAX_COLOR_ATTACHMENTS GL_MAX_COLOR_ATTACHMENTS_EXT
#  define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#  define GL_COLOR_ATTACHMENT1 GL_COLOR_ATTACHMENT1_EXT
#  define GL_COLOR_ATTACHMENT2 GL_COLOR_ATTACHMENT2_EXT
#  define GL_COLOR_ATTACHMENT3 GL_COLOR_ATTACHMENT3_EXT
#  define GL_COLOR_ATTACHMENT4 GL_COLOR_ATTACHMENT4_EXT
#  define GL_COLOR_ATTACHMENT5 GL_COLOR_ATTACHMENT5_EXT
#  define GL_COLOR_ATTACHMENT6 GL_COLOR_ATTACHMENT6_EXT
#  define GL_COLOR_ATTACHMENT7 GL_COLOR_ATTACHMENT7_EXT
#  define GL_COLOR_ATTACHMENT8 GL_COLOR_ATTACHMENT8_EXT
#  define GL_COLOR_ATTACHMENT9 GL_COLOR_ATTACHMENT9_EXT
#  define GL_COLOR_ATTACHMENT10 GL_COLOR_ATTACHMENT10_EXT
#  define GL_COLOR_ATTACHMENT11 GL_COLOR_ATTACHMENT11_EXT
#  define GL_COLOR_ATTACHMENT12 GL_COLOR_ATTACHMENT12_EXT
#  define GL_COLOR_ATTACHMENT13 GL_COLOR_ATTACHMENT13_EXT
#  define GL_COLOR_ATTACHMENT14 GL_COLOR_ATTACHMENT14_EXT
#  define GL_COLOR_ATTACHMENT15 GL_COLOR_ATTACHMENT15_EXT
#  define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#  define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
#  define GL_RENDERBUFFER_WIDTH GL_RENDERBUFFER_WIDTH_EXT
#  define GL_RENDERBUFFER_HEIGHT GL_RENDERBUFFER_HEIGHT_EXT
#  define GL_RENDERBUFFER_INTERNAL_FORMAT GL_RENDERBUFFER_INTERNAL_FORMAT_EXT

#  define GL_RENDERBUFFER_RED_SIZE GL_RENDERBUFFER_RED_SIZE_EXT
#  define GL_RENDERBUFFER_GREEN_SIZE GL_RENDERBUFFER_GREEN_SIZE_EXT
#  define GL_RENDERBUFFER_BLUE_SIZE GL_RENDERBUFFER_BLUE_SIZE_EXT
#  define GL_RENDERBUFFER_ALPHA_SIZE GL_RENDERBUFFER_ALPHA_SIZE_EXT
#  define GL_RENDERBUFFER_DEPTH_SIZE GL_RENDERBUFFER_DEPTH_SIZE_EXT
#  define GL_RENDERBUFFER_STENCIL_SIZE GL_RENDERBUFFER_STENCIL_SIZE_EXT

#  ifndef NO_STENCIL_BUFFER
#    define GL_STENCIL_ATTACHMENT GL_STENCIL_ATTACHMENT_EXT
#    define GL_STENCIL_INDEX1 GL_STENCIL_INDEX1_EXT
#    define GL_STENCIL_INDEX4 GL_STENCIL_INDEX4_EXT
#    define GL_STENCIL_INDEX8 GL_STENCIL_INDEX8_EXT
#    define GL_STENCIL_INDEX16 GL_STENCIL_INDEX16_EXT
#  endif

//// niOSX ///////////////////////////////////////////////////////////////////////////
#elif defined niOSX
#  include <niLang/Platforms/OSX/osxgl.h>

#  include <OpenGL/gl.h>
#  include <OpenGL/glext.h>
#  define TSGL_DESKTOP
#  define __TSGL_STATIC_CORE__
#  define __TSGL_STATIC_EXT__
#  define __TSGL_KIND_DESKTOP__
using GLhandle = GLhandleARB;

#define glCreateShaderObject glCreateShaderObjectARB
#define glCreateProgramObject glCreateProgramObjectARB
#define glDeleteObject glDeleteObjectARB
#define glAttachObject glAttachObjectARB
#define glUseProgramObject glUseProgramObjectARB
#define glGetObjectParameteriv glGetObjectParameterivARB
#define glGetInfoLog glGetInfoLogARB

//// niLinux /////////////////////////////////////////////////////////////////////////
#elif defined niLinuxDesktop
#  include <niLang/Platforms/Linux/linuxgl.h>

#  define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
#  include <GL/glx.h>
#  include <GL/glext.h>
#  define TSGL_DESKTOP
#  define __TSGL_KIND_DESKTOP__
#  define USE_OQ
using GLhandle = void*;

#  undef Bool
#  undef Status
#  undef True
#  undef False

//////////////////////////////////////////////////////////////////////////////////////
#else
#  error "Unknown OpenGL Platform."
#endif

#if !defined TSGL_DESKTOP && !defined TSGL_EMBEDDED
#  error "TSGL_DESKTOP or TSGL_EMBEDDED should be defined."
#endif

//===========================================================================
//
//  OpenGL APIs
//
//===========================================================================
// #define __TSGL_STATIC_CORE__
// #define __TSGL_STATIC_EXT__
// #define __TSGL_KIND_ES1__
// #define __TSGL_KIND_ES2__
// #define __TSGL_KIND_DESKTOP__
// #define __TSGL_IMPLEMENT_API__

#ifdef __GLES1__
#  error "__GLES1__ shouldnt be defined somewhere else."
#endif
#ifdef __GLES2__
#  error "__GLES2__ shouldnt be defined somewhere else."
#endif
#ifdef __GLDESKTOP__
#  error "__GLDESKTOP__ shouldnt be defined somewhere else."
#endif

#ifdef __TSGL_KIND_ES1__
#  define __GLES__
#  define __GLES1__
// #warning("# TSGL GLES1")
#elif defined __TSGL_KIND_ES2__
#  define __GLES__
#  define __GLES2__
// #warning("# TSGL GLES2")
#elif defined __TSGL_KIND_DESKTOP__
#  define __GLDESKTOP__
// #warning("# TSGL FULLGL1")
#else
#  error("# __TSGL_KIND_*__ not defined.")
#endif // __TSGL_KIND_*__

#ifndef EGL_CONTEXT_CLIENT_VERSION
#  define EGL_CONTEXT_CLIENT_VERSION 0x3098
#endif

#ifndef EGL_OPENGL_ES2_BIT
#  define EGL_OPENGL_ES2_BIT 4
#endif

#ifndef TSGL_UNUSED
#  define TSGL_UNUSED(X) (void)(X);
#endif

#ifndef TSGL_RETURN_ERROR
#  define TSGL_RETURN_ERROR(CODE,MSG) niError(MSG); return TSGL_ERR_##CODE;
#endif

#ifndef TSGL_MALLOC
#  define TSGL_MALLOC(SIZE) niMalloc(SIZE)
#endif

#ifndef TSGL_FREE
#  define TSGL_FREE(PTR,SIZE) niFree(PTR)
#endif

#ifndef TSGL_ZEROMEMORY
#  define TSGL_ZEROMEMORY(PTR,SIZE) memset((void*)PTR,0,SIZE)
#endif

#ifndef TSGL_APIENTRY
#  ifdef _WIN32
#    ifndef GL_APIENTRY
#      error "GL_APIENTRY not defined."
#    endif
#    define TSGL_APIENTRY GL_APIENTRY
#  else
#    define TSGL_APIENTRY
#  endif
#endif

// TSGL_CORE_PROC, loaded dynamically or through compiler linking
#if defined __TSGL_IMPLEMENT_API__
#  define TSGL_CORE_PROC(RET,FUNC,PARAMS)           \
  typedef RET (TSGL_APIENTRY *tpfn_##FUNC) PARAMS;  \
  tpfn_##FUNC _##FUNC = nullptr;
#else
#  define TSGL_CORE_PROC(RET,FUNC,PARAMS)           \
  typedef RET (TSGL_APIENTRY *tpfn_##FUNC) PARAMS;  \
  extern tpfn_##FUNC _##FUNC;
#endif

#ifndef TSGL_EXT_PROC
#define TSGL_EXT_PROC TSGL_CORE_PROC
#endif

#ifndef TSGL_OPT_EXT_PROC
#define TSGL_OPT_EXT_PROC TSGL_EXT_PROC
#endif

//===========================================================================
//
//  Context
//
//===========================================================================
#if !defined __TSGL_NOCONTEXT__

#  define TSGL_CONTEXT

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

/* Config attributes (intentionally the same as EGL's config attributes values) */
#define TSGL_BUFFER_SIZE                 0x3020
#define TSGL_ALPHA_SIZE                  0x3021
#define TSGL_BLUE_SIZE                   0x3022
#define TSGL_GREEN_SIZE                  0x3023
#define TSGL_RED_SIZE                    0x3024
#define TSGL_DEPTH_SIZE                  0x3025
#define TSGL_STENCIL_SIZE                0x3026
#define TSGL_CONFIG_CAVEAT               0x3027
#define TSGL_CONFIG_ID                   0x3028
#define TSGL_LEVEL                       0x3029
#define TSGL_MAX_PBUFFER_HEIGHT          0x302A
#define TSGL_MAX_PBUFFER_PIXELS          0x302B
#define TSGL_MAX_PBUFFER_WIDTH           0x302C
#define TSGL_NATIVE_RENDERABLE           0x302D
#define TSGL_NATIVE_VISUAL_ID            0x302E
#define TSGL_NATIVE_VISUAL_TYPE          0x302F
#define TSGL_SAMPLES                     0x3031
#define TSGL_SAMPLE_BUFFERS              0x3032
#define TSGL_SURFACE_TYPE                0x3033
#define TSGL_TRANSPARENT_TYPE            0x3034
#define TSGL_TRANSPARENT_BLUE_VALUE      0x3035
#define TSGL_TRANSPARENT_GREEN_VALUE     0x3036
#define TSGL_TRANSPARENT_RED_VALUE       0x3037
#define TSGL_NONE                        0x3038
#define TSGL_BIND_TO_TEXTURE_RGB         0x3039
#define TSGL_BIND_TO_TEXTURE_RGBA        0x303A
#define TSGL_MIN_SWAP_INTERVAL           0x303B
#define TSGL_MAX_SWAP_INTERVAL           0x303C
#define TSGL_LUMINANCE_SIZE              0x303D
#define TSGL_ALPHA_MASK_SIZE             0x303E
#define TSGL_COLOR_BUFFER_TYPE           0x303F
#define TSGL_RENDERABLE_TYPE             0x3040

typedef struct tsglContext_tag {
#  if defined TSGL_DESKTOP
  /// Desktop OpenGL ///
#    ifdef _WIN32
  HWND  mhWnd;
  HDC   mhDC;
  HGLRC mhGLRC;
#    elif defined(__APPLE__) || defined niLinux
  ni::tBool mbInitialized;
  ni::iOSWindow* mpWindow;
#    else
#      error "Unsupported platform."
#    endif
#  else
  /// Embedded OpenGL ///
  int width;
  int height;
#    ifdef __GLES__
  EGLint majorVersion;
  EGLint minorVersion;
  EGLContext eglContext;
  EGLSurface eglSurface;
  EGLConfig eglConfig;
  EGLDisplay eglDisplay;
#    endif
#    ifdef _WIN32
  HWND displayHWND;
  HDC  displayHDC;
#    endif
#    ifdef ANDROID
  NativeWindowType displayWindow;
#    endif
#    ifdef niSuperH
  fbdev_window fbdevWindow;
#    endif
#  endif
} tsglContext;

int tsglCreateContext(
    tsglContext** appCtx,
    ni::iOSWindow* apWindow,
    ni::tU32 anColorBits, ni::tU32 anDepthBits, ni::tU32 anStencilBits,
    ni::tU32 anAA, ni::tU32 anSwapInterval);
void tsglDestroyContext(tsglContext* apCtx);

void tsglMakeCurrent(tsglContext* apCtx);
void tsglSwapBuffers(tsglContext* apCtx, ni::tBool abDoNotWait);

#endif // __TSGL_NOCONTEXT__

//===========================================================================
//
//  API
//
//===========================================================================
#if defined niWindows || defined niJSCC || defined niAndroid || defined niOSX || defined niIOS || defined niQNX || defined niLinux
#define USE_FBO
#endif

niExportFunc(void*) tsglGetCoreProcAddress(const char *name);
niExportFunc(void*) tsglGetExtProcAddress(const char *name);
niExportFunc(tBool) tsglLoadLibrary();

#ifdef __GLDESKTOP__
#include "GDRV_GLContext_sym_core_desktop.h"
#include "GDRV_GLContext_sym_ext_desktop.h"
#endif
#ifdef __GLES1__
#include "GDRV_GLContext_sym_core_gles1.h"
#endif
#ifdef __GLES2__
#include "GDRV_GLContext_sym_core_gles2.h"
#endif

#endif // __GLCONTEXT_H_234535FDSF_FDSF_123213_H__
