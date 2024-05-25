#ifndef __IWIDGETLISTBOX_47171268_H__
#define __IWIDGETLISTBOX_47171268_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Listbox notification messages
enum eWidgetListBoxCmd
{
  //! Sent when the selection has changed
  eWidgetListBoxCmd_SelectionChanged = 0,
  //! \internal
  eWidgetListBoxCmd_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Listbox style
enum eWidgetListBoxStyle
{
  //! Has header.
  eWidgetListBoxStyle_HasHeader = niBit(eWidgetStyle_MaxBit+0),
  //! Clicking the Header will sort the items Ascendant/Descendant.
  eWidgetListBoxStyle_HeaderSort = niBit(eWidgetStyle_MaxBit+1),
  //! Allow multiple selection.
  eWidgetListBoxStyle_Multiselect = niBit(eWidgetStyle_MaxBit+2),
  //! Select when mouse moves.
  eWidgetListBoxStyle_SelectOnMove = niBit(eWidgetStyle_MaxBit+3),
  //! Click down add selection, Ctrl+Click set selection. (This is the opposite of the default)
  eWidgetListBoxStyle_ClickAddSelection = niBit(eWidgetStyle_MaxBit+4),
  //! \internal
  eWidgetListBoxStyle_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Listbox widget interface.
struct iWidgetListBox : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetListBox,0x72906a6e,0xe5d6,0x4a1c,0xb0,0xd9,0xac,0x68,0x79,0x86,0xbf,0xf6)

  //! Add a column with the specified name and size.
  virtual void __stdcall AddColumn(const achar* aaszName, tU32 anSize) = 0;
  //! Remove the specified columns.
  //! \remark The listbox must have at least one column, so deleting the last column will always fail.
  virtual tBool __stdcall RemoveColumn(tU32 anColumn) = 0;
  //! Get the number of columns in the list box.
  //! {Property}
  virtual tU32 __stdcall GetNumColumns() const = 0;
  //! Set the name/header of the column.
  //! {Property}
  virtual tBool __stdcall SetColumnName(tU32 anColumn, const achar* aaszName) = 0;
  //! Get the name/header of the column.
  //! {Property}
  virtual const achar* __stdcall GetColumnName(tU32 anColumn) const = 0;
  //! Set the width of the specified column.
  //! {Property}
  virtual tBool __stdcall SetColumnWidth(tU32 anColumn, tU32 anWidth) = 0;
  //! Get the width of the specified column.
  //! {Property}
  virtual tU32 __stdcall GetColumnWidth(tU32 anColumn) const = 0;
  //! Set a column name and width.
  virtual tBool __stdcall SetColumn(tU32 anColumn, const achar* aaszName, tU32 anSize) = 0;

  //! Get the number of items in the listbox.
  //! {Property}
  virtual tU32 __stdcall GetNumItems() const = 0;
  //! Remove all items in the list box.
  virtual void __stdcall ClearItems() = 0;
  //! Add an item.
  virtual tU32 __stdcall AddItem(const achar* aaszText) = 0;
  //! Remove an item.
  virtual tBool __stdcall RemoveItem(tU32 anItem) = 0;
  //! Set the text of an item element.
  virtual tBool __stdcall SetItemText(tU32 anColumn, tU32 anItem, const achar* aaszText) = 0;
  //! Get the text of an item element.
  virtual const achar* __stdcall GetItemText(tU32 anColumn, tU32 anItem) const = 0;
  //! Set the widget of an item element.
  virtual tBool __stdcall SetItemWidget(tU32 anColumn, tU32 anItem, iWidget* apWidget) = 0;
  //! Get the widget of an item element.
  virtual iWidget* __stdcall GetItemWidget(tU32 anColumn, tU32 anItem) const = 0;
  //! Set the icon of an item.
  //! {Property}
  virtual tBool __stdcall SetItemIcon(tU32 anItem, iOverlay* apIcon) = 0;
  //! Get the icon of an item.
  //! {Property}
  virtual iOverlay* __stdcall GetItemIcon(tU32 anItem) const = 0;
  //! Set the data of an item.
  //! {Property}
  virtual tBool __stdcall SetItemData(tU32 anItem, iUnknown* apData) = 0;
  //! Get the data of an item.
  //! {Property}
  virtual iUnknown* __stdcall GetItemData(tU32 anItem) const = 0;
  //! Set the sorting key column.
  //! \param  anKeyColumn is the column to use as key for sorting.
  //! \return If the key index is invalid sorting is disabled, and false is returned.
  //! {Property}
  virtual tBool __stdcall SetSortKey(tU32 anKeyColumn) = 0;
  //! Get the sorting key column.
  //! \return The current sort key column index.
  //! {Property}
  virtual tU32 __stdcall GetSortKey() const = 0;
  //! Set whether to use ascendant or descendant sorting.
  //! \param abAscendant decide the sorting Mode.
  //! {Property}
  virtual void __stdcall SetSortAscendant(tBool abAscendant) = 0;
  //! Get whether to use ascendant or descendant sorting.
  //! {Property}
  virtual tBool __stdcall GetSortAscendant() const = 0;
  //! Get the index of the first item that has the specified text in the specified column.
  virtual tU32 __stdcall GetItemFromText(tU32 anColumn, const achar* aaszName) const = 0;

