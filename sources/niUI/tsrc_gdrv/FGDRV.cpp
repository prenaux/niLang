#include "stdafx.h"
#include "FGDRV.h"
#include <niLang/Utils/Trace.h>

namespace ni {

niLetK knNumStepsInCI = 10;

niDeclareModuleTrace_(Test,TraceOSWindow);
#define TRACE_OSWINDOW(FMT) niModuleTrace_(Test,TraceOSWindow,FMT)

struct FGDRV_WindowHandler : public ImplRC<iMessageHandler> {
  const tU64 _threadId;
  sFGDRV_Base* _context;
  UnitTest::TestResults* _testResults = nullptr;
  tU32 _debugKeyMod;

  FGDRV_WindowHandler(sFGDRV_Base* aContext)
      : _threadId(ni::ThreadGetCurrentThreadID())
      , _context(aContext)
  {}

  void __stdcall Invalidate() niImpl {
    _context = nullptr;
    _testResults = nullptr;
  }

  tU64 __stdcall GetThreadID() const niImpl {
    return _threadId;
  }

  void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) niImpl {
    if (!_context)
      return;
    iOSWindow* wnd = _context->_window;
    switch (anMsg) {
      case eOSWindowMessage_Close:
        TRACE_OSWINDOW(("eOSWindowMessage_Close: \n"));
        wnd->Invalidate();
        break;
      case eOSWindowMessage_SwitchIn:
        TRACE_OSWINDOW(("eOSWindowMessage_SwitchIn: \n"));
        break;
      case eOSWindowMessage_SwitchOut:
        TRACE_OSWINDOW(("eOSWindowMessage_SwitchOut: \n"));
        break;
      case eOSWindowMessage_Paint: {
        // TRACE_OSWINDOW(("eOSWindowMessage_Paint: \n"));
        if (_context && _testResults) {
          _context->BeforePaint(*_testResults);
          _context->OnPaint(*_testResults);
          _context->AfterPaint(*_testResults);
        }
        break;
      }
      case eOSWindowMessage_Size:
        TRACE_OSWINDOW(("eOSWindowMessage_Size: %s\n",_ASZ(wnd->GetSize())));
        break;
      case eOSWindowMessage_Move:
        TRACE_OSWINDOW(("eOSWindowMessage_Move: %s\n",_ASZ(wnd->GetPosition())));
        break;
      case eOSWindowMessage_KeyDown:
        TRACE_OSWINDOW(("eOSWindowMessage_KeyDown: %d (%s)\n",a.mU32,niEnumToChars(eKey,a.mU32)));
        switch (a.mU32) {
          case eKey_LControl:
            niFlagOn(_debugKeyMod,eKeyMod_Control);
            break;
          case eKey_LAlt:
            niFlagOn(_debugKeyMod,eKeyMod_Alt);
            break;
          case eKey_LShift:
            niFlagOn(_debugKeyMod,eKeyMod_Shift);
            break;
          case eKey_F:
            wnd->SetFullScreen(wnd->GetFullScreen() == eInvalidHandle ? 0 : eInvalidHandle);
            break;
          case eKey_Z:
            wnd->SetCursorPosition(sVec2i::Zero());
            break;
          case eKey_X:
            wnd->SetCursorPosition(wnd->GetClientSize()/2);
            break;
          case eKey_C:
            wnd->SetCursorPosition(wnd->GetClientSize());
            break;
          case eKey_Q:
            wnd->SetCursor(eOSCursor_None);
            break;
          case eKey_W:
            wnd->SetCursor(eOSCursor_Arrow);
            break;
          case eKey_E:
            wnd->SetCursor(eOSCursor_Text);
            break;
          case eKey_K:
            wnd->SetCursorCapture(!wnd->GetCursorCapture());
            break;
          case eKey_Space: {
            _context->_animated = !_context->_animated;
            niLog(Info,niFmt("Toggled animation: %z.",_context->_animated));
            break;
          }
          default: {
            switch (a.mU32 | _debugKeyMod) {
              // trigger a breakpoint
              case eKey_B|(eKeyMod_Control|eKeyMod_Alt|eKeyMod_Shift): {
                TRACE_OSWINDOW(("... Trigger Breakpoint ..."));
                ni_debug_break();
                break;
              }
            }
            break;
          }
        }
        break;
      case eOSWindowMessage_KeyUp:
        TRACE_OSWINDOW(("eOSWindowMessage_KeyUp: %d (%s)\n",a.mU32,niEnumToChars(eKey,a.mU32)));
        switch (a.mU32) {
          case eKey_LControl:
            niFlagOff(_debugKeyMod,eKeyMod_Control);
            break;
          case eKey_LAlt:
            niFlagOff(_debugKeyMod,eKeyMod_Alt);
            break;
          case eKey_LShift:
            niFlagOff(_debugKeyMod,eKeyMod_Shift);
            break;
        }
        break;
      case eOSWindowMessage_KeyChar:
        TRACE_OSWINDOW(("eOSWindowMessage_KeyChar: %c (%d) \n",a.mU32,a.mU32));
        break;
      case eOSWindowMessage_MouseButtonDown:
        TRACE_OSWINDOW(("eOSWindowMessage_MouseButtonDown: %d\n",a.mU32));
        break;
      case eOSWindowMessage_MouseButtonUp:
        TRACE_OSWINDOW(("eOSWindowMessage_MouseButtonUp: %d\n",a.mU32));
        break;
      case eOSWindowMessage_MouseButtonDoubleClick:
        TRACE_OSWINDOW(("eOSWindowMessage_MouseButtonDoubleClick: %d\n",a.mU32));
        if (a.mU32 == 9 /*ePointerButton_DoubleClickRight*/) {
          //                     wnd->SetFullScreen(!wnd->GetFullScreen());
        }
        else if (a.mU32 == 8 /*ePointerButton_DoubleClickLeft*/) {
          //                     wnd->SetFullScreen(!wnd->GetFullScreen());
        }
        break;
      case eOSWindowMessage_MouseWheel:
        TRACE_OSWINDOW(("eOSWindowMessage_MouseWheel: %f, %f\n",a.GetFloatValue(),b.GetFloatValue()));
        break;
      case eOSWindowMessage_LostFocus: {
        TRACE_OSWINDOW(("eOSWindowMessage_LostFocus: \n"));
        break;
      }
      case eOSWindowMessage_SetFocus:
        TRACE_OSWINDOW(("eOSWindowMessage_SetFocus: \n"));
        break;
      case eOSWindowMessage_Drop:
        TRACE_OSWINDOW(("eOSWindowMessage_Drop: \n"));
        break;

      case eOSWindowMessage_RelativeMouseMove:
#ifdef TRACE_MOUSE_MOVE
        TRACE_OSWINDOW(("eOSWindowMessage_RelativeMouseMove: [%d,%d]\n"),
                    a.mV2L[0],a.mV2L[1]));
#endif
        break;

      case eOSWindowMessage_MouseMove:
#ifdef TRACE_MOUSE_MOVE
        TRACE_OSWINDOW(("eOSWindowMessage_MouseMove: [%d,%d] [%d,%d] (contentsScale: %g)\n"),
                    a.mV2L[0],a.mV2L[1],
                    b.mV2L[0],b.mV2L[1],
                    wnd->GetContentsScale()));
#endif
        break;
    }
  }
};

