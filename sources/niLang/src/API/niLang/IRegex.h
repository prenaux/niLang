#pragma once
#ifndef __IREGEX_61444864_H__
#define __IREGEX_61444864_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"
#include "ILang.h"

namespace ni {
struct iHString;

/** \addtogroup niLang
 * @{
 */

#if niMinFeatures(15)

//! PCRE option flags
enum ePCREOptionsFlags {
  //! Do caseless (case insensitive) matching.
  ePCREOptionsFlags_Caseless            = 0x00000001,
  //! '^' and '$' match newlines within data.
  ePCREOptionsFlags_Multiline           = 0x00000002,
  //! '.' matches anything including NL.
  ePCREOptionsFlags_DotAll              = 0x00000004,
  //! Ignore whitespace and # comments.
  ePCREOptionsFlags_Extended            = 0x00000008,
  //! Force pattern anchoring.
  ePCREOptionsFlags_Anchored            = 0x00000010,
  //! '$' not to match newline at end.
  ePCREOptionsFlags_DollarEndOnly       = 0x00000020,
  //! PCRE extra features (not much use currently).
  ePCREOptionsFlags_Extra               = 0x00000040,
  //! This option specifies that first character of the subject
  //! string is not the beginning of a line, so the circumflex
  //! metacharacter should not match before it. Setting this without
  //! MULTILINE (at compile time) causes circumflex never to
  //! match. This option affects only the behaviour of the
  //! circumflex metacharacter. It does not affect '\\A'.
  ePCREOptionsFlags_NotBOL              = 0x00000080,
  //! This option specifies that the end of the subject string is
  //! not the end of a line, so the dollar metacharacter should not
  //! match it nor (except in multiline mode) a newline immediately
  //! before it. Setting this without PCRE_MULTILINE (at compile
  //! time) causes dollar never to match. This option affects only
  //! the behaviour of the dollar metacharacter. It does not affect
  //! '\\Z' or '\\z'.
  ePCREOptionsFlags_NotEOL              = 0x00000100,
  //! Invert greediness of quantifiers.
  ePCREOptionsFlags_Ungreedy            = 0x00000200,
  //! An empty string is not considered to be a valid match if this
  //! option is set. If there are alternatives in the pattern, they
  //! are tried. If all the alternatives match the empty string, the
  //! entire match fails. For example, if the pattern.  "a?b?"  is
  //! applied to a string not beginning with "a" or "b", it matches
  //! the empty string at the start of the subject. With NOTEMPTY
  //! set, this match is not valid, so PCRE searches further into
  //! the string for occurrences of "a" or "b".
  ePCREOptionsFlags_NotEmpty            = 0x00000400,
  //! Run in UTF-8 mode.
  ePCREOptionsFlags_UTF8                = 0x00000800,
  //! Disable numbered capturing parentheses (named ones available).
  ePCREOptionsFlags_NoAutoCapture       = 0x00001000,
  //! Do not check the pattern for UTF-8 validity.
  ePCREOptionsFlags_NoUTF8Check         = 0x00002000,
  //! Compile automatic callouts.
  ePCREOptionsFlags_AutoCallout         = 0x00004000,
  //! Setting this option causes the matching algorithm to stop as
  //! soon as it has found one match. Because of the way the
  //! alternative algorithm works, this is necessarily the shortest
  //! possible match at the first possible matching point in the
  //! subject string.
  ePCREOptionsFlags_DFAShortest         = 0x00010000,
  //! Force matching to be before newline.
  ePCREOptionsFlags_FirstLine           = 0x00040000,
  //! Allow duplicate names for subpatterns.
  ePCREOptionsFlags_DupNames            = 0x00080000,
  //! Set CR as the newline sequence.
  ePCREOptionsFlags_NewLineCR           = 0x00100000,
  //! Set LF as the newline sequence.
  ePCREOptionsFlags_NewLineLF           = 0x00200000,
  //! Set CRLF as the newline sequence.
  ePCREOptionsFlags_NewLineCRLF         = 0x00300000,
  //! Recognize any Unicode newline sequence.
  ePCREOptionsFlags_NewLineAny          = 0x00400000,
  //! Recognize CR, LF, and CRLF as newline sequences.
  ePCREOptionsFlags_NewLineAnyCRLF      = 0x00500000,
  //! '\\R' matches only CR, LF, or CRLF.
  ePCREOptionsFlags_BsrAnyCRLF          = 0x00800000,
  //! '\\R' matches all Unicode line endings
  ePCREOptionsFlags_BsrUnicode          = 0x01000000,
  //! Global regular expression.
  ePCREOptionsFlags_Global              = niBit(30),
  //! Optimize the regular expression.
  ePCREOptionsFlags_Optimize            = niBit(31),
  //! \internal
  ePCREOptionsFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! PCRE option flags type. \see ni::ePCREOptionsFlags
typedef tU32 tPCREOptionsFlags;

//! PCRE compilation error
enum ePCREError {
  ePCREError_OK = 0,
  ePCREError_NoMatch        = (tU32)(-1),
  ePCREError_Null           = (tU32)(-2),
  ePCREError_BadOption      = (tU32)(-3),
  ePCREError_BadMagic       = (tU32)(-4),
  ePCREError_UnknownOpcode  = (tU32)(-5),
  ePCREError_NoMemory       = (tU32)(-6),
  ePCREError_NoSubstring    = (tU32)(-7),
  ePCREError_MatchLimit     = (tU32)(-8),
  ePCREError_Callout        = (tU32)(-9),
  ePCREError_BadUTF8        = (tU32)(-10),
  ePCREError_BadUTF8Offset  = (tU32)(-11),
  ePCREError_Partial        = (tU32)(-12),
  ePCREError_BadPartial     = (tU32)(-13),
  ePCREError_Internal       = (tU32)(-14),
  ePCREError_BadCount       = (tU32)(-15),
  ePCREError_DFA_UITEM      = (tU32)(-16),
  ePCREError_DFA_UCOND      = (tU32)(-17),
  ePCREError_DFA_UMLIMIT    = (tU32)(-18),
  ePCREError_DFA_WSSIZE     = (tU32)(-19),
  ePCREError_DFA_RECURSE    = (tU32)(-20),
  ePCREError_RecursionLimit = (tU32)(-21),
  ePCREError_NullWSLimit    = (tU32)(-22),
  ePCREError_BadNewLine     = (tU32)(-23),
  ePCREError_LoopLimit      = (tU32)(-24),
  ePCREError_BadRegexString = (tU32)(-1000),
  //! \internal
  ePCREError_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

#endif

//! Generic regular expression interface.
struct iRegex : public iUnknown
{
  niDeclareInterfaceUUID(iRegex,0xc48d7f86,0xd623,0x4327,0xb9,0xdb,0xdb,0x35,0x9e,0x1b,0xc0,0xe6)
  //! Get the implementation type of the regular experssion.
  //! {Property}
  virtual const achar* __stdcall GetImplType() const = 0;
  //! Search for the next match of the expression.
  virtual tBool __stdcall DoesMatch(const achar* aaszText) const = 0;
};

#if niMinFeatures(15)

//! PCRE Regular Expression interface.
struct iPCRE : public iRegex
{
  niDeclareInterfaceUUID(iPCRE,0xb168fd05,0x0fa2,0x4238,0x8b,0x44,0x51,0x5f,0xec,0x76,0x24,0x24)

