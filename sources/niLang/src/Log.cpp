// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/IOSProcess.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/Utils/ThreadImpl.h"
#include "API/niLang/Utils/Trace.h"
#include "API/niLang.h"
#include "API/niLang/Utils/CrashReport.h"

#define LOG_LAST_LOGS

// Log for Windows
#if defined niWinDesktop
#  include "API/niLang/Platforms/Win32/Win32_UTF.h"
#  define LOG_OUTPUT_DEBUG_STRING
#  if defined niWinDesktop
#    define LOG_STDIO
#    define LOG_FILE
#  endif
// Log for Android
#elif defined niAndroid
#  include <android/log.h>
#  define LOG_ANDROID_LOG_WRITE
// Log for iOS & macOS
#elif defined niIOS || defined niOSX
#  define LOG_APPLE_OS_LOG
#  if defined niOSX
#    define LOG_STDIO
#    define LOG_FILE
#  endif
// Log for Linux & QNX
#elif defined niLinux || defined niQNX
#  define LOG_STDIO
#  define LOG_FILE
// Log for JSCC
#elif defined niJSCC
#  define LOG_STDIO
#else
#  error "Unknown Log method for this platform."
#endif

#ifdef LOG_APPLE_OS_LOG
#  include <os/log.h>
// We don't enable the explicit info & debug os_log levels because they are
// not shown by default in Console.app
// #  define LOG_APPLE_OS_LOG_WITH_INFO_DEBUG
#endif

#ifdef LOG_FILE
#  include "FileFd.h"
#endif

#if defined niWinDesktop || defined niOSX || defined niLinux
#define HAS_CONSOLE_COLORS
#endif

#ifdef HAS_CONSOLE_COLORS
#include <niLang/STL/run_once.h>

using namespace ni;

#if defined niWindows

// Tells us whether we have been launched from the explorer (or spawned
// independantly by another process) or if we inherited a preexisting console.
static tBool _IsSeparateWindowsConsole() {
  DWORD procId;
  DWORD count = GetConsoleProcessList(&procId, 1);
  ni::GetLang()->SetProperty("ni.log.console_process_count",niFmt("%d",(tU32)count));
  return (count <= 1);
}

static void _SetWindowsConsoleColors(HANDLE aHandle, const ni::tU32 logType) {
  static WORD _previousConsoleBgColor = 0;
  static WORD _previousConsoleFgColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  niRunOnce {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(aHandle, &csbi);
    _previousConsoleBgColor = csbi.wAttributes & 0xF0;
    _previousConsoleFgColor = csbi.wAttributes & 0x0F;
  };

  int back = _previousConsoleBgColor, fore = _previousConsoleFgColor;
  if (niFlagIs(logType, ni::eLogFlags_Error)) {
    // back = BACKGROUND_RED;
    // fore = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    fore = FOREGROUND_RED | FOREGROUND_INTENSITY;
  }
  else if (niFlagIs(logType, ni::eLogFlags_Warning)) {
    fore = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  }
  else if (niFlagIs(logType, ni::eLogFlags_Debug)) {
    fore = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  }
  else if (niFlagIs(logType, ni::eLogFlags_Info)) {
    fore = FOREGROUND_BLUE | FOREGROUND_GREEN;
  }
  SetConsoleTextAttribute(aHandle, back | fore);
}
#endif

static void _AppendPosixConsoleColors(ni::cString& final, const ni::tU32 logType, ni::tBool abOverwriteNewLine) {
  ni::tBool appendNewLine = ni::eFalse;
  if (abOverwriteNewLine) {
    if (final.back() == '\n') {
      final.resize(final.size()-1);
      appendNewLine = ni::eTrue;
    }
  }
  if (niFlagIs(logType, ni::eLogFlags_Error)) {
    // red
    final << "\033[31m";
  }
  else if (niFlagIs(logType, ni::eLogFlags_Warning)) {
    // yellow
    final << "\033[33m";
  }
  else if (niFlagIs(logType, ni::eLogFlags_Debug)) {
    // green
    final << "\033[32m";
  }
  else if (niFlagIs(logType, ni::eLogFlags_Info)) {
    // cyan
    final << "\033[36m";
  }
  else {
    // default
    final << "\033[0m";
  }
  if (appendNewLine) {
    final.appendChar('\n');
  }
}

