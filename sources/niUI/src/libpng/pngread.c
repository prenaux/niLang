
/* pngread.c - read a PNG file
 *
 * Last changed in libpng 1.2.15 January 5, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This file contains routines that an application calls directly to
 * read a PNG file or stream.
 */

#define PNG_INTERNAL
#include "png.h"

#if defined(PNG_READ_SUPPORTED)

/* Create a PNG structure for reading, and allocate any memory needed. */
ni_png_structp PNGAPI
ni_png_create_read_struct(ni_png_const_charp user_png_ver, ni_png_voidp error_ptr,
   ni_png_error_ptr error_fn, ni_png_error_ptr warn_fn)
{

#ifdef PNG_USER_MEM_SUPPORTED
   return (ni_png_create_read_struct_2(user_png_ver, error_ptr, error_fn,
      warn_fn, ni_png_voidp_NULL, ni_png_malloc_ptr_NULL, ni_png_free_ptr_NULL));
}

/* Alternate create PNG structure for reading, and allocate any memory needed. */
ni_png_structp PNGAPI
ni_png_create_read_struct_2(ni_png_const_charp user_png_ver, ni_png_voidp error_ptr,
   ni_png_error_ptr error_fn, ni_png_error_ptr warn_fn, ni_png_voidp mem_ptr,
   ni_png_malloc_ptr malloc_fn, ni_png_free_ptr free_fn)
{
#endif /* PNG_USER_MEM_SUPPORTED */

   ni_png_structp ni_png_ptr;

#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
   jmp_buf jmpbuf;
#endif
#endif

   int i;

   ni_png_debug(1, "in ni_png_create_read_struct\n");
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_ptr = (ni_png_structp)ni_png_create_struct_2(PNG_STRUCT_PNG,
      (ni_png_malloc_ptr)malloc_fn, (ni_png_voidp)mem_ptr);
#else
   ni_png_ptr = (ni_png_structp)ni_png_create_struct(PNG_STRUCT_PNG);
#endif
   if (ni_png_ptr == NULL)
      return (NULL);

#if !defined(PNG_1_0_X)
#ifdef PNG_MMX_CODE_SUPPORTED
   ni_png_init_mmx_flags(ni_png_ptr);   /* 1.2.0 addition */
#endif
#endif /* PNG_1_0_X */

   /* added at libpng-1.2.6 */
#ifdef PNG_SET_USER_LIMITS_SUPPORTED
   ni_png_ptr->user_width_max=PNG_USER_WIDTH_MAX;
   ni_png_ptr->user_height_max=PNG_USER_HEIGHT_MAX;
#endif

#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
#else
   if (setjmp(ni_png_ptr->jmpbuf))
#endif
   {
      ni_png_free(ni_png_ptr, ni_png_ptr->zbuf);
      ni_png_ptr->zbuf=NULL;
#ifdef PNG_USER_MEM_SUPPORTED
      ni_png_destroy_struct_2((ni_png_voidp)ni_png_ptr,
         (ni_png_free_ptr)free_fn, (ni_png_voidp)mem_ptr);
#else
      ni_png_destroy_struct((ni_png_voidp)ni_png_ptr);
#endif
      return (NULL);
   }
#ifdef USE_FAR_KEYWORD
   ni_png_memcpy(ni_png_ptr->jmpbuf,jmpbuf,ni_png_sizeof(jmp_buf));
#endif
#endif

#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_set_mem_fn(ni_png_ptr, mem_ptr, malloc_fn, free_fn);
#endif

   ni_png_set_error_fn(ni_png_ptr, error_ptr, error_fn, warn_fn);

   i=0;
   do
   {
     if(user_png_ver[i] != ni_png_libpng_ver[i])
        ni_png_ptr->flags |= PNG_FLAG_LIBRARY_MISMATCH;
   } while (ni_png_libpng_ver[i++]);

   if (ni_png_ptr->flags & PNG_FLAG_LIBRARY_MISMATCH)
   {
     /* Libpng 0.90 and later are binary incompatible with libpng 0.89, so
      * we must recompile any applications that use any older library version.
      * For versions after libpng 1.0, we will be compatible, so we need
      * only check the first digit.
      */
     if (user_png_ver == NULL || user_png_ver[0] != ni_png_libpng_ver[0] ||
         (user_png_ver[0] == '1' && user_png_ver[2] != ni_png_libpng_ver[2]) ||
         (user_png_ver[0] == '0' && user_png_ver[2] < '9'))
     {
#if !defined(PNG_NO_STDIO) && !defined(_WIN32_WCE)
        char msg[80];
        if (user_png_ver)
        {
          sprintf(msg, "Application was compiled with png.h from libpng-%.20s",
             user_png_ver);
          ni_png_warning(ni_png_ptr, msg);
        }
        sprintf(msg, "Application  is  running with png.c from libpng-%.20s",
           ni_png_libpng_ver);
        ni_png_warning(ni_png_ptr, msg);
#endif
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
        ni_png_ptr->flags=0;
#endif
        ni_png_error(ni_png_ptr,
           "Incompatible libpng version in application and library");
     }
   }

   /* initialize zbuf - compression buffer */
   ni_png_ptr->zbuf_size = PNG_ZBUF_SIZE;
   ni_png_ptr->zbuf = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
     (ni_png_uint_32)ni_png_ptr->zbuf_size);
   ni_png_ptr->zstream.zalloc = (zlib_alloc_func)ni_png_zalloc;
   ni_png_ptr->zstream.zfree = ni_png_zfree;
   ni_png_ptr->zstream.opaque = (voidpf)ni_png_ptr;

   switch (inflateInit(&ni_png_ptr->zstream))
   {
     case Z_OK: /* Do nothing */ break;
     case Z_MEM_ERROR:
     case Z_STREAM_ERROR: ni_png_error(ni_png_ptr, "zlib memory error"); break;
     case Z_VERSION_ERROR: ni_png_error(ni_png_ptr, "zlib version error"); break;
     default: ni_png_error(ni_png_ptr, "Unknown zlib error");
   }

   ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
   ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;

   ni_png_set_read_fn(ni_png_ptr, ni_png_voidp_NULL, ni_png_rw_ptr_NULL);

#ifdef PNG_SETJMP_SUPPORTED
/* Applications that neglect to set up their own setjmp() and then encounter
   a ni_png_error() will longjmp here.  Since the jmpbuf is then meaningless we
   abort instead of returning. */
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
      PNG_ABORT();
   ni_png_memcpy(ni_png_ptr->jmpbuf,jmpbuf,ni_png_sizeof(jmp_buf));
#else
   if (setjmp(ni_png_ptr->jmpbuf))
      PNG_ABORT();
#endif
#endif
   return (ni_png_ptr);
}

#if defined(PNG_1_0_X) || defined(PNG_1_2_X)
/* Initialize PNG structure for reading, and allocate any memory needed.
   This interface is deprecated in favour of the ni_png_create_read_struct(),
   and it will disappear as of libpng-1.3.0. */
#undef ni_png_read_init
void PNGAPI
ni_png_read_init(ni_png_structp ni_png_ptr)
{
   /* We only come here via pre-1.0.7-compiled applications */
   ni_png_read_init_2(ni_png_ptr, "1.0.6 or earlier", 0, 0);
}

