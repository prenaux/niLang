// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Bitmap2D.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Internal functions

#define NI_BI_RGB          0
#define NI_BI_RLE8         1
#define NI_BI_RLE4         2
#define NI_BI_BITFIELDS    3

#define OS2INFOHEADERSIZE  12
#define WININFOHEADERSIZE  40

typedef struct NI_BITMAPFILEHEADER
{
  unsigned long  bfType;
  unsigned long  bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned long  bfOffBits;
} NI_BITMAPFILEHEADER;

typedef struct NI_BITMAPINFOHEADER
{
  unsigned long  biWidth;
  unsigned long  biHeight;
  unsigned short biBitCount;
  unsigned long  biCompression;
} NI_BITMAPINFOHEADER;

typedef struct WINBMPINFOHEADER
{
  unsigned long  biWidth;
  unsigned long  biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned long  biCompression;
  unsigned long  biSizeImage;
  unsigned long  biXPelsPerMeter;
  unsigned long  biYPelsPerMeter;
  unsigned long  biClrUsed;
  unsigned long  biClrImportant;
} WINBMPINFOHEADER;

typedef struct OS2BMPINFOHEADER
{
  unsigned short biWidth;
  unsigned short biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
} OS2BMPINFOHEADER;


///////////////////////////////////////////////
static int read_bmfileheader(iFile *pFile, NI_BITMAPFILEHEADER *fileheader)
{
  fileheader->bfType = pFile->ReadLE16();
  fileheader->bfSize = pFile->ReadLE32();
  fileheader->bfReserved1 = pFile->ReadLE16();
  fileheader->bfReserved2 = pFile->ReadLE16();
  fileheader->bfOffBits = pFile->ReadLE32();

  return 1;
}

///////////////////////////////////////////////
static int read_win_bminfoheader(iFile *pFile, NI_BITMAPINFOHEADER *infoheader)
{
  WINBMPINFOHEADER win_infoheader;

  win_infoheader.biWidth = pFile->ReadLE32();
  win_infoheader.biHeight = pFile->ReadLE32();
  win_infoheader.biPlanes = pFile->ReadLE16();
  win_infoheader.biBitCount = pFile->ReadLE16();
  win_infoheader.biCompression = pFile->ReadLE32();
  win_infoheader.biSizeImage = pFile->ReadLE32();
  win_infoheader.biXPelsPerMeter = pFile->ReadLE32();
  win_infoheader.biYPelsPerMeter = pFile->ReadLE32();
  win_infoheader.biClrUsed = pFile->ReadLE32();
  win_infoheader.biClrImportant = pFile->ReadLE32();
  infoheader->biWidth = win_infoheader.biWidth;
  infoheader->biHeight = win_infoheader.biHeight;
  infoheader->biBitCount = win_infoheader.biBitCount;
  infoheader->biCompression = win_infoheader.biCompression;

  return 1;
}

///////////////////////////////////////////////
static int read_os2_bminfoheader(iFile *pFile, NI_BITMAPINFOHEADER *infoheader)
{
  OS2BMPINFOHEADER os2_infoheader;

  os2_infoheader.biWidth = pFile->ReadLE16();
  os2_infoheader.biHeight = pFile->ReadLE16();
  os2_infoheader.biPlanes = pFile->ReadLE16();
  os2_infoheader.biBitCount = pFile->ReadLE16();

  infoheader->biWidth = os2_infoheader.biWidth;
  infoheader->biHeight = os2_infoheader.biHeight;
  infoheader->biBitCount = os2_infoheader.biBitCount;
  infoheader->biCompression = 0;

  return 1;
}

///////////////////////////////////////////////
static void read_bmicolors(int ncols, tU32* palette, iFile *pFile, int win_flag)
{
  int i;

  for (i = 0; i < ncols; i++)
  {
    if (palette) {
      tU8 b = pFile->Read8();
      tU8 g = pFile->Read8();
      tU8 r = pFile->Read8();
      palette[i] = ULColorBuild(r,g,b,255);
    }
    else {
      pFile->Read8();
      pFile->Read8();
      pFile->Read8();
    }

    if (win_flag)
      pFile->Read8();
  }
}

///////////////////////////////////////////////
static void read_8bit_line(int length, iFile *pFile, iBitmap2D *pBmp, int line)
{
  tU8 b[4];
  tU32 n;
  int i, j, k;
  tU8* pLine = (tU8*)(pBmp->GetData()+(pBmp->GetPitch()*line));

  for (i = 0; i < length; i++)
  {
    j = i % 4;

    if (j == 0)
    {
      n = pFile->ReadLE32();

      for (k = 0; k < 4; k++)
      {
        b[k] = (tU8)(n & 0xFF);
        n = n >> 8;
      }
    }

    pLine[i] = b[j];
  }
}

