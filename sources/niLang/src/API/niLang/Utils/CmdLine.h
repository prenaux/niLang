#pragma once
#ifndef __CMDLINE_H_FBE040FF_0F8A_4BB1_9F92_35517A9DFE09__
#define __CMDLINE_H_FBE040FF_0F8A_4BB1_9F92_35517A9DFE09__

#include "../Types.h"
#include "../StringDef.h"
#include "../IOSProcess.h"
#include "../ILang.h"
#include "../STL/function_ref.h"
#include "../STL/optional.h"

namespace ni {

static inline const achar* GetCurrentOSProcessCmdLine() {
  if (ni::GetOSProcessManager() && ni::GetOSProcessManager()->GetCurrentProcess()) {
    return ni::GetOSProcessManager()->GetCurrentProcess()->GetCommandLine();
  }
  return AZEROSTR;
}

static inline tI32 CmdLineGetParametersPos(const cString& str)
{
  if (str.empty())
    return -1;
  tI32 pos = -1;
  if (str[0] == '\"') {
    pos = 0;
    return str.find('\"',pos+1)+1;
  }
  pos = str.find(' ');
  if (pos >= 0) {
    return pos+1;
  }
  pos = str.find('\t');
  if (pos >= 0) {
    return pos+1;
  }
  return 0;
}

static inline cString CmdLineStrCharItReadFile(StrCharIt& it, const tU32 anEndOnChar = 0, const tU32 anSkipBeginingChar = '=') {
  cString r;

  tU32 endAtQuote = 0;
  tU32 c = eInvalidHandle;

  // skip an equal characters at as the first character
  if (it.peek_next() == anSkipBeginingChar) {
    it.next();
  }

  // skip the starting spaces if a double a quote the file is read until the
  // next double quote
  for (;;) {
    if (it.is_end())
      return r;
    c = it.next();
    if (c == anEndOnChar)
      return r;
    if (c == '\"' || c == '\'') {
      endAtQuote = c;
      break;
    }
    else if (!StrIsSpace(c)) {
      r.appendChar(c);
      break;
    }
  }

  if (endAtQuote) {
    // get all the characters until the next double quote (\")
    for (;;) {
      if (it.is_end())
        return r;
      c = it.next();
      if (c == anEndOnChar)
        return r;
      if (c == endAtQuote) {
        tU32 nextC = it.next();
        if (nextC == endAtQuote) {
          // if the nextC is the closing quote append it instead of closing
          r.appendChar(nextC);
          continue;
        }
        else {
          // skip the end quote and return
          return r;
        }
      }
      else {
        r.appendChar(c);
      }
    }
  }
  else {
    // get all the characters until the next space character
    for (;;) {
      if (it.is_end())
        return r;
      c = it.next();
      if (c == anEndOnChar)
        return r;
      if (StrIsSpace(c)) {
        return r;
      }
      else {
        r.appendChar(c);
      }
    }
  }
}

typedef astl::function_ref<void(const achar* aProperty, const achar* aValue, tBool abIsShortHand)> tCmdLineParseArgFn;
typedef astl::function_ref<void(const cString& aFilename)> tCmdLineParseFileFn;

// Standard command line parsing. Handles -D & -- system properties.
static inline tBool ParseCommandLine(
  const achar* aaszCmdLine,
  astl::optional<tCmdLineParseArgFn> afnParseArg,
  astl::optional<tCmdLineParseFileFn> afnParseFile) {
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
      if (c == '-' && StrIsSpace(it.peek_next())) {
        // -- with a space means the begining of the list of files
        break;
      }

      // --prop=value, -Dprop=value otherwise considered shorthand
      const tBool isShorthand = (c != '-' && c != 'D');
      if (isShorthand) {
        it.prior();
      }
      tOffset nameOffset = tOffset{0};
      cString pname = ni::CmdLineStrCharItReadFile(it,'=');
      const tU32 prior = it.peek_prior();
      cString pvalue;
      if (prior == '=') {
        pvalue = ni::CmdLineStrCharItReadFile(it);
      }
      else {
        if (pname.StartsWith("no-")) {
          pvalue = "false";
          nameOffset = 3;
        }
        else {
          pvalue = "true";
        }
      }
      if (afnParseArg.has_value()) {
        afnParseArg.value()(pname.Chars()+nameOffset,pvalue.Chars(),isShorthand);
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

  // read the remaining as files
  if (afnParseFile.has_value()) {
    const tCmdLineParseFileFn parseFile = afnParseFile.value();
    // get the other arguments...
    while (!it.is_end()) {
      cString arg = ni::CmdLineStrCharItReadFile(it,0,0);
      if (!arg.empty()) {
        parseFile(arg);
      }
    }
  }

  return eTrue;
}

niDeprecated(20230807, ParseCommandLine(tCmdLineParseArgFn,tCmdLineParseFileFn))
static inline tBool ParseCommandLine(const achar* aaszCmdLine, ni::cString* apInputFileName = NULL, ni::tStringCVec* apOtherFiles = NULL, ni::tStringCVec* apUnknownParams = NULL) {

  tU32 numFiles = 0;
  return ParseCommandLine(
    aaszCmdLine,
    tCmdLineParseArgFn{[&](const achar* aProperty, const achar* aValue, tBool aIsShorthand) {
      if (aIsShorthand) {
        if (apUnknownParams) {
          if (niStringIsOK(aValue)) {
            if (ni::StrIEq(aValue,"true")) {
              apUnknownParams->push_back(aProperty);
            }
            else {
              // Pierre: This is jank in quite a few ways but at least we dont lose the value?
              apUnknownParams->push_back(niFmt("'%s'='%s'", aProperty, aValue));
            }
          }
          else {
            apUnknownParams->push_back(aProperty);
          }
        }
      }
      else {
        ni::GetLang()->SetProperty(aProperty,aValue);
      }
    }},
    tCmdLineParseFileFn{[&](const cString& aFile) {
      if (numFiles == 0) {
        if (apInputFileName) {
          *apInputFileName = aFile;
        }
      }
      else {
        if (apOtherFiles) {
          apOtherFiles->Add(aFile);
        }
      }
      ++numFiles;
    }});
}

// Parse a command line that can optionally be split in multiple sections
// using the aToolSplitter character.
// \param aProcessCmdLine the command line to parse
// \param aToolSplitter the delimiter character between sections/tools
// \param aLambda `[&](const achar* aTool, const achar* aCmdLine, int argc, const achar** argv) -> int`
template <typename T>
static int ParseToolsCommandLine(const achar* aProcessCmdLine,
                                 const achar aToolSplitter,
                                 const T& aLambda)
{
  cString processCmdLine = aProcessCmdLine;
  processCmdLine.Trim();

  const tI32 parametersPos = ni::CmdLineGetParametersPos(processCmdLine);
  if (parametersPos <= 0 || parametersPos == processCmdLine.size()) {
    return eInvalidHandle; // no parameters
  }

  const cString exePath = processCmdLine.slice(0,parametersPos).GetWithoutBEQuote();

  tBool isTool = eFalse;
  StrCharIt it = processCmdLine.charZIt(parametersPos);
  while (!it.is_end()) {
    const tU32 c = it.next();
    if (aToolSplitter && c == (tU32)aToolSplitter) {
      isTool = eTrue;
      break;
    }
    if (!StrIsSpace(c)) {
      it.prior();
      break;
    }
  }

  const achar aszToolSpliter[2] = { aToolSplitter, 0 };

  for ( ; !it.is_end(); ) {
    // get the command line
    int inQuote = 0;
    cString tool = "@default";
    astl::vector<cString> args;
    cString cmdLine = _ASTR("\"") + exePath;
    const tBool arg0isTool = isTool;
    if (isTool) {
      cmdLine += aszToolSpliter;
      args.push_back(aszToolSpliter);
    }
    else {
      args.push_back("");
      cmdLine += "\" ";
    }
    while (!it.is_end()) {
      const tU32 c = it.next();
      if (aToolSplitter && c == (tU32)aToolSplitter) {
        isTool = eTrue;
        break;
      }
      else if (c == 0) {
        break;
      }
      else if (c == '\"') {
        if (inQuote) {
          --inQuote;
        }
        else {
          ++inQuote;
        }
      }
      else if (!inQuote && StrIsSpace(c)) {
        if (isTool && args.size() == 1) {
          cmdLine.appendChar('"');
        }
        // new argument
        args.push_back("");
      }
      else {
        args.back().appendChar(c);
      }
      cmdLine.appendChar(c);
    }
    if (args.back().empty()) {
      args.pop_back();
    }

    if (arg0isTool) {
      tool = args[0];
      args[0] = niFmt("\"%s%s\"", exePath, tool);
    }

    astl::vector<const char*> argsPtr;
    // TRACE_CMD_LINE(("TOOL: %s", tool));
    // TRACE_CMD_LINE(("CMDLINE: %s", cmdLine));
    niLoop(i,args.size()) {
      // TRACE_CMD_LINE(("ARGS[%d]: '%s'", i, args[i]));
      argsPtr.push_back(args[i].Chars());
    }

    int r = aLambda(tool.Chars(), cmdLine.Chars(), (int)argsPtr.size(), (const achar**)argsPtr.data());
    if (r != 0) {
      return r;
    }
  }

  return 0;
}

}
#endif // __CMDLINE_H_FBE040FF_0F8A_4BB1_9F92_35517A9DFE09__
