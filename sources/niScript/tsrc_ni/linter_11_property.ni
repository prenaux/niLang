
local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")

function main():(lang) void {
  local r = lang.property["foo"];
  ::LintAssertType("resolved_type<string>", r);

  local t = lang.current_time;
  ::LintAssertType("property_def<iLang,set:NULL,get:GetCurrentTime>", t);
}
