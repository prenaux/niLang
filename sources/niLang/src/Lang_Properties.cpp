#include "Lang.h"

#if defined niWin32
  #ifdef niWindows
    #include <windows.h>
  #endif
  #include <process.h>
#elif defined niLinux || defined niQNX
  #include <unistd.h>
#elif defined niOSX || defined niIOS
  #include <sys/param.h>
  #include <sys/sysctl.h>
#elif defined niJSCC
  #include <limits.h>
  #include <unistd.h>
#else
  #error "Unknown platform."
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

static constexpr const char* _kniAppDirName = ".niApp";

///////////////////////////////////////////////
static ni::tI32 _GetNumProcessors()
{
  unsigned int numProcessors = 1;
#if defined niWinDesktop
  {
    SYSTEM_INFO SysInfo;
    ZeroMemory(&SysInfo, sizeof(SYSTEM_INFO));
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
    mib[1] = HW_AVAILCPU; // alternatively, try HW_NCPU;
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
  #error "Unknown platform."
#endif
  return numProcessors;
}

///////////////////////////////////////////////
niExportFunc(achar*) FixSystemDir(achar* aaszOutput, const achar* aaszBuffer,
                                  const achar* aaszToCreate, tBool abCreate)
{
  achar tmp[AMAX_PATH] = AZEROSTR;
  if (!aaszBuffer || !aaszOutput)
    return NULL;
  StrCat(tmp, aaszBuffer);
  StrPutPathSep(tmp);
  if (niIsStringOK(aaszToCreate)) {
    StrCat(tmp, aaszToCreate);
    StrPutPathSep(tmp);
    if (abCreate) {
      if (!ni::GetRootFS()->FileMakeDir(tmp)) {
        return NULL;
      }
    }
  }
  StrFixPath(aaszOutput, tmp, AMAX_SIZE);
  return aaszOutput;
}

///////////////////////////////////////////////
static achar* _GetHomeDir(achar* aaszOutput)
{
#if defined niIOS || defined niOSX
  achar buffer[AMAX_PATH] = { 0 };
  _AppleGetDirHome(buffer);
  return FixSystemDir(aaszOutput, buffer, NULL, eFalse);
#elif defined niWinDesktop
  achar buffer[AMAX_PATH] = { 0 };
  BOOL res = ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PROFILE, NULL,
                                               SHGFP_TYPE_DEFAULT, buffer);
  if (!res) {
    return NULL;
  }
  return FixSystemDir(aaszOutput, buffer, NULL, eFalse);
#else
  cString strHome = agetenv(_A("HOME"));
  if (strHome.empty())
    return NULL;
  return FixSystemDir(aaszOutput, strHome.Chars(), NULL, eFalse);
#endif
}

///////////////////////////////////////////////
static achar* _GetDownloadsDir(achar* aaszOutput)
{
#if defined niIOS || defined niOSX
  achar buffer[AMAX_PATH] = { 0 };
  _AppleGetDirDownloads(buffer);
  return FixSystemDir(aaszOutput, buffer, NULL, eFalse);
#elif defined niWinDesktop
  achar buffer[AMAX_PATH] = { 0 };
  BOOL res = ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PROFILE, NULL,
                                               SHGFP_TYPE_DEFAULT, buffer);
  if (!res) {
    return NULL;
  }
  return FixSystemDir(aaszOutput, buffer, "Downloads", eTrue);
#else
  return NULL;
#endif
}

///////////////////////////////////////////////
static achar* _GetTempDir(achar* aaszOutput)
{
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
static achar* _GetDocumentsDir(achar* aaszOutput)
{
#if defined niIOS || defined niOSX
  achar buffer[AMAX_PATH] = { 0 };
  _AppleGetDirDocs(buffer);
  return FixSystemDir(aaszOutput, buffer, NULL, eFalse);
#elif defined niWinDesktop
  achar buffer[AMAX_PATH] = { 0 };
  // CSIDL_PROFILE: %USERPROFILE%
  // CSIDL_PERSONAL: %MYDOCUMENTS%
  BOOL res = ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL,
                                               SHGFP_TYPE_DEFAULT, buffer);
  if (!res) {
    return NULL;
  }
  return FixSystemDir(aaszOutput, buffer, NULL, eFalse);
#else
  return NULL;
#endif
}

///////////////////////////////////////////////
static cString _FindWorkDir(const achar* aaszBinDir)
{
  cString v;

#if defined niOSX || defined niIOS
  #if defined niOSX
  if (StrEndsWithI(aaszBinDir, "macos/"))
  #endif
  {
    v = aaszBinDir;
  #ifdef niOSX
    v << "../Resources/";
  #else
    v << "/";
  #endif
    // GetLang()->MessageBox(NULL, "BLA V", v.Chars(), eOSMessageBoxFlags_Ok);
    if (ni::DirExists(v.Chars())) {
      return ni::GetRootFS()->GetAbsolutePath(v.Chars());
    }
    else {
      niPanicUnreachable(niFmt("Cant find macOS Resources directory '%s'.", v));
      return AZEROSTR;
    }
  }
#endif

  // BINDIR/../../
  {
    v = aaszBinDir;
    v << "../../niLang/data/";
    if (ni::DirExists(v.Chars())) {
      v = aaszBinDir;
      v << "../../";
      return ni::GetRootFS()->GetAbsolutePath(v.Chars());
    }
  }

  // By default we return the bin folder
  return aaszBinDir;
}

