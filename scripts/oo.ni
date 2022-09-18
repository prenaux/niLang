// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("assert.ni")

::oo <- {
  base = {
    function new(_args_) {
      local __lint = { this_key_notfound_callk = 0 }
      local t = this.Clone()
      t.__SetCanCallMetaMethod(true)
      if ("_initialize" in t) {
        local args = [t];
        if (_args_ && _args_.GetSize()) {
          args % _args_;
        }
        t._initialize.acall(args)
      }
      return t
    }
  }

  function createClass(aClassName, aTable, aBaseTable) {
    ::assertType("table", aTable, "Class definition table");
    ::assertHasnt("_className", aTable, "Class definition table");
    aTable._className <- ::assertName(aClassName, "Class definition name");
    return ::delegate(aBaseTable || base, aTable);
  }
}
