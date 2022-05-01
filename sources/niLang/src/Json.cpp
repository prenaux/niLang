#include "Json.h"

#if niMinFeatures(20)
#include "API/niLang/IDataTable.h"
#include "API/niLang/IFile.h"
#include "API/niLang/Utils/UnknownImpl.h"

//======================================================================
// Json Core API
//======================================================================
namespace ni {

struct sJsonParserFile : public ni::sJsonParser {
  ni::Ptr<ni::iJsonParserSink> _sink;
  ni::Ptr<ni::iFile> _fp;
  ni::tU32 _line;

  sJsonParserFile(ni::iFile* apFile, ni::iJsonParserSink* apSink) {
    _line = 0;
    _fp = apFile;
    _sink = apSink;
  }
  ~sJsonParserFile() {
  }

  virtual tBool __stdcall OnParseBegin() {
    if (!_fp.IsOK())
      return eFalse;
    return eTrue;
  }
  virtual tBool __stdcall OnReadLine(cString& astrOut) {
    if (_fp->GetPartialRead())
      return eFalse; // EOF
    astrOut = _fp->ReadStringLine();
    ++_line;
    return eTrue;
  }
  virtual void __stdcall OnError(const achar* aaszReason) {
    if (_sink.IsOK()) {
      _sink->OnJsonParserSink_Error(aaszReason, _line, 0);
    }
  }
  virtual void __stdcall OnValue(eJsonType aType, const achar* aValue) {
    if (_sink.IsOK()) {
      _sink->OnJsonParserSink_Value(aType, aValue);
    }
  }
};

niExportFunc(ni::tBool) JsonParseFile(ni::iFile* apFile, ni::iJsonParserSink* apSink) {
  niCheckIsOK(apFile,eFalse);
  sJsonParserFile parser(apFile,apSink);
  return parser.Parse();
}

niExportFunc(ni::tBool) JsonParseString(const ni::cString& aString, ni::iJsonParserSink* apSink) {
  ni::Ptr<iFile> fp = ni::CreateFileMemory((tPtr)aString.Chars(),aString.size(),eFalse,NULL);
  return JsonParseFile(fp,apSink);
}

struct sJsonWriterSink : public cIUnknownImpl<sJsonWriter> {
  tBool _prettyPrint;
  tU32 _indent;
  Ptr<iJsonWriterSink> _sink;

  sJsonWriterSink(iJsonWriterSink* apSink, tBool abPrettyPrint) {
    _sink = apSink;
    _prettyPrint = abPrettyPrint;
    _indent = 0;
  }
  ~sJsonWriterSink() {
  }
  virtual void __stdcall OnError(const achar* aaszReason) {
    if (_sink.IsOK()) {
      _sink->OnJsonWriterSink_Error(aaszReason);
    }
  }
  virtual tBool __stdcall OnWrite(eJsonType aType, const achar* aaszString) {
    if (_sink.IsOK()) {
      _sink->OnJsonWriterSink_Write(aType, aaszString);
    }
    if (_prettyPrint) {
      tI32 indent = 0;
      if (aType == eJsonType_ObjectBegin || aType == eJsonType_ArrayBegin) {
        indent = +1;
      }
      else if (aType == eJsonType_ObjectEnd || aType == eJsonType_ArrayEnd) {
        indent = -1;
      }
      _indent += indent;
      if (aType == eJsonType_Syntax && *aaszString == ',') {
        cString strIndent = "\n";
        niLoop(i,_indent) {
          strIndent.appendChar(' ');
        }
        _sink->OnJsonWriterSink_Write(eJsonType_Syntax,strIndent.Chars());
      }
    }
    return eTrue;
  }
};

struct sJsonWriterFile : public cIUnknownImpl<sJsonWriter> {
  tBool _prettyPrint;
  tU32 _indent;
  Ptr<iFile> _fp;
  eJsonType _prevType = eJsonType_Syntax;

