#include "API/niLang/Types.h"

#if defined niLinuxDesktop || defined niQNX

#include "API/niLang/IOSProcess.h"
#include "API/niLang/Utils/JNIUtils.h"
#include "API/niLang/Utils/CrashReport.h"

#include "Lang.h"
#include "FileFd.h"

using namespace ni;

//--------------------------------------------------------------------------------------------
//
//  Platform implementation
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iOSProcessManager* cLang::GetProcessManager() const {
#ifdef niNoProcess
  return NULL;
#else
  return ni::GetOSProcessManager();
#endif
}

///////////////////////////////////////////////
void cLang::_PlatformExit(tU32 aulErrorCode) {
  exit(aulErrorCode);
}
void cLang::FatalError(const achar* aszMsg) {
  niPrintln(niFmt("FATAL ERROR:\n%s\n",aszMsg));
  this->Exit(0xDEADBEEF);
}

///////////////////////////////////////////////
void cLang::SetEnv(const achar* aaszEnv, const achar* aaszValue) const {
  cString envStr;
  envStr = aaszEnv;
  envStr += _A("=");
  envStr += aaszValue;
  aputenv(envStr.Chars());
}
cString cLang::GetEnv(const achar* aaszEnv) const {
  return agetenv(aaszEnv);
}

///////////////////////////////////////////////
cString _GetCommandLine() {

  //
  // /proc/[number]/cmdline (http://linux.die.net/man/5/proc)
  //
  // This holds the complete command line for the process, unless
  // the process is a zombie.  In the latter case, there is nothing
  // in this file: that is, a read on this file will return 0
  // characters.  The command-line arguments appear in this file as
  // a set of null- separated strings, with a further null byte
  // ('\0') after the last string.
  //

  cString procCmdLine = niFmt(_A("/proc/%d/cmdline"),getpid());
  int fd = open(procCmdLine.Chars(),O_RDONLY);
  if (fd >= 0) {
    char cl[256];
    int ret;
    cString r;
    do {
      ret = read(fd,cl,sizeof(cl)-1);
      if (ret) {
        // We'll have an extra space at the end because the file includes
        // an extra zero. We need to do this to make sure that if the cmd
        // line is longer than the cl buffer size we dont happen to be just
        // on a "separator" \0 that would cause it to be ignored and thus
        // handled incorrectly.
        niLoop(i,ret) {
          if (cl[i] == 0)
            cl[i] = ' ';
        }
        cl[ret] = 0;
        r << cl;
      }
    } while (ret);
    close(fd);
    return r;
  }
  return AZEROSTR;
}

niExportFunc(achar*) ni_get_exe_path(achar* buffer) {
  *buffer = '\0';
  cString procCmdLine = niFmt(_A("/proc/%d/cmdline"),getpid());
  int fd = open(procCmdLine.Chars(),O_RDONLY);
  if (fd >= 0) {
    char cl[AMAX_PATH];
    int ret = read(fd,cl,sizeof(cl)-1);
    if (ret) {
      cl[ret] = 0;
      cString r = cl;
      StrCpy(buffer,r.Chars());
    }
  }
  return buffer;
}

///////////////////////////////////////////////
niExportFunc(int) ni_debug_assert(
    int expression,     // The assert occur if this is eFalse
    const char* exp,  // Expression
    int line,     // Line number where the assert occur
    const char* file, // File where the assert occur
    int *alwaysignore,  // Pointer to the alwaysignore flag
    const char* desc) // Description of the assert
{
  if(alwaysignore && *alwaysignore == 1)
    return 0;

  if (!expression) {
    if (ni_debug_get_print_asserts()) {
      cString fmt;
      fmt.Format(_A("%s:%d: ASSERT: %s%s%s%s"),
                 file, line,
                 exp,
                 desc?_A(": "):_A(""),
                 desc?desc:_A(""),
                 desc?(desc[StrSize(desc)-1]=='\n'?_A(""):_A("\n")):_A(""));
      fmt.CatFormat("\nSTACK:\n");
      ni_stack_get_current(fmt,NULL);
      niPrintln(fmt.Chars());
      // always ignore after first print...
      if (alwaysignore) *alwaysignore = 1;
    }
  }

  return 0;
}

#endif
