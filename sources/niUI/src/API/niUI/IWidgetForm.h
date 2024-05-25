#ifndef __IWIDGETFORM_516843_H__
#define __IWIDGETFORM_516843_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Widget form flags.
enum eWidgetFormStyle
{
  //! Allow the form to dock by a parent docking manager.
  eWidgetFormStyle_Dock = niBit(eWidgetStyle_MaxBit+0),
  //! Allow the form to be moved.
  eWidgetFormStyle_Move = niBit(eWidgetStyle_MaxBit+1),
  //! The form can be resized using the top left corner.
  eWidgetFormStyle_ResizeTopLeftCorner = niBit(eWidgetStyle_MaxBit+2),
  //! The form can be resized using the top right corner.
  eWidgetFormStyle_ResizeTopRightCorner = niBit(eWidgetStyle_MaxBit+3),
  //! The form can be resized using the bottom left corner.
  eWidgetFormStyle_ResizeBottomLeftCorner = niBit(eWidgetStyle_MaxBit+4),
  //! The form can be resized using the bottom right corner.
  eWidgetFormStyle_ResizeBottomRightCorner = niBit(eWidgetStyle_MaxBit+5),
  //! The form can be resized from any corner.
  eWidgetFormStyle_ResizeAnyCorner = eWidgetFormStyle_ResizeTopLeftCorner|eWidgetFormStyle_ResizeTopRightCorner|eWidgetFormStyle_ResizeBottomLeftCorner|eWidgetFormStyle_ResizeBottomRightCorner,
  //! The form can be resized using the left edge.
  eWidgetFormStyle_ResizeLeftEdge = niBit(eWidgetStyle_MaxBit+6),
  //! The form can be resized using the right edge.
  eWidgetFormStyle_ResizeRightEdge = niBit(eWidgetStyle_MaxBit+7),
  //! The form can be resized using the top edge.
  eWidgetFormStyle_ResizeTopEdge = niBit(eWidgetStyle_MaxBit+8),
  //! The form can be resized using the bottom edge.
  eWidgetFormStyle_ResizeBottomEdge = niBit(eWidgetStyle_MaxBit+9),
  //! The form can be resized from any edge.
  eWidgetFormStyle_ResizeAnyEdge = eWidgetFormStyle_ResizeLeftEdge|eWidgetFormStyle_ResizeRightEdge|eWidgetFormStyle_ResizeTopEdge|eWidgetFormStyle_ResizeBottomEdge,
  //! The form can be resized using any of the edges and corners.
  eWidgetFormStyle_ResizeAny = eWidgetFormStyle_ResizeAnyCorner|eWidgetFormStyle_ResizeAnyEdge,
  //! The form has a close button. (ID_Close)
  //! \remark The default handler will destroy the form.
  eWidgetFormStyle_CloseButton = niBit(eWidgetStyle_MaxBit+10),
  //! The form has the form manager buttons, these are the minimize, maximize and restore buttons. (ID_Minimize, ID_Maximize, ID_Restore)
  //! \remark The default handler will call the parent form manager with the command corresponding to the pressed button.
  eWidgetFormStyle_FormManagerButtons = niBit(eWidgetStyle_MaxBit+11),
  //! The docking overlays are always displayed when moving. The
  //! default behavior is that the 'Control' input modifier needs to
  //! be pressed while moving the form to display the docking
  //! overlays.
  eWidgetFormStyle_DockNoInputModifier = niBit(eWidgetStyle_MaxBit+12),
  //! Default form flags.
  eWidgetFormStyle_Default = eWidgetFormStyle_Dock|eWidgetFormStyle_ResizeAny|eWidgetFormStyle_Move,
  //! Default form flags with close button.
  eWidgetFormStyle_DefaultButtons = eWidgetFormStyle_Dock|eWidgetFormStyle_ResizeAny|eWidgetFormStyle_Move|eWidgetFormStyle_CloseButton|eWidgetFormStyle_FormManagerButtons,
  //! Default form flags with close and form manager buttons.
  eWidgetFormStyle_DefaultManagerButtons = eWidgetFormStyle_Dock|eWidgetFormStyle_ResizeAny|eWidgetFormStyle_Move,
  //! \internal
  eWidgetFormStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Form widget interface.
//! \remark Form widgets implements iWidgetDockable.
struct iWidgetForm : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetForm,0x3b96ee80,0x4ae2,0x4aed,0x85,0x1c,0x7f,0xbf,0xd5,0xfd,0x3a,0xfb)

  //! Set the form's title.
  //! {Property}
  virtual tBool __stdcall SetTitle(const achar* aVal) = 0;
  //! Get the form's title.
  //! {Property}
  virtual const achar* __stdcall GetTitle() const = 0;
  //! Set the size/area of the resize border.
  //! {Property}
  virtual tBool __stdcall SetResizeBorderArea(tF32 aVal) = 0;
  //! Get the size/area of the resize border.
  //! {Property}
  virtual tF32 __stdcall GetResizeBorderArea() const = 0;

  //! Get the number of widgets in the title bar.
  //! {Property}
  virtual tU32 __stdcall GetNumTitleWidgets() const = 0;
  //! Get the title bar widget at the specified index.
  //! {Property}
  virtual iWidget* __stdcall GetTitleWidget(tU32 anIndex) const = 0;
  //! Get the index of the specified title bar widget.
  //! {Property}
  virtual tU32 __stdcall GetTitleWidgetIndex(iWidget* apWidget) const = 0;
  //! Get the title bar widget with the specified ID.
  //! {Property}
  virtual iWidget* __stdcall GetTitleWidgetFromID(iHString* ahspID) const = 0;
  //! Add a title bar widget.
  //! \remark The widget's parent is always set to the form's 'title dock area'.
  virtual tBool __stdcall AddTitleWidget(iWidget* apWidget, tF32 afWidth) = 0;

  //! Set the border frame flags.
  //! {Property}
  virtual void __stdcall SetFormFrameFlags(tRectFrameFlags aFlags) = 0;
  //! Get the form frame flags.
  //! {Property}
  virtual tRectFrameFlags __stdcall GetFormFrameFlags() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETFORM_516843_H__
