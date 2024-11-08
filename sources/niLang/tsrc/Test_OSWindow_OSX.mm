#include "stdafx.h"
#ifdef niOSX

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <MetalKit/MetalKit.h>
#include <OpenGL/gl.h>
#include "../src/API/niLang/Platforms/OSX/osxgl.h"
#include "../src/API/niLang/Platforms/OSX/osxMetal.h"
#include "../src/API/niLang/Math/MathMatrix.h"
#include "../src/API/niLang_ModuleDef.h"
#include "../src/Platform_OSX.h"
#include "../../niUI/src/API/niUI/GraphicsEnum.h"
#include "../../niUI/src/API/niUI/FVF.h"
#include <niLang/STL/set.h>
#include "shaders/_allShaders.osx.metallib.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>
#include <MoltenVK/mvk_vulkan.h>
#include "../../niUI/src/API/niUI/IVertexArray.h"
#include "../../niUI/src/API/niUI/IIndexArray.h"
#define niVulkanMemoryAllocator_Implement
#include "../../thirdparty/VulkanMemoryAllocator/niVulkanMemoryAllocator.h"
#include "shaders/triangle_vs.spv.h"
#include "shaders/triangle_ps.spv.h"
#include "../../niUI/tsrc/data/A.jpg.hxx"
#include "../../niUI/src/API/niUI/IGraphics.h"
#include "shaders/texture_vs.spv.h"
#include "shaders/texture_ps.spv.h"

// #define TRACE_MOUSE_MOVE

namespace ni {

struct FOSWindowOSX {
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

TEST_FIXTURE(FOSWindowOSX,OpenGL) {
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

struct sMetalWindowSink : public ImplRC<iMessageHandler> {
  const tU32 _threadId;
  Ptr<iOSXMetalAPI> _metalAPI;
  id<MTLCommandQueue> _commandQueue;

  sMetalWindowSink() : _threadId(ni::ThreadGetCurrentThreadID()) {
  }

  ~sMetalWindowSink() {
  }

  tU64 __stdcall GetThreadID() const {
    return _threadId;
  }

  virtual tBool Init(iOSWindow* apWnd) {
    osxMetalSetDefaultDevice();
    _metalAPI = osxMetalCreateAPIForWindow(osxMetalGetDevice(),apWnd);
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
      case eOSWindowMessage_Paint: {
        this->Draw();
        break;
      };
    }
  }
};

TEST_FIXTURE(FOSWindowOSX,MetalClear) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK_RETURN_IF_FAILED(wnd.IsOK());

  Ptr<sTestOSXWindowSink> sink = niNew sTestOSXWindowSink(wnd);
  wnd->GetMessageHandlers()->AddSink(sink.ptr());

  struct sMetalClear_MetalWindowSink : public sMetalWindowSink {
    virtual void Draw() {
      id<MTLRenderCommandEncoder> commandEncoder = (__bridge id<MTLRenderCommandEncoder>)_metalAPI->NewRenderCommandEncoder(
        Vec4<tF64>(0,1,1,1), 1.0f, 0);
      _metalAPI->PresentAndCommit(NULL);
    };
  };

  Ptr<sMetalWindowSink> metalSink = niNew sMetalClear_MetalWindowSink();
  CHECK_RETURN_IF_FAILED(metalSink->Init(wnd));
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

TEST_FIXTURE(FOSWindowOSX,MetalTriangle) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK_RETURN_IF_FAILED(wnd.IsOK());

  Ptr<sTestOSXWindowSink> sink = niNew sTestOSXWindowSink(wnd);
  wnd->GetMessageHandlers()->AddSink(sink.ptr());

  struct sMetalTriangle_MetalWindowSink : public sMetalWindowSink {
    id<MTLRenderPipelineState> _pipeline;
    id<MTLBuffer> _vertexBuffer;

    sMetalTriangle_MetalWindowSink() {
    }

    tBool Init(iOSWindow* apWnd) override {
      niCheck(sMetalWindowSink::Init(apWnd),eFalse);
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

  Ptr<sMetalWindowSink> metalSink = niNew sMetalTriangle_MetalWindowSink();
  CHECK_RETURN_IF_FAILED(metalSink->Init(wnd));
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

#define VULKAN_TRACE(aFmt) niDebugFmt(aFmt)

static VkPrimitiveTopology Vulkan_GetPrimitiveType(eGraphicsPrimitiveType aType) {
  switch (aType) {
    case eGraphicsPrimitiveType_PointList:
      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case eGraphicsPrimitiveType_LineList:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case eGraphicsPrimitiveType_LineStrip:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case eGraphicsPrimitiveType_TriangleList:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case eGraphicsPrimitiveType_TriangleStrip:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case eGraphicsPrimitiveType_LineListAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case eGraphicsPrimitiveType_LineStripAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case eGraphicsPrimitiveType_TriangleListAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case eGraphicsPrimitiveType_TriangleStripAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    default:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  }
}

static astl::vector<VkVertexInputAttributeDescription> Vulkan_CreateVertexInputDesc(tFVF aFVF) {
  astl::vector<VkVertexInputAttributeDescription> attrs;
  tU32 location = 0;
  tU32 offset = 0;

  if (eFVF_HasPosition(aFVF)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offset
      });
    offset += sizeof(sVec3f);

    if (eFVF_HasWeights(aFVF)) {
      const tU32 numWeights = eFVF_NumWeights(aFVF);
      switch (numWeights) {
        case 1:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32);
          break;
        case 2:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32G32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32) * 2;
          break;
        case 3:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32G32B32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32) * 3;
          break;
        case 4:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32G32B32A32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32) * 4;
          break;
        default:
          niPanicUnreachable(niFmt("Unexpected number of vertex weights '%d'.",numWeights));
          break;
      }
    }
  }

  if (niFlagIs(aFVF,eFVF_Indices)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32_UINT,
        .offset = offset
      });
    offset += sizeof(tU32);
  }

  if (niFlagIs(aFVF,eFVF_Normal)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offset
      });
    offset += sizeof(sVec3f);
  }

  if (niFlagIs(aFVF,eFVF_ColorA)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32_UINT,
        .offset = offset
      });
    offset += sizeof(tU32);
  }

  for (tU32 i = 0; i < eFVF_TexNumCoo(aFVF); ++i) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offset
      });
    offset += sizeof(sVec2f);
  }

  return attrs;
}

struct sVulkanDriver {
  VkDevice _device = VK_NULL_HANDLE;
  VmaAllocator _allocator = nullptr;
  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkCommandPool _commandPool = VK_NULL_HANDLE;

