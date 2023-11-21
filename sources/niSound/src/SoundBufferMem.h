#ifndef __SOUNDBUFFERMEM_31974512_H__
#define __SOUNDBUFFERMEM_31974512_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundBufferMem declaration.

//! Sound buffer implementation.
class cSoundBufferMem : public cIUnknownImpl<iSoundBuffer>
{
  niBeginClass(cSoundBufferMem);

 public:
  //! Constructor.
  cSoundBufferMem(iDeviceResourceManager* apRM, iSoundData* apData, iHString* ahspName);
  //! Destructor.
  ~cSoundBufferMem();

  //! Sanity check.
  tBool __stdcall IsOK() const;
  //! Invalidate.
  void __stdcall Invalidate();

  //// iSoundBuffer /////////////////////////////
  iSoundBuffer* __stdcall CreateInstance();
  tBool __stdcall GetIsInstance() const;
  iHString* __stdcall GetDeviceResourceName() const;
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }
  iDeviceResource* __stdcall Bind(iUnknown*) { return this; }
  tBool __stdcall GetIsStreamed() const;
  iSoundData* __stdcall GetStreamSoundData() const { return NULL; }
  eSoundFormat __stdcall GetFormat() const;
  tU32 __stdcall GetFrequency() const;
  void __stdcall ResetPosition();
  tBool __stdcall ReadRaw(tPtr apOut, tU32 anBytes, tBool abLoop);
  //// iSoundBuffer /////////////////////////////

  tBool __stdcall InstanceRead(tSize& anPos, tPtr apOut, tU32 anBytes, tBool abLoop);

 private:
  iDeviceResourceManager* mpRM;
  tHStringPtr   mhspName;
  eSoundFormat  mFormat;
  tU32      mnFrequency;
  Ptr<iFile>    mptrFile;

  niEndClass(cSoundBufferMem);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SOUNDBUFFERMEM_31974512_H__
