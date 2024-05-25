#ifndef __IWIDGETTEXT_16284140_H__
#define __IWIDGETTEXT_16284140_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "ITextObject.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! Widget text style
enum eWidgetTextStyle {
  //! Allow text selection with the mouse
  eWidgetTextStyle_MouseSelect = niBit(eWidgetStyle_MaxBit+1),
  //! \internal
  eWidgetTextStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! iWidgetText interface
//! \remark Implements the iWidgetText interface.
struct iWidgetText : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetText,0x4c922f2d,0x7732,0x4093,0x8d,0x68,0x5c,0x77,0x00,0x6f,0xa6,0x8e);

  //! Get the text object of the text widget
  //! {Property}
  virtual iTextObject* __stdcall GetTextObject() const = 0;
};

#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETTEXT_16284140_H__
