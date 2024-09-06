// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("closure.ni")

::algo <- {
  ///////////////////////////////////////////////
  function array(that) switch (typeof that) {
    case "array": {
      return that;
    }
    default: {
      return [that];
    }
  }

  ///////////////////////////////////////////////
  function unboxArray(that) {
    if (typeof(that) == "array" && that.len() == 1) {
      return that[0]
    }
    else {
      return that
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
  function first(that,aProperty) {
    foreach (k,v in that) {
      return aProperty ? v[aProperty] : v
    }
    return null
  }

  ///////////////////////////////////////////////
  function last(that,aProperty) {
    local r
    if (aProperty) {
      foreach (k,v in that) {
        r = v[aProperty]
      }
    }
    else {
      foreach (k,v in that) {
        r = v
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function all(that,fun) {
    foreach (k,v in that) {
      if (!fun(v,k))
        return false
    }
    return true
  }

  ///////////////////////////////////////////////
  function any(that,fun) {
    foreach (k,v in that) {
      if (fun(v,k))
        return true
    }
    return false
  }

  ///////////////////////////////////////////////
  function has(that,aValue) {
    foreach (k,v in that) {
      if (v == aValue)
        return true
    }
    return false
  }

  ///////////////////////////////////////////////
  function hasnt(that,aValue) {
    foreach (k,v in that) {
      if (v != aValue)
        return true
    }
    return false
  }

  ///////////////////////////////////////////////
  function fold(that,aAcc0,fun) {
    foreach (k,v in that) {
      aAcc0 = fun(aAcc0,v,k)
    }
    return aAcc0
  }

  ///////////////////////////////////////////////
  function scan(that,aAcc0,fun,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      aAcc0 = fun(aAcc0,v,k)
      r.Add(aAcc0)
    }
    return r
  }

  ///////////////////////////////////////////////
  function forEach(that,fun) {
    foreach (k,v in that) {
      fun(v,k)
    }
    return that
  }

  ///////////////////////////////////////////////
  function map(that,fun,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      r.Add(fun(v,k,r))
    }
    return r
  }

  ///////////////////////////////////////////////
  function mapValid(that,fun,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      local t = fun(v,k,r)
      if (t)
        r.Add(t)
    }
    return r
  }

  ///////////////////////////////////////////////
  function none(that,fun) {
    foreach (k,v in that) {
      if (fun(v,k))
        return false
    }
    return true
  }

  ///////////////////////////////////////////////
  function drop(that,aPred,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      if (!aPred(v,k,r)) {
        r.Add(v)
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function dropN(that,aN,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    local c = 0
    foreach (k,v in that) {
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
  function max(that,aProperty) {
    if (that.empty())
      return null
    local m = ::algo.first(that,aProperty)
    if (aProperty) {
      foreach (k,o in that) {
        local v = o[aProperty]
        if (v > m)
          m = v
      }
    }
    else {
      foreach (k,v in that) {
        if (v > m)
          m = v
      }
    }
    return m
  }

  ///////////////////////////////////////////////
  function min(that,aProperty) {
    if (that.empty())
      return null
    local m = ::algo.first(that,aProperty)
    if (aProperty) {
      foreach (k,o in that) {
        local v = o[aProperty]
        if (v < m)
          m = v
      }
    }
    else {
      foreach (k,v in that) {
        if (v < m)
          m = v
      }
    }
    return m
  }

  ///////////////////////////////////////////////
  function partition(that,aPred,aTrueArray,aFalseArray) {
    local a = (aTrueArray ? aTrueArray : [])
    local b = (aFalseArray ? aFalseArray : [])
    foreach (k,v in that) {
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
  function sum(that,aProperty) {
    if (that.empty())
      return null
    local z = ::algo.first(that,aProperty)
    local s = z
    if (aProperty) {
      foreach (k,v in that) {
        s += v[aProperty]
      }
    }
    else {
      foreach (k,v in that) {
        s += v
      }
    }
    s -= z
    return s
  }

  ///////////////////////////////////////////////
  function prod(that,aProperty) {
    if (that.empty())
      return null
    local s = 1
    if (aProperty) {
      foreach (k,v in that) {
        s *= v[aProperty]
      }
    }
    else {
      foreach (k,v in that) {
        s *= v
      }
    }
    return s
  }

  ///////////////////////////////////////////////
  function take(that,aPred,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      if (aPred(v,k,r)) {
        r.Add(v)
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function takeN(that,aN,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    local c = 0
    foreach (k,v in that) {
      if (c++ >= aN)
        break;
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function keys(that,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      r.Add(k)
    }
    return r
  }

  ///////////////////////////////////////////////
  function values(that,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function keysValues(that,aOutputArray) {
    local r = (aOutputArray ? aOutputArray : [])
    foreach (k,v in that) {
      r.Add(k)
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function findKey(that,aPred) {
    if (typeof aPred == "function") {
      foreach (k,v in that) {
        if (aPred(v,k))
          return k
      }
    }
    else {
      foreach (k,v in that) {
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
  local __lint = {
    this_key_notfound_getk = 0
  }
  if ("_delegate" in this) {
    return this._delegate;
  }
  local d = {}
  foreach (k,f in ::algo) {
    if (k[0] == '_')
      continue
    d[k] <- ::closure.thisAsFirstParm(::algo,f)
  }
  this._delegate <- d
  return this._delegate
}
