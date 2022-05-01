#ifndef __IWIDGETCANVAS_16498857_H__
#define __IWIDGETCANVAS_16498857_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Canvas widget styles
enum eWidgetCanvasStyle
{
  //! A horizontal scroll bar will be shown if the canvas client size is bigger
  //! than the canvas's rectangle.
  //! \remark The width of the client rectangle wont be adjusted to the
  //!     canvas rectangle.
  eWidgetCanvasStyle_ScrollH = niBit(eWidgetStyle_MaxBit+0),
  //! A vertical scroll bar will be shown if the canvas client size is bigger
  //! than the canvas's rectangle.
  //! \remark The height of the client rectangle wont be adjusted to the
  //!     canvas rectangle.
  eWidgetCanvasStyle_ScrollV = niBit(eWidgetStyle_MaxBit+1),
  //! The horizontal scroll bar will never be set visible.
  eWidgetCanvasStyle_HideScrollH = niBit(eWidgetStyle_MaxBit+2),
  //! The vertical scroll bar will never be set visible.
  eWidgetCanvasStyle_HideScrollV = niBit(eWidgetStyle_MaxBit+3),
  //! \internal
  eWidgetCanvasStyle_ForceDWORD = 0xFFFFFFFF
};

//! Canvas widget interface.
struct iWidgetCanvas : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetCanvas,0xcd67d2fc,0x71da,0x4054,0xae,0x79,0xbf,0x6d,0xd2,0x27,0x44,0xd6)

  //! Get the vertical scroll bar.
  //! {Property}
  virtual iWidget* __stdcall GetScrollV() const = 0;
  //! Set the scroll bar vertical step size.
  //! {Property}
  //! \remark The default is 20
  virtual void __stdcall SetScrollStepV(tF32 afV) = 0;
  //! Get the scroll bar vertical step size.
  //! {Property}
  virtual tF32 __stdcall GetScrollStepV() const = 0;
  //! Set the scroll bar vertical range margin.
  //! {Property}
  //! \remark The default is zero
  virtual void __stdcall SetScrollMarginV(tF32 afV) = 0;
  //! Get the scroll bar vertical range margin.
  //! {Property}
  virtual tF32 __stdcall GetScrollMarginV() const = 0;

  //! Get the horizontal scroll bar.
  //! {Property}
  virtual iWidget* __stdcall GetScrollH() const = 0;
  //! Set the scroll bar horizontal step size.
  //! {Property}
  //! \remark The default is 20
  virtual void __stdcall SetScrollStepH(tF32 afV) = 0;
  //! Get the scroll bar horizontal step size.
  //! {Property}
  virtual tF32 __stdcall GetScrollStepH() const = 0;
  //! Set the scroll bar horizontal range margin.
  //! {Property}
  //! \remark The default is zero
  virtual void __stdcall SetScrollMarginH(tF32 afV) = 0;
  //! Get the scroll bar horizontal range margin.
  //! {Property}
  virtual tF32 __stdcall GetScrollMarginH() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETCANVAS_16498857_H__
