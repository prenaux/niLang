// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"
#include "API/niLang/Utils/CollectionImpl.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/IProf.h"
#include "API/niLang/STL/run_once.h"

#ifdef niWinDesktop
#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#include "API/niLang/Platforms/Win32/Win32_UTF.h"
#include <shellapi.h>
#endif

using namespace ni;

#if defined niIOS || defined niOSX
niExportFunc(int) _OSXOpenURL(const char* aUrl);
#endif

#if defined niJSCC
#include <emscripten.h>
#include <emscripten/html5.h>
extern "C" void JSCC_InitModule_niLang();
#endif

static tBool _LogMessageCallback(tLogFlags type, const tF64 afTime, const char* file, const char* func, int line, const char* msg)
{
  Ptr<tMessageHandlerSinkLst> mt = ni::GetLang()->GetSystemMessageHandlers();
  if (mt.IsOK()) {
    static tF64 _fPrevTime = 0.0f;
    cString final;
    ni_log_format_message(final,type,file,line,func,msg,afTime,_fPrevTime);
    _fPrevTime = afTime;
    ni::SendMessages(mt,eSystemMessage_Log,type,final);
  }
  return eTrue;
}

///////////////////////////////////////////////
static sPropertyBool _logSystemInfo("log.system_info",eFalse);

