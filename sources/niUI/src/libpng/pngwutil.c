
/* pngwutil.c - utilities to write a PNG file
 *
 * Last changed in libpng 1.2.15 January 5, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 */

#define PNG_INTERNAL
#include "png.h"
#ifdef PNG_WRITE_SUPPORTED

/* Place a 32-bit number into a buffer in PNG byte order.  We work
 * with unsigned numbers for convenience, although one supported
 * ancillary chunk uses signed (two's complement) numbers.
 */
void PNGAPI
ni_png_save_uint_32(ni_png_bytep buf, ni_png_uint_32 i)
{
   buf[0] = (ni_png_byte)((i >> 24) & 0xff);
   buf[1] = (ni_png_byte)((i >> 16) & 0xff);
   buf[2] = (ni_png_byte)((i >> 8) & 0xff);
   buf[3] = (ni_png_byte)(i & 0xff);
}

/* The ni_png_save_int_32 function assumes integers are stored in two's
 * complement format.  If this isn't the case, then this routine needs to
 * be modified to write data in two's complement format.
 */
void PNGAPI
ni_png_save_int_32(ni_png_bytep buf, ni_png_int_32 i)
{
   buf[0] = (ni_png_byte)((i >> 24) & 0xff);
   buf[1] = (ni_png_byte)((i >> 16) & 0xff);
   buf[2] = (ni_png_byte)((i >> 8) & 0xff);
   buf[3] = (ni_png_byte)(i & 0xff);
}

/* Place a 16-bit number into a buffer in PNG byte order.
 * The parameter is declared unsigned int, not ni_png_uint_16,
 * just to avoid potential problems on pre-ANSI C compilers.
 */
void PNGAPI
ni_png_save_uint_16(ni_png_bytep buf, unsigned int i)
{
   buf[0] = (ni_png_byte)((i >> 8) & 0xff);
   buf[1] = (ni_png_byte)(i & 0xff);
}

/* Write a PNG chunk all at once.  The type is an array of ASCII characters
 * representing the chunk name.  The array must be at least 4 bytes in
 * length, and does not need to be null terminated.  To be safe, pass the
 * pre-defined chunk names here, and if you need a new one, define it
 * where the others are defined.  The length is the length of the data.
 * All the data must be present.  If that is not possible, use the
 * ni_png_write_chunk_start(), ni_png_write_chunk_data(), and ni_png_write_chunk_end()
 * functions instead.
 */
void PNGAPI
ni_png_write_chunk(ni_png_structp ni_png_ptr, ni_png_bytep chunk_name,
   ni_png_bytep data, ni_png_size_t length)
{
   if(ni_png_ptr == NULL) return;
   ni_png_write_chunk_start(ni_png_ptr, chunk_name, (ni_png_uint_32)length);
   ni_png_write_chunk_data(ni_png_ptr, data, length);
   ni_png_write_chunk_end(ni_png_ptr);
}

/* Write the start of a PNG chunk.  The type is the chunk type.
 * The total_length is the sum of the lengths of all the data you will be
 * passing in ni_png_write_chunk_data().
 */
void PNGAPI
ni_png_write_chunk_start(ni_png_structp ni_png_ptr, ni_png_bytep chunk_name,
   ni_png_uint_32 length)
{
   ni_png_byte buf[4];
   ni_png_debug2(0, "Writing %s chunk (%lu bytes)\n", chunk_name, length);
   if(ni_png_ptr == NULL) return;

   /* write the length */
   ni_png_save_uint_32(buf, length);
   ni_png_write_data(ni_png_ptr, buf, (ni_png_size_t)4);

   /* write the chunk name */
   ni_png_write_data(ni_png_ptr, chunk_name, (ni_png_size_t)4);
   /* reset the crc and run it over the chunk name */
   ni_png_reset_crc(ni_png_ptr);
   ni_png_calculate_crc(ni_png_ptr, chunk_name, (ni_png_size_t)4);
}

/* Write the data of a PNG chunk started with ni_png_write_chunk_start().
 * Note that multiple calls to this function are allowed, and that the
 * sum of the lengths from these calls *must* add up to the total_length
 * given to ni_png_write_chunk_start().
 */
void PNGAPI
ni_png_write_chunk_data(ni_png_structp ni_png_ptr, ni_png_bytep data, ni_png_size_t length)
{
   /* write the data, and run the CRC over it */
   if(ni_png_ptr == NULL) return;
   if (data != NULL && length > 0)
   {
      ni_png_calculate_crc(ni_png_ptr, data, length);
      ni_png_write_data(ni_png_ptr, data, length);
   }
}

/* Finish a chunk started with ni_png_write_chunk_start(). */
void PNGAPI
ni_png_write_chunk_end(ni_png_structp ni_png_ptr)
{
   ni_png_byte buf[4];

   if(ni_png_ptr == NULL) return;

   /* write the crc */
   ni_png_save_uint_32(buf, ni_png_ptr->crc);

   ni_png_write_data(ni_png_ptr, buf, (ni_png_size_t)4);
}

/* Simple function to write the signature.  If we have already written
 * the magic bytes of the signature, or more likely, the PNG stream is
 * being embedded into another stream and doesn't need its own signature,
 * we should call ni_png_set_sig_bytes() to tell libpng how many of the
 * bytes have already been written.
 */
void /* PRIVATE */
ni_png_write_sig(ni_png_structp ni_png_ptr)
{
   ni_png_byte ni_png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
   /* write the rest of the 8 byte signature */
   ni_png_write_data(ni_png_ptr, &ni_png_signature[ni_png_ptr->sig_bytes],
      (ni_png_size_t)8 - ni_png_ptr->sig_bytes);
   if(ni_png_ptr->sig_bytes < 3)
      ni_png_ptr->mode |= PNG_HAVE_PNG_SIGNATURE;
}

#if defined(PNG_WRITE_TEXT_SUPPORTED) || defined(PNG_WRITE_iCCP_SUPPORTED)
/*
 * This pair of functions encapsulates the operation of (a) compressing a
 * text string, and (b) issuing it later as a series of chunk data writes.
 * The compression_state structure is shared context for these functions
 * set up by the caller in order to make the whole mess thread-safe.
 */

typedef struct
{
    char *input;   /* the uncompressed input data */
    int input_len;   /* its length */
    int num_output_ptr; /* number of output pointers used */
    int max_output_ptr; /* size of output_ptr */
    ni_png_charpp output_ptr; /* array of pointers to output */
} compression_state;

/* compress given text into storage in the ni_png_ptr structure */
static int /* PRIVATE */
ni_png_text_compress(ni_png_structp ni_png_ptr,
        ni_png_charp text, ni_png_size_t text_len, int compression,
        compression_state *comp)
{
   int ret;

   comp->num_output_ptr = 0;
   comp->max_output_ptr = 0;
   comp->output_ptr = NULL;
   comp->input = NULL;
   comp->input_len = 0;

   /* we may just want to pass the text right through */
   if (compression == PNG_TEXT_COMPRESSION_NONE)
   {
       comp->input = text;
       comp->input_len = text_len;
       return((int)text_len);
   }

   if (compression >= PNG_TEXT_COMPRESSION_LAST)
   {
#if !defined(PNG_NO_STDIO) && !defined(_WIN32_WCE)
      char msg[50];
      sprintf(msg, "Unknown compression type %d", compression);
      ni_png_warning(ni_png_ptr, msg);
#else
      ni_png_warning(ni_png_ptr, "Unknown compression type");
#endif
   }

   /* We can't write the chunk until we find out how much data we have,
    * which means we need to run the compressor first and save the
    * output.  This shouldn't be a problem, as the vast majority of
    * comments should be reasonable, but we will set up an array of
    * malloc'd pointers to be sure.
    *
    * If we knew the application was well behaved, we could simplify this
    * greatly by assuming we can always malloc an output buffer large
    * enough to hold the compressed text ((1001 * text_len / 1000) + 12)
    * and malloc this directly.  The only time this would be a bad idea is
    * if we can't malloc more than 64K and we have 64K of random input
    * data, or if the input string is incredibly large (although this
    * wouldn't cause a failure, just a slowdown due to swapping).
    */

   /* set up the compression buffers */
   ni_png_ptr->zstream.avail_in = (uInt)text_len;
   ni_png_ptr->zstream.next_in = (Bytef *)text;
   ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
   ni_png_ptr->zstream.next_out = (Bytef *)ni_png_ptr->zbuf;

   /* this is the same compression loop as in ni_png_write_row() */
   do
   {
      /* compress the data */
      ret = deflate(&ni_png_ptr->zstream, Z_NO_FLUSH);
      if (ret != Z_OK)
      {
         /* error */
         if (ni_png_ptr->zstream.msg != NULL)
            ni_png_error(ni_png_ptr, ni_png_ptr->zstream.msg);
         else
            ni_png_error(ni_png_ptr, "zlib error");
      }
      /* check to see if we need more room */
      if (!(ni_png_ptr->zstream.avail_out))
      {
         /* make sure the output array has room */
         if (comp->num_output_ptr >= comp->max_output_ptr)
         {
            int old_max;

            old_max = comp->max_output_ptr;
            comp->max_output_ptr = comp->num_output_ptr + 4;
            if (comp->output_ptr != NULL)
            {
               ni_png_charpp old_ptr;

               old_ptr = comp->output_ptr;
               comp->output_ptr = (ni_png_charpp)ni_png_malloc(ni_png_ptr,
                  (ni_png_uint_32)(comp->max_output_ptr *
                  ni_png_sizeof (ni_png_charpp)));
               ni_png_memcpy(comp->output_ptr, old_ptr, old_max
                  * ni_png_sizeof (ni_png_charp));
               ni_png_free(ni_png_ptr, old_ptr);
            }
            else
               comp->output_ptr = (ni_png_charpp)ni_png_malloc(ni_png_ptr,
                  (ni_png_uint_32)(comp->max_output_ptr *
                  ni_png_sizeof (ni_png_charp)));
         }

         /* save the data */
         comp->output_ptr[comp->num_output_ptr] = (ni_png_charp)ni_png_malloc(ni_png_ptr,
            (ni_png_uint_32)ni_png_ptr->zbuf_size);
         ni_png_memcpy(comp->output_ptr[comp->num_output_ptr], ni_png_ptr->zbuf,
            ni_png_ptr->zbuf_size);
         comp->num_output_ptr++;

         /* and reset the buffer */
         ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
         ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
      }
   /* continue until we don't have any more to compress */
   } while (ni_png_ptr->zstream.avail_in);

   /* finish the compression */
   do
   {
      /* tell zlib we are finished */
      ret = deflate(&ni_png_ptr->zstream, Z_FINISH);

      if (ret == Z_OK)
      {
         /* check to see if we need more room */
         if (!(ni_png_ptr->zstream.avail_out))
         {
            /* check to make sure our output array has room */
            if (comp->num_output_ptr >= comp->max_output_ptr)
            {
               int old_max;

               old_max = comp->max_output_ptr;
               comp->max_output_ptr = comp->num_output_ptr + 4;
               if (comp->output_ptr != NULL)
               {
                  ni_png_charpp old_ptr;

                  old_ptr = comp->output_ptr;
                  /* This could be optimized to realloc() */
                  comp->output_ptr = (ni_png_charpp)ni_png_malloc(ni_png_ptr,
                     (ni_png_uint_32)(comp->max_output_ptr *
                     ni_png_sizeof (ni_png_charpp)));
                  ni_png_memcpy(comp->output_ptr, old_ptr,
                     old_max * ni_png_sizeof (ni_png_charp));
                  ni_png_free(ni_png_ptr, old_ptr);
               }
               else
                  comp->output_ptr = (ni_png_charpp)ni_png_malloc(ni_png_ptr,
                     (ni_png_uint_32)(comp->max_output_ptr *
                     ni_png_sizeof (ni_png_charp)));
            }

            /* save off the data */
            comp->output_ptr[comp->num_output_ptr] =
               (ni_png_charp)ni_png_malloc(ni_png_ptr, (ni_png_uint_32)ni_png_ptr->zbuf_size);
            ni_png_memcpy(comp->output_ptr[comp->num_output_ptr], ni_png_ptr->zbuf,
               ni_png_ptr->zbuf_size);
            comp->num_output_ptr++;

            /* and reset the buffer pointers */
            ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
            ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
         }
      }
      else if (ret != Z_STREAM_END)
      {
         /* we got an error */
         if (ni_png_ptr->zstream.msg != NULL)
            ni_png_error(ni_png_ptr, ni_png_ptr->zstream.msg);
         else
            ni_png_error(ni_png_ptr, "zlib error");
      }
   } while (ret != Z_STREAM_END);

   /* text length is number of buffers plus last buffer */
   text_len = ni_png_ptr->zbuf_size * comp->num_output_ptr;
   if (ni_png_ptr->zstream.avail_out < ni_png_ptr->zbuf_size)
      text_len += ni_png_ptr->zbuf_size - (ni_png_size_t)ni_png_ptr->zstream.avail_out;

   return((int)text_len);
}

