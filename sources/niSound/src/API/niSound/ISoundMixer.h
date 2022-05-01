#ifndef __ISOUNDMIXER_6451531_H__
#define __ISOUNDMIXER_6451531_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "ISoundData.h"

namespace ni {
/** \addtogroup niSound
 * @{
 */

//! Sound mode
enum eSoundMode {
  //! Normal 2D sound playing.
  //! \remark Ignores all 3d properties.
  eSoundMode_Normal2D = 0,
  //! Normal 3D sound playing.
  //! \remark Ignores panning.
  eSoundMode_Normal3D = 1,
  //! Listener relative 3d sound playing.
  //! \remark Ignores panning.
  eSoundMode_Relative3D = 2,
  //! \internal
  eSoundMode_ForceDWORD = 0xFFFFFFFF
};

//! Sound mixer interface.
struct iSoundMixer : public iUnknown
{
  niDeclareInterfaceUUID(iSoundMixer,0xa45cfb5d,0x7391,0x4bf6,0x92,0xd9,0x6d,0x4e,0x74,0x1f,0xf1,0x14)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! To call when the application is activated.
  //! \remark Usually the owner of the mixer will call it.
  virtual tBool __stdcall SwitchIn() = 0;
  //! To call when the application is deactivated.
  //! \remark Usually the owner of the mixer will call it.
  virtual tBool __stdcall SwitchOut() = 0;

  //! Set the amplification of the mixer.
  //! {Property}
  virtual void __stdcall SetAmplification(tI32 ampli) = 0;
  //! Get the amplification of the mixer.
  //! {Property}
  virtual tI32  __stdcall GetAmplification(void) const = 0;

  //! Set the saturation check of the mixer.
  //! {Property}
  virtual void  __stdcall SetSaturationCheck(tBool check) = 0;
  //! Get the saturation check of the mixer.
  //! {Property}
  virtual tBool __stdcall GetSaturationCheck(void) const = 0;

  //! Set the reverse stereo of the mixer.
  //! {Property}
  virtual void  __stdcall SetReverseStereo(tBool reverse) = 0;
  //! Get the reverse stereo of the mixer.
  //! {Property}
  virtual tBool __stdcall GetReverseStereo(void) const = 0;

  //! Set the master volume of the mixer.
  //! {Property}
  virtual void __stdcall SetMasterVolume(tI32 vol) = 0;
  //! Get the master volume of the mixer.
  //! {Property}
  virtual tI32 __stdcall GetMasterVolume() const = 0;
  //! @}

  //########################################################################################
  //! \name Channels
  //########################################################################################
  //! @{

  //! Get the number of channels available.
  virtual tU32 __stdcall GetNumChannels() const = 0;

  //! Start playing the specified channel.
  virtual tBool __stdcall StartChannel(tU32 nchan) = 0;
  //! Stop playing the specified channel.
  virtual tBool __stdcall StopChannel(tU32 nchan) = 0;
  //! Stop playing all channels.
  virtual void __stdcall StopAllChannels(void) = 0;

  //! Set the master volume of the specified channel.
  //! {Property}
  virtual void __stdcall SetChannelMasterVolume(tU32 nchan, tI32 vol) = 0;
  //! Get the master volume of the specified channel.
  //! {Property}
  virtual tI32  __stdcall GetChannelMasterVolume(tU32 nchan) const = 0;

  //! Set the master pan of the specified channel.
  //! {Property}
  virtual void __stdcall SetChannelMasterPan(tU32 nchan, tI32 pan) = 0;
  //! Get the master pan of the specified channel.
  //! {Property}
  virtual tI32  __stdcall GetChannelMasterPan(tU32 nchan) const = 0;

  //! Set the buffer of the specified channel.
  //! {Property}
  virtual tBool __stdcall SetChannelBuffer(tU32 anChan, iSoundBuffer* apBuffer) = 0;
  //! Get the buffer of the specified channel.
  //! {Property}
  virtual iSoundBuffer* __stdcall GetChannelBuffer(tU32 anChan) const = 0;

  //! Set the volume of the specified channel.
  //! {Property}
  virtual void __stdcall SetChannelVolume(tU32 nchan, tI32 vol) = 0;
  //! Get the volume of the specified channel.
  //! {Property}
  virtual tI32  __stdcall GetChannelVolume(tU32 nchan) const = 0;

  //! Set the panning of the specified channel.
  //! {Property}
  virtual void __stdcall SetChannelPan(tU32 nchan, tI32 pan) = 0;
  //! Get the panning of the specified channel.
  //! {Property}
  virtual tI32  __stdcall GetChannelPan(tU32 nchan) const = 0;

  //! Set the frequency of the specified channel.
  //! {Property}
  virtual void __stdcall SetChannelFrequency(tU32 nchan, tI32 frequency) = 0;
  //! Get the frequency of the specified channel.
  //! {Property}
  virtual tI32  __stdcall GetChannelFrequency(tU32 nchan) const = 0;

  //! Set whether the specified channel loops.
  //! {Property}
  virtual void __stdcall SetChannelLoop(tU32 nchan, tBool loop) = 0;
  //! Get whether the specified channel loops.
  //! {Property}
  virtual tBool __stdcall GetChannelLoop(tU32 nchan) const = 0;

  //! Set whether the specified channel is paused.
  //! {Property}
  virtual void __stdcall SetChannelPause(tU32 nchan, tBool pause) = 0;
  //! Get whether the specified channel is paused.
  //! {Property}
  virtual tBool __stdcall GetChannelPause(tU32 nchan) const = 0;