  //########################################################################################
  //! \name Compilation
  //########################################################################################
  //! @{

  //! Compile a regular expression.
  //! \param aaszRegEx is the regular expression to compile.
  //! \param aaszOpt is perl style character modifier.<br/>
  //!        <ul>
  //!         <li>g = ePCREOptionsFlags_Global</li>
  //!         <li>i = ePCREOptionsFlags_Caseless</li>
  //!         <li>m = ePCREOptionsFlags_Multiline</li>
  //!         <li>s = ePCREOptionsFlags_DotAll</li>
  //!         <li>x = ePCREOptionsFlags_Extended</li>
  //!         <li>U = ePCREOptionsFlags_Ungreedy</li>
  //!        </ul>
  virtual ePCREError __stdcall Compile(const achar* aaszRegEx, const achar* aaszOpt) = 0;
  //! Compile a regular expression.
  //! \remark Uses option flags directly.
  virtual ePCREError __stdcall Compile2(const achar* aaszRegEx, tPCREOptionsFlags aOpt) = 0;
  //! Get whether the last compile has been successfull.
  //! {Property}
  virtual tBool __stdcall GetIsCompiled() const = 0;

  //! Get the last compilation error code.
  //! {Property}
  virtual ePCREError __stdcall GetLastCompileError() const = 0;
  //! Get the last compilation error description.
  //! {Property}
  virtual const achar* __stdcall GetLastCompileErrorDesc() const = 0;
  //! Get the last compilation error offset.
  //! {Property}
  //! \remark The offset is the 'index' of the character in the
  //!         compiled regex where the error has been detected.
  virtual tU32 __stdcall GetLastCompileErrorOffset() const = 0;

