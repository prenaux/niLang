#pragma once
#ifndef __GDRV_GLES2_H_27CBBAD8_91E3_F345_AD28_98AC1E631560__
#define __GDRV_GLES2_H_27CBBAD8_91E3_F345_AD28_98AC1E631560__

#include <niLang/Types.h>
#include <niLang/Utils/Trace.h>
#include <niLang/Utils/ConcurrentImpl.h>
#include <niLang/STL/scope_guard.h>

// I cant get decent perf out glBufferSubData for Dynamic arrays on macOS &
// iOS. A system memory buffer is always significantly quicker. On Windows
// perfs are the same with both... (last test ran in June 2019)
#if !defined niJSCC
#define USE_SYSTEM_MEMORY_FOR_DYNAMIC_BUFFERS
#endif

#ifdef _DEBUG
#  ifndef DO_GL_DEBUG_LOG
// #    define DO_GL_DEBUG_LOG
#  endif
#  ifndef CHECK_GLERR
#    define CHECK_GLERR
#  endif
#endif

#if !defined niEmbedded
#  ifndef CHECK_GLERR
#    define CHECK_GLERR
#  endif
#endif

#ifdef DO_GL_DEBUG_LOG
#define GL_DEBUG_LOG(X) niDebugFmt(X)
#else
#define GL_DEBUG_LOG(X)
#endif

#define GL2_TRACE_BUFFER(aFmt) GL_DEBUG_LOG(aFmt)

// #ifdef niJSCC
#define NO_STENCIL_BUFFER // not supported in IE11 WebGL
// #endif

#ifdef _DEBUG
#define CHECK_GLERR
#endif

// #define USE_GL_UNBIND_BUFFERS
// #define USE_GL_UNBIND_TEXTURES
// #define USE_GL_ENABLE_FOR_TEXTURE
// #define DISABLE_ALL_VAA_AFTER_DRAWOP

// GL_LUMINANCE_ALPHA is not supported in IE11's WebGL, its also
// deprecated in GL 4.2 so we've just removed it atm...
// #define USE_TEXFMT_LUMINANCE_ALPHA

// GL_ALPHA is not supported in IE11's WebGL, also color is white which is basically useless...
// #define USE_TEXFMT_ALPHA

// GL_RGB is not supported in IE11's WebGL, also its padded to a 32bits texture on 99% of the HW...
// #define USE_TEXFMT_GL_RGB

// #if defined niJSCC
// #define USE_TEXFMT_USE_DEFAULT_ONLY
// #endif

#if defined niIOS && defined ni64
#define USE_GLES3
#define USE_GL_BIND_VAO
#define _glBindVertexArray glBindVertexArray
#define USE_GL_BIND_SAMPLER
#define _glBindSampler glBindSampler
#endif

/*
  Valid values for glEnable/glDisable in ES2 :

  GL_CULL_FACE
  GL_POLYGON_OFFSET_FILL
  GL_SAMPLE_ALPHA_TO_COVERAGE
  GL_SAMPLE_COVERAGE
  GL_SCISSOR_TEST
  GL_STENCIL_TEST
  GL_DEPTH_TEST
  GL_BLEND
  GL_DITHER

  GL_TEXTURE_* are invalid...... yeye...

  http://www.khronos.org/registry/gles/specs/2.0/es_full_spec_2.0.25.pdf

*/

/*

  Note for glGen* & glDelete*.

  glGen must be called right before the bind. If its called first and other
  gen are called before the resource generated has been bound and initialized
  it is possible that glGen returns the handle previous returned.

  The OpenGL specs are ambiguous on this point and most online resource seems
  to say that it isnt the case, but *it is* the case in practise.

 */
#if defined __JSCC__ || defined niAndroid || defined niOSX || defined niIOS || defined niLinux || defined niQNX
#define __TSGL_ES2__
#define __TSGL_NOCONTEXT__
#define __GLES__
#define __GLES2__

#if defined __JSCC__
#  include <GLES2/gl2.h>
#  include <GLES2/gl2ext.h>

