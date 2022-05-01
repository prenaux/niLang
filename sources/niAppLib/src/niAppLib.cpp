#include "API/niAppLib.h"
#include <niLang_ModuleDef.h>

#ifdef niJSCC
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#define niProfiler
#include <niLang/Utils/ProfGather.h>
#include <niLang/Utils/CmdLine.h>

#define APPLIB_DEFAULT_WINDOW_TITLE (niIsStringOK(aaszTitle) ? aaszTitle : "niAppLib")

namespace app {

using namespace ni;

AppConfig::AppConfig() {
  graphicsDriver = _H("Auto");
  drawFPS = 0;
  maxFPS = 0;
  fixedFPS = 0;
  swapInterval = 1;
  windowStyle = eOSWindowStyleFlags_Regular;
  windowCentered = eTrue;
  windowRect = sRecti::Null();
  windowShow = ni::GetProperty("ni.app.windowShow","true").Bool();
  backgroundUpdate = eFalse;
  backBufferFlags = eTextureFlags_Default|eTextureFlags_Default|eTextureFlags_RTAA4Samples;
  clearBuffers = eClearBuffersFlags_ColorDepthStencil;
  clearColor = 0;
  contentsScale = 0;
};

struct AppWindow : public cIUnknownImpl<iMessageHandler,eIUnknownImplFlags_Default,iWidgetSink> {
 private:
  AppWindow(AppWindow&);

 public:
  AppContext*                         mpContext;
  tU64                                mnThreadID;
  Ptr<iFont>                          mptrFontHUD;
  tU32                                mnDebugKeyMod;

  AppWindow(AppContext* apContext) : mpContext(apContext), mnThreadID(ni::ThreadGetCurrentThreadID()) {
    mpContext->_mq = ni::GetOrCreateMessageQueue(mnThreadID);
  }

  virtual tBool __stdcall OnStartup(iOSWindow* apWnd, iRunnable* apOnStarted) {
    // Init the window
    mpContext->_window = apWnd;
    niCheck(mpContext->_window.IsOK(),eFalse);
    mpContext->_window->GetMessageHandlers()->AddSink(this);
    if (!mpContext->_config.windowRect.IsNull()) {
      mpContext->_window->SetRect(mpContext->_config.windowRect);
    }
    if (mpContext->_config.windowCentered) {
      mpContext->_window->CenterWindow();
    }
    if (mpContext->_config.windowShow) {
      mpContext->_window->SetShow(eOSWindowShowFlags_Show);
      mpContext->_window->UpdateWindow(ni::eFalse);
      mpContext->_window->ActivateWindow();
    }
    ni::GetLang()->SetGlobalInstance(niGetInterfaceID(iOSWindow),mpContext->_window);

    // Initialize graphics
    mpContext->_graphics = (iGraphics*)niCreateInstance(niUI,Graphics,niVarNull,niVarNull);
    niCheck(mpContext->_graphics.IsOK(),eFalse);

    niCheck(mpContext->_graphics->InitializeDriver(mpContext->_config.graphicsDriver),eFalse);

    mpContext->_graphics->RegisterSystemFonts();

    mpContext->_graphicsContext = mpContext->_graphics->CreateContextForWindow(
      mpContext->_window,
      _A("R8G8B8A8"),_A("D24S8"),
      mpContext->_config.swapInterval,
      mpContext->_config.backBufferFlags);
    niCheck(mpContext->_graphicsContext.IsOK(),eFalse);

    // Initialize the UI context
    mpContext->_uiContext = (iUIContext*)niCreateInstance(
      niUI,UIContext,
      mpContext->_graphicsContext.ptr(),
      mpContext->GetContentsScale());
    niCheck(mpContext->_uiContext.IsOK(),eFalse);
    ni::GetLang()->SetGlobalInstance(niGetInterfaceID(iUIContext),mpContext->_uiContext);

    mpContext->_uiContext->GetRootWidget()->AddSink(this);

    if (apOnStarted) {
      if (!apOnStarted->Run().mU32) {
        niError("OnStartup failed.");
        return eFalse;
      }
    }

    return eTrue;
  }