void PNGAPI
ni_png_read_init_2(ni_png_structp ni_png_ptr, ni_png_const_charp user_png_ver,
   ni_png_size_t ni_png_struct_size, ni_png_size_t ni_png_info_size)
{
   /* We only come here via pre-1.0.12-compiled applications */
   if(ni_png_ptr == NULL) return;
#if !defined(PNG_NO_STDIO) && !defined(_WIN32_WCE)
   if(ni_png_sizeof(ni_png_struct) > ni_png_struct_size ||
      ni_png_sizeof(ni_png_info) > ni_png_info_size)
   {
      char msg[80];
      ni_png_ptr->warning_fn=NULL;
      if (user_png_ver)
      {
        sprintf(msg, "Application was compiled with png.h from libpng-%.20s",
           user_png_ver);
        ni_png_warning(ni_png_ptr, msg);
      }
      sprintf(msg, "Application  is  running with png.c from libpng-%.20s",
         ni_png_libpng_ver);
      ni_png_warning(ni_png_ptr, msg);
   }
#endif
   if(ni_png_sizeof(ni_png_struct) > ni_png_struct_size)
     {
       ni_png_ptr->error_fn=NULL;
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
       ni_png_ptr->flags=0;
#endif
       ni_png_error(ni_png_ptr,
       "The png struct allocated by the application for reading is too small.");
     }
   if(ni_png_sizeof(ni_png_info) > ni_png_info_size)
     {
       ni_png_ptr->error_fn=NULL;
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
       ni_png_ptr->flags=0;
#endif
       ni_png_error(ni_png_ptr,
         "The info struct allocated by application for reading is too small.");
     }
   ni_png_read_init_3(&ni_png_ptr, user_png_ver, ni_png_struct_size);
}
#endif /* PNG_1_0_X || PNG_1_2_X */

void PNGAPI
ni_png_read_init_3(ni_png_structpp ptr_ptr, ni_png_const_charp user_png_ver,
   ni_png_size_t ni_png_struct_size)
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf tmp_jmp;  /* to save current jump buffer */
#endif

   int i=0;

   ni_png_structp ni_png_ptr=*ptr_ptr;

   if(ni_png_ptr == NULL) return;

   do
   {
     if(user_png_ver[i] != ni_png_libpng_ver[i])
     {
#ifdef PNG_LEGACY_SUPPORTED
       ni_png_ptr->flags |= PNG_FLAG_LIBRARY_MISMATCH;
#else
       ni_png_ptr->warning_fn=NULL;
       ni_png_warning(ni_png_ptr,
        "Application uses deprecated ni_png_read_init() and should be recompiled.");
       break;
#endif
     }
   } while (ni_png_libpng_ver[i++]);

   ni_png_debug(1, "in ni_png_read_init_3\n");

#ifdef PNG_SETJMP_SUPPORTED
   /* save jump buffer and error functions */
   ni_png_memcpy(tmp_jmp, ni_png_ptr->jmpbuf, ni_png_sizeof (jmp_buf));
#endif

   if(ni_png_sizeof(ni_png_struct) > ni_png_struct_size)
     {
       ni_png_destroy_struct(ni_png_ptr);
       *ptr_ptr = (ni_png_structp)ni_png_create_struct(PNG_STRUCT_PNG);
       ni_png_ptr = *ptr_ptr;
     }

   /* reset all variables to 0 */
   ni_png_memset(ni_png_ptr, 0, ni_png_sizeof (ni_png_struct));

#ifdef PNG_SETJMP_SUPPORTED
   /* restore jump buffer */
   ni_png_memcpy(ni_png_ptr->jmpbuf, tmp_jmp, ni_png_sizeof (jmp_buf));
#endif

   /* added at libpng-1.2.6 */
#ifdef PNG_SET_USER_LIMITS_SUPPORTED
   ni_png_ptr->user_width_max=PNG_USER_WIDTH_MAX;
   ni_png_ptr->user_height_max=PNG_USER_HEIGHT_MAX;
#endif

   /* initialize zbuf - compression buffer */
   ni_png_ptr->zbuf_size = PNG_ZBUF_SIZE;
   ni_png_ptr->zbuf = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
     (ni_png_uint_32)ni_png_ptr->zbuf_size);
   ni_png_ptr->zstream.zalloc = (zlib_alloc_func)ni_png_zalloc;
   ni_png_ptr->zstream.zfree = ni_png_zfree;
   ni_png_ptr->zstream.opaque = (voidpf)ni_png_ptr;

   switch (inflateInit(&ni_png_ptr->zstream))
   {
     case Z_OK: /* Do nothing */ break;
     case Z_MEM_ERROR:
     case Z_STREAM_ERROR: ni_png_error(ni_png_ptr, "zlib memory"); break;
     case Z_VERSION_ERROR: ni_png_error(ni_png_ptr, "zlib version"); break;
     default: ni_png_error(ni_png_ptr, "Unknown zlib error");
   }

   ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
   ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;

   ni_png_set_read_fn(ni_png_ptr, ni_png_voidp_NULL, ni_png_rw_ptr_NULL);
}

#ifndef PNG_NO_SEQUENTIAL_READ_SUPPORTED
/* Read the information before the actual image data.  This has been
 * changed in v0.90 to allow reading a file that already has the magic
 * bytes read from the stream.  You can tell libpng how many bytes have
 * been read from the beginning of the stream (up to the maximum of 8)
 * via ni_png_set_sig_bytes(), and we will only check the remaining bytes
 * here.  The application can then have access to the signature bytes we
 * read if it is determined that this isn't a valid PNG file.
 */