  VkCommandBuffer BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandPool = _commandPool,
      .commandBufferCount = 1
    };

    VkCommandBuffer cmdBuf;
    vkAllocateCommandBuffers(_device, &allocInfo, &cmdBuf);

    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(cmdBuf, &beginInfo);
    return cmdBuf;
  }

  void EndSingleTimeCommands(VkCommandBuffer cmdBuf) {
    vkEndCommandBuffer(cmdBuf);

    VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmdBuf
    };

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &cmdBuf);
  }
};

struct sVulkanFramebuffer {
  VkImage _image = VK_NULL_HANDLE;
  VkImageView _view = VK_NULL_HANDLE;
  tU32 _width = 0;
  tU32 _height = 0;

  tBool CreateFromMTKView(MTKView* apView, VkDevice aDevice) {
    MTLRenderPassDescriptor* passDesc = apView.currentRenderPassDescriptor;
    niCheck(passDesc != nullptr, eFalse);

    id<MTLTexture> texture = passDesc.colorAttachments[0].texture;
    _width = texture.width;
    _height = texture.height;

#if 0
    niLog(Info,niFmt("Creating Vulkan framebuffer: %dx%d from MTLTexture (format: %lu, usage: %lu)",
                     _width, _height,
                     texture.pixelFormat,
                     texture.usage));
#endif

    VkImportMetalTextureInfoEXT importInfo = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT,
      .plane = VK_IMAGE_ASPECT_COLOR_BIT,
      .mtlTexture = texture
    };

    VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = &importInfo,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_B8G8R8A8_UNORM,
      .extent = {_width, _height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

    niCheck(vkCreateImage(aDevice, &imageInfo, nullptr, &_image) == VK_SUCCESS, eFalse);

    VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = _image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_B8G8R8A8_UNORM,
      .components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY
      },
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    return vkCreateImageView(aDevice, &viewInfo, nullptr, &_view) == VK_SUCCESS;
  }

  void Destroy(VkDevice device) {
    if (_view) {
      vkDestroyImageView(device, _view, nullptr);
      _view = VK_NULL_HANDLE;
    }
    if (_image) {
      vkDestroyImage(device, _image, nullptr);
      _image = VK_NULL_HANDLE;
    }
  }
};

struct sVulkanBuffer {
  nn_mut<sVulkanDriver> _driver;
  VkBuffer _vkBuffer = VK_NULL_HANDLE;
  VmaAllocation _vmaAllocation = nullptr;
  tBool _locked = eFalse;
  const eArrayUsage _usage;

  sVulkanBuffer(ain_nn_mut<sVulkanDriver> aDriver, eArrayUsage aUsage)
      : _driver(aDriver)
      , _usage(aUsage)
  {}

  tBool Create(tU32 anSize, VkBufferUsageFlags aUsage) {
    VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = anSize,
      .usage = aUsage
    };

    VmaAllocationCreateInfo allocInfo = {
      .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    niCheck(vmaCreateBuffer(
      _driver->_allocator, &bufferInfo, &allocInfo,
      &_vkBuffer, &_vmaAllocation, nullptr) == VK_SUCCESS, eFalse);

    return eTrue;
  }

  void Destroy() {
    if (_locked) {
      Unlock();
    }
    if (_vkBuffer) {
      vmaDestroyBuffer(_driver->_allocator, _vkBuffer, _vmaAllocation);
      _vkBuffer = VK_NULL_HANDLE;
      _vmaAllocation = nullptr;
    }
  }

  tBool IsOK() const {
    return _vkBuffer != VK_NULL_HANDLE;
  }

  tPtr Lock(tU32 anStart, tU32 anSize, eLock aLock) {
    niCheck(!_locked,nullptr);

    // TODO: Bad (but should be correct) as it stalls the whole pipeline. Only
    // for the first implementation. Take a look at
    // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html#usage_patterns_advanced_data_uploading
    // to do something better for dynamic arrays.
    vkDeviceWaitIdle(_driver->_device);

    tPtr mappedData;
    niCheck(vmaMapMemory(_driver->_allocator, _vmaAllocation, (void**)&mappedData) == VK_SUCCESS, nullptr);
    _locked = eTrue;
    return mappedData + anStart;
  }

  tBool Unlock() {
    niCheck(_locked,eFalse);
    vmaUnmapMemory(_driver->_allocator, _vmaAllocation);
    _locked = eFalse;
    return eTrue;
  }
};

struct sVulkanVertexArray : public ImplRC<iVertexArray> {
  niBeginClass(sVulkanVertexArray);

  sVulkanBuffer _buffer;
  const cFVFDescription _fvf;
  const tU32 _numVertices;

  sVulkanVertexArray(ain_nn_mut<sVulkanDriver> aDriver, tU32 aNumVertices, tU32 aFVF, eArrayUsage aUsage)
      : _buffer(aDriver, aUsage)
      , _numVertices(aNumVertices)
      , _fvf(aFVF)
  {
    niCheck(_buffer.Create(
      _numVertices * _fvf.GetStride(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), ;);

    VULKAN_TRACE((
      ">>> MetalVertexArray: FVF:%s, NumVertex: %d, Stride: %d, Size: %s.",
      FVFToString(_fvf.GetFVF()).Chars(),
      this->GetNumVertices(),_fvf.GetStride(),
      StringBytesToReadableSize(_fvf.GetStride() * _numVertices * 12357, eTrue)));
  }

  ~sVulkanVertexArray() {
    _buffer.Destroy();
  }

  tBool __stdcall IsOK() const override {
    niClassIsOK(sVulkanVertexArray);
    return _buffer.IsOK();
  }

  tFVF __stdcall GetFVF() const override {
    return _fvf.GetFVF();
  }

  tU32 __stdcall GetNumVertices() const override {
    return _numVertices;
  }

  eArrayUsage __stdcall GetUsage() const override {
    return eArrayUsage_Dynamic;
  }

  tPtr __stdcall Lock(tU32 aFirstVertex, tU32 aNumVertex, eLock aLock) override {
    const tU32 offset = aFirstVertex * _fvf.GetStride();
    const tU32 size = (aNumVertex ? aNumVertex : _numVertices) * _fvf.GetStride();
    return _buffer.Lock(offset,size,aLock);
  }

  tBool __stdcall Unlock() override {
    return _buffer.Unlock();
  }

  tBool __stdcall GetIsLocked() const override {
    return _buffer._locked;
  }

  iHString* __stdcall GetDeviceResourceName() const override {
    return NULL;
  }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) override {
    return eFalse;
  }
  tBool __stdcall ResetDeviceResource() override {
    return eFalse;
  }
  iDeviceResource* __stdcall Bind(iUnknown*) override {
    return this;
  }

  niEndClass(sVulkanVertexArray);
};

