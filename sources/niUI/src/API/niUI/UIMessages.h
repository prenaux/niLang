#ifndef __UIMESSAGES_73335480_H__
#define __UIMESSAGES_73335480_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/MessageID.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! UI message
//! \remark G, general messages
//! \remark L, layout messages
//! \remark P, painting/drawing messages
//! \remark I, input messages
//! \remark F, finger messages
//! \remark A, action messages
//! \remark C, click messages
//! \remark S, serialization messages
//! \remark Z, user messages
//! \remark X, context messages, sent only to the root widget
enum eUIMessage
{
  //! \name General
  //! @{

  //! Message sent to the sink being added.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_SinkAttached = niMessageID('_','U','I','G',0),
  //! Message sent to the sink being removed.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_SinkDetached = niMessageID('_','U','I','G',1),
  //! A child widget has been added.
  //! \param A: the child added
  //! \param B: unused
  eUIMessage_ChildAdded = niMessageID('_','U','I','G',2),
  //! A child widget has been removed.
  //! \param A: the child removed
  //! \param B: unused
  eUIMessage_ChildRemoved = niMessageID('_','U','I','G',3),
  //! The widget has been destroyed.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Destroy = niMessageID('_','U','I','G',4),
  //! Notification of message received by a child widget.
  //! \param A: the widget that received the notified message.
  //! \param B: iMessage instance that contains the message.
  eUIMessage_Notify = niMessageID('_','U','I','G',5),
  //! Received when the widget should set the cursor.
  //! \remark If no cursor is set the default cursor will be set.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_SetCursor = niMessageID('_','U','I','G',6),
  //! Received when the widget gets focus.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_SetFocus = niMessageID('_','U','I','G',7),
  //! \param A: unused
  //! \param B: unused
  eUIMessage_LostFocus = niMessageID('_','U','I','G',8),
  //! The widget has been activated.
  //! \param A: unused
  //! \param B: unused
  //! \remark A widget can be activated manually using iUIContext::Activate, or before receiving
  //!     focus if the style eWidgetStyle_FocusActivate is set.
  //! \remark When a widget is activated it is not put on Top of the ZOrder, if it's the desired
  //!     behavior then the widget should set itself on the top when this message is received.
  eUIMessage_Activate = niMessageID('_','U','I','G',9),
  //! The widget has been deactivated.
  //! \remark A widget is deactivated when another widget is activated.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Deactivate = niMessageID('_','U','I','G',10),
  //! A child widget has send a command.
  //! \param A: a iWidgetCommand instance.
  //! \param B: unused.
  //! \remark The command message is sent to the parent widget if it is not handled.
  //! \remark The standard Form widget always handle this message.
  eUIMessage_Command = niMessageID('_','U','I','G',11),
  //! Message sent when a timer expires.
  //! \param A: timer id
  //! \param B: timer duration
  eUIMessage_Timer = niMessageID('_','U','I','G',12),
  //! Enabled message, sent when a widget is enabled or disabled.
  //! \param A: eTrue if the widget is enabled, eFalse if the widget is disabled.
  //! \param B: unused
  eUIMessage_Enabled = niMessageID('_','U','I','G',13),
  //! Visible message, sent when a widget is made visible or invisible.
  //! \param A: eTrue if the widget is made visible, eFalse if the widget is made invisible.
  //! \param B: unused
  eUIMessage_Visible = niMessageID('_','U','I','G',14),
  //! The widget should update its layout.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Layout = niMessageID('_','U','I','G',15),
  //! The widget's style has been changed.
  //! \param A: previous style
  //! \param B: unused
  eUIMessage_StyleChanged = niMessageID('_','U','I','G',16),
  //! The widget's font has been changed.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_FontChanged = niMessageID('_','U','I','G',17),
  //! Text changed.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_TextChanged = niMessageID('_','U','I','G',18),
  //! Called when the text should be set, when iWidget::GetText is called.
  //! \param A: true if serializing the text property, else false
  //! \param B: unused
  eUIMessage_SetText = niMessageID('_','U','I','G',19),
  //! Called when the skin is changed.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_SkinChanged = niMessageID('_','U','I','G',20),
  //! The context menu has changed.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_ContextMenuChanged = niMessageID('_','U','I','G',21),
  //! Hover text changed.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_HoverTextChanged = niMessageID('_','U','I','G',22),
  //! Configuration files should be loaded.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_LoadConfig = niMessageID('_','U','I','G',23),
  //! Configuration files should be saved.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_SaveConfig = niMessageID('_','U','I','G',24),
  //! The context (root widget) has been resized.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_ContextResized = niMessageID('_','U','I','G',25),
  //! Locale has changed.
  //! \param A: unused
  //! \param B: unused
  //! \remark This is sent when the active locale's translations has changed or when the current locale is changed.
  eUIMessage_LocaleChanged = niMessageID('_','U','I','G',26),
  //! @}

