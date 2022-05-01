#ifndef __ISOUNDDRIVER_142023_H__
#define __ISOUNDDRIVER_142023_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "ISoundMixer.h"

namespace ni {
/** \addtogroup niSound
 * @{
 */

//! Sound driver caps.
enum eSoundDriverCapFlags
{
  //! Access directly the primary sound buffer.
  eSoundDriverCapFlags_Buffer = niBit(0),
  //! Own 2d sound mixer.
  eSoundDriverCapFlags_Mixer = niBit(1),
  //! Own 3d sound mixer.
  eSoundDriverCapFlags_Mixer3D = niBit(2),
  //! \internal
  eSoundDriverCapFlags_ForceDWORD = 0xFFFFFFFF
};

//! Sound driver caps flags.
typedef tU32 tSoundDriverCapFlags;

//! Sound driver data callback.
struct iSoundDriverBufferDataSink : public iUnknown
{
  niDeclareInterfaceUUID(iSoundDriverBufferDataSink,0x17f3f5d0,0xa4fe,0x4e30,0xbb,0x8c,0x9b,0x7b,0xcd,0x75,0x91,0x85)

  //! Called when data needs to be written to the buffer.
  virtual void __stdcall OnSoundDriverBufferDataSink(void* ptr, tU32 size) = 0;
};

//! Sound driver buffer.
struct iSoundDriverBuffer : public iUnknown
{
  niDeclareInterfaceUUID(iSoundDriverBuffer,0xfe0e1afa,0x668c,0x43bc,0x90,0x03,0x50,0xfa,0x77,0x59,0x41,0xa3)

  //! Switch in the application.
  virtual tBool __stdcall SwitchIn() = 0;
  //! Switch out of the applicatio.
  virtual tBool __stdcall SwitchOut() = 0;
  //! Play the sound buffer.
  virtual tBool __stdcall Play(eSoundFormat aFormat, tU32 anFreq) = 0;
  //! Stop the sound buffer.
  virtual tBool __stdcall Stop() = 0;
  //! Get the size of the sound buffer.
  virtual tSize __stdcall GetSize() const = 0;
  //! Set the get sound data sink.
  virtual void __stdcall SetSink(iSoundDriverBufferDataSink* apSink) = 0;
  //! Get the sound data sink.
  virtual iSoundDriverBufferDataSink* __stdcall GetSink() const = 0;
  //! Update the sound buffer.
  virtual void __stdcall UpdateBuffer() = 0;
};

//! Sound driver interface.
struct iSoundDriver : public iUnknown
{
  niDeclareInterfaceUUID(iSoundDriver,0x9b0219cb,0x3f6b,0x4c8b,0x91,0x00,0xe3,0x1d,0xb7,0x72,0x41,0x69)

  //! Get the driver's name.
  //! {Property}
  virtual iHString* __stdcall GetName() const = 0;
  //! Get the driver's caps.
  //! {Property}
  virtual tSoundDriverCapFlags __stdcall GetCaps() const = 0;

  //! Startup the sound driver.
  virtual tBool __stdcall Startup(eSoundFormat aFormat, tU32 anFrequency, tIntPtr aWindowHandle) = 0;
  //! Shutdown the sound driver.
  virtual tBool __stdcall Shutdown() = 0;

  //! To call when the application is activated.
  virtual tBool __stdcall SwitchIn() = 0;
  //! To call when the application is deactivated.
  virtual tBool __stdcall SwitchOut() = 0;

  //! Get the driver's sound buffer.
  //! {Property}
  virtual iSoundDriverBuffer* __stdcall GetBuffer() const = 0;
  //! Get the driver's mixer.
  //! {Property}
  virtual iSoundMixer* __stdcall GetMixer() const = 0;
  //! Get the driver's 3d mixer.
  //! {Property}
  virtual iSoundMixer3D* __stdcall GetMixer3D() const = 0;

  //! Update the driver's computations.
  virtual void __stdcall Update() = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISOUNDDRIVER_142023_H__
