
/* pngrutil.c - utilities to read a PNG file
 *
 * Last changed in libpng 1.2.17 May 15, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This file contains routines that are only called from within
 * libpng itself during the course of reading an image.
 */

#define PNG_INTERNAL
#include "png.h"

#if defined(PNG_READ_SUPPORTED)

#ifdef PNG_FLOATING_POINT_SUPPORTED
#  if defined(_WIN32_WCE)
/* strtod() function is not supported on WindowsCE */
__inline double ni_png_strtod(ni_png_structp ni_png_ptr, const char *nptr, char **endptr)
{
   double result = 0;
   int len;
   wchar_t *str, *end;

   len = MultiByteToWideChar(CP_ACP, 0, nptr, -1, NULL, 0);
   str = (wchar_t *)ni_png_malloc(ni_png_ptr, len * sizeof(wchar_t));
   if ( NULL != str )
   {
      MultiByteToWideChar(CP_ACP, 0, nptr, -1, str, len);
      result = wcstod(str, &end);
      len = WideCharToMultiByte(CP_ACP, 0, end, -1, NULL, 0, NULL, NULL);
      *endptr = (char *)nptr + (ni_png_strlen(nptr) - len + 1);
      ni_png_free(ni_png_ptr, str);
   }
   return result;
}
#  else
#    define ni_png_strtod(p,a,b) strtod(a,b)
#  endif
#endif

ni_png_uint_32 PNGAPI
ni_png_get_uint_31(ni_png_structp ni_png_ptr, ni_png_bytep buf)
{
   ni_png_uint_32 i = ni_png_get_uint_32(buf);
   if (i > PNG_UINT_31_MAX)
     ni_png_error(ni_png_ptr, "PNG unsigned integer out of range.");
   return (i);
}
#ifndef PNG_READ_BIG_ENDIAN_SUPPORTED
/* Grab an unsigned 32-bit integer from a buffer in big-endian format. */
ni_png_uint_32 PNGAPI
ni_png_get_uint_32(ni_png_bytep buf)
{
   ni_png_uint_32 i = ((ni_png_uint_32)(*buf) << 24) +
      ((ni_png_uint_32)(*(buf + 1)) << 16) +
      ((ni_png_uint_32)(*(buf + 2)) << 8) +
      (ni_png_uint_32)(*(buf + 3));

   return (i);
}

/* Grab a signed 32-bit integer from a buffer in big-endian format.  The
 * data is stored in the PNG file in two's complement format, and it is
 * assumed that the machine format for signed integers is the same. */
ni_png_int_32 PNGAPI
ni_png_get_int_32(ni_png_bytep buf)
{
   ni_png_int_32 i = ((ni_png_int_32)(*buf) << 24) +
      ((ni_png_int_32)(*(buf + 1)) << 16) +
      ((ni_png_int_32)(*(buf + 2)) << 8) +
      (ni_png_int_32)(*(buf + 3));

   return (i);
}

/* Grab an unsigned 16-bit integer from a buffer in big-endian format. */
ni_png_uint_16 PNGAPI
ni_png_get_uint_16(ni_png_bytep buf)
{
   ni_png_uint_16 i = (ni_png_uint_16)(((ni_png_uint_16)(*buf) << 8) +
      (ni_png_uint_16)(*(buf + 1)));

   return (i);
}
#endif /* PNG_READ_BIG_ENDIAN_SUPPORTED */

/* Read data, and (optionally) run it through the CRC. */
void /* PRIVATE */
ni_png_crc_read(ni_png_structp ni_png_ptr, ni_png_bytep buf, ni_png_size_t length)
{
   if(ni_png_ptr == NULL) return;
   ni_png_read_data(ni_png_ptr, buf, length);
   ni_png_calculate_crc(ni_png_ptr, buf, length);
}

/* Optionally skip data and then check the CRC.  Depending on whether we
   are reading a ancillary or critical chunk, and how the program has set
   things up, we may calculate the CRC on the data and print a message.
   Returns '1' if there was a CRC error, '0' otherwise. */
int /* PRIVATE */
ni_png_crc_finish(ni_png_structp ni_png_ptr, ni_png_uint_32 skip)
{
   ni_png_size_t i;
   ni_png_size_t istop = ni_png_ptr->zbuf_size;

   for (i = (ni_png_size_t)skip; i > istop; i -= istop)
   {
      ni_png_crc_read(ni_png_ptr, ni_png_ptr->zbuf, ni_png_ptr->zbuf_size);
   }
   if (i)
   {
      ni_png_crc_read(ni_png_ptr, ni_png_ptr->zbuf, i);
   }

   if (ni_png_crc_error(ni_png_ptr))
   {
      if (((ni_png_ptr->chunk_name[0] & 0x20) &&                /* Ancillary */
           !(ni_png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN)) ||
          (!(ni_png_ptr->chunk_name[0] & 0x20) &&             /* Critical  */
          (ni_png_ptr->flags & PNG_FLAG_CRC_CRITICAL_USE)))
      {
         ni_png_chunk_warning(ni_png_ptr, "CRC error");
      }
      else
      {
         ni_png_chunk_error(ni_png_ptr, "CRC error");
      }
      return (1);
   }

   return (0);
}

/* Compare the CRC stored in the PNG file with that calculated by libpng from
   the data it has read thus far. */
int /* PRIVATE */
ni_png_crc_error(ni_png_structp ni_png_ptr)
{
   ni_png_byte crc_bytes[4];
   ni_png_uint_32 crc;
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

   ni_png_read_data(ni_png_ptr, crc_bytes, 4);

   if (need_crc)
   {
      crc = ni_png_get_uint_32(crc_bytes);
      return ((int)(crc != ni_png_ptr->crc));
   }
   else
      return (0);
}

#if defined(PNG_READ_zTXt_SUPPORTED) || defined(PNG_READ_iTXt_SUPPORTED) || \
    defined(PNG_READ_iCCP_SUPPORTED)
/*
 * Decompress trailing data in a chunk.  The assumption is that chunkdata
 * points at an allocated area holding the contents of a chunk with a
 * trailing compressed part.  What we get back is an allocated area
 * holding the original prefix part and an uncompressed version of the
 * trailing part (the malloc area passed in is freed).
 */
ni_png_charp /* PRIVATE */
ni_png_decompress_chunk(ni_png_structp ni_png_ptr, int comp_type,
                              ni_png_charp chunkdata, ni_png_size_t chunklength,
                              ni_png_size_t prefix_size, ni_png_size_t *newlength)
{
   const static char msg[] = "Error decoding compressed text";
   ni_png_charp text;
   ni_png_size_t text_size;

   if (comp_type == PNG_COMPRESSION_TYPE_BASE)
   {
      int ret = Z_OK;
      ni_png_ptr->zstream.next_in = (ni_png_bytep)(chunkdata + prefix_size);
      ni_png_ptr->zstream.avail_in = (uInt)(chunklength - prefix_size);
      ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
      ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;

      text_size = 0;
      text = NULL;

      while (ni_png_ptr->zstream.avail_in)
      {
         ret = inflate(&ni_png_ptr->zstream, Z_PARTIAL_FLUSH);
         if (ret != Z_OK && ret != Z_STREAM_END)
         {
            if (ni_png_ptr->zstream.msg != NULL)
               ni_png_warning(ni_png_ptr, ni_png_ptr->zstream.msg);
            else
               ni_png_warning(ni_png_ptr, msg);
            inflateReset(&ni_png_ptr->zstream);
            ni_png_ptr->zstream.avail_in = 0;

            if (text ==  NULL)
            {
               text_size = prefix_size + ni_png_sizeof(msg) + 1;
               text = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, text_size);
               if (text ==  NULL)
                 {
                    ni_png_free(ni_png_ptr,chunkdata);
                    ni_png_error(ni_png_ptr,"Not enough memory to decompress chunk");
                 }
               ni_png_memcpy(text, chunkdata, prefix_size);
            }

            text[text_size - 1] = 0x00;

            /* Copy what we can of the error message into the text chunk */
            text_size = (ni_png_size_t)(chunklength - (text - chunkdata) - 1);
            text_size = ni_png_sizeof(msg) > text_size ? text_size :
               ni_png_sizeof(msg);
            ni_png_memcpy(text + prefix_size, msg, text_size + 1);
            break;
         }
         if (!ni_png_ptr->zstream.avail_out || ret == Z_STREAM_END)
         {
            if (text == NULL)
            {
               text_size = prefix_size +
                   ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out;
               text = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, text_size + 1);
               if (text ==  NULL)
                 {
                    ni_png_free(ni_png_ptr,chunkdata);
                    ni_png_error(ni_png_ptr,"Not enough memory to decompress chunk.");
                 }
               ni_png_memcpy(text + prefix_size, ni_png_ptr->zbuf,
                    text_size - prefix_size);
               ni_png_memcpy(text, chunkdata, prefix_size);
               *(text + text_size) = 0x00;
            }
            else
            {
               ni_png_charp tmp;

               tmp = text;
               text = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr,
                  (ni_png_uint_32)(text_size +
                  ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out + 1));
               if (text == NULL)
               {
                  ni_png_free(ni_png_ptr, tmp);
                  ni_png_free(ni_png_ptr, chunkdata);
                  ni_png_error(ni_png_ptr,"Not enough memory to decompress chunk..");
               }
               ni_png_memcpy(text, tmp, text_size);
               ni_png_free(ni_png_ptr, tmp);
               ni_png_memcpy(text + text_size, ni_png_ptr->zbuf,
                  (ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out));
               text_size += ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out;
               *(text + text_size) = 0x00;
            }
            if (ret == Z_STREAM_END)
               break;
            else
            {
               ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
               ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
            }
         }
      }
      if (ret != Z_STREAM_END)
      {
#if !defined(PNG_NO_STDIO) && !defined(_WIN32_WCE)
         char umsg[52];

         if (ret == Z_BUF_ERROR)
            sprintf(umsg,"Buffer error in compressed datastream in %s chunk",
                ni_png_ptr->chunk_name);
         else if (ret == Z_DATA_ERROR)
            sprintf(umsg,"Data error in compressed datastream in %s chunk",
                ni_png_ptr->chunk_name);
         else
            sprintf(umsg,"Incomplete compressed datastream in %s chunk",
                ni_png_ptr->chunk_name);
         ni_png_warning(ni_png_ptr, umsg);
#else
         ni_png_warning(ni_png_ptr,
            "Incomplete compressed datastream in chunk other than IDAT");
#endif
         text_size=prefix_size;
         if (text ==  NULL)
         {
            text = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, text_size+1);
            if (text == NULL)
              {
                ni_png_free(ni_png_ptr, chunkdata);
                ni_png_error(ni_png_ptr,"Not enough memory for text.");
              }
            ni_png_memcpy(text, chunkdata, prefix_size);
         }
         *(text + text_size) = 0x00;
      }

      inflateReset(&ni_png_ptr->zstream);
      ni_png_ptr->zstream.avail_in = 0;

      ni_png_free(ni_png_ptr, chunkdata);
      chunkdata = text;
      *newlength=text_size;
   }
   else /* if (comp_type != PNG_COMPRESSION_TYPE_BASE) */
   {
#if !defined(PNG_NO_STDIO) && !defined(_WIN32_WCE)
      char umsg[50];

      sprintf(umsg, "Unknown zTXt compression type %d", comp_type);
      ni_png_warning(ni_png_ptr, umsg);
#else
      ni_png_warning(ni_png_ptr, "Unknown zTXt compression type");
#endif

      *(chunkdata + prefix_size) = 0x00;
      *newlength=prefix_size;
   }

   return chunkdata;
}
#endif

