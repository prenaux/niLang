// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../stdafx.h"
#include "../SoundMixerBufferSize.h"
#include <SDL/SDL_audio.h>

iSoundMixer* __stdcall New_SoundMixerSoftware(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels);
iSoundMixer3D* __stdcall New_SoundMixerSoftware3D(iSoundMixer* apBase, tU32 anNum3DChannels, tU32 anNumAudioChannels);

_HDecl(SDL);

//! Sound driver buffer waveout implementation.
class cSoundDriverBufferSDL : public cIUnknownImpl<iSoundDriverBuffer,eIUnknownImplFlags_Default>
{
  niBeginClass(cSoundDriverBufferSDL);

 public:
  ///////////////////////////////////////////////
  cSoundDriverBufferSDL() {
    wave_buffer_size = 0;
    is_playing = false;
    sink = NULL;
  }

  ///////////////////////////////////////////////
  ~cSoundDriverBufferSDL() {
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

    /* Set the audio format */
    SDL_AudioSpec wanted, obtained;
    wanted.freq = anFreq;
    wanted.format = nBits == 8 ? AUDIO_U8 : AUDIO_S16;
    wanted.channels = bStereo ? 2 : 1;    /* 1 = mono, 2 = stereo */
    wanted.samples = 1024;                /* Good low-latency value for callback */
    wanted.callback = _FillAudio;
    wanted.userdata = this;

    /* Open the audio device, forcing the desired format */
    if (SDL_OpenAudio(&wanted, &obtained) < 0 ) {
      niError(niFmt("Couldn't open audio: %s\n", SDL_GetError()));
      goto error;
    }

    wave_buffer_size = obtained.samples;
    wave_buffer_size *= obtained.channels;
    if (nBits == 16)
      wave_buffer_size *= 2;

    niLog(Info,niFmt("SDL_audio started with freq: %d, channels: %d, samples: %d",
                     obtained.freq, obtained.channels, obtained.samples));

    SDL_PauseAudio(0);

    is_playing = true;
    return eTrue;

 error:;
    Stop();
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Stop() {
    if (!is_playing)
      return eTrue;
    is_playing = false;
    SDL_CloseAudio();
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall GetSize() const {
    return wave_buffer_size;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSink(iSoundDriverBufferDataSink* apSink) {
    sink = apSink;
  }

  ///////////////////////////////////////////////
  virtual iSoundDriverBufferDataSink* __stdcall GetSink() const {
    return sink;
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

  ///////////////////////////////////////////////
  static void _FillAudio(void *udata, Uint8 *stream, int len) {
    cSoundDriverBufferSDL* buffer = (cSoundDriverBufferSDL*)udata;
    if (buffer->sink.IsOK()) {
      buffer->sink->OnSoundDriverBufferDataSink(stream, len);
    }
  }

  Ptr<iSoundDriverBufferDataSink> sink;
  bool is_playing;
  int wave_buffer_size;

  niEndClass(cSoundDriverBufferSDL);
};

//////////////////////////////////////////////////////////////////////////////////////////////
class cSoundDriverSDL : public cIUnknownImpl<iSoundDriver>
{
  niBeginClass(cSoundDriverSDL);

 public:
  ///////////////////////////////////////////////
  cSoundDriverSDL()
  {
    ZeroMembers();
  }

  ///////////////////////////////////////////////
  ~cSoundDriverSDL() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundDriverSDL);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iHString* __stdcall GetName() const {
    return _HC(SDL);
  }

  ///////////////////////////////////////////////
  virtual tSoundDriverCapFlags __stdcall GetCaps() const {
    return eSoundDriverCapFlags_Buffer;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Startup(eSoundFormat aSoundFormat, tU32 anFrequency, tIntPtr aWindowHandle) {
    mptrBuffer = niNew cSoundDriverBufferSDL();
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

  niEndClass(cSoundDriverSDL);
};

///////////////////////////////////////////////
iSoundDriver* __stdcall New_SoundDriverSDL() {
  return niNew cSoundDriverSDL();
}