  //! Get whether the specified channel is busy (playing, or, playing and paused).
  //! {Property}
  virtual tBool __stdcall GetIsChannelBusy(tU32 nchan) const = 0;

  //! Set the user id of the specified channel.
  //! \remark Stop channel reset the user id to zero.
  //! {Property}
  virtual void __stdcall SetChannelUserID(tU32 nchan, tIntPtr anUserId) = 0;
  //! Get the user id of the specified channel.
  //! {Property}
  virtual tIntPtr __stdcall GetChannelUserID(tU32 nchan) const = 0;

  //! @}

  //########################################################################################
  //! \name Update
  //########################################################################################
  //! @{

  //! Update the sound mixer.
  virtual tBool __stdcall UpdateMixer() = 0;
  //! @}
};

//! Sound mixer 3d interface.
struct iSoundMixer3D : public iUnknown
{
  niDeclareInterfaceUUID(iSoundMixer3D,0xededed66,0xbea9,0x4dc4,0xac,0xd5,0x27,0x6c,0xfe,0x1f,0x72,0xfb)

  //########################################################################################
  //! \name 3D channels
  //########################################################################################
  //! @{

  //! Get the index of the first 3d channel.
  //! \remark The 3d channels are at the end, so from first 3d channel to the last channel index are all 3d channels.
  //! {Property}
  virtual tU32 __stdcall GetFirst3DChannel() const = 0;
  //! Get whether the specified channel is 3d.
  //! \remark 3D channels dont support panning
  //! {Property}
  virtual tBool __stdcall GetIsChannel3D(tU32 nchan) const = 0;

  //! Set the channel's 3d mode.
  //! {Property}
  virtual void __stdcall SetChannelMode(tU32 nchan, eSoundMode aMode) = 0;
  //! Get the channel's 3d mode.
  //! {Property}
  virtual eSoundMode __stdcall GetChannelMode(tU32 nchan) const = 0;

  //! Set the channel's 3d Position.
  //! {Property}
  virtual void __stdcall SetChannelPosition(tU32 nchan, const sVec3f& avPosition) = 0;
  //! Get the channel's 3d Position.
  //! {Property}
  virtual sVec3f __stdcall GetChannelPosition(tU32 nchan) = 0;

  //! Set the channel's 3d Velocity.
  //! {Property}
  virtual void __stdcall SetChannelVelocity(tU32 nchan, const sVec3f& avVelocity) = 0;
  //! Get the channel's 3d Velocity.
  //! {Property}
  virtual sVec3f __stdcall GetChannelVelocity(tU32 nchan) = 0;

  //! Set the channel's 3D minimum distance.
  //! {Property}
  virtual void __stdcall SetChannelMinDistance(tU32 nchan, tF32 afMinDistance) = 0;
  //! Set the channel's 3D minimum distance.
  //! {Property}
  virtual tF32 __stdcall GetChannelMinDistance(tU32 nchan) const = 0;

  //! Set the channel's 3D maximum distance.
  //! {Property}
  virtual void __stdcall SetChannelMaxDistance(tU32 nchan, tF32 afMaxDistance) = 0;
  //! Set the channel's 3D maximum distance.
  //! {Property}
  virtual tF32 __stdcall GetChannelMaxDistance(tU32 nchan) const = 0;

  //! Set the channel's 3D cone inside angle.
  //! {Property}
  //! \remark The angle's range is 0 (no cone) to 2pi/360deg (full sphere). The angle is in radian.
  //! \remark Default is zero, no cone attenuation.
  virtual void __stdcall SetChannelConeInner(tU32 nchan, tF32 afInsideAngle) = 0;
  //! Set the channel's 3D cone inside angle.
  //! {Property}
  virtual tF32 __stdcall GetChannelConeInner(tU32 nchan) const = 0;

  //! Set the channel's 3D cone outside angle.
  //! {Property}
  //! \remark The angle's range is 0 (no cone) to 2pi/360deg (full sphere). The angle is in radian.
  //! \remark Default is zero, no cone attenuation.
  virtual void __stdcall SetChannelConeOuter(tU32 nchan, tF32 afOutsideAngle) = 0;
  //! Set the channel's 3D cone outside angle.
  //! {Property}
  virtual tF32 __stdcall GetChannelConeOuter(tU32 nchan) const = 0;

  //! Sets the volume of the sound outside the outer angle of the sound projection cone.
  //! {Property}
  virtual void __stdcall SetChannelConeOuterVolume(tU32 nchan, tU32 anOuterVolume) = 0;
  //! Set the channel's 3D cone outside angle.
  //! {Property}
  virtual tU32 __stdcall GetChannelConeOuterVolume(tU32 nchan) const = 0;

  //! Set the channel's 3D cone direction.
  //! {Property}
  virtual void __stdcall SetChannelConeDirection(tU32 nchan, const sVec3f& avDir) = 0;
  //! Set the channel's 3D cone direction.
  //! {Property}
  virtual sVec3f __stdcall GetChannelConeDirection(tU32 nchan) const = 0;
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
  //! \name Update
  //########################################################################################
  //! @{

  //! Update the 3d sounds mixer.
  //! \remark This function will usually be called once per frame. It can be ignored if
  //!     no 3d sounds has been modified.
  virtual tBool __stdcall UpdateMixer3D() = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISOUNDMIXER_6451531_H__