enum class eSetConsoleColorMode : tU32 {
  BEFORE_LINE,
  END_OF_LINE,
  AFTER_FLUSH
};

static void _SetConsoleColors(ni::cString& final, tBool aIsStdout, const ni::tU32 logType, eSetConsoleColorMode aMode) {
  static ni::tBool _isTTY = ni::eFalse;
  static ni::tBool _coloredOutput = ni::eFalse;
  niRunOnce {
    const tBool envNoColor = agetenv("NO_COLOR").Bool(eFalse);
    ni::GetLang()->SetProperty("ni.log.env_no_color",envNoColor?"yes":"no");
    if (!envNoColor) {
      if (isatty(fileno(stdout))) {
        _isTTY = eTrue;
      }
      else {
        const tBool envInsideEmacs = agetenv("INSIDE_EMACS").IsNotEmpty();
        ni::GetLang()->SetProperty("ni.log.env_inside_emacs",envInsideEmacs?"yes":"no");
        _coloredOutput = ni::eTrue;
      }
      ni::GetLang()->SetProperty("ni.log.is_tty",_isTTY?"yes":"no");
    }

    _coloredOutput = ni::GetLang()->GetProperty("ni.log.colored_output").Bool(
      !envNoColor && (_isTTY || _coloredOutput));
    ni::GetLang()->SetProperty("ni.log.colored_output",_coloredOutput?"yes":"no");

#ifdef niWindows
    if (_isTTY && _IsSeparateWindowsConsole())
    {
      HWND hwnd = GetConsoleWindow();

      RECT workArea;
      SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

#if 1
      // Setup the console window size in function of the work area size
      const auto width = (workArea.right - workArea.left) / 4 * 3;
      const auto height = (workArea.bottom - workArea.top) / 4 * 3;
#else
      // Keep the console window size
      RECT consoleRect;
      GetWindowRect(hwnd, &consoleRect);
      const auto width = consoleRect.right - consoleRect.left;
      const auto height = consoleRect.bottom - consoleRect.top;
#endif

      SetWindowPos(hwnd, NULL,
                   workArea.right - width,  // X pos
                   workArea.bottom - height,  // Y pos
                   width, height,
                   //SWP_NOSIZE |
                   SWP_NOZORDER);
    }
#endif
  }

  if (_coloredOutput) {
#ifdef niWindows
    if (_isTTY) {
      if (aMode == eSetConsoleColorMode::BEFORE_LINE ||
          aMode == eSetConsoleColorMode::AFTER_FLUSH)
      {
        _SetWindowsConsoleColors(GetStdHandle(aIsStdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), logType);
      }
      return;
    }
#endif

    if (aMode == eSetConsoleColorMode::BEFORE_LINE) {
      _AppendPosixConsoleColors(final,logType,eFalse);
    }
    else if (aMode == eSetConsoleColorMode::END_OF_LINE) {
      _AppendPosixConsoleColors(final,logType,eTrue);
    }
  }
}
#endif

