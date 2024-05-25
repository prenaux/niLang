#ifndef __IWIDGET_6023245_H__
#define __IWIDGET_6023245_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include <niLang/IConcurrent.h>
#include "IGraphics.h"

namespace ni {

#undef GetClassName // Define by the Win32 API

struct iUIContext;
struct iWidget;
struct iWidgetSink;

/** \addtogroup niUI
 * @{
 */

//! Widget style.
enum eWidgetStyle
{
  // Is drawn in the NC area.
  eWidgetStyle_NCRelative = niBit(0),
  //! Free widget.
  //! \remark Free widgets are not affected by the ZOrder nor the clipping of their parent.
  //! \remark Free widgets are organized by the desktop widget.
  eWidgetStyle_Free = niBit(1),
  //! The widget will receive input focus.
  eWidgetStyle_HoldFocus = niBit(2),
  //! The widget will receive input focus when the cursor is above it.
  eWidgetStyle_OverFocus = niBit(3),
  //! The widget wont be clipped by it's parent.
  eWidgetStyle_NoClip = niBit(4),
  //! No click and double click messages will be generated.
  //! \remark If the click messages are not used it's a bit cheaper to not generate them at all.
  eWidgetStyle_NoClick = niBit(5),
  //! The parent widget is notified through the ParentNotify message when a message is sent to this widget.
  //! \remark The ParentNotify message is never notified to the parent widget.
  eWidgetStyle_NotifyParent = niBit(6),
  //! Activate the widget just before receiving focus.
  eWidgetStyle_FocusActivate = niBit(7),
  //! Don't serialize.
  eWidgetStyle_DontSerialize = niBit(8),
  //! The item the widget is attached to owns it. This is to instruct
  //! items (such as tree nodes or list entry) to destroy/invalidate the widget when it
  //! isnt used anymore.
  eWidgetStyle_ItemOwned = niBit(9),
  //! The the widget is a dragging source.
  eWidgetStyle_DragSource = niBit(10),
  //! The widget is a dragging destination.
  eWidgetStyle_DragDestination = niBit(11),
  //! The widget is temporary, as for example the hover labels.
  eWidgetStyle_Temp = niBit(12),
  //! Focus won't be set on the widget as a response to the MoveFocus message.
  eWidgetStyle_NoMoveFocus = niBit(13),
  //! Dont redraw the widget automatically on input events.
  eWidgetStyle_NoAutoRedraw = niBit(14),
  //! Widget is a "move focus group".
  //! \remark A move focus group indicates a logical organization of widgets which
  //!         should be all cycled through before going to the next group.
  eWidgetStyle_MoveFocusGroup = niBit(15),
  //! Max bit, user styles should use the bits above this.
  eWidgetStyle_MaxBit = 16,
  //! \internal
  eWidgetStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget style flags type.
typedef tU32 tWidgetStyleFlags;

//! Widget dock style.
//! \remark Docking flags allows to snap/dock the widget on the edges of it's parent control client area.
//! \remark Snap only moves the widget to the specified edges, left and right are mutually exclusive, and
//!     top and bottom are mutually exclusive as well.
//! \remark Dock stretch the widget to reach the specified edge and update the docking area, so that
//!     a fill dock will fill only the empty space left.
//! \remark The widgets are docked in the ZOrder.
enum eWidgetDockStyle
{
  //! No docking.
  eWidgetDockStyle_None = 0,
  //! Snap the widget on the left.
  eWidgetDockStyle_SnapLeft = 1,
  //! Snap the widget on the right.
  eWidgetDockStyle_SnapRight = 2,
  //! Snap the widget on the top.
  eWidgetDockStyle_SnapTop = 3,
  //! Snap the widget on the bottom.
  eWidgetDockStyle_SnapBottom = 4,
  //! Snap the widget on the center left.
  eWidgetDockStyle_SnapCenterLeft = 5,
  eWidgetDockStyle_SnapLeftCenter = 5,
  //! Snap the widget on the center right.
  eWidgetDockStyle_SnapCenterRight = 6,
  eWidgetDockStyle_SnapRightCenter = 6,
  //! Snap the widget on the center top.
  eWidgetDockStyle_SnapCenterTop = 7,
  eWidgetDockStyle_SnapTopCenter = 7,
  //! Snap the widget on the center bottom.
  eWidgetDockStyle_SnapCenterBottom = 8,
  eWidgetDockStyle_SnapBottomCenter = 8,
  //! Snap the widget on the top left.
  eWidgetDockStyle_SnapTopLeft = 9,
  eWidgetDockStyle_SnapLeftTop = 9,
  //! Snap the widget on the top right.
  eWidgetDockStyle_SnapTopRight = 10,
  eWidgetDockStyle_SnapRightTop = 10,
  //! Snap the widget on the bottom left.
  eWidgetDockStyle_SnapBottomLeft = 11,
  eWidgetDockStyle_SnapLeftBottom = 11,
  //! Snap the widget on the bottom right.
  eWidgetDockStyle_SnapBottomRight = 12,
  eWidgetDockStyle_SnapRightBottom = 12,
  //! Snap the widget in the center.
  eWidgetDockStyle_SnapCenter = 13,
  //! Snap the widget in the center horizontally.
  eWidgetDockStyle_SnapCenterH = 14,
  //! Snap the widget in the center vertically.
  eWidgetDockStyle_SnapCenterV = 15,
  //! Dock the widget on the left.
  eWidgetDockStyle_DockLeft = 16,
  //! Dock the widget on the right.
  eWidgetDockStyle_DockRight = 17,
  //! Dock the widget on the top.
  eWidgetDockStyle_DockTop = 18,
  //! Dock the widget on the bottom.
  eWidgetDockStyle_DockBottom = 19,
  //! Dock the widget to fill the widget.
  eWidgetDockStyle_DockFill = 20,
  //! Dock the widget to fill the widget width.
  eWidgetDockStyle_DockFillWidth = 21,
  //! Dock the widget to fill the widget height.
  eWidgetDockStyle_DockFillHeight = 22,
  //! Dock the widget to fill the all widget, this is meant to be used by overlays.
  eWidgetDockStyle_DockFillOverlay = 23,
  //! Place the widget in a grid.
  eWidgetDockStyle_Grid = 24,
  //! \internal
  eWidgetDockStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget ZOrder.
//! \remark These are the reserved ZOrder, to define a ZOrder explicitly
//!     pass a value less that eWidgetZOrder_Max.
//!     Zero is the overlay widget, higher values are below it.
enum eWidgetZOrder
{
  //! Places the widget in the background behind all background widgets.
  eWidgetZOrder_BackgroundBottom = 0,
  //! Places the widget in the background.
  eWidgetZOrder_Background = 1,
  //! Places the widget at the bottom of the Z order.
  eWidgetZOrder_Bottom = 2,
  //! Places the widget at the top of the Z order.
  eWidgetZOrder_Top = 3,
  //! Places the widget above all non-topmost widgets but below all top-most widgets.
  eWidgetZOrder_TopMostBottom = 4,
  //! Places the widget above all non-topmost widgets.
  eWidgetZOrder_TopMost = 5,
  //! Places the widget above all other widgets, but below all overlay widgets.
  eWidgetZOrder_OverlayBottom = 6,
  //! Overlay widget.
  eWidgetZOrder_Overlay = 7,
  //! \internal
  eWidgetZOrder_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget serialize flags.
enum eWidgetSerializeFlags
{
  //! Serialize write.
  //! \remark Read or Write must be specified, both can't be specified simultaneously.
  //! \remark This is intentionally the same as ni::eSerializeFlags_Write
  eWidgetSerializeFlags_Write = niBit(0),
  //! Serialize read.
  //! \remark Read or Write must be specified, both can't be specified simultaneously.
  //! \remark This is intentionally the same as ni::eSerializeFlags_Read
  eWidgetSerializeFlags_Read = niBit(1),
  //! Serialize (read/write) the children of the widget.
  eWidgetSerializeFlags_Children = niBit(2),
  //! Don't serialize the root widget's information.
  //! \remark What is considered the root widget is the widget passed to SerializeLayout.
  eWidgetSerializeFlags_NoRoot = niBit(3),
  //! Edition property box serialization.
  //! \remark Additional or different informations that are more suited for
  //!     edition should be serialized when writting in a property box.
  //! \remark This is intentionally the same as ni::eSerializeFlags_TypeInfoMetadata.
  eWidgetSerializeFlags_PropertyBox = niBit(10),
  //! \internal
  eWidgetSerializeFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

niCAssert((ni::tU32)eWidgetSerializeFlags_Write == (ni::tU32)eSerializeFlags_Write);
niCAssert((ni::tU32)eWidgetSerializeFlags_Read == (ni::tU32)eSerializeFlags_Read);
niCAssert((ni::tU32)eWidgetSerializeFlags_PropertyBox == (ni::tU32)eSerializeFlags_TypeInfoMetadata);

//! Widget serialize flags type. \see ni::eWidgetSerializeFlags
typedef tU32 tWidgetSerializeFlags;

//! Widget auto layout flags.
enum eWidgetAutoLayoutFlags
{
  //! Compute the children's relative sizes.
  eWidgetAutoLayoutFlags_Relative = niBit(0),
  //! Compute the children's docking positions.
  eWidgetAutoLayoutFlags_Dock = niBit(1),
  //! Compute the size to fit the children.
  eWidgetAutoLayoutFlags_Size = niBit(2),
  //! \internal
  eWidgetAutoLayoutFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF,
};

//! Widget serialize flags type. \see ni::eWidgetAutoLayoutFlags
typedef tU32 tWidgetAutoLayoutFlags;

//! Widget sink interface.
//{DispatchWrapper}
struct iWidgetSink : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetSink,0x6905dfa1,0xfa65,0x4444,0xa3,0x90,0xba,0x80,0x95,0x80,0xdb,0xf6)
  //! Widget sink message handler.
  virtual tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const ni::Var& varParam0, const ni::Var& varParam1) = 0;
};

//! Widget command interface.
struct iWidgetCommand : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetCommand,0x6d19a44f,0x87ad,0x4517,0xb5,0xae,0xfe,0x57,0xc7,0xa2,0xf2,0x5e)
  //! Copy the specified command in this command.
  virtual tBool __stdcall Copy(const iWidgetCommand* apSrc) = 0;
  //! Clone this command.
  virtual iWidgetCommand* __stdcall Clone() const = 0;
  //! Set the command sender.
  //! {Property}
  virtual tBool __stdcall SetSender(iWidget* apSender) = 0;
  //! Get the command sender.
  //! {Property}
  virtual iWidget* __stdcall GetSender() const = 0;
  //! Set the command id.
  //! {Property}
  virtual void __stdcall SetID(tU32 anID) = 0;
  //! Get the command id.
  //! {Property}
  virtual tU32 __stdcall GetID() const = 0;
  //! Set the extra parameter 1.
  //! {Property}
  virtual void __stdcall SetExtra1(const Var& aVar) = 0;
  //! Get the extra parameter 1.
  //! {Property}
  virtual const Var& __stdcall GetExtra1() const = 0;
  //! Set the extra parameter 2.
  //! {Property}
  virtual void __stdcall SetExtra2(const Var& aVar) = 0;
  //! Get the extra parameter 2.
  //! {Property}
  virtual const Var& __stdcall GetExtra2() const = 0;
};

