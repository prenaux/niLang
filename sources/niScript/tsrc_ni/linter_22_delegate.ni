local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

tTheDelegate <- {
  delegated_squirrel = 789
}

tTheTable <- {
  simple_squirrel = "yata!"
}

function delegate_set() {
  local t = tTheTable
  ::LINT_CHECK_TYPE("table:tTheTable", t);
  ::LINT_CHECK_TYPE("string", t.simple_squirrel);
  ::LINT_CHECK_TYPE("null", t.delegated_squirrel);

  local t2 = tTheTable.SetDelegate(tTheDelegate)
  ::LINT_CHECK_TYPE("table:tTheTable", t2);
  ::LINT_CHECK_TYPE("table:tTheDelegate", t2.GetDelegate());
  ::LINT_CHECK_TYPE("string", t2.simple_squirrel);
  ::LINT_CHECK_TYPE("I32", t2.delegated_squirrel);

  tTheTable.SetDelegate(44719);
}
