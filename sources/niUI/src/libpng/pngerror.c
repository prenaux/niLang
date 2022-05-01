
/* pngerror.c - stub functions for i/o and memory allocation
 *
 * Last changed in libpng 1.2.13 November 13, 2006
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2006 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This file provides a location for all error handling.  Users who
 * need special error handling are expected to write replacement functions
 * and use ni_png_set_error_fn() to use those functions.  See the instructions
 * at each function.
 */

#define PNG_INTERNAL
#include "png.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
static void /* PRIVATE */
ni_png_default_error PNGARG((ni_png_structp ni_png_ptr,
  ni_png_const_charp error_message));
static void /* PRIVATE */
ni_png_default_warning PNGARG((ni_png_structp ni_png_ptr,
  ni_png_const_charp warning_message));

/* This function is called whenever there is a fatal error.  This function
 * should not be changed.  If there is a need to handle errors differently,
 * you should supply a replacement error function and use ni_png_set_error_fn()
 * to replace the error function at run-time.
 */
void PNGAPI
ni_png_error(ni_png_structp ni_png_ptr, ni_png_const_charp error_message)
{
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
   char msg[16];
   if (ni_png_ptr != NULL)
   {
     if (ni_png_ptr->flags&
       (PNG_FLAG_STRIP_ERROR_NUMBERS|PNG_FLAG_STRIP_ERROR_TEXT))
     {
       if (*error_message == '#')
       {
           int offset;
           for (offset=1; offset<15; offset++)
              if (*(error_message+offset) == ' ')
                  break;
           if (ni_png_ptr->flags&PNG_FLAG_STRIP_ERROR_TEXT)
           {
              int i;
              for (i=0; i<offset-1; i++)
                 msg[i]=error_message[i+1];
              msg[i]='\0';
              error_message=msg;
           }
           else
              error_message+=offset;
       }
       else
       {
           if (ni_png_ptr->flags&PNG_FLAG_STRIP_ERROR_TEXT)
           {
              msg[0]='0';
              msg[1]='\0';
              error_message=msg;
           }
       }
     }
   }
#endif
   if (ni_png_ptr != NULL && ni_png_ptr->error_fn != NULL)
      (*(ni_png_ptr->error_fn))(ni_png_ptr, error_message);

   /* If the custom handler doesn't exist, or if it returns,
      use the default handler, which will not return. */
   ni_png_default_error(ni_png_ptr, error_message);
}

/* This function is called whenever there is a non-fatal error.  This function
 * should not be changed.  If there is a need to handle warnings differently,
 * you should supply a replacement warning function and use
 * ni_png_set_error_fn() to replace the warning function at run-time.
 */
void PNGAPI
ni_png_warning(ni_png_structp ni_png_ptr, ni_png_const_charp warning_message)
{
   int offset = 0;
   if (ni_png_ptr != NULL)
   {
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
   if (ni_png_ptr->flags&
     (PNG_FLAG_STRIP_ERROR_NUMBERS|PNG_FLAG_STRIP_ERROR_TEXT))
#endif
     {
       if (*warning_message == '#')
       {
           for (offset=1; offset<15; offset++)
              if (*(warning_message+offset) == ' ')
                  break;
       }
     }
     if (ni_png_ptr != NULL && ni_png_ptr->warning_fn != NULL)
        (*(ni_png_ptr->warning_fn))(ni_png_ptr, warning_message+offset);
   }
   else
      ni_png_default_warning(ni_png_ptr, warning_message+offset);
}

/* These utilities are used internally to build an error message that relates
 * to the current chunk.  The chunk name comes from ni_png_ptr->chunk_name,
 * this is used to prefix the message.  The message is limited in length
 * to 63 bytes, the name characters are output as hex digits wrapped in []
 * if the character is invalid.
 */
#define isnonalpha(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))
static PNG_CONST char ni_png_digit[16] = {
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
   'A', 'B', 'C', 'D', 'E', 'F'
};

static void /* PRIVATE */
ni_png_format_buffer(ni_png_structp ni_png_ptr, ni_png_charp buffer, ni_png_const_charp
   error_message)
{
   int iout = 0, iin = 0;

   while (iin < 4)
   {
      int c = ni_png_ptr->chunk_name[iin++];
      if (isnonalpha(c))
      {
         buffer[iout++] = '[';
         buffer[iout++] = ni_png_digit[(c & 0xf0) >> 4];
         buffer[iout++] = ni_png_digit[c & 0x0f];
         buffer[iout++] = ']';
      }
      else
      {
         buffer[iout++] = (ni_png_byte)c;
      }
   }

   if (error_message == NULL)
      buffer[iout] = 0;
   else
   {
      buffer[iout++] = ':';
      buffer[iout++] = ' ';
      ni_png_strncpy(buffer+iout, error_message, 63);
      buffer[iout+63] = 0;
   }
}

void PNGAPI
ni_png_chunk_error(ni_png_structp ni_png_ptr, ni_png_const_charp error_message)
{
   char msg[18+64];
   if (ni_png_ptr == NULL)
     ni_png_error(ni_png_ptr, error_message);
   else
   {
     ni_png_format_buffer(ni_png_ptr, msg, error_message);
     ni_png_error(ni_png_ptr, msg);
   }
}