//! Widget system timers.
enum eWidgetSystemTimer
{
  //! First system timer id.
  eWidgetSystemTimer_First = eInvalidHandle+1,
  //! Hover system timer.
  eWidgetSystemTimer_Hover = eWidgetSystemTimer_First+0,
  //! Non-client area hover system timer.
  eWidgetSystemTimer_NCHover = eWidgetSystemTimer_First+1,
  //! Left doubleclick timer.
  eWidgetSystemTimer_LeftDoubleClick = eWidgetSystemTimer_First+2,
  //! Right doubleclick timer.
  eWidgetSystemTimer_RightDoubleClick = eWidgetSystemTimer_First+3,
  //! \internal
  eWidgetSystemTimer_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Widget interface.
struct iWidget : public iMessageHandler
{
  niDeclareInterfaceUUID(iWidget,0x84722d61,0x18b8,0x4081,0xab,0xce,0x5a,0xfe,0x18,0x6f,0x3f,0xda)

  //########################################################################################
  //! \name Widget
  //########################################################################################
  //! @{

  //! Destroy the widget and it's children.
  //! \remark Will send the eUIMessage_Destroy message and invalidate the widget.
  //! \remark This is not done automatically when releasing the widget, it's meant to be
  //!     used to force the closing of application widgets at run-time manually.
  virtual void __stdcall Destroy() = 0;
  //! Get the widget's Graphics object.
  //! {Property}
  virtual iGraphics* __stdcall GetGraphics() const = 0;
  //! Get the widget's Graphics Context.
  //! {Property}
  virtual iGraphicsContext* __stdcall GetGraphicsContext() const = 0;
  //! Get the widget's UI context.
  //! {Property}
  virtual iUIContext* __stdcall GetUIContext() const = 0;
  //! Get the widget's class.
  //! {Property}
  virtual iHString* __stdcall GetClassName() const = 0;
  //! Set the widget's ID.
  //! {Property}
  virtual void __stdcall SetID(iHString* ahspID) = 0;
  //! Get the widget's ID.
  //! {Property}
  virtual iHString* __stdcall GetID() const = 0;
  //! Set the widget's style.
  //! {Property}
  virtual tBool __stdcall SetStyle(tU32 anStyle) = 0;
  //! Get the widget's style.
  //! {Property}
  virtual tU32 __stdcall GetStyle() const = 0;
  //! Set the parent widget.
  //! {Property}
  virtual void __stdcall SetParent(iWidget* apParent) = 0;
  //! Get the parent widget.
  //! {Property}
  virtual iWidget* __stdcall GetParent() const = 0;
  //! Set the widget's ZOrder.
  //! {Property}
  //! \see eWidgetZOrder
  virtual void __stdcall SetZOrder(eWidgetZOrder aZOrder) = 0;
  //! Get the widget's ZOrder.
  //! {Property}
  virtual eWidgetZOrder __stdcall GetZOrder() const = 0;
  //! Place the widget above (on top of) the specified sibling widget in the ZOrder.
  virtual void __stdcall SetZOrderAbove(iWidget* apWidget) = 0;
  //! Get the drawing order.
  //! {Property}
  virtual tU32 __stdcall GetDrawOrder() const = 0;
  //! Set if the widget's auto layout flags.
  //! {Property}
  //! \remark If a child's relative position or size is set, the relative auto layout will be set.
  //! \remark If a child's dock style is different of none the dock auto layout will be set.
  virtual void __stdcall SetAutoLayout(tWidgetAutoLayoutFlags aFlags) = 0;
  //! Get if the widget's auto layout flags.
  //! {Property}
  virtual tWidgetAutoLayoutFlags __stdcall GetAutoLayout() const = 0;
  //! Computer the widget children's auto layout.
  virtual void __stdcall ComputeAutoLayout(tWidgetAutoLayoutFlags aFlags) = 0;
  //! @}

