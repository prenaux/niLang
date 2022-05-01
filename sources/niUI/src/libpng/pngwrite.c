
/* pngwrite.c - general routines to write a PNG file
 *
 * Last changed in libpng 1.2.15 January 5, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 */

/* get internal access to png.h */
#define PNG_INTERNAL
#include "png.h"
#ifdef PNG_WRITE_SUPPORTED

/* Writes all the PNG information.  This is the suggested way to use the
 * library.  If you have a new chunk to add, make a function to write it,
 * and put it in the correct location here.  If you want the chunk written
 * after the image data, put it in ni_png_write_end().  I strongly encourage
 * you to supply a PNG_INFO_ flag, and check info_ptr->valid before writing
 * the chunk, as that will keep the code from breaking if you want to just
 * write a plain PNG file.  If you have long comments, I suggest writing
 * them in ni_png_write_end(), and compressing them.
 */
void PNGAPI
ni_png_write_info_before_PLTE(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   ni_png_debug(1, "in ni_png_write_info_before_PLTE\n");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;
   if (!(ni_png_ptr->mode & PNG_WROTE_INFO_BEFORE_PLTE))
   {
   ni_png_write_sig(ni_png_ptr); /* write PNG signature */
#if defined(PNG_MNG_FEATURES_SUPPORTED)
   if((ni_png_ptr->mode&PNG_HAVE_PNG_SIGNATURE)&&(ni_png_ptr->mng_features_permitted))
   {
      ni_png_warning(ni_png_ptr,"MNG features are not allowed in a PNG datastream");
      ni_png_ptr->mng_features_permitted=0;
   }
#endif
   /* write IHDR information. */
   ni_png_write_IHDR(ni_png_ptr, info_ptr->width, info_ptr->height,
      info_ptr->bit_depth, info_ptr->color_type, info_ptr->compression_type,
      info_ptr->filter_type,
#if defined(PNG_WRITE_INTERLACING_SUPPORTED)
      info_ptr->interlace_type);
#else
      0);
#endif
   /* the rest of these check to see if the valid field has the appropriate
      flag set, and if it does, writes the chunk. */
#if defined(PNG_WRITE_gAMA_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_gAMA)
   {
#  ifdef PNG_FLOATING_POINT_SUPPORTED
      ni_png_write_gAMA(ni_png_ptr, info_ptr->gamma);
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
      ni_png_write_gAMA_fixed(ni_png_ptr, info_ptr->int_gamma);
#  endif
#endif
   }
#endif
#if defined(PNG_WRITE_sRGB_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_sRGB)
      ni_png_write_sRGB(ni_png_ptr, (int)info_ptr->srgb_intent);
#endif
#if defined(PNG_WRITE_iCCP_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_iCCP)
      ni_png_write_iCCP(ni_png_ptr, info_ptr->iccp_name, PNG_COMPRESSION_TYPE_BASE,
                     info_ptr->iccp_profile, (int)info_ptr->iccp_proflen);
#endif
#if defined(PNG_WRITE_sBIT_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_sBIT)
      ni_png_write_sBIT(ni_png_ptr, &(info_ptr->sig_bit), info_ptr->color_type);
#endif
#if defined(PNG_WRITE_cHRM_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_cHRM)
   {
#ifdef PNG_FLOATING_POINT_SUPPORTED
      ni_png_write_cHRM(ni_png_ptr,
         info_ptr->x_white, info_ptr->y_white,
         info_ptr->x_red, info_ptr->y_red,
         info_ptr->x_green, info_ptr->y_green,
         info_ptr->x_blue, info_ptr->y_blue);
#else
#  ifdef PNG_FIXED_POINT_SUPPORTED
      ni_png_write_cHRM_fixed(ni_png_ptr,
         info_ptr->int_x_white, info_ptr->int_y_white,
         info_ptr->int_x_red, info_ptr->int_y_red,
         info_ptr->int_x_green, info_ptr->int_y_green,
         info_ptr->int_x_blue, info_ptr->int_y_blue);
#  endif
#endif
   }
#endif
#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
   if (info_ptr->unknown_chunks_num)
   {
       ni_png_unknown_chunk *up;

       ni_png_debug(5, "writing extra chunks\n");

       for (up = info_ptr->unknown_chunks;
            up < info_ptr->unknown_chunks + info_ptr->unknown_chunks_num;
            up++)
       {
         int keep=ni_png_handle_as_unknown(ni_png_ptr, up->name);
         if (keep != PNG_HANDLE_CHUNK_NEVER &&
            up->location && !(up->location & PNG_HAVE_PLTE) &&
            !(up->location & PNG_HAVE_IDAT) &&
            ((up->name[3] & 0x20) || keep == PNG_HANDLE_CHUNK_ALWAYS ||
            (ni_png_ptr->flags & PNG_FLAG_KEEP_UNSAFE_CHUNKS)))
         {
            ni_png_write_chunk(ni_png_ptr, up->name, up->data, up->size);
         }
       }
   }
#endif
      ni_png_ptr->mode |= PNG_WROTE_INFO_BEFORE_PLTE;
   }
}

