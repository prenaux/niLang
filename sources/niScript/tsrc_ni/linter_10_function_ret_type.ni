local __lint = {
  // _pedantic = 1
  _explicit = 1
  _experimental = 1
}

function ret_string() string {
  return "hello"
}

function ret_string_nopes() string {
  return 123; // mismatched type
}

function ret_string_property() string {
  local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")
  return lang.GetProperty("foo")
}

function ret_string_property_nopes() int {
  local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")
  return lang.GetProperty("foo") // mismatched type
}

function ret_int_after_call() int {
  return ret_string_property_nopes();
}

local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")
function ret_string_property2():(lang) string {
  return lang.property["foo"]
}

function ret_string2(string v) string {
  return v.normalize()
}

function main() void {
  local r = ret_string();
  ::LintAssertType("resolved_type<string>", r);
}