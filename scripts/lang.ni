// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("niLang");
::Import("niScript");

// Get the Lang instance.
::gLang <- ::CreateInstance("niLang.Lang")

::CreateGlobalInstance <- function(aObjID,avarA,avarB) {
  local o = ::gLang.global_instance[aObjID];
  if (o)
    return o
  o = ::CreateInstance(aObjID,avarA,avarB)
  if (!o)
    return null
  ::gLang.global_instance[aObjID] = o
  return o
}

// Get the Math instance.
::gMath <- ::CreateGlobalInstance("niLang.Math")

// Set the delegate on a table and then return it
::delegate <- function(aTableToSetAsDelegate,aTable) {
  aTable.SetDelegate(aTableToSetAsDelegate)
  return aTable
}

// Create a namespace and merge the specified table
::namespace <- function(name,table,nsTable) {
  nsTable = nsTable || ::getvmroottable()
  local toks = name.split(".")
  foreach (t in toks) {
    // ::dbg("... " + t)
    if (!nsTable.Has(t)) {
      nsTable[t] <- {}
    }
    nsTable = nsTable[t]
  }
  if (table) {
    local initialize = table.?_initialize
    nsTable.merge(table)
    if (initialize) {
      initialize.call(nsTable)
    }
  }
  return nsTable
}

// Create a vector container, see ni::CreateContainerVector
::Vector <- function(aType) {
  if (typeof(aType) == "string") {
    aType = ::eType[aType]
  }
  return ::CreateCollectionVector(aType)
}

// Create a map container, see ni::CreateContainerMap
::Map <- function(aKeyType,aValueType) {
  if (typeof(aKeyType) == "string") {
    aKeyType = ::eType[aKeyType]
  }
  if (typeof(aValueType) == "string") {
    aValueType = ::eType[aValueType]
  }
  return ::CreateCollectionMap(aKeyType,aValueType)
}

// Get the main thread id
::GetMainThreadID <- function() {
  // patch the GetMainThreadID function in function of the type of VM we're running
  if ("Concurrent_GetMainThreadID" in ::getroottable()) {
    ::GetMainThreadID <- ::Concurrent_GetMainThreadID
  }
  else {
    ::logWarning("Trying to get the main thread id with a single threaded VM.")
    ::GetMainThreadID <- function() { return 0; }
  }
  return ::GetMainThreadID()
}

// Get the current thread id
::GetCurrentThreadID <- function() {
  // patch the GetCurrentThreadID function in function of the type of VM we're running
  if ("Concurrent_GetCurrentThreadID" in ::getroottable()) {
    ::GetCurrentThreadID <- ::Concurrent_GetCurrentThreadID
  }
  else {
    ::logWarning("Trying to get the current thread id with a single threaded VM.")
    ::GetCurrentThreadID <- function() { return 0; }
  }
  return ::GetCurrentThreadID()
}

// Create a DataTable, see ni::iSystem::CreateDataTable
::DataTable <- function(aName,aInitWith) {
  if (typeof aName == "table") {
    aInitWith = aName
    aName = ""
  }
  aName = (aName != null) ? aName : "NoName"
  local dt = ::gLang.CreateDataTable(aName)
  if (aInitWith) {
    dt % aInitWith
  }
  return dt
}
::DT <- ::DataTable

//--------------------------------------------------------------------------------------------
//
//  String utils
//
//--------------------------------------------------------------------------------------------
{
  local d = ::GetLangDelegate("string")
  if (!d) throw "Can't get 'string' LangDelegate."
  d.GetNumChars <- #() {
    local it = this.CreateCharIt(0)
    return it.num_chars
  }
  d.quote <- function() {
    local str = this;
    return "\"" + str + "\""
  }
  d.unquote <- function() {
    local str = this;
    if (str.startswith("\"")) {
      str = str.slice(1)
    }
    if (str.endswith("\"")) {
      str = str.slice(0,-1)
    }
    return str;
  }
  d.replace <- function(aSearchFor, aReplaceBy) {
    if (typeof aSearchFor == "table") {
      local s = this;
      foreach (k,v in aSearchFor) {
        s = s.replace(k,v);
      }
      return s;
    }
    local subStrLen = aSearchFor.?len();
    if (!subStrLen) {
      return this;
    }
    local startIndex = 0;
    local o = "";
    for (;;) {
      local p = this.find(aSearchFor,startIndex);
      if (p == null) {
        break;
      }
      o += this.slice(startIndex,p);
      o += aReplaceBy;
      // ::println("FOUND" aSearchFor "AT" p)
      startIndex = p + subStrLen;
    }

    o += this.slice(startIndex);
    return o;
  }
  d.occurCount <- function(aSearchFor) {
    local subStrLen = aSearchFor.?len();
    if (!subStrLen) {
      return this;
    }
    local occurCount = 0
    local startIndex = 0;
    for (;;) {
      local p = this.find(aSearchFor,startIndex);
      if (p == null) {
        break;
      }
      ++occurCount;
      startIndex = p + subStrLen;
    }
    return occurCount;
  }
}

