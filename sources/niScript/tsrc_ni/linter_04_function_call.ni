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

function fargs(_args_) {
}

function call_varargs():(f0) {
  // should all succeed
  fargs();
  fargs(1);
  fargs(1,2,3);
}

// 2 args min, 4 max. Underscore indicate to the linter that the parameter is
// optional, must all be the last arguments.
function f_atleast2(a,b,_c,_d) : (f0,f3) {
}

function call_atleast2() {
  // should all succeed
  f_atleast2(1,2);
  f_atleast2(1,2,3);
  f_atleast2(1,2,3,4);
  // should fail
  f_atleast2();
  f_atleast2(1);
  f_atleast2(1,2,3,4,5);
}

function f_borkedoptional(a,b,_c,d) : (f0,f3)  {
}

function call_borkedoptional() {
  // should all succeed
  f_borkedoptional(1,2);
  f_borkedoptional(1,2,3);
  f_borkedoptional(1,2,3,4);
  // should fail
  f_borkedoptional();
  f_borkedoptional(1);
  f_borkedoptional(1,2,3,4,5);
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