const tU32 knVulkanIndexSize = sizeof(tU32);

struct sVulkanIndexArray : public ni::ImplRC<iIndexArray> {
  sVulkanBuffer _buffer;
  const eGraphicsPrimitiveType _primType;
  const tU32 _numIndices;
  const VkIndexType  _indexType = VK_INDEX_TYPE_UINT32;

  sVulkanIndexArray(ain_nn_mut<sVulkanDriver> aDriver, eGraphicsPrimitiveType aPrimType, tU32 anNumIndices, eArrayUsage aUsage)
      : _buffer(aDriver, aUsage)
      , _primType(aPrimType)
      , _numIndices(anNumIndices)
  {
    niCheck(_buffer.Create(
      knVulkanIndexSize * _numIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), ;);

    niDebugFmt((
      ">>> VulkanIndexArray: PT: %s, MaxVertexIndex:%d, NumIndices: %d, Stride: %d, Size: %s.",
      _primType,
      0xFFFFFFFF,
      this->GetNumIndices(), knVulkanIndexSize,
      StringBytesToReadableSize(knVulkanIndexSize * _numIndices)));
  }

  ~sVulkanIndexArray() {
    _buffer.Destroy();
  }

  tBool __stdcall IsOK() const niImpl {
    return _buffer.IsOK();
  }

  iHString* __stdcall GetDeviceResourceName() const niImpl {
    return NULL;
  }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }
  iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    return this;
  }

  virtual eGraphicsPrimitiveType __stdcall GetPrimitiveType() const niImpl {
    return _primType;
  }
  virtual tU32 __stdcall GetNumIndices() const niImpl {
    return _numIndices;
  }
  virtual tU32 __stdcall GetMaxVertexIndex() const niImpl {
    return 0xFFFFFFFF;
  }
  virtual eArrayUsage __stdcall GetUsage() const niImpl {
    return _buffer._usage;
  }

  tPtr __stdcall Lock(tU32 anFirstIndex, tU32 anNumIndex, eLock aLock) niImpl {
    return _buffer.Lock(anFirstIndex * knVulkanIndexSize, anNumIndex * knVulkanIndexSize, aLock);
  }
  tBool __stdcall Unlock() niImpl {
    return _buffer.Unlock();
  }
  virtual tBool __stdcall GetIsLocked() const niImpl {
    return _buffer._locked;
  }
};

struct sVulkanWindowSink : public sVulkanDriver, public ImplRC<iMessageHandler> {
  const tU32 _threadId;
  Ptr<iOSXMetalAPI> _metalAPI;
  VkInstance _instance = VK_NULL_HANDLE;
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE; // This object will be implicitly destroyed when the VkInstance is destroyed.
  typedef astl::map<cString,tU32> tVkExtensionsMap;
  tVkExtensionsMap _extensions;
  VkSurfaceKHR _surface = VK_NULL_HANDLE;
  tBool _enableValidationLayers = eTrue;
  typedef astl::set<cString> tVkInstanceLayersSet;
  tVkInstanceLayersSet _instanceLayers;
  tU32 _queueFamilyIndex = 0;
  VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
  sVulkanFramebuffer _currentFb;
  VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE;
  VkSemaphore _renderFinishedSemaphore = VK_NULL_HANDLE;
  VkFence _inFlightFence = VK_NULL_HANDLE;

  sVulkanWindowSink() : _threadId(ni::ThreadGetCurrentThreadID()) {
  }

  ~sVulkanWindowSink() {
    this->Cleanup();
  }

  tU64 __stdcall GetThreadID() const {
    return _threadId;
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL _DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/) {

    switch (severity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        niLog(Error,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        niLog(Warning,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        niLog(Info,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
      default:
        niLog(Debug,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
    }
    return VK_FALSE;
  }

  tBool _CreateInstance(const achar* aAppName) {
    {
      tU32 layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
      astl::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
      for (const auto& layer : availableLayers) {
        _instanceLayers.insert(layer.layerName);
      }
      {
        cString o;
        niLoopit(tVkInstanceLayersSet::const_iterator,it,_instanceLayers) {
          if (it != _instanceLayers.begin())
            o << ", ";
          o << *it;
        }
        niLog(Info,niFmt("Vulkan instance layers[%d]: %s", _instanceLayers.size(), o));
      }
    }

    astl::vector<const char*> extensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    astl::vector<const char*> layers;
    if (_enableValidationLayers) {
      // NOTE: Statically linking directly MoltenVK we cannot use the
      // validation layer.
      const achar* validationLayerName = "VK_LAYER_KHRONOS_validation";
      if (!astl::contains(_instanceLayers,validationLayerName)) {
        _enableValidationLayers = eFalse;
        niWarning(niFmt("Vulkan validation layer '%s' not found, disabling it.",
                        validationLayerName));
      }
      else {
        layers.push_back(validationLayerName);
      }
    }

    VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = aAppName,
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "niVlk",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_0
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT|
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT),
      .messageType = (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT|
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT|
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
      .pfnUserCallback = _DebugCallback
    };

    VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = (tU32)layers.size(),
      .ppEnabledLayerNames = layers.data(),
      .enabledExtensionCount = (tU32)extensions.size(),
      .ppEnabledExtensionNames = extensions.data()
    };
    createInfo.pNext = _enableValidationLayers ? &debugCreateInfo : nullptr;

    return vkCreateInstance(&createInfo, nullptr, &_instance) == VK_SUCCESS;
  }

  tBool _InitPhysicalDevice() {
    tU32 deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    niCheck(deviceCount > 0, eFalse);

    vkEnumeratePhysicalDevices(_instance, &deviceCount, &_physicalDevice);

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(_physicalDevice, &props);
    niLog(Info,niFmt("Vulkan Device: %s", props.deviceName));
    niLog(Info,niFmt("Vulkan Driver Version: %d.%d.%d",
                     VK_VERSION_MAJOR(props.driverVersion),
                     VK_VERSION_MINOR(props.driverVersion),
                     VK_VERSION_PATCH(props.driverVersion)));
    niLog(Info,niFmt("Vulkan API: %d.%d.%d",
                     VK_VERSION_MAJOR(props.apiVersion),
                     VK_VERSION_MINOR(props.apiVersion),
                     VK_VERSION_PATCH(props.apiVersion)));

    // Get extensions
    {
      tU32 extensionCount = 0;
      vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);
      astl::vector<VkExtensionProperties> extensions(extensionCount);
      vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, extensions.data());
      for (const auto& extension : extensions) {
        astl::upsert(_extensions,extension.extensionName,extension.specVersion);
      }
      {
        cString o;
        niLoopit(tVkExtensionsMap::const_iterator,it,_extensions) {
          if (it != _extensions.begin())
            o << ", ";
          o << it->first << "=" << it->second;
        }
        niLog(Info,niFmt("Vulkan extensions[%d]: %s", _extensions.size(), o));
      }
    }

    return eTrue;
  }