///////////////////////////////////////////////
static tBool _ParseCmdLineProperties(tStringCMap* props,
                                     const achar* aaszCmdLine)
{
  if (!niStringIsOK(aaszCmdLine))
    return eTrue;
  // niDebugFmt(("parseCommandLine: %s", aaszCmdLine));

  cString strCmdLine = aaszCmdLine;
  tI32 parametersPos = CmdLineGetParametersPos(strCmdLine);
  // niDebugFmt(("parametersPos: %d", parametersPos));
  if (parametersPos <= 0)
    return eTrue;

  StrCharIt it = strCmdLine.charZIt(parametersPos);
  // niDebugFmt(("parseCommandLine:params: %s", it.current()));
  // Read the VM arguments
  tU32 prevChar = 0;
  while (!it.is_end()) {
    const tU32 c = it.next();
    if (prevChar == '-') {
      switch (c) {
      case '-': // -- is a synonym for -D
      case 'D': {
        cString pname = CmdLineStrCharItReadFile(it, '=');
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
    else {
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
  //---- NI ------------------------------------------------------
  {
    (*props)["ni.features"] = cString().Set((tI32)niFeatures);
  }

  //---- App -----------------------------------------------------
  {
    achar exePathBuff[AMAX_PATH];
    cPath exePath(ni_get_exe_path(exePathBuff));
    (*props)["ni.app.name"] = exePath.GetFileNoExt();
    (*props)["ni.app.version"] = "v0.0.0";
    (*props)["ni.dirs.bin"] = exePath.GetDirectory();
    (*props)["ni.dirs.work"] = _FindWorkDir((*props)["ni.dirs.bin"].c_str());
  }

  //---- LOA -----------------------------------------------------
  {
    (*props)["ni.loa.linker"] = niLinker;
    (*props)["ni.loa.os"] = niOS;
#ifdef niJNI
  #if !defined _BUILD_JNI
    #error \
      "C++ compiler DEFINE flags inconsistency: niJNI defined but _BUILD_JNI isnt defined."
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
    achar tmp[AMAX_PATH];

    (*props)["ni.dirs.home"] = _GetHomeDir(tmp);

    {
      const cString v = (*props)["ni.dirs.home"] + _kniAppDirName + "/";
      ni::GetRootFS()->FileMakeDir(v.Chars());
      if (ni::DirExists(v.Chars())) {
        (*props)["ni.dirs.niApp"] = v.Chars();
      }
    }

    if (_GetDocumentsDir(tmp)) {
      (*props)["ni.dirs.documents"] = tmp;
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

    if (_GetDownloadsDir(tmp)) {
      (*props)["ni.dirs.downloads"] = tmp;
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

    if (_GetTempDir(tmp)) {
      (*props)["ni.dirs.temp"] = tmp;
    }
    else if (props->Contains(_ASTR("ni.dirs.home"))) {
      const cString v = (*props)["ni.dirs.home"] + _kniAppDirName + "/Temp/";
      ni::GetRootFS()->FileMakeDir(v.Chars());
      if (ni::DirExists(v.Chars())) {
        (*props)["ni.dirs.temp"] = v.Chars();
      }
    }

    if (props->Contains(_ASTR("ni.dirs.home"))) {
      const cString v = (*props)["ni.dirs.home"] + _kniAppDirName + "/Logs/";
      ni::GetRootFS()->FileMakeDir(v.Chars());
      if (ni::DirExists(v.Chars())) {
        (*props)["ni.dirs.logs"] = v.Chars();
      }
    }

    if (props->Contains(_ASTR("ni.dirs.home"))) {
      const cString v = (*props)["ni.dirs.home"] + _kniAppDirName + "/Config/";
      ni::GetRootFS()->FileMakeDir(v.Chars());
      if (ni::DirExists(v.Chars())) {
        (*props)["ni.dirs.config"] = v.Chars();
      }
    }
  }

  //---- Command line -----------------------------------------
  _ParseCmdLineProperties(props, ni::GetCurrentOSProcessCmdLine());
}

//----------------------------------------------------------------------------
//
// Section: cLang
//
//----------------------------------------------------------------------------
#define CHECK_SYSTEM_PROPERTIES() niAssert(mptrSystemProperties.IsOK());

const tStringCMap* __stdcall cLang::GetProperties() const
{
  CHECK_SYSTEM_PROPERTIES();
  return mptrSystemProperties;
}
tBool __stdcall cLang::HasProperty(const achar* aaszName) const
{
  CHECK_SYSTEM_PROPERTIES();
  return mptrSystemProperties->Contains(_ASTR(aaszName));
}
void __stdcall cLang::SetProperty(const achar* aaszName, const achar* aaszValue)
{
  CHECK_SYSTEM_PROPERTIES();
  (*mptrSystemProperties)[aaszName] = aaszValue;
}
cString __stdcall cLang::GetProperty(const achar* aaszName) const
{
  CHECK_SYSTEM_PROPERTIES();
  return (*mptrSystemProperties)[aaszName];
}
