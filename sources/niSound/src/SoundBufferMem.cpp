// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "SoundBufferMem.h"

#pragma niTodo("Mix the two stereo channels in a mono channel for 3d output, add a 3d sound flag to the creation method.")
#pragma niTodo("3D Sounds works only with mono sounds.")

//! Sound buffer instance implementation.
class cSoundBufferMemInstance : public cIUnknownImpl<iSoundBuffer>
{
  niBeginClass(cSoundBufferMemInstance);

 public:
  //! Constructor.
  cSoundBufferMemInstance(iDeviceResourceManager* apRM, cSoundBufferMem* apBuffer) {
    ZeroMembers();
    mptrBase = apBuffer;
    mhspName = _H(niFmt(_A("%s_I%p"),niHStr(apBuffer->GetDeviceResourceName()),this));
    mpRM = apRM;
    mpRM->Register(this);
  }
  //! Destructor.
  ~cSoundBufferMemInstance() {
    Invalidate();
  }

  //! Zeros all the class members.
  void ZeroMembers() {
    mnPos = 0;
    mpRM = NULL;
  }

  //! Sanity check.
  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundBufferMemInstance);
    return eTrue;
  }

  //! Invalidate.
  void __stdcall Invalidate() {
    if (mpRM) {
      mptrBase = NULL;
      mpRM->Unregister(this);
      mpRM = NULL;
    }
  }

  //// iSoundBuffer /////////////////////////////
  iSoundBuffer* __stdcall CreateInstance() {
    if (!mptrBase.IsOK())
      return NULL;
    return niNew cSoundBufferMemInstance(mpRM,mptrBase);
  }
  tBool __stdcall GetIsInstance() const {
    return eTrue;
  }
  iHString* __stdcall GetDeviceResourceName() const {
    return mhspName;
  }
  iDeviceResource* __stdcall Bind(iUnknown*) { return this; }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }
  tBool __stdcall GetIsStreamed() const {
    if (!mptrBase.IsOK()) return eFalse;
    return mptrBase->GetIsStreamed();
  }
  iSoundData* __stdcall GetStreamSoundData() const {
    return NULL;
  }
  eSoundFormat __stdcall GetFormat() const {
    if (!mptrBase.IsOK()) return eSoundFormat_Unknown;
    return mptrBase->GetFormat();
  }
  tU32 __stdcall GetFrequency() const {
    if (!mptrBase.IsOK()) return 0;
    return mptrBase->GetFrequency();
  }
  void __stdcall ResetPosition() {
    mnPos = 0;
  }
  tBool __stdcall ReadRaw(tPtr apOut, tU32 anBytes, tBool abLoop) {
    if (!mptrBase.IsOK()) return eFalse;
    return mptrBase->InstanceRead(mnPos,apOut,anBytes,abLoop);
  }
  //// iSoundBuffer /////////////////////////////

 private:
  tHStringPtr       mhspName;
  iDeviceResourceManager* mpRM;
  Ptr<cSoundBufferMem>  mptrBase;
  tSize mnPos;

  niEndClass(cSoundBufferMemInstance);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundBufferMem implementation.