  //########################################################################################
  //! \name Widget rectangle
  //########################################################################################
  //! @{

  //! Set the widget's position.
  //! \remark For a child window the position is relative to it's parent client top-left cornet,
  //!     else the position is relative to the top-left corner of the screen.
  //! {Property}
  virtual void __stdcall SetPosition(const sVec2f& avPos) = 0;
  //! Get the widget's position.
  //! \remark For a child window the position is relative to it's parent client top-left cornet,
  //!     else the position is relative to the top-left corner of the screen.
  //! {Property}
  virtual sVec2f __stdcall GetPosition() const = 0;
  //! Set the widget's size.
  //! {Property}
  virtual void __stdcall SetSize(const sVec2f& avSize) = 0;
  //! Get the widget's size.
  //! {Property}
  virtual sVec2f __stdcall GetSize() const = 0;
  //! Set the widget's minimum size.
  //! \remark If a component is <= 0 the limit is not enforced.
  //! {Property}
  virtual void __stdcall SetMinimumSize(sVec2f avMinSize) = 0;
  //! Get the widget's minimum size.
  //! {Property}
  virtual sVec2f __stdcall GetMinimumSize() const = 0;
  //! Set the widget's maximum size.
  //! \remark If a component is <= 0 the limit is not enforced.
  //! {Property}
  virtual void __stdcall SetMaximumSize(sVec2f avMaxSize) = 0;
  //! Get the widget's maximum size.
  //! {Property}
  virtual sVec2f __stdcall GetMaximumSize() const = 0;
  //! Set the widget's rectangle.
  //! \remark For a child window the position is relative to it's parent client top-left cornet,
  //!     else the position is relative to the top-left corner of the screen.
  //! {Property}
  virtual void __stdcall SetRect(const sRectf& aRect) = 0;
  //! Get the widget's rectangle.
  //! \remark For a child window the position is relative to it's parent client top-left cornet,
  //!     else the position is relative to the top-left corner of the screen.
  //! {Property}
  virtual sRectf __stdcall GetRect() const = 0;
  //! Get the widget rectangle. Relative to it's own top-left corner.
  //! {Property}
  virtual sRectf __stdcall GetWidgetRect() const = 0;
  //! Get the widget's dock fill rectangle.
  //! \remark The DockFill rectangle is where DockFill widget at the end of the draw
  //!     order will be placed.
  //! \remark The DockFill rectangle is relative to the client area.
  //! \remark The DockFill rectangle is reliable only after ComputeLayout has been called.
  //! {Property}
  virtual sRectf __stdcall GetDockFillRect() const = 0;
  //! @}

