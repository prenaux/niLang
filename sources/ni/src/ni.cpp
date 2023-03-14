// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#if defined _FLYMAKE
#define NI_CONSOLE
#endif

#if defined NI_CONSOLE
#pragma message("Building Console Ni")
#elif defined NI_WINDOWED
#pragma message("Building Windowed Ni")
#else
#error "Ni Build type not defined."
#endif

#include <niLang/Utils/CrashReport.h>
#include <niLang/Utils/ConcurrentImpl.h>
#include <niLang/Utils/ThreadImpl.h>
#include <niLang/STL/set.h>
#include <niLang/Utils/Path.h>
#include <niLang/Utils/StrBreakIt.h>
#include <niLang/IZip.h>
#include <niLang_ModuleDef.h>
#include <niScript.h>
#ifdef NI_WINDOWED
#include <niAppLib.h>
#endif
#include <niLang/Utils/CmdLine.h>
using namespace ni;

#include <stdio.h>

_HDecl(print);

//////////////////////////////////////////////////////////////////////////////////////////////
// VM version string.
#if defined NI_CONSOLE
static const achar* _aszVersion = _A("9.0");
#ifndef NI_EXE_NAME
#define NI_EXE_NAME "ni"
#endif
#elif defined NI_WINDOWED
static const achar* _aszVersion = _A("9.0w");
#ifndef NI_EXE_NAME
#define NI_EXE_NAME "niw"
#endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Options
struct sOptions {
  // Whether to include debug infos.
  tBool _bDebugInfos;
  // Binary output.
  cString _strOutput;
  // Input script.
  cString _strInput;
  // Include directories
  astl::vector<cString> _vIncludes;
  // Library directories
  astl::vector<cString> _vLibraries;
  // Script VM.
  QPtr<iScriptVM> _ptrScriptVM;
  // Args
  Ptr<tStringCVec> _vArgs;
  // Run.
  tBool _bRun;
  // Entry point script (for archives)
  cString _strEntryPoint;
  // Script Main (::main function)
  cString _strScriptMain;
#ifdef NI_WINDOWED
  // Run a hosted app.
  // Remark: This is only for niw so that "ni" doesn't have a dependency on niUI.
  cString _strHostedAppName;
#endif
#ifdef NI_CONSOLE
  // REPL.
  tBool _bRunREPL;
#endif

  sOptions() {
    _bRun = eTrue;
    _strEntryPoint = "main.niw";
    _strScriptMain = "::main";
  }
  void Invalidate() {
    _strOutput = _strInput = _strEntryPoint = _strScriptMain = AZEROSTR;
    _vIncludes.clear();
    _vLibraries.clear();
    _ptrScriptVM = NULL;
    _vArgs = NULL;
  }
};
static sOptions* _GetOptions() {
  static sOptions _options;
  return &_options;
}

//////////////////////////////////////////////////////////////////////////////////////////////
#include <niLang/ObjModel.h>
#include <niLang/Utils/VMCallCImpl.h>

///////////////////////////////////////////////
const iCollection* __stdcall GetArgs() {
  return _GetOptions()->_vArgs;
}
IDLC_STATIC_METH_BEGIN(ni,GetArgs,0) {
  IDLC_DECL_RETVAR(const iCollection*,_Ret);
  IDLC_STATIC_METH_CALL(_Ret,(void),GetArgs,0,());
  IDLC_RET_FROM_INTF(ni::iCollection,_Ret);
} IDLC_STATIC_METH_END(ni,GetArgs,0);
const ni::sMethodDef kFuncDecl_GetArgs = {
  "GetArgs",
  eType_IUnknownPtr|eTypeFlags_Constant, NULL, NULL,
  0, NULL,
  VMCall_GetArgs
};

//////////////////////////////////////////////////////////////////////////////////////////////
cString GetVersionString() {
  return niFmt(NI_EXE_NAME " version %s\n"
               "Build %s, built on %s at %s\n"
               "%s\n",
               _aszVersion,
               niBuildType,
               __DATE__,
               __TIME__,
               niCopyright);
}

cString GetHelpString() {
  cString strHelp =
      _A("usage: " NI_EXE_NAME " [ option... ] scriptfilename... [ script arguments... ]\n")
      _A("\n")
      _A("Available options are:\n")
      _A("   -v             \t displays the version infos\n")
      _A("   -h             \t prints help\n")
      _A("   -d             \t generates debug infos\n")
      _A("   -c             \t compiles only (no output by default)\n")
      _A("   -F logfilter   \t sets the logs to filter out\n")
      _A("   -o filename    \t specifies output file for the -c option\n")
      _A("   -I directory   \t add a lookup folder for import scripts\n")
      _A("   -L directory   \t add a lookup folder for DLL imports\n")
      _A("   -D pname=value \t set the value of a system property\n")
      _A("   -E filename    \t set the entry point script (default: main.niw)\n")
      _A("   -M entrypoint  \t set the main script function (default: ::main)\n")
#ifdef NI_WINDOWED
      _A("   -A appName     \t run as hosted app, the main function is ran in OnAppStarted\n")
#else
      _A("   -i             \t run the repl after executing the script\n")
      _A("\n")
      _A("If the REPL is not ran the script's ::main function is called.")
#endif
      _A("\n")
      ;
  return strHelp;
}

static void PutString(FILE* f, const ni::achar* str, bool bForceNewLine = true) {
  fputs(str,f);
  if (bForceNewLine) {
    if (str[strlen(str)-1] != '\n')
      fputs(_A("\n"),f);
  }
  fflush(f);
}

