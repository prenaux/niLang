local __lint = {
  call_warning = 1
  key_notfound_callk = 1
}

local myout = 789, myout2 = ""

function randHamster(niLang:iMath aMath,aMin,aMax) : (myout,myout2) {
  local hamster = 456;
  aMath.NotaMethod(aMin,aMax); // should key_notfound_callk
  aMath.RandIntRange(aMin,aMax,123); // too many arguments
  aMath.RandIntRange(456); // not enough arguments
  return aMath.RandIntRange(aMin+myout,aMax+hamster); // should be good
}

function main() {
  randHamster(null,100,300)
}
