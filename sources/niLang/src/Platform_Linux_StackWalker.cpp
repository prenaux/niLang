#include "API/niLang/Types.h"

#if defined niLinux && !defined niNoCrashReport

#include "API/niLang/StringDef.h"

using namespace ni;

#define UNW_LOCAL_ONLY
#include <cxxabi.h>
#include <libunwind.h>

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void* apExp) {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  int i = 0;

  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }

    ++i;
    aOutput.CatFormat("%d:", i);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      aOutput.CatFormat(" %s + %d\n", nameptr, offset);
      std::free(demangled);
    } else {
      aOutput << " -- error: unable to obtain symbol name for this frame\n";
    }
  }
  return aOutput;
}

#endif
