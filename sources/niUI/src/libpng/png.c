
/* png.c - location for general purpose libpng functions
 *
 * Last changed in libpng 1.2.17 May 15, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 */

#define PNG_INTERNAL
#define PNG_NO_EXTERN
#include "png.h"

/* Generate a compiler error if there is an old png.h in the search path. */
typedef version_1_2_18 Your_png_h_is_not_version_1_2_18;

/* Version information for C files.  This had better match the version
 * string defined in png.h.  */

#ifdef PNG_USE_GLOBAL_ARRAYS
/* ni_png_libpng_ver was changed to a function in version 1.0.5c */
const char ni_png_libpng_ver[18] = PNG_LIBPNG_VER_STRING;

#ifdef PNG_READ_SUPPORTED

/* ni_png_sig was changed to a function in version 1.0.5c */
/* Place to hold the signature string for a PNG file. */
const ni_png_byte FARDATA ni_png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
#endif /* PNG_READ_SUPPORTED */

/* Invoke global declarations for constant strings for known chunk types */
PNG_IHDR;
PNG_IDAT;
PNG_IEND;
PNG_PLTE;
PNG_bKGD;
PNG_cHRM;
PNG_gAMA;
PNG_hIST;
PNG_iCCP;
PNG_iTXt;
PNG_oFFs;
PNG_pCAL;
PNG_sCAL;
PNG_pHYs;
PNG_sBIT;
PNG_sPLT;
PNG_sRGB;
PNG_tEXt;
PNG_tIME;
PNG_tRNS;
PNG_zTXt;

#ifdef PNG_READ_SUPPORTED
/* arrays to facilitate easy interlacing - use pass (0 - 6) as index */

/* start of interlace block */
const int FARDATA ni_png_pass_start[] = {0, 4, 0, 2, 0, 1, 0};

/* offset to next interlace block */
const int FARDATA ni_png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1};

/* start of interlace block in the y direction */
const int FARDATA ni_png_pass_ystart[] = {0, 0, 4, 0, 2, 0, 1};

/* offset to next interlace block in the y direction */
const int FARDATA ni_png_pass_yinc[] = {8, 8, 8, 4, 4, 2, 2};

/* width of interlace block (used in assembler routines only) */
#ifdef PNG_HAVE_MMX_COMBINE_ROW
const int FARDATA ni_png_pass_width[] = {8, 4, 4, 2, 2, 1, 1};
#endif

/* Height of interlace block.  This is not currently used - if you need
 * it, uncomment it here and in png.h
const int FARDATA ni_png_pass_height[] = {8, 8, 4, 4, 2, 2, 1};
*/

/* Mask to determine which pixels are valid in a pass */
const int FARDATA ni_png_pass_mask[] = {0x80, 0x08, 0x88, 0x22, 0xaa, 0x55, 0xff};

/* Mask to determine which pixels to overwrite while displaying */
const int FARDATA ni_png_pass_dsp_mask[]
   = {0xff, 0x0f, 0xff, 0x33, 0xff, 0x55, 0xff};

#endif /* PNG_READ_SUPPORTED */
#endif /* PNG_USE_GLOBAL_ARRAYS */

/* Tells libpng that we have already handled the first "num_bytes" bytes
 * of the PNG file signature.  If the PNG data is embedded into another
 * stream we can set num_bytes = 8 so that libpng will not attempt to read
 * or write any of the magic bytes before it starts on the IHDR.
 */

#ifdef PNG_READ_SUPPORTED
void PNGAPI
ni_png_set_sig_bytes(ni_png_structp ni_png_ptr, int num_bytes)
{
   if(ni_png_ptr == NULL) return;
   ni_png_debug(1, "in ni_png_set_sig_bytes\n");
   if (num_bytes > 8)
      ni_png_error(ni_png_ptr, "Too many bytes for PNG signature.");

   ni_png_ptr->sig_bytes = (ni_png_byte)(num_bytes < 0 ? 0 : num_bytes);
}

