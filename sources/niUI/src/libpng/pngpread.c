
/* pngpread.c - read a png file in push mode
 *
 * Last changed in libpng 1.2.17 May 15, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 */

#define PNG_INTERNAL
#include "png.h"

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED

/* push model modes */
#define PNG_READ_SIG_MODE   0
#define PNG_READ_CHUNK_MODE 1
#define PNG_READ_IDAT_MODE  2
#define PNG_SKIP_MODE       3
#define PNG_READ_tEXt_MODE  4
#define PNG_READ_zTXt_MODE  5
#define PNG_READ_DONE_MODE  6
#define PNG_READ_iTXt_MODE  7
#define PNG_ERROR_MODE      8

void PNGAPI
ni_png_process_data(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_bytep buffer, ni_png_size_t buffer_size)
{
   if(ni_png_ptr == NULL) return;
   ni_png_push_restore_buffer(ni_png_ptr, buffer, buffer_size);

   while (ni_png_ptr->buffer_size)
   {
      ni_png_process_some_data(ni_png_ptr, info_ptr);
   }
}

/* What we do with the incoming data depends on what we were previously
 * doing before we ran out of data...
 */
void /* PRIVATE */
ni_png_process_some_data(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   if(ni_png_ptr == NULL) return;
   switch (ni_png_ptr->process_mode)
   {
      case PNG_READ_SIG_MODE:
      {
         ni_png_push_read_sig(ni_png_ptr, info_ptr);
         break;
      }
      case PNG_READ_CHUNK_MODE:
      {
         ni_png_push_read_chunk(ni_png_ptr, info_ptr);
         break;
      }
      case PNG_READ_IDAT_MODE:
      {
         ni_png_push_read_IDAT(ni_png_ptr);
         break;
      }
#if defined(PNG_READ_tEXt_SUPPORTED)
      case PNG_READ_tEXt_MODE:
      {
         ni_png_push_read_tEXt(ni_png_ptr, info_ptr);
         break;
      }
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
      case PNG_READ_zTXt_MODE:
      {
         ni_png_push_read_zTXt(ni_png_ptr, info_ptr);
         break;
      }
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
      case PNG_READ_iTXt_MODE:
      {
         ni_png_push_read_iTXt(ni_png_ptr, info_ptr);
         break;
      }
#endif
      case PNG_SKIP_MODE:
      {
         ni_png_push_crc_finish(ni_png_ptr);
         break;
      }
      default:
      {
         ni_png_ptr->buffer_size = 0;
         break;
      }
   }
}

/* Read any remaining signature bytes from the stream and compare them with
 * the correct PNG signature.  It is possible that this routine is called
 * with bytes already read from the signature, either because they have been
 * checked by the calling application, or because of multiple calls to this
 * routine.
 */
void /* PRIVATE */
ni_png_push_read_sig(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   ni_png_size_t num_checked = ni_png_ptr->sig_bytes,
             num_to_check = 8 - num_checked;

   if (ni_png_ptr->buffer_size < num_to_check)
   {
      num_to_check = ni_png_ptr->buffer_size;
   }

   ni_png_push_fill_buffer(ni_png_ptr, &(info_ptr->signature[num_checked]),
      num_to_check);
   ni_png_ptr->sig_bytes = (ni_png_byte)(ni_png_ptr->sig_bytes+num_to_check);

   if (ni_png_sig_cmp(info_ptr->signature, num_checked, num_to_check))
   {
      if (num_checked < 4 &&
          ni_png_sig_cmp(info_ptr->signature, num_checked, num_to_check - 4))
         ni_png_error(ni_png_ptr, "Not a PNG file");
      else
         ni_png_error(ni_png_ptr, "PNG file corrupted by ASCII conversion");
   }
   else
   {
      if (ni_png_ptr->sig_bytes >= 8)
      {
         ni_png_ptr->process_mode = PNG_READ_CHUNK_MODE;
      }
   }
}

void /* PRIVATE */
ni_png_push_read_chunk(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
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
#if defined(PNG_READ_sRGB_SUPPORTED)
      PNG_sRGB;
#endif
#if defined(PNG_READ_sPLT_SUPPORTED)
      PNG_sPLT;
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
   /* First we make sure we have enough data for the 4 byte chunk name
    * and the 4 byte chunk length before proceeding with decoding the
    * chunk data.  To fully decode each of these chunks, we also make
    * sure we have enough data in the buffer for the 4 byte CRC at the
    * end of every chunk (except IDAT, which is handled separately).
    */
   if (!(ni_png_ptr->mode & PNG_HAVE_CHUNK_HEADER))
   {
      ni_png_byte chunk_length[4];

      if (ni_png_ptr->buffer_size < 8)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }

      ni_png_push_fill_buffer(ni_png_ptr, chunk_length, 4);
      ni_png_ptr->push_length = ni_png_get_uint_31(ni_png_ptr,chunk_length);
      ni_png_reset_crc(ni_png_ptr);
      ni_png_crc_read(ni_png_ptr, ni_png_ptr->chunk_name, 4);
      ni_png_ptr->mode |= PNG_HAVE_CHUNK_HEADER;
   }

   if (!ni_png_memcmp(ni_png_ptr->chunk_name, (ni_png_bytep)ni_png_IDAT, 4))
     if(ni_png_ptr->mode & PNG_AFTER_IDAT)
        ni_png_ptr->mode |= PNG_HAVE_CHUNK_AFTER_IDAT;

   if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IHDR, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_IHDR(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IEND, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_IEND(ni_png_ptr, info_ptr, ni_png_ptr->push_length);

      ni_png_ptr->process_mode = PNG_READ_DONE_MODE;
      ni_png_push_have_end(ni_png_ptr, info_ptr);
   }