void PNGAPI
ni_png_write_info(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
#if defined(PNG_WRITE_TEXT_SUPPORTED) || defined(PNG_WRITE_sPLT_SUPPORTED)
   int i;
#endif

   ni_png_debug(1, "in ni_png_write_info\n");

   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   ni_png_write_info_before_PLTE(ni_png_ptr, info_ptr);

   if (info_ptr->valid & PNG_INFO_PLTE)
      ni_png_write_PLTE(ni_png_ptr, info_ptr->palette,
         (ni_png_uint_32)info_ptr->num_palette);
   else if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      ni_png_error(ni_png_ptr, "Valid palette required for paletted images");

#if defined(PNG_WRITE_tRNS_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_tRNS)
      {
#if defined(PNG_WRITE_INVERT_ALPHA_SUPPORTED)
         /* invert the alpha channel (in tRNS) */
         if ((ni_png_ptr->transformations & PNG_INVERT_ALPHA) &&
            info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
         {
            int j;
            for (j=0; j<(int)info_ptr->num_trans; j++)
               info_ptr->trans[j] = (ni_png_byte)(255 - info_ptr->trans[j]);
         }
#endif
      ni_png_write_tRNS(ni_png_ptr, info_ptr->trans, &(info_ptr->trans_values),
         info_ptr->num_trans, info_ptr->color_type);
      }
#endif
#if defined(PNG_WRITE_bKGD_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_bKGD)
      ni_png_write_bKGD(ni_png_ptr, &(info_ptr->background), info_ptr->color_type);
#endif
#if defined(PNG_WRITE_hIST_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_hIST)
      ni_png_write_hIST(ni_png_ptr, info_ptr->hist, info_ptr->num_palette);
#endif
#if defined(PNG_WRITE_oFFs_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_oFFs)
      ni_png_write_oFFs(ni_png_ptr, info_ptr->x_offset, info_ptr->y_offset,
         info_ptr->offset_unit_type);
#endif
#if defined(PNG_WRITE_pCAL_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_pCAL)
      ni_png_write_pCAL(ni_png_ptr, info_ptr->pcal_purpose, info_ptr->pcal_X0,
         info_ptr->pcal_X1, info_ptr->pcal_type, info_ptr->pcal_nparams,
         info_ptr->pcal_units, info_ptr->pcal_params);
#endif
#if defined(PNG_WRITE_sCAL_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_sCAL)
#if defined(PNG_FLOATING_POINT_SUPPORTED) && !defined(PNG_NO_STDIO)
      ni_png_write_sCAL(ni_png_ptr, (int)info_ptr->scal_unit,
          info_ptr->scal_pixel_width, info_ptr->scal_pixel_height);
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
      ni_png_write_sCAL_s(ni_png_ptr, (int)info_ptr->scal_unit,
          info_ptr->scal_s_width, info_ptr->scal_s_height);
#else
      ni_png_warning(ni_png_ptr,
          "ni_png_write_sCAL not supported; sCAL chunk not written.");
#endif
#endif
#endif
#if defined(PNG_WRITE_pHYs_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_pHYs)
      ni_png_write_pHYs(ni_png_ptr, info_ptr->x_pixels_per_unit,
         info_ptr->y_pixels_per_unit, info_ptr->phys_unit_type);
#endif
#if defined(PNG_WRITE_tIME_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_tIME)
   {
      ni_png_write_tIME(ni_png_ptr, &(info_ptr->mod_time));
      ni_png_ptr->mode |= PNG_WROTE_tIME;
   }
#endif
#if defined(PNG_WRITE_sPLT_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_sPLT)
     for (i = 0; i < (int)info_ptr->splt_palettes_num; i++)
       ni_png_write_sPLT(ni_png_ptr, info_ptr->splt_palettes + i);
#endif
#if defined(PNG_WRITE_TEXT_SUPPORTED)
   /* Check to see if we need to write text chunks */
   for (i = 0; i < info_ptr->num_text; i++)
   {
      ni_png_debug2(2, "Writing header text chunk %d, type %d\n", i,
         info_ptr->text[i].compression);
      /* an internationalized chunk? */
      if (info_ptr->text[i].compression > 0)
      {
#if defined(PNG_WRITE_iTXt_SUPPORTED)
          /* write international chunk */
          ni_png_write_iTXt(ni_png_ptr,
                         info_ptr->text[i].compression,
                         info_ptr->text[i].key,
                         info_ptr->text[i].lang,
                         info_ptr->text[i].lang_key,
                         info_ptr->text[i].text);
#else
          ni_png_warning(ni_png_ptr, "Unable to write international text");
#endif
          /* Mark this chunk as written */
          info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
      }
      /* If we want a compressed text chunk */
      else if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_zTXt)
      {
#if defined(PNG_WRITE_zTXt_SUPPORTED)
         /* write compressed chunk */
         ni_png_write_zTXt(ni_png_ptr, info_ptr->text[i].key,
            info_ptr->text[i].text, 0,
            info_ptr->text[i].compression);
#else
         ni_png_warning(ni_png_ptr, "Unable to write compressed text");
#endif
         /* Mark this chunk as written */
         info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_zTXt_WR;
      }
      else if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_NONE)
      {
#if defined(PNG_WRITE_tEXt_SUPPORTED)
         /* write uncompressed chunk */
         ni_png_write_tEXt(ni_png_ptr, info_ptr->text[i].key,
                         info_ptr->text[i].text,
                         0);
#else
         ni_png_warning(ni_png_ptr, "Unable to write uncompressed text");
#endif
         /* Mark this chunk as written */
         info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
      }
   }
#endif
#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
   if (info_ptr->unknown_chunks_num)
   {
       ni_png_unknown_chunk *up;

       ni_png_debug(5, "writing extra chunks\n");

       for (up = info_ptr->unknown_chunks;
            up < info_ptr->unknown_chunks + info_ptr->unknown_chunks_num;
            up++)
       {
         int keep=ni_png_handle_as_unknown(ni_png_ptr, up->name);
         if (keep != PNG_HANDLE_CHUNK_NEVER &&
            up->location && (up->location & PNG_HAVE_PLTE) &&
            !(up->location & PNG_HAVE_IDAT) &&
            ((up->name[3] & 0x20) || keep == PNG_HANDLE_CHUNK_ALWAYS ||
            (ni_png_ptr->flags & PNG_FLAG_KEEP_UNSAFE_CHUNKS)))
         {
            ni_png_write_chunk(ni_png_ptr, up->name, up->data, up->size);
         }
       }
   }
#endif
}

/* Writes the end of the PNG file.  If you don't want to write comments or
 * time information, you can pass NULL for info.  If you already wrote these
 * in ni_png_write_info(), do not write them again here.  If you have long
 * comments, I suggest writing them here, and compressing them.
 */