/* Checks whether the supplied bytes match the PNG signature.  We allow
 * checking less than the full 8-byte signature so that those apps that
 * already read the first few bytes of a file to determine the file type
 * can simply check the remaining bytes for extra assurance.  Returns
 * an integer less than, equal to, or greater than zero if sig is found,
 * respectively, to be less than, to match, or be greater than the correct
 * PNG signature (this is the same behaviour as strcmp, memcmp, etc).
 */
int PNGAPI
ni_png_sig_cmp(ni_png_bytep sig, ni_png_size_t start, ni_png_size_t num_to_check)
{
   ni_png_byte ni_png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
   if (num_to_check > 8)
      num_to_check = 8;
   else if (num_to_check < 1)
      return (-1);

   if (start > 7)
      return (-1);

   if (start + num_to_check > 8)
      num_to_check = 8 - start;

   return ((int)(ni_png_memcmp(&sig[start], &ni_png_signature[start], num_to_check)));
}

#if defined(PNG_1_0_X) || defined(PNG_1_2_X)
/* (Obsolete) function to check signature bytes.  It does not allow one
 * to check a partial signature.  This function might be removed in the
 * future - use ni_png_sig_cmp().  Returns true (nonzero) if the file is a PNG.
 */
int PNGAPI
ni_png_check_sig(ni_png_bytep sig, int num)
{
  return ((int)!ni_png_sig_cmp(sig, (ni_png_size_t)0, (ni_png_size_t)num));
}
#endif
#endif /* PNG_READ_SUPPORTED */

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
/* Function to allocate memory for zlib and clear it to 0. */
#ifdef PNG_1_0_X
voidpf PNGAPI
#else
voidpf /* private */
#endif
ni_png_zalloc(voidpf ni_png_ptr, uInt items, uInt size)
{
   ni_png_voidp ptr;
   ni_png_structp p=(ni_png_structp)ni_png_ptr;
   ni_png_uint_32 save_flags=p->flags;
   ni_png_uint_32 num_bytes;

   if(ni_png_ptr == NULL) return (NULL);
   if (items > PNG_UINT_32_MAX/size)
   {
     ni_png_warning (p, "Potential overflow in ni_png_zalloc()");
     return (NULL);
   }
   num_bytes = (ni_png_uint_32)items * size;

   p->flags|=PNG_FLAG_MALLOC_NULL_MEM_OK;
   ptr = (ni_png_voidp)ni_png_malloc((ni_png_structp)ni_png_ptr, num_bytes);
   p->flags=save_flags;

#if defined(PNG_1_0_X) && !defined(PNG_NO_ZALLOC_ZERO)
   if (ptr == NULL)
       return ((voidpf)ptr);

   if (num_bytes > (ni_png_uint_32)0x8000L)
   {
      ni_png_memset(ptr, 0, (ni_png_size_t)0x8000L);
      ni_png_memset((ni_png_bytep)ptr + (ni_png_size_t)0x8000L, 0,
         (ni_png_size_t)(num_bytes - (ni_png_uint_32)0x8000L));
   }
   else
   {
      ni_png_memset(ptr, 0, (ni_png_size_t)num_bytes);
   }
#endif
   return ((voidpf)ptr);
}

/* function to free memory for zlib */
#ifdef PNG_1_0_X
void PNGAPI
#else
void /* private */
#endif
ni_png_zfree(voidpf ni_png_ptr, voidpf ptr)
{
   ni_png_free((ni_png_structp)ni_png_ptr, (ni_png_voidp)ptr);
}

/* Reset the CRC variable to 32 bits of 1's.  Care must be taken
 * in case CRC is > 32 bits to leave the top bits 0.
 */
void /* PRIVATE */
ni_png_reset_crc(ni_png_structp ni_png_ptr)
{
   ni_png_ptr->crc = crc32(0, Z_NULL, 0);
}

/* Calculate the CRC over a section of data.  We can only pass as
 * much data to this routine as the largest single buffer size.  We
 * also check that this data will actually be used before going to the
 * trouble of calculating it.
 */