#ifdef PNG_HANDLE_AS_UNKNOWN_SUPPORTED
   else if (ni_png_handle_as_unknown(ni_png_ptr, ni_png_ptr->chunk_name))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
         ni_png_ptr->mode |= PNG_HAVE_IDAT;
      ni_png_handle_unknown(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
      if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_PLTE, 4))
         ni_png_ptr->mode |= PNG_HAVE_PLTE;
      else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))
      {
         if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
            ni_png_error(ni_png_ptr, "Missing IHDR before IDAT");
         else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
                  !(ni_png_ptr->mode & PNG_HAVE_PLTE))
            ni_png_error(ni_png_ptr, "Missing PLTE before IDAT");
      }
   }
#endif
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_PLTE, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_PLTE(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, (ni_png_bytep)ni_png_IDAT, 4))
   {
      /* If we reach an IDAT chunk, this means we have read all of the
       * header chunks, and we can start reading the image (or if this
       * is called after the image has been read - we have an error).
       */
     if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
       ni_png_error(ni_png_ptr, "Missing IHDR before IDAT");
     else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
         !(ni_png_ptr->mode & PNG_HAVE_PLTE))
       ni_png_error(ni_png_ptr, "Missing PLTE before IDAT");

      if (ni_png_ptr->mode & PNG_HAVE_IDAT)
      {
         if (!(ni_png_ptr->mode & PNG_HAVE_CHUNK_AFTER_IDAT))
           if (ni_png_ptr->push_length == 0)
              return;

         if (ni_png_ptr->mode & PNG_AFTER_IDAT)
            ni_png_error(ni_png_ptr, "Too many IDAT's found");
      }

      ni_png_ptr->idat_size = ni_png_ptr->push_length;
      ni_png_ptr->mode |= PNG_HAVE_IDAT;
      ni_png_ptr->process_mode = PNG_READ_IDAT_MODE;
      ni_png_push_have_info(ni_png_ptr, info_ptr);
      ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->irowbytes;
      ni_png_ptr->zstream.next_out = ni_png_ptr->row_buf;
      return;
   }
#if defined(PNG_READ_gAMA_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_gAMA, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_gAMA(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_sBIT_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sBIT, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_sBIT(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_cHRM_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_cHRM, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_cHRM(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_sRGB_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sRGB, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_sRGB(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_iCCP_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_iCCP, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_iCCP(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_sPLT_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sPLT, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_sPLT(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_tRNS_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tRNS, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_tRNS(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_bKGD_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_bKGD, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_bKGD(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_hIST_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_hIST, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_hIST(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_pHYs_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_pHYs, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_pHYs(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_oFFs_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_oFFs, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_oFFs(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_pCAL_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_pCAL, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_pCAL(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_sCAL_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_sCAL, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_sCAL(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_tIME_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tIME, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_handle_tIME(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_tEXt_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_tEXt, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_push_handle_tEXt(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_zTXt, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_push_handle_zTXt(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
   else if (!ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_iTXt, 4))
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_push_handle_iTXt(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }
#endif
   else
   {
      if (ni_png_ptr->push_length + 4 > ni_png_ptr->buffer_size)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }
      ni_png_push_handle_unknown(ni_png_ptr, info_ptr, ni_png_ptr->push_length);
   }

   ni_png_ptr->mode &= ~PNG_HAVE_CHUNK_HEADER;
}

void /* PRIVATE */
ni_png_push_crc_skip(ni_png_structp ni_png_ptr, ni_png_uint_32 skip)
{
   ni_png_ptr->process_mode = PNG_SKIP_MODE;
   ni_png_ptr->skip_length = skip;
}

void /* PRIVATE */
ni_png_push_crc_finish(ni_png_structp ni_png_ptr)
{
   if (ni_png_ptr->skip_length && ni_png_ptr->save_buffer_size)
   {
      ni_png_size_t save_size;

      if (ni_png_ptr->skip_length < (ni_png_uint_32)ni_png_ptr->save_buffer_size)
         save_size = (ni_png_size_t)ni_png_ptr->skip_length;
      else
         save_size = ni_png_ptr->save_buffer_size;

      ni_png_calculate_crc(ni_png_ptr, ni_png_ptr->save_buffer_ptr, save_size);

      ni_png_ptr->skip_length -= save_size;
      ni_png_ptr->buffer_size -= save_size;
      ni_png_ptr->save_buffer_size -= save_size;
      ni_png_ptr->save_buffer_ptr += save_size;
   }
   if (ni_png_ptr->skip_length && ni_png_ptr->current_buffer_size)
   {
      ni_png_size_t save_size;

      if (ni_png_ptr->skip_length < (ni_png_uint_32)ni_png_ptr->current_buffer_size)
         save_size = (ni_png_size_t)ni_png_ptr->skip_length;
      else
         save_size = ni_png_ptr->current_buffer_size;

      ni_png_calculate_crc(ni_png_ptr, ni_png_ptr->current_buffer_ptr, save_size);

      ni_png_ptr->skip_length -= save_size;
      ni_png_ptr->buffer_size -= save_size;
      ni_png_ptr->current_buffer_size -= save_size;
      ni_png_ptr->current_buffer_ptr += save_size;
   }
   if (!ni_png_ptr->skip_length)
   {
      if (ni_png_ptr->buffer_size < 4)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }

      ni_png_crc_finish(ni_png_ptr, 0);
      ni_png_ptr->process_mode = PNG_READ_CHUNK_MODE;
   }
}

void PNGAPI
ni_png_push_fill_buffer(ni_png_structp ni_png_ptr, ni_png_bytep buffer, ni_png_size_t length)
{
   ni_png_bytep ptr;

   if(ni_png_ptr == NULL) return;
   ptr = buffer;
   if (ni_png_ptr->save_buffer_size)
   {
      ni_png_size_t save_size;

      if (length < ni_png_ptr->save_buffer_size)
         save_size = length;
      else
         save_size = ni_png_ptr->save_buffer_size;

      ni_png_memcpy(ptr, ni_png_ptr->save_buffer_ptr, save_size);
      length -= save_size;
      ptr += save_size;
      ni_png_ptr->buffer_size -= save_size;
      ni_png_ptr->save_buffer_size -= save_size;
      ni_png_ptr->save_buffer_ptr += save_size;
   }
   if (length && ni_png_ptr->current_buffer_size)
   {
      ni_png_size_t save_size;

      if (length < ni_png_ptr->current_buffer_size)
         save_size = length;
      else
         save_size = ni_png_ptr->current_buffer_size;

      ni_png_memcpy(ptr, ni_png_ptr->current_buffer_ptr, save_size);
      ni_png_ptr->buffer_size -= save_size;
      ni_png_ptr->current_buffer_size -= save_size;
      ni_png_ptr->current_buffer_ptr += save_size;
   }
}

