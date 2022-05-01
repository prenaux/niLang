// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

class cSoundDataOGG_STB : public cIUnknownImpl<iSoundData>
{
  niBeginClass(cSoundDataOGG_STB);

 public:

  ///////////////////////////////////////////////
  cSoundDataOGG_STB(iFile* apFile)
  {
    mnSize = 0;
    mnLength = 0;
    mnPosition = 0;

    mptrFile = apFile;

    // Copy to memory if it isn't already
    if (!mptrFile->GetBase()) {
      Ptr<iFile> origFile = apFile;
      mptrFile = ni::CreateFileMemoryAlloc(origFile->GetSize(),origFile->GetSourcePath());
      origFile->SeekSet(0);
      mptrFile->WriteFile(origFile->GetFileBase(),origFile->GetSize());
    }

    int vorbisError = 0;
    mStream = stb_vorbis_open_memory(mptrFile->GetBase(),mptrFile->GetSize(),&vorbisError,NULL);
    if (!mStream) {
      niError(niFmt("Can't open ogg file: %d.", vorbisError));
      return;
    }

    mInfo = stb_vorbis_get_info(mStream);

    if (mInfo.channels == 1)
      mFormat = eSoundFormat_Mono16;
    else if (mInfo.channels == 2)
      mFormat = eSoundFormat_Stereo16;
    else {
      niError(niFmt(_A("Unsupported number of channels '%d'."),mInfo.channels));
      return;
    }

    tI64 totalPCM = stb_vorbis_stream_length_in_samples(mStream);
    if (totalPCM <= 0) {
      niError(_A("OGG opening failed."));
      return;
    }

    mnLength = (tU32)totalPCM;
    mnSize = mnLength*SoundFormatGetFrameSize(mFormat);
  }

  ///////////////////////////////////////////////
  ~cSoundDataOGG_STB() {
    stb_vorbis_close(mStream);
  }

  ///////////////////////////////////////////////
  //! Sanity check.
  tBool __stdcall IsOK() const
  {
    niClassIsOK(cSoundDataOGG_STB);
    return mnLength != 0;
  }

  ///////////////////////////////////////////////
  //! Get the sound frequency in hertz.
  tU32 __stdcall GetFrequency() const
  {
    return mInfo.sample_rate;
  }

  ///////////////////////////////////////////////
  //! Get the sound data format.
  eSoundFormat __stdcall GetFormat() const
  {
    return mFormat;
  }

  ///////////////////////////////////////////////
  //! Read a chunk of the specified size.
  tI32 __stdcall ReadRaw(tPtr apOut, tU32 anSize)
  {
    if (mnPosition >= mnSize)
      return 0;

    tU32 size = 0;
    while (size < anSize) {
      int ns = stb_vorbis_get_samples_short_interleaved(mStream, mInfo.channels, (short*)(apOut+size), (anSize-size)/2);
      if (ns < 0) {
        // decoding error...
        niError(niFmt("OGG decoding error: %d", ns));
        break;
      }
      else if (ns == 0) {
        break;
      }
      size += ns * mInfo.channels * 2;
    }

    mnPosition += size;
    return size;
  }

  ///////////////////////////////////////////////
  void __stdcall Reset() {
    stb_vorbis_close(mStream);
    mptrFile->SeekSet(0);
    int vorbisError = 0;
    mStream = stb_vorbis_open_memory(mptrFile->GetBase(),mptrFile->GetSize(),&vorbisError,NULL);
    mnPosition = 0;
  }

  ///////////////////////////////////////////////
  iSoundData* __stdcall Clone() const {
#pragma niTodo("Should reopen a file handle and not use auto seek...")
    if (!mptrFile.IsOK()) return NULL;
    Ptr<iFile> ptrFile = ni::CreateFileWindow(mptrFile->GetFileBase(),0,0,NULL,eTrue);
    return niNew cSoundDataOGG_STB(ptrFile);
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetLength() const {
    return (tU32)mnLength;
  }

  tU32           mnLength;
  Ptr<iFile>     mptrFile;
  eSoundFormat   mFormat;
  tU32           mnSize;
  tU32           mnPosition;
  stb_vorbis*     mStream;
  stb_vorbis_info mInfo;

  niEndClass(cSoundDataOGG_STB);
};

class cSoundDataLoaderOGG_STB : public cIUnknownImpl<iSoundDataLoader>
{
  iSoundData* __stdcall LoadSoundData(iFile* apFile) {
    return niNew cSoundDataOGG_STB(apFile);
  }
};

iSoundDataLoader* New_SoundDataLoaderOGG_STB() {
  return niNew cSoundDataLoaderOGG_STB();
}
