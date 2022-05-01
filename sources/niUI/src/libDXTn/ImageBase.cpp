/*

  Image.cpp - Generic 24 bit image class functions

*/

#include "stdafx.h"
#include "ImageBase.h"
#include "CodeBook4MMX.h"
#include "fCodeBook.h"
#include "Lloyd.h"
#include "MedianCut.h"

QuantMethodType Image::QuantMethod = QM_MedianCut;
bool Image::QuantDiffusion = false;

Image::Image()
{
  pPixels = NULL;
  XSize = YSize = 0;
}

Image::~Image()
{
  ReleaseAll();
}

double Image::Diff(Image *pComp)
{
  double Result;

  if(GetXSize() != pComp->GetXSize() || GetYSize() != pComp->GetYSize())
    return 1000000000000000.0;  // BIG number

  Result = 0;

  Image *pImg1, *pImg2;
  ni::tU8 *pSrc1, *pSrc2;
  ni::tU32  Count, TempResult = 0;

  pImg1 = (Image *)this;
  pSrc1 = (ni::tU8 *)pImg1->GetPixels();
  pImg2 = (Image *)pComp;
  pSrc2 = (ni::tU8 *)pImg2->GetPixels();

  Count = XSize * YSize;

#if defined USE_CODEBOOKMMX
  ni::tU32 Count2;
  while(Count)
  {
    Count2 = Count;
    if(Count2 > 0x7fffff)
      Count2 = 0x7fffff;

    Count -= Count2;

    __asm {
      mov     eax, dword ptr pSrc1;
      mov     ebx, dword ptr pSrc2;
      mov     ecx, [Count2]
          $Loop:
          movd    mm0, [eax]
          movd    mm1, [ebx]
          pxor    mm2, mm2

          punpcklbw mm0, mm2
          punpcklbw mm1, mm2

          add     eax, 4
          psubsw    mm0, mm1

          add     ebx, 4
          pmaddwd   mm0, mm0

          movq    mm1, mm0
          psrlq   mm0, 32
          paddd   mm0, mm1
          movd    edx, mm0

          add     [TempResult], edx

          dec     ecx
          jg      $Loop
          emms
          }

    Result += TempResult;
    TempResult = 0;
  }
#else
  for (tU32 i = 0; i < Count; ++i) {
    double Val = (double)abs((long)pSrc1[i] - (long)pSrc2[i]);
    Val *= Val;
    Result += Val;
  }
#endif

  return Result;
}

double Image::MSE(Image *pComp)
{
  double diff;

  diff = Diff(pComp);
  diff /= (double)(XSize * YSize * 4);
  return diff;
}

AlphaType Image::AlphaUsage(unsigned char *pAlpha1, unsigned char *pAlpha0)
{
  long  x, y;
  long  Usage[256];
  long  Unique;
  Color *pPix;

  memset(Usage, 0, sizeof(Usage));    // Reset the usage array

  // Count all the different values used
  pPix = ((Image *)this)->GetPixels();
  for(y=0; y<YSize; y++)
  {
    for(x=0; x<XSize; x++)
      Usage[pPix[x].a]++;

    pPix += XSize;
  }


  // Count the number of unique alpha values
  Unique = 0;
  for(x=0; x<256; x++)
    Unique += (Usage[x] != 0);

  if(pAlpha1) *pAlpha1 = 0xff;
  if(pAlpha0) *pAlpha0 = 0x00;

  // Based on the unique alphas, classify the image
  switch(Unique)
  {
    case 0: // Only possible if the texture is of size 0 x 0
      return AT_None;

    case 1:
      if(Usage[0xff])
        return AT_None;
      else
      {
        if(pAlpha1)
        {
          *pAlpha1 = ((Image *)this)->GetPixels()[0].a;
        }
        return AT_Constant;
      }

    case 2:
      if(Usage[0] && Usage[0xff])
        return AT_Binary;
      else if(Usage[0])
      {
        if(pAlpha1) // Find the alpha value
        {
          for(x=1; x<256; x++)
            if(Usage[x])
              *pAlpha1 = (unsigned char)x;
        }
        return AT_ConstantBinary;
      }
      else
      {
        if(pAlpha0 && pAlpha1)
        {
          x = 0;
          while(x<256 && Usage[x] == 0)
            x++;
          *pAlpha0 = (unsigned char)x;

          x++;
          while(x<256 && Usage[x] == 0)
            x++;
          *pAlpha1 = (unsigned char)x;
        }
        return AT_DualConstant;
      }
      break;

    default:
      return AT_Modulated;
  }
}

