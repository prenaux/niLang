#ifndef __ISOUNDDATA_73552864_H__
#define __ISOUNDDATA_73552864_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niSound
 * @{
 */

#define niSoundFormatBuild(BITS,CHANNELS,EXTRA) ((tU32)((tU32(BITS&0xFF)|(tU32(CHANNELS&0xFF)<<8)|(tU32(EXTRA&0xFFFF)<<16))))

//! Common sound formats.
enum eSoundFormat {
  //! Unknown format
  eSoundFormat_Unknown = 0,
  //! Mono 8 bits.
  eSoundFormat_Mono8 = (8)|((1)<<8),
  //! Stereo 8 bits.
  eSoundFormat_Stereo8 = (8)|((2)<<8),
  //! Mono 16 bits.
  eSoundFormat_Mono16 = (16)|((1)<<8),
  //! Stereo 16 bits.
  eSoundFormat_Stereo16 = (16)|((2)<<8),
  //! \internal
  eSoundFormat_ForceDWORD = 0xFFFFFFFF
};

//! Sound data interface.
struct iSoundData : public iUnknown
{
  niDeclareInterfaceUUID(iSoundData,0x2fe75840,0x416e,0x4d36,0x82,0xff,0xb4,0x3d,0x17,0xf4,0x0b,0x05)

  //! Clone the sound data.
  //! \remark This is necessary for streaming as streamed data need separate read pointers.
  virtual iSoundData* __stdcall Clone() const = 0;

  //! Get the sound's format bits.
  //! {Property}
  virtual eSoundFormat __stdcall GetFormat() const = 0;
  //! Get the sound frequency in hertz.
  //! {Property}
  virtual tU32 __stdcall GetFrequency() const = 0;

  //! Read a chunk of the specified size.
  //! \param  aptrOut is the buffer where to output the data.
  //! \param  anSize is the number of bytes to read.
  //! \return The number of bytes read and a negative value on error.
  virtual tI32 __stdcall ReadRaw(tPtr apOut, tU32 anSize) = 0;
  //! Reset data stream to the begining.
  virtual void __stdcall Reset() = 0;

  //! Get the sound length in frames.
  //! {Property}
  virtual tU32 __stdcall GetLength() const = 0;
};

//! Sound data loader plugin interface.
struct iSoundDataLoader : public iUnknown
{
  niDeclareInterfaceUUID(iSoundDataLoader,0x6ce55475,0xa8ca,0x4d0c,0x8b,0xae,0x3e,0xe1,0x21,0x7c,0x4e,0x14);

  virtual iSoundData* __stdcall LoadSoundData(iFile* apFile) = 0;
};

inline tU32 SoundFormatBuild(tU8 anBits, tU8 anChannels, tU16 anExtra) {
  return niSoundFormatBuild(anBits,anChannels,anExtra);
}
inline tU8 SoundFormatGetBits(tU32 anFormat) {
  return tU8(anFormat&0xFF);
}
inline tU8 SoundFormatGetChannels(tU32 anFormat) {
  return tU8((anFormat>>8)&0xFF);
}
inline tU16 SoundFormatGetExtra(tU32 anFormat) {
  return tU16((anFormat>>16)&0xFFFF);
}
inline tU32 SoundFormatSetBits(tU32 anFormat, tU8 anBits) {
  return SoundFormatBuild(
      anBits,
      SoundFormatGetChannels(anFormat),
      SoundFormatGetExtra(anFormat));
}
inline tU32 SoundFormatSetChannels(tU32 anFormat, tU8 anChannels) {
  return SoundFormatBuild(
      SoundFormatGetBits(anFormat),
      anChannels,
      SoundFormatGetExtra(anFormat));
}
inline tU32 SoundFormatSetExtra(tU32 anFormat, tU16 anExtra) {
  return SoundFormatBuild(
      SoundFormatGetBits(anFormat),
      SoundFormatGetChannels(anFormat),
      anExtra);
}
inline tU32 SoundFormatGetBase(tU32 anFormat) {
  return anFormat&0xFFFF;
}
inline tU32 SoundFormatGetSampleSize(tU32 anFormat) {
  return niBytesPerPixel(SoundFormatGetBits(anFormat));
}
inline tU32 SoundFormatGetFrameSize(tU32 anFormat) {
  return SoundFormatGetSampleSize(anFormat) * SoundFormatGetChannels(anFormat);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISOUNDDATA_73552864_H__
