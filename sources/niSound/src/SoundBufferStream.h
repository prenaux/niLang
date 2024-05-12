#ifndef __SOUNDBUFFERSTREAM_87914654_H__
#define __SOUNDBUFFERSTREAM_87914654_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundBufferStream declaration.

//! Sound buffer implementation.
class cSoundBufferStream : public ImplRC<iSoundBuffer>
{
  niBeginClass(cSoundBufferStream);

 public:
  //! Constructor.
  cSoundBufferStream(iDeviceResourceManager* apRM, iSoundData* apData, iHString* ahspName, tBool abInstance);
  //! Destructor.
  ~cSoundBufferStream();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;
  //! Invalidate.
  void __stdcall Invalidate();

  //// iSoundBuffer /////////////////////////////
  iSoundBuffer* __stdcall CreateInstance();
  tBool __stdcall GetIsInstance() const;
  iHString* __stdcall GetDeviceResourceName() const;
  iDeviceResource* __stdcall Bind(iUnknown*) { return this; }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }
  tBool __stdcall GetIsStreamed() const;
  iSoundData* __stdcall GetStreamSoundData() const { return mptrData; }
  eSoundFormat __stdcall GetFormat() const;
  tU32 __stdcall GetFrequency() const;
  void __stdcall ResetPosition();
  tBool __stdcall ReadRaw(tPtr apOut, tU32 anBytes, tBool abLoop);
  //// iSoundBuffer /////////////////////////////

 private:
  iDeviceResourceManager* mpRM;
  tHStringPtr       mhspName;
  Ptr<iSoundData> mptrData;
  tBool         mbInstance;

  niEndClass(cSoundBufferStream);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SOUNDBUFFERSTREAM_87914654_H__