/* ship the compressed text out via chunk writes */
static void /* PRIVATE */
ni_png_write_compressed_data_out(ni_png_structp ni_png_ptr, compression_state *comp)
{
   int i;

   /* handle the no-compression case */
   if (comp->input)
   {
       ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)comp->input,
                            (ni_png_size_t)comp->input_len);
       return;
   }

   /* write saved output buffers, if any */
   for (i = 0; i < comp->num_output_ptr; i++)
   {
      ni_png_write_chunk_data(ni_png_ptr,(ni_png_bytep)comp->output_ptr[i],
         ni_png_ptr->zbuf_size);
      ni_png_free(ni_png_ptr, comp->output_ptr[i]);
      comp->output_ptr[i]=NULL;
   }
   if (comp->max_output_ptr != 0)
      ni_png_free(ni_png_ptr, comp->output_ptr);
      comp->output_ptr=NULL;
   /* write anything left in zbuf */
   if (ni_png_ptr->zstream.avail_out < (ni_png_uint_32)ni_png_ptr->zbuf_size)
      ni_png_write_chunk_data(ni_png_ptr, ni_png_ptr->zbuf,
         ni_png_ptr->zbuf_size - ni_png_ptr->zstream.avail_out);

   /* reset zlib for another zTXt/iTXt or image data */
   deflateReset(&ni_png_ptr->zstream);
   ni_png_ptr->zstream.data_type = Z_BINARY;
}
#endif

/* Write the IHDR chunk, and update the ni_png_struct with the necessary
 * information.  Note that the rest of this code depends upon this
 * information being correct.
 */
void /* PRIVATE */
ni_png_write_IHDR(ni_png_structp ni_png_ptr, ni_png_uint_32 width, ni_png_uint_32 height,
   int bit_depth, int color_type, int compression_type, int filter_type,
   int interlace_type)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_IHDR;
#endif
   ni_png_byte buf[13]; /* buffer to store the IHDR info */

   ni_png_debug(1, "in ni_png_write_IHDR\n");
   /* Check that we have valid input data from the application info */
   switch (color_type)
   {
      case PNG_COLOR_TYPE_GRAY:
         switch (bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8:
            case 16: ni_png_ptr->channels = 1; break;
            default: ni_png_error(ni_png_ptr,"Invalid bit depth for grayscale image");
         }
         break;
      case PNG_COLOR_TYPE_RGB:
         if (bit_depth != 8 && bit_depth != 16)
            ni_png_error(ni_png_ptr, "Invalid bit depth for RGB image");
         ni_png_ptr->channels = 3;
         break;
      case PNG_COLOR_TYPE_PALETTE:
         switch (bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8: ni_png_ptr->channels = 1; break;
            default: ni_png_error(ni_png_ptr, "Invalid bit depth for paletted image");
         }
         break;
      case PNG_COLOR_TYPE_GRAY_ALPHA:
         if (bit_depth != 8 && bit_depth != 16)
            ni_png_error(ni_png_ptr, "Invalid bit depth for grayscale+alpha image");
         ni_png_ptr->channels = 2;
         break;
      case PNG_COLOR_TYPE_RGB_ALPHA:
         if (bit_depth != 8 && bit_depth != 16)
            ni_png_error(ni_png_ptr, "Invalid bit depth for RGBA image");
         ni_png_ptr->channels = 4;
         break;
      default:
         ni_png_error(ni_png_ptr, "Invalid image color type specified");
   }

   if (compression_type != PNG_COMPRESSION_TYPE_BASE)
   {
      ni_png_warning(ni_png_ptr, "Invalid compression type specified");
      compression_type = PNG_COMPRESSION_TYPE_BASE;
   }

   /* Write filter_method 64 (intrapixel differencing) only if
    * 1. Libpng was compiled with PNG_MNG_FEATURES_SUPPORTED and
    * 2. Libpng did not write a PNG signature (this filter_method is only
    *    used in PNG datastreams that are embedded in MNG datastreams) and
    * 3. The application called ni_png_permit_mng_features with a mask that
    *    included PNG_FLAG_MNG_FILTER_64 and
    * 4. The filter_method is 64 and
    * 5. The color_type is RGB or RGBA
    */
   if (
#if defined(PNG_MNG_FEATURES_SUPPORTED)
      !((ni_png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) &&
      ((ni_png_ptr->mode&PNG_HAVE_PNG_SIGNATURE) == 0) &&
      (color_type == PNG_COLOR_TYPE_RGB ||
       color_type == PNG_COLOR_TYPE_RGB_ALPHA) &&
      (filter_type == PNG_INTRAPIXEL_DIFFERENCING)) &&
#endif
      filter_type != PNG_FILTER_TYPE_BASE)
   {
      ni_png_warning(ni_png_ptr, "Invalid filter type specified");
      filter_type = PNG_FILTER_TYPE_BASE;
   }

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   if (interlace_type != PNG_INTERLACE_NONE &&
      interlace_type != PNG_INTERLACE_ADAM7)
   {
      ni_png_warning(ni_png_ptr, "Invalid interlace type specified");
      interlace_type = PNG_INTERLACE_ADAM7;
   }
#else
   interlace_type=PNG_INTERLACE_NONE;
#endif

   /* save off the relevent information */
   ni_png_ptr->bit_depth = (ni_png_byte)bit_depth;
   ni_png_ptr->color_type = (ni_png_byte)color_type;
   ni_png_ptr->interlaced = (ni_png_byte)interlace_type;
#if defined(PNG_MNG_FEATURES_SUPPORTED)
   ni_png_ptr->filter_type = (ni_png_byte)filter_type;
#endif
   ni_png_ptr->compression_type = (ni_png_byte)compression_type;
   ni_png_ptr->width = width;
   ni_png_ptr->height = height;

   ni_png_ptr->pixel_depth = (ni_png_byte)(bit_depth * ni_png_ptr->channels);
   ni_png_ptr->rowbytes = PNG_ROWBYTES(ni_png_ptr->pixel_depth, width);
   /* set the usr info, so any transformations can modify it */
   ni_png_ptr->usr_width = ni_png_ptr->width;
   ni_png_ptr->usr_bit_depth = ni_png_ptr->bit_depth;
   ni_png_ptr->usr_channels = ni_png_ptr->channels;

   /* pack the header information into the buffer */
   ni_png_save_uint_32(buf, width);
   ni_png_save_uint_32(buf + 4, height);
   buf[8] = (ni_png_byte)bit_depth;
   buf[9] = (ni_png_byte)color_type;
   buf[10] = (ni_png_byte)compression_type;
   buf[11] = (ni_png_byte)filter_type;
   buf[12] = (ni_png_byte)interlace_type;

   /* write the chunk */
   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_IHDR, buf, (ni_png_size_t)13);

   /* initialize zlib with PNG info */
   ni_png_ptr->zstream.zalloc = (zlib_alloc_func)ni_png_zalloc;
   ni_png_ptr->zstream.zfree = ni_png_zfree;
   ni_png_ptr->zstream.opaque = (voidpf)ni_png_ptr;
   if (!(ni_png_ptr->do_filter))
   {
      if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE ||
         ni_png_ptr->bit_depth < 8)
         ni_png_ptr->do_filter = PNG_FILTER_NONE;
      else
         ni_png_ptr->do_filter = PNG_ALL_FILTERS;
   }
   if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_STRATEGY))
   {
      if (ni_png_ptr->do_filter != PNG_FILTER_NONE)
         ni_png_ptr->zlib_strategy = Z_FILTERED;
      else
         ni_png_ptr->zlib_strategy = Z_DEFAULT_STRATEGY;
   }
   if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_LEVEL))
      ni_png_ptr->zlib_level = Z_DEFAULT_COMPRESSION;
   if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_MEM_LEVEL))
      ni_png_ptr->zlib_mem_level = 8;
   if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_WINDOW_BITS))
      ni_png_ptr->zlib_window_bits = 15;
   if (!(ni_png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_METHOD))
      ni_png_ptr->zlib_method = 8;
   if (deflateInit2(&ni_png_ptr->zstream, ni_png_ptr->zlib_level,
      ni_png_ptr->zlib_method, ni_png_ptr->zlib_window_bits,
      ni_png_ptr->zlib_mem_level, ni_png_ptr->zlib_strategy) != Z_OK)
       ni_png_error(ni_png_ptr, "zlib failed to initialize compressor");
   ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
   ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
   /* libpng is not interested in zstream.data_type */
   /* set it to a predefined value, to avoid its evaluation inside zlib */
   ni_png_ptr->zstream.data_type = Z_BINARY;

   ni_png_ptr->mode = PNG_HAVE_IHDR;
}

/* write the palette.  We are careful not to trust ni_png_color to be in the
 * correct order for PNG, so people can redefine it to any convenient
 * structure.
 */
void /* PRIVATE */
ni_png_write_PLTE(ni_png_structp ni_png_ptr, ni_png_colorp palette, ni_png_uint_32 num_pal)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_PLTE;
#endif
   ni_png_uint_32 i;
   ni_png_colorp pal_ptr;
   ni_png_byte buf[3];

   ni_png_debug(1, "in ni_png_write_PLTE\n");
   if ((
#if defined(PNG_MNG_FEATURES_SUPPORTED)
        !(ni_png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) &&
#endif
        num_pal == 0) || num_pal > 256)
   {
     if (ni_png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
     {
        ni_png_error(ni_png_ptr, "Invalid number of colors in palette");
     }
     else
     {
        ni_png_warning(ni_png_ptr, "Invalid number of colors in palette");
        return;
     }
   }

   if (!(ni_png_ptr->color_type&PNG_COLOR_MASK_COLOR))
   {
      ni_png_warning(ni_png_ptr,
        "Ignoring request to write a PLTE chunk in grayscale PNG");
      return;
   }

   ni_png_ptr->num_palette = (ni_png_uint_16)num_pal;
   ni_png_debug1(3, "num_palette = %d\n", ni_png_ptr->num_palette);

   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_PLTE, num_pal * 3);
#ifndef PNG_NO_POINTER_INDEXING
   for (i = 0, pal_ptr = palette; i < num_pal; i++, pal_ptr++)
   {
      buf[0] = pal_ptr->red;
      buf[1] = pal_ptr->green;
      buf[2] = pal_ptr->blue;
      ni_png_write_chunk_data(ni_png_ptr, buf, (ni_png_size_t)3);
   }