void PNGAPI
ni_png_read_info(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   if(ni_png_ptr == NULL) return;
   ni_png_debug(1, "in ni_png_read_info\n");
   /* If we haven't checked all of the PNG signature bytes, do so now. */
   if (ni_png_ptr->sig_bytes < 8)
   {
      ni_png_size_t num_checked = ni_png_ptr->sig_bytes,
                 num_to_check = 8 - num_checked;

      ni_png_read_data(ni_png_ptr, &(info_ptr->signature[num_checked]), num_to_check);
      ni_png_ptr->sig_bytes = 8;

      if (ni_png_sig_cmp(info_ptr->signature, num_checked, num_to_check))
      {
         if (num_checked < 4 &&
             ni_png_sig_cmp(info_ptr->signature, num_checked, num_to_check - 4))
            ni_png_error(ni_png_ptr, "Not a PNG file");
         else
            ni_png_error(ni_png_ptr, "PNG file corrupted by ASCII conversion");
      }
      if (num_checked < 3)
         ni_png_ptr->mode |= PNG_HAVE_PNG_SIGNATURE;
   }

   for(;;)
   {
#ifdef PNG_USE_LOCAL_ARRAYS
      PNG_IHDR;
      PNG_IDAT;
      PNG_IEND;
      PNG_PLTE;
#if defined(PNG_READ_bKGD_SUPPORTED)
      PNG_bKGD;
#endif
#if defined(PNG_READ_cHRM_SUPPORTED)
      PNG_cHRM;
#endif
#if defined(PNG_READ_gAMA_SUPPORTED)
      PNG_gAMA;
#endif
#if defined(PNG_READ_hIST_SUPPORTED)
      PNG_hIST;
#endif
#if defined(PNG_READ_iCCP_SUPPORTED)
      PNG_iCCP;
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
      PNG_iTXt;
#endif
#if defined(PNG_READ_oFFs_SUPPORTED)
      PNG_oFFs;
#endif
#if defined(PNG_READ_pCAL_SUPPORTED)
      PNG_pCAL;
#endif
#if defined(PNG_READ_pHYs_SUPPORTED)
      PNG_pHYs;
#endif
#if defined(PNG_READ_sBIT_SUPPORTED)
      PNG_sBIT;
#endif
#if defined(PNG_READ_sCAL_SUPPORTED)
      PNG_sCAL;
#endif
#if defined(PNG_READ_sPLT_SUPPORTED)
      PNG_sPLT;
#endif
#if defined(PNG_READ_sRGB_SUPPORTED)
      PNG_sRGB;
#endif
#if defined(PNG_READ_tEXt_SUPPORTED)
      PNG_tEXt;
#endif
#if defined(PNG_READ_tIME_SUPPORTED)
      PNG_tIME;
#endif
#if defined(PNG_READ_tRNS_SUPPORTED)
      PNG_tRNS;
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
      PNG_zTXt;
#endif
#endif /* PNG_USE_LOCAL_ARRAYS */
      ni_png_byte chunk_length[4];
      ni_png_uint_32 length;

      ni_png_read_data(ni_png_ptr, chunk_length, 4);
      length = ni_png_get_uint_31(ni_png_ptr,chunk_length);

      ni_png_reset_crc(ni_png_ptr);
      ni_png_crc_read(ni_png_ptr, ni_png_ptr->chunk_name, 4);

      ni_png_debug2(0, "Reading %s chunk, length=%lu.\n", ni_png_ptr->chunk_name,
         length);

      /* This should be a binary subdivision search or a hash for
       * matching the chunk name rather than a linear search.
       */
      if (!ni_png_memcmp(ni_png_ptr->chunk_name, (ni_png_bytep)ni_png_IDAT, 4))
        if(ni_png_ptr->mode & PNG_AFTER_IDAT)
          ni_png_ptr->mode |= PNG_HAVE_CHUNK_AFTER_IDAT;

      if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IHDR, 4))
         ni_png_handle_IHDR(ni_png_ptr, info_ptr, length);
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IEND, 4))
         ni_png_handle_IEND(ni_png_ptr, info_ptr, length);
#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
      else if (ni_png_handle_as_unknown(ni_png_ptr, ni_png_ptr->chunk_name))
      {
         if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
            ni_png_ptr->mode |= PNG_HAVE_IDAT;
         ni_png_handle_unknown(ni_png_ptr, info_ptr, length);
         if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_PLTE, 4))
            ni_png_ptr->mode |= PNG_HAVE_PLTE;
         else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
         {
            if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
               ni_png_error(ni_png_ptr, "Missing IHDR before IDAT");
            else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
                     !(ni_png_ptr->mode & PNG_HAVE_PLTE))
               ni_png_error(ni_png_ptr, "Missing PLTE before IDAT");
            break;
         }
      }
#endif
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_PLTE, 4))
         ni_png_handle_PLTE(ni_png_ptr, info_ptr, length);
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
      {
         if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
            ni_png_error(ni_png_ptr, "Missing IHDR before IDAT");
         else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
                  !(ni_png_ptr->mode & PNG_HAVE_PLTE))
            ni_png_error(ni_png_ptr, "Missing PLTE before IDAT");

         ni_png_ptr->idat_size = length;
         ni_png_ptr->mode |= PNG_HAVE_IDAT;
         break;
      }
#if defined(PNG_READ_bKGD_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_bKGD, 4))
         ni_png_handle_bKGD(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_cHRM_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_cHRM, 4))
         ni_png_handle_cHRM(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_gAMA_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_gAMA, 4))
         ni_png_handle_gAMA(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_hIST_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_hIST, 4))
         ni_png_handle_hIST(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_oFFs_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_oFFs, 4))
         ni_png_handle_oFFs(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_pCAL_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_pCAL, 4))
         ni_png_handle_pCAL(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sCAL_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sCAL, 4))
         ni_png_handle_sCAL(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_pHYs_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_pHYs, 4))
         ni_png_handle_pHYs(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sBIT_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sBIT, 4))
         ni_png_handle_sBIT(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sRGB_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sRGB, 4))
         ni_png_handle_sRGB(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_iCCP_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_iCCP, 4))
         ni_png_handle_iCCP(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sPLT_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sPLT, 4))
         ni_png_handle_sPLT(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tEXt_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tEXt, 4))
         ni_png_handle_tEXt(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tIME_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tIME, 4))
         ni_png_handle_tIME(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tRNS_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tRNS, 4))
         ni_png_handle_tRNS(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_zTXt, 4))
         ni_png_handle_zTXt(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_iTXt, 4))
         ni_png_handle_iTXt(ni_png_ptr, info_ptr, length);
#endif
      else
         ni_png_handle_unknown(ni_png_ptr, info_ptr, length);
   }
}
#endif /* PNG_NO_SEQUENTIAL_READ_SUPPORTED */

/* optional call to update the users info_ptr structure */
void PNGAPI
ni_png_read_update_info(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   ni_png_debug(1, "in ni_png_read_update_info\n");
   if(ni_png_ptr == NULL) return;
   if (!(ni_png_ptr->flags & PNG_FLAG_ROW_INIT))
      ni_png_read_start_row(ni_png_ptr);
   else
      ni_png_warning(ni_png_ptr,
      "Ignoring extra ni_png_read_update_info() call; row buffer not reallocated");
   ni_png_read_transform_info(ni_png_ptr, info_ptr);
}

#ifndef PNG_NO_SEQUENTIAL_READ_SUPPORTED
/* Initialize palette, background, etc, after transformations
 * are set, but before any reading takes place.  This allows
 * the user to obtain a gamma-corrected palette, for example.
 * If the user doesn't call this, we will do it ourselves.
 */
void PNGAPI
ni_png_start_read_image(ni_png_structp ni_png_ptr)
{
   ni_png_debug(1, "in ni_png_start_read_image\n");
   if(ni_png_ptr == NULL) return;
   if (!(ni_png_ptr->flags & PNG_FLAG_ROW_INIT))
      ni_png_read_start_row(ni_png_ptr);
}
#endif /* PNG_NO_SEQUENTIAL_READ_SUPPORTED */

#ifndef PNG_NO_SEQUENTIAL_READ_SUPPORTED
void PNGAPI
ni_png_read_row(ni_png_structp ni_png_ptr, ni_png_bytep row, ni_png_bytep dsp_row)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_IDAT;
   const int ni_png_pass_dsp_mask[7] = {0xff, 0x0f, 0xff, 0x33, 0xff, 0x55, 0xff};
   const int ni_png_pass_mask[7] = {0x80, 0x08, 0x88, 0x22, 0xaa, 0x55, 0xff};