//============================================================================
//
// Core Log
//
//============================================================================
namespace ni {

niExportFuncCPP(const char*) GetTraceFile(const char* aFile) {
  {
    tI32 index = StrZRFindChar(aFile,StrLen(aFile),'/');
    if (index >= 0)
      return aFile + index + 1;
  }
  {
    tI32 index = StrZRFindChar(aFile,StrLen(aFile),'\\');
    if (index >= 0)
      return aFile + index + 1;
  }
  return aFile;
}

niExportFuncCPP(ni::cString) GetTraceFunc(const char* aPrettyFunction) {
  ni::cString f = aPrettyFunction;

  auto index = f.find("(");
  if (index == ni::cString::npos)
    return f;

  f.erase(index);
  index = f.rfind(" ");
  if (index == ni::cString::npos)
    return f;

  if (astl::at_default(f, index + 1, 0) == '*') {
    index += 1;
  }

  f.erase(0, index + 1);
  return f;
}

static __forceinline cString _ToLogTimeIndexString(const tF64 afTime) {
  return niFmt(_A("%.04f"),afTime);
}

static __forceinline cString _LogTime(const tF64 afTime, const tF64 afPrevTime) {
  cString final;
  if (afTime > 0) {
    final << _A("[");
    if (afPrevTime > 0) {
      final << _ToLogTimeIndexString(afTime-afPrevTime);
    }
    final << _A(":");
    final << _ToLogTimeIndexString(afTime);
    final << _A("]");
  }
  return final;
}

static __forceinline const char* _LogType(tU32 aType)
{
  switch (aType&eLogFlags_All) {
    case eLogFlags_Debug: return "D/";
    case eLogFlags_Info: return "I/";
    case eLogFlags_Warning: return "W/";
    case eLogFlags_Error: return "E/";
    default: return "?/";
  }
}

static __forceinline const char* _LogTypeMSVC(tU32 aType)
{
  switch (aType&eLogFlags_All) {
    case eLogFlags_Debug: return "debug";
    case eLogFlags_Info: return "information";
    case eLogFlags_Warning: return "warning";
    case eLogFlags_Error: return "error";
    default: return "";
  }
}

static __forceinline bool _LogIsStdout(tU32 aType)
{
  if (aType&eLogFlags_Stdout)
    return true;
#ifdef __JSCC__
  switch (aType&eLogFlags_All) {
    case eLogFlags_Error:
      return false;
    default:
      return true;
  }
#else
  return false;
#endif
}

//! Convert a 'log type' message to a formated string for print
niExportFunc(void) ni_log_format_message(
    cString& final,
    tU32 aType,
    const char* aaszFile,
    const ni::tU32 anLine,
    const char* aaszFunction,
    const char* aaszMsg,
    const tF64 afTime,
    const tF64 afPrevTime)
{
  if (niFlagIs(aType,eLogFlags_Raw)) {
    final << aaszMsg;
    return;
  }

  if (!niStringIsOK(aaszMsg))
    return;

  {
    // TYPE/MSG [time] [file:line]
    // or (if Java)
    // TYPE/MSG [time] (file:line)
    const char* typeMark = niFlagIs(aType,eLogFlags_NoLogTypePrefix) ? NULL : _LogType(aType);
    const char* p = aaszMsg;
    const char* b = p;
    tU32 curSize = 0;
    tU32 numLines = 0;
    while (1) {
      if (!*p || *p == '\n' || *p == '\r') {
        if (curSize) {
          if (typeMark) {
            final << typeMark;
          }
          final.appendEx(b,curSize);
          if (numLines++ == 0) {
            if (!niFlagIs(aType,eLogFlags_FormatMSVC) && niStringIsOK(aaszFile))
            {
              if (afTime >= 0) {
                final << " " << _LogTime(afTime,afPrevTime);
              }
              if (niStringIsOK(aaszFile)) {
                const tU32 slashChars[] = {'\\','/'};
                StrCharIt itFile(aaszFile);
                itFile.to_end();
                int slashPos = ni::Max(
                    0,StrCharItRFindFirstOf(
                        itFile,slashChars,niCountOf(slashChars),NULL)+1);
                final << " (" << (aaszFile+slashPos);
                if (anLine != ~0 && anLine != eInvalidHandle) {
                  final << ":" << anLine;
                }
                if (niStringIsOK(aaszFunction)) {
                  final << " in " << aaszFunction;
                }
                final << ")";
              }
            }
          }
          final << "\n";
        }
        if (!*p)
          break;
        ++p;
        b = p;
        curSize = 0;
      }
      else {
        ++p;
        ++curSize;
      }
    }
  }

  if (niFlagIs(aType,eLogFlags_FormatMSVC) && niStringIsOK(aaszFile)) {
    if (!final.empty() && final[final.size()-1] != '\n')
      final << "\n";
    const char* typeMarkMSVC = _LogTypeMSVC(aType);
    final << "        " << aaszFile << "(" << anLine << "): ";
    if (typeMarkMSVC)
      final << typeMarkMSVC << " ";
    final << "in " << (niStringIsOK(aaszFunction) ? aaszFunction : "unknown") << "\n";
  }
}

#define __sync_log() AutoThreadLock __lock(_GetLogMutex());
static ThreadMutex& _GetLogMutex() {
  static ThreadMutex _logMutex;
  return _logMutex;
}

static tU32 _logFilter = 0;

niExportFunc(tU32) ni_log_set_filter(tU32 exclude) {
  __sync_log();
  const tU32 prevFilter = _logFilter;
  _logFilter = exclude;
  return prevFilter;
}
niExportFunc(tU32) ni_log_get_filter() {
  __sync_log();
  return _logFilter;
}

static tpfnLogMessage _logCallback = NULL;
niExportFunc(void) ni_log_set_callback(tpfnLogMessage apfnCallback) {
  _logCallback = apfnCallback;
}

static sPropertyBool _logStdoutEnabled("log.stdout",niTrue);
static sPropertyBool _logStderrEnabled("log.stderr",niTrue);
static sPropertyBool _logFileEnabled("log.file",niFalse);
static sPropertyBool _logFileOverwrite("log.file.overwrite",niTrue);
static sPropertyString _logFilePath("log.file.path","");
static tBool _logFileInitialized = eFalse;
static Ptr<iFile> _logFile = NULL;

#ifdef LOG_LAST_LOGS
// This is needed since its possible that sLastLogs get destructed before
// other global variable destructors that call the logger.
static SyncCounter _lastLogDestructed;

struct sLastLogs {
private:
  const tU32 _maxStoredLogs = 1000;
  astl::deque<cString> _logs;

public:
  sLastLogs() {
  }
  ~sLastLogs() {
    _lastLogDestructed.Set(1);
  }

