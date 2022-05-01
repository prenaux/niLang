#ifndef __SOUNDFACTORY_30524441_H__
#define __SOUNDFACTORY_30524441_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Utils/AnimatedVariable.h>

class cSoundSource;

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundFactory declaration.

//! Sound factory implementation.
class cSoundFactory : public cIUnknownImpl<iSoundFactory>
{
  niBeginClass(cSoundFactory);

  typedef astl::list<cSoundSource*> tSoundSourceLst;
  typedef tSoundSourceLst::iterator tSoundSourceLstIt;
  typedef tSoundSourceLst::const_iterator tSoundSourceLstCIt;

 public:
  //! Constructor.
  cSoundFactory();
  //! Destructor.
  ~cSoundFactory();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  void __stdcall Invalidate();

  //// iSoundFactory ////////////////////////////
  tU32 __stdcall GetNumDrivers() const;
  iSoundDriver* __stdcall GetDriver(tU32 anIndex) const;
  iHString* __stdcall GetDriverName(tU32 anIndex) const;
  tSoundDriverCapFlags __stdcall GetDriverCaps(tU32 anIndex) const;
  tU32 __stdcall GetDriverIndex(iHString* ahspName) const;
  tBool __stdcall StartupDriver(tU32 anDriver, eSoundFormat aSoundFormat, tU32 anFrequency, tIntPtr aWindowHandle);
  tBool __stdcall ShutdownDriver();
  tU32 __stdcall GetActiveDriver() const;
  iSoundDriverBuffer* __stdcall GetBuffer() const;
  iSoundMixer* __stdcall GetMixer() const;
  iSoundMixer3D* __stdcall GetMixer3D() const;
  tBool __stdcall SwitchIn();
  tBool __stdcall SwitchOut();

  iSoundData* __stdcall LoadSoundData(iFile* apFile);

  iSoundBuffer* __stdcall CreateSoundBufferEx(iSoundData* apData, tBool abStream, iHString* ahspName);
  iSoundBuffer* __stdcall CreateSoundBuffer(iFile* apFile, tBool abStream, iHString* ahspName);
  ni::iSoundBuffer* __stdcall CreateSoundBufferFromRes(ni::iHString* ahspName, ni::tBool abStream);
  iSoundBuffer* __stdcall GetSoundBuffer(iHString* ahspName);

  //! Update the sounds.
  tBool __stdcall Update(ni::tF32 afDelta);

  void __stdcall SetListenerPosition(const sVec3f& aV);
  sVec3f __stdcall GetListenerPosition() const;
  void __stdcall SetListenerVelocity(const sVec3f& aV);
  sVec3f __stdcall GetListenerVelocity() const;
  void __stdcall SetListenerForward(const sVec3f& aV);
  sVec3f __stdcall GetListenerForward() const;
  void __stdcall SetListenerUp(const sVec3f& aV);
  sVec3f __stdcall GetListenerUp() const;
  void __stdcall SetListenerDistanceScale(tF32 afScale);
  tF32 __stdcall GetListenerDistanceScale() const;
  void __stdcall SetListenerRolloffScale(tF32 afScale);
  tF32 __stdcall GetListenerRolloffScale() const;
  void __stdcall SetListenerDopplerScale(tF32 afScale);
  tF32 __stdcall GetListenerDopplerScale() const;

  tU32 __stdcall FindFreeChannel(eSoundMode aMode) const;
  tU32 __stdcall FindFreeChannelPriority(eSoundMode aMode, tU32 anPriority) const;

  iSoundSource* __stdcall CreateSoundSource(iSoundBuffer* apBuffer);

  ni::tBool __stdcall MusicPlay(iSoundSource* apSoundSource, ni::tF32 afVolume, ni::tF32 afSpeed, ni::tF32 afBlendTime);
  ni::tBool __stdcall MusicStop(ni::tF32 afBlendTime);
  ni::tBool __stdcall MusicSpeed(ni::tF32 afSpeed, ni::tF32 afBlendTime);
  ni::tBool __stdcall MusicVolume(ni::tF32 afVolume, ni::tF32 afBlendTime);
  //// iSoundFactory ////////////////////////////

  //! Play a sound source on the specified channel.
  tBool __stdcall _PlaySoundSource(iSoundSource* apSource, tU32 anChannel, tU32 anPriority);

 private:
  astl::vector<Ptr<iSoundDriver> >  mvDrivers;

  // sound driver
  tU32                mnActiveDriver;
  Ptr<iSoundDriver>   mptrDriver;
  Ptr<iSoundMixer>    mptrMixer;
  Ptr<iSoundMixer3D>  mptrMixer3D;

  // sounds
  Ptr<iDeviceResourceManager> mptrSoundBufferMgr;

  // channels
  struct sChannel {
    tU32          nPriority;
  };
  astl::vector<sChannel>    mvChannels;

  ni::sAnimatedVariable<ni::tF32> mMusicFadeOut;
  ni::tBool             mbMusicFadeOut;
  ni::sAnimatedVariable<ni::tF32> mMusicFadeIn;
  ni::tBool             mbMusicFadeIn;
  ni::sAnimatedVariable<ni::tF32> mMusicNewSpeed;
  ni::tBool             mbMusicSpeed;
  ni::sAnimatedVariable<ni::tF32> mMusicNewVolume;
  ni::tBool             mbMusicVolume;
  ni::Ptr<ni::iSoundSource> mptrMusicCurrent;
  ni::Ptr<ni::iSoundSource> mptrMusicNext;

  niEndClass(cSoundFactory);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SOUNDFACTORY_30524441_H__