  //########################################################################################
  //! \name Widget client rectangle
  //########################################################################################
  //! @{

  //! Set the widget's client position.
  //! \remark The position is relative to the widget's rectangle top-left corner.
  //! {Property}
  virtual void __stdcall SetClientPosition(const sVec2f& avPos) = 0;
  //! Get the widget's client position.
  //! \remark The position is relative to the widget's rectangle top-left corner.
  //! {Property}
  virtual sVec2f __stdcall GetClientPosition() const = 0;
  //! Set the widget's client size.
  //! {Property}
  virtual void __stdcall SetClientSize(const sVec2f& avSize) = 0;
  //! Get the widget's client size.
  //! {Property}
  virtual sVec2f __stdcall GetClientSize() const = 0;
  //! Set the widget's client rectangle.
  //! \remark The position is relative to the widget's rectangle top-left corner.
  //! {Property}
  virtual void __stdcall SetClientRect(const sRectf& aRect) = 0;
  //! Get the widget's client rectangle.
  //! \remark The position is relative to the widget's rectangle top-left corner.
  //! {Property}
  virtual sRectf __stdcall GetClientRect() const = 0;
  //! Compute a rectangle that fits around the specified client rectangle.
  //! \param  aRect is the client rectangle in absolute coordinates.
  //! \return The rectangle, is absolute coordinates, that fits are the specified client rectangle.
  //! \remark Return a rectangle that can be used with SetAbsoluteRect.
  virtual sRectf __stdcall ComputeFitRect(const sRectf& aRect) const = 0;
  //! Set a rectangle that fits around the specified client rectangle.
  //! \param  aRect is the client rectangle in absolute coordinates.
  //! {Property}
  virtual void __stdcall SetFitRect(const sRectf& aRect) = 0;
  //! Set a rectangle that fits around the specified client size.
  //! \param avSize is the client size.
  //! {Property}
  virtual void __stdcall SetFitSize(const sVec2f avSize) = 0;
  //! @}

