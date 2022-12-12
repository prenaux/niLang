#include <niAppLib.h>
#include <niUI/Utils/DebugUI.h>
#include <niLang/Utils/CrashReport.h>
niCrashReport_DeclareHandler();

#define HELLOUI_INCLUDE_DATA
#include "HelloUI_data.h"

using namespace ni;

niExportFunc(iWidgetSink*) New_HelloUI_Widget();
niExportFunc(iWidgetSink*) New_HelloUI_Shader();
niExportFunc(iWidgetSink*) New_HelloUI_AR();

static Nonnull<app::AppContext> gAppContext = ni::MakeNonnull<app::AppContext>();

ni::Var OnAppStarted() {
  niInitScriptVMForDebugUI();

  Ptr<iWidgetSink> sink;
  // if (!sink.IsOK()) { sink = New_HelloUI_AR(); }
  // if (!sink.IsOK()) { sink = New_HelloUI_Shader(); }
  if (!sink.IsOK()) { sink = New_HelloUI_Widget(); }
  gAppContext->_uiContext->GetRootWidget()->AddSink(sink);
  return ni::eTrue;
}

niConsoleMain() {
  // gAppContext._config.drawFPS = 2;
  // bg update, makes profiling/debugging a lot simpler
  gAppContext->_config.backgroundUpdate = eTrue;
#ifdef niJSCC
  const tBool bWebGLTranslucent = ni::GetProperty("WebGL.Translucent", "false").Bool();
  gAppContext->_config.clearColor = bWebGLTranslucent ? 0 : ULColorBuild(88,33,33,0);
#else
  gAppContext->_config.clearColor = 0xFF333333;
#endif

  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine());

  if (!app::AppNativeStartup(gAppContext, "HelloUI_cpp", 0, 0,
                             ni::Runnable<ni::tpfnRunnable>(OnAppStarted),
                             NULL))
  {
    ni::GetLang()->FatalError("Can't start application.");
    return -1;
  }

  return app::AppNativeMainLoop(gAppContext);
}
