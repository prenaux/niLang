// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "SoundBufferStream.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundBufferStream implementation.

///////////////////////////////////////////////
cSoundBufferStream::cSoundBufferStream(iDeviceResourceManager* apRM, iSoundData* apData, iHString* ahspName, tBool abInstance)
{

  ZeroMembers();

  if (!niIsOK(apData)) {
    niError(_A("Invalid sound data."));
    return;
  }

  mptrData = apData->Clone();
  if (!mptrData.IsOK()) {
    niError(_A("Can't clone sound data."));
    return;
  }

  mbInstance = abInstance;
  if (abInstance) {
    mhspName = _H(niFmt(_A("%s_I%p"),niHStr(ahspName),this));
  }
  else {
    mhspName = ahspName;
  }
  mpRM = apRM;
  mpRM->Register(this);
  ResetPosition();
}

///////////////////////////////////////////////
cSoundBufferStream::~cSoundBufferStream()
{
  Invalidate();
}

///////////////////////////////////////////////
void cSoundBufferStream::Invalidate()
{
  if (mpRM) {
    if (mptrData.IsOK()) {
      mptrData->Invalidate();
      mptrData = NULL;
    }
    mpRM->Unregister(this);
    mpRM = NULL;
  }
}

///////////////////////////////////////////////
//! Get the resource's name.
iHString* __stdcall cSoundBufferStream::GetDeviceResourceName() const
{
  return mhspName.IsOK()?mhspName.ptr():(iHString*)NULL;
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cSoundBufferStream::ZeroMembers()
{
  mptrData = NULL;
  mpRM = NULL;
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cSoundBufferStream::IsOK() const
{
  niClassIsOK(cSoundBufferStream);
  return mptrData.IsOK();
}

///////////////////////////////////////////////
iSoundBuffer* __stdcall cSoundBufferStream::CreateInstance()
{
  return niNew cSoundBufferStream(mpRM,mptrData,NULL,eTrue);
}

///////////////////////////////////////////////
tBool __stdcall cSoundBufferStream::GetIsInstance() const
{
  return mbInstance;
}

///////////////////////////////////////////////
//! Return eTrue if the sound buffer is streamed.
tBool __stdcall cSoundBufferStream::GetIsStreamed() const
{
  return eTrue;
}

///////////////////////////////////////////////
eSoundFormat __stdcall cSoundBufferStream::GetFormat() const
{
  if (!mptrData.IsOK()) return eSoundFormat_Unknown;
  return mptrData->GetFormat();
}

///////////////////////////////////////////////
tU32 __stdcall cSoundBufferStream::GetFrequency() const
{
  if (!mptrData.IsOK()) return 0;
  return mptrData->GetFrequency();
}

///////////////////////////////////////////////
void __stdcall cSoundBufferStream::ResetPosition()
{
  if (mptrData.IsOK())
    mptrData->Reset();
}

///////////////////////////////////////////////
tBool __stdcall cSoundBufferStream::ReadRaw(tPtr apOut, tU32 anBytes, tBool abLoop)
{
  if (!mptrData.IsOK()) return eFalse;
  tSize read = 0;
  while (read < anBytes) {
    tU32 nNumRead = mptrData->ReadRaw(apOut+read,anBytes-read);
    read += nNumRead;
    if (nNumRead == 0) {
      if (abLoop) {
        mptrData->Reset();
      }
      else {
        if (anBytes-read)
          memset(apOut+read,0,anBytes-read);
        return eFalse;
      }
    }
  }
  return eTrue;
}