#  ifndef GL_HALF_FLOAT_OES
#    error "Emscripten GLES OES extensions not included"
#  endif

#elif defined niAndroid
#  include <GLES2/gl2.h>
#  include <GLES2/gl2ext.h>
// Use separated alpha blending to composite the alpha channel additively so
// that translucent backgrounds work correctly.
#  define GLDRV_CORRECT_ALPHA_COMPOSITING

#elif defined niIOS

#ifdef USE_GLES3
#  import <OpenGLES/ES3/gl.h>
#  import <OpenGLES/ES3/glext.h>
#else
#  import <OpenGLES/ES2/gl.h>
#  import <OpenGLES/ES2/glext.h>
#endif

#elif defined niOSX
#  include "GDRV_GLContext.h"
#  undef __TSGL_NOCONTEXT__
#  include <OpenGL/glext.h>

#elif defined niQNX
#  include "GLES2/gl2.h"

#elif defined niLinux
#  include "GDRV_GLContext.h"
#  undef __TSGL_NOCONTEXT__
#  include <GL/glext.h>

#else
#  error "Unknown platform."
#endif

#define NO_TSGL

#define _glGetError glGetError
#define _glGetString glGetString
#define _glFinish glFinish
#define _glFlush glFlush
#define _glViewport glViewport
#define _glScissor glScissor

#define _glTexParameteri glTexParameteri

#define _glEnable glEnable
#define _glDisable glDisable

#define _glBlendFunc glBlendFunc

#define _glDepthMask glDepthMask
#define _glDepthFunc glDepthFunc

#ifndef NO_STENCIL_BUFFER
#define _glStencilFunc glStencilFunc
#define _glStencilOp glStencilOp
#define _glClearStencil glClearStencil
#endif

#define _glCullFace glCullFace
#define _glFrontFace glFrontFace

#define _glColorMask glColorMask

#define _glClear glClear
#define _glClearColor glClearColor

#if defined niOSX
#define _glClearDepthf glClearDepth
#define _glPolygonMode glPolygonMode
#else
#if !defined __GLES2__
#define _glPolygonMode glPolygonMode
#endif
#define _glClearDepthf glClearDepthf
#endif

#define _glGetIntegerv glGetIntegerv

#define _glDeleteTextures glDeleteTextures
#define _glGenTextures glGenTextures
#define _glBindTexture glBindTexture
#define _glTexImage2D glTexImage2D
#define _glActiveTexture glActiveTexture

#define _glBindAttribLocation glBindAttribLocation
#define _glShaderSource glShaderSource
#define _glCompileShader glCompileShader
#define _glGetShaderInfoLog glGetShaderInfoLog
#define _glLinkProgram glLinkProgram
#define _glGetProgramiv glGetProgramiv
#define _glGetProgramInfoLog glGetProgramInfoLog
#define _glCreateShader glCreateShader
#define _glDeleteShader glDeleteShader
#define _glGetShaderiv glGetShaderiv
#define _glUseProgram glUseProgram
#define _glGetAttribLocation glGetAttribLocation
#define _glGetUniformLocation glGetUniformLocation

#define _glUniformMatrix4fv glUniformMatrix4fv
#define _glUniform4fv glUniform4fv
#define _glUniform1i glUniform1i
#define _glUniform1iv glUniform1iv
#define _glUniform4iv glUniform4iv
#define _glUniform1f glUniform1f

#define _glCreateProgram glCreateProgram
#define _glAttachShader glAttachShader
#define _glDeleteProgram glDeleteProgram

#define _glGenBuffers glGenBuffers
#define _glBindBuffer glBindBuffer
#define _glDeleteBuffers glDeleteBuffers
#define _glBufferData glBufferData
#define _glBufferSubData glBufferSubData

#define _glEnableVertexAttribArray glEnableVertexAttribArray
#define _glDisableVertexAttribArray glDisableVertexAttribArray
#define _glVertexAttribPointer glVertexAttribPointer

#define _glDrawElements glDrawElements
#define _glDrawArrays glDrawArrays

