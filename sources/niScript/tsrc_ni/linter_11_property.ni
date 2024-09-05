local __lint = {
  _experimental = 1
}

local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")

function property_current_time():(lang) iTime {
  return lang.current_time;
}

function property_current_time_as_int():(lang) int {
  return lang.current_time; // should fail linting, non matching types
}

function property_char_it(string v) int {
  local it = v.CreateCharIt(0)
  ::LINT_CHECK_TYPE("interface_def<iHStringCharIt>", it);
  // should fail. no semicolon to make sure lint error line is at the correct position.
  it.num_chars_hamster
  return it.num_chars
}

function property_set_value(string val):(lang) {
  lang.default_locale = val;
  lang.hamster = val; // should fail
}

function property_set_key(string val):(lang) {
  lang.env["FOO"] = val;
  lang.fagiano["FOO"] = val;
}

function main():(lang) void {
  local r = lang.property["foo"];
  ::LINT_CHECK_TYPE("resolved_type<string>", r);

  local t = property_current_time();
  ::LINT_CHECK_TYPE("interface_def<iTime>", t);

  local i = property_current_time_as_int();
  ::LINT_CHECK_TYPE("resolved_type<int>", i);

  local it = property_char_it("bla");
  ::LINT_CHECK_TYPE("resolved_type<int>", it);
}