#else
   /* This is a little slower but some buggy compilers need to do this instead */
   pal_ptr=palette;
   for (i = 0; i < num_pal; i++)
   {
      buf[0] = pal_ptr[i].red;
      buf[1] = pal_ptr[i].green;
      buf[2] = pal_ptr[i].blue;
      ni_png_write_chunk_data(ni_png_ptr, buf, (ni_png_size_t)3);
   }
#endif
   ni_png_write_chunk_end(ni_png_ptr);
   ni_png_ptr->mode |= PNG_HAVE_PLTE;
}

/* write an IDAT chunk */
void /* PRIVATE */
ni_png_write_IDAT(ni_png_structp ni_png_ptr, ni_png_bytep data, ni_png_size_t length)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_IDAT;
#endif
   ni_png_debug(1, "in ni_png_write_IDAT\n");

   /* Optimize the CMF field in the zlib stream. */
   /* This hack of the zlib stream is compliant to the stream specification. */
   if (!(ni_png_ptr->mode & PNG_HAVE_IDAT) &&
       ni_png_ptr->compression_type == PNG_COMPRESSION_TYPE_BASE)
   {
      unsigned int z_cmf = data[0];  /* zlib compression method and flags */
      if ((z_cmf & 0x0f) == 8 && (z_cmf & 0xf0) <= 0x70)
      {
         /* Avoid memory underflows and multiplication overflows. */
         /* The conditions below are practically always satisfied;
            however, they still must be checked. */
         if (length >= 2 &&
             ni_png_ptr->height < 16384 && ni_png_ptr->width < 16384)
         {
            ni_png_uint_32 uncompressed_idat_size = ni_png_ptr->height *
               ((ni_png_ptr->width *
               ni_png_ptr->channels * ni_png_ptr->bit_depth + 15) >> 3);
            unsigned int z_cinfo = z_cmf >> 4;
            unsigned int half_z_window_size = 1 << (z_cinfo + 7);
            while (uncompressed_idat_size <= half_z_window_size &&
                   half_z_window_size >= 256)
            {
               z_cinfo--;
               half_z_window_size >>= 1;
            }
            z_cmf = (z_cmf & 0x0f) | (z_cinfo << 4);
            if (data[0] != (ni_png_byte)z_cmf)
            {
               data[0] = (ni_png_byte)z_cmf;
               data[1] &= 0xe0;
               data[1] += (ni_png_byte)(0x1f - ((z_cmf << 8) + data[1]) % 0x1f);
            }
         }
      }
      else
         ni_png_error(ni_png_ptr,
            "Invalid zlib compression method or flags in IDAT");
   }

   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_IDAT, data, length);
   ni_png_ptr->mode |= PNG_HAVE_IDAT;
}

/* write an IEND chunk */
void /* PRIVATE */
ni_png_write_IEND(ni_png_structp ni_png_ptr)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_IEND;
#endif
   ni_png_debug(1, "in ni_png_write_IEND\n");
   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_IEND, ni_png_bytep_NULL,
     (ni_png_size_t)0);
   ni_png_ptr->mode |= PNG_HAVE_IEND;
}

#if defined(PNG_WRITE_gAMA_SUPPORTED)
/* write a gAMA chunk */
#ifdef PNG_FLOATING_POINT_SUPPORTED
void /* PRIVATE */
ni_png_write_gAMA(ni_png_structp ni_png_ptr, double file_gamma)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_gAMA;
#endif
   ni_png_uint_32 igamma;
   ni_png_byte buf[4];

   ni_png_debug(1, "in ni_png_write_gAMA\n");
   /* file_gamma is saved in 1/100,000ths */
   igamma = (ni_png_uint_32)(file_gamma * 100000.0 + 0.5);
   ni_png_save_uint_32(buf, igamma);
   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_gAMA, buf, (ni_png_size_t)4);
}
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
void /* PRIVATE */
ni_png_write_gAMA_fixed(ni_png_structp ni_png_ptr, ni_png_fixed_point file_gamma)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_gAMA;
#endif
   ni_png_byte buf[4];

   ni_png_debug(1, "in ni_png_write_gAMA\n");
   /* file_gamma is saved in 1/100,000ths */
   ni_png_save_uint_32(buf, (ni_png_uint_32)file_gamma);
   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_gAMA, buf, (ni_png_size_t)4);
}
#endif
#endif

#if defined(PNG_WRITE_sRGB_SUPPORTED)
/* write a sRGB chunk */
void /* PRIVATE */
ni_png_write_sRGB(ni_png_structp ni_png_ptr, int srgb_intent)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_sRGB;
#endif
   ni_png_byte buf[1];

   ni_png_debug(1, "in ni_png_write_sRGB\n");
   if(srgb_intent >= PNG_sRGB_INTENT_LAST)
         ni_png_warning(ni_png_ptr,
            "Invalid sRGB rendering intent specified");
   buf[0]=(ni_png_byte)srgb_intent;
   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_sRGB, buf, (ni_png_size_t)1);
}
#endif

#if defined(PNG_WRITE_iCCP_SUPPORTED)
/* write an iCCP chunk */
void /* PRIVATE */
ni_png_write_iCCP(ni_png_structp ni_png_ptr, ni_png_charp name, int compression_type,
   ni_png_charp profile, int profile_len)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_iCCP;
#endif
   ni_png_size_t name_len;
   ni_png_charp new_name;
   compression_state comp;
   int embedded_profile_len = 0;

   ni_png_debug(1, "in ni_png_write_iCCP\n");

   comp.num_output_ptr = 0;
   comp.max_output_ptr = 0;
   comp.output_ptr = NULL;
   comp.input = NULL;
   comp.input_len = 0;

   if (name == NULL || (name_len = ni_png_check_keyword(ni_png_ptr, name,
      &new_name)) == 0)
   {
      ni_png_warning(ni_png_ptr, "Empty keyword in iCCP chunk");
      return;
   }

   if (compression_type != PNG_COMPRESSION_TYPE_BASE)
      ni_png_warning(ni_png_ptr, "Unknown compression type in iCCP chunk");

   if (profile == NULL)
      profile_len = 0;

   if (profile_len > 3)
      embedded_profile_len =
          ((*( (ni_png_bytep)profile  ))<<24) |
          ((*( (ni_png_bytep)profile+1))<<16) |
          ((*( (ni_png_bytep)profile+2))<< 8) |
          ((*( (ni_png_bytep)profile+3))    );

   if (profile_len < embedded_profile_len)
     {
        ni_png_warning(ni_png_ptr,
          "Embedded profile length too large in iCCP chunk");
        return;
     }

   if (profile_len > embedded_profile_len)
     {
        ni_png_warning(ni_png_ptr,
          "Truncating profile to actual length in iCCP chunk");
        profile_len = embedded_profile_len;
     }

   if (profile_len)
       profile_len = ni_png_text_compress(ni_png_ptr, profile, (ni_png_size_t)profile_len,
          PNG_COMPRESSION_TYPE_BASE, &comp);

   /* make sure we include the NULL after the name and the compression type */
   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_iCCP,
          (ni_png_uint_32)name_len+profile_len+2);
   new_name[name_len+1]=0x00;
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)new_name, name_len + 2);

   if (profile_len)
      ni_png_write_compressed_data_out(ni_png_ptr, &comp);

   ni_png_write_chunk_end(ni_png_ptr);
   ni_png_free(ni_png_ptr, new_name);
}
#endif

#if defined(PNG_WRITE_sPLT_SUPPORTED)
/* write a sPLT chunk */
void /* PRIVATE */
ni_png_write_sPLT(ni_png_structp ni_png_ptr, ni_png_sPLT_tp spalette)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_sPLT;
#endif
   ni_png_size_t name_len;
   ni_png_charp new_name;
   ni_png_byte entrybuf[10];
   int entry_size = (spalette->depth == 8 ? 6 : 10);
   int palette_size = entry_size * spalette->nentries;
   ni_png_sPLT_entryp ep;
#ifdef PNG_NO_POINTER_INDEXING
   int i;
#endif

   ni_png_debug(1, "in ni_png_write_sPLT\n");
   if (spalette->name == NULL || (name_len = ni_png_check_keyword(ni_png_ptr,
      spalette->name, &new_name))==0)
   {
      ni_png_warning(ni_png_ptr, "Empty keyword in sPLT chunk");
      return;
   }

   /* make sure we include the NULL after the name */
   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_sPLT,
          (ni_png_uint_32)(name_len + 2 + palette_size));
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)new_name, name_len + 1);
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)&spalette->depth, 1);

   /* loop through each palette entry, writing appropriately */
#ifndef PNG_NO_POINTER_INDEXING
   for (ep = spalette->entries; ep<spalette->entries+spalette->nentries; ep++)
   {
       if (spalette->depth == 8)
       {
           entrybuf[0] = (ni_png_byte)ep->red;
           entrybuf[1] = (ni_png_byte)ep->green;
           entrybuf[2] = (ni_png_byte)ep->blue;
           entrybuf[3] = (ni_png_byte)ep->alpha;
           ni_png_save_uint_16(entrybuf + 4, ep->frequency);
       }
       else
       {
           ni_png_save_uint_16(entrybuf + 0, ep->red);
           ni_png_save_uint_16(entrybuf + 2, ep->green);
           ni_png_save_uint_16(entrybuf + 4, ep->blue);
           ni_png_save_uint_16(entrybuf + 6, ep->alpha);
           ni_png_save_uint_16(entrybuf + 8, ep->frequency);
       }
       ni_png_write_chunk_data(ni_png_ptr, entrybuf, (ni_png_size_t)entry_size);
   }
#else
   ep=spalette->entries;
   for (i=0; i>spalette->nentries; i++)
   {
       if (spalette->depth == 8)
       {
           entrybuf[0] = (ni_png_byte)ep[i].red;
           entrybuf[1] = (ni_png_byte)ep[i].green;
           entrybuf[2] = (ni_png_byte)ep[i].blue;
           entrybuf[3] = (ni_png_byte)ep[i].alpha;
           ni_png_save_uint_16(entrybuf + 4, ep[i].frequency);
       }
       else
       {
           ni_png_save_uint_16(entrybuf + 0, ep[i].red);
           ni_png_save_uint_16(entrybuf + 2, ep[i].green);
           ni_png_save_uint_16(entrybuf + 4, ep[i].blue);
           ni_png_save_uint_16(entrybuf + 6, ep[i].alpha);
           ni_png_save_uint_16(entrybuf + 8, ep[i].frequency);
       }
       ni_png_write_chunk_data(ni_png_ptr, entrybuf, entry_size);
   }
#endif

   ni_png_write_chunk_end(ni_png_ptr);
   ni_png_free(ni_png_ptr, new_name);
}
#endif

#if defined(PNG_WRITE_sBIT_SUPPORTED)
/* write the sBIT chunk */
void /* PRIVATE */
ni_png_write_sBIT(ni_png_structp ni_png_ptr, ni_png_color_8p sbit, int color_type)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_sBIT;
#endif
   ni_png_byte buf[4];
   ni_png_size_t size;

   ni_png_debug(1, "in ni_png_write_sBIT\n");
   /* make sure we don't depend upon the order of PNG_COLOR_8 */
   if (color_type & PNG_COLOR_MASK_COLOR)
   {
      ni_png_byte maxbits;

      maxbits = (ni_png_byte)(color_type==PNG_COLOR_TYPE_PALETTE ? 8 :
                ni_png_ptr->usr_bit_depth);
      if (sbit->red == 0 || sbit->red > maxbits ||
          sbit->green == 0 || sbit->green > maxbits ||
          sbit->blue == 0 || sbit->blue > maxbits)
      {
         ni_png_warning(ni_png_ptr, "Invalid sBIT depth specified");
         return;
      }
      buf[0] = sbit->red;
      buf[1] = sbit->green;
      buf[2] = sbit->blue;
      size = 3;
   }
   else
   {
      if (sbit->gray == 0 || sbit->gray > ni_png_ptr->usr_bit_depth)
      {
         ni_png_warning(ni_png_ptr, "Invalid sBIT depth specified");
         return;
      }
      buf[0] = sbit->gray;
      size = 1;
   }

   if (color_type & PNG_COLOR_MASK_ALPHA)
   {
      if (sbit->alpha == 0 || sbit->alpha > ni_png_ptr->usr_bit_depth)
      {
         ni_png_warning(ni_png_ptr, "Invalid sBIT depth specified");
         return;
      }
      buf[size++] = sbit->alpha;
   }

   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_sBIT, buf, size);
}
#endif

