#ifndef __IJPEGREADER_4271446_H__
#define __IJPEGREADER_4271446_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Colors.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Jpeg reader interface.
struct iJpegReader : public iUnknown
{
  niDeclareInterfaceUUID(iJpegReader,0x8e453086,0x379f,0x4848,0xbc,0xb4,0x6f,0x03,0x03,0xa1,0xc6,0xaf)
  //! Read the jpeg header tables only.
  virtual tBool __stdcall ReadHeaderTables() = 0;
  //! Start reading a jpeg image.
  virtual tBool __stdcall BeginRead() = 0;
  //! End reading a jpeg image.
  virtual tBool __stdcall EndRead() = 0;
  //! Discard existing bytes in the buffer.
  virtual tBool __stdcall DiscardBuffer() = 0;
  //! Get the file from which the data are read.
  //! {Property}
  virtual iFile* __stdcall GetFile() const = 0;
  //! Get the width of the current image.
  //! \remark The value will be valid only between Begin/EndRead;
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Get the height of the current image.
  //! \remark The value will be valid only between Begin/EndRead;
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! Get the number of 8bits components.
  //! {Property}
  virtual tU32 __stdcall GetNumComponents() const = 0;
  //! Get the color space.
  //! {Property}
  virtual eColorSpace __stdcall GetColorSpace() const = 0;
  //! Read the next scanline.
  virtual tBool __stdcall ReadScanline(iFile* apOut) = 0;
  //! Read a 2d bitmap from the result of the next begin/end read.
  virtual iBitmap2D* __stdcall ReadBitmap(iGraphics* apGraphics) = 0;
};

niExportFunc(iUnknown*) New_niUI_JpegReader(const Var&, const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IJPEGREADER_4271446_H__