  //! Clear the selection.
  virtual void __stdcall ClearSelection() = 0;
  //! Add a selection.
  virtual tBool __stdcall AddSelection(tU32 anItem) = 0;
  //! Remove a selection.
  virtual tBool __stdcall RemoveSelection(tU32 anItem) = 0;
  //! Get the number of selections.
  //! {Property}
  virtual tU32 __stdcall GetNumSelections() const = 0;
  //! Get the active selection at the specified index.
  //! {Property}
  virtual tU32 __stdcall GetSelection(tU32 anIndex) const = 0;
  //! Set all selections to the selection specified.
  //! \remark This will set one active selection.
  //! {Property}
  virtual tBool __stdcall SetSelected(tU32 anSelection) = 0;
  //! Get the first selection.
  //! {Property}
  virtual tU32 __stdcall GetSelected() const = 0;
  //! Check if the specified item is selected.
  //! {Property}
  virtual tBool __stdcall GetIsItemSelected(tU32 anItem) const = 0;
  //! Get the first item with the selected text in the specified column and add it to the selection.
  virtual tBool __stdcall AddSelectedItem(tU32 anCol, const achar* aaszText) = 0;
  //! Get the text in the column of the selection at the specified index.
  virtual const achar* __stdcall GetSelectedItemText(tU32 anCol, tU32 anIndex) const = 0;
  //! Get the widget in the column of the selection at the specified index.
  virtual iWidget* __stdcall GetSelectedItemWidget(tU32 anCol, tU32 anIndex) const = 0;
  //! Get the first item with the selected text in the specified column and set it as the selection.
  //! {Property}
  virtual tBool __stdcall SetSelectedItem(tU32 anCol, const achar* aaszText) = 0;
  //! Get the text in the specified column of the first selection.
  //! {Property}
  virtual const achar* __stdcall GetSelectedItem(tU32 anCol) const = 0;
  //! Set the height of an item.
  //! {Property}
  //! \remark This is meant to be used to define the maximum size of controls embedded in the listbox.
  //! \remark The height will always be >= than the font height no matter what this value is.
  virtual tBool __stdcall SetItemHeight(tF32 afHeight) = 0;
  //! Get the height of an item.
  //! {Property}
  virtual tF32 __stdcall GetItemHeight() const = 0;

  //! Auto scroll to the selection or last item if no item is selected.
  virtual void __stdcall AutoScroll() = 0;

  //! Set the color of the text of the specified column/item. Set anColumn to eInvalidHandle to set all the columns at once.
  virtual tBool __stdcall SetItemTextColor(tU32 anColumn, tU32 anItem, tU32 anTextColor) = 0;
  //! Set the color of the text of the specified column/item.
  virtual tU32 __stdcall GetItemTextColor(tU32 anColumn, tU32 anItem) const = 0;


  //! Set the maximum number of items.
  //! \remark When the maximum number of items the items at the top of the
  //!         list are removed, this is meant to be used for a log window or
  //!         similar.
  //! {Property}
  virtual void __stdcall SetMaxNumItems(tU32 anMaxItems) = 0;
  //! Get the maximum number of items.
  //! {Property}
  virtual tU32 __stdcall GetMaxNumItems() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETLISTBOX_47171268_H__
