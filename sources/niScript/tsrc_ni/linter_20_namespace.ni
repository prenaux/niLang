local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

function create_namespaces() {
  // This is the recommended pattern to safely create namespaces that don't
  // override prexisting ones, ::namespace should be discouraged.
  ::LINT_CHECK_TYPE("null", ::?bar) // should be ok
  ::bar <- {
  }

  ::bar.bli <- { // should be ok
  }
  ::LINT_CHECK_TYPE("null", ::?bar.?bli) // should fail

  ::LINT_CHECK_TYPE("null", ::?bar.?qoo) // should be ok
  ::bar.qoo.hamster <- { // should fail
  }

  ::foo <- {}
  ::LINT_CHECK_TYPE("null", ::?foo) // should fail because we already declared foo
  ::foo <- {
  }
}
