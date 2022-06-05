#ifndef __IBUTTONWIDGET_76418096_H__
#define __IBUTTONWIDGET_76418096_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IWidget.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */

enum eWidgetButtonStyle {
  //! Toggle button.
  eWidgetButtonStyle_OnOff = niBit(eWidgetStyle_MaxBit+0),
  //! Check box button.
  eWidgetButtonStyle_CheckBox = niBit(eWidgetStyle_MaxBit+1),
  //! Radio button.
  eWidgetButtonStyle_RadioButton = niBit(eWidgetStyle_MaxBit+2),
  //! Tab button.
  eWidgetButtonStyle_TabButton = niBit(eWidgetStyle_MaxBit+3),
  //! Draw the button in a selected state when checked.
  eWidgetButtonStyle_Select = niBit(eWidgetStyle_MaxBit+4),
  //! Dont draw the text.
  eWidgetButtonStyle_NoText = niBit(eWidgetStyle_MaxBit+5),
  //! The button will stay pressed and 'click' once it has been pressed, even
  //! if the mouse moves out of it.
  eWidgetButtonStyle_Sticky = niBit(eWidgetStyle_MaxBit+6),
  //! Dont draw the button frame.
  eWidgetButtonStyle_NoFrame = niBit(eWidgetStyle_MaxBit+7),
  //! Set the icon size automatically to fit into the button.
  //! \remark Fit keeps the ratio of the button.
  eWidgetButtonStyle_IconFit = niBit(eWidgetStyle_MaxBit+8),
  //! Set the icon size automatically to fill the button.
  //! \remark Stretch doesnt keep the icon ratio.
  eWidgetButtonStyle_IconStretch = niBit(eWidgetStyle_MaxBit+9),
  //! The icon position is on the left. The text position is on the right.
  //! \remark This is the default if not position is specified.
  eWidgetButtonStyle_IconLeft = 0,
  //! The icon position is on the right. The text position is on the left.
  eWidgetButtonStyle_IconRight = niBit(eWidgetStyle_MaxBit+10),
  //! The icon position is on the top. The text position is in the bottom.
  eWidgetButtonStyle_IconTop = niBit(eWidgetStyle_MaxBit+11),
  //! The icon position is in the bottom. The text position is on the top.
  eWidgetButtonStyle_IconBottom = niBit(eWidgetStyle_MaxBit+12),
  //! The icon position is in the center. The text position is over the icon.
  eWidgetButtonStyle_IconCenter = niBit(eWidgetStyle_MaxBit+13),
  //! The text is left/top aligned.
  eWidgetButtonStyle_TextLeft = niBit(eWidgetStyle_MaxBit+14),
  //! The text is right/bottom aligned.
  eWidgetButtonStyle_TextRight = niBit(eWidgetStyle_MaxBit+15),
  //! The text is center aligned.
  //! \remark This is the default if no text position is specified.
  eWidgetButtonStyle_TextCenter = 0,
  //! Bitmap button. Text is drawn over the icon.
  eWidgetButtonStyle_BitmapButton = eWidgetButtonStyle_IconCenter,
  //! Icon button, same as bitmap button excepted that the text is drawn below the icon.
  eWidgetButtonStyle_IconButton = eWidgetButtonStyle_IconTop,
  //! \internal
  eWidgetButtonStyle_ForceDWORD = 0xFFFFFFFF
};

//! Button notification messages.
enum eWidgetButtonCmd {
  //! The button has been clicked.
  eWidgetButtonCmd_Clicked = 0,
  //! The button has been pushed.
  eWidgetButtonCmd_Pushed = 1,
  //! The button has been unpushed.
  eWidgetButtonCmd_UnPushed = 2,
  //! The button has been checked.
  eWidgetButtonCmd_Checked = 3,
  //! The button has been unchecked.
  eWidgetButtonCmd_UnChecked = 4,
};

//! Button widget interface.
struct iWidgetButton : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetButton,0x719eeac7,0x6cb0,0x4c6a,0x95,0xbe,0x24,0x20,0xde,0xe1,0x0e,0xb8)

  //! Set the button's icon
  //! {Property}
  virtual void __stdcall SetIcon(iOverlay *apIcon) = 0;
  //! Get the button's icon
  //! {Property}
  virtual iOverlay* __stdcall GetIcon() const = 0;
  //! Set the button's icon in pressed state.
  //! {Property}
  virtual void __stdcall SetIconPressed(iOverlay *apIcon) = 0;
  //! Get the button's icon in pressed state.
  //! {Property}
  virtual iOverlay* __stdcall GetIconPressed() const = 0;
  //! Set the button's icon in hover state.
  //! {Property}
  virtual void __stdcall SetIconHover(iOverlay *apIcon) = 0;
  //! Get the button's icon in hover state.
  //! {Property}
  virtual iOverlay* __stdcall GetIconHover() const = 0;
  //! Set the button's icon size.
  //! {Property}
  //! \remark If zero the size of the icon image is used.
  virtual void __stdcall SetIconSize(const sVec2f& avSize) = 0;
  //! Get the button's icon size.
  //! {Property}
  virtual sVec2f __stdcall GetIconSize() const = 0;
  //! Set the button check status
  //! {Property}
  virtual void __stdcall SetCheck(tBool abCkecked) = 0;
  //! Get the button check status
  //! {Property}
  virtual tBool __stdcall GetCheck() const = 0;
  //! Set the button's group id.
  //! {Property}
  virtual tBool __stdcall SetGroupID(iHString* aVal) = 0;
  //! Get the button's group id.
  //! {Property}
  virtual iHString* __stdcall GetGroupID() const = 0;
  //! Set the icon margin.
  //! {Property}
  //! \remark Default margin is Vec4(3,3,3,3)
  virtual void __stdcall SetIconMargin(const sVec4f& avMargin) = 0;
  //! Get the icon margin.
  //! {Property}
  virtual sVec4f __stdcall GetIconMargin() const = 0;
  //! Get the icon's drawing rectangle, client rectangle relative.
  //! {Property}
  virtual sRectf __stdcall GetIconDrawRect() const = 0;
  //! Set the draw frame flags.
  //! {Property}
  virtual void __stdcall SetDrawFrameFlags(tRectFrameFlags aFlags) = 0;
  //! Get the draw frame flags.
  //! {Property}
  virtual tRectFrameFlags __stdcall GetDrawFrameFlags() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IBUTTONWIDGET_76418096_H__