void InfoExit(const achar* aszMsg) {
  PutString(stdout,aszMsg);
#ifdef NI_WINDOWED
  ni::GetLang()->FatalError(aszMsg);
#endif
  ni::GetLang()->Exit(0);
}

void ErrorHelp(const achar* aszMsg) {
  cString str;
  str += GetVersionString();
  str += _A("\n");
  str += _A("Error:\n");
  str += aszMsg;
  str += _A("\n");
  str += GetHelpString();
  str += _A("\n");
  PutString(stdout,str.Chars());
#ifdef NI_WINDOWED
  ni::GetLang()->FatalError(str.Chars());
#endif
  ni::GetLang()->Exit(0x99);
}

void ErrorExit(const achar* aszMsg) {
  cString str;
  str = _A("Error:\n");
  str += aszMsg;
  str += _A("\n");
  PutString(stdout,str.Chars());
#ifdef NI_WINDOWED
  ni::GetLang()->FatalError(str.Chars());
#endif
  ni::GetLang()->Exit(0x99);
}

void ErrorScript(const astl::vector<cString>* apErrors, const achar* aszMsg) {
  cString str;
  str = _A("Error:\n");
  str += aszMsg;
  if (apErrors) {
    str += "\nDetails:\n";
    niLoop(i,apErrors->size()) {
      if (i != 0) {
        if ((*apErrors)[i] == (*apErrors)[i-1])
          continue;
      }
      str << (*apErrors)[i].Chars();
    }
  }
  str += _A("\n");
  PutString(stdout,str.Chars());
#ifdef NI_WINDOWED
  ni::GetLang()->FatalError(str.Chars());
#endif
  ni::GetLang()->Exit(0x99);
}

void Warning(const achar* aszMsg) {
  fputs(_A("W/"),stdout);
  PutString(stdout,aszMsg);
}

