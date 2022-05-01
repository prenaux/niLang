// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")

::assertTest <- #(testResult,actual,msg) {
  if (!testResult) {
    throw (msg ? msg : "Assert") + " ==> " +
      "test failed" + (actual ? (" with value '" + ::lang.toString(actual) + "'") : ("")) +  "."
  }
  return actual;
}

::assertEquals <- #(expected,actual,msg) {
  if (!(expected == actual)) {
    throw (msg ? msg : "Assert") + " ==> " +
      "expected '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotEqual <- #(expected,actual,msg) {
  if (expected == actual) {
    throw (msg ? msg : "Assert") + " ==> " +
      "expected values to be different '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotNull <- #(actual,msg) {
  if (actual == null) {
    throw (msg ? msg : "Assert") + " ==> " +
      "value can't be null."
  }
  return actual;
}

::assertIsNull <- #(actual,msg) {
  if (actual != null) {
    throw (msg ? msg : "Assert") + " ==> " +
      "value should be null but is '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertHas <- #(expected,actual,msg) {
  if (typeof expected == "array") {
    foreach (e in expected) {
      ::assertHas(e, actual, msg);
    }
  }
  else {
    if (!(expected in actual)) {
      throw (msg ? msg : "Assert") + " ==> " +
        "'"+ ::lang.toString(expected) +
        "' not in '" + ::lang.toString(actual) + "'."
    }
  }
  return actual;
}

::assertHasType <- #(expectedSlotType,expected,actual,msg) {
  if (typeof expected == "array") {
    foreach (e in expected) {
      ::assertHasType(expectedSlotType, e, actual, msg);
    }
  }
  else {
    if (!(expected in actual)) {
      throw (msg ? msg : "Assert") + " ==> " +
        "'"+ ::lang.toString(expected) +
        "' not in '" + ::lang.toString(actual) + "'."
    }
    ::assertType(expectedSlotType, actual[expected], msg);
  }
  return actual;
}

::assertHasnt <- #(expected,actual,msg) {
  if (typeof expected == "array") {
    foreach (e in expected) {
      ::assertHasnt(e, actual, msg);
    }
  }
  else {
    if (expected in actual) {
      throw (msg ? msg : "Assert") + " ==> " +
        "'"+ ::lang.toString(expected) +
        "' should not be in '" + ::lang.toString(actual) + "'."
    }
  }
  return actual;
}

::assertAtLeast <- #(expected,actual,msg) {
  if (expected > actual) {
    throw (msg ? msg : "Assert") + " ==> " +
      "expected at least '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotMoreThan <- #(expected,actual,msg) {
  if (actual > expected) {
    throw (msg ? msg : "Assert") + " ==> " +
      "expected not more than '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertType <- #(expected,actual,msg) {
  local actualType = (typeof actual)
  if (!(expected == actualType)) {
    throw (msg ? msg : "Assert") + " ==> " +
      "expected type '"+ ::lang.toString(expected) +
      "' but was '" + ::lang.toString(actual) + "'."
  }
  return actual;
}

::assertNotEmpty <- #(actual,msg) {
  if (actual.?empty() || !actual.?len()) {
    throw (msg ? msg : "Assert") + " ==> " +
      "value can't be empty."
  }
  return actual;
}

::assertBetween <- #(atLeast,notMoreThan,actual,msg) {
  ::assertAtLeast(atLeast,actual,msg);
  ::assertNotMoreThan(notMoreThan,actual,msg);
  return actual;
}

::assertName <- #(actual,msg) {
  ::assertType("string",actual,msg);
  ::assertNotEmpty(actual,msg);
  return actual;
}
