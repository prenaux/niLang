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
      return that[?0]
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
  function first(that,_aProperty) {
    foreach (k,v in that) {
      return _aProperty ? v[_aProperty] : v
    }
    return null
  }

  ///////////////////////////////////////////////
  function last(that,_aProperty) {
    local r
    if (_aProperty) {
      foreach (k,v in that) {
        r = v[_aProperty]
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
  function scan(that,aAcc0,fun,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
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
  function map(that,fun,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
    foreach (k,v in that) {
      r.Add(fun(v,k,r))
    }
    return r
  }

  ///////////////////////////////////////////////
  function mapValid(that,fun,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
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
  function drop(that,aPred,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
    foreach (k,v in that) {
      if (!aPred(v,k,r)) {
        r.Add(v)
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function dropN(that,aN,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
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
  function max(that,_aProperty) {
    if (that.empty())
      return null
    local m = ::algo.first(that,_aProperty)
    if (_aProperty) {
      foreach (k,o in that) {
        local v = o[_aProperty]
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
  function min(that,_aProperty) {
    if (that.empty())
      return null
    local m = ::algo.first(that,_aProperty)
    if (_aProperty) {
      foreach (k,o in that) {
        local v = o[_aProperty]
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
  function partition(that,aPred,_aTrueArray,_aFalseArray) {
    local a = (_aTrueArray ? _aTrueArray : [])
    local b = (_aFalseArray ? _aFalseArray : [])
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
  function sum(that,_aProperty) {
    if (that.empty())
      return null
    local z = ::algo.first(that,_aProperty)
    local s = z
    if (_aProperty) {
      foreach (k,v in that) {
        s += v[_aProperty]
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
  function prod(that,_aProperty) {
    if (that.empty())
      return null
    local s = 1
    if (_aProperty) {
      foreach (k,v in that) {
        s *= v[_aProperty]
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
  function take(that,aPred,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
    foreach (k,v in that) {
      if (aPred(v,k,r)) {
        r.Add(v)
      }
    }
    return r
  }

  ///////////////////////////////////////////////
  function takeN(that,aN,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
    local c = 0
    foreach (k,v in that) {
      if (c++ >= aN)
        break;
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function keys(that,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
    foreach (k,v in that) {
      r.Add(k)
    }
    return r
  }

  ///////////////////////////////////////////////
  function values(that,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
    foreach (k,v in that) {
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function keysValues(that,_aOutputArray) {
    local r = (_aOutputArray ? _aOutputArray : [])
    foreach (k,v in that) {
      r.Add(k)
      r.Add(v)
    }
    return r
  }

  ///////////////////////////////////////////////
  function findKey(that,_aPred) {
    if (typeof _aPred == "function") {
      foreach (k,v in that) {
        if (_aPred(v,k))
          return k
      }
    }
    else {
      foreach (k,v in that) {
        if (v == _aPred)
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