  void Add(const cString& aLog) {
    __sync_log();
    _logs.push_back(aLog);
    while (_logs.size() > _maxStoredLogs) {
      _logs.pop_front();
    }
  }

  tU32 Get(astl::vector<cString>* logs, tSize numLogs) {
    __sync_log();
    numLogs = ni::Min(numLogs,_logs.size());
    if (logs && (numLogs > 0)) {
      logs->reserve(logs->size() + numLogs);
      for (astl::deque<cString>::const_iterator it = _logs.end()-numLogs; it != _logs.end(); ++it) {
        logs->push_back(*it);
      }
    }
    return (tU32)numLogs;
  }
};
static sLastLogs* _GetLastLogs() {
  static sLastLogs _lastLogs;
  // this can happen in a termination handler
  if (_lastLogDestructed.Get())
    return nullptr;
  return &_lastLogs;
}
niExportFunc(tU32) ni_get_last_logs(astl::vector<cString>* logs, tSize numLogs) {
  sLastLogs* lastLogs = _GetLastLogs();
  if (!lastLogs)
    return 0;
  tU32 r = lastLogs->Get(logs,numLogs);
  return r;
}
#else
niExportFunc(tU32) ni_get_last_logs(astl::vector<cString>* logs, tSize numLogs) {
  return 0;
}
#endif

niExportFunc(void) ni_log(tLogFlags logType, const char* logMsg, const char* logFile, int logLine, const char* logFunc)
{
  if (logType & _logFilter)
    return;

  static niThreadLocal11 ni::tF64 kLogTimeStart = ni::TimerInSeconds();
  static niThreadLocal11 ni::tF64 _fPrevTime = 0.0f;
  const ni::tF64 logTime = ni::TimerInSeconds() - kLogTimeStart;

  if (!niFlagIs(logType,eLogFlags_NoCallbackOutput) && _logCallback) {
    static niThreadLocal11 SyncCounter _inCallback(0);
    tBool callbackRet = eTrue;
    _inCallback.Inc();
    if (_inCallback.Get() < 10) {
      // _logCallback must be thread safe
      callbackRet = _logCallback(logType,logTime,logFile,logFunc,logLine,logMsg);
    }
    else {
      ni_log(logType|eLogFlags_NoCallbackOutput,
             niFmt("Too many re-entrant log callback '%d'.",_inCallback.Get()),
             logFile, logLine, logFunc);
    }
    _inCallback.Dec();
    if (!callbackRet) {
      return;
    }
  }

  const tBool isStdoutLogtype = _LogIsStdout(logType);
  if (!niFlagIs(logType,eLogFlags_NoRegularOutput) &&
      ((_logStdoutEnabled.get() && isStdoutLogtype) ||
       (_logStderrEnabled.get() && !isStdoutLogtype)))
  {
    __sync_log();

#ifdef LOG_LAST_LOGS
    sLastLogs* lastLogs = _GetLastLogs();
    if (lastLogs) {
      cString final;
      ni_log_format_message(final,logType,logFile,logLine,logFunc,logMsg,logTime,_fPrevTime);
      if (!niFlagIs(logType,eLogFlags_NoNewLine) &&
          (final.empty() || final[final.size()-1] != '\n')) {
        final.appendChar('\n');
      }
      lastLogs->Add(final);
    }
#endif

#if defined LOG_ANDROID_LOG_WRITE
    {
      cString final;
      ni_log_format_message(final,logType,logFile,logLine,logFunc,logMsg,logTime,_fPrevTime);
      switch (logType) {
        case eLogFlags_Error:
          __android_log_write(ANDROID_LOG_ERROR,"ni",final.Chars());
          break;
        case eLogFlags_Warning:
          __android_log_write(ANDROID_LOG_WARN,"ni",final.Chars());
          break;
        case eLogFlags_Debug:
          __android_log_write(ANDROID_LOG_DEBUG,"ni",final.Chars());
          break;
        case eLogFlags_Info:
          __android_log_write(ANDROID_LOG_INFO,"ni",final.Chars());
          break;
        default:
          __android_log_write(ANDROID_LOG_VERBOSE,"ni",final.Chars());
          break;
      }
    }
#endif // LOG_ANDROID_LOG_WRITE

#if defined LOG_STDIO
    {
      cString final;

#ifdef HAS_CONSOLE_COLORS
      _SetConsoleColors(final,isStdoutLogtype,logType,eSetConsoleColorMode::BEFORE_LINE);
#endif // HAS_CONSOLE_COLORS
      ni_log_format_message(final,logType,logFile,logLine,logFunc,logMsg,logTime,_fPrevTime);
#ifdef HAS_CONSOLE_COLORS
      _SetConsoleColors(final,isStdoutLogtype,0,eSetConsoleColorMode::END_OF_LINE);
#endif

      iFile* fpOut = ni::GetOSProcessManager()->GetCurrentProcess()->GetFile(
        isStdoutLogtype ? eOSProcessFile_StdOut : eOSProcessFile_StdErr);
      niPanicAssert(niIsOK(fpOut));
      fpOut->WriteString(final.Chars());
      if (!niFlagIs(logType,eLogFlags_NoNewLine) &&
          (final.empty() || final[final.size()-1] != '\n')) {
        fpOut->WriteString("\n");
      }
      fpOut->Flush();

#ifdef HAS_CONSOLE_COLORS
      _SetConsoleColors(final,isStdoutLogtype,0,eSetConsoleColorMode::AFTER_FLUSH);
#endif
    }
#endif // LOG_STDIO

#if defined LOG_FILE
    if (_logFileEnabled.get()) {
      if (!_logFileInitialized) {
        _logFileInitialized = eTrue;
        cPath path;
        if (!_logFilePath.get().IsEmpty()) {
          path = _logFilePath.get();
        }
        else if (ni::GetLang()->HasProperty("ni.dirs.logs")) {
          path.SetDirectory(ni::GetLang()->GetProperty("ni.dirs.logs").Chars());
          cString appName = ni::GetLang()->GetProperty("ni.app.name");
          if (appName.empty()) {
            appName = "niApp";
          }
          // we add the PID so that we handle multiple process of the same app correctly
          iOSProcessManager* pProcMan = ni::GetLang()->GetProcessManager();
          if (pProcMan) {
            appName << "-pid-";
            appName << (tI64)pProcMan->GetCurrentProcess()->GetPID();
          }
          path.SetFile(appName.Chars());
          path.SetExtension("log.txt");
        }
        if (!path.IsEmpty()) {
          _logFile = ni::GetRootFS()->FileOpen(path.GetPath().Chars(), _logFileOverwrite.get() ? ni::eFileOpenMode_Write : eFileOpenMode_Append);
          if (_logFile.IsOK()) {
            cString str;
            str << "I/=================================================================\n";
            str << "I/ LOG START\n";
            str << "I/=================================================================\n";
            _logFile->WriteString(str.Chars());
            _logFile->Flush();
          }
        }
      }
      if (_logFile.IsOK()) {
        cString final;
        ni_log_format_message(final,logType,logFile,logLine,logFunc,logMsg,logTime,_fPrevTime);
        _logFile->WriteString(final.Chars());
        if (!niFlagIs(logType,eLogFlags_NoNewLine) &&
            (final.empty() || final[final.size()-1] != '\n')) {
          _logFile->WriteString("\n");
        }
        _logFile->Flush();
      }
    }
#endif

#if defined LOG_OUTPUT_DEBUG_STRING
    {
      cString formatted;
      ni_log_format_message(formatted,logType|eLogFlags_FormatMSVC,
                            logFile,logLine,logFunc,logMsg,logTime,_fPrevTime);

      cString final;
      StrCharIt it = formatted.charIt();
      while (!it.is_end()) {
        const tU32 c = it.next();
        if (c <= 127) {
          final.appendChar(c);
        }
        else {
          StringCatFormat(final, "\\x%X", c);
        }
      }
      if (!niFlagIs(logType,eLogFlags_NoNewLine) &&
          (final.empty() || final[final.size() - 1] != '\n')) {
        final.appendChar('\n');
      }

#if 1
      ::OutputDebugStringA(final.Chars());
#else
      // VS and DbgView++ doesn't print unicode, OutputDebugStringW just
      // replace the unicode characters by '?' (a question mark).
      ni::Windows::UTF16Buffer wMsg;
      niWin32_UTF8ToUTF16(wMsg,final.Chars());
      ::OutputDebugStringW(wMsg.begin());
#endif
    }
#endif

#if defined LOG_APPLE_OS_LOG
    {
      cString final;
      ni_log_format_message(final,logType,logFile,logLine,logFunc,logMsg,-1,-1);
      if (niFlagIs(logType, ni::eLogFlags_Error)) {
        os_log_fault(OS_LOG_DEFAULT, "%{public}s", final.Chars());
      }
      else if (niFlagIs(logType, ni::eLogFlags_Warning)) {
        os_log_error(OS_LOG_DEFAULT, "%{public}s", final.Chars());
      }
#if defined LOG_APPLE_OS_LOG_WITH_INFO_DEBUG
      else if (niFlagIs(logType, ni::eLogFlags_Debug)) {
        os_log_debug(OS_LOG_DEFAULT, "%{public}s", final.Chars());
      }
      else if (niFlagIs(logType, ni::eLogFlags_Info)) {
        os_log_info(OS_LOG_DEFAULT, "%{public}s", final.Chars());
      }
#endif
      else {
        os_log(OS_LOG_DEFAULT, "%{public}s", final.Chars());
      }
    }
#endif // LOG_APPLE_OS_LOG

    // Previous time shouldn't be updated when the relative time is not printed
    if (!niFlagIs(logType,eLogFlags_Raw)) {
      _fPrevTime = logTime;
    }
  }
}

}

using namespace ni;

///////////////////////////////////////////////
void __stdcall cLang::SetLogFilter(tU32 exclude) {
  ni_log_set_filter(exclude);
}
tU32 __stdcall cLang::GetLogFilter() const {
  return ni_log_get_filter();
}
void __stdcall cLang::Log(tLogFlags type, const achar* msg, const achar* file, tU32 line, const achar* func) {
  ni_log(type,msg,file,line,func);
}
