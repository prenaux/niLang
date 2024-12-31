#include "Platform_Generic_StackWalker.h"
#include "API/niLang/Utils/Path.h"

#if !defined niEmbedded
#include "backward.hpp"
#include "API/niLang/Utils/ThreadImpl.h"
#define HAS_CODE_SNIPPET
#endif

namespace ni {

#ifdef HAS_CODE_SNIPPET
static ni::tBool _StackCatCodeSnippet(
  cString& aOutput,
  const std::string& aFileName, int aLine,
  const achar* aIndent, tU32 aContextLines)
{
  // TODO: This is meh, its a cache of all the source files it opened.
  static ThreadMutex _snippetFactoryLock;
  static backward::SnippetFactory _snippetFactory;

  AutoThreadLock lock(_snippetFactoryLock);

  auto lines = _snippetFactory.get_snippet(aFileName, aLine, aContextLines);
  if (!lines.empty()) {
    for (auto it = lines.begin(); it != lines.end(); ++it) {
      if ((int)it->first == aLine) {
        aOutput << aIndent << ">";
      } else {
        aOutput << aIndent << " ";
      }
      aOutput << it->first << ": " << it->second.c_str() << "\n";
    }
    return eTrue;
  }

  return eFalse;
}
#endif

niExportFunc(void) _StackCatEntry(cString& aOutput, tInt frameIndex, const achar* file, tInt line, const achar* func, const achar* module, tIntPtr address, tBool abIncludeCodeSnippet) {
  aOutput.CatFormat("[%d]", frameIndex);

  const char* fileName;
  if (!niStringIsOK(file)) {
    fileName = "noFile";
  }
  else {
#if 0
    // get the filename only, skip the directory
    fileName = StrRChr(file, '/');
    if (!fileName) {
      fileName = StrRChr(file, '\\');
    }
    if (!fileName) {
      fileName = "noFile";
    }
    else {
      fileName += 1; // skip the path separator
    }
#else
    fileName = file;
#endif
  }
  aOutput.CatFormat(" [%s:%d]", fileName, line);

  aOutput.appendChar(' ');
  aOutput.append(niStringIsOK(func) ? func : "noFunc");

  if (niStringIsOK(module)) {
    aOutput.CatFormat(" (%s:%p)", module, address);
  }
  else if (address) {
    aOutput.CatFormat(" (%p)", address);
  }

  aOutput.append("\n");

#ifdef HAS_CODE_SNIPPET
  if (abIncludeCodeSnippet && niStringIsOK(file)) {
    _StackCatCodeSnippet(aOutput, file, line, "    ", 1);
  }
#endif
}

}

#if defined niJSCC || defined niAndroid

namespace ni {

niExportFuncCPP(const cString&) ni_stack_get_current(cString& aOutput, void*, int) {
  aOutput << "--- NO STACK ---\n";
  return aOutput;
}

}
#endif
