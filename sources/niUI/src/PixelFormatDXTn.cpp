// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "PixelFormatDXTn.h"

#if !defined PIXELFORMAT_NO_DXT

#if !defined PIXELFORMAT_NO_DXT_COMPRESSION
#include "libDXTn/ImageDXTC.h"
#endif

#pragma niTodo("Add a dxtn pixel format which autodetect the best format to use (DXT1, DXT3 or DXT5)")
#pragma niTodo("Add a parameter for the automatic dithering. (img.DiffuseError(8, 5, 6, 5))")
#pragma niTodo("Add a parameter for the alpha thresold for DXT1 compression.")

/*
  DXT1/DXT3/DXT5 texture decompression

  ---

  Copyright (C) 2009 Benjamin Dobell, Glass Echidna

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ---
*/
static void DecompressBlockDXT1Internal (const tU8* block,
                                         tU32* output,
                                         tU32 outputStride,
                                         const tU8* alphaValues)
{
  tU32 temp, code;

  tU16 color0, color1;
  tU8 r0, g0, b0, r1, g1, b1;

  int i, j;

  color0 = *(const tU16*)(block);
  color1 = *(const tU16*)(block + 2);

  temp = (color0 >> 11) * 255 + 16;
  r0 = (tU8)((temp/32 + temp)/32);
  temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
  g0 = (tU8)((temp/64 + temp)/64);
  temp = (color0 & 0x001F) * 255 + 16;
  b0 = (tU8)((temp/32 + temp)/32);

  temp = (color1 >> 11) * 255 + 16;
  r1 = (tU8)((temp/32 + temp)/32);
  temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
  g1 = (tU8)((temp/64 + temp)/64);
  temp = (color1 & 0x001F) * 255 + 16;
  b1 = (tU8)((temp/32 + temp)/32);

  code = *(const tU32*)(block + 4);

  if (color0 > color1) {
    for (j = 0; j < 4; ++j) {
      for (i = 0; i < 4; ++i) {
        tU32 finalColor, positionCode;
        tU8 alpha;

        alpha = alphaValues [j*4+i];

        finalColor = 0;
        positionCode = (code >>  2*(4*j+i)) & 0x03;

        switch (positionCode) {
          case 0:
            finalColor = ULColorBuild(r0, g0, b0, alpha);
            break;
          case 1:
            finalColor = ULColorBuild(r1, g1, b1, alpha);
            break;
          case 2:
            finalColor = ULColorBuild((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, alpha);
            break;
          case 3:
            finalColor = ULColorBuild((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, alpha);
            break;
        }

        output [j*outputStride + i] = finalColor;
      }
    }
  } else {
    for (j = 0; j < 4; ++j) {
      for (i = 0; i < 4; ++i) {
        tU32 finalColor, positionCode;
        tU8 alpha;

        alpha = alphaValues [j*4+i];

        finalColor = 0;
        positionCode = (code >>  2*(4*j+i)) & 0x03;

        switch (positionCode) {
          case 0:
            finalColor = ULColorBuild(r0, g0, b0, alpha);
            break;
          case 1:
            finalColor = ULColorBuild(r1, g1, b1, alpha);
            break;
          case 2:
            finalColor = ULColorBuild((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, alpha);
            break;
          case 3:
            finalColor = ULColorBuild(0, 0, 0, alpha);
            break;
        }

        output [j*outputStride + i] = finalColor;
      }
    }
  }
}

/*
  void DecompressBlockDXT1(): Decompresses one block of a DXT1 texture and stores the resulting pixels at the appropriate offset in 'image'.

  tU32 x:           x-coordinate of the first pixel in the block.
  tU32 y:           y-coordinate of the first pixel in the block.
  tU32 width:         width of the texture being decompressed.
  const tU8 *blockStorage:  pointer to the block to decompress.
  tU32 *image:        pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockDXT1(tU32 x, tU32 y, tU32 width,
                         const tU8* blockStorage,
                         tU32* image)
{
  static const tU8 const_alpha [] = {
    255, 255, 255, 255,
    255, 255, 255, 255,
    255, 255, 255, 255,
    255, 255, 255, 255
  };

  DecompressBlockDXT1Internal (blockStorage,
                               image + x + (y * width), width, const_alpha);
}

/*
  void DecompressBlockDXT5(): Decompresses one block of a DXT5 texture and stores the resulting pixels at the appropriate offset in 'image'.

  tU32 x:           x-coordinate of the first pixel in the block.
  tU32 y:           y-coordinate of the first pixel in the block.
  tU32 width:         width of the texture being decompressed.
  const tU8 *blockStorage:  pointer to the block to decompress.
  tU32 *image:        pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockDXT5(tU32 x, tU32 y, tU32 width,
                         const tU8* blockStorage, tU32* image)
{
  tU8 alpha0, alpha1;
  const tU8* bits;
  tU32 alphaCode1;
  tU16 alphaCode2;

  tU16 color0, color1;
  tU8 r0, g0, b0, r1, g1, b1;

  int i, j;

  tU32 temp, code;

  alpha0 = *(blockStorage);
  alpha1 = *(blockStorage + 1);

  bits = blockStorage + 2;
  alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
  alphaCode2 = bits[0] | (bits[1] << 8);

  color0 = *(const tU16*)(blockStorage + 8);
  color1 = *(const tU16*)(blockStorage + 10);

  temp = (color0 >> 11) * 255 + 16;
  r0 = (tU8)((temp/32 + temp)/32);
  temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
  g0 = (tU8)((temp/64 + temp)/64);
  temp = (color0 & 0x001F) * 255 + 16;
  b0 = (tU8)((temp/32 + temp)/32);

  temp = (color1 >> 11) * 255 + 16;
  r1 = (tU8)((temp/32 + temp)/32);
  temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
  g1 = (tU8)((temp/64 + temp)/64);
  temp = (color1 & 0x001F) * 255 + 16;
  b1 = (tU8)((temp/32 + temp)/32);

  code = *(const tU32*)(blockStorage + 12);

  for (j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++) {
      tU8 finalAlpha;
      int alphaCode, alphaCodeIndex;
      tU8 colorCode;
      tU32 finalColor;

      alphaCodeIndex = 3*(4*j+i);
      if (alphaCodeIndex <= 12) {
        alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
      } else if (alphaCodeIndex == 15) {
        alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
      } else /* alphaCodeIndex >= 18 && alphaCodeIndex <= 45 */ {
        alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
      }

      if (alphaCode == 0) {
        finalAlpha = alpha0;
      } else if (alphaCode == 1) {
        finalAlpha = alpha1;
      } else {
        if (alpha0 > alpha1) {
          finalAlpha = (tU8)(((8-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/7);
        } else {
          if (alphaCode == 6) {
            finalAlpha = 0;
          } else if (alphaCode == 7) {
            finalAlpha = 255;
          } else {
            finalAlpha = (tU8)(((6-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/5);
          }
        }
      }

      colorCode = (code >> 2*(4*j+i)) & 0x03;
      finalColor = 0;

      switch (colorCode) {
        case 0:
          finalColor = ULColorBuild(r0, g0, b0, finalAlpha);
          break;
        case 1:
          finalColor = ULColorBuild(r1, g1, b1, finalAlpha);
          break;
        case 2:
          finalColor = ULColorBuild((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, finalAlpha);
          break;
        case 3:
          finalColor = ULColorBuild((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, finalAlpha);
          break;
      }

      image [i + x + (width* (y+j))] = finalColor;
    }
  }
}

/*
  void DecompressBlockDXT3(): Decompresses one block of a DXT3 texture and stores the resulting pixels at the appropriate offset in 'image'.

  tU32 x:           x-coordinate of the first pixel in the block.
  tU32 y:           y-coordinate of the first pixel in the block.
  tU32 height:        height of the texture being decompressed.
  const tU8 *blockStorage:  pointer to the block to decompress.
  tU32 *image:        pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockDXT3(tU32 x, tU32 y, tU32 width,
                         const tU8* blockStorage,
                         tU32* image)
{
  int i;

  tU8 alphaValues [16] = { 0 };

  for (i = 0; i < 4; ++i) {
    const tU16* alphaData = (const tU16*) (blockStorage);

    alphaValues [i*4 + 0] = (((*alphaData) >> 0) & 0xF ) * 17;
    alphaValues [i*4 + 1] = (((*alphaData) >> 4) & 0xF ) * 17;
    alphaValues [i*4 + 2] = (((*alphaData) >> 8) & 0xF ) * 17;
    alphaValues [i*4 + 3] = (((*alphaData) >> 12) & 0xF) * 17;

    blockStorage += 2;
  }

  DecompressBlockDXT1Internal (blockStorage,
                               image + x + (y * width), width, alphaValues);
}


/**
 * unsigned long ULColorBuild(): Helper method that packs RGBA channels into a single 4 byte pixel.
 *
 * unsigned char r: red channel.
 * unsigned char g: green channel.
 * unsigned char b: blue channel.
 * unsigned char a: alpha channel.
 */
unsigned long ULColorBuild_(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
  return ((r << 24) | (g << 16) | (b << 8) | a);
}


/**
 * void DecompressBlockDXT5(): Decompresses one block of a DXT5 texture and stores the resulting pixels at the appropriate offset in 'image'.
 *
 * unsigned long x:           x-coordinate of the first pixel in the block.
 * unsigned long y:           y-coordinate of the first pixel in the block.
 * unsigned long width:         width of the texture being decompressed.
 * unsigned long height:        height of the texture being decompressed.
 * const unsigned char *blockStorage: pointer to the block to decompress.
 * unsigned long *image:        pointer to image where the decompressed pixel data should be stored.
 */
void DecompressBlockDXT5_(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, unsigned long *image) {
  unsigned char alpha0 = *reinterpret_cast<const unsigned char *>(blockStorage);
  unsigned char alpha1 = *reinterpret_cast<const unsigned char *>(blockStorage + 1);

  const unsigned char *bits = blockStorage + 2;
  unsigned long alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
  unsigned short alphaCode2 = bits[0] | (bits[1] << 8);

  unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage + 8);
  unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 10);

  unsigned long temp;

  temp = (color0 >> 11) * 255 + 16;
  unsigned char r0 = (unsigned char)((temp/32 + temp)/32);
  temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
  unsigned char g0 = (unsigned char)((temp/64 + temp)/64);
  temp = (color0 & 0x001F) * 255 + 16;
  unsigned char b0 = (unsigned char)((temp/32 + temp)/32);

  temp = (color1 >> 11) * 255 + 16;
  unsigned char r1 = (unsigned char)((temp/32 + temp)/32);
  temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
  unsigned char g1 = (unsigned char)((temp/64 + temp)/64);
  temp = (color1 & 0x001F) * 255 + 16;
  unsigned char b1 = (unsigned char)((temp/32 + temp)/32);

  unsigned long code = *reinterpret_cast<const unsigned long *>(blockStorage + 12);

  for (int j=0; j < 4; j++) {
    for (int i=0; i < 4; i++) {
      int alphaCodeIndex = 3*(4*j+i);
      int alphaCode;

      if (alphaCodeIndex <= 12) {
        alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
      } else if (alphaCodeIndex == 15) {
        alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
      } else { // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
        alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
      }

      unsigned char finalAlpha;
      if (alphaCode == 0) {
        finalAlpha = alpha0;
      } else if (alphaCode == 1) {
        finalAlpha = alpha1;
      } else {
        if (alpha0 > alpha1) {
          finalAlpha = ((8-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/7;
        } else {
          if (alphaCode == 6) {
            finalAlpha = 0;
          } else if (alphaCode == 7) {
            finalAlpha = 255;
          } else {
            finalAlpha = ((6-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/5;
          }
        }
      }

      unsigned char colorCode = (code >> 2*(4*j+i)) & 0x03;

      unsigned long finalColor;
      switch (colorCode) {
        case 0:
          finalColor = ULColorBuild_(r0, g0, b0, finalAlpha);
          break;
        case 1:
          finalColor = ULColorBuild_(r1, g1, b1, finalAlpha);
          break;
        case 2:
          finalColor = ULColorBuild_((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, finalAlpha);
          break;
        case 3:
          finalColor = ULColorBuild_((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, finalAlpha);
          break;
      }

      if (x + i < width) image[(y + j)*width + (x + i)] = finalColor;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cPixelFormatDXTn implementation

///////////////////////////////////////////////
cPixelFormatDXTn::cPixelFormatDXTn(const achar* aszFormat)
{
  ZeroMembers();
  if (ni::StrICmp(aszFormat, _A("dxt1")) == 0) {
    mulFlags |= DXTFLAG_1;
  }
  else if (ni::StrICmp(aszFormat, _A("dxt1a")) == 0) {
    mulFlags |= DXTFLAG_1;
    mulFlags |= DXTFLAG_ALPHA1;
  }
  else if (ni::StrICmp(aszFormat, _A("dxt3")) == 0) {
    mulFlags |= DXTFLAG_3;
  }
  else {
    mulFlags |= DXTFLAG_5;
  }
}

///////////////////////////////////////////////
cPixelFormatDXTn::~cPixelFormatDXTn()
{
  niSafeFree(mpUnpackBuffer);
}

///////////////////////////////////////////////
void cPixelFormatDXTn::ZeroMembers()
{
  mulFlags = 0;
  mpUnpackBuffer = NULL;
}

///////////////////////////////////////////////
tBool cPixelFormatDXTn::IsOK() const
{
  return eTrue;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetNumParameters() const
{
  return 0;
}

///////////////////////////////////////////////
tBool cPixelFormatDXTn::SetParameter(tU32 ulParameter, tU32 ulValue)
{
  return eTrue;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetParameter(tU32 ulParameter) const
{
  return 0;
}

///////////////////////////////////////////////
tBool cPixelFormatDXTn::IsSamePixelFormat(const iPixelFormat* pPixFmt) const
{
  return ni::StrICmp(pPixFmt->GetFormat(), GetFormat()) == 0;
}

///////////////////////////////////////////////
iPixelFormat* cPixelFormatDXTn::Clone() const
{
  return niNew cPixelFormatDXTn(GetFormat());
}

///////////////////////////////////////////////
const achar* cPixelFormatDXTn::GetFormat() const
{
  return niFlagIs(mulFlags,DXTFLAG_1) ?
      (niFlagIs(mulFlags,DXTFLAG_ALPHA1)?_A("DXT1A"):_A("DXT1")):
      (niFlagIs(mulFlags,DXTFLAG_5)?_A("DXT5"):_A("DXT3"));
}

///////////////////////////////////////////////
ePixelFormatCaps cPixelFormatDXTn::GetCaps() const
{
  return ePixelFormatCaps(
      ePixelFormatCaps_Blit   |
      ePixelFormatCaps_BlitStretchHalf |
      ePixelFormatCaps_UnpackPixel |
      ePixelFormatCaps_BlockCompressed);
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetBitsPerPixel() const
{
  return 16;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetBytesPerPixel() const
{
  return 4;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetSize(tU32 ulW, tU32 ulH, tU32 ulD) const
{
  tU32 blockSize = 16;
  if (mulFlags & DXTFLAG_1)
    blockSize = 8;
  if (ulD) {
    return ((ulW+3)/4)*((ulH+3)/4)*((ulD+3)/4)*blockSize;
  }
  else {
    return ((ulW+3)/4)*((ulH+3)/4)*blockSize;
  }
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetNumComponents() const
{
  return (mulFlags&DXTFLAG_3)?4:3;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetNumRBits() const
{
  return 5;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetNumGBits() const
{
  return 6;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetNumBBits() const
{
  return 5;
}

///////////////////////////////////////////////
tU32 cPixelFormatDXTn::GetNumABits() const
{
  if (niFlagIs(mulFlags,DXTFLAG_ALPHA1))
    return 1;
  if (niFlagIs(mulFlags,DXTFLAG_5))
    return 8;
  if (niFlagIs(mulFlags,DXTFLAG_3))
    return 4;
  return 0;
}

///////////////////////////////////////////////
tPtr cPixelFormatDXTn::BuildPixelub(tPtr pOut, tU8 r, tU8 g, tU8 b, tU8 a) const
{
  return pOut;
}

///////////////////////////////////////////////
tPtr cPixelFormatDXTn::BeginUnpackPixels(tPtr pSrc, tU32 ulPitch, tU32 ulX, tU32 ulY, tU32 ulW, tU32 ulH)
{
  if (ulX != 0 || ulY != 0) {
    return NULL;
  }

  niSafeFree(mpUnpackBuffer);

  mpUnpackBuffer = (tPtr)niMalloc(ni::Max(4,ulW) * ni::Max(4,ulH) * 4);
  const tU32 blockCountX = (ulW + 3) / 4;
  const tU32 blockCountY = (ulH + 3) / 4;
  if (niFlagIs(mulFlags,DXTFLAG_5)) {
    const tU32 blockSize = 16;
    for (tU32 j = 0; j < blockCountY; j++) {
      for (tU32 i = 0; i < blockCountX; i++) {
        DecompressBlockDXT5(i*4, j*4, ulW, pSrc + i * blockSize, (tU32*)mpUnpackBuffer);
      }
      pSrc += blockCountX * blockSize;
    }
  }
  else if (niFlagIs(mulFlags,DXTFLAG_3)) {
    const tU32 blockSize = 16;
    for (tU32 j = 0; j < blockCountY; j++) {
      for (tU32 i = 0; i < blockCountX; i++) {
        DecompressBlockDXT3(i*4, j*4, ulW, pSrc + i * blockSize, (tU32*)mpUnpackBuffer);
      }
      pSrc += blockCountX * blockSize;
    }
  }
  else {
    const tU32 blockSize = 8;
    for (tU32 j = 0; j < blockCountY; j++) {
      for (tU32 i = 0; i < blockCountX; i++) {
        DecompressBlockDXT1(i*4, j*4, ulW, pSrc + i * blockSize, (tU32*)mpUnpackBuffer);
      }
      pSrc += blockCountX * blockSize;
    }
  }

  return mpUnpackBuffer;
}

///////////////////////////////////////////////
void cPixelFormatDXTn::EndUnpackPixels()
{
  niSafeFree(mpUnpackBuffer);
}

///////////////////////////////////////////////
sColor4f cPixelFormatDXTn::UnpackPixelf(tPtr pColor) const
{
  const tU32 c = *(tU32*)pColor;
  return Vec4f(
      ULColorGetRf(c),
      ULColorGetGf(c),
      ULColorGetBf(c),
      ULColorGetAf(c));
}

///////////////////////////////////////////////
sVec4i cPixelFormatDXTn::UnpackPixelul(tPtr pColor) const
{
  const tU32 c = *(tU32*)pColor;
  return Vec4i(
      ULColorGetR(c) << 24,
      ULColorGetG(c) << 24,
      ULColorGetB(c) << 24,
      ULColorGetA(c) << 24);
}

///////////////////////////////////////////////
sColor4ub cPixelFormatDXTn::UnpackPixelub(tPtr pColor) const
{
  const tU32 c = *(tU32*)pColor;
  sColor4ub rc = {
    ULColorGetR(c),
    ULColorGetG(c),
    ULColorGetB(c),
    ULColorGetA(c)
  };
  return rc;
}

///////////////////////////////////////////////
sVec4i cPixelFormatDXTn::UnpackPixelus(tPtr pColor) const
{
  const tU32 c = *(tU32*)pColor;
  return Vec4i(
      ULColorGetR(c) << 8,
      ULColorGetG(c) << 8,
      ULColorGetB(c) << 8,
      ULColorGetA(c) << 8);
}


///////////////////////////////////////////////
tBool cPixelFormatDXTn::Blit(tPtr pDst, tU32 ulDestPitch, tU32 dx, tU32 dy,
                             tPtr pSrcData, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy,
                             tU32 w, tU32 h, ePixelFormatBlit blitFlags) const
{
  if (blitFlags != ePixelFormatBlit_Normal || dx != 0 || dy != 0)
    return eFalse;

  if (IsSamePixelFormat(pSrcFmt))
  {
    if (sx != 0 || sy != 0)
      return eFalse;

    memcpy(pDst, pSrcData, GetSize(w,h,0));
  }
  else
  { // Pack
    if (!(pSrcFmt->GetCaps() & ePixelFormatCaps_UnpackPixel))
      return eFalse;

#if defined PIXELFORMAT_NO_DXT_COMPRESSION
    niError("Blit: DXT compression not supported on this platform.");
    return eFalse;
#else
    // Fill an Image
    tPtr pUnpackedSrc = pSrcFmt->BeginUnpackPixels(pSrcData, ulSrcPitch, sx, sy, w, h);
    if (pUnpackedSrc) {
      Image img;
      sColor4ub c;
      img.SetSize(w, h);
      Color* pPixels = img.GetPixels();
      tU32 ulSBpp = pSrcFmt->GetBytesPerPixel();
      for (tU32 y = 0; y < h; ++y)
      {
        tU8* pS = ((tU8*)pUnpackedSrc)+(ulSrcPitch*y)+(sx*ulSBpp);
        Color* pD = pPixels+(w*y);
        for (tU32 x = 0; x < w; ++x)
        {
          c = pSrcFmt->UnpackPixelub(tPtr(pS));
          pD->a = pSrcFmt->GetNumABits()?c.w:255;
          pD->r = c.x;
          pD->g = c.y;
          pD->b = c.z;
          pS += ulSBpp;
          pD++;
        }
      }

      // Floyd/Steinberg (modified) error diffusion
      img.DiffuseError(8, 5, 6, 5);

      // Pack the image
      ImageDXTC dxtc;
      dxtc.FromImage32(&img, (mulFlags&DXTFLAG_1)?DC_DXT1:DC_DXT3);

      // Copy the packed version in the destination
      memcpy(pDst, dxtc.GetBlocks(), GetSize(w,h,0));

      pSrcFmt->EndUnpackPixels();
    }
#endif
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool cPixelFormatDXTn::BlitStretch(tPtr pDst, tU32 ulDestPitch, tU32 ulDestWidth, tU32 ulDestHeight, tU32 dx, tU32 dy, tU32 dw, tU32 dh,
                                    tPtr pSrc, tU32 ulSrcPitch, iPixelFormat* pSrcFmt, tU32 sx, tU32 sy, tU32 sw, tU32 sh) const
{
#if defined PIXELFORMAT_NO_DXT_COMPRESSION
  niError("Blit: DXT compression not supported on this platform.");
  return eFalse;
#else
  if (!(sw&1) && !(sh&1) && dw == sw/2 && dh == sh/2 && dx == 0 && dy == 0 && sx == 0 && sy == 0)
  {
    /// This has to work for older assets... cause well... some smart ass think mipmaps are optional
    /// for DXT textures...
    Image img;
    if (IsSamePixelFormat(pSrcFmt))
    {
      Image imgSrc;
      ImageDXTC dxtcSrc;
      dxtcSrc.SetMethod((mulFlags&DXTFLAG_1)?DC_DXT1:DC_DXT3);
      dxtcSrc.SetSize(sw, sh);
      memcpy(dxtcSrc.GetBlocks(), pSrc, pSrcFmt->GetSize(sw,sh,0));
      dxtcSrc.ToImage32(&imgSrc);
      imgSrc.Quarter(img);
    }
    else
    {
      if (!(pSrcFmt->GetCaps() & ePixelFormatCaps_UnpackPixel))
        return eFalse;

      pSrc = pSrcFmt->BeginUnpackPixels(pSrc, ulSrcPitch, sx, sy, sw, sh);
      if (pSrc)
      {
        img.SetSize(dw, dh);
        tU8* pCol;
        tU32 x, y;
        Color* pPixels = img.GetPixels();
        tU32 ulSrcByPP = pSrcFmt->GetBytesPerPixel();

        for (y = 0; y < dh; ++y)
        {
          Color* pD = pPixels+(dw*y);
          for (x = 0; x < dw; ++x)
          {
            pCol = ((tU8*)pSrc)+(ulSrcPitch*(y<<1))+((x<<1)*ulSrcByPP);
            sColor4f ca = pSrcFmt->UnpackPixelf(tPtr(pCol));

            pCol += ulSrcByPP;  // x+1
            sColor4f cb = pSrcFmt->UnpackPixelf(tPtr(pCol));

            pCol += ulSrcPitch;           // y+1, x+1
            sColor4f cc = pSrcFmt->UnpackPixelf(tPtr(pCol));

            pCol -= ulSrcByPP;  // y+1, x
            sColor4f cd = pSrcFmt->UnpackPixelf(tPtr(pCol));

            //col /= 4; // /4 is replace by *63.75f instead of *255.0f
            pD->r = tU8((ca.x+cb.x+cc.x+cd.x) * 63.75f);
            pD->g = tU8((ca.y+cb.y+cc.y+cd.y) * 63.75f);
            pD->b = tU8((ca.z+cb.z+cc.z+cd.z) * 63.75f);
            pD->a = tU8((ca.w+cb.w+cc.w+cd.w) * 63.75f);
            pD++;
          }
        }
      }
    }

    if (img.GetXSize() && img.GetYSize())
    {
      // Floyd/Steinberg (modified) error diffusion
      img.DiffuseError(8, 5, 6, 5);

      // Pack the image
      ImageDXTC dxtc;
      dxtc.FromImage32(&img, (mulFlags&DXTFLAG_1)?DC_DXT1:DC_DXT3);

      // Copy the packed version in the destination
      memcpy(pDst, dxtc.GetBlocks(), GetSize(img.GetXSize(),img.GetYSize(),0));
    }
    return eTrue;
  }

  return eFalse;
#endif
}

#endif // PIXELFORMAT_NO_DXT
