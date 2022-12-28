#include <niAppLib.h>
#include "API/niUnitTest.h"

namespace UnitTest {

using namespace ni;

astl::non_null<app::AppContext*> GetTestAppContext() {
  static Nonnull<app::AppContext> _appContext = ni::MakeNonnull<app::AppContext>();
  return _appContext;
}

static int gTestResults = -1;
static bool gFinishedTests = false;

struct UnitTestWidgetSink : public cWidgetSinkImpl<>
{
  tBool __stdcall OnSinkAttached() niImpl {
    niLog(Info, "UnitTestWidgetSink::OnSinkAttached");
    return eTrue;
  };

  tBool __stdcall OnSinkDetached() niImpl {
    niLog(Info, "UnitTestWidgetSink::OnSinkDetached");
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    if (!gFinishedTests) {
      if (!UnitTest::TestRunner_RunNext()) {
        gTestResults = UnitTest::TestRunner_ReportSummary();
        UnitTest::TestRunner_Shutdown();
        GetTestAppContext()->_window->SetRequestedClose(eTrue);
        gFinishedTests = true;
      }
    }

    return eFalse;
  }
};

static WeakPtr<ni::iWidgetSink> _currentTestWidgetSink;
void TestAppSetCurrentTestWidgetSink(iWidgetSink* apSink, ni::tBool abInteractive) {
  astl::non_null<app::AppContext*> appContext = GetTestAppContext();
  QPtr<ni::iWidgetSink> currentSink = _currentTestWidgetSink;
  if (currentSink.IsOK()) {
    _currentTestWidgetSink.SetNull();
    Ptr<iWidget> rootWidget = appContext->_uiContext->GetRootWidget();
    rootWidget->InvalidateChildren(); // remove all children that might have been added by the test case
    rootWidget->RemoveSink(currentSink);
  }
  else if (abInteractive && !GetTestAppContext()->_config.windowShow) {
    // No sink set so this is the first run, show the window
    if (ni::GetProperty("ni.app.windowMaximized","false").Bool()) {
      GetTestAppContext()->_window->SetShow(eOSWindowShowFlags_Show|eOSWindowShowFlags_Maximize);
    }
    else {
      GetTestAppContext()->_window->SetShow(eOSWindowShowFlags_Show);
    }
    GetTestAppContext()->_window->ActivateWindow();
  }
  currentSink = niGetIfOK(apSink);
  if (currentSink.IsOK()) {
    _currentTestWidgetSink = currentSink;
    Ptr<iWidget> rootWidget = appContext->_uiContext->GetRootWidget();
    rootWidget->AddSink(currentSink);
    rootWidget->SetStyle(eWidgetStyle_HoldFocus);
    if (!rootWidget->HasChild(appContext->_uiContext->GetFocusedWidget(),eTrue))
      rootWidget->SetFocus();
    ni::GetLang()->SetProperty(
        "ni.app.build_text",
        niFmt("TEST: %s, BUILD: %s - %s", UnitTest::TestRunner_GetCurrentTestName(), __DATE__, __TIME__));

  }
}

static WeakPtr<UnitTestWidgetSink> _unitTestWidgetSink;
static ni::Var OnAppStarted() {
  Ptr<UnitTestWidgetSink> sink = niNew UnitTestWidgetSink();
  _unitTestWidgetSink = sink;
  GetTestAppContext()->_uiContext->GetRootWidget()->AddSink(sink);
  return ni::eTrue;
}

static ni::Var OnAppShutdown() {
  QPtr<UnitTestWidgetSink> sink = _unitTestWidgetSink;
  if (sink.IsOK()) {
    GetTestAppContext()->_uiContext->GetRootWidget()->RemoveSink(sink);
  }
  _unitTestWidgetSink.SetNull();
  return ni::eTrue;
}

int TestAppNativeMainLoop(const char* aTitle, const char* aFixtureName) {
  GetTestAppContext()->_config.drawFPS = 2;
  GetTestAppContext()->_config.backgroundUpdate = eTrue;
  // Refresh as fast as possible. Without this we'll have a sleep on some
  // platforms to save energy since the window is considered to be inactive.
  GetTestAppContext()->_config.windowRefreshTimer = 0.0f;
  // By we dont display the app window, use GetTestAppContext()->ShowWindow()
  // to show it. Note that by default the TEST_WIDGET macros will show the
  // window when a test is run interactively.
  GetTestAppContext()->_config.windowShow = eFalse;
  UnitTest::TestRunner_Startup(aFixtureName);

  if (!app::AppNativeStartup(GetTestAppContext(),
                             aTitle, 0, 0,
                             ni::Runnable<ni::tpfnRunnable>(OnAppStarted),
                             ni::Runnable<ni::tpfnRunnable>(OnAppShutdown)))
  {
    ni::GetLang()->FatalError("Can't start application.");
    return -1;
  }

  app::AppNativeMainLoop(GetTestAppContext());
  return gTestResults;
}

int TestAppNativeMainLoop(const char* aTitle, int argc, const char** argv) {
  cString fixtureName;
  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
  return TestAppNativeMainLoop(aTitle, fixtureName.Chars());
}

}