  tBool _CreateSurface() {
    MTKView* mtkView = (__bridge MTKView*)_metalAPI->GetMTKView();
    CAMetalLayer* metalLayer = (CAMetalLayer*)mtkView.layer;
    VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .pLayer = metalLayer
    };
    return vkCreateMetalSurfaceEXT(_instance, &surfaceCreateInfo, nullptr, &_surface) == VK_SUCCESS;
  }

  tBool _FindQueueFamily(tU32& aQueueFamilyIndex) {
    tU32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
    astl::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());

    niLog(Info,"Vulkan Queue Families:");
    for (tU32 i = 0; i < queueFamilyCount; i++) {
      niLog(Info,niFmt("  Family %d:", i));
      niLog(Info,niFmt("    Queue Count: %d", queueFamilies[i].queueCount));
      niLog(Info,niFmt("    Graphics: %s", (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No"));
      niLog(Info,niFmt("    Compute: %s", (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No"));
      niLog(Info,niFmt("    Transfer: %s", (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No"));
      niLog(Info,niFmt("    Sparse: %s", (queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No"));
    }

    for (tU32 i = 0; i < queueFamilyCount; i++) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        aQueueFamilyIndex = i;
        return eTrue;
      }
    }

    return eFalse;
  }

  tBool _CreateLogicalDevice() {
    tF32 queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = _queueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };

    VkDeviceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledExtensionCount = 0,
      .enabledLayerCount = 0
    };

    niCheck(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) == VK_SUCCESS, eFalse);
    vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &_graphicsQueue);
    return eTrue;
  }

  tBool _CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = _queueFamilyIndex,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    };

    niCheck(vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) == VK_SUCCESS, eFalse);

    VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = _commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1
    };

    return vkAllocateCommandBuffers(_device, &allocInfo, &_commandBuffer) == VK_SUCCESS;
  }

  tBool _CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT  // Start signaled so first frame doesn't wait
    };

    niCheck(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore) == VK_SUCCESS, eFalse);
    niCheck(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore) == VK_SUCCESS, eFalse);
    niCheck(vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFence) == VK_SUCCESS, eFalse);

    return eTrue;
  }

  tBool _CreateAllocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _physicalDevice;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    niCheck(vmaCreateAllocator(&allocatorInfo, &_allocator) == VK_SUCCESS, eFalse);
    return eTrue;
  }

  virtual void Cleanup() {
    if (_allocator != nullptr) {
      vmaDestroyAllocator(_allocator);
      _allocator = nullptr;
    }

    if (_inFlightFence) {
      vkDestroyFence(_device, _inFlightFence, nullptr);
      _inFlightFence = VK_NULL_HANDLE;
    }
    if (_renderFinishedSemaphore) {
      vkDestroySemaphore(_device, _renderFinishedSemaphore, nullptr);
      _renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    if (_imageAvailableSemaphore) {
      vkDestroySemaphore(_device, _imageAvailableSemaphore, nullptr);
      _imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    _currentFb.Destroy(_device);

    if (_commandBuffer) {
      vkFreeCommandBuffers(_device, _commandPool, 1, &_commandBuffer);
      _commandBuffer = VK_NULL_HANDLE;
    }
    if (_commandPool) {
      vkDestroyCommandPool(_device, _commandPool, nullptr);
      _commandPool = VK_NULL_HANDLE;
    }
    if (_device) {
      vkDestroyDevice(_device, nullptr);
      _device = VK_NULL_HANDLE;
    }
    if (_surface) {
      vkDestroySurfaceKHR(_instance, _surface, nullptr);
      _surface = VK_NULL_HANDLE;
    }
    if (_instance) {
      vkDestroyInstance(_instance, nullptr);
      _instance = VK_NULL_HANDLE;
    }
  }

  virtual tBool __stdcall Init(iOSWindow* apWnd, const achar* aAppName) {
    osxMetalSetDefaultDevice();
    _metalAPI = osxMetalCreateAPIForWindow(osxMetalGetDevice(),apWnd);
    niCheck(_metalAPI.IsOK(),eFalse);
    niCheck(_CreateInstance(aAppName),eFalse);
    niCheck(_InitPhysicalDevice(),eFalse);
    niCheck(_CreateSurface(),eFalse);
    niCheck(_FindQueueFamily(_queueFamilyIndex), eFalse);
    niLog(Info,niFmt("Vulkan using Queue Family: %d",_queueFamilyIndex));
    niCheck(_CreateLogicalDevice(), eFalse);
    niCheck(_CreateCommandPool(), eFalse);
    niCheck(_CreateSyncObjects(), eFalse);
    niCheck(_CreateAllocator(), eFalse);

    MTKView* mtkView = (__bridge MTKView*)_metalAPI->GetMTKView();
    niCheck(_currentFb.CreateFromMTKView(mtkView, _device), eFalse);

    return eTrue;
  }

  tBool PresentAndCommit() {
    niCheck(vkEndCommandBuffer(_commandBuffer) == VK_SUCCESS, eFalse);

    VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 0,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &_renderFinishedSemaphore,
      .commandBufferCount = 1,
      .pCommandBuffers = &_commandBuffer
    };

    niCheck(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFence) == VK_SUCCESS, eFalse);

    id<CAMetalDrawable> drawable = [(__bridge MTKView*)_metalAPI->GetMTKView() currentDrawable];
    [drawable present];
    return eTrue;
  }

  tBool BeginFrame() {
    niCheck(vkWaitForFences(_device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX) == VK_SUCCESS, eFalse);
    niCheck(vkResetFences(_device, 1, &_inFlightFence) == VK_SUCCESS, eFalse);

    MTKView* mtkView = (__bridge MTKView*)_metalAPI->GetMTKView();
    MTLRenderPassDescriptor* passDesc = mtkView.currentRenderPassDescriptor;
    niCheck(passDesc != nil, eFalse);

    _currentFb.Destroy(_device);
    niCheck(_currentFb.CreateFromMTKView(mtkView, _device), eFalse);

    vkResetCommandBuffer(_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    niCheck(vkBeginCommandBuffer(_commandBuffer, &beginInfo) == VK_SUCCESS, eFalse);
    return eTrue;
  }

  virtual void Draw() = 0;

  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
    switch (anMsg) {
      case eOSWindowMessage_Paint: {
        this->Draw();
        break;
      };
    }
  }
};