#endif
   int ret;
   if(ni_png_ptr == NULL) return;
   ni_png_debug2(1, "in ni_png_read_row (row %lu, pass %d)\n",
      ni_png_ptr->row_number, ni_png_ptr->pass);
   if (!(ni_png_ptr->flags & PNG_FLAG_ROW_INIT))
      ni_png_read_start_row(ni_png_ptr);
   if (ni_png_ptr->row_number == 0 && ni_png_ptr->pass == 0)
   {
   /* check for transforms that have been set but were defined out */
#if defined(PNG_WRITE_INVERT_SUPPORTED) && !defined(PNG_READ_INVERT_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_INVERT_MONO)
      ni_png_warning(ni_png_ptr, "PNG_READ_INVERT_SUPPORTED is not defined.");
#endif
#if defined(PNG_WRITE_FILLER_SUPPORTED) && !defined(PNG_READ_FILLER_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_FILLER)
      ni_png_warning(ni_png_ptr, "PNG_READ_FILLER_SUPPORTED is not defined.");
#endif
#if defined(PNG_WRITE_PACKSWAP_SUPPORTED) && !defined(PNG_READ_PACKSWAP_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_PACKSWAP)
      ni_png_warning(ni_png_ptr, "PNG_READ_PACKSWAP_SUPPORTED is not defined.");
#endif
#if defined(PNG_WRITE_PACK_SUPPORTED) && !defined(PNG_READ_PACK_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_PACK)
      ni_png_warning(ni_png_ptr, "PNG_READ_PACK_SUPPORTED is not defined.");
#endif
#if defined(PNG_WRITE_SHIFT_SUPPORTED) && !defined(PNG_READ_SHIFT_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_SHIFT)
      ni_png_warning(ni_png_ptr, "PNG_READ_SHIFT_SUPPORTED is not defined.");
#endif
#if defined(PNG_WRITE_BGR_SUPPORTED) && !defined(PNG_READ_BGR_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_BGR)
      ni_png_warning(ni_png_ptr, "PNG_READ_BGR_SUPPORTED is not defined.");
#endif
#if defined(PNG_WRITE_SWAP_SUPPORTED) && !defined(PNG_READ_SWAP_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_SWAP_BYTES)
      ni_png_warning(ni_png_ptr, "PNG_READ_SWAP_SUPPORTED is not defined.");
#endif
   }

