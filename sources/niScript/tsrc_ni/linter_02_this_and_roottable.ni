someTable <- {
  foo = 1
  function check(aCall) {
    aCall.call(::getroottable())
  }
}

function ::functionInRoot() {
}

function main() {

  ::vmprintln("... thistable:" + this.__debug_name);

  someTable.foo

  someTable.check(function() {
    functionInRoot()
    ::functionInRoot()
  });

  if (false) {
    someTable.check(function() {
      // TODO: this shouldn't be necessary to lint since `this` is the
      // roottable
      local __lint = { _all = 1 }
      // should fail lint
      ::functionNotInRoot()
    });
  }
}
