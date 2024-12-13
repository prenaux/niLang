#include "stdafx.h"
#ifdef niOSX

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include "../src/API/niLang/Platforms/OSX/osxMetal.h"
#include "../src/API/niLang_ModuleDef.h"
#include "../../niUI/src/API/niUI/FVF.h"
#include <niLang/STL/set.h>
#include "shaders/_allShaders.osx.metallib.h"

// #define TRACE_MOUSE_MOVE

namespace ni {

struct FOSWindowMetal {
};

struct sMetalWindowSink : public ImplRC<iMessageHandler> {
  const tU32 _threadId;
  iOSWindow* _w;
  Ptr<iOSXMetalAPI> _metalAPI;
  id<MTLCommandQueue> _commandQueue;

  sMetalWindowSink(iOSWindow* aWindow)
      : _threadId(ni::ThreadGetCurrentThreadID())
      , _w(aWindow)
  {
  }

  ~sMetalWindowSink() {
  }

  tU64 __stdcall GetThreadID() const {
    return _threadId;
  }

  virtual tBool Init() {
    osxMetalSetDefaultDevice();
    _metalAPI = osxMetalCreateAPIForWindow(osxMetalGetDevice(),_w);
    niCheck(_metalAPI.IsOK(),eFalse);

    id<MTLDevice> device = (__bridge id<MTLDevice>)_metalAPI->GetDevice();

    NSError *error = nil;
    dispatch_data_t data = dispatch_data_create(_allShaders_DATA, _allShaders_DATA_SIZE, NULL, NULL);
    id<MTLLibrary> library = [device
                              newLibraryWithData:data
                              error:&error];
    if (!library) {
      NSLog(@"Error occurred when compiling shader library: %@", error);
      return eFalse;
    }

    _commandQueue = [device newCommandQueue];
    return eTrue;
  }

  virtual void Draw() = 0;

  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
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
      case eOSWindowMessage_Paint: {
        this->Draw();
        break;
      };
    }
  }
};

TEST_FIXTURE(FOSWindowMetal,Clear) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK_RETURN_IF_FAILED(wnd.IsOK());

  struct sMetalClear_MetalWindowSink : public sMetalWindowSink {
    sMetalClear_MetalWindowSink(iOSWindow* w) : sMetalWindowSink(w) {}
    virtual void Draw() {
      id<MTLRenderCommandEncoder> commandEncoder = (__bridge id<MTLRenderCommandEncoder>)_metalAPI->NewRenderCommandEncoder(
        Vec4<tF64>(0,1,1,1), 1.0f, 0);
      _metalAPI->PresentAndCommit(NULL);
    };
  };

  Ptr<sMetalWindowSink> metalSink = niNew sMetalClear_MetalWindowSink(wnd);
  CHECK_RETURN_IF_FAILED(metalSink->Init());
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

TEST_FIXTURE(FOSWindowMetal,Triangle) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK_RETURN_IF_FAILED(wnd.IsOK());

  struct sMetalTriangle_MetalWindowSink : public sMetalWindowSink {
    id<MTLRenderPipelineState> _pipeline;
    id<MTLBuffer> _vertexBuffer;

    sMetalTriangle_MetalWindowSink(iOSWindow* w) : sMetalWindowSink(w) {}

    tBool Init() override {
      niCheck(sMetalWindowSink::Init(),eFalse);
      id<MTLDevice> device = (__bridge id<MTLDevice>)_metalAPI->GetDevice();

      // Triangle, red a the top, green on the left, blue on the right
      // (important to make sure rgb/bgr aren't swapped)
      sVertexPA vertices[3];
      vertices[0].pos = {  0.0,  0.5, 0 }; vertices[0].colora = ULColorBuildf( 1, 0, 0, 1 );
      vertices[1].pos = { -0.5, -0.5, 0 }; vertices[1].colora = ULColorBuildf( 0, 1, 0, 1 );
      vertices[2].pos = {  0.5, -0.5, 0 }; vertices[2].colora = ULColorBuildf( 0, 0, 1, 1 );
      _vertexBuffer = [device newBufferWithBytes:vertices
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

      _pipeline = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor
                   error:&error];
      if (!_pipeline) {
        NSLog(@"Error occurred when creating render pipeline state: %@", error);
      }

      return eTrue;
    }

    void Draw() override {
      id<MTLRenderCommandEncoder> commandEncoder = (__bridge id<MTLRenderCommandEncoder>)_metalAPI->NewRenderCommandEncoder(
        Vec4<tF64>(0,1,1,1), 1.0f, 0);
      [commandEncoder setRenderPipelineState:_pipeline];
      [commandEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
      [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
      _metalAPI->PresentAndCommit(NULL);
    };
  };

  Ptr<sMetalWindowSink> metalSink = niNew sMetalTriangle_MetalWindowSink(wnd);
  CHECK_RETURN_IF_FAILED(metalSink->Init());
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