/* read and check the IDHR chunk */
void /* PRIVATE */
ni_png_handle_IHDR(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_byte buf[13];
   ni_png_uint_32 width, height;
   int bit_depth, color_type, compression_type, filter_type;
   int interlace_type;

   ni_png_debug(1, "in ni_png_handle_IHDR\n");

   if (ni_png_ptr->mode & PNG_HAVE_IHDR)
      ni_png_error(ni_png_ptr, "Out of place IHDR");

   /* check the length */
   if (length != 13)
      ni_png_error(ni_png_ptr, "Invalid IHDR chunk");

   ni_png_ptr->mode |= PNG_HAVE_IHDR;

   ni_png_crc_read(ni_png_ptr, buf, 13);
   ni_png_crc_finish(ni_png_ptr, 0);

   width = ni_png_get_uint_31(ni_png_ptr, buf);
   height = ni_png_get_uint_31(ni_png_ptr, buf + 4);
   bit_depth = buf[8];
   color_type = buf[9];
   compression_type = buf[10];
   filter_type = buf[11];
   interlace_type = buf[12];

   /* set internal variables */
   ni_png_ptr->width = width;
   ni_png_ptr->height = height;
   ni_png_ptr->bit_depth = (ni_png_byte)bit_depth;
   ni_png_ptr->interlaced = (ni_png_byte)interlace_type;
   ni_png_ptr->color_type = (ni_png_byte)color_type;
#if defined(PNG_MNG_FEATURES_SUPPORTED)
   ni_png_ptr->filter_type = (ni_png_byte)filter_type;
#endif
   ni_png_ptr->compression_type = (ni_png_byte)compression_type;

   /* find number of channels */
   switch (ni_png_ptr->color_type)
   {
      case PNG_COLOR_TYPE_GRAY:
      case PNG_COLOR_TYPE_PALETTE:
         ni_png_ptr->channels = 1;
         break;
      case PNG_COLOR_TYPE_RGB:
         ni_png_ptr->channels = 3;
         break;
      case PNG_COLOR_TYPE_GRAY_ALPHA:
         ni_png_ptr->channels = 2;
         break;
      case PNG_COLOR_TYPE_RGB_ALPHA:
         ni_png_ptr->channels = 4;
         break;
   }

   /* set up other useful info */
   ni_png_ptr->pixel_depth = (ni_png_byte)(ni_png_ptr->bit_depth *
   ni_png_ptr->channels);
   ni_png_ptr->rowbytes = PNG_ROWBYTES(ni_png_ptr->pixel_depth,ni_png_ptr->width);
   ni_png_debug1(3,"bit_depth = %d\n", ni_png_ptr->bit_depth);
   ni_png_debug1(3,"channels = %d\n", ni_png_ptr->channels);
   ni_png_debug1(3,"rowbytes = %lu\n", ni_png_ptr->rowbytes);
   ni_png_set_IHDR(ni_png_ptr, info_ptr, width, height, bit_depth,
      color_type, interlace_type, compression_type, filter_type);
}

