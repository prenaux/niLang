::Import("lang.ni")
::Import("fs.ni")
::Import("seq.ni")

args <- ::GetArgs()
argIndexBase <- 1

// Parse Command
command <- args[?argIndexBase++]
// ::println("... command:" command)

function commandArg(aIndex) {
  local ai = argIndexBase + aIndex
  return args[?ai]
}

dataTables <- []

workDir <- ::gLang.env["WORK"]
// ::println("... workDir:" workDir)

cacheDir <- "".setdir(workDir).adddirback("_idlquery")

function listDataTablesInDir(aDir,aFilter) {
  local dirs = ::fs.listFiles(aDir, "*", ::eFileAttrFlags.AllDirectories, false, true)
  foreach (i,d in dirs) {
    // ::println("... sub dir["+i+"]:" d.path)
    local files = ::fs.listFiles("".setdir(d.path).adddirback("src"), "*_ModuleDef.xml", ::eFileAttrFlags.AllFiles, true, true)
    foreach (i,f in files) {
      local filePath = f.path
      if (aFilter) {
        if (!filePath.getfile().fpatmatch(aFilter))
          continue;
      }
      // ::println("... idl["+i+"]:" filePath)
      local dt = ::lang.loadDataTable("xml",filePath)
      dataTables.Add({ path = filePath, dt = dt })
    }
  }
}

function findAndLoadDataTables(aFilter) {
  local dirs = ::fs.listFiles(workDir, "*", ::eFileAttrFlags.AllDirectories, false, true)
  foreach (i,d in dirs ) {
    // ::println("... dir["+i+"]:" d.path)
    listDataTablesInDir("".setdir(d.path).adddirback("sources"),aFilter)
  }
}

function processDT(aMatchTable,aDT,aDTIndex,aSkipRoot) {
  local dt = aDT.?dt || aDT

  if (!aSkipRoot) {
    local handler = aMatchTable[?dt.name]
    if (handler) {
      if (handler.call(aMatchTable,dt,aDTIndex) == true)
        return  // break the recursion if the handler returns true
    }
  }

  foreach (childIndex in dt.num_children) {
    local c = dt.child_from_index[childIndex]
    processDT(aMatchTable,c,childIndex)
    if (aMatchTable.?_break)
      return
  }
}

tBuildCache <- {
  _uniqueCheck = {}
  _outputs = {}
  _fp = null
  _dtPathEncodedUrl = null

  function getDocOneLineDesc(aDT) {
    local desc = aDT.child["comments"].?string.?desc
    if (desc.?len()) {
      local descLines = desc.split("\n")
      return descLines[?0]
    }
    return null
  }

  function output(aName,aAnnotation,aMeta,aDocNode) {
    local baseName = aName

    if (!aMeta) {
      aMeta = baseName
      local desc = getDocOneLineDesc(aDocNode)
      if (desc.?len()) {
        aMeta += ": " + desc
      }
    }

    aName = aName + "; " + (aAnnotation||"") + "; " + (aMeta||"") + ";"
    if (aName.len() <= 2)
      return

    local c0 = aName[0]
    if (!
        ((c0 >= '0' && c0 <= '9') ||
         (c0 >= 'a' && c0 <= 'z') ||
         (c0 >= 'A' && c0 <= 'Z') ||
         c0 == '_'))
      return

    if (_dtPathEncodedUrl) {
      local parentName = aDocNode.parent.string.name
      local docPath = _dtPathEncodedUrl + "/" + parentName + "/" + baseName
      aName += " " + docPath + ";"
    }

    if (aName in _uniqueCheck)
      return

    local cat = aName.slice(0,2).tolower()
    local out = _outputs[?cat]
    if (!out) {
      out = []
      _outputs[cat] <- out
    }

    out.Add(aName)

    _uniqueCheck[aName] <- true
  }

  function namespace(aDT,aDTIndex) {
    // ::println("... namespace:" aDT.string.name)
  }
  function enum(aDT,aDTIndex) {
    local name = aDT.string.name
    output(name, "enum", null, aDT)
  }
  function interface(aDT,aDTIndex) {
    local name = aDT.string.name
    output(name, "interface", null, aDT)
  }
  function variable(aDT,aDTIndex) {
    local name = aDT.string.name
    output(name,"constant", null, aDT)
  }
  function value(aDT,aDTIndex) {
    local name = aDT.string.name
    local meta = "__" + name + "__"
    meta += " = " + "__" + aDT.string.value.replace(" ","") + "__"
    local desc = getDocOneLineDesc(aDT)
    if (desc.?len()) {
      meta += ": " + desc
    }
    output(name,"enumvalue",meta, aDT)
  }
  function method(aDT,aDTIndex) {
    local name = aDT.string.name
    local meta = ""
    meta += "__" + aDT.parent.string.name
    meta += "::"
    meta += name + "__("
    foreach (ci in aDT.num_children) {
      local c = aDT.child_from_index[ci]
      if (c.name == "parameter") {
        meta += "_" + c.string.typec.replace("*","\\*") + "_"
        meta += " " + c.string.name
        local isLast = c.int.last
        if (!isLast)
          meta += ", "
      }
    }
    meta += ") -> _" + aDT.child["return"].?string.?typec.replace("*","\\*") + "_"

    local desc = getDocOneLineDesc(aDT)
    if (desc.?len()) {
      meta += ": " + desc
    }

    output(name, "function", meta, aDT)
  }
}

