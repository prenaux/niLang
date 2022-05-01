
/* pngset.c - storage of image information into info struct
 *
 * Last changed in libpng 1.2.17 May 15, 2007
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2007 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * The functions here are used during reads to store data from the file
 * into the info struct, and during writes to store application data
 * into the info struct for writing into the file.  This abstracts the
 * info struct and allows us to change the structure in the future.
 */

#define PNG_INTERNAL
#include "png.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)

#if defined(PNG_bKGD_SUPPORTED)
void PNGAPI
ni_png_set_bKGD(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_color_16p background)
{
   ni_png_debug1(1, "in %s storage function\n", "bKGD");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   ni_png_memcpy(&(info_ptr->background), background, ni_png_sizeof(ni_png_color_16));
   info_ptr->valid |= PNG_INFO_bKGD;
}
#endif

#if defined(PNG_cHRM_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
void PNGAPI
ni_png_set_cHRM(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   double white_x, double white_y, double red_x, double red_y,
   double green_x, double green_y, double blue_x, double blue_y)
{
   ni_png_debug1(1, "in %s storage function\n", "cHRM");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   if (white_x < 0.0 || white_y < 0.0 ||
         red_x < 0.0 ||   red_y < 0.0 ||
       green_x < 0.0 || green_y < 0.0 ||
        blue_x < 0.0 ||  blue_y < 0.0)
   {
      ni_png_warning(ni_png_ptr,
        "Ignoring attempt to set negative chromaticity value");
      return;
   }
   if (white_x > 21474.83 || white_y > 21474.83 ||
         red_x > 21474.83 ||   red_y > 21474.83 ||
       green_x > 21474.83 || green_y > 21474.83 ||
        blue_x > 21474.83 ||  blue_y > 21474.83)
   {
      ni_png_warning(ni_png_ptr,
        "Ignoring attempt to set chromaticity value exceeding 21474.83");
      return;
   }

   info_ptr->x_white = (float)white_x;
   info_ptr->y_white = (float)white_y;
   info_ptr->x_red   = (float)red_x;
   info_ptr->y_red   = (float)red_y;
   info_ptr->x_green = (float)green_x;
   info_ptr->y_green = (float)green_y;
   info_ptr->x_blue  = (float)blue_x;
   info_ptr->y_blue  = (float)blue_y;
#ifdef PNG_FIXED_POINT_SUPPORTED
   info_ptr->int_x_white = (ni_png_fixed_point)(white_x*100000.+0.5);
   info_ptr->int_y_white = (ni_png_fixed_point)(white_y*100000.+0.5);
   info_ptr->int_x_red   = (ni_png_fixed_point)(  red_x*100000.+0.5);
   info_ptr->int_y_red   = (ni_png_fixed_point)(  red_y*100000.+0.5);
   info_ptr->int_x_green = (ni_png_fixed_point)(green_x*100000.+0.5);
   info_ptr->int_y_green = (ni_png_fixed_point)(green_y*100000.+0.5);
   info_ptr->int_x_blue  = (ni_png_fixed_point)( blue_x*100000.+0.5);
   info_ptr->int_y_blue  = (ni_png_fixed_point)( blue_y*100000.+0.5);
#endif
   info_ptr->valid |= PNG_INFO_cHRM;
}
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
void PNGAPI
ni_png_set_cHRM_fixed(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_fixed_point white_x, ni_png_fixed_point white_y, ni_png_fixed_point red_x,
   ni_png_fixed_point red_y, ni_png_fixed_point green_x, ni_png_fixed_point green_y,
   ni_png_fixed_point blue_x, ni_png_fixed_point blue_y)
{
   ni_png_debug1(1, "in %s storage function\n", "cHRM");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   if (white_x < 0 || white_y < 0 ||
         red_x < 0 ||   red_y < 0 ||
       green_x < 0 || green_y < 0 ||
        blue_x < 0 ||  blue_y < 0)
   {
      ni_png_warning(ni_png_ptr,
        "Ignoring attempt to set negative chromaticity value");
      return;
   }
#ifdef PNG_FLOATING_POINT_SUPPORTED
   if (white_x > (double) PNG_UINT_31_MAX ||
       white_y > (double) PNG_UINT_31_MAX ||
         red_x > (double) PNG_UINT_31_MAX ||
         red_y > (double) PNG_UINT_31_MAX ||
       green_x > (double) PNG_UINT_31_MAX ||
       green_y > (double) PNG_UINT_31_MAX ||
        blue_x > (double) PNG_UINT_31_MAX ||
        blue_y > (double) PNG_UINT_31_MAX)
#else
   if (white_x > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L ||
       white_y > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L ||
         red_x > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L ||
         red_y > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L ||
       green_x > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L ||
       green_y > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L ||
        blue_x > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L ||
        blue_y > (ni_png_fixed_point) PNG_UINT_31_MAX/100000L)
#endif
   {
      ni_png_warning(ni_png_ptr,
        "Ignoring attempt to set chromaticity value exceeding 21474.83");
      return;
   }
   info_ptr->int_x_white = white_x;
   info_ptr->int_y_white = white_y;
   info_ptr->int_x_red   = red_x;
   info_ptr->int_y_red   = red_y;
   info_ptr->int_x_green = green_x;
   info_ptr->int_y_green = green_y;
   info_ptr->int_x_blue  = blue_x;
   info_ptr->int_y_blue  = blue_y;
#ifdef PNG_FLOATING_POINT_SUPPORTED
   info_ptr->x_white = (float)(white_x/100000.);
   info_ptr->y_white = (float)(white_y/100000.);
   info_ptr->x_red   = (float)(  red_x/100000.);
   info_ptr->y_red   = (float)(  red_y/100000.);
   info_ptr->x_green = (float)(green_x/100000.);
   info_ptr->y_green = (float)(green_y/100000.);
   info_ptr->x_blue  = (float)( blue_x/100000.);
   info_ptr->y_blue  = (float)( blue_y/100000.);
#endif
   info_ptr->valid |= PNG_INFO_cHRM;
}
#endif
#endif

