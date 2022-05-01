#ifndef __NIISOUNDFACTORY_H__
#define __NIISOUNDFACTORY_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "ISoundSource.h"

namespace ni {
/** \addtogroup niSound
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Sound factory interface
struct iSoundFactory : public iUnknown
{
  niDeclareInterfaceUUID(iSoundFactory,0x76551a09,0x010b,0x4074,0x97,0xc3,0x54,0x58,0x2b,0x3c,0xe6,0x55)

  //########################################################################################
  //! \name Driver
  //########################################################################################
  //! @{

  //! Get the number of sound drivers available.
  //! {Property}
  virtual tU32 __stdcall GetNumDrivers() const = 0;
  //! Get the driver at the specified index.
  //! {Property}
  virtual iSoundDriver* __stdcall GetDriver(tU32 anIndex) const = 0;
  //! Get the name of the sound driver at the specified index.
  //! {Property}
  virtual iHString* __stdcall GetDriverName(tU32 anIndex) const = 0;
  //! Get the caps of the sound driver at the specified index.
  //! {Property}
  virtual tSoundDriverCapFlags __stdcall GetDriverCaps(tU32 anIndex) const = 0;
  //! Get the index of the driver with the specified name.
  //! {Property}
  virtual tU32 __stdcall GetDriverIndex(iHString* ahspName) const = 0;

  //! Initialize the specified sound driver.
  //! \param anDriver is the index of the driver to start.
  //!      If eInvalidHandle the best driver available will be selected.
  //! \param aFormat is the format to use in the driver.
  //! \param anFrequency is the frequency to use in the driver.
  //! \param aWindowHandle, if not NULL specify the window handle to pass to the sound driver.
  //!     This is ignored by some drivers, and is usefull only when using the sound module without
  //!     the full SDK, since the SDK will already holds a proper window handle.
  //! \remark The bits, channels and frequency parameters are used mostly by the software sound mixer,
  //!     with a hardware accelerated mixer driver those parameters might be ignored.
  virtual tBool __stdcall StartupDriver(tU32 anDriver, eSoundFormat aFormat, tU32 anFrequency, tIntPtr aWindowHandle) = 0;
  //! Shutdown the current sound driver.
  virtual tBool __stdcall ShutdownDriver() = 0;

  //! Get the active driver.
  //! {Property}
  virtual tU32 __stdcall GetActiveDriver() const = 0;
  //! Get the driver's sound buffer.
  //! {Property}
  virtual iSoundDriverBuffer* __stdcall GetBuffer() const = 0;
  //! Get the sound mixer.
  //! {Property}
  virtual iSoundMixer* __stdcall GetMixer() const = 0;
  //! Get the 3d sound mixer.
  //! {Property}
  virtual iSoundMixer3D* __stdcall GetMixer3D() const = 0;

  //! To call when the application is activated.
  virtual tBool __stdcall SwitchIn() = 0;
  //! To call when the application is deactivated.
  virtual tBool __stdcall SwitchOut() = 0;
  //! @}

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Find the first free channel.
  tU32 __stdcall FindFreeChannel(eSoundMode aMode) const;
  //! Find the first free channel, possibily over a used channel with the specified priority.
  tU32 __stdcall FindFreeChannelPriority(eSoundMode aMode, tU32 anPriority) const;
  //! Update the sounds.
  virtual tBool __stdcall Update(ni::tF32 afDeltaTime) = 0;
  //! @}

  //########################################################################################
  //! \name Sound Data, Sound Buffer & Sound Source
  //########################################################################################
  //! @{

  //! Load sound data from the specified file.
  virtual iSoundData* __stdcall LoadSoundData(iFile* apFile) = 0;

  //! Create a sound buffer from the specified sound data.
  virtual iSoundBuffer* __stdcall CreateSoundBufferEx(iSoundData* apData, tBool abStream, iHString* ahspName = NULL) = 0;
  //! Create a sound buffer from the specified file.
  //! \remark This method will load sound data internally.
  virtual iSoundBuffer* __stdcall CreateSoundBuffer(iFile* apFile, tBool abStream, iHString* ahspName = NULL) = 0;
  //! Create a sound buffer from the specified resource.
  virtual ni::iSoundBuffer* __stdcall CreateSoundBufferFromRes(ni::iHString* ahspName, ni::tBool abStream) = 0;
  //! Get the sound buffer with the given name.
  //! {Property}
  virtual iSoundBuffer* __stdcall GetSoundBuffer(iHString* ahspName) = 0;

  //! Create a new sound source.
  virtual iSoundSource* __stdcall CreateSoundSource(iSoundBuffer* apBuffer) = 0;
  //! @}

  //########################################################################################
  //! \name Listener
  //########################################################################################
  //! @{

  //! Set the listener's position.
  //! {Property}
  virtual void __stdcall SetListenerPosition(const sVec3f& aV) = 0;
  //! Get the listener's position.
  //! {Property}
  virtual sVec3f __stdcall GetListenerPosition() const = 0;
  //! Set the listener's velocity.
  //! {Property}
  virtual void __stdcall SetListenerVelocity(const sVec3f& aV) = 0;
  //! Get the listener's velocity.
  //! {Property}
  virtual sVec3f __stdcall GetListenerVelocity() const = 0;
  //! Set the listener's forward/looking direction.
  //! {Property}
  virtual void __stdcall SetListenerForward(const sVec3f& aV) = 0;
  //! Get the listener's forward/looking direction.
  //! {Property}
  virtual sVec3f __stdcall GetListenerForward() const = 0;
  //! Set the listener's up direction.
  //! {Property}
  virtual void __stdcall SetListenerUp(const sVec3f& aV) = 0;
  //! Get the listener's up direction.
  //! {Property}
  virtual sVec3f __stdcall GetListenerUp() const = 0;
  //! Set the listener's distance scale.
  //! {Property}
  //! \remark By default 0.1 - aka the default unit system 10 units = 1 meter
  virtual void __stdcall SetListenerDistanceScale(tF32 afScale) = 0;
  //! Get the listener's distance scale.
  //! {Property}
  virtual tF32 __stdcall GetListenerDistanceScale() const = 0;
  //! Set the listener's rolloff scale.
  //! {Property}
  virtual void __stdcall SetListenerRolloffScale(tF32 afScale) = 0;
  //! Get the listener's rolloff scale.
  //! {Property}
  virtual tF32 __stdcall GetListenerRolloffScale() const = 0;
  //! Set the listener's doppler scale.
  //! {Property}
  virtual void __stdcall SetListenerDopplerScale(tF32 afScale) = 0;
  //! Get the listener's doppler scale.
  //! {Property}
  virtual tF32 __stdcall GetListenerDopplerScale() const = 0;
  //! @}

  //########################################################################################
  //! \name Music playback API
  //########################################################################################
  //! @{

  //! Play a music.
  virtual ni::tBool __stdcall MusicPlay(iSoundSource* apSoundSource, ni::tF32 afVolume, ni::tF32 afSpeed, ni::tF32 afBlendTime) = 0;
  //! Stop the current music.
  virtual ni::tBool __stdcall MusicStop(ni::tF32 afBlendTime) = 0;
  //! Set the new speed of the music.
  virtual ni::tBool __stdcall MusicSpeed(ni::tF32 afSpeed, ni::tF32 afBlendTime) = 0;
  //! Set the new volume of the music.
  virtual ni::tBool __stdcall MusicVolume(ni::tF32 afVolume, ni::tF32 afBlendTime) = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __NIISOUNDFACTORY_H__
