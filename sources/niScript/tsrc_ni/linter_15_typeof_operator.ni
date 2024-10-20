::Import("lang.ni")

local __lint = {
  _all = 1
  _experimental = 1
  getk_in_null = 1
  typeof_usage = 1
}

function get_some_type() string { return "sometype"; }

function typeof_usage() {
  local str = "astring"
  if (typeof(str) == "int") { // redundant typeof usage
  }

  local t = { pstr = 123 }
  if (typeof(t.pstr) == "int") { // redundant typeof usage
  }

  local hamster

  if (typeof(hamster) == "SomeHamster") { // invalid type
  }
  if (typeof(hamster) == 123) { // typedef definition not a literal string
  }

  if (typeof(hamster) == "SomeHamster" || // invalid type
      "SomeHamster2" == typeof(hamster)) // invalid type
  { // invalid type
  }

  local someType = this.get_some_type()
  if (typeof(hamster) == someType) { // typedef not a literal string
  }
  someType += "123"
  if (typeof(hamster) == someType) { // typedef not a literal string
  }
  if (someType == typeof(hamster)) { // typedef not a literal string
  }

  local myType = "int"
  if (typeof(hamster) == myType) { // should be ok, typedef is still a literal string
  }

  // TODO: Need to implement the operators...
  myType += "123"

  // TODO: should be invalid typedef, literal is now "int123", or it could be
  // considered a resolved_type aswell? Maybe that'd be a reasonable first
  // implementation of this, all operator return resolved_type<INPUT_TYPE>.
  if (typeof(hamster) == myType) {
  }
}

function typeof_t_append(t) {
  if (typeof(t) == "array") {
    t.append(123); // should be ok
  }
}

function typeof_switch1(t) {
  switch (typeof(t)) {
    case "array": {
      t.append(123); // should be ok
      break;
    }
  }

  t.append(123); // should fail, not an array
}

function typeof_switch2(t) {
  switch (typeof(t)) {
    case "array": {
      t.append(123); // should be ok
      t.startswith("weee"); // should fail, not a string
      break;
    }
    case "string": {
      t.startswith("weee"); // should be ok
      t.append(123); // should fail, not an array
      break;
    }
    case "stringnarf": { // invalid typeof test type
      t.startswith("weee"); // should fail, not a string
      break;
    }
    default: {
      t.startswith("weee"); // should fail, not a string
      break;
    }
  }

  t.startswith("weee"); // should fail, not a string
  t.append(123); // should fail, not an array
}

function typeof_if_string_eq_t() void {
  local t
  if ("string" == typeof(t)) {
    t.startswith("bla"); // should be ok
  }
  else {
    t.startswith("qoo"); // should fail
  }
  t.startswith("qee"); // should fail
}

function typeof_if_t_eq_string() void {
  local t
  if (typeof(t) == "string") {
    t.startswith("bla"); // should be ok
  }
  else {
    t.startswith("bla"); // should fail
  }
  t.startswith("qee"); // should fail
}

function typeof_if_n_eq_string() void {
  local n = 123
  if (typeof(n) == "string") {
    n.startswith("bla"); // should be ok
  }
  else {
    n.startswith("bla"); // should fail
  }
  n.startswith("qeee"); // should fail
}

function typeof_table_member(table t) {
  local t = { pstr = null }
  // TODO: Should warn: Prefer typeof local variable. Note that the obvious
  // trivial implementation of looking whether the stack position is a local
  // variable isn't reliable since those stack positions will be reused if the
  // local in question isnt used anymore.
  if (typeof(t.pstr) == "string") {
    // Should succeed? This one would require to intro some shadowing... which
    // would be cool but just replacing this by fetching in a local variable
    // first does the same without the significant increase in complexity in
    // the linter.
    t.pstr.startswith("foo");
  }
}

function typeof_iunknown() {
  local __lint = {
    typeof_usage = 0
  }

  if (typeof ::gLang == "iunknown") {
  }
  // should fail, only the lower case variant is returned by the VM
  if (typeof ::gLang == "iUnknown") {
  }

  // should fail, void isnt a valid typename, only can be used as return type to indicate "return nothing/null"
  if (typeof ::gLang == "void") {
  }
}

function main() void {
  local t, u
  if (typeof(t) == "array") {
    t.append(123); // should be ok
    if (typeof(u) == "array") {
      u.append(789); // should be ok
    }
    else {
      u.startswith("nopes"); // should fail
    }
  }
  else if ("string" == typeof(t)) {
    t.startswith("bla"); // should be ok
  }

  // TODO
  t.append(123); // should fail, using typeof(t) should tag 't' has "must have an explicit type"
  t.startswith("bla"); // should fail, using typeof(t) should tag 't' has "must have an explicit type"
  // TODO
  u.append(123); // should fail, using typeof(t) should tag 'u' has "must have an explicit type"
}
