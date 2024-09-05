local __lint = {
  _all = 1
  _pedantic = 1
  _explicit = 1
}

function main() void {
  local lang = ::CreateInstance("niLang.Lang")
  ::LINT_CHECK_TYPE("interface_def<iUnknown>", lang)
  lang.Log(0, "Hellow", "nofile", -1, "nofunc") // should fail

  lang = lang.QueryInterface("iLang")
  ::LINT_CHECK_TYPE("interface_def<iLang>", lang)
  lang.Log(0, "Hellow", "nofile", -1, "nofunc") // should be ok
}