#if defined(PNG_gAMA_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
void PNGAPI
ni_png_set_gAMA(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, double file_gamma)
{
   double gamma;
   ni_png_debug1(1, "in %s storage function\n", "gAMA");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   /* Check for overflow */
   if (file_gamma > 21474.83)
   {
      ni_png_warning(ni_png_ptr, "Limiting gamma to 21474.83");
      gamma=21474.83;
   }
   else
      gamma=file_gamma;
   info_ptr->gamma = (float)gamma;
#ifdef PNG_FIXED_POINT_SUPPORTED
   info_ptr->int_gamma = (int)(gamma*100000.+.5);
#endif
   info_ptr->valid |= PNG_INFO_gAMA;
   if(gamma == 0.0)
      ni_png_warning(ni_png_ptr, "Setting gamma=0");
}
#endif
void PNGAPI
ni_png_set_gAMA_fixed(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_fixed_point
   int_gamma)
{
   ni_png_fixed_point gamma;

   ni_png_debug1(1, "in %s storage function\n", "gAMA");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   if (int_gamma > (ni_png_fixed_point) PNG_UINT_31_MAX)
   {
     ni_png_warning(ni_png_ptr, "Limiting gamma to 21474.83");
     gamma=PNG_UINT_31_MAX;
   }
   else
   {
     if (int_gamma < 0)
     {
       ni_png_warning(ni_png_ptr, "Setting negative gamma to zero");
       gamma=0;
     }
     else
       gamma=int_gamma;
   }
#ifdef PNG_FLOATING_POINT_SUPPORTED
   info_ptr->gamma = (float)(gamma/100000.);
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   info_ptr->int_gamma = gamma;
#endif
   info_ptr->valid |= PNG_INFO_gAMA;
   if(gamma == 0)
      ni_png_warning(ni_png_ptr, "Setting gamma=0");
}
#endif

#if defined(PNG_hIST_SUPPORTED)
void PNGAPI
ni_png_set_hIST(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_uint_16p hist)
{
   int i;

   ni_png_debug1(1, "in %s storage function\n", "hIST");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;
   if (info_ptr->num_palette <= 0 || info_ptr->num_palette
       > PNG_MAX_PALETTE_LENGTH)
   {
       ni_png_warning(ni_png_ptr,
          "Invalid palette size, hIST allocation skipped.");
       return;
   }

#ifdef PNG_FREE_ME_SUPPORTED
   ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_HIST, 0);
#endif
   /* Changed from info->num_palette to PNG_MAX_PALETTE_LENGTH in version
      1.2.1 */
   ni_png_ptr->hist = (ni_png_uint_16p)ni_png_malloc_warn(ni_png_ptr,
      (ni_png_uint_32)(PNG_MAX_PALETTE_LENGTH * ni_png_sizeof (ni_png_uint_16)));
   if (ni_png_ptr->hist == NULL)
     {
       ni_png_warning(ni_png_ptr, "Insufficient memory for hIST chunk data.");
       return;
     }

   for (i = 0; i < info_ptr->num_palette; i++)
       ni_png_ptr->hist[i] = hist[i];
   info_ptr->hist = ni_png_ptr->hist;
   info_ptr->valid |= PNG_INFO_hIST;

#ifdef PNG_FREE_ME_SUPPORTED
   info_ptr->free_me |= PNG_FREE_HIST;
#else
   ni_png_ptr->flags |= PNG_FLAG_FREE_HIST;
#endif
}
#endif

void PNGAPI
ni_png_set_IHDR(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_uint_32 width, ni_png_uint_32 height, int bit_depth,
   int color_type, int interlace_type, int compression_type,
   int filter_type)
{
   ni_png_debug1(1, "in %s storage function\n", "IHDR");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   /* check for width and height valid values */
   if (width == 0 || height == 0)
      ni_png_error(ni_png_ptr, "Image width or height is zero in IHDR");
#ifdef PNG_SET_USER_LIMITS_SUPPORTED
   if (width > ni_png_ptr->user_width_max || height > ni_png_ptr->user_height_max)
      ni_png_error(ni_png_ptr, "image size exceeds user limits in IHDR");
#else
   if (width > PNG_USER_WIDTH_MAX || height > PNG_USER_HEIGHT_MAX)
      ni_png_error(ni_png_ptr, "image size exceeds user limits in IHDR");
#endif
   if (width > PNG_UINT_31_MAX || height > PNG_UINT_31_MAX)
      ni_png_error(ni_png_ptr, "Invalid image size in IHDR");
   if ( width > (PNG_UINT_32_MAX
                 >> 3)      /* 8-byte RGBA pixels */
                 - 64       /* bigrowbuf hack */
                 - 1        /* filter byte */
                 - 7*8      /* rounding of width to multiple of 8 pixels */
                 - 8)       /* extra max_pixel_depth pad */
      ni_png_warning(ni_png_ptr, "Width is too large for libpng to process pixels");

   /* check other values */
   if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 &&
      bit_depth != 8 && bit_depth != 16)
      ni_png_error(ni_png_ptr, "Invalid bit depth in IHDR");

   if (color_type < 0 || color_type == 1 ||
      color_type == 5 || color_type > 6)
      ni_png_error(ni_png_ptr, "Invalid color type in IHDR");

   if (((color_type == PNG_COLOR_TYPE_PALETTE) && bit_depth > 8) ||
       ((color_type == PNG_COLOR_TYPE_RGB ||
         color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
         color_type == PNG_COLOR_TYPE_RGB_ALPHA) && bit_depth < 8))
      ni_png_error(ni_png_ptr, "Invalid color type/bit depth combination in IHDR");

   if (interlace_type >= PNG_INTERLACE_LAST)
      ni_png_error(ni_png_ptr, "Unknown interlace method in IHDR");

   if (compression_type != PNG_COMPRESSION_TYPE_BASE)
      ni_png_error(ni_png_ptr, "Unknown compression method in IHDR");