#define _glBlendEquationSeparate glBlendEquationSeparate
#define _glBlendFuncSeparate glBlendFuncSeparate

#elif defined niWindows

#define __TSGL_IMPLEMENT_STATIC__
#define __TSGL_WRAPPER__
#define __TSGL_FULL1__
#define __TSGL_IMPLEMENT__
#define _glClearDepthf _glClearDepth
#define _glPolygonMode _glPolygonMode
// #pragma comment(lib,"opengl32.lib")
#include "GDRV_GLContext.h"
#undef __TSGL_NOCONTEXT__
#include <niLang/Platforms/Win32/Win32_Redef.h>
#include <GL/gl.h>
#include "GL/glext.h"
#define GL_APIENTRY APIENTRY
// #undef GL_CLAMP_TO_EDGE
// #define GL_CLAMP_TO_EDGE GL_CLAMP
// #undef GL_RESCALE_NORMAL
// #define GL_RESCALE_NORMAL GL_NORMALIZE
#define USE_OQ

#define GL_INVALID_FRAMEBUFFER_OPERATION GL_INVALID_FRAMEBUFFER_OPERATION_EXT
#define GL_MAX_RENDERBUFFER_SIZE GL_MAX_RENDERBUFFER_SIZE_EXT
#define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_EXT
#define GL_RENDERBUFFER_BINDING GL_RENDERBUFFER_BINDING_EXT
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT
#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT
#define GL_FRAMEBUFFER_UNSUPPORTED GL_FRAMEBUFFER_UNSUPPORTED_EXT
#define GL_MAX_COLOR_ATTACHMENTS GL_MAX_COLOR_ATTACHMENTS_EXT
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#define GL_COLOR_ATTACHMENT1 GL_COLOR_ATTACHMENT1_EXT
#define GL_COLOR_ATTACHMENT2 GL_COLOR_ATTACHMENT2_EXT
#define GL_COLOR_ATTACHMENT3 GL_COLOR_ATTACHMENT3_EXT
#define GL_COLOR_ATTACHMENT4 GL_COLOR_ATTACHMENT4_EXT
#define GL_COLOR_ATTACHMENT5 GL_COLOR_ATTACHMENT5_EXT
#define GL_COLOR_ATTACHMENT6 GL_COLOR_ATTACHMENT6_EXT
#define GL_COLOR_ATTACHMENT7 GL_COLOR_ATTACHMENT7_EXT
#define GL_COLOR_ATTACHMENT8 GL_COLOR_ATTACHMENT8_EXT
#define GL_COLOR_ATTACHMENT9 GL_COLOR_ATTACHMENT9_EXT
#define GL_COLOR_ATTACHMENT10 GL_COLOR_ATTACHMENT10_EXT
#define GL_COLOR_ATTACHMENT11 GL_COLOR_ATTACHMENT11_EXT
#define GL_COLOR_ATTACHMENT12 GL_COLOR_ATTACHMENT12_EXT
#define GL_COLOR_ATTACHMENT13 GL_COLOR_ATTACHMENT13_EXT
#define GL_COLOR_ATTACHMENT14 GL_COLOR_ATTACHMENT14_EXT
#define GL_COLOR_ATTACHMENT15 GL_COLOR_ATTACHMENT15_EXT
#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
#define GL_RENDERBUFFER_WIDTH GL_RENDERBUFFER_WIDTH_EXT
#define GL_RENDERBUFFER_HEIGHT GL_RENDERBUFFER_HEIGHT_EXT
#define GL_RENDERBUFFER_INTERNAL_FORMAT GL_RENDERBUFFER_INTERNAL_FORMAT_EXT

#define GL_RENDERBUFFER_RED_SIZE GL_RENDERBUFFER_RED_SIZE_EXT
#define GL_RENDERBUFFER_GREEN_SIZE GL_RENDERBUFFER_GREEN_SIZE_EXT
#define GL_RENDERBUFFER_BLUE_SIZE GL_RENDERBUFFER_BLUE_SIZE_EXT
#define GL_RENDERBUFFER_ALPHA_SIZE GL_RENDERBUFFER_ALPHA_SIZE_EXT
#define GL_RENDERBUFFER_DEPTH_SIZE GL_RENDERBUFFER_DEPTH_SIZE_EXT
#define GL_RENDERBUFFER_STENCIL_SIZE GL_RENDERBUFFER_STENCIL_SIZE_EXT

