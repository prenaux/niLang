local __lint = {
  call_warning = 1
}

local myout = 789, myout2 = ""

function foobar(niLang:iMath a,b,c) : (myout,myout2) {
  local hamster = 456;
  return a.RandIntRange(b+myout,c+hamster);
}

function main() {
  foobar(null,100,300)
}