TEST_FIXTURE(FOSWindowOSX,VulkanClear) {
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

  struct sVulkanClear_VulkanWindowSink : public sVulkanWindowSink {
    VkClearValue _clearValue = {
      // r, g, b, a
      .color = {{ 0.0f, 1.0f, 1.0f, 1.0f }}
    };
    tF64 _clearTimer = 0.0f;

    void Draw() override {
      niCheck(BeginFrame(),;);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearValue = {
          .color = {{
              (tF32)RandFloat(), // r
              (tF32)RandFloat(), // g
              (tF32)RandFloat(), // b
              1.0f               // a
            }}
        };
        _clearTimer = ni::TimerInSeconds();
      }

      VkRenderingAttachmentInfo colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _currentFb._view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = _clearValue
      };

      VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, {_currentFb._width, _currentFb._height}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
      };

      vkCmdBeginRendering(_commandBuffer, &renderingInfo);
      {
        // Nothing, we're just clearing the buffer
      }
      vkCmdEndRendering(_commandBuffer);

      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> metalSink = niNew sVulkanClear_VulkanWindowSink();
  CHECK_RETURN_IF_FAILED(metalSink->Init(wnd, m_testName));
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

struct sVulkanPipelineVertexPA {
  VkShaderModule _vertexShader = VK_NULL_HANDLE;
  VkShaderModule _pixelShader = VK_NULL_HANDLE;
  VkPipelineLayout _vkPipelineLayout = VK_NULL_HANDLE;
  VkPipeline _vkPipeline = VK_NULL_HANDLE;

  tBool _CreateShaders(VkDevice avkDevice) {
    VkShaderModuleCreateInfo vertInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (size_t)triangle_vs_spv_DATA_SIZE,
      .pCode = (const uint32_t*)triangle_vs_spv_DATA
    };
    niCheck(vkCreateShaderModule(avkDevice, &vertInfo, nullptr, &_vertexShader) == VK_SUCCESS, eFalse);

    VkShaderModuleCreateInfo fragInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (size_t)triangle_ps_spv_DATA_SIZE,
      .pCode = (const uint32_t*)triangle_ps_spv_DATA
    };
    niCheck(vkCreateShaderModule(avkDevice, &fragInfo, nullptr, &_pixelShader) == VK_SUCCESS, eFalse);
    return eTrue;
  }

  tBool _CreatePipeline(VkDevice avkDevice) {
    VkPipelineLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 0
    };
    niCheck(vkCreatePipelineLayout(avkDevice, &layoutInfo, nullptr, &_vkPipelineLayout) == VK_SUCCESS, eFalse);

    VkVertexInputBindingDescription bindingDesc = {
      .binding = 0,
      .stride = sizeof(sVertexPA),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    astl::vector<VkVertexInputAttributeDescription> attrDesc = Vulkan_CreateVertexInputDesc(sVertexPA::eFVF);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDesc,
      .vertexAttributeDescriptionCount = (tU32)attrDesc.size(),
      .pVertexAttributeDescriptions = attrDesc.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE
    };

    VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = dynamicStates
    };

    VkPipelineViewportStateCreateInfo viewportState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = _vertexShader,
        .pName = "main"
      },
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = _pixelShader,
        .pName = "main"
      }
    };

    VkFormat swapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapChainFormat
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pDynamicState = &dynamicState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &colorBlending,
      .layout = _vkPipelineLayout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
      .pNext = &renderingInfo
    };

    niCheck(vkCreateGraphicsPipelines(avkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_vkPipeline) == VK_SUCCESS, eFalse);
    return eTrue;
  }

  tBool Init(VkDevice aDevice) {
    niCheck(_CreateShaders(aDevice), eFalse);
    niCheck(_CreatePipeline(aDevice), eFalse);
    return eTrue;
  }

  void Cleanup(VkDevice aDevice) {
    if (_vertexShader != VK_NULL_HANDLE) {
      vkDestroyShaderModule(aDevice, _vertexShader, nullptr);
      _vertexShader = VK_NULL_HANDLE;
    }
    if (_pixelShader != VK_NULL_HANDLE) {
      vkDestroyShaderModule(aDevice, _pixelShader, nullptr);
      _pixelShader = VK_NULL_HANDLE;
    }
    if (_vkPipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(aDevice, _vkPipeline, nullptr);
      _vkPipeline = VK_NULL_HANDLE;
    }
  }
};

struct sVulkanPipelineVertexPAT1 {
  VkShaderModule _vertexShader = VK_NULL_HANDLE;
  VkShaderModule _pixelShader = VK_NULL_HANDLE;
  VkPipelineLayout _vkPipelineLayout = VK_NULL_HANDLE;
  VkPipeline _vkPipeline = VK_NULL_HANDLE;

  tBool _CreateShaders(VkDevice avkDevice) {
    VkShaderModuleCreateInfo vertInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (size_t)texture_vs_spv_DATA_SIZE,
      .pCode = (const uint32_t*)texture_vs_spv_DATA
    };
    niCheck(vkCreateShaderModule(avkDevice, &vertInfo, nullptr, &_vertexShader) == VK_SUCCESS, eFalse);

    VkShaderModuleCreateInfo fragInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (size_t)texture_ps_spv_DATA_SIZE,
      .pCode = (const uint32_t*)texture_ps_spv_DATA
    };
    niCheck(vkCreateShaderModule(avkDevice, &fragInfo, nullptr, &_pixelShader) == VK_SUCCESS, eFalse);
    return eTrue;
  }

  tBool _CreatePipeline(VkDevice avkDevice, VkDescriptorSetLayout avkDescSetLayout) {
    VkPipelineLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &avkDescSetLayout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr
    };
    niCheck(vkCreatePipelineLayout(avkDevice, &layoutInfo, nullptr, &_vkPipelineLayout) == VK_SUCCESS, eFalse);

    VkVertexInputBindingDescription bindingDesc = {
      .binding = 0,
      .stride = sizeof(sVertexPAT1),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    astl::vector<VkVertexInputAttributeDescription> attrDesc = Vulkan_CreateVertexInputDesc(sVertexPAT1::eFVF);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDesc,
      .vertexAttributeDescriptionCount = (tU32)attrDesc.size(),
      .pVertexAttributeDescriptions = attrDesc.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE
    };

    VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = dynamicStates
    };

    VkPipelineViewportStateCreateInfo viewportState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = _vertexShader,
        .pName = "main"
      },
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = _pixelShader,
        .pName = "main"
      }
    };

    VkFormat swapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapChainFormat
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pDynamicState = &dynamicState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &colorBlending,
      .layout = _vkPipelineLayout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
      .pNext = &renderingInfo
    };

    niCheck(vkCreateGraphicsPipelines(avkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_vkPipeline) == VK_SUCCESS, eFalse);
    return eTrue;
  }

  tBool Init(VkDevice aDevice, VkDescriptorSetLayout avkDescSetLayout) {
    niCheck(_CreateShaders(aDevice), eFalse);
    niCheck(_CreatePipeline(aDevice, avkDescSetLayout), eFalse);
    return eTrue;
  }

  void Cleanup(VkDevice aDevice) {
    if (_vertexShader != VK_NULL_HANDLE) {
      vkDestroyShaderModule(aDevice, _vertexShader, nullptr);
      _vertexShader = VK_NULL_HANDLE;
    }
    if (_pixelShader != VK_NULL_HANDLE) {
      vkDestroyShaderModule(aDevice, _pixelShader, nullptr);
      _pixelShader = VK_NULL_HANDLE;
    }
    if (_vkPipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(aDevice, _vkPipeline, nullptr);
      _vkPipeline = VK_NULL_HANDLE;
    }
  }
};