#ifndef NO_STENCIL_BUFFER
#define GL_STENCIL_ATTACHMENT GL_STENCIL_ATTACHMENT_EXT
#define GL_STENCIL_INDEX1 GL_STENCIL_INDEX1_EXT
#define GL_STENCIL_INDEX4 GL_STENCIL_INDEX4_EXT
#define GL_STENCIL_INDEX8 GL_STENCIL_INDEX8_EXT
#define GL_STENCIL_INDEX16 GL_STENCIL_INDEX16_EXT
#endif

#endif

#if !defined NO_TSGL
#define __TSGL_IMPLEMENT__
#define __TSGL_IMPLEMENT_STATIC__
#include "GLES/tsgl.h"
#endif

#define GL_SYNC_BUFFER_UPLOAD()
#define GL_SYNC_SWAPBUFFER()
#define GL_SYNC_LINK_SHADER()    //{ _glFinish(); _glFlush(); }
#define GL_SYNC_COMPILE_SHADER()
#define GL_SYNC_TEX_UPLOAD()     //{ _glFinish(); _glFlush(); }

// #define NO_GLTEXTURE_SYSCOPY

#define GL_IsDeviceTexture(TEX) eTrue

// #define LAZY_CLEAR_BUFFERS

#if defined GL_HALF_FLOAT
#  define MY_GL_HALF_FLOAT GL_HALF_FLOAT
#elif defined GL_HALF_FLOAT_ARB
#  define MY_GL_HALF_FLOAT GL_HALF_FLOAT_ARB
#elif defined GL_HALF_FLOAT_OES
// GL_OES_texture_half_float
#  define MY_GL_HALF_FLOAT GL_HALF_FLOAT_OES
#elif defined GL_HALF_FLOAT_EXT
#  define MY_GL_HALF_FLOAT GL_HALF_FLOAT_EXT
#else
#  error "GDRV_GL2: No GL_HALF_FLOAT definition available!"
#endif

#if 0
// TODO: To support GL_RGBA16F & GL_RGBA32F we MUST detect the extensions, we
// cannot assume they are supported as they are not available on most WebGL
// implementations for example (as of 2022).

#if defined GL_RGBA16F
#  define MY_GL_RGBA16F    GL_RGBA16F
#elif defined GL_RGBA16F_ARB
#  define MY_GL_RGBA16F    GL_RGBA16F_ARB
#elif defined GL_RGBA16F_OES
#  define MY_GL_RGBA16F    GL_RGBA16F_OES
#elif defined GL_RGBA16F_EXT
// GL_EXT_color_buffer_half_float
#  define MY_GL_RGBA16F    GL_RGBA16F_EXT
#else
#  error "GDRV_GL2: No GL_RGBA16F definition available!"
#endif

#if defined GL_RGBA32F
#  define MY_GL_RGBA32F    GL_RGBA32F
#elif defined GL_RGBA32F_ARB
#  define MY_GL_RGBA32F    GL_RGBA32F_ARB
#elif defined GL_RGBA32F_OES
#  define MY_GL_RGBA32F    GL_RGBA32F_OES
#elif defined GL_RGBA32F_EXT
// GL_EXT_texture_storage
#  define MY_GL_RGBA32F    GL_RGBA32F_EXT
#else
#  error "GDRV_GL2: No GL_RGBA32F definition available!"
#endif

#endif

#define GLES_TEXFMT_RGB16  0
#define GLES_TEXFMT_RGBA16 1
#define GLES_TEXFMT_RGBA32 3
#define GLES_TEXFMT_RGBA15 4
#define GLES_TEXFMT_RGB24  5
#define GLES_TEXFMT_LA16   6
#define GLES_TEXFMT_A8     7
#define GLES_TEXFMT_RGBA32F 8
#define GLES_TEXFMT_RGBA16F 9