#if defined(PNG_WRITE_cHRM_SUPPORTED)
/* write the cHRM chunk */
#ifdef PNG_FLOATING_POINT_SUPPORTED
void /* PRIVATE */
ni_png_write_cHRM(ni_png_structp ni_png_ptr, double white_x, double white_y,
   double red_x, double red_y, double green_x, double green_y,
   double blue_x, double blue_y)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_cHRM;
#endif
   ni_png_byte buf[32];
   ni_png_uint_32 itemp;

   ni_png_debug(1, "in ni_png_write_cHRM\n");
   /* each value is saved in 1/100,000ths */
   if (white_x < 0 || white_x > 0.8 || white_y < 0 || white_y > 0.8 ||
       white_x + white_y > 1.0)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM white point specified");
#if !defined(PNG_NO_CONSOLE_IO)
      fprintf(stderr,"white_x=%f, white_y=%f\n",white_x, white_y);
#endif
      return;
   }
   itemp = (ni_png_uint_32)(white_x * 100000.0 + 0.5);
   ni_png_save_uint_32(buf, itemp);
   itemp = (ni_png_uint_32)(white_y * 100000.0 + 0.5);
   ni_png_save_uint_32(buf + 4, itemp);

   if (red_x < 0 ||  red_y < 0 || red_x + red_y > 1.0)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM red point specified");
      return;
   }
   itemp = (ni_png_uint_32)(red_x * 100000.0 + 0.5);
   ni_png_save_uint_32(buf + 8, itemp);
   itemp = (ni_png_uint_32)(red_y * 100000.0 + 0.5);
   ni_png_save_uint_32(buf + 12, itemp);

   if (green_x < 0 || green_y < 0 || green_x + green_y > 1.0)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM green point specified");
      return;
   }
   itemp = (ni_png_uint_32)(green_x * 100000.0 + 0.5);
   ni_png_save_uint_32(buf + 16, itemp);
   itemp = (ni_png_uint_32)(green_y * 100000.0 + 0.5);
   ni_png_save_uint_32(buf + 20, itemp);

   if (blue_x < 0 || blue_y < 0 || blue_x + blue_y > 1.0)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM blue point specified");
      return;
   }
   itemp = (ni_png_uint_32)(blue_x * 100000.0 + 0.5);
   ni_png_save_uint_32(buf + 24, itemp);
   itemp = (ni_png_uint_32)(blue_y * 100000.0 + 0.5);
   ni_png_save_uint_32(buf + 28, itemp);

   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_cHRM, buf, (ni_png_size_t)32);
}
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
void /* PRIVATE */
ni_png_write_cHRM_fixed(ni_png_structp ni_png_ptr, ni_png_fixed_point white_x,
   ni_png_fixed_point white_y, ni_png_fixed_point red_x, ni_png_fixed_point red_y,
   ni_png_fixed_point green_x, ni_png_fixed_point green_y, ni_png_fixed_point blue_x,
   ni_png_fixed_point blue_y)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_cHRM;
#endif
   ni_png_byte buf[32];

   ni_png_debug(1, "in ni_png_write_cHRM\n");
   /* each value is saved in 1/100,000ths */
   if (white_x > 80000L || white_y > 80000L || white_x + white_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid fixed cHRM white point specified");
#if !defined(PNG_NO_CONSOLE_IO)
      fprintf(stderr,"white_x=%ld, white_y=%ld\n",white_x, white_y);
#endif
      return;
   }
   ni_png_save_uint_32(buf, (ni_png_uint_32)white_x);
   ni_png_save_uint_32(buf + 4, (ni_png_uint_32)white_y);

   if (red_x + red_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid cHRM fixed red point specified");
      return;
   }
   ni_png_save_uint_32(buf + 8, (ni_png_uint_32)red_x);
   ni_png_save_uint_32(buf + 12, (ni_png_uint_32)red_y);

   if (green_x + green_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid fixed cHRM green point specified");
      return;
   }
   ni_png_save_uint_32(buf + 16, (ni_png_uint_32)green_x);
   ni_png_save_uint_32(buf + 20, (ni_png_uint_32)green_y);

   if (blue_x + blue_y > 100000L)
   {
      ni_png_warning(ni_png_ptr, "Invalid fixed cHRM blue point specified");
      return;
   }
   ni_png_save_uint_32(buf + 24, (ni_png_uint_32)blue_x);
   ni_png_save_uint_32(buf + 28, (ni_png_uint_32)blue_y);

   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_cHRM, buf, (ni_png_size_t)32);
}
#endif
#endif

#if defined(PNG_WRITE_tRNS_SUPPORTED)
/* write the tRNS chunk */
void /* PRIVATE */
ni_png_write_tRNS(ni_png_structp ni_png_ptr, ni_png_bytep trans, ni_png_color_16p tran,
   int num_trans, int color_type)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_tRNS;
#endif
   ni_png_byte buf[6];

   ni_png_debug(1, "in ni_png_write_tRNS\n");
   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (num_trans <= 0 || num_trans > (int)ni_png_ptr->num_palette)
      {
         ni_png_warning(ni_png_ptr,"Invalid number of transparent colors specified");
         return;
      }
      /* write the chunk out as it is */
      ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_tRNS, trans, (ni_png_size_t)num_trans);
   }
   else if (color_type == PNG_COLOR_TYPE_GRAY)
   {
      /* one 16 bit value */
      if(tran->gray >= (1 << ni_png_ptr->bit_depth))
      {
         ni_png_warning(ni_png_ptr,
           "Ignoring attempt to write tRNS chunk out-of-range for bit_depth");
         return;
      }
      ni_png_save_uint_16(buf, tran->gray);
      ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_tRNS, buf, (ni_png_size_t)2);
   }
   else if (color_type == PNG_COLOR_TYPE_RGB)
   {
      /* three 16 bit values */
      ni_png_save_uint_16(buf, tran->red);
      ni_png_save_uint_16(buf + 2, tran->green);
      ni_png_save_uint_16(buf + 4, tran->blue);
      if(ni_png_ptr->bit_depth == 8 && (buf[0] | buf[2] | buf[4]))
         {
            ni_png_warning(ni_png_ptr,
              "Ignoring attempt to write 16-bit tRNS chunk when bit_depth is 8");
            return;
         }
      ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_tRNS, buf, (ni_png_size_t)6);
   }
   else
   {
      ni_png_warning(ni_png_ptr, "Can't write tRNS with an alpha channel");
   }
}
#endif

#if defined(PNG_WRITE_bKGD_SUPPORTED)
/* write the background chunk */
void /* PRIVATE */
ni_png_write_bKGD(ni_png_structp ni_png_ptr, ni_png_color_16p back, int color_type)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_bKGD;
#endif
   ni_png_byte buf[6];

   ni_png_debug(1, "in ni_png_write_bKGD\n");
   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (
#if defined(PNG_MNG_FEATURES_SUPPORTED)
          (ni_png_ptr->num_palette ||
          (!(ni_png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE))) &&
#endif
         back->index > ni_png_ptr->num_palette)
      {
         ni_png_warning(ni_png_ptr, "Invalid background palette index");
         return;
      }
      buf[0] = back->index;
      ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_bKGD, buf, (ni_png_size_t)1);
   }
   else if (color_type & PNG_COLOR_MASK_COLOR)
   {
      ni_png_save_uint_16(buf, back->red);
      ni_png_save_uint_16(buf + 2, back->green);
      ni_png_save_uint_16(buf + 4, back->blue);
      if(ni_png_ptr->bit_depth == 8 && (buf[0] | buf[2] | buf[4]))
         {
            ni_png_warning(ni_png_ptr,
              "Ignoring attempt to write 16-bit bKGD chunk when bit_depth is 8");
            return;
         }
      ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_bKGD, buf, (ni_png_size_t)6);
   }
   else
   {
      if(back->gray >= (1 << ni_png_ptr->bit_depth))
      {
         ni_png_warning(ni_png_ptr,
           "Ignoring attempt to write bKGD chunk out-of-range for bit_depth");
         return;
      }
      ni_png_save_uint_16(buf, back->gray);
      ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_bKGD, buf, (ni_png_size_t)2);
   }
}
#endif

#if defined(PNG_WRITE_hIST_SUPPORTED)
/* write the histogram */
void /* PRIVATE */
ni_png_write_hIST(ni_png_structp ni_png_ptr, ni_png_uint_16p hist, int num_hist)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_hIST;
#endif
   int i;
   ni_png_byte buf[3];

   ni_png_debug(1, "in ni_png_write_hIST\n");
   if (num_hist > (int)ni_png_ptr->num_palette)
   {
      ni_png_debug2(3, "num_hist = %d, num_palette = %d\n", num_hist,
         ni_png_ptr->num_palette);
      ni_png_warning(ni_png_ptr, "Invalid number of histogram entries specified");
      return;
   }

   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_hIST, (ni_png_uint_32)(num_hist * 2));
   for (i = 0; i < num_hist; i++)
   {
      ni_png_save_uint_16(buf, hist[i]);
      ni_png_write_chunk_data(ni_png_ptr, buf, (ni_png_size_t)2);
   }
   ni_png_write_chunk_end(ni_png_ptr);
}
#endif

#if defined(PNG_WRITE_TEXT_SUPPORTED) || defined(PNG_WRITE_pCAL_SUPPORTED) || \
    defined(PNG_WRITE_iCCP_SUPPORTED) || defined(PNG_WRITE_sPLT_SUPPORTED)
/* Check that the tEXt or zTXt keyword is valid per PNG 1.0 specification,
 * and if invalid, correct the keyword rather than discarding the entire
 * chunk.  The PNG 1.0 specification requires keywords 1-79 characters in
 * length, forbids leading or trailing whitespace, multiple internal spaces,
 * and the non-break space (0x80) from ISO 8859-1.  Returns keyword length.
 *
 * The new_key is allocated to hold the corrected keyword and must be freed
 * by the calling routine.  This avoids problems with trying to write to
 * static keywords without having to have duplicate copies of the strings.
 */
