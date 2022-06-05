// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niScriptCpp/ScriptCpp.h"

#if defined niNoScriptCpp

namespace ni {

niExportFunc(ni::iUnknown*) New_ScriptingHost_Cpp(const Var& /*avarA*/, const Var& /*avarB*/) {
  niError("ScriptCpp not supported on this platform.");
  return NULL;
}

}
#else

#include <niLang/StringDef.h>
#include <niLang/Utils/Path.h>
#include <niLang/Utils/ModuleUtils.h>
#include <niLang/Utils/FileEnum.h>
#include <niLang/Utils/Trace.h>
#include <niLang/ITime.h>

namespace ni {

niDeclareModuleTrace_(niScriptCpp,Trace);
#define SCRIPTCPP_TRACE(X) niModuleTrace_(niScriptCpp,Trace,X)

typedef ni::iUnknown* (__ni_export_call_decl *tpfnNewInstance)();

#ifdef _DEBUG
#define SCRIPTCPP_BUILD_DA
#endif

static cString _GetBinDir() {
  cString dir;
  dir << "bin/"
      << GetLang()->GetProperty("ni.loa.os")
      << "-"
      << GetLang()->GetProperty("ni.loa.arch");
  return dir;
}

niExportFunc(tBool) ScriptCpp_GetCompileEnabled() {
  return ni::GetProperty(SCRIPTCPP_COMPILE_PROPERTY,"0").Bool();
}

niExportFuncCPP(cString) ScriptCpp_GetCompileModuleType() {
  return ni::GetProperty(SCRIPTCPP_MODULE_TYPE_PROPERTY,niModuleFileNameBuildType);
}

static cString _GetModuleFileName(const cString& aModuleName) {
  return ni::GetModuleFileName(aModuleName.Chars(), niDLLSuffix, ScriptCpp_GetCompileModuleType().Chars());
}

static sScriptCppStats _scriptCppStates;

niExportFunc(sScriptCppStats*) ScriptCpp_GetStats() {
  return &_scriptCppStates;
}

inline cString ToString(iTime* apToString) {
  return apToString ? apToString->Format(NULL) : _ASTR("NULLTIME");
}

inline tBool EnvCopyIfExists(tStringCMap* envMap, iLang* apLang, const achar* aEnvVarName) {
  cString v = apLang->GetEnv(aEnvVarName);
  if (v.IsNotEmpty()) {
    astl::upsert(*envMap, aEnvVarName, v);
    return eTrue;
  }
  else {
    return eFalse;
  }
}

static cString _GetStampString(const iTime* time) {
  return niFmt("%04d%02d%02d_%02d%02d%02d_rtcpp",
               time->GetYear(),
               time->GetMonth(),
               time->GetDay(),
               time->GetHour(),
               time->GetMinute(),
               time->GetSecond());
}

static Ptr<iTime> _GetFileTime(const cString& strFilePath) {
  Ptr<iFile> fp = GetRootFS()->FileOpen(strFilePath.Chars(),eFileOpenMode_Read);
  if (!fp.IsOK()) {
    return NULL;
  }
  Ptr<iTime> fileTime = GetLang()->GetCurrentTime()->Clone();
  fp->GetTime(eFileTime_LastWrite,fileTime);
  return fileTime;
}

static cString _FindHamPath(cString& hamHome) {
  hamHome = GetLang()->GetEnv("HAM_HOME");
  if (hamHome.IsEmpty()) {
    hamHome = GetRootFS()->GetAbsolutePath(
        (ni::GetLang()->GetProperty("ni.dirs.bin") + "../../../ham").Chars());
    SCRIPTCPP_TRACE(("hamHome from ni.dirs.bin: %s", hamHome));
  }
  else {
    SCRIPTCPP_TRACE(("hamHome from envvar: %s", hamHome));
  }

  cPath hamPath;
  hamPath.SetDirectory(hamHome.Chars());
  hamPath.AddDirectoryBack("bin");
  hamPath.SetFile(SCRIPTCPP_HAMEXE);
  if (!ni::GetRootFS()->FileExists(hamPath.GetPath().Chars(),eFileAttrFlags_AllFiles)) {
    niWarning(niFmt("Can't find " SCRIPTCPP_HAMEXE " script '%s'.",hamPath.GetPath()));
    return AZEROSTR;
  }

  return hamPath.GetPath();
}

static tBool ScriptCpp_TryCompileSource(
  sScriptCppModuleCache& mc,
  const cString& strSourcePath,
  const cString& strSourceAppDir)
{
  if (strSourcePath.IsEmpty()) {
    niError(niFmt("Module '%s': Can't find source path.", mc.name));
    return eFalse;
  }

  if (!mc.date.IsOK()) {
    mc.date = _GetFileTime(mc.path);
    if (!mc.date.IsOK()) {
      niError(niFmt("Module '%s': Can't get datetime.", mc.name));
      return eFalse;
    }
  }
  SCRIPTCPP_TRACE(("Module '%s', date: '%s', path: '%s'.",mc.name,ToString(mc.date.ptr()),mc.path));

  Ptr<iTime> sourceTime = _GetFileTime(strSourcePath);
  if (!sourceTime.IsOK()) {
    niError(niFmt("Module '%s': Can't get datetime of source: %s.",
                  mc.name, strSourcePath));
    return eFalse;
  }
  SCRIPTCPP_TRACE(("Module '%s', source date: '%s', source: %s.",mc.name,ToString(sourceTime.ptr()),strSourcePath));

  if (mc.date->Compare(sourceTime) >= 0) {
    ++_scriptCppStates._numUpToDate;
    SCRIPTCPP_TRACE(("'%s' up-to-date.",mc.path));
    if (!mc.hDLL) {
      mc.hDLL = ni_dll_load(mc.path.Chars());
      if (!mc.hDLL) {
        niError(niFmt("Module '%s': Can't load up-to-date dll: %s", mc.name, mc.path));
        return eFalse;
      }
      SCRIPTCPP_TRACE(("Loaded up-to-date DLL '%s'.",mc.path));
    }
    return eTrue;
  }
  else {
    ++_scriptCppStates._numOutOfDate;
    SCRIPTCPP_TRACE(("'%s' outdated by '%s'.",mc.path,strSourcePath));
  }

  cString hamHome;
  cString hamPath = _FindHamPath(hamHome);
  if (hamPath.IsEmpty()) {
    niError(niFmt("Module '%s': Can't find ham.", mc.name));
    return eFalse;
  }
  SCRIPTCPP_TRACE(("HAM_HOME: %s", hamHome));

  cString hamCmd;

  {
    cPath pathBash;
#if defined niWindows
    pathBash.SetDirectory(hamHome.Chars());
    pathBash.AddDirectoryBack("bin/nt-x86");
    pathBash.SetFile("bash.exe");
#elif defined niOSX
    pathBash.SetFile("/bin/bash");
#else
    {
      niError(niFmt("Module '%s': This platform can't runtime compile cpp.", mc.name));
      return eFalse;
    }
#endif

    SCRIPTCPP_TRACE(("pathBash: %s", pathBash.GetPath()));
    hamCmd << "\"" << pathBash.GetPath() << "\" ";
  }

  const cString stamp = _GetStampString(sourceTime);

  Ptr<tStringCMap> envMap = ni::GetOSProcessManager()->GetEnviron();
  astl::upsert(*envMap, "HAM_HOME", hamHome);
  EnvCopyIfExists(envMap, ni::GetLang(), "CLANG_SANITIZE");

  cString strSourceAppDirUrl;
  StringEncodeUrl(strSourceAppDirUrl,strSourceAppDir);

  hamCmd << "\"" << hamPath << + "\""
         << " -q"
         << " -T default"
         << " -D " << strSourceAppDirUrl
         << " RTCPP=1"
         << " BUILD=" << ScriptCpp_GetCompileModuleType()
         << " pass1"
         << " STAMP=" << stamp
         << " " << mc.name
      ;

  SCRIPTCPP_TRACE(("hamPath: %s", hamPath));
  SCRIPTCPP_TRACE(("hamCmd: %s", hamCmd));

  cPath pathOutputNotStamped;
  cPath pathOutput;
  {
    pathOutput.SetDirectory(strSourceAppDir.Chars());
    pathOutput.AddDirectoryBack(_GetBinDir().Chars());
    pathOutput.SetFile(_GetModuleFileName(mc.name).Chars());
    pathOutputNotStamped = pathOutput;
    // add the stamp
    const cString ext = pathOutput.GetExtension();
    pathOutput.SetExtension("");
    pathOutput.SetFile((pathOutput.GetFile() + "-" + stamp).Chars());
    pathOutput.SetExtension(ext.Chars());
  }

  SCRIPTCPP_TRACE(("outputPathNotStamped: %s", pathOutputNotStamped.GetPath()));
  SCRIPTCPP_TRACE(("outputPath: %s", pathOutput.GetPath()));

  Ptr<iOSProcessManager> pm = GetLang()->GetProcessManager();
  sVec2i procRet = {eFalse,0};
  Ptr<iOSProcess> proc = pm->SpawnProcessEx(
    hamCmd.Chars(), NULL, envMap, eOSProcessSpawnFlags_StdFiles|eOSProcessSpawnFlags_Detached);
  if (proc.IsOK()) {
    niLog(Raw, "=== Build Output ===");
    // drain stdout...
    Ptr<iFile> procStdOut = proc->GetFile(eOSProcessFile_StdOut);
    Ptr<iFile> procStdErr = proc->GetFile(eOSProcessFile_StdErr);
    while (1) {
      if (procStdOut.IsOK() && procStdOut->GetCanRead()) {
        // SCRIPTCPP_TRACE(("Reading stdout."));
        cString line = procStdOut->ReadStringLine();
        niLog(Raw, line.Chars());
      }
      if (procStdErr.IsOK() && procStdErr->GetCanRead()) {
        // SCRIPTCPP_TRACE(("Reading stderr."));
        cString line = procStdErr->ReadStringLine();
        niLog(Raw, line.Chars());
      }

      procRet = proc->WaitForExitCode(0);
      if (procRet.x) {
        SCRIPTCPP_TRACE(("Ham exited: %s.", procRet));
        break;
      }
    }
  }
  if (procRet.x && procRet.y == 0) {
    niLog(Info,niFmt("Module '%s': build succeeded: %s", mc.name, pathOutput.GetPath()));
    mc.path = pathOutput.GetPath().Chars();
    mc.date = NULL;
    mc.hDLL = ni_dll_load(mc.path.Chars());
    if (!mc.hDLL) {
      niError(niFmt("Module '%s': Can't load compiled dll '%s'.", mc.name, mc.path));
      return eFalse;
    }
  }
  else {
    niError(niFmt("Module '%s': Build of '%s' failed.", mc.name, pathOutput.GetPath()));
    return eFalse;
  }

  ++_scriptCppStates._numCompiled;
  return eTrue;
}

static void _ScriptCpp_CleanupDLLs(const achar* aDir) {
  if (!niStringIsOK(aDir))
    return;

  cPath path;
  path.SetDirectory(aDir);
  if (!ni::GetRootFS()->FileExists(path.GetPath().c_str(), eFileAttrFlags_AllDirectories)) {
    SCRIPTCPP_TRACE(("ScriptCpp Cleanup: Can't find directory: %s", path.GetPath()));
    return;
  }
  path.SetFile("*_*-*_rtcpp.*");
  SCRIPTCPP_TRACE(("ScriptCpp Cleanup: DLLs: %s", path.GetPath()));

  FindFile ff;
  if (ff.First(path.GetPath().Chars()))
    do {
      path.SetDirectory(ni::GetLang()->GetProperty("ni.dirs.bin").Chars());
      path.SetFile(ff.FileName());
      tBool r = ni::GetRootFS()->FileDelete(path.GetPath().Chars());
      niLog(Info, niFmt("ScriptCpp Cleanup: Removing artifact %s: %s",
                        r ? "succeeded" : "failed",
                        path.GetPath()));
    } while(ff.Next());
}

niExportFunc(void) ScriptCpp_CleanupDLLs() {
  _ScriptCpp_CleanupDLLs(ni::GetLang()->GetProperty("ni.dirs.app").Chars());
  _ScriptCpp_CleanupDLLs(ni::GetLang()->GetProperty("ni.dirs.bin").Chars());
}

static const char* _BinDirProps[] = {
  "ni.dirs.app",
  "ni.dirs.bin",
};

static const tBool _IsBinDir(const cString& strBinDir) {
  return strBinDir.contains("/bin/") || strBinDir.EndsWithI("/MacOS/");
}

static void _FindSourcePathAndAppDir(
    const cString& strSourceFileName,
    cString& strSourcePath,
    cString& strAppDir)
{
  niLoop(i,niCountOf(_BinDirProps)) {
    strAppDir = ni::GetLang()->GetProperty(_BinDirProps[i]);
    if (strAppDir.IsEmpty())
      continue;
    cPath pathSourceFileName;
    pathSourceFileName.SetDirectory(strAppDir.Chars());
    if (_IsBinDir(strAppDir)) {
      pathSourceFileName.RemoveDirectoryBack();
      pathSourceFileName.RemoveDirectoryBack();
      strAppDir = pathSourceFileName.GetDirectory();
    }
    pathSourceFileName.AddDirectoryBack("sources");
    pathSourceFileName.AddDirectoryBack(strSourceFileName.RBefore("/").Chars());
    pathSourceFileName.SetFile(strSourceFileName.RAfter("/").Chars());
    SCRIPTCPP_TRACE(("Trying '%s' source path '%s'",
                 _BinDirProps[i],
                 pathSourceFileName.GetPath()));
    if (ni::GetRootFS()->FileExists(pathSourceFileName.GetPath().Chars(),eFileAttrFlags_AllFiles)) {
      strSourcePath = pathSourceFileName.GetPath();
      return;
    }
  }
}

static cString _FindModulePath(const cString& strModuleFileName) {
  niLoop(i,niCountOf(_BinDirProps)) {
    cString strAppDir = ni::GetLang()->GetProperty(_BinDirProps[i]);
    if (strAppDir.IsNotEmpty()) {
      cPath pathAppModuleFileName;
      pathAppModuleFileName.SetDirectory(strAppDir.Chars());
      if (!_IsBinDir(strAppDir)) {
        pathAppModuleFileName.AddDirectoryBack(_GetBinDir().Chars());
      }
      pathAppModuleFileName.SetFile(strModuleFileName.Chars());
      SCRIPTCPP_TRACE(("Trying '%s' module path '%s'",
                  _BinDirProps[i],
                  pathAppModuleFileName.GetPath()));
      if (ni::GetRootFS()->FileExists(pathAppModuleFileName.GetPath().Chars(),eFileAttrFlags_AllFiles)) {
        return pathAppModuleFileName.GetPath();
      }
    }
  }

  return AZEROSTR;
}

struct CppScriptingHost : public cIUnknownImpl<iScriptingHost> {
  niBeginClass(CppScriptingHost);

