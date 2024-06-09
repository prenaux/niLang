local __lint = {
  call_warning = 1
}

function foobar(niLang:iMath a,b,c) {
  return a.RandIntRange(b,c);
}

function main() {
  foobar(null,100,300)
}
