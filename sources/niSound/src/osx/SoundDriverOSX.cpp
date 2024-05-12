// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if defined TARGET_OS_IPHONE
// nothing to define...
#elif defined OSX
#define MACOSX_COREAUDIO 1
#else
#error "SoundDriverOSX: Unknown OS."
#endif

#if MACOSX_COREAUDIO
#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>
#endif

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

#include "../stdafx.h"
#include "../SoundMixerBufferSize.h"

iSoundMixer* __stdcall New_SoundMixerSoftware(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels);
iSoundMixer3D* __stdcall New_SoundMixerSoftware3D(iSoundMixer* apBase, tU32 anNum3DChannels, tU32 anNumAudioChannels);

_HDecl(OSX);

typedef void (*tpfnFillAudio)(void *udata, ni::tU8 *stream, int len);

struct MyCoreAudio {
#if MACOSX_COREAUDIO
  AudioDeviceID deviceID;
#endif
  AudioUnit audioUnit;
  int audioUnitOpened;
  void* fillAudioCallbackUserdata;
  tpfnFillAudio fillAudioCallback;
  void *buffer;
  UInt32 bufferOffset;
  UInt32 bufferSize;
  bool playing;
  bool enabled;
};

static void MyCoreAudio_Destroy(MyCoreAudio* apCoreAudio);

#define CHECK_RESULT(msg)                                         \
  if (result != noErr) {                                          \
    MyCoreAudio_Destroy(apCoreAudio);                           \
    niError(niFmt("CoreAudio error (%s): %d", msg, (int)result)); \
    return 0;                                                     \
  }

#if MACOSX_COREAUDIO
struct sAudioDevice {
  cString name;
  tBool isCapture;
  AudioDeviceID devId;
};

static const AudioObjectPropertyAddress devlist_address = {
  kAudioHardwarePropertyDevices,
  kAudioObjectPropertyScopeGlobal,
  kAudioObjectPropertyElementMaster
};

static void _BuildDeviceList(astl::vector<sAudioDevice>& aDevices, const tBool iscapture)
{
  OSStatus result = noErr;
  UInt32 size = 0;
  AudioDeviceID *devs = NULL;
  UInt32 i = 0;
  UInt32 max = 0;

  result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                          &devlist_address, 0, NULL, &size);
  if (result != kAudioHardwareNoError)
    return;

  devs = (AudioDeviceID *) alloca(size);
  if (devs == NULL)
    return;

  result = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                      &devlist_address, 0, NULL, &size, devs);
  if (result != kAudioHardwareNoError)
    return;

  max = size / sizeof (AudioDeviceID);
  for (i = 0; i < max; i++) {
    CFStringRef cfstr = NULL;
    char *ptr = NULL;
    AudioDeviceID dev = devs[i];
    AudioBufferList *buflist = NULL;
    int usable = 0;
    CFIndex len = 0;
    const AudioObjectPropertyAddress addr = {
      kAudioDevicePropertyStreamConfiguration,
      iscapture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
      kAudioObjectPropertyElementMaster
    };

    const AudioObjectPropertyAddress nameaddr = {
      kAudioObjectPropertyName,
      iscapture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
      kAudioObjectPropertyElementMaster
    };

    result = AudioObjectGetPropertyDataSize(dev, &addr, 0, NULL, &size);
    if (result != noErr)
      continue;

    buflist = (AudioBufferList *)niMalloc(size);
    if (buflist == NULL)
      continue;

    result = AudioObjectGetPropertyData(dev, &addr, 0, NULL,
                                        &size, buflist);

    if (result == noErr) {
      UInt32 j;
      for (j = 0; j < buflist->mNumberBuffers; j++) {
        if (buflist->mBuffers[j].mNumberChannels > 0) {
          usable = 1;
          break;
        }
      }
    }
    niFree(buflist);

    if (!usable)
      continue;


    size = sizeof (CFStringRef);
    result = AudioObjectGetPropertyData(dev, &nameaddr, 0, NULL, &size, &cfstr);
    if (result != kAudioHardwareNoError)
      continue;

    len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfstr),
                                            kCFStringEncodingUTF8);

    ptr = (char *)niMalloc(len + 1);
    usable = ((ptr != NULL) &&
              (CFStringGetCString
               (cfstr, ptr, len + 1, kCFStringEncodingUTF8)));

    CFRelease(cfstr);

    if (usable) {
      len = strlen(ptr);
      /* Some devices have whitespace at the end...trim it. */
      while ((len > 0) && (ptr[len - 1] == ' ')) {
        len--;
      }
      usable = (len > 0);
    }

    if (usable) {
      ptr[len] = '\0';

#if DEBUG_COREAUDIO
      printf("COREAUDIO: Found %s device #%d: '%s' (devid %d)\n",
             ((iscapture) ? "capture" : "output"),
             (int) *devCount, ptr, (int) dev);
#endif
      aDevices.push_back({ ptr, iscapture, dev });
    }
    niFree(ptr);  /* addfn() would have copied the string. */
  }
}
#endif

