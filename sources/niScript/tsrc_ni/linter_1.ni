//
// TODO:
// - [x] implicit_this: simple implict 'this' warning. `mX + mX`, implicit read access to mX.
// - [x] this_key_notfound: member variables access to this. `this.mX` should check whether `this` table has a field mX.
// - [ ] table_key_notfound: member variables access to a table. `table.mX` should check whether `table` has a field mX.
// - [ ] this_implicit_assignment: implicit this assignment. `mX = 123`, 123 assigned to implicit 'this' access to mX.
// - [ ] free/outer variables. `function() : (mX) { }`, check mX access when used in a free variable.
//
local __lint = {
  _all = 1
  // _pedantic = 0
  // _experimental = 0
  // implicit_this_getk = 0
  // implicit_this_callk = 0
  // this_key_notfound_callk = 1
  // this_key_notfound_getk = 1
  // key_notfound_callk = 1
  // key_notfound_getk = 1
}
local narf, squirrel

local localTable = {
  one = 10,
  two = 20,
  three = this,
  subTable = {
    subOne = 100,
    subTwo = 200

    function subMethod() {
      return "sub"
    }
  }

  // function t2() {}

  function implicitMethod() {
    return one +
      two +
      t2();
  }

  function explicitMethod() {
    return this.one +
      this.two +
      this.t2();
  }

  function narfMethod() {
    local n = this.DeepClone()
    return this.three.one;
  }
};

thisLocalTable <- localTable;
::inRootLocalTable <- localTable;

// y <- 123;

function foo(string a, int b, bool c) {
  local x = 0;
  local z = x +
    // blank to test line error number
  y;
  return z;
}

function bar() {
  local n = 0;
  {
    local x = 1;
    n += x + x;
  }
  {
    local y = 2;
    n += x + y;
  }
  return n;
}

// contextunkvar = 456;
function setUnknownVar() {
  unkvar = 123;
}

function invalidAccessOuterVar() : (localTable) {
  localTable.four;

  // Inside invalidAccessOuterVar so that we can still run the script without
  // an error because someOuter doesnt exist
  function outerNotFound() : (someOuter) {
    someOuter.one;
  }
}

function invalidAccessParameter(aLocalTable) {
  aLocalTable.four;
}

function stringmethods() {
  "stuff".startswith("123");
  "stuff".doesntExist("123");
}

function dostuff() : (localTable) {
  // foo()
  // bar();
  localTable.one;
  thisLocalTable.one;
  thisLocalTable.subTable.subOne;
  thisLocalTable.subTable.subDoesntExist;
  ::inRootLocalTable.one;
  ::inRootLocalTable.subTable.subOne;
  ::inRootLocalTable.subTable.subDoesntExist;
  ::println("Linter 1")
}

function main() {
  ::println("Linter 1")
}
