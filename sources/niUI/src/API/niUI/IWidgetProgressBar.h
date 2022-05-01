#ifndef __IPROGRESSBARWIDGET_38636000_H__
#define __IPROGRESSBARWIDGET_38636000_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

enum eWidgetProgressBarStyle {
  eWidgetProgressBarStyle_Vert = niBit(eWidgetStyle_MaxBit+1),
  eWidgetProgressBarStyle_DWORD = 0xFFFFFFFF
};

struct iWidgetProgressBar : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetProgressBar,0x05ab2a34,0x0ad3,0x4c69,0xaf,0x6b,0xd6,0x29,0x8b,0x01,0xeb,0x8a)

  virtual void __stdcall SetRange(tF32 afMin,tF32 afMax) = 0;
  //! {Property}
  virtual void __stdcall SetProgress(tF32 afPos) = 0;
  //! {Property}
  virtual tF32 __stdcall GetProgress() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IPROGRESSBARWIDGET_38636000_H__
