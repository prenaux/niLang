// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <stdafx.h>
#include "SoundMixerBufferSize.h"

iSoundMixer* __stdcall New_SoundMixerSoftware(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels);
iSoundMixer3D* __stdcall New_SoundMixerSoftware3D(iSoundMixer* apBase, tU32 anNum3DChannels, tU32 anNumAudioChannels);

_HDecl(Silent);

class cSoundDriverBufferSilent : public ImplRC<iSoundDriverBuffer,eImplFlags_Default>
{
  niBeginClass(cSoundDriverBufferSilent);

 public:
  ///////////////////////////////////////////////
  cSoundDriverBufferSilent() {
    mbIsPlaying = false;
  }

  ///////////////////////////////////////////////
  ~cSoundDriverBufferSilent() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    Stop();
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Play(eSoundFormat aFormat, tU32 anFreq) {
    int i;
    tBool bStereo = eFalse;
    tU32 nBits = 0;
    switch (aFormat) {
      case eSoundFormat_Mono8:
        nBits = 8;
        bStereo = eFalse;
        break;
      case eSoundFormat_Mono16:
        nBits = 16;
        bStereo = eFalse;
        break;
      case eSoundFormat_Stereo8:
        nBits = 8;
        bStereo = eTrue;
        break;
      case eSoundFormat_Stereo16:
        nBits = 16;
        bStereo = eTrue;
        break;
    }
    if ((nBits != 8) && (nBits!= 16)) {
      niError(niFmt(_A("Bad bits format [%d], only 8 & 16 are supported."),nBits));
      return eFalse;
    }

    const tU32 samples = 1024;
    const tU32 channels = bStereo ? 2 : 1;    /* 1 = mono, 2 = stereo */

    mnBufferSize = samples;
    mnBufferSize *= channels;
    if (nBits == 16)
      mnBufferSize *= 2;

    niLog(Info,niFmt("Silent driver started with bits: %d, freq: %d, channels: %d",
                     nBits, anFreq, channels));

    mbIsPlaying = true;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Stop() {
    if (!mbIsPlaying)
      return eTrue;
    mbIsPlaying = false;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall GetSize() const {
    return mnBufferSize;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSink(iSoundDriverBufferDataSink* apSink) {
     mSink = apSink;
  }

  ///////////////////////////////////////////////
  virtual iSoundDriverBufferDataSink* __stdcall GetSink() const {
    return mSink;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchIn() {
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchOut() {
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall UpdateBuffer() {
  }

  Ptr<iSoundDriverBufferDataSink> mSink;
  tBool mbIsPlaying = eFalse;
  tU32  mnBufferSize = 0;

  niEndClass(cSoundDriverBufferSilent);
};

//////////////////////////////////////////////////////////////////////////////////////////////
class cSoundDriverSilent : public ImplRC<iSoundDriver>
{
  niBeginClass(cSoundDriverSilent);

 public:
  ///////////////////////////////////////////////
  cSoundDriverSilent() {
  }

  ///////////////////////////////////////////////
  ~cSoundDriverSilent() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundDriverSilent);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iHString* __stdcall GetName() const {
    return _HC(Silent);
  }

  ///////////////////////////////////////////////
  virtual tSoundDriverCapFlags __stdcall GetCaps() const {
    return eSoundDriverCapFlags_Buffer;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Startup(eSoundFormat aSoundFormat, tU32 anFrequency, tIntPtr aWindowHandle) {
    mptrBuffer = niNew cSoundDriverBufferSilent();
    if (!mptrBuffer.IsOK()) {
      niError(_A("Can't create the sound driver buffer."));
      return eFalse;
    }

    mptrMixer = New_SoundMixerSoftware(aSoundFormat,anFrequency,mptrBuffer,64);
    if (!mptrMixer.IsOK()) {
      niError(_A("Can't create the software mixer."));
      return eFalse;
    }

    mptrMixer3D = New_SoundMixerSoftware3D(mptrMixer,32,(aSoundFormat==eSoundFormat_Stereo16||aSoundFormat==eSoundFormat_Stereo8)?2:1);
    if (!mptrMixer3D.IsOK()) {
      niError(_A("Can't create the 3d software mixer."));
      return eFalse;
    }

    QPtr<iSoundDriverBufferDataSink> ptrDataSink = mptrMixer;
    if (!ptrDataSink.IsOK()) {
      niError(_A("Can't get the mixer's sound driver buffer data sink."));
      return eFalse;
    }
    mptrBuffer->SetSink(ptrDataSink);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Shutdown() {
    mptrMixer3D = NULL;
    if (mptrMixer.IsOK()) {
      mptrMixer->Invalidate();
      mptrMixer = NULL;
    }
    if (mptrBuffer.IsOK()) {
      mptrBuffer->Invalidate();
      mptrBuffer = NULL;
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    Shutdown();
  }

  ///////////////////////////////////////////////
  tBool __stdcall SwitchIn() {
    if (mptrBuffer.IsOK())
      mptrBuffer->SwitchIn();
    if (mptrMixer.IsOK())
      mptrMixer->SwitchIn();
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SwitchOut() {
    if (mptrMixer.IsOK())
      mptrMixer->SwitchOut();
    if (mptrBuffer.IsOK())
      mptrBuffer->SwitchOut();
    return eTrue;
  }

  ///////////////////////////////////////////////
  iSoundDriverBuffer* __stdcall GetBuffer() const {
    return mptrBuffer;
  }

  ///////////////////////////////////////////////
  iSoundMixer* __stdcall GetMixer() const {
    return mptrMixer;
  }

  ///////////////////////////////////////////////
  iSoundMixer3D* __stdcall GetMixer3D() const {
    return mptrMixer3D;
  }

  ///////////////////////////////////////////////
  void __stdcall Update() {
    if (mptrBuffer.IsOK()) {
      mptrBuffer->UpdateBuffer();
    }
    if (mptrMixer3D.IsOK()) {
      mptrMixer3D->UpdateMixer3D();
    }
    if (mptrMixer.IsOK()) {
      mptrMixer->UpdateMixer();
    }
  }

 public:
  Ptr<iSoundDriverBuffer> mptrBuffer;
  Ptr<iSoundMixer>      mptrMixer;
  Ptr<iSoundMixer3D>    mptrMixer3D;

  niEndClass(cSoundDriverSilent);
};

///////////////////////////////////////////////
iSoundDriver* __stdcall New_SoundDriverSilent() {
  return niNew cSoundDriverSilent();
}