  tScriptCppModuleMap _modules;

  CppScriptingHost() {
  }
  ~CppScriptingHost() {
    this->Cleanup();
  }

  virtual tBool __stdcall Cleanup() {
    if (ScriptCpp_GetCompileEnabled()) {
      ScriptCpp_CleanupDLLs();
    }
    return eTrue;
  }

  virtual tBool __stdcall EvalString(iHString* ahspContext, const ni::achar* aaszCode) {
    return eFalse;
  }

  virtual tBool __stdcall CanEvalImpl(iHString* ahspContext, iHString* ahspCodeResource) {
    return StrEndsWith(niHStr(ahspCodeResource),".cpp") ||
        StrEndsWith(niHStr(ahspCodeResource),".cni");
  }

  // DIR/FILENAME.cpp -> MODULE=DIR, CLASS=FILENAME
  // MODULE#DIR/FILENAME.cpp -> CLASS=FILENAME
  // MODULE#CLASS#DIR/FILENAME.cpp
  virtual iUnknown* __stdcall EvalImpl(iHString* ahspContext, iHString* ahspCodeResource, const tUUID& aIID) {
    cString strClass;
    cString strResource = niHStr(ahspCodeResource);
    cString strModule = strResource.Before("/");
    if (strResource.contains("#")) {
      astl::vector<cString> toks;
      StringSplitSep(strResource,"#",&toks);
      if (toks.size() == 2) {
        strModule = toks[0];
        strResource = toks[1];
        strClass = strResource.RAfter("/").RBefore(".");
      }
      else if (toks.size() == 3) {
        strModule = toks[0];
        strClass = toks[1];
        strResource = toks[2];
      }
      else {
        niError(niFmt("Invalid ScriptCpp '#' resource definition '%s'. Expected 2 or 3 part, but got %d.", ahspCodeResource, toks.size()));
        return NULL;
      }
    }
    else {
      strClass = strResource.RAfter("/").RBefore(".");
    }
    cString strModuleFileName = _GetModuleFileName(strModule);
    cString strCreateFunctionName = _ASTR("New_") + niHStr(ahspContext) + "_" + strClass;
    cString strSourcePath, strSourceAppDir;
    _FindSourcePathAndAppDir(strResource.Chars(),strSourcePath,strSourceAppDir);

    SCRIPTCPP_TRACE(("Context: %s, Resource: %s, UUID: %s, Module: %s, ModuleFile: %s, Class: %s, CreateFun: %s, SourcePath: %s, SourceAppDir: %s",
                ahspContext, ahspCodeResource, aIID,
                strModule, strModuleFileName,
                strClass, strCreateFunctionName,
                strSourcePath, strSourceAppDir));

    tScriptCppModuleMap::iterator itModule = _modules.find(strModuleFileName);
    if (itModule != _modules.end()) {
      SCRIPTCPP_TRACE(("Using already loaded module"));
    }
    else {
      sScriptCppModuleCache newModule;
      newModule.name = strModule;
      newModule.path = _FindModulePath(strModuleFileName);
      itModule = astl::upsert(_modules,strModuleFileName,newModule);
    }

    if (ScriptCpp_GetCompileEnabled()) {
      if (!ScriptCpp_TryCompileSource(itModule->second,strSourcePath,strSourceAppDir)) {
        niWarning(niFmt("Can't compile module '%s' for code resource '%s'.",
                        strModule, ahspCodeResource));
      }
    }
    else {
      SCRIPTCPP_TRACE(("ScriptCpp compile disabled. (use -D" SCRIPTCPP_COMPILE_PROPERTY "=1 to enable it)"));
    }

    {
      sScriptCppModuleCache& mc = itModule->second;
      if (!mc.hDLL && !mc.path.IsEmpty()) {
        mc.hDLL = ni_dll_load(mc.path.Chars());
        if (!mc.hDLL) {
          niError(niFmt("Can't load module file '%s' for code resource '%s'.",
                        strModule, ahspCodeResource));
          return NULL;
        }
      }
      if (!mc.hDLL) {
        niError(niFmt("Can't find module file '%s' for code resource '%s'.",
                      strModule, ahspCodeResource));
        return NULL;
      }
    }

    tpfnNewInstance pfnNewInstance = (tpfnNewInstance)ni_dll_get_proc(
        itModule->second.hDLL,strCreateFunctionName.Chars());
    if (!pfnNewInstance) {
      niError(niFmt("Can't get function '%s' from module file '%s' for code resource '%s'.",
                    strCreateFunctionName, strModule, ahspCodeResource));
      return NULL;
    }

    Ptr<iUnknown> ptrInst = pfnNewInstance();
    if (!ptrInst.IsOK()) {
      niError(niFmt("Can't create instance with function '%s' from module file '%s' for code resource '%s'.",
                    strCreateFunctionName, strModule, ahspCodeResource));
      return NULL;
    }

    if (aIID != kuuidZero) {
      ptrInst = ptrInst->QueryInterface(aIID);
      if (!ptrInst.IsOK()) {
        const sInterfaceDef* pInterfaceDef = ni::GetLang()->GetInterfaceDefFromUUID(aIID);
        niError(niFmt("Can't query interface '%s' (%s) on instance created by function '%s' from module file '%s' for code resource '%s'.",
                      pInterfaceDef ? pInterfaceDef->maszName : "",
                      aIID,
                      strCreateFunctionName, strModule, ahspCodeResource));
        return NULL;
      }
    }

    return ptrInst.GetRawAndSetNull();
  }

  virtual void __stdcall Service(tBool abForceGC) {
  }

  niEndClass(CppScriptingHost);
};

niExportFunc(iScriptingHost*) ScriptCpp_CreateScriptingHost() {
  return niNew CppScriptingHost();
}

niExportFunc(ni::iUnknown*) New_ScriptingHost_Cpp(const Var& /*avarA*/, const Var& /*avarB*/) {
  return ScriptCpp_CreateScriptingHost();
}

}
#endif