void Image::AlphaToBinary(unsigned char Threshold)
{
  long  x, y;
  Color *pPix;
  unsigned char NewAlpha[256];

  memset(NewAlpha, 0, Threshold);
  memset(NewAlpha+Threshold, 255, 256-Threshold);

  pPix = ((Image *)this)->GetPixels();
  for(y=0; y<YSize; y++)
  {
    for(x=0; x<XSize; x++)
      pPix[x].a = NewAlpha[ pPix[x].a ];
    pPix += XSize;
  }
}

void Image::ReleaseAll(void)
{
  niSafeFree(pPixels);
  XSize = YSize = 0;
}

void Image::SetSize(long x, long y)
{
  ReleaseAll();

  XSize = x;
  YSize = y;
  if (x * y)
  {
    tU32 numCol = ni::Max(4,XSize)*ni::Max(4,YSize);
    pPixels = (Color*)niMalloc(sizeof(Color)*numCol);
    memset(pPixels,0,sizeof(Color)*numCol);
  }
}

long Image::UniqueColors(void)
{
  ImgCodeBook Codes;
  Codes.FromImageUnique(this);
  return Codes.NumCodes();
}

float Image::AverageSlope(void)
{
  long  x, y, YMinus1, XMinus1, XPlus1;
  Color *pPix;
  double  Slope = 0;
  long  r, g, b, s, Count;

  XMinus1 = XSize-1;
  XPlus1 = XSize+1;
  YMinus1 = YSize-1;

  pPix = pPixels;
  Count = 0;
  for(y=0; y<YMinus1; y++)
  {
    for(x=0; x<XMinus1; x++)
    {
      r = abs((long)pPix[x].r - (long)pPix[x+XPlus1].r);
      g = abs((long)pPix[x].g - (long)pPix[x+XPlus1].g);
      b = abs((long)pPix[x].b - (long)pPix[x+XPlus1].b);

      s = r+g+b;
      if(s)
      {
        Slope += s;
        Count++;
      }
    }
    pPix += XSize;
  }

  Slope /= (double)Count;
  return (float)Slope;
}

/*Image &Image::operator=(Image &Src)
  {
  Image *pSrc = (Image *)&Src;

  SetSize(pSrc->XSize, pSrc->YSize);
  memcpy(pPixels, pSrc->pPixels, XSize*YSize*4);
  return *this;
  }*/

typedef struct
{
  short r, g, b, a;
} ShortCol;

inline ni::tU8 Clamp(short Val)
{
  if(Val > 255) Val = 255;
  else if(Val < 0) Val = 0;
  return (ni::tU8)Val;
}

