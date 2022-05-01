// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#ifdef GDRV_DX9

#include "GDRV_DX9_FormatMap.h"

const cD3DFormatMap* GetD3DFormatMap() {
  static cD3DFormatMap _D3DFormatMap;
  return &_D3DFormatMap;
}

///////////////////////////////////////////////
cD3DFormatMap::cD3DFormatMap()
{
  mmapD3DFormats[_A("B8G8R8")] =      sD3DFormat(D3DFMT_R8G8B8,eFalse);
  mmapD3DFormats[_A("B8G8R8A8")] =    sD3DFormat(D3DFMT_A8R8G8B8,eTrue);
  mmapD3DFormats[_A("B8G8R8X8")] =    sD3DFormat(D3DFMT_X8R8G8B8,eFalse);
  mmapD3DFormats[_A("B5G6R5")] =      sD3DFormat(D3DFMT_R5G6B5,eFalse);
  mmapD3DFormats[_A("B5G5R5X1")] =    sD3DFormat(D3DFMT_X1R5G5B5,eFalse);
  mmapD3DFormats[_A("B5G5R5A1")] =    sD3DFormat(D3DFMT_A1R5G5B5,eTrue);
  mmapD3DFormats[_A("B4G4R4A4")] =    sD3DFormat(D3DFMT_A4R4G4B4,eTrue);
  mmapD3DFormats[_A("B3G3R2")] =      sD3DFormat(D3DFMT_R3G3B2,eFalse);
  mmapD3DFormats[_A("A8")] =        sD3DFormat(D3DFMT_A8,eTrue);
  mmapD3DFormats[_A("B3G3R2A8")] =    sD3DFormat(D3DFMT_A8R3G3B2,eTrue);
  mmapD3DFormats[_A("B4G4R4X4")] =    sD3DFormat(D3DFMT_X4R4G4B4,eFalse);
  mmapD3DFormats[_A("R10G10B10A2")] =   sD3DFormat(D3DFMT_A2B10G10R10,eTrue);
  mmapD3DFormats[_A("R8G8B8A8")] =    sD3DFormat(D3DFMT_A8B8G8R8,eTrue);
  mmapD3DFormats[_A("R8G8B8X8")] =    sD3DFormat(D3DFMT_X8B8G8R8,eFalse);
  mmapD3DFormats[_A("R16G16")] =      sD3DFormat(D3DFMT_G16R16,eFalse);
  mmapD3DFormats[_A("B10G10R10A2")] =   sD3DFormat(D3DFMT_A2R10G10B10,eTrue);
  mmapD3DFormats[_A("R16G16B16A16")] =  sD3DFormat(D3DFMT_A16B16G16R16,eTrue);
  mmapD3DFormats[_A("L8")] =        sD3DFormat(D3DFMT_L8,eFalse);
  mmapD3DFormats[_A("L16")] =       sD3DFormat(D3DFMT_L16,eFalse);
  mmapD3DFormats[_A("L8A8")] =      sD3DFormat(D3DFMT_A8L8,eTrue);
  mmapD3DFormats[_A("L4A4")] =      sD3DFormat(D3DFMT_A4L4,eTrue);
  mmapD3DFormats[_A("U8V8")] =      sD3DFormat(D3DFMT_V8U8,eFalse);
  mmapD3DFormats[_A("U8V8W8Q8")] =    sD3DFormat(D3DFMT_Q8W8V8U8,eFalse);
  mmapD3DFormats[_A("U16V16")] =      sD3DFormat(D3DFMT_V16U16,eFalse);
  mmapD3DFormats[_A("U16V16W16Q16")] =  sD3DFormat(D3DFMT_Q16W16V16U16,eFalse);
  mmapD3DFormats[_A("U6V5L5")] =      sD3DFormat(D3DFMT_L6V5U5,eFalse);
  mmapD3DFormats[_A("U8V8L8X8")] =    sD3DFormat(D3DFMT_X8L8V8U8,eFalse);
  mmapD3DFormats[_A("U10V10W10A2")] =   sD3DFormat(D3DFMT_A2W10V10U10,eTrue);
  mmapD3DFormats[_A("DXT1")] =      sD3DFormat(D3DFMT_DXT1,eFalse);
  //mmapD3DFormats[_A("DXT1A")] =     sD3DFormat(D3DFMT_DXT1,eTrue);
  mmapD3DFormats[_A("DXT2")] =      sD3DFormat(D3DFMT_DXT2,eTrue);
  mmapD3DFormats[_A("DXT3")] =      sD3DFormat(D3DFMT_DXT3,eTrue);
  mmapD3DFormats[_A("DXT4")] =      sD3DFormat(D3DFMT_DXT4,eTrue);
  mmapD3DFormats[_A("DXT5")] =      sD3DFormat(D3DFMT_DXT5,eTrue);
  mmapD3DFormats[_A("FR16")] =      sD3DFormat(D3DFMT_R16F,eFalse);
  mmapD3DFormats[_A("FR16G16")] =     sD3DFormat(D3DFMT_G16R16F,eFalse);
  mmapD3DFormats[_A("FR16G16B16A16")] = sD3DFormat(D3DFMT_A16B16G16R16F,eTrue);
  mmapD3DFormats[_A("FR32")] =      sD3DFormat(D3DFMT_R32F,eFalse);
  mmapD3DFormats[_A("FR32G32")] =     sD3DFormat(D3DFMT_G32R32F,eFalse);
  mmapD3DFormats[_A("FR32G32B32A32")] = sD3DFormat(D3DFMT_A32B32G32R32F,eTrue);
  mmapD3DFormats[_A("D16")] =       sD3DFormat(D3DFMT_D16,eFalse);
#pragma niNote("FD16 is not a D3D format, but it's a good way to remove the ambiguity with lockable target, as there isnt any D16F format.")
  mmapD3DFormats[_A("FD16")] =      sD3DFormat(D3DFMT_D16_LOCKABLE,eFalse);
  mmapD3DFormats[_A("D15S1")] =     sD3DFormat(D3DFMT_D15S1,eFalse);
  mmapD3DFormats[_A("D24S8")] =     sD3DFormat(D3DFMT_D24S8,eFalse);
  mmapD3DFormats[_A("FD24S8")] =      sD3DFormat(D3DFMT_D24FS8,eFalse);
  mmapD3DFormats[_A("D24X8")] =     sD3DFormat(D3DFMT_D24X8,eFalse);
  mmapD3DFormats[_A("D24B4S4")] =     sD3DFormat(D3DFMT_D24X4S4,eFalse);
  mmapD3DFormats[_A("D32")] =       sD3DFormat(D3DFMT_D32,eFalse);
  mmapD3DFormats[_A("FD32")] =      sD3DFormat(D3DFMT_D32F_LOCKABLE,eFalse);
}