#if defined(PNG_READ_INTERLACING_SUPPORTED)
   /* if interlaced and we do not need a new row, combine row and return */
   if (ni_png_ptr->interlaced && (ni_png_ptr->transformations & PNG_INTERLACE))
   {
      switch (ni_png_ptr->pass)
      {
         case 0:
            if (ni_png_ptr->row_number & 0x07)
            {
               if (dsp_row != NULL)
                  ni_png_combine_row(ni_png_ptr, dsp_row,
                     ni_png_pass_dsp_mask[ni_png_ptr->pass]);
               ni_png_read_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 1:
            if ((ni_png_ptr->row_number & 0x07) || ni_png_ptr->width < 5)
            {
               if (dsp_row != NULL)
                  ni_png_combine_row(ni_png_ptr, dsp_row,
                     ni_png_pass_dsp_mask[ni_png_ptr->pass]);
               ni_png_read_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 2:
            if ((ni_png_ptr->row_number & 0x07) != 4)
            {
               if (dsp_row != NULL && (ni_png_ptr->row_number & 4))
                  ni_png_combine_row(ni_png_ptr, dsp_row,
                     ni_png_pass_dsp_mask[ni_png_ptr->pass]);
               ni_png_read_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 3:
            if ((ni_png_ptr->row_number & 3) || ni_png_ptr->width < 3)
            {
               if (dsp_row != NULL)
                  ni_png_combine_row(ni_png_ptr, dsp_row,
                     ni_png_pass_dsp_mask[ni_png_ptr->pass]);
               ni_png_read_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 4:
            if ((ni_png_ptr->row_number & 3) != 2)
            {
               if (dsp_row != NULL && (ni_png_ptr->row_number & 2))
                  ni_png_combine_row(ni_png_ptr, dsp_row,
                     ni_png_pass_dsp_mask[ni_png_ptr->pass]);
               ni_png_read_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 5:
            if ((ni_png_ptr->row_number & 1) || ni_png_ptr->width < 2)
            {
               if (dsp_row != NULL)
                  ni_png_combine_row(ni_png_ptr, dsp_row,
                     ni_png_pass_dsp_mask[ni_png_ptr->pass]);
               ni_png_read_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 6:
            if (!(ni_png_ptr->row_number & 1))
            {
               ni_png_read_finish_row(ni_png_ptr);
               return;
            }
            break;
      }
   }
#endif

   if (!(ni_png_ptr->mode & PNG_HAVE_IDAT))
      ni_png_error(ni_png_ptr, "Invalid attempt to read row data");

   ni_png_ptr->zstream.next_out = ni_png_ptr->row_buf;
   ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->irowbytes;
   do
   {
      if (!(ni_png_ptr->zstream.avail_in))
      {
         while (!ni_png_ptr->idat_size)
         {
            ni_png_byte chunk_length[4];

            ni_png_crc_finish(ni_png_ptr, 0);

            ni_png_read_data(ni_png_ptr, chunk_length, 4);
            ni_png_ptr->idat_size = ni_png_get_uint_31(ni_png_ptr,chunk_length);

            ni_png_reset_crc(ni_png_ptr);
            ni_png_crc_read(ni_png_ptr, ni_png_ptr->chunk_name, 4);
            if (ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
               ni_png_error(ni_png_ptr, "Not enough image data");
         }
         ni_png_ptr->zstream.avail_in = (uInt)ni_png_ptr->zbuf_size;
         ni_png_ptr->zstream.next_in = ni_png_ptr->zbuf;
         if (ni_png_ptr->zbuf_size > ni_png_ptr->idat_size)
            ni_png_ptr->zstream.avail_in = (uInt)ni_png_ptr->idat_size;
         ni_png_crc_read(ni_png_ptr, ni_png_ptr->zbuf,
            (ni_png_size_t)ni_png_ptr->zstream.avail_in);
         ni_png_ptr->idat_size -= ni_png_ptr->zstream.avail_in;
      }
      ret = inflate(&ni_png_ptr->zstream, Z_PARTIAL_FLUSH);
      if (ret == Z_STREAM_END)
      {
         if (ni_png_ptr->zstream.avail_out || ni_png_ptr->zstream.avail_in ||
            ni_png_ptr->idat_size)
            ni_png_error(ni_png_ptr, "Extra compressed data");
         ni_png_ptr->mode |= PNG_AFTER_IDAT;
         ni_png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
         break;
      }
      if (ret != Z_OK)
         ni_png_error(ni_png_ptr, ni_png_ptr->zstream.msg ? ni_png_ptr->zstream.msg :
                   "Decompression error");

   } while (ni_png_ptr->zstream.avail_out);

   ni_png_ptr->row_info.color_type = ni_png_ptr->color_type;
   ni_png_ptr->row_info.width = ni_png_ptr->iwidth;
   ni_png_ptr->row_info.channels = ni_png_ptr->channels;
   ni_png_ptr->row_info.bit_depth = ni_png_ptr->bit_depth;
   ni_png_ptr->row_info.pixel_depth = ni_png_ptr->pixel_depth;
   ni_png_ptr->row_info.rowbytes = PNG_ROWBYTES(ni_png_ptr->row_info.pixel_depth,
       ni_png_ptr->row_info.width);

   if(ni_png_ptr->row_buf[0])
   ni_png_read_filter_row(ni_png_ptr, &(ni_png_ptr->row_info),
      ni_png_ptr->row_buf + 1, ni_png_ptr->prev_row + 1,
      (int)(ni_png_ptr->row_buf[0]));

   ni_png_memcpy_check(ni_png_ptr, ni_png_ptr->prev_row, ni_png_ptr->row_buf,
      ni_png_ptr->rowbytes + 1);

#if defined(PNG_MNG_FEATURES_SUPPORTED)
   if((ni_png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) &&
      (ni_png_ptr->filter_type == PNG_INTRAPIXEL_DIFFERENCING))
   {
      /* Intrapixel differencing */
      ni_png_do_read_intrapixel(&(ni_png_ptr->row_info), ni_png_ptr->row_buf + 1);
   }
#endif


   if (ni_png_ptr->transformations || (ni_png_ptr->flags&PNG_FLAG_STRIP_ALPHA))
      ni_png_do_read_transformations(ni_png_ptr);

#if defined(PNG_READ_INTERLACING_SUPPORTED)
   /* blow up interlaced rows to full size */
   if (ni_png_ptr->interlaced &&
      (ni_png_ptr->transformations & PNG_INTERLACE))
   {
      if (ni_png_ptr->pass < 6)
/*       old interface (pre-1.0.9):
         ni_png_do_read_interlace(&(ni_png_ptr->row_info),
            ni_png_ptr->row_buf + 1, ni_png_ptr->pass, ni_png_ptr->transformations);
 */
         ni_png_do_read_interlace(ni_png_ptr);

      if (dsp_row != NULL)
         ni_png_combine_row(ni_png_ptr, dsp_row,
            ni_png_pass_dsp_mask[ni_png_ptr->pass]);
      if (row != NULL)
         ni_png_combine_row(ni_png_ptr, row,
            ni_png_pass_mask[ni_png_ptr->pass]);
   }
   else
#endif
   {
      if (row != NULL)
         ni_png_combine_row(ni_png_ptr, row, 0xff);
      if (dsp_row != NULL)
         ni_png_combine_row(ni_png_ptr, dsp_row, 0xff);
   }
   ni_png_read_finish_row(ni_png_ptr);

   if (ni_png_ptr->read_row_fn != NULL)
      (*(ni_png_ptr->read_row_fn))(ni_png_ptr, ni_png_ptr->row_number, ni_png_ptr->pass);
}
#endif /* PNG_NO_SEQUENTIAL_READ_SUPPORTED */

#ifndef PNG_NO_SEQUENTIAL_READ_SUPPORTED
/* Read one or more rows of image data.  If the image is interlaced,
 * and ni_png_set_interlace_handling() has been called, the rows need to
 * contain the contents of the rows from the previous pass.  If the
 * image has alpha or transparency, and ni_png_handle_alpha()[*] has been
 * called, the rows contents must be initialized to the contents of the
 * screen.
 *
 * "row" holds the actual image, and pixels are placed in it
 * as they arrive.  If the image is displayed after each pass, it will
 * appear to "sparkle" in.  "display_row" can be used to display a
 * "chunky" progressive image, with finer detail added as it becomes
 * available.  If you do not want this "chunky" display, you may pass
 * NULL for display_row.  If you do not want the sparkle display, and
 * you have not called ni_png_handle_alpha(), you may pass NULL for rows.
 * If you have called ni_png_handle_alpha(), and the image has either an
 * alpha channel or a transparency chunk, you must provide a buffer for
 * rows.  In this case, you do not have to provide a display_row buffer
 * also, but you may.  If the image is not interlaced, or if you have
 * not called ni_png_set_interlace_handling(), the display_row buffer will
 * be ignored, so pass NULL to it.
 *
 * [*] ni_png_handle_alpha() does not exist yet, as of this version of libpng
 */

void PNGAPI
ni_png_read_rows(ni_png_structp ni_png_ptr, ni_png_bytepp row,
   ni_png_bytepp display_row, ni_png_uint_32 num_rows)
{
   ni_png_uint_32 i;
   ni_png_bytepp rp;
   ni_png_bytepp dp;

   ni_png_debug(1, "in ni_png_read_rows\n");
   if(ni_png_ptr == NULL) return;
   rp = row;
   dp = display_row;
   if (rp != NULL && dp != NULL)
      for (i = 0; i < num_rows; i++)
      {
         ni_png_bytep rptr = *rp++;
         ni_png_bytep dptr = *dp++;

         ni_png_read_row(ni_png_ptr, rptr, dptr);
      }
   else if(rp != NULL)
      for (i = 0; i < num_rows; i++)
      {
         ni_png_bytep rptr = *rp;
         ni_png_read_row(ni_png_ptr, rptr, ni_png_bytep_NULL);
         rp++;
      }
   else if(dp != NULL)
      for (i = 0; i < num_rows; i++)
      {
         ni_png_bytep dptr = *dp;
         ni_png_read_row(ni_png_ptr, ni_png_bytep_NULL, dptr);
         dp++;
      }
}
#endif /* PNG_NO_SEQUENTIAL_READ_SUPPORTED */

#ifndef PNG_NO_SEQUENTIAL_READ_SUPPORTED
/* Read the entire image.  If the image has an alpha channel or a tRNS
 * chunk, and you have called ni_png_handle_alpha()[*], you will need to
 * initialize the image to the current image that PNG will be overlaying.
 * We set the num_rows again here, in case it was incorrectly set in
 * ni_png_read_start_row() by a call to ni_png_read_update_info() or
 * ni_png_start_read_image() if ni_png_set_interlace_handling() wasn't called
 * prior to either of these functions like it should have been.  You can
 * only call this function once.  If you desire to have an image for
 * each pass of a interlaced image, use ni_png_read_rows() instead.
 *
 * [*] ni_png_handle_alpha() does not exist yet, as of this version of libpng
 */
void PNGAPI
ni_png_read_image(ni_png_structp ni_png_ptr, ni_png_bytepp image)
{
   ni_png_uint_32 i,image_height;
   int pass, j;
   ni_png_bytepp rp;

   ni_png_debug(1, "in ni_png_read_image\n");
   if(ni_png_ptr == NULL) return;

#ifdef PNG_READ_INTERLACING_SUPPORTED
   pass = ni_png_set_interlace_handling(ni_png_ptr);
#else
   if (ni_png_ptr->interlaced)
      ni_png_error(ni_png_ptr,
        "Cannot read interlaced image -- interlace handler disabled.");
   pass = 1;
#endif


   image_height=ni_png_ptr->height;
   ni_png_ptr->num_rows = image_height; /* Make sure this is set correctly */

   for (j = 0; j < pass; j++)
   {
      rp = image;
      for (i = 0; i < image_height; i++)
      {
         ni_png_read_row(ni_png_ptr, *rp, ni_png_bytep_NULL);
         rp++;
      }
   }
}
#endif /* PNG_NO_SEQUENTIAL_READ_SUPPORTED */

#ifndef PNG_NO_SEQUENTIAL_READ_SUPPORTED
/* Read the end of the PNG file.  Will not read past the end of the
 * file, will verify the end is accurate, and will read any comments
 * or time information at the end of the file, if info is not NULL.
 */
void PNGAPI
ni_png_read_end(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   ni_png_byte chunk_length[4];
   ni_png_uint_32 length;

   ni_png_debug(1, "in ni_png_read_end\n");
   if(ni_png_ptr == NULL) return;
   ni_png_crc_finish(ni_png_ptr, 0); /* Finish off CRC from last IDAT chunk */

   do
   {
#ifdef PNG_USE_LOCAL_ARRAYS
      PNG_IHDR;
      PNG_IDAT;
      PNG_IEND;
      PNG_PLTE;
#if defined(PNG_READ_bKGD_SUPPORTED)
      PNG_bKGD;
#endif
#if defined(PNG_READ_cHRM_SUPPORTED)
      PNG_cHRM;
#endif
#if defined(PNG_READ_gAMA_SUPPORTED)
      PNG_gAMA;
#endif
#if defined(PNG_READ_hIST_SUPPORTED)
      PNG_hIST;
#endif
#if defined(PNG_READ_iCCP_SUPPORTED)
      PNG_iCCP;
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
      PNG_iTXt;
#endif
#if defined(PNG_READ_oFFs_SUPPORTED)
      PNG_oFFs;
#endif
#if defined(PNG_READ_pCAL_SUPPORTED)
      PNG_pCAL;
#endif
#if defined(PNG_READ_pHYs_SUPPORTED)
      PNG_pHYs;
#endif
#if defined(PNG_READ_sBIT_SUPPORTED)
      PNG_sBIT;
#endif
#if defined(PNG_READ_sCAL_SUPPORTED)
      PNG_sCAL;
#endif
#if defined(PNG_READ_sPLT_SUPPORTED)
      PNG_sPLT;
#endif
#if defined(PNG_READ_sRGB_SUPPORTED)
      PNG_sRGB;
#endif
#if defined(PNG_READ_tEXt_SUPPORTED)
      PNG_tEXt;
#endif
#if defined(PNG_READ_tIME_SUPPORTED)
      PNG_tIME;
#endif
#if defined(PNG_READ_tRNS_SUPPORTED)
      PNG_tRNS;
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
      PNG_zTXt;
#endif
#endif /* PNG_USE_LOCAL_ARRAYS */

      ni_png_read_data(ni_png_ptr, chunk_length, 4);
      length = ni_png_get_uint_31(ni_png_ptr,chunk_length);

      ni_png_reset_crc(ni_png_ptr);
      ni_png_crc_read(ni_png_ptr, ni_png_ptr->chunk_name, 4);

      ni_png_debug1(0, "Reading %s chunk.\n", ni_png_ptr->chunk_name);

      if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IHDR, 4))
         ni_png_handle_IHDR(ni_png_ptr, info_ptr, length);
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IEND, 4))
         ni_png_handle_IEND(ni_png_ptr, info_ptr, length);
#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
      else if (ni_png_handle_as_unknown(ni_png_ptr, ni_png_ptr->chunk_name))
      {
         if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
         {
            if ((length > 0) || (ni_png_ptr->mode & PNG_HAVE_CHUNK_AFTER_IDAT))
               ni_png_error(ni_png_ptr, "Too many IDAT's found");
         }
         ni_png_handle_unknown(ni_png_ptr, info_ptr, length);
         if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_PLTE, 4))
            ni_png_ptr->mode |= PNG_HAVE_PLTE;
      }
#endif
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
      {
         /* Zero length IDATs are legal after the last IDAT has been
          * read, but not after other chunks have been read.
          */
         if ((length > 0) || (ni_png_ptr->mode & PNG_HAVE_CHUNK_AFTER_IDAT))
            ni_png_error(ni_png_ptr, "Too many IDAT's found");
         ni_png_crc_finish(ni_png_ptr, length);
      }
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_PLTE, 4))
         ni_png_handle_PLTE(ni_png_ptr, info_ptr, length);
