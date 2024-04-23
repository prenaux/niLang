#include <niAppLib.h>
#include <niUI/Utils/DebugUI.h>
#include <niLang/Utils/CrashReport.h>
#include "openxr/openxr.h"

niCrashReport_DeclareHandler();

using namespace ni;

static Nonnull<app::AppContext> gAppContext = ni::MakeNonnull<app::AppContext>();

ni::Var OnAppStarted() {
  niInitScriptVMForDebugUI();
  XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
  createInfo.next = nullptr;
  createInfo.enabledExtensionCount = 0;
  createInfo.enabledExtensionNames = nullptr;

  createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

  XrInstance instance;
  XR_FAILED(xrCreateInstance(&createInfo, &instance));
  return ni::eTrue;
}

niConsoleMain() {
  ni_log_system_info_once();

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

  if (!app::AppNativeStartup(gAppContext, "HelloVR_cpp", 0, 0,
                             ni::Runnable<ni::tpfnRunnable>(OnAppStarted),
                             NULL))
  {
    ni::GetLang()->FatalError("Can't start application.");
    return -1;
  }

  return app::AppNativeMainLoop(gAppContext);
}
