#ifndef __ISOUNDSOURCE_29833722_H__
#define __ISOUNDSOURCE_29833722_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niSound
 * @{
 */

//! Sound source interface.
struct iSoundSource : public iDeviceResource
{
  niDeclareInterfaceUUID(iSoundSource,0x6fae2f58,0x754c,0x4369,0xa3,0xf6,0x51,0x71,0xfc,0x7c,0xa2,0x3e)

  //! Set the sound buffer.
  //! \remark If currently playing the sound source is stopped first.
  //! {Property}
  virtual tBool __stdcall SetSoundBuffer(iSoundBuffer* apBuffer) = 0;
  //! Get the sound buffer.
  //! {Property}
  virtual iSoundBuffer* __stdcall GetSoundBuffer() const = 0;
  //! Set the sound mode.
  //! \remark The sound mode is changed only next time the sound source starts to play.
  //! {Property}
  virtual void __stdcall SetMode(eSoundMode aMode) = 0;
  //! Get the sound mode.
  //! {Property}
  virtual eSoundMode __stdcall GetMode() const = 0;
  //! Set the source's position.
  //! \remark Zero by default.
  //! {Property}
  virtual void __stdcall SetPosition(const sVec3f& avPos) = 0;
  //! Get the source's position.
  //! {Property}
  virtual sVec3f __stdcall GetPosition() const = 0;
  //! Set the source's velocity.
  //! \remark Zero by default.
  //! {Property}
  virtual void __stdcall SetVelocity(const sVec3f& avVel) = 0;
  //! Get the source's velocity.
  //! {Property}
  virtual sVec3f __stdcall GetVelocity() const = 0;
  //! Set the minimum audible distance.
  //! \remark 10 by default.
  //! {Property}
  virtual void __stdcall SetMinDistance(tF32 fMin) = 0;
  //! Get the minimum audible distance.
  //! \remark 0 by default.
  //! {Property}
  virtual tF32 __stdcall GetMinDistance() const = 0;
  //! Set the maximum audible distance.
  //! \remark 1000 by default.
  //! {Property}
  virtual void __stdcall SetMaxDistance(tF32 fMax) = 0;
  //! Get the maximum audible distance.
  //! {Property}
  virtual tF32 __stdcall GetMaxDistance() const = 0;
  //! Set the sound panning.
  //! \remark 0 by default, -1 left, 1 right.
  //! {Property}
  virtual void __stdcall SetPan(tF32 afPanning) = 0;
  //! Get the sound volume.
  //! {Property}
  virtual tF32 __stdcall GetPan() const = 0;
  //! Set the sound volume.
  //! \remark 1 by default.
  //! {Property}
  virtual void __stdcall SetVolume(tF32 afVolume) = 0;
  //! Get the sound volume.
  //! {Property}
  virtual tF32 __stdcall GetVolume() const = 0;
  //! Set the sound speed.
  //! \remark 1 by default.
  //! {Property}
  virtual void __stdcall SetSpeed(tF32 afPitch) = 0;
  //! Get the sound speed.
  //! {Property}
  virtual tF32 __stdcall GetSpeed() const = 0;
  //! Set the source in loop mode.
  //! \remark eFalse by default.
  //! {Property}
  virtual void __stdcall SetLoop(tBool abLoop) = 0;
  //! Get the source loop mode.
  //! {Property}
  virtual tBool __stdcall GetLoop() const = 0;
  //! Starts playing the sound.
  virtual tBool __stdcall Play() = 0;
  //! Stops playing the sound.
  virtual tBool __stdcall Stop() = 0;
  //! Pause the sound.
  virtual tBool __stdcall Pause() = 0;
  //! Check if the sound is currently playing.
  //! {Property}
  virtual tBool __stdcall GetIsPlaying() const = 0;
  //! Check if the sound is currently paused.
  //! {Property}
  virtual tBool __stdcall GetIsPaused() const = 0;

  //! Play as a 2d sound FX.
  virtual ni::tBool __stdcall Play2D(ni::tF32 afVolume, ni::tF32 afSpeed, ni::tF32 afPan) = 0;
  //! Play as a 3d sound FX.
  virtual ni::tBool __stdcall Play3D(ni::tF32 afVolume, ni::tF32 afSpeed, ni::sVec3f avPosition, ni::tBool abListenerRelative, ni::tF32 afMinDistance, ni::tF32 afMaxDistance) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISOUNDSOURCE_29833722_H__
