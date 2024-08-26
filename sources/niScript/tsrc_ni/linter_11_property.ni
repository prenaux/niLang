
local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")

function property_current_time():(lang) iTime {
  return lang.current_time;
}

function property_current_time_as_int():(lang) int {
  return lang.current_time; // should fail linting
}

function main():(lang) void {
  local r = lang.property["foo"];
  ::LintAssertType("resolved_type<string>", r);

  local t = property_current_time();
  ::LintAssertType("interface_def<iTime>", t);

  local i = property_current_time_as_int();
  ::LintAssertType("resolved_type<int>", i);
}
