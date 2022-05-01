#ifndef __SOUNDSOURCE_11059727_H__
#define __SOUNDSOURCE_11059727_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

enum eSoundSourceFlags
{
  eSoundSourceFlags_Loop = niBit(0),
  eSoundSourceFlags_Paused = niBit(1),
  eSoundSourceFlags_Channel3D = niBit(2),
};

typedef tU32  tSoundSourceFlags;

#define STMBUFFER_SIZE    (4096*4)
#define STMBUFFER_COUNT   (4)
#define STMBUFFER_UPDATEMINTIME (100)

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundSource declaration.

//! Sound source implementation.
class cSoundSource : public cIUnknownImpl<iSoundSource>
{
  niBeginClass(cSoundSource);

 public:
  //! Constructor.
  cSoundSource(cSoundFactory* apSoundF, ni::iSoundBuffer* apBuffer);
  //! Destructor.
  ~cSoundSource();

  //! Sanity check.
  tBool __stdcall IsOK() const;
  //! Invalidate
  void __stdcall Invalidate();

  //// iSoundSource /////////////////////////////
  //! Get the sound buffer.
  tBool __stdcall SetSoundBuffer(iSoundBuffer* apBuffer);
  iSoundBuffer* __stdcall GetSoundBuffer() const;
  iHString* __stdcall GetDeviceResourceName() const;
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }
  iDeviceResource* __stdcall Bind(iUnknown*) { return this; }
  void __stdcall SetMode(eSoundMode aMode);
  eSoundMode __stdcall GetMode() const;
  void __stdcall SetPosition(const sVec3f& avPos);
  sVec3f __stdcall GetPosition() const;
  void __stdcall SetVelocity(const sVec3f& avVel);
  sVec3f __stdcall GetVelocity() const;
  void __stdcall SetMinDistance(tF32 afMin);
  tF32 __stdcall GetMinDistance() const;
  void __stdcall SetMaxDistance(tF32 afMax);
  tF32 __stdcall GetMaxDistance() const;
  void __stdcall SetPan(tF32 afPan);
  tF32 __stdcall GetPan() const;
  void __stdcall SetVolume(tF32 afVolume);
  tF32 __stdcall GetVolume() const;
  void __stdcall SetSpeed(tF32 afSpeed);
  tF32 __stdcall GetSpeed() const;
  void __stdcall SetLoop(tBool abLoop);
  tBool __stdcall GetLoop() const;
  tBool __stdcall Play();
  tBool __stdcall Stop();
  tBool __stdcall Pause();
  tBool __stdcall GetIsPaused() const;
  tBool __stdcall GetIsPlaying() const;
  ni::tBool __stdcall Play2D(ni::tF32 afVolume, ni::tF32 afSpeed, ni::tF32 afPan);
  ni::tBool __stdcall Play3D(ni::tF32 afVolume, ni::tF32 afSpeed, ni::sVec3f avPosition, ni::tBool abListenerRelative, ni::tF32 afMinDistance, ni::tF32 afMaxDistance);
  //// iSoundSource /////////////////////////////

  tBool SetChannel(iSoundMixer* apMixer, iSoundMixer3D* apMixer3D, tU32 anChannel);
  tU32 GetChannel() const;
  void UpdateChannel();

 private:
  Ptr<cSoundFactory>  mptrSoundFactory;
  Ptr<iSoundMixer>    mptrMixer;
  Ptr<iSoundMixer3D>  mptrMixer3D;
  Ptr<iSoundBuffer>   mptrBuffer;

  tU32    mnChannel;
  tU32    mnPriority;

  eSoundMode mMode;
  sVec3f mvPosition;
  sVec3f mvVelocity;
  tF32    mfAttFactory;
  tF32    mfMinDistance;
  tF32    mfMaxDistance;
  tF32    mfPan;
  tF32    mfVolume;
  tF32    mfSpeed;
  tSoundSourceFlags mFlags;

  niEndClass(cSoundSource);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SOUNDSOURCE_11059727_H__
