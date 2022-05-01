#ifndef __IEDITBOXWIDGET_88521336_H__
#define __IEDITBOXWIDGET_88521336_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Simple edit box styles.
enum eWidgetEditBoxStyle {
  //! Multi-line edit box.
  eWidgetEditBoxStyle_MultiLine = niBit(eWidgetStyle_MaxBit+1),
  //! Read-only edit box.
  eWidgetEditBoxStyle_ReadOnly = niBit(eWidgetStyle_MaxBit+2),
  //! Auto scroll edit box. Scrolling made in function of the cursor's position.
  eWidgetEditBoxStyle_AutoScroll = niBit(eWidgetStyle_MaxBit+3),
  //! Validate on paste.
  eWidgetEditBoxStyle_PasteValidate = niBit(eWidgetStyle_MaxBit+4),
  //! Dont draw the cursor.
  eWidgetEditBoxStyle_NoCursor = niBit(eWidgetStyle_MaxBit+5),
  //! Dont allow selection.
  //! \remark The clipboard is also disabled in this case.
  eWidgetEditBoxStyle_NoSelect = niBit(eWidgetStyle_MaxBit+6),
  //! Dont clear selection when lost focus.
  eWidgetEditBoxStyle_DontLoseSelection = niBit(eWidgetStyle_MaxBit+7),
  //! Validate when lost focus
  eWidgetEditBoxStyle_ValidateOnLostFocus = niBit(eWidgetStyle_MaxBit+8),
  //! Select all when gaining focus
  eWidgetEditBoxStyle_SelectAllOnSetFocus = niBit(eWidgetStyle_MaxBit+9),
  //! Can insert tabs, but capture the MoveFocus message.
  eWidgetEditBoxStyle_CanInsertTabs = niBit(eWidgetStyle_MaxBit+10),
  //! \internal
  eWidgetEditBoxStyle_ForceDWORD = 0xFFFFFFFF
};

//! Simple edit box notification messages.
enum eWidgetEditBoxCmd {
  //! Sent when the edit box has been validated.
  eWidgetEditBoxCmd_Validated = 0,
  //! Sent when the edit box's text has been modified.
  eWidgetEditBoxCmd_Modified = 1,
  //! Sent when the edit box has lost focus.
  eWidgetEditBoxCmd_LostFocus = 2,
  //! \internal
  eWidgetEditBoxCmd_ForceDWORD = 0xFFFFFFFF
};

//! Simple editbox widget interface.
struct iWidgetEditBox : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetEditBox,0xe1f70622,0x8a53,0x4f98,0xab,0x3c,0xd8,0xe9,0x90,0x36,0xa5,0xab)

  //! Add the specified text at the end of the edit box.
  virtual void __stdcall AddText(const achar* aaszText) = 0;

  //! Set the replacement character for all text displayed.
  //! \remark 0 will disable the feature
  //! {Property}
  virtual void __stdcall SetReplaceChar(tU32 anChar) = 0;
  //! Get the replacement character for all text displayed.
  //! {Property}
  virtual tU32 __stdcall GetReplaceChar() const = 0;

  //! Move the cursor to the begining of the line or the text.
  virtual void __stdcall MoveCursorHome(tBool abLine) = 0;
  //! Move the cursor at the end of the line or the text.
  virtual void __stdcall MoveCursorEnd(tBool abLine) = 0;
  //! Move the cursor to the specified position (line,column)
  virtual void __stdcall MoveCursor(const sVec2i& pos, tBool abForceLogicalCol) = 0;
  //! Move the current cursor line.
  virtual void __stdcall MoveCursorLineDelta(tI32 line) = 0;
  //! Move the current cursor column.
  virtual void __stdcall MoveCursorColumnDelta(tI32 line) = 0;

  //! Get the cursor's position.
  //! {Property}
  //! \remark Vec2(line,column)
  virtual sVec2i __stdcall GetCursorPosition() const = 0;
  //! Get the cursor's current line.
  //! {Property}
  virtual tU32 __stdcall GetCursorLine() const = 0;
  //! Get the cursor's current column.
  //! {Property}
  virtual tU32 __stdcall GetCursorColumn() const = 0;

  //! Scroll to the current cursor position.
  virtual void __stdcall AutoScroll() = 0;

  //! Get the text in the specified range, Vec4(startLine,startCol,endLine,endCol).
  virtual cString __stdcall GetTextInRange(const sVec4i& aRange) const = 0;
  //! Get the text range which includes the whole text.
  //! {Property}
  virtual sVec4i __stdcall GetAllTextRange() const = 0;
  //! Set the selection to the specified range, Vec4(startLine,startCol,endLine,endCol)
  //! {Property}
  virtual void __stdcall SetSelection(const sVec4i& aSelection) = 0;
  //! Get the current selection range, Vec4(startLine,startCol,endLine,endCol).
  //! {Property}
  virtual sVec4i __stdcall GetSelection() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IEDITBOXWIDGET_88521336_H__
