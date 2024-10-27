#include "stdafx.h"
#ifdef niOSX

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <OpenGL/gl.h>
#include "../src/API/niLang/Platforms/OSX/osxgl.h"
#include "../src/API/niLang/Platforms/OSX/osxMetal.h"
#include "../src/API/niLang/Math/MathMatrix.h"
#include "../src/API/niLang_ModuleDef.h"
#include "../src/Platform_OSX.h"
#include "../../niUI/src/API/niUI/GraphicsEnum.h"
#include "../../niUI/src/API/niUI/FVF.h"
#include "shaders/_allShaders.osx.metallib.h"

namespace ni {

struct FOSWindowOSX {
};

struct sTestOSXWindowSink : public ImplRC<iMessageHandler> {
  iOSWindow* w;
  sTestOSXWindowSink(iOSWindow* _w) : w(_w) {}

  tU64 __stdcall GetThreadID() const {
    return ni::ThreadGetCurrentThreadID();
  }

  void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
    switch (anMsg) {
      case eOSWindowMessage_Close:
        niDebugFmt((_A("eOSWindowMessage_Close: \n")));
        w->Invalidate();
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
        niDebugFmt((_A("eOSWindowMessage_Size: %s\n"),_ASZ(w->GetSize())));
        break;
      case eOSWindowMessage_Move:
        niDebugFmt((_A("eOSWindowMessage_Move: %s\n"),_ASZ(w->GetPosition())));
        break;
      case eOSWindowMessage_KeyDown:
        niDebugFmt((_A("eOSWindowMessage_KeyDown: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
        switch (a.mU32) {
          case eKey_F:
            w->SetFullScreen(w->GetFullScreen() == eInvalidHandle ? 0 : eInvalidHandle);
            break;
          case eKey_Z:
            w->SetCursorPosition(sVec2i::Zero());
            break;
          case eKey_X:
            w->SetCursorPosition(w->GetClientSize()/2);
            break;
          case eKey_C:
            w->SetCursorPosition(w->GetClientSize());
            break;
          case eKey_Q:
            w->SetCursor(eOSCursor_None);
            break;
          case eKey_W:
            w->SetCursor(eOSCursor_Arrow);
            break;
          case eKey_E:
            w->SetCursor(eOSCursor_Text);
            break;
          case eKey_K:
            w->SetCursorCapture(!w->GetCursorCapture());
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
          //                     w->SetFullScreen(!w->GetFullScreen());
        }
        else if (a.mU32 == 8 /*ePointerButton_DoubleClickLeft*/) {
          //                     w->SetFullScreen(!w->GetFullScreen());
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
        niDebugFmt((_A("eOSWindowMessage_RelativeMouseMove: [%d,%d]\n"),
                    a.mV2L[0],a.mV2L[1]));
        break;

      case eOSWindowMessage_MouseMove:
        niDebugFmt((_A("eOSWindowMessage_MouseMove: [%d,%d] [%d,%d] (contentsScale: %g)\n"),
                    a.mV2L[0],a.mV2L[1],
                    b.mV2L[0],b.mV2L[1],
                    w->GetContentsScale()));
        break;
    }
  }
};

TEST_FIXTURE(FOSWindowOSX,OpenGL) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK(wnd.IsOK());

  Ptr<sTestOSXWindowSink> sink = niNew sTestOSXWindowSink(wnd);
  wnd->GetMessageHandlers()->AddSink(sink.ptr());

  if (!osxglIsStarted()) {
    CHECK(osxglStartup());
  }

  sOSXGLConfig glConfig;
  CHECK(osxglCreateContext(wnd,&glConfig));
  CHECK(osxglHasContext(wnd));
  osxglMakeContextCurrent(wnd);
  CHECK(osxglGetCurrentContext() == wnd);

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

TEST_FIXTURE(FOSWindowOSX,Metal) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK(wnd.IsOK());

  Ptr<sTestOSXWindowSink> sink = niNew sTestOSXWindowSink(wnd);
  wnd->GetMessageHandlers()->AddSink(sink.ptr());

  struct sMetalWindowSink : public ImplRC<iMessageHandler> {
    const tU32 _threadId;
    Ptr<iOSXMetalAPI> metalAPI;
    id<MTLCommandQueue> commandQueue;
    id<MTLRenderPipelineState> pipeline;
    id<MTLBuffer> vertexBuffer;

    sMetalWindowSink() : _threadId(ni::ThreadGetCurrentThreadID()) {
    }

    ~sMetalWindowSink() {
      niDebugFmt(("... ~sMetalWindowSink"));
    }

    tU64 __stdcall GetThreadID() const {
      return _threadId;
    }

    tBool Init(iOSWindow* apWnd) {
      osxMetalSetDefaultDevice();
      metalAPI = osxMetalCreateAPIForWindow(osxMetalGetDevice(),apWnd);
      niCheck(metalAPI.IsOK(),eFalse);

      id<MTLDevice> device = (__bridge id<MTLDevice>)metalAPI->GetDevice();

      // Triangle, red a the top, green on the left, blue on the right
      // (important to make sure rgb/bgr aren't swapped)
      sVertexPA vertices[3];
      vertices[0].pos = {  0.0,  0.5, 0 }; vertices[0].colora = ULColorBuildf( 1, 0, 0, 1 );
      vertices[1].pos = { -0.5, -0.5, 0 }; vertices[1].colora = ULColorBuildf( 0, 1, 0, 1 );
      vertices[2].pos = {  0.5, -0.5, 0 }; vertices[2].colora = ULColorBuildf( 0, 0, 1, 1 );
      vertexBuffer = [device newBufferWithBytes:vertices
                      length:sizeof(vertices)
                      options:MTLResourceCPUCacheModeDefaultCache];

      NSError *error = nil;
      dispatch_data_t data = dispatch_data_create(_allShaders_DATA, _allShaders_DATA_SIZE, NULL, NULL);
      id<MTLLibrary> library = [device
                                newLibraryWithData:data
                                error:&error];
      if (!library) {
        NSLog(@"Error occurred when compiling shader library: %@", error);
        return eFalse;
      }

      id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
      id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];

      MTLRenderPipelineDescriptor *pipelineDescriptor = [MTLRenderPipelineDescriptor new];
      pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
      pipelineDescriptor.vertexFunction = vertexFunc;
      pipelineDescriptor.fragmentFunction = fragmentFunc;
      // pipelineDescriptor.vertexDescriptor = _CreateMetalVertDesc(0, sVertexPA::eFVF);

      pipeline = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor
                  error:&error];
      if (!pipeline) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
      }

      commandQueue = [device newCommandQueue];
      return eTrue;
    }

    void Draw() {
      id<MTLRenderCommandEncoder> commandEncoder = (__bridge id<MTLRenderCommandEncoder>)metalAPI->NewRenderCommandEncoder(
        Vec4<tF64>(0,1,1,1), 1.0f, 0);
      [commandEncoder setRenderPipelineState:pipeline];
      [commandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
      [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
      metalAPI->PresentAndCommit(NULL);
    };

    void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
      switch (anMsg) {
        case eOSWindowMessage_Paint: {
          this->Draw();
          break;
        };
      }
    }
  };
  Ptr<sMetalWindowSink> metalSink = niNew sMetalWindowSink();
  CHECK(metalSink->Init(wnd));
  wnd->GetMessageHandlers()->AddSink(metalSink);

  if (isInteractive) {
    wnd->CenterWindow();
    wnd->SetShow(eOSWindowShowFlags_Show);
    wnd->ActivateWindow();
    while (!wnd->GetRequestedClose()) {
      wnd->UpdateWindow(eTrue);
    }
  }
}
} // end of namespace ni
#endif