void /* PRIVATE */
ni_png_calculate_crc(ni_png_structp ni_png_ptr, ni_png_bytep ptr, ni_png_size_t length)
{
   int need_crc = 1;

   if (ni_png_ptr->chunk_name[0] & 0x20)                     /* ancillary */
   {
      if ((ni_png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_MASK) ==
          (PNG_FLAG_CRC_ANCILLARY_USE | PNG_FLAG_CRC_ANCILLARY_NOWARN))
         need_crc = 0;
   }
   else                                                    /* critical */
   {
      if (ni_png_ptr->flags & PNG_FLAG_CRC_CRITICAL_IGNORE)
         need_crc = 0;
   }

   if (need_crc)
      ni_png_ptr->crc = crc32(ni_png_ptr->crc, ptr, (uInt)length);
}

/* Allocate the memory for an info_struct for the application.  We don't
 * really need the ni_png_ptr, but it could potentially be useful in the
 * future.  This should be used in favour of malloc(ni_png_sizeof(ni_png_info))
 * and ni_png_info_init() so that applications that want to use a shared
 * libpng don't have to be recompiled if ni_png_info changes size.
 */
ni_png_infop PNGAPI
ni_png_create_info_struct(ni_png_structp ni_png_ptr)
{
   ni_png_infop info_ptr;

   ni_png_debug(1, "in ni_png_create_info_struct\n");
   if(ni_png_ptr == NULL) return (NULL);
#ifdef PNG_USER_MEM_SUPPORTED
   info_ptr = (ni_png_infop)ni_png_create_struct_2(PNG_STRUCT_INFO,
      ni_png_ptr->malloc_fn, ni_png_ptr->mem_ptr);
#else
   info_ptr = (ni_png_infop)ni_png_create_struct(PNG_STRUCT_INFO);
#endif
   if (info_ptr != NULL)
      ni_png_info_init_3(&info_ptr, ni_png_sizeof(ni_png_info));

   return (info_ptr);
}

/* This function frees the memory associated with a single info struct.
 * Normally, one would use either ni_png_destroy_read_struct() or
 * ni_png_destroy_write_struct() to free an info struct, but this may be
 * useful for some applications.
 */
void PNGAPI
ni_png_destroy_info_struct(ni_png_structp ni_png_ptr, ni_png_infopp info_ptr_ptr)
{
   ni_png_infop info_ptr = NULL;
   if(ni_png_ptr == NULL) return;

   ni_png_debug(1, "in ni_png_destroy_info_struct\n");
   if (info_ptr_ptr != NULL)
      info_ptr = *info_ptr_ptr;

   if (info_ptr != NULL)
   {
      ni_png_info_destroy(ni_png_ptr, info_ptr);

#ifdef PNG_USER_MEM_SUPPORTED
      ni_png_destroy_struct_2((ni_png_voidp)info_ptr, ni_png_ptr->free_fn,
          ni_png_ptr->mem_ptr);
#else
      ni_png_destroy_struct((ni_png_voidp)info_ptr);
#endif
      *info_ptr_ptr = NULL;
   }
}

/* Initialize the info structure.  This is now an internal function (0.89)
 * and applications using it are urged to use ni_png_create_info_struct()
 * instead.
 */
#if defined(PNG_1_0_X) || defined(PNG_1_2_X)
#undef ni_png_info_init
void PNGAPI
ni_png_info_init(ni_png_infop info_ptr)
{
   /* We only come here via pre-1.0.12-compiled applications */
   ni_png_info_init_3(&info_ptr, 0);
}
#endif

void PNGAPI
ni_png_info_init_3(ni_png_infopp ptr_ptr, ni_png_size_t ni_png_info_struct_size)
{
   ni_png_infop info_ptr = *ptr_ptr;

   if(info_ptr == NULL) return;

   ni_png_debug(1, "in ni_png_info_init_3\n");

   if(ni_png_sizeof(ni_png_info) > ni_png_info_struct_size)
     {
       ni_png_destroy_struct(info_ptr);
       info_ptr = (ni_png_infop)ni_png_create_struct(PNG_STRUCT_INFO);
       *ptr_ptr = info_ptr;
     }

   /* set everything to 0 */
   ni_png_memset(info_ptr, 0, ni_png_sizeof (ni_png_info));
}

