#ifndef __IJPEGWRITER_61487884_H__
#define __IJPEGWRITER_61487884_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Jpeg write flags
enum eJpegWriteFlags
{
  //! No flags.
  eJpegWriteFlags_None = 0,
  //! Use the custom YCoCg color space.
  eJpegWriteFlags_YCoCg = niBit(0),
  //! Has an alpha channel, implies the use of the YCoCg(A) colorspace.
  eJpegWriteFlags_Alpha = niBit(1),
  //! \internal
  eJpegWriteFlags_ForceDWORD = 0xFFFFFFFF
};

//! Jpeg write flags type. \see ni::eJpegWriteFlags
typedef tU32 tJpegWriteFlags;

//////////////////////////////////////////////////////////////////////////////////////////////
//! Jpeg writer interface.
struct iJpegWriter : public iUnknown
{
  niDeclareInterfaceUUID(iJpegWriter,0xd8ddbbf9,0x9609,0x44ee,0x80,0xe5,0x7a,0x99,0x97,0x3a,0xf9,0x03)
  //! Begin writing a new jpeg file.
  virtual tBool __stdcall BeginWrite(iFile* apDest,
                                     tU32 anWidth, tU32 anHeight,
                                     eColorSpace aInCS, tU32 anC,
                                     tU32 anQuality, tJpegWriteFlags aFlags) = 0;
  //! End writing the jpeg file.
  virtual tBool __stdcall EndWrite() = 0;
  //! Write a scanline.
  virtual tBool __stdcall WriteScanline(iFile* apSrc) = 0;
  //! Write a 2d bitmap.
  //! \remark Supports R8G8B8 and R8G8B8A8.
  virtual tBool __stdcall WriteBitmap(iFile* apDest, const iBitmap2D* apBmp,
                                      tU32 anQuality, tJpegWriteFlags aFlags) = 0;
};

niExportFunc(iUnknown*) New_niUI_JpegWriter(const Var&, const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IJPEGWRITER_61487884_H__