TEST_FIXTURE(FOSWindowOSX,VulkanTriangle) {
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

  struct sVulkanTriangle_VulkanWindowSink : public sVulkanWindowSink {
    VkClearValue _clearValue = {
      // r, g, b, a
      .color = {{ 0.0f, 1.0f, 1.0f, 1.0f }}
    };
    tF64 _clearTimer = 0.0f;
    Ptr<sVulkanVertexArray> _va;
    sVulkanPipelineVertexPA _pipeline;

    tBool _CreateArrays() {
      _va = niNew sVulkanVertexArray(
        as_nn(this), 3, sVertexPA::eFVF, eArrayUsage_Static);
      niCheckIsOK(_va, eFalse);
      {
        sVertexPA* verts = (sVertexPA*)_va->Lock(0, 3, eLock_Discard);
        niCheck(verts != nullptr, eFalse);
        verts[0] = {{0.0f, -0.5f, 0.0f}, 0xFF0000FF}; // Red
        verts[1] = {{0.5f, 0.5f, 0.0f}, 0xFF00FF00};  // Green
        verts[2] = {{-0.5f, 0.5f, 0.0f}, 0xFFFF0000}; // Blue
        _va->Unlock();
      }
      return eTrue;
    }

    tBool Init(iOSWindow* apWnd, const achar* aAppName) override {
      niCheck(sVulkanWindowSink::Init(apWnd,aAppName), eFalse);
      niCheck(_CreateArrays(), eFalse);
      niCheck(_pipeline.Init(_device), eFalse);
      return eTrue;
    }

    void Cleanup() override {
      _va = nullptr;
      _pipeline.Cleanup(_device);
      sVulkanWindowSink::Cleanup();
    }

    void Draw() override {
      niCheck(BeginFrame(),;);

      VkViewport viewport = {
        .width = (float)_currentFb._width,
        .height = (float)_currentFb._height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
      };
      VkRect2D scissor = {{0, 0}, {_currentFb._width, _currentFb._height}};

      vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearValue = {
          .color = {{
              (tF32)RandFloat(), // r
              (tF32)RandFloat(), // g
              (tF32)RandFloat(), // b
              1.0f               // a
            }}
        };
        _clearTimer = ni::TimerInSeconds();
      }

      VkRenderingAttachmentInfo colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _currentFb._view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = _clearValue
      };

      VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, {_currentFb._width, _currentFb._height}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
      };

      vkCmdBeginRendering(_commandBuffer, &renderingInfo);
      {
        vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline._vkPipeline);
        VkBuffer vertexBuffers[] = {_va->_buffer._vkBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdDraw(_commandBuffer, 3, 1, 0, 0);
      }
      vkCmdEndRendering(_commandBuffer);

      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> metalSink = niNew sVulkanTriangle_VulkanWindowSink();
  CHECK_RETURN_IF_FAILED(metalSink->Init(wnd, m_testName));
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

TEST_FIXTURE(FOSWindowOSX,VulkanSquare) {
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

  struct sVulkanSquare_VulkanWindowSink : public sVulkanWindowSink {
    VkClearValue _clearValue = {
      // r, g, b, a
      .color = {{ 0.0f, 1.0f, 1.0f, 1.0f }}
    };
    tF64 _clearTimer = 0.0f;
    Ptr<sVulkanVertexArray> _va;
    Ptr<sVulkanIndexArray> _ia;
    sVulkanPipelineVertexPA _pipeline;

    tBool _CreateArrays() {
      {
        _va = niNew sVulkanVertexArray(
          as_nn(this), 4, sVertexPA::eFVF, eArrayUsage_Static);
        niCheckIsOK(_va, eFalse);
        sVertexPA* pVerts = (sVertexPA*)_va->Lock(0, 4, eLock_Discard);
        pVerts[0] = {{-0.5f, -0.5f, 0.0f}, 0xFF0000FF}; // Bottom Left - Red
        pVerts[1] = {{ 0.5f, -0.5f, 0.0f}, 0xFF00FF00}; // Bottom Right - Green
        pVerts[2] = {{ 0.5f,  0.5f, 0.0f}, 0xFFFF0000}; // Top Right - Blue
        pVerts[3] = {{-0.5f,  0.5f, 0.0f}, 0xFFFFFFFF}; // Top Left - White
        _va->Unlock();
      }
      {
        _ia = niNew sVulkanIndexArray(
          as_nn(this), eGraphicsPrimitiveType_TriangleList, 6, eArrayUsage_Static);
        niCheckIsOK(_ia, eFalse);
        tU32* pIndices = (tU32*)_ia->Lock(0, 6, eLock_Discard);
        pIndices[0] = 0; pIndices[1] = 1; pIndices[2] = 2; // First triangle
        pIndices[3] = 2; pIndices[4] = 3; pIndices[5] = 0; // Second triangle
        _ia->Unlock();
      }
      return eTrue;
    }

    tBool Init(iOSWindow* apWnd, const achar* aAppName) override {
      niCheck(sVulkanWindowSink::Init(apWnd,aAppName), eFalse);
      niCheck(_pipeline.Init(_device), eFalse);
      niCheck(_CreateArrays(), eFalse);
      return eTrue;
    }

    void Cleanup() override {
      _va = nullptr;
      _ia = nullptr;
      _pipeline.Cleanup(_device);
      sVulkanWindowSink::Cleanup();
    }

    void Draw() override {
      niCheck(BeginFrame(),;);

      VkViewport viewport = {
        .width = (float)_currentFb._width,
        .height = (float)_currentFb._height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
      };
      VkRect2D scissor = {{0, 0}, {_currentFb._width, _currentFb._height}};

      vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearValue = {
          .color = {{
              (tF32)RandFloat(), // r
              (tF32)RandFloat(), // g
              (tF32)RandFloat(), // b
              1.0f               // a
            }}
        };
        _clearTimer = ni::TimerInSeconds();
      }

      VkRenderingAttachmentInfo colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _currentFb._view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = _clearValue
      };

      VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, {_currentFb._width, _currentFb._height}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
      };

      vkCmdBeginRendering(_commandBuffer, &renderingInfo);
      {
        vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline._vkPipeline);
        VkBuffer vertexBuffers[] = {_va->_buffer._vkBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(_commandBuffer, _ia->_buffer._vkBuffer, 0, _ia->_indexType);
        vkCmdDrawIndexed(_commandBuffer, _ia->GetNumIndices(), 1, 0, 0, 0);
      }
      vkCmdEndRendering(_commandBuffer);

      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> metalSink = niNew sVulkanSquare_VulkanWindowSink();
  CHECK_RETURN_IF_FAILED(metalSink->Init(wnd, m_testName));
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

