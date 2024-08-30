local __lint = {
  _all = 1
  _experimental = 1
}

function main() void {
  local t = {}
  if ("x" in t) {
    // TODO
    t["x"]; // should be ok
  }
  else if ("y" in t) {
    // TODO
    t["y"]; // should be ok
  }

  // TODO
  t["x"]; // should fail
  // TODO
  t["y"]; // should fail
}
