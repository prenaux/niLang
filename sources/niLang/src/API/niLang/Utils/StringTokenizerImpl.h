#ifndef __STRINGTOKENIZERIMPL_30700010_H__
#define __STRINGTOKENIZERIMPL_30700010_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/UnknownImpl.h>
#include <niLang/StringLib.h>
#include "../IStringTokenizer.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Default tokenizer
/** Default tokenizer, the separator is the space character, strings between two quote are
 * one token, '\' char is ignored, '\\' is a '\', and '\"' is '"'.
 */
class cDefaultStringTokenizer : public cIUnknownImpl<iStringTokenizer>
{
 public:
  cDefaultStringTokenizer()
  {
    mbInString = eFalse;
    mbPrevBackslash = eFalse;
  }

  eStringTokenizerCharType __stdcall GetCharType(tU32 c)
  {
    eStringTokenizerCharType type = eStringTokenizerCharType_Normal;
    if (c == '\\')
    {
      if (mbPrevBackslash)
      {
        mbPrevBackslash = eFalse;
      }
      else
      {
        type = eStringTokenizerCharType_Skip;
        mbPrevBackslash = eTrue;
      }
    }
    else
    {
      if (c == '"')
      {
        if (!mbPrevBackslash)
          mbInString = mbInString?eFalse:eTrue;
      }
      else if (!mbInString && StrIsSpace(c))
      {
        type = eStringTokenizerCharType_Splitter;
      }
      mbPrevBackslash = eFalse;
    }
    return type;
  }

  void __stdcall OnNewLine() {
  }

 private:
  tBool mbPrevBackslash;
  tBool mbInString;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Function tokenizer
/** Function tokenizer, the separator are the space character, ',', '(' and ')', strings
 * between two quote are one token, '\' char is ignored, '\\' is a '\', and '\"' is '"'.
 */
class cFunctionStringTokenizer : public cIUnknownImpl<iStringTokenizer>
{
 public:
  cFunctionStringTokenizer()
  {
    mbInString = eFalse;
    mbPrevBackslash = eFalse;
  }

  eStringTokenizerCharType __stdcall GetCharType(tU32 c)
  {
    eStringTokenizerCharType type = eStringTokenizerCharType_Normal;
    if (c == '\\')
    {
      if (mbPrevBackslash)
      {
        mbPrevBackslash = eFalse;
      }
      else
      {
        type = eStringTokenizerCharType_Skip;
        mbPrevBackslash = eTrue;
      }
    }
    else
    {
      if (c == '"')
      {
        if (!mbPrevBackslash)
          mbInString = mbInString?eFalse:eTrue;
      }
      else if (!mbInString)
      {
        if (StrIsSpace(c))
        {
          type = eStringTokenizerCharType_Splitter;
        }
        else if (c == '(' || c == ')' || c == ',')
        {
          type = eStringTokenizerCharType_SplitterAndToken;
        }
      }
      mbPrevBackslash = eFalse;
    }
    return type;
  }

  void __stdcall OnNewLine() {
  }

 private:
  tBool mbPrevBackslash;
  tBool mbInString;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Flags tokenizer
/** Flags tokenizer, the separator are the '|' and space characters, strings
 * between two quote are one token, '\' char is ignored, '\\' is a '\', and '\"' is '"'.
 */
class cFlagsStringTokenizer : public cIUnknownImpl<iStringTokenizer>
{
 public:
  cFlagsStringTokenizer()
  {
    mbInString = eFalse;
    mbPrevBackslash = eFalse;
  }

  eStringTokenizerCharType __stdcall GetCharType(tU32 c)
  {
    eStringTokenizerCharType type = eStringTokenizerCharType_Normal;
    if (c == '\\')
    {
      if (mbPrevBackslash)
      {
        mbPrevBackslash = eFalse;
      }
      else
      {
        type = eStringTokenizerCharType_Skip;
        mbPrevBackslash = eTrue;
      }
    }
    else
    {
      if (c == '"')
      {
        if (!mbPrevBackslash)
          mbInString = mbInString?eFalse:eTrue;
      }
      else if (!mbInString && (StrIsSpace(c) || c == '|'))
      {
        type = eStringTokenizerCharType_Splitter;
      }
      mbPrevBackslash = eFalse;
    }
    return type;
  }

  void __stdcall OnNewLine() {
  }

 private:
  tBool mbPrevBackslash;
  tBool mbInString;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! CommandLine tokenizer
/** CommandLine tokenizer, the separator is the space character, strings between two quotes
 * are one token.
 */
class cCommandLineStringTokenizer : public cIUnknownImpl<iStringTokenizer>
{
 public:
  cCommandLineStringTokenizer()
  {
    mcInString = 0;
    mbInString = eFalse;
  }