#ifdef PNG_FREE_ME_SUPPORTED
void PNGAPI
ni_png_data_freer(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   int freer, ni_png_uint_32 mask)
{
   ni_png_debug(1, "in ni_png_data_freer\n");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;
   if(freer == PNG_DESTROY_WILL_FREE_DATA)
      info_ptr->free_me |= mask;
   else if(freer == PNG_USER_WILL_FREE_DATA)
      info_ptr->free_me &= ~mask;
   else
      ni_png_warning(ni_png_ptr,
         "Unknown freer parameter in ni_png_data_freer.");
}
#endif

void PNGAPI
ni_png_free_data(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 mask,
   int num)
{
   ni_png_debug(1, "in ni_png_free_data\n");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

#if defined(PNG_TEXT_SUPPORTED)
/* free text item num or (if num == -1) all text items */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_TEXT) & info_ptr->free_me)
#else
if (mask & PNG_FREE_TEXT)
#endif
{
   if (num != -1)
   {
     if (info_ptr->text && info_ptr->text[num].key)
     {
         ni_png_free(ni_png_ptr, info_ptr->text[num].key);
         info_ptr->text[num].key = NULL;
     }
   }
   else
   {
       int i;
       for (i = 0; i < info_ptr->num_text; i++)
           ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_TEXT, i);
       ni_png_free(ni_png_ptr, info_ptr->text);
       info_ptr->text = NULL;
       info_ptr->num_text=0;
   }
}
#endif

#if defined(PNG_tRNS_SUPPORTED)
/* free any tRNS entry */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_TRNS) & info_ptr->free_me)
#else
if ((mask & PNG_FREE_TRNS) && (ni_png_ptr->flags & PNG_FLAG_FREE_TRNS))
#endif
{
    ni_png_free(ni_png_ptr, info_ptr->trans);
    info_ptr->valid &= ~PNG_INFO_tRNS;
#ifndef PNG_FREE_ME_SUPPORTED
    ni_png_ptr->flags &= ~PNG_FLAG_FREE_TRNS;
#endif
    info_ptr->trans = NULL;
}
#endif

#if defined(PNG_sCAL_SUPPORTED)
/* free any sCAL entry */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_SCAL) & info_ptr->free_me)
#else
if (mask & PNG_FREE_SCAL)
#endif
{
#if defined(PNG_FIXED_POINT_SUPPORTED) && !defined(PNG_FLOATING_POINT_SUPPORTED)
    ni_png_free(ni_png_ptr, info_ptr->scal_s_width);
    ni_png_free(ni_png_ptr, info_ptr->scal_s_height);
    info_ptr->scal_s_width = NULL;
    info_ptr->scal_s_height = NULL;
#endif
    info_ptr->valid &= ~PNG_INFO_sCAL;
}
#endif

#if defined(PNG_pCAL_SUPPORTED)
/* free any pCAL entry */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_PCAL) & info_ptr->free_me)
#else
if (mask & PNG_FREE_PCAL)
#endif
{
    ni_png_free(ni_png_ptr, info_ptr->pcal_purpose);
    ni_png_free(ni_png_ptr, info_ptr->pcal_units);
    info_ptr->pcal_purpose = NULL;
    info_ptr->pcal_units = NULL;
    if (info_ptr->pcal_params != NULL)
    {
        int i;
        for (i = 0; i < (int)info_ptr->pcal_nparams; i++)
        {
          ni_png_free(ni_png_ptr, info_ptr->pcal_params[i]);
          info_ptr->pcal_params[i]=NULL;
        }
        ni_png_free(ni_png_ptr, info_ptr->pcal_params);
        info_ptr->pcal_params = NULL;
    }
    info_ptr->valid &= ~PNG_INFO_pCAL;
}
#endif