  //! Get the options used during the last compilation.
  //! {Property}
  virtual tPCREOptionsFlags __stdcall GetOptions() const = 0;
  //! @}

  //########################################################################################
  //! \name Matches
  //########################################################################################
  //! @{

  //! Resets the regex object.
  virtual void __stdcall Reset() = 0;
  //! Get the number of matches returned by the last Match/Sub call.
  //! {Property}
  virtual tU32 __stdcall GetNumMarkers() const = 0;

  //! Get the range of the last match.
  //! {Property}
  //! \remark A range is 'x' first character index matched, 'y' last character index matched.
  virtual sVec2i __stdcall GetMarker(tU32 anIndex) const = 0;
  //! Get the substring in string matched at the specified index.
  //! {Property}
  virtual cString __stdcall GetString(tU32 anIndex) const = 0;

  //! Get the number of named subpatterns.
  //! {Property}
  virtual tU32 __stdcall GetNumNamed() const = 0;
  //! Get the name of the subpattern at the specified index.
  //! {Property}
  virtual const achar* __stdcall GetNamedName(tU32 anIndex) const = 0;
  //! Get the range of a named match.
  //! {Property}
  //! \remark A range is 'x' first character index matched, 'y' last character index matched.
  //! \remark Convenience for GetMarker(GetNamedIndex(NAME))
  virtual sVec2i __stdcall GetNamedMarker(const achar* aaszName) const = 0;
  //! Get the substring in string matched at the specified index.
  //! {Property}
  //! \remark Convenience for GetString(GetNamedIndex(NAME))
  virtual cString __stdcall GetNamedString(const achar* aaszName) const = 0;
  //! Get the index of a named subpattern.
  //! \return eInvalidHandle if the subpattern hasnt been matched.
  //! \remark The index points in the markers.
  //! {Property}
  virtual tU32 __stdcall GetNamedIndex(const achar* aaszName) const = 0;

  //! Match against this regular expression.
  //! \return The number of matches found, -1 on error.
  virtual tI32 __stdcall MatchRaw(const achar* aaszString) = 0;
  //! Match against this regular expression.
  //! \return The number of matches found, -1 on error.
  virtual tI32 __stdcall Match(iHString* ahspString, tU32 anOffset) = 0;

  //! Splits based on delimiters matching the regex.
  //! \param aaszString is the string to process
  //! \param anMaxFields is the maximum number of fields to split
  //!        out. Zero (0) means split all fields, but discard any
  //!        trailing empty bits. Negative means split all fields
  //!        and keep trailing empty bits. Positive means keep up to
  //!        N fields including any empty fields lass than
  //!        N. Anything remaining is in the last field.
  virtual tI32 __stdcall Split(const achar* aaszString, tI32 anMaxFields) = 0;

  //! Substitues out whatever matches the regex for the second parameter.
  virtual cString __stdcall Sub(const achar* aaszString, const achar* aaszReplacement, tBool abDoDollarSub) = 0;
  //! @}
};

//! Creates a new PCRE regular expression object
niExportFunc(iPCRE*) CreatePCRE(const achar* aRegex = NULL, const achar* aOptions = NULL);

niExportFunc(ni::iUnknown*) New_niLang_PCRE(const ni::Var&,const ni::Var&);

#endif

niExportFunc(int) afilepattern_isvalid(const achar* pat);
niExportFunc(int) afilepattern_submatch(const achar* pat, const achar* fname);
niExportFunc(int) afilepattern_match(const achar* pat, const achar* fname);
niExportFunc(int) afilepattern_matchn(const achar* pat, const achar* fname);

//! Create a File Pattern regex object.
niExportFunc(ni::iRegex*) CreateFilePatternRegex(const ni::achar* aaszPattern, const ni::achar* aaszSep  = _A("|"));

niExportFunc(ni::iUnknown*) New_niLang_FilePatternRegex(const ni::Var&,const ni::Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IREGEX_61444864_H__