///////////////////////////////////////////////
cD3DFormatMap::~cD3DFormatMap()
{
}

///////////////////////////////////////////////
D3DFORMAT cD3DFormatMap::GetD3DFormat(const achar* szFormat, tBool* apbHasAlpha) const
{
  tD3DFormatMapCIt it = mmapD3DFormats.find(cString(szFormat).ToUpper());
  if (it == mmapD3DFormats.end())
    return D3DFMT_UNKNOWN;
  if (apbHasAlpha) *apbHasAlpha = it->second.bHasAlpha;
  return it->second.format;
}

///////////////////////////////////////////////
const achar* cD3DFormatMap::GetPixelFormat(D3DFORMAT fmt, tBool* apbHasAlpha, const achar* aaszErr) const
{
  for (tD3DFormatMapCIt it = mmapD3DFormats.begin(); it != mmapD3DFormats.end(); ++it)
  {
    if (it->second.format == fmt)
    {
      if (apbHasAlpha) *apbHasAlpha = it->second.bHasAlpha;
      return it->first.Chars();
    }
  }
  return aaszErr;
}

///////////////////////////////////////////////
D3DFORMAT cD3DFormatMap::GetNearestD3DFormat(iPixelFormat* pPxf, tBool* apbHasAlpha) const
{
  D3DFORMAT D3DFormat = GetD3DFormat(pPxf->GetFormat(),apbHasAlpha);
  if (D3DFormat == D3DFMT_UNKNOWN)
  { // Get the D3D Format that map the format of the given bitmap the best
    // if there's no one format that map it one on one.
    if (pPxf->GetNumABits()) {
      if (apbHasAlpha) *apbHasAlpha = eTrue;
      if (!pPxf->GetNumRBits() &&
          !pPxf->GetNumGBits() &&
          !pPxf->GetNumBBits())
      { // Only an alpha channel
        D3DFormat = D3DFMT_A8;
      }
      else
      {
        if (pPxf->GetBitsPerPixel() <= 24)
        {
          if (pPxf->GetNumABits() >= pPxf->GetBitsPerPixel()/4)
            D3DFormat = D3DFMT_A4R4G4B4;
          else
            D3DFormat = D3DFMT_A1R5G5B5;
        }
        else
        {
          D3DFormat = D3DFMT_A8R8G8B8;
        }
      }
    }
    else
    {
      if (apbHasAlpha) *apbHasAlpha = eFalse;
      if (pPxf->GetCaps()&ePixelFormatCaps_Signed)
      {
        if (pPxf->GetBitsPerPixel() < 24)
          D3DFormat = D3DFMT_V8U8;
        else if (pPxf->GetBitsPerPixel() == 24)
          D3DFormat = D3DFMT_R8G8B8;
        else
          D3DFormat = D3DFMT_V16U16;
      }
      else
      {
        if (pPxf->GetBitsPerPixel() < 24)
          D3DFormat = D3DFMT_R5G6B5;
        else if (pPxf->GetBitsPerPixel() == 24)
          D3DFormat = D3DFMT_R8G8B8;
        else
          D3DFormat = D3DFMT_X8R8G8B8;
      }
    }
  }

  return D3DFormat;
}

