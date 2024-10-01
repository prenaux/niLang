// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")

module <- {
  _rootFS = ::gLang.GetRootFS()

  tFileEntry = {
    path = "",
    attr = 0 // ::eFileAttrFlags
  }

  ///////////////////////////////////////////////
  function createStringFile(string)
  {
    local f = ::gLang.CreateFileDynamicMemory(0,"--string--")
    f.BeginTextFileWrite(::eTextEncodingFormat.Native,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }

  ///////////////////////////////////////////////
  function createStringFileEx(string,name)
  {
    local f = ::gLang.CreateFileDynamicMemory(0,name)
    f.BeginTextFileWrite(::eTextEncodingFormat.Native,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }

  ///////////////////////////////////////////////
  function createUnicodeStringFile(string)
  {
    local f = ::gLang.CreateFileDynamicMemory(0,"--string--")
    f.BeginTextFileWrite(::eTextEncodingFormat.Unicode,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }

  ///////////////////////////////////////////////
  function createUnicodeStringFileEx(string,name)
  {
    local f = ::gLang.CreateFileDynamicMemory(0,name)
    f.BeginTextFileWrite(::eTextEncodingFormat.Unicode,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }

  ///////////////////////////////////////////////
  function createUTF8StringFile(string) {
    local f = ::gLang.CreateFileDynamicMemory(0,"--string--")
    f.BeginTextFileWrite(::eTextEncodingFormat.UTF8,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }
  function createUTF8StringFileEx(string,name) {
    local f = ::gLang.CreateFileDynamicMemory(0,name)
    f.BeginTextFileWrite(::eTextEncodingFormat.UTF8,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }

  ///////////////////////////////////////////////
  function createUTF16StringFile(string) {
    local f = ::gLang.CreateFileDynamicMemory(0,"--string--")
    f.BeginTextFileWrite(::eTextEncodingFormat.UTF16LE,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }
  function createUTF16StringFileEx(string,name) {
    local f = ::gLang.CreateFileDynamicMemory(0,name)
    f.BeginTextFileWrite(::eTextEncodingFormat.UTF16LE,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }

  ///////////////////////////////////////////////
  function createUTF32StringFile(string) {
    local f = ::gLang.CreateFileDynamicMemory(0,"--string--")
    f.BeginTextFileWrite(::eTextEncodingFormat.UTF32LE,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }
  function createUTF32StringFileEx(string,name) {
    local f = ::gLang.CreateFileDynamicMemory(0,name)
    f.BeginTextFileWrite(::eTextEncodingFormat.UTF32LE,true)
    f.WriteString(string)
    f.SeekSet(0)
    return f
  }

  ///////////////////////////////////////////////
  // Returns an array of tFileEntry
  function listFiles(aPath,_aFilter,_aAttr,_aRecursive,_aAppendBaseDir)
  {
    aPath = "".setdir(aPath)
    local sink = {
      mAppendBaseDir = false
      mBaseDir = ""
      mFilter = "*.*"
      mAttr = 0
      mRecursive = false
      mResult = []
      mCurDir = [""]
      mFS = _rootFS
      function OnFound(aPath,aAttr,aSize)
      {
        if ((aAttr & ::eFileAttrFlags.Directory) && (mAttr & ::eFileAttrFlags.Directory)) {
          local t = { path = mCurDir.top()+aPath, attr = aAttr }
          if (mAppendBaseDir) {
            t.path = mBaseDir + t.path
          }
          mResult.append(t)
        }
        else if ((!(aAttr & ::eFileAttrFlags.Directory))) {
          local t = { path = mCurDir.top()+aPath, attr = aAttr }
          if (mAppendBaseDir) {
            t.path = mBaseDir + t.path
          }
          mResult.append(t)
        }
        if (mRecursive && (aAttr & ::eFileAttrFlags.Directory)) {
          local newPath = mCurDir.top().adddirback(aPath)
          mCurDir.append(newPath)
          mFS.FileEnum("".setdir(mBaseDir+mCurDir.top()).setfile(mFilter),
                       mAttr|::eFileAttrFlags.Directory,this)
          mCurDir.pop()
        }
        return true
      }
    }
    sink.mAppendBaseDir = _aAppendBaseDir;
    sink.mBaseDir = aPath;
    sink.mRecursive = _aRecursive;
    sink.mFilter = _aFilter || "*.*";
    sink.mAttr = (_aAttr == null) ? ::eFileAttrFlags.AllFiles : _aAttr;

    _rootFS.FileEnum(
      aPath.setfile(sink.mFilter),
      sink.mAttr|(_aRecursive?::eFileAttrFlags.Directory:0),
      sink)
    return sink.mResult
  }

  ///////////////////////////////////////////////
  function homeFilePath(aAppName,aPath) {
    local path = "";
    path = path.adddirback(::gLang.property["ni.dirs.home"])
    path = path.adddirback(aAppName)
    path += aPath;
    return path
  }

  ///////////////////////////////////////////////
  function homeFileOpenWrite(aAppName,aPath,_aAppend) {
    local path = "";
    path = path.adddirback(::gLang.property["ni.dirs.home"])
    path = path.adddirback(aAppName)
    _rootFS.FileMakeDir(path)
    path += aPath;
    return fileOpenWrite(path,_aAppend)
  }

  ///////////////////////////////////////////////
  function homeFileOpenRead(aAppName,aPath) {
    local path = homeFilePath(aAppName,aPath)
    return fileOpenRead(path)
  }

  ///////////////////////////////////////////////
  // Open a file in write mode
  function fileOpenWrite(aPath,_aAppend)
  {
    if (typeof(aPath) == "iunknown") {
      local fp = aPath.QueryInterface("iFile")
      if (!fp || !(fp.file_flags&::eFileFlags.Write)) {
        throw "Given file doesn't have write access.";
      }
      return fp
    }
    else {
      return _rootFS.FileOpen(aPath,::eFileOpenMode.Write|(_aAppend?::eFileOpenMode.Append:0))
    }
  }

  ///////////////////////////////////////////////
  // Open a file in read mode
  function fileOpenRead(aPath) {
    if (typeof(aPath) == "iunknown") {
      local fp = aPath.QueryInterface("iFile")
      if (!fp || !(fp.file_flags&::eFileFlags.Read)) {
        throw "Given file doesn't have read access.";
      }
      return fp
    }
    else {
      return _rootFS.FileOpen(aPath,::eFileOpenMode.Read)
    }
  }

  ///////////////////////////////////////////////
  function getAbsolutePath(aPath) {
    return _rootFS.GetAbsolutePath(aPath)
  }

  ///////////////////////////////////////////////
  // Get the size of the specified file
  function fileSize(aPath) {
    if (!aPath.?len()) return -1
    return _rootFS.FileSize(aPath)
  }

  ///////////////////////////////////////////////
  // Check if a file exists
  function fileExists(aPath) {
    if (!aPath.?len())
      return false
    return _rootFS.FileExists(aPath,::eFileAttrFlags.AllFiles)
  }

  ///////////////////////////////////////////////
  // Check if a directory exists
  function dirExists(aPath) {
    return _rootFS.FileExists("".setdir(aPath),::eFileAttrFlags.AllDirectories)
  }

  ///////////////////////////////////////////////
  function makeDir(aPath) {
    return _rootFS.FileMakeDir(aPath)
  }

  ///////////////////////////////////////////////
  function writeString(aString,aPath,_aAppend)
  {
    local fp = ::fs.fileOpenWrite(aPath,_aAppend)
    if (!fp) throw "Can't open file '"+aPath+"'."
    fp.WriteString(aString)
  }

  ///////////////////////////////////////////////
  function readString(aPath)
  {
    local fp = ::lang.urlOpen(aPath)
    if (!fp) throw "Can't open file '"+aPath+"'."
    return fp.ReadString()
  }

  ///////////////////////////////////////////////
  // Get the temporary directory
  function getTempDir() {
    local env = ::gLang.GetEnv("TEMP")
    local dir = "".setdir(env)
    return dir
  }

  ///////////////////////////////////////////////
  function fileTime(file) {
    local fp = ::lang.urlOpen(file)
    local time = ::gLang.current_time.Clone()
    fp.GetTime(::eFileTime.LastWrite,time)
    return time
  }

  ///////////////////////////////////////////////
  function fileTimeCompare(fileA, fileB) {
    local fpA = ::lang.urlOpen(fileA)
    if (!fpA) return -1
    local timeA = ::gLang.current_time.Clone()
    fpA.GetTime(::eFileTime.LastWrite,timeA)
    local fpB = ::lang.urlOpen(fileB)
    if (!fpB) return -1
    local timeB = ::gLang.current_time.Clone()
    fpB.GetTime(::eFileTime.LastWrite,timeB)
    return timeA.Compare(timeB)
  }
}

::LINT_CHECK_TYPE("null", ::?gRootFS);
::gRootFS <- module._rootFS
::LINT_CHECK_TYPE("null", ::?fs);
::fs <- module