// depth format 10+
#define GLES_TEXFMT_D16    10
#define GLES_TEXFMT_D24    11
#define GLES_TEXFMT_D32    12

#define GLES_TEXFMT_RGBA_DEFAULT  GLES_TEXFMT_RGBA32
#define GLES_TEXFMT_RGB_DEFAULT   GLES_TEXFMT_RGB24

#ifndef NO_STENCIL_BUFFER
#define GLES_TEXFMT_D24S8  12
#endif

#if !defined GL_TEXTURE_CUBE_MAP
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define	GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS   0x8CD9
#endif

#define GLDRV_MAX_TEXTURE_UNIT 8 // OpenGL ES 2 supports 8 texture units only
#define GLDRV_INVALID_HANDLE   ((GLuint)0)

// #define GLDRV_CORRECT_ALPHA_COMPOSITING

static const GLenum GLEXT_TEXTURE_MAX_ANISOTROPY     = 0x84FE;
static const GLenum GLEXT_MAX_TEXTURE_MAX_ANISOTROPY = 0x84FF;

#if 1 && defined _DEBUG
#define CAN_USE_GL_DEBUG_MARKERS
#endif

#if defined CAN_USE_GL_DEBUG_MARKERS && defined niIOS
#define USE_GL_DEBUG_MARKER
#define USE_GL_DEBUG_LABEL
#endif

#ifdef USE_GL_DEBUG_MARKER
struct sDebugMarkerCounters {
  tU32 DrawOp;
  tU32 ClearBuffers;
  tU32 ApplyContext;
  sDebugMarkerCounters() {
    Clear();
  }
  void Clear() {
    memset((void*)this,0,sizeof(*this));
  }
} _debugMarkerCounter;
struct sGLPushGroupMarker {
  sGLPushGroupMarker(const char* aaszName, tU32 aCount) {
    glPushGroupMarkerEXT(0,niFmt("%s_%d",aaszName,aCount));
  }
  ~sGLPushGroupMarker() {
    glPopGroupMarkerEXT();
  }
};
#define GL_DEBUG_SWAP_BUFFERS() _debugMarkerCounter.Clear()
#define GL_DEBUG_MARKER_GROUP(NAME) sGLPushGroupMarker __debugMarker_##NAME(#NAME,_debugMarkerCounter.NAME++)
#define GL_DEBUG_EVENT_MARKER(NAME) glInsertEventMarkerEXT(0,#NAME)
#else
#define GL_DEBUG_SWAP_BUFFERS()
#define GL_DEBUG_MARKER_GROUP(NAME)
#define GL_DEBUG_EVENT_MARKER(NAME)
#endif

#ifdef USE_GL_DEBUG_LABEL
#define GL_DEBUG_LABEL(TYPE,OBJECT,STR) glLabelObjectEXT(TYPE, OBJECT, 0, STR)
#else
#define GL_DEBUG_LABEL(TYPE,OBJECT,STR)
#endif

#ifdef _DEBUG
#define GLDRV_HANDLE_TRACE(PARAMS) // niTraceFmt(PARAMS)
#else
#define GLDRV_HANDLE_TRACE(PARAMS)
#endif