void PNGAPI
ni_png_write_end(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   ni_png_debug(1, "in ni_png_write_end\n");
   if (ni_png_ptr == NULL)
      return;
   if (!(ni_png_ptr->mode & PNG_HAVE_IDAT))
      ni_png_error(ni_png_ptr, "No IDATs written into file");

   /* see if user wants us to write information chunks */
   if (info_ptr != NULL)
   {
#if defined(PNG_WRITE_TEXT_SUPPORTED)
      int i; /* local index variable */
#endif
#if defined(PNG_WRITE_tIME_SUPPORTED)
      /* check to see if user has supplied a time chunk */
      if ((info_ptr->valid & PNG_INFO_tIME) &&
         !(ni_png_ptr->mode & PNG_WROTE_tIME))
         ni_png_write_tIME(ni_png_ptr, &(info_ptr->mod_time));
#endif
#if defined(PNG_WRITE_TEXT_SUPPORTED)
      /* loop through comment chunks */
      for (i = 0; i < info_ptr->num_text; i++)
      {
         ni_png_debug2(2, "Writing trailer text chunk %d, type %d\n", i,
            info_ptr->text[i].compression);
         /* an internationalized chunk? */
         if (info_ptr->text[i].compression > 0)
         {
#if defined(PNG_WRITE_iTXt_SUPPORTED)
             /* write international chunk */
             ni_png_write_iTXt(ni_png_ptr,
                         info_ptr->text[i].compression,
                         info_ptr->text[i].key,
                         info_ptr->text[i].lang,
                         info_ptr->text[i].lang_key,
                         info_ptr->text[i].text);
#else
             ni_png_warning(ni_png_ptr, "Unable to write international text");
#endif
             /* Mark this chunk as written */
             info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
         }
         else if (info_ptr->text[i].compression >= PNG_TEXT_COMPRESSION_zTXt)
         {
#if defined(PNG_WRITE_zTXt_SUPPORTED)
            /* write compressed chunk */
            ni_png_write_zTXt(ni_png_ptr, info_ptr->text[i].key,
               info_ptr->text[i].text, 0,
               info_ptr->text[i].compression);
#else
            ni_png_warning(ni_png_ptr, "Unable to write compressed text");
#endif
            /* Mark this chunk as written */
            info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_zTXt_WR;
         }
         else if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_NONE)
         {
#if defined(PNG_WRITE_tEXt_SUPPORTED)
            /* write uncompressed chunk */
            ni_png_write_tEXt(ni_png_ptr, info_ptr->text[i].key,
               info_ptr->text[i].text, 0);
#else
            ni_png_warning(ni_png_ptr, "Unable to write uncompressed text");
#endif

            /* Mark this chunk as written */
            info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
         }
      }
#endif
#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
   if (info_ptr->unknown_chunks_num)
   {
       ni_png_unknown_chunk *up;

       ni_png_debug(5, "writing extra chunks\n");

       for (up = info_ptr->unknown_chunks;
            up < info_ptr->unknown_chunks + info_ptr->unknown_chunks_num;
            up++)
       {
         int keep=ni_png_handle_as_unknown(ni_png_ptr, up->name);
         if (keep != PNG_HANDLE_CHUNK_NEVER &&
            up->location && (up->location & PNG_AFTER_IDAT) &&
            ((up->name[3] & 0x20) || keep == PNG_HANDLE_CHUNK_ALWAYS ||
            (ni_png_ptr->flags & PNG_FLAG_KEEP_UNSAFE_CHUNKS)))
         {
            ni_png_write_chunk(ni_png_ptr, up->name, up->data, up->size);
         }
       }
   }
#endif
   }

   ni_png_ptr->mode |= PNG_AFTER_IDAT;

   /* write end of PNG file */
   ni_png_write_IEND(ni_png_ptr);
#if 0
/* This flush, added in libpng-1.0.8,  causes some applications to crash
   because they do not set ni_png_ptr->output_flush_fn */
   ni_png_flush(ni_png_ptr);
#endif
}

#if defined(PNG_WRITE_tIME_SUPPORTED)
#if !defined(_WIN32_WCE)
/* "time.h" functions are not supported on WindowsCE */
void PNGAPI
ni_png_convert_from_struct_tm(ni_png_timep ptime, struct tm FAR * ttime)
{
   ni_png_debug(1, "in ni_png_convert_from_struct_tm\n");
   ptime->year = (ni_png_uint_16)(1900 + ttime->tm_year);
   ptime->month = (ni_png_byte)(ttime->tm_mon + 1);
   ptime->day = (ni_png_byte)ttime->tm_mday;
   ptime->hour = (ni_png_byte)ttime->tm_hour;
   ptime->minute = (ni_png_byte)ttime->tm_min;
   ptime->second = (ni_png_byte)ttime->tm_sec;
}

void PNGAPI
ni_png_convert_from_time_t(ni_png_timep ptime, time_t ttime)
{
   struct tm *tbuf;

   ni_png_debug(1, "in ni_png_convert_from_time_t\n");
   tbuf = gmtime(&ttime);
   ni_png_convert_from_struct_tm(ptime, tbuf);
}
#endif
#endif

/* Initialize ni_png_ptr structure, and allocate any memory needed */
ni_png_structp PNGAPI
ni_png_create_write_struct(ni_png_const_charp user_png_ver, ni_png_voidp error_ptr,
   ni_png_error_ptr error_fn, ni_png_error_ptr warn_fn)
{
#ifdef PNG_USER_MEM_SUPPORTED
   return (ni_png_create_write_struct_2(user_png_ver, error_ptr, error_fn,
      warn_fn, ni_png_voidp_NULL, ni_png_malloc_ptr_NULL, ni_png_free_ptr_NULL));
}

/* Alternate initialize ni_png_ptr structure, and allocate any memory needed */
ni_png_structp PNGAPI
ni_png_create_write_struct_2(ni_png_const_charp user_png_ver, ni_png_voidp error_ptr,
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
   ni_png_debug(1, "in ni_png_create_write_struct\n");
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_ptr = (ni_png_structp)ni_png_create_struct_2(PNG_STRUCT_PNG,
      (ni_png_malloc_ptr)malloc_fn, (ni_png_voidp)mem_ptr);
#else
   ni_png_ptr = (ni_png_structp)ni_png_create_struct(PNG_STRUCT_PNG);
#endif /* PNG_USER_MEM_SUPPORTED */
   if (ni_png_ptr == NULL)
      return (NULL);

#if !defined(PNG_1_0_X)
#ifdef PNG_ASSEMBLER_CODE_SUPPORTED
#ifdef PNG_MMX_CODE_SUPPORTED
   ni_png_init_mmx_flags(ni_png_ptr);   /* 1.2.0 addition */
#endif
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
      ni_png_destroy_struct(ni_png_ptr);
      return (NULL);
   }
#ifdef USE_FAR_KEYWORD
   ni_png_memcpy(ni_png_ptr->jmpbuf,jmpbuf,ni_png_sizeof(jmp_buf));
#endif
#endif

#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_set_mem_fn(ni_png_ptr, mem_ptr, malloc_fn, free_fn);
#endif /* PNG_USER_MEM_SUPPORTED */
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

   ni_png_set_write_fn(ni_png_ptr, ni_png_voidp_NULL, ni_png_rw_ptr_NULL,
      ni_png_flush_ptr_NULL);

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   ni_png_set_filter_heuristics(ni_png_ptr, PNG_FILTER_HEURISTIC_DEFAULT,
      1, ni_png_doublep_NULL, ni_png_doublep_NULL);
#endif

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

/* Initialize ni_png_ptr structure, and allocate any memory needed */
#if defined(PNG_1_0_X) || defined(PNG_1_2_X)
/* Deprecated. */
#undef ni_png_write_init
void PNGAPI
ni_png_write_init(ni_png_structp ni_png_ptr)
{
   /* We only come here via pre-1.0.7-compiled applications */
   ni_png_write_init_2(ni_png_ptr, "1.0.6 or earlier", 0, 0);
}

void PNGAPI
ni_png_write_init_2(ni_png_structp ni_png_ptr, ni_png_const_charp user_png_ver,
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
       "The png struct allocated by the application for writing is too small.");
     }
   if(ni_png_sizeof(ni_png_info) > ni_png_info_size)
     {
       ni_png_ptr->error_fn=NULL;
#ifdef PNG_ERROR_NUMBERS_SUPPORTED
       ni_png_ptr->flags=0;
#endif
       ni_png_error(ni_png_ptr,
       "The info struct allocated by the application for writing is too small.");
     }
   ni_png_write_init_3(&ni_png_ptr, user_png_ver, ni_png_struct_size);
}
#endif /* PNG_1_0_X || PNG_1_2_X */