  //! \name Layout
  //! @{

  //! Non-client area resized.
  //! \param A: new widget size
  //! \param B: unused
  eUIMessage_NCSize = niMessageID('_','U','I','L',0),
  //! Client area resized.
  //! \param A: new client size
  //! \param B: unused
  eUIMessage_Size = niMessageID('_','U','I','L',1),
  //! The widget's padding has been modified.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Padding = niMessageID('_','U','I','L',2),
  //! The widget's border has been modified.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Border = niMessageID('_','U','I','L',3),
  //! The widget's border style has been modified.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_BorderStyle = niMessageID('_','U','I','L',4),
  //! The widget's margin has been modified.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Margin = niMessageID('_','U','I','L',5),
  //! The widget's margin merge has been modified.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_MarginMerge = niMessageID('_','U','I','L',6),
  //! The widget has been folded.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Folded = niMessageID('_','U','I','L',7),
  //! The widget has been unfolded.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Unfolded = niMessageID('_','U','I','L',8),
  //! The widget has been docked or undocked.
  //! \param A: the widget the form is going to be docked in.
  //! \param B: unused.
  eUIMessage_BeforeDocked = niMessageID('_','U','I','L',9),
  //! The widget has been docked or undocked.
  //! \param A: the tab widget inside which the widget has just been docked.
  //! \param B: unused
  eUIMessage_AfterDocked = niMessageID('_','U','I','L',10),
  //! The widget has been docked or undocked.
  //! \param A: unused.
  //! \param B: unused.
  eUIMessage_Undocked = niMessageID('_','U','I','L',11),
  //! @}

  //! \name Drawing
  //! @{

  //! Draw the non-client area.
  //! \param A: cursor position, non-client relative.
  //! \param B: the iCanvas to draw into
  eUIMessage_NCPaint = niMessageID('_','U','I','P',1),
  //! Draw the client area.
  //! \param A: cursor position, client relative.
  //! \param B: the iCanvas to draw into
  eUIMessage_Paint = niMessageID('_','U','I','P',2),
  //! @}

  //! \name Input
  //! @{

  //! Left click down in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCLeftClickDown = niMessageID('_','U','I','I',0),
  //! Left click down in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_LeftClickDown = niMessageID('_','U','I','I',1),
  //! Left click up in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCLeftClickUp = niMessageID('_','U','I','I',2),
  //! Left click up in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_LeftClickUp = niMessageID('_','U','I','I',3),
  //! Right click down in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCRightClickDown = niMessageID('_','U','I','I',4),
  //! Right click down in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_RightClickDown = niMessageID('_','U','I','I',5),
  //! Right click up in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCRightClickUp = niMessageID('_','U','I','I',6),
  //! Right click up in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_RightClickUp = niMessageID('_','U','I','I',7),
  //! The cursor moved inside the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCMouseMove = niMessageID('_','U','I','I',8),
  //! The cursor moved inside the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_MouseMove = niMessageID('_','U','I','I',9),
  //! The wheel button has been pressed in the non client area.
  //! \param A: wheel movement value
  //! \param B: absolute cursor position.
  eUIMessage_NCWheel = niMessageID('_','U','I','I',10),
  //! The wheel button has been pressed in the client area.
  //! \param A: wheel movement value
  //! \param B: absolute cursor position.
  eUIMessage_Wheel = niMessageID('_','U','I','I',11),
  //! A character has been entered.
  //! \param A: 'achar' code
  //! \param B: unused
  eUIMessage_KeyChar = niMessageID('_','U','I','I',12),
  //! Submit message.
  //! \param A: iWidget, sender widget
  //! \param B: iMessage, input message that triggered the submit
  //! \remark The submit message is sent to the parent widget if it is not handled.
  //! \remark The standard Form widget always handle this message.
  eUIMessage_Submit = niMessageID('_','U','I','I',13),