tBool parseCommandLine(const achar* aaszCmdLine) {
  // niDebugFmt(("parseCommandLine: %s", aaszCmdLine));

  cString strCmdLine = aaszCmdLine;
  tI32 parametersPos = ni::CmdLineGetParametersPos(strCmdLine);
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
        case 'v': {
          cString msg;
          msg << GetVersionString();
          InfoExit(msg.Chars());
          break;
        }
        case '?':
        case 'h': {
          cString msg;
          msg << GetVersionString();
          msg << "\n";
          msg << GetHelpString();
          InfoExit(msg.Chars());
          break;
        }
        case 'e':
          // Ignored, not removed because its used in old scripts
          break;
        case 'F': {
          cString strLogFilter = ni::CmdLineStrCharItReadFile(it,0);
          tU32 filter = 0;
          StrCharIt it = strLogFilter.charIt(0);
          while (!it.is_end()) {
            tU32 c = it.next();
            switch (c) {
              case 'R': filter |= eLogFlags_Raw; break;
              case 'D': filter |= eLogFlags_Debug; break;
              case 'I': filter |= eLogFlags_Info; break;
              case 'W': filter |= eLogFlags_Warning; break;
              case 'E': filter |= eLogFlags_Error; break;
              default: {
                niWarning(niFmt("Unknown log filter '%c' (%d).",c,c));
                break;
              }
            }
          }
          ni::GetLang()->SetLogFilter(filter);
          break;
        }
        case 'd':
          // niDebugFmt(("debug infos"));
          _GetOptions()->_bDebugInfos = ni::eTrue;
          break;
        case 'c':
          // niDebugFmt(("compile only"));
          _GetOptions()->_bRun = ni::eFalse;
          break;
#ifdef NI_CONSOLE
        case 'i':
          // niDebugFmt(("repl"));
          _GetOptions()->_bRunREPL = ni::eTrue;
          ni::GetLang()->SetProperty("ni.repl.version",_aszVersion);
          break;
#endif
        case 'o': {
          cString outputFile = ni::CmdLineStrCharItReadFile(it);
          // niDebugFmt(("compile output file: %s",outputFile));
          if (outputFile.empty()) {
            ErrorHelp("-o option, invalid output file name.");
          }
          _GetOptions()->_strOutput = outputFile;
          break;
        }
        case '-': // -- is a synonym for -D
        case 'D': {
          cString pname = ni::CmdLineStrCharItReadFile(it,'=');
          cString pvalue = ni::CmdLineStrCharItReadFile(it);
          // niDebugFmt(("system property: %s = %s",pname,pvalue));
          ni::GetLang()->SetProperty(pname.Chars(),pvalue.Chars());
          break;
        }
        case 'I': {
          cString incl = ni::CmdLineStrCharItReadFile(it,0);
          // niDebugFmt(("add include: %s",incl));
          _GetOptions()->_vIncludes.push_back(incl);
          break;
        }
        case 'L': {
          cString incl = ni::CmdLineStrCharItReadFile(it,0);
          // niDebugFmt(("add library: %s",incl));
          _GetOptions()->_vLibraries.push_back(incl);
          break;
        }
        case 'E': {
          _GetOptions()->_strEntryPoint = ni::CmdLineStrCharItReadFile(it,0);
          break;
        }
        case 'M': {
          _GetOptions()->_strScriptMain = ni::CmdLineStrCharItReadFile(it,0);
          break;
        }
#ifdef NI_WINDOWED
        case 'A': {
          _GetOptions()->_strHostedAppName = ni::CmdLineStrCharItReadFile(it,0);
          niDebugFmt(("run as hosted app: %s", _GetOptions()->_strHostedAppName));
          break;
        }
#endif
        default: {
          ErrorHelp(niFmt("Unknown option -%c",c));
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

  // read the input script
  _GetOptions()->_strInput = ni::CmdLineStrCharItReadFile(it);
  if (_GetOptions()->_strInput.empty()) {
#ifdef NI_CONSOLE
    if (_GetOptions()->_bRunREPL) {
      return eTrue;
    }
    else
#endif
    {
      ErrorHelp("empty input file name.");
    }
  }

  // arg[0] is the input file name
  // niDebugFmt(("I/input file: %s",_GetOptions()->_strInput));
  if (!_GetOptions()->_vArgs.IsOK())
    _GetOptions()->_vArgs = tStringCVec::Create();
  _GetOptions()->_vArgs->Add(_GetOptions()->_strInput);

  // get the other arguments...
  while (!it.is_end()) {
    cString arg = ni::CmdLineStrCharItReadFile(it,0,0);
    if (!arg.empty()) {
      _GetOptions()->_vArgs->Add(arg);
      // niDebugFmt(("I/script arg[%d]: %s",
      // _GetOptions()->_vArgs->GetSize()-1,
      // _GetOptions()->_vArgs->Get(_GetOptions()->_vArgs->GetSize()-1)));
    }
  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  REPL function
//
//--------------------------------------------------------------------------------------------
#ifdef NI_CONSOLE

#include <niScript/VMAPI.h>

struct sMonitoredFile {
  cString          fileName;
  Ptr<iTime> fileTime;
  sMonitoredFile() {
  }
  sMonitoredFile(const achar* aaszFileName) {
    fileName = aaszFileName;
  }
  bool operator < (const sMonitoredFile& aRight) const {
    return fileName.cmp(aRight.fileName) < 0;
  }
  bool updateFileTime() const {
    Ptr<iFile> fp = _GetOptions()->_ptrScriptVM->ImportFileOpen(fileName.Chars());
    if (!fp.IsOK()) {
      PutString(stdout,niFmt("\nE/isOutOfDate: can't open '%s'.",fileName));
      return false;
    }

    Ptr<iTime> newFileTime = ni::GetLang()->GetCurrentTime()->Clone();
    fp->GetTime(eFileTime_LastWrite,newFileTime);

    if (!fileTime.IsOK() || (newFileTime->Compare(fileTime) != 0)) {
      niThis(sMonitoredFile)->fileTime = newFileTime;
      return true;
    }

    return false;
  }
};
typedef astl::set<sMonitoredFile> tMonitoredFiles;

void monitoredImport(tMonitoredFiles& aSet, const achar* aaszFileName) {
  Ptr<iFile> fp = _GetOptions()->_ptrScriptVM->ImportFileOpen(aaszFileName);
  if (!fp.IsOK()) {
    PutString(stdout,niFmt("E/import: can't find file '%s'.",aaszFileName));
    return;
  }

  tMonitoredFiles::iterator itFile = aSet.find(aaszFileName);
  if (itFile != aSet.end()) {
    PutString(stdout,niFmt("I/import: already monitoring '%s'.",aaszFileName));
    return;
  }

  if (!_GetOptions()->_ptrScriptVM->NewImport(_H(aaszFileName),NULL)) {
    PutString(stdout,niFmt("I/import: can't import '%s'.",aaszFileName));
    return;
  }

  sMonitoredFile mf;
  mf.fileName = aaszFileName;
  mf.updateFileTime();
  aSet.insert(mf);

  PutString(stdout,niFmt("I/import: monitoring '%s'.",aaszFileName));
}

void monitoredUpdate(tMonitoredFiles& aSet) {
  niLoopit(tMonitoredFiles::iterator,it,aSet) {
    if (it->updateFileTime()) {
      if (_GetOptions()->_ptrScriptVM->NewImport(_H(it->fileName),NULL)) {
        PutString(stdout,niFmt("I/import: updated '%s'.",it->fileName));
      }
      else {
        PutString(stdout,niFmt("E/import: can't update '%s'.",it->fileName));
      }
    }
    else {
      PutString(stdout,niFmt("I/import: '%s' up-to-date.",it->fileName));
    }
  }
}

#ifdef niWindows
#include <niLang/Platforms/Win32/Win32_File.h>
static cOSWinFile winStdIn(
    ::GetStdHandle(STD_INPUT_HANDLE),
    ::GetStdHandle(STD_INPUT_HANDLE),
    eOSWinFileFlags_DontOwnRW,
    "STDIN");
#endif

static tBool _bREPLClose = eFalse;
static int _lastCompilerErrorLine = -1;
static int _lastCompilerErrorColumn = -1;

///////////////////////////////////////////////
void __stdcall REPL_Close() {
  _bREPLClose = eTrue;
}
IDLC_STATIC_METH_BEGIN(ni,REPL_Close,0) {
  IDLC_STATIC_METH_CALL_VOID(,(void),REPL_Close,0,());
} IDLC_STATIC_METH_END(ni,REPL_Close,0);
const ni::sMethodDef kFuncDecl_REPL_Close = {
  "REPL_Close",
  eType_Null,NULL,NULL,
  0,NULL,
  VMCall_REPL_Close
};

///////////////////////////////////////////////
static Ptr<iCallback> _REPLCompletionCallback = NULL;
tBool __stdcall REPL_SetCompletionCallback(iCallback* apCallback) {
  _REPLCompletionCallback = apCallback;
  return _REPLCompletionCallback.IsOK();
}
IDLC_STATIC_METH_BEGIN(ni,REPL_SetCompletionCallback,1) {
  IDLC_DECL_VAR(iCallback*,aCallback);
  IDLC_BUF_TO_INTF(iCallback,aCallback);
  IDLC_DECL_RETVAR(tBool,_Ret);
  IDLC_STATIC_METH_CALL(_Ret,,REPL_SetCompletionCallback,1,(aCallback));
  IDLC_RET_FROM_BASE(eType_Bool,_Ret);
} IDLC_STATIC_METH_END(ni,REPL_SetCompletionCallback,1);
const ni::sMethodDef kFuncDecl_REPL_SetCompletionCallback = {
	"REPL_SetCompletionCallback",
  eType_I8,NULL,NULL,
  1,NULL,
  VMCall_REPL_SetCompletionCallback
};

///////////////////////////////////////////////
static Ptr<iCallback> _REPLRunCallback = NULL;
tBool __stdcall REPL_SetRunCallback(iCallback* apCallback) {
  _REPLRunCallback = apCallback;
  return _REPLRunCallback.IsOK();
}
IDLC_STATIC_METH_BEGIN(ni,REPL_SetRunCallback,1) {
  IDLC_DECL_VAR(iCallback*,aCallback);
  IDLC_BUF_TO_INTF(iCallback,aCallback);
  IDLC_DECL_RETVAR(tBool,_Ret);
  IDLC_STATIC_METH_CALL(_Ret,,REPL_SetRunCallback,1,(aCallback));
  IDLC_RET_FROM_BASE(eType_Bool,_Ret);
} IDLC_STATIC_METH_END(ni,REPL_SetRunCallback,1);
const ni::sMethodDef kFuncDecl_REPL_SetRunCallback = {
  "REPL_SetRunCallback",
  eType_I8,NULL,NULL,
  1,NULL,
  VMCall_REPL_SetRunCallback
};

///////////////////////////////////////////////
static void REPL_CompilerErrorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, int line, int column)
{
  cString strDesc;
  ni_log_format_message(
      strDesc,eLogFlags_Error,
      NULL,0,NULL,
      niFmt("Compilation Error (L%d C%d): %s",line,column,desc),
      -1,-1);
  PutString(stderr, strDesc.Chars());
  _lastCompilerErrorLine = line;
  _lastCompilerErrorColumn = column;
}

#if defined niPosix
niExportFunc(int) waitForNextTerminalChar();
#else
static int waitForNextTerminalChar() {
  return getchar();
}
#endif

///////////////////////////////////////////////
void REPL(HSQUIRRELVM v)
{
#ifdef niWindows
  static HANDLE _hStdIn = ::GetStdHandle(STD_INPUT_HANDLE);
  DWORD dwConsoleMode = 0;
  GetConsoleMode(_hStdIn,&dwConsoleMode);
  dwConsoleMode |= ENABLE_QUICK_EDIT_MODE | ENABLE_PROCESSED_OUTPUT;
  SetConsoleMode(_hStdIn,dwConsoleMode);
#endif


#define MAXINPUT 4096
  SQChar buffer[MAXINPUT];
  SQInt blocks =0;
  SQInt string=0;
  SQInt retval=0;
  astl::set<sMonitoredFile> monitored;

  PutString(stdout,GetVersionString().Chars(),false);
  PutString(stdout,niFmt("\nREPL running in '%s' thread '0x%x'",
                         (ni::GetConcurrent()->GetMainThreadID() ==
                          ni::GetConcurrent()->GetCurrentThreadID()) ? "main" : "background",
                         ni::GetConcurrent()->GetCurrentThreadID()),
            false);
  PutString(stdout,"\nInput ':q' to exit the REPL",false);
  PutString(stdout,"\nInput ':import filename' to import a script to be monitored",false);
  PutString(stdout,_A("\nni> "),false);

  while (!_bREPLClose)
  {
    int lastNonEmptyCharOfLine = 0;
    tBool isMultiLine = ni::eFalse;
    SQInt i = 0;

    while (!_bREPLClose) {
      int c = waitForNextTerminalChar();
      if (_bREPLClose)
        return;

      if ((i+2+(blocks*2)) >= MAXINPUT) {
        PutString(stdout,"E/Input too long.");
        i = 0;
        break;
      }

      if (c && !StrIsSpace(c))
        lastNonEmptyCharOfLine = c;

      if (c == _A('\n')) {
        if (i>0 && buffer[i-1] == _A('\\')) {
          buffer[i-1] = _A('\n');
        }
        else if (blocks == 0) {
          if (i > 0 &&
              (lastNonEmptyCharOfLine == _A('&') ||
               lastNonEmptyCharOfLine == _A('|') ||
               lastNonEmptyCharOfLine == _A('^') ||
               lastNonEmptyCharOfLine == _A('<') ||
               lastNonEmptyCharOfLine == _A('>') ||
               lastNonEmptyCharOfLine == _A('=') ||
               lastNonEmptyCharOfLine == _A('%') ||
               lastNonEmptyCharOfLine == _A('+') ||
               lastNonEmptyCharOfLine == _A('-') ||
               lastNonEmptyCharOfLine == _A('*') ||
               lastNonEmptyCharOfLine == _A('/') ||
               lastNonEmptyCharOfLine == _A('?') ||
               lastNonEmptyCharOfLine == _A(':') ||
               lastNonEmptyCharOfLine == _A(',') ||
               lastNonEmptyCharOfLine == _A('!')))
          {
            // continue to the next line
          }
          else {
            // end of code to eval
            break;
          }
        }
        buffer[i++] = _A('\n');

        static cString indent;
        const tU32 indentLen = 4 + blocks*2;
        if (indent.size() < indentLen) {
          indent.resize(indentLen);
        }
        {
          achar* pWriteBuffer = indent.data();
          pWriteBuffer[indentLen] = 0;
          memset(pWriteBuffer,' ',indentLen);
          PutString(stdout,pWriteBuffer,false);
        }

        niLoop(j,blocks*2) {
          buffer[i++] = ' ';
        }

        lastNonEmptyCharOfLine = 0;
        isMultiLine = ni::eTrue;
      }
      else if (!string && (c==_A('}') || c==_A(']') || c==_A(')'))) {
        blocks--;
        buffer[i++] = (SQChar)c;
      }
      else if (!string && (c==_A('{') || c==_A('[') || c==_A('('))) {
        blocks++;
        buffer[i++] = (SQChar)c;
      }
      else if(c==_A('"') || c==_A('\'')){
        string=!string;
        buffer[i++] = (SQChar)c;
      }
      else {
        buffer[i++] = (SQChar)c;
      }
    }
    if (_bREPLClose)
      return;

    buffer[i] = _A('\0');

    // add the return statement before the expression
    cString codeToRun;

    // :repl commands
    if (!isMultiLine && (buffer[0] == ':' && buffer[1] != ':')) {
      if (StrICmp(buffer,":q") == 0) {
        _bREPLClose = eTrue;
        return;
      }
      cString cmd = buffer;
      if (cmd.StartsWith(":import ")) {
        cString path = cmd.After(":import ");
        path.ToLower();
        monitoredImport(monitored, path.Chars());
        continue;
      }
      else if (cmd.StartsWith(":clear ")) {
        codeToRun << cmd.After(":clear ");
        if (codeToRun.empty())
          continue;
      }
      else {
        PutString(stdout,niFmt("W/Unknown repl command '%s'",buffer));
        continue;
      }
    }
    else {
      codeToRun << buffer;
      codeToRun.Trim();
      if (!codeToRun.StartsWith("for") &&
          !codeToRun.StartsWith("if") &&
          !codeToRun.StartsWith("do") &&
          !codeToRun.StartsWith("while") &&
          !codeToRun.StartsWith("switch"))
      {
        codeToRun = _ASTR("return ") + codeToRun;
      }
    }

    monitoredUpdate(monitored);

    // niDebugFmt(("I/COMPILING: %s", codeToRun));

    Ptr<iRunnable> runCode = ni::Runnable([&v,&retval,codeToRun]() -> tBool {
        retval = 1;
        SQInt oldtop = sq_gettop(v);
        sq_setcompilererrorhandler(v, REPL_CompilerErrorHandler);
        if (SQ_SUCCEEDED(sq_compilebuffer(
                v,
                codeToRun.Chars(),codeToRun.size(),
                _A("REPL"),ni::eTrue)))
        {
          sq_pushroottable(v);
          if (SQ_SUCCEEDED(sq_call(v,1,retval)) && retval) {
            if (codeToRun.contains("print")) {
              PutString(stdout,_A("\n==> "),false);
            }
            else {
              PutString(stdout,_A("==> "),false);
            }
            sq_pushroottable(v);
            sq_pushstring(v,_HC(print));
            sq_get(v,-2);
            sq_pushroottable(v);
            sq_push(v,-4);
            sq_call(v,2,ni::eFalse);
            retval = 0;
          }
        }
        else {
          int line = 1;
          StrBreakIt<StrBreakLine> breaker(codeToRun.charIt());
          while (!breaker.is_end()) {
            PutString(stderr,cString(breaker.current()).Chars());
            if (_lastCompilerErrorLine == line &&
                (_lastCompilerErrorColumn > 0 &&
                 _lastCompilerErrorColumn < 120))
            {
              cString marker;
              marker.reserve(_lastCompilerErrorColumn+2);
              niLoop(i,_lastCompilerErrorColumn-1) {
                marker.appendChar(' ');
              }
              marker.appendChar('^');
              PutString(stderr,marker.Chars());
            }
            ++line;
            breaker.next();
          }
        }
        sq_setcompilererrorhandler(v, NULL);
        sq_settop(v,oldtop);

        PutString(stdout,_A("\nni> "),false);
        return eTrue;
      });


    if (_REPLRunCallback.IsOK()) {
      _REPLRunCallback->RunCallback(runCode.ptr(),codeToRun);
    }
    else {
      runCode->Run();
    }
  }
}
#endif

//--------------------------------------------------------------------------------------------
//
//  URLFileHandler
//
//--------------------------------------------------------------------------------------------
struct URLFileHandler_Zip_ScriptHandler : public cIUnknownImpl<iURLFileHandler> {
  Ptr<iURLFileHandler> _zipURLFileHandler;

  URLFileHandler_Zip_ScriptHandler(iURLFileHandler* zipURLFileHandler) {
    _zipURLFileHandler = zipURLFileHandler;
  }

  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    cString path = StringURLGetPath(aURL);
    Ptr<iFile> fp = _zipURLFileHandler->URLOpen(path.Chars());
    if (!fp.IsOK()) {
      path = "scripts/";
      path += StringURLGetPath(aURL);
      fp = _zipURLFileHandler->URLOpen(path.Chars());
    }
    return fp.GetRawAndSetNull();
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    cString path = StringURLGetPath(aURL);
    if (!_zipURLFileHandler->URLExists(path.Chars())) {
      path = "scripts/";
      path += StringURLGetPath(aURL);
      return _zipURLFileHandler->URLExists(path.Chars());
    }
    else {
      return eTrue;
    }
  }
};

struct URLFileHandler_Directory : public cIUnknownImpl<iURLFileHandler> {
  astl::vector<cString> _dirs;

  URLFileHandler_Directory() {
    cPath path;
    path.SetDirectory(ni::GetLang()->GetProperty("ni.dirs.data").Chars());
    _dirs.push_back(path.GetPath());
  }

  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    const cString path = _FindFile(StringURLGetPath(aURL));
    // niDebugFmt(("... URLFileHandler.dir.URLOpen: %s -> %s", aURL, path));
    if (path.IsEmpty()) return NULL;
    return ni::GetRootFS()->FileOpen(path.Chars(),eFileOpenMode_Read);
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    const cString path = _FindFile(StringURLGetPath(aURL));
    if (path.IsEmpty()) return eFalse;
    return eTrue;
  }

  cString __stdcall _FindFile(const cString& basePath) {
    if (ni::GetRootFS()->FileExists(basePath.Chars(),eFileAttrFlags_AllFiles)) {
      return basePath;
    }
    niLoopr(i,(tU32)_dirs.size()) {
      cString tryPath = _dirs[i] + basePath;
      if (ni::GetRootFS()->FileExists(tryPath.Chars(),eFileAttrFlags_AllFiles)) {
        return tryPath;
      }
    }
    return AZEROSTR;
  }
};