/* The CoreAudio callback */
static OSStatus _OutputCallback(
  void *inRefCon,
  AudioUnitRenderActionFlags * ioActionFlags,
  const AudioTimeStamp * inTimeStamp,
  UInt32 inBusNumber, UInt32 inNumberFrames,
  AudioBufferList * ioData)
{
  MyCoreAudio* pCoreAudio = (MyCoreAudio*)inRefCon;
  AudioBuffer *abuf;
  UInt32 remaining, len;
  void *ptr;
  UInt32 i;

  /* Only do anything if audio is enabled and not paused */
  if (!pCoreAudio->playing) {
    for (i = 0; i < ioData->mNumberBuffers; i++) {
      abuf = &ioData->mBuffers[i];
      ni::MemSet((tPtr)abuf->mData, 0, abuf->mDataByteSize);
    }
    return 0;
  }

  /* No conversion should be needed here, ever, since we accept any input
     format in OpenAudio, and leave the conversion to CoreAudio.
  */
  for (i = 0; i < ioData->mNumberBuffers; i++) {
    abuf = &ioData->mBuffers[i];
    remaining = abuf->mDataByteSize;
    ptr = abuf->mData;
    while (remaining > 0) {
      if (pCoreAudio->bufferOffset >= pCoreAudio->bufferSize) {
        pCoreAudio->fillAudioCallback(
          pCoreAudio->fillAudioCallbackUserdata, (ni::tU8*)pCoreAudio->buffer, pCoreAudio->bufferSize);
        pCoreAudio->bufferOffset = 0;
      }

      len = pCoreAudio->bufferSize - pCoreAudio->bufferOffset;
      if (len > remaining)
        len = remaining;
      ni::MemCopy((tPtr)ptr, (tPtr)pCoreAudio->buffer+pCoreAudio->bufferOffset, len);
      ptr = (char *)ptr + len;
      remaining -= len;
      pCoreAudio->bufferOffset += len;
    }
  }

  return 0;
}

static OSStatus _InputCallback(
  void *inRefCon,
  AudioUnitRenderActionFlags * ioActionFlags,
  const AudioTimeStamp * inTimeStamp,
  UInt32 inBusNumber, UInt32 inNumberFrames,
  AudioBufferList * ioData)
{
  // TODO: Implement
  return noErr;
}

#if MACOSX_COREAUDIO
static const AudioObjectPropertyAddress _kAliveAddress = {
  kAudioDevicePropertyDeviceIsAlive,
  kAudioObjectPropertyScopeGlobal,
  kAudioObjectPropertyElementMaster
};

static OSStatus _DeviceUnplugged(AudioObjectID devid, UInt32 num_addr, const AudioObjectPropertyAddress *addrs, void *data)
{
  MyCoreAudio* myCoreAudio = (MyCoreAudio*)data;
  tBool dead = eFalse;
  UInt32 isAlive = 1;
  UInt32 size = sizeof (isAlive);
  OSStatus error;

  if (!myCoreAudio->enabled) {
    return 0;  /* already known to be dead. */
  }

  error = AudioObjectGetPropertyData(myCoreAudio->deviceID, &_kAliveAddress,
                                     0, NULL, &size, &isAlive);

  if (error == kAudioHardwareBadDeviceError) {
    dead = eTrue;  /* device was unplugged. */
  } else if ((error == kAudioHardwareNoError) && (!isAlive)) {
    dead = eTrue;  /* device died in some other way. */
  }

  if (dead) {
    // TODO: Actually implement so mechanism to signal the disconnection of the audio device...
    niWarning("Audio device disconnected.");
    myCoreAudio->enabled = eFalse;
  }

  return 0;
}