  //! Sent when the mouse enter the widget non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCMouseEnter = niMessageID('_','U','I','I',15),
  //! Sent when the mouse enter the widget client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_MouseEnter = niMessageID('_','U','I','I',16),
  //! Sent when the mouse leaves the widget non-client area.
  //! \param A: unused.
  //! \param B: unused.
  eUIMessage_NCMouseLeave = niMessageID('_','U','I','I',17),
  //! Sent when the mouse leaves the widget client area.
  //! \param A: unused.
  //! \param B: unused.
  eUIMessage_MouseLeave = niMessageID('_','U','I','I',18),
  //! Sent when the mouse stand over the client area without moving for the time specified in the widget's hover delay.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCMouseHover = niMessageID('_','U','I','I',19),
  //! Sent when the mouse stand over the non-client area without moving for the time specified in the widget's hover delay.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_MouseHover = niMessageID('_','U','I','I',20),
  //! The cursor moved inside the non-client area when dragging an item.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCDragMouseMove = niMessageID('_','U','I','I',21),
  //! The cursor moved inside the client area when dragging an item.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_DragMouseMove = niMessageID('_','U','I','I',22),
  //! Sent when the mouse enter the widget non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCDragMouseEnter = niMessageID('_','U','I','I',23),
  //! Sent when the mouse enter the widget client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_DragMouseEnter = niMessageID('_','U','I','I',24),
  //! Sent when the mouse leaves the widget non-client area.
  //! \param A: unused.
  //! \param B: unused.
  eUIMessage_NCDragMouseLeave = niMessageID('_','U','I','I',25),
  //! Sent when the mouse leaves the widget client area.
  //! \param A: unused.
  //! \param B: unused.
  eUIMessage_DragMouseLeave = niMessageID('_','U','I','I',26),

  //! Sent when the mouse movement is considered to begin dragging in the non-client area. That is the left mouse button has been pressed and a movement of at least 'drag threshold' pixel has been made.
  //! \remark The message is sent to all widgets, including the ones that aren't drag sources or targets.
  //! \param A: Drag start position in screen space.
  //! \param B: unused.
  eUIMessage_NCDragBegin = niMessageID('_','U','I','I',27),
  //! Sent when the mouse movement is considered to begin dragging in the client area. That is the left mouse button has been pressed and a movement of at least 'drag threshold' pixel has been made.
  //! \remark The message is sent to all widgets, including the ones that aren't drag sources or targets.
  //! \param A: Drag start position in screen space.
  //! \param B: unused.
  eUIMessage_DragBegin = niMessageID('_','U','I','I',28),
  //! Sent when the drag button has been released, aka when the dragging ended in the non-client area.
  //! \remark The message is sent to all widgets, including the ones that aren't drag sources or targets.
  //! \param A: Drag end position in screen space.
  //! \param B: unused.
  eUIMessage_NCDragEnd = niMessageID('_','U','I','I',29),
  //! Sent when the drag button has been released, aka when the dragging ended.
  //! \remark The message is sent to all widgets, including the ones that aren't drag sources or targets.
  //! \param A: Drag end position in screen space.
  //! \param B: unused.
  eUIMessage_DragEnd = niMessageID('_','U','I','I',30),

  //! A key has been pressed.
  //! \param A: eKey code
  //! \param B: eKeyMod
  eUIMessage_KeyDown = niMessageID('_','U','I','I',32),
  //! A key has been release.
  //! \param A: 'achar' code
  //! \param B: eKeyMod
  eUIMessage_KeyUp = niMessageID('_','U','I','I',33),

  //! A game controller's state changed.
  //! \param A: the game controller
  //! \param B: unused
  eUIMessage_GameCtrl = niMessageID('_','U','I','I',34),
  //! A game controller's button is down.
  //! \param A: the game controller
  //! \param B: the game controller button pressed
  eUIMessage_GameCtrlButtonDown = niMessageID('_','U','I','I',35),
  //! A game controller's button is up.
  //! \param A: the game controller
  //! \param B: the game controller button released
  eUIMessage_GameCtrlButtonUp = niMessageID('_','U','I','I',36),
  //! @}

  //! \name Finger Input (Multi Touch)
  //! @{

  //! Finger down in the non-client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_NCFingerDown = niMessageID('_','U','I','F',0),
  //! Finger click down in the client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_FingerDown = niMessageID('_','U','I','F',1),
  //! Finger up in the non-client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_NCFingerUp = niMessageID('_','U','I','F',2),
  //! Finger click up in the client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_FingerUp = niMessageID('_','U','I','F',3),

  //! The finger moved inside the non-client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_NCFingerMove = niMessageID('_','U','I','F',8),
  //! The finger moved inside the client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_FingerMove = niMessageID('_','U','I','F',9),

  //! Sent when a finger enter the widget non-client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_NCFingerEnter = niMessageID('_','U','I','F',15),
  //! Sent when a finger enter the widget client area.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_FingerEnter = niMessageID('_','U','I','F',16),
  //! Sent when a finger leaves the widget non-client area.
  //! \param A: unused.
  //! \param B: unused.
  eUIMessage_NCFingerLeave = niMessageID('_','U','I','F',17),
  //! Sent when a finger leaves the widget client area.
  //! \param A: unused.
  //! \param B: unused.
  eUIMessage_FingerLeave = niMessageID('_','U','I','F',18),

