#ifndef __IWIDGETSEARCHBAR_H_11F113D2_8794_4D35_9644_080F65DB125B__
#define __IWIDGETSEARCHBAR_H_11F113D2_8794_4D35_9644_080F65DB125B__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */
#if niMinFeatures(20)

//! Search bar widget notify messages.
enum eWidgetSearchBarCmd
{
  //! Sent when the selection changed
  eWidgetSearchBarCmd_SelectionChanged = 0,
  //! Sent when the search bar becomes visible
  eWidgetSearchBarCmd_Begin = 1,
  //! This is a good time to destroy or hide the search bar
  eWidgetSearchBarCmd_End = 2,
  //! \internal
  eWidgetSearchBarCmd_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Search container
struct iSearchContainer : public iUnknown
{
  niDeclareInterfaceUUID(iSearchContainer, 0x80858780, 0x8fe7, 0x44c2, 0xb3, 0xcd, 0xfe, 0xb9, 0xf8, 0xb5, 0xdd, 0x86)
  virtual void __stdcall AddResult(tU32 anIndex, const achar* aaszValue) = 0;
};

//! Searchable interface, to be searched impl it
//! {DispatchWrapper}
struct iSearchable : public iUnknown
{
  niDeclareInterfaceUUID(iSearchable, 0xc55ffa63, 0xd700, 0x4d67, 0xb9, 0x13, 0x9c, 0xc9, 0xe4, 0x82, 0x28, 0x7a)
  virtual void __stdcall OnSelect(tU32 nIndex) = 0;
  virtual void __stdcall GetCorpus(iSearchContainer* apContainer) = 0;
};

//! Search bar widget interface.
struct iWidgetSearchBar : public iUnknown
{
  niDeclareInterfaceUUID(iWidgetSearchBar,0xe46216f9, 0x1735, 0x4482, 0x8e, 0xad, 0x79, 0x35, 0x91, 0xed, 0x7d, 0xc2)
  //! Set a searchable to be search
  //! {Property}
  virtual void __stdcall SetSearchable(iSearchable* aS) = 0;
  //! Get a searchable object.
  //! {Property}
  virtual iSearchable* __stdcall GetSearchable() const = 0;
};

#endif
/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IWIDGETSEARCHBAR_H_11F113D2_8794_4D35_9644_080F65DB125B__