  //########################################################################################
  //! \name Widget absolute rectangle
  //########################################################################################
  //! @{

  //! Set the widget's absolute position.
  //! \remark The position is relative to the screen's top-left corner.
  //! {Property}
  virtual void __stdcall SetAbsolutePosition(const sVec2f& avPos) = 0;
  //! Get the widget's absolute position.
  //! \remark The position is relative to the screen's top-left corner.
  //! {Property}
  virtual sVec2f __stdcall GetAbsolutePosition() const = 0;
  //! Set the widget's absolute rectangle.
  //! \remark The absolute rectangle size is the same as the widget's size.
  //! {Property}
  virtual void __stdcall SetAbsoluteRect(const sRectf& aRect) = 0;
  //! Get the widget's absolute rectangle.
  //! {Property}
  virtual sRectf __stdcall GetAbsoluteRect() const = 0;
  //! @}

  //########################################################################################
  //! \name Widget clipped rectangle.
  //########################################################################################
  //! @{

  //! Get the widget's rect clipped to the parent client rectangle.
  //! \remark The rectangle will not be clipped only if the NoClip style is specified.
  //! {Property}
  virtual sRectf __stdcall GetClippedRect() const = 0;
  //! Get the widget's absolute rect clipped to the parent client rectangle.
  //! \remark The rectangle will not be clipped only if the NoClip style is specified.
  //! {Property}
  virtual sRectf __stdcall GetAbsoluteClippedRect() const = 0;
  //! Get the widget's client rect clipped to the parent client rectangle.
  //! \remark The rectangle will not be clipped only if the NoClip style is specified.
  //! {Property}
  virtual sRectf __stdcall GetClippedClientRect() const = 0;
  //! Get the widget's absolute client rect clipped to the parent client rectangle.
  //! \remark The rectangle will not be clipped only if the NoClip style is specified.
  //! {Property}
  virtual sRectf __stdcall GetAbsoluteClippedClientRect() const = 0;
  //! @}

  //########################################################################################
  //! \name Widget relative rectangle
  //########################################################################################
  //! @{

  //! Set the widget's relative position.
  //! {Property}
  //! \remark Will enabled the relative auto layout.
  virtual void __stdcall SetRelativePosition(const sVec2f& avPos) = 0;
  //! Get the widget's relative position.
  //! {Property}
  virtual sVec2f __stdcall GetRelativePosition() const = 0;
  //! Set the widget's relative size.
  //! {Property}
  //! \remark Will enabled the relative auto layout.
  virtual void __stdcall SetRelativeSize(const sVec2f& avSize) = 0;
  //! Get the widget's relative size.
  //! {Property}
  virtual sVec2f __stdcall GetRelativeSize() const = 0;
  //! Set the widget's relative rectangle.
  //! {Property}
  //! \remark Will enabled the relative auto layout.
  virtual void __stdcall SetRelativeRect(const sRectf& aRect) = 0;
  //! Get the widget's relative rectangle.
  //! {Property}
  virtual sRectf __stdcall GetRelativeRect() const = 0;
  //! @}

  //########################################################################################
  //! \name Widget padding, border and margin.
  //########################################################################################
  //! @{

  //! Set the padding size.
  //! \remark The padding rectangle specifies the internal padding of the client area for each side of the widget.
  //! {Property}
  virtual void __stdcall SetPadding(const sVec4f& aRect) = 0;
  //! Get the padding size.
  //! {Property}
  virtual sVec4f __stdcall GetPadding() const = 0;
  //! Get whether a padding is specified (non-zero).
  //! {Property}
  virtual tBool __stdcall GetHasPadding() const = 0;

  //! Set the margin rectangle.
  //! \remark The margin rectangle specifies the margin for each side of the widget.
  //! {Property}
  virtual void __stdcall SetMargin(const sVec4f& aRect) = 0;
  //! Get the margin rectangle.
  //! {Property}
  virtual sVec4f __stdcall GetMargin() const = 0;
  //! Get whether a margin is specified (non-zero).
  //! {Property}
  virtual tBool __stdcall GetHasMargin() const = 0;
  //! @}

  //########################################################################################
  //! \name Input
  //########################################################################################
  //! @{

  //! Ask to get the input focus.
  virtual tBool __stdcall SetFocus() = 0;
  //! Move the focus to the previous or next widget.
  virtual tBool __stdcall MoveFocus(tBool abToPrevious) = 0;
  //! Set the capture of the input.
  //! {Property}
  virtual void __stdcall SetCapture(tBool abEnable) = 0;
  //! Get the capture of the input state.
  //! {Property}
  virtual tBool __stdcall GetCapture() const = 0;
  //! Set exclusive.
  //! {Property}
  virtual void __stdcall SetExclusive(tBool abEnable) = 0;
  //! Get the exclusive state.
  //! {Property}
  virtual tBool __stdcall GetExclusive() const = 0;
  //! Set the input submit flags.
  //! \see ni::eUIInputSubmitFlags
  //! {Property}
  virtual void __stdcall SetInputSubmitFlags(tU32 aSubmitFlags) = 0;
  //! Get the input submit flags.
  //! \see ni::eUIInputSubmitFlags
  //! {Property}
  virtual tU32 __stdcall GetInputSubmitFlags() const = 0;
  //! @}