void /* PRIVATE */
ni_png_push_save_buffer(ni_png_structp ni_png_ptr)
{
   if (ni_png_ptr->save_buffer_size)
   {
      if (ni_png_ptr->save_buffer_ptr != ni_png_ptr->save_buffer)
      {
         ni_png_size_t i,istop;
         ni_png_bytep sp;
         ni_png_bytep dp;

         istop = ni_png_ptr->save_buffer_size;
         for (i = 0, sp = ni_png_ptr->save_buffer_ptr, dp = ni_png_ptr->save_buffer;
            i < istop; i++, sp++, dp++)
         {
            *dp = *sp;
         }
      }
   }
   if (ni_png_ptr->save_buffer_size + ni_png_ptr->current_buffer_size >
      ni_png_ptr->save_buffer_max)
   {
      ni_png_size_t new_max;
      ni_png_bytep old_buffer;

      if (ni_png_ptr->save_buffer_size > PNG_SIZE_MAX -
         (ni_png_ptr->current_buffer_size + 256))
      {
        ni_png_error(ni_png_ptr, "Potential overflow of save_buffer");
      }
      new_max = ni_png_ptr->save_buffer_size + ni_png_ptr->current_buffer_size + 256;
      old_buffer = ni_png_ptr->save_buffer;
      ni_png_ptr->save_buffer = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
         (ni_png_uint_32)new_max);
      ni_png_memcpy(ni_png_ptr->save_buffer, old_buffer, ni_png_ptr->save_buffer_size);
      ni_png_free(ni_png_ptr, old_buffer);
      ni_png_ptr->save_buffer_max = new_max;
   }
   if (ni_png_ptr->current_buffer_size)
   {
      ni_png_memcpy(ni_png_ptr->save_buffer + ni_png_ptr->save_buffer_size,
         ni_png_ptr->current_buffer_ptr, ni_png_ptr->current_buffer_size);
      ni_png_ptr->save_buffer_size += ni_png_ptr->current_buffer_size;
      ni_png_ptr->current_buffer_size = 0;
   }
   ni_png_ptr->save_buffer_ptr = ni_png_ptr->save_buffer;
   ni_png_ptr->buffer_size = 0;
}

void /* PRIVATE */
ni_png_push_restore_buffer(ni_png_structp ni_png_ptr, ni_png_bytep buffer,
   ni_png_size_t buffer_length)
{
   ni_png_ptr->current_buffer = buffer;
   ni_png_ptr->current_buffer_size = buffer_length;
   ni_png_ptr->buffer_size = buffer_length + ni_png_ptr->save_buffer_size;
   ni_png_ptr->current_buffer_ptr = ni_png_ptr->current_buffer;
}

void /* PRIVATE */
ni_png_push_read_IDAT(ni_png_structp ni_png_ptr)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_IDAT;
#endif
   if (!(ni_png_ptr->mode & PNG_HAVE_CHUNK_HEADER))
   {
      ni_png_byte chunk_length[4];

      if (ni_png_ptr->buffer_size < 8)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }

      ni_png_push_fill_buffer(ni_png_ptr, chunk_length, 4);
      ni_png_ptr->push_length = ni_png_get_uint_31(ni_png_ptr,chunk_length);
      ni_png_reset_crc(ni_png_ptr);
      ni_png_crc_read(ni_png_ptr, ni_png_ptr->chunk_name, 4);
      ni_png_ptr->mode |= PNG_HAVE_CHUNK_HEADER;

      if (ni_png_memcmp(ni_png_ptr->chunk_name, (ni_png_bytep)ni_png_IDAT, 4))
      {
         ni_png_ptr->process_mode = PNG_READ_CHUNK_MODE;
         if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_FINISHED))
            ni_png_error(ni_png_ptr, "Not enough compressed data");
         return;
      }

      ni_png_ptr->idat_size = ni_png_ptr->push_length;
   }
   if (ni_png_ptr->idat_size && ni_png_ptr->save_buffer_size)
   {
      ni_png_size_t save_size;

      if (ni_png_ptr->idat_size < (ni_png_uint_32)ni_png_ptr->save_buffer_size)
      {
         save_size = (ni_png_size_t)ni_png_ptr->idat_size;
         /* check for overflow */
         if((ni_png_uint_32)save_size != ni_png_ptr->idat_size)
            ni_png_error(ni_png_ptr, "save_size overflowed in pngpread");
      }
      else
         save_size = ni_png_ptr->save_buffer_size;

      ni_png_calculate_crc(ni_png_ptr, ni_png_ptr->save_buffer_ptr, save_size);
      if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_FINISHED))
         ni_png_process_IDAT_data(ni_png_ptr, ni_png_ptr->save_buffer_ptr, save_size);
      ni_png_ptr->idat_size -= save_size;
      ni_png_ptr->buffer_size -= save_size;
      ni_png_ptr->save_buffer_size -= save_size;
      ni_png_ptr->save_buffer_ptr += save_size;
   }
   if (ni_png_ptr->idat_size && ni_png_ptr->current_buffer_size)
   {
      ni_png_size_t save_size;

      if (ni_png_ptr->idat_size < (ni_png_uint_32)ni_png_ptr->current_buffer_size)
      {
         save_size = (ni_png_size_t)ni_png_ptr->idat_size;
         /* check for overflow */
         if((ni_png_uint_32)save_size != ni_png_ptr->idat_size)
            ni_png_error(ni_png_ptr, "save_size overflowed in pngpread");
      }
      else
         save_size = ni_png_ptr->current_buffer_size;

      ni_png_calculate_crc(ni_png_ptr, ni_png_ptr->current_buffer_ptr, save_size);
      if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_FINISHED))
        ni_png_process_IDAT_data(ni_png_ptr, ni_png_ptr->current_buffer_ptr, save_size);

      ni_png_ptr->idat_size -= save_size;
      ni_png_ptr->buffer_size -= save_size;
      ni_png_ptr->current_buffer_size -= save_size;
      ni_png_ptr->current_buffer_ptr += save_size;
   }
   if (!ni_png_ptr->idat_size)
   {
      if (ni_png_ptr->buffer_size < 4)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }

      ni_png_crc_finish(ni_png_ptr, 0);
      ni_png_ptr->mode &= ~PNG_HAVE_CHUNK_HEADER;
      ni_png_ptr->mode |= PNG_AFTER_IDAT;
   }
}

