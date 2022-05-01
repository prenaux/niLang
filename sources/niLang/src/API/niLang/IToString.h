#pragma once
#ifndef __ITOSTRING_56944860_H__
#define __ITOSTRING_56944860_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

enum eRawToStringEncoding {
  eRawToStringEncoding_Hex = 0,
  eRawToStringEncoding_Base64 = 1,
  eRawToStringEncoding_Base32 = 2,
  //! \internal
  eRawToStringEncoding_ForceDWORD = 0xFFFFFFFF
};

niExportFunc(tInt) Base32EncodeOutputSize(tInt slen);
niExportFunc(tInt) Base32Encode(tPtr dst, tInt dlen, const tPtr src, tInt slen, const tBool abUpperCase = eFalse);
niExportFunc(tInt) Base32DecodeOutputSize(const char* src, tInt slen);
niExportFunc(tInt) Base32Decode(tPtr dst, tInt dlen, const char* src, tInt slen);
niExportFuncCPP(cString) Base32EncodeToString(const tPtr src, tInt slen, const tBool abUpperCase = eFalse);

niExportFunc(tInt) Base64EncodeOutputSize(tInt slen);
niExportFunc(tInt) Base64Encode(tPtr dst, tInt dlen, const tPtr src, tInt slen);
niExportFunc(tInt) Base64DecodeOutputSize(const char* src, tInt slen);
niExportFunc(tInt) Base64Decode(tPtr dst, tInt dlen, const char* src, tInt slen);
niExportFuncCPP(cString) Base64EncodeToString(const tPtr src, tInt slen);

niExportFunc(tInt) HexEncodeOutputSize(tInt slen);
niExportFunc(tInt) HexEncode(tPtr dst, tInt dlen, const tPtr src, tInt slen, const tBool abUpperCase = eFalse);
niExportFunc(tInt) HexDecodeOutputSize(const char* src, tInt slen);
niExportFunc(tInt) HexDecode(tPtr dst, tInt dlen, const char* src, tInt slen);
niExportFuncCPP(cString) HexEncodeToString(const tPtr src, tInt slen, const tBool abUpperCase = eFalse);

//! iToString interface.
/** ToString objects are all objects that can be converted to a string. To do so the String
 * classes support the << operator and constructor for all iToString objects.
 */
struct iToString : public iUnknown
{
  niDeclareInterfaceUUID(iToString,0x78b80893,0x3543,0x408e,0xaa,0x0f,0xac,0xa5,0x59,0x8c,0x2d,0x05)
  //! Convert the object to a string.
  virtual ni::cString __stdcall ToString() const = 0;
};

//template <typename T, eStrEncoding TENC>
//inline tBool ToString(const iToString* apToString, cStringBase<T,TENC>& dest);
// implemented in StringBase.h

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ITOSTRING_56944860_H__
