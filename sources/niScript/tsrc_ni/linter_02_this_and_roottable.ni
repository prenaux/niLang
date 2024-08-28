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
      local __lint = { key_notfound_callk = 1 }
      // should fail lint
      ::functionNotInRoot()
    });
  }
}
