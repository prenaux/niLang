local __lint = {
  _all = 1
  _pedantic = 1
  _explicit = 1
}

function main() void {
  ::vmprintln("Hello Linter!") // ok
  ::vmprintln("Hello Linter!", 123) // invalid number of arg

  local lang = ::CreateInstance("niLang.Lang")
  local lang1 = ::CreateInstance("niLang.Lang",1,2,3) // too many args
  local lang2 = ::CreateInstance() // not enough args

  local math = ::CreateGlobalInstance("niLang.Math")
  local math1 = ::CreateGlobalInstance("niLang.Math",1,2,3) // too many args
  local math2 = ::CreateGlobalInstance() // not enough args

  local hamster = ::CreateGlobalInstance("niLang.TheUnknownHamster")
}
