#pragma once
#ifndef __IHSTRING_H__
#define __IHSTRING_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "STL/hash_map.h"
#include "STL/map.h"
#include "Utils/SmartPtr.h"
#include "StringLib.h"

namespace ni {

struct iHStringCharIt;

/** \addtogroup niLang
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! HString interface.
struct iHString : public iUnknown
{
  niDeclareInterfaceUUID(iHString,0x0f0162e8,0x64c3,0x4276,0x9e,0x1e,0x42,0xbc,0x02,0x42,0x43,0x23)
  //! Get the string's characters.
  //! {Property}
  virtual const achar* __stdcall GetChars() const = 0;
  //! Get the string's length.
  //! {Property}
  virtual tU32 __stdcall GetLength() const = 0;
  //! Compare this string with another HString.
  //! \remark If the passed string is not from the same string table
  //!         a regular string compare will be performed.
  virtual tI32 __stdcall Cmp(const iHString* ahspString) const = 0;
  //! Case insensitive compare with another HString.
  //! \remark If the passed string is not from the same string table
  //!         a regular string compare will be performed.
  virtual tI32 __stdcall ICmp(const iHString* ahspString) const = 0;
  //! Get the string localized in the default locale.
  //! {Property}
  virtual iHString* __stdcall GetLocalized() const = 0;
  //! Get the string localized in the specified locale.
  virtual iHString* __stdcall GetLocalizedEx(iHString* locale) const = 0;
  //! Get the whether the string is localized in the specified locale.
  virtual tBool __stdcall IsLocalized(iHString* locale) const = 0;
  //! Return a character iterator to iterate over the string
  //! starting at the specified byte offset.
  //! \return NULL if the offset is out of range, else the new iterator.
  virtual iHStringCharIt* __stdcall CreateCharIt(tU32 offset) const = 0;
  //! Return a character iterator to iterate over the specifed byte
  //! range in the string.
  //! \return NULL if the offset or size is out of range, else the new iterator.
  virtual iHStringCharIt* __stdcall CreateRangeIt(tU32 offset, tU32 size) const = 0;
};

//! HString smart pointer.
typedef Ptr<iHString> tHStringPtr;

//! Get the string in a HString. Returns an empty string if the HString is null.
inline const achar* HStringGetStringEmpty(const iHString* ahsp) { return ahsp?ahsp->GetChars():_A(""); }
//! Get the string in a HString. Returns an null string if the HString is null.
inline const achar* HStringGetStringNull(const iHString* ahsp)  { return ahsp?ahsp->GetChars():NULL; }
//! Check if an HString is valid and not empty.
inline tBool HStringIsNotEmpty(const iHString* apStr) { return apStr && apStr->GetLength()>0; }
//! Check if an HString is valid and empty.
inline tBool HStringIsEmpty(const iHString* apStr)  { return apStr == NULL || apStr->GetLength()<=0; }

//! Get the localized string in a HString. Returns an empty string if the HString is null.
inline const achar* HStringGetStringLocalized(const iHString* ahsp) {
  return ahsp?HStringGetStringEmpty(ahsp->GetLocalized()):_A("");
}
//! Get the localized string in a HString. Returns an empty string if the HString is null.
inline const achar* HStringGetStringLocalized(iHString* locale, const iHString* ahsp) {
  return ahsp?HStringGetStringEmpty(ahsp->GetLocalizedEx(locale)):_A("");
}

//! Get the hstring chars
#define niHStr(X)     ni::HStringGetStringEmpty(X)
//! Get the localized hstring chars
#define niLStr(X)     ni::HStringGetStringLocalized(X)

//////////////////////////////////////////////////////////////////////////////////////////////
//! HString character iterator interface.
struct iHStringCharIt : public iUnknown
{
  niDeclareInterfaceUUID(iHStringCharIt,0xa6885894,0xf6a9,0x40fb,0x82,0x06,0x57,0x1f,0x3c,0x97,0x42,0x78);

  //! Get the string being iterated.
  //! {Property}
  virtual iHString* __stdcall GetString() const = 0;
  //! Clone the iterator.
  virtual iHStringCharIt* __stdcall Clone() const = 0;
  //! Get whether the current position is the start of the iterator.
  //! {Property}
  virtual tBool __stdcall GetIsStart() const = 0;
  //! Get whether the current position is the end of the iterator.
  //! {Property}
  virtual tBool __stdcall GetIsEnd() const = 0;
  //! Get the current position from the start of the iterator in bytes.
  //! {Property}
  virtual tU32 __stdcall GetPosition() const = 0;
  //! Move the iterator to its starting point.
  virtual void __stdcall ToStart() = 0;
  //! Move the iterator to its end point.
  virtual tSize __stdcall ToEnd() = 0;
  //! Get the number of characters covered by the iterator.
  //! {Property}
  virtual tSize __stdcall GetNumChars() const = 0;
  //! Get the number of bytes covered by the iterator.
  //! {Property}
  virtual tSize __stdcall GetNumBytes() const = 0;
  //! Return the next character in the string without moving the iterator.
  virtual tU32 __stdcall PeekNext() const = 0;
  //! Return the next character in the string moving the iterator forward.
  virtual tU32 __stdcall Next() = 0;
  //! Return the previous character in the string without moving the iterator.
  virtual tU32 __stdcall PeekPrior() const = 0;
  //! Return the next character in the string moving the iterator backward.
  virtual tU32 __stdcall Prior() = 0;
  //! Return the nth character in the string without moving the iterator.
  virtual tU32 __stdcall PeekAdvance(tU32 fwd) const = 0;
  //! Return the nth character in the string moving the iterator forward.
  virtual tU32 __stdcall Advance(tU32 n) = 0;
  //! Return the nth previous character in the string without moving the iterator.
  virtual tU32 __stdcall PeekRewind(tU32 back) const = 0;
  //! Return the nth previous character in the string moving the iterator backward.
  virtual tU32 __stdcall Rewind(tU32 n) = 0;
  //! Move the iterator to the specified position in bytes.
  virtual void __stdcall ToPosition(tU32 anOffsetInBytes) = 0;
};

/**@}*/
}

namespace astl {

//! Hash a HString
struct hstring_hash {
  size_t operator()(const ni::iHString* __x) const {
    return (size_t)__x;
  }
};

//! HString less compare
struct hstring_less_cmp {
  bool operator()(const ni::iHString* s1, const ni::iHString* s2) const {
    return ni::StrCmp(niHStr(s1), niHStr(s2)) < 0;
  }
};

//! HString less case insensitive compare
struct hstring_less_icmp {
  bool operator()(const ni::iHString* s1, const ni::iHString* s2) const {
    return ni::StrICmp(niHStr(s1), niHStr(s2)) < 0;
  }
};

//! HString hash map
template <typename T>
struct hstring_hash_map : public astl::hash_map<ni::tHStringPtr,T,hstring_hash> {};

//! HString map, case sensitive sorted
template<typename T>
struct hstring_map_cmp : public astl::map<ni::tHStringPtr,T,hstring_less_cmp> {};

//! HString map, case insensitive sorted
template<typename T>
struct hstring_map_icmp : public astl::map<ni::tHStringPtr,T,hstring_less_icmp> {};

}

namespace eastl {

ASTL_TEMPLATE_NULL
struct hash<ni::tHStringPtr> : public astl::hstring_hash {
};

}
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __IHSTRING_H__