//--------------------------------------------------------------------------------------------
//
//  Array as iMutableCollection
//
//--------------------------------------------------------------------------------------------
{
  local d = ::GetLangDelegate("array")
  if (!d) throw "Can't get 'array' LangDelegate."
  d.Clear <- d.clear
  d.Add <- d.append
  d.Reserve <- d.reserve
  d.GetSize <- d.len
  d.Put <- d.insert
  d.IsEmpty <- d.empty
  d.back <- d.top
  d.size <- d.len
  d.Clone <- d.DeepClone

  {
    local wasReverse = d.reverse
    d.reverse = function () : (wasReverse) {
      local _this = this;
      wasReverse.call(_this);
      return _this
    }
  }

  // Join all elements of an array into a string
  d.join <- function (aSeparator,aToString) {
    aSeparator = aSeparator || " ";
    aToString = aToString || ::lang.toString;
    local r = "";
    local lastValidIndex = this.GetSize()-1;
    foreach (i,v in this) {
      r += aToString(v);
      if (aSeparator && i != lastValidIndex) {
        r += aSeparator;
      }
    }
    return r;
  }

  // Join all elements of an array into a text, same as the regular join with
  // spaces but it takes into account punctuation to decide whether or not to
  // insert a space.
  d.joinText <- function(aFirst,aLast,aToString) {
    aFirst = aFirst || 0;
    aLast = aLast || this.len();
    aToString = aToString || ::lang.toString;
    local r = "";
    for (local i = aFirst; i < aLast; ++i) {
      local v = aToString(this[i]);
      local c = v[?0];
      if (i != aFirst && c != ' ' && c != ',' && c != ':') {
        r += " ";
      }
      r += v;
    }
    return r;
  }
}

//--------------------------------------------------------------------------------------------
//
//  Table as iMutableCollection
//
//--------------------------------------------------------------------------------------------
{
  local d = ::GetLangDelegate("table")
  if (!d) throw "Can't get 'table' LangDelegate."
  d.Clear <- d.clear
  d.Add <- function(e) {
    this[e[0]] <- e[1]
    return this
  }
  d.Reserve <- function() { return this }
  d.GetSize <- d.len
  d.Put <- function(k,v) { this[k] <- v }
  d.IsEmpty <- d.empty
  d.Clone <- d.DeepClone
  d.Has <- function(k) { return (k in this); }
  d.GetKeys <- function(arr) {
    arr = arr || [];
    foreach (k,v in this) {
      arr.append(k);
    }
    return arr;
  }
  d.GetValues <- function(arr) {
    arr = arr || [];
    foreach (k,v in this) {
      arr.append(v);
    }
    return arr;
  }
}

//--------------------------------------------------------------------------------------------
//
//  Vec4
//
//--------------------------------------------------------------------------------------------
{
  local d = ::GetLangDelegate("vec4f")
  if (!d) throw "Can't get 'vec4f' LangDelegate."
  d.ToRectString <- function() {
    return "Rect(" + x + "," + y + "," + width + "," + height + ")"
  }
}

//--------------------------------------------------------------------------------------------
//
//  Merge
//
//--------------------------------------------------------------------------------------------
{
  local doMergeInArray = function(aColl) {
    switch (typeof aColl) {
      case "array":
      this.extend(aColl)
      break;
      case "iunknown":
      if (!aColl.QueryInterface("iCollection")) {
        this.Add(aColl)
        break;
      }
      // fallthrough to iteration
      case "table":
      foreach (v in aColl) {
        this.Add(v)
      }
      break;
      // aColl is a value type...
      default:
      this.Add(aColl)
      break;
    }
    return this
  }

  local doMergeInTable = function(aColl) {
    foreach (k,v in aColl) {
      this[k] <- v
    }
    return this
  }

  local doMergeInCollection = function(aColl) {
    local isTable =
      ((typeof aColl) == "table") || (aColl.?key_type);
    if (isTable) {
      // collection is some kind of map
      foreach (k,v in aColl) {
        this.Put(k,v)
      }
    }
    else {
      // collection is a vector, list or set
      switch (typeof aColl) {
        case "iunknown":
        if (!aColl.QueryInterface("iCollection")) {
          this.Add(aColl);
          break;
        }
        // fallthrough to iteration
        case "array":
        case "table":
        foreach (v in aColl) {
          this.Add(v)
        }
        break;
        default:
        this.Add(aColl)
        break;
      }
    }
    return this
  }

  local d = ::GetLangDelegate("array")
  if (!d) throw "Can't get LangDelegate 'array'"
  d.merge <- doMergeInArray
  d._modulo <- doMergeInArray

  local d = ::GetLangDelegate("table")
  if (!d) throw "Can't get LangDelegate 'table'"
  d.merge <- doMergeInTable
  d._modulo <- doMergeInTable

  local d = ::GetLangDelegate("iunknown:iCollection")
  if (!d) throw "Can't get LangDelegate 'iunknown:iCollection'"
  d.merge <- doMergeInCollection
  d.len <- d.GetSize
  d._modulo <- doMergeInCollection
  d._clone <- function() {
    local c
    if (this.?key_type) {
      c = ::Map(this.key_type,this.value_type)
    }
    else {
      c = ::Vector(this.value_type)
    }
    c.Copy(this)
    return c
  }

  d.ToString <- function() {
    local isTable = this.?key_type;
    local s = "Coll"
    if (isTable) {
      s += "{"
      local idx = 0
      foreach (k,v in this) {
        if (idx) s += ","
        s += k + "=" + v
        ++idx
      }
      s += "}"
    }
    else {
      s += "["
      local idx = 0
      foreach (v in this) {
        if (idx) s += ","
        s += v
        ++idx
      }
      s += "]"
    }
    return s
  }
}