///////////////////////////////////////////////
static void read_24bit_line(int length, iFile *pFile, iBitmap2D *pBmp, int line)
{
  int i, nbytes;
  int r, g, b;
  tU8* pLine = (tU8*)(pBmp->GetData()+(pBmp->GetPitch()*line));

  nbytes = 0;

  for (i = 0; i < length; i++)
  {
    b = pFile->Read8();
    g = pFile->Read8();
    r = pFile->Read8();
    pLine[(i*3) + 2] = r;
    pLine[(i*3) + 1] = g;
    pLine[(i*3) + 0] = b;
    nbytes += 3;
  }

  nbytes = nbytes % 4;

  if (nbytes != 0)
  {
    for (i = nbytes; i < 4; i++)
      pFile->Read8();
  }
}


///////////////////////////////////////////////
static void read_32bit_line(int length, iFile *pFile, iBitmap2D *pBmp, int line)
{
  int i, nbytes;
  int r, g, b, a;
  tU8* pLine = (tU8*)(pBmp->GetData()+(pBmp->GetPitch()*line));

  nbytes = 0;

  for (i = 0; i < length; i++)
  {
    b = pFile->Read8();
    g = pFile->Read8();
    r = pFile->Read8();
    a = pFile->Read8();
    pLine[(i*4) + 3] = a;
    pLine[(i*4) + 2] = r;
    pLine[(i*4) + 1] = g;
    pLine[(i*4) + 0] = b;
    nbytes += 4;
  }

  nbytes = nbytes % 4;

  if (nbytes != 0)
  {
    for (i = nbytes; i < 4; i++)
      pFile->Read8();
  }
}

///////////////////////////////////////////////
static int read_image(iFile *f, iBitmap2D *bmp, NI_BITMAPINFOHEADER *infoheader)
{
  int i;

  for (i = 0; i <(int)infoheader->biHeight; i++)
  {
    switch (infoheader->biBitCount)
    {
      case 8:
        read_8bit_line(infoheader->biWidth, f, bmp, infoheader->biHeight - i - 1);
        break;

      case 24:
        read_24bit_line(infoheader->biWidth, f, bmp, infoheader->biHeight - i - 1);
        break;

      case 32:
        read_32bit_line(infoheader->biWidth, f, bmp, infoheader->biHeight - i - 1);
        break;

      default: return 0;
    }
  }

  return 1;
}

