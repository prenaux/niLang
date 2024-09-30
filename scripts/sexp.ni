// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("tostring.ni")

module <- {
  function newToString() ::tToString {
    local s = ::tToString.new()
    s._isIdentifier = #(v) => v[?0] != '`'
    s._arrayOpen = "("
    s._arrayClose = ")"
    s._arraySeparator = " "
    return s
  }

  function toString(v,_aSpace) {
    return newToString().stringify(v,_aSpace)
  }
  function toIndentedString(v) {
    return newToString().indented(v)
  }

  function elementToSymbols(e) {
    switch (typeof(e)) {
      case "string": {
        return e.parsesymbol()
      }
      case "array": {
        return arrayToSymbols(e)
      }
      default: {
        return e
      }
    }
  }

  function arrayToSymbols(S) {
    local O = []
    foreach (e in S) {
      O.push(elementToSymbols(e))
    }
    return O
  }

  function toSymbols(S) {
    if (typeof(S) == "array")
      return arrayToSymbols(S)
    return elementToSymbols(S)
  }

  function toSymbolsString(S) {
    return toString(toSymbols(S), null)
  }
}

module.utils <- {
  function getAfter(sexp,k,aDefault) {
    foreach (i,e in sexp) {
      if (e == k) {
        return sexp[?(i+1)]
      }
    }
    if (aDefault)
      return aDefault;
    throw "Can't get key '"+k+"'."
  }
  function getValue(sexp,k,aDefault) {
    foreach (e in sexp) {
      if (e[0] == k) {
        return e[?1]
      }
    }
    if (aDefault)
      return aDefault;
    throw "Can't get key '"+k+"'."
  }
  function getValues(sexp,k,aDefault) {
    foreach (e in sexp) {
      if (e[0] == k) {
        if (e.GetSize() > 1) {
          return e.slice(1)
        }
        return null
      }
    }
    if (aDefault)
      return aDefault;
    throw "Can't get key '"+k+"'."
  }
  function cleanString(v) {
    if (v[0] == '`')
      return v.slice(1)
    return v
  }
}

::LINT_CHECK_TYPE("null", ::?seq);
::LINT_CHECK_TYPE("null", ::?seq.?utils);
::sexp <- module
