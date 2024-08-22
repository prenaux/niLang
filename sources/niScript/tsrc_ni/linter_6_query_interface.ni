local __lint = {
  call_warning = 1
  key_notfound_callk = 1
}

::gMath <- ::CreateGlobalInstance("niLang.Math")

local myout = 789, myout2 = ""

function randHamster(aMin,aMax) : (myout,myout2) {
  local hamster = 456;
  return ::gMath.RandIntRange(aMin+myout,aMax+hamster);
}

function writeHamster(niLang:iFileSystem aFS,aFileName) {
  local hamster = "This is the story of a hamster";
  local fp = aFS.FileOpen(aFileName,::eFileOpenMode.Write);
  fp.WriteString(hamster);
  return fp.GetSize();
}

function main() {
  local lang = ::CreateInstance("niLang.Lang")
  local fs = lang.root_fs

  // ok
  randHamster(100,300)
  // should give us an invalid iunknown type mistmatch
  writeHamster(lang,"test_write_hamster.txt")
  // ok
  writeHamster(fs,"test_write_hamster.txt")
}
