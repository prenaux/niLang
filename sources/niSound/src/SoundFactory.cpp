// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "SoundFactory.h"
#include "SoundBufferMem.h"
#include "SoundBufferStream.h"
#include "SoundSource.h"

#ifdef niWindows
#pragma comment(lib,"msacm32.lib")
#pragma comment(lib,"winmm.lib")
#endif

#define CheckValid(RET) if (!mptrMixer.IsOK()) return RET;
#define CheckValid3D(RET) if (!mptrMixer3D.IsOK()) return RET;

iSoundDriver* __stdcall New_SoundDriverSilent();
iSoundDataLoader* New_SoundDataLoaderWAV();
iSoundDataLoader* New_SoundDataLoaderOGG_STB();

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundFactory implementation.

static inline void _RegisterSoundDataLoader(const achar* aName, iUnknown* apInstance) {
  ni::GetLang()->SetGlobalInstance(niFmt("SoundDataLoader.%s",aName), apInstance);
}

///////////////////////////////////////////////
cSoundFactory::cSoundFactory()
{
  ZeroMembers();

  // register drivers
#if !defined NO_SOUND

#if defined niWindows
  mvDrivers.push_back(New_SoundDriverWaveOut());
  if (!niIsOK(mvDrivers.back())) mvDrivers.erase(mvDrivers.begin()+mvDrivers.size()-1);
#elif defined niAndroid
  mvDrivers.push_back(New_SoundDriverJNI());
  if (!niIsOK(mvDrivers.back())) mvDrivers.erase(mvDrivers.begin()+mvDrivers.size()-1);
#elif defined niOSX || defined niIOS
  mvDrivers.push_back(New_SoundDriverOSX());
  if (!niIsOK(mvDrivers.back())) mvDrivers.erase(mvDrivers.begin()+mvDrivers.size()-1);
#else
  mvDrivers.push_back(New_SoundDriverSDL());
  if (!niIsOK(mvDrivers.back())) mvDrivers.erase(mvDrivers.begin()+mvDrivers.size()-1);
#endif

#endif

  mvDrivers.push_back(New_SoundDriverSilent());
  if (!niIsOK(mvDrivers.back())) mvDrivers.erase(mvDrivers.begin()+mvDrivers.size()-1);

  // sound buffer manager
  mptrSoundBufferMgr = ni::GetLang()->CreateDeviceResourceManager(_A("SoundBuffer"));
  if (!niIsOK(mptrSoundBufferMgr)) {
    niError(_A("Can't create the sound buffer resource manager."));
    return;
  }

  // plugins
  _RegisterSoundDataLoader("wav",New_SoundDataLoaderWAV());
  _RegisterSoundDataLoader("ogg",New_SoundDataLoaderOGG_STB());

  SetListenerPosition(sVec3f::Zero());
  SetListenerVelocity(sVec3f::Zero());
  SetListenerForward(sVec3f::ZAxis());
  SetListenerUp(sVec3f::YAxis());
  SetListenerDistanceScale(0.1f);
  SetListenerDopplerScale(1.0f);
  SetListenerRolloffScale(1.0f);

  // Log drivers available
  if (!GetNumDrivers()) {
    niLog(Info,niFmt(_A("No sound driver available.")));
  }
  else {
    niLoop(i,GetNumDrivers()) {
      iSoundDriver* pDrv = GetDriver(i);
      niLog(Info,niFmt(_A("Sound driver '%s' found."),niHStr(pDrv->GetName())));
    }
  }
}

///////////////////////////////////////////////
cSoundFactory::~cSoundFactory()
{
  Invalidate();
}

///////////////////////////////////////////////
tU32 __stdcall cSoundFactory::GetNumDrivers() const
{
  return mvDrivers.size();
}

///////////////////////////////////////////////
iSoundDriver* __stdcall cSoundFactory::GetDriver(tU32 anIndex) const
{
  niCheckSilent(anIndex < mvDrivers.size(),NULL);
  return mvDrivers[anIndex];
}