#define GLDRV_GEN_HANDLE(VAR,FUNC) {                                    \
    FUNC(1,&VAR);                                                       \
    GLDRV_HANDLE_TRACE((#FUNC " generated handle: %d", VAR));           \
  }

/* The niExec(Main) method has the drawback that if you create a lot of
 * resources and delete them in a loop without ever running the main executor,
 * you'll never actually release it. But then again I can't see a better way
 * of making sure the resource deletion happens in the main thread which is
 * required by some OpenGL drivers - without this they just crash.
 */
#define GLDRV_GEN_DELETE_HANDLE(VAR,FUNC) {                             \
    GLuint handleToDelete = VAR;                                        \
    niExec(Main,handleToDelete) {                                       \
      GLDRV_HANDLE_TRACE((#FUNC " deleting handle: %d", handleToDelete)); \
      FUNC(1,&handleToDelete);                                          \
      return eTrue;                                                     \
    };                                                                  \
    VAR = GLDRV_INVALID_HANDLE;                                         \
  }

#define GLDRV_CREATE_HANDLE(VAR,FUNC,PARAMS) {                          \
    VAR = FUNC PARAMS;                                                  \
    GLDRV_HANDLE_TRACE((#FUNC " in " __FUNCTION__ " created handle: %d", VAR)); \
  }

#define GLDRV_CREATE_DELETE_HANDLE_(VAR,FUNC) {                         \
    GLDRV_HANDLE_TRACE((#FUNC " in " __FUNCTION__ " deleting handle: %d", VAR)); \
    FUNC(VAR);                                               \
    VAR = GLDRV_INVALID_HANDLE;                                         \
  }

#define GLDRV_CREATE_DELETE_HANDLE(VAR,FUNC) {          \
    GLuint _handleToDelete = VAR;                       \
    niExec(Main,_handleToDelete) {                      \
      GLuint handleToDelete = _handleToDelete;          \
      GLDRV_CREATE_DELETE_HANDLE_(handleToDelete,FUNC); \
      return eTrue;                                     \
    };                                                  \
    VAR = GLDRV_INVALID_HANDLE;                         \
  }

//--------------------------------------------------------------------------------------------
//
//  Errors
//
//--------------------------------------------------------------------------------------------
#ifdef CHECK_GLERR
static const char* GL_Error(GLenum err) {
  switch (err) {
    case GL_NO_ERROR:          return _A("GL_NO_ERROR");
    case GL_INVALID_ENUM:      return _A("GL_INVALID_ENUM");
    case GL_INVALID_VALUE:     return _A("GL_INVALID_VALUE");
    case GL_INVALID_OPERATION: return _A("GL_INVALID_OPERATION");
#ifdef GL_STACK_OVERFLOW
    case GL_STACK_OVERFLOW:    return _A("GL_STACK_OVERFLOW");
#endif
#ifdef GL_STACK_UNDERFLOW
    case GL_STACK_UNDERFLOW:   return _A("GL_STACK_UNDERFLOW");
#endif
    case GL_OUT_OF_MEMORY:     return _A("GL_OUT_OF_MEMORY");
  }
  return _A("GL_UNKNOWN_ERROR");
}

niDeclareModuleTrace_(niUI,GDRV_GL2);
#define _CheckGLError() niModuleShouldTrace_(niUI,GDRV_GL2)

#define GLERR_WARN() if (_CheckGLError()) {                             \
    GLenum err = _glGetError();                                         \
    if (err != GL_NO_ERROR) {                                           \
      ni::GetLang()->Log(eLogFlags_Warning,__FILE__,__FUNCTION__,__LINE__, \
                         niFmt(_A("GL Error: %d, %s\n"),                \
                               err,GL_Error(err)));                     \
      niAssertUnreachable("GL Error");                                  \
    }                                                                   \
  }

#define GLERR_RET(RET) if (_CheckGLError()) {                           \
    GLenum err = _glGetError();                                         \
    if (err != GL_NO_ERROR) {                                           \
      ni::GetLang()->Log(eLogFlags_Error,__FILE__,__FUNCTION__,__LINE__, \
                         niFmt(_A("GL Error: %d, %s\n"),                \
                               err,GL_Error(err)));                     \
      niAssertUnreachable("GL Error");                                  \
      return RET;                                                       \
    }                                                                   \
  }

#else

#define GLERR_WARN()
#define GLERR_RET(RET)

#endif

#define GLCALL_WARN(CODE) {                     \
    CODE;                                       \
    GLERR_WARN();                               \
  }

#define GLCALL_ERR(CODE,RET) {                  \
    CODE;                                       \
    GLERR_RET(RET);                             \
  }

#endif // __GDRV_GLES2_H_27CBBAD8_91E3_F345_AD28_98AC1E631560__
