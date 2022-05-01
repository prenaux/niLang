// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("assert.ni")

::closure <- {
  _sig = "`closure"
  _sigThisCallToFirstParam = "`thisCallToFirstParam"

  function new(aThis,aFunc) {
    if (!aThis)
      return aFunc
    if (aFunc.SafeGetFreeVar(0) == _sig) {
      aFunc = aFunc.GetFreeVar(2)
      ::assertEquals("function",typeof aFunc)
    }
    local paramCount = aFunc.GetNumParams()
    switch (paramCount) {
      case 0: return #() : (_sig,aThis,aFunc) => aFunc.call(aThis)
      case 1: return #(a0) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0)
      case 2: return #(a0,a1) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0,a1)
      case 3: return #(a0,a1,a2) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0,a1,a2)
      case 4: return #(a0,a1,a2,a3) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0,a1,a2,a3)
      case 5: return #(a0,a1,a2,a3,a4) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0,a1,a2,a3,a4)
      case 6: return #(a0,a1,a2,a3,a4,a5) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0,a1,a2,a3,a4,a5)
      case 7: return #(a0,a1,a2,a3,a4,a5,a6) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0,a1,a2,a3,a4,a5,a6)
      case 8: return #(a0,a1,a2,a3,a4,a5,a6,a7) : (_sig,aThis,aFunc) => aFunc.call(aThis,a0,a1,a2,a3,a4,a5,a6,a7)
      default: throw "Invalid number of parameters '" + paramCount + "'."
    }
  }

  function thisAsFirstParm(aThis,aFunc) {
    if (!aThis)
      return aFunc
    if (aFunc.SafeGetFreeVar(0) == _sig) {
      aFunc = aFunc.GetFreeVar(2)
      ::assertEquals("function",typeof aFunc)
    }
    local paramCount = aFunc.GetNumParams()
    switch (paramCount) {
      case 0: return #() : (_sig,aThis,aFunc) => aFunc.call(aThis,this)
      case 1: return #(a0) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0)
      case 2: return #(a0,a1) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0,a1)
      case 3: return #(a0,a1,a2) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0,a1,a2)
      case 4: return #(a0,a1,a2,a3) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0,a1,a2,a3)
      case 5: return #(a0,a1,a2,a3,a4) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0,a1,a2,a3,a4)
      case 6: return #(a0,a1,a2,a3,a4,a5) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0,a1,a2,a3,a4,a5)
      case 7: return #(a0,a1,a2,a3,a4,a5,a6) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0,a1,a2,a3,a4,a5,a6)
      case 8: return #(a0,a1,a2,a3,a4,a5,a6,a7) : (_sig,aThis,aFunc) => aFunc.call(aThis,this,a0,a1,a2,a3,a4,a5,a6,a7)
      default: throw "Invalid number of parameters '" + paramCount + "'."
    }
  }

  // Creates a member function which passes the this pointer as the first parameter of the function specified.
  function thisCallToFirstParam(aFunc) {
    if (aFunc.SafeGetFreeVar(0) == _sigThisCallToFirstParam) {
      return aFunc
    }
    local paramCount = aFunc.GetNumParams()
    switch (paramCount) {
      case 0: return #() : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this)
      case 1: return #(a0) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0)
      case 2: return #(a0,a1) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0,a1)
      case 3: return #(a0,a1,a2) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0,a1,a2)
      case 4: return #(a0,a1,a2,a3) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0,a1,a2,a3)
      case 5: return #(a0,a1,a2,a3,a4) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0,a1,a2,a3,a4)
      case 6: return #(a0,a1,a2,a3,a4,a5) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0,a1,a2,a3,a4,a5)
      case 7: return #(a0,a1,a2,a3,a4,a5,a6) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0,a1,a2,a3,a4,a5,a6)
      case 8: return #(a0,a1,a2,a3,a4,a5,a6,a7) : (_sigThisCallToFirstParam,aFunc) => aFunc.call(this,this,a0,a1,a2,a3,a4,a5,a6,a7)
      default: throw "Invalid number of parameters '" + paramCount + "'."
    }
  }

  function isClosure(aFunc) {
    return aFunc.SafeGetFreeVar(0) == _sig
  }
}