void PNGAPI
ni_png_write_init_3(ni_png_structpp ptr_ptr, ni_png_const_charp user_png_ver,
   ni_png_size_t ni_png_struct_size)
{
   ni_png_structp ni_png_ptr=*ptr_ptr;
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf tmp_jmp; /* to save current jump buffer */
#endif

   int i = 0;

   if (ni_png_ptr == NULL)
      return;

   do
   {
     if (user_png_ver[i] != ni_png_libpng_ver[i])
     {
#ifdef PNG_LEGACY_SUPPORTED
       ni_png_ptr->flags |= PNG_FLAG_LIBRARY_MISMATCH;
#else
       ni_png_ptr->warning_fn=NULL;
       ni_png_warning(ni_png_ptr,
     "Application uses deprecated ni_png_write_init() and should be recompiled.");
       break;
#endif
     }
   } while (ni_png_libpng_ver[i++]);

   ni_png_debug(1, "in ni_png_write_init_3\n");

#ifdef PNG_SETJMP_SUPPORTED
   /* save jump buffer and error functions */
   ni_png_memcpy(tmp_jmp, ni_png_ptr->jmpbuf, ni_png_sizeof (jmp_buf));
#endif

   if (ni_png_sizeof(ni_png_struct) > ni_png_struct_size)
     {
       ni_png_destroy_struct(ni_png_ptr);
       ni_png_ptr = (ni_png_structp)ni_png_create_struct(PNG_STRUCT_PNG);
       *ptr_ptr = ni_png_ptr;
     }

   /* reset all variables to 0 */
   ni_png_memset(ni_png_ptr, 0, ni_png_sizeof (ni_png_struct));

   /* added at libpng-1.2.6 */
#ifdef PNG_SET_USER_LIMITS_SUPPORTED
   ni_png_ptr->user_width_max=PNG_USER_WIDTH_MAX;
   ni_png_ptr->user_height_max=PNG_USER_HEIGHT_MAX;
#endif

#if !defined(PNG_1_0_X)
#ifdef PNG_ASSEMBLER_CODE_SUPPORTED
#ifdef PNG_MMX_CODE_SUPPORTED
   ni_png_init_mmx_flags(ni_png_ptr);   /* 1.2.0 addition */
#endif
#endif
#endif /* PNG_1_0_X */

#ifdef PNG_SETJMP_SUPPORTED
   /* restore jump buffer */
   ni_png_memcpy(ni_png_ptr->jmpbuf, tmp_jmp, ni_png_sizeof (jmp_buf));
#endif

   ni_png_set_write_fn(ni_png_ptr, ni_png_voidp_NULL, ni_png_rw_ptr_NULL,
      ni_png_flush_ptr_NULL);

   /* initialize zbuf - compression buffer */
   ni_png_ptr->zbuf_size = PNG_ZBUF_SIZE;
   ni_png_ptr->zbuf = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
      (ni_png_uint_32)ni_png_ptr->zbuf_size);

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   ni_png_set_filter_heuristics(ni_png_ptr, PNG_FILTER_HEURISTIC_DEFAULT,
      1, ni_png_doublep_NULL, ni_png_doublep_NULL);
#endif
}

/* Write a few rows of image data.  If the image is interlaced,
 * either you will have to write the 7 sub images, or, if you
 * have called ni_png_set_interlace_handling(), you will have to
 * "write" the image seven times.
 */
void PNGAPI
ni_png_write_rows(ni_png_structp ni_png_ptr, ni_png_bytepp row,
   ni_png_uint_32 num_rows)
{
   ni_png_uint_32 i; /* row counter */
   ni_png_bytepp rp; /* row pointer */

   ni_png_debug(1, "in ni_png_write_rows\n");

   if (ni_png_ptr == NULL)
      return;

   /* loop through the rows */
   for (i = 0, rp = row; i < num_rows; i++, rp++)
   {
      ni_png_write_row(ni_png_ptr, *rp);
   }
}

/* Write the image.  You only need to call this function once, even
 * if you are writing an interlaced image.
 */
void PNGAPI
ni_png_write_image(ni_png_structp ni_png_ptr, ni_png_bytepp image)
{
   ni_png_uint_32 i; /* row index */
   int pass, num_pass; /* pass variables */
   ni_png_bytepp rp; /* points to current row */

   if (ni_png_ptr == NULL)
      return;

   ni_png_debug(1, "in ni_png_write_image\n");
#if defined(PNG_WRITE_INTERLACING_SUPPORTED)
   /* intialize interlace handling.  If image is not interlaced,
      this will set pass to 1 */
   num_pass = ni_png_set_interlace_handling(ni_png_ptr);
#else
   num_pass = 1;
#endif
   /* loop through passes */
   for (pass = 0; pass < num_pass; pass++)
   {
      /* loop through image */
      for (i = 0, rp = image; i < ni_png_ptr->height; i++, rp++)
      {
         ni_png_write_row(ni_png_ptr, *rp);
      }
   }
}

