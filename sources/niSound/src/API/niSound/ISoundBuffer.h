#ifndef __ISOUNDBUFFER_22652770_H__
#define __ISOUNDBUFFER_22652770_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niSound
 * @{
 */

//! Sound buffer interface.
struct iSoundBuffer : public iDeviceResource
{
  niDeclareInterfaceUUID(iSoundBuffer,0xca0f6aff,0x998c,0x401f,0x9a,0xff,0xad,0xaf,0x39,0xba,0x76,0xb3)

  //! Create an instance of this sound buffer.
  virtual iSoundBuffer* __stdcall CreateInstance() = 0;

  //! Get whether the sound buffer is an instance.
  //! {Property}
  virtual tBool __stdcall GetIsInstance() const = 0;
  //! Get whether the sound is streamed.
  //! {Property}
  virtual tBool __stdcall GetIsStreamed() const = 0;

  //! Get the sound buffer's format.
  //! {Property}
  virtual eSoundFormat __stdcall GetFormat() const = 0;
  //! Get the sound buffer frequency.
  //! {Property}
  virtual tU32 __stdcall GetFrequency() const = 0;

  //! Get the streamed sound data.
  //! {Property}
  virtual iSoundData* __stdcall GetStreamSoundData() const = 0;
  //! Reset the sound buffer position.
  virtual void __stdcall ResetPosition() = 0;
  //! Retrieve data in the sound buffer.
  virtual tBool __stdcall ReadRaw(tPtr apOut, tU32 anBytes, tBool abLoop) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISOUNDBUFFER_22652770_H__
