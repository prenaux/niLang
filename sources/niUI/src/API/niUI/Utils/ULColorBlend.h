#ifndef __ULCOLORBLEND_18435894_H__
#define __ULCOLORBLEND_18435894_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//! Blend two 15 bits colors (RGB/BGR 555 colors) using a 0-255 blend factor
__forceinline tU32 ULColorBlend15(tU32 x, tU32 y, tU32 n) {
  ni::tU32 result;
  if (n) n = (n + 1) / 8;
  x = ((x & 0xFFFF) | (x << 16)) & 0x3E07C1F;
  y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;
  result = ((((x - y) * n) / 32) + y) & 0x3E07C1F;
  return ((result & 0xFFFF) | (result >> 16));
}

//! Blend two 16 bits colors (RGB/BGR 565 colors) using a 0-255 blend factor
__forceinline tU32 ULColorBlend16(tU32 x, tU32 y, tU32 n) {
  tU32 result;

  if(n)
    n = (n + 1) / 8;

  x = ((x & 0xFFFF) | (x << 16)) & 0x7E0F81F;
  y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

  result = ((((x - y) * n) / 32) + y) & 0x7E0F81F;

  return ((result & 0xFFFF) | (result >> 16));
}

__forceinline tU32 ULColorBlend24(tU32 x, tU32 y, tU32 t) {
  const tU32 s = 255 - t;
  return (
    (((((y >> 0)  & 0xff) * s +
       ((x >> 0)  & 0xff) * t) >> 8)) |
    (((((y >> 8)  & 0xff) * s +
       ((x >> 8)  & 0xff) * t)     )  & ~0xff) |
    (((((y >> 16) & 0xff) * s +
       ((x >> 16) & 0xff) * t) << 8)  & ~0xffff) |
    0xff000000
  );
}

__forceinline tU32 ULColorBlend32(tU32 x, tU32 y, tU32 t) {
  const tU32 s = 255 - t;
  return (
    (((((y >> 0)  & 0xff) * s +
       ((x >> 0)  & 0xff) * t) >> 8)) |
    (((((y >> 8)  & 0xff) * s +
       ((x >> 8)  & 0xff) * t)     )  & ~0xff) |
    (((((y >> 16) & 0xff) * s +
       ((x >> 16) & 0xff) * t) << 8)  & ~0xffff) |
    (((((y >> 24) & 0xff) * s +
       ((x >> 24) & 0xff) * t) << 16) & ~0xffffff)
  );
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __ULCOLORBLEND_18435894_H__
