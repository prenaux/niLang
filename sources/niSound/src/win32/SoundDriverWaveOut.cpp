// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../stdafx.h"
#include "../SoundMixerBufferSize.h"

iSoundMixer* __stdcall New_SoundMixerSoftware(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels);
iSoundMixer3D* __stdcall New_SoundMixerSoftware3D(iSoundMixer* apBase, tU32 anNum3DChannels, tU32 anNumAudioChannels);

#include <niLang/Platforms/Win32/Win32_Redef.h>

#define NSPLIT             8
#define NSPLIT_SHIFT       3
#define NFIXED             12
#define NBUFFERS           2
#define WAVE_BUFFER_SIZE   2048
#include <mmsystem.h>

_HDecl(WaveOut);

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundDriverBufferWaveOut declaration.

#define THREADED_WAVEOUT

static DWORD WINAPI _WaveOutThreadProc(LPVOID apData);

//! Sound driver buffer waveout implementation.
class cSoundDriverBufferWaveOut : public ImplRC<iSoundDriverBuffer,eImplFlags_Default>
{
  niBeginClass(cSoundDriverBufferWaveOut);

 public:
  ///////////////////////////////////////////////
  cSoundDriverBufferWaveOut() {
    ZeroMembers();
  }

  ///////////////////////////////////////////////
  ~cSoundDriverBufferWaveOut() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    Stop();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
    memset(waves,0,sizeof(waves));
    wave_device = NULL;
    wave_buffer_size = 0;
    is_playing = false;
    sink = NULL;
#ifdef THREADED_WAVEOUT
    wait_thread = NULL;
    wait_thread_id = 0;
#endif
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Play (eSoundFormat aFormat, tU32 anFreq) {
    int i;
    HRESULT hr;
    WAVEFORMATEX pcmwf;
    tBool bStereo = eFalse;
    WORD nBits = 0;
    switch (aFormat) {
      case eSoundFormat_Mono8:    nBits = 8; bStereo = eFalse; break;
      case eSoundFormat_Mono16:   nBits = 16; bStereo = eFalse; break;
      case eSoundFormat_Stereo8:    nBits = 8; bStereo = eTrue; break;
      case eSoundFormat_Stereo16:   nBits = 16; bStereo = eTrue; break;
    }

    if ((nBits != 8) && (nBits!= 16)) {
      niError(niFmt(_A("Bad bits format [%d], only 8 & 16 are supported."),nBits));
      return eFalse;
    }

    wave_buffer_size = sMixerBufferSize(200,anFreq,bStereo).mnBufferSize;

    is_playing = false;

    if(bStereo)
      wave_buffer_size <<= 1;
    if(nBits == 16)
      wave_buffer_size <<= 1;
    wave_buffer_size = (wave_buffer_size<<2)>>NSPLIT_SHIFT;

    pcmwf.wFormatTag = WAVE_FORMAT_PCM;

    if(bStereo == false)
      pcmwf.nChannels = 1;
    else
      pcmwf.nChannels = 2;
    pcmwf.nSamplesPerSec = anFreq;
    pcmwf.wBitsPerSample = (WORD)nBits;
    pcmwf.nBlockAlign = pcmwf.wBitsPerSample*pcmwf.nChannels/8;
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec*pcmwf.nBlockAlign;
    pcmwf.cbSize = 0;

    hr = waveOutOpen(&wave_device, WAVE_MAPPER, &pcmwf, 0, 0, 0);
    if (FAILED(hr)) {
      niError(_A("Can't open WAVE device."));
      goto error;
    }

    for(i = 0; i < NBUFFERS; i++)
    {
      waves[i].dwBufferLength = wave_buffer_size;
      waves[i].lpData = (char*)niMalloc(waves[i].dwBufferLength);
      if (!waves[i].lpData) {
        niError(niFmt(_A("Unable to alloc memory to waves %d."), i));
        goto error;
      }
      waves[i].dwFlags = 0;
      waves[i].reserved = 0;
      waveOutPrepareHeader(wave_device, &waves[i], sizeof(WAVEHDR));
      waves[i].dwFlags |= WHDR_DONE;
      ZeroMemory(waves[i].lpData, waves[i].dwBufferLength);
      waveOutWrite(wave_device, &waves[i], sizeof(WAVEHDR));
    }

#ifdef THREADED_WAVEOUT
    wait_thread = CreateThread(0, 0, _WaveOutThreadProc, (LPVOID)this, CREATE_SUSPENDED, &wait_thread_id);
    SetThreadPriority(wait_thread, THREAD_PRIORITY_TIME_CRITICAL);
#endif

    is_playing = true;
    ThreadPlay();

    return eTrue;

 error:;
    Stop();
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Stop() {
    int i;

    is_playing = false;

#ifdef THREADED_WAVEOUT
    if(wait_thread)
    {
      WaitForSingleObject(wait_thread, 1000);
      CloseHandle(wait_thread);
      wait_thread = NULL;
    }
#endif

    if(wave_device)
    {
      waveOutReset(wave_device);
      for(i = 0; i < NBUFFERS; i++) {
        waveOutUnprepareHeader(wave_device, &waves[i], sizeof(WAVEHDR));
      }
      waveOutClose(wave_device);
      wave_device = NULL;
    }

    for(i = 0; i < NBUFFERS; i++)
    {
      niFree(waves[i].lpData);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall GetSize() const {
    return wave_buffer_size;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSink(iSoundDriverBufferDataSink* apSink) {
    sink = apSink;
    ThreadPlay();
  }

  ///////////////////////////////////////////////
  virtual iSoundDriverBufferDataSink* __stdcall GetSink() const {
    return sink;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchIn() {
    ThreadPlay();
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchOut() {
    ThreadStop();
    return eTrue;
  }

  void __stdcall UpdateBuffer() {
#ifndef THREADED_WAVEOUT
    DoUpdate();
#endif
  }

  //! Update the sound buffer.
  void __stdcall DoUpdate() {
    if (sink.IsOK()) {
      for (tU32 i = 0; i < NBUFFERS; i++)
      {
        if (waves[i].dwFlags & WHDR_DONE) {
          sink->OnSoundDriverBufferDataSink(waves[i].lpData, wave_buffer_size);
          waveOutWrite(wave_device, &waves[i], sizeof(WAVEHDR));
        }
      }
    }
  }

  ///////////////////////////////////////////////
  void ThreadPlay() {
#ifdef THREADED_WAVEOUT
    if (is_playing && wait_thread && sink.IsOK()) {
      ResumeThread(wait_thread);
    }
#endif
  }
  void ThreadStop() {
#ifdef THREADED_WAVEOUT
    if (wait_thread)
      SuspendThread(wait_thread);
#endif
  }

  Ptr<iSoundDriverBufferDataSink> sink;

  WAVEHDR waves[NBUFFERS];
  HWAVEOUT wave_device;

  unsigned long wave_buffer_size;
  bool is_playing;

#ifdef THREADED_WAVEOUT
  HANDLE wait_thread;
  unsigned long wait_thread_id;
#endif

  niEndClass(cSoundDriverBufferWaveOut);
};

#ifdef THREADED_WAVEOUT
///////////////////////////////////////////////
// Thread that fills the sound buffer
static DWORD WINAPI _WaveOutThreadProc(LPVOID apData)
{
  cSoundDriverBufferWaveOut* pBuf = niUnsafeCast(cSoundDriverBufferWaveOut*,apData);

  while (pBuf->is_playing)
  {
    pBuf->DoUpdate();
    Sleep(1);
  }

  ExitThread(0);
  return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
class cSoundDriverWaveOut : public ImplRC<iSoundDriver>
{
  niBeginClass(cSoundDriverWaveOut);

 public:
  ///////////////////////////////////////////////
  cSoundDriverWaveOut()
  {
    ZeroMembers();
  }

  ///////////////////////////////////////////////
  ~cSoundDriverWaveOut() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundDriverWaveOut);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iHString* __stdcall GetName() const {
    return _HC(WaveOut);
  }

  ///////////////////////////////////////////////
  virtual tSoundDriverCapFlags __stdcall GetCaps() const {
    return eSoundDriverCapFlags_Buffer;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Startup(eSoundFormat aSoundFormat, tU32 anFrequency, tIntPtr aWindowHandle) {
    mptrBuffer = niNew cSoundDriverBufferWaveOut();
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

  niEndClass(cSoundDriverWaveOut);
};

///////////////////////////////////////////////
iSoundDriver* __stdcall New_SoundDriverWaveOut() {
  return niNew cSoundDriverWaveOut();
}
