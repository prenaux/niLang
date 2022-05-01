#ifndef __ILABELWIDGET_1093531_H__
#define __ILABELWIDGET_1093531_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Label widget style.
enum eWidgetLabelStyle {
  eWidgetLabelStyle_TransparentBackground = niBit(eWidgetStyle_MaxBit+1),
  eWidgetLabelStyle_DWORD = 0xFFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Label widget interface.
struct iWidgetLabel : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetLabel,0xa407259f,0x7fa3,0x4eac,0x9b,0xf0,0x58,0x2c,0x5c,0xb3,0x4e,0x76);

  //! Set the label's font format flags.
  //! {Property}
  virtual void __stdcall SetFontFormatFlags(tFontFormatFlags aFlags) = 0;
  //! Get the label's font format flags.
  //! {Property}
  virtual tFontFormatFlags __stdcall GetFontFormatFlags() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ILABELWIDGET_1093531_H__