///////////////////////////////////////////////
iHString* __stdcall cSoundFactory::GetDriverName(tU32 anIndex) const
{
  niCheckSilent(anIndex < mvDrivers.size(),NULL);
  return mvDrivers[anIndex]->GetName();
}

///////////////////////////////////////////////
tSoundDriverCapFlags __stdcall cSoundFactory::GetDriverCaps(tU32 anIndex) const
{
  niCheckSilent(anIndex < mvDrivers.size(),0);
  return mvDrivers[anIndex]->GetCaps();
}

///////////////////////////////////////////////
tU32 __stdcall cSoundFactory::GetDriverIndex(iHString* ahspName) const
{
  for (tU32 i = 0; i < mvDrivers.size(); ++i) {
    if (mvDrivers[i]->GetName() == ahspName)
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tBool __stdcall cSoundFactory::StartupDriver(tU32 anDriver, eSoundFormat aSoundFormat, tU32 anFrequency, tIntPtr aWindowHandle)
{
  ShutdownDriver();

  if (anDriver == eInvalidHandle) {
    // the last driver is supposed to be the best one... so we try to init that driver first... if it fails we try until we exhausted all possibilities
    for (tI32 i = mvDrivers.size()-1; i >= 0; --i) {
      Ptr<iSoundDriver> ptrDrv = mvDrivers[i];
      if (ptrDrv->Startup(aSoundFormat,anFrequency,aWindowHandle)) {
        mnActiveDriver = i;
        mptrDriver = ptrDrv;
        break;
      }
    }
    if (!mptrDriver.IsOK()) {
      niError(_A("Can't find a driver that can be started."));
      return eFalse;
    }
  }
  else{
    niCheck(anDriver < GetNumDrivers(),eFalse);
    Ptr<iSoundDriver> ptrDrv = mvDrivers[anDriver];
    if (!ptrDrv->Startup(aSoundFormat,anFrequency,aWindowHandle)) {
      niError(niFmt(_A("Can't startup the driver '%s'."),niHStr(mvDrivers[mnActiveDriver]->GetName())));
      return eFalse;
    }
    mptrDriver = ptrDrv;
    mnActiveDriver = anDriver;
  }

  mptrMixer = mptrDriver->GetMixer();
  if (!mptrMixer.IsOK()) {
    niError(_A("Can't get the sound driver's mixer."));
    return eFalse;
  }

  mptrMixer3D = mptrDriver->GetMixer3D();

  {
    mvChannels.resize(mptrMixer->GetNumChannels());
    sChannel c;
    c.nPriority = 0;
    astl::fill(mvChannels,c);
  }
  niLog(Info,niFmt(_A("Sound driver '%s' started."),niHStr(mptrDriver->GetName())));
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cSoundFactory::ShutdownDriver()
{
  if (mnActiveDriver == eInvalidHandle)
    return eFalse;
  mptrMixer3D = NULL;
  if (mptrMixer.IsOK()) {
    for (tU32 i = 0; i < mptrMixer->GetNumChannels(); ++i) {
      mptrMixer->StopChannel(i);
      mptrMixer->SetChannelBuffer(i,NULL);
    }
    mptrMixer = NULL;
  }
  if (mptrDriver.IsOK()) {
    mptrDriver->Shutdown();
    mptrDriver = NULL;
  }
  mnActiveDriver = eInvalidHandle;
  return eTrue;
}

///////////////////////////////////////////////
tU32 __stdcall cSoundFactory::GetActiveDriver() const
{
  return mnActiveDriver;
}

///////////////////////////////////////////////
iSoundDriverBuffer* __stdcall cSoundFactory::GetBuffer() const
{
  niCheckSilent(mptrDriver.IsOK(),NULL);
  return mptrDriver->GetBuffer();
}

///////////////////////////////////////////////
iSoundMixer* __stdcall cSoundFactory::GetMixer() const
{
  return mptrMixer;
}

///////////////////////////////////////////////
iSoundMixer3D* __stdcall cSoundFactory::GetMixer3D() const
{
  return mptrMixer3D;
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::Invalidate()
{
  // stop all audio playback and audio threads first
  ShutdownDriver();
  if (mptrDriver.IsOK()) {
    mptrDriver->Invalidate();
  }
  mptrMixer = NULL;
  mptrDriver = NULL;
  mptrSoundBufferMgr->Clear();
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cSoundFactory::ZeroMembers()
{
  mnActiveDriver = eInvalidHandle;
  mbMusicFadeOut = eFalse;
  mbMusicFadeIn = eFalse;
  mbMusicSpeed = eFalse;
  mbMusicVolume = eFalse;
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cSoundFactory::IsOK() const
{
  niClassIsOK(cSoundFactory);
  return eTrue;
}

///////////////////////////////////////////////
//! Load sound data from the specified file.
iSoundData* __stdcall cSoundFactory::LoadSoundData(iFile* apFile)
{
  niCheckIsOK(apFile,NULL);

  cString ext = _ASTR(apFile->GetSourcePath()).RAfter(".").ToLower();
  QPtr<iSoundDataLoader> ptrLoader = ni::GetLang()->GetGlobalInstance(niFmt("SoundDataLoader.%s",ext));
  if (!ptrLoader.IsOK()) {
    niError(niFmt("Can't find a sound data loader for file type '%s'.",ext));
    return NULL;
  }

  return ptrLoader->LoadSoundData(apFile);
}

///////////////////////////////////////////////
//! Create a sound buffer from the specified sound data.
iSoundBuffer* __stdcall cSoundFactory::CreateSoundBufferEx(iSoundData* apData, tBool abStream, iHString* ahspName)
{
  iSoundBuffer* pBuffer = NULL;
  if (HStringIsNotEmpty(ahspName)) {
    pBuffer = GetSoundBuffer(ahspName);
    if (niIsOK(pBuffer))
      return pBuffer;
  }

  if (abStream) {
    pBuffer = niNew cSoundBufferStream(mptrSoundBufferMgr, apData, ahspName, eFalse);
  }
  else {
    pBuffer = niNew cSoundBufferMem(mptrSoundBufferMgr, apData, ahspName);
  }
  if (!niIsOK(pBuffer))
  {
    niSafeRelease(pBuffer);
    niError(_A("Can't create new sound buffer."));
    return NULL;
  }

  return pBuffer;
}

///////////////////////////////////////////////
//! Create a sound buffer from the specified file.
//! \remark This method will load sound data internally.
iSoundBuffer* __stdcall cSoundFactory::CreateSoundBuffer(iFile* apFile, tBool abStream, iHString* ahspName)
{
  iSoundBuffer* pBuffer = NULL;
  if (HStringIsNotEmpty(ahspName)) {
    pBuffer = GetSoundBuffer(ahspName);
    if (niIsOK(pBuffer))
      return pBuffer;
  }

  Ptr<iSoundData> ptrData = LoadSoundData(apFile);
  if (!niIsOK(ptrData)) {
    niError(_A("Can't load sound data from the given file."));
    return NULL;
  }

  if (abStream) {
    pBuffer = niNew cSoundBufferStream(mptrSoundBufferMgr, ptrData, ahspName, eFalse);
  }
  else {
    pBuffer = niNew cSoundBufferMem(mptrSoundBufferMgr, ptrData, ahspName);
  }
  if (!niIsOK(pBuffer)) {
    niSafeRelease(pBuffer);
    niError(_A("Can't create new sound buffer."));
    return NULL;
  }

  return pBuffer;
}

///////////////////////////////////////////////
iSoundBuffer* __stdcall cSoundFactory::GetSoundBuffer(iHString* ahspName)
{
  return static_cast<iSoundBuffer*>(mptrSoundBufferMgr->GetFromName(ahspName));
}

///////////////////////////////////////////////
tU32 __stdcall cSoundFactory::FindFreeChannel(eSoundMode aMode) const
{
  CheckValid(eInvalidHandle);
  tU32 nStart = 0;
  tU32 nCount = mptrMixer->GetNumChannels();
  if (aMode != eSoundMode_Normal2D) {
    if (!mptrMixer3D.IsOK())
      return eInvalidHandle;
    nStart = mptrMixer3D->GetFirst3DChannel();
  }

  for (tU32 i = nStart; i < nCount; ++i) {
    if (!mptrMixer->GetIsChannelBusy(i))
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tU32 __stdcall cSoundFactory::FindFreeChannelPriority(eSoundMode aMode, tU32 anPriority) const
{
  CheckValid(eInvalidHandle);
  tU32 nStart = 0;
  tU32 nCount = mptrMixer->GetNumChannels();
  if (aMode != eSoundMode_Normal2D) {
    if (!mptrMixer3D.IsOK())
      return eInvalidHandle;
    nStart = mptrMixer3D->GetFirst3DChannel();
  }

  for (tU32 i = nStart; i < nCount; ++i) {
    if (!mptrMixer->GetIsChannelBusy(i))
      return i;
  }
  tU32 nLowestPriority = eInvalidHandle;
  tU32 nLowestIndex = eInvalidHandle;
  for (tU32 i = nStart; i < nCount; ++i) {
    if (mvChannels[i].nPriority < anPriority) {
      if (nLowestPriority >= mvChannels[i].nPriority) {
        nLowestPriority = mvChannels[i].nPriority;
        nLowestIndex = i;
      }
    }
  }
  return nLowestIndex;
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::SetListenerPosition(const sVec3f& aV)
{
  CheckValid3D(;);
  mptrMixer3D->SetListenerPosition(aV);
}

///////////////////////////////////////////////
sVec3f __stdcall cSoundFactory::GetListenerPosition() const
{
  CheckValid3D(sVec3f::Zero());
  return mptrMixer3D->GetListenerPosition();
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::SetListenerVelocity(const sVec3f& aV)
{
  CheckValid3D(;);
  mptrMixer3D->SetListenerVelocity(aV);
}

///////////////////////////////////////////////
sVec3f __stdcall cSoundFactory::GetListenerVelocity() const
{
  CheckValid3D(sVec3f::Zero());
  return mptrMixer3D->GetListenerVelocity();
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::SetListenerForward(const sVec3f& aV)
{
  CheckValid3D(;);
  mptrMixer3D->SetListenerForward(aV);
}

///////////////////////////////////////////////
sVec3f __stdcall cSoundFactory::GetListenerForward() const
{
  CheckValid3D(sVec3f::ZAxis());
  return mptrMixer3D->GetListenerForward();
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::SetListenerUp(const sVec3f& aV)
{
  CheckValid3D(;);
  mptrMixer3D->SetListenerUp(aV);
}

///////////////////////////////////////////////
sVec3f __stdcall cSoundFactory::GetListenerUp() const
{
  CheckValid3D(sVec3f::YAxis());
  return mptrMixer3D->GetListenerUp();
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::SetListenerDistanceScale(tF32 afScale)
{
  CheckValid3D(;);
  mptrMixer3D->SetListenerDistanceScale(afScale);
}

///////////////////////////////////////////////
tF32 __stdcall cSoundFactory::GetListenerDistanceScale() const
{
  CheckValid3D(0.0f);
  return mptrMixer3D->GetListenerDistanceScale();
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::SetListenerRolloffScale(tF32 afScale)
{
  CheckValid3D(;);
  mptrMixer3D->SetListenerDistanceScale(afScale);
}

///////////////////////////////////////////////
tF32 __stdcall cSoundFactory::GetListenerRolloffScale() const
{
  CheckValid3D(0.0f);
  return mptrMixer3D->GetListenerRolloffScale();
}

///////////////////////////////////////////////
void __stdcall cSoundFactory::SetListenerDopplerScale(tF32 afScale)
{
  CheckValid3D(;);
  mptrMixer3D->SetListenerDistanceScale(afScale);
}

///////////////////////////////////////////////
tF32 __stdcall cSoundFactory::GetListenerDopplerScale() const
{
  CheckValid3D(0.0f);
  return mptrMixer3D->GetListenerDopplerScale();
}

///////////////////////////////////////////////
ni::iSoundBuffer* __stdcall cSoundFactory::CreateSoundBufferFromRes(ni::iHString* ahspName, ni::tBool abStream) {
  ni::Ptr<ni::iSoundBuffer> ptrBuffer = GetSoundBuffer(ahspName);
  if (!ptrBuffer.IsOK()) {
    ni::Ptr<ni::iFile> ptrFile = ni::GetLang()->URLOpen(niHStr(ahspName));
    if (ptrFile.IsOK()) {
      ptrBuffer = CreateSoundBuffer(ptrFile,abStream,ahspName);
      if (!ptrBuffer.IsOK()) {
        niWarning(niFmt(_A("Can't create sound buffer '%s'."),niHStr(ahspName)));
      }
    }
    else {
      niWarning(niFmt(_A("Can't open sound file '%s'."),niHStr(ahspName)));
    }
  }
  return ptrBuffer.GetRawAndSetNull();
}

///////////////////////////////////////////////
ni::tBool __stdcall cSoundFactory::MusicPlay(iSoundSource* apSoundSource, ni::tF32 afVolume, ni::tF32 afSpeed, ni::tF32 afBlendTime)
{
  ni::Ptr<ni::iSoundSource> ptrSource = apSoundSource;
  if (!ptrSource.IsOK()) {
    return ni::eFalse;
  }
  if (mptrMusicNext.IsOK() && mptrMusicNext->GetIsPlaying()) {
    mptrMusicNext->Stop();
  }

  mptrMusicNext = ptrSource;
  mptrMusicNext->SetMode(ni::eSoundMode_Normal2D);
  mptrMusicNext->SetVolume(0.0f);
  mptrMusicNext->SetSpeed(afSpeed);
  if (mptrMusicCurrent.IsOK()) {
    mbMusicFadeOut = ni::eTrue;
    mMusicFadeOut.SetValue(mptrMusicCurrent->GetVolume());
    mMusicFadeOut.SetTarget(0.0f,afBlendTime);
  }
  else {
    mbMusicFadeOut = ni::eFalse;
  }
  mbMusicFadeIn = ni::eTrue;
  mMusicFadeIn.SetValue(0.0f);
  mMusicFadeIn.SetTarget(afVolume,afBlendTime);
  return ni::eTrue;
}

///////////////////////////////////////////////
ni::tBool __stdcall cSoundFactory::MusicStop(ni::tF32 afBlendTime)
{
  if (mptrMusicCurrent.IsOK()) {
    niDebugFmt(("... MusicStop: %g -> %g", mptrMusicCurrent->GetVolume(), 0));
    mbMusicFadeOut = ni::eTrue;
    mMusicFadeOut.SetValue(mptrMusicCurrent->GetVolume());
    mMusicFadeOut.SetTarget(0.0f,afBlendTime);
  }
  return ni::eTrue;
}

///////////////////////////////////////////////
ni::tBool __stdcall cSoundFactory::MusicSpeed(ni::tF32 afSpeed, ni::tF32 afBlendTime)
{
  if (mptrMusicCurrent.IsOK()) {
    mbMusicSpeed = ni::eTrue;
    mMusicNewSpeed.SetValue(mptrMusicCurrent->GetSpeed());
    mMusicNewSpeed.SetTarget(afSpeed,afBlendTime);
  }
  return ni::eTrue;
}

///////////////////////////////////////////////
ni::tBool __stdcall cSoundFactory::MusicVolume(ni::tF32 afVolume, ni::tF32 afBlendTime)
{
  if (mptrMusicCurrent.IsOK()) {
    mbMusicVolume = ni::eTrue;
    mMusicNewVolume.SetValue(mptrMusicCurrent->GetVolume());
    mMusicNewVolume.SetTarget(afVolume,afBlendTime);
  }
  return ni::eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cSoundFactory::Update(ni::tF64 afUpdateTime)
{
  if (!mptrDriver.IsOK())
    return eFalse;

  if (mbMusicFadeOut) {
    mMusicFadeOut.Update(afUpdateTime);
    mptrMusicCurrent->SetVolume(mMusicFadeOut.GetValue());
    if (mMusicFadeOut.GetHasReachedTarget()) {
      mbMusicFadeOut = ni::eFalse;
      mptrMusicCurrent->Stop();
      mptrMusicCurrent = NULL;
    }
    // niDebugFmt(("... MusicFadeOut: %g, %g", afUpdateTime, mMusicFadeOut.GetValue()));
  }

  if (mbMusicFadeIn) {
    mMusicFadeIn.Update(afUpdateTime);
    mptrMusicNext->SetVolume(mMusicFadeIn.GetValue());
    if (!mptrMusicNext->GetIsPlaying()) {
      mptrMusicNext->Play();
    }
    if (mMusicFadeIn.GetHasReachedTarget()) {
      mbMusicFadeOut = ni::eFalse;
      mbMusicFadeIn = ni::eFalse;
      mptrMusicCurrent = mptrMusicNext;
      mptrMusicNext = NULL;
    }
  }

  if (mbMusicVolume) {
    if (!mptrMusicCurrent.IsOK()) {
      mbMusicVolume = ni::eFalse;
    }
    else {
      mMusicNewVolume.Update(afUpdateTime);
      mptrMusicCurrent->SetVolume(mMusicNewVolume.GetValue());
      if (mMusicNewVolume.GetHasReachedTarget()) {
        mbMusicVolume = ni::eFalse;
      }
    }
  }
  if (mbMusicSpeed) {
    if (!mptrMusicCurrent.IsOK()) {
      mbMusicSpeed = ni::eFalse;
    }
    else {
      mMusicNewSpeed.Update(afUpdateTime);
      mptrMusicCurrent->SetSpeed(mMusicNewSpeed.GetValue());
      if (mMusicNewSpeed.GetHasReachedTarget()) {
        mbMusicSpeed = ni::eFalse;
      }
    }
  }

  mptrDriver->Update();
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cSoundFactory::SwitchIn() {
  if (mptrMixer.IsOK()) {
    mptrMixer->SwitchIn();
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cSoundFactory::SwitchOut() {
  if (mptrMixer.IsOK()) {
    mptrMixer->SwitchOut();
  }
  return eTrue;
}

///////////////////////////////////////////////
iSoundSource* __stdcall cSoundFactory::CreateSoundSource(iSoundBuffer* apBuffer)
{
  niCheckIsOK(apBuffer,NULL);
  return niNew cSoundSource(this,apBuffer);
}

///////////////////////////////////////////////
tBool __stdcall cSoundFactory::_PlaySoundSource(iSoundSource* apSource, tU32 anChannel, tU32 anPriority)
{
  CheckValid(eFalse);
  niCheckSilent(niIsOK(apSource),eFalse);
  niCheckSilent(anChannel < mvChannels.size(),eFalse);
  mptrMixer->StopChannel(anChannel);
  if (!((cSoundSource*)(apSource))->SetChannel(mptrMixer,mptrMixer3D,anChannel)) {
    return eFalse;
  }
  mvChannels[anChannel].nPriority = anPriority;
  mptrMixer->StartChannel(anChannel);
  return eTrue;
}

niExportFunc(iUnknown*) New_niSound_SoundFactory(const Var&, const Var&) {
  return niNew cSoundFactory();
}
