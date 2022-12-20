#include "Lang.h"

#if defined niWin32
#  ifdef niWindows
#    include <windows.h>
#  endif
#  include <process.h>
#elif defined niLinux || defined niQNX
#  include <unistd.h>
#elif defined niOSX || defined niIOS
#  include <sys/param.h>
#  include <sys/sysctl.h>
#elif defined niJSCC
#  include <limits.h>
#  include <unistd.h>
#else
#  error "Unknown platform."
#endif

#ifdef niAndroid
#define JVM_ONLY
#endif
#if defined JVM_ONLY && !defined niJNI
#  error "JVM_ONLY defined but not niJNI."
#endif
#ifdef niJNI
#  include "API/niLang/Utils/JNIUtils.h"
#endif

#include "FileFd.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/Utils/CmdLine.h"

using namespace ni;

#if defined niIOS || defined niOSX
achar* _AppleGetDirHome(achar* buffer);
achar* _AppleGetDirTemp(achar* buffer);
achar* _AppleGetDirDocs(achar* buffer);
achar* _AppleGetDirDownloads(achar* buffer);
cString _AppleGetInfoPlistPropertyValue(const achar* aProperty);
#endif

///////////////////////////////////////////////
static ni::tI32 _GetNumProcessors() {
  unsigned int numProcessors = 1;
#if defined niWinDesktop
  {
    SYSTEM_INFO SysInfo;
    ZeroMemory(&SysInfo, sizeof (SYSTEM_INFO));
    GetSystemInfo(&SysInfo);
    numProcessors = SysInfo.dwNumberOfProcessors;
  }
#elif defined niQNX
  {
    numProcessors = 2; // TODO: Find a better way for core counts
  }
#elif defined niLinux
  {
    numProcessors = sysconf(_SC_NPROCESSORS_ONLN);
  }
#elif defined niOSX || defined niIOS
  {
    int mib[4];
    size_t len = sizeof(numProcessors);
    /* set the mib for hw.ncpu */
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;
    /* get the number of CPUs from the system */
    sysctl(mib, 2, &numProcessors, &len, NULL, 0);
    if (numProcessors < 1) {
      mib[1] = HW_NCPU;
      sysctl(mib, 2, &numProcessors, &len, NULL, 0);
      if (numProcessors < 1) {
        numProcessors = 1;
      }
    }
  }
#elif defined niJSCC
  numProcessors = 1;
#else
#  error "Unknown platform."
#endif
  return numProcessors;
}

///////////////////////////////////////////////
niExportFunc(achar*) FixSystemDir(achar* aaszOutput, const achar* aaszBuffer, const achar* aaszToCreate, tBool abCreate) {
  achar tmp[AMAX_PATH] = AZEROSTR;
  if (!aaszBuffer || !aaszOutput)
    return NULL;
  StrCat(tmp,aaszBuffer);
  StrPutPathSep(tmp);
  if (niIsStringOK(aaszToCreate)) {
    StrCat(tmp,aaszToCreate);
    StrPutPathSep(tmp);
    if (abCreate) {
      if (!ni::GetRootFS()->FileMakeDir(tmp)) {
        return NULL;
      }
    }
  }
  StrFixPath(aaszOutput,tmp,AMAX_SIZE);
  return aaszOutput;
}

///////////////////////////////////////////////
#if !defined JVM_ONLY
static achar* _GetHomeDir(achar* aaszOutput) {
#  if defined niIOS || defined niOSX
  achar buffer[AMAX_PATH] = {0};
  _AppleGetDirHome(buffer);
  return FixSystemDir(aaszOutput,buffer,NULL,eFalse);
#  elif defined niWinDesktop
  achar buffer[AMAX_PATH] = {0};
  BOOL res = ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_DEFAULT, buffer);
  if (!res) {
    return NULL;
  }
  return FixSystemDir(aaszOutput,buffer,NULL,eFalse);
#  else
  cString strHome = agetenv(_A("HOME"));
  if (strHome.empty())
    return NULL;
  return FixSystemDir(aaszOutput,strHome.Chars(),NULL,eFalse);
#  endif
}
#endif