  //########################################################################################
  //! \name Status
  //########################################################################################
  //! @{

  //! Set the widget visibility status.
  //! {Property}
  virtual void __stdcall SetVisible(tBool abVisible) = 0;
  //! Get the widget visibility status.
  //! {Property}
  virtual tBool __stdcall GetVisible() const = 0;
  //! Set the widget enabled status.
  //! {Property}
  virtual void __stdcall SetEnabled(tBool abEnabled) = 0;
  //! Get the widget enabled status.
  //! {Property}
  virtual tBool __stdcall GetEnabled() const = 0;
  //! Set the ignore input status.
  //! {Property}
  virtual void __stdcall SetIgnoreInput(tBool abIgnoreInput) = 0;
  //! Get the ignore input status.
  //! {Property}
  virtual tBool __stdcall GetIgnoreInput() const = 0;
  //! Set all the status.
  virtual void __stdcall SetStatus(tBool abVisible, tBool abEnabled, tBool abIgnoreInput) = 0;
  //! Set whether all client children of the widget are hidden.
  //! {Property}
  virtual void __stdcall SetHideChildren(tBool abHideChildren) = 0;
  //! Get whether all client children of the widget are hidden.
  //! {Property}
  virtual tBool __stdcall GetHideChildren() const = 0;
  //! Get whether the mouse is over this widget's client area.
  //! {Property}
  virtual tBool __stdcall GetIsMouseOver() const = 0;
  //! Get whether the main mouse button is pressed while the mouse is over the widget's client area.
  //! {Property}
  virtual tBool __stdcall GetIsPressed() const = 0;
  //! Get whether the mouse is over this widget's non-client area.
  //! {Property}
  virtual tBool __stdcall GetIsNcMouseOver() const = 0;
  //! Get whether the main mouse button is pressed while the mouse is over the widget's non-client area.
  //! {Property}
  virtual tBool __stdcall GetIsNcPressed() const = 0;
  //! Get whether the widget has input (keyboard) focus.
  //! {Property}
  virtual tBool __stdcall GetHasFocus() const = 0;
  //! Get whether the widget is the current draggin source.
  //! {Property}
  virtual tBool __stdcall GetDraggingSource() const = 0;
  //! Get whether something is currently being dragged from this widget.
  //! {Property}
  virtual tBool __stdcall GetDragging() const = 0;
  //! Force redrawing of the widget's content.
  virtual void __stdcall Redraw() = 0;
  //! @}

  //########################################################################################
  //! \name Timer
  //########################################################################################
  //! @{

  //! Set a timer.
  //! \param  anID is the ID of the timer to set.
  //! \param  afTime is the time in seconds after which the timer will expire.
  //!     If < 0 the timer will be removed.
  //! \remark When the timer expires a eUIMessage_Timer will be sent to the widget with
  //!     the timer ID as parameter.
  //! {Property}
  virtual void __stdcall SetTimer(tU32 anID, tF32 afTime) = 0;
  //! Get a timer.
  //! \remark If the ID specified is not set -1 is returned.
  //! {Property}
  virtual tF32 __stdcall GetTimer(tU32 anID) const = 0;
  //! @}

  //########################################################################################
  //! \name Sinks
  //########################################################################################
  //! @{

  //! Add a widget sink as first sink called.
  virtual tBool __stdcall AddSink(iWidgetSink* apSink) = 0;
  //! Add a widget sink as last sink called.
  virtual tBool __stdcall AddPostSink(iWidgetSink* apSink) = 0;
  //! Add a new instance of a widget sink as first sink called.
  virtual iWidgetSink* __stdcall AddClassSink(const achar* aaszClassName) = 0;
  //! Add a new instance of a widget sink as last sink called.
  virtual iWidgetSink* __stdcall AddClassPostSink(const achar* aaszClassName) = 0;
  //! Remove a widget sink.
  virtual void __stdcall RemoveSink(iWidgetSink* apSink) = 0;
  //! @}

  //########################################################################################
  //! \name Children
  //########################################################################################
  //! @{

