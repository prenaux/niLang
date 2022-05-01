// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "CppScriptingHost.h"

#if !defined niNoRTCpp

#include "API/niLang/Utils/RTCppImpl.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/Utils/ModuleUtils.h"
#include "API/niLang/Utils/FileEnum.h"
#include "API/niLang/Utils/Trace.h"
#include "API/niLang/ITime.h"

using namespace ni;

niDeclareModuleTrace_(niLang,TraceRTCpp);
#define RTCPP_TRACE(X) niModuleTrace_(niLang,TraceRTCpp,X)

typedef ni::iUnknown* (__ni_export_call_decl *tpfnNewInstance)();

#define RTCPP_HAMEXE "ham-rtcpp"

#ifdef _DEBUG
#define RTCPP_BUILD_DA
#endif

static cString RTCpp_GetBinDir() {
  cString dir;
  dir << "bin/"
      << GetLang()->GetProperty("ni.loa.os")
      << "-"
      << GetLang()->GetProperty("ni.loa.arch");
  return dir;
}

niExportFunc(tBool) RTCpp_GetCompileEnabled() {
  return ni::GetProperty(RTCPP_COMPILE_PROPERTY,"0").Bool();
}

niExportFuncCPP(cString) RTCpp_GetCompileModuleType() {
  return ni::GetProperty("rtcpp.module_type",niModuleFileNameBuildType);
}

static cString _GetModuleFileName(const cString& aModuleName) {
  return ni::GetModuleFileName(aModuleName.Chars(), niDLLSuffix, RTCpp_GetCompileModuleType().Chars());
}

static sRTCppStats _rtcppStats;

niExportFunc(sRTCppStats*) RTCpp_GetStats() {
  return &_rtcppStats;
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
    RTCPP_TRACE(("hamHome from ni.dirs.bin: %s", hamHome));
  }
  else {
    RTCPP_TRACE(("hamHome from envvar: %s", hamHome));
  }

  cPath hamPath;
  hamPath.SetDirectory(hamHome.Chars());
  hamPath.AddDirectoryBack("bin");
  hamPath.SetFile(RTCPP_HAMEXE);
  if (!ni::GetRootFS()->FileExists(hamPath.GetPath().Chars(),eFileAttrFlags_AllFiles)) {
    niWarning(niFmt("Can't find " RTCPP_HAMEXE " script '%s'.",hamPath.GetPath()));
    return AZEROSTR;
  }

  return hamPath.GetPath();
}

static tBool RTCpp_TryCompileSource(
  sRTCppModuleCache& mc,
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
  RTCPP_TRACE(("Module '%s', date: '%s', path: '%s'.",mc.name,ToString(mc.date.ptr()),mc.path));

  Ptr<iTime> sourceTime = _GetFileTime(strSourcePath);
  if (!sourceTime.IsOK()) {
    niError(niFmt("Module '%s': Can't get datetime of source: %s.",
                  mc.name, strSourcePath));
    return eFalse;
  }
  RTCPP_TRACE(("Module '%s', source date: '%s', source: %s.",mc.name,ToString(sourceTime.ptr()),strSourcePath));

  if (mc.date->Compare(sourceTime) >= 0) {
    ++_rtcppStats._numUpToDate;
    RTCPP_TRACE(("'%s' up-to-date.",mc.path));
    if (!mc.hDLL) {
      mc.hDLL = ni_dll_load(mc.path.Chars());
      if (!mc.hDLL) {
        niError(niFmt("Module '%s': Can't load up-to-date dll: %s", mc.name, mc.path));
        return eFalse;
      }
      RTCPP_TRACE(("Loaded up-to-date DLL '%s'.",mc.path));
    }
    return eTrue;
  }
  else {
    ++_rtcppStats._numOutOfDate;
    RTCPP_TRACE(("'%s' outdated by '%s'.",mc.path,strSourcePath));
  }

  cString hamHome;
  cString hamPath = _FindHamPath(hamHome);
  if (hamPath.IsEmpty()) {
    niError(niFmt("Module '%s': Can't find ham.", mc.name));
    return eFalse;
  }
  RTCPP_TRACE(("HAM_HOME: %s", hamHome));

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

    RTCPP_TRACE(("pathBash: %s", pathBash.GetPath()));
    hamCmd << "\"" << pathBash.GetPath() << "\" ";
  }

  const cString stamp = _GetStampString(sourceTime);

  Ptr<tStringCMap> envMap = tStringCMap::Create();
  astl::upsert(*envMap, "HAM_HOME", hamHome);
  EnvCopyIfExists(envMap, ni::GetLang(), "CLANG_SANITIZE");

  cString strSourceAppDirUrl;
  StringEncodeUrl(strSourceAppDirUrl,strSourceAppDir);

  hamCmd << "\"" << hamPath << + "\""
         << " -T default"
         << " -D " << strSourceAppDirUrl
         << " RTCPP=1"
         << " BUILD=" << RTCpp_GetCompileModuleType()
         << " pass1"
         << " STAMP=" << stamp
         << " " << mc.name
      ;

  RTCPP_TRACE(("hamPath: %s", hamPath));
  RTCPP_TRACE(("hamCmd: %s", hamCmd));

  cPath pathOutputNotStamped;
  cPath pathOutput;
  {
    pathOutput.SetDirectory(strSourceAppDir.Chars());
    pathOutput.AddDirectoryBack(RTCpp_GetBinDir().Chars());
    pathOutput.SetFile(_GetModuleFileName(mc.name).Chars());
    pathOutputNotStamped = pathOutput;
    // add the stamp
    const cString ext = pathOutput.GetExtension();
    pathOutput.SetExtension("");
    pathOutput.SetFile((pathOutput.GetFile() + "-" + stamp).Chars());
    pathOutput.SetExtension(ext.Chars());
  }

  RTCPP_TRACE(("outputPathNotStamped: %s", pathOutputNotStamped.GetPath()));
  RTCPP_TRACE(("outputPath: %s", pathOutput.GetPath()));

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
        // RTCPP_TRACE(("Reading stdout."));
        cString line = procStdOut->ReadStringLine();
        niLog(Raw, line.Chars());
      }
      if (procStdErr.IsOK() && procStdErr->GetCanRead()) {
        // RTCPP_TRACE(("Reading stderr."));
        cString line = procStdErr->ReadStringLine();
        niLog(Raw, line.Chars());
      }

      procRet = proc->WaitForExitCode(0);
      if (procRet.x) {
        RTCPP_TRACE(("Ham exited: %s.", procRet));
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

  ++_rtcppStats._numCompiled;
  return eTrue;
}

