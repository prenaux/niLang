local __lint = {
  _all = 1
  _pedantic = 1
  _explicit = 1
}

function main() void {
  local lang = ::CreateInstance("niLang.Lang");

  ::LINT_CHECK_TYPE("interface_def<iUnknown>", lang);
  lang.Log(0, "Hellow", "nofile", -1, "nofunc"); // should fail

  // use lint_as_type which assumes the type is what says it is
  lang = ::LINT_AS_TYPE("iTheUnknownHamster", lang); // should fail with cant_find_interface_def
  ::LINT_CHECK_TYPE(
    "error_code<error_code_cant_find_interface_def,Cant find interface definition 'iTheUnknownHamster'.>",
    lang);
  lang = ::LINT_AS_TYPE("iLang", lang); // should be ok
  ::LINT_CHECK_TYPE("interface_def<iLang>", lang);
  lang.Log(0, "Hellow", "nofile", -1, "nofunc"); // should be ok
}