void Image::DiffuseError(long aBits, long rBits, long gBits, long bBits)
{
  long x, y;
  ShortCol *pTempPix, *pDest;
  short r, g, b, a, rErr, gErr, bErr, aErr;
  short rMask, gMask, bMask, aMask;
  Color *pSrc;

  rMask = (1 << (12-rBits)) - 1;
  gMask = (1 << (12-gBits)) - 1;
  bMask = (1 << (12-bBits)) - 1;
  aMask = (1 << (12-aBits)) - 1;

  pTempPix = (ShortCol*)niMalloc(sizeof(ShortCol) * XSize * YSize);
  pDest = pTempPix;
  pSrc = pPixels;

  for(y=0; y<YSize; y++)
  {
    for(x=0; x<XSize; x++)
    {
      r = pSrc[x].r;
      g = pSrc[x].g;
      b = pSrc[x].b;
      a = pSrc[x].a;
      pDest[x].r = r << 4;
      pDest[x].g = g << 4;
      pDest[x].b = b << 4;
      pDest[x].a = a << 4;
    }
    pSrc += XSize;
    pDest += XSize;
  }

  pDest = pTempPix;
  for(y=0; y<YSize-1; y++)
  {
    for(x=0; x<XSize-1; x++)
    {
      r = pDest[x].r;
      g = pDest[x].g;
      b = pDest[x].b;
      a = pDest[x].a;

      rErr = r - ((r + rMask/2) & ~rMask);
      gErr = g - ((g + gMask/2) & ~gMask);
      bErr = b - ((b + bMask/2) & ~bMask);
      aErr = a - ((a + aMask/2) & ~aMask);

      r -= rErr;
      g -= gErr;
      b -= bErr;
      a -= aErr;

      pDest[x].r = r;
      pDest[x].g = g;
      pDest[x].b = b;
      pDest[x].a = a;

      pDest[x+1].r += rErr / 2;
      pDest[x+1].g += gErr / 2;
      pDest[x+1].b += bErr / 2;
      pDest[x+1].a += aErr / 2;

      pDest[x+XSize].r += rErr / 4;
      pDest[x+XSize].g += gErr / 4;
      pDest[x+XSize].b += bErr / 4;
      pDest[x+XSize].a += aErr / 4;

      if(x)
      {
        pDest[x+XSize-1].r += rErr / 8;
        pDest[x+XSize-1].g += gErr / 8;
        pDest[x+XSize-1].b += bErr / 8;
        pDest[x+XSize-1].a += aErr / 8;

        if(x > 2)
        {
          pDest[x+XSize-3].r += rErr / 8;
          pDest[x+XSize-3].g += gErr / 8;
          pDest[x+XSize-3].b += bErr / 8;
          pDest[x+XSize-3].a += aErr / 8;
        }
      }
    }

    r = pDest[x].r;
    g = pDest[x].g;
    b = pDest[x].b;
    a = pDest[x].a;

    rErr = r - ((r + rMask/2) & ~rMask);
    gErr = g - ((g + gMask/2) & ~gMask);
    bErr = b - ((b + bMask/2) & ~bMask);
    aErr = a - ((a + aMask/2) & ~aMask);

    r -= rErr;
    g -= gErr;
    b -= bErr;
    a -= aErr;

    pDest[x].r = r;
    pDest[x].g = g;
    pDest[x].b = b;
    pDest[x].a = a;

    pDest += XSize;
  }

  for(x=0; x<XSize; x++)
  {
    r = pDest[x].r;
    g = pDest[x].g;
    b = pDest[x].b;
    a = pDest[x].a;

    rErr = r - ((r + rMask/2) & ~rMask);
    gErr = g - ((g + gMask/2) & ~gMask);
    bErr = b - ((b + bMask/2) & ~bMask);
    aErr = a - ((a + aMask/2) & ~aMask);

    r -= rErr;
    g -= gErr;
    b -= bErr;
    a -= aErr;

    pDest[x].r = r;
    pDest[x].g = g;
    pDest[x].b = b;
    pDest[x].a = a;
  }

  pDest = pTempPix;
  pSrc = pPixels;
  rMask >>= 4;
  gMask >>= 4;
  bMask >>= 4;
  aMask >>= 4;
  for(y=0; y<YSize; y++)
  {
    for(x=0; x<XSize; x++)
    {
      pSrc[x].r = Clamp(pDest[x].r >> 4) & ~rMask;
      pSrc[x].g = Clamp(pDest[x].g >> 4) & ~gMask;
      pSrc[x].b = Clamp(pDest[x].b >> 4) & ~bMask;
      pSrc[x].a = Clamp(pDest[x].a >> 4) & ~aMask;
    }
    pSrc += XSize;
    pDest += XSize;
  }
  niFree(pTempPix);
}

bool Image::Crop(long x1, long y1, long x2, long y2)
{
  Color *pNewPix, *pSrc, *pDest;
  long NewX, NewY, x, y;

  if(x2 < x1 || y2 < y1)
    return false;

  NewX = x2 - x1 + 1;
  NewY = y2 - y1 + 1;

  pNewPix = (Color*)niMalloc(sizeof(Color)*NewX * NewY);
  pSrc = pPixels + y1 * XSize;
  pDest = pNewPix;
  for(y=y1; y<y2; y++)
  {
    for(x=x1; x<x2; x++)
      *pDest++ = pSrc[x];

    pSrc += XSize;
  }
  niFree(pPixels);
  pPixels = pNewPix;
  XSize = NewX;
  YSize = NewY;
  return true;
}