  sJsonWriterFile(iFile* apFile, tBool abPrettyPrint) {
    _fp = apFile;
    _prettyPrint = abPrettyPrint;
    _indent = 0;
  }
  ~sJsonWriterFile() {
  }
  virtual void __stdcall OnError(const achar* aaszReason) {
    niError(niFmt(_A("Json Error: %s\n"),aaszReason));
  }
  virtual tBool __stdcall OnWrite(eJsonType aType, const achar* aaszString) {
    if (_prettyPrint && _indent) {
      if (aType == eJsonType_ObjectEnd || aType == eJsonType_ArrayEnd) {
        if (!(aType == eJsonType_ObjectEnd && _prevType == eJsonType_ObjectBegin) &&
            !(aType == eJsonType_ArrayEnd && _prevType == eJsonType_ArrayBegin))
        {
          _fp->WriteString("\n");
          niLoop(i,_indent-1) {
            _fp->Write8(' ');
          }
        }
      }
      else if (_prevType == eJsonType_ObjectBegin ||
               _prevType == eJsonType_ArrayBegin) {
        _fp->WriteString("\n");
        niLoop(i,_indent) {
          _fp->Write8(' ');
        }
      }
    }

    _fp->WriteString(aaszString);
    if (_prettyPrint) {
      tI32 indent = 0;
      if (aType == eJsonType_ObjectBegin || aType == eJsonType_ArrayBegin) {
        indent = +1;
      }
      else if (aType == eJsonType_ObjectEnd || aType == eJsonType_ArrayEnd) {
        indent = -1;
      }
      _indent += indent;
      if (aType == eJsonType_Syntax && *aaszString == ',') {
        _fp->WriteString("\n");
        niLoop(i,_indent) {
          _fp->Write8(' ');
        }
      }
      _prevType = aType;
    }
    return eTrue;
  }
};

niExportFunc(ni::iJsonWriter*) CreateJsonSinkWriter(ni::iJsonWriterSink* apSink, ni::tBool abPrettyPrint) {
  niCheckIsOK(apSink,NULL);
  return niNew sJsonWriterSink(apSink,abPrettyPrint);
}

niExportFunc(ni::iJsonWriter*) CreateJsonFileWriter(ni::iFile* apFile, ni::tBool abPrettyPrint) {
  niCheckIsOK(apFile,NULL);
  return niNew sJsonWriterFile(apFile,abPrettyPrint);
}

}

