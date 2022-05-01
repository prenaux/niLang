#ifndef __WIN32_DC_H_5E6BE155_53F3_4658_B343_A6FA20631A00__
#define __WIN32_DC_H_5E6BE155_53F3_4658_B343_A6FA20631A00__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Win32_UTF.h"

namespace ni { namespace Windows {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Win32
 * @{
 */

//! \brief Device context (DC) wrapper
class DC
{
 public:
  DC() { mhDC = NULL; };
  DC(HDC hDC) { mhDC = hDC; };
  virtual ~DC() { };

  void DrawLine(UINT iX1, UINT iY1, UINT iX2, UINT iY2, COLORREF clrColor)
  {
    HPEN hPen;
    HGDIOBJ hObj = NULL;

    hPen = ::CreatePen(PS_SOLID, 1, clrColor);
    hObj = SelectObject(hPen);
    ::MoveToEx(mhDC, iX1, iY1, NULL);
    ::LineTo(mhDC, iX2, iY2);
    SelectObject(hObj);
    ::DeleteObject(hPen);
  };

  void MoveTo(UINT iX, UINT iY) { ::MoveToEx(mhDC, iX, iY, NULL); };

  void LineTo(UINT iX, UINT iY, COLORREF clrColor)
  {
    HPEN hPen;
    HGDIOBJ hObj = NULL;

    hPen = ::CreatePen(PS_SOLID, 1, clrColor);
    hObj = SelectObject(hPen);
    ::LineTo(mhDC, iX, iY);
    SelectObject(hObj);
    ::DeleteObject(hPen);
  };

  void LineTo(UINT iX, UINT iY) { ::LineTo(mhDC, iX, iY); };

  //! \remarks Caller is responsible for deleting font
  HFONT CreateAndSelectFont(const TCHAR *pszFace, UINT iSize, INT iWeight = FW_NORMAL)
  {
    HFONT hFont = ::CreateFontA(-(int)iSize,
                                0, 0, 0,
                                iWeight,
                                0, 0, 0, 0, 0, 0,
#if defined niMSVC && _MSC_VER > 1310
                                CLEARTYPE_QUALITY,
#else
                                ANTIALIASED_QUALITY,
#endif
                                0,
                                pszFace);
    if (hFont)
      SelectObject(hFont);
    return hFont;
  };

  void CreateCompatible(HDC hDC) { mhDC = ::CreateCompatibleDC(hDC); };

  void Delete()
  {
    if (mhDC)
    {
      niAssert(DeleteDC(mhDC) == TRUE);
      mhDC = NULL;
    }
  };

  void ReleaseDC(HWND hWnd) {
    ::ReleaseDC(hWnd,mhDC);
  }

  int DrawText(const char *pszString, RECT *rcBounds,
               UINT iFormat = DT_LEFT | DT_END_ELLIPSIS)
  {
    UTF16Buffer wString;
    niWin32_UTF8ToUTF16(wString,pszString);
    return niWin32API(DrawText)(mhDC, wString.begin(), -1, rcBounds, iFormat);
  };

  BOOL TextOut(const TCHAR *pszString, UINT iX, UINT iY)
  {
    return ::TextOut(mhDC, iX, iY, pszString, (UINT)_tcslen(pszString));
  };

  COLORREF GetPixel(UINT iX, UINT iY) {
    return ::GetPixel(mhDC,iX,iY);
  }
  BOOL SetPixel(UINT iX, UINT iY, COLORREF clrColor) {
    return ::SetPixelV(mhDC, iX, iY, clrColor);
  }

  HDC GetDC() const { return mhDC; };

  int FillRect(const RECT& rc, HBRUSH hBrush)
  { return ::FillRect(mhDC, &rc, hBrush); };
  BOOL DrawFrameControl(RECT *pRect, UINT iType, UINT iState)
  { return ::DrawFrameControl(mhDC, pRect, iType, iState); };
  HGDIOBJ SelectObject(HGDIOBJ hObj)
  { return ::SelectObject(mhDC, hObj); };
  int SetBkMode(INT iMode)
  { return ::SetBkMode(mhDC, iMode); };
  COLORREF SetTextColor(COLORREF clrColor)
  { return ::SetTextColor(mhDC, clrColor); };
  COLORREF SetBkColor(COLORREF clrColor)
  { return ::SetBkColor(mhDC, clrColor); };
  BOOL Ellipse(int iLeftRect, int iTopRect, int iRightRect, int iBottomRect)
  { return ::Ellipse(mhDC, iLeftRect, iTopRect, iRightRect, iBottomRect); };

  static tBool FillBitmapInfo(BITMAPINFOHEADER* bi, int w, int h, int bpp)
  {
    ZeroMemory(bi, sizeof(BITMAPINFOHEADER));
    bi->biSize = sizeof(BITMAPINFOHEADER);
    bi->biBitCount = (WORD)bpp;
    bi->biPlanes = 1;
    bi->biWidth = w;
    bi->biHeight = -h;
    bi->biClrUsed = 256;
    bi->biCompression = BI_RGB;
    return eTrue;
  }

  tBool StretchBlitToDC(int bmp_w, int bmp_h, int bmp_bpp, BYTE* bmp_pixels,
                        int src_x, int src_y, int src_w, int src_h,
                        int dest_x, int dest_y, int dest_w, int dest_h)
  {
    if (!bmp_pixels)
      return eFalse;

    BITMAPINFO bi; ZeroMemory(&bi,sizeof(bi));

    if (!FillBitmapInfo(&bi.bmiHeader, bmp_w, bmp_h, bmp_bpp))
      return eFalse;

    if (StretchDIBits(mhDC, dest_x, dest_y, dest_w, dest_h,
                      src_x, bmp_h - src_y - src_h, src_w, src_h,
                      bmp_pixels, &bi, DIB_RGB_COLORS, SRCCOPY) == GDI_ERROR)
      return eFalse;

    return eTrue;
  }

  tBool BlitToDC(int bmp_w, int bmp_h, int bmp_bpp, BYTE* bmp_pixels,
                 int src_x, int src_y, int dest_x, int dest_y,
                 int w, int h)
  {
    return StretchBlitToDC(bmp_w,bmp_h,bmp_bpp,bmp_pixels,
                           src_x, src_y, w, h, dest_x, dest_y, w, h);
  }

 protected:
  BOOL mbReleaseDC;
  HDC mhDC;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}} // namespace ni { namespace Windows {
#endif // __WIN32_DC_H_5E6BE155_53F3_4658_B343_A6FA20631A00__
