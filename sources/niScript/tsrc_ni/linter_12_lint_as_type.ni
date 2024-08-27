local __lint = {
  _all = 1
  _pedantic = 1
  _explicit = 1
}

function main() void {
  local lang = ::CreateInstance("niLang.Lang");

  ::LintAssertType("interface_def<iUnknown>", lang);
  lang.Log(0, "Hellow", "nofile", -1, "nofunc"); // should fail

  // use LintAsType which assumes the type is what says it is
  lang = ::LintAsType("iTheUnknownHamster", lang); // should fail with cant_find_interface_def
  ::LintAssertType("error_code<error_code_cant_find_interface_def,iTheUnknownHamster>", lang);
  lang = ::LintAsType("iLang", lang); // should be ok
  ::LintAssertType("interface_def<iLang>", lang);
  lang.Log(0, "Hellow", "nofile", -1, "nofunc"); // should be ok
}
