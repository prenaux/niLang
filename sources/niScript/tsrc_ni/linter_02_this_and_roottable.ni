__debug_name <- "linter_02_test_thistable"

someTable <- {
  foo = 1
  function check(aCall) {
    aCall.call(::getroottable())
  }
}
::LINT_CHECK_TYPE("table:linter_02_test_thistable", this);

function ::functionInRoot() {
}
::LINT_CHECK_TYPE("table:linter_02_test_thistable", this);

function main() {
  ::LINT_CHECK_TYPE("table:linter_02_test_thistable", this);

  ::vmprintln("... thistable:" + this.__debug_name);

  someTable.foo

  someTable.check(function() {
    ::LINT_CHECK_TYPE("table:linter_02_test_thistable", this);
    functionInRoot() // should fail this table is not the root table
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