bool Image::SizeCanvas(long NewX, long NewY)
{
  Color *pNewPix, *pSrc, *pDest;
  long x, y;
  long XRun, YRun;

  pNewPix = (Color*)niMalloc(sizeof(Color) * NewX * NewY);
  memset(pNewPix, 0, NewX * NewY * sizeof(Color));

  XRun = ni::Min(XSize, NewX);
  YRun = ni::Min(YSize, NewY);

  pSrc = pPixels;
  pDest = pNewPix;
  for(y=0; y<YRun; y++)
  {
    for(x=0; x<XRun; x++)
      pDest[x] = pSrc[x];

    pDest += NewX;
    pSrc += XSize;
  }
  niFree(pPixels);
  pPixels = pNewPix;
  XSize = NewX;
  YSize = NewY;
  return true;
}

bool Image::Quarter(Image &Dest)
{
  long  x, y, NewX, NewY;
  Color *pSrcPix, *pDestPix;
  long  r, g, b, a;

  if((XSize | YSize) & 1)   // Not an even size - Can't quarter it
    return false;

  NewX = XSize / 2;
  NewY = YSize / 2;

  Dest.SetSize(NewX, NewY);
  pSrcPix = GetPixels();
  pDestPix = Dest.GetPixels();

  for(y=0; y<NewY; y++)
  {
    for(x=0; x<NewX; x++)
    {
      r = (long)pSrcPix[0].r + (long)pSrcPix[1].r + (long)pSrcPix[XSize].r + (long)pSrcPix[XSize+1].r;
      g = (long)pSrcPix[0].g + (long)pSrcPix[1].g + (long)pSrcPix[XSize].g + (long)pSrcPix[XSize+1].g;
      b = (long)pSrcPix[0].b + (long)pSrcPix[1].b + (long)pSrcPix[XSize].b + (long)pSrcPix[XSize+1].b;
      a = (long)pSrcPix[0].a + (long)pSrcPix[1].a + (long)pSrcPix[XSize].a + (long)pSrcPix[XSize+1].a;

      pDestPix[x].r = (unsigned char)((r+3) >> 2);
      pDestPix[x].g = (unsigned char)((g+3) >> 2);
      pDestPix[x].b = (unsigned char)((b+3) >> 2);
      pDestPix[x].a = (unsigned char)((a+3) >> 2);

      pSrcPix += 2;
    }
    pSrcPix += XSize;
    pDestPix += NewX;
  }
  return true;
}

bool Image::HalfX(Image &Dest)
{
  long  x, y, NewX;
  Color *pSrcPix, *pDestPix;
  long  r, g, b, a;

  if(XSize & 1)   // Not an even size - Can't half it
    return false;

  NewX = XSize / 2;

  Dest.SetSize(NewX, YSize);
  pSrcPix = GetPixels();
  pDestPix = Dest.GetPixels();

  for(y=0; y<YSize; y++)
  {
    for(x=0; x<NewX; x++)
    {
      r = (long)pSrcPix[0].r + (long)pSrcPix[1].r;
      g = (long)pSrcPix[0].g + (long)pSrcPix[1].g;
      b = (long)pSrcPix[0].b + (long)pSrcPix[1].b;
      a = (long)pSrcPix[0].a + (long)pSrcPix[1].a;

      pDestPix[x].r = (unsigned char)((r+1) >> 1);
      pDestPix[x].g = (unsigned char)((g+1) >> 1);
      pDestPix[x].b = (unsigned char)((b+1) >> 1);
      pDestPix[x].a = (unsigned char)((a+1) >> 1);

      pSrcPix += 2;
    }
    pDestPix += NewX;
  }
  return true;
}

