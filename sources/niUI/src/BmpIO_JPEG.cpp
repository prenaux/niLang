// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "API/niUI/IJpegReader.h"
#include "API/niUI/IJpegWriter.h"

static const tU8 _kAirplayHeader[] = { 0xFF, 0xD8, 0xFF, 0xDB, 0x00, 0x84 };

///////////////////////////////////////////////
struct BitmapLoader_JPEG : public cIUnknownImpl<iBitmapLoader> {
  iBitmapBase* __stdcall LoadBitmap(iGraphics* apGraphics, iFile* pFile) niImpl
  {
    QPtr<iJpegReader> ptrReader = niCreateInstance(niUI,JpegReader,pFile,0);
    if (!niIsOK(ptrReader)) {
      niError(_A("Can't open the jpeg reader."));
      return NULL;
    }
    return ptrReader->ReadBitmap(apGraphics);
  }
};

///////////////////////////////////////////////
struct BitmapSaver_JPEG : public cIUnknownImpl<iBitmapSaver> {
  tBool __stdcall SaveBitmap(iGraphics* apGraphics, iFile* pDest, iBitmapBase* pBmpBase, tU32 ulCompression) niImpl
  {
    QPtr<iJpegWriter> ptrWriter = niCreateInstance(niUI,JpegWriter,0,0);
    if (!ptrWriter.IsOK()) {
      niError("Can't create the jpeg writer.");
      return eFalse;
    }

    QPtr<iBitmap2D> ptrBmp = pBmpBase;
    if (!ptrBmp.IsOK()) {
      niError("Can't query iBitmap2D interface.");
      return eFalse;
    }

    if (ni::StrCmp(ptrBmp->GetPixelFormat()->GetFormat(),_A("R8G8B8")) != 0) {
      ptrBmp = static_cast<iBitmap2D*>(ptrBmp->CreateConvertedFormat(apGraphics->CreatePixelFormat(_A("R8G8B8"))));
      if (!niIsOK(ptrBmp)) {
        niError(_A("Can't convert bitmap to R8G8B8 format."));
        return eFalse;
      }
    }

    return ptrWriter->WriteBitmap(pDest,ptrBmp,100-ulCompression,eJpegWriteFlags_None);
  }
};

///////////////////////////////////////////////
niExportFunc(iUnknown*) New_BitmapLoader_jpeg(const Var&,const Var&) {
  return niNew BitmapLoader_JPEG();
}
niExportFunc(iUnknown*) New_BitmapSaver_jpeg(const Var&,const Var&) {
  return niNew BitmapSaver_JPEG();
}