#if defined(PNG_MNG_FEATURES_SUPPORTED)
   /* Accept filter_method 64 (intrapixel differencing) only if
    * 1. Libpng was compiled with PNG_MNG_FEATURES_SUPPORTED and
    * 2. Libpng did not read a PNG signature (this filter_method is only
    *    used in PNG datastreams that are embedded in MNG datastreams) and
    * 3. The application called ni_png_permit_mng_features with a mask that
    *    included PNG_FLAG_MNG_FILTER_64 and
    * 4. The filter_method is 64 and
    * 5. The color_type is RGB or RGBA
    */
   if((ni_png_ptr->mode&PNG_HAVE_PNG_SIGNATURE)&&ni_png_ptr->mng_features_permitted)
      ni_png_warning(ni_png_ptr,"MNG features are not allowed in a PNG datastream");
   if(filter_type != PNG_FILTER_TYPE_BASE)
   {
     if(!((ni_png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) &&
        (filter_type == PNG_INTRAPIXEL_DIFFERENCING) &&
        ((ni_png_ptr->mode&PNG_HAVE_PNG_SIGNATURE) == 0) &&
        (color_type == PNG_COLOR_TYPE_RGB ||
         color_type == PNG_COLOR_TYPE_RGB_ALPHA)))
        ni_png_error(ni_png_ptr, "Unknown filter method in IHDR");
     if(ni_png_ptr->mode&PNG_HAVE_PNG_SIGNATURE)
        ni_png_warning(ni_png_ptr, "Invalid filter method in IHDR");
   }
#else
   if(filter_type != PNG_FILTER_TYPE_BASE)
      ni_png_error(ni_png_ptr, "Unknown filter method in IHDR");
#endif

   info_ptr->width = width;
   info_ptr->height = height;
   info_ptr->bit_depth = (ni_png_byte)bit_depth;
   info_ptr->color_type =(ni_png_byte) color_type;
   info_ptr->compression_type = (ni_png_byte)compression_type;
   info_ptr->filter_type = (ni_png_byte)filter_type;
   info_ptr->interlace_type = (ni_png_byte)interlace_type;
   if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      info_ptr->channels = 1;
   else if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
      info_ptr->channels = 3;
   else
      info_ptr->channels = 1;
   if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      info_ptr->channels++;
   info_ptr->pixel_depth = (ni_png_byte)(info_ptr->channels * info_ptr->bit_depth);

   /* check for potential overflow */
   if ( width > (PNG_UINT_32_MAX
                 >> 3)      /* 8-byte RGBA pixels */
                 - 64       /* bigrowbuf hack */
                 - 1        /* filter byte */
                 - 7*8      /* rounding of width to multiple of 8 pixels */
                 - 8)       /* extra max_pixel_depth pad */
      info_ptr->rowbytes = (ni_png_size_t)0;
   else
      info_ptr->rowbytes = PNG_ROWBYTES(info_ptr->pixel_depth,width);
}

#if defined(PNG_oFFs_SUPPORTED)
void PNGAPI
ni_png_set_oFFs(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_int_32 offset_x, ni_png_int_32 offset_y, int unit_type)
{
   ni_png_debug1(1, "in %s storage function\n", "oFFs");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->x_offset = offset_x;
   info_ptr->y_offset = offset_y;
   info_ptr->offset_unit_type = (ni_png_byte)unit_type;
   info_ptr->valid |= PNG_INFO_oFFs;
}
#endif

#if defined(PNG_pCAL_SUPPORTED)
void PNGAPI
ni_png_set_pCAL(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_charp purpose, ni_png_int_32 X0, ni_png_int_32 X1, int type, int nparams,
   ni_png_charp units, ni_png_charpp params)
{
   ni_png_uint_32 length;
   int i;

   ni_png_debug1(1, "in %s storage function\n", "pCAL");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   length = ni_png_strlen(purpose) + 1;
   ni_png_debug1(3, "allocating purpose for info (%lu bytes)\n", length);
   info_ptr->pcal_purpose = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length);
   if (info_ptr->pcal_purpose == NULL)
     {
       ni_png_warning(ni_png_ptr, "Insufficient memory for pCAL purpose.");
       return;
     }
   ni_png_memcpy(info_ptr->pcal_purpose, purpose, (ni_png_size_t)length);

   ni_png_debug(3, "storing X0, X1, type, and nparams in info\n");
   info_ptr->pcal_X0 = X0;
   info_ptr->pcal_X1 = X1;
   info_ptr->pcal_type = (ni_png_byte)type;
   info_ptr->pcal_nparams = (ni_png_byte)nparams;

   length = ni_png_strlen(units) + 1;
   ni_png_debug1(3, "allocating units for info (%lu bytes)\n", length);
   info_ptr->pcal_units = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length);
   if (info_ptr->pcal_units == NULL)
     {
       ni_png_warning(ni_png_ptr, "Insufficient memory for pCAL units.");
       return;
     }
   ni_png_memcpy(info_ptr->pcal_units, units, (ni_png_size_t)length);

   info_ptr->pcal_params = (ni_png_charpp)ni_png_malloc_warn(ni_png_ptr,
      (ni_png_uint_32)((nparams + 1) * ni_png_sizeof(ni_png_charp)));
   if (info_ptr->pcal_params == NULL)
     {
       ni_png_warning(ni_png_ptr, "Insufficient memory for pCAL params.");
       return;
     }

   info_ptr->pcal_params[nparams] = NULL;

   for (i = 0; i < nparams; i++)
   {
      length = ni_png_strlen(params[i]) + 1;
      ni_png_debug2(3, "allocating parameter %d for info (%lu bytes)\n", i, length);
      info_ptr->pcal_params[i] = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length);
      if (info_ptr->pcal_params[i] == NULL)
        {
          ni_png_warning(ni_png_ptr, "Insufficient memory for pCAL parameter.");
          return;
        }
      ni_png_memcpy(info_ptr->pcal_params[i], params[i], (ni_png_size_t)length);
   }

   info_ptr->valid |= PNG_INFO_pCAL;
#ifdef PNG_FREE_ME_SUPPORTED
   info_ptr->free_me |= PNG_FREE_PCAL;
#endif
}
#endif

