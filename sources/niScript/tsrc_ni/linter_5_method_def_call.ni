local __lint = {
  call_warning = 1
  key_notfound_callk = 1
}

local myout = 789, myout2 = ""

function randHamster(iMath aMath,aMin,aMax) : (myout,myout2) {
  local hamster = 456;
  aMath.NotaMethod(aMin,aMax); // should key_notfound_callk
  aMath.RandIntRange(aMin,aMax,123); // too many arguments
  aMath.RandIntRange(456); // not enough arguments
  return aMath.RandIntRange(aMin+myout,aMax+hamster); // should be good
}

function writeHamster(iFileSystem aFS,aFileName) {
  local hamster = "This is the story of a hamster";
  local fp = aFS.FileOpen(aFileName,::eFileOpenMode.Write);
  fp.WriteString(hamster);
  fp.WriteString(); // Incorrect number of arguments
  return fp.GetSize();
}

function main() {
  randHamster(null,100,300)
  writeHamster(null,"test_write_hamster.txt")
}
