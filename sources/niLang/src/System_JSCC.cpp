#include "API/niLang/Types.h"

#ifdef niJSCC
#include "API/niLang/IOSWindow.h"
#include "API/niLang_ModuleDef.h"
#include "API/niLang/IOSWindow.h"
#include "API/niLang/IConsole.h"
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/Utils/TimerSleep.h>

#include "Lang.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>

#include <EGL/egl.h>

using namespace ni;

#define TRACE_JSCC_CAPI(X) niDebugFmt(X)
#define TRACE_JSCC_KEY(X) // niDebugFmt(X)
#define TRACE_JSCC_MOUSE_MOVE(X) // niDebugFmt(X)
#define TRACE_JSCC_MOUSE_BT(X) // niDebugFmt(X)
#define TRACE_JSCC_FINGER(X) //niDebugFmt(X)

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) { niError(niFmt("%s returned %s.\n", #x, emscripten_result_to_string(ret))); };

static struct sJSCCWindow* _jsccWindow = NULL;
static int _jsccDefaultW = 300;
static int _jsccDefaultH = 300;
static tF32 _jsccDefaultContentsScale = 1.0f;

static em_arg_callback_func _jsccMainLoopFn = nullptr;
niExportJSCC(void) niJSCC_SetMainLoopFn(em_arg_callback_func aFn, void* apArg) {
  if (_jsccMainLoopFn == aFn)
    return;
  if (_jsccMainLoopFn) {
    // there was a main loop, cancel it
    emscripten_cancel_main_loop();
  }
  _jsccMainLoopFn = aFn;
  if (_jsccMainLoopFn) {
    // set the new main loop if there's one
    emscripten_set_main_loop_arg(_jsccMainLoopFn, apArg, 0, true);
  }
}

/*

  See: https://emscripten.org/docs/api_reference/emscripten.h.html#c.emscripten_set_main_loop_arg

  The timing value to activate for the main loop. This value interpreted
  differently according to the mode parameter:

  If mode is EM_TIMING_SETTIMEOUT, then value specifies the number of milliseconds
  to wait between subsequent ticks to the main loop and updates occur independent
  of the vsync rate of the display (vsync off). This method uses the JavaScript
  setTimeout function to drive the animation.

  If mode is EM_TIMING_RAF, then updates are performed using the
  requestAnimationFrame function (with vsync enabled), and this value is
  interpreted as a “swap interval” rate for the main loop. The value of 1
  specifies the runtime that it should render at every vsync (typically 60fps),
  whereas the value 2 means that the main loop callback should be called only
  every second vsync (30fps). As a general formula, the value n means that the
  main loop is updated at every n’th vsync, or at a rate of 60/n for 60Hz
  displays, and 120/n for 120Hz displays.

  If mode is EM_TIMING_SETIMMEDIATE, then updates are performed using the
  setImmediate function, or if not available, emulated via postMessage. See
  setImmediate on MDN
  <https://developer.mozilla.org/en-US/docs/Web/API/Window/setImmediate> for more
  information. Note that this mode is strongly not recommended to be used when
  deploying Emscripten output to the web, since it depends on an unstable web
  extension that is in draft status, browsers other than IE do not currently
  support it, and its implementation has been considered controversial in review.

*/
struct sJSCCMainLoopRefreshMode {
  int _setMainLoopRefreshMode = -1;
  int _setMainLoopRefreshValue = -1;

  void ApplyWindowRefreshMode(const tF32 afRefreshTimer, const tBool abIsActive) {
    int requestedMainLoopRefreshMode = EM_TIMING_RAF;
    int requestedMainLoopRefreshValue = 0;
    if (afRefreshTimer < 0 && !abIsActive) {
      // bg refresh, 5fps
      requestedMainLoopRefreshMode = EM_TIMING_SETTIMEOUT;
      requestedMainLoopRefreshValue = 200;
    }
    else {
      requestedMainLoopRefreshMode = EM_TIMING_RAF;
      // 0 should be "whatever is considered the native platform's refresh rate"
      // 20fps = 1/20 = 0.05
      // 30fps = 1/30 = 0.03333333333
      // 40fps = 1/40 = 0.025
      // 50fps = 1/50 = 0.02
      // 60fps = 1/60 = 0.01666666667
      if (afRefreshTimer >= (1.0f/15.0f)) {
        // VSync 3, ~15fps. !!! This will be 7fps in low-power mode in browser.
        requestedMainLoopRefreshValue = 4;
      }
      else if (afRefreshTimer >= (1.0f/20.0f)) {
        // VSync 3, ~15fps. !!! This will be 10fps in low-power mode in browser.
        requestedMainLoopRefreshValue = 3;
      }
      else if (afRefreshTimer >= (1.0f/35.0f)) {
        // VSync 2, ~30fps. !!! This will be 15fps in low-power mode in browser.
        requestedMainLoopRefreshValue = 2;
      }
      else {
        // VSync 1, ~60fps. !!! This will be 30fps in low-power mode in browser.
        requestedMainLoopRefreshValue = 1;
      }
    }

    if (_jsccMainLoopFn &&
        ((requestedMainLoopRefreshMode != _setMainLoopRefreshMode) ||
         (requestedMainLoopRefreshValue != _setMainLoopRefreshValue))) {
      // emscripten_set_main_loop_timing is another mind-bending emscripten API
      // that only works when planets are aligned, so we brute force it here...
      // ApplyWindowRefreshMode should be called before SwapBuffers to make sure
      // its actually been applied
      emscripten_set_main_loop_timing(requestedMainLoopRefreshMode,
                                      requestedMainLoopRefreshValue);
      _setMainLoopRefreshMode = requestedMainLoopRefreshMode;
      _setMainLoopRefreshValue = requestedMainLoopRefreshValue;
    }
  }
};
struct sJSCCMainLoopRefreshMode _jsccMainLoopRefreshMode;

struct sJSCCWindow : public ni::cIUnknownImpl<ni::iOSWindow,ni::eIUnknownImplFlags_Default> {
  niBeginClass(sJSCCWindow);

  sJSCCWindow()
  {
    mCursor = eOSCursor_Arrow;
    mbRequestedClose = eFalse;
    mbMouseOverClient = eFalse;
    mbDropTarget = eFalse;
    mfRefreshTimer = -1;
    mbIsActive = eFalse;

    mptrMT = tMessageHandlerSinkLst::Create();
    SetCursor(eOSCursor_Arrow);

    const tBool bWebGLAA = ni::GetProperty("WebGL.AA", "true").Bool();
    const tBool bWebGLTranslucent = ni::GetProperty("WebGL.Translucent", "false").Bool();
    niLog(Info, niFmt("WebGL.AA: %y, WebGL.Translucent: %y", bWebGLAA, bWebGLTranslucent));

    const EGLint s_configAttribs[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      // Back buffer config, 888, not really used by emscripten atm, but here for completeness
      EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE,  8,
      // Transparent backbuffer, blends with the browser's content.
      // 0: off, 8: on
      EGL_ALPHA_SIZE, bWebGLTranslucent ? 8 : 0,
      // Depth buffer
      // 0: off, 24: on
      EGL_DEPTH_SIZE, 24,
      // Stencil buffer
      // 0: off, 24: on
      EGL_STENCIL_SIZE, 0,
      // Anti-aliasing 0: off, 1: on
      EGL_SAMPLE_BUFFERS, bWebGLAA ? 1 : 0,
      EGL_NONE
    };

    const EGLint eglSurfaceAttrs[] =
    {
      EGL_RENDER_BUFFER,  EGL_BACK_BUFFER,
      EGL_NONE
    };

    const EGLint eglContextAttribList[] =
    {
	    EGL_CONTEXT_CLIENT_VERSION, 2,
	    EGL_NONE
    };

    mRect = Recti(0,0,_jsccDefaultW,_jsccDefaultH);
    mfContentsScale = _jsccDefaultContentsScale;

    egldisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(eglGetError() == EGL_SUCCESS);

    eglInitialize(egldisplay, NULL, NULL);
    assert(eglGetError() == EGL_SUCCESS);

    eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(numconfigs == 1);

    eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, NULL, eglSurfaceAttrs);

    eglcontext = eglCreateContext(egldisplay, eglconfig, EGL_NO_CONTEXT, eglContextAttribList);

    {
      auto ret = eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
      if (ret == EGL_FALSE) {
        niError(niFmt("eglMakeCurrent failed:%s\n", ret));
        return;
      }
    }

    _jsccWindow = this;
  }

  ~sJSCCWindow() {
    if (this == _jsccWindow) {
      _jsccWindow = NULL;
    }
    if (egldisplay) {
      eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (eglcontext) {
      // eglDestroyContext(eglcontext);
      eglcontext = NULL;
    }
    if (eglsurface) {
      // eglDestroySurface(eglsurface);
      eglsurface = NULL;
    }
    if (egldisplay) {
      eglTerminate(egldisplay);
      egldisplay = NULL;
    }
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const niImpl {
    niClassIsOK(sJSCCWindow);
    return eglcontext != NULL;
  }

  ///////////////////////////////////////////////
  iOSWindow* __stdcall GetParent() const niImpl {
    return NULL;
  }

  tIntPtr __stdcall GetPID() const niImpl {
    return (tIntPtr)0;
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetHandle() const niImpl {
    return (tIntPtr)1; // Fake...
  }
  virtual tBool __stdcall GetIsHandleOwned() const niImpl {
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall SetClientAreaWindow(tIntPtr aHandle) niImpl {
  }
  tIntPtr __stdcall GetClientAreaWindow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall ActivateWindow() niImpl {
    mbIsActive = eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchIn(tU32 anReason) niImpl {
    mbIsActive = eTrue;
    _SendMessage(eOSWindowMessage_SwitchIn, anReason);
    return eTrue;
  }
  virtual tBool __stdcall SwitchOut(tU32 anReason) niImpl {
    mbIsActive = eFalse;
    _SendMessage(eOSWindowMessage_SwitchOut, anReason);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsActive() const niImpl {
    return mbIsActive;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTitle(const achar* aaszTitle) niImpl {
    // Note: We don't want the native code to set web page's title...
    // emscripten_set_window_title(aaszTitle);
  }
  virtual const achar* __stdcall GetTitle() const niImpl {
    return emscripten_get_window_title();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetStyle(tOSWindowStyleFlags aStyle) niImpl {
  }
  virtual tOSWindowStyleFlags __stdcall GetStyle() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetShow(tOSWindowShowFlags aShow) niImpl {
    const tBool bShow = !niFlagIs(aShow,eOSWindowShowFlags_Hide);
    // niDebugFmt(("... sJSCCWindow::SetShow: %d", bShow));
    if (bShow) {
      emscripten_run_script("NIAPP.NotifyHost('ShowWindow');");
    }
    else {
      emscripten_run_script("NIAPP.NotifyHost('HideWindow');");
    }
  }
  virtual tOSWindowShowFlags __stdcall GetShow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetZOrder(eOSWindowZOrder aZOrder) niImpl {
  }
  virtual eOSWindowZOrder __stdcall GetZOrder() const niImpl {
    return eOSWindowZOrder_Normal;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSize(const sVec2i& avSize) niImpl {
    sRecti rect = GetRect();
    rect.SetSize(avSize);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetSize() const niImpl {
    return GetRect().GetSize();
  }
  virtual void __stdcall SetPosition(const sVec2i& avPos) niImpl {
    sRecti rect = GetRect();
    rect.SetTopLeft(avPos);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetPosition() const niImpl {
    return sVec2i::Zero();
  }
  virtual void __stdcall SetRect(const sRecti& aRect) niImpl {
    mRect = aRect;
  }
  virtual sRecti __stdcall GetRect() const niImpl {
    return mRect;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetClientSize(const sVec2i& avSize) niImpl {
    return SetSize(avSize);
  }
  virtual sVec2i __stdcall GetClientSize() const niImpl {
    return GetSize();
  }

  ///////////////////////////////////////////////
  virtual tF32 __stdcall GetContentsScale() const {
    return mfContentsScale;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Clear() niImpl {
  }

  ///////////////////////////////////////////////
  virtual tMessageHandlerSinkLst* __stdcall GetMessageHandlers() const niImpl {
    return mptrMT;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall UpdateWindow(tBool abBlockingMessages) niImpl {
    _jsccMainLoopRefreshMode.ApplyWindowRefreshMode(mfRefreshTimer, mbIsActive);
    this->_SendMessage(eOSWindowMessage_Paint);
    eglSwapBuffers(egldisplay, eglsurface);
    return eFalse;
  }

  virtual tBool __stdcall RedrawWindow() niImpl {
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall CenterWindow() niImpl {
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetRequestedClose() const niImpl {
    return mbRequestedClose;
  }
  virtual void __stdcall SetRequestedClose(tBool abRequested) niImpl {
    mbRequestedClose = abRequested;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCursor(eOSCursor aCursor) niImpl {
    if (mCursor == aCursor)
      return;
    mCursor = aCursor;
  }

  virtual eOSCursor __stdcall GetCursor() const niImpl {
    return mCursor;
  }

  void _UpdateCursor(eOSCursor aCursor) {
  }

  virtual tBool __stdcall InitCustomCursor(tIntPtr aID, tU32 anWidth, tU32 anHeight, tU32 anPivotX, tU32 anPivotY, const tU32* apData) niImpl {
    return eTrue;
  }
  virtual tIntPtr __stdcall GetCustomCursorID() const niImpl {
    return 0;
  }

  virtual void __stdcall SetCursorPosition(const sVec2i& avCursorPos) niImpl {
  }
  virtual sVec2i __stdcall GetCursorPosition() const niImpl {
    return sVec2i::Zero();
  }
  virtual void __stdcall SetCursorCapture(tBool abCapture) niImpl {
  }
  virtual tBool __stdcall GetCursorCapture() const niImpl {
    return eFalse;
  }
  virtual tBool __stdcall GetIsCursorOverClient() const niImpl {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall TryClose() niImpl {
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDropTarget(tBool abDropTarget) niImpl
  {
  }

  virtual tBool __stdcall GetDropTarget() const niImpl
  {
    return false;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetMonitor() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetFullScreen(tU32 anScreenId) niImpl {
    return eFalse;
  }
  tU32 __stdcall GetFullScreen() const niImpl {
    return 1;
  }
  tBool __stdcall GetIsMinimized() const niImpl {
    return eFalse;
  }
  tBool __stdcall GetIsMaximized() const niImpl {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFocus() niImpl {
  }
  virtual tBool __stdcall GetHasFocus() const niImpl {
    return mbIsActive;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetRefreshTimer(tF32 afRefreshTimer) niImpl {
    if (mfRefreshTimer == afRefreshTimer)
      return;
    mfRefreshTimer = afRefreshTimer;
  }
  virtual tF32 __stdcall GetRefreshTimer() const niImpl {
    return mfRefreshTimer;
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetParentHandle() const niImpl {
    return 0;
  }
  virtual tU32 __stdcall IsParentWindow(tIntPtr aHandle) const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall AttachGraphicsAPI(iOSGraphicsAPI* apAPI) {
    niCheckIsOK(apAPI,eFalse);

    return eTrue;
  }
  virtual iOSGraphicsAPI* __stdcall GetGraphicsAPI() const {
    return mptrAPI;
  }

  ///////////////////////////////////////////////
  __forceinline tBool _SendMessage(eOSWindowMessage aMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) {
    return ni::SendMessages(mptrMT,aMsg,avarA,avarB);
  }

  EGLDisplay egldisplay;
  EGLConfig  eglconfig;
  EGLSurface eglsurface;
  EGLContext eglcontext;
  EGLint numconfigs;

  Ptr<tMessageHandlerSinkLst> mptrMT;
  tBool                       mbRequestedClose;
  tBool                       mbMouseOverClient;
  tBool                       mbDropTarget;
  tBool                       mbIsActive;
  tF32                        mfRefreshTimer;

  sVec2i mvPrevMousePos = {eInvalidHandle,eInvalidHandle};
  sRecti mRect;
  tF32 mfContentsScale;

  eOSCursor        mCursor;
  Ptr<iOSGraphicsAPI> mptrAPI;

  niEndClass(sJSCCWindow);
};

iOSWindow* __stdcall cLang::CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle) {
  Ptr<sJSCCWindow> wnd = niNew sJSCCWindow();
  return wnd.GetRawAndSetNull();
}

#define CHECK_WINDOW(RETERR)                    \
  sJSCCWindow* wnd = _jsccWindow;               \
  if (!wnd) {                                   \
    niWarning("No active window.");             \
    return RETERR;                              \
  }

// For some reason this isnt declared in key_codes.h
static const tI32 DOM_VK_MINUS = 0xBA;

niExportJSCC(int) niJSCC_TranslateDOMKeyCode(int aDOMKeyCode) {
  switch (aDOMKeyCode) {

    case DOM_VK_BACK_SPACE: return eKey_BackSpace;
    case DOM_VK_TAB: return eKey_Tab;
    case DOM_VK_RETURN: return eKey_Enter;
    case DOM_VK_PAUSE: return eKey_Pause;
    case DOM_VK_ESCAPE: return eKey_Escape;
    case DOM_VK_SPACE: return eKey_Space;

    case DOM_VK_DELETE: return eKey_Delete;

    case DOM_VK_NUMPAD0: return eKey_NumPad0;
    case DOM_VK_NUMPAD1: return eKey_NumPad1;
    case DOM_VK_NUMPAD2: return eKey_NumPad2;
    case DOM_VK_NUMPAD3: return eKey_NumPad3;
    case DOM_VK_NUMPAD4: return eKey_NumPad4;
    case DOM_VK_NUMPAD5: return eKey_NumPad5;
    case DOM_VK_NUMPAD6: return eKey_NumPad6;
    case DOM_VK_NUMPAD7: return eKey_NumPad7;
    case DOM_VK_NUMPAD8: return eKey_NumPad8;
    case DOM_VK_NUMPAD9: return eKey_NumPad9;
    case DOM_VK_DECIMAL: return eKey_NumPadPeriod;
    case DOM_VK_DIVIDE: return eKey_NumPadSlash;
    case DOM_VK_MULTIPLY: return eKey_NumPadStar;
    case DOM_VK_SUBTRACT: return eKey_NumPadMinus;
    case DOM_VK_ADD: return eKey_NumPadPlus;

    case DOM_VK_UP: return eKey_Up;
    case DOM_VK_DOWN: return eKey_Down;
    case DOM_VK_RIGHT: return eKey_Right;
    case DOM_VK_LEFT: return eKey_Left;
    case DOM_VK_INSERT: return eKey_Insert;
    case DOM_VK_HOME: return eKey_Home;
    case DOM_VK_END: return eKey_End;
    case DOM_VK_PAGE_UP: return eKey_PgUp;
    case DOM_VK_PAGE_DOWN: return eKey_PgDn;

    case DOM_VK_F1: return eKey_F1;
    case DOM_VK_F2: return eKey_F2;
    case DOM_VK_F3: return eKey_F3;
    case DOM_VK_F4: return eKey_F4;
    case DOM_VK_F5: return eKey_F5;
    case DOM_VK_F6: return eKey_F6;
    case DOM_VK_F7: return eKey_F7;
    case DOM_VK_F8: return eKey_F8;
    case DOM_VK_F9: return eKey_F9;
    case DOM_VK_F10: return eKey_F10;
    case DOM_VK_F11: return eKey_F11;
    case DOM_VK_F12: return eKey_F12;
    case DOM_VK_F13: return eKey_F13;
    case DOM_VK_F14: return eKey_F14;
    case DOM_VK_F15: return eKey_F15;


    case DOM_VK_A: return eKey_A;
    case DOM_VK_B: return eKey_B;
    case DOM_VK_C: return eKey_C;
    case DOM_VK_D: return eKey_D;
    case DOM_VK_E: return eKey_E;
    case DOM_VK_F: return eKey_F;
    case DOM_VK_G: return eKey_G;
    case DOM_VK_H: return eKey_H;
    case DOM_VK_I: return eKey_I;
    case DOM_VK_J: return eKey_J;
    case DOM_VK_K: return eKey_K;
    case DOM_VK_L: return eKey_L;
    case DOM_VK_M: return eKey_M;
    case DOM_VK_N: return eKey_N;
    case DOM_VK_O: return eKey_O;
    case DOM_VK_P: return eKey_P;
    case DOM_VK_Q: return eKey_Q;
    case DOM_VK_R: return eKey_R;
    case DOM_VK_S: return eKey_S;
    case DOM_VK_T: return eKey_T;
    case DOM_VK_U: return eKey_U;
    case DOM_VK_V: return eKey_V;
    case DOM_VK_W: return eKey_W;
    case DOM_VK_X: return eKey_X;
    case DOM_VK_Y: return eKey_Y;
    case DOM_VK_Z: return eKey_Z;

    case DOM_VK_0: return eKey_n0;
    case DOM_VK_1: return eKey_n1;
    case DOM_VK_2: return eKey_n2;
    case DOM_VK_3: return eKey_n3;
    case DOM_VK_4: return eKey_n4;
    case DOM_VK_5: return eKey_n5;
    case DOM_VK_6: return eKey_n6;
    case DOM_VK_7: return eKey_n7;
    case DOM_VK_8: return eKey_n8;
    case DOM_VK_9: return eKey_n9;

    case DOM_VK_CANCEL: return eKey_Unknown;
    case DOM_VK_HELP: return eKey_Unknown;
    case DOM_VK_CLEAR: return eKey_Unknown;
    case DOM_VK_ENTER: return eKey_Enter;
    case DOM_VK_SHIFT: return eKey_LShift;
    case DOM_VK_CONTROL: return eKey_LControl;
    case DOM_VK_ALT: return eKey_LAlt;
    case DOM_VK_CAPS_LOCK: return eKey_CapsLock;
    case DOM_VK_JUNJA: return eKey_Unknown;
    case DOM_VK_FINAL: return eKey_Unknown;
    case DOM_VK_CONVERT: return eKey_Unknown;
    case DOM_VK_NONCONVERT: return eKey_Unknown;
    case DOM_VK_ACCEPT: return eKey_Unknown;
    case DOM_VK_MODECHANGE: return eKey_Unknown;
    case DOM_VK_SELECT: return eKey_Unknown;
    case DOM_VK_PRINT: return eKey_Unknown;
    case DOM_VK_EXECUTE: return eKey_Unknown;
    case DOM_VK_PRINTSCREEN: return eKey_PrintScreen;
    case DOM_VK_SEMICOLON: return eKey_Semicolon;
    case DOM_VK_MINUS: return eKey_Minus;
    case DOM_VK_EQUALS: return eKey_Equals;

    case DOM_VK_CONTEXT_MENU: return eKey_Apps;
    case DOM_VK_SLEEP: return eKey_Unknown;
    case DOM_VK_SEPARATOR: return eKey_Unknown;
    case DOM_VK_F16: return eKey_Unknown;
    case DOM_VK_F17: return eKey_Unknown;
    case DOM_VK_F18: return eKey_Unknown;
    case DOM_VK_F19: return eKey_Unknown;
    case DOM_VK_F20: return eKey_Unknown;
    case DOM_VK_F21: return eKey_Unknown;
    case DOM_VK_F22: return eKey_Unknown;
    case DOM_VK_F23: return eKey_Unknown;
    case DOM_VK_F24: return eKey_Unknown;
    case DOM_VK_NUM_LOCK: return eKey_NumLock;
    case DOM_VK_SCROLL_LOCK: return eKey_Scroll;
    case DOM_VK_COMMA: return eKey_Comma;
    case DOM_VK_PERIOD: return eKey_Period;
    case DOM_VK_SLASH: return eKey_Slash;
    case DOM_VK_BACK_QUOTE: return eKey_Tilde;
    case DOM_VK_OPEN_BRACKET: return eKey_LBracket;
    case DOM_VK_BACK_SLASH: return eKey_BackSlash;
    case DOM_VK_CLOSE_BRACKET: return eKey_RBracket;
    case DOM_VK_QUOTE: return eKey_Unknown;
    case DOM_VK_META: return eKey_RAlt;

    default: return eKey_Unknown;
  }
};

niExportJSCC(int) niJSCC_TranslateDOMMouseButton(int aDOMMouseButton) {
  switch (aDOMMouseButton) {
    case 0: return ePointerButton_Left;
    case 1: return ePointerButton_Middle;
    case 2: return ePointerButton_Right;
  }
  return ePointerButton_Last;
}

niExportJSCC(void) niJSCC_SetProperty(const char* name, const char* value) {
  TRACE_JSCC_CAPI(("... niJSCC_SetProperty: name: '%s' value: '%s'", name, value));
  ni::GetLang()->SetProperty(name,value);
}

niExportJSCC(const char*) niJSCC_GetProperty(const char* name) {
  TRACE_JSCC_CAPI(("... niJSCC_GetProperty: name: '%s'", name));
  static ni::cString _lastPropertyValue;
  _lastPropertyValue = ni::GetLang()->GetProperty(name);
  return _lastPropertyValue.Chars();
}

niExportJSCC(int) niJSCC_RunCommand(const char* aCmd) {
  TRACE_JSCC_CAPI(("... niJSCC_RunCommand"));
  ni::iConsole* pConsole = ni::GetConsole();
  if (pConsole) {
    return pConsole->RunCommand(aCmd);
  }
  return eFalse;
}

niExportJSCC(int) niJSCC_HasWindow() {
  TRACE_JSCC_CAPI(("... niJSCC_HasWindow"));
  CHECK_WINDOW(eFalse);
  return eTrue;
}

niExportJSCC(void) niJSCC_WndNotifyResize(ni::tI32 w, ni::tI32 h, ni::tF32 afContentsScale) {
  TRACE_JSCC_CAPI(("... niJSCC_WndNotifyResize: w: %d  h: %d contentsScale: %g",
                   w, h, afContentsScale));
  _jsccDefaultW = w;
  _jsccDefaultH = h;
  _jsccDefaultContentsScale = afContentsScale;
  CHECK_WINDOW(;);
  wnd->SetRect(ni::Recti(0,0,w,h));
  wnd->mfContentsScale = afContentsScale;
}

niExportJSCC(void) niJSCC_WndNotifyFocus(ni::tBool abFocused) {
  TRACE_JSCC_CAPI(("... niJSCC_WndNotifyFocus: focused: %d", abFocused));
  CHECK_WINDOW(;);
  if (abFocused) {
    wnd->SwitchIn(eOSWindowSwitchReason_SetFocus);
  } else {
    wnd->SwitchOut(eOSWindowSwitchReason_LostFocus);
  }
}

niExportJSCC(void) niJSCC_WndInputKey(ni::eKey aKey, ni::tBool abIsDown) {
  TRACE_JSCC_KEY(("... niJSCC_WndKey: key: %d, isDown: %d", aKey, abIsDown));
  CHECK_WINDOW(;);
  wnd->_SendMessage(
      abIsDown ? eOSWindowMessage_KeyDown : eOSWindowMessage_KeyUp,
      aKey, niVarNull);
}
niExportJSCC(void) niJSCC_WndInputString(const ni::achar* aaszString) {
  TRACE_JSCC_KEY(("... niJSCC_WndInputString"));
  CHECK_WINDOW(;);
  StrCharIt it(aaszString);
  while (!it.is_end()) {
    const tU32 c = it.next();
    wnd->_SendMessage(eOSWindowMessage_KeyChar,c);
  }
}

niExportJSCC(void) niJSCC_WndInputMouseMove(ni::tF32 x, ni::tF32 y)
{
  TRACE_JSCC_MOUSE_MOVE(("... niJSCC_WndInputMouseMove: x: %g y: %g", x, y));
  CHECK_WINDOW(;);
  wnd->_SendMessage(eOSWindowMessage_MouseMove,Vec2i(x,y));
}
niExportJSCC(void) niJSCC_WndInputMouseRelativeMove(ni::tF32 rx, ni::tF32 ry)
{
  TRACE_JSCC_MOUSE_MOVE(("... niJSCC_WndInputMouseRelativeMove: rx: %g ry: %g", rx, ry));
  CHECK_WINDOW(;);
  wnd->_SendMessage(eOSWindowMessage_RelativeMouseMove,Vec2f(rx,ry));
}
niExportJSCC(void) niJSCC_WndInputMouseButton(tI32 aPointerButton, ni::tBool abIsDown) {
  TRACE_JSCC_MOUSE_BT(("... niJSCC_WndInputMouseButton: button: %d, isDown: %d",
                       aPointerButton, abIsDown));
  CHECK_WINDOW(;);
  wnd->_SendMessage(
    abIsDown ? eOSWindowMessage_MouseButtonDown : eOSWindowMessage_MouseButtonUp,
    aPointerButton);
}
niExportJSCC(void) niJSCC_WndInputMouseDoubleClick(tI32 aPointerButton) {
  TRACE_JSCC_MOUSE_BT(("... niJSCC_WndInputMouseDoubleClick: button: %d", aPointerButton));
  CHECK_WINDOW(;);
  wnd->_SendMessage(eOSWindowMessage_MouseButtonDoubleClick, aPointerButton);
}
niExportJSCC(void) niJSCC_WndInputMouseWheel(tF32 deltaY) {
  TRACE_JSCC_MOUSE_BT(("... niJSCC_WndInputMouseWheel: %g", deltaY));
  CHECK_WINDOW(;);
  wnd->_SendMessage(eOSWindowMessage_MouseWheel, deltaY);
}

///////////////////////////////////////////////
const tI32 MAX_TOUCHES = 10;
static ni::sVec2f  _lastTouchPosition[MAX_TOUCHES] = {{}};
static ni::tIntPtr _touches[MAX_TOUCHES] = {};

static tI32 _GetFingerIndexFromTouch(tIntPtr touch) {
  for (tI32 i = 0; i < MAX_TOUCHES; ++i) {
    if (_touches[i] == touch) {
      return i;
    }
  }
  return -1;
}
static tI32 _AddNewTouch(tIntPtr touch) {
  for (tI32 i = 0; i < MAX_TOUCHES; ++i) {
    if (!_touches[i]) {
      _touches[i] = touch;
      return i;
    }
  }
  return -1;
}
static tI32 _RemoveFingerIndex(const int fingerIndex) {
  niAssert(fingerIndex < MAX_TOUCHES);
  _touches[fingerIndex] = 0;
  return -1;
}
static tI32 _CountNumTouches() {
  int count = 0;
  for (int i = 0; i < MAX_TOUCHES; ++i) {
    if (_touches[i]) {
      ++count;
    }
  }
  return count;
}

niExportJSCC(void) niJSCC_WndInputFingerMove(
    ni::tI32 fingerIdentifier,
    ni::tF32 x, ni::tF32 y, ni::tF32 pressure)
{
  TRACE_JSCC_FINGER(("... niJSCC_WndInputFingerMove: id: %d x: %g y: %g pressure: %g",
                     fingerIdentifier, x, y, pressure));
  CHECK_WINDOW(;);

  const tI32 fingerIndex = _GetFingerIndexFromTouch(fingerIdentifier);
  if (fingerIndex >= 0) {
    const sVec3f v = Vec3f(x,y,pressure);
    TRACE_JSCC_FINGER(("... JSCC: FingerMove: %d, %s", fingerIndex, v));
    wnd->_SendMessage(eOSWindowMessage_FingerMove,fingerIndex,Vec3f(x,y,pressure));
    {
      const ni::tF32 relativeMoveSpeedScale = wnd->GetContentsScale();
      const ni::sVec2f delta = (Vec2f(x,y) - _lastTouchPosition[fingerIndex]) * relativeMoveSpeedScale;
      const sVec3f rv = Vec3f(delta.x,delta.y,pressure);
      TRACE_JSCC_FINGER(("... JSCC: FingerRelativeMove: %d, %s", fingerIndex, rv));
      wnd->_SendMessage(eOSWindowMessage_FingerRelativeMove,fingerIndex, rv);
    }
    _lastTouchPosition[fingerIndex] = Vec2f(x,y);
  }
}
niExportJSCC(void) niJSCC_WndInputFingerPress(
    ni::tI32 fingerIdentifier, ni::tBool isDown,
    ni::tF32 x, ni::tF32 y, ni::tF32 pressure)
{
  TRACE_JSCC_FINGER(("... niJSCC_WndInputFingerPress: id: %d x: %g y: %g pressure: %g",
                     fingerIdentifier, x, y, pressure));
  CHECK_WINDOW(;);

  const sVec3f v = Vec3f(x,y,pressure);
  if (isDown) {
    const tI32 fingerIndex = _AddNewTouch(fingerIdentifier);
    if (fingerIndex >= 0) {
      _lastTouchPosition[fingerIndex] = Vec2(v.ptr());
      TRACE_JSCC_FINGER(("... JSCC: FingerDown: %d, %s", fingerIndex, v));
      wnd->_SendMessage(eOSWindowMessage_FingerDown, fingerIndex, v);
    }
  }
  else {
    const tI32 fingerIndex = _GetFingerIndexFromTouch(fingerIdentifier);
    if (fingerIndex >= 0) {
      _lastTouchPosition[fingerIndex] = Vec2(v.ptr());
      TRACE_JSCC_FINGER(("... JSCC: FingerUp: %d, %s", fingerIndex, v));
      wnd->_SendMessage(eOSWindowMessage_FingerUp, fingerIndex, v);
      _RemoveFingerIndex(fingerIndex);
    }
  }
}
#endif
