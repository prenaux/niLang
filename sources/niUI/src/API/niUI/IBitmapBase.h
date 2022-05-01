#ifndef __IBITMAPBASE_14599053_H__
#define __IBITMAPBASE_14599053_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IPixelFormat.h"

namespace ni {
struct iGraphics;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Bitmap type
enum eBitmapType
{
  //! 2D Bitmap.
  eBitmapType_2D      = 0,
  //! Cube Bitmap. Six 2D Bitmaps.
  eBitmapType_Cube    = 1,
  //! 3D Bitmap.
  eBitmapType_3D      = 2,
  //! \internal
  eBitmapType_ForceDWORD  = 0xFFFFFFFF
};

//! Bitmap format interface.
struct iBitmapFormat : public iUnknown
{
  niDeclareInterfaceUUID(iBitmapFormat,0x08633bcb,0xb637,0xdf11,0xa0,0xfb,0xf7,0x8a,0x46,0xea,0x41,0xc1);

  //! Copy another bitmap format.
  virtual tBool __stdcall Copy(const iBitmapFormat* apFormat) = 0;
  //! Clone the bitmap format.
  virtual iBitmapFormat* __stdcall Clone() const = 0;
  //! Reset the bitmap format to its default values.
  virtual void __stdcall Reset() = 0;

  //! Set the bitmap type. (default eBitmapType_2D)
  //! {Property}
  virtual void __stdcall SetType(eBitmapType aV) = 0;
  //! Get the bitmap type.
  //! {Property}
  virtual eBitmapType __stdcall GetType() const = 0;
  //! Set the texture width. (default 16)
  //! {Property}
  virtual void __stdcall SetWidth(tU32 aV) = 0;
  //! Get the texture width. (default 16)
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Set the texture height.
  //! {Property}
  virtual void __stdcall SetHeight(tU32 aV) = 0;
  //! Get the texture height.
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! Set the texture depth. (default 0)
  //! {Property}
  virtual void __stdcall SetDepth(tU32 aV) = 0;
  //! Get the texture depth.
  //! {Property}
  virtual tU32 __stdcall GetDepth() const = 0;
  //! Set the number of mipmaps. (default 0)
  //! {Property}
  virtual void __stdcall SetNumMipMaps(tU32 aV) = 0;
  //! Get the number of mipmaps.
  //! {Property}
  virtual tU32 __stdcall GetNumMipMaps() const = 0;
  //! Set the pixel format. (default R8G8B8A8)
  //! {Property}
  virtual void __stdcall SetPixelFormat(iPixelFormat* aV) = 0;
  //! Get the pixel format.
  //! {Property}
  virtual iPixelFormat* __stdcall GetPixelFormat() const = 0;
};

//! Base bitmap interface.
struct iBitmapBase : public iUnknown
{
  niDeclareInterfaceUUID(iBitmapBase,0x28d2f00b,0xb685,0x492a,0xb0,0x3c,0x13,0x85,0xad,0x06,0x9f,0xf5)

  //! Return the bitmap type.
  //! {Property}
  virtual eBitmapType __stdcall GetType() const = 0;
  //! Get the texture width.
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Get the texture height.
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! Get the texture depth.
  //! {Property}
  virtual tU32 __stdcall GetDepth() const = 0;
  //! Get the pixel format of the texture.
  //! {Property}
  virtual iPixelFormat* __stdcall GetPixelFormat() const = 0;
  //! Create the mipmaps.
  virtual tBool __stdcall CreateMipMaps(tU32 anNumMipMaps, tBool abCompute) = 0;
  //! Remove the mipmaps.
  virtual void __stdcall RemoveMipMaps() = 0;
  //! Get the number of mipmaps.
  //! {Property}
  virtual tU32 __stdcall GetNumMipMaps() const = 0;
  //! Create a copy of the bitmap.
  virtual iBitmapBase* __stdcall Clone(ePixelFormatBlit eBlit = ePixelFormatBlit_Normal) const = 0;
  //! Create a copy of the bitmap that use the given format.
  virtual iBitmapBase* __stdcall CreateConvertedFormat(const iPixelFormat* pFmt) const = 0;
  //! Create a gamma corrected bitmap.
  virtual iBitmapBase* __stdcall CreateGammaCorrected(tF32 factor) const = 0;
  //! Correct gamma of the bitmap.
  virtual tBool __stdcall GammaCorrect(tF32 factor) = 0;
};

//! Bitmap Loader
//! {DispatchWrapper}
struct iBitmapLoader : public iUnknown {
  niDeclareInterfaceUUID(iBitmapLoader, 0x89af4ed6,0xf146,0x4abb,0x81,0x06,0xb1,0x2a,0x40,0xc8,0x24,0x08);
  virtual iBitmapBase*  __stdcall LoadBitmap(iGraphics* apGraphics, iFile* apFile) = 0;
};

//! Bitmap Saver
//! {DispatchWrapper}
struct iBitmapSaver : public iUnknown {
  niDeclareInterfaceUUID(iBitmapSaver, 0x89af4ed6,0xf146,0x4abb,0x81,0x06,0xb1,0x2a,0x40,0xc8,0x24,0x08);
  virtual tBool  __stdcall SaveBitmap(iGraphics* apGraphics, iFile* apFile, iBitmapBase* apBmp, tU32 anCompression) = 0;
};

niExportFunc(iUnknown*) New_BitmapLoader_bmp(const Var&,const Var&);
niExportFunc(iUnknown*) New_BitmapSaver_bmp(const Var&,const Var&);

niExportFunc(iUnknown*) New_BitmapLoader_jpeg(const Var&,const Var&);
niExportFunc(iUnknown*) New_BitmapSaver_jpeg(const Var&,const Var&);

niExportFunc(iUnknown*) New_BitmapLoader_png(const Var&,const Var&);
niExportFunc(iUnknown*) New_BitmapSaver_png(const Var&,const Var&);

niExportFunc(iUnknown*) New_BitmapLoader_tga(const Var&,const Var&);
niExportFunc(iUnknown*) New_BitmapSaver_tga(const Var&,const Var&);

niExportFunc(iUnknown*) New_BitmapLoader_dds(const Var&,const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IBITMAPBASE_14599053_H__