/* read and check the palette */
void /* PRIVATE */
ni_png_handle_PLTE(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_color palette[PNG_MAX_PALETTE_LENGTH];
   int num, i;
#ifndef PNG_NO_POINTER_INDEXING
   ni_png_colorp pal_ptr;
#endif

   ni_png_debug(1, "in ni_png_handle_PLTE\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before PLTE");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid PLTE after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (ni_png_ptr->mode & PNG_HAVE_PLTE)
      ni_png_error(ni_png_ptr, "Duplicate PLTE chunk");

   ni_png_ptr->mode |= PNG_HAVE_PLTE;

   if (!(ni_png_ptr->color_type&PNG_COLOR_MASK_COLOR))
   {
      ni_png_warning(ni_png_ptr,
        "Ignoring PLTE chunk in grayscale PNG");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
#if !defined(PNG_READ_OPT_PLTE_SUPPORTED)
   if (ni_png_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
   {
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
#endif

   if (length > 3*PNG_MAX_PALETTE_LENGTH || length % 3)
   {
      if (ni_png_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
      {
         ni_png_warning(ni_png_ptr, "Invalid palette chunk");
         ni_png_crc_finish(ni_png_ptr, length);
         return;
      }
      else
      {
         ni_png_error(ni_png_ptr, "Invalid palette chunk");
      }
   }

   num = (int)length / 3;

#ifndef PNG_NO_POINTER_INDEXING
   for (i = 0, pal_ptr = palette; i < num; i++, pal_ptr++)
   {
      ni_png_byte buf[3];

      ni_png_crc_read(ni_png_ptr, buf, 3);
      pal_ptr->red = buf[0];
      pal_ptr->green = buf[1];
      pal_ptr->blue = buf[2];
   }
#else
   for (i = 0; i < num; i++)
   {
      ni_png_byte buf[3];

      ni_png_crc_read(ni_png_ptr, buf, 3);
      /* don't depend upon ni_png_color being any order */
      palette[i].red = buf[0];
      palette[i].green = buf[1];
      palette[i].blue = buf[2];
   }
#endif

   /* If we actually NEED the PLTE chunk (ie for a paletted image), we do
      whatever the normal CRC configuration tells us.  However, if we
      have an RGB image, the PLTE can be considered ancillary, so
      we will act as though it is. */
#if !defined(PNG_READ_OPT_PLTE_SUPPORTED)
   if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
#endif
   {
      ni_png_crc_finish(ni_png_ptr, 0);
   }
#if !defined(PNG_READ_OPT_PLTE_SUPPORTED)
   else if (ni_png_crc_error(ni_png_ptr))  /* Only if we have a CRC error */
   {
      /* If we don't want to use the data from an ancillary chunk,
         we have two options: an error abort, or a warning and we
         ignore the data in this chunk (which should be OK, since
         it's considered ancillary for a RGB or RGBA image). */
      if (!(ni_png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_USE))
      {
         if (ni_png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN)
         {
            ni_png_chunk_error(ni_png_ptr, "CRC error");
         }
         else
         {
            ni_png_chunk_warning(ni_png_ptr, "CRC error");
            return;
         }
      }
      /* Otherwise, we (optionally) emit a warning and use the chunk. */
      else if (!(ni_png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN))
      {
         ni_png_chunk_warning(ni_png_ptr, "CRC error");
      }
   }
#endif

   ni_png_set_PLTE(ni_png_ptr, info_ptr, palette, num);

#if defined(PNG_READ_tRNS_SUPPORTED)
   if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tRNS))
      {
         if (ni_png_ptr->num_trans > (ni_png_uint_16)num)
         {
            ni_png_warning(ni_png_ptr, "Truncating incorrect tRNS chunk length");
            ni_png_ptr->num_trans = (ni_png_uint_16)num;
         }
         if (info_ptr->num_trans > (ni_png_uint_16)num)
         {
            ni_png_warning(ni_png_ptr, "Truncating incorrect info tRNS chunk length");
            info_ptr->num_trans = (ni_png_uint_16)num;
         }
      }
   }
#endif

}

void /* PRIVATE */
ni_png_handle_IEND(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_debug(1, "in ni_png_handle_IEND\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR) || !(ni_png_ptr->mode & PNG_HAVE_IDAT))
   {
      ni_png_error(ni_png_ptr, "No image in file");
   }

   ni_png_ptr->mode |= (PNG_AFTER_IDAT | PNG_HAVE_IEND);

   if (length != 0)
   {
      ni_png_warning(ni_png_ptr, "Incorrect IEND chunk length");
   }
   ni_png_crc_finish(ni_png_ptr, length);

   if (&info_ptr == NULL) /* quiet compiler warnings about unused info_ptr */
      return;
}

#if defined(PNG_READ_gAMA_SUPPORTED)
void /* PRIVATE */
ni_png_handle_gAMA(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_fixed_point igamma;
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float file_gamma;
#endif
   ni_png_byte buf[4];

   ni_png_debug(1, "in ni_png_handle_gAMA\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before gAMA");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid gAMA after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (ni_png_ptr->mode & PNG_HAVE_PLTE)
      /* Should be an error, but we can cope with it */
      ni_png_warning(ni_png_ptr, "Out of place gAMA chunk");

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_gAMA)
#if defined(PNG_READ_sRGB_SUPPORTED)
      && !(info_ptr->valid & PNG_INFO_sRGB)
#endif
      )
   {
      ni_png_warning(ni_png_ptr, "Duplicate gAMA chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (length != 4)
   {
      ni_png_warning(ni_png_ptr, "Incorrect gAMA chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, 4);
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   igamma = (ni_png_fixed_point)ni_png_get_uint_32(buf);
   /* check for zero gamma */
   if (igamma == 0)
      {
         ni_png_warning(ni_png_ptr,
           "Ignoring gAMA chunk with gamma=0");
         return;
      }

#if defined(PNG_READ_sRGB_SUPPORTED)
   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sRGB))
      if (PNG_OUT_OF_RANGE(igamma, 45500L, 500))
      {
         ni_png_warning(ni_png_ptr,
           "Ignoring incorrect gAMA value when sRGB is also present");
#ifndef PNG_NO_CONSOLE_IO
         fprintf(stderr, "gamma = (%d/100000)\n", (int)igamma);
#endif
         return;
      }
#endif /* PNG_READ_sRGB_SUPPORTED */

#ifdef PNG_FLOATING_POINT_SUPPORTED
   file_gamma = (float)igamma / (float)100000.0;
#  ifdef PNG_READ_GAMMA_SUPPORTED
     ni_png_ptr->gamma = file_gamma;
#  endif
     ni_png_set_gAMA(ni_png_ptr, info_ptr, file_gamma);
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   ni_png_set_gAMA_fixed(ni_png_ptr, info_ptr, igamma);
#endif
}
#endif

#if defined(PNG_READ_sBIT_SUPPORTED)
void /* PRIVATE */
ni_png_handle_sBIT(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_size_t truelen;
   ni_png_byte buf[4];

   ni_png_debug(1, "in ni_png_handle_sBIT\n");

   buf[0] = buf[1] = buf[2] = buf[3] = 0;

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before sBIT");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid sBIT after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (ni_png_ptr->mode & PNG_HAVE_PLTE)
   {
      /* Should be an error, but we can cope with it */
      ni_png_warning(ni_png_ptr, "Out of place sBIT chunk");
   }
   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sBIT))
   {
      ni_png_warning(ni_png_ptr, "Duplicate sBIT chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      truelen = 3;
   else
      truelen = (ni_png_size_t)ni_png_ptr->channels;

   if (length != truelen || length > 4)
   {
      ni_png_warning(ni_png_ptr, "Incorrect sBIT chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, truelen);
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   if (ni_png_ptr->color_type & PNG_COLOR_MASK_COLOR)
   {
      ni_png_ptr->sig_bit.red = buf[0];
      ni_png_ptr->sig_bit.green = buf[1];
      ni_png_ptr->sig_bit.blue = buf[2];
      ni_png_ptr->sig_bit.alpha = buf[3];
   }
   else
   {
      ni_png_ptr->sig_bit.gray = buf[0];
      ni_png_ptr->sig_bit.red = buf[0];
      ni_png_ptr->sig_bit.green = buf[0];
      ni_png_ptr->sig_bit.blue = buf[0];
      ni_png_ptr->sig_bit.alpha = buf[1];
   }
   ni_png_set_sBIT(ni_png_ptr, info_ptr, &(ni_png_ptr->sig_bit));
}
#endif

#if defined(PNG_READ_cHRM_SUPPORTED)
void /* PRIVATE */
ni_png_handle_cHRM(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_byte buf[4];
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y;
#endif
   ni_png_fixed_point int_x_white, int_y_white, int_x_red, int_y_red, int_x_green,
      int_y_green, int_x_blue, int_y_blue;

   ni_png_uint_32 uint_x, uint_y;

   ni_png_debug(1, "in ni_png_handle_cHRM\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before cHRM");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (ni_png_ptr->mode & PNG_HAVE_PLTE)
      /* Should be an error, but we can cope with it */
      ni_png_warning(ni_png_ptr, "Missing PLTE before cHRM");

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_cHRM)
#if defined(PNG_READ_sRGB_SUPPORTED)
      && !(info_ptr->valid & PNG_INFO_sRGB)
#endif
      )
   {
      ni_png_warning(ni_png_ptr, "Duplicate cHRM chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (length != 32)
   {
      ni_png_warning(ni_png_ptr, "Incorrect cHRM chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_x = ni_png_get_uint_32(buf);

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_y = ni_png_get_uint_32(buf);

   if (uint_x > 80000L || uint_y > 80000L ||
      uint_x + uint_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM white point");
      ni_png_crc_finish(ni_png_ptr, 24);
      return;
   }
   int_x_white = (ni_png_fixed_point)uint_x;
   int_y_white = (ni_png_fixed_point)uint_y;

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_x = ni_png_get_uint_32(buf);

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_y = ni_png_get_uint_32(buf);

   if (uint_x + uint_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM red point");
      ni_png_crc_finish(ni_png_ptr, 16);
      return;
   }
   int_x_red = (ni_png_fixed_point)uint_x;
   int_y_red = (ni_png_fixed_point)uint_y;

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_x = ni_png_get_uint_32(buf);

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_y = ni_png_get_uint_32(buf);

   if (uint_x + uint_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM green point");
      ni_png_crc_finish(ni_png_ptr, 8);
      return;
   }
   int_x_green = (ni_png_fixed_point)uint_x;
   int_y_green = (ni_png_fixed_point)uint_y;

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_x = ni_png_get_uint_32(buf);

   ni_png_crc_read(ni_png_ptr, buf, 4);
   uint_y = ni_png_get_uint_32(buf);

   if (uint_x + uint_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM blue point");
      ni_png_crc_finish(ni_png_ptr, 0);
      return;
   }
   int_x_blue = (ni_png_fixed_point)uint_x;
   int_y_blue = (ni_png_fixed_point)uint_y;

#ifdef PNG_FLOATING_POINT_SUPPORTED
   white_x = (float)int_x_white / (float)100000.0;
   white_y = (float)int_y_white / (float)100000.0;
   red_x   = (float)int_x_red   / (float)100000.0;
   red_y   = (float)int_y_red   / (float)100000.0;
   green_x = (float)int_x_green / (float)100000.0;
   green_y = (float)int_y_green / (float)100000.0;
   blue_x  = (float)int_x_blue  / (float)100000.0;
   blue_y  = (float)int_y_blue  / (float)100000.0;
#endif

#if defined(PNG_READ_sRGB_SUPPORTED)
   if ((info_ptr != NULL) && (info_ptr->valid & PNG_INFO_sRGB))
      {
      if (PNG_OUT_OF_RANGE(int_x_white, 31270,  1000) ||
          PNG_OUT_OF_RANGE(int_y_white, 32900,  1000) ||
          PNG_OUT_OF_RANGE(int_x_red,   64000L, 1000) ||
          PNG_OUT_OF_RANGE(int_y_red,   33000,  1000) ||
          PNG_OUT_OF_RANGE(int_x_green, 30000,  1000) ||
          PNG_OUT_OF_RANGE(int_y_green, 60000L, 1000) ||
          PNG_OUT_OF_RANGE(int_x_blue,  15000,  1000) ||
          PNG_OUT_OF_RANGE(int_y_blue,   6000,  1000))
         {
            ni_png_warning(ni_png_ptr,
              "Ignoring incorrect cHRM value when sRGB is also present");
#ifndef PNG_NO_CONSOLE_IO
#ifdef PNG_FLOATING_POINT_SUPPORTED
            fprintf(stderr,"wx=%f, wy=%f, rx=%f, ry=%f\n",
               white_x, white_y, red_x, red_y);
            fprintf(stderr,"gx=%f, gy=%f, bx=%f, by=%f\n",
               green_x, green_y, blue_x, blue_y);
#else
            fprintf(stderr,"wx=%ld, wy=%ld, rx=%ld, ry=%ld\n",
               int_x_white, int_y_white, int_x_red, int_y_red);
            fprintf(stderr,"gx=%ld, gy=%ld, bx=%ld, by=%ld\n",
               int_x_green, int_y_green, int_x_blue, int_y_blue);
#endif
#endif /* PNG_NO_CONSOLE_IO */
         }
         ni_png_crc_finish(ni_png_ptr, 0);
         return;
      }
#endif /* PNG_READ_sRGB_SUPPORTED */

#ifdef PNG_FLOATING_POINT_SUPPORTED
   ni_png_set_cHRM(ni_png_ptr, info_ptr,
      white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y);
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   ni_png_set_cHRM_fixed(ni_png_ptr, info_ptr,
      int_x_white, int_y_white, int_x_red, int_y_red, int_x_green,
      int_y_green, int_x_blue, int_y_blue);
#endif
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;
}
#endif

#if defined(PNG_READ_sRGB_SUPPORTED)
void /* PRIVATE */
ni_png_handle_sRGB(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   int intent;
   ni_png_byte buf[1];

   ni_png_debug(1, "in ni_png_handle_sRGB\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before sRGB");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid sRGB after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (ni_png_ptr->mode & PNG_HAVE_PLTE)
      /* Should be an error, but we can cope with it */
      ni_png_warning(ni_png_ptr, "Out of place sRGB chunk");

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sRGB))
   {
      ni_png_warning(ni_png_ptr, "Duplicate sRGB chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (length != 1)
   {
      ni_png_warning(ni_png_ptr, "Incorrect sRGB chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, 1);
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   intent = buf[0];
   /* check for bad intent */
   if (intent >= PNG_sRGB_INTENT_LAST)
   {
      ni_png_warning(ni_png_ptr, "Unknown sRGB intent");
      return;
   }

#if defined(PNG_READ_gAMA_SUPPORTED) && defined(PNG_READ_GAMMA_SUPPORTED)
   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_gAMA))
   {
   ni_png_fixed_point igamma;
#ifdef PNG_FIXED_POINT_SUPPORTED
      igamma=info_ptr->int_gamma;
#else
#  ifdef PNG_FLOATING_POINT_SUPPORTED
      igamma=(ni_png_fixed_point)(info_ptr->gamma * 100000.);
#  endif
#endif
      if (PNG_OUT_OF_RANGE(igamma, 45500L, 500))
      {
         ni_png_warning(ni_png_ptr,
           "Ignoring incorrect gAMA value when sRGB is also present");
#ifndef PNG_NO_CONSOLE_IO
#  ifdef PNG_FIXED_POINT_SUPPORTED
         fprintf(stderr,"incorrect gamma=(%d/100000)\n",(int)ni_png_ptr->int_gamma);
#  else
#    ifdef PNG_FLOATING_POINT_SUPPORTED
         fprintf(stderr,"incorrect gamma=%f\n",ni_png_ptr->gamma);
#    endif
#  endif
#endif
      }
   }
#endif /* PNG_READ_gAMA_SUPPORTED */

#ifdef PNG_READ_cHRM_SUPPORTED
#ifdef PNG_FIXED_POINT_SUPPORTED
   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_cHRM))
      if (PNG_OUT_OF_RANGE(info_ptr->int_x_white, 31270,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->int_y_white, 32900,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->int_x_red,   64000L, 1000) ||
          PNG_OUT_OF_RANGE(info_ptr->int_y_red,   33000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->int_x_green, 30000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->int_y_green, 60000L, 1000) ||
          PNG_OUT_OF_RANGE(info_ptr->int_x_blue,  15000,  1000) ||
          PNG_OUT_OF_RANGE(info_ptr->int_y_blue,   6000,  1000))
         {
            ni_png_warning(ni_png_ptr,
              "Ignoring incorrect cHRM value when sRGB is also present");
         }
#endif /* PNG_FIXED_POINT_SUPPORTED */
#endif /* PNG_READ_cHRM_SUPPORTED */

   ni_png_set_sRGB_gAMA_and_cHRM(ni_png_ptr, info_ptr, intent);
}
#endif /* PNG_READ_sRGB_SUPPORTED */

#if defined(PNG_READ_iCCP_SUPPORTED)
void /* PRIVATE */
ni_png_handle_iCCP(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
/* Note: this does not properly handle chunks that are > 64K under DOS */
{
   ni_png_charp chunkdata;
   ni_png_byte compression_type;
   ni_png_bytep pC;
   ni_png_charp profile;
   ni_png_uint_32 skip = 0;
   ni_png_uint_32 profile_size, profile_length;
   ni_png_size_t slength, prefix_length, data_length;

   ni_png_debug(1, "in ni_png_handle_iCCP\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before iCCP");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid iCCP after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (ni_png_ptr->mode & PNG_HAVE_PLTE)
      /* Should be an error, but we can cope with it */
      ni_png_warning(ni_png_ptr, "Out of place iCCP chunk");

   if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_iCCP))
   {
      ni_png_warning(ni_png_ptr, "Duplicate iCCP chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

#ifdef PNG_MAX_MALLOC_64K
   if (length > (ni_png_uint_32)65535L)
   {
      ni_png_warning(ni_png_ptr, "iCCP chunk too large to fit in memory");
      skip = length - (ni_png_uint_32)65535L;
      length = (ni_png_uint_32)65535L;
   }
#endif

   chunkdata = (ni_png_charp)ni_png_malloc(ni_png_ptr, length + 1);
   slength = (ni_png_size_t)length;
   ni_png_crc_read(ni_png_ptr, (ni_png_bytep)chunkdata, slength);

   if (ni_png_crc_finish(ni_png_ptr, skip))
   {
      ni_png_free(ni_png_ptr, chunkdata);
      return;
   }

   chunkdata[slength] = 0x00;

   for (profile = chunkdata; *profile; profile++)
      /* empty loop to find end of name */ ;

   ++profile;

   /* there should be at least one zero (the compression type byte)
      following the separator, and we should be on it  */
   if ( profile >= chunkdata + slength)
   {
      ni_png_free(ni_png_ptr, chunkdata);
      ni_png_warning(ni_png_ptr, "Malformed iCCP chunk");
      return;
   }

   /* compression_type should always be zero */
   compression_type = *profile++;
   if (compression_type)
   {
      ni_png_warning(ni_png_ptr, "Ignoring nonzero compression type in iCCP chunk");
      compression_type=0x00;  /* Reset it to zero (libpng-1.0.6 through 1.0.8
                                 wrote nonzero) */
   }

   prefix_length = profile - chunkdata;
   chunkdata = ni_png_decompress_chunk(ni_png_ptr, compression_type, chunkdata,
                                    slength, prefix_length, &data_length);

   profile_length = data_length - prefix_length;

   if ( prefix_length > data_length || profile_length < 4)
   {
      ni_png_free(ni_png_ptr, chunkdata);
      ni_png_warning(ni_png_ptr, "Profile size field missing from iCCP chunk");
      return;
   }

   /* Check the profile_size recorded in the first 32 bits of the ICC profile */
   pC = (ni_png_bytep)(chunkdata+prefix_length);
   profile_size = ((*(pC  ))<<24) |
                  ((*(pC+1))<<16) |
                  ((*(pC+2))<< 8) |
                  ((*(pC+3))    );

   if(profile_size < profile_length)
      profile_length = profile_size;

   if(profile_size > profile_length)
   {
      ni_png_free(ni_png_ptr, chunkdata);
      ni_png_warning(ni_png_ptr, "Ignoring truncated iCCP profile.");
      return;
   }

   ni_png_set_iCCP(ni_png_ptr, info_ptr, chunkdata, compression_type,
                chunkdata + prefix_length, profile_length);
   ni_png_free(ni_png_ptr, chunkdata);
}
#endif /* PNG_READ_iCCP_SUPPORTED */

#if defined(PNG_READ_sPLT_SUPPORTED)
void /* PRIVATE */
ni_png_handle_sPLT(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
/* Note: this does not properly handle chunks that are > 64K under DOS */
{
   ni_png_bytep chunkdata;
   ni_png_bytep entry_start;
   ni_png_sPLT_t new_palette;
#ifdef PNG_NO_POINTER_INDEXING
   ni_png_sPLT_entryp pp;
#endif
   int data_length, entry_size, i;
   ni_png_uint_32 skip = 0;
   ni_png_size_t slength;

   ni_png_debug(1, "in ni_png_handle_sPLT\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before sPLT");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid sPLT after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

#ifdef PNG_MAX_MALLOC_64K
   if (length > (ni_png_uint_32)65535L)
   {
      ni_png_warning(ni_png_ptr, "sPLT chunk too large to fit in memory");
      skip = length - (ni_png_uint_32)65535L;
      length = (ni_png_uint_32)65535L;
   }
#endif

   chunkdata = (ni_png_bytep)ni_png_malloc(ni_png_ptr, length + 1);
   slength = (ni_png_size_t)length;
   ni_png_crc_read(ni_png_ptr, (ni_png_bytep)chunkdata, slength);

   if (ni_png_crc_finish(ni_png_ptr, skip))
   {
      ni_png_free(ni_png_ptr, chunkdata);
      return;
   }

   chunkdata[slength] = 0x00;

   for (entry_start = chunkdata; *entry_start; entry_start++)
      /* empty loop to find end of name */ ;
   ++entry_start;

   /* a sample depth should follow the separator, and we should be on it  */
   if (entry_start > chunkdata + slength)
   {
      ni_png_free(ni_png_ptr, chunkdata);
      ni_png_warning(ni_png_ptr, "malformed sPLT chunk");
      return;
   }

   new_palette.depth = *entry_start++;
   entry_size = (new_palette.depth == 8 ? 6 : 10);
   data_length = (slength - (entry_start - chunkdata));

   /* integrity-check the data length */
   if (data_length % entry_size)
   {
      ni_png_free(ni_png_ptr, chunkdata);
      ni_png_warning(ni_png_ptr, "sPLT chunk has bad length");
      return;
   }

   new_palette.nentries = (ni_png_int_32) ( data_length / entry_size);
   if ((ni_png_uint_32) new_palette.nentries > (ni_png_uint_32) (PNG_SIZE_MAX /
       ni_png_sizeof(ni_png_sPLT_entry)))
   {
       ni_png_warning(ni_png_ptr, "sPLT chunk too long");
       return;
   }
   new_palette.entries = (ni_png_sPLT_entryp)ni_png_malloc_warn(
       ni_png_ptr, new_palette.nentries * ni_png_sizeof(ni_png_sPLT_entry));
   if (new_palette.entries == NULL)
   {
       ni_png_warning(ni_png_ptr, "sPLT chunk requires too much memory");
       return;
   }

#ifndef PNG_NO_POINTER_INDEXING
   for (i = 0; i < new_palette.nentries; i++)
   {
      ni_png_sPLT_entryp pp = new_palette.entries + i;

      if (new_palette.depth == 8)
      {
          pp->red = *entry_start++;
          pp->green = *entry_start++;
          pp->blue = *entry_start++;
          pp->alpha = *entry_start++;
      }
      else
      {
          pp->red   = ni_png_get_uint_16(entry_start); entry_start += 2;
          pp->green = ni_png_get_uint_16(entry_start); entry_start += 2;
          pp->blue  = ni_png_get_uint_16(entry_start); entry_start += 2;
          pp->alpha = ni_png_get_uint_16(entry_start); entry_start += 2;
      }
      pp->frequency = ni_png_get_uint_16(entry_start); entry_start += 2;
   }
#else
   pp = new_palette.entries;
   for (i = 0; i < new_palette.nentries; i++)
   {

      if (new_palette.depth == 8)
      {
          pp[i].red   = *entry_start++;
          pp[i].green = *entry_start++;
          pp[i].blue  = *entry_start++;
          pp[i].alpha = *entry_start++;
      }
      else
      {
          pp[i].red   = ni_png_get_uint_16(entry_start); entry_start += 2;
          pp[i].green = ni_png_get_uint_16(entry_start); entry_start += 2;
          pp[i].blue  = ni_png_get_uint_16(entry_start); entry_start += 2;
          pp[i].alpha = ni_png_get_uint_16(entry_start); entry_start += 2;
      }
      pp->frequency = ni_png_get_uint_16(entry_start); entry_start += 2;
   }
#endif

   /* discard all chunk data except the name and stash that */
   new_palette.name = (ni_png_charp)chunkdata;

   ni_png_set_sPLT(ni_png_ptr, info_ptr, &new_palette, 1);

   ni_png_free(ni_png_ptr, chunkdata);
   ni_png_free(ni_png_ptr, new_palette.entries);
}
#endif /* PNG_READ_sPLT_SUPPORTED */

#if defined(PNG_READ_tRNS_SUPPORTED)
void /* PRIVATE */
ni_png_handle_tRNS(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_byte readbuf[PNG_MAX_PALETTE_LENGTH];

   ni_png_debug(1, "in ni_png_handle_tRNS\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before tRNS");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid tRNS after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tRNS))
   {
      ni_png_warning(ni_png_ptr, "Duplicate tRNS chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (ni_png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
   {
      ni_png_byte buf[2];

      if (length != 2)
      {
         ni_png_warning(ni_png_ptr, "Incorrect tRNS chunk length");
         ni_png_crc_finish(ni_png_ptr, length);
         return;
      }

      ni_png_crc_read(ni_png_ptr, buf, 2);
      ni_png_ptr->num_trans = 1;
      ni_png_ptr->trans_values.gray = ni_png_get_uint_16(buf);
   }
   else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_RGB)
   {
      ni_png_byte buf[6];

      if (length != 6)
      {
         ni_png_warning(ni_png_ptr, "Incorrect tRNS chunk length");
         ni_png_crc_finish(ni_png_ptr, length);
         return;
      }
      ni_png_crc_read(ni_png_ptr, buf, (ni_png_size_t)length);
      ni_png_ptr->num_trans = 1;
      ni_png_ptr->trans_values.red = ni_png_get_uint_16(buf);
      ni_png_ptr->trans_values.green = ni_png_get_uint_16(buf + 2);
      ni_png_ptr->trans_values.blue = ni_png_get_uint_16(buf + 4);
   }
   else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (!(ni_png_ptr->mode & PNG_HAVE_PLTE))
      {
         /* Should be an error, but we can cope with it. */
         ni_png_warning(ni_png_ptr, "Missing PLTE before tRNS");
      }
      if (length > (ni_png_uint_32)ni_png_ptr->num_palette ||
          length > PNG_MAX_PALETTE_LENGTH)
      {
         ni_png_warning(ni_png_ptr, "Incorrect tRNS chunk length");
         ni_png_crc_finish(ni_png_ptr, length);
         return;
      }
      if (length == 0)
      {
         ni_png_warning(ni_png_ptr, "Zero length tRNS chunk");
         ni_png_crc_finish(ni_png_ptr, length);
         return;
      }
      ni_png_crc_read(ni_png_ptr, readbuf, (ni_png_size_t)length);
      ni_png_ptr->num_trans = (ni_png_uint_16)length;
   }
   else
   {
      ni_png_warning(ni_png_ptr, "tRNS chunk not allowed with alpha channel");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (ni_png_crc_finish(ni_png_ptr, 0))
   {
      ni_png_ptr->num_trans = 0;
      return;
   }

   ni_png_set_tRNS(ni_png_ptr, info_ptr, readbuf, ni_png_ptr->num_trans,
      &(ni_png_ptr->trans_values));
}
#endif

#if defined(PNG_READ_bKGD_SUPPORTED)
void /* PRIVATE */
ni_png_handle_bKGD(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_size_t truelen;
   ni_png_byte buf[6];

   ni_png_debug(1, "in ni_png_handle_bKGD\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before bKGD");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid bKGD after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE &&
            !(ni_png_ptr->mode & PNG_HAVE_PLTE))
   {
      ni_png_warning(ni_png_ptr, "Missing PLTE before bKGD");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_bKGD))
   {
      ni_png_warning(ni_png_ptr, "Duplicate bKGD chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      truelen = 1;
   else if (ni_png_ptr->color_type & PNG_COLOR_MASK_COLOR)
      truelen = 6;
   else
      truelen = 2;

   if (length != truelen)
   {
      ni_png_warning(ni_png_ptr, "Incorrect bKGD chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, truelen);
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   /* We convert the index value into RGB components so that we can allow
    * arbitrary RGB values for background when we have transparency, and
    * so it is easy to determine the RGB values of the background color
    * from the info_ptr struct. */
   if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      ni_png_ptr->background.index = buf[0];
      if(info_ptr->num_palette)
      {
          if(buf[0] > info_ptr->num_palette)
          {
             ni_png_warning(ni_png_ptr, "Incorrect bKGD chunk index value");
             return;
          }
          ni_png_ptr->background.red =
             (ni_png_uint_16)ni_png_ptr->palette[buf[0]].red;
          ni_png_ptr->background.green =
             (ni_png_uint_16)ni_png_ptr->palette[buf[0]].green;
          ni_png_ptr->background.blue =
             (ni_png_uint_16)ni_png_ptr->palette[buf[0]].blue;
      }
   }
   else if (!(ni_png_ptr->color_type & PNG_COLOR_MASK_COLOR)) /* GRAY */
   {
      ni_png_ptr->background.red =
      ni_png_ptr->background.green =
      ni_png_ptr->background.blue =
      ni_png_ptr->background.gray = ni_png_get_uint_16(buf);
   }
   else
   {
      ni_png_ptr->background.red = ni_png_get_uint_16(buf);
      ni_png_ptr->background.green = ni_png_get_uint_16(buf + 2);
      ni_png_ptr->background.blue = ni_png_get_uint_16(buf + 4);
   }

   ni_png_set_bKGD(ni_png_ptr, info_ptr, &(ni_png_ptr->background));
}
#endif

#if defined(PNG_READ_hIST_SUPPORTED)
void /* PRIVATE */
ni_png_handle_hIST(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   unsigned int num, i;
   ni_png_uint_16 readbuf[PNG_MAX_PALETTE_LENGTH];

   ni_png_debug(1, "in ni_png_handle_hIST\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before hIST");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid hIST after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (!(ni_png_ptr->mode & PNG_HAVE_PLTE))
   {
      ni_png_warning(ni_png_ptr, "Missing PLTE before hIST");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_hIST))
   {
      ni_png_warning(ni_png_ptr, "Duplicate hIST chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   num = length / 2 ;
   if (num != (unsigned int) ni_png_ptr->num_palette || num >
      (unsigned int) PNG_MAX_PALETTE_LENGTH)
   {
      ni_png_warning(ni_png_ptr, "Incorrect hIST chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   for (i = 0; i < num; i++)
   {
      ni_png_byte buf[2];

      ni_png_crc_read(ni_png_ptr, buf, 2);
      readbuf[i] = ni_png_get_uint_16(buf);
   }

   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   ni_png_set_hIST(ni_png_ptr, info_ptr, readbuf);
}
#endif

#if defined(PNG_READ_pHYs_SUPPORTED)
void /* PRIVATE */
ni_png_handle_pHYs(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_byte buf[9];
   ni_png_uint_32 res_x, res_y;
   int unit_type;

   ni_png_debug(1, "in ni_png_handle_pHYs\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before pHYs");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid pHYs after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_pHYs))
   {
      ni_png_warning(ni_png_ptr, "Duplicate pHYs chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (length != 9)
   {
      ni_png_warning(ni_png_ptr, "Incorrect pHYs chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, 9);
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   res_x = ni_png_get_uint_32(buf);
   res_y = ni_png_get_uint_32(buf + 4);
   unit_type = buf[8];
   ni_png_set_pHYs(ni_png_ptr, info_ptr, res_x, res_y, unit_type);
}
#endif

#if defined(PNG_READ_oFFs_SUPPORTED)
void /* PRIVATE */
ni_png_handle_oFFs(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_byte buf[9];
   ni_png_int_32 offset_x, offset_y;
   int unit_type;

   ni_png_debug(1, "in ni_png_handle_oFFs\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before oFFs");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid oFFs after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_oFFs))
   {
      ni_png_warning(ni_png_ptr, "Duplicate oFFs chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (length != 9)
   {
      ni_png_warning(ni_png_ptr, "Incorrect oFFs chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, 9);
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   offset_x = ni_png_get_int_32(buf);
   offset_y = ni_png_get_int_32(buf + 4);
   unit_type = buf[8];
   ni_png_set_oFFs(ni_png_ptr, info_ptr, offset_x, offset_y, unit_type);
}
#endif

#if defined(PNG_READ_pCAL_SUPPORTED)
/* read the pCAL chunk (described in the PNG Extensions document) */
void /* PRIVATE */
ni_png_handle_pCAL(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_charp purpose;
   ni_png_int_32 X0, X1;
   ni_png_byte type, nparams;
   ni_png_charp buf, units, endptr;
   ni_png_charpp params;
   ni_png_size_t slength;
   int i;

   ni_png_debug(1, "in ni_png_handle_pCAL\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before pCAL");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid pCAL after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_pCAL))
   {
      ni_png_warning(ni_png_ptr, "Duplicate pCAL chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_debug1(2, "Allocating and reading pCAL chunk data (%lu bytes)\n",
      length + 1);
   purpose = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length + 1);
   if (purpose == NULL)
     {
       ni_png_warning(ni_png_ptr, "No memory for pCAL purpose.");
       return;
     }
   slength = (ni_png_size_t)length;
   ni_png_crc_read(ni_png_ptr, (ni_png_bytep)purpose, slength);

   if (ni_png_crc_finish(ni_png_ptr, 0))
   {
      ni_png_free(ni_png_ptr, purpose);
      return;
   }

   purpose[slength] = 0x00; /* null terminate the last string */

   ni_png_debug(3, "Finding end of pCAL purpose string\n");
   for (buf = purpose; *buf; buf++)
      /* empty loop */ ;

   endptr = purpose + slength;

   /* We need to have at least 12 bytes after the purpose string
      in order to get the parameter information. */
   if (endptr <= buf + 12)
   {
      ni_png_warning(ni_png_ptr, "Invalid pCAL data");
      ni_png_free(ni_png_ptr, purpose);
      return;
   }

   ni_png_debug(3, "Reading pCAL X0, X1, type, nparams, and units\n");
   X0 = ni_png_get_int_32((ni_png_bytep)buf+1);
   X1 = ni_png_get_int_32((ni_png_bytep)buf+5);
   type = buf[9];
   nparams = buf[10];
   units = buf + 11;

   ni_png_debug(3, "Checking pCAL equation type and number of parameters\n");
   /* Check that we have the right number of parameters for known
      equation types. */
   if ((type == PNG_EQUATION_LINEAR && nparams != 2) ||
       (type == PNG_EQUATION_BASE_E && nparams != 3) ||
       (type == PNG_EQUATION_ARBITRARY && nparams != 3) ||
       (type == PNG_EQUATION_HYPERBOLIC && nparams != 4))
   {
      ni_png_warning(ni_png_ptr, "Invalid pCAL parameters for equation type");
      ni_png_free(ni_png_ptr, purpose);
      return;
   }
   else if (type >= PNG_EQUATION_LAST)
   {
      ni_png_warning(ni_png_ptr, "Unrecognized equation type for pCAL chunk");
   }

   for (buf = units; *buf; buf++)
      /* Empty loop to move past the units string. */ ;

   ni_png_debug(3, "Allocating pCAL parameters array\n");
   params = (ni_png_charpp)ni_png_malloc_warn(ni_png_ptr, (ni_png_uint_32)(nparams
      *ni_png_sizeof(ni_png_charp))) ;
   if (params == NULL)
     {
       ni_png_free(ni_png_ptr, purpose);
       ni_png_warning(ni_png_ptr, "No memory for pCAL params.");
       return;
     }

   /* Get pointers to the start of each parameter string. */
   for (i = 0; i < (int)nparams; i++)
   {
      buf++; /* Skip the null string terminator from previous parameter. */

      ni_png_debug1(3, "Reading pCAL parameter %d\n", i);
      for (params[i] = buf; *buf != 0x00 && buf <= endptr; buf++)
         /* Empty loop to move past each parameter string */ ;

      /* Make sure we haven't run out of data yet */
      if (buf > endptr)
      {
         ni_png_warning(ni_png_ptr, "Invalid pCAL data");
         ni_png_free(ni_png_ptr, purpose);
         ni_png_free(ni_png_ptr, params);
         return;
      }
   }

   ni_png_set_pCAL(ni_png_ptr, info_ptr, purpose, X0, X1, type, nparams,
      units, params);

   ni_png_free(ni_png_ptr, purpose);
   ni_png_free(ni_png_ptr, params);
}
#endif

#if defined(PNG_READ_sCAL_SUPPORTED)
/* read the sCAL chunk */
void /* PRIVATE */
ni_png_handle_sCAL(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_charp buffer, ep;
#ifdef PNG_FLOATING_POINT_SUPPORTED
   double width, height;
   ni_png_charp vp;
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
   ni_png_charp swidth, sheight;
#endif
#endif
   ni_png_size_t slength;

   ni_png_debug(1, "in ni_png_handle_sCAL\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before sCAL");
   else if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
      ni_png_warning(ni_png_ptr, "Invalid sCAL after IDAT");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_sCAL))
   {
      ni_png_warning(ni_png_ptr, "Duplicate sCAL chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_debug1(2, "Allocating and reading sCAL chunk data (%lu bytes)\n",
      length + 1);
   buffer = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length + 1);
   if (buffer == NULL)
     {
       ni_png_warning(ni_png_ptr, "Out of memory while processing sCAL chunk");
       return;
     }
   slength = (ni_png_size_t)length;
   ni_png_crc_read(ni_png_ptr, (ni_png_bytep)buffer, slength);

   if (ni_png_crc_finish(ni_png_ptr, 0))
   {
      ni_png_free(ni_png_ptr, buffer);
      return;
   }

   buffer[slength] = 0x00; /* null terminate the last string */

   ep = buffer + 1;        /* skip unit byte */

#ifdef PNG_FLOATING_POINT_SUPPORTED
   width = ni_png_strtod(ni_png_ptr, ep, &vp);
   if (*vp)
   {
       ni_png_warning(ni_png_ptr, "malformed width string in sCAL chunk");
       return;
   }
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
   swidth = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, ni_png_strlen(ep) + 1);
   if (swidth == NULL)
     {
       ni_png_warning(ni_png_ptr, "Out of memory while processing sCAL chunk width");
       return;
     }
   ni_png_memcpy(swidth, ep, (ni_png_size_t)ni_png_strlen(ep));
#endif
#endif

   for (ep = buffer; *ep; ep++)
      /* empty loop */ ;
   ep++;

#ifdef PNG_FLOATING_POINT_SUPPORTED
   height = ni_png_strtod(ni_png_ptr, ep, &vp);
   if (*vp)
   {
       ni_png_warning(ni_png_ptr, "malformed height string in sCAL chunk");
       return;
   }
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
   sheight = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, ni_png_strlen(ep) + 1);
   if (swidth == NULL)
     {
       ni_png_warning(ni_png_ptr, "Out of memory while processing sCAL chunk height");
       return;
     }
   ni_png_memcpy(sheight, ep, (ni_png_size_t)ni_png_strlen(ep));
#endif
#endif

   if (buffer + slength < ep
#ifdef PNG_FLOATING_POINT_SUPPORTED
      || width <= 0. || height <= 0.
#endif
      )
   {
      ni_png_warning(ni_png_ptr, "Invalid sCAL data");
      ni_png_free(ni_png_ptr, buffer);
#if defined(PNG_FIXED_POINT_SUPPORTED) && !defined(PNG_FLOATING_POINT_SUPPORTED)
      ni_png_free(ni_png_ptr, swidth);
      ni_png_free(ni_png_ptr, sheight);
#endif
      return;
   }


#ifdef PNG_FLOATING_POINT_SUPPORTED
   ni_png_set_sCAL(ni_png_ptr, info_ptr, buffer[0], width, height);
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
   ni_png_set_sCAL_s(ni_png_ptr, info_ptr, buffer[0], swidth, sheight);
#endif
#endif

   ni_png_free(ni_png_ptr, buffer);
#if defined(PNG_FIXED_POINT_SUPPORTED) && !defined(PNG_FLOATING_POINT_SUPPORTED)
   ni_png_free(ni_png_ptr, swidth);
   ni_png_free(ni_png_ptr, sheight);
#endif
}
#endif

#if defined(PNG_READ_tIME_SUPPORTED)
void /* PRIVATE */
ni_png_handle_tIME(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_byte buf[7];
   ni_png_time mod_time;

   ni_png_debug(1, "in ni_png_handle_tIME\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Out of place tIME chunk");
   else if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tIME))
   {
      ni_png_warning(ni_png_ptr, "Duplicate tIME chunk");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   if (ni_png_ptr->mode & PNG_HAVE_IDAT)
      ni_png_ptr->mode |= PNG_AFTER_IDAT;

   if (length != 7)
   {
      ni_png_warning(ni_png_ptr, "Incorrect tIME chunk length");
      ni_png_crc_finish(ni_png_ptr, length);
      return;
   }

   ni_png_crc_read(ni_png_ptr, buf, 7);
   if (ni_png_crc_finish(ni_png_ptr, 0))
      return;

   mod_time.second = buf[6];
   mod_time.minute = buf[5];
   mod_time.hour = buf[4];
   mod_time.day = buf[3];
   mod_time.month = buf[2];
   mod_time.year = ni_png_get_uint_16(buf);

   ni_png_set_tIME(ni_png_ptr, info_ptr, &mod_time);
}
#endif

#if defined(PNG_READ_tEXt_SUPPORTED)
/* Note: this does not properly handle chunks that are > 64K under DOS */
void /* PRIVATE */
ni_png_handle_tEXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_textp text_ptr;
   ni_png_charp key;
   ni_png_charp text;
   ni_png_uint_32 skip = 0;
   ni_png_size_t slength;
   int ret;

   ni_png_debug(1, "in ni_png_handle_tEXt\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before tEXt");

   if (ni_png_ptr->mode & PNG_HAVE_IDAT)
      ni_png_ptr->mode |= PNG_AFTER_IDAT;

#ifdef PNG_MAX_MALLOC_64K
   if (length > (ni_png_uint_32)65535L)
   {
      ni_png_warning(ni_png_ptr, "tEXt chunk too large to fit in memory");
      skip = length - (ni_png_uint_32)65535L;
      length = (ni_png_uint_32)65535L;
   }
#endif

   key = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length + 1);
   if (key == NULL)
   {
     ni_png_warning(ni_png_ptr, "No memory to process text chunk.");
     return;
   }
   slength = (ni_png_size_t)length;
   ni_png_crc_read(ni_png_ptr, (ni_png_bytep)key, slength);

   if (ni_png_crc_finish(ni_png_ptr, skip))
   {
      ni_png_free(ni_png_ptr, key);
      return;
   }

   key[slength] = 0x00;

   for (text = key; *text; text++)
      /* empty loop to find end of key */ ;

   if (text != key + slength)
      text++;

   text_ptr = (ni_png_textp)ni_png_malloc_warn(ni_png_ptr,
      (ni_png_uint_32)ni_png_sizeof(ni_png_text));
   if (text_ptr == NULL)
   {
     ni_png_warning(ni_png_ptr, "Not enough memory to process text chunk.");
     ni_png_free(ni_png_ptr, key);
     return;
   }
   text_ptr->compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr->key = key;
#ifdef PNG_iTXt_SUPPORTED
   text_ptr->lang = NULL;
   text_ptr->lang_key = NULL;
   text_ptr->itxt_length = 0;
#endif
   text_ptr->text = text;
   text_ptr->text_length = ni_png_strlen(text);

   ret=ni_png_set_text_2(ni_png_ptr, info_ptr, text_ptr, 1);

   ni_png_free(ni_png_ptr, key);
   ni_png_free(ni_png_ptr, text_ptr);
   if (ret)
     ni_png_warning(ni_png_ptr, "Insufficient memory to process text chunk.");
}
#endif

#if defined(PNG_READ_zTXt_SUPPORTED)
/* note: this does not correctly handle chunks that are > 64K under DOS */
void /* PRIVATE */
ni_png_handle_zTXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_textp text_ptr;
   ni_png_charp chunkdata;
   ni_png_charp text;
   int comp_type;
   int ret;
   ni_png_size_t slength, prefix_len, data_len;

   ni_png_debug(1, "in ni_png_handle_zTXt\n");
   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before zTXt");

   if (ni_png_ptr->mode & PNG_HAVE_IDAT)
      ni_png_ptr->mode |= PNG_AFTER_IDAT;

#ifdef PNG_MAX_MALLOC_64K
   /* We will no doubt have problems with chunks even half this size, but
      there is no hard and fast rule to tell us where to stop. */
   if (length > (ni_png_uint_32)65535L)
   {
     ni_png_warning(ni_png_ptr,"zTXt chunk too large to fit in memory");
     ni_png_crc_finish(ni_png_ptr, length);
     return;
   }
#endif

   chunkdata = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length + 1);
   if (chunkdata == NULL)
   {
     ni_png_warning(ni_png_ptr,"Out of memory processing zTXt chunk.");
     return;
   }
   slength = (ni_png_size_t)length;
   ni_png_crc_read(ni_png_ptr, (ni_png_bytep)chunkdata, slength);
   if (ni_png_crc_finish(ni_png_ptr, 0))
   {
      ni_png_free(ni_png_ptr, chunkdata);
      return;
   }

   chunkdata[slength] = 0x00;

   for (text = chunkdata; *text; text++)
      /* empty loop */ ;

   /* zTXt must have some text after the chunkdataword */
   if (text == chunkdata + slength)
   {
      comp_type = PNG_TEXT_COMPRESSION_NONE;
      ni_png_warning(ni_png_ptr, "Zero length zTXt chunk");
   }
   else
   {
       comp_type = *(++text);
       if (comp_type != PNG_TEXT_COMPRESSION_zTXt)
       {
          ni_png_warning(ni_png_ptr, "Unknown compression type in zTXt chunk");
          comp_type = PNG_TEXT_COMPRESSION_zTXt;
       }
       text++;        /* skip the compression_method byte */
   }
   prefix_len = text - chunkdata;

   chunkdata = (ni_png_charp)ni_png_decompress_chunk(ni_png_ptr, comp_type, chunkdata,
                                    (ni_png_size_t)length, prefix_len, &data_len);

   text_ptr = (ni_png_textp)ni_png_malloc_warn(ni_png_ptr,
     (ni_png_uint_32)ni_png_sizeof(ni_png_text));
   if (text_ptr == NULL)
   {
     ni_png_warning(ni_png_ptr,"Not enough memory to process zTXt chunk.");
     ni_png_free(ni_png_ptr, chunkdata);
     return;
   }
   text_ptr->compression = comp_type;
   text_ptr->key = chunkdata;
#ifdef PNG_iTXt_SUPPORTED
   text_ptr->lang = NULL;
   text_ptr->lang_key = NULL;
   text_ptr->itxt_length = 0;
#endif
   text_ptr->text = chunkdata + prefix_len;
   text_ptr->text_length = data_len;

   ret=ni_png_set_text_2(ni_png_ptr, info_ptr, text_ptr, 1);

   ni_png_free(ni_png_ptr, text_ptr);
   ni_png_free(ni_png_ptr, chunkdata);
   if (ret)
     ni_png_error(ni_png_ptr, "Insufficient memory to store zTXt chunk.");
}
#endif

#if defined(PNG_READ_iTXt_SUPPORTED)
/* note: this does not correctly handle chunks that are > 64K under DOS */
void /* PRIVATE */
ni_png_handle_iTXt(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_textp text_ptr;
   ni_png_charp chunkdata;
   ni_png_charp key, lang, text, lang_key;
   int comp_flag;
   int comp_type = 0;
   int ret;
   ni_png_size_t slength, prefix_len, data_len;

   ni_png_debug(1, "in ni_png_handle_iTXt\n");

   if (!(ni_png_ptr->mode & PNG_HAVE_IHDR))
      ni_png_error(ni_png_ptr, "Missing IHDR before iTXt");

   if (ni_png_ptr->mode & PNG_HAVE_IDAT)
      ni_png_ptr->mode |= PNG_AFTER_IDAT;

#ifdef PNG_MAX_MALLOC_64K
   /* We will no doubt have problems with chunks even half this size, but
      there is no hard and fast rule to tell us where to stop. */
   if (length > (ni_png_uint_32)65535L)
   {
     ni_png_warning(ni_png_ptr,"iTXt chunk too large to fit in memory");
     ni_png_crc_finish(ni_png_ptr, length);
     return;
   }
#endif

   chunkdata = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length + 1);
   if (chunkdata == NULL)
   {
     ni_png_warning(ni_png_ptr, "No memory to process iTXt chunk.");
     return;
   }
   slength = (ni_png_size_t)length;
   ni_png_crc_read(ni_png_ptr, (ni_png_bytep)chunkdata, slength);
   if (ni_png_crc_finish(ni_png_ptr, 0))
   {
      ni_png_free(ni_png_ptr, chunkdata);
      return;
   }

   chunkdata[slength] = 0x00;

   for (lang = chunkdata; *lang; lang++)
      /* empty loop */ ;
   lang++;        /* skip NUL separator */

   /* iTXt must have a language tag (possibly empty), two compression bytes,
      translated keyword (possibly empty), and possibly some text after the
      keyword */

   if (lang >= chunkdata + slength)
   {
      comp_flag = PNG_TEXT_COMPRESSION_NONE;
      ni_png_warning(ni_png_ptr, "Zero length iTXt chunk");
   }
   else
   {
       comp_flag = *lang++;
       comp_type = *lang++;
   }

   for (lang_key = lang; *lang_key; lang_key++)
      /* empty loop */ ;
   lang_key++;        /* skip NUL separator */

   for (text = lang_key; *text; text++)
      /* empty loop */ ;
   text++;        /* skip NUL separator */

   prefix_len = text - chunkdata;

   key=chunkdata;
   if (comp_flag)
       chunkdata = ni_png_decompress_chunk(ni_png_ptr, comp_type, chunkdata,
          (size_t)length, prefix_len, &data_len);
   else
       data_len=ni_png_strlen(chunkdata + prefix_len);
   text_ptr = (ni_png_textp)ni_png_malloc_warn(ni_png_ptr,
      (ni_png_uint_32)ni_png_sizeof(ni_png_text));
   if (text_ptr == NULL)
   {
     ni_png_warning(ni_png_ptr,"Not enough memory to process iTXt chunk.");
     ni_png_free(ni_png_ptr, chunkdata);
     return;
   }
   text_ptr->compression = (int)comp_flag + 1;
   text_ptr->lang_key = chunkdata+(lang_key-key);
   text_ptr->lang = chunkdata+(lang-key);
   text_ptr->itxt_length = data_len;
   text_ptr->text_length = 0;
   text_ptr->key = chunkdata;
   text_ptr->text = chunkdata + prefix_len;

   ret=ni_png_set_text_2(ni_png_ptr, info_ptr, text_ptr, 1);

   ni_png_free(ni_png_ptr, text_ptr);
   ni_png_free(ni_png_ptr, chunkdata);
   if (ret)
     ni_png_error(ni_png_ptr, "Insufficient memory to store iTXt chunk.");
}
#endif

/* This function is called when we haven't found a handler for a
   chunk.  If there isn't a problem with the chunk itself (ie bad
   chunk name, CRC, or a critical chunk), the chunk is silently ignored
   -- unless the PNG_FLAG_UNKNOWN_CHUNKS_SUPPORTED flag is on in which
   case it will be saved away to be written out later. */
void /* PRIVATE */
ni_png_handle_unknown(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_32 length)
{
   ni_png_uint_32 skip = 0;

   ni_png_debug(1, "in ni_png_handle_unknown\n");

   if (ni_png_ptr->mode & PNG_HAVE_IDAT)
   {
#ifdef PNG_USE_LOCAL_ARRAYS
      PNG_IDAT;
#endif
      if (ni_png_memcmp(ni_png_ptr->chunk_name, ni_png_IDAT, 4))  /* not an IDAT */
         ni_png_ptr->mode |= PNG_AFTER_IDAT;
   }

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
   }

#if defined(PNG_READ_UNKNOWN_CHUNKS_SUPPORTED)
   if ((ni_png_ptr->flags & PNG_FLAG_KEEP_UNKNOWN_CHUNKS) ||
       (ni_png_ptr->read_user_chunk_fn != NULL))
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
      skip = length;

   ni_png_crc_finish(ni_png_ptr, skip);

#if !defined(PNG_READ_USER_CHUNKS_SUPPORTED)
   if (&info_ptr == NULL) /* quiet compiler warnings about unused info_ptr */
      return;
#endif
}