  //! Invalidate all children.
  virtual void __stdcall InvalidateChildren() = 0;
  //! Get the number of child widgets.
  //! {Property}
  virtual tU32 __stdcall GetNumChildren() const = 0;
  //! Get the index of the specified widget, if not a child widget return eInvalidHandle.
  //! {Property}
  virtual tU32 __stdcall GetChildIndex(iWidget* apWidget) const = 0;
  //! Get the child widget at the specified index.
  //! {Property}
  virtual iWidget* __stdcall GetChildFromIndex(tU32 anIndex) const = 0;
  //! Get the child widget with the specified ID.
  //! {Property}
  virtual iWidget* __stdcall GetChildFromID(iHString* ahspID) const = 0;
  //! Get the child widget with the specified draw order.
  //! {Property}
  //! \remark 0 is the widget that is drawn first (in the background).
  virtual iWidget* __stdcall GetChildFromDrawOrder(tU32 anDrawOrder) const = 0;
  //! Find the first widget with the specified ID.
  //! \remark This include this widget.
  virtual iWidget* __stdcall FindWidget(iHString* ahspID) const = 0;
  //! Find the top widget that intersects the specified absolute position.
  //! \remark This include this widget.
  virtual iWidget* __stdcall FindWidgetByPos(const sVec2f& avPos) const = 0;
  //! Check whether the specified widget is a child of this widget.
  //! \param abRecursive if true the check will recursively check in all the children of the children.
  virtual tBool __stdcall HasChild(const iWidget* apW, tBool abRecursive) const = 0;
  //! Walks up the parent to check if the specified widget is one of the parent of this widget.
  virtual tBool __stdcall HasParent(const iWidget* apW) const = 0;
  //! @}

  //########################################################################################
  //! \name Appearance
  //########################################################################################
  //! @{

  //! Set the widget's text.
  //! {Property}
  //! \remark The widget's text is meant to store limited amount of text in a standard way.
  //! \remark Controls like the edit box, which edits text should use the standard text
  //!     only to store the default text value - aka value when the widget is created
  //!     from a datatable. Those control's edited text should provide a text_buffer
  //!     property to store the edited text. SetText on those control should also
  //!     reset the content of the text buffer, but GetText still returns only
  //!     the text set with SetText, while GetTextBuffer returns the edited text.
  virtual void __stdcall SetText(iHString* ahspText) = 0;
  //! Get the widget's title.
  //! {Property}
  virtual iHString* __stdcall GetText() const = 0;
  //! Get the localized text.
  //! {Property}
  virtual iHString* __stdcall GetLocalizedText() const = 0;
  //! Set the widget's hovering text.
  //! {Property}
  //! \remark Text drawn in a label when hovering message is sent on the client area. That
  //!     is the default behavior that can be overriden by intersepting the hover
  //!     message.
  virtual void __stdcall SetHoverText(iHString* ahspText) = 0;
  //! Get the widget's hovering text.
  //! {Property}
  virtual iHString* __stdcall GetHoverText() const = 0;
  //! Create a default hover widget (Text only).
  virtual ni::iWidget* __stdcall CreateDefaultHoverWidget(ni::iHString* ahspHoverText) = 0;
  //! Show a hover widget.
  virtual tBool __stdcall ShowHoverWidget(ni::iWidget* apWidget, const sVec2f& avAbsPos) = 0;
  //! Reset the hover widget.
  //! \param abRestart: if true and hover delay >=0 restarts the hover timer, else stops the timer.
  virtual void __stdcall ResetHoverWidget(tBool abRestart) = 0;
  //! Get the currently displayed hover widget.
  //! {Property}
  virtual ni::iWidget* __stdcall GetHoverWidget() const = 0;
  //! Set the widget's font.
  //! \remark By default if not set the skin's font will be used.
  //! \return eTrue if the font is valid, else eFalse.
  //! {Property}
  virtual tBool __stdcall SetFont(iFont* apFont) = 0;
  //! Get the widget's font.
  //! {Property}
  virtual iFont* __stdcall GetFont() const = 0;
  //! Set the widget's dock style.
  //! {Property}
  virtual tBool __stdcall SetDockStyle(eWidgetDockStyle aStyle) = 0;
  //! Get the widget's dock style.
  //! {Property}
  virtual eWidgetDockStyle __stdcall GetDockStyle() const = 0;

  //! Set the widget's skin.
  //! {Property}
  virtual void __stdcall SetSkin(iHString* ahspSkin) = 0;
  //! Get the widget's skin.
  //! {Property}
  virtual iHString* __stdcall GetSkin() const = 0;

  //! Set the widget's skin classs.
  //! {Property}
  //! \remark If not specified it's the widget's class name.
  virtual void __stdcall SetSkinClass(iHString* ahspSkinClass) = 0;
  //! Get the widget's skin class.
  //! {Property}
  virtual iHString* __stdcall GetSkinClass() const = 0;

  //! Set the widget's locale.
  //! {Property}
  virtual void __stdcall SetLocale(iHString* ahspLocale) = 0;
  //! Get the widget's locale.
  //! {Property}
  virtual iHString* __stdcall GetLocale() const = 0;
  //! Get the locale actually used by the widget.
  //! {Property}
  //! \remark This will get the parent's locale if no locale is specified on this widget.
  virtual iHString* __stdcall GetActiveLocale() const = 0;