/* called by user to write a row of image data */
void PNGAPI
ni_png_write_row(ni_png_structp ni_png_ptr, ni_png_bytep row)
{
   if (ni_png_ptr == NULL)
      return;
   ni_png_debug2(1, "in ni_png_write_row (row %ld, pass %d)\n",
      ni_png_ptr->row_number, ni_png_ptr->pass);

   /* initialize transformations and other stuff if first time */
   if (ni_png_ptr->row_number == 0 && ni_png_ptr->pass == 0)
   {
   /* make sure we wrote the header info */
   if (!(ni_png_ptr->mode & PNG_WROTE_INFO_BEFORE_PLTE))
      ni_png_error(ni_png_ptr,
         "ni_png_write_info was never called before ni_png_write_row.");

   /* check for transforms that have been set but were defined out */
#if !defined(PNG_WRITE_INVERT_SUPPORTED) && defined(PNG_READ_INVERT_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_INVERT_MONO)
      ni_png_warning(ni_png_ptr, "PNG_WRITE_INVERT_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_FILLER_SUPPORTED) && defined(PNG_READ_FILLER_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_FILLER)
      ni_png_warning(ni_png_ptr, "PNG_WRITE_FILLER_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_PACKSWAP_SUPPORTED) && defined(PNG_READ_PACKSWAP_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_PACKSWAP)
      ni_png_warning(ni_png_ptr, "PNG_WRITE_PACKSWAP_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_PACK_SUPPORTED) && defined(PNG_READ_PACK_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_PACK)
      ni_png_warning(ni_png_ptr, "PNG_WRITE_PACK_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_SHIFT_SUPPORTED) && defined(PNG_READ_SHIFT_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_SHIFT)
      ni_png_warning(ni_png_ptr, "PNG_WRITE_SHIFT_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_BGR_SUPPORTED) && defined(PNG_READ_BGR_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_BGR)
      ni_png_warning(ni_png_ptr, "PNG_WRITE_BGR_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_SWAP_SUPPORTED) && defined(PNG_READ_SWAP_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_SWAP_BYTES)
      ni_png_warning(ni_png_ptr, "PNG_WRITE_SWAP_SUPPORTED is not defined.");
#endif

      ni_png_write_start_row(ni_png_ptr);
   }

#if defined(PNG_WRITE_INTERLACING_SUPPORTED)
   /* if interlaced and not interested in row, return */
   if (ni_png_ptr->interlaced && (ni_png_ptr->transformations & PNG_INTERLACE))
   {
      switch (ni_png_ptr->pass)
      {
         case 0:
            if (ni_png_ptr->row_number & 0x07)
            {
               ni_png_write_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 1:
            if ((ni_png_ptr->row_number & 0x07) || ni_png_ptr->width < 5)
            {
               ni_png_write_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 2:
            if ((ni_png_ptr->row_number & 0x07) != 4)
            {
               ni_png_write_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 3:
            if ((ni_png_ptr->row_number & 0x03) || ni_png_ptr->width < 3)
            {
               ni_png_write_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 4:
            if ((ni_png_ptr->row_number & 0x03) != 2)
            {
               ni_png_write_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 5:
            if ((ni_png_ptr->row_number & 0x01) || ni_png_ptr->width < 2)
            {
               ni_png_write_finish_row(ni_png_ptr);
               return;
            }
            break;
         case 6:
            if (!(ni_png_ptr->row_number & 0x01))
            {
               ni_png_write_finish_row(ni_png_ptr);
               return;
            }
            break;
      }
   }
#endif

   /* set up row info for transformations */
   ni_png_ptr->row_info.color_type = ni_png_ptr->color_type;
   ni_png_ptr->row_info.width = ni_png_ptr->usr_width;
   ni_png_ptr->row_info.channels = ni_png_ptr->usr_channels;
   ni_png_ptr->row_info.bit_depth = ni_png_ptr->usr_bit_depth;
   ni_png_ptr->row_info.pixel_depth = (ni_png_byte)(ni_png_ptr->row_info.bit_depth *
      ni_png_ptr->row_info.channels);

   ni_png_ptr->row_info.rowbytes = PNG_ROWBYTES(ni_png_ptr->row_info.pixel_depth,
      ni_png_ptr->row_info.width);

   ni_png_debug1(3, "row_info->color_type = %d\n", ni_png_ptr->row_info.color_type);
   ni_png_debug1(3, "row_info->width = %lu\n", ni_png_ptr->row_info.width);
   ni_png_debug1(3, "row_info->channels = %d\n", ni_png_ptr->row_info.channels);
   ni_png_debug1(3, "row_info->bit_depth = %d\n", ni_png_ptr->row_info.bit_depth);
   ni_png_debug1(3, "row_info->pixel_depth = %d\n", ni_png_ptr->row_info.pixel_depth);
   ni_png_debug1(3, "row_info->rowbytes = %lu\n", ni_png_ptr->row_info.rowbytes);

   /* Copy user's row into buffer, leaving room for filter byte. */
   ni_png_memcpy_check(ni_png_ptr, ni_png_ptr->row_buf + 1, row,
      ni_png_ptr->row_info.rowbytes);

#if defined(PNG_WRITE_INTERLACING_SUPPORTED)
   /* handle interlacing */
   if (ni_png_ptr->interlaced && ni_png_ptr->pass < 6 &&
      (ni_png_ptr->transformations & PNG_INTERLACE))
   {
      ni_png_do_write_interlace(&(ni_png_ptr->row_info),
         ni_png_ptr->row_buf + 1, ni_png_ptr->pass);
      /* this should always get caught above, but still ... */
      if (!(ni_png_ptr->row_info.width))
      {
         ni_png_write_finish_row(ni_png_ptr);
         return;
      }
   }
#endif

   /* handle other transformations */
   if (ni_png_ptr->transformations)
      ni_png_do_write_transformations(ni_png_ptr);

#if defined(PNG_MNG_FEATURES_SUPPORTED)
   /* Write filter_method 64 (intrapixel differencing) only if
    * 1. Libpng was compiled with PNG_MNG_FEATURES_SUPPORTED and
    * 2. Libpng did not write a PNG signature (this filter_method is only
    *    used in PNG datastreams that are embedded in MNG datastreams) and
    * 3. The application called ni_png_permit_mng_features with a mask that
    *    included PNG_FLAG_MNG_FILTER_64 and
    * 4. The filter_method is 64 and
    * 5. The color_type is RGB or RGBA
    */
   if((ni_png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) &&
      (ni_png_ptr->filter_type == PNG_INTRAPIXEL_DIFFERENCING))
   {
      /* Intrapixel differencing */
      ni_png_do_write_intrapixel(&(ni_png_ptr->row_info), ni_png_ptr->row_buf + 1);
   }
#endif

   /* Find a filter if necessary, filter the row and write it out. */
   ni_png_write_find_filter(ni_png_ptr, &(ni_png_ptr->row_info));

   if (ni_png_ptr->write_row_fn != NULL)
      (*(ni_png_ptr->write_row_fn))(ni_png_ptr, ni_png_ptr->row_number, ni_png_ptr->pass);
}

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
/* Set the automatic flush interval or 0 to turn flushing off */
void PNGAPI
ni_png_set_flush(ni_png_structp ni_png_ptr, int nrows)
{
   ni_png_debug(1, "in ni_png_set_flush\n");
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->flush_dist = (nrows < 0 ? 0 : nrows);
}

/* flush the current output buffers now */
void PNGAPI
ni_png_write_flush(ni_png_structp ni_png_ptr)
{
   int wrote_IDAT;

   ni_png_debug(1, "in ni_png_write_flush\n");
   if (ni_png_ptr == NULL)
      return;
   /* We have already written out all of the data */
   if (ni_png_ptr->row_number >= ni_png_ptr->num_rows)
     return;

   do
   {
      int ret;

      /* compress the data */
      ret = deflate(&ni_png_ptr->zstream, Z_SYNC_FLUSH);
      wrote_IDAT = 0;

      /* check for compression errors */
      if (ret != Z_OK)
      {
         if (ni_png_ptr->zstream.msg != NULL)
            ni_png_error(ni_png_ptr, ni_png_ptr->zstream.msg);
         else
            ni_png_error(ni_png_ptr, "zlib error");
      }

      if (!(ni_png_ptr->zstream.avail_out))
      {
         /* write the IDAT and reset the zlib output buffer */
         ni_png_write_IDAT(ni_png_ptr, ni_png_ptr->zbuf,
                        ni_png_ptr->zbuf_size);
         ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
         ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
         wrote_IDAT = 1;
      }
   } while(wrote_IDAT == 1);

   /* If there is any data left to be output, write it into a new IDAT */
   if (ni_png_ptr->zbuf_size != ni_png_ptr->zstream.avail_out)
   {
      /* write the IDAT and reset the zlib output buffer */
      ni_png_write_IDAT(ni_png_ptr, ni_png_ptr->zbuf,
                     ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out);
      ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
      ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
   }
   ni_png_ptr->flush_rows = 0;
   ni_png_flush(ni_png_ptr);
}
#endif /* PNG_WRITE_FLUSH_SUPPORTED */

/* free all memory used by the write */
void PNGAPI
ni_png_destroy_write_struct(ni_png_structpp ni_png_ptr_ptr, ni_png_infopp info_ptr_ptr)
{
   ni_png_structp ni_png_ptr = NULL;
   ni_png_infop info_ptr = NULL;
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_free_ptr free_fn = NULL;
   ni_png_voidp mem_ptr = NULL;
#endif

   ni_png_debug(1, "in ni_png_destroy_write_struct\n");
   if (ni_png_ptr_ptr != NULL)
   {
      ni_png_ptr = *ni_png_ptr_ptr;
#ifdef PNG_USER_MEM_SUPPORTED
      free_fn = ni_png_ptr->free_fn;
      mem_ptr = ni_png_ptr->mem_ptr;
#endif
   }

   if (info_ptr_ptr != NULL)
      info_ptr = *info_ptr_ptr;

   if (info_ptr != NULL)
   {
      ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_ALL, -1);

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
      if (ni_png_ptr->num_chunk_list)
      {
         ni_png_free(ni_png_ptr, ni_png_ptr->chunk_list);
         ni_png_ptr->chunk_list=NULL;
         ni_png_ptr->num_chunk_list=0;
      }
#endif

#ifdef PNG_USER_MEM_SUPPORTED
      ni_png_destroy_struct_2((ni_png_voidp)info_ptr, (ni_png_free_ptr)free_fn,
         (ni_png_voidp)mem_ptr);
#else
      ni_png_destroy_struct((ni_png_voidp)info_ptr);
#endif
      *info_ptr_ptr = NULL;
   }

   if (ni_png_ptr != NULL)
   {
      ni_png_write_destroy(ni_png_ptr);
#ifdef PNG_USER_MEM_SUPPORTED
      ni_png_destroy_struct_2((ni_png_voidp)ni_png_ptr, (ni_png_free_ptr)free_fn,
         (ni_png_voidp)mem_ptr);
#else
      ni_png_destroy_struct((ni_png_voidp)ni_png_ptr);
#endif
      *ni_png_ptr_ptr = NULL;
   }
}


/* Free any memory used in ni_png_ptr struct (old method) */
void /* PRIVATE */
ni_png_write_destroy(ni_png_structp ni_png_ptr)
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf tmp_jmp; /* save jump buffer */
#endif
   ni_png_error_ptr error_fn;
   ni_png_error_ptr warning_fn;
   ni_png_voidp error_ptr;
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_free_ptr free_fn;
#endif

   ni_png_debug(1, "in ni_png_write_destroy\n");
   /* free any memory zlib uses */
   deflateEnd(&ni_png_ptr->zstream);

   /* free our memory.  ni_png_free checks NULL for us. */
   ni_png_free(ni_png_ptr, ni_png_ptr->zbuf);
   ni_png_free(ni_png_ptr, ni_png_ptr->row_buf);
   ni_png_free(ni_png_ptr, ni_png_ptr->prev_row);
   ni_png_free(ni_png_ptr, ni_png_ptr->sub_row);
   ni_png_free(ni_png_ptr, ni_png_ptr->up_row);
   ni_png_free(ni_png_ptr, ni_png_ptr->avg_row);
   ni_png_free(ni_png_ptr, ni_png_ptr->paeth_row);

#if defined(PNG_TIME_RFC1123_SUPPORTED)
   ni_png_free(ni_png_ptr, ni_png_ptr->time_buffer);
#endif

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   ni_png_free(ni_png_ptr, ni_png_ptr->prev_filters);
   ni_png_free(ni_png_ptr, ni_png_ptr->filter_weights);
   ni_png_free(ni_png_ptr, ni_png_ptr->inv_filter_weights);
   ni_png_free(ni_png_ptr, ni_png_ptr->filter_costs);
   ni_png_free(ni_png_ptr, ni_png_ptr->inv_filter_costs);
#endif

#ifdef PNG_SETJMP_SUPPORTED
   /* reset structure */
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

/* Allow the application to select one or more row filters to use. */
void PNGAPI
ni_png_set_filter(ni_png_structp ni_png_ptr, int method, int filters)
{
   ni_png_debug(1, "in ni_png_set_filter\n");
   if (ni_png_ptr == NULL)
      return;
#if defined(PNG_MNG_FEATURES_SUPPORTED)
   if((ni_png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) &&
      (method == PNG_INTRAPIXEL_DIFFERENCING))
         method = PNG_FILTER_TYPE_BASE;
#endif
   if (method == PNG_FILTER_TYPE_BASE)
   {
      switch (filters & (PNG_ALL_FILTERS | 0x07))
      {
         case 5:
         case 6:
         case 7: ni_png_warning(ni_png_ptr, "Unknown row filter for method 0");
         case PNG_FILTER_VALUE_NONE:  ni_png_ptr->do_filter=PNG_FILTER_NONE; break;
         case PNG_FILTER_VALUE_SUB:   ni_png_ptr->do_filter=PNG_FILTER_SUB;  break;
         case PNG_FILTER_VALUE_UP:    ni_png_ptr->do_filter=PNG_FILTER_UP;   break;
         case PNG_FILTER_VALUE_AVG:   ni_png_ptr->do_filter=PNG_FILTER_AVG;  break;
         case PNG_FILTER_VALUE_PAETH: ni_png_ptr->do_filter=PNG_FILTER_PAETH;break;
         default: ni_png_ptr->do_filter = (ni_png_byte)filters; break;
      }

      /* If we have allocated the row_buf, this means we have already started
       * with the image and we should have allocated all of the filter buffers
       * that have been selected.  If prev_row isn't already allocated, then
       * it is too late to start using the filters that need it, since we
       * will be missing the data in the previous row.  If an application
       * wants to start and stop using particular filters during compression,
       * it should start out with all of the filters, and then add and
       * remove them after the start of compression.
       */
      if (ni_png_ptr->row_buf != NULL)
      {
         if ((ni_png_ptr->do_filter & PNG_FILTER_SUB) && ni_png_ptr->sub_row == NULL)
         {
            ni_png_ptr->sub_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
              (ni_png_ptr->rowbytes + 1));
            ni_png_ptr->sub_row[0] = PNG_FILTER_VALUE_SUB;
         }

         if ((ni_png_ptr->do_filter & PNG_FILTER_UP) && ni_png_ptr->up_row == NULL)
         {
            if (ni_png_ptr->prev_row == NULL)
            {
               ni_png_warning(ni_png_ptr, "Can't add Up filter after starting");
               ni_png_ptr->do_filter &= ~PNG_FILTER_UP;
            }
            else
            {
               ni_png_ptr->up_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
                  (ni_png_ptr->rowbytes + 1));
               ni_png_ptr->up_row[0] = PNG_FILTER_VALUE_UP;
            }
         }

         if ((ni_png_ptr->do_filter & PNG_FILTER_AVG) && ni_png_ptr->avg_row == NULL)
         {
            if (ni_png_ptr->prev_row == NULL)
            {
               ni_png_warning(ni_png_ptr, "Can't add Average filter after starting");
               ni_png_ptr->do_filter &= ~PNG_FILTER_AVG;
            }
            else
            {
               ni_png_ptr->avg_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
                  (ni_png_ptr->rowbytes + 1));
               ni_png_ptr->avg_row[0] = PNG_FILTER_VALUE_AVG;
            }
         }

         if ((ni_png_ptr->do_filter & PNG_FILTER_PAETH) &&
             ni_png_ptr->paeth_row == NULL)
         {
            if (ni_png_ptr->prev_row == NULL)
            {
               ni_png_warning(ni_png_ptr, "Can't add Paeth filter after starting");
               ni_png_ptr->do_filter &= (ni_png_byte)(~PNG_FILTER_PAETH);
            }
            else
            {
               ni_png_ptr->paeth_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
                  (ni_png_ptr->rowbytes + 1));
               ni_png_ptr->paeth_row[0] = PNG_FILTER_VALUE_PAETH;
            }
         }

         if (ni_png_ptr->do_filter == PNG_NO_FILTERS)
            ni_png_ptr->do_filter = PNG_FILTER_NONE;
      }
   }
   else
      ni_png_error(ni_png_ptr, "Unknown custom filter method");
}

/* This allows us to influence the way in which libpng chooses the "best"
 * filter for the current scanline.  While the "minimum-sum-of-absolute-
 * differences metric is relatively fast and effective, there is some
 * question as to whether it can be improved upon by trying to keep the
 * filtered data going to zlib more consistent, hopefully resulting in
 * better compression.
 */
#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)      /* GRR 970116 */
void PNGAPI
ni_png_set_filter_heuristics(ni_png_structp ni_png_ptr, int heuristic_method,
   int num_weights, ni_png_doublep filter_weights,
   ni_png_doublep filter_costs)
{
   int i;

   ni_png_debug(1, "in ni_png_set_filter_heuristics\n");
   if (ni_png_ptr == NULL)
      return;
   if (heuristic_method >= PNG_FILTER_HEURISTIC_LAST)
   {
      ni_png_warning(ni_png_ptr, "Unknown filter heuristic method");
      return;
   }

   if (heuristic_method == PNG_FILTER_HEURISTIC_DEFAULT)
   {
      heuristic_method = PNG_FILTER_HEURISTIC_UNWEIGHTED;
   }

   if (num_weights < 0 || filter_weights == NULL ||
      heuristic_method == PNG_FILTER_HEURISTIC_UNWEIGHTED)
   {
      num_weights = 0;
   }

   ni_png_ptr->num_prev_filters = (ni_png_byte)num_weights;
   ni_png_ptr->heuristic_method = (ni_png_byte)heuristic_method;

   if (num_weights > 0)
   {
      if (ni_png_ptr->prev_filters == NULL)
      {
         ni_png_ptr->prev_filters = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
            (ni_png_uint_32)(ni_png_sizeof(ni_png_byte) * num_weights));

         /* To make sure that the weighting starts out fairly */
         for (i = 0; i < num_weights; i++)
         {
            ni_png_ptr->prev_filters[i] = 255;
         }
      }

      if (ni_png_ptr->filter_weights == NULL)
      {
         ni_png_ptr->filter_weights = (ni_png_uint_16p)ni_png_malloc(ni_png_ptr,
            (ni_png_uint_32)(ni_png_sizeof(ni_png_uint_16) * num_weights));

         ni_png_ptr->inv_filter_weights = (ni_png_uint_16p)ni_png_malloc(ni_png_ptr,
            (ni_png_uint_32)(ni_png_sizeof(ni_png_uint_16) * num_weights));
         for (i = 0; i < num_weights; i++)
         {
            ni_png_ptr->inv_filter_weights[i] =
            ni_png_ptr->filter_weights[i] = PNG_WEIGHT_FACTOR;
         }
      }

      for (i = 0; i < num_weights; i++)
      {
         if (filter_weights[i] < 0.0)
         {
            ni_png_ptr->inv_filter_weights[i] =
            ni_png_ptr->filter_weights[i] = PNG_WEIGHT_FACTOR;
         }
         else
         {
            ni_png_ptr->inv_filter_weights[i] =
               (ni_png_uint_16)((double)PNG_WEIGHT_FACTOR*filter_weights[i]+0.5);
            ni_png_ptr->filter_weights[i] =
               (ni_png_uint_16)((double)PNG_WEIGHT_FACTOR/filter_weights[i]+0.5);
         }
      }
   }

   /* If, in the future, there are other filter methods, this would
    * need to be based on ni_png_ptr->filter.
    */
   if (ni_png_ptr->filter_costs == NULL)
   {
      ni_png_ptr->filter_costs = (ni_png_uint_16p)ni_png_malloc(ni_png_ptr,
         (ni_png_uint_32)(ni_png_sizeof(ni_png_uint_16) * PNG_FILTER_VALUE_LAST));

      ni_png_ptr->inv_filter_costs = (ni_png_uint_16p)ni_png_malloc(ni_png_ptr,
         (ni_png_uint_32)(ni_png_sizeof(ni_png_uint_16) * PNG_FILTER_VALUE_LAST));

      for (i = 0; i < PNG_FILTER_VALUE_LAST; i++)
      {
         ni_png_ptr->inv_filter_costs[i] =
         ni_png_ptr->filter_costs[i] = PNG_COST_FACTOR;
      }
   }

   /* Here is where we set the relative costs of the different filters.  We
    * should take the desired compression level into account when setting
    * the costs, so that Paeth, for instance, has a high relative cost at low
    * compression levels, while it has a lower relative cost at higher
    * compression settings.  The filter types are in order of increasing
    * relative cost, so it would be possible to do this with an algorithm.
    */
   for (i = 0; i < PNG_FILTER_VALUE_LAST; i++)
   {
      if (filter_costs == NULL || filter_costs[i] < 0.0)
      {
         ni_png_ptr->inv_filter_costs[i] =
         ni_png_ptr->filter_costs[i] = PNG_COST_FACTOR;
      }
      else if (filter_costs[i] >= 1.0)
      {
         ni_png_ptr->inv_filter_costs[i] =
            (ni_png_uint_16)((double)PNG_COST_FACTOR / filter_costs[i] + 0.5);
         ni_png_ptr->filter_costs[i] =
            (ni_png_uint_16)((double)PNG_COST_FACTOR * filter_costs[i] + 0.5);
      }
   }
}
#endif /* PNG_WRITE_WEIGHTED_FILTER_SUPPORTED */

void PNGAPI
ni_png_set_compression_level(ni_png_structp ni_png_ptr, int level)
{
   ni_png_debug(1, "in ni_png_set_compression_level\n");
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_LEVEL;
   ni_png_ptr->zlib_level = level;
}

void PNGAPI
ni_png_set_compression_mem_level(ni_png_structp ni_png_ptr, int mem_level)
{
   ni_png_debug(1, "in ni_png_set_compression_mem_level\n");
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_MEM_LEVEL;
   ni_png_ptr->zlib_mem_level = mem_level;
}

void PNGAPI
ni_png_set_compression_strategy(ni_png_structp ni_png_ptr, int strategy)
{
   ni_png_debug(1, "in ni_png_set_compression_strategy\n");
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_STRATEGY;
   ni_png_ptr->zlib_strategy = strategy;
}

void PNGAPI
ni_png_set_compression_window_bits(ni_png_structp ni_png_ptr, int window_bits)
{
   if (ni_png_ptr == NULL)
      return;
   if (window_bits > 15)
      ni_png_warning(ni_png_ptr, "Only compression windows <= 32k supported by PNG");
   else if (window_bits < 8)
      ni_png_warning(ni_png_ptr, "Only compression windows >= 256 supported by PNG");
#ifndef WBITS_8_OK
   /* avoid libpng bug with 256-byte windows */
   if (window_bits == 8)
     {
       ni_png_warning(ni_png_ptr, "Compression window is being reset to 512");
       window_bits=9;
     }
#endif
   ni_png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_WINDOW_BITS;
   ni_png_ptr->zlib_window_bits = window_bits;
}

void PNGAPI
ni_png_set_compression_method(ni_png_structp ni_png_ptr, int method)
{
   ni_png_debug(1, "in ni_png_set_compression_method\n");
   if (ni_png_ptr == NULL)
      return;
   if (method != 8)
      ni_png_warning(ni_png_ptr, "Only compression method 8 is supported by PNG");
   ni_png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_METHOD;
   ni_png_ptr->zlib_method = method;
}

void PNGAPI
ni_png_set_write_status_fn(ni_png_structp ni_png_ptr, ni_png_write_status_ptr write_row_fn)
{
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->write_row_fn = write_row_fn;
}

#if defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
void PNGAPI
ni_png_set_write_user_transform_fn(ni_png_structp ni_png_ptr, ni_png_user_transform_ptr
   write_user_transform_fn)
{
   ni_png_debug(1, "in ni_png_set_write_user_transform_fn\n");
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->transformations |= PNG_USER_TRANSFORM;
   ni_png_ptr->write_user_transform_fn = write_user_transform_fn;
}
#endif


#if defined(PNG_INFO_IMAGE_SUPPORTED)
void PNGAPI
ni_png_write_png(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
              int transforms, voidp params)
{
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;
#if defined(PNG_WRITE_INVERT_ALPHA_SUPPORTED)
   /* invert the alpha channel from opacity to transparency */
   if (transforms & PNG_TRANSFORM_INVERT_ALPHA)
       ni_png_set_invert_alpha(ni_png_ptr);
#endif

   /* Write the file header information. */
   ni_png_write_info(ni_png_ptr, info_ptr);

   /* ------ these transformations don't touch the info structure ------- */

#if defined(PNG_WRITE_INVERT_SUPPORTED)
   /* invert monochrome pixels */
   if (transforms & PNG_TRANSFORM_INVERT_MONO)
       ni_png_set_invert_mono(ni_png_ptr);
#endif

#if defined(PNG_WRITE_SHIFT_SUPPORTED)
   /* Shift the pixels up to a legal bit depth and fill in
    * as appropriate to correctly scale the image.
    */
   if ((transforms & PNG_TRANSFORM_SHIFT)
               && (info_ptr->valid & PNG_INFO_sBIT))
       ni_png_set_shift(ni_png_ptr, &info_ptr->sig_bit);
#endif

#if defined(PNG_WRITE_PACK_SUPPORTED)
   /* pack pixels into bytes */
   if (transforms & PNG_TRANSFORM_PACKING)
       ni_png_set_packing(ni_png_ptr);
#endif

#if defined(PNG_WRITE_SWAP_ALPHA_SUPPORTED)
   /* swap location of alpha bytes from ARGB to RGBA */
   if (transforms & PNG_TRANSFORM_SWAP_ALPHA)
       ni_png_set_swap_alpha(ni_png_ptr);
#endif

#if defined(PNG_WRITE_FILLER_SUPPORTED)
   /* Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
    * RGB (4 channels -> 3 channels). The second parameter is not used.
    */
   if (transforms & PNG_TRANSFORM_STRIP_FILLER)
       ni_png_set_filler(ni_png_ptr, 0, PNG_FILLER_BEFORE);
#endif

#if defined(PNG_WRITE_BGR_SUPPORTED)
   /* flip BGR pixels to RGB */
   if (transforms & PNG_TRANSFORM_BGR)
       ni_png_set_bgr(ni_png_ptr);
#endif

#if defined(PNG_WRITE_SWAP_SUPPORTED)
   /* swap bytes of 16-bit files to most significant byte first */
   if (transforms & PNG_TRANSFORM_SWAP_ENDIAN)
       ni_png_set_swap(ni_png_ptr);
#endif

#if defined(PNG_WRITE_PACKSWAP_SUPPORTED)
   /* swap bits of 1, 2, 4 bit packed pixel formats */
   if (transforms & PNG_TRANSFORM_PACKSWAP)
       ni_png_set_packswap(ni_png_ptr);
#endif

   /* ----------------------- end of transformations ------------------- */

   /* write the bits */
   if (info_ptr->valid & PNG_INFO_IDAT)
       ni_png_write_image(ni_png_ptr, info_ptr->row_pointers);

   /* It is REQUIRED to call this to finish writing the rest of the file */
   ni_png_write_end(ni_png_ptr, info_ptr);

   if(transforms == 0 || params == NULL)
      /* quiet compiler warnings */ return;
}
#endif
#endif /* PNG_WRITE_SUPPORTED */
