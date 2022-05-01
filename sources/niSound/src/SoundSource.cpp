// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "SoundFactory.h"
#include "SoundSource.h"

//#define STREAMING_TRACE

#define SETMIXER(VAR,VAL) {                     \
    tU32 nChannel = GetChannel();               \
    if (nChannel != eInvalidHandle) {           \
      mptrMixer->SetChannel##VAR(nChannel,VAL); \
    }                                           \
  }

#define SETMIXER3D(VAR,VAL) {                                           \
    tU32 nChannel = GetChannel();                                       \
    if (nChannel != eInvalidHandle && niFlagIs(mFlags,eSoundSourceFlags_Channel3D)) { \
      mptrMixer3D->SetChannel##VAR(nChannel,VAL);                       \
    }                                                                   \
  }

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundSource implementation.

///////////////////////////////////////////////
cSoundSource::cSoundSource(cSoundFactory* apSoundF, ni::iSoundBuffer* apBuffer)
{
  mMode = eSoundMode_Normal2D;
  mnChannel = eInvalidHandle;
  mFlags = 0;
  mnPriority = 1;
  mptrSoundFactory = apSoundF;
  SetSoundBuffer(apBuffer);
  SetMode(eSoundMode_Normal2D);
  SetPosition(sVec3f::Zero());
  SetVelocity(sVec3f::Zero());
  SetMinDistance(10.0f);
  SetMaxDistance(1000.0f);
  SetVolume(1.0f);
  SetSpeed(1.0f);
  SetPan(0.0f);
  SetLoop(eFalse);
}

///////////////////////////////////////////////
cSoundSource::~cSoundSource()
{
  Invalidate();
}

///////////////////////////////////////////////
tBool __stdcall cSoundSource::SetSoundBuffer(iSoundBuffer* apBuffer)
{
  if (mptrBuffer == apBuffer)
    return eTrue;
  Stop();
  if (niIsOK(apBuffer)) {
    mptrBuffer = apBuffer;
    SetSpeed(mfSpeed);
  }
  else {
    mptrBuffer = NULL;
  }
  return eTrue;
}