  virtual void __stdcall Invalidate() {
    {
      Ptr<tGlobalInstanceCMap> map = ni::GetLang()->GetGlobalInstanceMap();
      map->Remove(niGetInterfaceID(iOSWindow));
      map->Remove(niGetInterfaceID(iUIContext));
      map->Remove("niUI.Graphics");
    }

    if (mpContext->_uiContext.IsOK()) {
      if (mpContext->_uiContext->GetRootWidget()) {
        mpContext->_uiContext->GetRootWidget()->Invalidate();
      }
      mpContext->_uiContext->Invalidate();
      mpContext->_uiContext = NULL;
    }
    if (mpContext->_graphicsContext.IsOK()) {
      mpContext->_graphicsContext->Invalidate();
      mpContext->_graphicsContext = NULL;
    }
    if (mpContext->_graphics.IsOK()) {
      mpContext->_graphics->Invalidate();
      mpContext->_graphics = NULL;
    }
    if (mpContext->_window.IsOK()) {
      mpContext->_window->GetMessageHandlers()->RemoveSink(this);
      mpContext->_window->Invalidate();
      mpContext->_window = NULL;
    }

    if (mpContext->_mq.IsOK()) {
      mpContext->_mq->Invalidate();
      mpContext->_mq = NULL;
    }
  }

  void _UpdateGameCtrls(iUIContext* apUIContext) {
    niLoop(i, ni::GetLang()->GetNumGameCtrls()) {
      iGameCtrl* ctrl = ni::GetLang()->GetGameCtrl(i);
      if (ctrl && ctrl->Update()) {
        apUIContext->InputGameCtrl(ctrl);
      }
    }
  }

  virtual tBool __stdcall OnUpdate() {
    niProf(AppLib_OnUpdate);

    // update the window... dispatch queue input events, etc...
    mpContext->_window->UpdateWindow(mpContext->_config.backgroundUpdate ? false : true);
    _UpdateGameCtrls(mpContext->_uiContext);

    // update the system message queue
    sMessageDesc msg;
    while (mpContext->_mq->Poll(&msg)) {
      // dispatch
      msg.mptrHandler->HandleMessage(msg.mnMsg, msg.mvarA, msg.mvarB);
      if (msg.mnMsg == eOSWindowMessage_Paint)
        break;
    }

    // Return true if we should continue or false if the application should terminate
    return !mpContext->_window->GetRequestedClose();
  }

  void __stdcall _Paint() {
    niProf(AppLib_Paint);
    if ((!mpContext->_uiContext.IsOK()) || (!mpContext->_graphicsContext.IsOK()))
      return;

    // service all scripting hosts
    ni::GetLang()->ServiceAllScriptingHosts(false);

    // update the main executor
    ni::GetConcurrent()->GetExecutorMain()->Update(10);

    // update the console
    ni::GetConsole()->PopAndRunAllCommands();

    // Update the UI Context if the window is active
    if (mpContext->_config.backgroundUpdate || mpContext->_window->GetIsActive()) {
      mpContext->_uiContext->Resize(
        Rectf(0,0,
              mpContext->_window->GetClientSize().x,
              mpContext->_window->GetClientSize().y),
        mpContext->GetContentsScale());
      mpContext->_uiContext->Update(ni::GetLang()->GetFrameTime());
    }

    if (mpContext->_config.clearBuffers) {
      mpContext->_graphicsContext->ClearBuffers(
        mpContext->_config.clearBuffers, mpContext->_config.clearColor, 1.0f, 0);
    }

    // Draw the UI Context
    mpContext->_uiContext->Draw();
#if defined niWindows || defined niOSX
    mpContext->_uiContext->DrawCursor(mpContext->_window.ptr());
#endif

    // Display the context
    mpContext->_graphicsContext->Display(0, sRecti::Null());

    // Update the frametime & profiler
    {
      if (mpContext->_config.fixedFPS > 0) {
        static tF64 _fFixedElapsedTime = 0.0;
        ni::GetLang()->UpdateFrameTime(_fFixedElapsedTime);
        _fFixedElapsedTime += ni::FDiv(1.0, (tF64)mpContext->_config.fixedFPS);
      }
      else {
        ni::GetLang()->UpdateFrameTime(ni::TimerInSeconds());
      }
      ni::GetProf()->Update();
    }
  }