#if defined(PNG_iCCP_SUPPORTED)
/* free any iCCP entry */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_ICCP) & info_ptr->free_me)
#else
if (mask & PNG_FREE_ICCP)
#endif
{
    ni_png_free(ni_png_ptr, info_ptr->iccp_name);
    ni_png_free(ni_png_ptr, info_ptr->iccp_profile);
    info_ptr->iccp_name = NULL;
    info_ptr->iccp_profile = NULL;
    info_ptr->valid &= ~PNG_INFO_iCCP;
}
#endif

#if defined(PNG_sPLT_SUPPORTED)
/* free a given sPLT entry, or (if num == -1) all sPLT entries */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_SPLT) & info_ptr->free_me)
#else
if (mask & PNG_FREE_SPLT)
#endif
{
   if (num != -1)
   {
      if(info_ptr->splt_palettes)
      {
          ni_png_free(ni_png_ptr, info_ptr->splt_palettes[num].name);
          ni_png_free(ni_png_ptr, info_ptr->splt_palettes[num].entries);
          info_ptr->splt_palettes[num].name = NULL;
          info_ptr->splt_palettes[num].entries = NULL;
      }
   }
   else
   {
       if(info_ptr->splt_palettes_num)
       {
         int i;
         for (i = 0; i < (int)info_ptr->splt_palettes_num; i++)
            ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_SPLT, i);

         ni_png_free(ni_png_ptr, info_ptr->splt_palettes);
         info_ptr->splt_palettes = NULL;
         info_ptr->splt_palettes_num = 0;
       }
       info_ptr->valid &= ~PNG_INFO_sPLT;
   }
}
#endif

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
  if(ni_png_ptr->unknown_chunk.data)
  {
    ni_png_free(ni_png_ptr, ni_png_ptr->unknown_chunk.data);
    ni_png_ptr->unknown_chunk.data = NULL;
  }
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_UNKN) & info_ptr->free_me)
#else
if (mask & PNG_FREE_UNKN)
#endif
{
   if (num != -1)
   {
       if(info_ptr->unknown_chunks)
       {
          ni_png_free(ni_png_ptr, info_ptr->unknown_chunks[num].data);
          info_ptr->unknown_chunks[num].data = NULL;
       }
   }
   else
   {
       int i;

       if(info_ptr->unknown_chunks_num)
       {
         for (i = 0; i < (int)info_ptr->unknown_chunks_num; i++)
            ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_UNKN, i);

         ni_png_free(ni_png_ptr, info_ptr->unknown_chunks);
         info_ptr->unknown_chunks = NULL;
         info_ptr->unknown_chunks_num = 0;
       }
   }
}
#endif

#if defined(PNG_hIST_SUPPORTED)
/* free any hIST entry */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_HIST)  & info_ptr->free_me)
#else
if ((mask & PNG_FREE_HIST) && (ni_png_ptr->flags & PNG_FLAG_FREE_HIST))
#endif
{
    ni_png_free(ni_png_ptr, info_ptr->hist);
    info_ptr->hist = NULL;
    info_ptr->valid &= ~PNG_INFO_hIST;
#ifndef PNG_FREE_ME_SUPPORTED
    ni_png_ptr->flags &= ~PNG_FLAG_FREE_HIST;
#endif
}
#endif

/* free any PLTE entry that was internally allocated */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_PLTE) & info_ptr->free_me)
#else
if ((mask & PNG_FREE_PLTE) && (ni_png_ptr->flags & PNG_FLAG_FREE_PLTE))
#endif
{
    ni_png_zfree(ni_png_ptr, info_ptr->palette);
    info_ptr->palette = NULL;
    info_ptr->valid &= ~PNG_INFO_PLTE;
#ifndef PNG_FREE_ME_SUPPORTED
    ni_png_ptr->flags &= ~PNG_FLAG_FREE_PLTE;
#endif
    info_ptr->num_palette = 0;
}