///////////////////////////////////////////////
static void read_RLE8_compressed_image(iFile *pFile, iBitmap2D *pBmp, NI_BITMAPINFOHEADER *infoheader)
{
  unsigned char count, val, val0;
  int j, pos, line;
  int eolflag, eopicflag;
  tU8* pData = pBmp->GetData();
  tU32 nPitch = pBmp->GetPitch();

  eopicflag = 0;
  line = infoheader->biHeight - 1;

  while (eopicflag == 0)
  {
    pos = 0;
    eolflag = 0;

    while ((eolflag == 0) &&(eopicflag == 0))
    {
      count = pFile->Read8();
      val = pFile->Read8();

      if (count > 0)
      {
        for (j = 0; j < count; j++)
        {
          ((tU8*)(pData+(line*nPitch)))[pos] = val;
          pos++;
        }
      }
      else
      {
        switch (val)
        {
          case 0:
            eolflag = 1;
            break;

          case 1:
            eopicflag = 1;
            break;

          case 2:
            count = pFile->Read8();
            val = pFile->Read8();
            pos += count;
            line -= val;
            break;

          default:
            for (j = 0; j < val; j++)
            {
              val0 = pFile->Read8();
              ((tU8*)(pData+(line*nPitch)))[pos] = val0;
              pos++;
            }

            if (j%2 == 1)
              val0 = pFile->Read8();
            break;
        }
      }

      if (pos >(int)infoheader->biWidth)
        eolflag = 1;
    }

    line--;
    if (line < 0)
      eopicflag = 1;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cBmp interface implementation
struct BitmapLoader_BMP : public ImplRC<iBitmapLoader> {
  virtual iBitmapBase* __stdcall LoadBitmap(iGraphics* apGraphics, iFile* pFile) niImpl
  {
    niCheckIsOK(apGraphics,NULL);
    niCheckIsOK(pFile,NULL);

    NI_BITMAPFILEHEADER fileheader;
    NI_BITMAPINFOHEADER infoheader;
    int ncol;
    unsigned long biSize;
    int bpp;
    cString strPixelFormat;
    astl::vector<tU32> palette;
    Ptr<iBitmap2D> ptrBmp;

    if (!read_bmfileheader(pFile, &fileheader)) {
      niError(_A("Not a valid bitmap file."));
      return NULL;
    }

    biSize = pFile->ReadLE32();

    if (biSize == WININFOHEADERSIZE)
    {
      if (!read_win_bminfoheader(pFile, &infoheader))
      {
        niError(_A("Can't read win header."));
        return NULL;
      }

      ncol = (fileheader.bfOffBits - 54) / 4;
      bpp = 1;

    }
    else if (biSize == OS2INFOHEADERSIZE)
    {
      if (!read_os2_bminfoheader(pFile, &infoheader))
      {
        niError(_A("Can't read OS2 header."));
        return NULL;
      }

      ncol = (fileheader.bfOffBits - 26) / 3;
      bpp = 0;
    }
    else
    {
      niError(_A("Unknow bitmap header."));
      return NULL;
    }

    if (infoheader.biBitCount == 8) {
      palette.resize(256);
      read_bmicolors(ncol, palette.data(), pFile, bpp);
    }
    else {
      read_bmicolors(ncol, NULL, pFile, bpp);
    }

    if (infoheader.biBitCount == 24) {
      bpp = 24;
      strPixelFormat = _A("B8G8R8");
    }
    else if (infoheader.biBitCount == 32) {
      bpp = 32;
      strPixelFormat = _A("B8G8R8A8");
    }
    else if (infoheader.biBitCount == 8) {
      bpp = 8;
      strPixelFormat = _A("A8");
    }
    else {
      niError(niFmt(_A("Can't load bitmap of '%d' bit count."), infoheader.biBitCount));
      return NULL;
    }

    ptrBmp = apGraphics->CreateBitmap2DEx(infoheader.biWidth, infoheader.biHeight, apGraphics->CreatePixelFormat(strPixelFormat.Chars()));
    niCheck(ptrBmp.IsOK(), NULL);

    switch (infoheader.biCompression) {
      case NI_BI_RGB: {
        if (!read_image(pFile, ptrBmp, &infoheader)) {
          niError("Can't read RGB BMP image.");
          return NULL;
        }
        break;
      }

      case NI_BI_RLE8: {
        read_RLE8_compressed_image(pFile, ptrBmp, &infoheader);
        break;
      }

      default: {
        niError(niFmt("Can't read BMP image, unknown compression '%d'.",infoheader.biCompression));
        return NULL;
      }
    }

    if (!palette.empty()) {
      Ptr<iBitmap2D> convertedBmp = apGraphics->CreateBitmap2D(
          infoheader.biWidth, infoheader.biHeight, "B8G8R8A8");
      BmpUtils_BlitPaletteTo32Bits(
          convertedBmp->GetData(), ptrBmp->GetData(),
          infoheader.biWidth * infoheader.biHeight,
          palette.data());
      ptrBmp = convertedBmp;
    }

    return ptrBmp.GetRawAndSetNull();
  }
};

struct BitmapSaver_BMP : public ImplRC<iBitmapSaver> {
  tBool __stdcall SaveBitmap(iGraphics* apGraphics, iFile* pFile, iBitmapBase* pBmpBase, tU32 ulCompression) niImpl
  {
    niAssert(pBmpBase->GetType() == eBitmapType_2D);

    QPtr<iBitmap2D> pBmp = pBmpBase;
    if (!pBmp.IsOK()) {
      niError("Can't query iBitmap2D interface.");
      return eFalse;
    }

    if (!(pBmpBase->GetPixelFormat()->GetCaps()&ePixelFormatCaps_UnpackPixel)) {
      niError(niFmt("Can't unpack pixel format '%s'.", pBmpBase->GetPixelFormat()->GetFormat()));
      return eFalse;
    }

    iPixelFormat* pPixFmt = pBmp->GetPixelFormat();
    int bfSize;
    int bpp = 0;
    tI32 i, j;

    if (pPixFmt->GetNumABits() && pPixFmt->GetNumRBits() == 0) {
      // alpha only
      bpp = 8;
      bfSize = 54 + 256*4 + pBmp->GetWidth()*pBmp->GetHeight()*1;
    }
    else if (pPixFmt->GetNumABits()) {
      // rgba
      bpp = 32;
      bfSize = 54 + pBmp->GetWidth()*pBmp->GetHeight()*4;
    }
    else {
      // rgb
      bpp = 24;
      bfSize = 54 + pBmp->GetWidth()*pBmp->GetHeight()*3;
    }

    tU32 filler = 3 - ((pBmp->GetWidth()*(bpp/8) - 1) & 3);

    tBool bInvertWrite = eFalse;

    if (ulCompression != eInvalidHandle) {
      pFile->WriteLE16(0x4D42);
      pFile->WriteLE32(bfSize);
      pFile->WriteLE16(0);
      pFile->WriteLE16(0);
      if (bpp == 8) {
        pFile->WriteLE32(54 + 256*4);
      }
      else {
        pFile->WriteLE32(54);
      }
    }
    else {
      //bInvertWrite = eTrue;
    }

    if (bpp == 8) {
      bfSize -= 54 + 256*4;
    }
    else {
      bfSize -= 54;
    }

    pFile->WriteLE32(40);
    pFile->WriteLE32(pBmp->GetWidth());
    pFile->WriteLE32(pBmp->GetHeight());
    pFile->WriteLE16(1);
    pFile->WriteLE16(bpp);
    pFile->WriteLE32(0);
    pFile->WriteLE32(bfSize);
    pFile->WriteLE32(0);
    pFile->WriteLE32(0);

    if (bpp == 8) {
      // write a greyscale palette
      pFile->WriteLE32(256);
      pFile->WriteLE32(256);
      for (i = 0; i < 256; i++) {
        pFile->Write8(i);
        pFile->Write8(i);
        pFile->Write8(i);
        pFile->Write8(i);
      }
    }
    else {
      pFile->WriteLE32(0);
      pFile->WriteLE32(0);
    }

    tBool bRet = eTrue;
    if (pBmp->BeginUnpackPixels()) {
      niDeclareTempPixel();
      if (bpp == 32) {
        if (bInvertWrite) {
          for (i = 0; i < (tI32)pBmp->GetHeight(); i++) {
            for (j = 0; j < (tI32)pBmp->GetWidth(); j++) {
              sColor4ub color = pPixFmt->UnpackPixelub(pBmp->GetPixel(j, i, niTempPixelPtr()));
              pFile->Write8(color.z);
              pFile->Write8(color.y);
              pFile->Write8(color.x);
              pFile->Write8(color.w);
            }
            for (j = 0; j < (tI32)filler; j++) pFile->Write8(0);
          }
        }
        else {
          for (i = pBmp->GetHeight()-1; i >= 0; i--) {
            for (j = 0; j < (tI32)pBmp->GetWidth(); j++) {
              sColor4ub color = pPixFmt->UnpackPixelub(pBmp->GetPixel(j, i, niTempPixelPtr()));
              pFile->Write8(color.z);
              pFile->Write8(color.y);
              pFile->Write8(color.x);
              pFile->Write8(color.w);
            }
            for (j = 0; j < (tI32)filler; j++) pFile->Write8(0);
          }
        }
      }
      else if (bpp == 24) {
        if (bInvertWrite) {
          for (i = 0; i < (tI32)pBmp->GetHeight(); i++) {
            for (j = 0; j < (tI32)pBmp->GetWidth(); j++) {
              sColor4ub color = pPixFmt->UnpackPixelub(pBmp->GetPixel(j, i, niTempPixelPtr()));
              pFile->Write8(color.z);
              pFile->Write8(color.y);
              pFile->Write8(color.x);
            }
            for (j = 0; j < (tI32)filler; j++) pFile->Write8(0);
          }
        }
        else {
          for (i = pBmp->GetHeight()-1; i >= 0; i--) {
            for (j = 0; j < (tI32)pBmp->GetWidth(); j++) {
              sColor4ub color = pPixFmt->UnpackPixelub(pBmp->GetPixel(j, i, niTempPixelPtr()));
              pFile->Write8(color.z);
              pFile->Write8(color.y);
              pFile->Write8(color.x);
            }
            for (j = 0; j < (tI32)filler; j++) pFile->Write8(0);
          }
        }
      }
      else if (bpp == 8)
      {
        if (bInvertWrite) {
          for (i = 0; i < (tI32)pBmp->GetHeight(); i++) {
            for (j = 0; j < (tI32)pBmp->GetWidth(); j++) {
              sColor4ub color = pPixFmt->UnpackPixelub(pBmp->GetPixel(j, i, niTempPixelPtr()));
              pFile->Write8(color.w);
            }
            for (j = 0; j < (tI32)filler; j++) pFile->Write8(0);
          }
        }
        else {
          for (i = pBmp->GetHeight()-1; i >= 0; i--) {
            for (j = 0; j < (tI32)pBmp->GetWidth(); j++) {
              const sColor4ub& color = pPixFmt->UnpackPixelub(pBmp->GetPixel(j, i, niTempPixelPtr()));
              pFile->Write8(color.w);
            }
            for (j = 0; j < (tI32)filler; j++) pFile->Write8(0);
          }
        }
      }
      else {
        niError(niFmt("Unsupported BMP bpp '%d'", bpp));
        bRet = eFalse;
      }
      pBmp->EndUnpackPixels();
    }
    else {
      niError("Can't unpack pixels in source bitmap");
      bRet = eFalse;
    }

    return bRet;
  }
};

niExportFunc(iUnknown*) New_BitmapLoader_bmp(const Var&,const Var&) {
  return niNew BitmapLoader_BMP();
}

niExportFunc(iUnknown*) New_BitmapSaver_bmp(const Var&,const Var&) {
  return niNew BitmapSaver_BMP();
}