struct sVulkanTexture : public ImplRC<iTexture> {
  nn_mut<sVulkanDriver> _driver;
  VkImage _vkImage = VK_NULL_HANDLE;
  VmaAllocation _vmaAllocation;
  VkImageView _vkView = VK_NULL_HANDLE;
  VkSampler _vkSampler = VK_NULL_HANDLE;
  tU32 _width = 0, _height = 0;
  const tHStringPtr _name;
  const tTextureFlags _flags = eTextureFlags_Default;

  sVulkanTexture(ain_nn_mut<sVulkanDriver> aDriver, iHString* ahspName)
      : _driver(aDriver)
      , _name(ahspName)
  {
  }

  ~sVulkanTexture() {
    if (_vkSampler) {
      vkDestroySampler(_driver->_device, _vkSampler, nullptr);
      _vkSampler = VK_NULL_HANDLE;
    }
    if (_vkView) {
      vkDestroyImageView(_driver->_device, _vkView, nullptr);
      _vkView = VK_NULL_HANDLE;
    }
    if (_vkImage) {
      vmaDestroyImage(_driver->_allocator, _vkImage, _vmaAllocation);
      _vkImage = VK_NULL_HANDLE;
    }
  }

  virtual iHString* __stdcall GetDeviceResourceName() const override {
    return _name;
  }

  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) override {
    return eFalse;
  }

  virtual tBool __stdcall ResetDeviceResource() override {
    return eTrue;
  }

  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) override {
    return this;
  }

  virtual eBitmapType __stdcall GetType() const override {
    return eBitmapType_2D;
  }

  virtual tU32 __stdcall GetWidth() const override {
    return _width;
  }

  virtual tU32 __stdcall GetHeight() const override {
    return _height;
  }

  virtual tU32 __stdcall GetDepth() const override {
    return 1;
  }

  virtual iPixelFormat* __stdcall GetPixelFormat() const override {
    niPanicUnreachable("NOT IMPLEMENTED");
    return nullptr;
  }

  virtual tU32 __stdcall GetNumMipMaps() const override {
    return 1;
  }

  virtual tTextureFlags __stdcall GetFlags() const override {
    return _flags;
  }

  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const override {
    return nullptr;
  }

  tBool Create(tU32 aWidth, tU32 aHeight, const tU32* apData) {
    _width = aWidth;
    _height = aHeight;

    // Create image
    VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent = {_width, _height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocInfo = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY
    };

    niCheck(vmaCreateImage(_driver->_allocator, &imageInfo, &allocInfo,
                           &_vkImage, &_vmaAllocation, nullptr) == VK_SUCCESS, eFalse);

    // Staging buffer
    VkBuffer vkStagingBuffer;
    VmaAllocation vmaStagingAllocation;

    VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = _width * _height * sizeof(tU32),
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    VmaAllocationCreateInfo stagingAllocInfo = {
      .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    niCheck(vmaCreateBuffer(_driver->_allocator, &bufferInfo, &stagingAllocInfo,
                            &vkStagingBuffer, &vmaStagingAllocation, nullptr) == VK_SUCCESS, eFalse);

    // Copy data to staging
    void* data;
    vmaMapMemory(_driver->_allocator, vmaStagingAllocation, &data);
    memcpy(data, apData, _width * _height * sizeof(tU32));
    vmaUnmapMemory(_driver->_allocator, vmaStagingAllocation);

    // Transition image layout for copy
    VkCommandBuffer cmdBuf = _driver->BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = _vkImage,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      },
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT
    };

    vkCmdPipelineBarrier(cmdBuf,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1
      },
      .imageOffset = {0, 0, 0},
      .imageExtent = {_width, _height, 1}
    };

    vkCmdCopyBufferToImage(cmdBuf, vkStagingBuffer, _vkImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition to shader read
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmdBuf,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    _driver->EndSingleTimeCommands(cmdBuf);

    // Cleanup staging
    vmaDestroyBuffer(_driver->_allocator, vkStagingBuffer, vmaStagingAllocation);

    // Create image view
    VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = _vkImage,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    niCheck(vkCreateImageView(_driver->_device, &viewInfo, nullptr, &_vkView) == VK_SUCCESS, eFalse);

    // Create sampler
    VkSamplerCreateInfo samplerInfo = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_FALSE,
      .maxAnisotropy = 1.0f,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE
    };

    niCheck(vkCreateSampler(_driver->_device, &samplerInfo, nullptr, &_vkSampler) == VK_SUCCESS, eFalse);

    return eTrue;
  }
};

