local __lint = {
  _all = 1
  _experimental = 1
}

::Import("lang.ni")

function import_native() {
  // Side effect of ::Import("niLang") in lang.ni
  local dtType1 = ::eDataTablePropertyType.Int;
  local dtType2 = ::eDataTablePropertyType.FluffyHamster; // should fail
}

function main() void {
  // TODO: Import script tests
  ::lang.getCurrentProcess();
}