  virtual tBool __stdcall OnWindowMessage(const tU32 anMsg, const Var& avarA, const Var& avarB) {
    switch (anMsg) {
      case eOSWindowMessage_Paint: {
        this->_Paint();
        break;
      }
      case eOSWindowMessage_KeyUp: {
        switch (avarA.mU32) {
          case eKey_LControl:
            niFlagOff(mnDebugKeyMod,eKeyMod_Control);
            break;
          case eKey_LAlt:
            niFlagOff(mnDebugKeyMod,eKeyMod_Alt);
            break;
          case eKey_LShift:
            niFlagOff(mnDebugKeyMod,eKeyMod_Shift);
            break;
        }
        break;
      }
      case eOSWindowMessage_KeyDown:
        switch (avarA.mU32) {
          case eKey_LControl:
            niFlagOn(mnDebugKeyMod,eKeyMod_Control);
            break;
          case eKey_LAlt:
            niFlagOn(mnDebugKeyMod,eKeyMod_Alt);
            break;
          case eKey_LShift:
            niFlagOn(mnDebugKeyMod,eKeyMod_Shift);
            break;
          default: {
            switch (avarA.mU32 | mnDebugKeyMod) {
              // move cursor up: C+S+M+Up
              case eKey_Up|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->InputMoveCursor(-1);
                return ni::eTrue;
              }
              // move cursor down: C+S+M+Down
              case eKey_Down|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->InputMoveCursor(1);
                return ni::eTrue;
              }
              // move history left: C+S+M+Left
              case eKey_Left|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->InputMoveFrame(-1);
                return ni::eTrue;
              }
              // move history right: C+S+M+Right
              case eKey_Right|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->InputMoveFrame(1);
                return ni::eTrue;
              }
              // pause/unpause: C+S+M+P
              case eKey_P|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->SetRecord(!ni::GetProf()->GetRecord());
                return ni::eTrue;
              }
              // select: C+S+M+Enter
              case eKey_Enter|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->InputSelect();
                return ni::eTrue;
              }
              // select parent: C+S+M+Backspace
              case eKey_BackSpace|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->InputSelectParent();
                return ni::eTrue;
              }
              // next report mode: C+S+M+R
              case eKey_R|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                ni::GetProf()->SetReportMode(
                  (eProfilerReportMode)((((ni::tU32)ni::GetProf()->GetReportMode())+1) % eProfilerReportMode_Last));
                return ni::eTrue;
              }
              // max fps down: C+S+M+Minus
              case eKey_Minus|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                if (mpContext->_config.maxFPS > 10) {
                  mpContext->_config.maxFPS /= 2;
                }
                return ni::eTrue;
              }
              // max fps up: C+S+M+Plus
              case eKey_Equals|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                if (mpContext->_config.maxFPS <= 0) {
                  mpContext->_config.maxFPS = 60;
                }
                else if (mpContext->_config.maxFPS < 240) {
                  mpContext->_config.maxFPS *= 2;
                }
                return ni::eTrue;
              }
              // toggle fps mode: C+S+M+Period
              case eKey_Period|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                mpContext->_config.drawFPS = (mpContext->_config.drawFPS + 1) % 4;
                return ni::eTrue;
              }
            }
          }
        }
        break;
    }
    return eFalse;
  }

  virtual tBool __stdcall OnRootMessage(iWidget* w, tU32 msg, const Var& a, const Var& b) {
    return eFalse;
  }

  virtual tU64 __stdcall GetThreadID() const niOverride {
    return mnThreadID;
  }

  virtual tBool __stdcall OnWidgetSink(iWidget* w, tU32 msg, const Var& a, const Var& b) niOverride
  {
    iWidget* pRootWidget = mpContext->_uiContext->GetRootWidget();
    if (w == pRootWidget) {
      tBool ret = this->OnRootMessage(w, msg, a, b);
      if (!ret) {
        if (msg == eUIMessage_ContextAfterDraw) {
          if (mpContext->_config.drawFPS) {
            const sRectf widgetRect = pRootWidget->GetAbsoluteRect();
            if (!mptrFontHUD.IsOK()) {
              mptrFontHUD = pRootWidget->GetFont()->CreateFontInstance(NULL);
              mptrFontHUD->SetSizeAndResolution(sVec2f::Zero(), 16, w->GetUIContext()->GetContentsScale());
            }
            Ptr<iFont> font = mptrFontHUD;
            Ptr<iCanvas> canvas = VarQueryInterface<iCanvas>(a);

            if (mpContext->_config.drawFPS >= 2) {
              Ptr<iProfDraw> drawer = mpContext->_uiContext->CreateProfDraw(canvas,font);
              ni::GetProf()->DrawGraph(
                drawer,
                5, widgetRect.GetHeight(),
                2, 10);
              if (mpContext->_config.drawFPS >= 3) {
                ni::GetProf()->DrawTable(
                  drawer,
                  5, 5,
                  widgetRect.GetWidth()/3, widgetRect.GetHeight(),
                  3);
              }
            }

            if (canvas.IsOK()) {
              static const tU32 textColor = ULColorBuild(240,240,240,255);
              static const tU32 bgColor = ULColorBuild(15,15,15,180);

              font->SetColor(textColor);
              const float fh = font->GetHeight();
              sVec4i memStats;
              ni_mem_get_stats(&memStats);
              const cString fpsText = niFmt(
                "fps: %g (ms: %d) liveAlloc: %d (objs: %d) ren: %s",
                ni::GetLang()->GetAverageFrameRate(),
                (ni::GetLang()->GetFrameTime() * 1000.0),
                memStats.x - memStats.y,
                memStats.z - memStats.w,
                canvas->GetGraphicsContext()->GetDriver()->GetName()
              );

              canvas->BlitFillAlpha(Vec4f(widgetRect.GetLeft(), widgetRect.GetBottom() - fh - 10,
                                          widgetRect.GetRight(), widgetRect.GetBottom()),
                                    bgColor);

              const sRectf textSize = canvas->BlitText(
                font,
                Vec4f(5, widgetRect.GetHeight() - fh - 5, widgetRect.GetWidth()-5, widgetRect.GetHeight() - 5),
                0,
                fpsText.Chars());

              const cString buildText = ni::GetLang()->GetProperty("ni.app.build_text");
              if (!buildText.empty() &&
                  ((textSize.GetRight()+buildText.length()*font->GetMaxCharWidth()) < widgetRect.GetWidth()))
              {
                canvas->BlitText(
                  font,
                  Vec4f(5, widgetRect.GetHeight() - fh - 5, widgetRect.GetWidth()-5, widgetRect.GetHeight() - 5),
                  eFontFormatFlags_Right,
                  buildText.Chars());
              }
              canvas->Flush();
            }
          }
        }
      }

      return ret;
    }

    return eFalse;
  }

  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& avarA, const Var& avarB)  niOverride {
    if (this->OnWindowMessage(anMsg,avarA,avarB))
      return;

    eOSWindowMessage msgId = (eOSWindowMessage)anMsg;
    if (msgId == eOSWindowMessage_Close) {
      mpContext->_window->SetRequestedClose(true);
      return;
    }
    else if (msgId == eOSWindowMessage_SwitchIn) {
      mpContext->_window->SetRefreshTimer(0);
    }
    else if (msgId == eOSWindowMessage_SwitchOut) {
      mpContext->_window->SetRefreshTimer(-1);
    }

    if (mpContext->_uiContext.IsOK()) {
      mpContext->_uiContext->SendWindowMessage(msgId, avarA, avarB);
    }
  }

};

