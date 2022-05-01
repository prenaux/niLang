#ifndef __TSGL_H_94E541FC_7131_DF11_BF12_598E439FC2CC__
#define __TSGL_H_94E541FC_7131_DF11_BF12_598E439FC2CC__

#ifndef __TSGL_INTERFACES_ONLY___

// #define __TSGL_WRAPPER__  // wrap statically linked gles, assumed OpenGL headers have been included before this header file
// #define __TSGL_IMPLEMENT__
// #define __TSGL_IMPLEMENT_STATIC__
// #define __TSGL_ES1__
// #define __TSGL_ES2__
// #define __TSGL_FULL1__

#if defined __TSGL_AUTOCONFIG_GL1__

#if !defined niEmbedded
/* PC/Desktop configs Full OpenGL */

#define __TSGL_WRAPPER__
#define __TSGL_FULL1__
#define __TSGL_IMPLEMENT__
#define _glClearDepthf _glClearDepth

#if defined niWindows

// #warning("# TSGL Windows Desktop")
#pragma comment(lib,"opengl32.lib")
#include <niLang/Platforms/Win32/Win32_Redef.h>
#include <GL/gl.h>
#include "PC/glext.h"

#elif defined niOSX
#pragma message("# TSGL OSX Desktop")

#define __GLEXT_PROTOTYPES__
#include <OpenGL/gl.h>
#include <AGL/agl.h>

#elif defined niLinux
// #warning("# TSGL Linux Desktop")

#include <GL/gl.h>
#include <GL/glx.h>
#undef Bool
#undef True
#undef False
#else
// #warning("# TSGL Unknown Desktop")

#include <GL/gl.h>
#include "PC/glext.h"
#endif

#define GL_APIENTRY APIENTRY
#undef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE GL_CLAMP
#undef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL GL_NORMALIZE

/* if !defined niEmbedded */
#elif defined niIOS

// #warning("# TSGL IOS Embedded")

#define __TSGL_ES1__
#define __TSGL_WRAPPER__
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

/* if !defined niEmbedded */
#elif defined niSuperH

// #warning("# TSGL SuperH Embedded")

#define __TSGL_ES1__
#define __TSGL_WRAPPER__
#define __TSGL_IMPLEMENT__
#define __TSGL_IMPLEMENT_STATIC__

#include "ST/gl.h"
#include "ST/egl.h"
#include "ST/fbdev_window.h"

/* if !defined niEmbedded */
#elif defined niAndroid

#ifdef __TSGL_ES2__
#pragma message("# TSGL ES2")
#include "Android/gl2.h"
#include "Android/egl.h"
#else
#pragma message("# TSGL ES1")
#include "Android/gl.h"
#include "Android/egl.h"
#define __TSGL_ES1__
#endif
#define __TSGL_IMPLEMENT__
#define __TSGL_IMPLEMENT_STATIC__
#define __TSGL_NOBINDAPI__

#endif

#endif // __TSGL_AUTOCONFIG__

#if !defined __TSGL_ES1__ && !defined __TSGL_ES2__ && !defined __GLES1__ && !defined __GLES2__ && !defined __TSGL_FULL1__
#error "No target OpenGL version defined."
#endif

#ifdef __TSGL_ES1__
#define __GLES__
#define __GLES1__
// #warning("# TSGL GLES1")
#elif defined __TSGL_ES2__
#define __GLES__
#define __GLES2__
// #warning("# TSGL GLES2")
#elif defined __TSGL_FULL1__
#define __GL1__
#define __FULLGL__
// #warning("# TSGL FULLGL1")
#else
#error("# TSGL Not Defined.")
#endif

#ifdef __GLES__
#ifdef niSuperH
#define TSGL_DEFAULT_WIDTH   1280
#define TSGL_DEFAULT_HEIGHT  720
#else
#define TSGL_DEFAULT_WIDTH   800
#define TSGL_DEFAULT_HEIGHT  480
#endif
#else
#define TSGL_DEFAULT_WIDTH   800
#define TSGL_DEFAULT_HEIGHT  600
#endif

//===========================================================================
//
//  Include headers
//
//===========================================================================
#if !defined __TSGL_WRAPPER__
#include <stdio.h>
#include <stdlib.h>
#if defined __linux__ || defined ANDROID
#include <dlfcn.h>
#elif defined _WIN32
#include <windows.h>
#endif
#endif

#ifndef EGL_CONTEXT_CLIENT_VERSION
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#endif

#ifndef EGL_OPENGL_ES2_BIT
#define EGL_OPENGL_ES2_BIT 4
#endif

