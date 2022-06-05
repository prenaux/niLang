#include <niAppLib.h>
#include <niUI/Utils/DebugUI.h>
#include <niLang/Utils/CrashReport.h>
niCrashReport_DeclareHandler();
#include <niScriptCpp/ScriptCpp.h>
#include <niScriptCpp_ModuleDef.h>

using namespace ni;

app::AppContext gAppContext;
static Ptr<iWidget> _appCanvas;
static QPtr<iWidgetSink> _appSink;

_HDecl(UIWidget);
_HDecl(ID_AppCanvas);

Ptr<iUnknown> EvalImpl(iHString* ahspContext, iHString* ahspCodeFile, const tUUID& aIID) {
  ni::Ptr<ni::iScriptingHost> host = ni::GetLang()->FindScriptingHost(
    ahspContext,ahspCodeFile);
  if (!host.IsOK()) {
    niError(niFmt("Can't find scripting host for code file '%s'.",ahspCodeFile));
    return NULL;
  }

  ni::Ptr<ni::iUnknown> ptrInst = host->EvalImpl(ahspContext,ahspCodeFile,aIID);
  if (!niIsOK(ptrInst)) {
    niError(niFmt("Can't create instance of '%s' from code file '%s'.",ahspContext,ahspCodeFile));
    return NULL;
  }

  return ptrInst;
};

void _ReloadApp() {
  if (!_appCanvas.IsOK()) {
    _appCanvas = gAppContext._uiContext->CreateWidget(
      "Canvas",
      gAppContext._uiContext->GetRootWidget(),
      Rectf(0,0,50,50),
      0, _HC(ID_AppCanvas));
    _appCanvas->SetDockStyle(eWidgetDockStyle_DockFillOverlay);
  }

  _appCanvas->RemoveSink(_appSink);
  _appSink = EvalImpl(
    _H("UIWidget"),
    _H("HelloUI_rtcpp_module#HelloUI#HelloUI/rtcpp_module/HelloUI_Widget_module.cpp"),
    niGetInterfaceUUID(iWidgetSink));
  _appCanvas->AddSink(_appSink);
}

ni::Var OnAppStarted() {
  niInitScriptVMForDebugUI();
  ScriptCpp_CleanupDLLs();
  _ReloadApp();
  gAppContext._window->GetMessageHandlers()->AddSink(ni::MessageHandler(
    [](const tU32 anMsg, const Var& avarA, const Var& avarB) {
      if (anMsg == eOSWindowMessage_KeyDown)
      {
        switch (avarA.mU32) {
          case eKey_F1: {
            niDebugFmt(("... F1"));
            gAppContext._uiContext->SetDebugDraw(
              !gAppContext._uiContext->GetDebugDraw());
            break;
          }
          case eKey_F9: {
            niDebugFmt(("... F9"));
            _ReloadApp();
            break;
          }
        }
      }
    }));
  return ni::eTrue;
}

ni::Var OnAppShutdown() {
  ScriptCpp_CleanupDLLs();
  return ni::eTrue;
}

niConsoleMain() {
  gAppContext._config.drawFPS = ni::GetProperty("drawFPS","1").Long();
  // bg update, makes profiling/debugging a lot simpler
  gAppContext._config.backgroundUpdate = eTrue;

  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine());

  QPtr<iScriptingHost> ptrScriptingHost = niCreateInstance(ScriptingHost,Cpp,NULL,NULL);
  ni::GetLang()->AddScriptingHost(_H("cpp"),ptrScriptingHost.ptr());
  ni::GetLang()->AddScriptingHost(_H("cni"),ptrScriptingHost.ptr());
  ni::SetProperty(SCRIPTCPP_COMPILE_PROPERTY,"1");

  if (!app::AppNativeStartup(&gAppContext, "HelloUI_rtcpp", 0, 0,
                             ni::Runnable<ni::tpfnRunnable>(OnAppStarted),
                             ni::Runnable<ni::tpfnRunnable>(OnAppShutdown)))
  {
    ni::GetLang()->FatalError("Can't start application.");
    return -1;
  }

  return app::AppNativeMainLoop(&gAppContext);
}