///////////////////////////////////////////////
static achar* _GetDownloadsDir(achar* aaszOutput) {
#if defined niIOS || defined niOSX
  achar buffer[AMAX_PATH] = {0};
  _AppleGetDirDownloads(buffer);
  return FixSystemDir(aaszOutput,buffer,NULL,eFalse);
#elif defined niWinDesktop
  achar buffer[AMAX_PATH] = {0};
  BOOL res = ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_DEFAULT, buffer);
  if (!res) {
    return NULL;
  }
  return FixSystemDir(aaszOutput,buffer,"Downloads",eTrue);
#else
  return NULL;
#endif
}

///////////////////////////////////////////////
static achar* _GetTempDir(achar* aaszOutput) {
  // macOS: not using the system provided temp dir because the path is random
  // and its impossible to find when we need it for debugging
#if 0 && (defined niIOS || defined niOSX)
  achar buffer[AMAX_PATH] = {0};
  _AppleGetDirTemp(buffer);
  return FixSystemDir(aaszOutput,buffer,NULL,eFalse);
#else
  return NULL;
#endif
}

///////////////////////////////////////////////
static achar* _GetDocumentsDir(achar* aaszOutput) {
#if defined niIOS || defined niOSX
  achar buffer[AMAX_PATH] = {0};
  _AppleGetDirDocs(buffer);
  return FixSystemDir(aaszOutput,buffer,NULL,eFalse);
#elif defined niWinDesktop
  achar buffer[AMAX_PATH] = {0};
  // CSIDL_PROFILE: %USERPROFILE%
  // CSIDL_PERSONAL: %MYDOCUMENTS%
  BOOL res = ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_DEFAULT, buffer);
  if (!res) {
    return NULL;
  }
  return FixSystemDir(aaszOutput,buffer,NULL,eFalse);
#else
  return NULL;
#endif
}

///////////////////////////////////////////////
static tBool _DirFromJVM(tStringCMap* props, const achar* aProperty) {
#ifdef niJNI
  cString v;
  if (niJVM_HasVM() && niJVM_GetProperty(niJVM_GetCurrentEnv(),v,aProperty)) {
    cPath path;
    path.SetDirectory(v.Chars());
    (*props)[aProperty] = path.GetDirectory();
    return eTrue;
  }
#endif
  return eFalse;
}

///////////////////////////////////////////////
static tBool _CopyFromJVM(tStringCMap* props, const achar* aProperty) {
#ifdef niJNI
  cString v;
  if (niJVM_HasVM() && niJVM_GetProperty(niJVM_GetCurrentEnv(),v,aProperty)) {
    (*props)[aProperty] = v;
    return eTrue;
  }
#endif
  return eFalse;
}

///////////////////////////////////////////////
#ifdef niJNI
static tBool _PathFromJVM(tStringCMap* props, const achar* aProperty) {
  cString v;
  if (niJVM_HasVM() && niJVM_GetProperty(niJVM_GetCurrentEnv(),v,aProperty)) {
    cPath path = v.Chars();
    (*props)[aProperty] = path.GetPath();
    return eTrue;
  }
  return eFalse;
}
#endif

///////////////////////////////////////////////
#ifdef niJNI
static cString _FindFile(const achar* aaszBinDir, const achar* aaszFileName) {
  cString v;
  // absolute path ?
  {
    v = aaszFileName;
    if (ni::FileExists(v.Chars())) {
      return ni::GetRootFS()->GetAbsolutePath(v.Chars());
    }
  }
  cString fileName = cPath(aaszFileName).GetFile();
  // in BIN/.. directory
  {
    v.clear();
    v << aaszBinDir << "../" << fileName;
    if (ni::FileExists(v.Chars())) {
      return ni::GetRootFS()->GetAbsolutePath(v.Chars());
    }
  }
  // in BIN/ directory
  {
    v.clear();
    v << aaszBinDir << fileName;
    if (ni::FileExists(v.Chars())) {
      return ni::GetRootFS()->GetAbsolutePath(v.Chars());
    }
  }
  // in BIN/../../ directory
  {
    v.clear();
    v << aaszBinDir << "../../" << fileName;
    if (ni::FileExists(v.Chars())) {
      return ni::GetRootFS()->GetAbsolutePath(v.Chars());
    }
  }
  // in BIN/../../jars/ directory
  {
    v.clear();
    v << aaszBinDir << "../../jars/" << fileName;
    if (ni::FileExists(v.Chars())) {
      return ni::GetRootFS()->GetAbsolutePath(v.Chars());
    }
  }
  return AZEROSTR;
}
#endif

