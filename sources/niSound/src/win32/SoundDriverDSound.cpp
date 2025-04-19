// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../stdafx.h"
#include "../SoundMixerBufferSize.h"
#include <niCC.h>

#pragma comment(lib, "dxguid.lib")
#define DIRECTSOUND_VERSION 0x0900
#include <dsound.h>
#include <niLang/Platforms/Win32/Win32_Redef.h>

iSoundMixer* __stdcall New_SoundMixerSoftware(eSoundFormat aFormat, tU32 anFreq,
                                              iSoundDriverBuffer* apBuffer,
                                              tU32 anNumChannels);
iSoundMixer3D* __stdcall New_SoundMixerSoftware3D(iSoundMixer* apBase,
                                                  tU32 anNum3DChannels,
                                                  tU32 anNumAudioChannels);

_HDecl(DSound);

#define NSPLIT 8
#define NSPLIT_SHIFT 3
#define NFIXED 12

// #define DSOUND_USE_HWND

///////////////////////////////////////////////
const achar* DSoundGetErrorString(HRESULT HResult)
{
  switch (HResult) {
  case DS_NO_VIRTUALIZATION: return _A("DS_NO_VIRTUALIZATION");
  case DSERR_ALLOCATED: return _A("DSERR_ALLOCATED");
  case DSERR_CONTROLUNAVAIL: return _A("DSERR_CONTROLUNAVAIL");
  case DSERR_INVALIDPARAM: return _A("DSERR_INVALIDPARAM");
  case DSERR_INVALIDCALL: return _A("DSERR_INVALIDCALL");
  case DSERR_GENERIC: return _A("DSERR_GENERIC");
  case DSERR_PRIOLEVELNEEDED: return _A("DSERR_PRIOLEVELNEEDED");
  case DSERR_OUTOFMEMORY: return _A("DSERR_OUTOFMEMORY");
  case DSERR_BADFORMAT: return _A("DSERR_BADFORMAT");
  case DSERR_UNSUPPORTED: return _A("DSERR_UNSUPPORTED");
  case DSERR_NODRIVER: return _A("DSERR_NODRIVER");
  case DSERR_ALREADYINITIALIZED: return _A("DSERR_ALREADYINITIALIZED");
  case DSERR_NOAGGREGATION: return _A("DSERR_NOAGGREGATION");
  case DSERR_BUFFERLOST: return _A("DSERR_BUFFERLOST");
  case DSERR_NOINTERFACE: return _A("DSERR_NOINTERFACE");
  case DSERR_UNINITIALIZED: return _A("DSERR_UNINITIALIZED");
  case DSERR_OTHERAPPHASPRIO: return _A("DSERR_OTHERAPPHASPRIO");
  case DSERR_ACCESSDENIED: return _A("DSERR_ACCESSDENIED");
  }

  return _A("DSERR_UNKNOWN");
}

struct sDSoundLoader {
  tIntPtr dsound_dll = 0;
  typedef HRESULT(WINAPI* tpfn_dsound_capture_create)(LPCGUID,
                                                      LPDIRECTSOUNDCAPTURE*,
                                                      LPUNKNOWN);
  tpfn_dsound_capture_create ptr_dsound_capture_create = nullptr;
  typedef HRESULT(WINAPI* tpfn_dsound_capture_create8)(LPCGUID,
                                                       LPDIRECTSOUNDCAPTURE8*,
                                                       LPUNKNOWN);
  tpfn_dsound_capture_create8 ptr_dsound_capture_create8 = nullptr;
  typedef HRESULT(WINAPI* tpfn_dsound_create)(LPCGUID, LPDIRECTSOUND*,
                                              LPUNKNOWN);
  tpfn_dsound_create ptr_dsound_create = nullptr;
  typedef HRESULT(WINAPI* tpfn_dsound_create8)(LPCGUID, LPDIRECTSOUND8*,
                                               LPUNKNOWN);
  tpfn_dsound_create8 ptr_dsound_create8 = nullptr;