TEST_FIXTURE(FOSWindowOSX,VulkanTexture) {
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

  struct sVulkanTexture_VulkanWindowSink : public sVulkanWindowSink {
    VkClearValue _clearValue = {
      // r, g, b, a
      .color = {{ 0.0f, 1.0f, 1.0f, 1.0f }}
    };
    tF64 _clearTimer = 0.0f;
    Ptr<sVulkanVertexArray> _va;
    Ptr<sVulkanIndexArray> _ia;
    sVulkanPipelineVertexPAT1 _pipeline;
    Ptr<sVulkanTexture> _texture;
    VkDescriptorSetLayout _vkDescSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool _vkDescPool = VK_NULL_HANDLE;
    VkDescriptorSet _vkDescSet = VK_NULL_HANDLE;

    tBool _CreateArrays() {
      {
        _va = niNew sVulkanVertexArray(
          as_nn(this), 4, sVertexPAT1::eFVF, eArrayUsage_Static);
        niCheckIsOK(_va, eFalse);
        sVertexPAT1* pVerts = (sVertexPAT1*)_va->Lock(0, 4, eLock_Discard);
        pVerts[0] = {{-0.5f, -0.5f, 0.0f}, 0xFFFFFFFF, {0.0f, 0.0f}}; // TL (White)
        pVerts[1] = {{ 0.5f, -0.5f, 0.0f}, 0xFFFF0000, {1.0f, 0.0f}}; // TR (Red)
        pVerts[2] = {{ 0.5f,  0.5f, 0.0f}, 0xFF00FF00, {1.0f, 1.0f}}; // BR (Green)
        pVerts[3] = {{-0.5f,  0.5f, 0.0f}, 0xFF0000FF, {0.0f, 1.0f}}; // BL (Blue)
        _va->Unlock();
      }
      {
        _ia = niNew sVulkanIndexArray(
          as_nn(this), eGraphicsPrimitiveType_TriangleList, 6, eArrayUsage_Static);
        niCheckIsOK(_ia, eFalse);
        tU32* pIndices = (tU32*)_ia->Lock(0, 6, eLock_Discard);
        pIndices[0] = 0; pIndices[1] = 1; pIndices[2] = 2; // First triangle
        pIndices[3] = 2; pIndices[4] = 3; pIndices[5] = 0; // Second triangle
        _ia->Unlock();
      }
      return eTrue;
    }

    tBool _CreateTextures() {
      QPtr<iGraphics>graphics = niCreateInstance(niUI,Graphics,niVarNull,niVarNull);
      niCheck(graphics.IsOK(),eFalse);

      Ptr<iFile> fp = niFileOpenBin2H(A_jpg);
      niCheck(fp.IsOK(),eFalse);
      QPtr<iBitmap2D> bmp = graphics->LoadBitmap(fp);
      niCheck(bmp.IsOK(),eFalse);
      // Make sure its RGBA
      {
        Ptr<iPixelFormat> pxf = graphics->CreatePixelFormat("R8G8B8A8");
        bmp = bmp->CreateConvertedFormat(pxf);
        niCheck(bmp.IsOK(),eFalse);
      }
      niDebugFmt(("... Loaded bitmap '%s' %dx%d %s",
                  fp->GetSourcePath(),
                  bmp->GetWidth(),
                  bmp->GetHeight(),
                  bmp->GetPixelFormat()->GetFormat()));

      _texture = niNew sVulkanTexture(as_nn(this), _H(fp->GetSourcePath()));
      niCheck(_texture->Create(
        bmp->GetWidth(), bmp->GetHeight(), (tU32*)bmp->GetData()), eFalse);

      return eTrue;
    }

    tBool _CreateDescriptorSetLayout() {
      VkDescriptorSetLayoutBinding bindings[2] = {
        {  // Texture
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        },
        {  // Sampler
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        }
      };

      VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = niCountOf(bindings),
        .pBindings = bindings
      };

      niCheck(vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_vkDescSetLayout) == VK_SUCCESS, eFalse);

      // Update pipeline layout creation to use descriptor set
      VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &_vkDescSetLayout
      };
      niCheck(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipeline._vkPipelineLayout) == VK_SUCCESS, eFalse);

      return eTrue;
    }

    tBool _CreateDescriptorPool() {
      VkDescriptorPoolSize poolSizes[2] = {
        {
          .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
          .descriptorCount = 1
        },
        {
          .type = VK_DESCRIPTOR_TYPE_SAMPLER,
          .descriptorCount = 1
        }
      };

      VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = niCountOf(poolSizes),
        .pPoolSizes = poolSizes
      };

      niCheck(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_vkDescPool) == VK_SUCCESS, eFalse);

      VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = _vkDescPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &_vkDescSetLayout
      };

      niCheck(vkAllocateDescriptorSets(_device, &allocInfo, &_vkDescSet) == VK_SUCCESS, eFalse);

      // Update descriptor with texture
      VkDescriptorImageInfo imageInfo = {
        .imageView = _texture->_vkView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      };

      VkDescriptorImageInfo samplerInfo = {
        .sampler = _texture->_vkSampler
      };

      VkWriteDescriptorSet writes[2] = {
        {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = _vkDescSet,
          .dstBinding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
          .descriptorCount = 1,
          .pImageInfo = &imageInfo
        },
        {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = _vkDescSet,
          .dstBinding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
          .descriptorCount = 1,
          .pImageInfo = &samplerInfo
        }
      };

      vkUpdateDescriptorSets(_device, niCountOf(writes), writes, 0, nullptr);
      return eTrue;
    }

    tBool Init(iOSWindow* apWnd, const achar* aAppName) override {
      niCheck(sVulkanWindowSink::Init(apWnd,aAppName), eFalse);
      niCheck(_CreateDescriptorSetLayout(), eFalse);
      niCheck(_pipeline.Init(_device,_vkDescSetLayout), eFalse);
      niCheck(_CreateArrays(), eFalse);
      niCheck(_CreateTextures(), eFalse);
      niCheck(_CreateDescriptorPool(), eFalse);
      return eTrue;
    }

    void Cleanup() override {
      _texture = nullptr;
      _va = nullptr;
      _ia = nullptr;
      _pipeline.Cleanup(_device);
      sVulkanWindowSink::Cleanup();
    }

    void Draw() override {
      niCheck(BeginFrame(),;);

      VkViewport viewport = {
        .width = (float)_currentFb._width,
        .height = (float)_currentFb._height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
      };
      VkRect2D scissor = {{0, 0}, {_currentFb._width, _currentFb._height}};

      vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearValue = {
          .color = {{
              (tF32)RandFloat(), // r
              (tF32)RandFloat(), // g
              (tF32)RandFloat(), // b
              1.0f               // a
            }}
        };
        _clearTimer = ni::TimerInSeconds();
      }

      VkRenderingAttachmentInfo colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _currentFb._view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = _clearValue
      };

      VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, {_currentFb._width, _currentFb._height}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
      };

      vkCmdBeginRendering(_commandBuffer, &renderingInfo);
      {
        vkCmdBindDescriptorSets(
          _commandBuffer,
          VK_PIPELINE_BIND_POINT_GRAPHICS,
          _pipeline._vkPipelineLayout,
          0, 1, &_vkDescSet,
          0, nullptr);

        vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline._vkPipeline);
        VkBuffer vertexBuffers[] = {_va->_buffer._vkBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(_commandBuffer, _ia->_buffer._vkBuffer, 0, _ia->_indexType);
        vkCmdDrawIndexed(_commandBuffer, _ia->GetNumIndices(), 1, 0, 0, 0);
      }
      vkCmdEndRendering(_commandBuffer);

      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> vulkanSink = niNew sVulkanTexture_VulkanWindowSink();
  CHECK_RETURN_IF_FAILED(vulkanSink->Init(wnd, m_testName));
  wnd->GetMessageHandlers()->AddSink(vulkanSink);

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
