
/* pngrio.c - functions for data input
 *
 * Last changed in libpng 1.2.13 November 13, 2006
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2006 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This file provides a location for all input.  Users who need
 * special handling are expected to write a function that has the same
 * arguments as this and performs a similar function, but that possibly
 * has a different input method.  Note that you shouldn't change this
 * function, but rather write a replacement function and then make
 * libpng use it at run time with ni_png_set_read_fn(...).
 */

#define PNG_INTERNAL
#include "png.h"

#if defined(PNG_READ_SUPPORTED)

/* Read the data from whatever input you are using.  The default routine
   reads from a file pointer.  Note that this routine sometimes gets called
   with very small lengths, so you should implement some kind of simple
   buffering if you are using unbuffered reads.  This should never be asked
   to read more then 64K on a 16 bit machine. */
void /* PRIVATE */
ni_png_read_data(ni_png_structp ni_png_ptr, ni_png_bytep data, ni_png_size_t length)
{
   ni_png_debug1(4,"reading %d bytes\n", (int)length);
   if (ni_png_ptr->read_data_fn != NULL)
      (*(ni_png_ptr->read_data_fn))(ni_png_ptr, data, length);
   else
      ni_png_error(ni_png_ptr, "Call to NULL read function");
}

#if !defined(PNG_NO_STDIO)
/* This is the function that does the actual reading of data.  If you are
   not reading from a standard C stream, you should create a replacement
   read_data function and use it at run time with ni_png_set_read_fn(), rather
   than changing the library. */
#ifndef USE_FAR_KEYWORD
void PNGAPI
ni_png_default_read_data(ni_png_structp ni_png_ptr, ni_png_bytep data, ni_png_size_t length)
{
   ni_png_size_t check;

   if(ni_png_ptr == NULL) return;
   /* fread() returns 0 on error, so it is OK to store this in a ni_png_size_t
    * instead of an int, which is what fread() actually returns.
    */
#if defined(_WIN32_WCE)
   if ( !ReadFile((HANDLE)(ni_png_ptr->io_ptr), data, length, &check, NULL) )
      check = 0;
#else
   check = (ni_png_size_t)fread(data, (ni_png_size_t)1, length,
      (ni_png_FILE_p)ni_png_ptr->io_ptr);
#endif

   if (check != length)
      ni_png_error(ni_png_ptr, "Read Error");
}
#else
/* this is the model-independent version. Since the standard I/O library
   can't handle far buffers in the medium and small models, we have to copy
   the data.
*/

#define NEAR_BUF_SIZE 1024
#define MIN(a,b) (a <= b ? a : b)

static void PNGAPI
ni_png_default_read_data(ni_png_structp ni_png_ptr, ni_png_bytep data, ni_png_size_t length)
{
   int check;
   ni_png_byte *n_data;
   ni_png_FILE_p io_ptr;

   if(ni_png_ptr == NULL) return;
   /* Check if data really is near. If so, use usual code. */
   n_data = (ni_png_byte *)CVT_PTR_NOCHECK(data);
   io_ptr = (ni_png_FILE_p)CVT_PTR(ni_png_ptr->io_ptr);
   if ((ni_png_bytep)n_data == data)
   {
#if defined(_WIN32_WCE)
      if ( !ReadFile((HANDLE)(ni_png_ptr->io_ptr), data, length, &check, NULL) )
         check = 0;
#else
      check = fread(n_data, 1, length, io_ptr);
#endif
   }
   else
   {
      ni_png_byte buf[NEAR_BUF_SIZE];
      ni_png_size_t read, remaining, err;
      check = 0;
      remaining = length;
      do
      {
         read = MIN(NEAR_BUF_SIZE, remaining);
#if defined(_WIN32_WCE)
         if ( !ReadFile((HANDLE)(io_ptr), buf, read, &err, NULL) )
            err = 0;
#else
         err = fread(buf, (ni_png_size_t)1, read, io_ptr);
#endif
         ni_png_memcpy(data, buf, read); /* copy far buffer to near buffer */
         if(err != read)
            break;
         else
            check += err;
         data += read;
         remaining -= read;
      }
      while (remaining != 0);
   }
   if ((ni_png_uint_32)check != (ni_png_uint_32)length)
      ni_png_error(ni_png_ptr, "read Error");
}
#endif
#endif

/* This function allows the application to supply a new input function
   for libpng if standard C streams aren't being used.

   This function takes as its arguments:
   ni_png_ptr      - pointer to a png input data structure
   io_ptr       - pointer to user supplied structure containing info about
                  the input functions.  May be NULL.
   read_data_fn - pointer to a new input function that takes as its
                  arguments a pointer to a ni_png_struct, a pointer to
                  a location where input data can be stored, and a 32-bit
                  unsigned int that is the number of bytes to be read.
                  To exit and output any fatal error messages the new write
                  function should call ni_png_error(ni_png_ptr, "Error msg"). */
void PNGAPI
ni_png_set_read_fn(ni_png_structp ni_png_ptr, ni_png_voidp io_ptr,
   ni_png_rw_ptr read_data_fn)
{
   if(ni_png_ptr == NULL) return;
   ni_png_ptr->io_ptr = io_ptr;

#if !defined(PNG_NO_STDIO)
   if (read_data_fn != NULL)
      ni_png_ptr->read_data_fn = read_data_fn;
   else
      ni_png_ptr->read_data_fn = ni_png_default_read_data;
#else
   ni_png_ptr->read_data_fn = read_data_fn;
#endif

   /* It is an error to write to a read device */
   if (ni_png_ptr->write_data_fn != NULL)
   {
      ni_png_ptr->write_data_fn = NULL;
      ni_png_warning(ni_png_ptr,
         "It's an error to set both read_data_fn and write_data_fn in the ");
      ni_png_warning(ni_png_ptr,
         "same structure.  Resetting write_data_fn to NULL.");
   }

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   ni_png_ptr->output_flush_fn = NULL;
#endif
}
#endif /* PNG_READ_SUPPORTED */