///////////////////////////////////////////////
#ifdef niJNI
static cString _FindPackage(const achar* aaszBinDir, const achar* aPackage) {
  astl::vector<cString> pkgs;
  cString v;
  if (niJVM_GetProperty(niJVM_GetCurrentEnv(),v,"java.class.path")) {
    if (v.icontains(aPackage)) {
#  ifdef niWindows
      StringSplit(v,";",&pkgs);
#  else
      StringSplit(v,":",&pkgs);
#  endif
      if (pkgs.empty())
        pkgs.push_back(v);
      niLoopr(ri,pkgs.size()) {
        if (pkgs[ri].icontains(aPackage)) {
          return _FindFile(aaszBinDir,pkgs[ri].Chars());
        }
      }
    }
    else {
      return _FindFile(aaszBinDir,aPackage);
    }
  }
  return AZEROSTR;
}
#endif

///////////////////////////////////////////////
static tBool _InitPkgProp(tStringCMap* props, const achar* binDir, const achar* baseName) {
#ifdef niJNI
  if (niJVM_HasVM()) {
    cString v;
    const cString propPkgName = _ASTR("ni.packages.") + baseName;
    if (_PathFromJVM(props,propPkgName.Chars())) {
      return eTrue;
    }
    else {
      const cString pkgName = _ASTR("ni-") + baseName + ".jar";
      const cString pkg = _FindPackage(binDir,pkgName.Chars());
      if (pkg.IsNotEmpty()) {
        (*props)[propPkgName] = pkg;
        return eTrue;
      }
    }
  }
#endif
  return eFalse;
}

///////////////////////////////////////////////
#if !defined JVM_ONLY
static cString _FindDir(const achar* aaszBinDir, const achar* aaszName) {
  cString v; cPath path;

#if defined niOSX
  {
    if (StrEndsWithI(aaszBinDir,"macos/")) {
      const cString resDirName = _AppleGetInfoPlistPropertyValue("niAppResourceDirName");
      niPanicAssertMsg(!resDirName.IsEmpty(),
                       "The property 'niAppResourceDirName' isnt set in the macOS app's Info.plist.");
      v.clear();
      v << aaszBinDir << "../Resources/" << resDirName << "/" << aaszName;
      // GetLang()->MessageBox(NULL, "BLA V", v.Chars(), eOSMessageBoxFlags_Ok);
      if (ni::DirExists(v.Chars())) {
        // GetLang()->MessageBox(NULL, "BLA U", "HAS DATA", eOSMessageBoxFlags_Ok);
        path.SetDirectory(ni::GetRootFS()->GetAbsolutePath(v.Chars()).Chars());
        return path.GetDirectory();
      }
    }
  }
#endif

  // in BIN/../../ directory
  {
    v.clear();
    v << aaszBinDir << "../../" << aaszName;
    if (ni::DirExists(v.Chars())) {
      path.SetDirectory(ni::GetRootFS()->GetAbsolutePath(v.Chars()).Chars());
      return path.GetDirectory();
    }
  }

  // in BIN/ directory
  {
    v.clear();
    v << aaszBinDir << aaszName;
    if (ni::DirExists(v.Chars())) {
      path.SetDirectory(ni::GetRootFS()->GetAbsolutePath(v.Chars()).Chars());
      return path.GetDirectory();
    }
  }

  // in BIN/.. directory
  {
    v.clear();
    v << aaszBinDir << "../" << aaszName;
    if (ni::DirExists(v.Chars())) {
      path.SetDirectory(ni::GetRootFS()->GetAbsolutePath(v.Chars()).Chars());
      return path.GetDirectory();
    }
  }

  return AZEROSTR;
}
#endif

///////////////////////////////////////////////
static tBool _InitDirProp(tStringCMap* props, const achar* binDir,
                          const achar* baseName,
                          const tBool abFindIfNotProp) {
  const cString propDirName = _ASTR("ni.dirs.") + baseName;
  if (props->find(propDirName) != props->end())
    return eTrue;

#ifdef niJNI
  if (_DirFromJVM(props,propDirName.Chars())) {
    return eTrue;
  }
#endif
#if !defined JVM_ONLY
  if (abFindIfNotProp) {
    const cString dir = _FindDir(binDir,baseName);
    if (dir.IsNotEmpty()) {
      (*props)[propDirName] = dir;
      return eTrue;
    }
  }
#endif

  return eFalse;
}

