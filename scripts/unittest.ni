// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
/* Example usage :

//// All these tests should fail
::Import("unittest.ni")

::TEST("CHECK",function() {
    local x = 1
    ::CHECK(1 == 1)
    ::CHECK(1 == 0)
})

::TEST("CHECK_EQUAL",function() {
    local y = 456
    ::CHECK_EQUAL(1,1)
    ::CHECK_EQUAL(123,456)
})

::FIXTURE("CHECK_CLOSE_FIXTURE", {
    tolerance10 = 10
})

::TEST("CHECK_CLOSE_FIXTURE","POSITIVE",function() {
    ::CHECK_CLOSE(100,105,tolerance10)
    ::CHECK_CLOSE(100,105,3)
})

::TEST("CHECK_CLOSE_FIXTURE","NEGATIVE",function() {
    ::CHECK_CLOSE(105,100,tolerance10)
    ::CHECK_CLOSE(105,100,3)
})

::RUN_TESTS() // will check the first argument passed on the command line...
*/
::Import("lang.ni")

::THROW <- function(aMsg,_aDesc) {
  local __lint = {
    // TODO: GetStackInfos should return a type instead?
    key_notfound_getk = 0
    // typeof is used to print the types
    typeof_usage = 0
  }

  local src = "unknown"
  local func = "unknown"
  local line = -1
  local msg = "E/"
  if (typeof(aMsg) != "string") {
    msg += ::lang.toString(aMsg)
  }
  else {
    msg += aMsg
  }
  if (_aDesc) {
    msg += "\nE/  Desc: " + _aDesc;
  }

  msg += "\nE/  Stack:\n"
  local locals
  for (local i = 3; i < 10; ++i) {
    local si = ::GetStackInfos(i)
    if (!si) break;
    if (i == 3) {
      locals = si.locals
    }
    msg += ::format("E/  - %s(%d): [%s]\n",si.src,si.line,si.func)
  }
  msg += "E/  Locals:"
  if (locals) {
    foreach (k,v in locals) {
      if (v == ::getroottable())
        v = "__roottable__"
      msg += "\nE/  - " + k + " : " + typeof(v) + " = " + ::lang.toString(v)
    }
  }

  throw msg
}

::CHECK <- function(shouldBeTrue, _aDesc) {
  if (!shouldBeTrue) {
    ::THROW("CHECK FAILED.",_aDesc)
  }
}

::CHECK_EQUAL <- function(a,b,_aDesc) {
  local m
  local r
  try {
    if (typeof a == "array" && typeof b == "array") {
      local len = a.GetSize();
      r = len == b.GetSize();
      if (r) {
        for (local i = 0; i < len; ++i) {
          r = (a[?i] == b[?i]);
          if (!r)
            break;
        }
      }
    }
    else {
      r = (a == b)
    }
  }
  catch (e) {
    m = "Compare failed: " + e
  }
  if (m) {
    r = false
  }
  else if (!r) {
    try {
      m = "Expected '"+::lang.toString(a)+"', got '"+::lang.toString(b)+"'."
    }
    catch (e) {
      m = "Can't convert test report to string."
    }
  }
  if (!r) {
    ::THROW("CHECK_EQUAL FAILED: " + m, _aDesc)
  }
}

::CHECK_NOT_EQUAL <- function(a,b,_aDesc) {
  local m
  local r
  try {
    if (typeof a == "array" && typeof b == "array") {
      local len = a.GetSize();
      r = len == b.GetSize();
      if (r) {
        for (local i = 0; i < len; ++i) {
          r = (a[?i] == b[?i]);
          if (!r)
            break;
        }
      }
    }
    else {
      r = (a == b)
    }
  }
  catch (e) {
    m = "Compare failed: " + e
  }
  if (m) {
    r = true
  }
  else if (r) {
    try {
      m = "Expected '"+::lang.toString(a)+"' to be different of '"+::lang.toString(b)+"'."
    }
    catch (e) {
      m = "Can't convert test report to string."
    }
  }
  if (r) {
    ::THROW("CHECK_NOT_EQUAL FAILED: " + m, _aDesc)
  }
}

::CHECK_CLOSE <- function(a,b,tolerance,_aDesc) {
  local m
  local r
  try {
    local diff = (b-a)
    if (diff < 0) diff = -diff
    r = (diff < tolerance)
  }
  catch (e) {
    m = "Compare failed."
  }
  if (m) {
    r = false
  }
  else if (!r) {
    try {
      m = "Expected '"+a+"' within '"+tolerance+"' but got '"+b+"'."
    }
    catch (e) {
      m = "Can't convert test to string."
    }
  }
  if (!r) {
    ::THROW("CHECK_CLOSE FAILED: " + m, _aDesc)
  }
}

::FIXTURE <- function(aName,_aTable) {
  if (typeof aName == "array") {
    foreach (v in aName) {
      ::FIXTURE(v,_aTable)
    }
    return;
  }
  if (aName in ::FIXTURES) {
    throw "Fixture '"+aName+"' already exist."
  }
  ::FIXTURES[aName] <- _aTable || {}
  ::FIXTURES[aName].name <- aName
}

::TEST <- function(aFixture,aName,_aMeth) {
  local __lint = { _all = 0 }

  // allow for function(aName,aMeth) --- no fixture...
  local meth
  if (_aMeth) {
    meth = _aMeth;
  }
  else {
    meth = aName
    aName = aFixture
    aFixture = "GLOBAL"
  }

  if (typeof(aFixture) == "string") {
    if (!(aFixture in ::FIXTURES))
      throw "No fixture '"+aFixture+"."
    aFixture = ::FIXTURES[aFixture]
  }
  if (typeof(aFixture) != "table")
    throw "Fixture should be a table."
  if (!("name" in aFixture))
    throw "Fixture table should have a name member."
  if (!aFixture.name)
    throw "Invalid fixture name."
  ::TESTS.append({
    fixture = aFixture.name,
    name = aFixture.name + "-" + aName,
    meth = meth
  }.SetDelegate(aFixture.Clone()));
}

::RUN_TESTS <- function(aName) {
  if (!aName) {
    if (::?GetArgs().GetSize() > 1) {
      aName = ::?GetArgs()[1]
    }
  }

  if (aName) {
    ::println("I/FIXTURE: " + aName)
  }

  local total = 0
  local numFailed = 0
  local numSucceded = 0
  foreach (t in ::TESTS) {
    if (aName && aName.len() && t.name != aName && t.fixture != aName) {
      continue;
    }
    ::println("### " + t.name + " ###")
    local start = ::clock()
    try {
      t.meth()
      ::println(::format("V/SUCCEEDED in %.6fs",::clock()-start))
      ++numSucceded
    }
    catch (e) {
      ::println(e)
      ::println(::format("E/FAILED in %.6fs",::clock()-start))
      ++numFailed
    }
    ++total
  }
  ::println("### "+total+" test"+(total>1?"s":"")+" ###")
  if (numFailed == total) {
    ::println("All failed.")
    return 1;
  }
  else if (numSucceded == total) {
    ::println("All succeded.")
  }
  else {
    ::println("'" + numFailed + "' failed")
    ::println("'" + numSucceded + "' succeded")
  }

  return numFailed
}

::FIXTURES <- {
  GLOBAL = {
    name = "GLOBAL"
  }
}
::TESTS <- []

function ::main(args) {
  return ::RUN_TESTS(args[?1]);
}
