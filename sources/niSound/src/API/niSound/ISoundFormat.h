#ifndef __SOUNDFORMAT_20163826_H__
#define __SOUNDFORMAT_20163826_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niSound
 * @{
 */

//! Utility format object.
struct iSoundFormat : public iUnknown
{
  niDeclareInterfaceUUID(iSoundFormat,0x03d84992,0xae0a,0x4ebc,0xbe,0xff,0x9a,0x4b,0x68,0x16,0xf1,0x9d)

  //! Get the full format.
  //! {Property}
  virtual tU32 __stdcall GetFormat() const = 0;
  //! Get the base format (format without extra parameter).
  //! {Property}
  virtual tU32 __stdcall GetBaseFormat() const = 0;

  //! Set format's bits.
  //! {Property}
  virtual void __stdcall SetBits(tU8 aVal) = 0;
  //! Get format's bits.
  //! {Property}
  virtual tU8 __stdcall GetBits() const = 0;
  //! Set format's num channels.
  //! {Property}
  virtual void __stdcall SetChannels(tU8 aVal) = 0;
  //! Get format's num channels.
  //! {Property}
  virtual tU8 __stdcall GetChannels() const = 0;
  //! Set format's extra parameter.
  //! {Property}
  virtual void __stdcall SetExtra(tU16 aVal) = 0;
  //! Get format's extra parameter.
  //! {Property}
  virtual tU16 __stdcall GetExtra() const = 0;

  //! Get the format's sample size in bytes.
  //! {Property}
  virtual tU32 __stdcall GetSampleSize() const = 0;
  //! Get the format's frame size in bytes.
  //! {Property}
  virtual tU32 __stdcall GetFrameSize() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __SOUNDFORMAT_20163826_H__
