// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../stdafx.h"
#include "../SoundMixerBufferSize.h"
#include <niLang/Utils/TimerSleep.h>
#include <niLang/Utils/DLLLoader.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <atomic>

iSoundMixer* __stdcall New_SoundMixerSoftware(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels);
iSoundMixer3D* __stdcall New_SoundMixerSoftware3D(iSoundMixer* apBase, tU32 anNum3DChannels, tU32 anNumAudioChannels);

_HDecl(ALSA);

////////////////////////////////////////////////////////////////////////////
// ni_dll_load_alsa
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_DECL(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_alsa.h"
#undef NI_DLL_PROC

NI_DLL_BEGIN_LOADER(alsa, "libasound.so");
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_LOAD(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_alsa.h"
#undef NI_DLL_PROC
NI_DLL_END_LOADER(alsa);

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundDriverBufferALSA declaration.

//#define THREADED_ALSA

class cSoundDriverBufferALSA : public ImplRC<iSoundDriverBuffer, eImplFlags_Default>
{
  niBeginClass(cSoundDriverBufferALSA);

 public:
  cSoundDriverBufferALSA() {
    mpPcmHandle = nullptr;
    mbIsPlaying.Set(0);
    mptrSink = nullptr;
#ifdef THREADED_ALSA
    mhPlaybackThread = 0;
#endif
    mnBufferSize = 0;
    mnChannels = 0;
    mnFormat = SND_PCM_FORMAT_S16_LE;
    mnFrequency = 0;
    mnFrameSize = 0;
  }

  ~cSoundDriverBufferALSA() {
    Invalidate();
  }

  void __stdcall Invalidate() {
    Stop();
  }

  ni::tBool __stdcall IsOK() const {
    return ni::eTrue;
  }

  tBool __stdcall Play(eSoundFormat aFormat, tU32 anFreq) {
    Stop();

    mnFrequency = anFreq;

    switch (aFormat) {
      case eSoundFormat_Mono8:    mnFormat = SND_PCM_FORMAT_U8; mnChannels = 1; break;
      case eSoundFormat_Mono16:   mnFormat = SND_PCM_FORMAT_S16_LE; mnChannels = 1; break;
      case eSoundFormat_Stereo8:  mnFormat = SND_PCM_FORMAT_U8; mnChannels = 2; break;
      case eSoundFormat_Stereo16: mnFormat = SND_PCM_FORMAT_S16_LE; mnChannels = 2; break;
      default:
        niError(_A("Unsupported sound format."));
        return eFalse;
    }

    mnFrameSize = (dll_snd_pcm_format_width(mnFormat) / 8) * mnChannels;

    if (!_InitializeALSA()) {
      return eFalse;
    }

    mvBuffer.resize(mnBufferSize);

    mbIsPlaying.Set(1);

#ifdef THREADED_ALSA
    if (pthread_create(&mhPlaybackThread, nullptr, _PlaybackThreadProc, this) != 0) {
      niError(_A("Failed to create playback thread."));
      mbIsPlaying.Set(0);
      return eFalse;
    }
#endif

    return eTrue;
  }

  tBool __stdcall Stop() {
    mbIsPlaying.Set(0);

#ifdef THREADED_ALSA
    if (mhPlaybackThread) {
      pthread_join(mhPlaybackThread, nullptr);
      mhPlaybackThread = 0;
    }
#endif

    if (mpPcmHandle) {
      dll_snd_pcm_drain(mpPcmHandle);
      dll_snd_pcm_close(mpPcmHandle);
      mpPcmHandle = nullptr;
    }

    mvBuffer.clear();

    return eTrue;
  }

  tSize __stdcall GetSize() const {
    return mnBufferSize;
  }

  void __stdcall SetSink(iSoundDriverBufferDataSink* apSink) {
    mptrSink = apSink;
  }

  iSoundDriverBufferDataSink* __stdcall GetSink() const {
    return mptrSink;
  }

  tBool __stdcall SwitchIn() {
    return eTrue;
  }

  tBool __stdcall SwitchOut() {
    return eTrue;
  }

  void __stdcall UpdateBuffer() {
#ifndef THREADED_ALSA
    _FillAudioBuffer();
#endif
  }

 private:
#ifdef THREADED_ALSA
  static void* _PlaybackThreadProc(void* arg) {
    cSoundDriverBufferALSA* self = static_cast<cSoundDriverBufferALSA*>(arg);
    while (self->mbIsPlaying.Get()) {
      self->_FillAudioBuffer();
    }
    return nullptr;
  }
#endif

  void _FillAudioBuffer() {
    if (!mptrSink.IsOK()) return;

    mptrSink->OnSoundDriverBufferDataSink(mvBuffer.data(), mnBufferSize);

    const tU8* sample_buf = mvBuffer.data();
    snd_pcm_uframes_t frames_left = mnBufferSize / mnFrameSize;

    while (frames_left > 0 && mbIsPlaying.Get()) {
      int status = dll_snd_pcm_writei(mpPcmHandle, sample_buf, frames_left);

      if (status < 0) {
        if (status == -EAGAIN) {
          ni::SleepMs(1);
          continue;
        }
        status = dll_snd_pcm_recover(mpPcmHandle, status, 0);
        if (status < 0) {
          niError(niFmt(_A("ALSA write failed (unrecoverable): %s"), dll_snd_strerror(status)));
          mbIsPlaying.Set(0);
          return;
        }
        continue;
      }
      else if (status == 0) {
        ni::SleepMs((frames_left * 1000) / mnFrequency);
      }

      sample_buf += status * mnFrameSize;
      frames_left -= status;
    }
  }

  tBool _InitializeALSA() {
    int err;

    if ((err = dll_snd_pcm_open(&mpPcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
      niError(niFmt(_A("Cannot open audio device: %s"), dll_snd_strerror(err)));
      return eFalse;
    }

    snd_pcm_hw_params_t *hwParams;
    err = dll_snd_pcm_hw_params_malloc(&hwParams);
    if (err < 0) {
      niError(niFmt(_A("Cannot allocate hardware parameter structure: %s"), dll_snd_strerror(err)));
      return eFalse;
    }
    niDefer {
      dll_snd_pcm_hw_params_free(hwParams);
    };
    dll_snd_pcm_hw_params_any(mpPcmHandle, hwParams);

    if ((err = dll_snd_pcm_hw_params_set_access(mpPcmHandle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
      niError(niFmt(_A("Cannot set access type: %s"), dll_snd_strerror(err)));
      return eFalse;
    }

    if ((err = dll_snd_pcm_hw_params_set_format(mpPcmHandle, hwParams, mnFormat)) < 0) {
      niError(niFmt(_A("Cannot set sample format: %s"), dll_snd_strerror(err)));
      return eFalse;
    }

    if ((err = dll_snd_pcm_hw_params_set_channels(mpPcmHandle, hwParams, mnChannels)) < 0) {
      niError(niFmt(_A("Cannot set channel count: %s"), dll_snd_strerror(err)));
      return eFalse;
    }

    unsigned int rate = mnFrequency;
    if ((err = dll_snd_pcm_hw_params_set_rate_near(mpPcmHandle, hwParams, &rate, 0)) < 0) {
      niError(niFmt(_A("Cannot set sample rate: %s"), dll_snd_strerror(err)));
      return eFalse;
    }

    snd_pcm_uframes_t bufferSize =
#ifdef THREADED_ALSA
        mnFrequency / 10 // 100ms buffer
#else
        mnFrequency / 50 // 20ms buffer... blocking non-threaded is a bit of a hack
#endif
        ;
    if ((err = dll_snd_pcm_hw_params_set_buffer_size_near(mpPcmHandle, hwParams, &bufferSize)) < 0) {
      niError(niFmt(_A("Cannot set buffer size: %s"), dll_snd_strerror(err)));
      return eFalse;
    }

    if ((err = dll_snd_pcm_hw_params(mpPcmHandle, hwParams)) < 0) {
      niError(niFmt(_A("Cannot set parameters: %s"), dll_snd_strerror(err)));
      return eFalse;
    }

    mnBufferSize = bufferSize * mnFrameSize;

    niLog(Info, niFmt(
      "ALSA sound driver initialized: format: %s, channels: %d, frequency: %d Hz, frame size: %d bytes, buffer: %d frames (%d bytes)",
      dll_snd_pcm_format_name(mnFormat),
      mnChannels,
      mnFrequency,
      mnFrameSize,
      mnBufferSize / mnFrameSize,
      mnBufferSize));

    return eTrue;
  }

  snd_pcm_t* mpPcmHandle;
  SyncCounter mbIsPlaying;
  Ptr<iSoundDriverBufferDataSink> mptrSink;
#ifdef THREADED_ALSA
  pthread_t mhPlaybackThread;
#endif
  tU32 mnBufferSize;
  tU32 mnChannels;
  snd_pcm_format_t mnFormat;
  tU32 mnFrequency;
  tU32 mnFrameSize;
  astl::vector<tU8> mvBuffer;

  niEndClass(cSoundDriverBufferALSA);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundDriverALSA declaration.
class cSoundDriverALSA : public ImplRC<iSoundDriver>
{
  niBeginClass(cSoundDriverALSA);

 public:
  cSoundDriverALSA() {
  }

  ~cSoundDriverALSA() {
    Invalidate();
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundDriverALSA);
    return eTrue;
  }

  virtual iHString* __stdcall GetName() const {
    return _HC(ALSA);
  }
 tSoundDriverCapFlags __stdcall GetCaps() const {
    return eSoundDriverCapFlags_Buffer;
  }

  tBool __stdcall Startup(eSoundFormat aSoundFormat, tU32 anFrequency, tIntPtr aWindowHandle) {
    mbHasALSA = ni_dll_load_alsa();
    niCheck(mbHasALSA, eFalse);

    mptrBuffer = niNew cSoundDriverBufferALSA();
    if (!mptrBuffer.IsOK()) {
      niError(_A("Can't create the sound driver buffer."));
      return eFalse;
    }

    mptrMixer = New_SoundMixerSoftware(aSoundFormat, anFrequency, mptrBuffer, 64);
    if (!mptrMixer.IsOK()) {
      niError(_A("Can't create the software mixer."));
      return eFalse;
    }

    mptrMixer3D = New_SoundMixerSoftware3D(mptrMixer, 32, (aSoundFormat == eSoundFormat_Stereo16 || aSoundFormat == eSoundFormat_Stereo8) ? 2 : 1);
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

  void __stdcall Invalidate() {
    Shutdown();
  }

  tBool __stdcall SwitchIn() {
    if (mptrBuffer.IsOK())
      mptrBuffer->SwitchIn();
    if (mptrMixer.IsOK())
      mptrMixer->SwitchIn();
    return eTrue;
  }

  tBool __stdcall SwitchOut() {
    if (mptrMixer.IsOK())
      mptrMixer->SwitchOut();
    if (mptrBuffer.IsOK())
      mptrBuffer->SwitchOut();
    return eTrue;
  }

  iSoundDriverBuffer* __stdcall GetBuffer() const {
    return mptrBuffer;
  }

  iSoundMixer* __stdcall GetMixer() const {
    return mptrMixer;
  }

  iSoundMixer3D* __stdcall GetMixer3D() const {
    return mptrMixer3D;
  }

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
  tBool                   mbHasALSA = eFalse;
  Ptr<iSoundDriverBuffer> mptrBuffer;
  Ptr<iSoundMixer>        mptrMixer;
  Ptr<iSoundMixer3D>      mptrMixer3D;

  niEndClass(cSoundDriverALSA);
};

///////////////////////////////////////////////
iSoundDriver* __stdcall New_SoundDriverALSA() {
  return niNew cSoundDriverALSA();
}
