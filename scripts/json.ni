// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

::Import("lang.ni")
::Import("fs.ni")

//////////////////////////////////////////////////////////////////////////////////////////////
::json <- {

  _parseSink = {
    _error = null
    _cur = []
    _name = []
    _top = null
    _ret = null
    _inObject = null
    function OnJsonParserSink_Error(aaszReason, anLine, anCol) {
      _error = "JSON Parsing Error ("+anLine+"):"+anCol+": " +aaszReason
    }
    function fetchTop() {
      _top = _cur.top();
      if (typeof(_top) == "table") {
        _inObject = true
      }
      else {
        _inObject = false
      }
    }
    function pushArray() {
      _cur.push([])
      fetchTop()
    }
    function pushObject() {
      _cur.push({})
      fetchTop()
    }
    function pop() {
      if (_cur.len() == 1)
        _ret = _cur.top()
      local prevTop = _top
      _cur.pop()
      if (!_cur.empty()) {
        fetchTop()
        pushValue(prevTop)
      }
    }
    function pushValue(aValue) {
      if (_inObject) {
        _top[_name.top()] <- aValue
        _name.pop()
      }
      else {
        _top.append(aValue)
      }
    }
    function OnJsonParserSink_Value(aType, aValue) {
      // ::dbg("TYPE: " + aType + " : " + aValue)
      switch (aType) {
        case 0: { // eJsonType_Syntax
          break;
        }
        case 1: { // eJsonType_Name
          _name.push(aValue)
          break;
        }
        case 2: { // eJsonType_String
          pushValue(aValue)
          break;
        }
        case 3: { // eJsonType_Number
          local v = (aValue.Contains(".")||aValue.Contains("e"))?aValue.ToFloat():aValue.ToInt()
          pushValue(v)
          break;
        }
        case 4: { // eJsonType_True
          pushValue(true)
          break;
        }
        case 5: { // eJsonType_False
          pushValue(false)
          break;
        }
        case 6: { // eJsonType_Null
          pushValue(null)
          break;
        }
        case 7: { // eJsonType_ObjectBegin
          pushObject();
          break;
        }
        case 8: { // eJsonType_ObjectEnd
          pop();
          break;
        }
        case 9: { // eJsonType_ArrayBegin
          pushArray();
          break;
        }
        case 10: { // eJsonType_ArrayEnd
          pop()
          break;
        }
      }
    }
  }


  ///////////////////////////////////////////////
  function parseFile(fp) {
    local fp = ::fs.fileOpenRead(fp)
    local sink = ::LINT_AS_TYPE("::json._parseSink",_parseSink.Clone())
    ::gLang.JsonParseFile(fp,sink)
    if (sink._error)
      throw sink._error
    return sink._ret
  }

  ///////////////////////////////////////////////
  function parseString(str) {
    local sink = ::LINT_AS_TYPE("::json._parseSink",_parseSink.Clone())
    ::gLang.JsonParseString(str,sink)
    if (sink._error)
      throw sink._error
    return sink._ret
  }

  ///////////////////////////////////////////////
  function toFile(v,fp,aPrettyPrint) {
    if (typeof(fp) == "string") fp = ::fs.fileOpenWrite(fp)
    local writer = ::gLang.CreateJsonFileWriter(fp,aPrettyPrint)
    toWriter(v,writer)
    return fp
  }

  ///////////////////////////////////////////////
  function toString(v,_aPrettyPrint) {
    local fp = ::gLang.CreateFileDynamicMemory(128,null)
    local writer = ::gLang.CreateJsonFileWriter(fp,_aPrettyPrint)
    toWriter(v,writer)
    fp.SeekSet(0)
    return fp.ReadString()
  }

  ///////////////////////////////////////////////
  function toWriter(v,aWriter) {
    local s = ""
    switch (typeof(v)) {
      case "array": {
        aWriter.ArrayBegin()
        foreach (a in v) {
          switch (typeof(a)) {
            case "table":
            case "array": {
              toWriter(a,aWriter)
              break;
            }
            case "int":
            case "float": {
              aWriter.ValueNumber(a.ToString())
              break;
            }
            case "null": {
              aWriter.ValueNull()
              break;
            }
            default: {
              aWriter.ValueString(::lang.toString(a))
              break;
            }
          }
        }
        aWriter.ArrayEnd()
        break;
      }
      case "table": {
        aWriter.ObjectBegin()
        foreach (k,a in v) {
          switch (typeof(a)) {
            case "table":
            case "array": {
              aWriter.Name(k)
              toWriter(a,aWriter)
              break;
            }
            case "int":
            case "float": {
              aWriter.ObjectNumber(k,a.ToString());
              break;
            }
            case "null": {
              aWriter.ObjectNull(k)
              break;
            }
            default: {
              aWriter.ObjectString(k,::lang.toString(a));
              break;
            }
          }
        }
        aWriter.ObjectEnd()
        break;
      }
      default: {
        throw "Can't write "+typeof(v)+" in JSON output.";
      }
    }
    return s
  }
}