//--------------------------------------------------------------------------------------------
//
//  Enum
//
//--------------------------------------------------------------------------------------------
{
  ::FlagsToString <- function (aValue, aEnum, aFlags) {
    return ::EnumToString(aValue, aEnum, aFlags | 2)
  }
  ::StringToFlags <- function (aValue, aEnum, aFlags) {
    return ::StringToEnum(aValue, aEnum, aFlags | 2)
  }

  local d = ::GetLangDelegate("enum")
  if (!d) throw "Can't get SystemDelegate 'enum'"
  d.GetTable <- d.gettable
  d.GetName <- d.enumName
}

//--------------------------------------------------------------------------------------------
//
//  DataTable
//
//--------------------------------------------------------------------------------------------
{
  local d = ::GetLangDelegate("iunknown:iDataTable")
  // if (!d) throw "Can't get SystemDelegate 'iunknown:iDataTable'"
  if (d) {
    d.merge <- function(aColl) {
      foreach (k,v in aColl) {
        if (typeof v == "table") {
          local cdt = this.child[k]
          if (!cdt) cdt = ::gLang.CreateDataTable(k)
          this.AddChild(cdt)
          cdt.merge(v)
        }
        else if (typeof v == "array") {
          this.var[k.ToString()] = ::lang.toString(v)
        }
        else {
          this.var[k.ToString()] = v
        }
      }
      return this
    }
    d._modulo <- d.merge
    d.ToString <- function(aMode) {
      return ::lang.dataTableToString(this,aMode || "xml")
    }

    local function dtGetStringPropertyToEnum(dt,pname,aEnumDef,aEnumToStringFlags,aDefaultValue) {
      local pindex = dt.property_index[pname]
      if (pindex == invalid)
        return aDefaultValue;
      local v = dt.var_from_index[pindex];
      switch (typeof v) {
        case "string": {
          return ::StringToEnum(v,aEnumDef,aEnumToStringFlags);
        }
        case "int":
        case "float": {
          return v.toint();
        }
        default: {
          return aDefaultValue;
        }
      }
    }

    local function dtSetEnumPropertyToString(dt,pname,aEnumDef,aEnumToStringFlags,aSetValue) {
      if (aEnumDef) {
        dt.string[pname] = ::EnumToString(aSetValue,aEnumDef,aEnumToStringFlags)
      }
      else {
        dt.int[pname] = aSetValue
      }
    }

    d.SetEnum = function(pname,enumDef,aValue) : (dtSetEnumPropertyToString) {
      return dtSetEnumPropertyToString(this,pname,enumDef,0,aValue)
    }
    d.SetFlags <- function(pname,enumDef,aValue) : (dtSetEnumPropertyToString) {
      return dtSetEnumPropertyToString(this,pname,enumDef,2,aValue)
    }

    d.GetEnumDefault = function(pname,enumDef,aDefault) : (dtGetStringPropertyToEnum) {
      return dtGetStringPropertyToEnum(this,pname,enumDef,0,aDefault)
    }
    d.GetFlagsDefault <- function(pname,enumDef,aDefault) : (dtGetStringPropertyToEnum) {
      return dtGetStringPropertyToEnum(this,pname,enumDef,2,aDefault)
    }

    local dr = ::GetLangDelegate("iunknown:iDataTableReadStack")
    dr.GetEnumDefault = function(pname,enumDef,aDefault) : (dtGetStringPropertyToEnum) {
      return dtGetStringPropertyToEnum(this.top,pname,enumDef,0,aDefault)
    }
    dr.GetFlagsDefault <- function(pname,enumDef,aDefault) : (dtGetStringPropertyToEnum) {
      return dtGetStringPropertyToEnum(this.top,pname,enumDef,2,aDefault)
    }

    local dw = ::GetLangDelegate("iunknown:iDataTableWriteStack")
    dw.SetEnum = function(pname,enumDef,aValue) : (dtSetEnumPropertyToString) {
      return dtSetEnumPropertyToString(this.top,pname,enumDef,0,aValue)
    }
    dw.SetFlags <- function(pname,enumDef,aValue) : (dtSetEnumPropertyToString) {
      return dtSetEnumPropertyToString(this.top,pname,enumDef,2,aValue)
    }
  }
}