//========================================================================
//
// Main
//
//========================================================================

static int _MainEnd(AppContext* apContext) {
  ni::SafeInvalidate(apContext->_uiContext.ptr());
  apContext->_uiContext = NULL;
  ni::SafeInvalidate(apContext->_graphicsContext.ptr());
  apContext->_graphicsContext = NULL;
  ni::SafeInvalidate(apContext->_graphics.ptr());
  apContext->_graphics = NULL;
  ni::SafeInvalidate(apContext->_window.ptr());
  apContext->_window = NULL;
  ni::SafeInvalidate(apContext->_mq.ptr());
  apContext->_mq = NULL;
  ni::SafeInvalidate(reinterpret_cast<ni::iUnknown*>(apContext->_appWnd.ptr()));
  apContext->_appWnd = NULL;

  // We must invalidate the concurrent object at the end of main() because if
  // we let the C++ library _doexit() method call the destructor implicitly
  // it'll deadlock on Windows. The deadlock is due to the fact that for some
  // reason the C++ library delete the thread handles before calling the
  // Concurrent object's destructor so the Windows Events wait forever.
  ni::GetConcurrent()->Invalidate();
  // And this makes sure we use a safe exit method, avoids crashes on exit...
  ni::GetLang()->Exit(0);
  return 0;
}