///////////////////////////////////////////////
cSoundBufferMem::cSoundBufferMem(iDeviceResourceManager* apRM, iSoundData* apData, iHString* ahspName)
{
  mFormat = eSoundFormat_Unknown;
  mnFrequency = 0;
  mpRM = NULL;
  if (!niIsOK(apData)) {
    niError(_A("Invalid sound data."));
    return;
  }

  mFormat = apData->GetFormat();
  mnFrequency = apData->GetFrequency();

  apData->Reset();
  //
  // Note: We use a astl::vector<> because a 64K buffer on the stack in
  // jscc/emscripten/wasm causes a stack overwrite. Seems the stack allocation
  // size isn't always honored :|
  //
  // Emscripten default stack size has been changed to 64K:
  // https://www.mail-archive.com/emscripten-discuss%40googlegroups.com/msg10104.html
  //
  // Compiler doesn't warn of this issue, yey.
  //
  // Note that I changed the emscripten stack size back to 512K using '-s
  // STACK_SIZE=524288' as linker flag. Still this code is better, so I've
  // kept the updated version that doesnt use the stack for the buffer.
  //
  astl::vector<tU8> vBuffer;
  vBuffer.resize(0xFFFF);
  mptrFile = ni::CreateFileDynamicMemory(0,NULL);
  tU32 numRead = 0;
  while (1) {
    numRead = apData->ReadRaw(vBuffer.data(), vBuffer.size());
    if (numRead == 0)
      break;
    mptrFile->WriteRaw(vBuffer.data(),numRead);
  }

  mhspName = ahspName;
  mpRM = apRM;
  mpRM->Register(this);
  ResetPosition();
}

///////////////////////////////////////////////
cSoundBufferMem::~cSoundBufferMem()
{
  Invalidate();
}

///////////////////////////////////////////////
void cSoundBufferMem::Invalidate()
{
  if (mpRM) {
    if (mptrFile.IsOK()) {
      mptrFile->Invalidate();
      mptrFile = NULL;
    }
    mpRM->Unregister(this);
    mpRM = NULL;
  }
}

///////////////////////////////////////////////
//! Get the resource's name.
iHString* __stdcall cSoundBufferMem::GetDeviceResourceName() const
{
  return mhspName.IsOK() ? mhspName.ptr() : (iHString*)NULL;
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cSoundBufferMem::IsOK() const
{
  niClassIsOK(cSoundBufferMem);
  return mptrFile.IsOK();
}

///////////////////////////////////////////////
iSoundBuffer* __stdcall cSoundBufferMem::CreateInstance()
{
  return niNew cSoundBufferMemInstance(mpRM,this);
}

///////////////////////////////////////////////
tBool __stdcall cSoundBufferMem::GetIsInstance() const
{
  return eFalse;
}

///////////////////////////////////////////////
//! Return eTrue if the sound buffer is streamed.
tBool __stdcall cSoundBufferMem::GetIsStreamed() const
{
  return eFalse;
}

///////////////////////////////////////////////
eSoundFormat __stdcall cSoundBufferMem::GetFormat() const
{
  return mFormat;
}

///////////////////////////////////////////////
tU32 __stdcall cSoundBufferMem::GetFrequency() const
{
  return mnFrequency;
}

///////////////////////////////////////////////
void __stdcall cSoundBufferMem::ResetPosition()
{
  if (mptrFile.IsOK())
    mptrFile->SeekSet(0);
}

///////////////////////////////////////////////
tBool __stdcall cSoundBufferMem::ReadRaw(tPtr apOut, tU32 anBytes, tBool abLoop)
{
  if (!mptrFile.IsOK())
    return eFalse;

  //niPrintln(niFmt(_A("BUFFER READ: %d, %d\n"),anBytes,mptrFile->Tell()));
  tSize read = 0;
  while (read < anBytes) {
    read += mptrFile->ReadRaw(apOut+read,anBytes-read);
    if (mptrFile->GetPartialRead()) {
      if (abLoop) {
        mptrFile->SeekSet(0);
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

///////////////////////////////////////////////
tBool __stdcall cSoundBufferMem::InstanceRead(tSize& anPos, tPtr apOut, tU32 anBytes, tBool abLoop)
{
  if (!mptrFile.IsOK())
    return eFalse;

  //niPrintln(niFmt(_A("BUFFER INST READ: %d, %d\n"),anBytes,anPos));
  tSize read = 0;
  tSize size = mptrFile->GetSize();
  while (read < anBytes) {
    mptrFile->SeekSet(anPos);
    read += mptrFile->ReadRaw(apOut+read,anBytes-read);
    anPos += read;
    if (anPos > size) {
      if (abLoop) {
        anPos = 0;
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
