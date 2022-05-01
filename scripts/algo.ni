// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("closure.ni")

::algo <- {
  ///////////////////////////////////////////////
  function array(_this) switch (typeof _this) {
    case "array": {
      return _this;
    }
    default: {
      return [_this];
    }
  }

  ///////////////////////////////////////////////
  function unboxArray(_this) {
    if (typeof(_this) == "array" && _this.len() == 1) {
      return _this[0]
    }
    else {
      return _this
    }
  }

  ///////////////////////////////////////////////
  function joinArray(_args_) {
    local a = [];
    foreach (arg in _args_) {
      if (typeof arg == "array")
        a.extend(arg);
      else
        a.push(arg)
    }
    return a;
  }

  ///////////////////////////////////////////////
  function first(_this,aProperty) {
    foreach (k,v in _this) {
      return aProperty ? v[aProperty] : v
    }
    return null
  }

  ///////////////////////////////////////////////
  function last(_this,aProperty) {
    local r
    if (aProperty) {
      foreach (k,v in _this) {
        r = v[aProperty]
      }
    }
    else {
      foreach (k,v in _this) {
        r = v
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function all(_this,fun) {
    foreach (k,v in _this) {
      if (!fun(v,k))
        return false
    }
    return true
  }

  ///////////////////////////////////////////////
  function any(_this,fun) {
    foreach (k,v in _this) {
      if (fun(v,k))
        return true
    }
    return false
  }

  ///////////////////////////////////////////////
  function has(_this,aValue) {
    foreach (k,v in _this) {
      if (v == aValue)
        return true
    }
    return false
  }

  ///////////////////////////////////////////////
  function hasnt(_this,aValue) {
    foreach (k,v in _this) {
      if (v != aValue)
        return true
    }
    return false
  }

  ///////////////////////////////////////////////
  function fold(_this,aAcc0,fun) {
    foreach (k,v in _this) {
      aAcc0 = fun(aAcc0,v,k)
    }
    return aAcc0
  }

  ///////////////////////////////////////////////
  function scan(_this,aAcc0,fun,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      aAcc0 = fun(aAcc0,v,k)
      r.Add(aAcc0)
    }
    return r
  }

  ///////////////////////////////////////////////
  function forEach(_this,fun) {
    foreach (k,v in _this) {
      fun(v,k)
    }
    return _this
  }

  ///////////////////////////////////////////////
  function map(_this,fun,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      r.Add(fun(v,k,r))
    }
    return r
  }

  ///////////////////////////////////////////////
  function mapValid(_this,fun,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      local t = fun(v,k,r)
      if (t)
        r.Add(t)
    }
    return r
  }

  ///////////////////////////////////////////////
  function none(_this,fun) {
    foreach (k,v in _this) {
      if (fun(v,k))
        return false
    }
    return true
  }

  ///////////////////////////////////////////////
  function drop(_this,aPred,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      if (!aPred(v,k,r)) {
        r.Add(v)
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function dropN(_this,aN,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    local c = 0
    foreach (k,v in _this) {
      if (c++ < aN) {
        // drop...
      }
      else {
        r.Add(v)
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function max(_this,aProperty) {
    if (_this.empty())
      return null
    local m = ::algo.first(_this,aProperty)
    if (aProperty) {
      foreach (k,o in _this) {
        local v = o[aProperty]
        if (v > m)
          m = v
      }
    }
    else {
      foreach (k,v in _this) {
        if (v > m)
          m = v
      }
    }
    return m
  }

  ///////////////////////////////////////////////
  function min(_this,aProperty) {
    if (_this.empty())
      return null
    local m = ::algo.first(_this,aProperty)
    if (aProperty) {
      foreach (k,o in _this) {
        local v = o[aProperty]
        if (v < m)
          m = v
      }
    }
    else {
      foreach (k,v in _this) {
        if (v < m)
          m = v
      }
    }
    return m
  }

  ///////////////////////////////////////////////
  function partition(_this,aPred,aTrueArray,aFalseArray) {
    local a = (aTrueArray ? aTrueArray : [])
    local b = (aFalseArray ? aFalseArray : [])
    foreach (k,v in _this) {
      if (aPred(v,k)) {
        a.Add(v)
      }
      else {
        b.Add(v)
      }
    }
    return [a,b]
  }

  ///////////////////////////////////////////////
  function sum(_this,aProperty) {
    if (_this.empty())
      return null
    local z = ::algo.first(_this,aProperty)
    local s = z
    if (aProperty) {
      foreach (k,v in _this) {
        s += v[aProperty]
      }
    }
    else {
      foreach (k,v in _this) {
        s += v
      }
    }
    s -= z
    return s
  }

  ///////////////////////////////////////////////
  function prod(_this,aProperty) {
    if (_this.empty())
      return null
    local s = 1
    if (aProperty) {
      foreach (k,v in _this) {
        s *= v[aProperty]
      }
    }
    else {
      foreach (k,v in _this) {
        s *= v
      }
    }
    return s
  }

  ///////////////////////////////////////////////
  function take(_this,aPred,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      if (aPred(v,k,r)) {
        r.Add(v)
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function takeN(_this,aN,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    local c = 0
    foreach (k,v in _this) {
      if (c++ >= aN)
        break;
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function keys(_this,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      r.Add(k)
    }
    return r
  }

  ///////////////////////////////////////////////
  function values(_this,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function keysValues(_this,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in _this) {
      r.Add(k)
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function findKey(_this,aPred) {
    if (typeof aPred == "function") {
      foreach (k,v in _this) {
        if (aPred(v,k))
          return k
      }
    }
    else {
      foreach (k,v in _this) {
        if (v == aPred)
          return k
      }
    }
    return null
  }
}
::algo.every <- ::algo.all
::algo.some <- ::algo.any
::algo.reduce <- ::algo.fold
::algo.filter <- ::algo.take
::algo.contains <- ::algo.has

function ::algo::_getAsDelegate() {
  if ("_delegate" in this)
    return this._delegate
  local d = {}
  foreach (k,f in ::algo) {
    if (k[0] == '_') continue
    d[k] <- ::closure.thisAsFirstParm(::algo,f)
  }
  this._delegate <- d
  return this._delegate
}