#if defined(PNG_READ_sCAL_SUPPORTED) || defined(PNG_WRITE_sCAL_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
void PNGAPI
ni_png_set_sCAL(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
             int unit, double width, double height)
{
   ni_png_debug1(1, "in %s storage function\n", "sCAL");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->scal_unit = (ni_png_byte)unit;
   info_ptr->scal_pixel_width = width;
   info_ptr->scal_pixel_height = height;

   info_ptr->valid |= PNG_INFO_sCAL;
}
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
void PNGAPI
ni_png_set_sCAL_s(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
             int unit, ni_png_charp swidth, ni_png_charp sheight)
{
   ni_png_uint_32 length;

   ni_png_debug1(1, "in %s storage function\n", "sCAL");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->scal_unit = (ni_png_byte)unit;

   length = ni_png_strlen(swidth) + 1;
   ni_png_debug1(3, "allocating unit for info (%d bytes)\n", length);
   info_ptr->scal_s_width = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length);
   if (info_ptr->scal_s_width == NULL)
   {
      ni_png_warning(ni_png_ptr, "Memory allocation failed while processing sCAL.");
   }
   ni_png_memcpy(info_ptr->scal_s_width, swidth, (ni_png_size_t)length);

   length = ni_png_strlen(sheight) + 1;
   ni_png_debug1(3, "allocating unit for info (%d bytes)\n", length);
   info_ptr->scal_s_height = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, length);
   if (info_ptr->scal_s_height == NULL)
   {
      ni_png_free (ni_png_ptr, info_ptr->scal_s_width);
      ni_png_warning(ni_png_ptr, "Memory allocation failed while processing sCAL.");
   }
   ni_png_memcpy(info_ptr->scal_s_height, sheight, (ni_png_size_t)length);

   info_ptr->valid |= PNG_INFO_sCAL;
#ifdef PNG_FREE_ME_SUPPORTED
   info_ptr->free_me |= PNG_FREE_SCAL;
#endif
}
#endif
#endif
#endif

#if defined(PNG_pHYs_SUPPORTED)
void PNGAPI
ni_png_set_pHYs(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_uint_32 res_x, ni_png_uint_32 res_y, int unit_type)
{
   ni_png_debug1(1, "in %s storage function\n", "pHYs");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->x_pixels_per_unit = res_x;
   info_ptr->y_pixels_per_unit = res_y;
   info_ptr->phys_unit_type = (ni_png_byte)unit_type;
   info_ptr->valid |= PNG_INFO_pHYs;
}
#endif

void PNGAPI
ni_png_set_PLTE(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_colorp palette, int num_palette)
{

   ni_png_debug1(1, "in %s storage function\n", "PLTE");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   if (num_palette < 0 || num_palette > PNG_MAX_PALETTE_LENGTH)
     {
       if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
         ni_png_error(ni_png_ptr, "Invalid palette length");
       else
       {
         ni_png_warning(ni_png_ptr, "Invalid palette length");
         return;
       }
     }

   /*
    * It may not actually be necessary to set ni_png_ptr->palette here;
    * we do it for backward compatibility with the way the ni_png_handle_tRNS
    * function used to do the allocation.
    */
#ifdef PNG_FREE_ME_SUPPORTED
   ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_PLTE, 0);
#endif

   /* Changed in libpng-1.2.1 to allocate PNG_MAX_PALETTE_LENGTH instead
      of num_palette entries,
      in case of an invalid PNG file that has too-large sample values. */
   ni_png_ptr->palette = (ni_png_colorp)ni_png_malloc(ni_png_ptr,
      PNG_MAX_PALETTE_LENGTH * ni_png_sizeof(ni_png_color));
   ni_png_memset(ni_png_ptr->palette, 0, PNG_MAX_PALETTE_LENGTH *
      ni_png_sizeof(ni_png_color));
   ni_png_memcpy(ni_png_ptr->palette, palette, num_palette * ni_png_sizeof (ni_png_color));
   info_ptr->palette = ni_png_ptr->palette;
   info_ptr->num_palette = ni_png_ptr->num_palette = (ni_png_uint_16)num_palette;

#ifdef PNG_FREE_ME_SUPPORTED
   info_ptr->free_me |= PNG_FREE_PLTE;
#else
   ni_png_ptr->flags |= PNG_FLAG_FREE_PLTE;
#endif

   info_ptr->valid |= PNG_INFO_PLTE;
}

#if defined(PNG_sBIT_SUPPORTED)
void PNGAPI
ni_png_set_sBIT(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_color_8p sig_bit)
{
   ni_png_debug1(1, "in %s storage function\n", "sBIT");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   ni_png_memcpy(&(info_ptr->sig_bit), sig_bit, ni_png_sizeof (ni_png_color_8));
   info_ptr->valid |= PNG_INFO_sBIT;
}
#endif

#if defined(PNG_sRGB_SUPPORTED)
void PNGAPI
ni_png_set_sRGB(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, int intent)
{
   ni_png_debug1(1, "in %s storage function\n", "sRGB");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->srgb_intent = (ni_png_byte)intent;
   info_ptr->valid |= PNG_INFO_sRGB;
}

void PNGAPI
ni_png_set_sRGB_gAMA_and_cHRM(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   int intent)
{
#if defined(PNG_gAMA_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float file_gamma;
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   ni_png_fixed_point int_file_gamma;
#endif
#endif
#if defined(PNG_cHRM_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   float white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y;
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   ni_png_fixed_point int_white_x, int_white_y, int_red_x, int_red_y, int_green_x,
      int_green_y, int_blue_x, int_blue_y;
#endif
#endif
   ni_png_debug1(1, "in %s storage function\n", "sRGB_gAMA_and_cHRM");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   ni_png_set_sRGB(ni_png_ptr, info_ptr, intent);

#if defined(PNG_gAMA_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   file_gamma = (float).45455;
   ni_png_set_gAMA(ni_png_ptr, info_ptr, file_gamma);
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
   int_file_gamma = 45455L;
   ni_png_set_gAMA_fixed(ni_png_ptr, info_ptr, int_file_gamma);
#endif
#endif

#if defined(PNG_cHRM_SUPPORTED)
#ifdef PNG_FIXED_POINT_SUPPORTED
   int_white_x = 31270L;
   int_white_y = 32900L;
   int_red_x   = 64000L;
   int_red_y   = 33000L;
   int_green_x = 30000L;
   int_green_y = 60000L;
   int_blue_x  = 15000L;
   int_blue_y  =  6000L;

   ni_png_set_cHRM_fixed(ni_png_ptr, info_ptr,
      int_white_x, int_white_y, int_red_x, int_red_y, int_green_x, int_green_y,
      int_blue_x, int_blue_y);
#endif
#ifdef PNG_FLOATING_POINT_SUPPORTED
   white_x = (float).3127;
   white_y = (float).3290;
   red_x   = (float).64;
   red_y   = (float).33;
   green_x = (float).30;
   green_y = (float).60;
   blue_x  = (float).15;
   blue_y  = (float).06;

   ni_png_set_cHRM(ni_png_ptr, info_ptr,
      white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y);
#endif
#endif
}
#endif


