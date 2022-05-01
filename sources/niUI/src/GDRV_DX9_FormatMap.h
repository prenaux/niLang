#ifndef __D3DFORMATMAP_35928060_H__
#define __D3DFORMATMAP_35928060_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "GDRV_DX9_ModeSelector.h"

class cD3DFormatMap
{
  struct sD3DFormat
  {
    D3DFORMAT format;
    tBool   bHasAlpha;
    sD3DFormat(D3DFORMAT aFormat = D3DFMT_UNKNOWN, tBool abHasAlpha = eFalse)
        : format(aFormat), bHasAlpha(abHasAlpha) {}
  };
  typedef astl::hash_map<cString,sD3DFormat>  tD3DFormatMap;
  typedef tD3DFormatMap::iterator       tD3DFormatMapIt;
  typedef tD3DFormatMap::const_iterator   tD3DFormatMapCIt;

 public:
  cD3DFormatMap();
  ~cD3DFormatMap();

  D3DFORMAT GetD3DFormat(const achar* szFormat, tBool* apbHasAlpha = NULL) const;
  const achar* GetPixelFormat(D3DFORMAT fmt, tBool* apbHasAlpha = NULL, const achar* aaszErr = NULL) const;
  D3DFORMAT GetNearestD3DFormat(iPixelFormat* pPxf, tBool* apbHasAlpha = NULL) const;
  D3DFORMAT GetNearestD3DFormat(iGraphics* apGraphics, const achar* aaszFormat, tBool* apbHasAlpha = NULL) const;
  D3DFORMAT GetD3DFormatColor(tU32 anColorBits, tU32 anAlphaBits) const;
  D3DFORMAT GetD3DFormatDepthStencil(tU32 anDepthBits, tU32 anStencilBits) const;
  tBool GetColorBits(D3DFORMAT aFmt, tU32& anColorBits, tU32& anAlphaBits) const;
  tBool GetDepthStencilBits(D3DFORMAT aFmt, tU32& anDepthBits, tU32& anStencilBits) const;

 private:
  tD3DFormatMap mmapD3DFormats;
};

const cD3DFormatMap* GetD3DFormatMap();

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __D3DFORMATMAP_35928060_H__