  //! The cursor moved inside the non-client area when dragging an item.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_NCDragFingerMove = niMessageID('_','U','I','F',21),
  //! The cursor moved inside the client area when dragging an item.
  //! \param A: finger index.
  //! \param B: finger position, client relative, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_DragFingerMove = niMessageID('_','U','I','F',22),

  //! Sent when a finger enter the widget non-client area.
  //! \param A: finger index.
  //! \param B: finger position, absolute, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_NCDragFingerEnter = niMessageID('_','U','I','F',23),
  //! Sent when a finger enter the widget client area.
  //! \param A: finger index.
  //! \param B: finger position, absolute, and pressure. Vec3(client x, client y, pressure).
  eUIMessage_DragFingerEnter = niMessageID('_','U','I','F',24),
  //! Sent when a finger leaves the widget non-client area.
  //! \param A: finger index.
  //! \param B: unused.
  eUIMessage_NCDragFingerLeave = niMessageID('_','U','I','F',25),
  //! Sent when a finger leaves the widget client area.
  //! \param A: finger index.
  //! \param B: unused.
  eUIMessage_DragFingerLeave = niMessageID('_','U','I','F',26),
  //! Sent when a finger movement is considered to begin dragging in
  //! the non-client area. That is the left finger button has been
  //! pressed and a movement of at least 'drag threshold' pixel has
  //! been made.
  //! \remark The message is sent to all widgets, including the ones
  //!         that aren't drag sources or targets.
  //! \param A: finger index.
  //! \param B: Drag start position in screen space.
  eUIMessage_NCDragFingerBegin = niMessageID('_','U','I','F',27),
  //! Sent when a finger movement is considered to begin dragging in
  //! the client area. That is the left finger button has been
  //! pressed and a movement of at least 'drag threshold' pixel has
  //! been made.
  //! \remark The message is sent to all widgets, including the ones
  //!         that aren't drag sources or targets.
  //! \param A: finger index.
  //! \param B: Drag start position in screen space.
  eUIMessage_DragFingerBegin = niMessageID('_','U','I','F',28),
  //! Sent when the drag button has been released, aka when the
  //! dragging ended in the non-client area.
  //! \remark The message is sent to all widgets, including the ones
  //!         that aren't drag sources or targets.  \param A: unused.
  //! \param A: finger index.
  //! \param B: Drag end position in screen space.
  eUIMessage_NCDragFingerEnd = niMessageID('_','U','I','F',29),
  //! Sent when the drag button has been released, aka when the
  //! dragging ended.
  //! \remark The message is sent to all widgets, including the ones
  //!         that aren't drag sources or targets.
  //! \param A: finger index.
  //! \param B: Drag end position in screen space.
  eUIMessage_DragFingerEnd = niMessageID('_','U','I','F',30),

  //! The finger moved inside the non-client area.
  //! \param A: finger index.
  //! \param B: finger relative position and pressure. Vec3(rel x, rel y, pressure).
  eUIMessage_NCFingerRelativeMove = niMessageID('_','U','I','F',31),
  //! The finger moved inside the client area.
  //! \param A: finger index.
  //! \param B: finger relative position and pressure. Vec3(rel x, rel y, pressure).
  eUIMessage_FingerRelativeMove = niMessageID('_','U','I','F',32),

  //! Pinch gesture.
  //! \param A: scale.
  //! \param B: eGestureState.
  eUIMessage_Pinch = niMessageID('_','U','I','F',33),
  //! @}

  //! \name Standard Input Actions
  //! @{

  //! Sent when the context menu should be opened.
  //! \param A: true if a mouse click generated the message, else false, meaning that
  //!     the widget should generate the context menu position from the current
  //!     selection or it's top left position.
  //! \param B: cursor position, client relative.
  //! \remark This message is sent from the default RightClickUp message handler.
  eUIMessage_ContextMenu = niMessageID('_','U','I','A',0),
  //! Context help.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_ContextHelp = niMessageID('_','U','I','A',1),
  //! Copy.
  //! \param A: clipboard datatable. \see ni::iDataTable
  //! \param B: unused
  eUIMessage_Copy = niMessageID('_','U','I','A',2),
  //! Cut.
  //! \param A: clipboard datatable. \see ni::iDataTable
  //! \param B: unused
  eUIMessage_Cut = niMessageID('_','U','I','A',3),
  //! Paste.
  //! \param A: clipboard datatable. \see ni::iDataTable
  //! \param A: unused
  eUIMessage_Paste = niMessageID('_','U','I','A',4),
  //! Undo.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Undo = niMessageID('_','U','I','A',5),
  //! Redo.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Redo = niMessageID('_','U','I','A',6),
  //! Move focus.
  //! \param A: true if the focus should be moved to the previous widget, false if it should be moved to the next widget
  //! \param B: unused
  eUIMessage_MoveFocus = niMessageID('_','U','I','A',7),
  //! Cancel message.
  //! \param A: unused
  //! \param B: unused
  eUIMessage_Cancel = niMessageID('_','U','I','A',8),
  //! @}