//--------------------------------------------------------------------------------------------
//
//  Win32
//
//--------------------------------------------------------------------------------------------
#ifdef niWindows
#include <niLang/Platforms/Win32/Win32_Registry.h>

static cString MyWinRegRead(const achar* aaszPath) {
  achar ret[4096] = {0};
  return Windows::WinRegRead(ret,aaszPath);
}

IDLC_STATIC_METH_BEGIN(ni,WinRegWrite,3) {
  IDLC_DECL_RETVAR(ni::tI8,_Ret);
  IDLC_DECL_VAR(ni::achar*,path);  IDLC_BUF_TO_BASE(ni::eType_ASZ,path);
  IDLC_DECL_VAR(ni::achar*,value); IDLC_BUF_TO_BASE(ni::eType_ASZ,value);
  IDLC_DECL_VAR(ni::achar*,type);  IDLC_BUF_TO_BASE(ni::eType_ASZ,type);
  IDLC_STATIC_METH_CALL_NS_VOID(_Ret,Windows,WinRegWrite,3,(path,value,type));
  IDLC_RET_FROM_BASE(ni::eType_I8,_Ret);
} IDLC_STATIC_METH_END(MsWin,WinRegWrite,3);
const ni::sMethodDef kFuncDecl_WinRegWrite = {
  "WinRegWrite",
  eType_I8,NULL,NULL,
  3,NULL,
  VMCall_WinRegWrite
};

