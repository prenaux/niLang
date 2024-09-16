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

function ret_iLang_to_iFile() iFile {
  local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")
  return lang; // mismatched type
}

function ret_iLang_to_iUnknown() iUnknown {
  local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")
  return lang; // should work because all interfaces are assumed to be based on iUnknown
}

function ret_iFutureValue_to_iFuture() iFuture {
  local v = ::LINT_AS_TYPE("iFutureValue",null)
  return v; // should work because iFutureValue inherits of iFuture
}

function ret_iFile_to_iFuture() iFuture {
  local v = ::LINT_AS_TYPE("iFile",null)
  return v; // should fail
}

function ret_cond_spaceship(iUnknown v) int {
  return v <=> null;
}
function ret_cond_eq(iUnknown v) int {
  return v == null;
}
function ret_cond_ne(iUnknown v) int {
  return v != null;
}
function ret_cond_g(iUnknown v) int {
  return v > null;
}
function ret_cond_ge(iUnknown v) int {
  return v >= null;
}
function ret_cond_l(iUnknown v) int {
  return v < null;
}
function ret_cond_le(iUnknown v) int {
  return v <= null;
}

function ret_cond_in(table v) int {
  return "foo" in v;
}

function ret_cond_or(iUnknown v) string {
  return null || v || "string";
}
function ret_cond_and(iUnknown v) string {
  return null && v && "string";
}

function main() void {
  local r = ret_string();
  ::LINT_CHECK_TYPE("resolved_type<string>", r);
}