#if defined(PNG_INFO_IMAGE_SUPPORTED)
/* free any image bits attached to the info structure */
#ifdef PNG_FREE_ME_SUPPORTED
if ((mask & PNG_FREE_ROWS) & info_ptr->free_me)
#else
if (mask & PNG_FREE_ROWS)
#endif
{
    if(info_ptr->row_pointers)
    {
       int row;
       for (row = 0; row < (int)info_ptr->height; row++)
       {
          ni_png_free(ni_png_ptr, info_ptr->row_pointers[row]);
          info_ptr->row_pointers[row]=NULL;
       }
       ni_png_free(ni_png_ptr, info_ptr->row_pointers);
       info_ptr->row_pointers=NULL;
    }
    info_ptr->valid &= ~PNG_INFO_IDAT;
}
#endif

#ifdef PNG_FREE_ME_SUPPORTED
   if(num == -1)
     info_ptr->free_me &= ~mask;
   else
     info_ptr->free_me &= ~(mask & ~PNG_FREE_MUL);
#endif
}

/* This is an internal routine to free any memory that the info struct is
 * pointing to before re-using it or freeing the struct itself.  Recall
 * that ni_png_free() checks for NULL pointers for us.
 */
void /* PRIVATE */
ni_png_info_destroy(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   ni_png_debug(1, "in ni_png_info_destroy\n");

   ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_ALL, -1);

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
   if (ni_png_ptr->num_chunk_list)
   {
       ni_png_free(ni_png_ptr, ni_png_ptr->chunk_list);
       ni_png_ptr->chunk_list=NULL;
       ni_png_ptr->num_chunk_list=0;
   }
#endif

   ni_png_info_init_3(&info_ptr, ni_png_sizeof(ni_png_info));
}
#endif /* defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED) */

/* This function returns a pointer to the io_ptr associated with the user
 * functions.  The application should free any memory associated with this
 * pointer before ni_png_write_destroy() or ni_png_read_destroy() are called.
 */
ni_png_voidp PNGAPI
ni_png_get_io_ptr(ni_png_structp ni_png_ptr)
{
   if(ni_png_ptr == NULL) return (NULL);
   return (ni_png_ptr->io_ptr);
}

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
#if !defined(PNG_NO_STDIO)
/* Initialize the default input/output functions for the PNG file.  If you
 * use your own read or write routines, you can call either ni_png_set_read_fn()
 * or ni_png_set_write_fn() instead of ni_png_init_io().  If you have defined
 * PNG_NO_STDIO, you must use a function of your own because "FILE *" isn't
 * necessarily available.
 */
void PNGAPI
ni_png_init_io(ni_png_structp ni_png_ptr, ni_png_FILE_p fp)
{
   ni_png_debug(1, "in ni_png_init_io\n");
   if(ni_png_ptr == NULL) return;
   ni_png_ptr->io_ptr = (ni_png_voidp)fp;
}
#endif

#if defined(PNG_TIME_RFC1123_SUPPORTED)
/* Convert the supplied time into an RFC 1123 string suitable for use in
 * a "Creation Time" or other text-based time string.
 */