void /* PRIVATE */
ni_png_process_IDAT_data(ni_png_structp ni_png_ptr, ni_png_bytep buffer,
   ni_png_size_t buffer_length)
{
   int ret;

   if ((ni_png_ptr->flags & PNG_FLAG_ZLIB_FINISHED) && buffer_length)
      ni_png_error(ni_png_ptr, "Extra compression data");

   ni_png_ptr->zstream.next_in = buffer;
   ni_png_ptr->zstream.avail_in = (uInt)buffer_length;
   for(;;)
   {
      ret = inflate(&ni_png_ptr->zstream, Z_PARTIAL_FLUSH);
      if (ret != Z_OK)
      {
         if (ret == Z_STREAM_END)
         {
            if (ni_png_ptr->zstream.avail_in)
               ni_png_error(ni_png_ptr, "Extra compressed data");
            if (!(ni_png_ptr->zstream.avail_out))
            {
               ni_png_push_process_row(ni_png_ptr);
            }

            ni_png_ptr->mode |= PNG_AFTER_IDAT;
            ni_png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
            break;
         }
         else if (ret == Z_BUF_ERROR)
            break;
         else
            ni_png_error(ni_png_ptr, "Decompression Error");
      }
      if (!(ni_png_ptr->zstream.avail_out))
      {
         if ((
#if defined(PNG_READ_INTERLACING_SUPPORTED)
             ni_png_ptr->interlaced && ni_png_ptr->pass > 6) ||
             (!ni_png_ptr->interlaced &&
#endif
             ni_png_ptr->row_number == ni_png_ptr->num_rows))
         {
           if (ni_png_ptr->zstream.avail_in)
             ni_png_warning(ni_png_ptr, "Too much data in IDAT chunks");
           ni_png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
           break;
         }
         ni_png_push_process_row(ni_png_ptr);
         ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->irowbytes;
         ni_png_ptr->zstream.next_out = ni_png_ptr->row_buf;
      }
      else
         break;
   }
}