///////////////////////////////////////////////
D3DFORMAT cD3DFormatMap::GetNearestD3DFormat(iGraphics* apGraphics, const achar* aaszFormat, tBool* apbHasAlpha) const
{
  D3DFORMAT D3DFormat = GetD3DFormat(aaszFormat,apbHasAlpha);
  if (D3DFormat == D3DFMT_UNKNOWN)
  { // Get the D3D Format that map the format of the given bitmap the best
    // if there's no one format that map it one on one.
    Ptr<iPixelFormat> pPxf = apGraphics->CreatePixelFormat(aaszFormat);
    if (!pPxf.IsOK()) return D3DFMT_UNKNOWN;

    if (ni::StrICmp(pPxf->GetFormat(),_A("NMAP")) == 0)
    {
#pragma niTodo("Use a real Normal Map format.")
      D3DFormat = D3DFMT_R8G8B8;
      if (apbHasAlpha) *apbHasAlpha = eFalse;
    }
    else if (pPxf->GetNumABits())
    {
      if (apbHasAlpha) *apbHasAlpha = eTrue;
      if (!pPxf->GetNumRBits() &&
          !pPxf->GetNumGBits() &&
          !pPxf->GetNumBBits())
      { // Only an alpha channel
        D3DFormat = D3DFMT_A8;
      }
      else
      {
        if (pPxf->GetBitsPerPixel() <= 24)
        {
          if (pPxf->GetNumABits() >= pPxf->GetBitsPerPixel()/4)
            D3DFormat = D3DFMT_A4R4G4B4;
          else
            D3DFormat = D3DFMT_A1R5G5B5;
        }
        else
        {
          D3DFormat = D3DFMT_A8R8G8B8;
        }
      }
    }
    else
    {
      if (apbHasAlpha) *apbHasAlpha = eFalse;
      if (pPxf->GetCaps()&ePixelFormatCaps_Signed)
      {
        if (pPxf->GetBitsPerPixel() < 24)
          D3DFormat = D3DFMT_V8U8;
        else if (pPxf->GetBitsPerPixel() == 24)
          D3DFormat = D3DFMT_R8G8B8;
        else
          D3DFormat = D3DFMT_V16U16;
      }
      else
      {
        if (pPxf->GetBitsPerPixel() < 24)
          D3DFormat = D3DFMT_R5G6B5;
        else if (pPxf->GetBitsPerPixel() == 24)
          D3DFormat = D3DFMT_R8G8B8;
        else
          D3DFormat = D3DFMT_X8R8G8B8;
      }
    }
  }

  return D3DFormat;
}