  eStringTokenizerCharType __stdcall GetCharType(tU32 c)
  {
    eStringTokenizerCharType type = eStringTokenizerCharType_Normal;
    if ((c == mcInString) || (mcInString == 0 && (c == '"' || c == '\'')))
    {
      mbInString = !mbInString;
      if (!mbInString) mcInString = 0;
    }
    else if (!mbInString && StrIsSpace(c))
    {
      type = eStringTokenizerCharType_Splitter;
    }
    return type;
  }

  void __stdcall OnNewLine() {
  }

 private:
  tU32  mcInString;
  tBool mbInString;
};

///////////////////////////////////////////////
template <typename TVEC>
inline tSize StringTokenize(const cString& aToTok, TVEC& vTokens, iStringTokenizer* apTok)
{
  niAssert(niIsOK(apTok));
  if (!niIsOK(apTok)) return 0;

  cString current; current.reserve(64);
  tSize numToks = 0;
  const achar* pIterator = aToTok.Chars();
  do {
    tU32 c = StrGetNextX(&pIterator);
    if (!c) break;

    switch (apTok->GetCharType(c)) {
      case eStringTokenizerCharType_Normal: {
        current.appendChar(c);
        break;
      }
      case eStringTokenizerCharType_Skip: {
        break;
      }
      case eStringTokenizerCharType_SplitterStart: {
        if (current.IsNotEmpty()) {
          vTokens.push_back(current);
          ++numToks;
        }
        current.Clear(64);
        current.appendChar(c);
        break;
      }
      case eStringTokenizerCharType_SplitterEnd: {
        current.appendChar(c);
        vTokens.push_back(current);
        ++numToks;
        current.Clear(64);
        break;
      }
      case eStringTokenizerCharType_Splitter: {
        if (current.IsNotEmpty()) {
          vTokens.push_back(current);
          ++numToks;
        }
        current.Clear(64);
        break;
      }
      case eStringTokenizerCharType_SplitterAndToken: {
        if (current.IsNotEmpty()) {
          vTokens.push_back(current);
          ++numToks;
        }
        cString tmp; tmp.appendChar(c);
        vTokens.push_back(tmp);
        current.Clear(64);
        break;
      }
      default: break;
    }
  } while (1);
  if (current.IsNotEmpty()) {
    vTokens.push_back(current);
    ++numToks;
  }
  return numToks;
}

template <typename TOKENIZER>
struct sTokenGenerator {
private:
  TOKENIZER _tokenizer;
  const achar* _pIterator;
  cString _token;
  tI64 _numTokens;
  tU32 _queuedChar;

public:
  sTokenGenerator(const achar* apChars) : sTokenGenerator(apChars,TOKENIZER()) {
  }

  sTokenGenerator(const achar* apChars, TOKENIZER&& aTokenizer)
      : _tokenizer(aTokenizer) {
    _token.clear();
    _token.reserve(64);
    _numTokens = 0;
    _pIterator = apChars;
    _queuedChar = 0;
  }

  const tI64 GetNumTokens() const {
    return _numTokens;
  }

  const tI64 GetIndex() const {
    return _numTokens - 1;
  }

  cString& GetCurrentToken() {
    return _token;
  }

  const cString& GetValue() const {
    return _token;
  }

  tBool GetNext() {
    if (!_pIterator || *_pIterator == 0)
      return eFalse;

    ++_numTokens;
    _token.clear();
    if (_queuedChar) {
      _token.appendChar(_queuedChar);
      _queuedChar = 0;
      return eTrue;
    }

    do {
      const tU32 c = StrGetNextX(&_pIterator);
      if (!c) {
        _pIterator = nullptr;
        break;
      }

      switch (_tokenizer.GetCharType(c)) {
        case eStringTokenizerCharType_Normal: {
          _token.appendChar(c);
          break;
        }
        case eStringTokenizerCharType_Skip: {
          break;
        }
        case eStringTokenizerCharType_SplitterStart: {
          if (_token.IsNotEmpty()) {
            return eTrue;
          }
          _token.appendChar(c);
          break;
        }
        case eStringTokenizerCharType_SplitterEnd: {
          _token.appendChar(c);
          return eTrue;
        }
        case eStringTokenizerCharType_Splitter: {
          if (_token.IsNotEmpty()) {
            return eTrue;
          }
          _token.clear();
          break;
        }
        case eStringTokenizerCharType_SplitterAndToken: {
          if (_token.IsNotEmpty()) {
            _queuedChar = c;
            return eTrue;
          }
          else {
            _token.appendChar(c);
            return eTrue;
          }
          break;
        }
        case eStringTokenizerCharType_ForceDWORD: {
          break;
        }
      }
    } while (1);

    if (_token.IsNotEmpty()) {
      return eTrue;
    }
    else {
      // because we inc the number of tokens at the begining
      --_numTokens;
      return eFalse;
    }
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __STRINGTOKENIZERIMPL_30700010_H__
