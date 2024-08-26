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
      // roottable. We're not using the implicit nor explicit this though, so
      // this is as it should be?
      local __lint = { key_notfound_callk = 1 }
      // should fail lint
      ::functionNotInRoot()
    });
  }
}