static void _RTCpp_CleanupDLLs(const achar* aDir) {
  if (!niStringIsOK(aDir))
    return;

  cPath path;
  path.SetDirectory(aDir);
  if (!ni::GetRootFS()->FileExists(path.GetPath().c_str(), eFileAttrFlags_AllDirectories)) {
    RTCPP_TRACE(("RTCpp Cleanup: Can't find directory: %s", path.GetPath()));
    return;
  }
  path.SetFile("*_*-*_rtcpp.*");
  RTCPP_TRACE(("RTCpp Cleanup: DLLs: %s", path.GetPath()));

  FindFile ff;
  if (ff.First(path.GetPath().Chars()))
    do {
      path.SetDirectory(ni::GetLang()->GetProperty("ni.dirs.bin").Chars());
      path.SetFile(ff.FileName());
      tBool r = ni::GetRootFS()->FileDelete(path.GetPath().Chars());
      niLog(Info, niFmt("RTCpp Cleanup: Removing artifact %s: %s",
                        r ? "succeeded" : "failed",
                        path.GetPath()));
    } while(ff.Next());
}

niExportFunc(void) RTCpp_CleanupDLLs() {
  _RTCpp_CleanupDLLs(ni::GetLang()->GetProperty("ni.dirs.app").Chars());
  _RTCpp_CleanupDLLs(ni::GetLang()->GetProperty("ni.dirs.bin").Chars());
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
    RTCPP_TRACE(("Trying '%s' source path '%s'",
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
        pathAppModuleFileName.AddDirectoryBack(RTCpp_GetBinDir().Chars());
      }
      pathAppModuleFileName.SetFile(strModuleFileName.Chars());
      RTCPP_TRACE(("Trying '%s' module path '%s'",
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

  tRTCppModuleMap _modules;

  CppScriptingHost() {
  }
  ~CppScriptingHost() {
    this->Cleanup();
  }

  virtual tBool __stdcall Cleanup() {
    if (RTCpp_GetCompileEnabled()) {
      RTCpp_CleanupDLLs();
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

  virtual iUnknown* __stdcall EvalImpl(iHString* ahspContext, iHString* ahspCodeResource, const tUUID& aIID) {
    cString strClass;
    cString strResource = niHStr(ahspCodeResource);
    if (strResource.contains("#")) {
      strClass = strResource.Before("#");
      strResource = strResource.After("#");
    }
    else {
      strClass = strResource.RAfter("/").RBefore(".");
    }
    cString strModule = strResource.Before("/");
    cString strModuleFileName = _GetModuleFileName(strModule);
    cString strCreateFunctionName = _ASTR("New_") + niHStr(ahspContext) + "_" + strClass;
    cString strSourcePath, strSourceAppDir;
    _FindSourcePathAndAppDir(strResource.Chars(),strSourcePath,strSourceAppDir);

    RTCPP_TRACE(("RTCpp: EvalImpl: Context: %s, Resource: %s, UUID: %s, Module: %s, ModuleFile: %s, Class: %s, CreateFun: %s, SourcePath: %s, SourceAppDir: %s",
                ahspContext, ahspCodeResource, aIID,
                strModule, strModuleFileName,
                strClass, strCreateFunctionName,
                strSourcePath, strSourceAppDir));

    tRTCppModuleMap::iterator itModule = _modules.find(strModuleFileName);
    if (itModule != _modules.end()) {
      RTCPP_TRACE(("Using already loaded module"));
    }
    else {
      sRTCppModuleCache newModule;
      newModule.name = strModule;
      newModule.path = _FindModulePath(strModuleFileName);
      itModule = astl::upsert(_modules,strModuleFileName,newModule);
    }

    if (RTCpp_GetCompileEnabled()) {
      if (!RTCpp_TryCompileSource(itModule->second,strSourcePath,strSourceAppDir)) {
        niWarning(niFmt("Can't compile module '%s' for code resource '%s'.",
                        strModule, ahspCodeResource));
      }
    }
    else {
      RTCPP_TRACE(("RTCpp compile disabled. (use -D" RTCPP_COMPILE_PROPERTY "=1 to enable it)"));
    }

    {
      sRTCppModuleCache& mc = itModule->second;
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

niExportFunc(iScriptingHost*) RTCpp_CreateScriptingHost() {
  return niNew CppScriptingHost();
}

#endif