ni_png_size_t /* PRIVATE */
ni_png_check_keyword(ni_png_structp ni_png_ptr, ni_png_charp key, ni_png_charpp new_key)
{
   ni_png_size_t key_len;
   ni_png_charp kp, dp;
   int kflag;
   int kwarn=0;

   ni_png_debug(1, "in ni_png_check_keyword\n");
   *new_key = NULL;

   if (key == NULL || (key_len = ni_png_strlen(key)) == 0)
   {
      ni_png_warning(ni_png_ptr, "zero length keyword");
      return ((ni_png_size_t)0);
   }

   ni_png_debug1(2, "Keyword to be checked is '%s'\n", key);

   *new_key = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, (ni_png_uint_32)(key_len + 2));
   if (*new_key == NULL)
   {
      ni_png_warning(ni_png_ptr, "Out of memory while procesing keyword");
      return ((ni_png_size_t)0);
   }

   /* Replace non-printing characters with a blank and print a warning */
   for (kp = key, dp = *new_key; *kp != '\0'; kp++, dp++)
   {
      if (*kp < 0x20 || (*kp > 0x7E && (ni_png_byte)*kp < 0xA1))
      {
#if !defined(PNG_NO_STDIO) && !defined(_WIN32_WCE)
         char msg[40];

         sprintf(msg, "invalid keyword character 0x%02X", *kp);
         ni_png_warning(ni_png_ptr, msg);
#else
         ni_png_warning(ni_png_ptr, "invalid character in keyword");
#endif
         *dp = ' ';
      }
      else
      {
         *dp = *kp;
      }
   }
   *dp = '\0';

   /* Remove any trailing white space. */
   kp = *new_key + key_len - 1;
   if (*kp == ' ')
   {
      ni_png_warning(ni_png_ptr, "trailing spaces removed from keyword");

      while (*kp == ' ')
      {
        *(kp--) = '\0';
        key_len--;
      }
   }

   /* Remove any leading white space. */
   kp = *new_key;
   if (*kp == ' ')
   {
      ni_png_warning(ni_png_ptr, "leading spaces removed from keyword");

      while (*kp == ' ')
      {
        kp++;
        key_len--;
      }
   }

   ni_png_debug1(2, "Checking for multiple internal spaces in '%s'\n", kp);

   /* Remove multiple internal spaces. */
   for (kflag = 0, dp = *new_key; *kp != '\0'; kp++)
   {
      if (*kp == ' ' && kflag == 0)
      {
         *(dp++) = *kp;
         kflag = 1;
      }
      else if (*kp == ' ')
      {
         key_len--;
         kwarn=1;
      }
      else
      {
         *(dp++) = *kp;
         kflag = 0;
      }
   }
   *dp = '\0';
   if(kwarn)
      ni_png_warning(ni_png_ptr, "extra interior spaces removed from keyword");

   if (key_len == 0)
   {
      ni_png_free(ni_png_ptr, *new_key);
      *new_key=NULL;
      ni_png_warning(ni_png_ptr, "Zero length keyword");
   }

   if (key_len > 79)
   {
      ni_png_warning(ni_png_ptr, "keyword length must be 1 - 79 characters");
      new_key[79] = '\0';
      key_len = 79;
   }

   return (key_len);
}
#endif

#if defined(PNG_WRITE_tEXt_SUPPORTED)
/* write a tEXt chunk */
void /* PRIVATE */
ni_png_write_tEXt(ni_png_structp ni_png_ptr, ni_png_charp key, ni_png_charp text,
   ni_png_size_t text_len)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_tEXt;
#endif
   ni_png_size_t key_len;
   ni_png_charp new_key;

   ni_png_debug(1, "in ni_png_write_tEXt\n");
   if (key == NULL || (key_len = ni_png_check_keyword(ni_png_ptr, key, &new_key))==0)
   {
      ni_png_warning(ni_png_ptr, "Empty keyword in tEXt chunk");
      return;
   }

   if (text == NULL || *text == '\0')
      text_len = 0;
   else
      text_len = ni_png_strlen(text);

   /* make sure we include the 0 after the key */
   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_tEXt, (ni_png_uint_32)key_len+text_len+1);
   /*
    * We leave it to the application to meet PNG-1.0 requirements on the
    * contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
    * any non-Latin-1 characters except for NEWLINE.  ISO PNG will forbid them.
    * The NUL character is forbidden by PNG-1.0 through PNG-1.2 and ISO PNG.
    */
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)new_key, key_len + 1);
   if (text_len)
      ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)text, text_len);

   ni_png_write_chunk_end(ni_png_ptr);
   ni_png_free(ni_png_ptr, new_key);
}
#endif

#if defined(PNG_WRITE_zTXt_SUPPORTED)
/* write a compressed text chunk */
void /* PRIVATE */
ni_png_write_zTXt(ni_png_structp ni_png_ptr, ni_png_charp key, ni_png_charp text,
   ni_png_size_t text_len, int compression)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_zTXt;
#endif
   ni_png_size_t key_len;
   char buf[1];
   ni_png_charp new_key;
   compression_state comp;

   ni_png_debug(1, "in ni_png_write_zTXt\n");

   comp.num_output_ptr = 0;
   comp.max_output_ptr = 0;
   comp.output_ptr = NULL;
   comp.input = NULL;
   comp.input_len = 0;

   if (key == NULL || (key_len = ni_png_check_keyword(ni_png_ptr, key, &new_key))==0)
   {
      ni_png_warning(ni_png_ptr, "Empty keyword in zTXt chunk");
      return;
   }

   if (text == NULL || *text == '\0' || compression==PNG_TEXT_COMPRESSION_NONE)
   {
      ni_png_write_tEXt(ni_png_ptr, new_key, text, (ni_png_size_t)0);
      ni_png_free(ni_png_ptr, new_key);
      return;
   }

   text_len = ni_png_strlen(text);

   ni_png_free(ni_png_ptr, new_key);

   /* compute the compressed data; do it now for the length */
   text_len = ni_png_text_compress(ni_png_ptr, text, text_len, compression,
       &comp);

   /* write start of chunk */
   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_zTXt, (ni_png_uint_32)
      (key_len+text_len+2));
   /* write key */
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)key, key_len + 1);
   buf[0] = (ni_png_byte)compression;
   /* write compression */
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)buf, (ni_png_size_t)1);
   /* write the compressed data */
   ni_png_write_compressed_data_out(ni_png_ptr, &comp);

   /* close the chunk */
   ni_png_write_chunk_end(ni_png_ptr);
}
#endif

#if defined(PNG_WRITE_iTXt_SUPPORTED)
/* write an iTXt chunk */
void /* PRIVATE */
ni_png_write_iTXt(ni_png_structp ni_png_ptr, int compression, ni_png_charp key,
    ni_png_charp lang, ni_png_charp lang_key, ni_png_charp text)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_iTXt;
#endif
   ni_png_size_t lang_len, key_len, lang_key_len, text_len;
   ni_png_charp new_lang, new_key;
   ni_png_byte cbuf[2];
   compression_state comp;

   ni_png_debug(1, "in ni_png_write_iTXt\n");

   comp.num_output_ptr = 0;
   comp.max_output_ptr = 0;
   comp.output_ptr = NULL;
   comp.input = NULL;

   if (key == NULL || (key_len = ni_png_check_keyword(ni_png_ptr, key, &new_key))==0)
   {
      ni_png_warning(ni_png_ptr, "Empty keyword in iTXt chunk");
      return;
   }
   if (lang == NULL || (lang_len = ni_png_check_keyword(ni_png_ptr, lang, &new_lang))==0)
   {
      ni_png_warning(ni_png_ptr, "Empty language field in iTXt chunk");
      new_lang = NULL;
      lang_len = 0;
   }

   if (lang_key == NULL)
     lang_key_len = 0;
   else
     lang_key_len = ni_png_strlen(lang_key);

   if (text == NULL)
      text_len = 0;
   else
     text_len = ni_png_strlen(text);

   /* compute the compressed data; do it now for the length */
   text_len = ni_png_text_compress(ni_png_ptr, text, text_len, compression-2,
      &comp);


   /* make sure we include the compression flag, the compression byte,
    * and the NULs after the key, lang, and lang_key parts */

   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_iTXt,
          (ni_png_uint_32)(
        5 /* comp byte, comp flag, terminators for key, lang and lang_key */
        + key_len
        + lang_len
        + lang_key_len
        + text_len));

   /*
    * We leave it to the application to meet PNG-1.0 requirements on the
    * contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
    * any non-Latin-1 characters except for NEWLINE.  ISO PNG will forbid them.
    * The NUL character is forbidden by PNG-1.0 through PNG-1.2 and ISO PNG.
    */
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)new_key, key_len + 1);

   /* set the compression flag */
   if (compression == PNG_ITXT_COMPRESSION_NONE || \
       compression == PNG_TEXT_COMPRESSION_NONE)
       cbuf[0] = 0;
   else /* compression == PNG_ITXT_COMPRESSION_zTXt */
       cbuf[0] = 1;
   /* set the compression method */
   cbuf[1] = 0;
   ni_png_write_chunk_data(ni_png_ptr, cbuf, 2);

   cbuf[0] = 0;
   ni_png_write_chunk_data(ni_png_ptr, (new_lang ? (ni_png_bytep)new_lang : cbuf), lang_len + 1);
   ni_png_write_chunk_data(ni_png_ptr, (lang_key ? (ni_png_bytep)lang_key : cbuf), lang_key_len + 1);
   ni_png_write_compressed_data_out(ni_png_ptr, &comp);

   ni_png_write_chunk_end(ni_png_ptr);
   ni_png_free(ni_png_ptr, new_key);
   if (new_lang)
     ni_png_free(ni_png_ptr, new_lang);
}
#endif

#if defined(PNG_WRITE_oFFs_SUPPORTED)
/* write the oFFs chunk */
void /* PRIVATE */
ni_png_write_oFFs(ni_png_structp ni_png_ptr, ni_png_int_32 x_offset, ni_png_int_32 y_offset,
   int unit_type)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_oFFs;
#endif
   ni_png_byte buf[9];

   ni_png_debug(1, "in ni_png_write_oFFs\n");
   if (unit_type >= PNG_OFFSET_LAST)
      ni_png_warning(ni_png_ptr, "Unrecognized unit type for oFFs chunk");

   ni_png_save_int_32(buf, x_offset);
   ni_png_save_int_32(buf + 4, y_offset);
   buf[8] = (ni_png_byte)unit_type;

   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_oFFs, buf, (ni_png_size_t)9);
}
#endif

#if defined(PNG_WRITE_pCAL_SUPPORTED)
/* write the pCAL chunk (described in the PNG extensions document) */
void /* PRIVATE */
ni_png_write_pCAL(ni_png_structp ni_png_ptr, ni_png_charp purpose, ni_png_int_32 X0,
   ni_png_int_32 X1, int type, int nparams, ni_png_charp units, ni_png_charpp params)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_pCAL;
#endif
   ni_png_size_t purpose_len, units_len, total_len;
   ni_png_uint_32p params_len;
   ni_png_byte buf[10];
   ni_png_charp new_purpose;
   int i;

   ni_png_debug1(1, "in ni_png_write_pCAL (%d parameters)\n", nparams);
   if (type >= PNG_EQUATION_LAST)
      ni_png_warning(ni_png_ptr, "Unrecognized equation type for pCAL chunk");

   purpose_len = ni_png_check_keyword(ni_png_ptr, purpose, &new_purpose) + 1;
   ni_png_debug1(3, "pCAL purpose length = %d\n", (int)purpose_len);
   units_len = ni_png_strlen(units) + (nparams == 0 ? 0 : 1);
   ni_png_debug1(3, "pCAL units length = %d\n", (int)units_len);
   total_len = purpose_len + units_len + 10;

   params_len = (ni_png_uint_32p)ni_png_malloc(ni_png_ptr, (ni_png_uint_32)(nparams
      *ni_png_sizeof(ni_png_uint_32)));

   /* Find the length of each parameter, making sure we don't count the
      null terminator for the last parameter. */
   for (i = 0; i < nparams; i++)
   {
      params_len[i] = ni_png_strlen(params[i]) + (i == nparams - 1 ? 0 : 1);
      ni_png_debug2(3, "pCAL parameter %d length = %lu\n", i, params_len[i]);
      total_len += (ni_png_size_t)params_len[i];
   }

   ni_png_debug1(3, "pCAL total length = %d\n", (int)total_len);
   ni_png_write_chunk_start(ni_png_ptr, (ni_png_bytep)ni_png_pCAL, (ni_png_uint_32)total_len);
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)new_purpose, purpose_len);
   ni_png_save_int_32(buf, X0);
   ni_png_save_int_32(buf + 4, X1);
   buf[8] = (ni_png_byte)type;
   buf[9] = (ni_png_byte)nparams;
   ni_png_write_chunk_data(ni_png_ptr, buf, (ni_png_size_t)10);
   ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)units, (ni_png_size_t)units_len);

   ni_png_free(ni_png_ptr, new_purpose);

   for (i = 0; i < nparams; i++)
   {
      ni_png_write_chunk_data(ni_png_ptr, (ni_png_bytep)params[i],
         (ni_png_size_t)params_len[i]);
   }

   ni_png_free(ni_png_ptr, params_len);
   ni_png_write_chunk_end(ni_png_ptr);
}
#endif

