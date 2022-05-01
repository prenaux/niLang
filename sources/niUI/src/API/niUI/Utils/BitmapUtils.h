#ifndef __BITMAPUTILS_6134573_H__
#define __BITMAPUTILS_6134573_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

inline tPtr BitmapScanline(iBitmap2D* bmp, int y, int x = 0) {
  return bmp->GetData() +
      (y * bmp->GetPitch()) +
      (x * bmp->GetPixelFormat()->GetBytesPerPixel());
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __BITMAPUTILS_6134573_H__