//--------------------------------------------------------------------------------------------
//
//  ::lang
//
//--------------------------------------------------------------------------------------------
::lang <- {

  _vmprint = ::print
  _vmprintln = ::println
  _isEmbedded = null

  ///////////////////////////////////////////////
  function buildAppModulePath(aName,aAppDir) {
    return "".setdir(aAppDir).adddirback("bin/"+::lang.getLOA()).setfile(::GetModuleFileName(aName));
  }

  ///////////////////////////////////////////////
  //! Load a module definition.
  //! \remark Does not import the module in the script vm.
  function loadModuleDef(aName,aAppDir) {
    local mod = ::gLang.module_def[::gLang.module_def_index[aName]]
    if (!mod) {
      aAppDir = aAppDir || ::gLang.property["ni.dirs.app"];
      local modulePath = buildAppModulePath(aName,aAppDir);
      // ::println("... ::lang.loadModuleDef:" modulePath);
      // Once a native module has been loaded with LoadModuleDef the module is
      // registered and available for regular ::Import.
      mod = ::gLang.LoadModuleDef(aName, modulePath)
      if (!mod) {
        throw "Can't load module '"+aName+"'."
      }
    }
    return mod
  }

  ///////////////////////////////////////////////
  function getArch() string {
    return ::gLang.property["ni.loa.arch"]
  }

  ///////////////////////////////////////////////
  function getLOA() string {
    return ::gLang.property["ni.loa.os"] + "-" + ::gLang.property["ni.loa.arch"]
  }

  ///////////////////////////////////////////////
  function getTitleBuildDesc() string {
    local buildDesc = " ("
    buildDesc += ::gLang.property["ni.loa.arch"]
    if (::gLang.property["ni.loa.build"][?0] == 'd') {
      buildDesc += " Debug"
    }
    if (::?gGraphicsContext) {
      buildDesc += " " + ::gGraphicsContext.driver.name
    }
    buildDesc += ")"
    return buildDesc
  }

  ///////////////////////////////////////////////
  function getHostOS() string {
    return ::gLang.property["ni.loa.os"]
  }

  ///////////////////////////////////////////////
  function getExeExtension() {
    local os = ::gLang.property["ni.loa.os"]
    return (os == "nt") ? "exe" : "";
  }

  ///////////////////////////////////////////////
  function getProperty(string aKey, string aDefault) string {
    local v = ::gLang.property[aKey]
    if (!(v.?len()))
      return aDefault
    return v
  }

  ///////////////////////////////////////////////
  function getPropertyPath(string aKey, string aDefault) string {
    local v = ::gLang.property[aKey]
    if (!(v.?len()))
      return ::lang.urlDecode(aDefault)
    return ::lang.urlDecode(v)
  }

  ///////////////////////////////////////////////
  function getPropertyBool(string aKey, bool aDefault) bool {
    local v = ::gLang.property[aKey]
    if (!(v.?len()))
      return aDefault
    return ::lang.toBool(v)
  }

  ///////////////////////////////////////////////
  function isEmbedded() bool {
    if (_isEmbedded == null) {
      _isEmbedded = false;
      local os = getHostOS();
      if (os == "adr" ||
          os == "ios" ||
          os.icontains("Embedded"))
        _isEmbedded = true;
    }
    return _isEmbedded
  }
  function isDesktop() bool {
    local os = getHostOS();
    return !isEmbedded() || os.icontains("(EmbeddedEmu)");
  }

  ///////////////////////////////////////////////
  function implements(obj,interface) {
    switch (typeof(obj)) {
      case "iunknown":
      return obj.QueryInterface(interface);
    }
    return null
  }

  ///////////////////////////////////////////////
  function toBool(v,aDefaultIfNull) {
    if (v == null) {
      return aDefaultIfNull;
    }
    if (typeof v == "string") {
      if (v.IEq("no") ||
          v.IEq("false") ||
          v.IEq("null") ||
          v.IEq("0"))
        return null;
    }
    if (!v) {
      return null;
    }
    return true;
  }

  ///////////////////////////////////////////////
  function isValid(obj) {
    switch (typeof(obj)) {
      case "iunknown":
      return obj.isvalid();
    }
    return false
  }

  ///////////////////////////////////////////////
  function isValidGlobal(name) {
    if (!(name in ::getroottable()))
      return false;
    return ::lang.isValid(::getroottable()[name])
  }

  ///////////////////////////////////////////////
  function isDebugVM() {
    return ::gLang.property["ni.loa.build"] == "da"
  }

  ///////////////////////////////////////////////
  // Calls a potentially long garbage collection
  function gc()
  {
    local collected = ::collectgarbage()
    if (collected) {
      ::lang.log("GC: " + collected)
    }
    return collected
  }

  ///////////////////////////////////////////////
  function checkInterface(aObject,aInterface)
  {
    local q = aObject.QueryInterface(aInterface);
    return (q != null)
  }

  ///////////////////////////////////////////////
  function unboxArgs(aArgs) {
    return (typeof(aArgs[?0]) == "array" && aArgs.len() == 1) ? aArgs[0] : aArgs;
  }

  ///////////////////////////////////////////////
  // Build a printable string
  function toPrintString(aMsg) {
    switch (typeof aMsg) {
      case "string": {
        return aMsg;
      }
      case "array": {
        return aMsg.joinText();
      }
      default: {
        return ::lang.toString(aMsg);
      }
    }
  }

  ///////////////////////////////////////////////
  function print(_args_) {
    ::lang._vmprint(::lang.toPrintString(_args_))
  }
  function println(_args_) {
    ::lang._vmprintln(::lang.toPrintString(_args_))
  }

  ///////////////////////////////////////////////
  // Log a message
  function logEx(aType,aMsg,aSILevel)
  {
    local src = "unknown"
    local func = "unknown"
    local line = -1
    local si = ::getbasestackinfos(aSILevel)
    if (si) {
      src = si.src
      func = si.func
      line = si.line
    }
    aMsg = ::lang.toPrintString(aMsg)
    ::gLang.Log(aType,src,func,line,aMsg);
  }

  function log(_args_) {
    ::lang.logEx(::eLogFlags.Info,_args_,3);
  }
  function dbg(_args_) {
    ::lang.logEx(::eLogFlags.Debug,_args_,3);
  }
  function logError(_args_) {
    ::lang.logEx(::eLogFlags.Error,_args_,3);
  }
  function logWarning(_args_) {
    ::lang.logEx(::eLogFlags.Warning,_args_,3);
  }

  function dbgs(_args_) {
    local src = "unknown"
    local func = "unknown"
    local line = -1
    local msg
    msg = ::lang.toPrintString(_args_)
    msg += "\n--- stack ---\n"
    for (local i = 2; i < 25; ++i) {
      local si = ::getbasestackinfos(i)
      if (!si) break;
      if (i == 2) {
        src = si.src
        func = si.func
        line = si.line
      }
      msg += ::format("%s(%d): [%s]\n",si.src,si.line,si.func)
    }
    msg += "-----------\n"

    ::gLang.Log(::eLogFlags.Debug,src,func,line,msg);
  }

  ///////////////////////////////////////////////
  function checkIUnknown(aObject,aInterfaces) {
    if (typeof(aObject) != "iunknown")
      throw "Object not iUnknown."
    foreach (i in aInterfaces) {
      if (!aObject.QueryInterface(i))
        throw "Object doesn't implement interface '"+aObject+"'."
    }
  }

  ///////////////////////////////////////////////
  function getCurrentProcess() {
    return ::gLang.process_manager.current_process
  }

  ///////////////////////////////////////////////
  function toQuotedString(str) {
    local theStr = str
    if (theStr.contains("\"") ||
        theStr.contains("\\") ||
        theStr.contains("\n"))
    {
      return "{[" + theStr + "]}"
    }
    else {
      return "\"" + theStr + "\""
    }
  }

  ///////////////////////////////////////////////
  function toValueString(v,aDefaultHandler,aRecursionGuard,aLimitToString) {
    if (typeof v == "string") {
      return toQuotedString(v)
    }
    return toString(v,aDefaultHandler,aRecursionGuard,aLimitToString)
  }

  ///////////////////////////////////////////////
  function toString(v,aDefaultHandler,aRecursionGuard,aLimitToString) {
    local s = ""
    switch (typeof(v)) {
      case "function": {
        s += "fun()"
        break
      }
      case "string": {
        s += v
        break
      }
      case "array": {
        aRecursionGuard = aRecursionGuard || {}
        local guardId = v.ToIntPtr()
        if (guardId in aRecursionGuard) {
          s += ::format("[:RECURSIVE:%d]", guardId)
        }
        else {
          aRecursionGuard[guardId] <- true
          s += "["
          foreach (idx,a in v) {
            if (aLimitToString && idx > 30) {
              s += "..."
              break
            }
            else if (idx != 0) {
              s += ","
            }
            if (aLimitToString && s.len() > 150) {
              s += "...";
              break;
            }
            s += ::lang.toValueString(a,aDefaultHandler,aRecursionGuard,aLimitToString)
          }
          if (s.endswith(","))
            s = s.slice(0,s.len()-1)
          s += "]"
        }
        break;
      }
      case "table": {
        aRecursionGuard = aRecursionGuard || {}
        local guardId = v.ToIntPtr()
        if (guardId in aRecursionGuard) {
          s += ::format("{:RECURSIVE:%d}", guardId)
        }
        else {
          aRecursionGuard[guardId] <- true
          s += "{"
          local idx = 0
          foreach (k,a in v) {
            if (aLimitToString && idx > 30) {
              s += "..."
              break
            }
            else if (idx != 0)  {
              s += ","
            }
            if (aLimitToString && s.len() > 150) {
              s += "...";
              break;
            }
            s += ::lang.toString(k,aDefaultHandler,aRecursionGuard,aLimitToString)
            s += "="
            if (aLimitToString && s.len() > 150) {
              s += "...";
              break;
            }
            s += ::lang.toValueString(a,aDefaultHandler,aRecursionGuard,aLimitToString)
            ++idx
          }
          if (s.endswith(","))
            s = s.slice(0,s.len()-1); // remove the end comma
          s += "}"
        }
        break;
      }
      case "iunknown": {
        if (v.?ToString) {
          s += v.ToString()
          break;
        }
      }
      default: {
        if (aDefaultHandler) {
          if (aLimitToString && s.len() > 150) {
            s += "...";
            break;
          }
          s += aDefaultHandler(v)
        }
        else {
          local rem = ::SetRaiseErrorMode(0)
          try {
            s += v
          }
          catch (e) {
            s += typeof v
          }
          ::SetRaiseErrorMode(rem)
        }
        break
      }
    }
    return s
  }

  ///////////////////////////////////////////////
  function getTimeMinutesString(aTime) {
    local time = aTime || ::gLang.current_time
    return ::format("%04d%02d%02d_%02d%02d",time.year,time.month,time.day,time.hour,time.minute)
  }
  function getTimeSecondsString(aTime) {
    local time = aTime || ::gLang.current_time
    return ::format("%04d%02d%02d_%02d%02d%02d",
                    time.year,time.month,time.day,
                    time.hour,time.minute,time.second)
  }

  ///////////////////////////////////////////////
  function eval(aExpr) {
    if (aExpr[?0] == '`') {
      // quoted string, just unquote it instead of making it go through the full evaluator
      return aExpr.slice(1);
    }
    local v = ::gLang.Eval(aExpr)
    switch (v.type) {
      case 0: {
        // eExpressionVariableType_Float = 0
        return v.float;
      }
      case 1: {
        // eExpressionVariableType_Vec2 = 1
        return v.vec2;
      }
      case 2: {
        // eExpressionVariableType_Vec3 = 2
        return v.vec3;
      }
      case 3: {
        // eExpressionVariableType_Vec4 = 3
        return v.vec4;
      }
      case 4: {
        // eExpressionVariableType_Matrix = 4
        return v.matrix;
      }
      case 5: {
        // eExpressionVariableType_String = 5
        return v.string;
      }
    }
  }

  ///////////////////////////////////////////////
  function createDataTableXML(aXMLString)
  {
    local dt = ::gLang.CreateDataTable("")
    local fp = ::fs.createStringFile(aXMLString)
    if (!::gLang.SerializeDataTable("xml",::eSerializeMode.Read,dt,fp))
      throw "Can't read the data table from the specified xml string.";
    return dt
  }

  ///////////////////////////////////////////////
  function dataTableFromString(aString,aType) {
    aType = aType || "xml"
    return ::lang.loadDataTable(aType,::fs.createStringFile(aString))
  }

  ///////////////////////////////////////////////
  // Load a data table
  function loadDataTable(aType,aPath,aDT)
  {
    local fp = ::lang.urlOpen(aPath)
    if (!fp)
      throw "Can't open file '"+aPath+"'."

    local dt = aDT ? aDT : ::gLang.CreateDataTable("")
    if (!::gLang.SerializeDataTable(aType,::eSerializeMode.ReadRaw,dt,fp))
      throw "Can't read data table from '"+aPath+"'."

    return dt
  }

  ///////////////////////////////////////////////
  // Write a data table
  function writeDataTable(aType,aDT,aPath)
  {
    local fp = ::fs.fileOpenWrite(aPath)
    if (!fp) throw "Can't open file '"+aPath+"'."

    if (!::gLang.SerializeDataTable(aType,::eSerializeMode.Write,aDT,fp))
      throw "Can't write data table to '"+aPath+"'."
  }
  function dataTableToString(aDT,aMode)
  {
    aMode = aMode || "xml-stream"
    local fp = ::gLang.CreateFileDynamicMemory(4096,"")
    if (!::gLang.SerializeDataTable(aMode,::eSerializeMode.Write,aDT,fp))
      throw "Can't write data table."
    fp.SeekSet(0);
    return fp.ReadString();
  }

  ///////////////////////////////////////////////
  // Convert a datatable to a table
  // Adds a __tables property which is an array of the subtables
  // Adds a __name property which is the datatable's name
  // Adds a set of child tables which have the same name as in the datatable
  // Thus child tables can be accessed by name using parentDT.CHILD_TABLE_NAME, or by index using parentDT.__tables[CHILD_TABLE_INDEX]
  // in case of conflict - property name overriding a table name or several copy of child tables with the same name - use
  // the tables array.
  function convertDataTableToTable(aDT) {
    function doConvert(aDT,aTable) {
      if (!aDT)
        return;
      if (aDT.num_children > 0) {
        aTable["__tables"] <- []
        local newTableCount = 0
        for (local i = 0; i < aDT.num_children; ++i) {
          local newTable = {}
          doConvert(aDT.child_from_index[i],newTable)
          aTable.__tables.append(newTable)
          aTable[aDT.child_from_index[i].name] <- newTable
        }
      }
      aTable["__name"] <- aDT.name
      for (local i = 0; i < aDT.num_properties; ++i) {
        local type = aDT.property_type_from_index[i]
        local name = aDT.property_name[i]
        switch (type) {
          case ::eDataTablePropertyType.String: aTable[name] <- aDT.string_from_index[i]; break
          case ::eDataTablePropertyType.Int32:
          case ::eDataTablePropertyType.Int64:  aTable[name] <- aDT.int_from_index[i];  break
          case ::eDataTablePropertyType.Bool:   aTable[name] <- aDT.bool_from_index[i]; break
          case ::eDataTablePropertyType.Float32:
          case ::eDataTablePropertyType.Float64: aTable[name] <- aDT.float_from_index[i]; break
          case ::eDataTablePropertyType.Vec2:   aTable[name] <- aDT.vec2_from_index[i]; break
          case ::eDataTablePropertyType.Vec3:   aTable[name] <- aDT.vec3_from_index[i]; break
          case ::eDataTablePropertyType.Vec4:   aTable[name] <- aDT.vec4_from_index[i]; break
          case ::eDataTablePropertyType.Matrix: aTable[name] <- aDT.matrix_from_index[i]; break
          case ::eDataTablePropertyType.IUnknown: aTable[name] <- aDT.iunknown_from_index[i]; break
          default:                aTable[name] <- aDT.var_from_index[i]; break
        }
      }
    }
    local newTable = {}
    doConvert(aDT,newTable)
    return newTable
  }

  ///////////////////////////////////////////////
  // Convert a table to a datatable
  // If a __name property exists and aName is null sets that property's value as the datatable's name
  // If a __tables property exists serialize those first and makes sure those tables arent serialized twice
  //    rem for this to work the extra added tables need to have a name which is different of all tables in the array
  function convertTableToDataTable(aTable,aName) {
    if (!aName) {
      if ("__name" in aTable) {
        aName = aTable["__name"]
      }
      else {
        aName = "Unnamed"
      }
    }
    local dt = ::gLang.CreateDataTable(aName)
    local subTables = null
    if ("__tables" in aTable) {
      subTables = {}
      foreach (t in aTable.__tables) {
        local childDT = convertTableToDataTable(t,null)
        dt.AddChild(childDT)
        subTables[childDT.name] <- 1
      }
    }
    foreach (k,v in aTable) {
      if (k.startswith("__")) // skip 'hidden/reserved' properties
        continue
      switch(typeof(v)) {
        case "string":  dt.string[k] = v; break
        case "int": dt.int[k] = v;  break
        case "float": dt.float[k] = v;  break
        case "Vec2":  dt.vec2[k] = v; break
        case "Vec3":  dt.vec3[k] = v; break
        case "Vec4":  dt.vec4[k] = v; break
        case "Matrix":  dt.matrix[k] = v; break
        case "iunknown":  dt.iunknown[k] = v; break
        case "table": {
          if (!subTables || !(k in subTables)) {
            local childDT = convertTableToDataTable(v,k)
            dt.AddChild(childDT)
            if (subTables) subTables[childDT.name] <- 1
          }
          break;
        }
        case "array": {
          foreach (idx,e in v) {
            local childDT = convertTableToDataTable(e,k)
            dt.AddChild(childDT)
          }
          break;
        }
        default: {
          local strVal = ""+v
          dt.string[k] = strVal
          break;
        }
      }
    }
    return dt
  }

  ///////////////////////////////////////////////
  // Load a table from a datatable file
  function loadTable(aType,aPath)
  {
    local fp = ::lang.urlOpen(aPath)
    if (!fp) throw "Can't open file '"+aPath+"'."
    local dt = ::gLang.CreateDataTable("")
    if (!::gLang.SerializeDataTable(aType,::eSerializeMode.ReadRaw,dt,fp))
      throw "Can't read data table from '"+aPath+"'."
    return convertDataTableToTable(dt)
  }

  ///////////////////////////////////////////////
  // Write a table in a datatable file
  function writeTable(aType,aTable,aPath)
  {
    local fp = ::fs.fileOpenWrite(aPath)
    if (!fp) throw "Can't open file '"+aPath+"'."

    local dt = convertTableToDataTable(aTable,null)

    if (!::gLang.SerializeDataTable(aType,::eSerializeMode.WriteRaw,dt,fp))
      throw "Can't write data table to '"+aPath+"'."

    return true
  }

  ///////////////////////////////////////////////
  function urlOpen(aURL) {
    if (typeof(aURL) == "iunknown") {
      local fp = aURL.QueryInterface("iFile")
      if (!fp || !(fp.file_flags&::eFileFlags.Read)) {
        return null
      }
      return fp
    }
    return ::gLang.URLOpen(aURL)
  }
  function urlExists(aURL) {
    return ::gLang.URLExists(aURL)
  }
  function urlDecode(aURL) {
    if (aURL.?startswith("url://")) {
      aURL = aURL.after("url://")
      aURL = aURL.DecodeUrl()
    }
    return aURL
  }
  function urlEncode(aURL) {
    if (aURL.startswith("url://")) {
      return aURL
    }
    return "url://" + aURL.EncodeUrl()
  }

  function newClosure(aThis,aFunction) {
    return function(_args_) : (aThis,aFunction) {
      return aFunction.athiscall(aThis,_args_);
    }
  }
}