ni_png_charp PNGAPI
ni_png_convert_to_rfc1123(ni_png_structp ni_png_ptr, ni_png_timep ptime)
{
   static PNG_CONST char short_months[12][4] =
        {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

   if(ni_png_ptr == NULL) return (NULL);
   if (ni_png_ptr->time_buffer == NULL)
   {
      ni_png_ptr->time_buffer = (ni_png_charp)ni_png_malloc(ni_png_ptr, (ni_png_uint_32)(29*
         ni_png_sizeof(char)));
   }

#if defined(_WIN32_WCE)
   {
      wchar_t time_buf[29];
      wsprintf(time_buf, TEXT("%d %S %d %02d:%02d:%02d +0000"),
          ptime->day % 32, short_months[(ptime->month - 1) % 12],
        ptime->year, ptime->hour % 24, ptime->minute % 60,
          ptime->second % 61);
      WideCharToMultiByte(CP_ACP, 0, time_buf, -1, ni_png_ptr->time_buffer, 29,
          NULL, NULL);
   }
#else
#ifdef USE_FAR_KEYWORD
   {
      char near_time_buf[29];
      sprintf(near_time_buf, "%d %s %d %02d:%02d:%02d +0000",
          ptime->day % 32, short_months[(ptime->month - 1) % 12],
          ptime->year, ptime->hour % 24, ptime->minute % 60,
          ptime->second % 61);
      ni_png_memcpy(ni_png_ptr->time_buffer, near_time_buf,
          29*ni_png_sizeof(char));
   }
#else
   sprintf(ni_png_ptr->time_buffer, "%d %s %d %02d:%02d:%02d +0000",
       ptime->day % 32, short_months[(ptime->month - 1) % 12],
       ptime->year, ptime->hour % 24, ptime->minute % 60,
       ptime->second % 61);
#endif
#endif /* _WIN32_WCE */
   return ((ni_png_charp)ni_png_ptr->time_buffer);
}
#endif /* PNG_TIME_RFC1123_SUPPORTED */

#if 0
/* Signature string for a PNG file. */
ni_png_bytep PNGAPI
ni_png_sig_bytes(void)
{
   return ((ni_png_bytep)"\211\120\116\107\015\012\032\012");
}
#endif
#endif /* defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED) */

ni_png_charp PNGAPI
ni_png_get_copyright(ni_png_structp ni_png_ptr)
{
   if (&ni_png_ptr != NULL)  /* silence compiler warning about unused ni_png_ptr */
   return ((ni_png_charp) "\n libpng version 1.2.18 - May 15, 2007\n\
   Copyright (c) 1998-2007 Glenn Randers-Pehrson\n\
   Copyright (c) 1996-1997 Andreas Dilger\n\
   Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.\n");
   return ((ni_png_charp) "");
}

/* The following return the library version as a short string in the
 * format 1.0.0 through 99.99.99zz.  To get the version of *.h files
 * used with your application, print out PNG_LIBPNG_VER_STRING, which
 * is defined in png.h.
 * Note: now there is no difference between ni_png_get_libpng_ver() and
 * ni_png_get_header_ver().  Due to the version_nn_nn_nn typedef guard,
 * it is guaranteed that png.c uses the correct version of png.h.
 */
ni_png_charp PNGAPI
ni_png_get_libpng_ver(ni_png_structp ni_png_ptr)
{
   /* Version of *.c files used when building libpng */
   if (&ni_png_ptr != NULL)  /* silence compiler warning about unused ni_png_ptr */
      return ((ni_png_charp) PNG_LIBPNG_VER_STRING);
   return ((ni_png_charp) "");
}

ni_png_charp PNGAPI
ni_png_get_header_ver(ni_png_structp ni_png_ptr)
{
   /* Version of *.h files used when building libpng */
   if (&ni_png_ptr != NULL)  /* silence compiler warning about unused ni_png_ptr */
      return ((ni_png_charp) PNG_LIBPNG_VER_STRING);
   return ((ni_png_charp) "");
}

ni_png_charp PNGAPI
ni_png_get_header_version(ni_png_structp ni_png_ptr)
{
   /* Returns longer string containing both version and date */
   if (&ni_png_ptr != NULL)  /* silence compiler warning about unused ni_png_ptr */
      return ((ni_png_charp) PNG_HEADER_VERSION_STRING);
   return ((ni_png_charp) "");
}

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
int PNGAPI
ni_png_handle_as_unknown(ni_png_structp ni_png_ptr, ni_png_bytep chunk_name)
{
   /* check chunk_name and return "keep" value if it's on the list, else 0 */
   int i;
   ni_png_bytep p;
   if((ni_png_ptr == NULL && chunk_name == NULL) || ni_png_ptr->num_chunk_list<=0)
      return 0;
   p=ni_png_ptr->chunk_list+ni_png_ptr->num_chunk_list*5-5;
   for (i = ni_png_ptr->num_chunk_list; i; i--, p-=5)
      if (!ni_png_memcmp(chunk_name, p, 4))
        return ((int)*(p+4));
   return 0;
}
#endif

/* This function, added to libpng-1.0.6g, is untested. */
int PNGAPI
ni_png_reset_zstream(ni_png_structp ni_png_ptr)
{
   if (ni_png_ptr == NULL) return Z_STREAM_ERROR;
   return (inflateReset(&ni_png_ptr->zstream));
}
#endif /* defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED) */

/* This function was added to libpng-1.0.7 */
ni_png_uint_32 PNGAPI
ni_png_access_version_number(void)
{
   /* Version of *.c files used when building libpng */
   return((ni_png_uint_32) PNG_LIBPNG_VER);
}


#if defined(PNG_READ_SUPPORTED) && defined(PNG_ASSEMBLER_CODE_SUPPORTED)
#if !defined(PNG_1_0_X)
#if defined(PNG_MMX_CODE_SUPPORTED)
/* this INTERNAL function was added to libpng 1.2.0 */
void /* PRIVATE */
ni_png_init_mmx_flags (ni_png_structp ni_png_ptr)
{
    if(ni_png_ptr == NULL) return;
    ni_png_ptr->mmx_rowbytes_threshold = 0;
    ni_png_ptr->mmx_bitdepth_threshold = 0;

#  if (defined(PNG_USE_PNGVCRD) || defined(PNG_USE_PNGGCCRD))

    ni_png_ptr->asm_flags |= PNG_ASM_FLAG_MMX_SUPPORT_COMPILED;

    if (ni_png_mmx_support() > 0) {
        ni_png_ptr->asm_flags |= PNG_ASM_FLAG_MMX_SUPPORT_IN_CPU
#    ifdef PNG_HAVE_MMX_COMBINE_ROW
                              | PNG_ASM_FLAG_MMX_READ_COMBINE_ROW
#    endif
#    ifdef PNG_HAVE_MMX_READ_INTERLACE
                              | PNG_ASM_FLAG_MMX_READ_INTERLACE
#    endif
#    ifndef PNG_HAVE_MMX_READ_FILTER_ROW
                              ;
#    else
                              | PNG_ASM_FLAG_MMX_READ_FILTER_SUB
                              | PNG_ASM_FLAG_MMX_READ_FILTER_UP
                              | PNG_ASM_FLAG_MMX_READ_FILTER_AVG
                              | PNG_ASM_FLAG_MMX_READ_FILTER_PAETH ;

        ni_png_ptr->mmx_rowbytes_threshold = PNG_MMX_ROWBYTES_THRESHOLD_DEFAULT;
        ni_png_ptr->mmx_bitdepth_threshold = PNG_MMX_BITDEPTH_THRESHOLD_DEFAULT;
#    endif
    } else {
        ni_png_ptr->asm_flags &= ~( PNG_ASM_FLAG_MMX_SUPPORT_IN_CPU
                               | PNG_MMX_READ_FLAGS
                               | PNG_MMX_WRITE_FLAGS );
    }

#  else /* !(PNGVCRD || PNGGCCRD) */

    /* clear all MMX flags; no support is compiled in */
    ni_png_ptr->asm_flags &= ~( PNG_MMX_FLAGS );

#  endif /* ?(PNGVCRD || PNGGCCRD) */
}

#endif /* !(PNG_MMX_CODE_SUPPORTED) */

/* this function was added to libpng 1.2.0 */
#if !defined(PNG_USE_PNGGCCRD) && \
    !(defined(PNG_MMX_CODE_SUPPORTED) && defined(PNG_USE_PNGVCRD))
int PNGAPI
ni_png_mmx_support(void)
{
    return -1;
}
#endif
#endif /* PNG_1_0_X  && PNG_ASSEMBLER_CODE_SUPPORTED */
#endif /* PNG_READ_SUPPORTED */

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
#ifdef PNG_SIZE_T
/* Added at libpng version 1.2.6 */
   PNG_EXTERN ni_png_size_t PNGAPI ni_png_convert_size PNGARG((size_t size));
ni_png_size_t PNGAPI
ni_png_convert_size(size_t size)
{
  if (size > (ni_png_size_t)-1)
     PNG_ABORT();  /* We haven't got access to ni_png_ptr, so no ni_png_error() */
  return ((ni_png_size_t)size);
}
#endif /* PNG_SIZE_T */
#endif /* defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED) */