#if defined(PNG_iCCP_SUPPORTED)
void PNGAPI
ni_png_set_iCCP(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
             ni_png_charp name, int compression_type,
             ni_png_charp profile, ni_png_uint_32 proflen)
{
   ni_png_charp new_iccp_name;
   ni_png_charp new_iccp_profile;

   ni_png_debug1(1, "in %s storage function\n", "iCCP");
   if (ni_png_ptr == NULL || info_ptr == NULL || name == NULL || profile == NULL)
      return;

   new_iccp_name = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, ni_png_strlen(name)+1);
   if (new_iccp_name == NULL)
   {
      ni_png_warning(ni_png_ptr, "Insufficient memory to process iCCP chunk.");
      return;
   }
   ni_png_strcpy(new_iccp_name, name);
   new_iccp_profile = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr, proflen);
   if (new_iccp_profile == NULL)
   {
      ni_png_free (ni_png_ptr, new_iccp_name);
      ni_png_warning(ni_png_ptr, "Insufficient memory to process iCCP profile.");
      return;
   }
   ni_png_memcpy(new_iccp_profile, profile, (ni_png_size_t)proflen);

   ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_ICCP, 0);

   info_ptr->iccp_proflen = proflen;
   info_ptr->iccp_name = new_iccp_name;
   info_ptr->iccp_profile = new_iccp_profile;
   /* Compression is always zero but is here so the API and info structure
    * does not have to change if we introduce multiple compression types */
   info_ptr->iccp_compression = (ni_png_byte)compression_type;
#ifdef PNG_FREE_ME_SUPPORTED
   info_ptr->free_me |= PNG_FREE_ICCP;
#endif
   info_ptr->valid |= PNG_INFO_iCCP;
}
#endif

#if defined(PNG_TEXT_SUPPORTED)
void PNGAPI
ni_png_set_text(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_textp text_ptr,
   int num_text)
{
   int ret;
   ret=ni_png_set_text_2(ni_png_ptr, info_ptr, text_ptr, num_text);
   if (ret)
     ni_png_error(ni_png_ptr, "Insufficient memory to store text");
}

int /* PRIVATE */
ni_png_set_text_2(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_textp text_ptr,
   int num_text)
{
   int i;

   ni_png_debug1(1, "in %s storage function\n", (ni_png_ptr->chunk_name[0] == '\0' ?
      "text" : (ni_png_const_charp)ni_png_ptr->chunk_name));

   if (ni_png_ptr == NULL || info_ptr == NULL || num_text == 0)
      return(0);

   /* Make sure we have enough space in the "text" array in info_struct
    * to hold all of the incoming text_ptr objects.
    */
   if (info_ptr->num_text + num_text > info_ptr->max_text)
   {
      if (info_ptr->text != NULL)
      {
         ni_png_textp old_text;
         int old_max;

         old_max = info_ptr->max_text;
         info_ptr->max_text = info_ptr->num_text + num_text + 8;
         old_text = info_ptr->text;
         info_ptr->text = (ni_png_textp)ni_png_malloc_warn(ni_png_ptr,
            (ni_png_uint_32)(info_ptr->max_text * ni_png_sizeof (ni_png_text)));
         if (info_ptr->text == NULL)
           {
             ni_png_free(ni_png_ptr, old_text);
             return(1);
           }
         ni_png_memcpy(info_ptr->text, old_text, (ni_png_size_t)(old_max *
            ni_png_sizeof(ni_png_text)));
         ni_png_free(ni_png_ptr, old_text);
      }
      else
      {
         info_ptr->max_text = num_text + 8;
         info_ptr->num_text = 0;
         info_ptr->text = (ni_png_textp)ni_png_malloc_warn(ni_png_ptr,
            (ni_png_uint_32)(info_ptr->max_text * ni_png_sizeof (ni_png_text)));
         if (info_ptr->text == NULL)
           return(1);
#ifdef PNG_FREE_ME_SUPPORTED
         info_ptr->free_me |= PNG_FREE_TEXT;
#endif
      }
      ni_png_debug1(3, "allocated %d entries for info_ptr->text\n",
         info_ptr->max_text);
   }
   for (i = 0; i < num_text; i++)
   {
      ni_png_size_t text_length,key_len;
      ni_png_size_t lang_len,lang_key_len;
      ni_png_textp textp = &(info_ptr->text[info_ptr->num_text]);

      if (text_ptr[i].key == NULL)
          continue;

      key_len = ni_png_strlen(text_ptr[i].key);

      if(text_ptr[i].compression <= 0)
      {
        lang_len = 0;
        lang_key_len = 0;
      }
      else
#ifdef PNG_iTXt_SUPPORTED
      {
        /* set iTXt data */
        if (text_ptr[i].lang != NULL)
          lang_len = ni_png_strlen(text_ptr[i].lang);
        else
          lang_len = 0;
        if (text_ptr[i].lang_key != NULL)
          lang_key_len = ni_png_strlen(text_ptr[i].lang_key);
        else
          lang_key_len = 0;
      }
#else
      {
        ni_png_warning(ni_png_ptr, "iTXt chunk not supported.");
        continue;
      }
#endif

      if (text_ptr[i].text == NULL || text_ptr[i].text[0] == '\0')
      {
         text_length = 0;
#ifdef PNG_iTXt_SUPPORTED
         if(text_ptr[i].compression > 0)
            textp->compression = PNG_ITXT_COMPRESSION_NONE;
         else
#endif
            textp->compression = PNG_TEXT_COMPRESSION_NONE;
      }
      else
      {
         text_length = ni_png_strlen(text_ptr[i].text);
         textp->compression = text_ptr[i].compression;
      }

      textp->key = (ni_png_charp)ni_png_malloc_warn(ni_png_ptr,
         (ni_png_uint_32)(key_len + text_length + lang_len + lang_key_len + 4));
      if (textp->key == NULL)
        return(1);
      ni_png_debug2(2, "Allocated %lu bytes at %x in ni_png_set_text\n",
         (ni_png_uint_32)(key_len + lang_len + lang_key_len + text_length + 4),
         (int)textp->key);

      ni_png_memcpy(textp->key, text_ptr[i].key,
         (ni_png_size_t)(key_len));
      *(textp->key+key_len) = '\0';
#ifdef PNG_iTXt_SUPPORTED
      if (text_ptr[i].compression > 0)
      {
         textp->lang=textp->key + key_len + 1;
         ni_png_memcpy(textp->lang, text_ptr[i].lang, lang_len);
         *(textp->lang+lang_len) = '\0';
         textp->lang_key=textp->lang + lang_len + 1;
         ni_png_memcpy(textp->lang_key, text_ptr[i].lang_key, lang_key_len);
         *(textp->lang_key+lang_key_len) = '\0';
         textp->text=textp->lang_key + lang_key_len + 1;
      }
      else
#endif
      {
#ifdef PNG_iTXt_SUPPORTED
         textp->lang=NULL;
         textp->lang_key=NULL;
#endif
         textp->text=textp->key + key_len + 1;
      }
      if(text_length)
         ni_png_memcpy(textp->text, text_ptr[i].text,
            (ni_png_size_t)(text_length));
      *(textp->text+text_length) = '\0';

#ifdef PNG_iTXt_SUPPORTED
      if(textp->compression > 0)
      {
         textp->text_length = 0;
         textp->itxt_length = text_length;
      }
      else
#endif
      {
         textp->text_length = text_length;
#ifdef PNG_iTXt_SUPPORTED
         textp->itxt_length = 0;
#endif
      }
#if 0 /* appears to be redundant; */
      info_ptr->text[info_ptr->num_text]= *textp;
#endif
      info_ptr->num_text++;
      ni_png_debug1(3, "transferred text chunk %d\n", info_ptr->num_text);
   }
   return(0);
}
#endif