void PNGAPI
ni_png_chunk_warning(ni_png_structp ni_png_ptr, ni_png_const_charp warning_message)
{
   char msg[18+64];
   if (ni_png_ptr == NULL)
     ni_png_warning(ni_png_ptr, warning_message);
   else
   {
     ni_png_format_buffer(ni_png_ptr, msg, warning_message);
     ni_png_warning(ni_png_ptr, msg);
   }
}

/* This is the default error handling function.  Note that replacements for
 * this function MUST NOT RETURN, or the program will likely crash.  This
 * function is used by default, or if the program supplies NULL for the
 * error function pointer in ni_png_set_error_fn().
 */
static void /* PRIVATE */
ni_png_default_error(ni_png_structp ni_png_ptr, ni_png_const_charp error_message)
{
#ifndef PNG_NO_CONSOLE_IO
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
   if (*error_message == '#')
   {
     int offset;
     char error_number[16];
     for (offset=0; offset<15; offset++)
     {
         error_number[offset] = *(error_message+offset+1);
         if (*(error_message+offset) == ' ')
             break;
     }
     if((offset > 1) && (offset < 15))
     {
       error_number[offset-1]='\0';
       fprintf(stderr, "libpng error no. %s: %s\n", error_number,
          error_message+offset);
     }
     else
       fprintf(stderr, "libpng error: %s, offset=%d\n", error_message,offset);
   }
   else
#endif
   fprintf(stderr, "libpng error: %s\n", error_message);
#endif

#ifdef PNG_SETJMP_SUPPORTED
   if (ni_png_ptr)
   {
#  ifdef USE_FAR_KEYWORD
   {
      jmp_buf jmpbuf;
      ni_png_memcpy(jmpbuf,ni_png_ptr->jmpbuf,ni_png_sizeof(jmp_buf));
      longjmp(jmpbuf, 1);
   }
#  else
   longjmp(ni_png_ptr->jmpbuf, 1);
#  endif
   }
#else
   PNG_ABORT();
#endif
#ifdef PNG_NO_CONSOLE_IO
   /* make compiler happy */ ;
   if (&error_message != NULL)
      return;
#endif
}

/* This function is called when there is a warning, but the library thinks
 * it can continue anyway.  Replacement functions don't have to do anything
 * here if you don't want them to.  In the default configuration, ni_png_ptr is
 * not used, but it is passed in case it may be useful.
 */
static void /* PRIVATE */
ni_png_default_warning(ni_png_structp ni_png_ptr, ni_png_const_charp warning_message)
{
#ifndef PNG_NO_CONSOLE_IO
#  ifdef PNG_ERROR_NUMBERS_SUPPORTED
   if (*warning_message == '#')
   {
     int offset;
     char warning_number[16];
     for (offset=0; offset<15; offset++)
     {
        warning_number[offset]=*(warning_message+offset+1);
        if (*(warning_message+offset) == ' ')
            break;
     }
     if((offset > 1) && (offset < 15))
     {
       warning_number[offset-1]='\0';
       fprintf(stderr, "libpng warning no. %s: %s\n", warning_number,
          warning_message+offset);
     }
     else
       fprintf(stderr, "libpng warning: %s\n", warning_message);
   }
   else
#  endif
     fprintf(stderr, "libpng warning: %s\n", warning_message);
#else
   /* make compiler happy */ ;
   if (warning_message)
     return;
#endif
   /* make compiler happy */ ;
   if (ni_png_ptr)
      return;
}

/* This function is called when the application wants to use another method
 * of handling errors and warnings.  Note that the error function MUST NOT
 * return to the calling routine or serious problems will occur.  The return
 * method used in the default routine calls longjmp(ni_png_ptr->jmpbuf, 1)
 */
void PNGAPI
ni_png_set_error_fn(ni_png_structp ni_png_ptr, ni_png_voidp error_ptr,
   ni_png_error_ptr error_fn, ni_png_error_ptr warning_fn)
{
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->error_ptr = error_ptr;
   ni_png_ptr->error_fn = error_fn;
   ni_png_ptr->warning_fn = warning_fn;
}


/* This function returns a pointer to the error_ptr associated with the user
 * functions.  The application should free any memory associated with this
 * pointer before ni_png_write_destroy and ni_png_read_destroy are called.
 */
ni_png_voidp PNGAPI
ni_png_get_error_ptr(ni_png_structp ni_png_ptr)
{
   if (ni_png_ptr == NULL)
      return NULL;
   return ((ni_png_voidp)ni_png_ptr->error_ptr);
}


#ifdef PNG_ERROR_NUMBERS_SUPPORTED
void PNGAPI
ni_png_set_strip_error_numbers(ni_png_structp ni_png_ptr, ni_png_uint_32 strip_mode)
{
   if(ni_png_ptr != NULL)
   {
     ni_png_ptr->flags &=
       ((~(PNG_FLAG_STRIP_ERROR_NUMBERS|PNG_FLAG_STRIP_ERROR_TEXT))&strip_mode);
   }
}
#endif
#endif /* PNG_READ_SUPPORTED || PNG_WRITE_SUPPORTED */
