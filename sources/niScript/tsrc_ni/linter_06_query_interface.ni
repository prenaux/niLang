local __lint = {
  _all = 1
  _pedantic = 0
  _explicit = 1
}

::gMath <- ::CreateGlobalInstance("niLang.Math").?QueryInterface("iMath")
::gMath1 <- ::CreateGlobalInstance("niLang.Math")
::gMath2 <- ::gMath1.QueryInterface("iMath")

function main() {
  ::gMath.RandIntRange(100,200) // should be ok
  ::gMath1.RandIntRange(100,200) // should fail, still iUnknown
  ::gMath2.RandIntRange(100,200) // should be ok
}