  //! Find the font of the specified item in the widget's skin.
  virtual iFont*   __stdcall FindSkinFont(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! Find the cursor of the specified item in the widget's skin.
  virtual iOverlay* __stdcall FindSkinCursor(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! Find the element of the specified item in the widget's skin.
  virtual iOverlay* __stdcall FindSkinElement(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! Find the color of the specified item in the widget's skin.
  virtual sColor4f __stdcall FindSkinColor(const sColor4f& aDefault, iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const = 0;
  //! Find a localized text.
  virtual iHString* __stdcall FindLocalized(iHString* ahspText) const = 0;
  //! @}

  //########################################################################################
  //! \name Context menu
  //########################################################################################
  //! @{

  //! Set the widget's context menu. \see ni::eUIMessage::eUIMessage_ContextMenu
  //! {Property}
  //! \remark The parent of the menu wont be changed.
  virtual tBool __stdcall SetContextMenu(iWidget* apMenu) = 0;
  //! Get the widget's context menu.
  //! {Property}
  virtual iWidget* __stdcall GetContextMenu() const = 0;
  //! @}

  //########################################################################################
  //! \name Message
  //########################################################################################
  //! @{

  //! Send a message to this widget.
  virtual tBool __stdcall SendMessage(tU32 anMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) = 0;
  //! Send a command to the specified widget.
  virtual tBool __stdcall SendCommand(iWidget* apDest, tU32 anCommand, const Var& avarExtra1 = niVarNull, const Var& avarExtra2 = niVarNull) = 0;
  //! Send a message to this widget and its children recursivly.
  virtual tBool __stdcall BroadcastMessage(tU32 anMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) = 0;
  //! @}

  //########################################################################################
  //! \name Canvas.
  //########################################################################################
  //! @{

  //! Set the VG canvas of the widget.
  //! {Property}
  virtual void __stdcall SetCanvas(iCanvas* apCanvas) = 0;
  //! Get the VG canvas of the widget.
  //! {Property}
  virtual iCanvas* __stdcall GetCanvas() const = 0;
  //! @}

  //########################################################################################
  //! \name Serialization.
  //########################################################################################
  //! @{

  //! Serialize the layout of this widget.
  virtual tBool __stdcall SerializeLayout(iDataTable* apDT, tWidgetSerializeFlags anFlags) = 0;
  //! Serialize the children widget.
  //! \remark The Children data table passed can contain a _regex_filter regular expression (iRegex) property.
  virtual tBool __stdcall SerializeChildren(iDataTable* apDT, tWidgetSerializeFlags anFlags) = 0;
  //! @}

  //########################################################################################
  //! \name Automated Placement Helpers
  //########################################################################################
  //! @{

  //! Set a dock style, and then restore the previous dock style.
  //! \remark This is useful to set position of widgets relative to their parent.
  virtual tBool __stdcall ApplyDockStyle(eWidgetDockStyle aStyle) = 0;

  //! Place the widget using the specified dock style, margin and rectangle.
  //! \param aRect is the parent-relative rectangle which specify the 'starting' position of the widget, if the size/pos is <= 1.0 the coordinates are assumed to be relative to the widget's parent.
  //! \param aStyle dock style to use the place the widget.
  //! \param avMargin is the size of the margin to use to place the widget.
  virtual tBool __stdcall Place(const sRectf& aRect, eWidgetDockStyle aStyle, const sVec4f& avMargin) = 0;

  //! Makes sure the widget is within the bounds of the specified 'container' widget.
  //! \param apContainer: the container widget, if null the root widget.
  //! \param afSnapMargin: the distance margin below which the
  //!        widget will be snapped to the container's inner border
  virtual tBool __stdcall SnapInside(iWidget* apContainer, tF32 afSnapMargin) = 0;
  //! Move the widget at the specified position making sure it is within the bounds of the specified 'container' widget.
  //! \param apContainer: the container widget, if null the root widget.
  //! \param avAbsPos: the position where the widget should be placed
  //! \param afSnapMargin: the distance margin below which the
  //!        widget will be snapped to the container's inner border
  virtual tBool __stdcall PopAt(iWidget* apContainer, const sVec2f& avAbsPos, tF32 afSnapMargin) = 0;

  //! Compute the Widget's layout.
  //! \param abChildren if true compute all the children's layout
  //!        recursively.
  //! \remark This method shouldn't be called manually in general ; it
  //!         recomputes the layout only if a request has been set, which
  //!         happens when the widget size changes and other various events.
  virtual void __stdcall Layout(tBool abChildren) = 0;
  //! @}

  //########################################################################################
  //! \name Finger Capture
  //########################################################################################
  //! @{

  //! Set the capture of a finger's input.
  //! {Property}
  virtual void __stdcall SetFingerCapture(tU32 anFinger, tBool abEnable) = 0;
  //! Get the capture of a finger's input.
  //! {Property}
  virtual tBool __stdcall GetFingerCapture(tU32 anFinger) const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IWIDGET_6023245_H__