static int _PrepareDevice(MyCoreAudio* apCoreAudio, AudioDeviceID devid, int iscapture)
{
  OSStatus result = noErr;
  UInt32 size = 0;
  UInt32 alive = 0;
  pid_t pid = 0;

  AudioObjectPropertyAddress addr = {
    0,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
  };

  if (devid == NULL) {
    size = sizeof (AudioDeviceID);
    addr.mSelector =
        ((iscapture) ? kAudioHardwarePropertyDefaultInputDevice :
         kAudioHardwarePropertyDefaultOutputDevice);
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr,
                                        0, NULL, &size, &devid);
    CHECK_RESULT("AudioHardwareGetProperty (default device)");
  }

  addr.mSelector = kAudioDevicePropertyDeviceIsAlive;
  addr.mScope = iscapture ? kAudioDevicePropertyScopeInput :
      kAudioDevicePropertyScopeOutput;

  size = sizeof (alive);
  result = AudioObjectGetPropertyData(devid, &addr, 0, NULL, &size, &alive);
  CHECK_RESULT("AudioDeviceGetProperty (kAudioDevicePropertyDeviceIsAlive)");

  if (!alive) {
    niError("CoreAudio: requested device exists, but isn't alive.");
    return 0;
  }

  addr.mSelector = kAudioDevicePropertyHogMode;
  size = sizeof (pid);
  result = AudioObjectGetPropertyData(devid, &addr, 0, NULL, &size, &pid);

  /* some devices don't support this property, so errors are fine here. */
  if ((result == noErr) && (pid != -1)) {
    niError("CoreAudio: requested device is being hogged.");
    return 0;
  }

  apCoreAudio->deviceID = devid;
  return 1;
}
#endif

static int _PrepareAudioUnit(
  MyCoreAudio* apCoreAudio, tIntPtr devId, int iscapture,
  const AudioStreamBasicDescription * strdesc)
{
  OSStatus result = noErr;
  AURenderCallbackStruct callback;
  AudioComponentDescription desc;
  AudioComponent comp = NULL;
  const AudioUnitElement output_bus = 0;
  const AudioUnitElement input_bus = 1;
  const AudioUnitElement bus = ((iscapture) ? input_bus : output_bus);
  const AudioUnitScope scope = ((iscapture) ? kAudioUnitScope_Output :
                                kAudioUnitScope_Input);

#if MACOSX_COREAUDIO
  if (!_PrepareDevice(apCoreAudio, devId, iscapture)) {
    return 0;
  }
#endif

  ni::MemSet((tPtr)&desc,0,sizeof(desc));
  desc.componentType = kAudioUnitType_Output;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;

#if MACOSX_COREAUDIO
  desc.componentSubType = kAudioUnitSubType_DefaultOutput;
#else
  desc.componentSubType = kAudioUnitSubType_RemoteIO;
#endif
  comp = AudioComponentFindNext(NULL, &desc);

  if (comp == NULL) {
    niError("Couldn't find requested CoreAudio component");
    return 0;
  }

  /* Open & initialize the audio unit */
  result = AudioComponentInstanceNew(comp, &apCoreAudio->audioUnit);
  CHECK_RESULT("AudioComponentInstanceNew");

  apCoreAudio->audioUnitOpened = 1;

#if MACOSX_COREAUDIO
  result = AudioUnitSetProperty(apCoreAudio->audioUnit,
                                kAudioOutputUnitProperty_CurrentDevice,
                                kAudioUnitScope_Global, 0,
                                &apCoreAudio->deviceID,
                                sizeof(AudioDeviceID));
  CHECK_RESULT("AudioUnitSetProperty (kAudioOutputUnitProperty_CurrentDevice)");
#endif

  /* Set the data format of the audio unit. */
  result = AudioUnitSetProperty(apCoreAudio->audioUnit,
                                kAudioUnitProperty_StreamFormat,
                                scope, bus, strdesc, sizeof(*strdesc));
  CHECK_RESULT("AudioUnitSetProperty (kAudioUnitProperty_StreamFormat)");

  /* Set the audio callback */
  ni::MemSet((tPtr)&callback, 0, sizeof(AURenderCallbackStruct));
  callback.inputProc = ((iscapture) ? _InputCallback : _OutputCallback);
  callback.inputProcRefCon = apCoreAudio;
  result = AudioUnitSetProperty(apCoreAudio->audioUnit,
                                kAudioUnitProperty_SetRenderCallback,
                                scope, bus, &callback, sizeof(callback));
  CHECK_RESULT("AudioUnitSetProperty (kAudioUnitProperty_SetRenderCallback)");

  /* Allocate a sample buffer */
  apCoreAudio->bufferOffset = apCoreAudio->bufferSize;
  apCoreAudio->buffer = niMalloc(apCoreAudio->bufferSize);

  result = AudioUnitInitialize(apCoreAudio->audioUnit);
  CHECK_RESULT("AudioUnitInitialize");

  /* Finally, start processing of the audio unit */
  result = AudioOutputUnitStart(apCoreAudio->audioUnit);
  CHECK_RESULT("AudioOutputUnitStart");

#if MACOSX_COREAUDIO
  /* Fire a callback if the device stops being "alive" (disconnected, etc). */
  AudioObjectAddPropertyListener(apCoreAudio->deviceID, &_kAliveAddress, _DeviceUnplugged, apCoreAudio);
#endif

  /* We're running! */
  return 1;
}