/* This function is called to verify that a chunk name is valid.
   This function can't have the "critical chunk check" incorporated
   into it, since in the future we will need to be able to call user
   functions to handle unknown critical chunks after we check that
   the chunk name itself is valid. */

#define isnonalpha(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))

void /* PRIVATE */
ni_png_check_chunk_name(ni_png_structp ni_png_ptr, ni_png_bytep chunk_name)
{
   ni_png_debug(1, "in ni_png_check_chunk_name\n");
   if (isnonalpha(chunk_name[0]) || isnonalpha(chunk_name[1]) ||
       isnonalpha(chunk_name[2]) || isnonalpha(chunk_name[3]))
   {
      ni_png_chunk_error(ni_png_ptr, "invalid chunk type");
   }
}

/* Combines the row recently read in with the existing pixels in the
   row.  This routine takes care of alpha and transparency if requested.
   This routine also handles the two methods of progressive display
   of interlaced images, depending on the mask value.
   The mask value describes which pixels are to be combined with
   the row.  The pattern always repeats every 8 pixels, so just 8
   bits are needed.  A one indicates the pixel is to be combined,
   a zero indicates the pixel is to be skipped.  This is in addition
   to any alpha or transparency value associated with the pixel.  If
   you want all pixels to be combined, pass 0xff (255) in mask.  */
