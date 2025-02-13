::Import("niLang")

local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

tSquirrel <- {
  _nutzStash = []

  function getNuts() {
    return ["itsy","nutty","qooty"]
  }
  function stashNuts() {
    return false
  }
}

tNutsProcessor <- {
  function countMyNuts() {
    ::LINT_CHECK_TYPE("table:tNutsProcessor",this);
    ::LINT_THIS_AS_TYPE("tSquirrel");
    ::LINT_CHECK_TYPE("table:tSquirrel",this);
    local numNuts = this.getNuts().?len() || 0;
    numNuts += this._nutzStash.?len() || 0;
    return numNuts;
  }

  function countInvalidNuts() {
    ::LINT_CHECK_TYPE("table:tNutsProcessor",this);
    ::LINT_THIS_AS_TYPE("tSquirrel");
    ::LINT_CHECK_TYPE("table:tSquirrel",this);
    this.getThatsNotAThing();
    return this.laserSaber;
  }
}

function main() {
  local rod = tSquirrel.ShallowClone()
  local nutz1 = tNutsProcessor.countMyNuts.call(rod)
  local nutz2 = tNutsProcessor.countMyNuts.acall([rod])
  ::vmprintln("nutz1: " + nutz1)
  ::vmprintln("nutz2: " + nutz2)
}
