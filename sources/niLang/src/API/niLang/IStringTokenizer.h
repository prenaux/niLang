#pragma once
#ifndef __ISTRINGTOKENIZER_25608894_H__
#define __ISTRINGTOKENIZER_25608894_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Char type returned by the methods of the tokenizer interface.
enum eStringTokenizerCharType
{
  //! The char is normal and will be added to the current token.
  eStringTokenizerCharType_Normal = 0,
  //! The char will be skipped.
  eStringTokenizerCharType_Skip = 1,
  //! The char mark a split.
  eStringTokenizerCharType_Splitter = 2,
  //! The char mark a split that include this character at the begining of the next token.
  eStringTokenizerCharType_SplitterStart = 3,
  //! The char mark a split that include this character at the end of the current token.
  eStringTokenizerCharType_SplitterEnd = 4,
  //! The char mark a split and is a token.
  eStringTokenizerCharType_SplitterAndToken = 5,
  //! To force the compiler to put the enum in a DWORD
  eStringTokenizerCharType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! iStringTokenizer interface
/** String tokenizer is used by the Tokenize() method of the string class to tokenize.
 * This allow to implement any rule of tokenizing.
 */
//! {DispatchWrapper}
struct iStringTokenizer : public iUnknown
{
  niDeclareInterfaceUUID(iStringTokenizer,0x59cd4f04,0xd5bb,0x4e1e,0xbb,0x23,0xbc,0x1a,0x5b,0xb4,0x5b,0xdc)
  virtual eStringTokenizerCharType __stdcall GetCharType(tU32 c) = 0;
  virtual void __stdcall OnNewLine() = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISTRINGTOKENIZER_25608894_H__
