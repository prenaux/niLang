#include "API/niLang/Types.h"

#if defined niOSX || defined niIOS || defined niLinux

#include "Platform_Generic_StackWalker.h"
#include "API/niLang/StringDef.h"
#include <stdio.h>
#include <dlfcn.h>     // for dladdr
#include <cxxabi.h>    // for __cxa_demangle
#include <execinfo.h>  // for backtrace

// #define USE_BACKTRACE_SYMBOLS
// #define USE_BACKTRACE_DLADDR
// #define USE_BOOST_STACKTRACE
// #define USE_BACKWARD

#define STACK_INCLUDE_CODE_SNIPPETS eTrue // Should this be a run-time flag?

#ifdef niLinux
#define USE_BACKWARD
#else
#define USE_BACKTRACE_DLADDR
#endif

#ifdef USE_BOOST_STACKTRACE
// cf: https://www.boost.org/doc/libs/1_86_0/doc/html/stacktrace/configuration_and_build.html
// #define BOOST_STACKTRACE_USE_BACKTRACE
// #define BOOST_STACKTRACE_BACKTRACE_INCLUDE_FILE </usr/lib/gcc/x86_64-linux-gnu/5/include/backtrace.h>
// #define BOOST_STACKTRACE_USE_NOOP // to disable backtracing
#define BOOST_STACKTRACE_USE_ADDR2LINE // slow but accurate, when addr2line cant be found we get an empty file/line
// #define BOOST_STACKTRACE_ADDR2LINE_LOCATION "/wrongloc_totest/addr2line"
#include <boost/stacktrace.hpp>
#endif

#if defined USE_BACKWARD
// Note: One of these must be enabled to extract the source:line info.
// #define BACKWARD_HAS_BFD 1 // apt-get install binutils-dev, pacman -S binutils
// #define BACKWARD_HAS_DWARF 1 // apt-get install libdwarf-dev, pacman has no usable package OOB :(
// #define BACKWARD_HAS_DW 1 // apt-get install libdw-dev, pacman has no usable package OOB :(
#include "backward_impl.hpp"
#endif

namespace ni {

niExportFuncCPP(const cString&) ni_stack_get_current(cString& aOutput, void* apExp, int skip) {
  niUnused(apExp);
  const int nMaxFrames = 100;
  aOutput.reserve(aOutput.size()+1000);

#if defined USE_BACKTRACE_SYMBOLS || defined USE_BACKTRACE_DLADDR
  {
    void* callstack[nMaxFrames] = {};
    const int nFrames = backtrace(callstack, nMaxFrames);

#ifdef USE_BACKTRACE_SYMBOLS
    char** symbols = backtrace_symbols(callstack, nFrames);
    {
      aOutput << "--- BACKTRACE SYMBOLS STACKTRACE ---\n";
      for (int i = skip+1; i < nFrames; ++i) {
        _StackCatEntry(
          aOutput, i-skip,
          nullptr, 0,
          symbols[i],
          nullptr, 0,
          STACK_INCLUDE_CODE_SNIPPETS);
      }
      if (nFrames == nMaxFrames) {
        aOutput << "[stack truncated]\n";
      }
    }
#endif

#ifdef USE_BACKTRACE_DLADDR
    aOutput << "--- BACKTRACE DLARR STACKTRACE ---\n";
    {
      for (int i = skip+1; i < nFrames; ++i) {
        Dl_info info;
        if (dladdr(callstack[i], &info)) {
          char *demangled = NULL;
          int status;
          demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
          _StackCatEntry(
            aOutput, i-skip,
            nullptr, 0,
            status == 0 ? demangled : info.dli_sname,
            nullptr, (tIntPtr)callstack[i],
            STACK_INCLUDE_CODE_SNIPPETS);
          free(demangled);
        }
        else {
          aOutput.CatFormat("%d: unknown\n", i);
        }
      }
      if (nFrames == nMaxFrames) {
        aOutput << "[stack truncated]\n";
      }
    }
#endif
  }
#endif

#ifdef USE_BOOST_STACKTRACE
  {
    aOutput << "--- BOOST STACKTRACE ---\n";
    boost::stacktrace::stacktrace st(0,nMaxFrames);
    for (int i = skip+1; i < st.size(); ++i) {
      auto& frame = st[i];
      _StackCatEntry(
        aOutput, i-skip,
        frame.source_file().c_str(), frame.source_line(),
        frame.name().c_str(),
        nullptr, (tIntPtr)frame.address(),
        STACK_INCLUDE_CODE_SNIPPETS);
    }
    if (st.size() == nMaxFrames) {
      aOutput << "[stack truncated]\n";
    }
  }
#endif

#ifdef USE_BACKWARD
  {
    using namespace backward;
#if 0
    StackTrace st; st.load_here(nMaxFrames);
    Printer p; p.print(st);
#else
    aOutput << "--- BACKWARD-CPP STACKTRACE ---\n";
    backward::StackTrace st;
    st.load_here(nMaxFrames);

    backward::TraceResolver resolver;
    resolver.load_stacktrace(st);

    for (int i = skip + 1; i < st.size(); ++i) {
        backward::ResolvedTrace trace = resolver.resolve(st[i]);
        _StackCatEntry(
          aOutput, i-skip,
          trace.source.filename.c_str(), trace.source.line,
          trace.object_function.c_str(),
          nullptr, (tIntPtr)trace.addr,
          STACK_INCLUDE_CODE_SNIPPETS);
    }

    if (st.size() == nMaxFrames) {
        aOutput << "[stack truncated]\n";
    }
#endif
  }
#endif

  return aOutput;
}

}
#endif // niOSX || niIOS || niLinux
