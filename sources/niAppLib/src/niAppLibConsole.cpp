#include "API/niAppLibConsole.h"

#ifdef niJSCC
#include <emscripten.h>
#endif

namespace app {

void AppNotifyHost(const char* aCmd) {
#ifdef niJSCC
  emscripten_run_script(niFmt("try { NIAPP.NotifyHost('%s'); } catch(e) {}",aCmd));
#endif
}

#ifdef niJSCC

static ni::cString _runAppConsoleName;
static ni::Ptr<ni::iRunnable> _runAppConsoleShutdown;
static ni::Ptr<ni::iRunnable> _runAppConsoleLoop;

static void _Emscripten_Loop() {
  ni::Var r = _runAppConsoleLoop->Run();
  if (!r.GetBoolValue(ni::eTrue)) {
    if (_runAppConsoleShutdown.IsOK()) {
      _runAppConsoleShutdown->Run();
    }
    niLog(Info, niFmt("App '%s' finished.", _runAppConsoleName));
    _runAppConsoleLoop = NULL;
    _runAppConsoleShutdown = NULL;
    _runAppConsoleName = AZEROSTR;
    emscripten_cancel_main_loop();
  }
}

void AppConsoleLoop(const ni::achar* aAppName,
                    ni::Ptr<ni::iRunnable> aLoop,
                    ni::Ptr<ni::iRunnable> aTestEnd) {
  niPanicAssert(aLoop.IsOK());
  _runAppConsoleLoop = aLoop;
  _runAppConsoleShutdown = aTestEnd;
  _runAppConsoleName = aAppName;
  emscripten_set_main_loop(_Emscripten_Loop,30,1);
}

#else

void AppConsoleLoop(const ni::achar* aAppName,
                    ni::Ptr<ni::iRunnable> aLoop,
                    ni::Ptr<ni::iRunnable> aShutdown) {
  niPanicAssert(aLoop.IsOK());
  while (1) {
      ni::Var r = aLoop->Run();
      if (!r.GetBoolValue(ni::eTrue)) {
        break;
      }
  }
  if (aShutdown.IsOK()) {
    aShutdown->Run();
  }
  niLog(Info, niFmt("App '%s' finished.", aAppName));
}

#endif

} // app
