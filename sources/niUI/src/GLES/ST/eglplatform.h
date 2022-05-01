/*
 * (C) COPYRIGHT 2009,2010 STMicroelectronics R&D Ltd
 * ALL RIGHTS RESERVED
 */

#ifndef __EGLPLATFORM_H__
#define __EGLPLATFORM_H__

#include "khrplatform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EGLAPIENTRY
#define EGLAPIENTRY
#endif
#ifndef EGLAPIENTRYP
#define EGLAPIENTRYP EGLAPIENTRY*
#endif
#ifndef EGLAPI
#define EGLAPI extern
#endif

  /*
   * We cannot include directfb.h here for the Mali library builds because the
   * headers have type clashes with Mali definitions of things like u8,u16
   * etc.
   */
#ifndef __DIRECTFB_H__
  typedef struct _IDirectFBSurface IDirectFBSurface;
  typedef unsigned int DFBScreenID;
#endif

  typedef IDirectFBSurface* NativeWindowType;
  typedef IDirectFBSurface* NativePixmapType;
  typedef DFBScreenID NativeDisplayType;

#ifdef __cplusplus
}
#endif

/* EGL 1.2 types, renamed for consistency in EGL 1.3 */
typedef NativeDisplayType EGLNativeDisplayType;
typedef NativePixmapType EGLNativePixmapType;
typedef NativeWindowType EGLNativeWindowType;

/* Define EGLint. This must be an integral type large enough to contain
 * all legal attribute names and values passed into and out of EGL,
 * whether their type is boolean, bitmask, enumerant (symbolic
 * constant), integer, handle, or other.
 * While in general a 32-bit integer will suffice, if handles are
 * represented as pointers, then EGLint should be defined as a 64-bit
 * integer type.
 */
typedef int EGLint;

#endif /* __EGLPLATFORM_H__ */

