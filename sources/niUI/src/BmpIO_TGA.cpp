// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "Bitmap2D.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Internal functions

#define RGB555_RGB565(dst, src) (dst) = tU16((((src)&0x7FE0)<<1)|((src)&0x001F))

///////////////////////////////////////////////
static void rle_tga_read(iFile *pFile, tU8 *b, tI32 w)
{
  tU8 value;
  tI32 count;
  tI32 c = 0;

  do
  {
    count = pFile->Read8();
    if (count & 0x80)
    {
      count =(count & 0x7F) + 1;
      c += count;
      value = pFile->Read8();
      while (count--)
        *(b++) = value;
    }
    else
    {
      count++;
      c += count;
      pFile->ReadRaw(b, count);
      b += count;
    }
  } while (c < w);
}

///////////////////////////////////////////////
static void rle_tga_read24(iFile *pFile, tU8 *b, tI32 w)
{
  tU8 value[4];
  tI32 count;
  tI32 c = 0;

  do
  {
    count = pFile->Read8();
    if (count & 0x80)
    {
      count =(count & 0x7F) + 1;
      c += count;
      pFile->ReadRaw(value, 3);
      while (count--)
      {
        b[2] = value[2];
        b[1] = value[1];
        b[0] = value[0];
        b += 3;
      }
    }
    else
    {
      count++;
      c += count;
      while (count--)
      {
        pFile->ReadRaw(value, 3);
        b[2] = value[2];
        b[1] = value[1];
        b[0] = value[0];
        b += 3;
      }
    }
  } while (c < w);
}

///////////////////////////////////////////////
static void rle_tga_read32(iFile *pFile, tU8 *b, tI32 w)
{
  tU8 value[4];
  tI32 count;
  tI32 c = 0;

  do
  {
    count = pFile->Read8();
    if (count & 0x80)
    {
      count =(count & 0x7F) + 1;
      c += count;
      pFile->ReadRaw(value, 4);
      while (count--)
      {
        b[3] = value[3];
        b[2] = value[2];
        b[1] = value[1];
        b[0] = value[0];
        b += 4;
      }
    }
    else
    {
      count++;
      c += count;
      while (count--)
      {
        pFile->ReadRaw(value, 4);
        b[3] = value[3];
        b[2] = value[2];
        b[1] = value[1];
        b[0] = value[0];
        b += 4;
      }
    }
  } while (c < w);
}