///////////////////////////////////////////////
iSoundBuffer* __stdcall cSoundSource::GetSoundBuffer() const
{
  return mptrBuffer;
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cSoundSource::IsOK() const
{
  niClassIsOK(cSoundSource);
  return eTrue;
}

///////////////////////////////////////////////
//! Invalidate
void __stdcall cSoundSource::Invalidate()
{
  if (!mptrMixer.IsOK())
    return;

  if (mnChannel != eInvalidHandle)
    Stop();

  SetSoundBuffer(NULL);
  mptrBuffer = NULL;
  mptrMixer3D = NULL;
  mptrMixer = NULL;
}

///////////////////////////////////////////////
//! Get the sound source's name.
iHString* cSoundSource::GetDeviceResourceName() const
{
  return NULL;
}

///////////////////////////////////////////////
void __stdcall cSoundSource::SetMode(eSoundMode aMode)
{
  mMode = aMode;
}

///////////////////////////////////////////////
eSoundMode __stdcall cSoundSource::GetMode() const
{
  return mMode;
}

///////////////////////////////////////////////
//! Set the source's position.
void __stdcall cSoundSource::SetPosition(const sVec3f& avPos)
{
  mvPosition = avPos;
  SETMIXER3D(Position,mvPosition);
}

///////////////////////////////////////////////
//! Get the source's position.
sVec3f __stdcall cSoundSource::GetPosition() const
{
  return mvPosition;
}

///////////////////////////////////////////////
//! Set the source's velocity.
void __stdcall cSoundSource::SetVelocity(const sVec3f& avVel)
{
  mvVelocity = avVel;
  SETMIXER3D(Velocity,mvVelocity);
}

///////////////////////////////////////////////
//! Get the source's velocity.
sVec3f __stdcall cSoundSource::GetVelocity() const
{
  return mvVelocity;
}

///////////////////////////////////////////////
//! Set the minimum audible distance.
void __stdcall cSoundSource::SetMinDistance(tF32 afMin)
{
  mfMinDistance = afMin;
  SETMIXER3D(MinDistance,mfMinDistance);
}

///////////////////////////////////////////////
//! Get the minimum audible distance.
tF32 __stdcall cSoundSource::GetMinDistance() const
{
  return mfMinDistance;
}

///////////////////////////////////////////////
//! Set the maximum audible distance.
void __stdcall cSoundSource::SetMaxDistance(tF32 afMax)
{
  mfMaxDistance = afMax;
  SETMIXER3D(MaxDistance,mfMaxDistance);
}

///////////////////////////////////////////////
//! Get the maximum audible distance.
tF32 __stdcall cSoundSource::GetMaxDistance() const
{
  return mfMaxDistance;
}

///////////////////////////////////////////////
void __stdcall cSoundSource::SetPan(tF32 afPan) {
  mfPan = ni::Clamp(afPan,-1.0f,1.0f);
  SETMIXER(Pan,int(mfPan*0.5f*255.0f)+128);
}
tF32 __stdcall cSoundSource::GetPan() const {
  return mfPan;
}

///////////////////////////////////////////////
//! Set the sound volume.
void __stdcall cSoundSource::SetVolume(tF32 afVolume)
{
  mfVolume = afVolume;
  SETMIXER(Volume,int(afVolume*255.0f));
}

///////////////////////////////////////////////
//! Get the sound volume.
tF32 __stdcall cSoundSource::GetVolume() const
{
  return mfVolume;
}

///////////////////////////////////////////////
//! Set the sound pitch.
void __stdcall cSoundSource::SetSpeed(tF32 afSpeed)
{
  mfSpeed  = afSpeed;
  if (mptrBuffer.IsOK()) {
    SETMIXER(Frequency,tI32(mptrBuffer->GetFrequency()*mfSpeed));
  }
}

///////////////////////////////////////////////
//! Get the sound pitch.
tF32 __stdcall cSoundSource::GetSpeed() const
{
  return mfSpeed;
}

///////////////////////////////////////////////
//! Set the source in loop mode.
//! \remark eFalse by default.
void __stdcall cSoundSource::SetLoop(tBool abLoop)
{
  niFlagOnIf(mFlags,eSoundSourceFlags_Loop, abLoop);
  SETMIXER(Loop,niFlagTest(mFlags, eSoundSourceFlags_Loop));
}

///////////////////////////////////////////////
//! Get the source loop mode.
tBool __stdcall cSoundSource::GetLoop() const
{
  return niFlagTest(mFlags, eSoundSourceFlags_Loop);
}

///////////////////////////////////////////////
//! Starts playing the sound.
tBool __stdcall cSoundSource::Play()
{
  niFlagOff(mFlags,eSoundSourceFlags_Paused); // one way or another wont be in paused state anymore

  tU32 nChannel = GetChannel();
  if (nChannel != eInvalidHandle) {
    if (niFlagIs(mFlags,eSoundSourceFlags_Paused)) {
      // just unpause...
      SETMIXER(Pause,eFalse);
      return eTrue;
    }
    Stop();
  }
  else {
    nChannel = mptrSoundFactory->FindFreeChannelPriority(mMode,mnPriority);
  }

  if (!mptrSoundFactory->_PlaySoundSource(this,nChannel,mnPriority)) {
    return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
//! Stops playing the sound.
tBool __stdcall cSoundSource::Stop()
{
  tU32 nChannel = GetChannel();
  if (nChannel != eInvalidHandle) {
    mptrMixer->StopChannel(nChannel);
  }
  niFlagOff(mFlags,eSoundSourceFlags_Paused);
  return eTrue;
}

///////////////////////////////////////////////
//! Pause the sound.
tBool __stdcall cSoundSource::Pause()
{
  mFlags |= eSoundSourceFlags_Paused;
  SETMIXER(Pause,eTrue);
  return eTrue;
}

///////////////////////////////////////////////
//! Check if the sound is currently paused.
tBool __stdcall cSoundSource::GetIsPaused() const
{
  return niFlagTest(mFlags, eSoundSourceFlags_Paused);
}

///////////////////////////////////////////////
//! Check if the sound is currently paused.
tBool __stdcall cSoundSource::GetIsPlaying() const
{
  tU32 nChannel = GetChannel();
  if (nChannel == eInvalidHandle) {
    return eFalse;
  }
  else {
    return eTrue;
  }
}

///////////////////////////////////////////////
tBool cSoundSource::SetChannel(iSoundMixer* apMixer, iSoundMixer3D* apMixer3D, tU32 anChannel)
{
  niCheckSilent(apMixer,eFalse);
  niCheckSilent(mptrBuffer.IsOK(),eFalse);

  mptrBuffer->ResetPosition();
  if (!apMixer->SetChannelBuffer(anChannel,mptrBuffer)) {
    mnChannel = eInvalidHandle;
    mptrMixer = NULL;
    mptrMixer3D = apMixer3D;
    return eFalse;
  }
  else {
    mnChannel = anChannel;
    mptrMixer = apMixer;
  }

  mptrMixer->SetChannelVolume(anChannel, int(mfVolume*255.0f));
  mptrMixer->SetChannelFrequency(anChannel, int(mfSpeed*mptrBuffer->GetFrequency()));
  mptrMixer->SetChannelLoop(anChannel, niFlagIs(mFlags,eSoundSourceFlags_Loop));
  mptrMixer->SetChannelPause(anChannel, niFlagIs(mFlags,eSoundSourceFlags_Paused));
  mptrMixer->SetChannelUserID(anChannel, tIntPtr(this));
  if (apMixer3D && apMixer3D->GetIsChannel3D(anChannel)) {
    mptrMixer3D = apMixer3D;
    niFlagOn(mFlags,eSoundSourceFlags_Channel3D);
    mptrMixer3D->SetChannelMode(anChannel,mMode);
    mptrMixer3D->SetChannelPosition(anChannel,mvPosition);
    mptrMixer3D->SetChannelVelocity(anChannel,mvVelocity);
    mptrMixer3D->SetChannelMinDistance(anChannel,mfMinDistance);
    mptrMixer3D->SetChannelMaxDistance(anChannel,mfMaxDistance);
  }
  else {
    mptrMixer3D = NULL;
    niFlagOff(mFlags,eSoundSourceFlags_Channel3D);
    mptrMixer->SetChannelPan(anChannel, int(mfPan*0.5f*255.0f)+128);
  }
  return eTrue;
}

///////////////////////////////////////////////
tU32 cSoundSource::GetChannel() const
{
  if (mnChannel == eInvalidHandle || !mptrMixer.IsOK())
    return eInvalidHandle;
  if ((mptrMixer->GetChannelUserID(mnChannel) != tIntPtr(this)) ||
      !mptrMixer->GetIsChannelBusy(mnChannel))
  {
    niThis(cSoundSource)->mnChannel = eInvalidHandle;
  }
  return mnChannel;
}

///////////////////////////////////////////////
ni::tBool __stdcall cSoundSource::Play2D(ni::tF32 afVolume, ni::tF32 afSpeed, ni::tF32 afPan)
{
  if (!mptrBuffer.IsOK()) return ni::eFalse;
  Stop();
  this->SetMode(ni::eSoundMode_Normal2D);
  this->SetPan(afPan);
  this->SetVolume(afVolume);
  this->SetSpeed(afSpeed);
  this->Play();
  return ni::eTrue;
}

///////////////////////////////////////////////
ni::tBool __stdcall cSoundSource::Play3D(ni::tF32 afVolume, ni::tF32 afSpeed, ni::sVec3f avPosition, ni::tBool abListenerRelative, ni::tF32 afMinDistance, ni::tF32 afMaxDistance)
{
  if (!mptrBuffer.IsOK()) return ni::eFalse;
  Stop();
  this->SetMode(abListenerRelative?ni::eSoundMode_Relative3D:ni::eSoundMode_Normal3D);
  this->SetPosition(avPosition);
  this->SetMinDistance(afMinDistance);
  this->SetMaxDistance(afMaxDistance);
  this->SetVolume(afVolume);
  this->SetSpeed(afSpeed);
  this->Play();
  return ni::eTrue;
}