::log <- ::lang.log
::logWarning <- ::lang.logWarning
::logError <- ::lang.logError
::trace <- ::lang.dbg
::dbg <- ::lang.dbg
::dbgs <- ::lang.dbgs
::print <- ::lang.print
::println <- ::lang.println
::isValid <- ::lang.isValid
::cat <- ::lang.toPrintString
// default empty main...
if (!("main" in ::getroottable())) {
  ::main <- function() {}
}

::interop <- {
  function doEval(path,iid,defaultDelegateTable) {
    local className = path.GetFileNoExt()
    local loadedTable = {}
    ::Import(path,loadedTable)
    local implTable = loadedTable[?className]
    if (!implTable) {
      if (defaultDelegateTable) {
        loadedTable.SetDelegate(defaultDelegateTable)
      }
      implTable = loadedTable
    }
    return ::QueryInterface(implTable,iid)
  }
  function evalImpl(aContext,aPath,aIID) {
    if (!(aContext in ::interop))
      throw "Unknown context '"+aContext+"'."
    return ::interop[aContext](aPath,aIID)
  }
}

// Import a native module
::ImportNative <- function(aName, aDir) {
  local moduleDef = ::lang.loadModuleDef(aName,aDir);
  ::Import(aName);
  return moduleDef;
}