void AppNotifyHost(const char* aCmd) {
#ifdef niJSCC
  emscripten_run_script(niFmt("try { niApp.NotifyHost('%s'); } catch(e) {}",aCmd));
#endif
}

tBool AppUpdate(AppContext* apContext) {
  if (apContext->_appWnd.IsOK()) {
    return ((AppWindow*)apContext->_appWnd.ptr())->OnUpdate();
  }
  return eFalse;
}

void AppRender(AppContext* apContext) {
  if (apContext->_appWnd.IsOK()) {
    apContext->_window->RedrawWindow();
    ++apContext->_appRenderCount;
    if (apContext->_appRenderCount == 1) {
      AppNotifyHost("FirstRenderAppLib");
    }
  }
}

tBool AppStartup(AppContext* apContext, iOSWindow* apWindow, iRunnable* apOnStarted, iRunnable* apOnShutdown) {
  niCheckIsOK(apWindow,eFalse);
  AppNotifyHost("BeforeStartup");

  if (ni::GetLang()->HasProperty("maxFPS")) {
    apContext->_config.maxFPS = ni::GetLang()->GetProperty("maxFPS").Long();
  }
  if (ni::GetLang()->HasProperty("fixedFPS")) {
    apContext->_config.fixedFPS = ni::GetLang()->GetProperty("fixedFPS").Long();
  }
  if (ni::GetLang()->HasProperty("drawFPS")) {
    apContext->_config.drawFPS = ni::GetLang()->GetProperty("drawFPS").Long();
  }
  else {
    ni::GetLang()->SetProperty("drawFPS",niFmt("%d",apContext->_config.drawFPS));
  }
  if (ni::GetLang()->HasProperty("renderer")) {
    apContext->_config.graphicsDriver = _H(ni::GetLang()->GetProperty("renderer"));
  }
  if (ni::GetLang()->HasProperty("swapinterval")) {
    apContext->_config.swapInterval = ni::GetLang()->GetProperty("swapinterval").Long();
  }
  if (ni::GetLang()->HasProperty("noaa")) {
    const ni::tBool bNoAA = ni::GetLang()->GetProperty("noaa").Bool(ni::eTrue);
    if (bNoAA) {
      apContext->_config.backBufferFlags &= ~eTextureFlags_RTAA4Samples;
    }
  }
  if (ni::GetLang()->HasProperty("windowRect")) {
    apContext->_config.windowCentered = ni::GetLang()->GetProperty("windowCentered").Bool(eFalse);
    apContext->_config.windowRect = ni::sRectf(ni::GetLang()->Eval(
      ni::GetLang()->GetProperty("windowRect").Chars())->GetVec4()).ToInt();
  }

  apContext->_appRenderCount = 0;
  apContext->_appOnShutdown = apOnShutdown;

  apContext->_appWnd = niNew AppWindow(apContext);
  if (!((AppWindow*)apContext->_appWnd.ptr())->OnStartup(apWindow,apOnStarted)) {
    niError("Can't start the application window.");
    return eFalse;
  }

  AppNotifyHost("Started");
  return eTrue;
}

