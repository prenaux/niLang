// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "libpng/png.h"
#include "Bitmap2D.h"

// Disables "interaction between '_setjmp' and C++ object destruction is non-portable"
#ifdef _MSC_VER
#pragma warning (disable : 4611)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// PNG lib support functions

///////////////////////////////////////////////
static void ComputeLinesPtrs(astl::vector<tPtr>& avOut, iBitmap2D* pBmp)
{
  tU32 nHeight = pBmp->GetHeight();
  tU32 nPitch = pBmp->GetPitch();
  avOut.resize(nHeight);
  avOut[0] = pBmp->GetData();
  for(tU32 i = 1; i < nHeight; ++i)
  {
    avOut[i] = avOut[i-1] + nPitch;
  }
}

///////////////////////////////////////////////
static void my_png_read_data(ni_png_structp ctx, ni_png_bytep area, ni_png_size_t size)
{
  iFile *fp = reinterpret_cast<iFile*>(ni_png_get_io_ptr(ctx));
  fp->ReadRaw(area, size);
}

///////////////////////////////////////////////
static void my_png_write_data(ni_png_structp ctx, ni_png_bytep area, ni_png_size_t size)
{
  iFile* fp = reinterpret_cast<iFile*>(ni_png_get_io_ptr(ctx));
  fp->WriteRaw(area, size);
}

///////////////////////////////////////////////
static void my_png_error(ni_png_structp ctx, ni_png_const_charp msg)
{
  niError(niFmt(_A("PNG Error: %s"), cString(msg).Chars()));
  longjmp(ctx->jmpbuf, 1);
}

///////////////////////////////////////////////
static void my_png_warning(ni_png_structp ctx, ni_png_const_charp msg)
{
  niLog(Warning,niFmt(_A("PNG Warning: %s"), cString(msg).Chars()));
}

///////////////////////////////////////////////
static void my_png_flush_data(ni_png_structp ctx)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
struct BitmapLoader_PNG : public ImplRC<iBitmapLoader> {
  virtual iBitmapBase* __stdcall LoadBitmap(iGraphics* apGraphics, iFile* pFile) niImpl {
    ni_png_structp ni_png_ptr = NULL;
    ni_png_infop   info_ptr = NULL;
    Ptr<iBitmap2D> ptrBmp = NULL;
    ni_png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    int i;

    ni_png_ptr = ni_png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!ni_png_ptr) {
      niError(_A("Can't create png reading structure."));
      return NULL;
    }

    info_ptr = ni_png_create_info_struct(ni_png_ptr);
    if (!info_ptr) {
      niError(_A("Can't create the info structure."));
      ni_png_destroy_read_struct(&ni_png_ptr, info_ptr ? &info_ptr :(ni_png_infopp)0, (ni_png_infopp)0);
      return NULL;
    }

    if (setjmp(ni_png_ptr->jmpbuf)) {
      niError(_A("PNG Error."));
      ni_png_destroy_read_struct(&ni_png_ptr, info_ptr ? &info_ptr :(ni_png_infopp)0, (ni_png_infopp)0);
      return NULL;
    }

    ni_png_set_error_fn(ni_png_ptr, NULL, my_png_error, my_png_warning);
    ni_png_set_read_fn(ni_png_ptr, pFile, my_png_read_data);