static tBool MyCoreAudio_Init(
  MyCoreAudio* apCoreAudio,
  tIntPtr devId,
  int isCapture,
  eSoundFormat aSoundFormat,
  tU32 anFrequency)
{
  AudioStreamBasicDescription strdesc;
  int valid_datatype = 0;

  ni::MemSet((tPtr)apCoreAudio, 0, (sizeof *apCoreAudio));

  /* Setup a AudioStreamBasicDescription with the requested format */
  ni::MemSet((tPtr)&strdesc, 0, sizeof(AudioStreamBasicDescription));
  strdesc.mFormatID = kAudioFormatLinearPCM;
  strdesc.mFormatFlags = kLinearPCMFormatFlagIsPacked;
  strdesc.mSampleRate = anFrequency;
  strdesc.mFramesPerPacket = 1;
  strdesc.mChannelsPerFrame =
      ((aSoundFormat == eSoundFormat_Stereo16) ||
       (aSoundFormat == eSoundFormat_Stereo8)) ? 2 : 1;
  strdesc.mBitsPerChannel = ((int)aSoundFormat)&0xFF;
  // strdesc.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
  // strdesc.mFormatFlags |= kLinearPCMFormatFlagIsFloat;
  if (strdesc.mBitsPerChannel == 16)
    strdesc.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;

  strdesc.mBytesPerFrame =
      strdesc.mBitsPerChannel * strdesc.mChannelsPerFrame / 8;
  strdesc.mBytesPerPacket =
      strdesc.mBytesPerFrame * strdesc.mFramesPerPacket;

  // setup the core audio buffer size
  apCoreAudio->bufferSize = 1024; // Number of samples, good low-latency value for callback
  apCoreAudio->bufferSize *= strdesc.mChannelsPerFrame;
  apCoreAudio->bufferSize *= strdesc.mBitsPerChannel/8;
  if (!_PrepareAudioUnit(apCoreAudio, devId, isCapture, &strdesc)) {
    MyCoreAudio_Destroy(apCoreAudio);
    return eFalse;
  }

  apCoreAudio->enabled = eTrue;
  return eTrue;   /* good to go. */
}

static void MyCoreAudio_Destroy(MyCoreAudio* apCoreAudio)
{
  if (apCoreAudio->audioUnitOpened) {
    OSStatus result = noErr;
    AURenderCallbackStruct callback;
    const AudioUnitElement output_bus = 0;
    const AudioUnitElement input_bus = 1;
    const AudioUnitElement bus = output_bus;
    const AudioUnitScope scope = kAudioUnitScope_Input;

    /* stop processing the audio unit */
    result = AudioOutputUnitStop(apCoreAudio->audioUnit);

    /* Remove the input callback */
    ni::MemSet((tPtr)&callback, 0, sizeof(AURenderCallbackStruct));
    result = AudioUnitSetProperty(apCoreAudio->audioUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  scope, bus, &callback,
                                  sizeof(callback));

#if MACOSX_COREAUDIO
    CloseComponent(apCoreAudio->audioUnit);
#else
    AudioComponentInstanceDispose(apCoreAudio->audioUnit);
#endif

    apCoreAudio->audioUnitOpened = 0;
  }

  if (apCoreAudio->buffer) {
    niFree(apCoreAudio->buffer);
  }
}