#if defined(PNG_tIME_SUPPORTED)
void PNGAPI
ni_png_set_tIME(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_timep mod_time)
{
   ni_png_debug1(1, "in %s storage function\n", "tIME");
   if (ni_png_ptr == NULL || info_ptr == NULL ||
       (ni_png_ptr->mode & PNG_WROTE_tIME))
      return;

   ni_png_memcpy(&(info_ptr->mod_time), mod_time, ni_png_sizeof (ni_png_time));
   info_ptr->valid |= PNG_INFO_tIME;
}
#endif

#if defined(PNG_tRNS_SUPPORTED)
void PNGAPI
ni_png_set_tRNS(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   ni_png_bytep trans, int num_trans, ni_png_color_16p trans_values)
{
   ni_png_debug1(1, "in %s storage function\n", "tRNS");
   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   if (trans != NULL)
   {
       /*
        * It may not actually be necessary to set ni_png_ptr->trans here;
        * we do it for backward compatibility with the way the ni_png_handle_tRNS
        * function used to do the allocation.
        */
#ifdef PNG_FREE_ME_SUPPORTED
       ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_TRNS, 0);
#endif
       /* Changed from num_trans to PNG_MAX_PALETTE_LENGTH in version 1.2.1 */
       ni_png_ptr->trans = info_ptr->trans = (ni_png_bytep)ni_png_malloc(ni_png_ptr,
           (ni_png_uint_32)PNG_MAX_PALETTE_LENGTH);
       if (num_trans <= PNG_MAX_PALETTE_LENGTH)
         ni_png_memcpy(info_ptr->trans, trans, (ni_png_size_t)num_trans);
#ifdef PNG_FREE_ME_SUPPORTED
       info_ptr->free_me |= PNG_FREE_TRNS;
#else
       ni_png_ptr->flags |= PNG_FLAG_FREE_TRNS;
#endif
   }

   if (trans_values != NULL)
   {
      ni_png_memcpy(&(info_ptr->trans_values), trans_values,
         ni_png_sizeof(ni_png_color_16));
      if (num_trans == 0)
        num_trans = 1;
   }
   info_ptr->num_trans = (ni_png_uint_16)num_trans;
   info_ptr->valid |= PNG_INFO_tRNS;
}
#endif