#ifndef PNG_HAVE_MMX_COMBINE_ROW
void /* PRIVATE */
ni_png_combine_row(ni_png_structp ni_png_ptr, ni_png_bytep row, int mask)
{
   ni_png_debug(1,"in ni_png_combine_row\n");
   if (mask == 0xff)
   {
      ni_png_memcpy(row, ni_png_ptr->row_buf + 1,
         PNG_ROWBYTES(ni_png_ptr->row_info.pixel_depth, ni_png_ptr->width));
   }
   else
   {
      switch (ni_png_ptr->row_info.pixel_depth)
      {
         case 1:
         {
            ni_png_bytep sp = ni_png_ptr->row_buf + 1;
            ni_png_bytep dp = row;
            int s_inc, s_start, s_end;
            int m = 0x80;
            int shift;
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = ni_png_ptr->width;

#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (ni_png_ptr->transformations & PNG_PACKSWAP)
            {
                s_start = 0;
                s_end = 7;
                s_inc = 1;
            }
            else
#endif
            {
                s_start = 7;
                s_end = 0;
                s_inc = -1;
            }

            shift = s_start;

            for (i = 0; i < row_width; i++)
            {
               if (m & mask)
               {
                  int value;

                  value = (*sp >> shift) & 0x01;
                  *dp &= (ni_png_byte)((0x7f7f >> (7 - shift)) & 0xff);
                  *dp |= (ni_png_byte)(value << shift);
               }

               if (shift == s_end)
               {
                  shift = s_start;
                  sp++;
                  dp++;
               }
               else
                  shift += s_inc;

               if (m == 1)
                  m = 0x80;
               else
                  m >>= 1;
            }
            break;
         }
         case 2:
         {
            ni_png_bytep sp = ni_png_ptr->row_buf + 1;
            ni_png_bytep dp = row;
            int s_start, s_end, s_inc;
            int m = 0x80;
            int shift;
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = ni_png_ptr->width;
            int value;

#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (ni_png_ptr->transformations & PNG_PACKSWAP)
            {
               s_start = 0;
               s_end = 6;
               s_inc = 2;
            }
            else
#endif
            {
               s_start = 6;
               s_end = 0;
               s_inc = -2;
            }

            shift = s_start;

            for (i = 0; i < row_width; i++)
            {
               if (m & mask)
               {
                  value = (*sp >> shift) & 0x03;
                  *dp &= (ni_png_byte)((0x3f3f >> (6 - shift)) & 0xff);
                  *dp |= (ni_png_byte)(value << shift);
               }

               if (shift == s_end)
               {
                  shift = s_start;
                  sp++;
                  dp++;
               }
               else
                  shift += s_inc;
               if (m == 1)
                  m = 0x80;
               else
                  m >>= 1;
            }
            break;
         }
         case 4:
         {
            ni_png_bytep sp = ni_png_ptr->row_buf + 1;
            ni_png_bytep dp = row;
            int s_start, s_end, s_inc;
            int m = 0x80;
            int shift;
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = ni_png_ptr->width;
            int value;

#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (ni_png_ptr->transformations & PNG_PACKSWAP)
            {
               s_start = 0;
               s_end = 4;
               s_inc = 4;
            }
            else
#endif
            {
               s_start = 4;
               s_end = 0;
               s_inc = -4;
            }
            shift = s_start;

            for (i = 0; i < row_width; i++)
            {
               if (m & mask)
               {
                  value = (*sp >> shift) & 0xf;
                  *dp &= (ni_png_byte)((0xf0f >> (4 - shift)) & 0xff);
                  *dp |= (ni_png_byte)(value << shift);
               }

               if (shift == s_end)
               {
                  shift = s_start;
                  sp++;
                  dp++;
               }
               else
                  shift += s_inc;
               if (m == 1)
                  m = 0x80;
               else
                  m >>= 1;
            }
            break;
         }
         default:
         {
            ni_png_bytep sp = ni_png_ptr->row_buf + 1;
            ni_png_bytep dp = row;
            ni_png_size_t pixel_bytes = (ni_png_ptr->row_info.pixel_depth >> 3);
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = ni_png_ptr->width;
            ni_png_byte m = 0x80;


            for (i = 0; i < row_width; i++)
            {
               if (m & mask)
               {
                  ni_png_memcpy(dp, sp, pixel_bytes);
               }

               sp += pixel_bytes;
               dp += pixel_bytes;

               if (m == 1)
                  m = 0x80;
               else
                  m >>= 1;
            }
            break;
         }
      }
   }
}
#endif /* !PNG_HAVE_MMX_COMBINE_ROW */

