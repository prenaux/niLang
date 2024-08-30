local __lint = {
  _all = 1
  _experimental = 1
  getk_null = 1
}

function main() void {
  local t
  if (typeof(t) == "array") {
    t.Add(123); // should be ok
  }
  else if (typeof(t) == "string") {
    t.startswith("bla"); // should be ok
  }

  // TODO
  t.Add(123); // should fail, using typeof(t) should tag 't' has "must have an explicit type"
  // TODO
  t.startswith("bla"); // should fail
}
