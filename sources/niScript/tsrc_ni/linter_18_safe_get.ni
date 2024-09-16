::Import("niLang")

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

function array_get() {
  local arr = [1,2,3];
  ::LINT_CHECK_TYPE("array", arr);
  arr[0]; // should be ok since we init with 3 values
  arr[2]; // should be ok since we init with 3 values
  arr[?2];
  arr[10]; // should fail, we know its out of bound
  arr[?10]; // should ok, safe get
}

function safeget_setparam(string aType) {
  aType = ::eType[?aType]
  return aType
}