bool Image::HalfY(Image &Dest)
{
  long  x, y, NewY;
  Color *pSrcPix, *pDestPix;
  long  r, g, b, a;

  if(YSize & 1)   // Not an even size - Can't half it
    return false;

  NewY = YSize / 2;

  Dest.SetSize(XSize, NewY);
  pSrcPix = GetPixels();
  pDestPix = Dest.GetPixels();

  for(y=0; y<NewY; y++)
  {
    for(x=0; x<XSize; x++)
    {
      r = (long)pSrcPix[0].r + (long)pSrcPix[XSize].r;
      g = (long)pSrcPix[0].g + (long)pSrcPix[XSize].g;
      b = (long)pSrcPix[0].b + (long)pSrcPix[XSize].b;
      a = (long)pSrcPix[0].a + (long)pSrcPix[XSize].a;

      pDestPix[x].r = (unsigned char)((r+1) >> 1);
      pDestPix[x].g = (unsigned char)((g+1) >> 1);
      pDestPix[x].b = (unsigned char)((b+1) >> 1);
      pDestPix[x].a = (unsigned char)((a+1) >> 1);

      pSrcPix++;
    }
    pSrcPix += XSize;
    pDestPix += XSize;
  }
  return true;
}


void Image::ResizeX(Image &Dest, long NewX)
{
  if(NewX < XSize)
    ScaleDownX(Dest, NewX);
  else if(NewX > XSize)
    ScaleUpX(Dest, NewX);
  else
    Dest = *this;
}

void Image::ScaleUpX(Image &Dest, long NewX)
{
  float XPos, XStep;
  float Scale, r, g, b, a;
  long x, y, xs;
  Color *pPix, *pDest;

  XStep = (float)XSize / (float)NewX;
  Dest.SetSize( NewX, YSize );

  pPix = pPixels;
  pDest = Dest.pPixels;

  for(y=0; y<YSize; y++)
  {
    XPos = 0;
    for(x=0; x<NewX; x++)
    {
      xs = (long)XPos;
      Scale = XPos - (float)xs;
      if((Scale > 0.001f) && ((xs+1) < XSize))
      {
        a = (float)pPix[xs].a * (1.0f-Scale) + (float)pPix[xs+1].a * Scale;
        r = (float)pPix[xs].r * (1.0f-Scale) + (float)pPix[xs+1].r * Scale;
        g = (float)pPix[xs].g * (1.0f-Scale) + (float)pPix[xs+1].g * Scale;
        b = (float)pPix[xs].b * (1.0f-Scale) + (float)pPix[xs+1].b * Scale;

        pDest[x].a = (ni::tU8)FtoL(a);
        pDest[x].r = (ni::tU8)FtoL(r);
        pDest[x].g = (ni::tU8)FtoL(g);
        pDest[x].b = (ni::tU8)FtoL(b);
      }
      else
        pDest[x] = pPix[xs];

      XPos += XStep;
    }
    pDest += NewX;
    pPix += XSize;
  }
}

void Image::ScaleDownX(Image &Dest, long NewX)
{
  float r, g, b, a;
  float XStart, XEnd, XStep, InvXStep, Scale;
  long x, y, xs, xe;
  Color *pPix, *pDest;

  Dest.SetSize( NewX, YSize );
  pPix = pPixels;
  pDest = Dest.pPixels;

  XStep = (float)XSize / (float)NewX;
  InvXStep = 1.0f / XStep;

  for(y=0; y<YSize; y++)
  {
    XStart = 0.0f;
    XEnd = XStart + XStep;
    if(XEnd > XSize)
      XEnd = (float)XSize;

    for(x=0; x<NewX; x++)
    {
      xs = (long)XStart;
      Scale = 1.0f - (XStart - (float)xs);

      a = (float)(pPix[xs].a) * Scale;
      r = (float)(pPix[xs].r) * Scale;
      g = (float)(pPix[xs].g) * Scale;
      b = (float)(pPix[xs].b) * Scale;
      xs++;

      xe = (long)XEnd;
      while(xs < xe)
      {
        a += (float)pPix[xs].a;
        r += (float)pPix[xs].r;
        g += (float)pPix[xs].g;
        b += (float)pPix[xs].b;
        xs++;
      }

      Scale = XEnd - (float)xe;
      if(Scale > 0.001f)
      {
        a += (float)(pPix[xs].a) * Scale;
        r += (float)(pPix[xs].r) * Scale;
        g += (float)(pPix[xs].g) * Scale;
        b += (float)(pPix[xs].b) * Scale;
      }

      a *= InvXStep;
      r *= InvXStep;
      g *= InvXStep;
      b *= InvXStep;

      pDest[x].a = (ni::tU8)FtoL(a);
      pDest[x].r = (ni::tU8)FtoL(r);
      pDest[x].g = (ni::tU8)FtoL(g);
      pDest[x].b = (ni::tU8)FtoL(b);

      XStart += XStep;
      XEnd += XStep;
    }
    pPix += XSize;
    pDest += NewX;
  }
}

