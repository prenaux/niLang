#ifndef __IWIDGETSPLITTER_14554643_H__
#define __IWIDGETSPLITTER_14554643_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Splitter widget style.
enum eWidgetSplitterStyle
{
  //! The splitter is horizontal.
  eWidgetSplitterStyle_Horizontal = niBit(eWidgetStyle_MaxBit+0),
  //! The splitter cant be resized with the mouse cursor.
  eWidgetSplitterStyle_NoCursorResize = niBit(eWidgetStyle_MaxBit+1),
  //! The splitter will start empty.
  //! \remark By default the splitter is created with one splitter.
  eWidgetSplitterStyle_Empty = niBit(eWidgetStyle_MaxBit+2),
  //! \internal
  eWidgetSplitterStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Splitter folding mode.
enum eWidgetSplitterFoldMode
{
  //! No folding.
  eWidgetSplitterFoldMode_None = 0,
  //! Fold left/top or right/bottom based on the resizable borders enabled. (default)
  eWidgetSplitterFoldMode_Auto = 1,
  //! Fold left/top & right/bottom regardless of the resizable borders.
  eWidgetSplitterFoldMode_All = 2,
  //! \internal
  eWidgetSplitterFoldMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Splitter widget interface.
struct iWidgetSplitter : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetSplitter,0x78bc04f6,0x9b0c,0x446c,0x87,0xa2,0x46,0xbc,0x8b,0xb6,0xfb,0xa1)
  //! Add a splitter in the front (top or left).
  virtual tBool __stdcall AddSplitterFront(tF32 afPos) = 0;
  //! Add a splitter in the back (bottom or right).
  virtual tBool __stdcall AddSplitterBack(tF32 afPos) = 0;
  //! Remove the splitter at the specified index.
  //! \remark The widget at the bottom (right if horizontal) will be removed.
  virtual tBool __stdcall RemoveSplitter(tU32 anIndex) = 0;
  //! Get the number of splitters.
  //! {Property}
  virtual tU32 __stdcall GetNumSplitters() const = 0;
  //! Get the number of splitter's widgets.
  //! {Property}
  virtual tU32 __stdcall GetNumSplitterWidgets() const = 0;
  //! Get a splitter's widget.
  //! {Property}
  //! \remark Widgets are numbered from top to bottom, and left to right.
  virtual iWidget* __stdcall GetSplitterWidget(tU32 anIndex) const = 0;
  //! Get a splitter's widget index.
  //! {Property}
  virtual tU32 __stdcall GetSplitterWidgetIndex(iWidget* apWidget) const = 0;
  //! Remove the specified splitter widget.
  //! \remark At least one widget will always be available, so the last widget cant be removed.
  //! \remark This will take care of removing the appropriate splitter.
  virtual tBool __stdcall RemoveSplitterWidget(tU32 anIndex) = 0;
  //! Add a splitter before the specified widget (top or left).
  //! \remark afPos is relative to the widget used for insertion.
  virtual tBool __stdcall AddSplitterBefore(tU32 anWidget, tF32 afPos) = 0;
  //! Add a splitter after the specified widget (bottom or right).
  //! \remark afPos is relative to the widget used for insertion.
  virtual tBool __stdcall AddSplitterAfter(tU32 anWidget, tF32 afPos) = 0;
  //! Set the position of the splitter at the given index.
  //! {Property}
  //! \remark The position is in percent, relative to the size of the widget.
  virtual tBool __stdcall SetSplitterPosition(tU32 anIndex, tF32 aVal) = 0;
  //! Get the position of the splitter at the given index
  //! {Property}
  //! \remark The position is in percent, relative to the size of the widget.
  virtual tF32 __stdcall GetSplitterPosition(tU32 anIndex) const = 0;
  //! Set the splitter's minimum position.
  //! {Property}
  //! \remark The minimum position is the minimum distance from the parent's borders in pixels.
  //! \remark If the minimum position is < 0 no limit is applied.
  virtual tBool __stdcall SetSplitterMinPosition(tF32 aVal) = 0;
  //! Get the splitter's minimum position.
  //! {Property}
  //! \remark The minimum position is the minimum distance from the parent's borders in pixels.
  virtual tF32 __stdcall GetSplitterMinPosition() const = 0;
  //! Set the splitter size.
  //! {Property}
  virtual tBool __stdcall SetSplitterSize(tF32 afSize) = 0;
  //! Get the splitter size.
  //! {Property}
  virtual tF32 __stdcall GetSplitterSize() const = 0;
  //! Swap two splitter widgets.
  virtual tBool __stdcall SwapSplitterWidget(tU32 anA, tU32 anB) = 0;
  //! Set the splitter's border size.
  //! {Property}
  virtual void __stdcall SetSplitterBorderSize(tF32 afSize) = 0;
  //! Get the splitter's border size.
  //! {Property}
  virtual tF32 __stdcall GetSplitterBorderSize() const = 0;
  //! Set the splitter's resizable borders.
  //! \param aEdges: edges flags. \see eRectEdges
  //! {Property}
  virtual void __stdcall SetSplitterResizableBorders(tU32 aEdges) = 0;
  //! Get the splitter's resizable borders.
  //! {Property}
  virtual tU32 __stdcall GetSplitterResizableBorders() const = 0;
  //! Set the minimum size that the parent dock rect can have.
  //! \remark This value is used only if a border is resizable, if the min size >= 0 and
  //!     if the splitter is docked.
  //! \remark By default the value is (20,20)
  //! {Property}
  virtual void __stdcall SetSplitterParentDockRectMinimumSize(sVec2f avMinSize) = 0;
  //! Get the minimum size that the parent dock rect can have.
  //! {Property}
  virtual sVec2f __stdcall GetSplitterParentDockRectMinimumSize() const = 0;
  //! Set the index of the filling splitter.
  //! \remark The filling splitter correspond to the index of the canvas that will fill the whole splitter's rectangle, so that it'll be the only widget visible.
  //!     This is meant to be used as a special mode to allow 'fullscreen' widgets with toggle to a splitted/composite view.
  //! {Property}
  virtual void __stdcall SetSplitterFillerIndex(ni::tU32 anIndex) = 0;
  //! Set the index of the filling splitter.
  //! {Property}
  virtual tU32 __stdcall GetSplitterFillerIndex() const = 0;
  //! Set the splitter's fold mode.
  //! {Property}
  virtual void __stdcall SetSplitterFoldMode(eWidgetSplitterFoldMode aFoldMode) = 0;
  //! Get the splitter's fold mode.
  //! {Property}
  virtual eWidgetSplitterFoldMode __stdcall GetSplitterFoldMode() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETSPLITTER_14554643_H__