#if defined(PNG_READ_bKGD_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_bKGD, 4))
         ni_png_handle_bKGD(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_cHRM_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_cHRM, 4))
         ni_png_handle_cHRM(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_gAMA_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_gAMA, 4))
         ni_png_handle_gAMA(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_hIST_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_hIST, 4))
         ni_png_handle_hIST(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_oFFs_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_oFFs, 4))
         ni_png_handle_oFFs(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_pCAL_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_pCAL, 4))
         ni_png_handle_pCAL(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sCAL_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sCAL, 4))
         ni_png_handle_sCAL(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_pHYs_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_pHYs, 4))
         ni_png_handle_pHYs(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sBIT_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sBIT, 4))
         ni_png_handle_sBIT(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sRGB_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sRGB, 4))
         ni_png_handle_sRGB(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_iCCP_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_iCCP, 4))
         ni_png_handle_iCCP(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_sPLT_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sPLT, 4))
         ni_png_handle_sPLT(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tEXt_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tEXt, 4))
         ni_png_handle_tEXt(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tIME_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tIME, 4))
         ni_png_handle_tIME(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tRNS_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tRNS, 4))
         ni_png_handle_tRNS(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_zTXt, 4))
         ni_png_handle_zTXt(ni_png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_iTXt, 4))
         ni_png_handle_iTXt(ni_png_ptr, info_ptr, length);
#endif
      else
         ni_png_handle_unknown(ni_png_ptr, info_ptr, length);
   } while (!(ni_png_ptr->mode & PNG_HAVE_IEND));
}
#endif /* PNG_NO_SEQUENTIAL_READ_SUPPORTED */

