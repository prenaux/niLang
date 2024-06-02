function f0() {
}

function f1(a) {
}

function f2(a,b) {
}

function f3(a,b,c) {
}

function main() {
  ::println("... thistable:" + this.__debug_name);

  // Should be fine
  f0();
  f1(1);
  f2(1,2);
  f3(1,2,3);

  // Not enough arguments
  f1();
  f2();
  f3();

  // too many arguments
  f0(1,2,3,4);
  f1(1,2,3,4);
  f2(1,2,3,4);
  f3(1,2,3,4);

  // key not found
  f_unknown();
}
