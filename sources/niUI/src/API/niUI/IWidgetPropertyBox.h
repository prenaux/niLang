#ifndef __IWIDGETPROPERTYBOX_8630971_H__
#define __IWIDGETPROPERTYBOX_8630971_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */
#if niMinFeatures(20)

//! PropertyBox widget interface.
//! \remark To notifications of the datatable's modifications register a iDataTableSink int the
//!     datatable of the property box.
//! {DispatchWrapper}
struct iWidgetPropertyBox : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetPropertyBox,0xa0370d2b,0x9d59,0x416f,0x93,0x94,0x35,0xc0,0xee,0x08,0xc9,0x28);

  //! Set the property box's data table.
  //! {Property}
  //! \remark By default an empty data table is created with the property box.
  virtual void __stdcall SetDataTable(iDataTable* apDT) = 0;
  //! Get the property box's data table.
  //! {Property}
  //! \remark By default an empty data table is created with the property box.
  virtual iDataTable* __stdcall GetDataTable() const = 0;
};

#endif
/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETPROPERTYBOX_8630971_H__