/* free all memory used by the read */
void PNGAPI
ni_png_destroy_read_struct(ni_png_structpp ni_png_ptr_ptr, ni_png_infopp info_ptr_ptr,
   ni_png_infopp end_info_ptr_ptr)
{
   ni_png_structp ni_png_ptr = NULL;
   ni_png_infop info_ptr = NULL, end_info_ptr = NULL;
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_free_ptr free_fn;
   ni_png_voidp mem_ptr;
#endif

   ni_png_debug(1, "in ni_png_destroy_read_struct\n");
   if (ni_png_ptr_ptr != NULL)
      ni_png_ptr = *ni_png_ptr_ptr;

   if (info_ptr_ptr != NULL)
      info_ptr = *info_ptr_ptr;

   if (end_info_ptr_ptr != NULL)
      end_info_ptr = *end_info_ptr_ptr;

#ifdef PNG_USER_MEM_SUPPORTED
   free_fn = ni_png_ptr->free_fn;
   mem_ptr = ni_png_ptr->mem_ptr;
#endif

   ni_png_read_destroy(ni_png_ptr, info_ptr, end_info_ptr);

   if (info_ptr != NULL)
   {
#if defined(PNG_TEXT_SUPPORTED)
      ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_TEXT, -1);
#endif

#ifdef PNG_USER_MEM_SUPPORTED
      ni_png_destroy_struct_2((ni_png_voidp)info_ptr, (ni_png_free_ptr)free_fn,
          (ni_png_voidp)mem_ptr);
#else
      ni_png_destroy_struct((ni_png_voidp)info_ptr);
#endif
      *info_ptr_ptr = NULL;
   }

   if (end_info_ptr != NULL)
   {
#if defined(PNG_READ_TEXT_SUPPORTED)
      ni_png_free_data(ni_png_ptr, end_info_ptr, PNG_FREE_TEXT, -1);
#endif
#ifdef PNG_USER_MEM_SUPPORTED
      ni_png_destroy_struct_2((ni_png_voidp)end_info_ptr, (ni_png_free_ptr)free_fn,
         (ni_png_voidp)mem_ptr);
#else
      ni_png_destroy_struct((ni_png_voidp)end_info_ptr);
#endif
      *end_info_ptr_ptr = NULL;
   }

   if (ni_png_ptr != NULL)
   {
#ifdef PNG_USER_MEM_SUPPORTED
      ni_png_destroy_struct_2((ni_png_voidp)ni_png_ptr, (ni_png_free_ptr)free_fn,
          (ni_png_voidp)mem_ptr);
#else
      ni_png_destroy_struct((ni_png_voidp)ni_png_ptr);
#endif
      *ni_png_ptr_ptr = NULL;
   }
}

/* free all memory used by the read (old method) */
void /* PRIVATE */
ni_png_read_destroy(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_infop end_info_ptr)
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf tmp_jmp;
#endif
   ni_png_error_ptr error_fn;
   ni_png_error_ptr warning_fn;
   ni_png_voidp error_ptr;
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_free_ptr free_fn;
#endif

   ni_png_debug(1, "in ni_png_read_destroy\n");
   if (info_ptr != NULL)
      ni_png_info_destroy(ni_png_ptr, info_ptr);

   if (end_info_ptr != NULL)
      ni_png_info_destroy(ni_png_ptr, end_info_ptr);

   ni_png_free(ni_png_ptr, ni_png_ptr->zbuf);
   ni_png_free(ni_png_ptr, ni_png_ptr->big_row_buf);
   ni_png_free(ni_png_ptr, ni_png_ptr->prev_row);
#if defined(PNG_READ_DITHER_SUPPORTED)
   ni_png_free(ni_png_ptr, ni_png_ptr->palette_lookup);
   ni_png_free(ni_png_ptr, ni_png_ptr->dither_index);
#endif
#if defined(PNG_READ_GAMMA_SUPPORTED)
   ni_png_free(ni_png_ptr, ni_png_ptr->gamma_table);
#endif
#if defined(PNG_READ_BACKGROUND_SUPPORTED)
   ni_png_free(ni_png_ptr, ni_png_ptr->gamma_from_1);
   ni_png_free(ni_png_ptr, ni_png_ptr->gamma_to_1);
#endif
#ifdef PNG_FREE_ME_SUPPORTED
   if (ni_png_ptr->free_me & PNG_FREE_PLTE)
      ni_png_zfree(ni_png_ptr, ni_png_ptr->palette);
   ni_png_ptr->free_me &= ~PNG_FREE_PLTE;
#else
   if (ni_png_ptr->flags & PNG_FLAG_FREE_PLTE)
      ni_png_zfree(ni_png_ptr, ni_png_ptr->palette);
   ni_png_ptr->flags &= ~PNG_FLAG_FREE_PLTE;
#endif
#if defined(PNG_tRNS_SUPPORTED) || \
    defined(PNG_READ_EXPAND_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED)
#ifdef PNG_FREE_ME_SUPPORTED
   if (ni_png_ptr->free_me & PNG_FREE_TRNS)
      ni_png_free(ni_png_ptr, ni_png_ptr->trans);
   ni_png_ptr->free_me &= ~PNG_FREE_TRNS;
#else
   if (ni_png_ptr->flags & PNG_FLAG_FREE_TRNS)
      ni_png_free(ni_png_ptr, ni_png_ptr->trans);
   ni_png_ptr->flags &= ~PNG_FLAG_FREE_TRNS;
#endif
#endif
#if defined(PNG_READ_hIST_SUPPORTED)
#ifdef PNG_FREE_ME_SUPPORTED
   if (ni_png_ptr->free_me & PNG_FREE_HIST)
      ni_png_free(ni_png_ptr, ni_png_ptr->hist);
   ni_png_ptr->free_me &= ~PNG_FREE_HIST;
#else
   if (ni_png_ptr->flags & PNG_FLAG_FREE_HIST)
      ni_png_free(ni_png_ptr, ni_png_ptr->hist);
   ni_png_ptr->flags &= ~PNG_FLAG_FREE_HIST;
#endif
#endif
#if defined(PNG_READ_GAMMA_SUPPORTED)
   if (ni_png_ptr->gamma_16_table != NULL)
   {
      int i;
      int istop = (1 << (8 - ni_png_ptr->gamma_shift));
      for (i = 0; i < istop; i++)
      {
         ni_png_free(ni_png_ptr, ni_png_ptr->gamma_16_table[i]);
      }
   ni_png_free(ni_png_ptr, ni_png_ptr->gamma_16_table);
   }
#if defined(PNG_READ_BACKGROUND_SUPPORTED)
   if (ni_png_ptr->gamma_16_from_1 != NULL)
   {
      int i;
      int istop = (1 << (8 - ni_png_ptr->gamma_shift));
      for (i = 0; i < istop; i++)
      {
         ni_png_free(ni_png_ptr, ni_png_ptr->gamma_16_from_1[i]);
      }
   ni_png_free(ni_png_ptr, ni_png_ptr->gamma_16_from_1);
   }
   if (ni_png_ptr->gamma_16_to_1 != NULL)
   {
      int i;
      int istop = (1 << (8 - ni_png_ptr->gamma_shift));
      for (i = 0; i < istop; i++)
      {
         ni_png_free(ni_png_ptr, ni_png_ptr->gamma_16_to_1[i]);
      }
   ni_png_free(ni_png_ptr, ni_png_ptr->gamma_16_to_1);
   }
#endif
#endif
#if defined(PNG_TIME_RFC1123_SUPPORTED)
   ni_png_free(ni_png_ptr, ni_png_ptr->time_buffer);
#endif

   inflateEnd(&ni_png_ptr->zstream);
#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
   ni_png_free(ni_png_ptr, ni_png_ptr->save_buffer);
#endif

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
#ifdef PNG_TEXT_SUPPORTED
   ni_png_free(ni_png_ptr, ni_png_ptr->current_text);
#endif /* PNG_TEXT_SUPPORTED */
#endif /* PNG_PROGRESSIVE_READ_SUPPORTED */

   /* Save the important info out of the ni_png_struct, in case it is
    * being used again.
    */
#ifdef PNG_SETJMP_SUPPORTED
   ni_png_memcpy(tmp_jmp, ni_png_ptr->jmpbuf, ni_png_sizeof (jmp_buf));
#endif

   error_fn = ni_png_ptr->error_fn;
   warning_fn = ni_png_ptr->warning_fn;
   error_ptr = ni_png_ptr->error_ptr;
