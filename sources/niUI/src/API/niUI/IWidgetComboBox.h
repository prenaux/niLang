#ifndef __ICOMBOBOX_3636236_H__
#define __ICOMBOBOX_3636236_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Combo box widget style.
enum eWidgetComboBoxStyle {
  //! Drop down combo box, the text cant be edited.
  eWidgetComboBoxStyle_DropDown = niBit(eWidgetStyle_MaxBit+0),
  //! Allow to select multiple items in the combo box.
  eWidgetComboBoxStyle_Multiselect = niBit(eWidgetStyle_MaxBit+1),
  //! Set the combo box's edity box to be read only.
  eWidgetComboBoxStyle_ReadOnly = niBit(eWidgetStyle_MaxBit+2),
  //! No default list box is created, the next child added will be the dropped widget.
  eWidgetComboBoxStyle_NoDefaultListBox = niBit(eWidgetStyle_MaxBit+3),
  //! For defautl dropped list box, click down add selection, ctrl+click set selection. (This is the opposite of the default)
  eWidgetComboBoxStyle_ClickAddSelection = niBit(eWidgetStyle_MaxBit+4),
  //! \internal
  eWidgetComboBoxStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Combo box widget notify messages.
enum eWidgetComboBoxCmd {
  //! The selection changed.
  eWidgetComboBoxCmd_SelectionChanged = 0,
  //! The edit box has been validated.
  eWidgetComboBoxCmd_Validated = 1,
  //! The edit box has been modified.
  eWidgetComboBoxCmd_Modified = 2,
  //! The drop is going to be shown.
  eWidgetComboBoxCmd_DropShown = 3,
  //! The drop is going to be hidden.
  eWidgetComboBoxCmd_DropHidden = 4,
  //! \internal
  eWidgetComboBoxCmd_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Combo box widget interface.
struct iWidgetComboBox : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetComboBox,0x69e43028,0xc01d,0x4cb9,0xb3,0xba,0xe3,0xcd,0x0c,0xd4,0x08,0x9e)
  //! Set the number of lines dropped.
  //! {Property}
  virtual void __stdcall SetNumLines(tU32 anNum) = 0;
  //! Get the number of lines dropped.
  //! {Property}
  virtual tU32 __stdcall GetNumLines() const = 0;
  //! Set the 'dropped' widget.
  //! {Property}
  virtual tBool __stdcall SetDroppedWidget(iWidget* apWidget) = 0;
  //! Get the 'dropped' widget.
  //! {Property}
  virtual iWidget* __stdcall GetDroppedWidget() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __ICOMBOBOX_3636236_H__