#ifdef PNG_READ_INTERLACING_SUPPORTED
#ifndef PNG_HAVE_MMX_READ_INTERLACE   /* else in pngvcrd.c, pnggccrd.c */
/* OLD pre-1.0.9 interface:
void ni_png_do_read_interlace(ni_png_row_infop row_info, ni_png_bytep row, int pass,
   ni_png_uint_32 transformations)
 */
void /* PRIVATE */
ni_png_do_read_interlace(ni_png_structp ni_png_ptr)
{
   ni_png_row_infop row_info = &(ni_png_ptr->row_info);
   ni_png_bytep row = ni_png_ptr->row_buf + 1;
   int pass = ni_png_ptr->pass;
   ni_png_uint_32 transformations = ni_png_ptr->transformations;
#ifdef PNG_USE_LOCAL_ARRAYS
   /* arrays to facilitate easy interlacing - use pass (0 - 6) as index */
   /* offset to next interlace block */
   const int ni_png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};
#endif

   ni_png_debug(1,"in ni_png_do_read_interlace (stock C version)\n");
   if (row != NULL && row_info != NULL)
   {
      ni_png_uint_32 final_width;

      final_width = row_info->width * ni_png_pass_inc[pass];

      switch (row_info->pixel_depth)
      {
         case 1:
         {
            ni_png_bytep sp = row + (ni_png_size_t)((row_info->width - 1) >> 3);
            ni_png_bytep dp = row + (ni_png_size_t)((final_width - 1) >> 3);
            int sshift, dshift;
            int s_start, s_end, s_inc;
            int jstop = ni_png_pass_inc[pass];
            ni_png_byte v;
            ni_png_uint_32 i;
            int j;

#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (transformations & PNG_PACKSWAP)
            {
                sshift = (int)((row_info->width + 7) & 0x07);
                dshift = (int)((final_width + 7) & 0x07);
                s_start = 7;
                s_end = 0;
                s_inc = -1;
            }
            else
#endif
            {
                sshift = 7 - (int)((row_info->width + 7) & 0x07);
                dshift = 7 - (int)((final_width + 7) & 0x07);
                s_start = 0;
                s_end = 7;
                s_inc = 1;
            }

            for (i = 0; i < row_info->width; i++)
            {
               v = (ni_png_byte)((*sp >> sshift) & 0x01);
               for (j = 0; j < jstop; j++)
               {
                  *dp &= (ni_png_byte)((0x7f7f >> (7 - dshift)) & 0xff);
                  *dp |= (ni_png_byte)(v << dshift);
                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }
                  else
                     dshift += s_inc;
               }
               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }
               else
                  sshift += s_inc;
            }
            break;
         }
         case 2:
         {
            ni_png_bytep sp = row + (ni_png_uint_32)((row_info->width - 1) >> 2);
            ni_png_bytep dp = row + (ni_png_uint_32)((final_width - 1) >> 2);
            int sshift, dshift;
            int s_start, s_end, s_inc;
            int jstop = ni_png_pass_inc[pass];
            ni_png_uint_32 i;

#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (transformations & PNG_PACKSWAP)
            {
               sshift = (int)(((row_info->width + 3) & 0x03) << 1);
               dshift = (int)(((final_width + 3) & 0x03) << 1);
               s_start = 6;
               s_end = 0;
               s_inc = -2;
            }
            else
#endif
            {
               sshift = (int)((3 - ((row_info->width + 3) & 0x03)) << 1);
               dshift = (int)((3 - ((final_width + 3) & 0x03)) << 1);
               s_start = 0;
               s_end = 6;
               s_inc = 2;
            }

            for (i = 0; i < row_info->width; i++)
            {
               ni_png_byte v;
               int j;

               v = (ni_png_byte)((*sp >> sshift) & 0x03);
               for (j = 0; j < jstop; j++)
               {
                  *dp &= (ni_png_byte)((0x3f3f >> (6 - dshift)) & 0xff);
                  *dp |= (ni_png_byte)(v << dshift);
                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }
                  else
                     dshift += s_inc;
               }
               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }
               else
                  sshift += s_inc;
            }
            break;
         }
         case 4:
         {
            ni_png_bytep sp = row + (ni_png_size_t)((row_info->width - 1) >> 1);
            ni_png_bytep dp = row + (ni_png_size_t)((final_width - 1) >> 1);
            int sshift, dshift;
            int s_start, s_end, s_inc;
            ni_png_uint_32 i;
            int jstop = ni_png_pass_inc[pass];

#if defined(PNG_READ_PACKSWAP_SUPPORTED)
            if (transformations & PNG_PACKSWAP)
            {
               sshift = (int)(((row_info->width + 1) & 0x01) << 2);
               dshift = (int)(((final_width + 1) & 0x01) << 2);
               s_start = 4;
               s_end = 0;
               s_inc = -4;
            }
            else
#endif
            {
               sshift = (int)((1 - ((row_info->width + 1) & 0x01)) << 2);
               dshift = (int)((1 - ((final_width + 1) & 0x01)) << 2);
               s_start = 0;
               s_end = 4;
               s_inc = 4;
            }

            for (i = 0; i < row_info->width; i++)
            {
               ni_png_byte v = (ni_png_byte)((*sp >> sshift) & 0xf);
               int j;

               for (j = 0; j < jstop; j++)
               {
                  *dp &= (ni_png_byte)((0xf0f >> (4 - dshift)) & 0xff);
                  *dp |= (ni_png_byte)(v << dshift);
                  if (dshift == s_end)
                  {
                     dshift = s_start;
                     dp--;
                  }
                  else
                     dshift += s_inc;
               }
               if (sshift == s_end)
               {
                  sshift = s_start;
                  sp--;
               }
               else
                  sshift += s_inc;
            }
            break;
         }
         default:
         {
            ni_png_size_t pixel_bytes = (row_info->pixel_depth >> 3);
            ni_png_bytep sp = row + (ni_png_size_t)(row_info->width - 1) * pixel_bytes;
            ni_png_bytep dp = row + (ni_png_size_t)(final_width - 1) * pixel_bytes;

            int jstop = ni_png_pass_inc[pass];
            ni_png_uint_32 i;

            for (i = 0; i < row_info->width; i++)
            {
               ni_png_byte v[8];
               int j;

               ni_png_memcpy(v, sp, pixel_bytes);
               for (j = 0; j < jstop; j++)
               {
                  ni_png_memcpy(dp, v, pixel_bytes);
                  dp -= pixel_bytes;
               }
               sp -= pixel_bytes;
            }
            break;
         }
      }
      row_info->width = final_width;
      row_info->rowbytes = PNG_ROWBYTES(row_info->pixel_depth,final_width);
   }
