// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("json.ni")
::Import("niUI")

::gConsole <- ::CreateGlobalInstance("niLang.Console").QueryInterface("iConsole")

::console <- {
  //! Console sink
  consoleSink = {
    function OnConsoleSink_NamespaceVariableChanged(aNS,aName,aValue) {
    }
    function OnConsoleSink_BeforeRunCommand(aCmd) {
      return true
    }
    function OnConsoleSink_AfterRunCommand(aCmd) {
    }
    function OnConsoleSink_BeforeRunScript(aHost,aCmd) {
      return true
    }
    function OnConsoleSink_AfterRunScript(aHost,aCmd) {
    }
  }

  //! Command sink prototype
  commandSink = {
    _name = ""
    _ns = ""
    _desc = ""
    _func = null
    _key = null
    _icon = null
    GetName = function() return _name
    GetNamespace = function() return _ns
    GetDescription = function() return _desc
    OnRun = function(aArgs,aCsl) {
      if (!_func) return false
      local r = _func(aArgs)
      return (r == null) ? true : r;
    }
  }

  _allCommands = {}

  //! Register the specified command
  function registerCommand(ns,name,desc,func,_aKey,_aIcon) {
    local cmdName = ns+"."+name;

    // Adding shortcut
    if (_aKey && ::?gUIContext) {
      local uiContext = ::LINT_AS_TYPE("iUIContext", ::?gUIContext)
      if (uiContext.shortcut_command[_aKey]) {
        ::logWarning("Binding '"+cmdName+"' to an already bound key.")
      }
      uiContext.AddShortcut(_aKey,cmdName)
    }

    local s = commandSink.Clone()
    s._name = name
    s._ns = ns
    s._desc = desc
    s._func = func
    s._key = _aKey
    s._icon = _aIcon
    ::gConsole.AddCommand(s)

    _allCommands[cmdName] <- s;
    return s;
  }

  //! Unregister the specified command
  function unregisterCommand(ns,name) {
    local cmdName = ns+"."+name;
    if (cmdName in _allCommands) {
      // ::dbg(" ... unregisterCommand found it:" ns name)
      local cmd = _allCommands[cmdName];
      if (cmd.?_key && ::?gUIContext) {
        local uiContext = ::LINT_AS_TYPE("iUIContext", ::?gUIContext)
        uiContext.RemoveShortcut(cmd.?_key)
      }
      ::gConsole.RemoveCommand(cmdName);
      delete _allCommands[cmdName];
      return true;
    }
    else {
      // ::dbg(" ... unregisterCommand didnt find it:" ns name)
      return false;
    }
  }

  function getCommandFromName(aCmdName) {
    return _allCommands[?aCmdName];
  }

  //! Namespace table
  nsTable = {
    _name = ""
    _commands = []  // array of command sink
  }

  //! Register the specifed namespace table.
  function registerNS(aName,_aFuncs,_aUnregister) {
    if (_aUnregister) {
      unregisterNS(aName,_aFuncs);
    }
    ::gConsole.AddNamespace(aName)
    foreach (key,cmd in _aFuncs) {
      if (typeof(cmd) == "table") {
        cmd = ::LINT_AS_TYPE("table:commandSink", cmd);
        this.registerCommand(aName,key,cmd.?_desc,cmd._func,cmd.?_key);
      }
    }
  }

  //! Unregister a namespace and all the functions in the specified command table
  function unregisterNS(aName,aFuncs) {
    if (aFuncs) {
      foreach (key,cmd in aFuncs) {
        if (typeof(cmd) != "table")
          continue
        unregisterCommand(aName,key)
      }
    }
    ::gConsole.RemoveNamespace(aName)
  }

  //--------------------------------------------------------------------------------------------
  //
  //  Commands
  //
  //--------------------------------------------------------------------------------------------
  function runCommand(cmd) {
    if (::gConsole.RunCommand(cmd)) {
      return ::gConsole.GetVariable("result")
    }
    else {
      return ""
    }
  }

  function pushCommand(cmd) {
    ::gConsole.PushCommand(cmd)
  }
}