//======================================================================
// Json parser
//======================================================================
namespace ni {

sJsonParser::sJsonParser() {
  _ParseReset();
}
sJsonParser::~sJsonParser() {
}

void sJsonParser::_ParseReset() {
  _line = AZEROSTR;
  _lineCount = 0;
  _p = NULL;
  _tok = 0;
}

tBool sJsonParser::Parse() {
  _ParseReset();
  if (!OnParseBegin()) {
    OnError(_A("ParseBegin failed."));
    return eFalse;
  }

  while (_NextToken()) {
    if (_TokenIsSpace()) {
      // skip 'spaces' chars (' ',\n,\r,and unicode variants)
    }
    else {
      switch (_tok) {
        // Objects {}
        case '{': {
          if (!_ParseObject())
            return eFalse;
          break;
        }
          // Array []
        case '[': {
          if (!_ParseArray())
            return eFalse;
          break;
        }
          // Other
        default:
          OnError(niFmt(_A("Invalid character '%c' (%d), should be { or [."),
                        _tok,_tok));
          return eFalse;
      }
    }
  };

  return eTrue;
}

tBool sJsonParser::_ReadLine() {
  while (1) {
    if (!OnReadLine(_line)) {
      _p = NULL;
      _tok = 0;
      return eFalse;
    }
    _p = _line.c_str();
    _tok = StrGetNext(_p);
    _lineCount++;
    if (_tok)
      break;
  }
  return eTrue;
}
tBool sJsonParser::_NextToken() {
  if (!_p) {
    return _ReadLine();
  }
  else {
    StrGetNextX(&_p);
    _tok = StrGetNext(_p);
    if (!_tok) {
      return _ReadLine();
    }
  }
  return eTrue;
}
tBool sJsonParser::_TokenIsSpace() const {
  return !!StrIsSpace(_tok);
}
tBool sJsonParser::_TokenIsNumberDigit() const {
  return (_tok >= '0' && _tok <= '9');
}
tBool sJsonParser::_TokenIsNumberDot() const {
  return (_tok == '.');
}
tBool sJsonParser::_TokenIsNumberSign() const {
  return (_tok == '+') || (_tok == '-');
}
tBool sJsonParser::_TokenIsNumberExp() const {
  return (_tok == 'e') || (_tok == 'E');
}
tBool sJsonParser::_TokenIsNumberBegining() const {
  return
      _TokenIsNumberSign() ||
      _TokenIsNumberDigit() ||
      _TokenIsNumberDot();
}
tBool sJsonParser::_TokenIsNumberPart() const {
  return
      _TokenIsNumberSign() ||
      _TokenIsNumberDigit() ||
      _TokenIsNumberExp() ||
      _TokenIsNumberDot();
}
tBool sJsonParser::_TokenIsEndOfValue() const {
  return
      _tok == ']' ||
      _tok == ',' ||
      _tok == '}';
}
tBool sJsonParser::_SkipSpaces() {
  while (_TokenIsSpace()) {
    if (!_NextToken())
      return eFalse;
  }
  return eTrue;
}

tBool sJsonParser::_TokenToChar(tU32& c) {
  if (_tok == '\\') {
    if (!_NextToken()) {
      OnError(_A("Unexpected end of stream when parsing escaped char"));
      return eFalse;
    }
    switch (_tok) {
      case '"':  c = '"'; break;
      case '\\': c = '\\'; break;
      case '/':  c = '/'; break;
      case 'b': c = '\b'; break;
      case 'f': c = '\f'; break;
      case 'n': c = '\n'; break;
      case 'r': c = '\r'; break;
      case 't': c = '\t'; break;
      case 'u': {
        achar buffer[64];
        achar* p = buffer;
        niLoop(i,4) {
          if (!_NextToken()) {
            OnError(niFmt("Unexpected end of stream when parsing unicode char [%d].",i));
            return eFalse;
          }
          StrSetCharX(&p,_tok);
        }
        *p = 0;
        const achar* endp;
        c = (tU32)StrToUL(buffer,&endp,16);
        break;
      }
      default:
        OnError(niFmt(_A("Invalid escaped char '%c' (%d)."),_tok,_tok));
        return eFalse;
    }
  }
  else {
    c = _tok;
  }
  return eTrue;
}

void sJsonParser::_ResetBuffer(const tSize aReserve) {
  _buf.clear(); _buf.reserve(aReserve);
}
void sJsonParser::_BufferAppend(tU32 c) {
  _buf.appendChar(c);
}
void sJsonParser::_OnValueBuffer(eJsonType aType) {
  _buf.push_back(0);
  OnValue(aType,_buf.Chars());
}

tBool sJsonParser::_ParseNumber() {
  if (!_SkipSpaces()) {
    OnError(_A("Unexpected end of stream when parsing number."));
    return eFalse;
  }
  if (!_TokenIsNumberBegining()) {
    OnError(_A("Expeced '+', '-', '.' or digit to begin number."));
    return eFalse;
  }

  _ResetBuffer();
  _BufferAppend(_tok); // append the number's first token
  while (1) {
    if (!_NextToken()) {
      OnError(_A("Unexpected end of stream when parsing number."));
      return eFalse;
    }
    if (_TokenIsSpace())
      continue; // skip
    if (_TokenIsEndOfValue())
      break; // done
    if (!_TokenIsNumberPart()) {
      OnError(_A("Expected number part token parsing number."));
      return eFalse;
    }
    _BufferAppend(_tok);
  }
  _OnValueBuffer(eJsonType_Number);
  return eTrue;
}
tBool sJsonParser::_ParseString(eJsonType aType) {
  if (!_SkipSpaces()) {
    OnError(_A("Unexpected end of stream when parsing string."));
    return eFalse;
  }
  if (_tok != '"') {
    OnError(_A("Expeced '\"' to begin string."));
    return eFalse;
  }

  _ResetBuffer();
  while (1) {
    if (!_NextToken()) {
      OnError(_A("Unexpected end of stream when parsing string."));
      return eFalse;
    }
    if (_tok == '"') {
      // end of string
      _OnValueBuffer(aType);
      _NextToken(); // skip the '"'
      break;
    }
    tU32 c = 0;
    if (!_TokenToChar(c)) {
      OnError(_A("Can't convert token to char when parsing string."));
      return eFalse;
    }
    _BufferAppend(c);
  }
  return eTrue;
}
tBool sJsonParser::_ParseValue() {
  if (!_SkipSpaces()) {
    OnError(_A("Unexpected end of stream when parsing value."));
    return eFalse;
  }

  if (_TokenIsNumberBegining()) {
    // number
    if (!_ParseNumber()) {
      OnError(_A("Can't parse value number."));
      return eFalse;
    }
  }
  else {
    switch (_tok) {
      // string
      case '"': {
        if (!_ParseString(eJsonType_String)) {
          OnError(_A("Can't parse value string."));
          return eFalse;
        }
        break;
      }
        // object
      case '{': {
        if (!_ParseObject()) {
          OnError(_A("Can't parse value object."));
          return eFalse;
        }
        break;
      }
        // array
      case '[': {
        if (!_ParseArray()) {
          OnError(_A("Can't parse value array."));
          return eFalse;
        }
        break;
      }
        // true/false/null
      default: {
        if (_TokenIsEndOfValue()) {
          OnError(niFmt(_A("Unexpected end of value character '%c' (%d)"),
                        _tok,_tok));
          return eFalse;
        }

        static const achar kAtomTrue[6] = {'t','r','u','e',0,0};
        static const achar kAtomFalse[6] = {'f','a','l','s','e',0};
        static const achar kAtomNull[6] = {'n','u','l','l',0,0};

        tU32 atomLen = 0;
        achar buffer[64];
        achar* p = buffer;
        niLoop(i,6) {
          atomLen += StrSetCharX(&p,_tok);
          if (!_NextToken()) {
            OnError(_A("Unexpected end of stream when parsing atom value."));
            return eFalse;
          }
          if (_TokenIsSpace() || _TokenIsEndOfValue())
            break;
        }
        *p = 0;
        if (!_TokenIsSpace() && !_TokenIsEndOfValue()) {
          OnError(niFmt(_A("Expected space or 'end of value' after atom value, got '%c' (%d)."),
                        _tok,_tok));
          return eFalse;
        }
        if (ni::MemCmp((tPtr)buffer,(tPtr)kAtomTrue,atomLen) == 0) {
          OnValue(eJsonType_True,kAtomTrue);
        }
        else if (ni::MemCmp((tPtr)buffer,(tPtr)kAtomFalse,atomLen) == 0) {
          OnValue(eJsonType_False,kAtomFalse);
        }
        else if (ni::MemCmp((tPtr)buffer,(tPtr)kAtomNull,atomLen) == 0) {
          OnValue(eJsonType_Null,kAtomNull);
        }
        else {
          OnError(niFmt(_A("Invalid atomic value type %s...."),buffer));
          return eFalse;
        }
        break;
      }
    }
  }

  return eTrue;
}
tBool sJsonParser::_ParsePair() {
  if (!_ParseString(eJsonType_Name)) {
    OnError(_A("Can't parse pair name."));
    return eFalse;
  }
  if (!_SkipSpaces()) {
    OnError(_A("Unexpected end of stream when parsing pair name separator."));
    return eFalse;
  }
  switch (_tok) {
    // End of object
    case '}':
      break;
      // Parse next pair...
    case ',':
      break;
      // Parse pair value
    case ':': {
      _NextToken(); // skip ':'
      if (!_ParseValue()) {
        OnError(_A("Can't parse pair value."));
        return eFalse;
      }
      if (!_SkipSpaces()) {
        OnError(_A("Unexpected end of stream when parsing pair separator."));
        return eFalse;
      }
      break;
    }
    default:
      OnError(niFmt(_A("Expected either end of object '}' or member or pair separator (',' or ':'), got '%c' (%d)."),_tok,_tok));
      return eFalse;
  }

  return eTrue;
}
tBool sJsonParser::_ParseObject() {
  if (!_SkipSpaces()) {
    OnError(_A("Unexpected end of stream when parsing object begining."));
    return eFalse;
  }
  if (_tok != '{') {
    OnError(_A("Expeced '{' to begin object."));
    return eFalse;
  }
  _NextToken(); // skip the '{'
  // Submit object begining
  OnValue(eJsonType_ObjectBegin,AZEROSTR);
  // Parse the object
  tU32 numPairs = 0;
  while (1) {
    if (!_SkipSpaces()) {
      OnError(_A("Unexpected end of stream while parsing object."));
      return eFalse;
    }
    if (_tok == '}') {
      OnValue(eJsonType_ObjectEnd,AZEROSTR);
      _NextToken(); // skip '}'
      return eTrue;
    }
    if (numPairs > 0) {
      if (_tok != ',') {
        OnError(niFmt(_A("Expected comma after pair [%d]."),numPairs));
        return eFalse;
      }
      _NextToken(); // skip ','
    }
    if (!_ParsePair()) {
      OnError(niFmt(_A("Can't parse pair [%d]."),numPairs));
      return eFalse;
    }
    ++numPairs;
  }
  return eTrue;
}
tBool sJsonParser::_ParseArray() {
  if (!_SkipSpaces()) {
    OnError(_A("Unexpected end of stream when parsing array begining."));
    return eFalse;
  }
  if (_tok != '[') {
    OnError(_A("Expeced '[' to begin array."));
    return eFalse;
  }
  _NextToken(); // skip the '['
  // Submit array begining
  OnValue(eJsonType_ArrayBegin,AZEROSTR);
  // Parse the array
  tU32 numValues = 0;
  while (1) {
    if (!_SkipSpaces()) {
      OnError(_A("Unexpected end of stream while parsing array."));
      return eFalse;
    }
    if (_tok == ']') {
      OnValue(eJsonType_ArrayEnd,AZEROSTR);
      _NextToken(); // skip ']'
      return eTrue;
    }
    if (numValues > 0) {
      if (_tok != ',') {
        OnError(niFmt(_A("Expected comma after value [%d]."),numValues));
        return eFalse;
      }
      _NextToken(); // skip ','
    }
    if (!_ParseValue()) {
      OnError(niFmt(_A("Can't parse value [%d]."),numValues));
      return eFalse;
    }
    ++numValues;
  }
  return eTrue;
}

//======================================================================
// Json writer
//======================================================================
sJsonWriter::sJsonWriter() {
  Reset();
}
sJsonWriter::~sJsonWriter() {
}

void sJsonWriter::Reset() {
  while (!_typeStack.empty()) { _typeStack.pop(); }
  _typeStack.push(0);
}

//// Arrays /////
tBool sJsonWriter::ArrayBegin() {
  if (_TypeIsName()) {
    _WriteColon();
    _TypePop();
  }
  else if (_TypeIsValue()) {
    _WriteComma();
    _TypePop();
  }
  _WriteValue(eJsonType_ArrayBegin,NULL);
  _TypePush('[');
  return eTrue;
}
tBool sJsonWriter::ArrayEnd() {
  if (_TypeIsValue()) {
    _TypePop(); // if some value added...
  }
  if (!_TypeIsArray()) {
    OnError(niFmt(_A("Expected array type, found '%c' (%d)."),
                  _Type(),_Type()));
    return eFalse;
  }
  _WriteValue(eJsonType_ArrayEnd,NULL);
  _TypePop();
  _TypePush(',');
  return eTrue;
}
tBool sJsonWriter::ArrayString(const achar* aStr) {
  return _Item(eJsonType_String,aStr);
}
tBool sJsonWriter::ArrayNumber(const achar* aStr) {
  return _Item(eJsonType_Number,aStr);
}
tBool sJsonWriter::ArrayBool(tBool abItem) {
  return _Item(abItem?eJsonType_True:eJsonType_False,NULL);
}
tBool sJsonWriter::ArrayNull() {
  return _Item(eJsonType_Null,NULL);
}

//// Objects /////
tBool sJsonWriter::ObjectBegin() {
  if (_TypeIsName()) {
    _WriteColon();
    _TypePop();
  }
  else if (_TypeIsValue()) {
    _WriteComma();
    _TypePop();
  }
  _WriteValue(eJsonType_ObjectBegin,NULL);
  _TypePush('{');
  return eTrue;
}
tBool sJsonWriter::ObjectEnd() {
  if (_TypeIsValue()) {
    _TypePop(); // if some value added...
  }
  if (!_TypeIsObject()) {
    OnError(niFmt(_A("Expected object type, found '%c' (%d)."),
                  _Type(),_Type()));
    return eFalse;
  }
  _WriteValue(eJsonType_ObjectEnd,NULL);
  _TypePop();
  _TypePush(',');
  return eTrue;
}
tBool sJsonWriter::Name(const achar* aName) {
  if (_TypeIsPair()) {
    _WriteComma();
    _TypePop();
  }
  else if (!_TypeIsObject()) {
    OnError(niFmt(_A("Expected to be in an object type, found '%c' (%d)."),
                  _Type(),_Type()));
    return eFalse;
  }
  _TypePush(':');
  _WriteString(eJsonType_Name,aName);
  return eTrue;
}

tBool sJsonWriter::ValueString(const achar* aStr) {
  return _Value(eJsonType_String,aStr);
}
tBool sJsonWriter::ValueNumber(const achar* aStr) {
  return _Value(eJsonType_Number,aStr);
}
tBool sJsonWriter::ValueBool(tBool abValue) {
  return _Value(abValue?eJsonType_True:eJsonType_False,NULL);
}
tBool sJsonWriter::ValueNull() {
  return _Value(eJsonType_Null,NULL);
}

// Value comes after a name
tBool sJsonWriter::_Value(eJsonType aType, const achar* aValue) {
  if (!_TypeIsName()) {
    return _Item(aType,aValue);
  }
  _TypePop(); // pop the 'pair name' type
  _WriteColon();
  _TypePush(',');
  _WriteValue(aType,aValue);
  return eTrue;
}

// An item is an entry in an array
tBool sJsonWriter::_Item(eJsonType aType, const achar* aValue) {
  if (_TypeIsArray()) {
  }
  else if (_TypeIsValue()) {
    _WriteComma();
    _TypePop(); // pop the 'value' type
  }
  else {
    OnError(niFmt(_A("Expected array or value type, found '%c' (%d)."),
                  _Type(),_Type()));
    return eFalse;
  }
  if (aType >= eJsonType_ObjectBegin) {
    OnError(_A("Nested arrays and objects can't be added in a single operation."));
    return eFalse;
  }
  _TypePush(',');
  _WriteValue(aType,aValue);
  return eTrue;
}

achar sJsonWriter::_Type() const { return _typeStack.top(); }
achar sJsonWriter::_TypeIsArray() const  { return _Type() == '['; }
achar sJsonWriter::_TypeIsObject() const { return _Type() == '{'; }
achar sJsonWriter::_TypeIsName() const   { return _Type() == ':'; }
achar sJsonWriter::_TypeIsValue() const  { return _Type() == ','; }
achar sJsonWriter::_TypeIsPair() const   { return _TypeIsName() || _TypeIsValue(); }
void sJsonWriter::_TypePush(achar c) { _typeStack.push(c); }
achar sJsonWriter::_TypePop() { achar c = _Type(); _typeStack.pop(); return c; }

tBool sJsonWriter::_WriteValue(eJsonType aType, const achar* aValue) {
  static const achar _atomTrue[] = {'t','r','u','e',0};
  static const achar _atomFalse[] = {'f','a','l','s','e',0};
  static const achar _atomNull[] = {'n','u','l','l',0};
  static const achar _arrayBegin[] = {'[',0};
  static const achar _arrayEnd[] = {']',0};
  static const achar _objectBegin[] = {'{',0};
  static const achar _objectEnd[] = {'}',0};
  switch (aType) {
    case eJsonType_Name:
    case eJsonType_String:      return _WriteString(aType,aValue);
    case eJsonType_Number:      return _Write(aType,aValue);
    case eJsonType_True:        return _Write(aType,_atomTrue);
    case eJsonType_False:       return _Write(aType,_atomFalse);
    case eJsonType_Null:        return _Write(aType,_atomNull);
    case eJsonType_ObjectBegin: return _Write(aType,_objectBegin);
    case eJsonType_ObjectEnd:   return _Write(aType,_objectEnd);
    case eJsonType_ArrayBegin:  return _Write(aType,_arrayBegin);
    case eJsonType_ArrayEnd:    return _Write(aType,_arrayEnd);
    default: return eFalse;
  }
}

tBool sJsonWriter::_WriteColon() {
  static const achar _colon[] = {':',0};
  return _Write(eJsonType_Syntax,_colon);
}
tBool sJsonWriter::_WriteComma() {
  static const achar _comma[] = {',',0};
  return _Write(eJsonType_Syntax,_comma);
}
tBool sJsonWriter::_WriteString(eJsonType aType, const achar* aaszString) {
  _ResetBuffer();
  const achar* strPtr = aaszString;
  _BufferAppend('"');
  tU32 c = StrGetNextX(&strPtr);
  while (c) {
    switch (c) {
      case '"':  _BufferAppend('\\'); _BufferAppend('"'); break;
      case '\\': _BufferAppend('\\'); _BufferAppend('\\'); break;
      case '\b': _BufferAppend('\\'); _BufferAppend('b'); break;
      case '\f': _BufferAppend('\\'); _BufferAppend('f'); break;
      case '\n': _BufferAppend('\\'); _BufferAppend('n'); break;
      case '\r': _BufferAppend('\\'); _BufferAppend('r'); break;
      case '\t': _BufferAppend('\\'); _BufferAppend('t'); break;
      default: _BufferAppend(c); break;
    }
    c = StrGetNextX(&strPtr);
  }
  _BufferAppend('"');
  return _WriteBuffer(aType);
}
tBool sJsonWriter::_WriteBuffer(eJsonType aType) {
  _BufferAppend(0);
  return _Write(aType,_buf.begin());
}
tBool sJsonWriter::_Write(eJsonType aType, const achar* aaszString) {
  return OnWrite(aType, aaszString);
}

void sJsonWriter::_ResetBuffer(const tSize aReserve) {
  _buf.clear(); _buf.reserve(aReserve);
}
void sJsonWriter::_BufferAppend(tU32 c) {
  _buf.appendChar(c);
}

//======================================================================
// Json to DataTable
//======================================================================
struct _JsonParseToDataTableSink : cIUnknownImpl<iJsonParserSink,eIUnknownImplFlags_Local> {
  cString _keyName;
  iDataTable* _dt;
  _JsonParseToDataTableSink(iDataTable* apDT) : _dt(apDT) {}

