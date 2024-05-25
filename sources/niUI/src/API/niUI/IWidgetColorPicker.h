#ifndef __IWIDGETCOLORPICKER_17135440_H__
#define __IWIDGETCOLORPICKER_17135440_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Color picker notification messages.
enum eWidgetColorPickerCmd {
  //! Sent when the color changed.
  eWidgetColorPickerCmd_ColorChanged = 0,
  //! \internal
  eWidgetColorPickerCmd_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Color picker style.
enum eWidgetColorPickerStyle
{
  //! Show the brightness control.
  eWidgetColorPickerStyle_Brightness = niBit(eWidgetStyle_MaxBit+0),
  //! \internal
  eWidgetColorPickerStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Widget ColorPicker viewer.
struct iWidgetColorPicker : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetColorPicker,0xde06c422,0xdfba,0x4bbb,0xba,0xba,0x35,0x81,0x2a,0x42,0x44,0xd4)

  //! Set the color picker's current color in HSV format.
  //! {Property}
  virtual void __stdcall SetHSV(const sColor3f& avHSV) = 0;
  //! Get the color picker's current color in HSV format.
  //! {Property}
  virtual sColor3f __stdcall GetHSV() const = 0;
  //! Set the color picker's current color in RGBA format.
  //! {Property}
  virtual void __stdcall SetRGBA(const sColor4f& avRGBA) = 0;
  //! Get the color picker's current color in RGBA format.
  //! {Property}
  virtual sColor4f __stdcall GetRGBA() const = 0;
  //! Set the color picker's current color in RGB format.
  //! {Property}
  virtual void __stdcall SetRGB(const sColor3f& avRGB) = 0;
  //! Get the color picker's current color in RGB format.
  //! {Property}
  virtual sColor3f __stdcall GetRGB() const = 0;
  //! Set the color picker's current color alpha.
  //! {Property}
  virtual void __stdcall SetAlpha(tF32 afAlpha) = 0;
  //! Get the color picker's current color alpha.
  //! {Property}
  virtual tF32 __stdcall GetAlpha() const = 0;
  //! Set the color picker's current color brightness.
  //! {Property}
  virtual void __stdcall SetBrightness(const tF32 afBrightness) = 0;
  //! Get the color picker's current color brightness.
  //! {Property}
  virtual tF32 __stdcall GetBrightness() const = 0;
  //! Set the color picker's current color maximum brightness.
  //! {Property}
  virtual void __stdcall SetMaxBrightness(const tF32 afMaxBrightness) = 0;
  //! Get the color picker's current color maximum brightness.
  //! {Property}
  virtual tF32 __stdcall GetMaxBrightness() const = 0;
  //! Set the color picker's current color from a color name. \see ni::eColor
  //! {Property}
  virtual tBool __stdcall SetColorName(iHString* ahspName) = 0;
  //! Get the color picker's current color as color name. \see ni::eColor
  //! {Property}
  virtual const achar* __stdcall GetColorName() const = 0;
  //! Set the color picker's current color from a mathematic expression.
  //! {Property}
  virtual tBool __stdcall SetExpression(iHString* ahspExpression) = 0;
  //! Get the color picker's current color as a mathematic expression.
  //! {Property}
  virtual iHString* __stdcall GetExpression() const  = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETCOLORPICKER_17135440_H__
