#include "stdafx.h"
#include "FGDRV.h"

namespace ni {

tBool FGDRV::_InitTest(const achar* aTestName) {
  _isInteractive = UnitTest::runFixtureName.Eq(aTestName);

  _mq = niCheckNN(_mq,ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID()),eFalse);

  // Create the window
  _window = niCheckNN(_window,ni::GetLang()->CreateWindow(
    nullptr,
    aTestName,
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular),eFalse);

  NN<FGDRV_WindowHandler> sink {niNew FGDRV_WindowHandler(this)};
  _window->GetMessageHandlers()->AddSink(sink);

  // Get the graphics driver name, can be set through -Drenderer=NAME
  _gdrvName = _H("Auto");
  if (ni::GetLang()->HasProperty("renderer")) {
    _gdrvName = _H(ni::GetLang()->GetProperty("renderer"));
  }

  // Initialize graphics
  _graphics = niCheckNN(_graphics,(iGraphics*)niCreateInstance(niUI,Graphics,niVarNull,niVarNull),eFalse);
  niCheck(_graphics->InitializeDriver(_gdrvName),eFalse);

  _graphicsContext = niCheckNN(_graphicsContext,_graphics->CreateContextForWindow(
    _window,
    _A("R8G8B8A8"),_A("D24S8"),
    _swapInterval,
    _backBufferFlags),eFalse);

  return eTrue;
}

void FGDRV::_Invalidate() {
    _graphicsContext->Invalidate();
    _graphics->Invalidate();
    _window->Invalidate();
    _mq->Invalidate();
}

FGDRV::~FGDRV() {
  this->_Invalidate();
}

FGDRV_WindowHandler::FGDRV_WindowHandler(FGDRV* aContext)
    : _threadId(ni::ThreadGetCurrentThreadID())
    , _context(aContext)
{}

tU64 __stdcall FGDRV_WindowHandler::GetThreadID() const {
  return _threadId;
}

void FGDRV_WindowHandler::OnPaint() {
  if (_context->_clearTimer <= 0.0f ||
      (ni::TimerInSeconds()-_context->_clearTimer) > 0.5f)
  {
    _context->_clearColor = ni::RandColorA(255);
    _context->_clearTimer = ni::TimerInSeconds();
  }
}

void FGDRV_WindowHandler::_Paint() {
  _context->_graphicsContext->ClearBuffers(
    _context->_clearBuffers, _context->_clearColor, 1.0f, 0);
  this->OnPaint();
  _context->_graphicsContext->Display(0, sRecti::Null());
}

void __stdcall FGDRV_WindowHandler::HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
  iOSWindow* wnd = _context->_window;
  switch (anMsg) {
    case eOSWindowMessage_Close:
      niDebugFmt((_A("eOSWindowMessage_Close: \n")));
      wnd->Invalidate();
      break;
    case eOSWindowMessage_SwitchIn:
      niDebugFmt((_A("eOSWindowMessage_SwitchIn: \n")));
      break;
    case eOSWindowMessage_SwitchOut:
      niDebugFmt((_A("eOSWindowMessage_SwitchOut: \n")));
      break;
    case eOSWindowMessage_Paint: {
      // niDebugFmt((_A("eOSWindowMessage_Paint: \n")));
      this->_Paint();
      break;
    }
    case eOSWindowMessage_Size:
      niDebugFmt((_A("eOSWindowMessage_Size: %s\n"),_ASZ(wnd->GetSize())));
      break;
    case eOSWindowMessage_Move:
      niDebugFmt((_A("eOSWindowMessage_Move: %s\n"),_ASZ(wnd->GetPosition())));
      break;
    case eOSWindowMessage_KeyDown:
      niDebugFmt((_A("eOSWindowMessage_KeyDown: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
      switch (a.mU32) {
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
      }
      break;
    case eOSWindowMessage_KeyUp:
      niDebugFmt((_A("eOSWindowMessage_KeyUp: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
      break;
    case eOSWindowMessage_KeyChar:
      niDebugFmt((_A("eOSWindowMessage_KeyChar: %c (%d) \n"),a.mU32,a.mU32));
      break;
    case eOSWindowMessage_MouseButtonDown:
      niDebugFmt((_A("eOSWindowMessage_MouseButtonDown: %d\n"),a.mU32));
      break;
    case eOSWindowMessage_MouseButtonUp:
      niDebugFmt((_A("eOSWindowMessage_MouseButtonUp: %d\n"),a.mU32));
      break;
    case eOSWindowMessage_MouseButtonDoubleClick:
      niDebugFmt((_A("eOSWindowMessage_MouseButtonDoubleClick: %d\n"),a.mU32));
      if (a.mU32 == 9 /*ePointerButton_DoubleClickRight*/) {
        //                     wnd->SetFullScreen(!wnd->GetFullScreen());
      }
      else if (a.mU32 == 8 /*ePointerButton_DoubleClickLeft*/) {
        //                     wnd->SetFullScreen(!wnd->GetFullScreen());
      }
      break;
    case eOSWindowMessage_MouseWheel:
      niDebugFmt((_A("eOSWindowMessage_MouseWheel: %f, %f\n"),a.GetFloatValue(),b.GetFloatValue()));
      break;
    case eOSWindowMessage_LostFocus: {
      niDebugFmt((_A("eOSWindowMessage_LostFocus: \n")));
      break;
    }
    case eOSWindowMessage_SetFocus:
      niDebugFmt((_A("eOSWindowMessage_SetFocus: \n")));
      break;
    case eOSWindowMessage_Drop:
      niDebugFmt((_A("eOSWindowMessage_Drop: \n")));
      break;

    case eOSWindowMessage_RelativeMouseMove:
#ifdef TRACE_MOUSE_MOVE
      niDebugFmt((_A("eOSWindowMessage_RelativeMouseMove: [%d,%d]\n"),
                  a.mV2L[0],a.mV2L[1]));
#endif
      break;

    case eOSWindowMessage_MouseMove:
#ifdef TRACE_MOUSE_MOVE
      niDebugFmt((_A("eOSWindowMessage_MouseMove: [%d,%d] [%d,%d] (contentsScale: %g)\n"),
                  a.mV2L[0],a.mV2L[1],
                  b.mV2L[0],b.mV2L[1],
                  wnd->GetContentsScale()));
#endif
      break;
  }
}

}
