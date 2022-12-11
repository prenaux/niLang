#include <niAppLib.h>
#include "API/niUnitTest.h"

namespace UnitTest {

using namespace ni;

static Nonnull<app::AppContext> gAppContext = ni::MakePtrNonnull<app::AppContext>();
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
        gAppContext->_window->SetRequestedClose(eTrue);
        gFinishedTests = true;
      }
    }

    return eFalse;
  }
};

static WeakPtr<ni::iWidgetSink> _currentTestWidgetSink;
void TestAppSetCurrentTestWidgetSink(iWidgetSink* apSink) {
  QPtr<ni::iWidgetSink> currentSink = _currentTestWidgetSink;
  if (currentSink.IsOK()) {
    _currentTestWidgetSink.SetNull();
    Ptr<iWidget> rootWidget = gAppContext->_uiContext->GetRootWidget();
    rootWidget->InvalidateChildren(); // remove all children that might have been added by the test case
    rootWidget->RemoveSink(currentSink);
  }
  currentSink = niGetIfOK(apSink);
  if (currentSink.IsOK()) {
    _currentTestWidgetSink = currentSink;
    Ptr<iWidget> rootWidget = gAppContext->_uiContext->GetRootWidget();
    rootWidget->AddSink(currentSink);
    rootWidget->SetStyle(eWidgetStyle_HoldFocus);
    if (!rootWidget->HasChild(gAppContext->_uiContext->GetFocusedWidget(),eTrue))
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
  gAppContext->_uiContext->GetRootWidget()->AddSink(sink);
  return ni::eTrue;
}

static ni::Var OnAppShutdown() {
  QPtr<UnitTestWidgetSink> sink = _unitTestWidgetSink;
  if (sink.IsOK()) {
    gAppContext->_uiContext->GetRootWidget()->RemoveSink(sink);
  }
  _unitTestWidgetSink.SetNull();
  return ni::eTrue;
}

int TestAppNativeMainLoop(const char* aTitle, const char* aFixtureName) {
  gAppContext->_config.drawFPS = 2;
  UnitTest::TestRunner_Startup(aFixtureName);

  if (!app::AppNativeStartup(gAppContext.non_null(),
                             aTitle, 0, 0,
                             ni::Runnable<ni::tpfnRunnable>(OnAppStarted),
                             ni::Runnable<ni::tpfnRunnable>(OnAppShutdown)))
  {
    ni::GetLang()->FatalError("Can't start application.");
    return -1;
  }

  app::AppNativeMainLoop(gAppContext.non_null());
  return gTestResults;
}

int TestAppNativeMainLoop(const char* aTitle, int argc, const char** argv) {
  gAppContext->_config.drawFPS = 2;

  cString fixtureName;
  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
  return TestAppNativeMainLoop(aTitle, fixtureName.Chars());
}

}