tBool sFGDRV_Base::Start(UnitTest::TestResults& testResults_) {
  _isInteractive = UnitTest::runFixtureName.Eq(m_testName);

  _mq = niCheckNN(_mq,ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID()),eFalse);

  _windowHandler = niCheckNN(_windowHandler,niNew FGDRV_WindowHandler(this),eFalse);

  // Create the window
  _window = niCheckNN(_window,ni::GetLang()->CreateWindow(
    nullptr,
    m_testName,
    sRecti(50,50,640,480),
    0,
    eOSWindowStyleFlags_Regular),eFalse);
  _window->SetClientSize(Vec2i(
    Vec2f(_window->GetClientSize())*_window->GetContentsScale()));
  _window->GetMessageHandlers()->AddSink(_windowHandler);

  // Get the graphics driver name, can be set through -Drenderer=NAME
  if (HStringIsEmpty(_gdrvName)) {
    _gdrvName = _H("Auto");
    if (ni::GetLang()->HasProperty("renderer")) {
      _gdrvName = _H(ni::GetLang()->GetProperty("renderer"));
    }
  }

  // Initialize graphics
  _graphics = niCheckNN(_graphics,(iGraphics*)niCreateInstance(niUI,Graphics,niVarNull,niVarNull),eFalse);
  niCheck(_graphics->InitializeDriver(_gdrvName),eFalse);

  _graphicsContext = niCheckNN(_graphicsContext,_graphics->CreateContextForWindow(
    _window,
    _A("R8G8B8A8"),_A("D24S8"),
    _swapInterval,
    _backBufferFlags),eFalse);

  niCheck(this->OnInit(testResults_),eFalse);
  return eTrue;
}

