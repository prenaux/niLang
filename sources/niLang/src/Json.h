#ifndef __JSON_H_FE9D777A_8A76_40EF_9D2E_1516F3649A38__
#define __JSON_H_FE9D777A_8A76_40EF_9D2E_1516F3649A38__
#include "API/niLang/Types.h"

#if niMinFeatures(20)
#include "API/niLang/StringDef.h"
#include "API/niLang/StringLibIt.h"
#include "API/niLang/IJson.h"
#include "API/niLang/STL/vector.h"
#include "API/niLang/STL/stack.h"

namespace ni {

//======================================================================
// Json parser
//======================================================================

struct sJsonParser {
  cString _line;
  tU32    _lineCount;
  const achar* _p;
  tU32    _tok;
  cString _buf;

  sJsonParser();
  ~sJsonParser();

  void _ParseReset();

  tBool Parse();

  tBool _ReadLine();
  tBool _NextToken();
  tBool _TokenIsSpace() const;
  tBool _TokenIsNumberDigit() const;
  tBool _TokenIsNumberDot() const;
  tBool _TokenIsNumberSign() const;
  tBool _TokenIsNumberExp() const;
  tBool _TokenIsNumberBegining() const;
  tBool _TokenIsNumberPart() const;
  tBool _TokenIsEndOfValue() const;
  tBool _SkipSpaces();

  tBool _TokenToChar(tU32& c);

  void _ResetBuffer(const tSize aReserve = 64);
  void _BufferAppend(tU32 c);
  void _OnValueBuffer(eJsonType aType);

  tBool _ParseNumber();
  tBool _ParseString(eJsonType aType);
  tBool _ParseValue();
  tBool _ParsePair();
  tBool _ParseObject();
  tBool _ParseArray();

  virtual tBool __stdcall OnParseBegin() = 0;
  virtual tBool __stdcall OnReadLine(cString& astrOut) = 0;
  virtual void __stdcall OnError(const achar* aaszReason) = 0;
  virtual void __stdcall OnValue(eJsonType aType, const achar* aValue) = 0;
};

struct sJsonWriter : public iJsonWriter {
  cString _buf;
  astl::stack<achar> _typeStack;

  sJsonWriter();
  ~sJsonWriter();

  void __stdcall Reset();

  //// Arrays /////
  tBool __stdcall ArrayBegin();
  tBool __stdcall ArrayEnd();
  tBool __stdcall ArrayString(const achar* aStr);
  tBool __stdcall ArrayNumber(const achar* aStr);
  tBool __stdcall ArrayBool(tBool abItem);
  tBool __stdcall ArrayNull();

  //// Objects /////
  tBool __stdcall ObjectBegin();
  tBool __stdcall ObjectEnd();

  tBool __stdcall ObjectString(const achar* aName, const achar* aStr) {
    if (!Name(aName)) return eFalse;
    if (!ValueString(aStr)) return eFalse;
    return eTrue;
  }
  tBool __stdcall ObjectNumber(const achar* aName, const achar* aStr) {
    if (!Name(aName)) return eFalse;
    if (!ValueNumber(aStr)) return eFalse;
    return eTrue;
  }
  tBool __stdcall ObjectBool(const achar* aName, tBool abValue) {
    if (!Name(aName)) return eFalse;
    if (!ValueBool(abValue)) return eFalse;
    return eTrue;
  }
  tBool __stdcall ObjectNull(const achar* aName) {
    if (!Name(aName)) return eFalse;
    if (!ValueNull()) return eFalse;
    return eTrue;
  }

  tBool __stdcall Key(const achar* aName) {
    return Name(aName);
  }

  tBool __stdcall Name(const achar* aName);
  tBool __stdcall ValueString(const achar* aStr);
  tBool __stdcall ValueNumber(const achar* aStr);
  tBool __stdcall ValueBool(tBool abValue);
  tBool __stdcall ValueNull();

  // Value comes after a name
  tBool _Value(eJsonType aType, const achar* aValue);

  // An item is an entry in an array
  tBool _Item(eJsonType aType, const achar* aValue);

  achar _Type() const;
  achar _TypeIsArray() const;
  achar _TypeIsObject() const;
  achar _TypeIsName() const;
  achar _TypeIsValue() const;
  achar _TypeIsPair() const;
  void _TypePush(achar c);
  achar _TypePop();

  tBool _WriteValue(eJsonType aType, const achar* aValue);

  tBool _WriteColon();
  tBool _WriteComma();
  tBool _WriteString(eJsonType aType, const achar* aaszString);
  tBool _WriteBuffer(eJsonType aType);
  tBool _Write(eJsonType aType, const achar* aaszString);

  void _ResetBuffer(const tSize aReserve = 64);
  void _BufferAppend(tU32 c);

  virtual void __stdcall OnError(const achar* aaszReason) = 0;
  virtual tBool __stdcall OnWrite(eJsonType aValue, const achar* aaszString) = 0;
};

}

#endif // niMinFeatures(20)
#endif // __JSON_H_FE9D777A_8A76_40EF_9D2E_1516F3649A38__
