// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")

::assertTest <- #(testResult,actual,_aMsg) {
  if (!testResult) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "test failed" + (actual ? (" with value '" + ::lang.toString(actual) + "'") : ("")) +  "."
  }
  return actual;
}

::assertEquals <- #(expected,actual,_aMsg) {
  if (!(expected == actual)) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "expected '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotEqual <- #(expected,actual,_aMsg) {
  if (expected == actual) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "expected values to be different '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotNull <- #(actual,_aMsg) {
  if (actual == null) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "value can't be null."
  }
  return actual;
}

::assertIsNull <- #(actual,_aMsg) {
  if (actual != null) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "value should be null but is '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertHas <- #(expected,actual,_aMsg) {
  if (typeof expected == "array") {
    foreach (e in expected) {
      ::assertHas(e, actual, _aMsg);
    }
  }
  else {
    if (!(expected in actual)) {
      throw (_aMsg ? _aMsg : "Assert") + " ==> " +
        "'"+ ::lang.toString(expected) +
        "' not in '" + ::lang.toString(actual) + "'."
    }
  }
  return actual;
}

::assertHasType <- #(expectedSlotType,expected,actual,_aMsg) {
  if (typeof expected == "array") {
    foreach (e in expected) {
      ::assertHasType(expectedSlotType, e, actual, _aMsg);
    }
  }
  else {
    if (!(expected in actual)) {
      throw (_aMsg ? _aMsg : "Assert") + " ==> " +
        "'"+ ::lang.toString(expected) +
        "' not in '" + ::lang.toString(actual) + "'."
    }
    ::assertType(expectedSlotType, actual[expected], _aMsg);
  }
  return actual;
}

::assertHasnt <- #(expected,actual,_aMsg) {
  if (typeof expected == "array") {
    foreach (e in expected) {
      ::assertHasnt(e, actual, _aMsg);
    }
  }
  else {
    if (expected in actual) {
      throw (_aMsg ? _aMsg : "Assert") + " ==> " +
        "'"+ ::lang.toString(expected) +
        "' should not be in '" + ::lang.toString(actual) + "'."
    }
  }
  return actual;
}

::assertAtLeast <- #(expected,actual,_aMsg) {
  if (expected > actual) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "expected at least '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotMoreThan <- #(expected,actual,_aMsg) {
  if (actual > expected) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "expected not more than '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertType <- #(expected,actual,_aMsg) {
  local __lint = { typeof_usage = 0 }
  local actualType = (typeof actual)
  if (!(expected == actualType)) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "expected type '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotEmpty <- #(actual,_aMsg) {
  if (actual.?empty() || !actual.?len()) {
    throw (_aMsg ? _aMsg : "Assert") + " ==> " +
      "value can't be empty."
  }
  return actual;
}

::assertBetween <- #(atLeast,notMoreThan,actual,_aMsg) {
  ::assertAtLeast(atLeast,actual,_aMsg);
  ::assertNotMoreThan(notMoreThan,actual,_aMsg);
  return actual;
}

::assertName <- #(actual,_aMsg) {
  ::assertType("string",actual,_aMsg);
  ::assertNotEmpty(actual,_aMsg);
  return actual;
}