#if defined(PNG_WRITE_sCAL_SUPPORTED)
/* write the sCAL chunk */
#if defined(PNG_FLOATING_POINT_SUPPORTED) && !defined(PNG_NO_STDIO)
void /* PRIVATE */
ni_png_write_sCAL(ni_png_structp ni_png_ptr, int unit, double width, double height)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_sCAL;
#endif
   char buf[64];
   ni_png_size_t total_len;

   ni_png_debug(1, "in ni_png_write_sCAL\n");

   buf[0] = (char)unit;
#if defined(_WIN32_WCE)
/* sprintf() function is not supported on WindowsCE */
   {
      wchar_t wc_buf[32];
      size_t wc_len;
      swprintf(wc_buf, TEXT("%12.12e"), width);
      wc_len = wcslen(wc_buf);
      WideCharToMultiByte(CP_ACP, 0, wc_buf, -1, buf + 1, wc_len, NULL, NULL);
      total_len = wc_len + 2;
      swprintf(wc_buf, TEXT("%12.12e"), height);
      wc_len = wcslen(wc_buf);
      WideCharToMultiByte(CP_ACP, 0, wc_buf, -1, buf + total_len, wc_len,
         NULL, NULL);
      total_len += wc_len;
   }
#else
   sprintf(buf + 1, "%12.12e", width);
   total_len = 1 + ni_png_strlen(buf + 1) + 1;
   sprintf(buf + total_len, "%12.12e", height);
   total_len += ni_png_strlen(buf + total_len);
#endif

   ni_png_debug1(3, "sCAL total length = %u\n", (unsigned int)total_len);
   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_sCAL, (ni_png_bytep)buf, total_len);
}
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
void /* PRIVATE */
ni_png_write_sCAL_s(ni_png_structp ni_png_ptr, int unit, ni_png_charp width,
   ni_png_charp height)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_sCAL;
#endif
   ni_png_byte buf[64];
   ni_png_size_t wlen, hlen, total_len;

   ni_png_debug(1, "in ni_png_write_sCAL_s\n");

   wlen = ni_png_strlen(width);
   hlen = ni_png_strlen(height);
   total_len = wlen + hlen + 2;
   if (total_len > 64)
   {
      ni_png_warning(ni_png_ptr, "Can't write sCAL (buffer too small)");
      return;
   }

   buf[0] = (ni_png_byte)unit;
   ni_png_memcpy(buf + 1, width, wlen + 1);      /* append the '\0' here */
   ni_png_memcpy(buf + wlen + 2, height, hlen);  /* do NOT append the '\0' here */

   ni_png_debug1(3, "sCAL total length = %u\n", (unsigned int)total_len);
   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_sCAL, buf, total_len);
}
#endif
#endif
#endif

#if defined(PNG_WRITE_pHYs_SUPPORTED)
/* write the pHYs chunk */
void /* PRIVATE */
ni_png_write_pHYs(ni_png_structp ni_png_ptr, ni_png_uint_32 x_pixels_per_unit,
   ni_png_uint_32 y_pixels_per_unit,
   int unit_type)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_pHYs;
#endif
   ni_png_byte buf[9];

   ni_png_debug(1, "in ni_png_write_pHYs\n");
   if (unit_type >= PNG_RESOLUTION_LAST)
      ni_png_warning(ni_png_ptr, "Unrecognized unit type for pHYs chunk");

   ni_png_save_uint_32(buf, x_pixels_per_unit);
   ni_png_save_uint_32(buf + 4, y_pixels_per_unit);
   buf[8] = (ni_png_byte)unit_type;

   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_pHYs, buf, (ni_png_size_t)9);
}
#endif

#if defined(PNG_WRITE_tIME_SUPPORTED)
/* Write the tIME chunk.  Use either ni_png_convert_from_struct_tm()
 * or ni_png_convert_from_time_t(), or fill in the structure yourself.
 */
void /* PRIVATE */
ni_png_write_tIME(ni_png_structp ni_png_ptr, ni_png_timep mod_time)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   PNG_tIME;
#endif
   ni_png_byte buf[7];

   ni_png_debug(1, "in ni_png_write_tIME\n");
   if (mod_time->month  > 12 || mod_time->month  < 1 ||
       mod_time->day    > 31 || mod_time->day    < 1 ||
       mod_time->hour   > 23 || mod_time->second > 60)
   {
      ni_png_warning(ni_png_ptr, "Invalid time specified for tIME chunk");
      return;
   }

   ni_png_save_uint_16(buf, mod_time->year);
   buf[2] = mod_time->month;
   buf[3] = mod_time->day;
   buf[4] = mod_time->hour;
   buf[5] = mod_time->minute;
   buf[6] = mod_time->second;

   ni_png_write_chunk(ni_png_ptr, (ni_png_bytep)ni_png_tIME, buf, (ni_png_size_t)7);
}
#endif

/* initializes the row writing capability of libpng */
void /* PRIVATE */
ni_png_write_start_row(ni_png_structp ni_png_ptr)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   /* arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* start of interlace block */
   int ni_png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* offset to next interlace block */
   int ni_png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   /* start of interlace block in the y direction */
   int ni_png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};

   /* offset to next interlace block in the y direction */
   int ni_png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif

   ni_png_size_t buf_size;

   ni_png_debug(1, "in ni_png_write_start_row\n");
   buf_size = (ni_png_size_t)(PNG_ROWBYTES(
      ni_png_ptr->usr_channels*ni_png_ptr->usr_bit_depth,ni_png_ptr->width)+1);

   /* set up row buffer */
   ni_png_ptr->row_buf = (ni_png_bytep)ni_png_malloc(ni_png_ptr, (ni_png_uint_32)buf_size);
   ni_png_ptr->row_buf[0] = PNG_FILTER_VALUE_NONE;

   /* set up filtering buffer, if using this filter */
   if (ni_png_ptr->do_filter & PNG_FILTER_SUB)
   {
      ni_png_ptr->sub_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
         (ni_png_ptr->rowbytes + 1));
      ni_png_ptr->sub_row[0] = PNG_FILTER_VALUE_SUB;
   }

   /* We only need to keep the previous row if we are using one of these. */
   if (ni_png_ptr->do_filter & (PNG_FILTER_AVG | PNG_FILTER_UP | PNG_FILTER_PAETH))
   {
     /* set up previous row buffer */
      ni_png_ptr->prev_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr, (ni_png_uint_32)buf_size);
      ni_png_memset(ni_png_ptr->prev_row, 0, buf_size);

      if (ni_png_ptr->do_filter & PNG_FILTER_UP)
      {
         ni_png_ptr->up_row = (ni_png_bytep )ni_png_malloc(ni_png_ptr,
            (ni_png_ptr->rowbytes + 1));
         ni_png_ptr->up_row[0] = PNG_FILTER_VALUE_UP;
      }

      if (ni_png_ptr->do_filter & PNG_FILTER_AVG)
      {
         ni_png_ptr->avg_row = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
            (ni_png_ptr->rowbytes + 1));
         ni_png_ptr->avg_row[0] = PNG_FILTER_VALUE_AVG;
      }

      if (ni_png_ptr->do_filter & PNG_FILTER_PAETH)
      {
         ni_png_ptr->paeth_row = (ni_png_bytep )ni_png_malloc(ni_png_ptr,
            (ni_png_ptr->rowbytes + 1));
         ni_png_ptr->paeth_row[0] = PNG_FILTER_VALUE_PAETH;
      }
   }

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   /* if interlaced, we need to set up width and height of pass */
   if (ni_png_ptr->interlaced)
   {
      if (!(ni_png_ptr->transformations & PNG_INTERLACE))
      {
         ni_png_ptr->num_rows = (ni_png_ptr->height + ni_png_pass_yinc[0] - 1 -
            ni_png_pass_ystart[0]) / ni_png_pass_yinc[0];
         ni_png_ptr->usr_width = (ni_png_ptr->width + ni_png_pass_inc[0] - 1 -
            ni_png_pass_start[0]) / ni_png_pass_inc[0];
      }
      else
      {
         ni_png_ptr->num_rows = ni_png_ptr->height;
         ni_png_ptr->usr_width = ni_png_ptr->width;
      }
   }
   else
#endif
   {
      ni_png_ptr->num_rows = ni_png_ptr->height;
      ni_png_ptr->usr_width = ni_png_ptr->width;
   }
   ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
   ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
}

/* Internal use only.  Called when finished processing a row of data. */
void /* PRIVATE */
ni_png_write_finish_row(ni_png_structp ni_png_ptr)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   /* arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* start of interlace block */
   int ni_png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* offset to next interlace block */
   int ni_png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   /* start of interlace block in the y direction */
   int ni_png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};

   /* offset to next interlace block in the y direction */
   int ni_png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif

   int ret;

   ni_png_debug(1, "in ni_png_write_finish_row\n");
   /* next row */
   ni_png_ptr->row_number++;

   /* see if we are done */
   if (ni_png_ptr->row_number < ni_png_ptr->num_rows)
      return;

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   /* if interlaced, go to next pass */
   if (ni_png_ptr->interlaced)
   {
      ni_png_ptr->row_number = 0;
      if (ni_png_ptr->transformations & PNG_INTERLACE)
      {
         ni_png_ptr->pass++;
      }
      else
      {
         /* loop until we find a non-zero width or height pass */
         do
         {
            ni_png_ptr->pass++;
            if (ni_png_ptr->pass >= 7)
               break;
            ni_png_ptr->usr_width = (ni_png_ptr->width +
               ni_png_pass_inc[ni_png_ptr->pass] - 1 -
               ni_png_pass_start[ni_png_ptr->pass]) /
               ni_png_pass_inc[ni_png_ptr->pass];
            ni_png_ptr->num_rows = (ni_png_ptr->height +
               ni_png_pass_yinc[ni_png_ptr->pass] - 1 -
               ni_png_pass_ystart[ni_png_ptr->pass]) /
               ni_png_pass_yinc[ni_png_ptr->pass];
            if (ni_png_ptr->transformations & PNG_INTERLACE)
               break;
         } while (ni_png_ptr->usr_width == 0 || ni_png_ptr->num_rows == 0);

      }

      /* reset the row above the image for the next pass */
      if (ni_png_ptr->pass < 7)
      {
         if (ni_png_ptr->prev_row != NULL)
            ni_png_memset(ni_png_ptr->prev_row, 0,
               (ni_png_size_t)(PNG_ROWBYTES(ni_png_ptr->usr_channels*
               ni_png_ptr->usr_bit_depth,ni_png_ptr->width))+1);
         return;
      }
   }