///////////////////////////////////////////////
D3DFORMAT cD3DFormatMap::GetD3DFormatColor(tU32 anColorBits, tU32 anAlphaBits) const
{
  if (anColorBits == 15 && anAlphaBits == 1)  return D3DFMT_A1R5G5B5;
  if (anColorBits == 30 && anAlphaBits == 2)  return D3DFMT_A2R10G10B10;
  if (anColorBits == 24 && anAlphaBits == 8)  return D3DFMT_A8R8G8B8;
  if (anColorBits == 16 && anAlphaBits == 0)  return D3DFMT_R5G6B5;
  if (anColorBits == 15 && anAlphaBits == 0)  return D3DFMT_X1R5G5B5;
  if (anColorBits == 24 && anAlphaBits == 0)  return D3DFMT_X8R8G8B8;
  return D3DFMT_UNKNOWN;
}

///////////////////////////////////////////////
D3DFORMAT cD3DFormatMap::GetD3DFormatDepthStencil(tU32 anDepthBits, tU32 anStencilBits) const
{
  if (anDepthBits == 16 && anStencilBits == 0)  return D3DFMT_D16;
  if (anDepthBits == 32 && anStencilBits == 0)  return D3DFMT_D32;
  if (anDepthBits == 24 && anStencilBits == 8)  return D3DFMT_D24S8;
  if (anDepthBits == 24 && anStencilBits == 4)  return D3DFMT_D24X4S4;
  if (anDepthBits == 24 && anStencilBits == 0)  return D3DFMT_D24X8;
  return D3DFMT_UNKNOWN;
}

///////////////////////////////////////////////
tBool cD3DFormatMap::GetColorBits(D3DFORMAT aFmt, tU32& anColorBits, tU32& anAlphaBits) const
{
  switch (aFmt) {
    case D3DFMT_A2R10G10B10:  anColorBits = 30; anAlphaBits = 2; break;
    case D3DFMT_A8R8G8B8:   anColorBits = 24; anAlphaBits = 8; break;
    case D3DFMT_X8R8G8B8:   anColorBits = 24; anAlphaBits = 0; break;
    case D3DFMT_R5G6B5:     anColorBits = 16; anAlphaBits = 0; break;
    case D3DFMT_X1R5G5B5:   anColorBits = 15; anAlphaBits = 1; break;
    default:  return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool cD3DFormatMap::GetDepthStencilBits(D3DFORMAT aFmt, tU32& anDepthBits, tU32& anStencilBits) const
{
  switch (aFmt) {
    case D3DFMT_D16:      anDepthBits = 16; anStencilBits = 0;  break;
    case D3DFMT_D32:      anDepthBits = 32; anStencilBits = 0;  break;
    case D3DFMT_D24S8:      anDepthBits = 24; anStencilBits = 8;  break;
    case D3DFMT_D24X8:      anDepthBits = 24; anStencilBits = 0;  break;
    case D3DFMT_D24X4S4:    anDepthBits = 24; anStencilBits = 4;  break;
    default:  return eFalse;
  }
  return eTrue;
}

#endif