///////////////////////////////////////////////
static void rle_tga_read16(iFile *pFile, tU16 *b, tI32 w)
{
  tU16 value;
  tU16 color;
  tI32 count;
  tI32 c = 0;

  do
  {
    count = pFile->Read8();

    if (count & 0x80)
    {
      count =(count & 0x7F) + 1;
      c += count;
      value = pFile->ReadLE16();
      RGB555_RGB565(color,value);
      while (count--)
        *(b++) = color;
    }
    else
    {
      count++;
      c += count;
      while (count--)
      {
        value = pFile->ReadLE16();
        RGB555_RGB565(color,value);
        *(b++) = color;
      }
    }
  } while (c < w);
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Interface implementation
struct BitmapLoader_TGA : public ImplRC<iBitmapLoader> {

  ///////////////////////////////////////////////
  iBitmapBase* __stdcall LoadBitmap(iGraphics* apGraphics, iFile* pFile)
  {
    tU8 image_id[256], image_palette[256][3], rgb[4];
    tU8 id_length, palette_type, image_type;
    tU8 bpp, descriptor_bits;
    tU16 image_width, image_height;
    tU32 c, yc;
    tI32 x, y;
    tU16 *s;
    tI32 compressed;
    tU16 palette_colors;
    Ptr<iBitmap2D> ptrBmp;
    astl::vector<tU32> palette;

    id_length = pFile->Read8();
    palette_type = pFile->Read8();
    image_type = pFile->Read8();
    pFile->ReadLE16();
    palette_colors = pFile->ReadLE16();
    pFile->Read8();
    pFile->ReadLE16();
    pFile->ReadLE16();
    image_width = pFile->ReadLE16();
    image_height = pFile->ReadLE16();
    bpp = pFile->Read8();
    descriptor_bits = pFile->Read8();

    pFile->ReadRaw(&image_id, id_length);
    pFile->ReadRaw(&image_palette, ni::Min(palette_colors,(tU16)256)*3);
    if (palette_colors > 256) {
      palette_colors -= 256;
      pFile->Seek(palette_colors*3);
      palette_colors = 256;
    }

    compressed = (image_type & 8);
    image_type &= 7;

    if ((image_type < 1) || (image_type > 3))
    {
      niError(_A("Unknow TGA type."));
      return NULL;
    }

    switch (image_type)
    {
      case 1: {
        if ((palette_type != 1) || (bpp != 8)) {
          niError(_A("No palette or not 8 bpp in TGA type1."));
          return NULL;
        }

        palette.resize(palette_colors);
        niLoop(i,palette_colors) {
          palette[i] = ULColorBuild(image_palette[i][2],image_palette[i][1],image_palette[i][0],255);
        }
        break;
      }

      case 2:
        if ((palette_type != 0) || (bpp != 16 && bpp != 24 && bpp != 32)) {
          niError(niFmt(_A("Can't load TGA type2 with this bpp [%d]."), bpp));
          return NULL;
        }
        break;

      case 3: {
        if ((palette_type != 0) || (bpp != 8)) {
          niError(_A("Palette or not 8bpp in TGA type3."));
          return NULL;
        }

        palette_colors = 256;
        palette.resize(palette_colors);
        niLoop(i,256) {
          palette[i] = ULColorBuild(tU8(i),tU8(i),tU8(i),255);
        }
        break;
      }

      default: {
        niError(niFmt(_A("Unknow TGA type [%d]."), image_type));
        return NULL;
      }
    }

    cString strFormat;
    switch (bpp)
    {
      case 8: strFormat = _A("A8"); break;
      case 16: strFormat = _A("B5G6R5"); break;
      case 24: strFormat = _A("B8G8R8"); break;
      case 32: strFormat = _A("B8G8R8A8"); break;
      default: {
        niError(niFmt(_A("Invalid bpp [%d]."), bpp));
        return NULL;
      }
    }

    ptrBmp = apGraphics->CreateBitmap2DEx(image_width, image_height, apGraphics->CreatePixelFormat(strFormat.Chars()));
    niCheck(ptrBmp.IsOK(), NULL);

    for (y = image_height; y; y--)
    {
      yc = (descriptor_bits & 0x20) ? image_height - y : y - 1;
      tPtr pLine = ptrBmp->GetData()+(yc*ptrBmp->GetPitch());

      switch (image_type)
      {
        case 1:
        case 3:
          if (compressed)
            rle_tga_read(pFile, pLine, image_width);
          else
            pFile->ReadRaw(pLine, image_width);
          break;

        case 2:
          if (bpp == 32)
          {
            if (compressed)
            {
              rle_tga_read32(pFile, pLine, image_width);
            }
            else
            {
              for (x = 0; x < image_width; x++)
              {
                pFile->ReadRaw(rgb, 4);
                ((tU8*)pLine)[x*4 + 3] = rgb[3];
                ((tU8*)pLine)[x*4 + 2] = rgb[2];
                ((tU8*)pLine)[x*4 + 1] = rgb[1];
                ((tU8*)pLine)[x*4 + 0] = rgb[0];
              }
            }
          }
          else if (bpp == 24)
          {
            if (compressed)
            {
              rle_tga_read24(pFile, pLine, image_width);
            }
            else
            {
              for (x = 0; x < image_width; x++)
              {
                pFile->ReadRaw(rgb, 3);
                ((tU8*)pLine)[x*3 + 2] = rgb[2];
                ((tU8*)pLine)[x*3 + 1] = rgb[1];
                ((tU8*)pLine)[x*3 + 0] = rgb[0];
              }
            }
          }
          else
          {
            if (compressed)
            {
              rle_tga_read16(pFile, ((tU16*)pLine), image_width);
            }
            else
            {
              s = ((tU16*)pLine);
              for (x = 0; x < image_width; ++x)
              {
                c = pFile->ReadLE16();
                RGB555_RGB565(s[x],c);
              }
            }
          }
          break;
      }
    }

    if (!palette.empty()) {
      Ptr<iBitmap2D> convertedBmp = apGraphics->CreateBitmap2D(
          ptrBmp->GetWidth(), ptrBmp->GetHeight(), "B8G8R8A8");
      BmpUtils_BlitPaletteTo32Bits(
          convertedBmp->GetData(), ptrBmp->GetData(), ptrBmp->GetWidth() * ptrBmp->GetHeight(), palette.data());
      ptrBmp = convertedBmp;
    }
    return ptrBmp.GetRawAndSetNull();
  }
};

struct BitmapSaver_TGA : public ImplRC<iBitmapSaver> {
  ///////////////////////////////////////////////
  tBool __stdcall SaveBitmap(iGraphics* apGraphics, iFile* pFile, iBitmapBase* pBmpBase, tU32 ulCompression)
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

    tU32 x, y;
    tI32 depth;

    if (pBmp->GetPixelFormat()->GetBitsPerPixel() <= 8 &&
        pBmp->GetPixelFormat()->GetNumComponents() <= 1)
    {
      depth = 8;
    }
    else if (pBmp->GetPixelFormat()->GetNumABits()) {
      depth = 32;
    }
    else if (pBmp->GetPixelFormat()->GetBitsPerPixel() <= 16 &&
             pBmp->GetPixelFormat()->GetNumComponents() <= 3)
    {
      depth = 16;
    }
    else if (pBmp->GetPixelFormat()->GetNumComponents() <= 3) {
      depth = 24;
    }
    else {
      depth = 32;
    }

    pFile->Write8(0);
    pFile->Write8((depth == 8) ? 1 : 0);
    pFile->Write8((depth == 8) ? 1 : 2);
    pFile->WriteLE16(0);
    pFile->WriteLE16((depth == 8) ? 256 : 0);
    pFile->Write8((depth == 8) ? 24 : 0);
    pFile->WriteLE16(0);
    pFile->WriteLE16(0);
    pFile->WriteLE16(tU16(pBmp->GetWidth()));
    pFile->WriteLE16(tU16(pBmp->GetHeight()));
    pFile->Write8(tU8(depth));
    pFile->Write8(0);

    if (depth == 8) {
      for (int i = 0; i < 256; ++i) {
        pFile->Write8(i);
        pFile->Write8(i);
        pFile->Write8(i);
      }
    }

    switch (depth)
    {
      case 8:
        {
          for (y = pBmp->GetHeight(); y; y--)
          {
            for (x = 0; x < pBmp->GetWidth(); x++)
            {
              tU8 c;
              pFile->Write8(*((tU8*)pBmp->GetPixel(x,y-1,tPtr(&c))));
            }
          }
        }
        break;

      case 16:
        {
          if (pBmp->BeginUnpackPixels())
          {
            tU16 c;
            iPixelFormat* pPixFmt = pBmp->GetPixelFormat();
            for (y = pBmp->GetHeight(); y; y--)
            {
              for (x = 0; x < pBmp->GetWidth(); x++)
              {
                sColor4ub col = pPixFmt->UnpackPixelub(pBmp->GetPixel(x,y-1,tPtr(&c)));
                pFile->WriteLE16(((col.x << 7)&0x7C00) | ((col.y << 2)&0x3E0) | ((col.z >> 3)&0x1F));
              }
            }

            pBmp->EndUnpackPixels();
          }
        }
        break;

      case 24:
        {
          if (pBmp->BeginUnpackPixels())
          {
            niDeclareTempPixel();
            iPixelFormat* pPixFmt = pBmp->GetPixelFormat();
            for (y = pBmp->GetHeight(); y; y--)
            {
              for (x = 0; x < pBmp->GetWidth(); x++)
              {
                sColor4ub col = pPixFmt->UnpackPixelub(pBmp->GetPixel(x,y-1,niTempPixelPtr()));
                pFile->Write8(col.z);
                pFile->Write8(col.y);
                pFile->Write8(col.x);
              }
            }

            pBmp->EndUnpackPixels();
          }
        }
        break;


      case 32:
        {
          if (pBmp->BeginUnpackPixels())
          {
            niDeclareTempPixel();
            iPixelFormat* pPixFmt = pBmp->GetPixelFormat();
            for (y = pBmp->GetHeight(); y; y--)
            {
              for (x = 0; x < pBmp->GetWidth(); x++)
              {
                sColor4ub col = pPixFmt->UnpackPixelub(pBmp->GetPixel(x,y-1,niTempPixelPtr()));
                pFile->Write8(col.z);
                pFile->Write8(col.y);
                pFile->Write8(col.x);
                pFile->Write8(col.w);
              }
            }

            pBmp->EndUnpackPixels();
          }
        }
        break;
    }

    return 1;
  }
};


niExportFunc(iUnknown*) New_BitmapLoader_tga(const Var&,const Var&) {
  return niNew BitmapLoader_TGA();
}

niExportFunc(iUnknown*) New_BitmapSaver_tga(const Var&,const Var&) {
  return niNew BitmapSaver_TGA();
}
