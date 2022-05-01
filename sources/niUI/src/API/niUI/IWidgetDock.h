#ifndef __IWIDGETDOCK_73335480_H__
#define __IWIDGETDOCK_73335480_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

/** \addtogroup niUI
 * @{
 */

//! Docking manager messages
enum eWidgetDockingManagerMessage
{
  //! The widget is beginning to be moved/dragged.
  eWidgetDockingManagerMessage_BeginMove = niMessageID('_','W','D','M','b'),
  //! The widget is finished to be moved/dragged.
  eWidgetDockingManagerMessage_EndMove = niMessageID('_','W','D','M','e'),
  //! The widget is moved.
  //! \param A is the absolute cursor position.
  eWidgetDockingManagerMessage_Move = niMessageID('_','W','D','N','m'),
  //! \internal
  eWidgetDockingManagerMessage_ForceDWORD = 0xFFFFFFFF
};

//! Dockable widget interface.
//! \remark This is the interface that widgets needs to implement to be dockable.
//! \remark Dockable widgets needs to call their parent widget's docking manager
//!     to handle their docking.
struct iWidgetDockable : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetDockable,0x74ad7011,0xe5fb,0x47d8,0x8a,0xad,0x4b,0x2c,0x65,0xc5,0x0d,0x35)

  //! Get the dock's name.
  //! {Property}
  //! \remark The dock name will be used as tab page name when docked.
  virtual const achar* __stdcall GetDockName() const = 0;
  //! Get the docking status.
  //! {Property}
  virtual tBool __stdcall GetIsDocked() const = 0;
};

//! Docking manager flags.
enum eWidgetDockingManagerFlags
{
  //! Show a navigator if the Navigator action is triggered.
  eWidgetDockingManagerFlags_Navigator = niBit(0),
  //! Hide the tab name if only one page is in the tab widget.
  //! \remark This can be set per tab page if necessary, this is the default value.
  eWidgetDockingManagerFlags_HideTabIfOnePage = niBit(1),
  //! Docking on the left is allowed.
  eWidgetDockingManagerFlags_DockLeft = niBit(2),
  //! Docking on the right is allowed.
  eWidgetDockingManagerFlags_DockRight = niBit(3),
  //! Docking on the top is allowed.
  eWidgetDockingManagerFlags_DockTop = niBit(4),
  //! Docking on the bottom is allowed.
  eWidgetDockingManagerFlags_DockBottom = niBit(5),
  //! Horizontal docking.
  eWidgetDockingManagerFlags_DockHorizontal = eWidgetDockingManagerFlags_DockLeft|eWidgetDockingManagerFlags_DockRight,
  //! Vertical docking.
  eWidgetDockingManagerFlags_DockVertical = eWidgetDockingManagerFlags_DockTop|eWidgetDockingManagerFlags_DockBottom,
  //! All docking
  eWidgetDockingManagerFlags_DockAll = eWidgetDockingManagerFlags_DockHorizontal|eWidgetDockingManagerFlags_DockVertical,
  //! Default manager flags.
  eWidgetDockingManagerFlags_Default = eWidgetDockingManagerFlags_DockAll|eWidgetDockingManagerFlags_Navigator,
  //! \internal
  eWidgetDockingManagerFlags_ForceDWORD = 0xFFFFFFFF
};

//! Docking manager flags type.
typedef tU32 tWidgetDockingManagerFlags;

//! Widget docking manager.
//! \remark The docking manager is a normal widget that implements iWidgetSink.
struct iWidgetDockingManager : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetDockingManager,0x3ecf2b94,0x14cf,0x4e24,0xb5,0xa9,0xc2,0xf0,0xbf,0xb8,0xf5,0x01)

  //########################################################################################
  //! \name Main
  //########################################################################################
  //! @{

  //! Set the docking manager's flags.
  //! {Property}
  virtual tBool __stdcall SetFlags(tWidgetDockingManagerFlags aFlags) = 0;
  //! Get the docking manager's flags.
  //! {Property}
  virtual tWidgetDockingManagerFlags __stdcall GetFlags() const = 0;
  //! @}

  //########################################################################################
  //! \name Management
  //########################################################################################
  //! @{

  //! Get the first dock area that contains a tab page with the specified name.
  //! {Property}
  virtual tU32 __stdcall GetDockAreaFromPageName(const achar* aaszName) const = 0;
  //! Get the dock area that is hovered by the specified absolute cursor position.
  //! {Property}
  virtual tU32 __stdcall GetDockAreaHovered(sVec2f avPos) const = 0;
  //! Dock the specified widget inside the specified dock area.
  //! \remark If anDock == eInvalidHandle the widget will be docked inside the dock area
  //!     created by eWidgetDockingManagerMessage_CreateHoveredDockArea, if none has
  //!     been created the method will return eFalse.
  virtual tBool __stdcall DockWidget(tU32 anDock, iWidget* apWidget) = 0;
  //! Undock the specified widget.
  virtual tBool __stdcall UndockWidget(iWidget* apWidget) = 0;
  //! Get the number of widgets docked.
  //! {Property}
  virtual tU32 __stdcall GetNumDockedWidgets() const = 0;
  //! Get the docked widget at the specified index.
  //! {Property}
  virtual iWidget* __stdcall GetDockedWidget(tU32 anIndex) const = 0;
  //! Get the number of dock area.
  //! {Property}
  virtual tU32 __stdcall GetNumDockAreas() const = 0;
  //! Get the dock area at the specified index.
  //! {Property}
  virtual iWidget* __stdcall GetDockArea(tU32 anIndex) const = 0;
  //! Set the context menu of the dock areas tabs.
  //! {Property}
  virtual tBool __stdcall SetDockAreaTabContextMenu(iWidget* apMenu) = 0;
  //! Get the context menu of the dock areas tabs.
  //! {Property}
  virtual iWidget* __stdcall GetDockAreaTabContextMenu() const = 0;
  //! Add a new empty dock area.
  //! \return The index of the new dock area.
  virtual tU32 __stdcall AddDockArea(tU32 anParent, tU32 aPos, sRectf aRect, tBool abLocal) = 0;
  //! Remove all dock areas.
  virtual void __stdcall ClearDockAreas() = 0;
  //! Clean all invalid/empty dock areas.
  virtual void __stdcall CleanDockAreas() = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETDOCK_73335480_H__
