local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

function this_get() {
  this.numfagiano;
  this.?numfagiano;
  this.cookfagiano();
  this.?cookfagiano();
}

function foo_get() {
  local foo = {
    bar = 123
    narf = 456

    function hellow() {
    }
  }

  foo.bar;
  foo.hamster;
  foo.?hamster;
  foo.narf;
  foo.?narf;
  foo.hellow();
  foo.?hellow();
  foo.byebye();
  foo.?byebye();
}

// TODO
// function array_get() {
//   local arr = [1,2,3];
//   ::LintAssertType("array", arr);
//   // TODO: Maybe this should pass but otoh the cases where we know the size of
//   // the array at lint time is probably close to zero, so it probably doesnt
//   // make sense?
//   // arr[0];
//   arr[10];
//   arr[?10];
// }