niExportFunc(void) ni_log_system_info_once() {
  niRunOnce {
    cString str;
    {
      const iTime* currentTime = ni::GetLang()->GetCurrentTime();
      if (currentTime) {
        str << "Start Date: " << currentTime->Format(NULL) << AEOL;
      }
    }
    str << "LOA: " << niLOA_Libs << AEOL;
    str << "ASAN: " << EA_ASAN_ENABLED << AEOL;

#if niMinFeatures(15)
    {
      iOSProcessManager* pProcMan = ni::GetLang()->GetProcessManager();
      if (pProcMan) {
        str << "PID: " << (tI64)pProcMan->GetCurrentProcess()->GetPID() << AEOL;
        str << "Exe Path: " << pProcMan->GetCurrentProcess()->GetExePath() << AEOL;
        str << "Command Line: " << pProcMan->GetCurrentProcess()->GetCommandLine() << AEOL;
      }
    }
#endif

    niLog(Info, str);
    str.Clear();

    str << "Properties:\n";
    {
      const Ptr<tStringCMap> props = ni::GetLang()->GetProperties();
      niLoopit(tStringCMap::const_iterator,it,*props) {
        str << "- "  << it->first
            << " = " << it->second << AEOL;
      }
    }
    niLog(Info, str);
    str.Clear();

    str << "niLang Build: ";
#ifdef _DEBUG
    str << "Debug";
#else
    str << "Release";
#endif
    str << " (" __DATE__ ")" << AEOL;
    niLog(Info, str);
    str.Clear();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
cLang::cLang()
    : mmapGlobalInstance(tGlobalInstanceCMap::Create())
    , mmapCreateInstance(tCreateInstanceCMap::Create())
{
  mbMarkMissingLocalization = eFalse;
  ni_log_set_callback(_LogMessageCallback);
}

///////////////////////////////////////////////
void cLang::_Construct()
{
#if defined niJSCC
  JSCC_InitModule_niLang();
#endif

  _StartupModules();

  // Init the default system properties
  mptrSystemProperties = tStringCMap::Create();
  _InitDefaultSystemProperties(mptrSystemProperties);
  niAssert(mptrSystemProperties.IsOK());

  // Initialize the PATH, this is needed by the JVM to find dependend
  // libraries when loading JNI DLLs.
#ifdef niJNI
  {
    const achar* envPATHSep =
#ifdef niWindows
        ";"
#else
        ":"
#endif
        ;
    cString envPATH = ni::GetLang()->GetEnv("PATH");
    envPATH = ni::GetLang()->GetProperty("ni.dirs.bin") +
        envPATHSep + envPATH;
    niDebugFmt(("I/NEWPATH: %s", envPATH));
    ni::GetLang()->SetEnv("PATH",envPATH.Chars());
  }
#endif

  // initialize the frame time
  ResetFrameTime();

  _PlatformStartup();
  mptrSystemMessageHandlers = tMessageHandlerSinkLst::Create();

  // Initialize the log
  if (_logSystemInfo.get()) {
    ni_log_system_info_once();
  }
}

///////////////////////////////////////////////
void __stdcall cLang::Invalidate()
{
  if (mptrSystemMessageHandlers.IsOK()) {
    ni::SendMessages(mptrSystemMessageHandlers,eSystemMessage_Exit,niVarNull,niVarNull);
  }
  if (mptrSystemMessageHandlers.IsOK()) {
    mptrSystemMessageHandlers->Invalidate();
    mptrSystemMessageHandlers = NULL;
  }
}

///////////////////////////////////////////////
cLang::~cLang()
{
  Invalidate();
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cLang::IsOK() const
{
  niClassIsOK(cLang);
  return eTrue;
}

///////////////////////////////////////////////
#if niMinFeatures(20)
tBool __stdcall cLang::JsonParseFile(iFile* apFile, iJsonParserSink* apSink) {
  return ni::JsonParseFile(apFile,apSink);
}
tBool __stdcall cLang::JsonParseString(const cString& aString, iJsonParserSink* apSink) {
  return ni::JsonParseString(aString,apSink);
}
iJsonWriter* __stdcall cLang::CreateJsonSinkWriter(iJsonWriterSink* apSink, tBool abPrettyPrint) {
  return ni::CreateJsonSinkWriter(apSink,abPrettyPrint);
}
iJsonWriter* __stdcall cLang::CreateJsonFileWriter(iFile* apFile, tBool abPrettyPrint) {
  return ni::CreateJsonFileWriter(apFile,abPrettyPrint);
}
#endif

///////////////////////////////////////////////
#if niMinFeatures(20)
ni::tBool __stdcall cLang::XmlParseFile(ni::iFile* apFile, ni::iXmlParserSink* apSink) {
  return ni::XmlParseFile(apFile, apSink);
}
ni::tBool __stdcall cLang::XmlParseString(const ni::cString& aString, ni::iXmlParserSink* apSink) {
  return ni::XmlParseString(aString, apSink);
}
#endif

///////////////////////////////////////////////
tBool __stdcall cLang::OnExit(iRunnable* apRunnable) {
  if (niIsOK(apRunnable)) {
    mvOnExit.push_back(apRunnable);
    return eTrue;
  }
  return eFalse;
}
void __stdcall cLang::Exit(tU32 anErrorCode) {
  niLoopr(ri,mvOnExit.size()) {
    Ptr<iRunnable> runOnExit = mvOnExit[ri];
    if (runOnExit.IsOK()) {
      runOnExit->Run();
    }
  }
  mvOnExit.clear();

  // Invalidate the concurrent queue to guarantee that it doesnt deadlock when
  // cleaned up by the C++ global variable destructor.
  ni::GetConcurrent()->Invalidate();

  _PlatformExit(anErrorCode);
}

///////////////////////////////////////////////
iProf* __stdcall cLang::GetProf() const {
  return ni::GetProf();
}

///////////////////////////////////////////////
const iTime* cLang::GetCurrentTime() const
{
  return ni::GetCurrentTime();
}

///////////////////////////////////////////////
tI32 __stdcall cLang::RunCommand(const achar* aaszCmd)
{
#if defined niWinRT || defined niAndroid || defined niIOS
  // Can't run command on WinRT
  return -1;
#elif defined niWinDesktop
  ni::Windows::UTF16Buffer wCmd;
  niWin32_UTF8ToUTF16(wCmd,aaszCmd);
  return _wsystem(wCmd.begin());
#else
  return system(aaszCmd);
#endif
}

///////////////////////////////////////////////
tI32 __stdcall cLang::StartPath(const achar* aaszURL)
{
  ni::cString strURL = aaszURL;
  // Trim leading and trailing spaces, some platforms (OSX) don't handle those
  // correctly.
  strURL.Trim();

  // map://LAT,LNG
  // map://Address
  // map://1.365778,103.863593
  // map://Singapore, 61 farleigh avenue
  if (strURL.StartsWithI("map://")) {
    cString strLatLng;
    StringEncodeUrl(strLatLng,strURL.AfterI("map://"));
#if defined niIOS
    strURL = _ASTR("maps://maps.apple.com/maps?q=") + strLatLng + "&amp;ll=";
#else
    strURL = _ASTR("http://maps.google.com?q=") + strLatLng + "&amp;ll=";
#endif
  }

  // niDebugFmt(("... StartPath: '%s'", strURL));

#if defined niJSCC
  const ni::tBool bIsURL = strURL.contains("://");
  if (bIsURL) {
    cString encodedUrl;
    StringEncodeUrl(encodedUrl, strURL);
    emscripten_run_script(niFmt("niApp.NotifyHost('OpenUrl %s');",encodedUrl));
  }
#elif defined niWinDesktop
  {
    const ni::tBool bIsURL =
        strURL.StartsWithI("http://") ||
        strURL.StartsWithI("https://") ||
        strURL.StartsWithI("ftps://") ||
        strURL.StartsWithI("ftp://");
    if (!bIsURL) {
      strURL = GetRootFS()->GetAbsolutePath(strURL.Chars());
      achar* strURLWriteBuffer = strURL.data();
      niLoop(i,strURL.size()) {
        if (strURLWriteBuffer[i] == '/')
          strURLWriteBuffer[i] = '\\';
      }
    }
    cPath path(strURL.Chars());
    tIntPtr ret = (tIntPtr)ShellExecute(NULL,"open",path.GetPath().Chars(),NULL,
                                        bIsURL?NULL:path.GetDirectory().Chars(),SW_SHOWNORMAL);
    switch (ret) {
      case 0: niError(niFmt("Start[%s]\nThe operating system is out of memory or resources.",path.GetPath().Chars())); return -1;
      case ERROR_FILE_NOT_FOUND: niError(niFmt("Start[%s]\nThe specified file was not found.",path.GetPath().Chars())); return -1;
      case ERROR_PATH_NOT_FOUND: niError(niFmt("Start[%s]\nThe specified path was not found.",path.GetPath().Chars())); return -1;
      case ERROR_BAD_FORMAT: niError(niFmt("Start[%s]\nThe .exe file is invalid (non-Microsoft Win32 .exe or error in .exe image).",path.GetPath().Chars())); return -1;
      case SE_ERR_ACCESSDENIED: niError(niFmt("Start[%s]\nThe operating system denied access to the specified file.",path.GetPath().Chars())); return -1;
      case SE_ERR_ASSOCINCOMPLETE: niError(niFmt("Start[%s]\nThe file name association is incomplete or invalid.",path.GetPath().Chars())); return -1;
      case SE_ERR_DDEBUSY: niError(niFmt("Start[%s]\nThe Dynamic Data Exchange (DDE) transaction could not be completed because other DDE transactions were being processed.",path.GetPath().Chars())); return -1;
      case SE_ERR_DDEFAIL: niError(niFmt("Start[%s]\nThe DDE transaction failed.",path.GetPath().Chars())); return -1;
      case SE_ERR_DDETIMEOUT: niError(niFmt("Start[%s]\nThe DDE transaction could not be completed because the request timed out.",path.GetPath().Chars())); return -1;
      case SE_ERR_DLLNOTFOUND: niError(niFmt("Start[%s]\nThe specified dynamic-link library (DLL) was not found.",path.GetPath().Chars())); return -1;
      case SE_ERR_NOASSOC: niError(niFmt("Start[%s]\nThere is no application associated with the given file name extension. This error will also be returned if you attempt to print a file that is not printable.",path.GetPath().Chars())); return -1;
      case SE_ERR_OOM: niError(niFmt("Start[%s]\nThere was not enough memory to complete the operation.",path.GetPath().Chars())); return -1;
      case SE_ERR_SHARE: niError(niFmt("Start[%s]\nA sharing violation occurred.",path.GetPath().Chars())); return -1;
    }
  }
#elif defined niIOS //|| defined niOSX
  {
    _OSXOpenURL(strURL.Chars());
  }
#else
  {
#ifdef niOSX
    cString runCmd("open ");
#else
    cString runCmd("start ");
#endif
    runCmd << "\"" << strURL << _A("\"");
    return system(runCmd.Chars());
  }
#endif
  return 0;
}

///////////////////////////////////////////////
tMessageHandlerSinkLst* __stdcall cLang::GetSystemMessageHandlers() const
{
  return mptrSystemMessageHandlers;
}

///////////////////////////////////////////////
iDataTableWriteStack* __stdcall cLang::CreateDataTableWriteStack(iDataTable* apDT)
{
  return niNew cDataTableWriteStack(apDT);
}

///////////////////////////////////////////////
iDataTableWriteStack* __stdcall cLang::CreateDataTableWriteStackFromName(const achar* aaszName)
{
  return niNew cDataTableWriteStack(aaszName);
}

///////////////////////////////////////////////
iDataTableReadStack* __stdcall cLang::CreateDataTableReadStack(iDataTable* apDT)
{
  return niNew cDataTableReadStack(apDT);
}

///////////////////////////////////////////////
tBool __stdcall cLang::LoadLocalization(iDataTable* apDT) {
  niCheckIsOK(apDT,eFalse);
  tHStringPtr hspNativeProperty = _H("native");
  niLoop(i,apDT->GetNumChildren()) {
    iDataTable* c = apDT->GetChildFromIndex(i);
    if (!StrEq(c->GetName(),"String"))
      continue;
    tHStringPtr hspNative = _H(c->GetString("native"));
    if (HStringIsEmpty(hspNative))
      continue;
    niLoop(j,c->GetNumProperties()) {
      tHStringPtr hspLocale = _H(c->GetPropertyName(j));
      if (hspLocale == hspNativeProperty)
        continue;
      tHStringPtr hspValue = _H(c->GetStringFromIndex(j));
      if (HStringIsEmpty(hspValue))
        continue;
      ni::GetLang()->SetLocalization(hspLocale,hspNative,hspValue);
      // niDebugFmt(("LOCALE[%s]: %s = %s",hspLocale,hspNative,hspValue));
    }
  }
  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
// ni::GetLang
//
//--------------------------------------------------------------------------------------------
#include "API/niLang/Utils/CrashReport.h"

static cLang* _pLangInstance = NULL;

namespace ni {

cLang* GetLangImpl() {
  if (!_pLangInstance) {
    static tU8 _pLangInstance_Impl_Mem[sizeof(cLang)];
    _pLangInstance = new(_pLangInstance_Impl_Mem) cLang();
    _pLangInstance->_Construct();
  }
  return _pLangInstance;
}

niExportFunc(ni::iLang*) GetLang() {
  return GetLangImpl();
}

niExportFunc(ni::iUnknown*) New_niLang_Lang(const ni::Var&,const ni::Var&) {
  return GetLangImpl();
}

}