void /* PRIVATE */
ni_png_push_process_row(ni_png_structp ni_png_ptr)
{
   ni_png_ptr->row_info.color_type = ni_png_ptr->color_type;
   ni_png_ptr->row_info.width = ni_png_ptr->iwidth;
   ni_png_ptr->row_info.channels = ni_png_ptr->channels;
   ni_png_ptr->row_info.bit_depth = ni_png_ptr->bit_depth;
   ni_png_ptr->row_info.pixel_depth = ni_png_ptr->pixel_depth;

   ni_png_ptr->row_info.rowbytes = PNG_ROWBYTES(ni_png_ptr->row_info.pixel_depth,
       ni_png_ptr->row_info.width);

   ni_png_read_filter_row(ni_png_ptr, &(ni_png_ptr->row_info),
      ni_png_ptr->row_buf + 1, ni_png_ptr->prev_row + 1,
      (int)(ni_png_ptr->row_buf[0]));

   ni_png_memcpy_check(ni_png_ptr, ni_png_ptr->prev_row, ni_png_ptr->row_buf,
      ni_png_ptr->rowbytes + 1);

   if (ni_png_ptr->transformations || (ni_png_ptr->flags&PNG_FLAG_STRIP_ALPHA))
      ni_png_do_read_transformations(ni_png_ptr);

#if defined(PNG_READ_INTERLACING_SUPPORTED)
   /* blow up interlaced rows to full size */
   if (ni_png_ptr->interlaced && (ni_png_ptr->transformations & PNG_INTERLACE))
   {
      if (ni_png_ptr->pass < 6)
/*       old interface (pre-1.0.9):
         ni_png_do_read_interlace(&(ni_png_ptr->row_info),
            ni_png_ptr->row_buf + 1, ni_png_ptr->pass, ni_png_ptr->transformations);
 */
         ni_png_do_read_interlace(ni_png_ptr);

    switch (ni_png_ptr->pass)
    {
         case 0:
         {
            int i;
            for (i = 0; i < 8 && ni_png_ptr->pass == 0; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
               ni_png_read_push_finish_row(ni_png_ptr); /* updates ni_png_ptr->pass */
            }
            if (ni_png_ptr->pass == 2) /* pass 1 might be empty */
            {
               for (i = 0; i < 4 && ni_png_ptr->pass == 2; i++)
               {
                  ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
                  ni_png_read_push_finish_row(ni_png_ptr);
               }
            }
            if (ni_png_ptr->pass == 4 && ni_png_ptr->height <= 4)
            {
               for (i = 0; i < 2 && ni_png_ptr->pass == 4; i++)
               {
                  ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
                  ni_png_read_push_finish_row(ni_png_ptr);
               }
            }
            if (ni_png_ptr->pass == 6 && ni_png_ptr->height <= 4)
            {
                ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
                ni_png_read_push_finish_row(ni_png_ptr);
            }
            break;
         }
         case 1:
         {
            int i;
            for (i = 0; i < 8 && ni_png_ptr->pass == 1; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            if (ni_png_ptr->pass == 2) /* skip top 4 generated rows */
            {
               for (i = 0; i < 4 && ni_png_ptr->pass == 2; i++)
               {
                  ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
                  ni_png_read_push_finish_row(ni_png_ptr);
               }
            }
            break;
         }
         case 2:
         {
            int i;
            for (i = 0; i < 4 && ni_png_ptr->pass == 2; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            for (i = 0; i < 4 && ni_png_ptr->pass == 2; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            if (ni_png_ptr->pass == 4) /* pass 3 might be empty */
            {
               for (i = 0; i < 2 && ni_png_ptr->pass == 4; i++)
               {
                  ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
                  ni_png_read_push_finish_row(ni_png_ptr);
               }
            }
            break;
         }
         case 3:
         {
            int i;
            for (i = 0; i < 4 && ni_png_ptr->pass == 3; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            if (ni_png_ptr->pass == 4) /* skip top two generated rows */
            {
               for (i = 0; i < 2 && ni_png_ptr->pass == 4; i++)
               {
                  ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
                  ni_png_read_push_finish_row(ni_png_ptr);
               }
            }
            break;
         }
         case 4:
         {
            int i;
            for (i = 0; i < 2 && ni_png_ptr->pass == 4; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            for (i = 0; i < 2 && ni_png_ptr->pass == 4; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            if (ni_png_ptr->pass == 6) /* pass 5 might be empty */
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            break;
         }
         case 5:
         {
            int i;
            for (i = 0; i < 2 && ni_png_ptr->pass == 5; i++)
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            if (ni_png_ptr->pass == 6) /* skip top generated row */
            {
               ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
               ni_png_read_push_finish_row(ni_png_ptr);
            }
            break;
         }
         case 6:
         {
            ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
            ni_png_read_push_finish_row(ni_png_ptr);
            if (ni_png_ptr->pass != 6)
               break;
            ni_png_push_have_row(ni_png_ptr, ni_png_bytep_NULL);
            ni_png_read_push_finish_row(ni_png_ptr);
         }
      }
   }
   else
#endif
   {
      ni_png_push_have_row(ni_png_ptr, ni_png_ptr->row_buf + 1);
      ni_png_read_push_finish_row(ni_png_ptr);
   }
}

void /* PRIVATE */
ni_png_read_push_finish_row(ni_png_structp ni_png_ptr)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   /* arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* start of interlace block */
   const int FARDATA ni_png_pass_start[] = {0, 4, 0, 2, 0, 1, 0};

   /* offset to next interlace block */
   const int FARDATA ni_png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1};

   /* start of interlace block in the y direction */
   const int FARDATA ni_png_pass_ystart[] = {0, 0, 4, 0, 2, 0, 1};

   /* offset to next interlace block in the y direction */
   const int FARDATA ni_png_pass_yinc[] = {8, 8, 8, 4, 4, 2, 2};

   /* Width of interlace block.  This is not currently used - if you need
    * it, uncomment it here and in png.h
   const int FARDATA ni_png_pass_width[] = {8, 4, 4, 2, 2, 1, 1};
   */

   /* Height of interlace block.  This is not currently used - if you need
    * it, uncomment it here and in png.h
   const int FARDATA ni_png_pass_height[] = {8, 8, 4, 4, 2, 2, 1};
   */
#endif

   ni_png_ptr->row_number++;
   if (ni_png_ptr->row_number < ni_png_ptr->num_rows)
      return;

   if (ni_png_ptr->interlaced)
   {
      ni_png_ptr->row_number = 0;
      ni_png_memset_check(ni_png_ptr, ni_png_ptr->prev_row, 0,
         ni_png_ptr->rowbytes + 1);
      do
      {
         ni_png_ptr->pass++;
         if ((ni_png_ptr->pass == 1 && ni_png_ptr->width < 5) ||
             (ni_png_ptr->pass == 3 && ni_png_ptr->width < 3) ||
             (ni_png_ptr->pass == 5 && ni_png_ptr->width < 2))
           ni_png_ptr->pass++;

         if (ni_png_ptr->pass > 7)
            ni_png_ptr->pass--;
         if (ni_png_ptr->pass >= 7)
            break;

         ni_png_ptr->iwidth = (ni_png_ptr->width +
            ni_png_pass_inc[ni_png_ptr->pass] - 1 -
            ni_png_pass_start[ni_png_ptr->pass]) /
            ni_png_pass_inc[ni_png_ptr->pass];

         ni_png_ptr->irowbytes = PNG_ROWBYTES(ni_png_ptr->pixel_depth,
            ni_png_ptr->iwidth) + 1;

         if (ni_png_ptr->transformations & PNG_INTERLACE)
            break;

         ni_png_ptr->num_rows = (ni_png_ptr->height +
            ni_png_pass_yinc[ni_png_ptr->pass] - 1 -
            ni_png_pass_ystart[ni_png_ptr->pass]) /
            ni_png_pass_yinc[ni_png_ptr->pass];

      } while (ni_png_ptr->iwidth == 0 || ni_png_ptr->num_rows == 0);
   }
}

#if defined(PNG_READ_tEXt_SUPPORTED)
void /* PRIVATE */
ni_png_push_handle_tEXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32
   length)
{
   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR) || (ni_png_ptr->mode & PNG_HAVE_IEND))
      {
         ni_png_error(ni_png_ptr, "Out of place tEXt");
         /* to quiet some compiler warnings */
         if(info_ptr == NULL) return;
      }

#ifdef PNG_MAX_MALLOC_64K
   ni_png_ptr->skip_length = 0;  /* This may not be necessary */

   if (length > (ni_png_uint_32)65535L) /* Can't hold entire string in memory */
   {
      ni_png_warning(ni_png_ptr, "tEXt chunk too large to fit in memory");
      ni_png_ptr->skip_length = length - (ni_png_uint_32)65535L;
      length = (ni_png_uint_32)65535L;
   }
#endif

   ni_png_ptr->current_text = (ni_png_charp)ni_png_malloc(ni_png_ptr,
         (ni_png_uint_32)(length+1));
   ni_png_ptr->current_text[length] = '\0';
   ni_png_ptr->current_text_ptr = ni_png_ptr->current_text;
   ni_png_ptr->current_text_size = (ni_png_size_t)length;
   ni_png_ptr->current_text_left = (ni_png_size_t)length;
   ni_png_ptr->process_mode = PNG_READ_tEXt_MODE;
}

void /* PRIVATE */
ni_png_push_read_tEXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   if (ni_png_ptr->buffer_size && ni_png_ptr->current_text_left)
   {
      ni_png_size_t text_size;

      if (ni_png_ptr->buffer_size < ni_png_ptr->current_text_left)
         text_size = ni_png_ptr->buffer_size;
      else
         text_size = ni_png_ptr->current_text_left;
      ni_png_crc_read(ni_png_ptr, (ni_png_bytep)ni_png_ptr->current_text_ptr, text_size);
      ni_png_ptr->current_text_left -= text_size;
      ni_png_ptr->current_text_ptr += text_size;
   }
   if (!(ni_png_ptr->current_text_left))
   {
      ni_png_textp text_ptr;
      ni_png_charp text;
      ni_png_charp key;
      int ret;

      if (ni_png_ptr->buffer_size < 4)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }

      ni_png_push_crc_finish(ni_png_ptr);

#if defined(PNG_MAX_MALLOC_64K)
      if (ni_png_ptr->skip_length)
         return;
#endif

      key = ni_png_ptr->current_text;

      for (text = key; *text; text++)
         /* empty loop */ ;

      if (text != key + ni_png_ptr->current_text_size)
         text++;

      text_ptr = (ni_png_textp)ni_png_malloc(ni_png_ptr,
         (ni_png_uint_32)ni_png_sizeof(ni_png_text));
      text_ptr->compression = PNG_TEXT_COMPRESSION_NONE;
      text_ptr->key = key;
#ifdef PNG_iTXt_SUPPORTED
      text_ptr->lang = NULL;
      text_ptr->lang_key = NULL;
#endif
      text_ptr->text = text;

      ret = ni_png_set_text_2(ni_png_ptr, info_ptr, text_ptr, 1);

      ni_png_free(ni_png_ptr, key);
      ni_png_free(ni_png_ptr, text_ptr);
      ni_png_ptr->current_text = NULL;

      if (ret)
        ni_png_warning(ni_png_ptr, "Insufficient memory to store text chunk.");
   }
}
#endif

