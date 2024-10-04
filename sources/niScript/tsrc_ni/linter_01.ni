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

function invalidAccessParameter(table aLocalTable) {
  aLocalTable.four;
}

function stringmethods() string {
  "stuff".startswith("123");
  "stuff".doesntExist("123");
  return "bla"
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
  ::vmprintln("Linter 1")
}

function someTable() table {
}

function newslot() {
  local t = {}
  t.foo <- 123

  local rt = this.someTable()
  ::LINT_CHECK_TYPE("resolved_type<table>", rt)
  rt.wee <- 456

  this.stringmethods().bla <- "weee" // should fail we cant create slots in anything but tables
}

function for_dmove(string path, something) {}

// this will generate a dmove opcode
function dmove(string path) {
  local something = {}
  this.for_dmove(path,something)
}

// this will generate a dmove opcode
function prepcall(string path) {
  local className = path.GetFileNoExt()
  local loadedTable = {}
  this[this.stringmethods()](path,loadedTable)
}

function main() {
  ::vmprintln("Linter 1")
}