  //! Called when a parsing error occured.
  virtual void __stdcall OnJsonParserSink_Error(const achar* aaszReason, tU32 anLine, tU32 anCol)
  {
    niWarning(niFmt("JsonParseToDataTableSink error:%d:%d: %s",
                    anLine, anCol, aaszReason));
  }

  //! Called when a value is parsed
  virtual void __stdcall OnJsonParserSink_Value(eJsonType aType, const achar* aValue) {
    switch (aType) {
      case eJsonType_Name: {
        _keyName = aValue;
        break;
      }
      case eJsonType_String: {
        _dt->SetString(_keyName.Chars(), aValue);
        break;
      }
      case eJsonType_Number: {
        _dt->SetFloat(_keyName.Chars(), StrAToF(aValue));
        break;
      }
      case eJsonType_True: {
        _dt->SetBool(_keyName.Chars(), eTrue);
        break;
      }
      case eJsonType_False: {
        _dt->SetBool(_keyName.Chars(), eFalse);
        break;
      }
      case eJsonType_Null: {
        _dt->SetVar(_keyName.Chars(), niVarNull);
        break;
      }
    }
  }
};

niExportFunc(ni::tBool) JsonParseFileToDataTable(ni::iFile* apFile, ni::iDataTable* apDT) {
  niCheckIsOK(apFile, ni::eFalse);
  niCheckIsOK(apDT, ni::eFalse);
  _JsonParseToDataTableSink sinkParser(apDT);
  return JsonParseFile(apFile,&sinkParser);
}

niExportFunc(ni::tBool) JsonParseStringToDataTable(const cString& aString, ni::iDataTable* apDT) {
  niCheckIsOK(apDT, ni::eFalse);
  _JsonParseToDataTableSink sinkParser(apDT);
  return JsonParseString(aString,&sinkParser);
}

}
#endif // niMinFeatures
