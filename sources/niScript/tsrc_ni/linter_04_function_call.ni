local __lint = {
  call_null = 1
}

function f0() {
}

function f3(a,b,c) {
}

// to test that the function is named f3b even if its not specified after the function keyword
::f3b <- function(a,b,c) {
}

function main() {
  ::vmprintln("... thistable:" + this.__debug_name);

  f0();
  f0(1);
  f0(1,2);

  f3();
  f3(1,2);

  f3(1,2,3,4);

  ::f3b(1,2,3,4);
}