function build_cache() {
  findAndLoadDataTables()

  local cache = tBuildCache.Clone()
  ::gRootFS.FileMakeDir(cacheDir)

  foreach (dt in dataTables) {
    cache._dtPathEncodedUrl = dt.path.encodeurl()
    processDT(cache, dt.dt, 0)
  }

  foreach (cat,out in cache._outputs) {
    local fp = ::fs.fileOpenWrite("".setdir(cacheDir).setfile(cat).setext("idlquery"))
    foreach (o in out) {
      fp.WriteString(o + "\n")
    }
    fp.Invalidate()
  }
}

function cached_candidates() {
  local prefix = commandArg(0)
  if (prefix.len() < 2)
    return

  local stringmatchfn = commandArg(1) ? commandArg(1) : "startswith"
  local maxOutput = commandArg(2) ? commandArg(2).toint() : null

  local cat = prefix.slice(0,2).tolower()
  local fp = ::fs.fileOpenRead("".setdir(cacheDir).setfile(cat).setext("idlquery"))
  if (fp) {
    local matchCount = 0
    foreach (l in ::seq.fileLines(fp)) {
      if (l[stringmatchfn](prefix)) {
        ::println(matchCount + "; " + l)
        ++matchCount
        if (maxOutput && matchCount >= maxOutput)
          break;
      }
    }
  }
}

function candidates() {
  build_cache()
  cached_candidates()
}