#endif

   /* if we get here, we've just written the last row, so we need
      to flush the compressor */
   do
   {
      /* tell the compressor we are done */
      ret = deflate(&ni_png_ptr->zstream, Z_FINISH);
      /* check for an error */
      if (ret == Z_OK)
      {
         /* check to see if we need more room */
         if (!(ni_png_ptr->zstream.avail_out))
         {
            ni_png_write_IDAT(ni_png_ptr, ni_png_ptr->zbuf, ni_png_ptr->zbuf_size);
            ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
            ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
         }
      }
      else if (ret != Z_STREAM_END)
      {
         if (ni_png_ptr->zstream.msg != NULL)
            ni_png_error(ni_png_ptr, ni_png_ptr->zstream.msg);
         else
            ni_png_error(ni_png_ptr, "zlib error");
      }
   } while (ret != Z_STREAM_END);

   /* write any extra space */
   if (ni_png_ptr->zstream.avail_out < ni_png_ptr->zbuf_size)
   {
      ni_png_write_IDAT(ni_png_ptr, ni_png_ptr->zbuf, ni_png_ptr->zbuf_size -
         ni_png_ptr->zstream.avail_out);
   }

   deflateReset(&ni_png_ptr->zstream);
   ni_png_ptr->zstream.data_type = Z_BINARY;
}

#if defined(PNG_WRITE_INTERLACING_SUPPORTED)
/* Pick out the correct pixels for the interlace pass.
 * The basic idea here is to go through the row with a source
 * pointer and a destination pointer (sp and dp), and copy the
 * correct pixels for the pass.  As the row gets compacted,
 * sp will always be >= dp, so we should never overwrite anything.
 * See the default: case for the easiest code to understand.
 */
void /* PRIVATE */
ni_png_do_write_interlace(ni_png_row_infop row_info, ni_png_bytep row, int pass)
{
#ifdef PNG_USE_LOCAL_ARRAYS
   /* arrays to facilitate easy interlacing - use pass (0 - 6) as index */

   /* start of interlace block */
   int ni_png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* offset to next interlace block */
   int ni_png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};
#endif

   ni_png_debug(1, "in ni_png_do_write_interlace\n");
   /* we don't have to do anything on the last pass (6) */
#if defined(PNG_USELESS_TESTS_SUPPORTED)
   if (row != NULL && row_info != NULL && pass < 6)
#else
   if (pass < 6)
#endif
   {
      /* each pixel depth is handled separately */
      switch (row_info->pixel_depth)
      {
         case 1:
         {
            ni_png_bytep sp;
            ni_png_bytep dp;
            int shift;
            int d;
            int value;
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = row_info->width;

            dp = row;
            d = 0;
            shift = 7;
            for (i = ni_png_pass_start[pass]; i < row_width;
               i += ni_png_pass_inc[pass])
            {
               sp = row + (ni_png_size_t)(i >> 3);
               value = (int)(*sp >> (7 - (int)(i & 0x07))) & 0x01;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 7;
                  *dp++ = (ni_png_byte)d;
                  d = 0;
               }
               else
                  shift--;

            }
            if (shift != 7)
               *dp = (ni_png_byte)d;
            break;
         }
         case 2:
         {
            ni_png_bytep sp;
            ni_png_bytep dp;
            int shift;
            int d;
            int value;
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = row_info->width;

            dp = row;
            shift = 6;
            d = 0;
            for (i = ni_png_pass_start[pass]; i < row_width;
               i += ni_png_pass_inc[pass])
            {
               sp = row + (ni_png_size_t)(i >> 2);
               value = (*sp >> ((3 - (int)(i & 0x03)) << 1)) & 0x03;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 6;
                  *dp++ = (ni_png_byte)d;
                  d = 0;
               }
               else
                  shift -= 2;
            }
            if (shift != 6)
                   *dp = (ni_png_byte)d;
            break;
         }
         case 4:
         {
            ni_png_bytep sp;
            ni_png_bytep dp;
            int shift;
            int d;
            int value;
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = row_info->width;

            dp = row;
            shift = 4;
            d = 0;
            for (i = ni_png_pass_start[pass]; i < row_width;
               i += ni_png_pass_inc[pass])
            {
               sp = row + (ni_png_size_t)(i >> 1);
               value = (*sp >> ((1 - (int)(i & 0x01)) << 2)) & 0x0f;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 4;
                  *dp++ = (ni_png_byte)d;
                  d = 0;
               }
               else
                  shift -= 4;
            }
            if (shift != 4)
               *dp = (ni_png_byte)d;
            break;
         }
         default:
         {
            ni_png_bytep sp;
            ni_png_bytep dp;
            ni_png_uint_32 i;
            ni_png_uint_32 row_width = row_info->width;
            ni_png_size_t pixel_bytes;

            /* start at the beginning */
            dp = row;
            /* find out how many bytes each pixel takes up */
            pixel_bytes = (row_info->pixel_depth >> 3);
            /* loop through the row, only looking at the pixels that
               matter */
            for (i = ni_png_pass_start[pass]; i < row_width;
               i += ni_png_pass_inc[pass])
            {
               /* find out where the original pixel is */
               sp = row + (ni_png_size_t)i * pixel_bytes;
               /* move the pixel */
               if (dp != sp)
                  ni_png_memcpy(dp, sp, pixel_bytes);
               /* next pixel */
               dp += pixel_bytes;
            }
            break;
         }
      }
      /* set new row width */
      row_info->width = (row_info->width +
         ni_png_pass_inc[pass] - 1 -
         ni_png_pass_start[pass]) /
         ni_png_pass_inc[pass];
         row_info->rowbytes = PNG_ROWBYTES(row_info->pixel_depth,
            row_info->width);
   }
}
#endif

/* This filters the row, chooses which filter to use, if it has not already
 * been specified by the application, and then writes the row out with the
 * chosen filter.
 */
#define PNG_MAXSUM (((ni_png_uint_32)(-1)) >> 1)
#define PNG_HISHIFT 10
#define PNG_LOMASK ((ni_png_uint_32)0xffffL)
#define PNG_HIMASK ((ni_png_uint_32)(~PNG_LOMASK >> PNG_HISHIFT))
void /* PRIVATE */
ni_png_write_find_filter(ni_png_structp ni_png_ptr, ni_png_row_infop row_info)
{
   ni_png_bytep prev_row, best_row, row_buf;
   ni_png_uint_32 mins, bpp;
   ni_png_byte filter_to_do = ni_png_ptr->do_filter;
   ni_png_uint_32 row_bytes = row_info->rowbytes;
#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   int num_p_filters = (int)ni_png_ptr->num_prev_filters;
#endif

   ni_png_debug(1, "in ni_png_write_find_filter\n");
   /* find out how many bytes offset each pixel is */
   bpp = (row_info->pixel_depth + 7) >> 3;

   prev_row = ni_png_ptr->prev_row;
   best_row = row_buf = ni_png_ptr->row_buf;
   mins = PNG_MAXSUM;

   /* The prediction method we use is to find which method provides the
    * smallest value when summing the absolute values of the distances
    * from zero, using anything >= 128 as negative numbers.  This is known
    * as the "minimum sum of absolute differences" heuristic.  Other
    * heuristics are the "weighted minimum sum of absolute differences"
    * (experimental and can in theory improve compression), and the "zlib
    * predictive" method (not implemented yet), which does test compressions
    * of lines using different filter methods, and then chooses the
    * (series of) filter(s) that give minimum compressed data size (VERY
    * computationally expensive).
    *
    * GRR 980525:  consider also
    *   (1) minimum sum of absolute differences from running average (i.e.,
    *       keep running sum of non-absolute differences & count of bytes)
    *       [track dispersion, too?  restart average if dispersion too large?]
    *  (1b) minimum sum of absolute differences from sliding average, probably
    *       with window size <= deflate window (usually 32K)
    *   (2) minimum sum of squared differences from zero or running average
    *       (i.e., ~ root-mean-square approach)
    */


   /* We don't need to test the 'no filter' case if this is the only filter
    * that has been chosen, as it doesn't actually do anything to the data.
    */
   if ((filter_to_do & PNG_FILTER_NONE) &&
       filter_to_do != PNG_FILTER_NONE)
   {
      ni_png_bytep rp;
      ni_png_uint_32 sum = 0;
      ni_png_uint_32 i;
      int v;

      for (i = 0, rp = row_buf + 1; i < row_bytes; i++, rp++)
      {
         v = *rp;
         sum += (v < 128) ? v : 256 - v;
      }

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         ni_png_uint_32 sumhi, sumlo;
         int j;
         sumlo = sum & PNG_LOMASK;
         sumhi = (sum >> PNG_HISHIFT) & PNG_HIMASK; /* Gives us some footroom */

         /* Reduce the sum if we match any of the previous rows */
         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_NONE)
            {
               sumlo = (sumlo * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               sumhi = (sumhi * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         /* Factor in the cost of this filter (this is here for completeness,
          * but it makes no sense to have a "cost" for the NONE filter, as
          * it has the minimum possible computational cost - none).
          */
         sumlo = (sumlo * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_NONE]) >>
            PNG_COST_SHIFT;
         sumhi = (sumhi * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_NONE]) >>
            PNG_COST_SHIFT;

         if (sumhi > PNG_HIMASK)
            sum = PNG_MAXSUM;
         else
            sum = (sumhi << PNG_HISHIFT) + sumlo;
      }
#endif
      mins = sum;
   }

   /* sub filter */
   if (filter_to_do == PNG_FILTER_SUB)
   /* it's the only filter so no testing is needed */
   {
      ni_png_bytep rp, lp, dp;
      ni_png_uint_32 i;
      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->sub_row + 1; i < bpp;
           i++, rp++, dp++)
      {
         *dp = *rp;
      }
      for (lp = row_buf + 1; i < row_bytes;
         i++, rp++, lp++, dp++)
      {
         *dp = (ni_png_byte)(((int)*rp - (int)*lp) & 0xff);
      }
      best_row = ni_png_ptr->sub_row;
   }

   else if (filter_to_do & PNG_FILTER_SUB)
   {
      ni_png_bytep rp, dp, lp;
      ni_png_uint_32 sum = 0, lmins = mins;
      ni_png_uint_32 i;
      int v;

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      /* We temporarily increase the "minimum sum" by the factor we
       * would reduce the sum of this filter, so that we can do the
       * early exit comparison without scaling the sum each time.
       */
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 lmhi, lmlo;
         lmlo = lmins & PNG_LOMASK;
         lmhi = (lmins >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_SUB)
            {
               lmlo = (lmlo * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               lmhi = (lmhi * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         lmlo = (lmlo * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_SUB]) >>
            PNG_COST_SHIFT;
         lmhi = (lmhi * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_SUB]) >>
            PNG_COST_SHIFT;

         if (lmhi > PNG_HIMASK)
            lmins = PNG_MAXSUM;
         else
            lmins = (lmhi << PNG_HISHIFT) + lmlo;
      }
#endif

      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->sub_row + 1; i < bpp;
           i++, rp++, dp++)
      {
         v = *dp = *rp;

         sum += (v < 128) ? v : 256 - v;
      }
      for (lp = row_buf + 1; i < row_bytes;
         i++, rp++, lp++, dp++)
      {
         v = *dp = (ni_png_byte)(((int)*rp - (int)*lp) & 0xff);

         sum += (v < 128) ? v : 256 - v;

         if (sum > lmins)  /* We are already worse, don't continue. */
            break;
      }

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 sumhi, sumlo;
         sumlo = sum & PNG_LOMASK;
         sumhi = (sum >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_SUB)
            {
               sumlo = (sumlo * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               sumhi = (sumhi * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         sumlo = (sumlo * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_SUB]) >>
            PNG_COST_SHIFT;
         sumhi = (sumhi * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_SUB]) >>
            PNG_COST_SHIFT;

         if (sumhi > PNG_HIMASK)
            sum = PNG_MAXSUM;
         else
            sum = (sumhi << PNG_HISHIFT) + sumlo;
      }