  //! \name Click messages
  //! @{
  //! Click and NCClick click messages are generated only if a up and down
  //! clicks have been performed without moving the cursor outside of the current widget.
  //! The input wont be captured when the mouse is down.
  //! These messages can be disabled by setting the eWidgetStyle_NoClick style.

  //! Left click in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCLeftClick = niMessageID('_','U','I','C',0),
  //! Left click in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_LeftClick = niMessageID('_','U','I','C',1),
  //! Left double-click in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCLeftDoubleClick = niMessageID('_','U','I','C',2),
  //! Left double-click in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_LeftDoubleClick = niMessageID('_','U','I','C',3),
  //! Right click in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCRightClick = niMessageID('_','U','I','C',4),
  //! Right click in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_RightClick = niMessageID('_','U','I','C',5),
  //! Right double-click in the non-client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_NCRightDoubleClick = niMessageID('_','U','I','C',6),
  //! Right double-click in the client area.
  //! \param A: cursor position, client relative.
  //! \param B: cursor position, non-client relative.
  eUIMessage_RightDoubleClick = niMessageID('_','U','I','C',7),
  //! @}

  //! \name Serialization
  //! @{

  //! SerializeLayout message.
  //! \param A: iDataTable interface where the widget should be serialized.
  //! \param B: the widget serialize flags \see eWidgetSerializeFlags.
  //! \see ni::iUIContext::SerializeWidget
  //! \see ni::iWidget::SerializeLayout
  //! \remark Not called if the NoRoot serialize flag is set.
  eUIMessage_SerializeLayout = niMessageID('_','U','I','S',0),
  //! SerializeWidget message.
  //! \param A: iDataTable interface where the widget should be serialized.
  //! \param B: the widget serialize flags \see eWidgetSerializeFlags.
  //! \see ni::iUIContext::SerializeWidget
  //! \remark This is always called, no matter what the serialize flags are.
  eUIMessage_SerializeWidget = niMessageID('_','U','I','S',1),
  //! SerializeChildren message.
  //! \param A: iDataTable interface where the widget should be serialized.
  //! \param B: the widget serialize flags \see eWidgetSerializeFlags.
  //! \see ni::iUIContext::SerializeWidget
  //! \see ni::iWidget::SerializeChildren
  //! \remark Not called if the Children serialize flag is not set.
  eUIMessage_SerializeChildren = niMessageID('_','U','I','S',2),
  //! SerializeFinalize message.
  //! \param A: iDataTable interface where the widget should be serialized.
  //! \param B: the widget serialize flags \see eWidgetSerializeFlags.
  //! \see  ni::iUIContext::SerializeWidget
  //! \see  ni::iWidget::SerializeFinalize
  //! \remark Called at the end of the serialization.
  eUIMessage_SerializeFinalize = niMessageID('_','U','I','S',3),
  //! @}

  //! \name User
  //! @{

  //! First user message id
  eUIMessage_UserMessage = niMessageID('_','U','I','Z',0),
  //! @}

  //! \name Context
  //! @{
  //! Context messages are sent only to the root widget.

  //! Sent after the context is updated.
  //! \param A: tF32 FrameTime.
  //! \param B: unused.
  eUIMessage_ContextUpdate = niMessageID('_','U','I','X','u'),
  //! Sent before the context is drawn.
  //! \param A: the Canvas of the root widget (rootCanvas)
  //! \param B: unused.
  eUIMessage_ContextBeforeDraw = niMessageID('_','U','I','X','d'),
  //! Sent after the context is drawn.
  //! \param A: the Canvas of the root widget (rootCanvas)
  //! \param B: unused.
  eUIMessage_ContextAfterDraw = niMessageID('_','U','I','X','D'),
  //! @}

  //! \internal
  eUIMessage_ForceDWORD = 0xFFFFFFFF
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __UIMESSAGES_73335480_H__
