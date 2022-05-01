// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("oo.ni")

::tToString  <- ::delegate(::oo.base, {
  function _initialize() {
    _gap <- ""
    _indent <- ""
    _rep <- null
    _isIdentifier <- null
    _arrayOpen <- "["
    _arrayClose <- "]"
    _arraySeparator <- ","
    _tableOpen <- "{"
    _tableClose <- "}"
    _tableKeyValSep <- " = "
    _tableSeparator <- ","
    _leftCol <- 0
    _newLineColumn <- 80
    _recursiveGuard <- {}
    _throwRecursive <- true
    _result <- ""

    function stringify(aValue,aSpace) {
      if (typeof aSpace == "int") {
        foreach (i in aSpace) {
          _indent += " "
        }
      }
      else if (typeof aSpace == "string") {
        _indent = aSpace
      }
      return str("_", { _ = aValue })
    }

    function indented(aValue) {
      return stringify(aValue,2)
    }
  }

  function quote(aStr) {
    local theStr = aStr
    if (theStr.contains("\"") ||
        theStr.contains("\\") ||
        theStr.contains("\n"))
    {
      return "{[" + theStr + "]}"
    }
    else {
      return "\"" + theStr + "\""
    }
  }

  function joinStr(aPartial,aMind,aOpen,aClose,aSep) {
    local r
    if (aPartial.len() == 0) {
      return aOpen + aClose
    }

    if (_gap.len() && !_newLineColumn) {
      return aOpen + "\n" + _gap +
        aPartial.join(aSep + "\n" + _gap) +
        "\n" + aMind + aClose
    }

    local flatJoin = aOpen + aPartial.join(aSep) + aClose
    if (_gap.len() && ((_leftCol + flatJoin.len()) > _newLineColumn)) {
      return aOpen + "\n" + _gap +
        aPartial.join(aSep + "\n" + _gap) +
        "\n" + aMind + aClose
    }

    return flatJoin
  }

  // Produce a string from aHolder[aKey]
  function str(aKey, aHolder) {
    local mind = _gap
    local value = aHolder[aKey]

    // if there's a replacer function call it to get the replacement value
    if (_rep) {
      value = _rep(aHolder, aKey, value)
    }

    switch (typeof value) {
      case "string": {
        return (_isIdentifier && _isIdentifier(value)) ? value : quote(value);
      }
      case "int":
      case "float": {
        return value.tostring();
      }
      case "array": {
        local arrayKey = value.ToIntPtr()
        if (arrayKey in _recursiveGuard) {
          local str = "{ "+aKey+" = RECURSIVE ARRAY ENTRY: " + arrayKey + " }"
          // ::println(str)
          if (_throwRecursive)
            throw str
          else
            return str
        }
        _recursiveGuard[arrayKey] <- true

        _gap += _indent
        local arrayWasLeftCol = _leftCol
        _leftCol = _gap.len()
        // hold the partial results of stringifying
        local partial = [];
        // stringify the array values
        foreach (i,el in value) {
          partial.append(str(i,value))
        }
        // join all the elements
        local r = joinStr(partial,mind,_arrayOpen,_arrayClose,_arraySeparator)
        _leftCol = arrayWasLeftCol;
        _gap = mind;

        delete _recursiveGuard[arrayKey]
        return r;
      }
      case "table": {
        local tableKey = value.ToIntPtr()
        if (tableKey in _recursiveGuard) {
          local str = "{ "+aKey+" = RECURSIVE TABLE ENTRY: " + tableKey + " }"
          // ::println(str)
          if (_throwRecursive)
            throw str
          else
            return str
        }
        _recursiveGuard[tableKey] <- true

        _gap += _indent
        local tableWasLeftCol = _leftCol
        _leftCol = _gap.len()
        // hold the partial results of stringifying
        local partial = [];
        foreach (k,el in value) {
          local keyStr = "" + k + _tableKeyValSep;
          local keyWasLeftCol = _leftCol
          _leftCol += keyStr.len()
          local vstr = str(k,value)
          partial.append(keyStr + vstr)
          _leftCol = keyWasLeftCol
        }
        partial.sort()
        // join all the elements
        local r = joinStr(partial,mind,_tableOpen,_tableClose,_tableSeparator)
        _leftCol = tableWasLeftCol;
        _gap = mind;

        delete _recursiveGuard[tableKey]
        return r;
      }
      default: {
        return ::lang.toString(value)
      }
    }
  }
});