#if !defined(PNG_READ_PACKSWAP_SUPPORTED)
   if (&transformations == NULL) /* silence compiler warning */
      return;
#endif
}
#endif /* !PNG_HAVE_MMX_READ_INTERLACE */
#endif /* PNG_READ_INTERLACING_SUPPORTED */

#ifndef PNG_HAVE_MMX_READ_FILTER_ROW
void /* PRIVATE */
ni_png_read_filter_row(ni_png_structp ni_png_ptr, ni_png_row_infop row_info, ni_png_bytep row,
   ni_png_bytep prev_row, int filter)
{
   ni_png_debug(1, "in ni_png_read_filter_row\n");
   ni_png_debug2(2,"row = %lu, filter = %d\n", ni_png_ptr->row_number, filter);
   switch (filter)
   {
      case PNG_FILTER_VALUE_NONE:
         break;
      case PNG_FILTER_VALUE_SUB:
      {
         ni_png_uint_32 i;
         ni_png_uint_32 istop = row_info->rowbytes;
         ni_png_uint_32 bpp = (row_info->pixel_depth + 7) >> 3;
         ni_png_bytep rp = row + bpp;
         ni_png_bytep lp = row;

         for (i = bpp; i < istop; i++)
         {
            *rp = (ni_png_byte)(((int)(*rp) + (int)(*lp++)) & 0xff);
            rp++;
         }
         break;
      }
      case PNG_FILTER_VALUE_UP:
      {
         ni_png_uint_32 i;
         ni_png_uint_32 istop = row_info->rowbytes;
         ni_png_bytep rp = row;
         ni_png_bytep pp = prev_row;

         for (i = 0; i < istop; i++)
         {
            *rp = (ni_png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
            rp++;
         }
         break;
      }
      case PNG_FILTER_VALUE_AVG:
      {
         ni_png_uint_32 i;
         ni_png_bytep rp = row;
         ni_png_bytep pp = prev_row;
         ni_png_bytep lp = row;
         ni_png_uint_32 bpp = (row_info->pixel_depth + 7) >> 3;
         ni_png_uint_32 istop = row_info->rowbytes - bpp;

         for (i = 0; i < bpp; i++)
         {
            *rp = (ni_png_byte)(((int)(*rp) +
               ((int)(*pp++) / 2 )) & 0xff);
            rp++;
         }

         for (i = 0; i < istop; i++)
         {
            *rp = (ni_png_byte)(((int)(*rp) +
               (int)(*pp++ + *lp++) / 2 ) & 0xff);
            rp++;
         }
         break;
      }
      case PNG_FILTER_VALUE_PAETH:
      {
         ni_png_uint_32 i;
         ni_png_bytep rp = row;
         ni_png_bytep pp = prev_row;
         ni_png_bytep lp = row;
         ni_png_bytep cp = prev_row;
         ni_png_uint_32 bpp = (row_info->pixel_depth + 7) >> 3;
         ni_png_uint_32 istop=row_info->rowbytes - bpp;

         for (i = 0; i < bpp; i++)
         {
            *rp = (ni_png_byte)(((int)(*rp) + (int)(*pp++)) & 0xff);
            rp++;
         }

         for (i = 0; i < istop; i++)   /* use leftover rp,pp */
         {
            int a, b, c, pa, pb, pc, p;

            a = *lp++;
            b = *pp++;
            c = *cp++;

            p = b - c;
            pc = a - c;

#ifdef PNG_USE_ABS
            pa = abs(p);
            pb = abs(pc);
            pc = abs(p + pc);
#else
            pa = p < 0 ? -p : p;
            pb = pc < 0 ? -pc : pc;
            pc = (p + pc) < 0 ? -(p + pc) : p + pc;
#endif

            /*
               if (pa <= pb && pa <= pc)
                  p = a;
               else if (pb <= pc)
                  p = b;
               else
                  p = c;
             */

            p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;

            *rp = (ni_png_byte)(((int)(*rp) + p) & 0xff);
            rp++;
         }
         break;
      }
      default:
         ni_png_warning(ni_png_ptr, "Ignoring bad adaptive filter type");
         *row=0;
         break;
   }
}
#endif /* !PNG_HAVE_MMX_READ_FILTER_ROW */

void /* PRIVATE */
ni_png_read_finish_row(ni_png_structp ni_png_ptr)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   /* arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* start of interlace block */
   const int ni_png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* offset to next interlace block */
   const int ni_png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   /* start of interlace block in the y direction */
   const int ni_png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};

   /* offset to next interlace block in the y direction */
   const int ni_png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif

   ni_png_debug(1, "in ni_png_read_finish_row\n");
   ni_png_ptr->row_number++;
   if (ni_png_ptr->row_number < ni_png_ptr->num_rows)
      return;

   if (ni_png_ptr->interlaced)
   {
      ni_png_ptr->row_number = 0;
      ni_png_memset_check(ni_png_ptr, ni_png_ptr->prev_row, 0, ni_png_ptr->rowbytes + 1);
      do
      {
         ni_png_ptr->pass++;
         if (ni_png_ptr->pass >= 7)
            break;
         ni_png_ptr->iwidth = (ni_png_ptr->width +
            ni_png_pass_inc[ni_png_ptr->pass] - 1 -
            ni_png_pass_start[ni_png_ptr->pass]) /
            ni_png_pass_inc[ni_png_ptr->pass];

         ni_png_ptr->irowbytes = PNG_ROWBYTES(ni_png_ptr->pixel_depth,
            ni_png_ptr->iwidth) + 1;

         if (!(ni_png_ptr->transformations & PNG_INTERLACE))
         {
            ni_png_ptr->num_rows = (ni_png_ptr->height +
               ni_png_pass_yinc[ni_png_ptr->pass] - 1 -
               ni_png_pass_ystart[ni_png_ptr->pass]) /
               ni_png_pass_yinc[ni_png_ptr->pass];
            if (!(ni_png_ptr->num_rows))
               continue;
         }
         else  /* if (ni_png_ptr->transformations & PNG_INTERLACE) */
            break;
      } while (ni_png_ptr->iwidth == 0);

      if (ni_png_ptr->pass < 7)
         return;
   }

   if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_FINISHED))
   {
#ifdef PNG_USE_LOCAL_ARRAYS
      PNG_IDAT;
#endif
      char extra;
      int ret;

      ni_png_ptr->zstream.next_out = (Byte *)&extra;
      ni_png_ptr->zstream.avail_out = (uInt)1;
      for(;;)
      {
         if (!(ni_png_ptr->zstream.avail_in))
         {
            while (!ni_png_ptr->idat_size)
            {
               ni_png_byte chunk_length[4];

               ni_png_crc_finish(ni_png_ptr, 0);

               ni_png_read_data(ni_png_ptr, chunk_length, 4);
               ni_png_ptr->idat_size = ni_png_get_uint_31(ni_png_ptr, chunk_length);
               ni_png_reset_crc(ni_png_ptr);
               ni_png_crc_read(ni_png_ptr, ni_png_ptr->chunk_name, 4);
               if (ni_png_memcmp(ni_png_ptr->chunk_name, (ni_png_bytep)ni_png_IDAT, 4))
                  ni_png_error(ni_png_ptr, "Not enough image data");

            }
            ni_png_ptr->zstream.avail_in = (uInt)ni_png_ptr->zbuf_size;
            ni_png_ptr->zstream.next_in = ni_png_ptr->zbuf;
            if (ni_png_ptr->zbuf_size > ni_png_ptr->idat_size)
               ni_png_ptr->zstream.avail_in = (uInt)ni_png_ptr->idat_size;
            ni_png_crc_read(ni_png_ptr, ni_png_ptr->zbuf, ni_png_ptr->zstream.avail_in);
            ni_png_ptr->idat_size -= ni_png_ptr->zstream.avail_in;
         }
         ret = inflate(&ni_png_ptr->zstream, Z_PARTIAL_FLUSH);
         if (ret == Z_STREAM_END)
         {
            if (!(ni_png_ptr->zstream.avail_out) || ni_png_ptr->zstream.avail_in ||
               ni_png_ptr->idat_size)
               ni_png_warning(ni_png_ptr, "Extra compressed data");
            ni_png_ptr->mode |= PNG_AFTER_IDAT;
            ni_png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
            break;
         }
         if (ret != Z_OK)
            ni_png_error(ni_png_ptr, ni_png_ptr->zstream.msg ? ni_png_ptr->zstream.msg :
                      "Decompression Error");

         if (!(ni_png_ptr->zstream.avail_out))
         {
            ni_png_warning(ni_png_ptr, "Extra compressed data.");
            ni_png_ptr->mode |= PNG_AFTER_IDAT;
            ni_png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
            break;
         }

      }
      ni_png_ptr->zstream.avail_out = 0;
   }

   if (ni_png_ptr->idat_size || ni_png_ptr->zstream.avail_in)
      ni_png_warning(ni_png_ptr, "Extra compression data");

   inflateReset(&ni_png_ptr->zstream);

   ni_png_ptr->mode |= PNG_AFTER_IDAT;
}

