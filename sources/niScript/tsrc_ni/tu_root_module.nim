::Import("unittest.ni")

::name <- "_tu_root_module_name"
::name2 <- "_tu_root_module_name2"

::tSomeTable <- {
  function GetRootName() {
    return ::name2
  }
}

module <- {
  function GetRootName() {
    return ::name
  }

  function createSomeTable() {
    return ::tSomeTable.Clone()
  }
}

::CHECK(::?tSomeTable != null);
::CHECK("tSomeTable" in ::getroottable());
