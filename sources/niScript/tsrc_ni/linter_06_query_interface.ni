::Import("niLang")

local __lint = {
  _all = 1
  _pedantic = 1
  _explicit = 1
}

::gMath <- ::CreateGlobalInstance("niLang.Math").?QueryInterface("iMath")
::gMath1 <- ::CreateGlobalInstance("niLang.Math")
::gMath2 <- ::gMath1.QueryInterface("iMath")

function iunknown_QueryInterface(iunknown obj) iMath {
  return obj.QueryInterface("iMath");
}

function global_QueryInterface(iunknown obj) iMath {
  local r = ::QueryInterface(obj,"iMath");
  ::LINT_CHECK_TYPE("interface_def<iMath>", r);
}

function main(iunknown m) void {
  ::gMath.RandIntRange(100,200) // should be ok
  ::gMath1.RandIntRange(100,200) // should fail, still iUnknown
  ::gMath2.RandIntRange(100,200) // should be ok
}