void /* PRIVATE */
ni_png_read_start_row(ni_png_structp ni_png_ptr)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   /* arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* start of interlace block */
   const int ni_png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* offset to next interlace block */
   const int ni_png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   /* start of interlace block in the y direction */
   const int ni_png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};

   /* offset to next interlace block in the y direction */
   const int ni_png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif

   int max_pixel_depth;
   ni_png_uint_32 row_bytes;

   ni_png_debug(1, "in ni_png_read_start_row\n");
   ni_png_ptr->zstream.avail_in = 0;
   ni_png_init_read_transformations(ni_png_ptr);
   if (ni_png_ptr->interlaced)
   {
      if (!(ni_png_ptr->transformations & PNG_INTERLACE))
         ni_png_ptr->num_rows = (ni_png_ptr->height + ni_png_pass_yinc[0] - 1 -
            ni_png_pass_ystart[0]) / ni_png_pass_yinc[0];
      else
         ni_png_ptr->num_rows = ni_png_ptr->height;

      ni_png_ptr->iwidth = (ni_png_ptr->width +
         ni_png_pass_inc[ni_png_ptr->pass] - 1 -
         ni_png_pass_start[ni_png_ptr->pass]) /
         ni_png_pass_inc[ni_png_ptr->pass];

         row_bytes = PNG_ROWBYTES(ni_png_ptr->pixel_depth,ni_png_ptr->iwidth) + 1;

         ni_png_ptr->irowbytes = (ni_png_size_t)row_bytes;
         if((ni_png_uint_32)ni_png_ptr->irowbytes != row_bytes)
            ni_png_error(ni_png_ptr, "Rowbytes overflow in ni_png_read_start_row");
   }
   else
   {
      ni_png_ptr->num_rows = ni_png_ptr->height;
      ni_png_ptr->iwidth = ni_png_ptr->width;
      ni_png_ptr->irowbytes = ni_png_ptr->rowbytes + 1;
   }
   max_pixel_depth = ni_png_ptr->pixel_depth;

#if defined(PNG_READ_PACK_SUPPORTED)
   if ((ni_png_ptr->transformations & PNG_PACK) && ni_png_ptr->bit_depth < 8)
      max_pixel_depth = 8;
#endif

#if defined(PNG_READ_EXPAND_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_EXPAND)
   {
      if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      {
         if (ni_png_ptr->num_trans)
            max_pixel_depth = 32;
         else
            max_pixel_depth = 24;
      }
      else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
      {
         if (max_pixel_depth < 8)
            max_pixel_depth = 8;
         if (ni_png_ptr->num_trans)
            max_pixel_depth *= 2;
      }
      else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_RGB)
      {
         if (ni_png_ptr->num_trans)
         {
            max_pixel_depth *= 4;
            max_pixel_depth /= 3;
         }
      }
   }
#endif

#if defined(PNG_READ_FILLER_SUPPORTED)
   if (ni_png_ptr->transformations & (PNG_FILLER))
   {
      if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
         max_pixel_depth = 32;
      else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
      {
         if (max_pixel_depth <= 8)
            max_pixel_depth = 16;
         else
            max_pixel_depth = 32;
      }
      else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_RGB)
      {
         if (max_pixel_depth <= 32)
            max_pixel_depth = 32;
         else
            max_pixel_depth = 64;
      }
   }
#endif

#if defined(PNG_READ_GRAY_TO_RGB_SUPPORTED)
   if (ni_png_ptr->transformations & PNG_GRAY_TO_RGB)
   {
      if (
#if defined(PNG_READ_EXPAND_SUPPORTED)
        (ni_png_ptr->num_trans && (ni_png_ptr->transformations & PNG_EXPAND)) ||
#endif
#if defined(PNG_READ_FILLER_SUPPORTED)
        (ni_png_ptr->transformations & (PNG_FILLER)) ||
#endif
        ni_png_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      {
         if (max_pixel_depth <= 16)
            max_pixel_depth = 32;
         else
            max_pixel_depth = 64;
      }
      else
      {
         if (max_pixel_depth <= 8)
           {
             if (ni_png_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
               max_pixel_depth = 32;
             else
               max_pixel_depth = 24;
           }
         else if (ni_png_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            max_pixel_depth = 64;
         else
            max_pixel_depth = 48;
      }
   }
#endif

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) && \
defined(PNG_USER_TRANSFORM_PTR_SUPPORTED)
   if(ni_png_ptr->transformations & PNG_USER_TRANSFORM)
     {
       int user_pixel_depth=ni_png_ptr->user_transform_depth*
         ni_png_ptr->user_transform_channels;
       if(user_pixel_depth > max_pixel_depth)
         max_pixel_depth=user_pixel_depth;
     }
#endif

   /* align the width on the next larger 8 pixels.  Mainly used
      for interlacing */
   row_bytes = ((ni_png_ptr->width + 7) & ~((ni_png_uint_32)7));
   /* calculate the maximum bytes needed, adding a byte and a pixel
      for safety's sake */
   row_bytes = PNG_ROWBYTES(max_pixel_depth,row_bytes) +
      1 + ((max_pixel_depth + 7) >> 3);
#ifdef PNG_MAX_MALLOC_64K
   if (row_bytes > (ni_png_uint_32)65536L)
      ni_png_error(ni_png_ptr, "This image requires a row greater than 64KB");
#endif
   ni_png_ptr->big_row_buf = (ni_png_bytep)ni_png_malloc(ni_png_ptr, row_bytes+64);
   ni_png_ptr->row_buf = ni_png_ptr->big_row_buf+32;
#if defined(PNG_DEBUG) && defined(PNG_USE_PNGGCCRD)
   ni_png_ptr->row_buf_size = row_bytes;
#endif

#ifdef PNG_MAX_MALLOC_64K
   if ((ni_png_uint_32)ni_png_ptr->rowbytes + 1 > (ni_png_uint_32)65536L)
      ni_png_error(ni_png_ptr, "This image requires a row greater than 64KB");
#endif
   if ((ni_png_uint_32)ni_png_ptr->rowbytes > (ni_png_uint_32)(PNG_SIZE_MAX - 1))
      ni_png_error(ni_png_ptr, "Row has too many bytes to allocate in memory.");
   ni_png_ptr->prev_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr, (ni_png_uint_32)(
      ni_png_ptr->rowbytes + 1));

   ni_png_memset_check(ni_png_ptr, ni_png_ptr->prev_row, 0, ni_png_ptr->rowbytes + 1);

   ni_png_debug1(3, "width = %lu,\n", ni_png_ptr->width);
   ni_png_debug1(3, "height = %lu,\n", ni_png_ptr->height);
   ni_png_debug1(3, "iwidth = %lu,\n", ni_png_ptr->iwidth);
   ni_png_debug1(3, "num_rows = %lu\n", ni_png_ptr->num_rows);
   ni_png_debug1(3, "rowbytes = %lu,\n", ni_png_ptr->rowbytes);
   ni_png_debug1(3, "irowbytes = %lu,\n", ni_png_ptr->irowbytes);

   ni_png_ptr->flags |= PNG_FLAG_ROW_INIT;
}
#endif /* PNG_READ_SUPPORTED */
