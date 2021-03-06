/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2008-2010 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * @file fbdev_window.h
 * @brief A window type for the framebuffer device (used by egl and tests)
 */

#ifndef _FBDEV_WINDOW_H_
#define _FBDEV_WINDOW_H_

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum
  {
    FBDEV_PIXMAP_DEFAULT = 0,
    FBDEV_PIXMAP_SUPPORTS_UMP = (1<<0),
    FBDEV_PIXMAP_NATIVE_COMPONENT_ORDER = (1<<1),
    FBDEV_PIXMAP_BIGENDIAN = (1<<2)
  } fbdev_pixmap_flags;

  typedef struct fbdev_window
  {
    unsigned short width;
    unsigned short height;
  } fbdev_window;

  typedef struct fbdev_pixmap
  {
    unsigned int height;
    unsigned int width;
    unsigned int bytes_per_pixel;
    unsigned char buffer_size;
    unsigned char red_size;
    unsigned char green_size;
    unsigned char blue_size;
    unsigned char alpha_size;
    unsigned char luminance_size;
    fbdev_pixmap_flags flags;
    unsigned short *data;
  } fbdev_pixmap;

#ifdef __cplusplus
}
#endif


#endif