#if defined(PNG_sPLT_SUPPORTED)
void PNGAPI
ni_png_set_sPLT(ni_png_structp ni_png_ptr,
             ni_png_infop info_ptr, ni_png_sPLT_tp entries, int nentries)
{
    ni_png_sPLT_tp np;
    int i;

    if (ni_png_ptr == NULL || info_ptr == NULL)
       return;

    np = (ni_png_sPLT_tp)ni_png_malloc_warn(ni_png_ptr,
        (info_ptr->splt_palettes_num + nentries) * ni_png_sizeof(ni_png_sPLT_t));
    if (np == NULL)
    {
      ni_png_warning(ni_png_ptr, "No memory for sPLT palettes.");
      return;
    }

    ni_png_memcpy(np, info_ptr->splt_palettes,
           info_ptr->splt_palettes_num * ni_png_sizeof(ni_png_sPLT_t));
    ni_png_free(ni_png_ptr, info_ptr->splt_palettes);
    info_ptr->splt_palettes=NULL;

    for (i = 0; i < nentries; i++)
    {
        ni_png_sPLT_tp to = np + info_ptr->splt_palettes_num + i;
        ni_png_sPLT_tp from = entries + i;

        to->name = (ni_png_charp)ni_png_malloc(ni_png_ptr,
            ni_png_strlen(from->name) + 1);
        /* TODO: use ni_png_malloc_warn */
        ni_png_strcpy(to->name, from->name);
        to->entries = (ni_png_sPLT_entryp)ni_png_malloc(ni_png_ptr,
            from->nentries * ni_png_sizeof(ni_png_sPLT_entry));
        /* TODO: use ni_png_malloc_warn */
        ni_png_memcpy(to->entries, from->entries,
            from->nentries * ni_png_sizeof(ni_png_sPLT_entry));
        to->nentries = from->nentries;
        to->depth = from->depth;
    }

    info_ptr->splt_palettes = np;
    info_ptr->splt_palettes_num += nentries;
    info_ptr->valid |= PNG_INFO_sPLT;
#ifdef PNG_FREE_ME_SUPPORTED
    info_ptr->free_me |= PNG_FREE_SPLT;
#endif
}
#endif /* PNG_sPLT_SUPPORTED */

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
void PNGAPI
ni_png_set_unknown_chunks(ni_png_structp ni_png_ptr,
   ni_png_infop info_ptr, ni_png_unknown_chunkp unknowns, int num_unknowns)
{
    ni_png_unknown_chunkp np;
    int i;

    if (ni_png_ptr == NULL || info_ptr == NULL || num_unknowns == 0)
        return;

    np = (ni_png_unknown_chunkp)ni_png_malloc_warn(ni_png_ptr,
        (info_ptr->unknown_chunks_num + num_unknowns) *
        ni_png_sizeof(ni_png_unknown_chunk));
    if (np == NULL)
    {
       ni_png_warning(ni_png_ptr, "Out of memory while processing unknown chunk.");
       return;
    }

    ni_png_memcpy(np, info_ptr->unknown_chunks,
           info_ptr->unknown_chunks_num * ni_png_sizeof(ni_png_unknown_chunk));
    ni_png_free(ni_png_ptr, info_ptr->unknown_chunks);
    info_ptr->unknown_chunks=NULL;

    for (i = 0; i < num_unknowns; i++)
    {
        ni_png_unknown_chunkp to = np + info_ptr->unknown_chunks_num + i;
        ni_png_unknown_chunkp from = unknowns + i;

        ni_png_strncpy((ni_png_charp)to->name, (ni_png_charp)from->name, 5);
        to->data = (ni_png_bytep)ni_png_malloc_warn(ni_png_ptr, from->size);
        if (to->data == NULL)
        {
           ni_png_warning(ni_png_ptr, "Out of memory processing unknown chunk.");
        }
        else
        {
           ni_png_memcpy(to->data, from->data, from->size);
           to->size = from->size;

           /* note our location in the read or write sequence */
           to->location = (ni_png_byte)(ni_png_ptr->mode & 0xff);
        }
    }

    info_ptr->unknown_chunks = np;
    info_ptr->unknown_chunks_num += num_unknowns;
#ifdef PNG_FREE_ME_SUPPORTED
    info_ptr->free_me |= PNG_FREE_UNKN;
#endif
}
void PNGAPI
ni_png_set_unknown_chunk_location(ni_png_structp ni_png_ptr, ni_png_infop info_ptr,
   int chunk, int location)
{
   if(ni_png_ptr != NULL && info_ptr != NULL && chunk >= 0 && chunk <
         (int)info_ptr->unknown_chunks_num)
      info_ptr->unknown_chunks[chunk].location = (ni_png_byte)location;
}
#endif

#if defined(PNG_1_0_X) || defined(PNG_1_2_X)
#if defined(PNG_READ_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED)
void PNGAPI
ni_png_permit_empty_plte (ni_png_structp ni_png_ptr, int empty_plte_permitted)
{
   /* This function is deprecated in favor of ni_png_permit_mng_features()
      and will be removed from libpng-1.3.0 */
   ni_png_debug(1, "in ni_png_permit_empty_plte, DEPRECATED.\n");
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->mng_features_permitted = (ni_png_byte)
     ((ni_png_ptr->mng_features_permitted & (~(PNG_FLAG_MNG_EMPTY_PLTE))) |
     ((empty_plte_permitted & PNG_FLAG_MNG_EMPTY_PLTE)));
}
#endif
#endif

#if defined(PNG_MNG_FEATURES_SUPPORTED)
ni_png_uint_32 PNGAPI
ni_png_permit_mng_features (ni_png_structp ni_png_ptr, ni_png_uint_32 mng_features)
{
   ni_png_debug(1, "in ni_png_permit_mng_features\n");
   if (ni_png_ptr == NULL)
      return (ni_png_uint_32)0;
   ni_png_ptr->mng_features_permitted =
     (ni_png_byte)(mng_features & PNG_ALL_MNG_FEATURES);
   return (ni_png_uint_32)ni_png_ptr->mng_features_permitted;
}
#endif

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
void PNGAPI
ni_png_set_keep_unknown_chunks(ni_png_structp ni_png_ptr, int keep, ni_png_bytep
   chunk_list, int num_chunks)
{
    ni_png_bytep new_list, p;
    int i, old_num_chunks;
    if (ni_png_ptr == NULL)
       return;
    if (num_chunks == 0)
    {
      if(keep == PNG_HANDLE_CHUNK_ALWAYS || keep == PNG_HANDLE_CHUNK_IF_SAFE)
        ni_png_ptr->flags |= PNG_FLAG_KEEP_UNKNOWN_CHUNKS;
      else
        ni_png_ptr->flags &= ~PNG_FLAG_KEEP_UNKNOWN_CHUNKS;

      if(keep == PNG_HANDLE_CHUNK_ALWAYS)
        ni_png_ptr->flags |= PNG_FLAG_KEEP_UNSAFE_CHUNKS;
      else
        ni_png_ptr->flags &= ~PNG_FLAG_KEEP_UNSAFE_CHUNKS;
      return;
    }
    if (chunk_list == NULL)
      return;
    old_num_chunks=ni_png_ptr->num_chunk_list;
    new_list=(ni_png_bytep)ni_png_malloc(ni_png_ptr,
       (ni_png_uint_32)(5*(num_chunks+old_num_chunks)));
    if(ni_png_ptr->chunk_list != NULL)
    {
       ni_png_memcpy(new_list, ni_png_ptr->chunk_list,
          (ni_png_size_t)(5*old_num_chunks));
       ni_png_free(ni_png_ptr, ni_png_ptr->chunk_list);
       ni_png_ptr->chunk_list=NULL;
    }
    ni_png_memcpy(new_list+5*old_num_chunks, chunk_list,
       (ni_png_size_t)(5*num_chunks));
    for (p=new_list+5*old_num_chunks+4, i=0; i<num_chunks; i++, p+=5)
       *p=(ni_png_byte)keep;
    ni_png_ptr->num_chunk_list=old_num_chunks+num_chunks;
    ni_png_ptr->chunk_list=new_list;
#ifdef PNG_FREE_ME_SUPPORTED
    ni_png_ptr->free_me |= PNG_FREE_LIST;
#endif
}
#endif