#if defined(PNG_READ_zTXt_SUPPORTED)
void /* PRIVATE */
ni_png_push_handle_zTXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32
   length)
{
   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR) || (ni_png_ptr->mode & PNG_HAVE_IEND))
      {
         ni_png_error(ni_png_ptr, "Out of place zTXt");
         /* to quiet some compiler warnings */
         if(info_ptr == NULL) return;
      }

#ifdef PNG_MAX_MALLOC_64K
   /* We can't handle zTXt chunks > 64K, since we don't have enough space
    * to be able to store the uncompressed data.  Actually, the threshold
    * is probably around 32K, but it isn't as definite as 64K is.
    */
   if (length > (ni_png_uint_32)65535L)
   {
      ni_png_warning(ni_png_ptr, "zTXt chunk too large to fit in memory");
      ni_png_push_crc_skip(ni_png_ptr, length);
      return;
   }
#endif

   ni_png_ptr->current_text = (ni_png_charp)ni_png_malloc(ni_png_ptr,
       (ni_png_uint_32)(length+1));
   ni_png_ptr->current_text[length] = '\0';
   ni_png_ptr->current_text_ptr = ni_png_ptr->current_text;
   ni_png_ptr->current_text_size = (ni_png_size_t)length;
   ni_png_ptr->current_text_left = (ni_png_size_t)length;
   ni_png_ptr->process_mode = PNG_READ_zTXt_MODE;
}

void /* PRIVATE */
ni_png_push_read_zTXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   if (ni_png_ptr->buffer_size && ni_png_ptr->current_text_left)
   {
      ni_png_size_t text_size;

      if (ni_png_ptr->buffer_size < (ni_png_uint_32)ni_png_ptr->current_text_left)
         text_size = ni_png_ptr->buffer_size;
      else
         text_size = ni_png_ptr->current_text_left;
      ni_png_crc_read(ni_png_ptr, (ni_png_bytep)ni_png_ptr->current_text_ptr, text_size);
      ni_png_ptr->current_text_left -= text_size;
      ni_png_ptr->current_text_ptr += text_size;
   }
   if (!(ni_png_ptr->current_text_left))
   {
      ni_png_textp text_ptr;
      ni_png_charp text;
      ni_png_charp key;
      int ret;
      ni_png_size_t text_size, key_size;

      if (ni_png_ptr->buffer_size < 4)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }

      ni_png_push_crc_finish(ni_png_ptr);

      key = ni_png_ptr->current_text;

      for (text = key; *text; text++)
         /* empty loop */ ;

      /* zTXt can't have zero text */
      if (text == key + ni_png_ptr->current_text_size)
      {
         ni_png_ptr->current_text = NULL;
         ni_png_free(ni_png_ptr, key);
         return;
      }

      text++;

      if (*text != PNG_TEXT_COMPRESSION_zTXt) /* check compression byte */
      {
         ni_png_ptr->current_text = NULL;
         ni_png_free(ni_png_ptr, key);
         return;
      }

      text++;

      ni_png_ptr->zstream.next_in = (ni_png_bytep )text;
      ni_png_ptr->zstream.avail_in = (uInt)(ni_png_ptr->current_text_size -
         (text - key));
      ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
      ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;

      key_size = text - key;
      text_size = 0;
      text = NULL;
      ret = Z_STREAM_END;

      while (ni_png_ptr->zstream.avail_in)
      {
         ret = inflate(&ni_png_ptr->zstream, Z_PARTIAL_FLUSH);
         if (ret != Z_OK && ret != Z_STREAM_END)
         {
            inflateReset(&ni_png_ptr->zstream);
            ni_png_ptr->zstream.avail_in = 0;
            ni_png_ptr->current_text = NULL;
            ni_png_free(ni_png_ptr, key);
            ni_png_free(ni_png_ptr, text);
            return;
         }
         if (!(ni_png_ptr->zstream.avail_out) || ret == Z_STREAM_END)
         {
            if (text == NULL)
            {
               text = (ni_png_charp)ni_png_malloc(ni_png_ptr,
                  (ni_png_uint_32)(ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out
                     + key_size + 1));
               ni_png_memcpy(text + key_size, ni_png_ptr->zbuf,
                  ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out);
               ni_png_memcpy(text, key, key_size);
               text_size = key_size + ni_png_ptr->zbuf_size -
                  ni_png_ptr->zstream.avail_out;
               *(text + text_size) = '\0';
            }
            else
            {
               ni_png_charp tmp;

               tmp = text;
               text = (ni_png_charp)ni_png_malloc(ni_png_ptr, text_size +
                  (ni_png_uint_32)(ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out
                   + 1));
               ni_png_memcpy(text, tmp, text_size);
               ni_png_free(ni_png_ptr, tmp);
               ni_png_memcpy(text + text_size, ni_png_ptr->zbuf,
                  ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out);
               text_size += ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out;
               *(text + text_size) = '\0';
            }
            if (ret != Z_STREAM_END)
            {
               ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
               ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
            }
         }
         else
         {
            break;
         }

         if (ret == Z_STREAM_END)
            break;
      }

      inflateReset(&ni_png_ptr->zstream);
      ni_png_ptr->zstream.avail_in = 0;

      if (ret != Z_STREAM_END)
      {
         ni_png_ptr->current_text = NULL;
         ni_png_free(ni_png_ptr, key);
         ni_png_free(ni_png_ptr, text);
         return;
      }

      ni_png_ptr->current_text = NULL;
      ni_png_free(ni_png_ptr, key);
      key = text;
      text += key_size;

      text_ptr = (ni_png_textp)ni_png_malloc(ni_png_ptr,
          (ni_png_uint_32)ni_png_sizeof(ni_png_text));
      text_ptr->compression = PNG_TEXT_COMPRESSION_zTXt;
      text_ptr->key = key;