void AppShutdown(AppContext* apContext) {
  AppNotifyHost("BeforeShutdown");
  if (apContext->_appOnShutdown.IsOK()) {
    apContext->_appOnShutdown->Run();
    apContext->_appOnShutdown = NULL;
  }
  if (apContext->_appWnd.IsOK()) {
    apContext->_appWnd->Invalidate();
    apContext->_appWnd = NULL;
  }
  AppNotifyHost("Shutdowned");
}

tBool AppIsStarted(AppContext* apContext) {
  return apContext->_appWnd.IsOK();
}

void AppGenericResize(AppContext* apContext, ni::tI32 w, ni::tI32 h) {
  if (apContext->_window.IsOK()) {
    apContext->_window->SetRect(sRecti(0,0,w,h));
  }
}

void AppGenericKey(AppContext* apContext, eKey aKey, tBool abIsDown)
{
  if (!apContext->_appWnd.IsOK())
    return;

  QPtr<iOSWindowGeneric> gw = apContext->_window.ptr();
  if (gw.IsOK()) {
    gw->GenericSendMessage(
      abIsDown ? eOSWindowMessage_KeyDown : eOSWindowMessage_KeyUp,
      aKey, niVarNull);
  }
}

void AppGenericInputString(AppContext* apContext, const ni::achar* aaszString)
{
  if (!apContext->_appWnd.IsOK())
    return;

  QPtr<iOSWindowGeneric> gw = apContext->_window.ptr();
  if (gw.IsOK()) {
    gw->GenericInputString(aaszString);
  }
}

void AppGenericFingerMove(AppContext* apContext, tI32 fingerId, tF32 x, tF32 y, tF32 pressure) {
  if (!apContext->_appWnd.IsOK())
    return;

  QPtr<iOSWindowGeneric> gw = apContext->_window.ptr();
  if (gw.IsOK()) {
    gw->GenericSendMessage(eOSWindowMessage_FingerMove,fingerId,Vec3f(x,y,pressure));
  }
}

void AppGenericFingerRelativeMove(AppContext* apContext, tI32 fingerId, tF32 rx, tF32 ry, tF32 pressure) {
  if (!apContext->_appWnd.IsOK())
    return;

  QPtr<iOSWindowGeneric> gw = apContext->_window.ptr();
  if (gw.IsOK()) {
    gw->GenericSendMessage(eOSWindowMessage_FingerRelativeMove,fingerId,Vec3f(rx,ry,pressure));
  }
}

void AppGenericFingerPress(AppContext* apContext, tI32 fingerId, tBool isDown, tF32 x, tF32 y, tF32 pressure) {
  if (!apContext->_appWnd.IsOK())
    return;

  QPtr<iOSWindowGeneric> gw = apContext->_window.ptr();
  if (gw.IsOK()) {
    gw->GenericSendMessage(
      isDown ? eOSWindowMessage_FingerDown : eOSWindowMessage_FingerUp,
      fingerId,
      Vec3f(x,y,pressure));
  }
}