#if defined(PNG_READ_USER_CHUNKS_SUPPORTED)
void PNGAPI
ni_png_set_read_user_chunk_fn(ni_png_structp ni_png_ptr, ni_png_voidp user_chunk_ptr,
   ni_png_user_chunk_ptr read_user_chunk_fn)
{
   ni_png_debug(1, "in ni_png_set_read_user_chunk_fn\n");
   if (ni_png_ptr == NULL)
      return;
   ni_png_ptr->read_user_chunk_fn = read_user_chunk_fn;
   ni_png_ptr->user_chunk_ptr = user_chunk_ptr;
}
#endif

#if defined(PNG_INFO_IMAGE_SUPPORTED)
void PNGAPI
ni_png_set_rows(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, ni_png_bytepp row_pointers)
{
   ni_png_debug1(1, "in %s storage function\n", "rows");

   if (ni_png_ptr == NULL || info_ptr == NULL)
      return;

   if(info_ptr->row_pointers && (info_ptr->row_pointers != row_pointers))
      ni_png_free_data(ni_png_ptr, info_ptr, PNG_FREE_ROWS, 0);
   info_ptr->row_pointers = row_pointers;
   if(row_pointers)
      info_ptr->valid |= PNG_INFO_IDAT;
}
#endif

#ifdef PNG_WRITE_SUPPORTED
void PNGAPI
ni_png_set_compression_buffer_size(ni_png_structp ni_png_ptr, ni_png_uint_32 size)
{
    if (ni_png_ptr == NULL)
       return;
    if(ni_png_ptr->zbuf)
       ni_png_free(ni_png_ptr, ni_png_ptr->zbuf);
    ni_png_ptr->zbuf_size = (ni_png_size_t)size;
    ni_png_ptr->zbuf = (ni_png_bytep)ni_png_malloc(ni_png_ptr, size);
    ni_png_ptr->zstream.next_out = ni_png_ptr->zbuf;
    ni_png_ptr->zstream.avail_out = (uInt)ni_png_ptr->zbuf_size;
}
#endif

void PNGAPI
ni_png_set_invalid(ni_png_structp ni_png_ptr, ni_png_infop info_ptr, int mask)
{
   if (ni_png_ptr && info_ptr)
      info_ptr->valid &= ~(mask);
}


#ifndef PNG_1_0_X
#ifdef PNG_ASSEMBLER_CODE_SUPPORTED
/* this function was added to libpng 1.2.0 and should always exist by default */
void PNGAPI
ni_png_set_asm_flags (ni_png_structp ni_png_ptr, ni_png_uint_32 asm_flags)
{
#ifdef PNG_MMX_CODE_SUPPORTED
    ni_png_uint_32 settable_asm_flags;
    ni_png_uint_32 settable_mmx_flags;
#endif
    if (ni_png_ptr == NULL)
       return;
#ifdef PNG_MMX_CODE_SUPPORTED

    settable_mmx_flags =
#ifdef PNG_HAVE_MMX_COMBINE_ROW
                         PNG_ASM_FLAG_MMX_READ_COMBINE_ROW  |
#endif
#ifdef PNG_HAVE_MMX_READ_INTERLACE
                         PNG_ASM_FLAG_MMX_READ_INTERLACE    |
#endif
#ifdef PNG_HAVE_MMX_READ_FILTER_ROW
                         PNG_ASM_FLAG_MMX_READ_FILTER_SUB   |
                         PNG_ASM_FLAG_MMX_READ_FILTER_UP    |
                         PNG_ASM_FLAG_MMX_READ_FILTER_AVG   |
                         PNG_ASM_FLAG_MMX_READ_FILTER_PAETH |
#endif
                         0;

    /* could be some non-MMX ones in the future, but not currently: */
    settable_asm_flags = settable_mmx_flags;

    if (!(ni_png_ptr->asm_flags & PNG_ASM_FLAG_MMX_SUPPORT_COMPILED) ||
        !(ni_png_ptr->asm_flags & PNG_ASM_FLAG_MMX_SUPPORT_IN_CPU))
    {
        /* clear all MMX flags if MMX isn't supported */
        settable_asm_flags &= ~settable_mmx_flags;
        ni_png_ptr->asm_flags &= ~settable_mmx_flags;
    }

    /* we're replacing the settable bits with those passed in by the user,
     * so first zero them out of the master copy, then bitwise-OR in the
     * allowed subset that was requested */

    ni_png_ptr->asm_flags &= ~settable_asm_flags;               /* zero them */
    ni_png_ptr->asm_flags |= (asm_flags & settable_asm_flags);  /* set them */
#endif /* ?PNG_MMX_CODE_SUPPORTED */
}

/* this function was added to libpng 1.2.0 */
void PNGAPI
ni_png_set_mmx_thresholds (ni_png_structp ni_png_ptr,
                        ni_png_byte mmx_bitdepth_threshold,
                        ni_png_uint_32 mmx_rowbytes_threshold)
{
    if (ni_png_ptr == NULL)
       return;
#ifdef PNG_MMX_CODE_SUPPORTED
    ni_png_ptr->mmx_bitdepth_threshold = mmx_bitdepth_threshold;
    ni_png_ptr->mmx_rowbytes_threshold = mmx_rowbytes_threshold;
#endif /* ?PNG_MMX_CODE_SUPPORTED */
}
#endif /* ?PNG_ASSEMBLER_CODE_SUPPORTED */

#ifdef PNG_SET_USER_LIMITS_SUPPORTED
/* this function was added to libpng 1.2.6 */
void PNGAPI
ni_png_set_user_limits (ni_png_structp ni_png_ptr, ni_png_uint_32 user_width_max,
    ni_png_uint_32 user_height_max)
{
    /* Images with dimensions larger than these limits will be
     * rejected by ni_png_set_IHDR().  To accept any PNG datastream
     * regardless of dimensions, set both limits to 0x7ffffffL.
     */
    if(ni_png_ptr == NULL) return;
    ni_png_ptr->user_width_max = user_width_max;
    ni_png_ptr->user_height_max = user_height_max;
}
#endif /* ?PNG_SET_USER_LIMITS_SUPPORTED */

#endif /* ?PNG_1_0_X */
#endif /* PNG_READ_SUPPORTED || PNG_WRITE_SUPPORTED */