///////////////////////////////////////////////
static tBool _ParseCmdLineProperties(tStringCMap* props, const achar* aaszCmdLine) {
  if (!niStringIsOK(aaszCmdLine))
    return eTrue;
  // niDebugFmt(("D/parseCommandLine: %s", aaszCmdLine));

  cString strCmdLine = aaszCmdLine;
  tI32 parametersPos = CmdLineGetParametersPos(strCmdLine);
  // niDebugFmt(("D/parametersPos: %d", parametersPos));
  if (parametersPos <= 0)
    return eTrue;

  StrCharIt it = strCmdLine.charZIt(parametersPos);
  // niDebugFmt(("D/parseCommandLine:params: %s", it.current()));
  // Read the VM arguments
  tU32 prevChar = 0;
  while (!it.is_end()) {
    const tU32 c = it.next();
    if (prevChar == '-') {
      switch (c) {
        case '-': // -- is a synonym for -D
        case 'D': {
          cString pname = CmdLineStrCharItReadFile(it,'=');
          cString pvalue = CmdLineStrCharItReadFile(it);
          (*props)[pname] = pvalue;
          break;
        }
      }
      prevChar = 0;
    }
    else if (StrIsSpace(c)) {
      continue;
    }
    else  {
      prevChar = c;
      if (prevChar != '-') {
        it.prior();
        break;
      }
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
void cLang::_InitDefaultSystemProperties(tStringCMap* props)
{
  //---- JVM -----------------------------------------------------
#ifdef niJNI
  if (niJVM_HasVM()) {
    _CopyFromJVM(props,"java.vm.version");
    _CopyFromJVM(props,"java.vm.vendor");
    _CopyFromJVM(props,"java.vm.name");
    _PathFromJVM(props,"ni.log.xml");
    _PathFromJVM(props,"ni.log.text");
    _CopyFromJVM(props,"ni.log.stderr");
  }
#endif

  //---- NI ------------------------------------------------------
  {
    (*props)["ni.features"] = cString().Set((tI32)niFeatures);
#ifdef niJNI
    _CopyFromJVM(props,"ni.build.default");
#endif
  }

  //---- App -----------------------------------------------------
  {
    achar exePathBuff[niStackBufferSize];
    cPath exePath(ni_get_exe_path(exePathBuff));
    exePath.SetExtension(AZEROSTR);
    (*props)["ni.app.name"] = exePath.GetFile();
    (*props)["ni.app.version"] = "v0.0.0";
    // Bin directory
    if (!_DirFromJVM(props, "ni.dirs.bin")) {
#if defined JVM_ONLY
      niAssertUnreachable("E/Can't initialize ni.dirs.bin");
#else
      (*props)["ni.dirs.bin"] = exePath.GetDirectory().Chars();
#endif
    }

    // Current working directory
    exePath.SetDirectory(agetcwd().c_str());
    (*props)["ni.app.cwd"] = exePath.GetDirectory().Chars();
  }

  //---- LOA -----------------------------------------------------
  {
    (*props)["ni.loa.linker"] = niLinker;
    (*props)["ni.loa.os"] = niOS;
#ifdef niJNI
#if !defined _BUILD_JNI
#error "C++ compiler DEFINE flags inconsistency: niJNI defined but _BUILD_JNI isnt defined."
#endif
    (*props)["ni.loa.arch"] = niCPUArch "j";
#else
    (*props)["ni.loa.arch"] = niCPUArch;
#endif
#if defined niDebug
    (*props)["ni.loa.build"] = "da";
#else
    (*props)["ni.loa.build"] = "ra";
#endif
  }

  //---- CPU -----------------------------------------------------
  {
    (*props)["ni.cpu.count"] = cString().Set(_GetNumProcessors());
  }

  //---- User Dirs -----------------------------------------------
  {
    if (!_DirFromJVM(props,"ni.dirs.home")) {
#if !defined JVM_ONLY
      achar tmp[AMAX_PATH];
      (*props)["ni.dirs.home"] = _GetHomeDir(tmp);
#else
      niAssertUnreachable("E/Can't initialize ni.dirs.home.");
#endif
    }

    if (!_DirFromJVM(props,"ni.dirs.documents")) {
      achar tmp[AMAX_PATH];
      if (_GetDocumentsDir(tmp)) {
        (*props)["ni.dirs.documents"] =  tmp;
      }
      else {
        if (props->Contains(_ASTR("ni.dirs.home"))) {
          const cString v = (*props)["ni.dirs.home"] + "Documents/";
          ni::GetRootFS()->FileMakeDir(v.Chars());
          if (ni::DirExists(v.Chars())) {
            (*props)["ni.dirs.documents"] = v.Chars();
          }
        }
      }
    }

    if (!_DirFromJVM(props,"ni.dirs.downloads")) {
      achar tmp[AMAX_PATH];
      if (_GetDownloadsDir(tmp)) {
        (*props)["ni.dirs.downloads"] =  tmp;
      }
      else {
        if (props->Contains(_ASTR("ni.dirs.home"))) {
          const cString v = (*props)["ni.dirs.home"] + "Downloads/";
          ni::GetRootFS()->FileMakeDir(v.Chars());
          if (ni::DirExists(v.Chars())) {
            (*props)["ni.dirs.downloads"] = v.Chars();
          }
        }
      }
    }

    if (!_DirFromJVM(props,"ni.dirs.temp")) {
      achar tmp[AMAX_PATH];
      if (_GetTempDir(tmp)) {
        (*props)["ni.dirs.temp"] = tmp;
      }
      else if (props->Contains(_ASTR("ni.dirs.home"))) {
        const cString v = (*props)["ni.dirs.home"] + "niApp/Temp/";
        ni::GetRootFS()->FileMakeDir(v.Chars());
        if (ni::DirExists(v.Chars())) {
          (*props)["ni.dirs.temp"] = v.Chars();
        }
      }
    }

    if (!_DirFromJVM(props,"ni.dirs.logs")) {
      // niDebugFmt(("... No logs dir from jvm"));
      if (props->Contains(_ASTR("ni.dirs.home"))) {
        const cString v = (*props)["ni.dirs.home"] + "niApp/Logs/";
        // niDebugFmt(("... Try to create home: %s", v));
        ni::GetRootFS()->FileMakeDir(v.Chars());
        if (ni::DirExists(v.Chars())) {
          (*props)["ni.dirs.logs"] = v.Chars();
        }
      }
    }

    if (!_DirFromJVM(props,"ni.dirs.config")) {
      if (props->Contains(_ASTR("ni.dirs.home"))) {
        const cString v = (*props)["ni.dirs.home"] + "niApp/Config/";
        ni::GetRootFS()->FileMakeDir(v.Chars());
        if (ni::DirExists(v.Chars())) {
          (*props)["ni.dirs.config"] = v.Chars();
        }
      }
    }
  }

  //---- Data Dirs / Packages ---------------------------------
  {
    const cString cwdDir = (*props)["ni.app.cwd"];
    const cString binDir = (*props)["ni.dirs.bin"];
    _CopyFromJVM(props,"ni.packages.jars");
    if (!_InitPkgProp(props, binDir.Chars(), "data")) {
      _InitDirProp(props, binDir.Chars(), "data", eTrue);
    }
    if (!_InitPkgProp(props, binDir.Chars(), "scripts")) {
      _InitDirProp(props, binDir.Chars(), "scripts", eTrue);
    }
  }

  //---- Command line -----------------------------------------
  _ParseCmdLineProperties(props,ni::GetCurrentOSProcessCmdLine());
}

//--------------------------------------------------------------------------------------------
//
// cLang
//
//--------------------------------------------------------------------------------------------
#define CHECK_SYSTEM_PROPERTIES()               \
  niAssert(mptrSystemProperties.IsOK());

const tStringCMap* __stdcall cLang::GetProperties() const {
  CHECK_SYSTEM_PROPERTIES();
  return mptrSystemProperties;
}
tBool __stdcall cLang::HasProperty(const achar* aaszName) const {
  CHECK_SYSTEM_PROPERTIES();
  return mptrSystemProperties->Contains(_ASTR(aaszName));
}
void __stdcall cLang::SetProperty(const achar* aaszName, const achar* aaszValue) {
  CHECK_SYSTEM_PROPERTIES();
  (*mptrSystemProperties)[aaszName] = aaszValue;
}
cString __stdcall cLang::GetProperty(const achar* aaszName) const {
  CHECK_SYSTEM_PROPERTIES();
  return (*mptrSystemProperties)[aaszName];
}