//===========================================================================
//
//  Definitions
//
//===========================================================================
#ifdef __cplusplus
extern "C" {
#endif

#if defined __TSGL_IMPLEMENT_STATIC__
#define TSGL_EXTERN static
#elif defined __TSGL_WRAPPER__ || defined __TSGL_IMPLEMENT__
#define TSGL_EXTERN
#else
#define TSGL_EXTERN extern
#endif

#ifdef __TSGL_WRAPPER__

  // Wrapper around the directly linked library

#elif defined _WIN32

  typedef HMODULE TSGL_DLLHANDLE;
#define TSGL_DLLEXT ".dll"

#ifndef TSGL_DLOPEN
#define TSGL_DLOPEN(file) LoadLibraryA(file)
#endif
#ifndef TSGL_DLFREE
#define TSGL_DLFREE(dll) FreeLibrary(dll)
#endif
#ifndef TSGL_DLSYM
#define TSGL_DLSYM(dll,func) GetProcAddress(dll, func)
#endif

#else

  typedef void* TSGL_DLLHANDLE;
#define TSGL_DLLEXT ".so"

#ifndef TSGL_DLOPEN
#define TSGL_DLOPEN(file) dlopen(file,RTLD_LAZY)
#endif
#ifndef TSGL_DLFREE
#define TSGL_DLFREE(dll) dlclose(dll)
#endif
#ifndef TSGL_DLSYM
#define TSGL_DLSYM(dll,func) dlsym(dll, func)
#endif

#endif

#ifndef TSGL_UNUSED
#define TSGL_UNUSED(X) (void)(X);
#endif

#ifndef TSGL_RETURN_ERROR
#define TSGL_RETURN_ERROR(CODE,MSG)                         \
  printf(MSG "\n"); fflush(stdout); return TSGL_ERR_##CODE;
#endif

#ifndef TSGL_MALLOC
#define TSGL_MALLOC(SIZE)   malloc(SIZE)
#endif

#ifndef TSGL_FREE
#define TSGL_FREE(PTR,SIZE) free(PTR)
#endif

#ifndef TSGL_ZEROMEMORY
#define TSGL_ZEROMEMORY(PTR,SIZE) memset((void*)PTR,0,SIZE)
#endif

#ifndef TSGL_APIENTRY
#ifdef _WIN32
#ifndef GL_APIENTRY
#error "GL_APIENTRY not defined."
#endif
#define TSGL_APIENTRY GL_APIENTRY
#else
#define TSGL_APIENTRY
#endif
#endif

#if defined __TSGL_WRAPPER__
#if defined __TSGL_IMPLEMENT_STATIC__
#define TSGL_PROTOTYPE(RET,FUNC,PARAMS)             \
  typedef RET (TSGL_APIENTRY *tpfn_##FUNC) PARAMS;  \
  static tpfn_##FUNC _##FUNC = FUNC;
#else
#define TSGL_PROTOTYPE(RET,FUNC,PARAMS)             \
  typedef RET (TSGL_APIENTRY *tpfn_##FUNC) PARAMS;  \
  tpfn_##FUNC _##FUNC = FUNC;
#endif
#elif defined __TSGL_IMPLEMENT_STATIC__
#define TSGL_PROTOTYPE(RET,FUNC,PARAMS)             \
  typedef RET (TSGL_APIENTRY *tpfn_##FUNC) PARAMS;  \
  static tpfn_##FUNC _##FUNC = NULL;
#elif defined __TSGL_IMPLEMENT__
#define TSGL_PROTOTYPE(RET,FUNC,PARAMS)             \
  typedef RET (TSGL_APIENTRY *tpfn_##FUNC) PARAMS;  \
  tpfn_##FUNC _##FUNC = NULL;
#else
#define TSGL_PROTOTYPE(RET,FUNC,PARAMS)             \
  typedef RET (TSGL_APIENTRY *tpfn_##FUNC) PARAMS;  \
  TSGL_EXTERN tpfn_##FUNC _##FUNC;
#endif

#endif // __GLES__

  //===========================================================================
  //
  //  OpenGL 1
  //
  //===========================================================================
#ifdef __GL1__
  TSGL_PROTOTYPE(void, glAccum, (GLenum op, GLfloat value));
  TSGL_PROTOTYPE(void, glAlphaFunc, (GLenum func, GLclampf ref));
  TSGL_PROTOTYPE(GLboolean, glAreTexturesResident, (GLsizei n, const GLuint *textures, GLboolean *residences));
  TSGL_PROTOTYPE(void, glArrayElement, (GLint i));
  TSGL_PROTOTYPE(void, glBegin, (GLenum mode));
  TSGL_PROTOTYPE(void, glBindTexture, (GLenum target, GLuint texture));
  TSGL_PROTOTYPE(void, glBitmap, (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap));
  TSGL_PROTOTYPE(void, glBlendFunc, (GLenum sfactor, GLenum dfactor));
  TSGL_PROTOTYPE(void, glCallList, (GLuint list));
  TSGL_PROTOTYPE(void, glCallLists, (GLsizei n, GLenum type, const GLvoid *lists));
  TSGL_PROTOTYPE(void, glClear, (GLbitfield mask));
  TSGL_PROTOTYPE(void, glClearAccum, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha));
  TSGL_PROTOTYPE(void, glClearColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
  TSGL_PROTOTYPE(void, glClearDepth, (GLclampd depth));
  TSGL_PROTOTYPE(void, glClearIndex, (GLfloat c));
  TSGL_PROTOTYPE(void, glClearStencil, (GLint s));
  TSGL_PROTOTYPE(void, glClipPlane, (GLenum plane, const GLdouble *equation));
  TSGL_PROTOTYPE(void, glColor3b, (GLbyte red, GLbyte green, GLbyte blue));
  TSGL_PROTOTYPE(void, glColor3bv, (const GLbyte *v));
  TSGL_PROTOTYPE(void, glColor3d, (GLdouble red, GLdouble green, GLdouble blue));
  TSGL_PROTOTYPE(void, glColor3dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glColor3f, (GLfloat red, GLfloat green, GLfloat blue));
  TSGL_PROTOTYPE(void, glColor3fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glColor3i, (GLint red, GLint green, GLint blue));
  TSGL_PROTOTYPE(void, glColor3iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glColor3s, (GLshort red, GLshort green, GLshort blue));
  TSGL_PROTOTYPE(void, glColor3sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glColor3ub, (GLubyte red, GLubyte green, GLubyte blue));
  TSGL_PROTOTYPE(void, glColor3ubv, (const GLubyte *v));
  TSGL_PROTOTYPE(void, glColor3ui, (GLuint red, GLuint green, GLuint blue));
  TSGL_PROTOTYPE(void, glColor3uiv, (const GLuint *v));
  TSGL_PROTOTYPE(void, glColor3us, (GLushort red, GLushort green, GLushort blue));
  TSGL_PROTOTYPE(void, glColor3usv, (const GLushort *v));
  TSGL_PROTOTYPE(void, glColor4b, (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha));
  TSGL_PROTOTYPE(void, glColor4bv, (const GLbyte *v));
  TSGL_PROTOTYPE(void, glColor4d, (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha));
  TSGL_PROTOTYPE(void, glColor4dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha));
  TSGL_PROTOTYPE(void, glColor4fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glColor4i, (GLint red, GLint green, GLint blue, GLint alpha));
  TSGL_PROTOTYPE(void, glColor4iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glColor4s, (GLshort red, GLshort green, GLshort blue, GLshort alpha));
  TSGL_PROTOTYPE(void, glColor4sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glColor4ub, (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha));
  TSGL_PROTOTYPE(void, glColor4ubv, (const GLubyte *v));
  TSGL_PROTOTYPE(void, glColor4ui, (GLuint red, GLuint green, GLuint blue, GLuint alpha));
  TSGL_PROTOTYPE(void, glColor4uiv, (const GLuint *v));
  TSGL_PROTOTYPE(void, glColor4us, (GLushort red, GLushort green, GLushort blue, GLushort alpha));
  TSGL_PROTOTYPE(void, glColor4usv, (const GLushort *v));
  TSGL_PROTOTYPE(void, glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha));
  TSGL_PROTOTYPE(void, glColorMaterial, (GLenum face, GLenum mode));
  TSGL_PROTOTYPE(void, glColorPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void, glCopyPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type));
  TSGL_PROTOTYPE(void, glCopyTexImage1D, (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border));
  TSGL_PROTOTYPE(void, glCopyTexImage2D, (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border));
  TSGL_PROTOTYPE(void, glCopyTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width));
  TSGL_PROTOTYPE(void, glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(void, glCullFace, (GLenum mode));
  TSGL_PROTOTYPE(void, glDeleteLists, (GLuint list, GLsizei range));
  TSGL_PROTOTYPE(void, glDeleteTextures, (GLsizei n, const GLuint *textures));
  TSGL_PROTOTYPE(void, glDepthFunc, (GLenum func));
  TSGL_PROTOTYPE(void, glDepthMask, (GLboolean flag));
  TSGL_PROTOTYPE(void, glDepthRange, (GLclampd zNear, GLclampd zFar));
  TSGL_PROTOTYPE(void, glDisable, (GLenum cap));
  TSGL_PROTOTYPE(void, glDisableClientState, (GLenum array));
  TSGL_PROTOTYPE(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count));
  TSGL_PROTOTYPE(void, glDrawBuffer, (GLenum mode));
  TSGL_PROTOTYPE(void, glDrawElements, (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices));
  TSGL_PROTOTYPE(void, glDrawPixels, (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels));
  TSGL_PROTOTYPE(void, glEdgeFlag, (GLboolean flag));
  TSGL_PROTOTYPE(void, glEdgeFlagPointer, (GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void, glEdgeFlagv, (const GLboolean *flag));
  TSGL_PROTOTYPE(void, glEnable, (GLenum cap));
  TSGL_PROTOTYPE(void, glEnableClientState, (GLenum array));
  TSGL_PROTOTYPE(void, glEnd, (void));
  TSGL_PROTOTYPE(void, glEndList, (void));
  TSGL_PROTOTYPE(void, glEvalCoord1d, (GLdouble u));
  TSGL_PROTOTYPE(void, glEvalCoord1dv, (const GLdouble *u));
  TSGL_PROTOTYPE(void, glEvalCoord1f, (GLfloat u));
  TSGL_PROTOTYPE(void, glEvalCoord1fv, (const GLfloat *u));
  TSGL_PROTOTYPE(void, glEvalCoord2d, (GLdouble u, GLdouble v));
  TSGL_PROTOTYPE(void, glEvalCoord2dv, (const GLdouble *u));
  TSGL_PROTOTYPE(void, glEvalCoord2f, (GLfloat u, GLfloat v));
  TSGL_PROTOTYPE(void, glEvalCoord2fv, (const GLfloat *u));
  TSGL_PROTOTYPE(void, glEvalMesh1, (GLenum mode, GLint i1, GLint i2));
  TSGL_PROTOTYPE(void, glEvalMesh2, (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2));
  TSGL_PROTOTYPE(void, glEvalPoint1, (GLint i));
  TSGL_PROTOTYPE(void, glEvalPoint2, (GLint i, GLint j));
  TSGL_PROTOTYPE(void, glFeedbackBuffer, (GLsizei size, GLenum type, GLfloat *buffer));
  TSGL_PROTOTYPE(void, glFinish, (void));
  TSGL_PROTOTYPE(void, glFlush, (void));
  TSGL_PROTOTYPE(void, glFogf, (GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glFogfv, (GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void, glFogi, (GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glFogiv, (GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void, glFrontFace, (GLenum mode));
  TSGL_PROTOTYPE(void, glFrustum, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar));
  TSGL_PROTOTYPE(GLuint, glGenLists, (GLsizei range));
  TSGL_PROTOTYPE(void, glGenTextures, (GLsizei n, GLuint *textures));
  TSGL_PROTOTYPE(void, glGetBooleanv, (GLenum pname, GLboolean *params));
  TSGL_PROTOTYPE(void, glGetClipPlane, (GLenum plane, GLdouble *equation));
  TSGL_PROTOTYPE(void, glGetDoublev, (GLenum pname, GLdouble *params));
  TSGL_PROTOTYPE(GLenum, glGetError, (void));
  TSGL_PROTOTYPE(void, glGetFloatv, (GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void, glGetIntegerv, (GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void, glGetLightfv, (GLenum light, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void, glGetLightiv, (GLenum light, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void, glGetMapdv, (GLenum target, GLenum query, GLdouble *v));
  TSGL_PROTOTYPE(void, glGetMapfv, (GLenum target, GLenum query, GLfloat *v));
  TSGL_PROTOTYPE(void, glGetMapiv, (GLenum target, GLenum query, GLint *v));
  TSGL_PROTOTYPE(void, glGetMaterialfv, (GLenum face, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void, glGetMaterialiv, (GLenum face, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void, glGetPixelMapfv, (GLenum map, GLfloat *values));
  TSGL_PROTOTYPE(void, glGetPixelMapuiv, (GLenum map, GLuint *values));
  TSGL_PROTOTYPE(void, glGetPixelMapusv, (GLenum map, GLushort *values));
  TSGL_PROTOTYPE(void, glGetPointerv, (GLenum pname, GLvoid* *params));
  TSGL_PROTOTYPE(void, glGetPolygonStipple, (GLubyte *mask));
  TSGL_PROTOTYPE(const GLubyte*, glGetString, (GLenum name));
  TSGL_PROTOTYPE(void, glGetTexEnvfv, (GLenum target, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void, glGetTexEnviv, (GLenum target, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void, glGetTexGendv, (GLenum coord, GLenum pname, GLdouble *params));
  TSGL_PROTOTYPE(void, glGetTexGenfv, (GLenum coord, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void, glGetTexGeniv, (GLenum coord, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void, glGetTexImage, (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels));
  TSGL_PROTOTYPE(void, glGetTexLevelParameterfv, (GLenum target, GLint level, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void, glGetTexLevelParameteriv, (GLenum target, GLint level, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void, glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void, glGetTexParameteriv, (GLenum target, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void, glHint, (GLenum target, GLenum mode));
  TSGL_PROTOTYPE(void, glIndexMask, (GLuint mask));
  TSGL_PROTOTYPE(void, glIndexPointer, (GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void, glIndexd, (GLdouble c));
  TSGL_PROTOTYPE(void, glIndexdv, (const GLdouble *c));
  TSGL_PROTOTYPE(void, glIndexf, (GLfloat c));
  TSGL_PROTOTYPE(void, glIndexfv, (const GLfloat *c));
  TSGL_PROTOTYPE(void, glIndexi, (GLint c));
  TSGL_PROTOTYPE(void, glIndexiv, (const GLint *c));
  TSGL_PROTOTYPE(void, glIndexs, (GLshort c));
  TSGL_PROTOTYPE(void, glIndexsv, (const GLshort *c));
  TSGL_PROTOTYPE(void, glIndexub, (GLubyte c));
  TSGL_PROTOTYPE(void, glIndexubv, (const GLubyte *c));
  TSGL_PROTOTYPE(void, glInitNames, (void));
  TSGL_PROTOTYPE(void, glInterleavedArrays, (GLenum format, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(GLboolean, glIsEnabled, (GLenum cap));
  TSGL_PROTOTYPE(GLboolean, glIsList, (GLuint list));
  TSGL_PROTOTYPE(GLboolean, glIsTexture, (GLuint texture));
  TSGL_PROTOTYPE(void, glLightModelf, (GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glLightModelfv, (GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void, glLightModeli, (GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glLightModeliv, (GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void, glLightf, (GLenum light, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glLightfv, (GLenum light, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void, glLighti, (GLenum light, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glLightiv, (GLenum light, GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void, glLineStipple, (GLint factor, GLushort pattern));
  TSGL_PROTOTYPE(void, glLineWidth, (GLfloat width));
  TSGL_PROTOTYPE(void, glListBase, (GLuint base));
  TSGL_PROTOTYPE(void, glLoadIdentity, (void));
  TSGL_PROTOTYPE(void, glLoadMatrixd, (const GLdouble *m));
  TSGL_PROTOTYPE(void, glLoadMatrixf, (const GLfloat *m));
  TSGL_PROTOTYPE(void, glLoadName, (GLuint name));
  TSGL_PROTOTYPE(void, glLogicOp, (GLenum opcode));
  TSGL_PROTOTYPE(void, glMap1d, (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points));
  TSGL_PROTOTYPE(void, glMap1f, (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points));
  TSGL_PROTOTYPE(void, glMap2d, (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points));
  TSGL_PROTOTYPE(void, glMap2f, (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points));
  TSGL_PROTOTYPE(void, glMapGrid1d, (GLint un, GLdouble u1, GLdouble u2));
  TSGL_PROTOTYPE(void, glMapGrid1f, (GLint un, GLfloat u1, GLfloat u2));
  TSGL_PROTOTYPE(void, glMapGrid2d, (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2));
  TSGL_PROTOTYPE(void, glMapGrid2f, (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2));
  TSGL_PROTOTYPE(void, glMaterialf, (GLenum face, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glMaterialfv, (GLenum face, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void, glMateriali, (GLenum face, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glMaterialiv, (GLenum face, GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void, glMatrixMode, (GLenum mode));
  TSGL_PROTOTYPE(void, glMultMatrixd, (const GLdouble *m));
  TSGL_PROTOTYPE(void, glMultMatrixf, (const GLfloat *m));
  TSGL_PROTOTYPE(void, glNewList, (GLuint list, GLenum mode));
  TSGL_PROTOTYPE(void, glNormal3b, (GLbyte nx, GLbyte ny, GLbyte nz));
  TSGL_PROTOTYPE(void, glNormal3bv, (const GLbyte *v));
  TSGL_PROTOTYPE(void, glNormal3d, (GLdouble nx, GLdouble ny, GLdouble nz));
  TSGL_PROTOTYPE(void, glNormal3dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glNormal3f, (GLfloat nx, GLfloat ny, GLfloat nz));
  TSGL_PROTOTYPE(void, glNormal3fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glNormal3i, (GLint nx, GLint ny, GLint nz));
  TSGL_PROTOTYPE(void, glNormal3iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glNormal3s, (GLshort nx, GLshort ny, GLshort nz));
  TSGL_PROTOTYPE(void, glNormal3sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glNormalPointer, (GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void, glOrtho, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar));
  TSGL_PROTOTYPE(void, glPassThrough, (GLfloat token));
  TSGL_PROTOTYPE(void, glPixelMapfv, (GLenum map, GLsizei mapsize, const GLfloat *values));
  TSGL_PROTOTYPE(void, glPixelMapuiv, (GLenum map, GLsizei mapsize, const GLuint *values));
  TSGL_PROTOTYPE(void, glPixelMapusv, (GLenum map, GLsizei mapsize, const GLushort *values));
  TSGL_PROTOTYPE(void, glPixelStoref, (GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glPixelStorei, (GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glPixelTransferf, (GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glPixelTransferi, (GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glPixelZoom, (GLfloat xfactor, GLfloat yfactor));
  TSGL_PROTOTYPE(void, glPointSize, (GLfloat size));
  TSGL_PROTOTYPE(void, glPolygonMode, (GLenum face, GLenum mode));
  TSGL_PROTOTYPE(void, glPolygonOffset, (GLfloat factor, GLfloat units));
  TSGL_PROTOTYPE(void, glPolygonStipple, (const GLubyte *mask));
  TSGL_PROTOTYPE(void, glPopAttrib, (void));
  TSGL_PROTOTYPE(void, glPopClientAttrib, (void));
  TSGL_PROTOTYPE(void, glPopMatrix, (void));
  TSGL_PROTOTYPE(void, glPopName, (void));
  TSGL_PROTOTYPE(void, glPrioritizeTextures, (GLsizei n, const GLuint *textures, const GLclampf *priorities));
  TSGL_PROTOTYPE(void, glPushAttrib, (GLbitfield mask));
  TSGL_PROTOTYPE(void, glPushClientAttrib, (GLbitfield mask));
  TSGL_PROTOTYPE(void, glPushMatrix, (void));
  TSGL_PROTOTYPE(void, glPushName, (GLuint name));
  TSGL_PROTOTYPE(void, glRasterPos2d, (GLdouble x, GLdouble y));
  TSGL_PROTOTYPE(void, glRasterPos2dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glRasterPos2f, (GLfloat x, GLfloat y));
  TSGL_PROTOTYPE(void, glRasterPos2fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glRasterPos2i, (GLint x, GLint y));
  TSGL_PROTOTYPE(void, glRasterPos2iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glRasterPos2s, (GLshort x, GLshort y));
  TSGL_PROTOTYPE(void, glRasterPos2sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glRasterPos3d, (GLdouble x, GLdouble y, GLdouble z));
  TSGL_PROTOTYPE(void, glRasterPos3dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glRasterPos3f, (GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void, glRasterPos3fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glRasterPos3i, (GLint x, GLint y, GLint z));
  TSGL_PROTOTYPE(void, glRasterPos3iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glRasterPos3s, (GLshort x, GLshort y, GLshort z));
  TSGL_PROTOTYPE(void, glRasterPos3sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glRasterPos4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w));
  TSGL_PROTOTYPE(void, glRasterPos4dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glRasterPos4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w));
  TSGL_PROTOTYPE(void, glRasterPos4fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glRasterPos4i, (GLint x, GLint y, GLint z, GLint w));
  TSGL_PROTOTYPE(void, glRasterPos4iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glRasterPos4s, (GLshort x, GLshort y, GLshort z, GLshort w));
  TSGL_PROTOTYPE(void, glRasterPos4sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glReadBuffer, (GLenum mode));
  TSGL_PROTOTYPE(void, glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels));
  TSGL_PROTOTYPE(void, glRectd, (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2));
  TSGL_PROTOTYPE(void, glRectdv, (const GLdouble *v1, const GLdouble *v2));
  TSGL_PROTOTYPE(void, glRectf, (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2));
  TSGL_PROTOTYPE(void, glRectfv, (const GLfloat *v1, const GLfloat *v2));
  TSGL_PROTOTYPE(void, glRecti, (GLint x1, GLint y1, GLint x2, GLint y2));
  TSGL_PROTOTYPE(void, glRectiv, (const GLint *v1, const GLint *v2));
  TSGL_PROTOTYPE(void, glRects, (GLshort x1, GLshort y1, GLshort x2, GLshort y2));
  TSGL_PROTOTYPE(void, glRectsv, (const GLshort *v1, const GLshort *v2));
  TSGL_PROTOTYPE(GLint, glRenderMode, (GLenum mode));
  TSGL_PROTOTYPE(void, glRotated, (GLdouble angle, GLdouble x, GLdouble y, GLdouble z));
  TSGL_PROTOTYPE(void, glRotatef, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void, glScaled, (GLdouble x, GLdouble y, GLdouble z));
  TSGL_PROTOTYPE(void, glScalef, (GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void, glScissor, (GLint x, GLint y, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(void, glSelectBuffer, (GLsizei size, GLuint *buffer));
  TSGL_PROTOTYPE(void, glShadeModel, (GLenum mode));
  TSGL_PROTOTYPE(void, glStencilFunc, (GLenum func, GLint ref, GLuint mask));
  TSGL_PROTOTYPE(void, glStencilMask, (GLuint mask));
  TSGL_PROTOTYPE(void, glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass));
  TSGL_PROTOTYPE(void, glTexCoord1d, (GLdouble s));
  TSGL_PROTOTYPE(void, glTexCoord1dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glTexCoord1f, (GLfloat s));
  TSGL_PROTOTYPE(void, glTexCoord1fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glTexCoord1i, (GLint s));
  TSGL_PROTOTYPE(void, glTexCoord1iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glTexCoord1s, (GLshort s));
  TSGL_PROTOTYPE(void, glTexCoord1sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glTexCoord2d, (GLdouble s, GLdouble t));
  TSGL_PROTOTYPE(void, glTexCoord2dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glTexCoord2f, (GLfloat s, GLfloat t));
  TSGL_PROTOTYPE(void, glTexCoord2fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glTexCoord2i, (GLint s, GLint t));
  TSGL_PROTOTYPE(void, glTexCoord2iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glTexCoord2s, (GLshort s, GLshort t));
  TSGL_PROTOTYPE(void, glTexCoord2sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glTexCoord3d, (GLdouble s, GLdouble t, GLdouble r));
  TSGL_PROTOTYPE(void, glTexCoord3dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glTexCoord3f, (GLfloat s, GLfloat t, GLfloat r));
  TSGL_PROTOTYPE(void, glTexCoord3fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glTexCoord3i, (GLint s, GLint t, GLint r));
  TSGL_PROTOTYPE(void, glTexCoord3iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glTexCoord3s, (GLshort s, GLshort t, GLshort r));
  TSGL_PROTOTYPE(void, glTexCoord3sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glTexCoord4d, (GLdouble s, GLdouble t, GLdouble r, GLdouble q));
  TSGL_PROTOTYPE(void, glTexCoord4dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glTexCoord4f, (GLfloat s, GLfloat t, GLfloat r, GLfloat q));
  TSGL_PROTOTYPE(void, glTexCoord4fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glTexCoord4i, (GLint s, GLint t, GLint r, GLint q));
  TSGL_PROTOTYPE(void, glTexCoord4iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glTexCoord4s, (GLshort s, GLshort t, GLshort r, GLshort q));
  TSGL_PROTOTYPE(void, glTexCoord4sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void, glTexEnvf, (GLenum target, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glTexEnvfv, (GLenum target, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void, glTexEnvi, (GLenum target, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glTexEnviv, (GLenum target, GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void, glTexGend, (GLenum coord, GLenum pname, GLdouble param));
  TSGL_PROTOTYPE(void, glTexGendv, (GLenum coord, GLenum pname, const GLdouble *params));
  TSGL_PROTOTYPE(void, glTexGenf, (GLenum coord, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glTexGenfv, (GLenum coord, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void, glTexGeni, (GLenum coord, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glTexGeniv, (GLenum coord, GLenum pname, const GLint *params));
#if defined __SH4__
  TSGL_PROTOTYPE(void, glTexImage1D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
  TSGL_PROTOTYPE(void, glTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
#else
  TSGL_PROTOTYPE(void, glTexImage1D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
  TSGL_PROTOTYPE(void, glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
#endif
  TSGL_PROTOTYPE(void, glTexParameterf, (GLenum target, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void, glTexParameterfv, (GLenum target, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void, glTexParameteri, (GLenum target, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void, glTexParameteriv, (GLenum target, GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void, glTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels));
  TSGL_PROTOTYPE(void, glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels));
  TSGL_PROTOTYPE(void, glTranslated, (GLdouble x, GLdouble y, GLdouble z));
  TSGL_PROTOTYPE(void, glTranslatef, (GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void, glVertex2d, (GLdouble x, GLdouble y));
  TSGL_PROTOTYPE(void, glVertex2dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glVertex2f, (GLfloat x, GLfloat y));
  TSGL_PROTOTYPE(void, glVertex2fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glVertex2i, (GLint x, GLint y));
  TSGL_PROTOTYPE(void, glVertex2iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glVertex2s, (GLshort x, GLshort y));
  TSGL_PROTOTYPE(void, glVertex2sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glVertex3d, (GLdouble x, GLdouble y, GLdouble z));
  TSGL_PROTOTYPE(void, glVertex3dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glVertex3f, (GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void, glVertex3fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glVertex3i, (GLint x, GLint y, GLint z));
  TSGL_PROTOTYPE(void, glVertex3iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glVertex3s, (GLshort x, GLshort y, GLshort z));
  TSGL_PROTOTYPE(void, glVertex3sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glVertex4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w));
  TSGL_PROTOTYPE(void, glVertex4dv, (const GLdouble *v));
  TSGL_PROTOTYPE(void, glVertex4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w));
  TSGL_PROTOTYPE(void, glVertex4fv, (const GLfloat *v));
  TSGL_PROTOTYPE(void, glVertex4i, (GLint x, GLint y, GLint z, GLint w));
  TSGL_PROTOTYPE(void, glVertex4iv, (const GLint *v));
  TSGL_PROTOTYPE(void, glVertex4s, (GLshort x, GLshort y, GLshort z, GLshort w));
  TSGL_PROTOTYPE(void, glVertex4sv, (const GLshort *v));
  TSGL_PROTOTYPE(void, glVertexPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void, glViewport, (GLint x, GLint y, GLsizei width, GLsizei height));
#endif

  //===========================================================================
  //
  //  GLES v1
  //
  //===========================================================================
#ifdef __GLES1__
  TSGL_PROTOTYPE(void,glActiveTexture, (GLenum texture));
  TSGL_PROTOTYPE(void,glAlphaFunc, (GLenum func, GLclampf ref));
  TSGL_PROTOTYPE(void,glClearColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
  TSGL_PROTOTYPE(void,glClearDepthf, (GLclampf depth));
  TSGL_PROTOTYPE(void,glClipPlanef, (GLenum plane, const GLfloat *equation));
  TSGL_PROTOTYPE(void,glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha));
  TSGL_PROTOTYPE(void,glDepthRangef, (GLclampf zNear, GLclampf zFar));
  TSGL_PROTOTYPE(void,glFogf, (GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void,glFogfv, (GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void,glFrustumf, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar));
  TSGL_PROTOTYPE(void,glGetClipPlanef, (GLenum pname, GLfloat eqn[4]));
  TSGL_PROTOTYPE(void,glGetFloatv, (GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void,glGetLightfv, (GLenum light, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void,glGetMaterialfv, (GLenum face, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void,glGetTexEnvfv, (GLenum env, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void,glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat *params));
  TSGL_PROTOTYPE(void,glLightModelf, (GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void,glLightModelfv, (GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void,glLightf, (GLenum light, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void,glLightfv, (GLenum light, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void,glLineWidth, (GLfloat width));
  TSGL_PROTOTYPE(void,glLoadMatrixf, (const GLfloat *m));
  TSGL_PROTOTYPE(void,glMaterialf, (GLenum face, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void,glMaterialfv, (GLenum face, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void,glMultMatrixf, (const GLfloat *m));
  TSGL_PROTOTYPE(void,glMultiTexCoord4f, (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q));
  TSGL_PROTOTYPE(void,glNormal3f, (GLfloat nx, GLfloat ny, GLfloat nz));
  TSGL_PROTOTYPE(void,glOrthof, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar));
  TSGL_PROTOTYPE(void,glPointParameterf, (GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void,glPointParameterfv, (GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void,glPointSize, (GLfloat size));
  TSGL_PROTOTYPE(void,glPolygonOffset, (GLfloat factor, GLfloat units));
  TSGL_PROTOTYPE(void,glRotatef, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void,glScalef, (GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void,glTexEnvf, (GLenum target, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void,glTexEnvfv, (GLenum target, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void,glTexParameterf, (GLenum target, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void,glTexParameterfv, (GLenum target, GLenum pname, const GLfloat *params));
  TSGL_PROTOTYPE(void,glTranslatef, (GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void,glAlphaFuncx, (GLenum func, GLclampx ref));
  TSGL_PROTOTYPE(void,glBindBuffer, (GLenum target, GLuint buffer));
  TSGL_PROTOTYPE(void,glBindTexture, (GLenum target, GLuint texture));
  TSGL_PROTOTYPE(void,glBlendFunc, (GLenum sfactor, GLenum dfactor));
  TSGL_PROTOTYPE(void,glBufferData, (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage));
  TSGL_PROTOTYPE(void,glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data));
  TSGL_PROTOTYPE(void,glClear, (GLbitfield mask));
  TSGL_PROTOTYPE(void,glClearColorx, (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha));
  TSGL_PROTOTYPE(void,glClearDepthx, (GLclampx depth));
  TSGL_PROTOTYPE(void,glClearStencil, (GLint s));
  TSGL_PROTOTYPE(void,glClientActiveTexture, (GLenum texture));
  TSGL_PROTOTYPE(void,glClipPlanex, (GLenum plane, const GLfixed *equation));
  TSGL_PROTOTYPE(void,glColor4ub, (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha));
  TSGL_PROTOTYPE(void,glColor4x, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha));
  TSGL_PROTOTYPE(void,glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha));
  TSGL_PROTOTYPE(void,glColorPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void,glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data));
  TSGL_PROTOTYPE(void,glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data));
  TSGL_PROTOTYPE(void,glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border));
  TSGL_PROTOTYPE(void,glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(void,glCullFace, (GLenum mode));
  TSGL_PROTOTYPE(void,glDeleteBuffers, (GLsizei n, const GLuint *buffers));
  TSGL_PROTOTYPE(void,glDeleteTextures, (GLsizei n, const GLuint *textures));
  TSGL_PROTOTYPE(void,glDepthFunc, (GLenum func));
  TSGL_PROTOTYPE(void,glDepthMask, (GLboolean flag));
  TSGL_PROTOTYPE(void,glDepthRangex, (GLclampx zNear, GLclampx zFar));
  TSGL_PROTOTYPE(void,glDisable, (GLenum cap));
  TSGL_PROTOTYPE(void,glDisableClientState, (GLenum array));
  TSGL_PROTOTYPE(void,glDrawArrays, (GLenum mode, GLint first, GLsizei count));
  TSGL_PROTOTYPE(void,glDrawElements, (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices));
  TSGL_PROTOTYPE(void,glEnable, (GLenum cap));
  TSGL_PROTOTYPE(void,glEnableClientState, (GLenum array));
  TSGL_PROTOTYPE(void,glFinish, (void));
  TSGL_PROTOTYPE(void,glFlush, (void));
  TSGL_PROTOTYPE(void,glFogx, (GLenum pname, GLfixed param));
  TSGL_PROTOTYPE(void,glFogxv, (GLenum pname, const GLfixed *params));
  TSGL_PROTOTYPE(void,glFrontFace, (GLenum mode));
  TSGL_PROTOTYPE(void,glFrustumx, (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar));
  TSGL_PROTOTYPE(void,glGetBooleanv, (GLenum pname, GLboolean *params));
  TSGL_PROTOTYPE(void,glGetBufferParameteriv, (GLenum target, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void,glGetClipPlanex, (GLenum pname, GLfixed eqn[4]));
  TSGL_PROTOTYPE(void,glGenBuffers, (GLsizei n, GLuint *buffers));
  TSGL_PROTOTYPE(void,glGenTextures, (GLsizei n, GLuint *textures));
  TSGL_PROTOTYPE(GLenum,glGetError, (void));
  TSGL_PROTOTYPE(void,glGetFixedv, (GLenum pname, GLfixed *params));
  TSGL_PROTOTYPE(void,glGetIntegerv, (GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void,glGetLightxv, (GLenum light, GLenum pname, GLfixed *params));
  TSGL_PROTOTYPE(void,glGetMaterialxv, (GLenum face, GLenum pname, GLfixed *params));
  TSGL_PROTOTYPE(void,glGetPointerv, (GLenum pname, void **params));
  TSGL_PROTOTYPE(const GLubyte *,glGetString, (GLenum name));
  TSGL_PROTOTYPE(void,glGetTexEnviv, (GLenum env, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void,glGetTexEnvxv, (GLenum env, GLenum pname, GLfixed *params));
  TSGL_PROTOTYPE(void,glGetTexParameteriv, (GLenum target, GLenum pname, GLint *params));
  TSGL_PROTOTYPE(void,glGetTexParameterxv, (GLenum target, GLenum pname, GLfixed *params));
  TSGL_PROTOTYPE(void,glHint, (GLenum target, GLenum mode));
  TSGL_PROTOTYPE(GLboolean,glIsBuffer, (GLuint buffer));
  TSGL_PROTOTYPE(GLboolean,glIsEnabled, (GLenum cap));
  TSGL_PROTOTYPE(GLboolean,glIsTexture, (GLuint texture));
  TSGL_PROTOTYPE(void,glLightModelx, (GLenum pname, GLfixed param));
  TSGL_PROTOTYPE(void,glLightModelxv, (GLenum pname, const GLfixed *params));
  TSGL_PROTOTYPE(void,glLightx, (GLenum light, GLenum pname, GLfixed param));
  TSGL_PROTOTYPE(void,glLightxv, (GLenum light, GLenum pname, const GLfixed *params));
  TSGL_PROTOTYPE(void,glLineWidthx, (GLfixed width));
  TSGL_PROTOTYPE(void,glLoadIdentity, (void));
  TSGL_PROTOTYPE(void,glLoadMatrixx, (const GLfixed *m));
  TSGL_PROTOTYPE(void,glLogicOp, (GLenum opcode));
  TSGL_PROTOTYPE(void,glMaterialx, (GLenum face, GLenum pname, GLfixed param));
  TSGL_PROTOTYPE(void,glMaterialxv, (GLenum face, GLenum pname, const GLfixed *params));
  TSGL_PROTOTYPE(void,glMatrixMode, (GLenum mode));
  TSGL_PROTOTYPE(void,glMultMatrixx, (const GLfixed *m));
  TSGL_PROTOTYPE(void,glMultiTexCoord4x, (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q));
  TSGL_PROTOTYPE(void,glNormal3x, (GLfixed nx, GLfixed ny, GLfixed nz));
  TSGL_PROTOTYPE(void,glNormalPointer, (GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void,glOrthox, (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar));
  TSGL_PROTOTYPE(void,glPixelStorei, (GLenum pname, GLint param));
  TSGL_PROTOTYPE(void,glPointParameterx, (GLenum pname, GLfixed param));
  TSGL_PROTOTYPE(void,glPointParameterxv, (GLenum pname, const GLfixed *params));
  TSGL_PROTOTYPE(void,glPointSizex, (GLfixed size));
  TSGL_PROTOTYPE(void,glPolygonOffsetx, (GLfixed factor, GLfixed units));
  TSGL_PROTOTYPE(void,glPopMatrix, (void));
  TSGL_PROTOTYPE(void,glPushMatrix, (void));
  TSGL_PROTOTYPE(void,glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels));
  TSGL_PROTOTYPE(void,glRotatex, (GLfixed angle, GLfixed x, GLfixed y, GLfixed z));
  TSGL_PROTOTYPE(void,glSampleCoverage, (GLclampf value, GLboolean invert));
  TSGL_PROTOTYPE(void,glSampleCoveragex, (GLclampx value, GLboolean invert));
  TSGL_PROTOTYPE(void,glScalex, (GLfixed x, GLfixed y, GLfixed z));
  TSGL_PROTOTYPE(void,glScissor, (GLint x, GLint y, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(void,glShadeModel, (GLenum mode));
  TSGL_PROTOTYPE(void,glStencilFunc, (GLenum func, GLint ref, GLuint mask));
  TSGL_PROTOTYPE(void,glStencilMask, (GLuint mask));
  TSGL_PROTOTYPE(void,glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass));
  TSGL_PROTOTYPE(void,glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void,glTexEnvi, (GLenum target, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void,glTexEnvx, (GLenum target, GLenum pname, GLfixed param));
  TSGL_PROTOTYPE(void,glTexEnviv, (GLenum target, GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void,glTexEnvxv, (GLenum target, GLenum pname, const GLfixed *params));
  TSGL_PROTOTYPE(void,glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
  TSGL_PROTOTYPE(void,glTexParameteri, (GLenum target, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void,glTexParameterx, (GLenum target, GLenum pname, GLfixed param));
  TSGL_PROTOTYPE(void,glTexParameteriv, (GLenum target, GLenum pname, const GLint *params));
  TSGL_PROTOTYPE(void,glTexParameterxv, (GLenum target, GLenum pname, const GLfixed *params));
  TSGL_PROTOTYPE(void,glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels));
  TSGL_PROTOTYPE(void,glTranslatex, (GLfixed x, GLfixed y, GLfixed z));
  TSGL_PROTOTYPE(void,glVertexPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
  TSGL_PROTOTYPE(void,glViewport, (GLint x, GLint y, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(void,glPointSizePointerOES, (GLenum type, GLsizei stride, const GLvoid *pointer));
#endif // __GLES1__

  //===========================================================================
  //
  //  GLES v2
  //
  //===========================================================================
#ifdef __GLES2__
  TSGL_PROTOTYPE(void        ,glActiveTexture, (GLenum texture));
  TSGL_PROTOTYPE(void        ,glAttachShader, (GLuint program, GLuint shader));
  TSGL_PROTOTYPE(void        ,glBindAttribLocation, (GLuint program, GLuint index, const char* name));
  TSGL_PROTOTYPE(void        ,glBindBuffer, (GLenum target, GLuint buffer));
  TSGL_PROTOTYPE(void        ,glBindFramebuffer, (GLenum target, GLuint framebuffer));
  TSGL_PROTOTYPE(void        ,glBindRenderbuffer, (GLenum target, GLuint renderbuffer));
  TSGL_PROTOTYPE(void        ,glBindTexture, (GLenum target, GLuint texture));
  TSGL_PROTOTYPE(void        ,glBlendColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
  TSGL_PROTOTYPE(void        ,glBlendEquation, ( GLenum mode ));
  TSGL_PROTOTYPE(void        ,glBlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha));
  TSGL_PROTOTYPE(void        ,glBlendFunc, (GLenum sfactor, GLenum dfactor));
  TSGL_PROTOTYPE(void        ,glBlendFuncSeparate, (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha));
  TSGL_PROTOTYPE(void        ,glBufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage));
  TSGL_PROTOTYPE(void        ,glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data));
  TSGL_PROTOTYPE(GLenum      ,glCheckFramebufferStatus, (GLenum target));
  TSGL_PROTOTYPE(void        ,glClear, (GLbitfield mask));
  TSGL_PROTOTYPE(void        ,glClearColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
  TSGL_PROTOTYPE(void        ,glClearDepthf, (GLclampf depth));
  TSGL_PROTOTYPE(void        ,glClearStencil, (GLint s));
  TSGL_PROTOTYPE(void        ,glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha));
  TSGL_PROTOTYPE(void        ,glCompileShader, (GLuint shader));
  TSGL_PROTOTYPE(void        ,glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data));
  TSGL_PROTOTYPE(void        ,glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data));
  TSGL_PROTOTYPE(void        ,glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border));
  TSGL_PROTOTYPE(void        ,glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(GLuint      ,glCreateProgram, (void));
  TSGL_PROTOTYPE(GLuint      ,glCreateShader, (GLenum type));
  TSGL_PROTOTYPE(void        ,glCullFace, (GLenum mode));
  TSGL_PROTOTYPE(void        ,glDeleteBuffers, (GLsizei n, const GLuint* buffers));
  TSGL_PROTOTYPE(void        ,glDeleteFramebuffers, (GLsizei n, const GLuint* framebuffers));
  TSGL_PROTOTYPE(void        ,glDeleteProgram, (GLuint program));
  TSGL_PROTOTYPE(void        ,glDeleteRenderbuffers, (GLsizei n, const GLuint* renderbuffers));
  TSGL_PROTOTYPE(void        ,glDeleteShader, (GLuint shader));
  TSGL_PROTOTYPE(void        ,glDeleteTextures, (GLsizei n, const GLuint* textures));
  TSGL_PROTOTYPE(void        ,glDepthFunc, (GLenum func));
  TSGL_PROTOTYPE(void        ,glDepthMask, (GLboolean flag));
  TSGL_PROTOTYPE(void        ,glDepthRangef, (GLclampf zNear, GLclampf zFar));
  TSGL_PROTOTYPE(void        ,glDetachShader, (GLuint program, GLuint shader));
  TSGL_PROTOTYPE(void        ,glDisable, (GLenum cap));
  TSGL_PROTOTYPE(void        ,glDisableVertexAttribArray, (GLuint index));
  TSGL_PROTOTYPE(void        ,glDrawArrays, (GLenum mode, GLint first, GLsizei count));
  TSGL_PROTOTYPE(void        ,glDrawElements, (GLenum mode, GLsizei count, GLenum type, const void* indices));
  TSGL_PROTOTYPE(void        ,glEnable, (GLenum cap));
  TSGL_PROTOTYPE(void        ,glEnableVertexAttribArray, (GLuint index));
  TSGL_PROTOTYPE(void        ,glFinish, (void));
  TSGL_PROTOTYPE(void        ,glFlush, (void));
  TSGL_PROTOTYPE(void        ,glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer));
  TSGL_PROTOTYPE(void        ,glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));
  TSGL_PROTOTYPE(void        ,glFrontFace, (GLenum mode));
  TSGL_PROTOTYPE(void        ,glGenBuffers, (GLsizei n, GLuint* buffers));
  TSGL_PROTOTYPE(void        ,glGenerateMipmap, (GLenum target));
  TSGL_PROTOTYPE(void        ,glGenFramebuffers, (GLsizei n, GLuint* framebuffers));
  TSGL_PROTOTYPE(void        ,glGenRenderbuffers, (GLsizei n, GLuint* renderbuffers));
  TSGL_PROTOTYPE(void        ,glGenTextures, (GLsizei n, GLuint* textures));
  TSGL_PROTOTYPE(void        ,glGetActiveAttrib, (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name));
  TSGL_PROTOTYPE(void        ,glGetActiveUniform, (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name));
  TSGL_PROTOTYPE(void        ,glGetAttachedShaders, (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders));
  TSGL_PROTOTYPE(int         ,glGetAttribLocation, (GLuint program, const char* name));
  TSGL_PROTOTYPE(void        ,glGetBooleanv, (GLenum pname, GLboolean* params));
  TSGL_PROTOTYPE(void        ,glGetBufferParameteriv, (GLenum target, GLenum pname, GLint* params));
  TSGL_PROTOTYPE(GLenum      ,glGetError, (void));
  TSGL_PROTOTYPE(void        ,glGetFloatv, (GLenum pname, GLfloat* params));
  TSGL_PROTOTYPE(void        ,glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint* params));
  TSGL_PROTOTYPE(void        ,glGetIntegerv, (GLenum pname, GLint* params));
  TSGL_PROTOTYPE(void        ,glGetProgramiv, (GLuint program, GLenum pname, GLint* params));
  TSGL_PROTOTYPE(void        ,glGetProgramInfoLog, (GLuint program, GLsizei bufsize, GLsizei* length, char* infolog));
  TSGL_PROTOTYPE(void        ,glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint* params));
  TSGL_PROTOTYPE(void        ,glGetShaderiv, (GLuint shader, GLenum pname, GLint* params));
  TSGL_PROTOTYPE(void        ,glGetShaderInfoLog, (GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog));
  TSGL_PROTOTYPE(void        ,glGetShaderPrecisionFormat, (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision));
  TSGL_PROTOTYPE(void        ,glGetShaderSource, (GLuint shader, GLsizei bufsize, GLsizei* length, char* source));
  TSGL_PROTOTYPE(const GLubyte*,glGetString, (GLenum name));
  TSGL_PROTOTYPE(void        ,glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat* params));
  TSGL_PROTOTYPE(void        ,glGetTexParameteriv, (GLenum target, GLenum pname, GLint* params));
  TSGL_PROTOTYPE(void        ,glGetUniformfv, (GLuint program, GLint location, GLfloat* params));
  TSGL_PROTOTYPE(void        ,glGetUniformiv, (GLuint program, GLint location, GLint* params));
  TSGL_PROTOTYPE(int         ,glGetUniformLocation, (GLuint program, const char* name));
  TSGL_PROTOTYPE(void        ,glGetVertexAttribfv, (GLuint index, GLenum pname, GLfloat* params));
  TSGL_PROTOTYPE(void        ,glGetVertexAttribiv, (GLuint index, GLenum pname, GLint* params));
  TSGL_PROTOTYPE(void        ,glGetVertexAttribPointerv, (GLuint index, GLenum pname, void** pointer));
  TSGL_PROTOTYPE(void        ,glHint, (GLenum target, GLenum mode));
  TSGL_PROTOTYPE(GLboolean   ,glIsBuffer, (GLuint buffer));
  TSGL_PROTOTYPE(GLboolean   ,glIsEnabled, (GLenum cap));
  TSGL_PROTOTYPE(GLboolean   ,glIsFramebuffer, (GLuint framebuffer));
  TSGL_PROTOTYPE(GLboolean   ,glIsProgram, (GLuint program));
  TSGL_PROTOTYPE(GLboolean   ,glIsRenderbuffer, (GLuint renderbuffer));
  TSGL_PROTOTYPE(GLboolean   ,glIsShader, (GLuint shader));
  TSGL_PROTOTYPE(GLboolean   ,glIsTexture, (GLuint texture));
  TSGL_PROTOTYPE(void        ,glLineWidth, (GLfloat width));
  TSGL_PROTOTYPE(void        ,glLinkProgram, (GLuint program));
  TSGL_PROTOTYPE(void        ,glPixelStorei, (GLenum pname, GLint param));
  TSGL_PROTOTYPE(void        ,glPolygonOffset, (GLfloat factor, GLfloat units));
  TSGL_PROTOTYPE(void        ,glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels));
  TSGL_PROTOTYPE(void        ,glReleaseShaderCompiler, (void));
  TSGL_PROTOTYPE(void        ,glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(void        ,glSampleCoverage, (GLclampf value, GLboolean invert));
  TSGL_PROTOTYPE(void        ,glScissor, (GLint x, GLint y, GLsizei width, GLsizei height));
  TSGL_PROTOTYPE(void        ,glShaderBinary, (GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length));
  TSGL_PROTOTYPE(void        ,glShaderSource, (GLuint shader, GLsizei count, const char** string, const GLint* length));
  TSGL_PROTOTYPE(void        ,glStencilFunc, (GLenum func, GLint ref, GLuint mask));
  TSGL_PROTOTYPE(void        ,glStencilFuncSeparate, (GLenum face, GLenum func, GLint ref, GLuint mask));
  TSGL_PROTOTYPE(void        ,glStencilMask, (GLuint mask));
  TSGL_PROTOTYPE(void        ,glStencilMaskSeparate, (GLenum face, GLuint mask));
  TSGL_PROTOTYPE(void        ,glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass));
  TSGL_PROTOTYPE(void        ,glStencilOpSeparate, (GLenum face, GLenum fail, GLenum zfail, GLenum zpass));
  TSGL_PROTOTYPE(void        ,glTexImage2D, (GLenum target, GLint level, GLint internalformat,  GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels));
  TSGL_PROTOTYPE(void        ,glTexParameterf, (GLenum target, GLenum pname, GLfloat param));
  TSGL_PROTOTYPE(void        ,glTexParameterfv, (GLenum target, GLenum pname, const GLfloat* params));
  TSGL_PROTOTYPE(void        ,glTexParameteri, (GLenum target, GLenum pname, GLint param));
  TSGL_PROTOTYPE(void        ,glTexParameteriv, (GLenum target, GLenum pname, const GLint* params));
  TSGL_PROTOTYPE(void        ,glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels));
  TSGL_PROTOTYPE(void        ,glUniform1f, (GLint location, GLfloat x));
  TSGL_PROTOTYPE(void        ,glUniform1fv, (GLint location, GLsizei count, const GLfloat* v));
  TSGL_PROTOTYPE(void        ,glUniform1i, (GLint location, GLint x));
  TSGL_PROTOTYPE(void        ,glUniform1iv, (GLint location, GLsizei count, const GLint* v));
  TSGL_PROTOTYPE(void        ,glUniform2f, (GLint location, GLfloat x, GLfloat y));
  TSGL_PROTOTYPE(void        ,glUniform2fv, (GLint location, GLsizei count, const GLfloat* v));
  TSGL_PROTOTYPE(void        ,glUniform2i, (GLint location, GLint x, GLint y));
  TSGL_PROTOTYPE(void        ,glUniform2iv, (GLint location, GLsizei count, const GLint* v));
  TSGL_PROTOTYPE(void        ,glUniform3f, (GLint location, GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void        ,glUniform3fv, (GLint location, GLsizei count, const GLfloat* v));
  TSGL_PROTOTYPE(void        ,glUniform3i, (GLint location, GLint x, GLint y, GLint z));
  TSGL_PROTOTYPE(void        ,glUniform3iv, (GLint location, GLsizei count, const GLint* v));
  TSGL_PROTOTYPE(void        ,glUniform4f, (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w));
  TSGL_PROTOTYPE(void        ,glUniform4fv, (GLint location, GLsizei count, const GLfloat* v));
  TSGL_PROTOTYPE(void        ,glUniform4i, (GLint location, GLint x, GLint y, GLint z, GLint w));
  TSGL_PROTOTYPE(void        ,glUniform4iv, (GLint location, GLsizei count, const GLint* v));
  TSGL_PROTOTYPE(void        ,glUniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
  TSGL_PROTOTYPE(void        ,glUniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
  TSGL_PROTOTYPE(void        ,glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
  TSGL_PROTOTYPE(void        ,glUseProgram, (GLuint program));
  TSGL_PROTOTYPE(void        ,glValidateProgram, (GLuint program));
  TSGL_PROTOTYPE(void        ,glVertexAttrib1f, (GLuint indx, GLfloat x));
  TSGL_PROTOTYPE(void        ,glVertexAttrib1fv, (GLuint indx, const GLfloat* values));
  TSGL_PROTOTYPE(void        ,glVertexAttrib2f, (GLuint indx, GLfloat x, GLfloat y));
  TSGL_PROTOTYPE(void        ,glVertexAttrib2fv, (GLuint indx, const GLfloat* values));
  TSGL_PROTOTYPE(void        ,glVertexAttrib3f, (GLuint indx, GLfloat x, GLfloat y, GLfloat z));
  TSGL_PROTOTYPE(void        ,glVertexAttrib3fv, (GLuint indx, const GLfloat* values));
  TSGL_PROTOTYPE(void        ,glVertexAttrib4f, (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w));
  TSGL_PROTOTYPE(void        ,glVertexAttrib4fv, (GLuint indx, const GLfloat* values));
  TSGL_PROTOTYPE(void        ,glVertexAttribPointer, (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr));
  TSGL_PROTOTYPE(void        ,glViewport, (GLint x, GLint y, GLsizei width, GLsizei height));
#endif // __GLES2__

  //===========================================================================
  //
  //  API
  //
  //===========================================================================
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

#ifdef __GLES__
#define TSGL_LOAD_EGL   (1<<0)
#endif
#ifdef __GLES1__
#define TSGL_LOAD_GLES1 (1<<1)
#endif
#ifdef __GLES2__
#define TSGL_LOAD_GLES2 (1<<2)
#endif
#define TSGL_LOAD_ALL   (~0)

#define TSGL_CREATECTX_16BPP
#define TSGL_CREATECTX_24BPP

#ifndef TSGL_GETSYM
#ifdef niMSVC
#define TSGL_GETSYM(LIB,SYM)                                            \
  do {                                                                  \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,#SYM); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@0"); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@4"); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@8"); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@12"); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@16"); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@24"); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@32"); if (_##SYM) break; \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM "@36"); if (_##SYM) break; \
    TSGL_RETURN_ERROR(GETSYM,"Can't get symbol " #SYM " nor " "_" #SYM " from library " #LIB "."); \
  } while (0);
#else
#define TSGL_GETSYM(LIB,SYM)                                            \
  _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,#SYM);                      \
  if (!_##SYM) {                                                        \
    _##SYM = (tpfn_##SYM)TSGL_DLSYM(_dll##LIB,"_" #SYM);                \
    if (!_##SYM) {                                                      \
      TSGL_RETURN_ERROR(GETSYM,"Can't get symbol " #SYM " nor " "_" #SYM " from library " #LIB "."); \
    }                                                                   \
  }
#endif
#endif

#define TSGL_GETSYMEXT(LIB,SYM)                 \
  _##SYM = (tpfn_##SYM)dlsym(_dll##LIB,#SYM);

  /////////////////////////////////////////////////////////////////
  // Prototypes
#ifdef __TSGL_WRAPPER__
  TSGL_EXTERN int tsglLoadLibrary(int flags) { return TSGL_OK; }
  TSGL_EXTERN int tsglIsLoaded() { return 1; }
#endif

#if !defined __TSGL_IMPLEMENT__

#if !defined __TSGL_WRAPPER__
  TSGL_EXTERN int tsglLoadLibrary(int flags);
  TSGL_EXTERN int tsglIsLoaded();
#endif

  /////////////////////////////////////////////////////////////////
  // Implementation
#else // __TSGL_IMPLEMENT__

#if !defined __TSGL_WRAPPER__

#ifdef __GLES1__
  static TSGL_DLLHANDLE _dllGL1 = NULL;
  static void* tsglGL1GetProc(const char* name) {
    return (void*)TSGL_DLSYM(_dllGL1,name);
  }
#endif

#ifdef __GLES2__
  static TSGL_DLLHANDLE _dllGL2 = NULL;
  static void* tsglGL2GetProc(const char* name) {
    return (void*)TSGL_DLSYM(_dllGL2,name);
  }
#endif

  static int _tsglLoaded = 0;

  TSGL_EXTERN int tsglLoadLibrary(int flags) {
    if (_tsglLoaded)
      return TSGL_OK;

    //// OpenGL ESv1 API ////
#ifdef __GLES1__
    if (flags&TSGL_LOAD_GLES1) {
      _dllGL1 = TSGL_DLOPEN("libGLESv1_CM" TSGL_DLLEXT);
      if (!_dllGL1) {
        _dllGL1 = TSGL_DLOPEN("libGLES_CM" TSGL_DLLEXT);
        if (!_dllGL1) {
          _dllGL1 = TSGL_DLOPEN("libGLESv1" TSGL_DLLEXT);
          if (!_dllGL1) {
            _dllGL1 = TSGL_DLOPEN("libGLES" TSGL_DLLEXT);
            if (!_dllGL1) {
              _dllGL1 = TSGL_DLOPEN("libGLES_CL" TSGL_DLLEXT);
              if (!_dllGL1) {
                TSGL_RETURN_ERROR(LIBGL1,"Can't load libGLES (v1).");
              }
            }
          }
        }
      }
      TSGL_GETSYM(GL1,glGetString);
      TSGL_GETSYM(GL1,glGetError);
      TSGL_GETSYM(GL1,glGetIntegerv);
      TSGL_GETSYM(GL1,glViewport);
      TSGL_GETSYM(GL1,glScissor);
      TSGL_GETSYM(GL1,glClearColor);
      TSGL_GETSYM(GL1,glClearDepthf);
      TSGL_GETSYM(GL1,glClearStencil);
      TSGL_GETSYM(GL1,glClear);
      TSGL_GETSYM(GL1,glFlush);
      TSGL_GETSYM(GL1,glEnableClientState);
      TSGL_GETSYM(GL1,glColorPointer);
      TSGL_GETSYM(GL1,glVertexPointer);
      TSGL_GETSYM(GL1,glMatrixMode);
      TSGL_GETSYM(GL1,glLoadIdentity);
      TSGL_GETSYM(GL1,glLoadMatrixf);
      TSGL_GETSYM(GL1,glRotatef);
      TSGL_GETSYM(GL1,glDrawArrays);
      TSGL_GETSYM(GL1,glDrawElements);
      TSGL_GETSYM(GL1,glDisableClientState);
      TSGL_GETSYM(GL1,glTexParameteri);
      TSGL_GETSYM(GL1,glBindTexture);
      TSGL_GETSYM(GL1,glTexCoordPointer);
      TSGL_GETSYM(GL1,glNormalPointer);
      TSGL_GETSYM(GL1,glEnable);
      TSGL_GETSYM(GL1,glDisable);
      TSGL_GETSYM(GL1,glBlendFunc);
      TSGL_GETSYM(GL1,glDepthMask);
      TSGL_GETSYM(GL1,glDepthFunc);
      TSGL_GETSYM(GL1,glDepthFunc);
      TSGL_GETSYM(GL1,glStencilFunc);
      TSGL_GETSYM(GL1,glStencilOp);
      TSGL_GETSYM(GL1,glFrontFace);
      TSGL_GETSYM(GL1,glCullFace);
      TSGL_GETSYM(GL1,glColorMask);
      TSGL_GETSYM(GL1,glMaterialfv);
      TSGL_GETSYM(GL1,glClipPlanef);
      TSGL_GETSYM(GL1,glHint);
      TSGL_GETSYM(GL1,glColor4f);
      TSGL_GETSYM(GL1,glFogf);
      TSGL_GETSYM(GL1,glFogfv);
      TSGL_GETSYM(GL1,glMaterialf);
      TSGL_GETSYM(GL1,glMaterialfv);
      TSGL_GETSYM(GL1,glLightf);
      TSGL_GETSYM(GL1,glLightfv);
      TSGL_GETSYM(GL1,glAlphaFunc);
      TSGL_GETSYM(GL1,glDeleteTextures);
      TSGL_GETSYM(GL1,glGenTextures);
      TSGL_GETSYM(GL1,glCompressedTexSubImage2D);
      TSGL_GETSYM(GL1,glTexSubImage2D);
      TSGL_GETSYM(GL1,glCompressedTexImage2D);
      TSGL_GETSYM(GL1,glTexImage2D);
      TSGL_GETSYM(GL1,glBindTexture);
      TSGL_GETSYM(GL1,glGenBuffers);
      TSGL_GETSYM(GL1,glBindBuffer);
      TSGL_GETSYM(GL1,glBufferData);
      TSGL_GETSYM(GL1,glBufferSubData);
      TSGL_GETSYM(GL1,glDeleteBuffers);
      TSGL_GETSYM(GL1,glTexEnvi);
      TSGL_GETSYM(GL1,glTexEnvf);
      TSGL_GETSYM(GL1,glTexEnvx);
      TSGL_GETSYM(GL1,glTexEnviv);
      TSGL_GETSYM(GL1,glTexEnvxv);
      TSGL_GETSYM(GL1,glTexEnvfv);
      TSGL_GETSYM(GL1,glActiveTexture);
      TSGL_GETSYM(GL1,glClientActiveTexture);
      TSGL_GETSYM(GL1,glShadeModel);
    }
#endif

    //// OpenGL ESv2 API ////
#ifdef __GLES2__
    if (flags&TSGL_LOAD_GLES2) {
      _dllGL2 = TSGL_DLOPEN("libGLESv2" TSGL_DLLEXT);
      if (!_dllGL2) {
        TSGL_RETURN_ERROR(LIBGL2,"Can't load GLES v2 library.");
      }
      TSGL_GETSYM(GL2,glActiveTexture);
      TSGL_GETSYM(GL2,glAttachShader);
      TSGL_GETSYM(GL2,glBindAttribLocation);
      TSGL_GETSYM(GL2,glBindBuffer);
      TSGL_GETSYM(GL2,glBindFramebuffer);
      TSGL_GETSYM(GL2,glBindRenderbuffer);
      TSGL_GETSYM(GL2,glBindTexture);
      TSGL_GETSYM(GL2,glBlendColor);
      TSGL_GETSYM(GL2,glBlendEquation);
      TSGL_GETSYM(GL2,glBlendEquationSeparate);
      TSGL_GETSYM(GL2,glBlendFunc);
      TSGL_GETSYM(GL2,glBlendFuncSeparate);
      TSGL_GETSYM(GL2,glBufferData);
      TSGL_GETSYM(GL2,glBufferSubData);
      TSGL_GETSYM(GL2,glCheckFramebufferStatus);
      TSGL_GETSYM(GL2,glClear);
      TSGL_GETSYM(GL2,glClearColor);
      TSGL_GETSYM(GL2,glClearDepthf);
      TSGL_GETSYM(GL2,glClearStencil);
      TSGL_GETSYM(GL2,glColorMask);
      TSGL_GETSYM(GL2,glCompileShader);
      TSGL_GETSYM(GL2,glCompressedTexImage2D);
      TSGL_GETSYM(GL2,glCompressedTexSubImage2D);
      TSGL_GETSYM(GL2,glCopyTexImage2D);
      TSGL_GETSYM(GL2,glCopyTexSubImage2D);
      TSGL_GETSYM(GL2,glCreateProgram);
      TSGL_GETSYM(GL2,glCreateShader);
      TSGL_GETSYM(GL2,glCullFace);
      TSGL_GETSYM(GL2,glDeleteBuffers);
      TSGL_GETSYM(GL2,glDeleteFramebuffers);
      TSGL_GETSYM(GL2,glDeleteProgram);
      TSGL_GETSYM(GL2,glDeleteRenderbuffers);
      TSGL_GETSYM(GL2,glDeleteShader);
      TSGL_GETSYM(GL2,glDeleteTextures);
      TSGL_GETSYM(GL2,glDepthFunc);
      TSGL_GETSYM(GL2,glDepthMask);
      TSGL_GETSYM(GL2,glDepthRangef);
      TSGL_GETSYM(GL2,glDetachShader);
      TSGL_GETSYM(GL2,glDisable);
      TSGL_GETSYM(GL2,glDisableVertexAttribArray);
      TSGL_GETSYM(GL2,glDrawArrays);
      TSGL_GETSYM(GL2,glDrawElements);
      TSGL_GETSYM(GL2,glEnable);
      TSGL_GETSYM(GL2,glEnableVertexAttribArray);
      TSGL_GETSYM(GL2,glFinish);
      TSGL_GETSYM(GL2,glFlush);
      TSGL_GETSYM(GL2,glFramebufferRenderbuffer);
      TSGL_GETSYM(GL2,glFramebufferTexture2D);
      TSGL_GETSYM(GL2,glFrontFace);
      TSGL_GETSYM(GL2,glGenBuffers);
      TSGL_GETSYM(GL2,glGenerateMipmap);
      TSGL_GETSYM(GL2,glGenFramebuffers);
      TSGL_GETSYM(GL2,glGenRenderbuffers);
      TSGL_GETSYM(GL2,glGenTextures);
      TSGL_GETSYM(GL2,glGetActiveAttrib);
      TSGL_GETSYM(GL2,glGetActiveUniform);
      TSGL_GETSYM(GL2,glGetAttachedShaders);
      TSGL_GETSYM(GL2,glGetAttribLocation);
      TSGL_GETSYM(GL2,glGetBooleanv);
      TSGL_GETSYM(GL2,glGetBufferParameteriv);
      TSGL_GETSYM(GL2,glGetError);
      TSGL_GETSYM(GL2,glGetFloatv);
      TSGL_GETSYM(GL2,glGetFramebufferAttachmentParameteriv);
      TSGL_GETSYM(GL2,glGetIntegerv);
      TSGL_GETSYM(GL2,glGetProgramiv);
      TSGL_GETSYM(GL2,glGetProgramInfoLog);
      TSGL_GETSYM(GL2,glGetRenderbufferParameteriv);
      TSGL_GETSYM(GL2,glGetShaderiv);
      TSGL_GETSYM(GL2,glGetShaderInfoLog);
      TSGL_GETSYM(GL2,glGetShaderPrecisionFormat);
      TSGL_GETSYM(GL2,glGetShaderSource);
      TSGL_GETSYM(GL2,glGetString);
      TSGL_GETSYM(GL2,glGetTexParameterfv);
      TSGL_GETSYM(GL2,glGetTexParameteriv);
      TSGL_GETSYM(GL2,glGetUniformfv);
      TSGL_GETSYM(GL2,glGetUniformiv);
      TSGL_GETSYM(GL2,glGetUniformLocation);
      TSGL_GETSYM(GL2,glGetVertexAttribfv);
      TSGL_GETSYM(GL2,glGetVertexAttribiv);
      TSGL_GETSYM(GL2,glGetVertexAttribPointerv);
      TSGL_GETSYM(GL2,glHint);
      TSGL_GETSYM(GL2,glIsBuffer);
      TSGL_GETSYM(GL2,glIsEnabled);
      TSGL_GETSYM(GL2,glIsFramebuffer);
      TSGL_GETSYM(GL2,glIsProgram);
      TSGL_GETSYM(GL2,glIsRenderbuffer);
      TSGL_GETSYM(GL2,glIsShader);
      TSGL_GETSYM(GL2,glIsTexture);
      TSGL_GETSYM(GL2,glLineWidth);
      TSGL_GETSYM(GL2,glLinkProgram);
      TSGL_GETSYM(GL2,glPixelStorei);
      TSGL_GETSYM(GL2,glPolygonOffset);
      TSGL_GETSYM(GL2,glReadPixels);
      TSGL_GETSYM(GL2,glReleaseShaderCompiler);
      TSGL_GETSYM(GL2,glRenderbufferStorage);
      TSGL_GETSYM(GL2,glSampleCoverage);
      TSGL_GETSYM(GL2,glScissor);
      TSGL_GETSYM(GL2,glShaderBinary);
      TSGL_GETSYM(GL2,glShaderSource);
      TSGL_GETSYM(GL2,glStencilFunc);
      TSGL_GETSYM(GL2,glStencilFuncSeparate);
      TSGL_GETSYM(GL2,glStencilMask);
      TSGL_GETSYM(GL2,glStencilMaskSeparate);
      TSGL_GETSYM(GL2,glStencilOp);
      TSGL_GETSYM(GL2,glStencilOpSeparate);
      TSGL_GETSYM(GL2,glTexImage2D);
      TSGL_GETSYM(GL2,glTexParameterf);
      TSGL_GETSYM(GL2,glTexParameterfv);
      TSGL_GETSYM(GL2,glTexParameteri);
      TSGL_GETSYM(GL2,glTexParameteriv);
      TSGL_GETSYM(GL2,glTexSubImage2D);
      TSGL_GETSYM(GL2,glUniform1f);
      TSGL_GETSYM(GL2,glUniform1fv);
      TSGL_GETSYM(GL2,glUniform1i);
      TSGL_GETSYM(GL2,glUniform1iv);
      TSGL_GETSYM(GL2,glUniform2f);
      TSGL_GETSYM(GL2,glUniform2fv);
      TSGL_GETSYM(GL2,glUniform2i);
      TSGL_GETSYM(GL2,glUniform2iv);
      TSGL_GETSYM(GL2,glUniform3f);
      TSGL_GETSYM(GL2,glUniform3fv);
      TSGL_GETSYM(GL2,glUniform3i);
      TSGL_GETSYM(GL2,glUniform3iv);
      TSGL_GETSYM(GL2,glUniform4f);
      TSGL_GETSYM(GL2,glUniform4fv);
      TSGL_GETSYM(GL2,glUniform4i);
      TSGL_GETSYM(GL2,glUniform4iv);
      TSGL_GETSYM(GL2,glUniformMatrix2fv);
      TSGL_GETSYM(GL2,glUniformMatrix3fv);
      TSGL_GETSYM(GL2,glUniformMatrix4fv);
      TSGL_GETSYM(GL2,glUseProgram);
      TSGL_GETSYM(GL2,glValidateProgram);
      TSGL_GETSYM(GL2,glVertexAttrib1f);
      TSGL_GETSYM(GL2,glVertexAttrib1fv);
      TSGL_GETSYM(GL2,glVertexAttrib2f);
      TSGL_GETSYM(GL2,glVertexAttrib2fv);
      TSGL_GETSYM(GL2,glVertexAttrib3f);
      TSGL_GETSYM(GL2,glVertexAttrib3fv);
      TSGL_GETSYM(GL2,glVertexAttrib4f);
      TSGL_GETSYM(GL2,glVertexAttrib4fv);
      TSGL_GETSYM(GL2,glVertexAttribPointer);
      TSGL_GETSYM(GL2,glViewport);
    }
#endif

    _tsglLoaded = 1;
    return TSGL_OK;
  }

  TSGL_EXTERN int tsglIsLoaded() {
    return _tsglLoaded;
  }

#endif // !defined __TSGL_WRAPPER__

#ifdef __cplusplus
}
#endif

#endif // __TSGL_INTERFACES_ONLY___

#endif // __TSGL_H_94E541FC_7131_DF11_BF12_598E439FC2CC__