void AppGenericMouseWheel(AppContext* apContext, tF32 delta) {
  if (!apContext->_appWnd.IsOK())
    return;

  QPtr<iOSWindowGeneric> gw = apContext->_window.ptr();
  if (gw.IsOK()) {
    gw->GenericSendMessage(eOSWindowMessage_MouseWheel,delta,niVarNull);
  }
}

void AppGenericPinch(AppContext* apContext, tF32 scale, const eGestureState aState) {
  if (!apContext->_appWnd.IsOK())
    return;

  QPtr<iOSWindowGeneric> gw = apContext->_window.ptr();
  if (gw.IsOK()) {
    gw->GenericSendMessage(eOSWindowMessage_Pinch,scale,(tU32)aState);
  }
}

tBool AppGenericStartup(AppContext* apContext,
                        const achar* aaszTitle, ni::tI32 anWidth, ni::tI32 anHeight,
                        iRunnable* apOnStarted, ni::iRunnable* apOnShutdown)
{
  Ptr<iOSWindow> ptrWindow = ni::CreateGenericWindow();
  if (!ptrWindow.IsOK()) {
    niError("Can't create window.");
    return false;
  }
  ptrWindow->SetTitle(APPLIB_DEFAULT_WINDOW_TITLE);
  ptrWindow->SetRect(sRecti(0,0,anWidth,anHeight));
  return app::AppStartup(apContext, ptrWindow, apOnStarted, apOnShutdown);
}

} // app

#include <niLang/Utils/TimerSleep.h>

namespace app {

tBool AppNativeStartup(AppContext* apContext,
                       const achar* aaszTitle, ni::tI32 anWidth, ni::tI32 anHeight,
                       iRunnable* apOnStarted, ni::iRunnable* apOnShutdown) {
  //// Create the window ////
  sRecti r = ni::GetLang()->GetMonitorRect(0);
  if (anWidth > 0 && anHeight > 0) {
    r.SetWidth(anWidth);
    r.SetHeight(anHeight);
  }
  else {
    r.SetWidth((int)(r.GetWidth() * 0.75));
    r.SetHeight((int)(r.GetHeight() * 0.75));
  }
  if ((r.GetWidth() < 10) || (r.GetHeight() < 10)) {
    r.Set(0, 0, 1024, 768);
  }

  Ptr<iOSWindow> ptrWindow = ni::GetLang()->CreateWindow(
    NULL,
    APPLIB_DEFAULT_WINDOW_TITLE,
    r,
    0,
    apContext->_config.windowStyle);
  if (!ptrWindow.IsOK()) {
    niError("Can't create window.");
    return eFalse;
  }

  return app::AppStartup(apContext, ptrWindow, apOnStarted, apOnShutdown);
}

int AppNativeMainLoop(AppContext* apContext) {
  if (!app::AppIsStarted(apContext)) {
    ni::GetLang()->FatalError("Application not started.");
    return -1;
  }

#ifdef niJSCC
  static std::function<void()> _fnJSCCLoop;
  struct _JSCCLoop {
    static void Loop() {
      _fnJSCCLoop();
    };
  };

  _fnJSCCLoop = [apContext] { app::AppUpdate(apContext); };
  emscripten_set_main_loop(_JSCCLoop::Loop,
                           // apContext->_config.maxFPS ? apContext->_config.maxFPS : 60,
                           0,
                           true);
#else
  for (;;) {
    const tF64 frameStartTime = ni::TimerInSeconds();
    if (!app::AppUpdate(apContext))
      break;
    const tF64 frameTime = ni::TimerInSeconds() - frameStartTime;
    if (apContext->_config.maxFPS != 0) {
      const tF64 minFrameTime = (1.0/(tF64)apContext->_config.maxFPS);
      if (frameTime < minFrameTime) {
        ni::SleepMs((tU32)((minFrameTime - frameTime) * 1000.0));
      }
    }
  }
#endif

  app::AppShutdown(apContext);
  return _MainEnd(apContext);
}

} // app
