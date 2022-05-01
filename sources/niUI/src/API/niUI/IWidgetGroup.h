#ifndef __IWIDGETGROUP_4899192_H__
#define __IWIDGETGROUP_4899192_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Group styles.
enum eWidgetGroupStyle {
  //! Toggle the folded state when double clicking on the title.
  eWidgetGroupStyle_Fold = niBit(eWidgetStyle_MaxBit+1),
  //! \internal
  eWidgetGroupStyle_ForceDWORD = 0xFFFFFFFF
};

//! Group widget interface.
struct iWidgetGroup : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetGroup,0xee88a289,0xfbc6,0x4741,0xa2,0x0e,0xe5,0x00,0xe5,0x1d,0x7e,0x50);

  //! Set whether the group is folded.
  //! {Property}
  virtual void __stdcall SetFolded(tBool abFolded) = 0;
  //! Get whether the group is folded.
  //! {Property}
  virtual tBool __stdcall GetFolded() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IWIDGETGROUP_4899192_H__