// Import a script module
tImportModule <- {
  _cache = {}

  function _createNewRootTable(aCurrentRoot) {
    aCurrentRoot = aCurrentRoot || ::getroottable();
    local newRoot = ::delegate(aCurrentRoot, {});
    // override getroottable since the parent will return the vm's root table
    newRoot.getroottable = function() {
      // Use return this so that we dont need to use a free variable which would
      // create a strong reference to it. This prevent us from creating a cycle
      // that we'd have to break with an explicit Invalidate() or by making sure
      // they are no more references and triggering a garbage collection.
      return this;
    }
    return newRoot;
  }

  function _finalize(m,aPath,aSlots,aShouldInitialize) {
    // ::println("... tImportModule::finalize:" m)
    local that = m;
    if (aSlots.?size()) {
      // ::println("... tImportModule::finalize: slots:" aSlots)
      local slots = {}
      foreach (slotName in aSlots) {
        local slot = m[?slotName]
        if (!slot) {
          throw "tImportModule::finalize: couldn't find slot '" + slotName + "' in module '" + aPath + "'."
        }
        if (typeof slot == "function") {
          slots[slotName] <- ::lang.newClosure(that,slot);
        }
        else {
          slots[slotName] <- slot;
        }
      }
      m = slots;
    }
    if (m.empty()) {
      throw "tImportModule::finalize: module '" + aPath + "' is empty."
    }
    // ::println("... tImportModule::finalize: returned:" m.?__requireImportPath "::" m)
    if (aShouldInitialize && ("_initialize" in that)) {
      that._initialize();
    }
    return m;
  }

  function newImport(aPath,aSlots,aThis) {
    local module, importException
    local fp = ::GetThisVM().ImportFileOpen(aPath);
    local moduleRoot = _createNewRootTable(::getroottable());
    try {
      local funcScriptObject = ::GetThisVM().Compile(fp,aPath);
      local func = ::IScriptObjectToObject(funcScriptObject);
      local moduleThis = {
        __module_path = aPath
        module = {}
      }
      if (aThis) {
        moduleThis = ::delegate(aThis,moduleThis);
      }
      func.SetRoot(moduleRoot);
      func.call(moduleThis);
      module = _finalize(moduleThis.module,aPath,aSlots,true);
      module.__module_root <- moduleRoot // we keep a strong reference to the root
    }
    catch (e) {
      importException = e
    }
    if (importException) {
      throw "ImportModule exception '"+ aPath +"':" + importException
    }
    // ::log(::format("NewImportModule '%s' imported.", aPath))
    return module
  }

  function import(aPath,aSlots) {
    local m
    local inCache = (aPath in _cache);
    if (inCache) {
      m = _cache[aPath]
      // ::log(::format("ImportModule '%s' from cache.", aPath))
    }
    else {
      m = newImport(aPath)
      _cache[aPath] <- m
      // ::log(::format("ImportModule '%s' newly imported.", aPath))
    }
    return _finalize(m,aPath,aSlots,!inCache)
  }
}

::NewImportModule <- ::lang.newClosure(tImportModule,tImportModule.newImport)
::ImportModule <- ::lang.newClosure(tImportModule,tImportModule.import)

function ::isModule(aThis) {
  ::assert(typeof aThis == "table" && aThis.?len());
  local isModule = ("__module_path" in aThis)
  return isModule;
}

function ::namespaceOrModule(aThis, aNamespaceName, aModule) {
  ::assert(typeof aNamespaceName == "string" && aNamespaceName.?len());
  ::assert(typeof aModule == "table" && aModule.?len());
  local isModule = ::isModule(aThis);
  if (!isModule) {
    if (aThis.?module) {
      throw "Not expected to find this module object when not loading as a module: " + ::lang.toString(aThis.?module)
    }
    if (::?module) {
      throw "Not expected to find global module object when not loading as a module: " + ::lang.toString(::?module)
    }
  }
  // ::log("..." aNamespaceName ": isModule:" isModule)

  if (!isModule) {
    ::namespace(aNamespaceName, aModule);
  }
  else {
    aThis.module = aModule
  }
}