  tBool Load()
  {
    dsound_dll = ni_dll_load("DSOUND.DLL");
    if (!dsound_dll) {
      niError(_A("Can't load dsound DLL !"));
      return eFalse;
    }

    ptr_dsound_create =
      (tpfn_dsound_create)ni_dll_get_proc(dsound_dll, "DirectSoundCreate");
    ptr_dsound_create8 =
      (tpfn_dsound_create8)ni_dll_get_proc(dsound_dll, "DirectSoundCreate8");
    ptr_dsound_capture_create = (tpfn_dsound_capture_create)ni_dll_get_proc(
      dsound_dll, "DirectSoundCaptureCreate");
    ptr_dsound_capture_create8 = (tpfn_dsound_capture_create8)ni_dll_get_proc(
      dsound_dll, "DirectSoundCaptureCreate8");
    return eTrue;
  }

  ///////////////////////////////////////////////
  HRESULT dsound_capture_create(LPCGUID aGUID, LPDIRECTSOUNDCAPTURE* appOut,
                                LPUNKNOWN apUnknown) const
  {
    if (!ptr_dsound_capture_create) {
      return S_FALSE;
    }
    return ptr_dsound_capture_create(aGUID, appOut, apUnknown);
  }

  ///////////////////////////////////////////////
  HRESULT dsound_capture_create8(LPCGUID aGUID, LPDIRECTSOUNDCAPTURE8* appOut,
                                 LPUNKNOWN apUnknown) const
  {
    if (!ptr_dsound_capture_create8) {
      return S_FALSE;
    }
    return ptr_dsound_capture_create8(aGUID, appOut, apUnknown);
  }

  ///////////////////////////////////////////////
  HRESULT dsound_create(LPCGUID aGUID, LPDIRECTSOUND* appOut,
                        LPUNKNOWN apUnknown) const
  {
    if (!ptr_dsound_create) {
      return S_FALSE;
    }
    return ptr_dsound_create(aGUID, appOut, apUnknown);
  }

  ///////////////////////////////////////////////
  HRESULT dsound_create8(LPCGUID aGUID, LPDIRECTSOUND8* appOut,
                         LPUNKNOWN apUnknown) const
  {
    if (!ptr_dsound_create8) {
      return S_FALSE;
    }
    return ptr_dsound_create8(aGUID, appOut, apUnknown);
  }
};