void Image::ResizeY(Image &Dest, long NewY)
{
  if(NewY < YSize)
    ScaleDownY(Dest, NewY);
  else if(NewY > YSize)
    ScaleUpY(Dest, NewY);
  else
    Dest = *this;
}

void Image::ScaleUpY(Image &Dest, long NewY)
{
  float YPos, YStep;
  float Scale, r, g, b, a;
  long x, y, ys;
  Color *pPix, *pPix2, *pDest;

  YStep = (float)YSize / (float)NewY;
  Dest.SetSize( XSize, NewY );

  pDest = Dest.pPixels;

  YPos = 0;
  for(y=0; y<NewY; y++)
  {
    ys = (long)YPos;
    pPix = pPixels + XSize * ys;
    pPix2 = pPix + XSize;
    Scale = YPos - (float)ys;

    if(Scale > 0.001f && (ys+1) < YSize)
    {
      for(x=0; x<XSize; x++)
      {
        a = (float)pPix[x].a * (1.0f-Scale) + (float)pPix2[x].a * Scale;
        r = (float)pPix[x].r * (1.0f-Scale) + (float)pPix2[x].r * Scale;
        g = (float)pPix[x].g * (1.0f-Scale) + (float)pPix2[x].g * Scale;
        b = (float)pPix[x].b * (1.0f-Scale) + (float)pPix2[x].b * Scale;

        pDest[x].a = (ni::tU8)FtoL(a);
        pDest[x].r = (ni::tU8)FtoL(r);
        pDest[x].g = (ni::tU8)FtoL(g);
        pDest[x].b = (ni::tU8)FtoL(b);
      }
    }
    else
    {
      for(x=0; x<XSize; x++)
        pDest[x] = pPix[x];
    }
    YPos += YStep;
    pDest += XSize;
  }
}

void Image::ScaleDownY(Image &Dest, long NewY)
{
  float r, g, b, a;
  float YStart, YEnd, YStep, InvYStep, Scale;
  long x, y, ys, ye;
  Color *pPix, *pPixStart, *pDest;

  Dest.SetSize(XSize, NewY);
  pDest = Dest.pPixels;

  YStep = (float)YSize / (float)NewY;
  InvYStep = 1.0f / YStep;

  YStart = 0.0f;
  YEnd = YStart + YStep;
  for(y=0; y<NewY; y++)
  {
    pPixStart = pPixels + (long)YStart * XSize;
    for(x=0; x<XSize; x++)
    {
      ys = (long)YStart;
      pPix = pPixStart + x;
      Scale = 1.0f - (YStart - (float)ys);

      a = (float)(pPix->a) * Scale;
      r = (float)(pPix->r) * Scale;
      g = (float)(pPix->g) * Scale;
      b = (float)(pPix->b) * Scale;

      ys++;
      pPix += XSize;

      ye = (long)YEnd;
      while(ys < ye)
      {
        a += (float)pPix->a;
        r += (float)pPix->r;
        g += (float)pPix->g;
        b += (float)pPix->b;

        ys++;
        pPix += XSize;
      }

      Scale = YEnd - (float)ye;
      if(Scale > 0.001f)
      {
        a += (float)(pPix->a) * Scale;
        r += (float)(pPix->r) * Scale;
        g += (float)(pPix->g) * Scale;
        b += (float)(pPix->b) * Scale;
      }

      a *= InvYStep;
      r *= InvYStep;
      g *= InvYStep;
      b *= InvYStep;

      pDest[x].a = (ni::tU8)FtoL(a);
      pDest[x].r = (ni::tU8)FtoL(r);
      pDest[x].g = (ni::tU8)FtoL(g);
      pDest[x].b = (ni::tU8)FtoL(b);
    }

    pDest += XSize;
    YStart += YStep;
    YEnd += YStep;
    if(YEnd > YSize)
      YEnd = (float)YSize;
  }
}
