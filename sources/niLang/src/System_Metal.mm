#include "API/niLang/Types.h"

#if defined niOSX || defined niIOS
#include "Platform_OSX.h"

#include "Lang.h"

#include "API/niLang/Platforms/OSX/osxMetal.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

using namespace ni;

//--------------------------------------------------------------------------------------------
//
//  Device
//
//--------------------------------------------------------------------------------------------
static id<MTLDevice> _metalDevice;
niExportFunc(tBool) osxMetalSetDefaultDevice() {
  _metalDevice = MTLCreateSystemDefaultDevice();
  return _metalDevice != NULL;
}
niExportFunc(tBool) osxMetalSetDevice(void* apDevice) {
  _metalDevice = (__bridge id<MTLDevice>)apDevice;
  return _metalDevice != NULL;
}
niExportFunc(void*) osxMetalGetDevice() {
  return (__bridge void*)_metalDevice;
}

//--------------------------------------------------------------------------------------------
//
//  Explicit MTKView
//
//--------------------------------------------------------------------------------------------
struct sMetalAPIForMTKView : public ImplRC<iOSXMetalAPI> {
  id<MTLDevice>               _device;
  id<MTLCommandQueue>         _commandQueue;
  id<MTLCommandBuffer>        _commandBuffer;
  id<MTLRenderCommandEncoder> _commandEncoder;
  MTKView*                    _mtkView;

  sMetalAPIForMTKView(void* apMetalDevice, MTKView* apView) {
    _device = (__bridge id<MTLDevice>)apMetalDevice;
    _mtkView = apView;
    _mtkView.autoResizeDrawable = YES;
    _commandQueue = [_device newCommandQueue];
  }

  virtual const achar* __stdcall GetName() const {
    return "Metal";
  }
  virtual void* __stdcall GetDevice() const niImpl {
    return (__bridge void*)_device;
  }
  virtual void* __stdcall GetCommandQueue() const niImpl {
    return (__bridge void*)_commandQueue;
  }
  virtual void* __stdcall GetMTKView() const niImpl {
    return (__bridge void*)_mtkView;
  }
  virtual void* __stdcall NewRenderCommandEncoder(const double4& aClearColor, tF32 aClearDepth, tU32 aClearStencil) niImpl {
    _commandBuffer = [_commandQueue commandBuffer];
    MTLRenderPassDescriptor* passDesc = [_mtkView currentRenderPassDescriptor];
    niCAssert(sizeof(MTLClearColor) == sizeof(aClearColor));
    passDesc.colorAttachments[0].clearColor = (MTLClearColor&)aClearColor;
    passDesc.depthAttachment.clearDepth = aClearDepth;
    passDesc.stencilAttachment.clearStencil = aClearStencil;
    _commandEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    return (__bridge void*)_commandEncoder;
  }
  virtual tBool __stdcall PresentAndCommit(iRunnable* apOnCompleted) niImpl {
    const tBool r = (_commandBuffer != NULL);
    if (_commandEncoder) {
      [_commandEncoder endEncoding];
    }

    if (_commandBuffer) {
      [_commandBuffer presentDrawable:[_mtkView currentDrawable]];
      if (apOnCompleted) {
        ni::Ptr<iRunnable> onCompleted = apOnCompleted;
        [_commandBuffer addCompletedHandler: ^(id<MTLCommandBuffer> _Nonnull) {
            onCompleted->Run();
          }];
      }
      [_commandBuffer commit];
    }
    _commandEncoder = NULL;
    _commandBuffer = NULL;
    return r;
  }
};

niExportFunc(iOSXMetalAPI*) osxMetalCreateAPIForMTKView(void* apDevice, void* apMTKView) {
  niCheck(apDevice!=NULL,NULL);
  niCheck(apMTKView!=NULL,NULL);
  return niNew sMetalAPIForMTKView(apDevice,(__bridge MTKView*)apMTKView);
}

//--------------------------------------------------------------------------------------------
//
//  OSX
//
//--------------------------------------------------------------------------------------------
#ifdef niOSX

struct sMetalAPIForWindow;

@interface NIMTKViewDelegate : NSObject<MTKViewDelegate>
- (id)initWithContext:(sMetalAPIForWindow*)apContext;
@end

struct sMetalAPIForWindow : public ImplRC<iOSXMetalAPI> {
  id<MTLDevice>            _device;
  ni::WeakPtr<iOSWindow>   _window;
  MTKView*                 _mtkView;
  NIMTKViewDelegate*       _mtkViewDelegate;
  id<MTLCommandQueue>      _commandQueue;
  id<MTLCommandBuffer>     _commandBuffer;
  id<MTLRenderCommandEncoder> _commandEncoder;