#endif

      if (sum < mins)
      {
         mins = sum;
         best_row = ni_png_ptr->sub_row;
      }
   }

   /* up filter */
   if (filter_to_do == PNG_FILTER_UP)
   {
      ni_png_bytep rp, dp, pp;
      ni_png_uint_32 i;

      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->up_row + 1,
           pp = prev_row + 1; i < row_bytes;
           i++, rp++, pp++, dp++)
      {
         *dp = (ni_png_byte)(((int)*rp - (int)*pp) & 0xff);
      }
      best_row = ni_png_ptr->up_row;
   }

   else if (filter_to_do & PNG_FILTER_UP)
   {
      ni_png_bytep rp, dp, pp;
      ni_png_uint_32 sum = 0, lmins = mins;
      ni_png_uint_32 i;
      int v;


#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 lmhi, lmlo;
         lmlo = lmins & PNG_LOMASK;
         lmhi = (lmins >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_UP)
            {
               lmlo = (lmlo * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               lmhi = (lmhi * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         lmlo = (lmlo * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_UP]) >>
            PNG_COST_SHIFT;
         lmhi = (lmhi * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_UP]) >>
            PNG_COST_SHIFT;

         if (lmhi > PNG_HIMASK)
            lmins = PNG_MAXSUM;
         else
            lmins = (lmhi << PNG_HISHIFT) + lmlo;
      }
#endif

      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->up_row + 1,
           pp = prev_row + 1; i < row_bytes; i++)
      {
         v = *dp++ = (ni_png_byte)(((int)*rp++ - (int)*pp++) & 0xff);

         sum += (v < 128) ? v : 256 - v;

         if (sum > lmins)  /* We are already worse, don't continue. */
            break;
      }

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 sumhi, sumlo;
         sumlo = sum & PNG_LOMASK;
         sumhi = (sum >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_UP)
            {
               sumlo = (sumlo * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               sumhi = (sumhi * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         sumlo = (sumlo * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_UP]) >>
            PNG_COST_SHIFT;
         sumhi = (sumhi * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_UP]) >>
            PNG_COST_SHIFT;

         if (sumhi > PNG_HIMASK)
            sum = PNG_MAXSUM;
         else
            sum = (sumhi << PNG_HISHIFT) + sumlo;
      }
#endif

      if (sum < mins)
      {
         mins = sum;
         best_row = ni_png_ptr->up_row;
      }
   }

   /* avg filter */
   if (filter_to_do == PNG_FILTER_AVG)
   {
      ni_png_bytep rp, dp, pp, lp;
      ni_png_uint_32 i;
      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->avg_row + 1,
           pp = prev_row + 1; i < bpp; i++)
      {
         *dp++ = (ni_png_byte)(((int)*rp++ - ((int)*pp++ / 2)) & 0xff);
      }
      for (lp = row_buf + 1; i < row_bytes; i++)
      {
         *dp++ = (ni_png_byte)(((int)*rp++ - (((int)*pp++ + (int)*lp++) / 2))
                 & 0xff);
      }
      best_row = ni_png_ptr->avg_row;
   }

   else if (filter_to_do & PNG_FILTER_AVG)
   {
      ni_png_bytep rp, dp, pp, lp;
      ni_png_uint_32 sum = 0, lmins = mins;
      ni_png_uint_32 i;
      int v;

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 lmhi, lmlo;
         lmlo = lmins & PNG_LOMASK;
         lmhi = (lmins >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_AVG)
            {
               lmlo = (lmlo * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               lmhi = (lmhi * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         lmlo = (lmlo * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_AVG]) >>
            PNG_COST_SHIFT;
         lmhi = (lmhi * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_AVG]) >>
            PNG_COST_SHIFT;

         if (lmhi > PNG_HIMASK)
            lmins = PNG_MAXSUM;
         else
            lmins = (lmhi << PNG_HISHIFT) + lmlo;
      }
#endif

      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->avg_row + 1,
           pp = prev_row + 1; i < bpp; i++)
      {
         v = *dp++ = (ni_png_byte)(((int)*rp++ - ((int)*pp++ / 2)) & 0xff);

         sum += (v < 128) ? v : 256 - v;
      }
      for (lp = row_buf + 1; i < row_bytes; i++)
      {
         v = *dp++ =
          (ni_png_byte)(((int)*rp++ - (((int)*pp++ + (int)*lp++) / 2)) & 0xff);

         sum += (v < 128) ? v : 256 - v;

         if (sum > lmins)  /* We are already worse, don't continue. */
            break;
      }

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 sumhi, sumlo;
         sumlo = sum & PNG_LOMASK;
         sumhi = (sum >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_NONE)
            {
               sumlo = (sumlo * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               sumhi = (sumhi * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         sumlo = (sumlo * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_AVG]) >>
            PNG_COST_SHIFT;
         sumhi = (sumhi * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_AVG]) >>
            PNG_COST_SHIFT;

         if (sumhi > PNG_HIMASK)
            sum = PNG_MAXSUM;
         else
            sum = (sumhi << PNG_HISHIFT) + sumlo;
      }
#endif

      if (sum < mins)
      {
         mins = sum;
         best_row = ni_png_ptr->avg_row;
      }
   }

   /* Paeth filter */
   if (filter_to_do == PNG_FILTER_PAETH)
   {
      ni_png_bytep rp, dp, pp, cp, lp;
      ni_png_uint_32 i;
      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->paeth_row + 1,
           pp = prev_row + 1; i < bpp; i++)
      {
         *dp++ = (ni_png_byte)(((int)*rp++ - (int)*pp++) & 0xff);
      }

      for (lp = row_buf + 1, cp = prev_row + 1; i < row_bytes; i++)
      {
         int a, b, c, pa, pb, pc, p;

         b = *pp++;
         c = *cp++;
         a = *lp++;

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

         p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;

         *dp++ = (ni_png_byte)(((int)*rp++ - p) & 0xff);
      }
      best_row = ni_png_ptr->paeth_row;
   }

   else if (filter_to_do & PNG_FILTER_PAETH)
   {
      ni_png_bytep rp, dp, pp, cp, lp;
      ni_png_uint_32 sum = 0, lmins = mins;
      ni_png_uint_32 i;
      int v;

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 lmhi, lmlo;
         lmlo = lmins & PNG_LOMASK;
         lmhi = (lmins >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_PAETH)
            {
               lmlo = (lmlo * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               lmhi = (lmhi * ni_png_ptr->inv_filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         lmlo = (lmlo * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_PAETH]) >>
            PNG_COST_SHIFT;
         lmhi = (lmhi * ni_png_ptr->inv_filter_costs[PNG_FILTER_VALUE_PAETH]) >>
            PNG_COST_SHIFT;

         if (lmhi > PNG_HIMASK)
            lmins = PNG_MAXSUM;
         else
            lmins = (lmhi << PNG_HISHIFT) + lmlo;
      }
#endif

      for (i = 0, rp = row_buf + 1, dp = ni_png_ptr->paeth_row + 1,
           pp = prev_row + 1; i < bpp; i++)
      {
         v = *dp++ = (ni_png_byte)(((int)*rp++ - (int)*pp++) & 0xff);

         sum += (v < 128) ? v : 256 - v;
      }

      for (lp = row_buf + 1, cp = prev_row + 1; i < row_bytes; i++)
      {
         int a, b, c, pa, pb, pc, p;

         b = *pp++;
         c = *cp++;
         a = *lp++;

#ifndef PNG_SLOW_PAETH
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
         p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;
#else /* PNG_SLOW_PAETH */
         p = a + b - c;
         pa = abs(p - a);
         pb = abs(p - b);
         pc = abs(p - c);
         if (pa <= pb && pa <= pc)
            p = a;
         else if (pb <= pc)
            p = b;
         else
            p = c;
#endif /* PNG_SLOW_PAETH */

         v = *dp++ = (ni_png_byte)(((int)*rp++ - p) & 0xff);

         sum += (v < 128) ? v : 256 - v;

         if (sum > lmins)  /* We are already worse, don't continue. */
            break;
      }

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
      if (ni_png_ptr->heuristic_method == PNG_FILTER_HEURISTIC_WEIGHTED)
      {
         int j;
         ni_png_uint_32 sumhi, sumlo;
         sumlo = sum & PNG_LOMASK;
         sumhi = (sum >> PNG_HISHIFT) & PNG_HIMASK;

         for (j = 0; j < num_p_filters; j++)
         {
            if (ni_png_ptr->prev_filters[j] == PNG_FILTER_VALUE_PAETH)
            {
               sumlo = (sumlo * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
               sumhi = (sumhi * ni_png_ptr->filter_weights[j]) >>
                  PNG_WEIGHT_SHIFT;
            }
         }

         sumlo = (sumlo * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_PAETH]) >>
            PNG_COST_SHIFT;
         sumhi = (sumhi * ni_png_ptr->filter_costs[PNG_FILTER_VALUE_PAETH]) >>
            PNG_COST_SHIFT;

         if (sumhi > PNG_HIMASK)
            sum = PNG_MAXSUM;
         else
            sum = (sumhi << PNG_HISHIFT) + sumlo;
      }
#endif

      if (sum < mins)
      {
         best_row = ni_png_ptr->paeth_row;
      }
   }

   /* Do the actual writing of the filtered row data from the chosen filter. */

   ni_png_write_filtered_row(ni_png_ptr, best_row);

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   /* Save the type of filter we picked this time for future calculations */
   if (ni_png_ptr->num_prev_filters > 0)
   {
      int j;
      for (j = 1; j < num_p_filters; j++)
      {
         ni_png_ptr->prev_filters[j] = ni_png_ptr->prev_filters[j - 1];
      }
      ni_png_ptr->prev_filters[j] = best_row[0];
   }
#endif
}


/* Do the actual writing of a previously filtered row. */
void /* PRIVATE */
ni_png_write_filtered_row(ni_png_structp ni_png_ptr, ni_png_bytep filtered_row)
{
   ni_png_debug(1, "in ni_png_write_filtered_row\n");
   ni_png_debug1(2, "filter = %d\n", filtered_row[0]);
   /* set up the zlib input buffer */

   ni_png_ptr->zstream.next_in = filtered_row;
   ni_png_ptr->zstream.avail_in = (uInt)ni_png_ptr->row_info.rowbytes + 1;
   /* repeat until we have compressed all the data */
   do
   {
      int ret; /* return of zlib */

      /* compress the data */
      ret = deflate(&ni_png_ptr->zstream, Z_NO_FLUSH);
      /* check for compression errors */
      if (ret != Z_OK)
      {
         if (ni_png_ptr->zstream.msg != NULL)
            ni_png_error(ni_png_ptr, ni_png_ptr->zstream.msg);
         else
            ni_png_error(ni_png_ptr, "zlib error");
      }

      /* see if it is time to write another IDAT */
      if (!(ni_png_ptr->zstream.avail_out))
      {
         /* write the IDAT and reset the zlib output buffer */
         ni_png_write_IDAT(ni_png_ptr, ni_png_ptr->zbuf, ni_png_ptr->zbuf_size);
         ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
         ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
      }
   /* repeat until all data has been compressed */
   } while (ni_png_ptr->zstream.avail_in);

   /* swap the current and previous rows */
   if (ni_png_ptr->prev_row != NULL)
   {
      ni_png_bytep tptr;

      tptr = ni_png_ptr->prev_row;
      ni_png_ptr->prev_row = ni_png_ptr->row_buf;
      ni_png_ptr->row_buf = tptr;
   }

   /* finish row - updates counters and flushes zlib if last row */
   ni_png_write_finish_row(ni_png_ptr);

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   ni_png_ptr->flush_rows++;

   if (ni_png_ptr->flush_dist > 0 &&
       ni_png_ptr->flush_rows >= ni_png_ptr->flush_dist)
   {
      ni_png_write_flush(ni_png_ptr);
   }
#endif
}
#endif /* PNG_WRITE_SUPPORTED */