IDLC_STATIC_METH_BEGIN(MsWin,WinRegRead,1) {
  IDLC_DECL_RETVAR(ni::cString,_Ret);
  IDLC_DECL_VAR(ni::achar*,path); IDLC_BUF_TO_BASE(ni::eType_ASZ,path);
  IDLC_STATIC_METH_CALL(_Ret,_,MyWinRegRead,1,(path));
  IDLC_RET_FROM_BASE(ni::eType_String,_Ret);
} IDLC_STATIC_METH_END(MsWin,WinRegRead,1);
const ni::sMethodDef kFuncDecl_WinRegRead = {
  "WinRegRead",
  eType_String,NULL,NULL,
  1,NULL,
  VMCall_WinRegRead
};

///////////////////////////////////////////////
void __stdcall MsWin_ScriptRegister(iScriptVM* apVM) {
  apVM->RegisterFunction(&kFuncDecl_WinRegWrite,_A("MsWinRegWrite"));
  apVM->RegisterFunction(&kFuncDecl_WinRegRead, _A("MsWinRegRead"));
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Main function
//
//--------------------------------------------------------------------------------------------
static Var OnExit() {
  ::fflush(stdout);
  ::fflush(stderr);
  _GetOptions()->_vArgs = NULL;
#pragma niNote("The script VM HAS to be invalidated before being released, otherwise resources will leak, this is because an instance of an object created in a table may contain a reference to this VM and so create a cycle.")
  if (_GetOptions()->_ptrScriptVM.IsOK()) {
    _GetOptions()->_ptrScriptVM->Invalidate();
    _GetOptions()->_ptrScriptVM = (iScriptVM*)NULL;
  }
  _GetOptions()->Invalidate();
  return eTrue;
}

int
#ifdef NI_CONSOLE
ni_main
#else
niw_main
#endif
(const achar* aCommandLine)
{
  // Init the options
  if (!parseCommandLine(aCommandLine)) {
    ErrorHelp("Invalid Command Line.");
  }

#ifdef NI_CONSOLE
  // Some sanity check for arguments conflict
  if (_GetOptions()->_bRunREPL) {
    if (!_GetOptions()->_bRun) {
      ErrorHelp("REPL specified with compile-only.");
    }
  }
#endif

  // Add the OnExit hook
  ni::GetLang()->OnExit(ni::Runnable<tpfnRunnable>(OnExit));

  // Initialize the VM
  {
    _GetOptions()->_ptrScriptVM = niCreateInstance(niScript,ScriptVM,ni::GetConcurrent(),0);
    if (!niIsOK(_GetOptions()->_ptrScriptVM))
      ErrorExit(_A("Cant create the virtual machine !\n"));

    ni::GetLang()->AddScriptingHost(_H("ni"),_GetOptions()->_ptrScriptVM);

    _GetOptions()->_ptrScriptVM->RegisterFunction(&kFuncDecl_GetArgs,"GetArgs");
#ifdef NI_CONSOLE
    if (_GetOptions()->_bRunREPL) {
      _GetOptions()->_ptrScriptVM->RegisterFunction(&kFuncDecl_REPL_Close,"REPL_Close");
      _GetOptions()->_ptrScriptVM->RegisterFunction(&kFuncDecl_REPL_SetCompletionCallback,"REPL_SetCompletionCallback");
      _GetOptions()->_ptrScriptVM->RegisterFunction(&kFuncDecl_REPL_SetRunCallback,"REPL_SetRunCallback");
    }
#endif

#ifdef niWindows
    MsWin_ScriptRegister(_GetOptions()->_ptrScriptVM);
#endif

    if (_GetOptions()->_bDebugInfos) {
      _GetOptions()->_ptrScriptVM->EnableDebugInfos(ni::eTrue);
    }
  }

  // Add the library directories...
  if (!_GetOptions()->_vLibraries.empty())
  {
    cString envPATH = ni::GetLang()->GetEnv("PATH");
    // niDebugFmt(("I/PATH: %s", envPATH));
    const cString binLOA = niFmt("%s-%s",
                                 ni::GetLang()->GetProperty("ni.loa.os"),
                                 ni::GetLang()->GetProperty("ni.loa.arch"));
    niLoop(i,_GetOptions()->_vLibraries.size()) {
      cPath path;
      path.SetDirectory(_GetOptions()->_vLibraries[i].Chars());
      path.AddDirectoryBack(binLOA.Chars());
      if (!ni::GetLang()->GetRootFS()->FileExists(
              path.GetPath().Chars(),ni::eFileAttrFlags_AllDirectories))
      {
        // remove the binLOA folder
        path.RemoveDirectoryBack();
        if (!ni::GetLang()->GetRootFS()->FileExists(
                path.GetPath().Chars(),ni::eFileAttrFlags_AllDirectories))
        {
          niWarning(niFmt("Can't add library directory '%s'.",path.GetPath().Chars()));
          continue;
        }
      }

      envPATH = path.GetPath() +
#ifdef niWindows
          ";"
#else
          ":"
#endif
          + envPATH;
    }
    // niDebugFmt(("I/NEWPATH: %s", envPATH));
    ni::GetLang()->SetEnv("PATH",envPATH.Chars());
  }

  // Add the include directories...
  {
    niLoop(i,_GetOptions()->_vIncludes.size()) {
      cString path = _GetOptions()->_vIncludes[i];
      if (ni::GetLang()->GetRootFS()->FileExists(
              path.Chars(),ni::eFileAttrFlags_AllDirectories))
      {
        path = ni::GetLang()->GetRootFS()->GetAbsolutePath(path.Chars());
        Ptr<iFileSystem> scriptsDirFS = ni::GetLang()->CreateFileSystemDir(
            path.Chars(),ni::eFileSystemRightsFlags_ReadOnly);
        _GetOptions()->_ptrScriptVM->GetImportFileSystems()->Add(scriptsDirFS.ptr());
      }
      else {
        niWarning(niFmt("Can't add include directory '%s'.",path));
      }
    }
  }

  // Process the input file...
  cString strInput = _GetOptions()->_strInput;
  ni::Ptr<ni::iFile> ptrInputFile;
  {
#ifdef NI_CONSOLE
    if (strInput.empty()) {
      if (!_GetOptions()->_bRunREPL) {
        ErrorHelp(_A("No input file specified."));
      }
    }
#endif

    {
      if (!strInput.empty()) {
        strInput = strInput.GetWithoutBEQuote();
        {
          ptrInputFile = ni::GetRootFS()->FileOpen(strInput.Chars(), eFileOpenMode_Read);
          if (!ptrInputFile.IsOK()) {
            ptrInputFile = _GetOptions()->_ptrScriptVM->ImportFileOpen(strInput.Chars());
          }
        }

        if (!ptrInputFile.IsOK()) {
          cString msg = niFmt(_A("Can't open input file '%s' !\n"),_GetOptions()->_strInput);
#ifdef NI_CONSOLE
          if (_GetOptions()->_bRunREPL) {
            if (!_GetOptions()->_strInput.empty()) {
              Warning(msg.Chars());
            }
          }
          else
#endif
          {
            ErrorHelp(msg.Chars());
          }
        }
      }

      if (ptrInputFile.IsOK()) {
        if (strInput.EndsWithI(".exe") ||
            strInput.EndsWithI(".jar") ||
            strInput.EndsWithI(".zip") ||
            strInput.EndsWithI(".niz"))
        {
          QPtr<iURLFileHandler> zipURLFileHandler = niCreateInstance(niLang,URLFileHandlerZip,ptrInputFile.ptr(),niVarNull);
          if (!zipURLFileHandler.IsOK()) {
#ifdef NI_CONSOLE
            if (!_GetOptions()->_bRunREPL) {
              ErrorExit(niFmt(_A("Can't open '%s' as an archive !\n"),strInput.Chars()));
            }
#endif
            ptrInputFile = NULL;
          }
          else {
            ni::GetLang()->SetGlobalInstance("URLFileHandler.default",zipURLFileHandler);
            ni::GetLang()->SetGlobalInstance("URLFileHandler.script",niNew URLFileHandler_Zip_ScriptHandler(zipURLFileHandler));

            ptrInputFile = zipURLFileHandler->URLOpen(_GetOptions()->_strEntryPoint.Chars());
            if (!ptrInputFile.IsOK()) {
              ErrorExit(niFmt(_A("Can't find entry point script '%s' in archive '%s' !\n"),
                              _GetOptions()->_strEntryPoint,
                              strInput.Chars()));
            }

            // The input is now the entry point in the archive
            strInput = _GetOptions()->_strEntryPoint;
          }
        }
        else {
          Ptr<URLFileHandler_Directory> dirURLHandler = niNew URLFileHandler_Directory();
          niLoop(i,_GetOptions()->_vIncludes.size()) {
            cString path = _GetOptions()->_vIncludes[i];
            dirURLHandler->_dirs.push_back(path);
          }
          // ni::GetLang()->SetGlobalInstance("URLFileHandler.script", dirURLHandler);
          ni::GetLang()->SetGlobalInstance("URLFileHandler.file", dirURLHandler);
          ni::GetLang()->SetGlobalInstance("URLFileHandler.default", dirURLHandler);
        }
      }
    }
  }

  // output
  if (!_GetOptions()->_bRun || _GetOptions()->_strOutput.IsNotEmpty()) {
      Ptr<iScriptObject> ptrCompiled = _GetOptions()->_ptrScriptVM->Compile(ptrInputFile);
    if (!niIsOK(ptrCompiled)) {
      ErrorExit(niFmt(_A("Script compilation error in '%s' !\n"),strInput.Chars()));
    }

    if (_GetOptions()->_strOutput.IsNotEmpty()) {
      ni::Ptr<ni::iFile> ptrOutFile = ni::GetRootFS()->FileOpen(
        _GetOptions()->_strOutput.Chars(),eFileOpenMode_Write);
      if (!niIsOK(ptrOutFile)) {
        ErrorHelp(niFmt(_A("Can't open the output file '%s'."),
                        _GetOptions()->_strOutput.Chars()));
      }
      if (!_GetOptions()->_ptrScriptVM->WriteClosure(ptrOutFile,ptrCompiled)) {
        ErrorHelp(niFmt(_A("Can't write the closure to the output file '%s'."),
                        _GetOptions()->_strOutput.Chars()));
      }
    }
  }

  tU32 nRet = 0;
  const tBool hasMainScript = (_GetOptions()->_bRun && !strInput.empty());

#ifdef NI_CONSOLE
  Ptr<iFuture> replFuture;
  if (_GetOptions()->_bRunREPL) {
    Ptr<iExecutor> replExecutor;
    if (!_GetOptions()->_ptrScriptVM->Import(_H("repl.ni"),NULL)) {
      Warning("REPL startup: Can't open repl.ni");
    }

    auto runREPL = [&]() -> tU32 {
      if (!_GetOptions()->_bRun) {
        // nothing ran before, so we'll import lang.ni & algo.ni
        if (!_GetOptions()->_ptrScriptVM->Import(_H("lang.ni"),NULL)) {
          ErrorExit("REPL startup: Can't open lang.ni");
        }
        if (!_GetOptions()->_ptrScriptVM->Import(_H("algo.ni"),NULL)) {
          ErrorExit("REPL startup: Can't open algo.ni");
        }
      }
      REPL((HSQUIRRELVM)_GetOptions()->_ptrScriptVM->GetHandle());
      return 0;
    };

    if (hasMainScript) {
      // run the repl in a thread
      replFuture = ni::GetConcurrent()->ThreadRun(ni::Runnable([&]() {
        runREPL();
        return eTrue;
      }));
    }
    else {
      // run the repl in the main thread
      runREPL();
    }
  }
#endif

  if (hasMainScript) {
    auto runMain = [&]() -> tU32 {
      // run
      if (!_GetOptions()->_ptrScriptVM->Import(ptrInputFile,NULL)) {
        ErrorExit("Script call error.");
      }

      // call ::main
      Var ret((ni::tU32)1);
      Var params[1] = { _GetOptions()->_vArgs.ptr() };
      _GetOptions()->_ptrScriptVM->ScriptCall(
        NULL,
        _GetOptions()->_strScriptMain.Chars(),
        params, niCountOf(params),
        &ret);
      return ret.mU32;
    };

#ifdef NI_WINDOWED
    if (_GetOptions()->_strHostedAppName.IsNotEmpty()) {
      niLog(Info,niFmt("Starting hosted app '%s'.", _GetOptions()->_strHostedAppName));
      ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine());

      Nonnull<app::AppContext> appContext = ni::MakeNonnull<app::AppContext>();
      if (!app::AppNativeStartup(
            appContext,
            _GetOptions()->_strHostedAppName.Chars(),
            0, 0,
            ni::Runnable([&]() { return runMain(); }),
            NULL))
      {
        ErrorExit("Can't start hosted application.");
      }
      nRet = app::AppNativeMainLoop(appContext);
    }
    else
#endif
    {
      nRet = runMain();
    }
  }

#ifdef NI_CONSOLE
  if (replFuture.IsOK()) {
    // Wait for the REPL to close...
    replFuture->Wait(eInvalidHandle);
  }
#endif

  niEndMain(nRet);
}

#if !defined NI_NO_MAIN
niCrashReport_DeclareHandler();

#ifdef NI_CONSOLE
niConsoleMain()
#else
niWindowedMain()
#endif
{
#ifdef NI_CONSOLE
  ni::GetLang()->SetProperty("ni.app.name","ni");
#else
  ni::GetLang()->SetProperty("ni.app.name","niw");
#endif

#ifdef NI_CONSOLE
  return ni_main(ni::GetOSProcessManager()->GetCurrentProcess()->GetCommandLine());
#else
  return niw_main(ni::GetOSProcessManager()->GetCurrentProcess()->GetCommandLine());
#endif
}

#endif