tPrintDoc <- {
  tBuildCache = tBuildCache
  processDT = processDT
  _parentName = null
  _nodeName = null
  _currentNamespace = ""

  function printDoc(aDT,aDTIndex) {
    local parentName = aDT.parent.string.name
    local nodeName = aDT.string.name

    // ::println("... parentName:" parentName "-" "nodeName:" nodeName)
    if (_parentName && _nodeName) {
      if (_parentName != parentName ||
          _nodeName != nodeName)
        return
    }
    else if (_parentName) {
      if (_parentName != nodeName)
        return
    }

    local out = ""
    local kind = aDT.name
    if (kind == "value") kind = "enumvalue"
    local comments = aDT.child["comments"]

    local that = this
    local function printTitle() : (comments,kind,nodeName,that) {
      if (kind == "namespace") {
        that._currentNamespace = nodeName
        return
      }
      ::println("\n## " + kind + " " + that._currentNamespace + "::" + nodeName)
    }

    local function printDesc() : (comments,kind,nodeName) {
      if (!comments)
        return;
      ::println(comments.string.desc)
      ::println("")
    }

    local function printParents() : (aDT) {
      local parents = aDT.child["parents"]
      if (!parents)
        return

      if (parents.num_children) {
        ::println("### Parents:")
        local numParents = 0
        for (local di = 0; di < parents.num_children; ++di) {
          local parent = parents.child_from_index[di]
          if (parent.name != "parent")
            continue
          local text = parent.string.name
          ::println("-" text)
          ++numParents
        }
        ::println("")
      }
    }

    local function printRemarks() : (comments,kind,nodeName) {
      if (!comments)
        return;

      local remarks = []
      for (local di = 0; di < comments.num_children; ++di) {
        local remark = comments.child_from_index[di]
        if (remark.name != "remark")
          continue

        local text = remark.string.text
        remarks.Add(text)
      }
      if (!remarks.empty()) {
        ::println("### Remarks:")
        foreach (r in remarks) {
          ::println("-" r)
        }
        ::println("")
      }
    }

    local function printParams() : (comments,kind,nodeName) {
      if (!comments)
        return;

      local params = []
      for (local di = 0; di < comments.num_children; ++di) {
        local param = comments.child_from_index[di]
        if (param.name != "params")
          continue

        local text = param.string.text
        params.Add(text)
      }
      if (!params.empty()) {
        ::println("### Parameters:")
        foreach (r in params) {
          ::println("*" r)
        }
        ::println("")
      }
    }

    switch (kind) {
      case "method": {
        local proto = aDT.child["return"].?string.?typec.replace("*","\\*")
        proto += " " + aDT.parent.string.name
        proto += "::"
        proto += nodeName + "("
        foreach (ci in aDT.num_children) {
          local c = aDT.child_from_index[ci]
          if (c.name == "parameter") {
            proto += c.string.typec.replace("*","\\*")
            proto += " " + c.string.name
            local isLast = c.int.last
            if (!isLast)
              proto += ", "
          }
        }
        proto += ")"
        printTitle()
        ::println(proto)
        ::println("")
        printDesc()
        printParams()
        printRemarks()
        break;
      }
      case "enum": {
        printTitle()
        printDesc()
        printParams()
        printRemarks()
        ::println("### Values:")
        local buildCache = tBuildCache.Clone()
        buildCache.output = function (aName,aAnnotation,aMeta,aDocNode) {
          ::println("-" aMeta)
        }
        processDT(buildCache, aDT, aDTIndex, true)
        break;
      }
      case "interface": {
        printTitle()
        printDesc()
        printParents()
        printRemarks()
        ::println("### Methods:")
        local buildCache = tBuildCache.Clone()
        buildCache.output = function (aName,aAnnotation,aMeta,aDocNode) {
          ::println("-" aMeta)
        }
        processDT(buildCache, aDT, aDTIndex, true)
        break;
      }
      case "namespace": {
        printTitle()
        printDesc()
        for (local di = 0; di < aDT.num_children; ++di) {
          local cdt = aDT.child_from_index[di]
          processDT(this, cdt, di, false)
        }
        break;
      }
      default: {
        printTitle()
        printDesc()
        printParams()
        printRemarks()
        break;
      }
    }

    this._break <- true
  }

  function namespace(aDT,aDTIndex) {
    printDoc(aDT,aDTIndex)
  }
  function enum(aDT,aDTIndex) {
    printDoc(aDT,aDTIndex)
  }
  function interface(aDT,aDTIndex) {
    printDoc(aDT,aDTIndex)
  }
  function variable(aDT,aDTIndex) {
    printDoc(aDT,aDTIndex)
  }
  function value(aDT,aDTIndex) {
    printDoc(aDT,aDTIndex)
  }
  function method(aDT,aDTIndex) {
    printDoc(aDT,aDTIndex)
  }
}

function doc() {
  local cmdArgs = commandArg(0).?split("/")
  local path = cmdArgs[?0].?decodeurl()
  local parentName = cmdArgs[?1].?decodeurl()
  local nodeName = cmdArgs[?2].?decodeurl()

  local dt
  if (!path.contains(".xml")) {
    findAndLoadDataTables("_"+path+"_ModuleDef.xml")
    if (dataTables.len() == 0) {
      throw "Couldn't find a datatable matching '"+path+"'."
    }
    else if (dataTables.len() > 1) {
      throw "More than one datatable matching '"+path+"'."
    }
    dt = dataTables[0].dt
  }
  else {
    dt = ::lang.loadDataTable("xml",path)
  }
  if (!dt)
    throw "Couldn't load datatable for: " + path

  ::println("# Module " + path)
  local printDoc = tPrintDoc.Clone()
  printDoc._parentName = parentName
  printDoc._nodeName = nodeName
  processDT(printDoc, dt, 0)
  foreach (i in dt.num_children) {
    processDT(printDoc, dt.child_from_index[i], i)
  }
}

if (!(command in this)) {
  ::println("E/Can't find command '"+command+"'.")
  ::gLang.Exit(-1)
}
this[command]()
