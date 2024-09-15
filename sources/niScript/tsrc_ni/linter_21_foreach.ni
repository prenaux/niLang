local __lint = {
  _all = 1
  _experimental = 1
}

::Import("niLang")

function foreach_array() {
  local arr = [1,2,3]
  foreach (i,v in arr) {
    ::LINT_CHECK_TYPE("array", arr);
    ::LINT_CHECK_TYPE("resolved_type<int>", i);
    ::LINT_CHECK_TYPE("null", v);
  }
}

function foreach_table() {
  local t = {
    foo = "foo"
    bar = 123
    narf = "yee"
  }

  foreach (k,v in t) {
    ::LINT_CHECK_TYPE("table", t);
    ::LINT_CHECK_TYPE("resolved_type<string>", k);
    ::LINT_CHECK_TYPE("null", v);
  }
}

function foreach_float(float t) {
  foreach (i,v in t) { // cannot iterate a float
    ::LINT_CHECK_TYPE("resolved_type<float>", t);
    ::LINT_CHECK_TYPE("null", i);
    ::LINT_CHECK_TYPE("null", v);
  }
}

function foreach_int(int t) {
  foreach (i,v in t) {
    ::LINT_CHECK_TYPE("resolved_type<int>", t);
    ::LINT_CHECK_TYPE("resolved_type<int>", i);
    ::LINT_CHECK_TYPE("resolved_type<int>", v);
  }
}

function foreach_string(string t) {
  foreach (i,v in t) {
    ::LINT_CHECK_TYPE("resolved_type<string>", t);
    ::LINT_CHECK_TYPE("resolved_type<int>", i);
    ::LINT_CHECK_TYPE("resolved_type<int>", v);
  }
}

function foreach_of_noniterable(iFile t) {
  foreach (i,v in t) {
    ::LINT_CHECK_TYPE("interface_def<iFile>", t);
    ::LINT_CHECK_TYPE("null", i);
    ::LINT_CHECK_TYPE("null", v);
  }
}
