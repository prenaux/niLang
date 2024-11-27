#include "stdafx.h"
#ifdef niOSX

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <MetalKit/MetalKit.h>
#include <OpenGL/gl.h>
#include "../src/API/niLang/Platforms/OSX/osxgl.h"
#include "../src/API/niLang/Platforms/OSX/osxMetal.h"
#include "../src/API/niLang_ModuleDef.h"
#include "../src/Platform_OSX.h"

// #define TRACE_MOUSE_MOVE

namespace ni {

struct FOSWindowOSXGL {
};

struct sTestOSXWindowSink : public ImplRC<iMessageHandler> {
  const tU64 _threadId;
  iOSWindow* _w;
  sTestOSXWindowSink(iOSWindow* aWindow)
      : _threadId(ni::ThreadGetCurrentThreadID())
      , _w(aWindow)
  {}

  tU64 __stdcall GetThreadID() const {
    return _threadId;
  }

  void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
    switch (anMsg) {
      case eOSWindowMessage_Close:
        niDebugFmt((_A("eOSWindowMessage_Close: \n")));
        _w->Invalidate();
        break;
      case eOSWindowMessage_SwitchIn:
        niDebugFmt((_A("eOSWindowMessage_SwitchIn: \n")));
        break;
      case eOSWindowMessage_SwitchOut:
        niDebugFmt((_A("eOSWindowMessage_SwitchOut: \n")));
        break;
      case eOSWindowMessage_Paint: {
        // niDebugFmt((_A("eOSWindowMessage_Paint: \n")));
        break;
      }
      case eOSWindowMessage_Size:
        niDebugFmt((_A("eOSWindowMessage_Size: %s\n"),_ASZ(_w->GetSize())));
        break;
      case eOSWindowMessage_Move:
        niDebugFmt((_A("eOSWindowMessage_Move: %s\n"),_ASZ(_w->GetPosition())));
        break;
      case eOSWindowMessage_KeyDown:
        niDebugFmt((_A("eOSWindowMessage_KeyDown: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
        switch (a.mU32) {
          case eKey_F:
            _w->SetFullScreen(_w->GetFullScreen() == eInvalidHandle ? 0 : eInvalidHandle);
            break;
          case eKey_Z:
            _w->SetCursorPosition(sVec2i::Zero());
            break;
          case eKey_X:
            _w->SetCursorPosition(_w->GetClientSize()/2);
            break;
          case eKey_C:
            _w->SetCursorPosition(_w->GetClientSize());
            break;
          case eKey_Q:
            _w->SetCursor(eOSCursor_None);
            break;
          case eKey_W:
            _w->SetCursor(eOSCursor_Arrow);
            break;
          case eKey_E:
            _w->SetCursor(eOSCursor_Text);
            break;
          case eKey_K:
            _w->SetCursorCapture(!_w->GetCursorCapture());
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
          //                     _w->SetFullScreen(!_w->GetFullScreen());
        }
        else if (a.mU32 == 8 /*ePointerButton_DoubleClickLeft*/) {
          //                     _w->SetFullScreen(!_w->GetFullScreen());
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
                    _w->GetContentsScale()));
#endif
        break;
    }
  }
};

TEST_FIXTURE(FOSWindowOSXGL,Basic) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK_RETURN_IF_FAILED(wnd.IsOK());

  Ptr<sTestOSXWindowSink> sink = niNew sTestOSXWindowSink(wnd);
  wnd->GetMessageHandlers()->AddSink(sink.ptr());

  if (!osxglIsStarted()) {
    CHECK(osxglStartup());
  }

  sOSXGLConfig glConfig;
  CHECK_RETURN_IF_FAILED(osxglCreateContext(wnd,&glConfig));
  CHECK_RETURN_IF_FAILED(osxglHasContext(wnd));
  osxglMakeContextCurrent(wnd);
  CHECK_RETURN_IF_FAILED(osxglGetCurrentContext() == wnd);

  struct sOpenGLWindowSink : public ImplRC<iMessageHandler> {
    const tU32 _threadId;
    WeakPtr<iOSWindow> _wnd;

    sOpenGLWindowSink(iOSWindow* apWnd) : _wnd(apWnd), _threadId(ni::ThreadGetCurrentThreadID()) {
    }

    tU64 __stdcall GetThreadID() const {
      return _threadId;
    }

    void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
      if (anMsg != eOSWindowMessage_Paint)
        return;
      QPtr<iOSWindow> wnd = _wnd;

      const sVec2i size = wnd->GetClientSize();

      glViewport( 0, 0, size.x, size.y );

      glMatrixMode( GL_PROJECTION );
      sMatrixf mtx;
      MatrixPerspectiveFovRH<tF32>(mtx, niRadf(45.0f), (tF32)size.x/(tF32)size.y, 0.1f, 100.0f);
      glLoadMatrixf(mtx.ptr());

      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();

      glColor4f(1,1,1,1);

      glTranslatef( -1.5f, 0.0f, -6.0f );   // Left 1.5 units, into screen 6.0

      glBegin( GL_TRIANGLES );             // Draw a triangle
      glVertex3f(  0.0f,  1.0f, 0.0f );    // Top
      glVertex3f( -1.0f, -1.0f, 0.0f );    // Bottom left
      glVertex3f(  1.0f, -1.0f, 0.0f );    // Bottom right
      glEnd();                             // Done with the triangle

      glTranslatef( 3.0f, 0.0f, 0.0f );    // Move right 3 units

      glBegin( GL_QUADS );                // Draw a quad
      glVertex3f( -1.0f,  1.0f, 0.0f );   // Top left
      glVertex3f(  1.0f,  1.0f, 0.0f );   // Top right
      glVertex3f(  1.0f, -1.0f, 0.0f );   // Bottom right
      glVertex3f( -1.0f, -1.0f, 0.0f );   // Bottom left
      glEnd();                            // Quad is complete

      osxglSwapBuffers(wnd,eFalse);
    }
  };
  wnd->GetMessageHandlers()->AddSink(niNew sOpenGLWindowSink(wnd));

  if (isInteractive) {
    wnd->CenterWindow();
    wnd->SetShow(eOSWindowShowFlags_Show);
    wnd->ActivateWindow();
    while (!wnd->GetRequestedClose()) {
      wnd->UpdateWindow(eTrue);
      mq->PollAndDispatch();
    }
    CHECK(osxglGetCurrentContext() == NULL);
    CHECK(!osxglHasContext(wnd));
  }
  else {
    osxglMakeContextCurrent(NULL);
    CHECK(osxglGetCurrentContext() == NULL);
    CHECK(osxglHasContext(wnd));
    osxglDestroyContext(wnd);
    CHECK(!osxglHasContext(wnd));
  }
}

} // end of namespace ni
#endif