#ifdef PNG_iTXt_SUPPORTED
      text_ptr->lang = NULL;
      text_ptr->lang_key = NULL;
#endif
      text_ptr->text = text;

      ret = ni_png_set_text_2(ni_png_ptr, info_ptr, text_ptr, 1);

      ni_png_free(ni_png_ptr, key);
      ni_png_free(ni_png_ptr, text_ptr);

      if (ret)
        ni_png_warning(ni_png_ptr, "Insufficient memory to store text chunk.");
   }
}
#endif

#if defined(PNG_READ_iTXt_SUPPORTED)
void /* PRIVATE */
ni_png_push_handle_iTXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32
   length)
{
   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR) || (ni_png_ptr->mode & PNG_HAVE_IEND))
      {
         ni_png_error(ni_png_ptr, "Out of place iTXt");
         /* to quiet some compiler warnings */
         if(info_ptr == NULL) return;
      }

#ifdef PNG_MAX_MALLOC_64K
   ni_png_ptr->skip_length = 0;  /* This may not be necessary */

   if (length > (ni_png_uint_32)65535L) /* Can't hold entire string in memory */
   {
      ni_png_warning(ni_png_ptr, "iTXt chunk too large to fit in memory");
      ni_png_ptr->skip_length = length - (ni_png_uint_32)65535L;
      length = (ni_png_uint_32)65535L;
   }
#endif

   ni_png_ptr->current_text = (ni_png_charp)ni_png_malloc(ni_png_ptr,
         (ni_png_uint_32)(length+1));
   ni_png_ptr->current_text[length] = '\0';
   ni_png_ptr->current_text_ptr = ni_png_ptr->current_text;
   ni_png_ptr->current_text_size = (ni_png_size_t)length;
   ni_png_ptr->current_text_left = (ni_png_size_t)length;
   ni_png_ptr->process_mode = PNG_READ_iTXt_MODE;
}

void /* PRIVATE */
ni_png_push_read_iTXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{

   if (ni_png_ptr->buffer_size && ni_png_ptr->current_text_left)
   {
      ni_png_size_t text_size;

      if (ni_png_ptr->buffer_size < ni_png_ptr->current_text_left)
         text_size = ni_png_ptr->buffer_size;
      else
         text_size = ni_png_ptr->current_text_left;
      ni_png_crc_read(ni_png_ptr, (ni_png_bytep)ni_png_ptr->current_text_ptr, text_size);
      ni_png_ptr->current_text_left -= text_size;
      ni_png_ptr->current_text_ptr += text_size;
   }
   if (!(ni_png_ptr->current_text_left))
   {
      ni_png_textp text_ptr;
      ni_png_charp key;
      int comp_flag;
      ni_png_charp lang;
      ni_png_charp lang_key;
      ni_png_charp text;
      int ret;

      if (ni_png_ptr->buffer_size < 4)
      {
         ni_png_push_save_buffer(ni_png_ptr);
         return;
      }

      ni_png_push_crc_finish(ni_png_ptr);

#if defined(PNG_MAX_MALLOC_64K)
      if (ni_png_ptr->skip_length)
         return;
#endif

      key = ni_png_ptr->current_text;

      for (lang = key; *lang; lang++)
         /* empty loop */ ;

      if (lang != key + ni_png_ptr->current_text_size)
         lang++;

      comp_flag = *lang++;
      lang++;     /* skip comp_type, always zero */

      for (lang_key = lang; *lang_key; lang_key++)
         /* empty loop */ ;
      lang_key++;        /* skip NUL separator */

      for (text = lang_key; *text; text++)
         /* empty loop */ ;

      if (text != key + ni_png_ptr->current_text_size)
         text++;

      text_ptr = (ni_png_textp)ni_png_malloc(ni_png_ptr,
         (ni_png_uint_32)ni_png_sizeof(ni_png_text));
      text_ptr->compression = comp_flag + 2;
      text_ptr->key = key;
      text_ptr->lang = lang;
      text_ptr->lang_key = lang_key;
      text_ptr->text = text;
      text_ptr->text_length = 0;
      text_ptr->itxt_length = ni_png_strlen(text);

      ret = ni_png_set_text_2(ni_png_ptr, info_ptr, text_ptr, 1);

      ni_png_ptr->current_text = NULL;

      ni_png_free(ni_png_ptr, text_ptr);
      if (ret)
        ni_png_warning(ni_png_ptr, "Insufficient memory to store iTXt chunk.");
   }
}
#endif

