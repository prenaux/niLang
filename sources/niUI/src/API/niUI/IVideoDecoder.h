#ifndef __IVIDEODECODER_11391338_H__
#define __IVIDEODECODER_11391338_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

//! Video decoder flags.
enum eVideoDecoderFlags
{
  //! Bitmap target.
  //! \remark Manual update only.
  //! \remark If no target type is specified no actual decoding will occur.
  eVideoDecoderFlags_TargetBitmap = niBit(0),
  //! Texture target.
  //! \remark Update the decoder when binded in the renderer.
  //! \remark If no target type is specified no actual decoding will occur.
  eVideoDecoderFlags_TargetTexture = niBit(1),
  //! If available decode and sync sound stream with the video.
  eVideoDecoderFlags_Sound = niBit(2),
  //! \internal
  eVideoDecoderFlags_ForceDWORD = 0xFFFFFFFF
};

//! Video decoder flags type. \see ni::eVideoDecoderFlags
typedef tU32 tVideoDecoderFlags;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Video decoder interface.
struct iVideoDecoder : public iUnknown
{
  niDeclareInterfaceUUID(iVideoDecoder,0x0aebf271,0x83de,0x40da,0xab,0xc0,0x4f,0x6a,0x37,0xee,0xd3,0xdb)

  //! Get the name of the decoder used.
  //! {Property}
  virtual const achar* __stdcall GetVideoDecoderName() const = 0;
  //! Get the original number of frames per second of the video.
  //! {Property}
  virtual tF64 __stdcall GetVideoFps() const = 0;

  //! Get the decoder flags.
  //! {Property}
  virtual tVideoDecoderFlags __stdcall GetFlags() const = 0;

  //! Get the video's length.
  //! \return The length in seconds of the video.
  //! {Property}
  virtual tF64 __stdcall GetLength() const = 0;
  //! Set the current time.
  //! \param afTime is the time to set.
  //! {Property}
  virtual void __stdcall SetTime(tF64 afTime) = 0;
  //! Get the current time.
  //! \return The current time.
  //! {Property}
  virtual tF64 __stdcall GetTime() const = 0;
  //! Set pause.
  //! \param  abPause, if eTrue the update and decoding will be paused.
  //! \remark This will pause the thread is the decoder is asyncronous.
  //! {Property}
  virtual void __stdcall SetPause(tBool abPause) = 0;
  //! Get the pause status.
  //! \return eTrue if the decoder is in pause mode, else eFalse.
  //! {Property}
  virtual tBool __stdcall GetPause() const = 0;
  //! Set the time's speed.
  //! \param  afSpeed is the speed to set, 1 is the original speed, 2 is twice
  //!     faster, 0.5 half the original speed and so on.
  //! {Property}
  virtual void __stdcall SetSpeed(tF32 afSpeed) = 0;
  //! Get the time's speed.
  //! {Property}
  virtual tF32 __stdcall GetSpeed() const = 0;
  //! Set the number of time the time should loop.
  //! {Property}
  //! \remark eInvalidHandle means infinite looping (the default value).
  //! \remark Unless infinite, the number of loops decrease every
  //!         time the video is looped until it reaches zero and the
  //!         video stops.
  virtual void __stdcall SetNumLoops(ni::tU32 anLoop) = 0;
  //! Get the number of loop.
  //! {Property}
  virtual ni::tU32 __stdcall GetNumLoops() const = 0;

  //! Update the video.
  //! \param abUpdateTarget, if eTrue will update the target else only the current frame
  //!        and time will be updated.
  //! \param afFrameTime is the time elapsed since the last frame. If you dont want the
  //!        method to compute the new frame set it to zero.
  //! \return eFalse if the target's update failed, else eTrue.
  virtual tBool __stdcall Update(tBool abUpdateTarget, tF32 afFrameTime) = 0;

  //! Get the target texture.
  //! {Property}
  virtual iTexture* __stdcall GetTargetTexture() = 0;
  //! Get the target bitmap.
  //! {Property}
  virtual iBitmap2D* __stdcall GetTargetBitmap() = 0;

  //! Get the number of sound tracks attached to the video.
  //! {Property}
  virtual tU32 __stdcall GetNumSoundTracks() const = 0;
  //! Get a sound data of a track associated with the video.
  //! {Property}
  //! \remark Note that sound is always decoded at the original
  //!         speed its found in the video.  The video speed needs
  //!         to be handled manually, by using
  //!         iSoundSource::SetSpeed for example, to keep the sound
  //!         properly in sync with the video.
  virtual iUnknown* __stdcall GetSoundTrackData(tU32 anNumTrack) = 0;

  //! Set whether the video should automatically be updated when bound for rendering.
  //! {Property}
  virtual void __stdcall SetUpdateOnBind(tBool abUpdateOnBind) = 0;
  //! Get whether the video should automatically be updated when bound for rendering.
  //! {Property}
  virtual tBool __stdcall GetUpdateOnBind() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IVIDEODECODER_11391338_H__