//! Sound driver buffer osx implementation.
class cSoundDriverBufferOSX : public ImplRC<iSoundDriverBuffer,eImplFlags_Default>
{
  niBeginClass(cSoundDriverBufferOSX);

 public:
  ///////////////////////////////////////////////
  cSoundDriverBufferOSX() {
    sink = NULL;
  }

  ///////////////////////////////////////////////
  ~cSoundDriverBufferOSX() {
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
    if (!MyCoreAudio_Init(
          &mCoreAudio,
          // Here we could specify a devId as found by _BuildDeviceList to output audio from a specific audio device
          NULL, 0,
          aFormat,
          anFreq))
    {
      niError("MyCoreAudio_Init failed.");
      return eFalse;
    }

    mCoreAudio.fillAudioCallbackUserdata = (void*)this;
    mCoreAudio.fillAudioCallback = _FillAudio;
    mCoreAudio.playing = true;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Stop() {
    if (!mCoreAudio.playing)
      return eTrue;
    mCoreAudio.playing = false;
    MyCoreAudio_Destroy(&mCoreAudio);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tSize __stdcall GetSize() const {
    return mCoreAudio.bufferSize;
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
  static void _FillAudio(void *udata, ni::tU8 *stream, int len) {
    cSoundDriverBufferOSX* buffer = (cSoundDriverBufferOSX*)udata;
    if (buffer->sink.IsOK()) {
      buffer->sink->OnSoundDriverBufferDataSink(stream, len);
    }
  }

  ni::Ptr<iSoundDriverBufferDataSink> sink;
  MyCoreAudio mCoreAudio;

  niEndClass(cSoundDriverBufferOSX);
};

//////////////////////////////////////////////////////////////////////////////////////////////
class cSoundDriverOSX : public ImplRC<iSoundDriver>
{
  niBeginClass(cSoundDriverOSX);

 public:
  ///////////////////////////////////////////////
  cSoundDriverOSX() {
    ZeroMembers();
  }

  ///////////////////////////////////////////////
  ~cSoundDriverOSX() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundDriverOSX);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iHString* __stdcall GetName() const {
    return _HC(OSX);
  }

  ///////////////////////////////////////////////
  virtual tSoundDriverCapFlags __stdcall GetCaps() const {
    return eSoundDriverCapFlags_Buffer;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Startup(eSoundFormat aSoundFormat, tU32 anFrequency, tIntPtr aWindowHandle) {
    _InitDeviceList();

    mptrBuffer = niNew cSoundDriverBufferOSX();
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

  ///////////////////////////////////////////////
  void _InitDeviceList() {
#ifdef MACOSX_COREAUDIO
    if (mDeviceList.empty()) {
      _BuildDeviceList(mDeviceList, 0);
      _BuildDeviceList(mDeviceList, 1);
      niLog(Info, niFmt("Found %d audio devices.", mDeviceList.size()));
      niLoop(i, mDeviceList.size()) {
        niLog(Info, niFmt("- Device %d: name: %s, isCapture: %d, devId: %d.",
                          i, mDeviceList[i].name, mDeviceList[i].isCapture, (tIntPtr)mDeviceList[i].devId));
      }
    }
#endif
  }

 public:
#ifdef MACOSX_COREAUDIO
  astl::vector<sAudioDevice> mDeviceList;
#endif
  ni::Ptr<iSoundDriverBuffer> mptrBuffer;
  ni::Ptr<iSoundMixer>      mptrMixer;
  ni::Ptr<iSoundMixer3D>    mptrMixer3D;

  niEndClass(cSoundDriverOSX);
};

///////////////////////////////////////////////
iSoundDriver* __stdcall New_SoundDriverOSX() {
  return niNew cSoundDriverOSX();
}