/* This function is called when we haven't found a handler for this
 * chunk.  If there isn't a problem with the chunk itself (ie a bad chunk
 * name or a critical chunk), the chunk is (currently) silently ignored.
 */
void /* PRIVATE */
ni_png_push_handle_unknown(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32
   length)
{
   ni_png_uint_32 skip=0;
   ni_png_check_chunk_name(ni_png_ptr, ni_png_ptr->chunk_name);

   if (!(ni_png_ptr->chunk_name[0] & 0x20))
   {
#if defined(PNG_READ_UNKNOWN_CHUNKS_SUPPORTED)
      if(ni_png_handle_as_unknown(ni_png_ptr, ni_png_ptr->chunk_name) !=
           PNG_HANDLE_CHUNK_ALWAYS
#if defined(PNG_READ_USER_CHUNKS_SUPPORTED)
           && ni_png_ptr->read_user_chunk_fn == NULL
#endif
         )
#endif
         ni_png_chunk_error(ni_png_ptr, "unknown critical chunk");

      /* to quiet compiler warnings about unused info_ptr */
      if (info_ptr == NULL)
         return;
   }

#if defined(PNG_READ_UNKNOWN_CHUNKS_SUPPORTED)
   if (ni_png_ptr->flags & PNG_FLAG_KEEP_UNKNOWN_CHUNKS)
   {
#ifdef PNG_MAX_MALLOC_64K
       if (length > (ni_png_uint_32)65535L)
       {
           ni_png_warning(ni_png_ptr, "unknown chunk too large to fit in memory");
           skip = length - (ni_png_uint_32)65535L;
           length = (ni_png_uint_32)65535L;
       }
#endif
       ni_png_strcpy((ni_png_charp)ni_png_ptr->unknown_chunk.name,
         (ni_png_charp)ni_png_ptr->chunk_name);
       ni_png_ptr->unknown_chunk.data = (ni_png_bytep)ni_png_malloc(ni_png_ptr, length);
       ni_png_ptr->unknown_chunk.size = (ni_png_size_t)length;
       ni_png_crc_read(ni_png_ptr, (ni_png_bytep)ni_png_ptr->unknown_chunk.data, length);
#if defined(PNG_READ_USER_CHUNKS_SUPPORTED)
       if(ni_png_ptr->read_user_chunk_fn != NULL)
       {
          /* callback to user unknown chunk handler */
          int ret;
          ret = (*(ni_png_ptr->read_user_chunk_fn))
            (ni_png_ptr, &ni_png_ptr->unknown_chunk);
          if (ret < 0)
             ni_png_chunk_error(ni_png_ptr, "error in user chunk");
          if (ret == 0)
          {
             if (!(ni_png_ptr->chunk_name[0] & 0x20))
                if(ni_png_handle_as_unknown(ni_png_ptr, ni_png_ptr->chunk_name) !=
                     PNG_HANDLE_CHUNK_ALWAYS)
                   ni_png_chunk_error(ni_png_ptr, "unknown critical chunk");
                ni_png_set_unknown_chunks(ni_png_ptr, info_ptr,
                   &ni_png_ptr->unknown_chunk, 1);
          }
       }
#else
       ni_png_set_unknown_chunks(ni_png_ptr, info_ptr, &ni_png_ptr->unknown_chunk, 1);
#endif
       ni_png_free(ni_png_ptr, ni_png_ptr->unknown_chunk.data);
       ni_png_ptr->unknown_chunk.data = NULL;
   }
   else
#endif
      skip=length;
   ni_png_push_crc_skip(ni_png_ptr, skip);
}

void /* PRIVATE */
ni_png_push_have_info(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   if (ni_png_ptr->info_fn != NULL)
      (*(ni_png_ptr->info_fn))(ni_png_ptr, info_ptr);
}

void /* PRIVATE */
ni_png_push_have_end(ni_png_structp ni_png_ptr, ni_png_infop info_ptr)
{
   if (ni_png_ptr->end_fn != NULL)
      (*(ni_png_ptr->end_fn))(ni_png_ptr, info_ptr);
}

void /* PRIVATE */
ni_png_push_have_row(ni_png_structp ni_png_ptr, ni_png_bytep row)
{
   if (ni_png_ptr->row_fn != NULL)
      (*(ni_png_ptr->row_fn))(ni_png_ptr, row, ni_png_ptr->row_number,
         (int)ni_png_ptr->pass);
}

void PNGAPI
ni_png_progressive_combine_row (ni_png_structp ni_png_ptr,
   ni_png_bytep old_row, ni_png_bytep new_row)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   const int FARDATA ni_png_pass_dsp_mask[7] =
      {0xff, 0x0f, 0xff, 0x33, 0xff, 0x55, 0xff};
#endif
   if(ni_png_ptr == NULL) return;
   if (new_row != NULL)    /* new_row must == ni_png_ptr->row_buf here. */
      ni_png_combine_row(ni_png_ptr, old_row, ni_png_pass_dsp_mask[ni_png_ptr->pass]);
}

void PNGAPI
ni_png_set_progressive_read_fn(ni_png_structp ni_png_ptr, ni_png_voidp progressive_ptr,
   ni_png_progressive_info_ptr info_fn, ni_png_progressive_row_ptr row_fn,
   ni_png_progressive_end_ptr end_fn)
{
   if(ni_png_ptr == NULL) return;
   ni_png_ptr->info_fn = info_fn;
   ni_png_ptr->row_fn = row_fn;
   ni_png_ptr->end_fn = end_fn;

   ni_png_set_read_fn(ni_png_ptr, progressive_ptr, ni_png_push_fill_buffer);
}

ni_png_voidp PNGAPI
ni_png_get_progressive_ptr(ni_png_structp ni_png_ptr)
{
   if(ni_png_ptr == NULL) return (NULL);
   return ni_png_ptr->io_ptr;
}
#endif /* PNG_PROGRESSIVE_READ_SUPPORTED */