void sFGDRV_Base::End(UnitTest::TestResults& testResults_) {
  ni::SafeInvalidate(_windowHandler.raw_ptr());
  ni::SafeInvalidate(_window.raw_ptr());
  ni::SafeInvalidate(_mq.raw_ptr());
  ni::SafeInvalidate(_graphicsContext.raw_ptr());
  ni::SafeInvalidate(_graphics.raw_ptr());
}

tBool sFGDRV_Base::Step(UnitTest::TestResults& testResults_) {
  // TRACE_OSWINDOW(("... STEP %s: %d", m_testName, _testStepCount));
  FGDRV_WindowHandler* windowHandler = (FGDRV_WindowHandler*)_windowHandler.raw_ptr();
  windowHandler->_testResults = &testResults_;
  niDefer {
    windowHandler->_testResults = nullptr;
    _testStepCount++;
  };
  if (_testStepCount == 0) {
    _window->CenterWindow();
    _window->SetShow(eOSWindowShowFlags_Show);
    _window->ActivateWindow();
    _window->SetRefreshTimer(0.0f);
  }
  {
    sMessageDesc msg;
    while (_mq->Poll(&msg)) {
      msg.mptrHandler->HandleMessage(msg.mnMsg, msg.mvarA, msg.mvarB);
      if (msg.mnMsg == eOSWindowMessage_Paint)
        break;
    }
  }
  if (_window->GetRequestedClose()) {
    return eFalse;
  }
  _window->UpdateWindow(eTrue);
  if (_isInteractive) {
    return eTrue;
  }
  else {
    return _testStepCount < (knNumStepsInCI-1);
  }
}

tBool sFGDRV_Base::OnInit(UnitTest::TestResults& testResults_) {
  _window->SetTitle(niFmt("%s (%s)", m_testName, _graphics->GetDriver()->GetName()));
  return eTrue;
}

tBool sFGDRV_Base::BeforePaint(UnitTest::TestResults& testResults_) {
  if (!(this->_clearTimer < 0) &&
      (this->_clearTimer == 0.0f ||
       (ni::TimerInSeconds()-this->_clearTimer) > 0.5f))
  {
    this->_clearColor = ni::RandColorA(255);
    this->_clearTimer = ni::TimerInSeconds();
  }

  this->_graphicsContext->ClearBuffers(
    this->_clearBuffers, this->_clearColor, 1.0f, 0);

  return eTrue;
}

tBool sFGDRV_Base::AfterPaint(UnitTest::TestResults& testResults_) {
  // Display
  this->_graphicsContext->Display(0, sRecti::Null());

  // Update the frame time
  ni::GetLang()->UpdateFrameTime(ni::TimerInSeconds());
  if (this->_animated) {
    this->_animationTime += ni::GetLang()->GetFrameTime();
    this->_pingpongTime = ni::Cos<tF32>((tF32)_animationTime * 2.0f) * 0.5f + 0.5f;
  }

  sVec4i memStats;
  ni_mem_get_stats(&memStats);
  cString fpsText = niFmt(
    "%s (%s) - %gfps %dms %dalloc %dobj",
    m_testName, _graphics->GetDriver()->GetName(),
    ni::GetLang()->GetAverageFrameRate(),
    (ni::GetLang()->GetFrameTime() * 1000.0),
    memStats.x - memStats.y,
    memStats.z - memStats.w
  );
  this->_window->SetTitle(fpsText.c_str());

  return eTrue;
}

tBool sFGDRV_Base::OnPaint(UnitTest::TestResults& testResults_) {
  return eTrue;
}

}
