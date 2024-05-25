#ifndef __IBITMAPCUBE_57046848_H__
#define __IBITMAPCUBE_57046848_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
struct iBitmap2D;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Bitmap cube faces.
//! \remark The coordinate system is Left-Handed as in DX7/8.
enum eBitmapCubeFace
{
  eBitmapCubeFace_PositiveX = 0,
  eBitmapCubeFace_NegativeX = 1,
  eBitmapCubeFace_PositiveY = 2,
  eBitmapCubeFace_NegativeY = 3,
  eBitmapCubeFace_PositiveZ = 4,
  eBitmapCubeFace_NegativeZ = 5,
  //! \internal
  eBitmapCubeFace_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Cube bitmap interface.
struct iBitmapCube : public iBitmapBase
{
  niDeclareInterfaceUUID(iBitmapCube,0x8da50ffa,0x7fd2,0x404c,0x85,0xe0,0xbc,0x52,0x51,0x1e,0xf4,0xc4)

  //! Get the content of the indicated face.
  //! {Property}
  virtual iBitmap2D* __stdcall GetFace(eBitmapCubeFace Face) const = 0;
  //! Create a resized cube map.
  virtual iBitmapCube* __stdcall CreateResized(tI32 nW) const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IBITMAPCUBE_57046848_H__
