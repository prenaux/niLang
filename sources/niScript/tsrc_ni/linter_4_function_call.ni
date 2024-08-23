//
// TODO:
// - [x] Check whether simple functions are called with the correct number of arguments
// - [ ] Check whether functions in a table are called with the correct number of arguments
// - [ ] Check whether simple native functions are called with correct number of arguments
// - [ ] Check whether iUnknown methods are called with the correct number of arguments
//
local __lint = {
  call_warning = 1
}

function f0() {
}

function f3(a,b,c) {
}

function main() {
  ::vmprintln("... thistable:" + this.__debug_name);

  f0();
  f0(1);
  f0(1,2);

  f3();
  f3(1,2);

  f3(1,2,3,4);
}
