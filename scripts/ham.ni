// Copyright 2007-2016 TalanSoft, Co. All Rights Reserved.
::Import("lang.ni")
::Import("fs.ni")

module <- {
  _osArch = null
  _build = null
  _hamPath = null
  _hamHome = null
  _bashPath = null
  _tempDir = null
  _tempFilesCollector = []
  _debugEchoAll = false

  function getHamPath() {
    if (_hamPath)
      return _hamPath;

    local function tryHomePath(string aHamHome) string {
      local hamPath = "".setdir(aHamHome).adddirback("bin").SetFile("ham");
      if (::fs.fileExists(hamPath)) {
        return ::gRootFS.GetAbsolutePath(hamPath)
      }
      return ::LINT_AS_TYPE("string",null)
    }

    local hamHome
    if (!_hamPath) {
      hamHome = "".setdir(::gLang.property["ni.dirs.ham_home"]);
      if (::fs.dirExists(hamHome)) {
        _hamPath = tryHomePath(hamHome)
      }
    }

    if (!_hamPath) {
      hamHome = "".setdir(::gLang.property["ni.dirs.bin"])
      hamHome = hamHome.adddirback("../../../../ham");
      if (::fs.dirExists(hamHome)) {
        _hamPath = tryHomePath(hamHome)
      }
    }
    if (!_hamPath) {
      hamHome = "".setdir(::gLang.property["ni.dirs.bin"])
      hamHome = hamHome.adddirback("../../../ham");
      if (::fs.dirExists(hamHome)) {
        _hamPath = tryHomePath(hamHome)
      }
    }
    if (!_hamPath) {
      hamHome = "".setdir(::gLang.property["ni.dirs.bin"])
      hamHome = hamHome.adddirback("../../ham");
      if (::fs.dirExists(hamHome)) {
        _hamPath = tryHomePath(hamHome)
      }
    }

    if (!_hamPath) {
      hamHome = ::gLang.env["HAM_HOME"];
      if (::fs.dirExists(hamHome)) {
        _hamPath = tryHomePath(hamHome);
      }
    }

    if (!_hamPath) {
      throw "Can't find ham executable."
    }
    _hamHome = "".setdir(::gRootFS.GetAbsolutePath(hamHome));
    return _hamPath;
  }

  function getHamHome() {
    if (_hamHome)
      return _hamHome;

    // will set ham home
    getHamPath();
    return _hamHome;
  }

  function getBuild() {
    if (_build)
      return _build;
    _build = ::gLang.property["ni.loa.build"];
    return _build;
  }

  function getOSArch() {
    if (_osArch)
      return _osArch;

    _osArch = ::gLang.property["ni.loa.os"] + "-" + ::gLang.property["ni.loa.arch"];
    return _osArch;
  }

  function getExePath(aPath) {
    switch (::lang.getHostOS().tolower()) {
      case "nt": {
        aPath = aPath.setext("exe")
        break;
      }
      case "osx": {
        break;
      }
      case "lin": {
        aPath = aPath.setext("x64")
        break;
      }
      default: {
        throw ::format("Unknown platform '%s', can't get exe path '%s'.", ::lang.getHostOS(), aPath)
      }
    }
    if (!::gRootFS.FileExists(aPath,::eFileAttrFlags.AllFiles)) {
      local tryPath = aPath.adddirback(getOSArch())
      if (::gRootFS.FileExists(tryPath,::eFileAttrFlags.AllFiles))
        return tryPath;

      tryPath = aPath.adddirback(::gLang.env["HAM_BIN_LOA"])
      if (::gRootFS.FileExists(tryPath,::eFileAttrFlags.AllFiles))
        return tryPath;

      throw ::format("Can't find executable '%s'.", aPath)
    }
    return aPath;
  }

  function getBinDir() {
    return "bin/" + getOSArch();
  }

  function getToolsetDir(aToolset) {
    local path = "".setdir(getHamHome())
    path = path.adddirback("toolsets")
    path = path.adddirback(aToolset)
    return path
  }

  function getToolsetBinDir(aToolset) {
    local path = getToolsetDir(aToolset)
    return path.adddirback(getOSArch())
  }

  function getBashPath() {
    if (_bashPath)
      return _bashPath

    local bashPath
    switch (::lang.getHostOS().tolower()) {
      case "nt": {
        bashPath = getHamHome().adddirback("toolsets/repos/nt-x86/git/bin").setfile("bash.exe")
        if (!::gRootFS.FileExists(bashPath,::eFileAttrFlags.AllFiles)) {
          bashPath = getHamHome().adddirback("bin/nt-x86").setfile("bash.exe")
          if (!::gRootFS.FileExists(bashPath,::eFileAttrFlags.AllFiles)) {
            throw "Can't find bash executable in: " + bashPath
          }
        }
        break;
      }
      case "osx": {
        bashPath = "/bin/bash"
        break;
      }
      case "lin":
      case "linux": {
        bashPath = "/usr/bin/bash"
        break;
      }
      default: {
        throw ::format("Unknown platform '%s', can't find bash's path.",
                       ::lang.getHostOS())
      }
    }

    _bashPath = bashPath
    return _bashPath;
  }

  function getTempDir() {
    if (_tempDir)
      return _tempDir;

    local tempDir

    tempDir = "".setdir(::gLang.property["ni.dirs.temp"])
    if (::fs.dirExists(tempDir)) {
      _tempDir = tempDir;
      return _tempDir;
    }

    tempDir = "".setdir(::gLang.env["TEMP"])
    if (::fs.dirExists(tempDir)) {
      _tempDir = tempDir;
      return _tempDir;
    }

    tempDir = "".setdir(::gLang.env["TMP"])
    if (::fs.dirExists(tempDir)) {
      _tempDir = tempDir;
      return _tempDir;
    }

    tempDir = "".setdir(::gLang.env["HOME"])
    if (::fs.dirExists(tempDir)) {
      tempDir = tempDir.adddirback("_ham/temp/");
      ::fs.makeDir(tempDir)
      _tempDir = tempDir;
      return _tempDir;
    }

    throw "Can't find the temp folder."
  }

  function addTempFile(aPath) {
    if (_debugEchoAll) {
      ::dbg("... Added temp file:" aPath)
    }
    _tempFilesCollector.Add(aPath)
  }

  function genTempFilePath(aExt,_aName,_aTag) {
    return getTempDir().setfile((_aName || "")+(_aTag || ::gLang.CreateGlobalUUID())).setext(aExt || "tmp")
  }

  function getNewTempFilePath(aExt,_aName,_aTag) {
    local path = genTempFilePath(aExt,_aName,_aTag)
    this.addTempFile(path)
    return path;
  }

  function deleteTempFiles() {
    local filesToCollect = _tempFilesCollector
    _tempFilesCollector = []
    foreach (f in filesToCollect) {
      local r = ::gRootFS.FileDelete(f)
      if (_debugEchoAll) {
        ::dbg("... Deleted temp file:" f "("+(r ? "yes" : "didnt exist")+")")
      }
    }
    ::dbg("... Deleted" filesToCollect.size() "temp ham files.")
  }

  // Return {
  //          succeeded = true|false,
  //          exitCode = program exit code,
  //          stdout = if abKeepStdOut "output in stdout",
  //        }
  function runProcess(aCmd,abKeepStdOut,abEchoStdout) {
    local pm = ::gLang.process_manager
    local curProc = pm.current_process
    local proc = pm.SpawnProcess(aCmd,::eOSProcessSpawnFlags.StdFiles)
    if (!proc)
      throw "Couldn't spawn process from command line: " + aCmd

    local stdout = ""

    do {
      local procStdOut = proc.file[1]
      local validCount = 0

      // drain stdout to stdout
      if (::lang.isValid(procStdOut) && !procStdOut.partial_read) {
        ++validCount
        local line = procStdOut.ReadStringLine()
        if (!line.?empty()) {
          if (abEchoStdout || _debugEchoAll) {
            if (_debugEchoAll) {
              curProc.file[1].WriteString("D/RUN-STDOUT: ")
            }
            curProc.file[1].WriteString(line)
            curProc.file[1].WriteString("\n")
          }
          if (abKeepStdOut) {
            stdout += line + "\n"
          }
        }
      }

      if (validCount == 0)
        break

    } while(1)

    local procRet = proc.WaitForExitCode(invalid);
    return {
      succeeded = procRet.x.toint()
      exitCode = procRet.y.toint()
      stdout = stdout
    }
  }

  function runDetachedProcess(aCmd) {
    local pm = ::gLang.process_manager
    local curProc = pm.current_process
    local proc = pm.SpawnProcess(aCmd,::eOSProcessSpawnFlags.Detached)
    if (!proc)
      throw "Couldn't spawn process from command line: " + aCmd
    return proc
  }

  function seqProcess(aCmd,aOptions) {
    local r = {
      _cmd = aCmd
      _options = {
        differentStdOutAndStdErr = aOptions.?differentStdOutAndStdErr || false
        drainStdErrBeforeStdIn = aOptions.?drainStdErrBeforeStdIn || false
        detached = aOptions.?detached || false
      }
      _debugEchoAll = _debugEchoAll

      function runCommand() {
        _count <- 0
        _pm <- ::gLang.process_manager
        _curProc <- _pm.current_process
        _proc <- _pm.SpawnProcess(
          _cmd,
          ::eOSProcessSpawnFlags.StdFiles|
            (_options.differentStdOutAndStdErr ? ::eOSProcessSpawnFlags.DifferentStdOutAndStdErr : 0)|
            (_options.detached ? ::eOSProcessSpawnFlags.Detached : 0)
        )
        if (!_proc) {
          throw "Couldn't spawn process from command line: " + _cmd
        }
        _procStdout <- _proc.file[1]
        _procStderr <- (_options.differentStdOutAndStdErr) ? _proc.file[2] : null
        if (!::lang.isValid(_procStderr) && !::lang.isValid(_procStdout)) {
          throw "Spawn process couldn't get valid stdout/stdin !"
        }
      }

      function empty() {
        return _cmd.?empty()
      }

      function _nexti(itr) {
        if (itr == null) {
          // initialize _fileStart if not already done
          runCommand()
          // don't reset the counter here...
          itr = [_count,_count]
        }

        if (itr[1].?stopped) {
          return null;
        }

        local validCount = 0
        local r = {
          pid = _proc.?pid
          proc = _proc
          stopped = false
          succeeded = null
          exitCode = invalid
          stdoutLine = ""
          stderrLine = ""
        }

        // drain stderr
        if (::lang.isValid(_procStderr) && !_procStderr.partial_read) {
          ++validCount
          local line = _procStderr.ReadStringLine()
          if (!line.?empty()) {
            if (_debugEchoAll) {
              ::dbg("D/SEQ-STDERR: " + line + "\n")
            }
            r.stderrLine = line
          }
        }

        // drain stdout
        if (::lang.isValid(_procStdout) && !_procStdout.partial_read) {
          if (_options.drainStdErrBeforeStdIn
              && (::lang.isValid(_procStderr) && !_procStderr.partial_read)
              && (validCount > 0))
          {
          }
          else {
            ++validCount
            local line = _procStdout.ReadStringLine()
            if (!line.?empty()) {
              if (_debugEchoAll) {
                ::dbg("D/SEQ-STDOUT: " + line + "\n")
              }
              r.stdoutLine = line
            }
          }
        }

        if (validCount == 0) {
          local ret = _proc.WaitForExitCode(invalid)
          r.stopped = true
          r.succeeded = ret.x.toint()
          r.exitCode = ret.y.toint()
        }

        itr[0] = _count++;
        itr[1] = r;
        return itr;
      }
    }

    r.__SetCanCallMetaMethod(true)
    return r
  }

  function _writeRawBashScriptToTempFile(aScript) {
    local tmpFilePath = getNewTempFilePath("sh")
    ::fs.writeString(aScript,tmpFilePath)
    return tmpFilePath
  }

  function runRawBash(aScript,abKeepStdOut,abEchoStdout) {
    local tmpFilePath = _writeRawBashScriptToTempFile(aScript)
    if (_debugEchoAll) {
      ::dbg(::format("I/Running from %s\n-----------------------\n%s\n-----------------------"
                     tmpFilePath, aScript));
    }
    return runProcess(
      getBashPath().quote() + " " + tmpFilePath.quote(),
      abKeepStdOut,abEchoStdout)
  }

  function runDetachedRawBash(aScript) {
    local tmpFilePath = _writeRawBashScriptToTempFile(aScript)
    {
      ::dbg(::format("I/Running detached from %s\n-----------------------\n%s\n-----------------------"
                     tmpFilePath, aScript));
    }
    return runDetachedProcess(
      getBashPath().quote() + " " + tmpFilePath.quote())
  }

  function seqRawBash(aScript,aOptions) {
    local tmpFilePath = _writeRawBashScriptToTempFile(aScript)
    if (_debugEchoAll) {
      ::dbg(::format("I/Running from %s\n-----------------------\n%s\n-----------------------"
                     tmpFilePath, aScript));
    }
    return seqProcess(
      getBashPath().quote() + " " + tmpFilePath.quote(),
      aOptions)
  }

  function _writeHamBashScriptToTempFile(aScript) {
    local text = ""

    // Make the script abort if any command fails
    text += "set -e\n";

    text += ::format({[export HAM_HOME="%s"]},getHamHome()) + "\n"
    // text += ::format({[echo ... HAM_HOME: $HAM_HOME]}) + "\n"

    text += ::format({[export WORK="%s"]},getHamHome().removedirback()) + "\n"
    // text += ::format({[echo ... WORK: $WORK]}) + "\n"

    text += ::format({[export BUILD_BIN_LOA="%s"]},getOSArch()) + "\n"
    text += ::format({[export BUILD="%s"]},getBuild()) + "\n"

    text += {[export BASH_START_PATH=""]} + "\n"
    text += {[export BASH_START_SILENT="yes"]} + "\n"
    text += {[. "$HAM_HOME/bin/ham-bash-setenv.sh"]} + "\n"

    text += aScript

    return _writeRawBashScriptToTempFile(text);
  }

  function runBash(aScript,abKeepStdOut,abEchoStdout) {
    local tmpFilePath = _writeHamBashScriptToTempFile(aScript)
    if (_debugEchoAll) {
      ::dbg(::format("I/Running from %s\n-----------------------\n%s\n-----------------------"
                     tmpFilePath, aScript));
    }
    return runProcess(
      getBashPath().quote() + " " + tmpFilePath.quote(),
      abKeepStdOut,abEchoStdout)
  }

  function runDetachedBash(aScript) {
    local tmpFilePath = _writeHamBashScriptToTempFile(aScript)
    if (_debugEchoAll) {
      ::dbg(::format("I/Running detached from %s\n-----------------------\n%s\n-----------------------"
                     tmpFilePath, aScript));
    }
    return runDetachedProcess(
      getBashPath().quote() + " " + tmpFilePath.quote())
  }

  function seqBash(aScript,_aOptions) {
    local tmpFilePath = _writeHamBashScriptToTempFile(aScript)
    if (_debugEchoAll) {
      ::dbg(::format("I/Running from %s\n-----------------------\n%s\n-----------------------"
                     tmpFilePath, aScript));
    }
    return seqProcess(
      getBashPath().quote() + " " + tmpFilePath.quote(),
      _aOptions)
  }
}

::?LINT_CHECK_TYPE("null", ::?ham);
::ham <- module

function ::bash(aScript) {
  return ::ham.runBash(aScript,true,true)
}
function ::bashDetached(aScript) {
  return ::ham.runDetachedBash(aScript)
}
function ::bashSeq(aScript) {
  return ::ham.seqBash(aScript, null)
}