  sMetalAPIForWindow(void* apMetalDevice, iOSWindow* apWindow) {
    QPtr<iOSWindowOSX> osxWindow = apWindow;
    if (!osxWindow.IsOK() || !osxWindow->GetNSWindow()) {
      niError("Not an OSX window.");
      return;
    }

    _device = (__bridge id<MTLDevice>)apMetalDevice;
    _window = apWindow;
    _commandQueue = [_device newCommandQueue];

    const sVec2i wndSize = apWindow->GetRect().GetSize();
    NSWindow* nsWindow = (__bridge NSWindow*)osxWindow->GetNSWindow();
    this->_mtkView = [[MTKView alloc] initWithFrame:CGRectMake(0,0,wndSize.x,wndSize.y) device:this->_device];
    this->_mtkViewDelegate = [[NIMTKViewDelegate alloc] initWithContext:this];
    this->_mtkView.delegate = this->_mtkViewDelegate;
    [[nsWindow contentView] addSubview:this->_mtkView];
  }
  ~sMetalAPIForWindow() {
    Invalidate();
  }

  tBool __stdcall IsOK() const niImpl {
    return _mtkView != NULL;
  }

  void __stdcall Invalidate() niImpl {
    if (!_mtkView)
      return;
    [_mtkView removeFromSuperview];
    _mtkView = NULL;
  }

  virtual const achar* __stdcall GetName() const {
    return "Metal";
  }
  virtual void* __stdcall GetDevice() const niImpl {
    return (__bridge void*)_device;
  }
  virtual void* __stdcall GetCommandQueue() const niImpl {
    return (__bridge void*)_commandQueue;
  }
  virtual void* __stdcall GetMTKView() const niImpl {
    return (__bridge void*)_mtkView;
  }
  virtual void* __stdcall NewRenderCommandEncoder(const double4& aClearColor, tF32 aClearDepth, tU32 aClearStencil) niImpl {
    _commandBuffer = [_commandQueue commandBuffer];
    MTLRenderPassDescriptor* passDesc = [_mtkView currentRenderPassDescriptor];
    niCAssert(sizeof(MTLClearColor) == sizeof(aClearColor));
    passDesc.colorAttachments[0].clearColor = (MTLClearColor&)aClearColor;
    passDesc.depthAttachment.clearDepth = aClearDepth;
    passDesc.stencilAttachment.clearStencil = aClearStencil;
    _commandEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    return (__bridge void*)_commandEncoder;
  }
  virtual tBool __stdcall PresentAndCommit(iRunnable* apOnCompleted) niImpl {
    const tBool r = (_commandBuffer != NULL);
    if (_commandEncoder) {
      [_commandEncoder endEncoding];
    }
    if (_commandBuffer) {
      [_commandBuffer presentDrawable:[_mtkView currentDrawable]];
      if (apOnCompleted) {
        ni::Ptr<iRunnable> onCompleted = apOnCompleted;
        [_commandBuffer addCompletedHandler: ^(id<MTLCommandBuffer> _Nonnull) {
          onCompleted->Run();
        }];
      }
      [_commandBuffer commit];
    }
    _commandEncoder = NULL;
    _commandBuffer = NULL;
    return r;
  }
};

@implementation NIMTKViewDelegate

sMetalAPIForWindow* _context;

- (id)initWithContext:(sMetalAPIForWindow*)apContext {
  self = [super init];
  _context = apContext;
  return self;
}

- (void)drawInMTKView:(MTKView *)view {
  // niDebugFmt(("... drawInMTKView"));
  if (_context) {
    QPtr<iOSWindow> wnd = _context->_window;
    if (wnd.IsOK()) {
      const sVec2i wndSize = wnd->GetRect().GetSize();
      if (wndSize.x != view.frame.size.width ||
          wndSize.y != view.frame.size.height)
      {
        view.frame = CGRectMake(0,0,wndSize.x,wndSize.y);
      }
      ni::SendMessages(wnd->GetMessageHandlers(),eOSWindowMessage_Paint,niVarNull,niVarNull);
    }
  }
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
  // niDebugFmt(("... MTKView drawableSizeWillChange: %d, %d", size.width, size.height));
}

@end

niExportFunc(iOSXMetalAPI*) osxMetalCreateAPIForWindow(void* apDevice, iOSWindow* apWindow) {
  niCheck(apDevice!=NULL,NULL);
  niCheckIsOK(apWindow,NULL);
  return niNew sMetalAPIForWindow(apDevice,apWindow);
}

#endif // niOSX

#endif // #if defined niOSX || defined niIOS
