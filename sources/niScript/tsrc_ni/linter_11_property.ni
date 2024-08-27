local __lint = {
  _experimental = 1
}

local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")

function property_current_time():(lang) iTime {
  return lang.current_time;
}

function property_current_time_as_int():(lang) int {
  return lang.current_time; // should fail linting
}

function property_char_it(string v) int {
  local it = v.CreateCharIt(0)
  ::LintAssertType("interface_def<iHStringCharIt>", it);
  it.num_chars_hamster; // should fail
  return it.num_chars;
}

function main():(lang) void {
  local r = lang.property["foo"];
  ::LintAssertType("resolved_type<string>", r);

  local t = property_current_time();
  ::LintAssertType("interface_def<iTime>", t);

  local i = property_current_time_as_int();
  ::LintAssertType("resolved_type<int>", i);

  local it = property_char_it("bla");
  ::LintAssertType("resolved_type<int>", it);
}
