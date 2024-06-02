local localTable = {
  _one = 10,

  function methodSet() {
    _one = 123
  }

  function methodSetUnknownKey() {
    _unknownkey = 123
  }
};

function main() : (localTable) {
  localTable._one = 456;
  ::println("Linter test for this_set_key_notfound")
}
