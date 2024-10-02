::Import("niLang")

local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")

function ihstring_from_arg(iHString hstr) {
  ::LINT_CHECK_TYPE("resolved_type<string>", hstr);
  local len = hstr.len();
  local bla = hstr.getfile(); // string type

  local hlen = hstr.length; // should fail, this is not the iHString interface
  hstr.IsLocalized(); // should fail, this is not the iHString interface
}
