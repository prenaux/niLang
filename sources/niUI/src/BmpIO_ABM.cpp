// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "API/niUI/IJpegReader.h"
#include "API/niUI/IJpegWriter.h"
#include <niLang/IZip.h>
#include "BmpIO_ABM.h"

struct BitmapLoader_ABM : public ImplRC<iBitmapLoader> {

  ///////////////////////////////////////////////
  tBool _ReadBitmap(iGraphics* apGraphics, iBitmap2D* apOut, iFile* apFile, iPixelFormat* apStoredFormat) {
    tU32 bmpType = apFile->ReadLE32();
    if (bmpType != _kfccBMP0 &&
        bmpType != _kfccBMPZ &&
        bmpType != _kfccBMPJ)
    {
      niError(_A("Invalid bitmap header."));
      return eFalse;
    }

    tSize leftToRead = apFile->ReadLE32();
    if (!apOut) {
      if (bmpType == _kfccBMP0) {
        // niDebugFmt((".... Skipped Raw Bitmap: %d bytes", leftToRead));
        apFile->Seek(leftToRead);
      }
      else if (bmpType == _kfccBMPZ) {
        tSize srcSize = apFile->ReadLE32();
        apFile->Seek(srcSize);
        // niDebugFmt((".... Skipped Zip Bitmap: %d bytes", 4 + srcSize));
      }
      else if (bmpType == _kfccBMPJ) {
        tU32 size = apFile->ReadLE32();
        apFile->Seek(size);
        // niDebugFmt((".... Skipped Jpeg Bitmap: %d bytes", 4 + size));
      }
      else {
        niError("Don't know how to skip bitmap header.");
        return eFalse;
      }

      return eTrue;
    }

    Ptr<iBitmap2D> bmpTmp;
    if ((bmpType == _kfccBMP0) || (bmpType == _kfccBMPZ)) {
      if (apOut->GetPixelFormat()->IsSamePixelFormat(apStoredFormat)) {
        bmpTmp = apOut;
      }
      else {
        bmpTmp = apGraphics->CreateBitmap2DEx(
            apOut->GetWidth(),apOut->GetHeight(),apStoredFormat);
      }

      tPtr p = bmpTmp->GetData();
      if (bmpTmp->GetSize() != leftToRead) {
        niError(niFmt(_A("Bitmap size mismatch, expected '%d' bytes, have to read '%d' bytes."),bmpTmp->GetSize(),leftToRead));
        return eFalse;
      }

      if (bmpType == _kfccBMP0) {
        while (leftToRead > 0) {
          tSize toRead = ni::Min(leftToRead,_knABMCompressBufferSize>>2);
          tSize read = apFile->ReadRaw(p,toRead); //lzoFP.ReadRaw(p,toRead);
          if (read != toRead) {
            niError(niFmt(_A("Can't read bitmap data, %d required, %d read."),toRead,read));
            return eFalse;
          }
          leftToRead -= read;
          p += read;
        }
      }
      else {
        tSize srcSize = apFile->ReadLE32();
        if (!GetZip()->ZipUncompressFileInBuffer(p,leftToRead,apFile,srcSize)) {
          niError(_A("Can't read zipped bitmap data."));
          return eFalse;
        }
      }
    }
    else if (bmpType == _kfccBMPJ) {
      // Read jpeg data
      tU32 size = apFile->ReadLE32();
      Ptr<iFile> fpJpeg = ni::CreateFileMemoryAlloc(size,NULL);
      niCheck(fpJpeg.IsOK(),eFalse);
      if (fpJpeg->WriteFile(apFile->GetFileBase(),size) != size) {
        niError(_A("Can't read jpeg data."));
        return eFalse;
      }
      fpJpeg->SeekSet(0);

      // Jpeg reader
      QPtr<iJpegReader> ptrJpegReader = niCreateInstance(niUI,JpegReader,fpJpeg.ptr(),0);
      niCheck(ptrJpegReader.IsOK(),eFalse);

      // Read the jpeg file
      bmpTmp = ptrJpegReader->ReadBitmap(apGraphics);
    }

    if (!bmpTmp.IsOK()) {
      niError(_A("Can't read tmp bitmap."));
      return eFalse;
    }

    if (bmpTmp != apOut) {
      if (!apOut->Blit(bmpTmp,0,0,0,0,apOut->GetWidth(),apOut->GetHeight())) {
        niError(_A("Can't blit to output bitmap."));
        return eFalse;
      }
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _GenMissingLevels(iBitmap2D* bmp2d, const tU32 numLevels, iPixelFormat* apStoredFormat) {
    const tBool isBlockCompressed = ni::StrStartsWithI(apStoredFormat->GetFormat(),"dxt");
    for (tU32 i = numLevels; i < (bmp2d->GetNumMipMaps()+1); ++i) {
      iBitmap2D* pPrevLevel = bmp2d->GetLevel(i-1);
      if (!pPrevLevel) {
        niError(niFmt(_A("Can't get prev level '%d'."),i));
        return eFalse;
      }
      iBitmap2D* pLevel = bmp2d->GetLevel(i);
      if (!pLevel) {
        niError(niFmt(_A("Can't get level '%d'."),i));
        return eFalse;
      }
      if (isBlockCompressed) {
        ni::MemZero(pLevel->GetData(),pLevel->GetSize());
      }
      else {
        pLevel->BlitStretch(pPrevLevel, 0, 0, 0, 0,
                            pPrevLevel->GetWidth(), pPrevLevel->GetHeight(),
                            pLevel->GetWidth(), pLevel->GetHeight());
      }
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _Load2D(iGraphics* apGraphics, iBitmapBase* bmp, iFile* apFile, iPixelFormat* apStoredFormat, const tU32 numLevels, const tU32 loadLevels) {
    // 2d bitmap
    QPtr<iBitmap2D> bmp2d = bmp;
    if (!bmp2d.IsOK()) {
      niError(_A("Can't get the 2d bitmap interface."));
      return NULL;
    }

    const tU32 skipLevels = numLevels - loadLevels;
    tU32 i = 0;
    for (; i < skipLevels; ++i) {
      if (!_ReadBitmap(apGraphics,NULL,apFile,apStoredFormat)) {
        niError(niFmt(_A("Can't read level '%d'."),i));
        return NULL;
      }
    }
    for (; i < numLevels; ++i) {
      iBitmap2D* pLevel = bmp2d->GetLevel(i-skipLevels);
      if (!pLevel) {
        niError(niFmt(_A("Can't get level '%d'."),i));
        return NULL;
      }
      if (!_ReadBitmap(apGraphics,pLevel,apFile,apStoredFormat)) {
        niError(niFmt(_A("Can't read level '%d'."),i));
        return NULL;
      }
    }
    _GenMissingLevels(bmp2d,numLevels,apStoredFormat);

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _LoadCube(iGraphics* apGraphics, iBitmapBase* bmp, iFile* apFile, iPixelFormat* apStoredFormat, const tU32 numLevels, const tU32 loadLevels) {
    QPtr<iBitmapCube> bmpCube = bmp;
    if (!bmpCube.IsOK()) {
      niError(_A("Can't get the cube bitmap interface."));
      return NULL;
    }
    niLoop(j,6) {
      iBitmap2D* bmp2d = bmpCube->GetFace(eBitmapCubeFace(j));
      if (!bmp2d) {
        niError(niFmt(_A("Can't get face '%d'."),j));
        return NULL;
      }

      const tU32 skipLevels = numLevels - loadLevels;
      tU32 i = 0;
      for (; i < skipLevels; ++i) {
        if (!_ReadBitmap(apGraphics,NULL,apFile,apStoredFormat)) {
          niError(niFmt(_A("Can't read level '%d'."),i));
          return NULL;
        }
      }
      for (; i < numLevels; ++i) {
        iBitmap2D* pLevel = bmp2d->GetLevel(i-skipLevels);
        if (!pLevel) {
          niError(niFmt(_A("Can't get level '%d'."),i));
          return NULL;
        }
        if (!_ReadBitmap(apGraphics,pLevel,apFile,apStoredFormat)) {
          niError(niFmt(_A("Can't read level '%d'."),i));
          return NULL;
        }
      }
      _GenMissingLevels(bmp2d,numLevels,apStoredFormat);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iBitmapBase*  __stdcall LoadBitmap(iGraphics* apGraphics, iFile* apFile) niImpl {
    niCheckIsOK(apGraphics,NULL);
    niCheckIsOK(apFile,NULL);

    const tU32 fccHeader = apFile->ReadLE32();
    if (fccHeader != _kfccABM4 && fccHeader != _kfccABM5) {
      niError(_A("Invalid ABM header."));
      return NULL;
    }

    // read the header
    apFile->BeginReadBits();
    const eBitmapType type = (eBitmapType)apFile->ReadBitsU8(2);
    if (fccHeader == _kfccABM5) {
      /*compression =*/ apFile->ReadBitsI8(7);
    }
    tU32 w = apFile->ReadBitsPackedU32();
    tU32 h = apFile->ReadBitsPackedU32();
    tU32 d = apFile->ReadBitsPackedU32();
    const tU32 numLevels = 1 + apFile->ReadBitsPackedU32();
    tU32 loadLevels = numLevels;
    const cString storedFormat = apFile->ReadBitsString();
    const tBool hasPalette = apFile->ReadBit();
    apFile->EndReadBits();

    if (loadLevels > 1) {
      const tU32 originalW = w;
      niUnused(originalW);
      tU32 maxTexSize = 0;
      // has mip maps, so we can potentially load only lower level mipmaps if
      // the graphics driver doesnt support the maximum resolution
      switch (type) {
        case eBitmapType_2D: {
          maxTexSize = apGraphics->GetDriverCaps(eGraphicsCaps_Texture2DMaxSize);
          break;
        }
        case eBitmapType_3D: {
          maxTexSize = apGraphics->GetDriverCaps(eGraphicsCaps_Texture3DMaxSize);
          break;
        }
        case eBitmapType_Cube: {
          maxTexSize = apGraphics->GetDriverCaps(eGraphicsCaps_TextureCubeMaxSize);
          break;
        }
      }
      // > 2, some minimum sanity check
      if (maxTexSize > 2) {
        while (loadLevels > 1 && w > maxTexSize) {
          w >>= 1;
          h >>= 1;
          d >>= 1;
          --loadLevels;
        }
        // niDebugFmt(("... Adjusted loadLevels: %d (%d) -> %d (%d)",
                    // numLevels, originalW, loadLevels, w));
      }
    }

    Ptr<iPixelFormat> storedPixelFormat = apGraphics->CreatePixelFormat(storedFormat.Chars());

    const tU32 numMips = ni::ComputeNumPow2Levels(w>>1,h>>1,d>>1);
    Ptr<iBitmapFormat> bmpFormat = apGraphics->CreateBitmapFormat(type, storedFormat.Chars(), numMips, w, h, d);
    if (!apGraphics->CheckTextureFormat(bmpFormat,0)) {
      niError(niFmt("Can't validate the stored pixel format '%s'.", storedFormat));
      return NULL;
    }

    if (hasPalette) {
      // This is for backward compatibility, palettes are not supported in ABM anymore
      apFile->Seek(256 * sizeof(tU32));
    }

    if (!storedFormat.IEq(bmpFormat->GetPixelFormat()->GetFormat())) {
      niWarning(niFmt("Loading time pixel format conversion '%s' -> '%s'.", storedFormat, bmpFormat->GetPixelFormat()->GetFormat()));
    }

    Ptr<iBitmapBase> bmp = apGraphics->CreateBitmapEx(NULL,type,bmpFormat->GetPixelFormat(),numMips,w,h,d);
    if (!bmp.IsOK()) {
      niError(_A("Can't create destination bitmap."));
      return NULL;
    }

    if (type == eBitmapType_2D) {
      if (!_Load2D(apGraphics,bmp,apFile,storedPixelFormat,numLevels,loadLevels))
        return NULL;
    }
    else if (type == eBitmapType_Cube) {
      if (!_LoadCube(apGraphics,bmp,apFile,storedPixelFormat,numLevels,loadLevels))
        return NULL;
    }
    else {
      niAssertUnreachable("Invalid bitmap type");
    }

    return bmp.GetRawAndSetNull();
  }
};

struct BitmapSaver_ABM : public ImplRC<iBitmapSaver> {

  ///////////////////////////////////////////////
  tBool _WriteBitmap(iGraphics* apGraphics, iBitmap2D* apSrc, iFile* apFile, tU32 anCompression) {
    const tBool hasAlpha = (apSrc->GetPixelFormat()->GetNumABits() &&
                            !cString(apSrc->GetPixelFormat()->GetFormat()).contains(_A("X8")));

    tU32 bmpType = _kfccBMP0;
    if ((!hasAlpha && anCompression > 0) || (anCompression > 20)) {
      bmpType = _kfccBMPJ;
    }
    else {
      bmpType = _kfccBMPZ;
    }
    apFile->WriteLE32(bmpType);

    tPtr p = apSrc->GetData();
    tSize leftToWrite = apSrc->GetSize();
    apFile->WriteLE32(leftToWrite);

    if (bmpType == _kfccBMP0) {
      while (leftToWrite > 0) {
        tSize toWrite = ni::Min(leftToWrite,_knABMCompressBufferSize>>2);
        tSize written = apFile->WriteRaw(p,toWrite);
        if (written != toWrite) {
          niError(niFmt(_A("Can't write bitmap data, %d required, %d written."),toWrite,written));
          return eFalse;
        }
        leftToWrite -= written;
        p += written;
      }
    }
    else if (bmpType == _kfccBMPZ) {
      const tI64 offset = apFile->Tell();
      apFile->WriteLE32(0); // dummy data
      tSize written = GetZip()->ZipCompressBufferInFile(apFile,p,leftToWrite,9);
      if (written == eInvalidHandle) {
        niError(_A("Can't write zipped bitmap data."));
        return eFalse;
      }
      const tI64 curPos = apFile->Tell();
      apFile->SeekSet(offset);
      apFile->WriteLE32(written);
      apFile->SeekSet(curPos);
    }
    else if (bmpType == _kfccBMPJ) {
      // Jpeg writer
      QPtr<iJpegWriter> ptrJpegWriter = niCreateInstance(niUI,JpegWriter,0,0);
      niCheck(ptrJpegWriter.IsOK(),eFalse);

      // Convert bitmap to supported format
      QPtr<iBitmap2D> bmp = apSrc;
      niCheck(bmp.IsOK(),eFalse);

      tJpegWriteFlags writeFlags = eJpegWriteFlags_YCoCg;
      if (hasAlpha) {
        writeFlags |= eJpegWriteFlags_Alpha;
      }
      if (!ni::StrEq(bmp->GetPixelFormat()->GetFormat(),_A("R8G8B8")) &&
          !ni::StrEq(bmp->GetPixelFormat()->GetFormat(),_A("R8G8B8A8")))
      {
        if (hasAlpha) {
          Ptr<iPixelFormat> pxf = apGraphics->CreatePixelFormat(_A("R8G8B8A8"));
          bmp = bmp->CreateConvertedFormat(pxf);
        }
        else {
          Ptr<iPixelFormat> pxf = apGraphics->CreatePixelFormat(_A("R8G8B8"));
          bmp = bmp->CreateConvertedFormat(pxf);
        }
      }
      niCheck(bmp.IsOK(),eFalse);

      // Write the bitmap
      Ptr<iFile> fpJpeg = ni::CreateFileDynamicMemory(65535,NULL);
      niCheck(fpJpeg.IsOK(),eFalse);
      if (!ptrJpegWriter->WriteBitmap(fpJpeg,bmp,100-ni::Min(anCompression,100),writeFlags)) {
        niError(_A("Can't write to jpeg stream."));
        return eFalse;
      }

      // Write buffer to file
      fpJpeg->SeekSet(0);
      apFile->WriteLE32((tU32)fpJpeg->GetSize());
      if (apFile->WriteFile(fpJpeg->GetFileBase(),fpJpeg->GetSize()) != fpJpeg->GetSize()) {
        niError(_A("Can't write jpeg data."));
        return eFalse;
      }
    }
    else {
      niAssertUnreachable("Invalid serialize type");
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall _Save2D(iGraphics* apGraphics, iBitmapBase* apBmp, iFile* apFile, tU32 anCompression) {
    QPtr<iBitmap2D> bmp2d = apBmp;
    if (!bmp2d.IsOK()) {
      niError(_A("Can't get the 2d bitmap interface."));
      return eFalse;
    }
    niLoop(i,bmp2d->GetNumMipMaps()+1) {
      iBitmap2D* pLevel = bmp2d->GetLevel(i);
      if (!pLevel) {
        niError(niFmt(_A("Can't get level '%d'."),i));
        return eFalse;
      }
      if (!_WriteBitmap(apGraphics,pLevel,apFile,anCompression)) {
        niError(niFmt(_A("Can't write level '%d'."),i));
        return eFalse;
      }
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall _SaveCube(iGraphics* apGraphics, iBitmapBase* apBmp, iFile* apFile, tU32 anCompression) {
    QPtr<iBitmapCube> bmpCube = apBmp;
    if (!bmpCube.IsOK()) {
      niError(_A("Can't get the cube bitmap interface."));
      return eFalse;
    }
    niLoop(j,6) {
      iBitmap2D* bmp2d = bmpCube->GetFace(eBitmapCubeFace(j));
      if (!bmp2d) {
        niError(niFmt(_A("Can't get face '%d'."),j));
        return eFalse;
      }

      niLoop(i,bmp2d->GetNumMipMaps()+1) {
        iBitmap2D* pLevel = bmp2d->GetLevel(i);
        if (!pLevel) {
          niError(niFmt(_A("Can't get level '%d' of face '%d'."),i,j));
          return eFalse;
        }
        if (!_WriteBitmap(apGraphics,pLevel,apFile,anCompression)) {
          niError(niFmt(_A("Can't write level '%d' of face '%d'."),i,j));
          return eFalse;
        }
      }
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SaveBitmap(iGraphics* apGraphics, iFile* apFile, iBitmapBase* apBmp, tU32 anCompression) niImpl {
    niCheckIsOK(apFile,eFalse);
    niCheckIsOK(apBmp,eFalse);

    Ptr<iPixelFormat> pxf = apBmp->GetPixelFormat();
    if (!pxf.IsOK()) {
      niError(_A("Can't get the bitmap's pixel format."));
      return eFalse;
    }

    // write the header
    apFile->WriteLE32(_kfccABM5);
    apFile->BeginWriteBits();
    apFile->WriteBits8((tU8)apBmp->GetType(),2);
    apFile->WriteBits8((tI8)ni::Clamp<tU32>(anCompression,0,100),7);
    apFile->WriteBitsPackedU32(apBmp->GetWidth());
    apFile->WriteBitsPackedU32(apBmp->GetHeight());
    apFile->WriteBitsPackedU32(apBmp->GetDepth());
    apFile->WriteBitsPackedU32(apBmp->GetNumMipMaps());
    apFile->WriteBitsString(pxf->GetFormat());
    apFile->WriteBit(/*hasPalette*/0); // for backward compatibility
    apFile->EndWriteBits();

    if (apBmp->GetType() == eBitmapType_2D) {
      if (!_Save2D(apGraphics,apBmp,apFile,anCompression))
        return NULL;
    }
    else if (apBmp->GetType() == eBitmapType_Cube) {
      if (!_SaveCube(apGraphics,apBmp,apFile,anCompression))
        return NULL;
    }
    else {
      niAssertUnreachable("Invalid bitmap type");
    }

    return eTrue;
  }
};

niExportFunc(iUnknown*) New_BitmapLoader_abm(const Var&,const Var&) {
  return niNew BitmapLoader_ABM();
}

niExportFunc(iUnknown*) New_BitmapSaver_abm(const Var&,const Var&) {
  return niNew BitmapSaver_ABM();
}
