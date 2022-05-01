// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

::Import("lang.ni")
::Import("fs.ni")
::Import("crypto.ni")
::Import("zip.ni")
::Import("algo.ni")
::Import("regex.ni")
::Import("ham.ni")

///////////////////////////////////////////////
::buildNotify <- {
  NOTIFY_SLACK_URL = "https://hooks.slack.com/services/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"
  NOTIFY_SLACK_CHANNEL = "build-hamster"

  function postToSlack(aTitle,aMessage,aIcon) {
    local cmdLine = ::format({[post-to-slack -u "%s" -c "%s" -t "%s" -m "%s" -l "%s"]},
                             NOTIFY_SLACK_URL, NOTIFY_SLACK_CHANNEL,
                             aTitle, aMessage, aIcon);
    ::ham.runBash(cmdLine,false,false);
  }

  function info(aTitle, aMessage) {
    postToSlack(aTitle, aMessage, "#0ff");
    ::println("I/"+aMessage);
  }

  function success(aTitle, aMessage) {
    postToSlack(aTitle, aMessage, "good");
    ::println("I/"+aMessage);
  }

  function error(aTitle, aMessage) {
    postToSlack(aTitle, aMessage, "danger");
    ::println("E/"+aMessage);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
::installer <- {
  _baseDT = null
  _script = null
  _baseDir = null
  _outputDir = "./"
  _outPath = null
  _outFinal = null
  _dirs = {}
  _regServer32 = null
  _regServer64 = null
  _addTimeString = true
  _timeString = null
  _password = null
  _allDestFiles = {}
  _signExe = null
  _signExe2 = null
  //////////////////////////////////////////////////////////////////////////////////////////////
  function new(dt,baseDir,outDir) {
    local n = this.Clone()
    n._baseDT = dt
    n._script = ::lang.convertDataTableToTable(dt)
    n._baseDir = baseDir
    n._timeString = ::lang.getTimeSecondsString()
    if (outDir)
      n._outputDir = outDir
    if (!n._script.Desc.vername)
      n._script.Desc.vername = "UnknownVersion"
    n._script.Desc.vername += "-" + n._timeString
    n._addTimeString = ::lang.toBool(n._script.Desc.?add_time_string,true)
    return n
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
  function getVersionName() {
    return _script.Desc.vername;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
  function getVersionDate() {
    return _timeString;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
  // DT
  //////////////////////////////////////////////////////////////////////////////////////////////
  function buildInit() {
    local scannedCount = 0
    local numFolder = _script.Folders.__tables.len()
    foreach (f in _script.Folders.__tables) {
      ::println("# Scanning folder '"+f.name+"' ["+(++scannedCount)+"/"+numFolder+"]")
      local dest = ("dest" in f) ? f.dest : "{app}/"+f.name
      local filter = ("filter" in f) ? ((f.filter.icompare("null")==0)?null:f.filter) : "*"
      local recursive = ("recursive"  in f) ?
        ((f.recursive.icompare("no")==0)?false:true) : true
      local flags = ("flags" in f) ? f.flags : "";
      local pkg = ("pkg" in f) ? ((f.pkg.icompare("yes")==0)?true:false) : false
      local sign = ("sign" in f) ? ((f.sign.icompare("yes")==0)?true:false) : false
      local excludeRegex
      if (f.?exclude) {
        excludeRegex = ::regex.pcre(f.exclude)
        // ::println("EXCLUDE:" f.exclude "- regex:" excludeRegex)
      }
      local d = addFolders(_dirs,f.name,dest,filter,excludeRegex,recursive,flags,pkg,sign)
      if (d && ("icongroup" in f)) {
        d.icongroup = f.icongroup
      }
    }

    if ("password" in _script.Desc) {
      local outFinal = getOutFinal()
      local outPwd = _outputDir.setfile(outFinal).setext("txt")
      _password = generatePassword(_script.Desc.password,outPwd)
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Utilities
  //////////////////////////////////////////////////////////////////////////////////////////////
  function getFinalDestName(d,f,dirSrc,dirDst) {
    if (!dirSrc) {
      dirSrc = dirSrc || d.source.after(_baseDir) + "/";
    }
    if (!dirDst) {
      dirDst = d.dest;
      if (dirDst.startswith("{app}/"))
        dirDst = dirDst.after("{app}/")
      if (dirDst.len() && !dirDst.endswith("/"))
        dirDst += "/"
    }

    local name = f.after(_baseDir).trimleftex("/")
    local destName = name
    switch (name.getext()) {
      case "nio":
      destName = name.setext("ni");
      break;
    }
    if (d.dest) {
      destName = dirDst.setfile(destName.getfile())
    }

    return destName
  }

  function addFolders(aDirs,aFolder,aDest,aFilesFilter,aExcludeRegex,aRecursive,aFlags,aPkg,aSign) {
    if (aExcludeRegex.?DoesMatch(aFolder)) {
      // ::println("SKIPPED FOLDER: " + aFolder);
      return null;
    }

    local path = aFolder
    if (!path.startswithi(_baseDir) && (path.len() < 1 || path[1] != ':'))
      path = _baseDir + aFolder

    // get directory table
    local d
    if (!(path in aDirs)) {
      d = {
        source = path
        dest = aDest
        files = []
        dirs = {}
        icongroup = null
        flags = aFlags
        pkg = aPkg
        sign = aSign
      }
      aDirs[path] <- d
    }
    else {
      d = aDirs[path]
    }

    local dirSrc = d.source.after(_baseDir) + "/";
    local dirDst = d.dest;
    if (dirDst.startswith("{app}/"))
      dirDst = dirDst.after("{app}/")
    if (dirDst.len() && !dirDst.endswith("/"))
      dirDst += "/"

    // list files
    if (aFilesFilter) {
      // ::println("FOLDER: " + aFolder);
      local lstFiles = ::fs.listFiles(path,aFilesFilter,::eFileAttrFlags.AllFiles,false,false)
      foreach (l in lstFiles) {
        if (aExcludeRegex) {
          local fn = aFolder + "/" + l.path;
          if (aExcludeRegex.DoesMatch(fn)) {
            // ::println("SKIP: " + fn);
            continue;
          }
          else {
            // ::println("KEEP: " + fn);
          }
        }

        local fpath = path+"/"+l.path
        local destName = getFinalDestName(d,fpath)
        if (destName in _allDestFiles) {
          ::println("W/File" destName "already added.")
        }
        else {
          d.files.append(fpath)
          _allDestFiles[destName] <- true
        }
      }

      if (aRecursive) {
        // list directories
        local lstDirs = ::fs.listFiles(path,"*",::eFileAttrFlags.AllDirectories,false,false)
        foreach (l in lstDirs) {
          if (l.path.tolower().startswith(".hg")) continue;
          if (l.path.tolower().startswith(".git")) continue;
          if (l.path.tolower().startswith(".svn")) continue;
          addFolders(d.dirs,aFolder+"/"+l.path,aDest+"/"+l.path,aFilesFilter,aExcludeRegex,true,aFlags,aPkg,aSign)
        }
      }
    }

    return d
  }

  ///////////////////////////////////////////////
  function generatePassword(aPwd,aOutFile) {
    local pwd = aPwd
    switch (pwd) {
      case "none": {
        return null
      }
      case "env": {
        pwd = ::gLang.env["HAM_INSTALLER_PASSWORD"];
        if (pwd.?len())
          break
      }
      case "":
      case "auto": {
        pwd = ""+::gLang.CreateGlobalUUID()
        pwd = pwd.replace("-","").toupper()
        break
      }
      case "autouuid": {
        pwd = ""+::gLang.CreateGlobalUUID()
        pwd = pwd.toupper()
        break
      }
    }
    if (aOutFile) {
      local pwdFile = ::fs.fileOpenWrite(aOutFile)
      if (!pwdFile)
        throw "Can't output password file: " + aOutFile
      pwdFile.WriteString("InstallerKey: "+pwd+"\r\n")
    }
    return pwd
  }

  ///////////////////////////////////////////////
  function printDirs(aDirs) {
    foreach(d in aDirs) {
      ::println(d.dest + "("+d.files.len()+"): " + d.source)
      printDirs(d.dirs)
    }
  }

  ///////////////////////////////////////////////
  function buildPrint() {
    printDirs(_dirs)
  }

  ///////////////////////////////////////////////
  function getOutFinal() {
    if (!_outFinal) {
      local name
      if ("filename" in _script.Desc) {
        _outFinal = _script.Desc.filename
      }
      else {
        _outFinal = _script.Desc.name
      }
      if (_addTimeString) {
        _outFinal += "-"+_timeString
      }
    }
    return _outFinal;
  }

  ///////////////////////////////////////////////
  function buildWin32(aBuild) {
    local outFinal = getOutFinal()
    local outISS = _outputDir.setfile(outFinal).setext("iss")
    local outExe = _outputDir.setfile(outFinal).setext("exe")
    local fpISS = ::fs.fileOpenWrite(outISS)
    if (!fpISS)
      throw "Can't open output file '" + outISS + "'."

    function o(str) : (fpISS) { fpISS.WriteString(str); fpISS.WriteString("\n") }
    function p(str) {
      local s = "\""
      foreach (c in str) {
        if (c == '/') s += "\\"
        else if (c == '^') s += "\"\""
        else           s += c.tochar()
      }
      s += "\""
      return s
    }

    // Header
    o(::format({[;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; --- Autogenerate InnoSetup ISS file for %s ---
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
]},_script.Desc.name))

    // [Setup] section
    o(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;")
    o("[Setup]")
    if ("noadmin" in _script.Desc && _script.Desc.noadmin == "yes") {
      o("PrivilegesRequired=none")
    }
    else {
      o("PrivilegesRequired=admin")
    }
    o("DisableDirPage=" + ((_script.Desc.?choose_dir == "yes") ? "no" : "yes"))
    o("DisableProgramGroupPage=" + ((_script.Desc.?choose_group == "yes") ? "no" : "yes"))
    if ("regserver32" in _script.Desc) {
      _regServer32 = ::regex.filePattern(_script.Desc.regserver32)
    }
    if ("regserver64" in _script.Desc) {
      _regServer64 = ::regex.filePattern(_script.Desc.regserver64)
    }
    if ("sign_exe" in _script.Desc && _script.Desc.sign_exe.?len()) {
      _signExe = _script.Desc.sign_exe
    }
    if ("sign_exe2" in _script.Desc && _script.Desc.sign_exe2.?len()) {
      _signExe2 = _script.Desc.sign_exe2
    }
    o("SourceDir=" + p(_baseDir))
    o("AppName=" + _script.Desc.name)
    o("AppVerName=" + _script.Desc.vername)
    o("AppCopyright=" + _script.Desc.copyright)
    o("AppPublisher=" + _script.Desc.publisher)
    o("AppPublisherURL=" + _script.Desc.website)
    o("DefaultDirName=" + p(_script.Desc.default_dir))
    o("DefaultGroupName=" + p(_script.Desc.default_group))
    o("OutputBaseFilename="+p(outFinal))
    o("OutputDir="+p(_outputDir))
    o("UsePreviousAppDir=yes")
    o("UsePreviousGroup=yes")
    o("RestartIfNeededByRun=no")
    if ("uninstall_display_name" in _script.Desc) {
      o("UninstallDisplayName="+_script.Desc.uninstall_display_name)
    }
    else {
      o("UninstallDisplayName="+_script.Desc.publisher+" "+_script.Desc.vername)
    }
    if ("uninstall_icon" in _script.Desc) {
      o("UninstallDisplayIcon=" + p(_script.Desc.uninstall_icon))
    }
    if ("setup_icon" in _script.Desc) {
      o("SetupIconFile=" + p(_script.Desc.setup_icon))
    }
    if (_password) {
      o("Encryption=yes")
      o("Password="+_password)
    }
    if ("compression" in _script.Desc) {
      local solid = "false"
      local compression = "lzma/normal"
      switch (_script.Desc.compression.tolower()) {
        case "solid.none": solid = "true"; compression = "none"; break
        case "none": compression = "none"; break
        case "solid.fast": solid = "true"; compression = "lzma/fast"; break
        case "fast": compression = "zip"; break
        case "solid.best": solid = "true"; compression = "lzma/max"; break
        case "best": compression = "lzma/max"; break
        case "solid.ultra": solid = "true"; compression = "lzma/ultra"; break
        case "ultra": compression = "lzma/ultra"; break
        case "solid.ultrax": solid = "true"; compression = "lzma/ultra64"; break
        case "ultrax": compression = "lzma/ultra64"; break
        case "solid.normal": solid = "true"; break
        case "normal":
        default: break;
      }
      o("Compression="+compression)
      o("SolidCompression="+solid)
    }
    o("InternalCompressLevel=ultra")
    if ("license" in _script.Desc) {
      o("LicenseFile="+p(_script.Desc.license))
    }
    if ("readme" in _script.Desc) {
      o("InfoAfterFile="+p(_script.Desc.readme))
    }
    if ("readme_before" in _script.Desc) {
      o("InfoBeforeFile="+p(_script.Desc.readme_before))
    }
    o("WizardImageBackColor=$222222")
    o("WizardImageStretch=no")
    if ("image" in _script.Desc) {
      o("WizardImageFile="+p(_script.Desc.image))
    }
    if ("small_image" in _script.Desc) {
      o("WizardSmallImageFile="+p(_script.Desc.small_image))
    }

    if ("disk_size" in _script.Desc) {
      o("DiskSpanning=yes")
      o("DiskSliceSize="+_script.Desc.disk_size)
    }
    o("")

    // [InstallDelete] section
    if ("install_deletepreviousfirst" in _script.Desc) {
      if (_script.Desc.install_deletepreviousfirst == "yes") {
        o(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;")
        o("[InstallDelete]")
        o({[Type: filesandordirs; Name: "{app}"]}) // remove files
        o({[Type: filesandordirs; Name: "{group}\*";]}) // remove icons
        o("")
      }
    }

    // Icons section
    if ("Icons" in _script) {
      o(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;")
      o("[Icons]")
      local iconMap = {}
      function addIcon(addoncecheck,name,filename,params,workdir,icon) : (iconMap) {
        if (addoncecheck) {
          if (filename in iconMap)
            return
        }
        local v = ""
        v += "Name: " + p(name)
        v += "; Filename: "+p(filename)
        if (params) v += "; Parameters: "+p(params)
        if (workdir) v += "; WorkingDir: "+p(workdir)
        else v += "; WorkingDir: "+p(filename.getdir())
        if (icon) v += "; IconFilename: "+p(icon)
        v += ";"
        o(v)
        iconMap[filename] <- name
      }
      foreach (i in _script.Icons.__tables) {
        addIcon(false,i.name,i.filename,
                ("parameters" in i)?i.parameters:null,
                ("working_dir" in i)?i.working_dir:null,
                ("icon" in i)?i.icon:null)
      }
      function outputIcons(aDirs) {
        foreach (d in aDirs) {
          if (d.icongroup) {
            local dir = d.icongroup.before("|")
            if (!dir.endswith("/")) dir += "/"
            local filter = ::regex.filePattern(d.icongroup.after("|"))
            foreach (f in d.files) {
              local filename = d.dest+"/"+f.getfile()
              if (filter.DoesMatch(filename)) {
                addIcon(true,dir+f.getfilenoext(),filename,null,null,null)
              }
            }
          }
          outputIcons(d.dirs)
        }
      }
      outputIcons(_dirs)
      o("")
    }

    // [Run] section
    if ("Run" in _script) {
      o(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;")
      o("[Run]")
      foreach (r in _script.Run.__tables) {
        local v = ""
        v += "Filename:" + p(r.?command || r.filename)
        v += "; WorkingDir:" + p(r.working_dir)
        v += "; Parameters:" + p(r.parameters)
        if ("status_msg" in r) {
          v += "; StatusMsg:" + p(r.status_msg)
        }
        if ("desc" in r) {
          v += "; Description:" + p(r.desc)
        }
        if ("flags" in r) {
          v += "; Flags: "+r.flags
        }
        else {
          v += "; Flags: runhidden"
        }
        v += ";"
        o(v)
      }
      o("")
    }


    // [Dirs] section
    o(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;")
    o("[Dirs]")
    function outputDirs(aDirs) : (fpISS) {
      foreach (d in aDirs) {
        if (d.pkg) continue;
        local flags = "";
        if (d.flags.?len()) {
          flags += " Flags: " + d.flags +";"
        }
        o("Name: " + p(d.dest) + ";" + flags)
        outputDirs(d.dirs)
      }
    }
    outputDirs(_dirs)
    o("")

    // [Files] section
    o(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;")
    o("[Files]")
    local filesToSign = []
    function outputFiles(aDirs) : (fpISS,filesToSign) {
      foreach (d in aDirs) {
        if (d.pkg)
          continue;
        foreach (f in d.files) {
          local flags = "";
          if (_regServer32 && _regServer32.DoesMatch(f.getfile())) {
            flags = " Flags: regserver 32bit;"
          }
          else if (_regServer64 && _regServer64.DoesMatch(f.getfile())) {
            flags = " Flags: regserver 64bit; Check: IsWin64;"
          }
          flags += " AfterInstall: DeleteFromVirtualStore;"
          if (d.sign) {
            switch (f.getext()) {
              case "exe":
              case "dll": {
                filesToSign.push(f);
                break
              }
            }
          }
          o("Source:" + p(f) + "; DestDir: " + p(d.dest) + ";" + flags)
        }
        outputFiles(d.dirs)
      }
    }
    outputFiles(_dirs)
    if (_signExe || _signExe2) {
      signFiles(filesToSign, _signExe, _signExe2)
    }
    o("")

    // Include
    o(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;")
    local incFile;
    local baseFile = "_base"
    if ("base" in _script.Desc)
      baseFile = _script.Desc.base

    function getISSPath(aFile) {
      local path = ::gRootFS.GetAbsolutePath(aFile);
      if (::fs.fileExists(path))
        return path.trimex("\"")

      local path = (_baseDir+"installer\\").setfile(aFile).setext("iss");
      if (::fs.fileExists(path))
        return path.trimex("\"")

      local path = "".SetDir(::gLang.property["ni.dirs.data"]).AddDirBack("../../installer").setfile(aFile).setext("iss");
      if (::fs.fileExists(path))
        return path.trimex("\"")

      local path = ::fs.getAbsolutePath("".setfile(aFile).setext("iss"));
      if (::fs.fileExists(path))
        return path.trimex("\"")

      throw "Can't find include ISS file '"+aFile+"'."
    }

    incFile = getISSPath(baseFile)
    o("#include \""+p(incFile)+"\"")
    if ("extended" in _script.Desc) {
      incFile = getISSPath(_script.Desc.extended)
      o("#include \""+p(incFile)+"\"")
    }
    o("")

    fpISS.Invalidate()
    fpISS = null
    if (aBuild) {
      local function findISCC() {
        local path;
        path = "".SetDir(::gLang.env["HAM_HOME"]).AddDirBack("toolsets/innosetup/nt-x86").SetFile("ISCC.exe")
        if (::fs.fileExists(path))
          return path

        throw "Can't find ISCC"
      }

      local exe = p(findISCC()).trimex("\"")
      local cmd = ::format({["%s" "%s"]},exe,outISS)
      ::println("Running: " + cmd)
      if (::ham.runBash(cmd,true,true).exitCode != 0)
        throw "ISCC execution failed."
    }

    return outExe
  }

  ///////////////////////////////////////////////
  function computeFileHash(aFilePath,aType) {
    local hashType
    switch (aType) {
      case "wrl": hashType = "Whirlpool"; break;
      case "sha1": hashType = "SHA-1"; break;
      default:
      throw "Unknown hash type."
    }
    local fp = (typeof aFilePath == "string") ? ::fs.fileOpenRead(aFilePath) : aFilePath
    if (!fp)
      throw "Can't open file to hash: " + aFilePath
    local hash = ::crypto.createHash(hashType)
    if (!hash.Update(fp,fp.size))
      throw "Can't update hash."
    local dig = hash.FinalString(0)
    if (!dig.?len())
      throw "Can't compute digest."
    return aType.toupper()+":"+dig
  }

  ///////////////////////////////////////////////
  function buildHash(aBaseExt,aType) {
    local hashType
    switch (aType) {
      case "wrl": hashType = "Whirlpool"; break;
      case "sha1": hashType = "SHA-1"; break;
      default:
      throw "Unknown hash type."
    }
    local outFinal = getOutFinal()
    local outPath = _outputDir.setfile(outFinal).setext(aBaseExt)
    local outHash = outPath + "." + aType
    local fp = ::fs.fileOpenRead(outPath)
    if (!fp)
      throw "Can't open file to hash: " + outPath
    local hash = ::crypto.createHash(hashType)
    ::println("Computing "+hashType+" Hash '"+outHash+"'...")
    if (!hash.Update(fp,fp.size))
      throw "Can't update hash."
    local dig = hash.FinalString(0)
    if (!dig.?len())
      throw "Can't compute digest."
    local fpOut = ::fs.fileOpenWrite(outHash)
    fpOut.WriteString(aType.toupper()+":"+dig+"\n")

    return outHash
  }

  ///////////////////////////////////////////////
  function buildFileList() {
    local outFinal = getOutFinal()
    local outFileList = _outputDir.setfile(outFinal).setext("filelist")
    local pack = ::fs.fileOpenWrite(outFileList)
    if (!::lang.isValid(pack))
      throw "Can't create destination file."

    ::println("Building file list:" outFileList)

    // Adding files
    function outputFileToList(aDirs) : (pack) {
      foreach (d in aDirs) {
        local dirDst = d.dest;
        if (dirDst.startswith("{app}/"))
          dirDst = dirDst.after("{app}/")
        if (dirDst.len() && !dirDst.endswith("/"))
          dirDst += "/"
        foreach (f in d.files) {
          local name = f.after(_baseDir).trimleftex("/").getpath()
          local destName = name
          switch (name.getext()) {
            case "nio":
            destName = name.setext("ni");
            break;
          }
          if (d.dest) {
            destName = dirDst.setfile(destName.getfile())
          }

          local hash = computeFileHash(f,"sha1")
          pack.WriteString(name+";"+destName+";"+hash+"\n")
        }
        outputFileToList(d.dirs)
        pack.Flush()
      }
    }
    outputFileToList(_dirs)

    ::println("Done.")

    pack = null

    return outFileList
  }

  ///////////////////////////////////////////////
  function buildZip() {
    local outFinal = getOutFinal()
    local outZip = _outputDir.setfile(outFinal).setext("zip")
    local fpZip = ::fs.fileOpenWrite(outZip)
    if (!::lang.isValid(fpZip))
      throw "Can't open zip destination."

    local pack = ::gZip.CreateZipArchive(fpZip,_password || "",null)
    if (!::lang.isValid(pack))
      throw "Can't create zip archive."

    ::println("Building zip: " outZip)

    // Adding files
    function outputFilesToZip(aPack,aDirs) {
      foreach (d in aDirs) {
        local dirSrc = d.source.after(_baseDir) + "/";
        local dirDst = d.dest;
        ::println("I/ Packing Dir '" + dirSrc + "' -> '" + dirDst + "'")
        if (dirDst.startswith("{app}/"))
          dirDst = dirDst.after("{app}/")
        if (dirDst.len() && !dirDst.endswith("/"))
          dirDst += "/"

        foreach (f in d.files) {
          local name = f.after(_baseDir).trimleftex("/")

          local destName = name
          switch (name.getext()) {
            case "nio":
            destName = name.setext("ni");
            break;
          }
          if (d.dest) {
            destName = dirDst.setfile(destName.getfile())
          }

          ::println("V/ Packing '"+name+"'"+((name==destName)?"":(" -> "+destName)))
          local fp = ::fs.fileOpenRead(f)
          if (!::lang.isValid(fp))
            throw "Can't open resource '"+f+"'."

          local ret = aPack.AddFileBlock(destName,fp,fp.size)
          if (ret == invalid) {
            throw "Failed adding '"+f+"'."
          }
        }
        outputFilesToZip(aPack,d.dirs)
      }
    }
    outputFilesToZip(pack,_dirs)

    ::println("Done.")
    pack.Invalidate()
    pack = null
    fpZip.Invalidate()
    fpZip = null

    return outZip
  }

  ///////////////////////////////////////////////
  function build7z(abRemoveZip) {
    local outFinal = getOutFinal()
    local outZip = _outputDir.setfile(outFinal).setext("zip")

    local cmdLine = "7z-fromzip \"" + outZip + "\""
    if (_password)
      cmdLine += " " + _password

    ::println("Running: " + cmdLine)
    if (::ham.runBash(cmdLine,true,true).exitCode != 0)
      throw "7z packaging failed."

    if (abRemoveZip) {
      ::gRootFS.FileDelete(outZip)
    }

    return outZip
  }

  ///////////////////////////////////////////////
  function signFiles(aPaths,aSignCmd1,aSignCmd2) {
    if (!aSignCmd1.?len() && !aSignCmd2.?len())
      throw "No sign command specified."

    local toPrint = ""
    foreach (path in aPaths) {
      if (toPrint.len() > 0)
        toPrint += ", "
      toPrint += path.getfile()
      if (toPrint.len() > 100) {
        toPrint += ", ..."
        break;
      }
    }
    ::println("# Signing: " + toPrint)

    local cmdLine1 = (aSignCmd1.?len()) ? (aSignCmd1 + " ") : null
    local cmdLine2 = (aSignCmd2.?len()) ? (aSignCmd2 + " ") : null
    foreach (path in aPaths) {
      if (cmdLine1)
        cmdLine1 += " \"" + path + "\""
      if (cmdLine2)
        cmdLine2 += " \"" + path + "\""
    }

    if (cmdLine1) {
      ::println("# Signing Phase1")
      if (::ham.runBash(cmdLine1,true,true).exitCode != 0)
        throw "Signing1 failed."
    }

    if (cmdLine2) {
      ::println("# Signing Phase2")
      if (::ham.runBash(cmdLine2,true,true).exitCode != 0)
        throw "Signing2 failed."
    }

    return true
  }
  //////////////////////////////////////////////////////////////////////////////////////////////
  // High level
  //////////////////////////////////////////////////////////////////////////////////////////////
  function build(aDTPath,aBasePath,aOutputDir,aOutputTypes,aOnInitInstaller) {
    local startTime = ::clock()

    aOutputTypes = ::algo.array(aOutputTypes);

    local bBuildExe = ::algo.has(aOutputTypes,"exe")
    local bBuildZip = ::algo.has(aOutputTypes,"zip")
    local bBuild7z = ::algo.has(aOutputTypes,"7z")
    local bBuildFileList = ::algo.has(aOutputTypes,"filelist")
    if (!bBuildExe && !bBuildZip && !bBuildFileList && !bBuild7z) {
      throw "No supported installer type specified."
    }

    local dt = ::lang.implements(aDTPath,"iDataTable")
    if (!dt) {
      dt = ::lang.loadDataTable("xml",aDTPath)
    }

    ::gRootFS.FileMakeDir(aOutputDir)
    if (!::fs.dirExists(aOutputDir))
      throw "Can't create output director: " + aOutputDir

    local myInstaller = ::installer.new(dt,aBasePath,aOutputDir)
    if (aOnInitInstaller) {
      aOnInitInstaller(myInstaller)
    }

    ::println("# Scanning folders...")
    ::println("BasePath: " + aBasePath)
    if (!::fs.dirExists(aBasePath)) {
      throw "E/Base path directory doesn't exists: " + aBasePath
    }

    ::println("OutputDir: " + aOutputDir)
    ::println("OutFinal: " + myInstaller.getOutFinal())
    myInstaller.buildInit()

    if (bBuildFileList) {
      ::println("# Building file list...")
      myInstaller.buildFileList()
    }

    if (bBuildExe) {
      ::println("# Building Win32 Installer...")
      local pathExe = myInstaller.buildWin32(true)
      myInstaller.buildHash("exe","sha1")
      if (myInstaller._signExe || myInstaller._signExe2) {
        myInstaller.signFiles([pathExe], myInstaller._signExe, myInstaller._signExe2)
      }
    }

    if (bBuildZip || bBuild7z) {
      ::println("# Building Zip...")
      local pathZip = myInstaller.buildZip()
      if (bBuildZip) {
        myInstaller.buildHash("zip","sha1")
      }
    }

    if (bBuild7z) {
      ::println("# Building 7z...")
      local path7Z = myInstaller.build7z(!bBuildZip)
      myInstaller.buildHash("7z","sha1")
    }

    local time = ::clock()-startTime
    ::println("# Done in" ::format("%g",time) " seconds.")

    return myInstaller
  }
}
