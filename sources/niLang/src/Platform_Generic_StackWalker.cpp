#include "Platform_Generic_StackWalker.h"
#include "API/niLang/Utils/Path.h"

#if !defined niEmbedded
#define BACKWARD_HAS_BFD 1
#define BACKWARD_HAS_UNWIND 1
#include "backward.hpp"
#include "API/niLang/Utils/ThreadImpl.h"
#endif

namespace ni {

#if !defined niEmbedded // we're not gonna have the source on embedded systems anyway...
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
      if (it->first == aLine) {
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

  if (abIncludeCodeSnippet && niStringIsOK(file)) {
    _StackCatCodeSnippet(aOutput, file, line, "    ", 5);
  }
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