#ifdef PNG_USER_MEM_SUPPORTED
   free_fn = ni_png_ptr->free_fn;
#endif

   ni_png_memset(ni_png_ptr, 0, ni_png_sizeof (ni_png_struct));

   ni_png_ptr->error_fn = error_fn;
   ni_png_ptr->warning_fn = warning_fn;
   ni_png_ptr->error_ptr = error_ptr;
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_ptr->free_fn = free_fn;
#endif

#ifdef PNG_SETJMP_SUPPORTED
   ni_png_memcpy(ni_png_ptr->jmpbuf, tmp_jmp, ni_png_sizeof (jmp_buf));
#endif

}

void PNGAPI
ni_png_set_read_status_fn(ni_png_structp ni_png_ptr, ni_png_read_status_ptr read_row_fn)
{
   if(ni_png_ptr == NULL) return;
   ni_png_ptr->read_row_fn = read_row_fn;
}


#ifndef PNG_NO_SEQUENTIAL_READ_SUPPORTED
#if defined(PNG_INFO_IMAGE_SUPPORTED)
void PNGAPI
ni_png_read_png(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
                           int transforms,
                           voidp params)
{
   int row;

   if(ni_png_ptr == NULL) return;
#if defined(PNG_READ_INVERT_ALPHA_SUPPORTED)
   /* invert the alpha channel from opacity to transparency
    */
   if (transforms & PNG_TRANSFORM_INVERT_ALPHA)
       ni_png_set_invert_alpha(ni_png_ptr);
#endif

   /* ni_png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).
    */
   ni_png_read_info(ni_png_ptr, info_ptr);
   if (info_ptr->height > PNG_UINT_32_MAX/ni_png_sizeof(ni_png_bytep))
      ni_png_error(ni_png_ptr,"Image is too high to process with ni_png_read_png()");

   /* -------------- image transformations start here ------------------- */

#if defined(PNG_READ_16_TO_8_SUPPORTED)
   /* tell libpng to strip 16 bit/color files down to 8 bits per color
    */
   if (transforms & PNG_TRANSFORM_STRIP_16)
       ni_png_set_strip_16(ni_png_ptr);
#endif

#if defined(PNG_READ_STRIP_ALPHA_SUPPORTED)
   /* Strip alpha bytes from the input data without combining with
    * the background (not recommended).
    */
   if (transforms & PNG_TRANSFORM_STRIP_ALPHA)
       ni_png_set_strip_alpha(ni_png_ptr);
#endif

#if defined(PNG_READ_PACK_SUPPORTED) && !defined(PNG_READ_EXPAND_SUPPORTED)
   /* Extract multiple pixels with bit depths of 1, 2, or 4 from a single
    * byte into separate bytes (useful for paletted and grayscale images).
    */
   if (transforms & PNG_TRANSFORM_PACKING)
       ni_png_set_packing(ni_png_ptr);
#endif

#if defined(PNG_READ_PACKSWAP_SUPPORTED)
   /* Change the order of packed pixels to least significant bit first
    * (not useful if you are using ni_png_set_packing).
    */
   if (transforms & PNG_TRANSFORM_PACKSWAP)
       ni_png_set_packswap(ni_png_ptr);
#endif

#if defined(PNG_READ_EXPAND_SUPPORTED)
   /* Expand paletted colors into true RGB triplets
    * Expand grayscale images to full 8 bits from 1, 2, or 4 bits/pixel
    * Expand paletted or RGB images with transparency to full alpha
    * channels so the data will be available as RGBA quartets.
    */
   if (transforms & PNG_TRANSFORM_EXPAND)
       if ((ni_png_ptr->bit_depth < 8) ||
           (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE) ||
           (ni_png_get_valid(ni_png_ptr, info_ptr, PNG_INFO_tRNS)))
         ni_png_set_expand(ni_png_ptr);
#endif

   /* We don't handle background color or gamma transformation or dithering.
    */

#if defined(PNG_READ_INVERT_SUPPORTED)
   /* invert monochrome files to have 0 as white and 1 as black
    */
   if (transforms & PNG_TRANSFORM_INVERT_MONO)
       ni_png_set_invert_mono(ni_png_ptr);
#endif

#if defined(PNG_READ_SHIFT_SUPPORTED)
   /* If you want to shift the pixel values from the range [0,255] or
    * [0,65535] to the original [0,7] or [0,31], or whatever range the
    * colors were originally in:
    */
   if ((transforms & PNG_TRANSFORM_SHIFT)
       && ni_png_get_valid(ni_png_ptr, info_ptr, PNG_INFO_sBIT))
   {
      ni_png_color_8p sig_bit;

      ni_png_get_sBIT(ni_png_ptr, info_ptr, &sig_bit);
      ni_png_set_shift(ni_png_ptr, sig_bit);
   }
#endif

#if defined(PNG_READ_BGR_SUPPORTED)
   /* flip the RGB pixels to BGR (or RGBA to BGRA)
    */
   if (transforms & PNG_TRANSFORM_BGR)
       ni_png_set_bgr(ni_png_ptr);
#endif

#if defined(PNG_READ_SWAP_ALPHA_SUPPORTED)
   /* swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR)
    */
   if (transforms & PNG_TRANSFORM_SWAP_ALPHA)
       ni_png_set_swap_alpha(ni_png_ptr);
#endif

#if defined(PNG_READ_SWAP_SUPPORTED)
   /* swap bytes of 16 bit files to least significant byte first
    */
   if (transforms & PNG_TRANSFORM_SWAP_ENDIAN)
       ni_png_set_swap(ni_png_ptr);
#endif

   /* We don't handle adding filler bytes */

   /* Optional call to gamma correct and add the background to the palette
    * and update info structure.  REQUIRED if you are expecting libpng to
    * update the palette for you (i.e., you selected such a transform above).
    */
   ni_png_read_update_info(ni_png_ptr, info_ptr);

   /* -------------- image transformations end here ------------------- */

#ifdef PNG_FREE_ME_SUPPORTED
   ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_ROWS, 0);
#endif
   if(info_ptr->row_pointers == NULL)
   {
      info_ptr->row_pointers = (ni_png_bytepp)ni_png_malloc(ni_png_ptr,
         info_ptr->height * ni_png_sizeof(ni_png_bytep));
#ifdef PNG_FREE_ME_SUPPORTED
      info_ptr->free_me |= PNG_FREE_ROWS;
#endif
      for (row = 0; row < (int)info_ptr->height; row++)
      {
         info_ptr->row_pointers[row] = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
            ni_png_get_rowbytes(ni_png_ptr, info_ptr));
      }
   }

   ni_png_read_image(ni_png_ptr, info_ptr->row_pointers);
   info_ptr->valid |= PNG_INFO_IDAT;

   /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
   ni_png_read_end(ni_png_ptr, info_ptr);

   if(transforms == 0 || params == NULL)
      /* quiet compiler warnings */ return;

}
#endif /* PNG_INFO_IMAGE_SUPPORTED */
#endif /* PNG_NO_SEQUENTIAL_READ_SUPPORTED */
#endif /* PNG_READ_SUPPORTED */