//////////////////////////////////////////////////////////////////////////////
class cSoundDriverBufferDSound
    : public ImplRC<iSoundDriverBuffer, eImplFlags_Default> {
  niBeginClass(cSoundDriverBufferDSound);

 public:
  ///////////////////////////////////////////////
  cSoundDriverBufferDSound(ain<sDSoundLoader> aLoader, HWND ahWnd)
      : _dsloader(aLoader)
      , _hwnd(ahWnd)
  {
    memset(events, 0, sizeof(events));
    memset(notify, 0, sizeof(notify));

    dsobj = NULL;
    dsbuff_buffer = NULL;
    dsbuff_primary_buffer = NULL;
    dsbuff_notify = NULL;

    dsbuff_buffers_size = 0;
    is_playing = false;
    dsbuff_play = false;

    wait_thread = NULL;
    wait_thread_id = 0;
  }

  ///////////////////////////////////////////////
  ~cSoundDriverBufferDSound()
  {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() niImpl
  {
    Stop();
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const niImpl
  {
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Play(eSoundFormat aFormat, tU32 anFreq) niImpl
  {
    void *pnt1, *pnt2;
    unsigned long size1, size2;
    int i;
    HRESULT hr;
    DSBUFFERDESC tds = {};
    WAVEFORMATEX pcmwf = {};

    tBool bStereo = eFalse;
    WORD nBits = 0;
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

    if ((nBits != 8) && (nBits != 16)) {
      niError(
        niFmt("Bad bits format [%d], only 8 & 16 are supported !", nBits));
      return eFalse;
    }

    is_playing = false;
    dsbuff_buffers_size = sMixerBufferSize(150, anFreq, bStereo).mnBufferSize;
    if (bStereo)
      dsbuff_buffers_size <<= 1;
    if (nBits == 16)
      dsbuff_buffers_size <<= 1;

    for (i = 0; i < NSPLIT; i++)
      events[i] = niWin32API(CreateEvent)(NULL, 0, 0, L"");

    hr = _dsloader.dsound_create8(NULL, &dsobj, NULL);
    if (FAILED(hr)) {
      niError(niFmt("Unable to create DSound Object, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

#ifdef DSOUND_USE_HWND
    if (!_hwnd) {
      niError("No valid window handle specified.");
      goto error;
    }
    hr = dsobj->SetCooperativeLevel(_hwnd, DSSCL_PRIORITY);
#else
    hr = dsobj->SetCooperativeLevel(::GetDesktopWindow(), DSSCL_NORMAL);
#endif
    if (FAILED(hr)) {
      niError(
        niFmt("Unable to set cooperative level for hWnd '%p', DSError: '%s'.",
              (tIntPtr)_hwnd, DSoundGetErrorString(hr)));
      goto error;
    }

    pcmwf.wFormatTag = WAVE_FORMAT_PCM;
    if (bStereo == false)
      pcmwf.nChannels = 1;
    else
      pcmwf.nChannels = 2;
    pcmwf.nSamplesPerSec = anFreq;
    pcmwf.wBitsPerSample = nBits;
    pcmwf.nBlockAlign = pcmwf.wBitsPerSample * pcmwf.nChannels / 8;
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;

    tds.dwSize = sizeof(DSBUFFERDESC);
    tds.dwReserved = 0;
#ifdef DSOUND_USE_HWND
    tds.dwFlags = DSBCAPS_PRIMARYBUFFER;
    tds.dwBufferBytes = 0;
    tds.lpwfxFormat = NULL;
#else
    tds.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
    tds.dwFlags |= DSBCAPS_GLOBALFOCUS;
    tds.dwBufferBytes = this->GetSize();
    tds.lpwfxFormat = &pcmwf;
#endif
    hr = dsobj->CreateSoundBuffer(&tds, &dsbuff_primary_buffer, NULL);
    if (FAILED(hr)) {
      niError(niFmt("Unable to create primary sound buffer, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

#ifdef DSOUND_USE_HWND
    hr = dsbuff_primary_buffer->SetFormat(&pcmwf);
    if (FAILED(hr)) {
      niError(niFmt("Unable to set primary sound buffer format, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }
#endif

    tds.dwSize = sizeof(DSBUFFERDESC);
    tds.dwReserved = 0;
    tds.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFREQUENCY |
                  DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
    tds.dwBufferBytes = dsbuff_buffers_size << 1;
    tds.lpwfxFormat = &pcmwf;
    hr = dsobj->CreateSoundBuffer(&tds, &dsbuff_buffer, NULL);
    if (FAILED(hr)) {
      niError(niFmt("Unable to create sound buffer, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

    hr = dsbuff_buffer->Lock(0, dsbuff_buffers_size << 1, &pnt1, &size1, &pnt2,
                             &size2, DSBLOCK_ENTIREBUFFER);
    if (FAILED(hr)) {
      niError(niFmt("Unable to lock sound buffer, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

    if (size1)
      memset(pnt1, 0, size1);
    if (size2)
      memset(pnt2, 0, size2);

    hr = dsbuff_buffer->Unlock(pnt1, size1, pnt2, size2);
    if (FAILED(hr)) {
      niError(niFmt("Unable to set unlock sound buffer, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

    hr = dsbuff_buffer->QueryInterface(IID_IDirectSoundNotify,
                                       (void**)&dsbuff_notify);
    if (FAILED(hr)) {
      niError(niFmt("Unable to query dsbuff_notify, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

    for (i = 0; i < NSPLIT; i++) {
      notify[i].dwOffset =
        ((i + 1) % NSPLIT) * (2 * dsbuff_buffers_size) / NSPLIT;
      notify[i].hEventNotify = events[i];
    }

    hr = dsbuff_notify->SetNotificationPositions(NSPLIT, notify);
    if (FAILED(hr)) {
      niError(niFmt("Unable to set notify position, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

    is_playing = true;

    current_split = NSPLIT - 1;

    wait_thread = CreateThread(0, 0, _DSBuffThreadProc, (LPVOID)this,
                               CREATE_SUSPENDED, &wait_thread_id);
    SetThreadPriority(wait_thread, THREAD_PRIORITY_TIME_CRITICAL);

    is_playing = true;
    ThreadPlay();

    hr = dsbuff_buffer->Play(0, 0, DSBPLAY_LOOPING);
    if (FAILED(hr)) {
      niError(niFmt("Unable to start DSound Playing, DSError: '%s'.",
                    DSoundGetErrorString(hr)));
      goto error;
    }

    dsbuff_play = true;

    return eTrue;

error:;
    Stop();
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Stop() niImpl
  {
    int i;

    is_playing = false;

    if (wait_thread) {
      WaitForSingleObject(wait_thread, 1000);
      CloseHandle(wait_thread);
      wait_thread = NULL;
    }

    if (dsbuff_buffer) {
      dsbuff_buffer->Stop();
      dsbuff_buffer->Release();
      dsbuff_buffer = NULL;
    }

    if (dsbuff_primary_buffer) {
      dsbuff_primary_buffer->Release();
      dsbuff_primary_buffer = NULL;
    }

    if (dsbuff_notify) {
      dsbuff_notify->Release();
      dsbuff_notify = NULL;
    }

    if (dsobj) {
      dsobj->Release();
      dsobj = NULL;
    }

    for (i = 0; i < NSPLIT; i++) {
      if (events[i]) {
        CloseHandle(events[i]);
        events[i] = NULL;
      }
    }

    dsbuff_play = false;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall GetSize() const niImpl
  {
    return (dsbuff_buffers_size << 1) >> NSPLIT_SHIFT;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSink(iSoundDriverBufferDataSink* apSink) niImpl
  {
    _sink = apSink;
    ThreadPlay();
  }

  ///////////////////////////////////////////////
  virtual iSoundDriverBufferDataSink* __stdcall GetSink() const niImpl
  {
    return _sink;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchIn() niImpl
  {
    if (is_playing) {
      dsbuff_play = true;
      dsbuff_buffer->Play(0, 0, DSBPLAY_LOOPING);
    }
    ThreadPlay();
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchOut() niImpl
  {
    ThreadStop();
    if (is_playing) {
      dsbuff_buffer->Stop();
      dsbuff_play = false;
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall UpdateBuffer() niImpl
  {
  }

  ///////////////////////////////////////////////
  void ThreadPlay()
  {
    if (is_playing && wait_thread && _sink.has_value()) {
      ResumeThread(wait_thread);
    }
  }
  void ThreadStop()
  {
    if (wait_thread)
      SuspendThread(wait_thread);
  }

  ///////////////////////////////////////////////
  void _FillSoundBuffer(int pos)
  {
    void *pnt1, *pnt2;
    unsigned long size1, size2;

    niLet hr =
      dsbuff_buffer->Lock(((pos << 1) * dsbuff_buffers_size) >> NSPLIT_SHIFT,
                          (dsbuff_buffers_size << 1) >> NSPLIT_SHIFT, &pnt1,
                          &size1, &pnt2, &size2, 0);
    niCheck(SUCCEEDED(hr), ;);

    if (pnt1 != NULL) {
      if (_sink.has_value()) {
        _sink->OnSoundDriverBufferDataSink(pnt1, size1);
      }
      else {
        memset(pnt1, 0, size1);
      }
    }

    dsbuff_buffer->Unlock(pnt1, size1, pnt2, size2);
  }

  ///////////////////////////////////////////////
  void ThreadLoop()
  {
    //int i = NSPLIT-1;
    while (is_playing) {
      DWORD ret = WaitForMultipleObjects(NSPLIT, events, FALSE, 1000);
      if (ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS) {
        tI32 index = ret - WAIT_OBJECT_0;
        niDebugAssert(index >= 0 && index < NSPLIT);
        _FillSoundBuffer(index);
      }
      /*if((WaitForSingleObject(events[i],1000) == WAIT_OBJECT_0)
        && (dsbuff_play))
      {
        fill_sound_buffer(i);
        i++;
        if(i == NSPLIT)
          i = 0;
      }*/
    }
  }

  ///////////////////////////////////////////////
  static DWORD WINAPI _DSBuffThreadProc(LPVOID apData)
  {
    cSoundDriverBufferDSound* pBuf =
      reinterpret_cast<cSoundDriverBufferDSound*>(apData);
    pBuf->ThreadLoop();
    //ExitThread(0);
    return 0;
  }

  const HWND _hwnd;
  Ptr<iSoundDriverBufferDataSink> _sink;

  const sDSoundLoader& _dsloader;

  HANDLE events[NSPLIT];
  DSBPOSITIONNOTIFY notify[NSPLIT];

  LPDIRECTSOUND8 dsobj;
  LPDIRECTSOUNDBUFFER dsbuff_buffer;
  LPDIRECTSOUNDBUFFER dsbuff_primary_buffer;
  LPDIRECTSOUNDNOTIFY dsbuff_notify;

  unsigned long dsbuff_buffers_size;
  bool is_playing;
  bool dsbuff_play;

  int current_split;

  HANDLE wait_thread;
  unsigned long wait_thread_id;

  niEndClass(cSoundDriverBufferDSound);
};

//////////////////////////////////////////////////////////////////////////////
class cSoundDriverDSound : public ImplRC<iSoundDriver> {
  niBeginClass(cSoundDriverDSound);

 public:
  ///////////////////////////////////////////////
  cSoundDriverDSound()
  {
    ZeroMembers();
  }

  ///////////////////////////////////////////////
  ~cSoundDriverDSound()
  {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers()
  {
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const
  {
    niClassIsOK(cSoundDriverDSound);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iHString* __stdcall GetName() const
  {
    return _HC(DSound);
  }

  ///////////////////////////////////////////////
  virtual tSoundDriverCapFlags __stdcall GetCaps() const
  {
    return eSoundDriverCapFlags_Buffer;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Startup(eSoundFormat aSoundFormat, tU32 anFrequency,
                          tIntPtr aWindowHandle)
  {
    niCheck(mDSLoader.Load(), eFalse);

    mptrBuffer = niNew cSoundDriverBufferDSound(mDSLoader, (HWND)aWindowHandle);
    if (!mptrBuffer.IsOK()) {
      niError(_A("Can't create the sound driver buffer."));
      return eFalse;
    }

    mptrMixer =
      New_SoundMixerSoftware(aSoundFormat, anFrequency, mptrBuffer, 64);
    niPanicAssert(mptrMixer.IsOK());
    if (!mptrMixer.IsOK()) {
      niError(_A("Can't create the software mixer."));
      return eFalse;
    }

    mptrMixer3D =
      New_SoundMixerSoftware3D(mptrMixer, 32,
                               (aSoundFormat == eSoundFormat_Stereo16 ||
                                aSoundFormat == eSoundFormat_Stereo8)
                                 ? 2
                                 : 1);
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
  tBool __stdcall Shutdown()
  {
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
  void __stdcall Invalidate()
  {
    Shutdown();
  }

  ///////////////////////////////////////////////
  tBool __stdcall SwitchIn()
  {
    if (mptrBuffer.IsOK())
      mptrBuffer->SwitchIn();
    if (mptrMixer.IsOK())
      mptrMixer->SwitchIn();
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SwitchOut()
  {
    if (mptrMixer.IsOK())
      mptrMixer->SwitchOut();
    if (mptrBuffer.IsOK())
      mptrBuffer->SwitchOut();
    return eTrue;
  }

  ///////////////////////////////////////////////
  iSoundDriverBuffer* __stdcall GetBuffer() const
  {
    return mptrBuffer;
  }

  ///////////////////////////////////////////////
  iSoundMixer* __stdcall GetMixer() const
  {
    return mptrMixer;
  }

  ///////////////////////////////////////////////
  iSoundMixer3D* __stdcall GetMixer3D() const
  {
    return mptrMixer3D;
  }

  ///////////////////////////////////////////////
  void __stdcall Update()
  {
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
  sDSoundLoader mDSLoader;
  Ptr<iSoundDriverBuffer> mptrBuffer;
  Ptr<iSoundMixer> mptrMixer;
  Ptr<iSoundMixer3D> mptrMixer3D;

  niEndClass(cSoundDriverDSound);
};

///////////////////////////////////////////////
iSoundDriver* __stdcall New_SoundDriverDSound()
{
  return niNew cSoundDriverDSound();
}