    ni_png_read_info(ni_png_ptr, info_ptr);
    ni_png_get_IHDR(ni_png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

    ni_png_set_strip_16(ni_png_ptr);

    ni_png_set_packing(ni_png_ptr);

    if (bit_depth < 8)
      ni_png_set_expand(ni_png_ptr);

    ni_png_set_bgr(ni_png_ptr);

    if (ni_png_get_valid(ni_png_ptr, info_ptr, PNG_INFO_tRNS))
      ni_png_set_expand(ni_png_ptr);

    // Commented, we want to get grayscale... well gray
    //if ((color_type == PNG_COLOR_TYPE_GRAY) ||
    //  (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
    //  ni_png_set_gray_to_rgb(ni_png_ptr);

    ni_png_read_update_info(ni_png_ptr, info_ptr);

    ni_png_get_IHDR(ni_png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

    cString str;
    switch (bit_depth*info_ptr->channels) {
      case 8: {
        str = _A("A8");
        break;
      }
      case 16: {
        if (info_ptr->channels == 1) {
          str = _A("R16");
        }
        else {
          str = _A("B5G6R5");
        }
        break;
      }
      case 24:  str = _A("B8G8R8"); break;
      case 32:  str = _A("B8G8R8A8"); break;
      case 64:  str = _A("B16G16R16A16"); break;
      case 128: str = _A("B32G32R32A32"); break;
      default: {
        ni_png_destroy_read_struct(&ni_png_ptr, info_ptr ? &info_ptr :(ni_png_infopp)0, (ni_png_infopp)0);
        niError(niFmt(_A("Invalid bpp: %d\n"),bit_depth*info_ptr->channels));
        return NULL;
      }
    }

    ptrBmp = apGraphics->CreateBitmap2DEx(width, height, apGraphics->CreatePixelFormat(str.Chars()));
    if (!ptrBmp.IsOK()) {
      niError(_A("Can't create bitmap."));
      ni_png_destroy_read_struct(&ni_png_ptr, info_ptr ? &info_ptr :(ni_png_infopp)0, (ni_png_infopp)0);
      return NULL;
    }

    astl::vector<tPtr> vLines;
    ComputeLinesPtrs(vLines, ptrBmp);
    ni_png_read_image(ni_png_ptr, &vLines[0]);

    ni_png_read_end(ni_png_ptr, info_ptr);

    astl::vector<tU32> palette;
    if ((color_type == PNG_COLOR_TYPE_GRAY) ||
        (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
    {
      // Grayscale, dont do anything if not specified its implicitly greyscale (alpha)
    }
    else {
      if (bit_depth*info_ptr->channels == 8) {
        if (info_ptr->num_palette > 256) {
          niWarning(niFmt("Invalid number of palette entries '%d'.",info_ptr->num_palette));
          return NULL;
        }

        palette.resize(256,0);
        for (i = 0; i < ni::Min(palette.size(),info_ptr->num_palette); i++) {
          palette[i]= ULColorBuild(
              info_ptr->palette[i].red,
              info_ptr->palette[i].green,
              info_ptr->palette[i].blue,
              255);
        }
      }
    }

    ni_png_destroy_read_struct(&ni_png_ptr, info_ptr ? &info_ptr :(ni_png_infopp)0, (ni_png_infopp)0);

    if (!palette.empty()) {
      Ptr<iBitmap2D> convertedBmp = apGraphics->CreateBitmap2D(
          ptrBmp->GetWidth(), ptrBmp->GetHeight(), "B8G8R8X8");
      BmpUtils_BlitPaletteTo32Bits(
          convertedBmp->GetData(), ptrBmp->GetData(), ptrBmp->GetWidth() * ptrBmp->GetHeight(), palette.data());
      ptrBmp = convertedBmp;
    }
    return ptrBmp.GetRawAndSetNull();
  }
};

///////////////////////////////////////////////
struct BitmapSaver_PNG : public ImplRC<iBitmapSaver> {
  tBool __stdcall SaveBitmap(iGraphics* apGraphics, iFile* pFile, iBitmapBase* pBmpBase, tU32 ulCompression) niImpl
  {
    QPtr<iBitmap2D> pBmp = pBmpBase;
    if (!pBmp.IsOK()) {
      niError("Can't query iBitmap2D interface.");
      return eFalse;
    }

    if (!(pBmpBase->GetPixelFormat()->GetCaps()&ePixelFormatCaps_UnpackPixel)) {
      niError(niFmt("Can't unpack pixel format '%s'.", pBmpBase->GetPixelFormat()->GetFormat()));
      return eFalse;
    }

    const tU32 pngColorType = pBmp->GetPixelFormat()->GetNumABits() ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB;
    if (pngColorType == PNG_COLOR_TYPE_RGB_ALPHA) {
      if (StrICmp(pBmpBase->GetPixelFormat()->GetFormat(),"B8G8R8A8") != 0) {
        pBmp = pBmp->CreateConvertedFormat(apGraphics->CreatePixelFormat("B8G8R8A8"));
        if (!pBmp.IsOK()) {
          niError("Can't convert bitmap to B8G8R8A8.");
          return eFalse;
        }
      }
    }
    else {
      if (StrICmp(pBmpBase->GetPixelFormat()->GetFormat(),"B8G8R8") != 0) {
        pBmp = pBmp->CreateConvertedFormat(apGraphics->CreatePixelFormat("B8G8R8"));
        if (!pBmp.IsOK()) {
          niError("Can't convert bitmap to B8G8R8.");
          return eFalse;
        }
      }
    }

    ni_png_structp ni_png_ptr;
    ni_png_infop info_ptr;

    ni_png_ptr = ni_png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (ni_png_ptr == NULL) {
      niError(_A("Can't the create PNG write struct."));
      return eFalse;
    }

    info_ptr = ni_png_create_info_struct(ni_png_ptr);
    if (info_ptr == NULL) {
      niError(_A("Can't create info ptr."));
      ni_png_destroy_write_struct(&ni_png_ptr, (ni_png_infopp)NULL);
      return eFalse;
    }

    if (setjmp(ni_png_jmpbuf(ni_png_ptr))) {
      niError(_A("PNG Write error."));
      ni_png_destroy_write_struct(&ni_png_ptr, &info_ptr);
      return NULL;
    }

    ni_png_set_write_fn(ni_png_ptr, (void*)pFile, my_png_write_data, my_png_flush_data);
    ni_png_set_error_fn(ni_png_ptr, NULL, my_png_error, my_png_warning);

    ni_png_set_IHDR(ni_png_ptr, info_ptr, pBmp->GetWidth(), pBmp->GetHeight(), 8, pngColorType,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    ni_png_set_packing(ni_png_ptr);
    ni_png_set_bgr(ni_png_ptr);
    ni_png_set_packswap(ni_png_ptr);

    ni_png_write_info(ni_png_ptr, info_ptr);

    astl::vector<tPtr> vLines;
    ComputeLinesPtrs(vLines, pBmp);
    ni_png_write_image(ni_png_ptr, &vLines[0]);

    ni_png_write_end(ni_png_ptr, info_ptr);

    ni_png_destroy_write_struct(&ni_png_ptr, &info_ptr);
    return eTrue;
  }
};


niExportFunc(iUnknown*) New_BitmapLoader_png(const Var&,const Var&) {
  return niNew BitmapLoader_PNG();
}

niExportFunc(iUnknown*) New_BitmapSaver_png(const Var&,const Var&) {
  return niNew BitmapSaver_PNG();
}
