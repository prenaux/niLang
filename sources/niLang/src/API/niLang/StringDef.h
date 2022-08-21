#pragma once
#ifndef __STRINGDEF_H_AE1C5EFC_3561_44FB_83F5_E5DACC64A138__
#define __STRINGDEF_H_AE1C5EFC_3561_44FB_83F5_E5DACC64A138__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "StringBase.h"
#include "Var.h"
#include "Utils/CollectionImpl.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_String
 * @{
 */

//--------------------------------------------------------------------------------------------
//
//  String format
//
//--------------------------------------------------------------------------------------------

#ifdef ni64
niCAssert(sizeof(cString) == 16);
#else
niCAssert(sizeof(cString) == 12);
#endif

#define niFmt       ni::cString().Format
#define _ASTR(X)    ni::cString(X)
#define _ASZ(X)     ni::cString(X).Chars()

//--------------------------------------------------------------------------------------------
//
//  String utilities
//
//--------------------------------------------------------------------------------------------

static inline const achar* ToChars(const achar* v) {
  return v;
}
static inline const achar* ToChars(const cString& v) {
  return v.c_str();
}

__forceinline void ni_log(tLogFlags type, const char* file, const char* func, int line, const cString& msg) {
  return ni_log(type, file, func, line, msg.c_str());
}

niExportFunc(tU32) ni_get_last_logs(astl::vector<cString>* logs, tSize numLogs);

niExportFuncCPP(cString&) StringToPropertyName(ni::cString& d, const achar* aszName);
niExportFuncCPP(cString&) StringToPropertyMethodName(ni::cString& d, const achar* aszName);
niExportFuncCPP(cString&) StringToPropertyName2(ni::cString& d, const achar* aszName);
niExportFuncCPP(cString&) StringToPropertyMethodName2(ni::cString& d, const achar* aszName);
niExportFuncCPP(cString&) StringEncodeXml(cString& aXMLString, const achar* aaszString);
niExportFuncCPP(cString&) StringDecodeXml(cString& aDecodedString, const achar* aaszXMLString);

niExportFuncCPP(cString&) StringEncodeUrl(cString& o, const char *str, tI32 strLen);
static inline cString& StringEncodeUrl(cString& o, const cString& str) {
  return StringEncodeUrl(o, str.Chars(), str.size());
}
niExportFuncCPP(cString&) StringDecodeUrl(cString& o, const char *str, tI32 strLen);
static inline cString& StringDecodeUrl(cString& o, const cString& str) {
  return StringDecodeUrl(o, str.Chars(), str.size());
}

static inline cString GetTypeString(tType aType) {
  char buf[64];
  return GetTypeString(buf,aType);
}

niExportFuncCPP(cString) StringURLGetProtocol(const achar* aURL);
niExportFuncCPP(cString) StringURLGetPath(const achar* aURL);

niExportFuncCPP(tU32) StringSplit(const cString& aToSplit, const achar* aaszSeparators, astl::vector<cString>* apOut);
niExportFuncCPP(void) StringSplitNameFlags(const cString& aToSplit, cString* pstrName, astl::vector<cString>* plstFlags);
niExportFuncCPP(tU32) StringSplitSep(const cString& aToSplit, const achar* aaszSeparators, astl::vector<cString>* apOut);
niExportFuncCPP(tU32) StringSplitSepQuoted(const cString& aToSplit, const achar* aaszSeparators, const tU32 aQuote, astl::vector<cString>* apOut);
niExportFuncCPP(cString) StringVecJoin(const astl::vector<cString>& aVec, const achar* aaszJoin);

niExportFuncCPP(tU32) StringSplitScript(
  astl::vector<cString>* apStatements,
  const achar* aScript,
  const achar* aSeparator,
  const achar* aCommentPrefix,
  const achar* aCommentBlockStart, const achar* aCommentBlockEnd,
  tBool abNormalizeEmptyChars);

niExportFunc(tBool) StringEncodeCsvShouldQuote(const achar *aaszString, const achar aDelim, const achar aQuote);
niExportFuncCPP(cString) StringEncodeCsvQuote(const char *aaszString, const char aQuote);

niExportFuncCPP(cString&) StringAppendCsvValue(
  cString& o, const achar aDelim, const achar aQuote,
  const achar* aValue, const tU32 i);

template <typename T>
cString& StringAppendCsvValues(
  cString& o, const achar aDelim, const achar aQuote,
  const T* apValues, const tU32 anNumValues)
{
  if (apValues && anNumValues > 0) {
    niLoop(i,anNumValues) {
      StringAppendCsvValue(o, aDelim, aQuote, ToChars(apValues[i]), i);
    }
  }
  return o;
}

template <typename T>
cString& StringAppendCsvValues(
  cString& o, const achar aDelim, const achar aQuote, const T& container)
{
  tU32 i = 0;
  astl::for_each(
    container.begin(), container.end(),
    [&](const typename T::value_type& v) {
      StringAppendCsvValue(o, aDelim, aQuote, ToChars(v), i++);
    });
  return o;
}

//! A valid quoted field starts with a quote, trailing spaces are not allowed.
niExportFunc(tBool) StringDecodeCsvShouldUnquote(const char *str, const tU32 aQuote);

niExportFuncCPP(cString&) StringDecodeCsvUnquote(
  cString& o, const tU32 aQuote, const char *str, tI32 strLen);
static inline cString& StringDecodeCsvUnquote(cString& o, const tU32 aQuote, const cString& str) {
  return StringDecodeCsvUnquote(o, aQuote, str.Chars(), str.size());
}

niExportFuncCPP(tU32) StringSplitCsvFields(const cString& aToSplit,
                                           const achar* aaszSeparators,
                                           const tU32 aQuote,
                                           astl::vector<cString>* apOut);

//--------------------------------------------------------------------------------------------
//
//  String vararg format
//
//--------------------------------------------------------------------------------------------

niExportFuncCPP(cString&) StringCatFormatEx(cString& o, iExpressionContext* apExprCtx, const achar* aaszFormat, const Var** apArgs, tU32 anNumArgs);

// No args
static inline cString& StringCatFormat(cString& o, const achar* aaszFormat) {
  return StringCatFormatEx(o,NULL,aaszFormat,NULL,0);
}
static inline cString& StringCatFormat(cString& o, iExpressionContext* apExprCtx, const achar* aaszFormat) {
  return StringCatFormatEx(o,apExprCtx,aaszFormat,NULL,0);
}
// 1 arg
static inline cString& StringCatFormat(cString& o, const achar* aaszFormat, const Var& a1) {
  const Var* args[1] = { &a1 };
  return StringCatFormatEx(o,NULL,aaszFormat,args,niCountOf(args));
}
static inline cString& StringCatFormat(cString& o, iExpressionContext* apExprCtx, const achar* aaszFormat, const Var& a1) {
  const Var* args[1] = { &a1 };
  return StringCatFormatEx(o,apExprCtx,aaszFormat,args,niCountOf(args));
}
// 2 args
static inline cString& StringCatFormat(cString& o, const achar* aaszFormat, const Var& a1, const Var& a2) {
  const Var* args[2] = { &a1, &a2 };
  return StringCatFormatEx(o,NULL,aaszFormat,args,niCountOf(args));
}
static inline cString& StringCatFormat(cString& o, iExpressionContext* apExprCtx, const achar* aaszFormat, const Var& a1, const Var& a2) {
  const Var* args[2] = { &a1, &a2 };
  return StringCatFormatEx(o,apExprCtx,aaszFormat,args,niCountOf(args));
}
// 3-4 args
static inline cString& StringCatFormat(cString& o, const achar* aaszFormat, const Var& a1, const Var& a2, const Var& a3, const Var& a4 = niVarNull) {
  const Var* args[4] = { &a1, &a2, &a3, &a4 };
  return StringCatFormatEx(o,NULL,aaszFormat,args,niCountOf(args));
}
static inline cString& StringCatFormat(cString& o, iExpressionContext* apExprCtx, const achar* aaszFormat, const Var& a1, const Var& a2, const Var& a3, const Var& a4 = niVarNull) {
  const Var* args[4] = { &a1, &a2, &a3, &a4 };
  return StringCatFormatEx(o,apExprCtx,aaszFormat,args,niCountOf(args));
}
// 5-8 args
static inline cString& StringCatFormat(cString& o, const achar* aaszFormat, const Var& a1, const Var& a2, const Var& a3, const Var& a4, const Var& a5, const Var& a6 = niVarNull, const Var& a7 = niVarNull, const Var& a8 = niVarNull) {
  const Var* args[8] = { &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8 };
  return StringCatFormatEx(o,NULL,aaszFormat,args,niCountOf(args));
}
static inline cString& StringCatFormat(cString& o, iExpressionContext* apExprCtx, const achar* aaszFormat, const Var& a1, const Var& a2, const Var& a3, const Var& a4, const Var& a5, const Var& a6 = niVarNull, const Var& a7 = niVarNull, const Var& a8 = niVarNull) {
  const Var* args[8] = { &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8 };
  return StringCatFormatEx(o,apExprCtx,aaszFormat,args,niCountOf(args));
}
// 9-24 args
static inline cString& StringCatFormat(cString& o,
                                       const achar* aaszFormat,
                                       const Var& a1, const Var& a2, const Var& a3,
                                       const Var& a4, const Var& a5, const Var& a6,
                                       const Var& a7, const Var& a8, const Var& a9,
                                       const Var& a10, const Var& a11, const Var& a12,
                                       const Var& a13, const Var& a14, const Var& a15,
                                       const Var& a16, const Var& a17, const Var& a18,
                                       const Var& a19, const Var& a20, const Var& a21,
                                       const Var& a22, const Var& a23, const Var& a24)
{
  const Var* args[24] = {
    &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8,
    &a9, &a10, &a11, &a12, &a13, &a14, &a15, &a16,
    &a17, &a18, &a19, &a20, &a21, &a22, &a23, &a24
  };
  return StringCatFormatEx(o,NULL,aaszFormat,args,niCountOf(args));
}
static inline cString& StringCatFormat(cString& o,
                                       iExpressionContext* apExprCtx,
                                       const achar* aaszFormat,
                                       const Var& a1, const Var& a2, const Var& a3,
                                       const Var& a4, const Var& a5, const Var& a6,
                                       const Var& a7, const Var& a8, const Var& a9,
                                       const Var& a10, const Var& a11, const Var& a12,
                                       const Var& a13, const Var& a14, const Var& a15,
                                       const Var& a16, const Var& a17, const Var& a18,
                                       const Var& a19, const Var& a20, const Var& a21,
                                       const Var& a22, const Var& a23, const Var& a24)
{
  const Var* args[24] = {
    &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8,
    &a9, &a10, &a11, &a12, &a13, &a14, &a15, &a16,
    &a17, &a18, &a19, &a20, &a21, &a22, &a23, &a24
  };
  return StringCatFormatEx(o,apExprCtx,aaszFormat,args,niCountOf(args));
}

//--------------------------------------------------------------------------------------------
//
//  String var dependant implementation
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
inline const achar* cString::Set(tF32 dValue) {
	return Format(ConstFmtFloat(),dValue);
}

///////////////////////////////////////////////
inline const achar* cString::Set(tF64 dValue) {
	return Format(ConstFmtFloat(),dValue);
}

///////////////////////////////////////////////
inline const achar* cString::Set(const tUUID& aUUID)
{
  return Format(ConstUUIDFmt(),
                niSwapLE32(aUUID.nData1),niSwapLE32(aUUID.nData2),niSwapLE32(aUUID.nData3),
                aUUID.nData4[0],aUUID.nData4[1],aUUID.nData4[2],aUUID.nData4[3],
                aUUID.nData4[4],aUUID.nData4[5],aUUID.nData4[6],aUUID.nData4[7]);
}

///////////////////////////////////////////////
inline const achar* cString::CatFormat(const achar* aaszFormat) {
  return StringCatFormat(*this,aaszFormat).Chars();
}
inline const achar* cString::CatFormat(iExpressionContext* apExprCtx, const achar* aaszFormat) {
  return StringCatFormat(*this,apExprCtx,aaszFormat).Chars();
}

///////////////////////////////////////////////
inline const achar* cString::CatFormat(const achar* aaszFormat, const Var& a1) {
  return StringCatFormat(*this,aaszFormat,a1).Chars();
}
inline const achar* cString::CatFormat(iExpressionContext* apExprCtx, const achar* aaszFormat, const Var& a1) {
  return StringCatFormat(*this,apExprCtx,aaszFormat,a1).Chars();
}

///////////////////////////////////////////////
inline const achar* cString::CatFormat(const achar* aaszFormat,
                                       const Var& a1, const Var& a2,
                                       const Var& a3, const Var& a4)
{
  return StringCatFormat(*this,aaszFormat,a1,a2,a3,a4).Chars();
}
inline const achar* cString::CatFormat(
  iExpressionContext* apExprCtx,
  const achar* aaszFormat,
  const Var& a1, const Var& a2,
  const Var& a3, const Var& a4)
{
  return StringCatFormat(*this,apExprCtx,aaszFormat,a1,a2,a3,a4).Chars();
}

///////////////////////////////////////////////
inline const achar* cString::CatFormat(
    const achar* aaszFormat,
    const Var& a1, const Var& a2, const Var& a3, const Var& a4, const Var& a5,
    const Var& a6, const Var& a7, const Var& a8)
{
  return StringCatFormat(*this,aaszFormat,a1,a2,a3,a4,a5,a6,a7,a8).Chars();
}
inline const achar* cString::CatFormat(
  iExpressionContext* apExprCtx,
  const achar* aaszFormat,
  const Var& a1, const Var& a2, const Var& a3, const Var& a4, const Var& a5,
  const Var& a6, const Var& a7, const Var& a8)
{
  return StringCatFormat(*this,apExprCtx,aaszFormat,a1,a2,a3,a4,a5,a6,a7,a8).Chars();
}

///////////////////////////////////////////////
inline const achar* cString::CatFormat(
    const achar* aaszFormat,
    const Var& a1, const Var& a2, const Var& a3,
    const Var& a4, const Var& a5, const Var& a6,
    const Var& a7, const Var& a8, const Var& a9,
    const Var& a10, const Var& a11, const Var& a12,
    const Var& a13, const Var& a14, const Var& a15,
    const Var& a16, const Var& a17, const Var& a18,
    const Var& a19, const Var& a20, const Var& a21,
    const Var& a22, const Var& a23, const Var& a24)
{
  return StringCatFormat(*this,aaszFormat,
                         a1, a2, a3, a4, a5, a6, a7, a8,
                         a9, a10, a11, a12, a13, a14, a15, a16,
                         a17, a18, a19, a20, a21, a22, a23, a24).Chars();
}
inline const achar* cString::CatFormat(
  iExpressionContext* apExprCtx,
  const achar* aaszFormat,
  const Var& a1, const Var& a2, const Var& a3,
  const Var& a4, const Var& a5, const Var& a6,
  const Var& a7, const Var& a8, const Var& a9,
  const Var& a10, const Var& a11, const Var& a12,
  const Var& a13, const Var& a14, const Var& a15,
  const Var& a16, const Var& a17, const Var& a18,
  const Var& a19, const Var& a20, const Var& a21,
  const Var& a22, const Var& a23, const Var& a24)
{
  return StringCatFormat(*this,apExprCtx,aaszFormat,
                         a1, a2, a3, a4, a5, a6, a7, a8,
                         a9, a10, a11, a12, a13, a14, a15, a16,
                         a17, a18, a19, a20, a21, a22, a23, a24).Chars();
}

//--------------------------------------------------------------------------------------------
//
//  String concat operations
//
//--------------------------------------------------------------------------------------------

__forceinline ni::cString& operator<<(ni::cString& s, const ni::sVec2f& v) {
  s << "Vec2f(";
  s.CatFormat("%g", v.x);
  s << ",";
  s.CatFormat("%g", v.y);
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const ni::sVec3f& v) {
  s << "Vec3f(";
  s.CatFormat("%g", v.x);
  s << ",";
  s.CatFormat("%g", v.y);
  s << ",";
  s.CatFormat("%g", v.z);
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const ni::sVec4f& v) {
  s << "Vec4f(";
  s.CatFormat("%g", v.x);
  s << ",";
  s.CatFormat("%g", v.y);
  s << ",";
  s.CatFormat("%g", v.z);
  s << ",";
  s.CatFormat("%g", v.w);
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const ni::sVec2i& v) {
  s << "Vec2i(";
  s.CatFormat("%d", v.x);
  s << ",";
  s.CatFormat("%d", v.y);
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const ni::sVec3i& v) {
  s << "Vec3i(";
  s.CatFormat("%d", v.x);
  s << ",";
  s.CatFormat("%d", v.y);
  s << ",";
  s.CatFormat("%d", v.z);
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const ni::sVec4i& v) {
  s << "Vec4i(";
  s.CatFormat("%d", v.x);
  s << ",";
  s.CatFormat("%d", v.y);
  s << ",";
  s.CatFormat("%d", v.z);
  s << ",";
  s.CatFormat("%d", v.w);
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const ni::sMatrixf& v) {
  s << "Matrixf(";
  s.CatFormat("%g", v._11);
  s << ",";
  s.CatFormat("%g", v._12);
  s << ",";
  s.CatFormat("%g", v._13);
  s << ",";
  s.CatFormat("%g", v._14);
  s << ", ";
  s.CatFormat("%g", v._21);
  s << ",";
  s.CatFormat("%g", v._22);
  s << ",";
  s.CatFormat("%g", v._23);
  s << ",";
  s.CatFormat("%g", v._24);
  s << ", ";
  s.CatFormat("%g", v._31);
  s << ",";
  s.CatFormat("%g", v._32);
  s << ",";
  s.CatFormat("%g", v._33);
  s << ",";
  s.CatFormat("%g", v._34);
  s << ", ";
  s.CatFormat("%g", v._41);
  s << ",";
  s.CatFormat("%g", v._42);
  s << ",";
  s.CatFormat("%g", v._43);
  s << ",";
  s.CatFormat("%g", v._44);
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const ni::Var& var) {
  s << "Var(";
  s << var.mType;
  s << ":";
  s << var.mInt;
  s << ")";
  return s;
}
__forceinline ni::cString& operator<<(ni::cString& s, const std::nullptr_t&) {
  s << "nullptr";
  return s;
}

/**@}*/
/**@}*/
}

//--------------------------------------------------------------------------------------------
//
//  String hash
//
//--------------------------------------------------------------------------------------------
namespace eastl {

ASTL_TEMPLATE_NULL
struct hash<ni::cString> {
  size_t operator()(const ni::cString& __s) const {
    return __s.Hash();
  }
};

}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __STRINGDEF_H_AE1C5EFC_3561_44FB_83F5_E5DACC64A138__
