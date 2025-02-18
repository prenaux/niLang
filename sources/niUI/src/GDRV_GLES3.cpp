// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#ifdef GDRV_GL3
#include "GDRV_GLES3.h"

iVertexArray* _CreateGenericVertexArray(tU32 anNumVertices, tFVF anFVF);
const cFVFDescription& _GetGenericVertexArrayFVFDesc(iVertexArray* apVA);
tPtr _GetGenericVertexArrayMemPtr(iVertexArray* apVA);
iIndexArray* _CreateGenericIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex);
tPtr _GetGenericIndexArrayMemPtr(iIndexArray* apVA);

static GLint knGLSamplerFilterAnisotropy = 8;
niLetK knOpenGLMaxFramesInFlight = 1_u32;

#include "Graphics.h"
#include "GDRV_Utils.h"
#include "API/niUI_ModuleDef.h"

#include <niLang/Math/MathMatrix.h>

#ifdef niAndroid
#include <niLang/Utils/JNIUtils.h>
#endif

#include "GDRV_StateCache.h"
#include "FixedShaders.h"
#include "GDRV_Gpu.h"
#include <niLang/Utils/IDGenerator.h>
// #define GL_DEBUG_MISSING_MIPMAPS 4

#define CLEAR_BUFFERS_WITH_FIXED_PIPELINE

//--------------------------------------------------------------------------------------------
//
//  Cache
//
//--------------------------------------------------------------------------------------------

struct sOpenGLEncoderFrameData;
struct sOpenGLRasterPipeline;
struct sOpenGLBuffer;
struct sOpenGLDriver;

enum eGLCache
{
  eGLCache_Context,
  eGLCache_ContextSyncCounter,
  eGLCache_DepthStencil,
  eGLCache_Rasterizer,
  eGLCache_RasterizerDoubleSided,
  eGLCache_RasterizerDepthOnly,
  eGLCache_RasterizerFlippedRT,
  eGLCache_AlwaysOn, // ALWAYS ON CACHING...
  eGLCache_MaterialChannel,
  eGLCache_Material,
  eGLCache_FixedStates,
  eGLCache_Last,
};
struct sGLCache : public sStateCache {
  sGLCache() : sStateCache(eGLCache_Last,eGLCache_AlwaysOn) {
    Reset();
  }
  ~sGLCache() {
  }
  void Reset() {
    sStateCache::Reset();
    _depthTest = ni::eFalse;
    _depthMask = ni::eFalse;
    _colorWriteMask = ni::eColorWriteMask_All;
    _scissorTest = ni::eFalse;
    _renderTargetFBO = ni::eInvalidHandle;
    _depthStencilFBO = ni::eInvalidHandle;
    niLoop(i,GLDRV_MAX_TEXTURE_UNIT) {
      _tuChannel[i] = eMaterialChannel_Last;
    }
  }

  GLuint            _depthStencilFBO;
  GLuint            _renderTargetFBO;
  GLuint            _colorWriteMask;
  tBool             _depthTest;
  tBool             _depthMask;
  tBool             _scissorTest;
  eMaterialChannel  _tuChannel[GLDRV_MAX_TEXTURE_UNIT];
};

//--------------------------------------------------------------------------------------------
//
//  Context
//
//--------------------------------------------------------------------------------------------

struct sOpenGLRenderingInfo {
  GLuint _fbo = 0; // Framebuffer Object
  GLuint _vbo = 0; // vertex buffer object;
  GLuint _vao = 0; // vertex array  object;
  GLuint _ebo = 0; // element buffer object;
  GLuint _colorTexture = 0; // Color attachment texture
  GLuint _depthTexture = 0; // Depth attachment texture
  sVec4f _clearColorValue = {1.0f, 0.0f, 1.0f, 0.0f}; // Default clear color (magenta)
  tF32 _clearDepthValue = 1.0f; // Default clear depth
  tU32 _clearStencilValue = 0; // Default clear stencil
  sRecti _scissor = Recti(0);
  sRecti _viewport = Recti(0);
  sVec2i _rtSize = { 0, 0 };
  tBool _rtFlipped = eFalse;

  void _BeginRenderingInfo(
    ain<GLuint> aColorTexture,
    ain<GLuint> aColorTextureView,
    ain<GLuint> aDepthTexture,
    ain<GLuint> aDepthTextureView,
    ain<tU32> anWidth,
    ain<tU32> anHeight,
    ain<sRecti> aViewport,
    ain<sRecti> aScissor,
    ain<sVec4f> aClearColor,
    ain<tF32> aClearDepth,
    ain<tU32> aClearStencil)
  {
    // Store the clear values
    _clearColorValue = aClearColor;
    _clearDepthValue = aClearDepth;
    _clearStencilValue = aClearStencil;

    // Create or reuse the framebuffer object (FBO)
    if (_fbo == 0) {
      glGenFramebuffers(1, &_fbo);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // Attach the color texture (if provided)
    if (aColorTextureView != 0) {
      _colorTexture = aColorTextureView;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture, 0);
    } else {
      _colorTexture = 0;
    }

    // Attach the depth texture (if provided)
    if (aDepthTextureView != 0) {
      _depthTexture = aDepthTextureView;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
    } else {
      _depthTexture = 0;
    }

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      niError("Framebuffer is not complete.");
      return;
    }

    // glViewport(aViewport.x, aViewport.y, aViewport.GetWidth(), aViewport.GetHeight());
    // glScissor(aScissor.x, aScissor.y, aScissor.GetWidth(), aScissor.GetHeight());

    // Clear the framebuffer
    // glClearColor(_clearColorValue.x, _clearColorValue.y, _clearColorValue.z, _clearColorValue.w);
    // glClearDepth(_clearDepthValue);
    // glClearStencil(_clearStencilValue);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }

  void _BeginDraw() {

  }

  void _ResumeRenderingInfo() {
    // Bind the framebuffer again
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // Set the viewport and scissor (if needed)
    // Note: OpenGL does not have a direct equivalent for "resuming" rendering,
    // so we assume the viewport and scissor are already set correctly.

    // If we want to clear again, we can call glClear here
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }

  ~sOpenGLRenderingInfo() {
    if (_fbo != 0) {
      glDeleteFramebuffers(1, &_fbo);
      _fbo = 0;
    }
  }
};


struct sOpenGLCommandEncoder : public ImplRC<iGpuCommandEncoder> {
  nn<sOpenGLDriver> _driver;
  GLuint _fbo = 0; // Framebuffer Object
  struct sCache {
    sMaterialDesc _lastMaterial;
    Ptr<sOpenGLRasterPipeline> _lastPipeline = nullptr;
    Ptr<sOpenGLBuffer> _lastBuffer = nullptr;
    tU32 _lastBufferOffset = 0;
    tFixedGpuPipelineId _lastFixedPipeline = 0;
  } _cache;
  GLsync _encoderInFlightFence = 0;
  astl::vector<NN<sOpenGLEncoderFrameData>> _frames;
  tU32 _currentFrame = 0;
  tBool _beganCmdBuffer = eFalse;
  sOpenGLRenderingInfo _renderingInfo;

  sOpenGLCommandEncoder(iGraphicsDriver* aDriver, ain<tU32> aFrameMaxInFlight);

  ~sOpenGLCommandEncoder();

  sOpenGLEncoderFrameData* _GetCurrentFrame();

  tBool _CreateCommandBuffer();

  tBool _BeginCmdBuffer();

  // tBool _BeginRendering();

  void _ResumeRendering();

  void _EndRendering();

  tBool _EndCmdBufferAndSubmit(GLsync aImageAvailableSemaphore, GLsync aRendererFinishedSemaphore);

  void _DoBindPipeline(iGpuPipeline* apPipeline, tFixedGpuPipelineId aFixedPipelineId);

  tBool _BindGpuFunction();

  tBool _BegainDraw();

  tBool _EndDraw();

  void __stdcall SetPipeline(iGpuPipeline* apPipeline);

  void __stdcall _SetFixedPipeline(iGpuPipeline* apPipeline, tFixedGpuPipelineId aFixedPipelineId);

  void __stdcall SetVertexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding);

  void __stdcall SetIndexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, eGpuIndexType aIndexType);

  void __stdcall SetUniformBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding);

  void __stdcall SetTexture(iTexture* apTexture, tU32 anBinding);

  void __stdcall SetSamplerState(tIntPtr ahSS, tU32 anBinding);

  tBool __stdcall StreamVertexBuffer(const tPtr apData, tU32 anSize, tU32 anBinding);

  tBool __stdcall StreamIndexBuffer(const tPtr apData, tU32 anSize, eGpuIndexType aIndexType);

  tBool __stdcall StreamUniformBuffer(const tPtr apData, tU32 anSize, tU32 anBinding);

  void __stdcall SetPolygonOffset(const sVec2f& avOffset);

  void __stdcall SetScissorRect(const sRecti& aRect);

  void __stdcall SetViewport(const sRecti& aRect);

  void __stdcall SetStencilReference(tI32 aRef);

  void __stdcall SetStencilMask(tU32 aMask);

  void __stdcall SetBlendColorConstant(const sColor4f& aColor);

  tBool __stdcall DrawIndexed(eGraphicsPrimitiveType aPrimType, tU32 anNumIndices, tU32 anFirstIndex);

  tBool __stdcall Draw(eGraphicsPrimitiveType aPrimType, tU32 anVertexCount, tU32 anFirstVertex);

  tBool __stdcall BuildAccelerationStructure(iAccelerationStructure* apAS) niImpl { return eFalse; }
  tBool __stdcall DispatchRays(iRayGpuPipeline* apPipeline, iTexture* apOutputImage) niImpl { return eFalse; }
};

struct sGLContext : public sGraphicsContext<1,ImplRC<iGraphicsContextRT,eImplFlags_DontInherit1,iGraphicsContext,iGraphicsContextGpu> > {
  sGLContext(iGraphics* apGraphics, iGraphicsDriver* aDriver)
    : tGraphicsContextBase(apGraphics)
    , mEncoder(MakeNN<sOpenGLCommandEncoder>(aDriver,knOpenGLMaxFramesInFlight)) {
    mnSyncCounter = 0;
  }

#ifdef TSGL_CONTEXT
  virtual tsglContext* __stdcall GetTSGLContext() const = 0;
#endif

  ///////////////////////////////////////////////
  void __stdcall SetViewport(const sRecti& aVal) override {
    mrectViewport = aVal;
    mnSyncCounter++;
  }

  ///////////////////////////////////////////////
  void __stdcall SetScissorRect(const sRecti& aVal) override {
    mrectScissor = aVal;
    mnSyncCounter++;
  }

  iGpuCommandEncoder* __stdcall GetCommandEncoder();
  void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil);
  tBool __stdcall ClearBuffersRect(tClearBuffersFlags aFlags, const sRectf& aRect, tU32 anColor, tF32 afZ);

  tBool _BeginFrame();

  tU32 mnSyncCounter;
  NN<sOpenGLCommandEncoder> mEncoder;
  tBool _beganFrame = eFalse;
};

static tU32 _kNumGL3TexUpload = 0;

class cGL3ContextWindow;
class cGL3ContextRT;

static void GLES3_DoClear(iGraphicsDriver* apDrv, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil);
static void GLES3_ClearBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil);
static tBool GLES3_DrawOperation(iGraphicsDriver* apDrv, sGLContext* apContext, iDrawOperation* apDrawOp, const tU32 anAA);
static tBool GLES3_SwapBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tBool abDoNotWait);
static tBool GLES3_ResetCache(iGraphicsDriver* apDrv);
static tBool GLES3_ResetContextDeviceResources(iGraphicsDriver* apDrv);
static tBool GLES3_InitContextDeviceResources(iGraphicsDriver* apDrv);
static sGLCache& GLES3_GetCache(iGraphicsDriver* apDrv);
static tBool GL3_ApplyMaterialChannel(
    sGLContext* apContext,
    iGraphics* apGraphics,
    sGLCache& aCache,
    tU32 anTSS, eMaterialChannel aChannel,
    const sMaterialDesc* apMaterial);

const tU32 eTextureFlags_DontOwnGLHandles = niBit(31);
const tU32 eTextureFlags_MainRT = niBit(30);
const tU32 eTextureFlags_MainDS = niBit(29);

//--------------------------------------------------------------------------------------------
//
//  Utils
//
//--------------------------------------------------------------------------------------------
// "GL_ARB_shader_texture_lod" (texture2D/Cube/3DLod in pixel shader - its supported in vertex shaders only by default ^^)
static bool hasTextureLod = true;
// "OES_standard_derivatives"
static bool hasStandardDerivatives = true;
// "GL_ARB_texture_non_power_of_two"
static bool hasPartialNP2 = false;
#ifdef USE_OQ
// "GL_ARB_occlusion_query"
static bool hasOQ = false;
#endif
// "GL_ARB_texture_cube_map", "GL_EXT_texture_cube_map"
static bool hasCubeMap =
#ifdef TSGL_DESKTOP
    true
#else
    false
#endif
;
static bool hasElementUInt =
#ifdef TSGL_DESKTOP
    true
#else
    false
#endif
    ;
static bool hasContextLost =
#ifdef niAndroid
    true
#else
    false
#endif
;

static bool hasTexFmtHalfFloat = false;
static bool hasTexFmtFloat = false;
static bool hasTexFmtDepth = false;

#if defined niWindows || defined niOSX || defined niLinux
#define USE_GL_GET_TEX_IMAGE
#endif

#if defined USE_GLES3
// TODO: This should go in a future GDRV_GLContext_sym_core_gles2.h header.
#define _glReadBuffer(X) glReadBuffer
#else
#define _glReadBuffer(X) // NOOP
#endif

#if defined niIOS
#define USE_FBO_MAINRT_IS_FBO
#endif

#ifdef GL_TEXTURE_WRAP_R
static bool hasWrapR = false; // false, and will stay for the time being, its needed only for 3d textures
#endif

#ifdef USE_GL_BIND_VAO
static bool hasBindVAO = false;
#endif

#ifdef USE_GL_BIND_SAMPLER
static bool hasBindSampler = false;
#endif

#ifdef USE_FBO
// "GL_EXT_framebuffer_object"
static const bool hasFBO = true; // NOTE: always true, it used to be optional
#ifdef USE_FBO_MAINRT_IS_FBO
static GLuint fboMainRTHandle = eInvalidHandle;
static GLuint fboMainDSHandle = eInvalidHandle;
#endif
#endif

// #define USE_BUFFER_SUBDATA

#ifdef USE_BUFFER_SUBDATA
static bool hasBufferSubdata = true;
#endif

// #define TRACE_BUFFER_BW

#ifdef TRACE_BUFFER_BW
static void _CountBufferUploadedBytes(tU32 anNumBytes) {
  static tU32 _numReportCount = 0;
  static tU64 _numBytes = 0;
  static tF64 _startTime = ni::TimerInSeconds();

  _numBytes += anNumBytes;

  tF64 newTime = ni::TimerInSeconds();
  if (newTime - _startTime > 1.0) {
    niDebugFmt(("... [%d] glBuffer upload bw, last sec: ~%dKb/s (%db)",
                _numReportCount++,
                _numBytes / 1024, _numBytes));
    _numBytes = 0;
    _startTime = newTime;
  }
}
#endif

#if defined __GLDESKTOP__
#define _glClearDepthf _glClearDepth
#define _glCreateShader _glCreateShaderObject
#define _glDeleteShader _glDeleteObject
#define _glCreateProgram _glCreateProgramObject
#define _glAttachShader _glAttachObject
#define _glUseProgram _glUseProgramObject
#define _glDeleteProgram _glDeleteObject
#endif

static GLint kGL3_MaxTU = GLDRV_MAX_TEXTURE_UNIT;
static GLint kGL3_MaxCubeTexSize = 8192;
static GLint kGL3_MaxRegularTexSize = 8192;
static GLint kGL3_MaxOverlayTexSize = 8192;
static GLint kGL3_MaxVertexAttrs = 16;
static GLint kGL3_MaxVertexUniforms = 0;
static GLint kGL3_MaxPixelUniforms = 0;

#ifdef USE_FBO

#ifndef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT 0x8CD8
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS 0x8CD9
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_FORMATS
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS 0x8CDA
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#endif

#ifdef CHECK_GLERR
static const achar* GL_FBOStatus(GLenum status) {
  switch (status) {
    case GL_FRAMEBUFFER_COMPLETE: return "FRAMEBUFFER_COMPLETE";
    case GL_FRAMEBUFFER_UNSUPPORTED: return "FRAMEBUFFER_UNSUPPORTED";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS: return "GL_FRAMEBUFFER_INCOMPLETE_FORMATS";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
    default: return "Unknown FrameBuffer Status";
  }
};
#define GLFBO_RET(ret) if (_CheckGLError()) {                     \
    GLenum fboStatus = _glCheckFramebufferStatus(GL_FRAMEBUFFER); \
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {                   \
      ni::GetLang()->Log(                                         \
        eLogFlags_Warning,                                        \
        niFmt(_A("GL FBO Incomplete: %d, %s\n"),                  \
              fboStatus,GL_FBOStatus(fboStatus)),                 \
        niSourceLoc);                                             \
      niAssertUnreachable("GL FBO Error");                        \
      return ret;                                                 \
    }                                                             \
  }
#else
#define GLFBO_RET(ret)
#endif

#endif

static tBool GL3_InitializeExt() {
  static tBool _printedInfos = eTrue;

  const cString strExt = (const cchar*)_glGetString(GL_EXTENSIONS);
  const cString strRenderer = (const cchar*)_glGetString(GL_RENDERER);
  const cString strVendor = (const cchar*)_glGetString(GL_VENDOR);
  const cString strVersion = (const cchar*)_glGetString(GL_VERSION);

  const tBool isES3 =
#if defined TSGL_DESKTOP
      eFalse
#else
      !strVersion.contains("ES 2")
#endif
      ;

  if (_printedInfos) {
    niDebugFmt((_A("--- GL3 Context Infos ---")));
    niDebugFmt((_A("GL_VENDOR: %s"),strVendor));
    niDebugFmt((_A("GL_RENDERER: %s"),strRenderer));
    niDebugFmt((_A("GL_VERSION: %s"),strVersion));
    niDebugFmt((_A("GL_EXTENSIONS: %s"),strExt));
  }

  if (strRenderer.empty() || strVendor.empty() || strVersion.empty()) {
    niError("Invalid OpenGL context.");
    return eFalse;
  }

  if (_printedInfos) {
    niDebugFmt(("--- GL3_CAPS ---"));
  }

  // max texture units
  {
    GLint maxTU = kGL3_MaxTU;
    _glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&maxTU);
    kGL3_MaxTU = ni::Min(maxTU,kGL3_MaxTU,GLDRV_MAX_TEXTURE_UNIT);
  }
  // shader caps
  {
    _glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&kGL3_MaxVertexAttrs);
#if defined niOSX || defined niWindows
    _glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB,&kGL3_MaxVertexUniforms);
    kGL3_MaxVertexUniforms /= 4;
    _glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB,&kGL3_MaxPixelUniforms);
    kGL3_MaxPixelUniforms /= 4;
#else
    _glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,&kGL3_MaxVertexUniforms);
    _glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS,&kGL3_MaxPixelUniforms);
#endif
  }
  // max texture size
  {
    GLint maxTexSize = 0;
    if (ni::GetLang()->HasProperty("GL3.GL_MAX_TEXTURE_SIZE")) {
      maxTexSize = ni::GetLang()->GetProperty("GL3.GL_MAX_TEXTURE_SIZE").Long();
    }
    if (maxTexSize < 4) {
      maxTexSize = kGL3_MaxRegularTexSize;
      _glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexSize);
    }
    if (ni::GetLang()->HasProperty("GL3.MaxRegularTexSize")) {
      kGL3_MaxRegularTexSize = ni::GetLang()->GetProperty("GL3.MaxRegularTexSize").Long();
    }
    else {
      kGL3_MaxRegularTexSize = ni::Min(maxTexSize,kGL3_MaxRegularTexSize);
    }
    if (ni::GetLang()->HasProperty("GL3.MaxOverlayTexSize")) {
      kGL3_MaxOverlayTexSize = ni::GetLang()->GetProperty("GL3.MaxOverlayTexSize").Long();
    }
    else {
      kGL3_MaxOverlayTexSize = ni::Min(maxTexSize,kGL3_MaxOverlayTexSize);
    }
    if (ni::GetLang()->HasProperty("GL3.MaxCubeTexSize")) {
      kGL3_MaxCubeTexSize = ni::GetLang()->GetProperty("GL3.MaxCubeTexSize").Long();
    }
    else {
      kGL3_MaxCubeTexSize = ni::Min(maxTexSize,kGL3_MaxCubeTexSize);
    }
  }
  // non power of 2 texture support
  {
#ifdef __GLES3__
    hasPartialNP2 = true;
#else
    hasPartialNP2 = (strExt.contains("GL_ARB_texture_non_power_of_two") ||
        strExt.contains("GL_IMG_texture_npot") ||
        strExt.contains("GL_NV_texture_npot_2D_mipmap") ||
        strExt.contains("GL_OES_texture_npot"));
#endif
  }
  // has cube maps
  {
#ifdef __GLES3__
    hasCubeMap = true;
#else
    hasCubeMap = !!(strExt.contains("GL_ARB_texture_cube_map") ||
                    strExt.contains("GL_EXT_texture_cube_map"));
#endif
  }
  // has element_uint
  {
#if !defined TSGL_DESKTOP
    hasElementUInt = isES3 || strExt.contains("element_index_uint");
#endif
  }

  // has float texture & depth texture
  {
    hasTexFmtHalfFloat = isES3 || strExt.contains("texture_half_float") || strExt.contains("NV_half_float");
    hasTexFmtFloat = isES3 || strExt.contains("texture_float");
    hasTexFmtDepth = isES3 || strExt.contains("depth_texture");
  }

  {
#ifdef USE_GL_BIND_VAO
#ifdef niIOS
    hasBindVAO = true;
#endif
#endif
  }

#ifdef USE_GL_BIND_SAMPLER
  {
#ifdef niIOS
    hasBindSampler = isES3;
#endif
  }
#endif

  // standard derivatives
  {
#ifdef niEmbedded
    hasStandardDerivatives = strExt.contains("OES_standard_derivatives");
#else
    if (ni::GetLang()->HasProperty("GL3.hasStandardDerivatives")) {
      hasStandardDerivatives = !!ni::GetLang()->GetProperty("GL3.hasStandardDerivatives").Bool(hasStandardDerivatives);
    }
    else {
      hasStandardDerivatives = true;
    }
#endif
  }

#ifdef USE_OQ
  if (strExt.contains("GL_ARB_occlusion_query")) {
    GLint bits;
    if (_glGetQueryiv) {
      _glGetQueryiv(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bits);
      if (bits) {
        hasOQ = true;
      }
    }
  }
#endif

  {
    GLint maxAnisotropic = 1;
    if (strExt.contains("EXT_texture_filter_anisotropic")) {
      _glGetIntegerv(GLEXT_MAX_TEXTURE_MAX_ANISOTROPY,&maxAnisotropic);
      if (maxAnisotropic > 1) {
        if (ni::GetLang()->HasProperty("GL3.samplerFilterAnisotropy")) {
          knGLSamplerFilterAnisotropy = ni::GetLang()->GetProperty("GL3.samplerFilterAnisotropy").Long();
        }
      }
    }
    knGLSamplerFilterAnisotropy = ni::Min(knGLSamplerFilterAnisotropy, maxAnisotropic);
  }

  if (ni::GetLang()->HasProperty("GL3.hasContextLost")) {
    hasContextLost = !!ni::GetLang()->GetProperty("GL3.hasContextLost").Bool(hasContextLost);
  }

  if (_printedInfos) {
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_SYSTEM_MEMORY
    niDebugFmt(("GL3 GL_DYNAMIC_BUFFER_MODE: SYSTEM_MEMORY"));
#elif GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING
    niDebugFmt(("GL3 GL_DYNAMIC_BUFFER_MODE: ORPHANING"));
#elif GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_NONE
    niDebugFmt(("GL3 GL_DYNAMIC_BUFFER_MODE: NONE"));
#else
    #error "No GL_DYNAMIC_BUFFER_MODE defined."
#endif

    niDebugFmt(("GL3 hasContextLost: %d",hasContextLost));
    niDebugFmt(("GL3 hasPartialNP2: %d",hasPartialNP2));
    niDebugFmt(("GL3 hasCubeMap: %d",hasCubeMap));
    niDebugFmt(("GL3 hasElementUInt: %d",hasElementUInt));
    niDebugFmt(("GL3 hasTexFmtHalfFloat: %d",hasTexFmtHalfFloat));
    niDebugFmt(("GL3 hasTexFmtFloat: %d",hasTexFmtFloat));
    niDebugFmt(("GL3 hasTexFmtDepth: %d",hasTexFmtDepth));
#ifdef GL_TEXTURE_WRAP_R
    niDebugFmt(("GL3 hasWrapR: %d",hasWrapR));
#else
    niDebugFmt(("GL3 NO WarpR at compile time"));
#endif
#ifdef USE_GL_BIND_VAO
    niDebugFmt(("GL3 hasBindVAO: %d",hasBindVAO));
#else
    niDebugFmt(("GL3 NO VAO at compile time"));
#endif
#ifdef USE_GL_BIND_SAMPLER
    niDebugFmt(("GL3 hasBindSampler: %d",hasBindSampler));
#else
    niDebugFmt(("GL3 NO Sampler at compile time"));
#endif
#ifdef USE_FBO
    niDebugFmt(("GL3 hasFBO: %d",hasFBO));
#else
    niDebugFmt(("GL3 NO FBO at compile time"));
#endif
#ifdef USE_OQ
    niDebugFmt(("GL3 hasOQ: %d",hasOQ));
#else
    niDebugFmt(("GL3 NO OQ at compile time"));
#endif
    niDebugFmt(("GL3 hasStandardDerivatives: %d",hasStandardDerivatives));
    niDebugFmt(("GL3 hasTextureLod: %d",hasTextureLod));
    niDebugFmt(("GL3 samplerFilterAnisotropy: %d",knGLSamplerFilterAnisotropy));
    niDebugFmt(("GL_MAX_TEXTURE_SIZE (Regular): %d",kGL3_MaxRegularTexSize));
    niDebugFmt(("GL_MAX_TEXTURE_SIZE (Overlay): %d",kGL3_MaxOverlayTexSize));
    niDebugFmt(("GL_MAX_TEXTURE_SIZE (CubeMap): %d",kGL3_MaxCubeTexSize));
    niDebugFmt(("GL_MAX_TEXTURE_UNITS: %d",kGL3_MaxTU));
    niDebugFmt(("GL_MAX_VERTEX_ATTRIBS: %d",kGL3_MaxVertexAttrs));
    niDebugFmt(("GL_MAX_VERTEX_UNIFORM_VECTORS: %d",kGL3_MaxVertexUniforms));
    niDebugFmt(("GL_MAX_FRAGMENT_UNIFORM_VECTORS: %d",kGL3_MaxPixelUniforms));
  }

  if (!hasElementUInt) {
    niError("OpenGL driver doesn't support UInt element buffers.");
    return eFalse;
  }

  _printedInfos = eFalse;
  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  States
//
//--------------------------------------------------------------------------------------------
static inline GLenum GL_Compare(eGraphicsCompare cmp) {
  switch (cmp) {
    case eGraphicsCompare_Never: return GL_NEVER;
    case eGraphicsCompare_Equal: return GL_EQUAL;
    case eGraphicsCompare_NotEqual: return GL_NOTEQUAL;
    case eGraphicsCompare_Less: return GL_LESS;
    case eGraphicsCompare_LessEqual: return GL_LEQUAL;
    case eGraphicsCompare_Greater: return GL_GREATER;
    case eGraphicsCompare_GreaterEqual: return GL_GEQUAL;
    case eGraphicsCompare_Always: return GL_ALWAYS;
  }
  return 0;
}

#ifndef NO_STENCIL_BUFFER
static inline GLenum GL_StencilOp(eStencilOp op) {
  switch (op) {
    case eStencilOp_Keep:   return GL_KEEP;
    case eStencilOp_Zero:   return GL_ZERO;
    case eStencilOp_Replace:  return GL_REPLACE;
    case eStencilOp_IncrSat:  return GL_INCR;
    case eStencilOp_DecrSat:  return GL_DECR;
    case eStencilOp_IncrWrap: return GL_INCR;
    case eStencilOp_DecrWrap: return GL_DECR;
    case eStencilOp_Invert:   return GL_INVERT;
  }
  return 0;
}
#endif

///////////////////////////////////////////////
static tBool GL_GetFirstStageBlendMode(eBlendMode aMode, GLenum& aSrc, GLenum& aDest) {
  switch (aMode) {
    case eBlendMode_NoBlending:
      aSrc = GL_ONE;
      aDest = GL_ZERO;
      break;
    case eBlendMode_ReplaceAlpha:
      aSrc = GL_ZERO;
      aDest = GL_ONE;
      break;
    default:
    case eBlendMode_Additive:
      aSrc = GL_ONE;
      aDest = GL_ONE;
      break;
    case eBlendMode_Modulate:
    case eBlendMode_ModulateReplaceAlpha:
      aSrc = GL_DST_COLOR;
      aDest = GL_ZERO;
      break;
    case eBlendMode_Modulate2x:
    case eBlendMode_Modulate2xReplaceAlpha:
      aSrc = GL_DST_COLOR;
      aDest = GL_SRC_COLOR;
      break;
    case eBlendMode_Translucent:
      aSrc = GL_SRC_ALPHA;
      aDest = GL_ONE_MINUS_SRC_ALPHA;
      break;
    case eBlendMode_TranslucentInvAlpha:
      aSrc = GL_ONE_MINUS_SRC_ALPHA;
      aDest = GL_SRC_ALPHA;
      break;
    case eBlendMode_TintedGlass:
      aSrc = GL_ONE; // GL_SRC_COLOR; -- this does an INVALID_ENUM error on GL ES (PowerVR)
      aDest = GL_ONE_MINUS_SRC_COLOR;
      break;
    case eBlendMode_PreMulAlpha:
      aSrc = GL_ONE;
      aDest = GL_ONE_MINUS_SRC_ALPHA;
      break;
  }
  return eTrue;
}

///////////////////////////////////////////////
static inline GLenum GL_Primitive(eGraphicsPrimitiveType prim)
{
  switch (prim) {
    case eGraphicsPrimitiveType_PointList:    return GL_POINTS;
    case eGraphicsPrimitiveType_LineList:   return GL_LINES;
    case eGraphicsPrimitiveType_LineStrip:    return GL_LINE_STRIP;
    case eGraphicsPrimitiveType_TriangleList: return GL_TRIANGLES;
    case eGraphicsPrimitiveType_TriangleStrip:  return GL_TRIANGLE_STRIP;
    default: return 0;
  }
}

///////////////////////////////////////////////
static inline GLenum GL_CubeMapFace(eBitmapCubeFace face) {
  return GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)face;
}

///////////////////////////////////////////////
static inline GLenum GL_Texture(eBitmapType type)
{
  if (type == eBitmapType_2D) {
    return GL_TEXTURE_2D;
  }
  if (type == eBitmapType_Cube) {
    return GL_TEXTURE_CUBE_MAP;
  }
  return 0;
}

///////////////////////////////////////////////
static inline void GL_TexSamplerWrap(GLenum texType, GLenum coo, eSamplerWrap aWrap, tBool isOverlay) {
  GLint type = GL_CLAMP_TO_EDGE;
  if (!isOverlay) {
    if (aWrap == eSamplerWrap_Mirror) {
      type = GL_MIRRORED_REPEAT;
    }
    else if (aWrap == eSamplerWrap_Repeat) {
      type = GL_REPEAT;
    }
  }
  _glTexParameteri(texType, coo, type);
  GLERR_RET(;);
}

///////////////////////////////////////////////
static inline void GL_TexSamplerFilter(GLenum texType, eSamplerFilter aFilter, tU32 anNumMipMaps) {
  // anNumMipMaps = 0;
  GLint minfilter = GL_NEAREST;
  GLint magfilter = GL_NEAREST;
  switch (aFilter) {
    case eSamplerFilter_Point:
      magfilter = GL_NEAREST;
      minfilter = anNumMipMaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
      break;
    case eSamplerFilter_Smooth:
      magfilter = GL_LINEAR;
      minfilter = anNumMipMaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
      break;
  }
  if (anNumMipMaps) {
    _glTexParameteri(texType, GLEXT_TEXTURE_MAX_ANISOTROPY, knGLSamplerFilterAnisotropy);
  }
  else {
    _glTexParameteri(texType, GLEXT_TEXTURE_MAX_ANISOTROPY, 1);
  }
  _glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, minfilter);
  GLERR_RET(;);
  _glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, magfilter);
  GLERR_RET(;);
}

static inline void GL_SetInitialTexParameters(GLenum texType, tU32 anNumMipMaps, tBool isPow2) {
  GL_TexSamplerWrap(texType, GL_TEXTURE_WRAP_S, eSamplerWrap_Clamp, isPow2);
  GL_TexSamplerWrap(texType, GL_TEXTURE_WRAP_T, eSamplerWrap_Clamp, isPow2);
#ifdef GL_TEXTURE_WRAP_R
  if (hasWrapR) {
    GL_TexSamplerWrap(texType, GL_TEXTURE_WRAP_R, eSamplerWrap_Clamp, isPow2);
  }
#endif
  GL_TexSamplerFilter(texType, eSamplerFilter_Smooth, anNumMipMaps);
  // _glTexParameteri(texType, GL_TEXTURE_BASE_LEVEL, 0);
  // _glTexParameteri(texType, GL_TEXTURE_MAX_LEVEL, mnNumMipMaps+1);
}

///////////////////////////////////////////////
static void GL_ApplyBlendMode(eBlendMode aBlendMode) {
  if (aBlendMode == eBlendMode_NoBlending) {
    _glDisable(GL_BLEND);
    GLERR_RET(;);
  }
  else {
    _glEnable(GL_BLEND);
    GLERR_RET(;);
    GLenum src,dst;
    GL_GetFirstStageBlendMode(aBlendMode,src,dst);

#ifdef GLDRV_CORRECT_ALPHA_COMPOSITING
    _glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    if (aBlendMode == eBlendMode_NoBlending) {
      _glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
    }
    else {
      _glBlendFuncSeparate(src, dst, GL_ONE, GL_ONE);
    }
#else
    _glBlendFunc(src,dst);
#endif

    GLERR_RET(;);
  }
}

///////////////////////////////////////////////
static void GL_ApplyDepthStencilStates(sGLCache& aCache, const sDepthStencilStatesDesc& v) {
  // Depth test //
  if (v.mbDepthTest) {
    _glEnable(GL_DEPTH_TEST);
    aCache._depthTest = ni::eTrue;
  }
  else {
    _glDisable(GL_DEPTH_TEST);
    aCache._depthTest = ni::eFalse;
  }
  GLERR_RET(;);

  if (v.mbDepthTestWrite) {
    aCache._depthMask = ni::eTrue;
    _glDepthMask(GL_TRUE);
  }
  else {
    aCache._depthMask = ni::eFalse;
    _glDepthMask(GL_FALSE);
  }
  GLERR_RET(;);

  _glDepthFunc(GL_Compare(v.mDepthTestCompare));
  GLERR_RET(;);

#ifndef NO_STENCIL_BUFFER
  // Stencil test //
  if (v.mStencilMode == eStencilMode_None) {
    _glDisable(GL_STENCIL_TEST);
    GLERR_RET(;);
  }
  else {
    _glEnable(GL_STENCIL_TEST);
    _glStencilFunc(GL_Compare(v.mStencilFrontCompare),
                   v.mnStencilRef,
                   v.mnStencilMask);
    GLERR_RET(;);
    _glStencilOp(GL_StencilOp(v.mStencilFrontFail),
                 GL_StencilOp(v.mStencilFrontPassDepthFail),
                 GL_StencilOp(v.mStencilFrontPassDepthPass));
    GLERR_RET(;);
  }
#endif
}

///////////////////////////////////////////////
static void GL_ApplyRasterizerStates(sGLCache& aCache, const sRasterizerStatesDesc& v,
                                     const tBool abDoubleSided,
                                     const tBool abDepthOnly,
                                     const tBool abFlippedRT)
{

  // Fill mode //
#ifdef _glPolygonMode
  if (v.mbWireframe) _glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  else               _glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
#endif

  // Culling //
  if (abDoubleSided) {
    _glDisable(GL_CULL_FACE);
    GLERR_RET(;);
  }
  else {
    switch (v.mCullingMode) {
      default:
      case eCullingMode_None:
        _glDisable(GL_CULL_FACE);
        GLERR_RET(;);
        break;
      case eCullingMode_CCW:
        _glEnable(GL_CULL_FACE);
        _glFrontFace(abFlippedRT ? GL_CCW : GL_CW);
        _glCullFace(GL_BACK);
        GLERR_RET(;);
        break;
      case eCullingMode_CW:
        _glEnable(GL_CULL_FACE);
        _glFrontFace(abFlippedRT ? GL_CW : GL_CCW);
        _glCullFace(GL_BACK);
        GLERR_RET(;);
        break;
    }
  }

  // Write mask //
  // if (0)
  {
    const tU32 m = aCache._colorWriteMask = abDepthOnly ? 0 : v.mColorWriteMask;
    _glColorMask(niFlagTest(m,eColorWriteMask_Red),
                 niFlagTest(m,eColorWriteMask_Green),
                 niFlagTest(m,eColorWriteMask_Blue),
                 niFlagTest(m,eColorWriteMask_Alpha));
    GLERR_RET(;);
  }

  // TODO: Depth Bias ? //
}

//--------------------------------------------------------------------------------------------
//
//  Textures
//
//--------------------------------------------------------------------------------------------
struct sOpenglTextureFormat
{
  GLenum kind;
  GLint  type;
  GLint  format;
  GLint  internalformat;
  tTextureFlags flags;
  Ptr<iPixelFormat> pxf;

  sOpenglTextureFormat() {
    flags = 0;
    kind = 0;
    type = -1;
    format = -1;
    internalformat = -1;
  }

  void _Set(
      eBitmapType aBitmapType,
      iPixelFormat* apPxf,
      tTextureFlags aFlags,
      GLint aType = -1,
      GLint aFormat = -1,
      GLint aInternalFormat = -1)
  {
    kind = GL_Texture(aBitmapType);
    pxf = apPxf;
    flags = aFlags;
    type = aType;
    format = aFormat;
    internalformat = aInternalFormat;
  }

  tBool _IsValidGLFormat() const {
    return format != -1 &&
        internalformat != -1 &&
        type != -1;
  }

  void _SetTexFmt(tU32 aTexFmt, cString* apstrFmt) {
    switch (aTexFmt) {
      case GLES_TEXFMT_LA16:
#if defined USE_TEXFMT_ALPHA
        {
          if (apstrFmt) *apstrFmt = "L8A8";
          internalformat = GL_LUMINANCE_ALPHA;
          format = GL_LUMINANCE_ALPHA;
          type = GL_UNSIGNED_BYTE;
        }
#else
        {
          if (apstrFmt) *apstrFmt = "R8G8B8A8";
          internalformat = GL_RGBA;
          format = GL_RGBA;
          type = GL_UNSIGNED_BYTE;
        }
#endif
        break;
      case GLES_TEXFMT_A8:
#if defined USE_TEXFMT_LUMINANCE_ALPHA
        {
          if (apstrFmt) *apstrFmt = "A8";
          internalformat = GL_ALPHA;
          format = GL_ALPHA;
          type = GL_UNSIGNED_BYTE;
        }
#else
        {
          if (apstrFmt) *apstrFmt = "R8G8B8A8";
          internalformat = GL_RGBA;
          format = GL_RGBA;
          type = GL_UNSIGNED_BYTE;
        }
#endif
        break;
      case GLES_TEXFMT_RGBA32F:
        if (apstrFmt) *apstrFmt = "FR32G32B32A32";
        internalformat = GL_RGBA;
        format = GL_RGBA;
        type = GL_FLOAT;
        break;
      case GLES_TEXFMT_RGBA16F:
        if (apstrFmt) *apstrFmt = "FR16G16B16A16";
        internalformat = GL_RGBA;
        format = GL_RGBA;
        type = MY_GL_HALF_FLOAT;
        break;
      case GLES_TEXFMT_RGBA32:
        if (apstrFmt) *apstrFmt = "R8G8B8A8";
        internalformat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
      case GLES_TEXFMT_RGBA16:
        if (apstrFmt) *apstrFmt = "A4B4G4R4";
        internalformat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_4_4_4_4;
        break;
      case GLES_TEXFMT_RGBA15:
        if (apstrFmt) *apstrFmt = "A1B5G5R5";
        internalformat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_5_5_5_1;
        break;
      case GLES_TEXFMT_RGB16:
        if (apstrFmt) *apstrFmt = "B5G6R5";
        internalformat = GL_RGB;
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
        break;

      case GLES_TEXFMT_RGB24:
#if defined USE_TEXFMT_GL_RGB
        {
          if (apstrFmt) *apstrFmt = "R8G8B8";
          internalformat = GL_RGB;
          format = GL_RGB;
          type = GL_UNSIGNED_BYTE;
        }
#else
        {
          if (apstrFmt) *apstrFmt = "R8G8B8X8";
          internalformat = GL_RGBA;
          format = GL_RGBA;
          type = GL_UNSIGNED_BYTE;
        }
#endif
        break;

      case GLES_TEXFMT_D32:
// TODO: GL_DEPTH_COMPONENT32_OES we need to check detect whether GL_OES_depth32
// extension is supported.
#if defined GL_DEPTH_COMPONENT32
        {
          if (apstrFmt) *apstrFmt = "D32";
          internalformat = GL_DEPTH_COMPONENT32;
          format = GL_DEPTH_COMPONENT;
          type = GL_UNSIGNED_INT;
          break;
        }
#endif

#ifdef GLES_TEXFMT_D24S8
      case GLES_TEXFMT_D24S8:
#ifdef GL_DEPTH24_STENCIL8_EXT
        {
          if (apstrFmt) *apstrFmt = "D24S8";
          internalformat = GL_DEPTH24_STENCIL8_EXT;
          format = GL_DEPTH_COMPONENT;
          type = GL_UNSIGNED_INT;
          break;
        }
#endif
#endif

      case GLES_TEXFMT_D24:
// TODO: for GL_DEPTH_COMPONENT24_OES we need to check detect whether GL_OES_depth24
// extension is supported.
#if defined GL_DEPTH_COMPONENT24
        {
          if (apstrFmt) *apstrFmt = "D24";
          internalformat = GL_DEPTH_COMPONENT24;
          format = GL_DEPTH_COMPONENT;
          type = GL_UNSIGNED_INT;
          break;
        }
#endif

      case GLES_TEXFMT_D16:
        {
          if (apstrFmt) *apstrFmt = "D16";
          internalformat = GL_DEPTH_COMPONENT16;
          format = GL_DEPTH_COMPONENT;
          type = GL_UNSIGNED_SHORT;
          break;
        }
    }
  }

  void _FindBestMatch(iGraphics* apGraphics,
                      const iPixelFormat* apPxf,
                      const tTextureFlags aFlags)
  {
    cString strFormat = apPxf->GetFormat();
    const achar* aszFmt = apPxf->GetFormat();

    if (aszFmt[0] == 'D' && ni::StrIsDigit(aszFmt[1])) {
      if (StrICmp(aszFmt, _A("D32")) == 0) {
        _SetTexFmt(GLES_TEXFMT_D32,&strFormat);
      }
      else if (StrICmp(aszFmt, _A("D24")) == 0 || StrICmp(aszFmt, _A("D24X8")) == 0) {
        _SetTexFmt(GLES_TEXFMT_D24,&strFormat);
      }
      else if (StrICmp(aszFmt, _A("D24S8")) == 0) {
#ifdef GLES_TEXFMT_D24S8
        _SetTexFmt(GLES_TEXFMT_D24S8,&strFormat);
#else
        niWarning(niFmt("No stencil support with depth format '%s' falling back to D24.", aszFmt));
        _SetTexFmt(GLES_TEXFMT_D24,&strFormat);
#endif
      }
      else {
        if (StrICmp(aszFmt, _A("D16")) != 0) {
          niWarning(niFmt("Unknown depth format '%s' falling back to D16.", aszFmt));
        }
        _SetTexFmt(GLES_TEXFMT_D16,&strFormat);
      }
    }
#if defined USE_TEXFMT_USE_DEFAULT_ONLY
    else if (apPxf->GetNumABits()) {
      niWarning(niFmt("'%s' texture format not supported, using RGBA_DEFAULT.", apPxf->GetFormat()));
      _SetTexFmt(GLES_TEXFMT_RGBA_DEFAULT,&strFormat);
    }
    else {
      niWarning(niFmt("'%s' texture format not supported, using RGB_DEFAULT.", apPxf->GetFormat()));
        _SetTexFmt(GLES_TEXFMT_RGB_DEFAULT,&strFormat);
    }
#else
    else if (StrICmp(aszFmt, _A("L8A8")) == 0) {
      _SetTexFmt(GLES_TEXFMT_LA16,&strFormat);
    }
    else if (StrICmp(aszFmt, _A("A8")) == 0) {
      _SetTexFmt(GLES_TEXFMT_A8,&strFormat);
    }
    else if (StrNICmp(aszFmt, _A("DXT"), 3) == 0) {
      if (aszFmt[3] == '1') {
        _SetTexFmt(GLES_TEXFMT_RGBA32,&strFormat);
      }
      else if (aszFmt[3] == '3' || aszFmt[3] == '5') {
        _SetTexFmt(GLES_TEXFMT_RGBA32,&strFormat);
      }
    }
    else if (StrICmp(aszFmt, _A("FA32R32G32B32")) == 0 ||
             StrICmp(aszFmt, _A("FA32B32G32R32")) == 0 ||
             StrICmp(aszFmt, _A("FX32R32G32B32")) == 0 ||
             StrICmp(aszFmt, _A("FX32B32G32R32")) == 0 ||
             StrICmp(aszFmt, _A("FR32G32B32A32")) == 0 ||
             StrICmp(aszFmt, _A("FB32G32R32A32")) == 0 ||
             StrICmp(aszFmt, _A("FR32G32B32X32")) == 0 ||
             StrICmp(aszFmt, _A("FB32G32R32X32")) == 0)
    {
      // RGBA32F
      if (hasTexFmtFloat) {
        _SetTexFmt(GLES_TEXFMT_RGBA32F, &strFormat);
      }
      else if (hasTexFmtHalfFloat) {
        niWarning("RGBA32F texture format not supported, using RGBA16F.");
        _SetTexFmt(GLES_TEXFMT_RGBA16F, &strFormat);
      }
      else {
        niWarning("RGBA32F texture format not supported, using RGBA32.");
        _SetTexFmt(GLES_TEXFMT_RGBA32, &strFormat);
      }
    }
    else if (StrICmp(aszFmt, _A("FA16R16G16B16")) == 0 ||
             StrICmp(aszFmt, _A("FA16B16G16R16")) == 0 ||
             StrICmp(aszFmt, _A("FX16R16G16B16")) == 0 ||
             StrICmp(aszFmt, _A("FX16B16G16R16")) == 0 ||
             StrICmp(aszFmt, _A("FR16G16B16A16")) == 0 ||
             StrICmp(aszFmt, _A("FB16G16R16A16")) == 0 ||
             StrICmp(aszFmt, _A("FR16G16B16X16")) == 0 ||
             StrICmp(aszFmt, _A("FB16G16R16X16")) == 0)
    {
      // RGBA16F
      if (hasTexFmtHalfFloat) {
        _SetTexFmt(GLES_TEXFMT_RGBA16F, &strFormat);
      }
      else if (hasTexFmtFloat) {
        niWarning("RGBA16F texture format not supported, using RGBA32F.");
        _SetTexFmt(GLES_TEXFMT_RGBA32F, &strFormat);
      }
      else {
        niWarning("RGBA16F texture format not supported, using RGBA8.");
        _SetTexFmt(GLES_TEXFMT_RGBA32, &strFormat);
      }
    }
    else {
      if (StrICmp(aszFmt, _A("A8R8G8B8")) == 0 ||
          StrICmp(aszFmt, _A("A8B8G8R8")) == 0 ||
          StrICmp(aszFmt, _A("X8R8G8B8")) == 0 ||
          StrICmp(aszFmt, _A("X8B8G8R8")) == 0 ||
          StrICmp(aszFmt, _A("R8G8B8A8")) == 0 ||
          StrICmp(aszFmt, _A("B8G8R8A8")) == 0 ||
          StrICmp(aszFmt, _A("R8G8B8X8")) == 0 ||
          StrICmp(aszFmt, _A("B8G8R8X8")) == 0)
      {
        _SetTexFmt(GLES_TEXFMT_RGBA32,&strFormat);
      }
      else if (StrICmp(aszFmt, _A("R8G8B8")) == 0 ||
               StrICmp(aszFmt, _A("B8G8R8")) == 0) {
        _SetTexFmt(GLES_TEXFMT_RGB24,&strFormat);
      }
      else if (StrICmp(aszFmt, _A("R4G4B4A4")) == 0 ||
               StrICmp(aszFmt, _A("A4R4G4B4")) == 0 ||
               StrICmp(aszFmt, _A("B4G4R4A4")) == 0 ||
               StrICmp(aszFmt, _A("A4B4G4R4")) == 0) {
        _SetTexFmt(GLES_TEXFMT_RGBA16,&strFormat);
      }
      else if (StrICmp(aszFmt, _A("B5G5R5A1")) == 0 ||
               StrICmp(aszFmt, _A("B5G5R5")) == 0 ||
               StrICmp(aszFmt, _A("R5G5B5A1")) == 0 ||
               StrICmp(aszFmt, _A("R5G5B5")) == 0)
      {
        _SetTexFmt(GLES_TEXFMT_RGBA15,&strFormat);
      }
      else if (StrICmp(aszFmt, _A("B5G6R5")) == 0 ||
               StrICmp(aszFmt, _A("R5G6B5")) == 0)
      {
        _SetTexFmt(GLES_TEXFMT_RGB16,&strFormat);
      }
      else if (apPxf->GetNumABits()) {
        niWarning(niFmt("'%s' texture format not supported, using RGBA_DEFAULT.", apPxf->GetFormat()));
        _SetTexFmt(GLES_TEXFMT_RGBA_DEFAULT,&strFormat);
      }
      else {
        niWarning(niFmt("'%s' texture format not supported, using RGB_DEFAULT.", apPxf->GetFormat()));
        _SetTexFmt(GLES_TEXFMT_RGB_DEFAULT,&strFormat);
      }
    }
#endif
    GL_DEBUG_LOG(("BEST MATCH FOR TEXTURE %s -> %s, int:%d, fmt:%d, type:%d",
                  aszFmt,strFormat,internalformat,format,type));
    pxf = apGraphics->CreatePixelFormat(strFormat.Chars());
  }

  tBool IsOK() const {
    return pxf.IsOK();
  }

  tBool _Initialize(iGraphics* apGraphics,
                    eBitmapType aType,
                    const achar* aaszFormat,
                    tU32 anNumMipMaps,
                    tU32 aW, tU32 aH, tU32 anD,
                    const tTextureFlags aFlags)
  {
    niCheckSilent(aType == eBitmapType_2D ||
                  aType == eBitmapType_Cube,
                  eFalse);
    _Set(aType,apGraphics->CreatePixelFormat(aaszFormat),aFlags);
    niCheckSilent(pxf.IsOK(),eFalse);
    _FindBestMatch(apGraphics,pxf,aFlags);
    return eTrue;
  }
};

///////////////////////////////////////////////
struct sOpenglTextureBase : public ImplRC<iGLTexture,eImplFlags_DontInherit1|eImplFlags_DontInherit2,iTexture,iDeviceResource> {
  iGraphicsDriver*  mpDriver;
  tHStringPtr       mhspName;
  sOpenglTextureFormat mFormat;
  GLuint            mGLHandle;
  tIntPtr           mhLastSamplerStateSet;

  sOpenglTextureBase() {
    mhLastSamplerStateSet = 0;
  }

  __forceinline iGraphics* GetGraphics() const {
    return mpDriver->GetGraphics();
  }
};

///////////////////////////////////////////////
struct sOpenglTexture : public sOpenglTextureBase
{
  tU32        mnWidth, mnHeight;
  tU32        mnNumMipMaps;
  Ptr<iBitmap2D> mptrBmpRestore;
  tBool       mbRestore;
  tBool       mbHasBeenReset;
#ifdef USE_FBO
  GLuint      mGLFBOHandle;
#endif
#ifdef GL_DEBUG_MISSING_MIPMAPS
  astl::vector<tBool> mvDidUploadLevel;
#endif

 public:
  ///////////////////////////////////////////////
  sOpenglTexture(iGraphicsDriver* apDriver, iHString* ahspName, tTextureFlags aTextureFlags = 0)
  {
    ZeroMembers();
    mpDriver = apDriver;
    mhspName = ahspName;
    mFormat.flags = aTextureFlags;
    if (niFlagIsNot(mFormat.flags,eTextureFlags_SubTexture)) {
      GetGraphics()->GetTextureDeviceResourceManager()->Register(this);
    }
  }

  ///////////////////////////////////////////////
  ~sOpenglTexture() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    return eTrue;
  }
  void __stdcall ZeroMembers() {
    mnWidth = mnHeight = 0;
    mnNumMipMaps = 0;
    mGLHandle = GLDRV_INVALID_HANDLE;
#ifdef USE_FBO
    mGLFBOHandle = GLDRV_INVALID_HANDLE;
#endif
    mbRestore = eFalse;
    mbHasBeenReset = eFalse;
  }
  void __stdcall Invalidate() {
    _DestroyTextureHandle();
    if (mptrBmpRestore.IsOK()) {
      mptrBmpRestore->Invalidate();
      mptrBmpRestore = NULL;
    }
    if (mpDriver) {
      if (niFlagIsNot(mFormat.flags,eTextureFlags_SubTexture)) {
        GetGraphics()->GetTextureDeviceResourceManager()->Unregister(this);
      }
      mhspName = NULL;
      mpDriver = NULL;
    }
  }

  //// iTexture /////////////////////////////////
  tU32 __stdcall GetGLHandle() const {
    return mGLHandle;
  }
  tU32 __stdcall GetGLFBOHandle() const {
#ifdef USE_FBO
    return mGLFBOHandle;
#else
    return 0;
#endif
  }
  iHString* __stdcall GetDeviceResourceName() const {
    return mhspName;
  }
  virtual tBool  __stdcall HasDeviceResourceBeenReset (tBool abClearFlag) {
    const tBool reset = mbHasBeenReset;
    if (abClearFlag) {
      mbHasBeenReset = eFalse;
    }
    return reset;
  }
  virtual tBool __stdcall ResetDeviceResource () {
    if (hasContextLost) {
      _DestroyTextureHandle();
      mbHasBeenReset = eTrue;
      mbRestore = eTrue;
    }
    return eTrue;
  }

  virtual eBitmapType __stdcall GetType() const {
    return eBitmapType_2D;
  }
  virtual tU32 __stdcall GetWidth() const {
    return mnWidth;
  }
  virtual tU32 __stdcall GetHeight() const {
    return mnHeight;
  }
  virtual tU32 __stdcall GetDepth() const {
    return 0;
  }
  virtual iPixelFormat* __stdcall GetPixelFormat() const {
    return mFormat.pxf;
  }
  virtual tU32 __stdcall GetNumMipMaps() const {
    return mnNumMipMaps;
  }
  virtual tTextureFlags __stdcall GetFlags() const {
    return mFormat.flags;
  }
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    return NULL;
  }
  //// iTexture /////////////////////////////////

  ///////////////////////////////////////////////
  void _InitMainRT(iPixelFormat* apPxf, tU32 anW, tU32 anH) {
    mnWidth = anW;
    mnHeight = anH;
    mnNumMipMaps = 0;
    mFormat._Set(eBitmapType_2D,apPxf,
                 eTextureFlags_RenderTarget|eTextureFlags_Surface|
                 eTextureFlags_MainRT|eTextureFlags_DontOwnGLHandles);
    mbRestore = eFalse;
#ifdef USE_FBO_MAINRT_IS_FBO
    mGLFBOHandle = GLDRV_INVALID_HANDLE;
#else
    mGLFBOHandle = 0;
#endif
  }
  void _InitMainDS(iPixelFormat* apPxf, tU32 anW, tU32 anH) {
    mnWidth = anW;
    mnHeight = anH;
    mnNumMipMaps = 0;
    mFormat._Set(eBitmapType_2D,apPxf,
                 eTextureFlags_DepthStencil|eTextureFlags_Surface|
                 eTextureFlags_MainDS|eTextureFlags_DontOwnGLHandles);
    mbRestore = eFalse;
#ifdef USE_FBO_MAINRT_IS_FBO
    mGLFBOHandle = GLDRV_INVALID_HANDLE;
#else
    mGLFBOHandle = 0;
#endif
  }
  void _InitCubeFace(iGraphics* apGraphics, tIntPtr aGLHandle, const sOpenglTextureFormat& aFmt, eBitmapCubeFace aFace, tU32 anW, tU32 anNumMipMaps) {
    mnWidth = anW;
    mnHeight = anW;
    mnNumMipMaps = anNumMipMaps;
    mGLHandle = aGLHandle;
    mFormat = aFmt;
    mFormat.flags = eTextureFlags_SubTexture|eTextureFlags_Surface|eTextureFlags_DontOwnGLHandles;
    mFormat.kind = GL_CubeMapFace(aFace);
    _InitBitmapRestore(apGraphics);
    mbRestore = eFalse;
  }

  ///////////////////////////////////////////////
  Ptr<iBitmap2D> _CreateMatchingBitmap(iGraphics* apGraphics) {
    return apGraphics->CreateBitmap2DEx(mnWidth,mnHeight,mFormat.pxf);
  }

  ///////////////////////////////////////////////
  tBool _InitBitmapRestore(iGraphics* apGraphics)
  {
    if (hasContextLost) {
      mptrBmpRestore = _CreateMatchingBitmap(apGraphics);
      mptrBmpRestore->Clear();
      if (mnNumMipMaps) {
        mptrBmpRestore->CreateMipMaps(mnNumMipMaps,eFalse);
        niLoop(i,mptrBmpRestore->GetNumMipMaps()) {
          mptrBmpRestore->GetMipMap(i)->Clear();
        }
      }
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _NewTexture(iGraphics* apGraphics,
                    const achar* aaszFormat,
                    tU32 anNumMipMaps,
                    tU32 aW, tU32 aH,
                    const tTextureFlags aFlags)
  {
    mnWidth = aW;
    mnHeight = aH;
    mnNumMipMaps = anNumMipMaps;
    if (!mFormat._Initialize(apGraphics,
                             eBitmapType_2D,
                             aaszFormat,
                             anNumMipMaps,
                             aW,aH,0,
                             aFlags))
    {
      niError(niFmt(_A("Texture [%s] %dx%d: Can't initialize texture format."),
                    niHStr(mhspName), mnWidth, mnHeight));
      return eFalse;
    }

    if (niFlagIs(aFlags,eTextureFlags_RenderTarget) || niFlagIs(aFlags,eTextureFlags_DepthStencil)) {
      // We're done, no bitmap is associated with render targets
    }
    else {
      _InitBitmapRestore(apGraphics);
    }
    mbRestore = eTrue;
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _BlitFromBitmap2D(
    iBitmap2D* apBmp,
    tU32 anDestLevel,
    const sRecti& aSrc,
    const sRecti& aDest,
    eTextureBlitFlags aFlags)
  {
    if (!mpDriver)
      return eFalse;

    if (mptrBmpRestore.IsOK()) {
      //
      // Here we delay upload to video memory to the Bind() call.
      // This is for two reasons :
      // a) glTexSubImage, which would be used in this case, seems to be buggy
      //    on some Open GL implementations (Android).
      // b) It reduces the number of uploads to the video memory to the
      //    minimum required for actual rendering.
      //
      Ptr<iBitmap2D> bmpLevel = mptrBmpRestore->GetLevel(anDestLevel);
      niCheck(bmpLevel.IsOK(),eFalse);

      bmpLevel->BlitStretch(
          apBmp,
          aSrc.GetLeft(),aSrc.GetTop(),
          aDest.GetLeft(),aDest.GetTop(),
          aSrc.GetWidth(),aSrc.GetHeight(),
          aDest.GetWidth(),aDest.GetHeight());

      mbRestore = eTrue;
    }
    else {
      niCheck(_BindAsTexture(),eFalse);

      const tU32 mipWidth = ni::Max(1, mnWidth >> anDestLevel);
      const tU32 mipHeight = ni::Max(1, mnHeight >> anDestLevel);
      // GL_DEBUG_LOG(("-- _BlitFromBitmap2D: src: %dx%d, %s, dest: %dx%d %s destLevel: %d src: %s dest: %s flags: %d",
                    // apBmp->GetWidth(), apBmp->GetHeight(), apBmp->GetPixelFormat()->GetFormat(),
                    // mipWidth, mipHeight, mFormat.pxf->GetFormat(),
                    // anDestLevel, aSrc, aDest, aFlags));

      Ptr<iBitmap2D> mipBmp;
      if (mipWidth != apBmp->GetWidth() || mipHeight != apBmp->GetHeight() ||
          !mFormat.pxf->IsSamePixelFormat(apBmp->GetPixelFormat()))
      {
        mipBmp = GetGraphics()->CreateBitmap2DEx(mipWidth,mipHeight,mFormat.pxf);
        // GL_DEBUG_LOG(("-- _BlitFromBitmap2D: Temporary Bitmap Needed: %s: src: %dx%d, %s, dest: %dx%d %s destLevel: %d src: %s dest: %s flags: %d",
                      // mhspName,
                      // apBmp->GetWidth(), apBmp->GetHeight(), apBmp->GetPixelFormat()->GetFormat(),
                      // mipWidth, mipHeight, mFormat.pxf->GetFormat(),
                      // anDestLevel, aSrc, aDest, aFlags));
        mipBmp->BlitStretch(apBmp,
                            0, 0, 0, 0,
                            apBmp->GetWidth(), apBmp->GetHeight(),
                            mipWidth, mipHeight);
      }
      else {
        mipBmp = apBmp;
      }
      _UploadBoundTextureLevel(anDestLevel, mFormat, mipBmp, mipWidth, mipHeight, anDestLevel);
    }
    return eTrue;
  }

  void __stdcall _CheckShouldRestore() {
    if (mbRestore) {
      if (hasContextLost) {
        _DestroyTextureHandle();
      }
      mbRestore = eFalse;
    }
  }

  tBool __stdcall _BindAsTexture() {
    _CheckShouldRestore();

    tBool bNewHandle = eFalse;
    if (mGLHandle == GLDRV_INVALID_HANDLE) {
      GLDRV_GEN_HANDLE(mGLHandle, _glGenTextures);
      GL_DEBUG_LABEL(GL_TEXTURE, mGLHandle, niHStr(mhspName));
      mhLastSamplerStateSet = 0; // reset the sampler state set
      bNewHandle = eTrue;
    }

    if (mGLHandle != GLDRV_INVALID_HANDLE) {
#if defined USE_GL_ENABLE_FOR_TEXTURE
      _glEnable(mFormat.kind);
      GLERR_RET(eFalse);
#endif
      _glBindTexture(mFormat.kind == GL_TEXTURE_2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, mGLHandle);

      GLERR_RET(eFalse);

      if (bNewHandle) {
        GL_SetInitialTexParameters(mFormat.kind,mnNumMipMaps,
                                   (IsPow2(mnWidth)&&IsPow2(mnHeight)));
        GLERR_RET(eFalse);
        if (mptrBmpRestore.IsOK()) {
          _UploadBoundTexture2D(mptrBmpRestore);
        }
        else {
          _UploadBoundTexture2D(NULL);
        }

        // If this texture is a RT/DS we make sure that the frame/renderbuffer
        // is initialized, this prevents garbage from being rendered if no one
        // draws in the RT.
        sGLCache& cache = GLES3_GetCache(mpDriver);
        if ((mFormat.flags & eTextureFlags_RenderTarget) &&
            (mGLFBOHandle == GLDRV_INVALID_HANDLE))
        {
          if (cache._renderTargetFBO != eInvalidHandle)
          {
            _BindAsRenderTarget();
            _glBindFramebuffer(GL_FRAMEBUFFER, cache._renderTargetFBO);
          }
        }
        else if ((mFormat.flags & eTextureFlags_DepthStencil) &&
                 (mGLFBOHandle == GLDRV_INVALID_HANDLE))
        {
          if (cache._depthStencilFBO != eInvalidHandle) {
            _BindAsDepthStencil();
            _glBindFramebuffer(GL_RENDERBUFFER, cache._depthStencilFBO);
          }
        }
      }
    }
    return eTrue;
  }
#ifdef USE_FBO
  tBool __stdcall _BindAsRenderTarget() {
    const tBool isMainRT = niFlagIs(mFormat.flags,eTextureFlags_MainRT);
    if (isMainRT) {
#ifdef USE_FBO_MAINRT_IS_FBO
      if (mGLFBOHandle == GLDRV_INVALID_HANDLE) {
        _glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&mGLFBOHandle));
        fboMainRTHandle = mGLFBOHandle;
        niLog(Info,niFmt("GL3 MainRT FBO: %d",mGLFBOHandle));
      }
#endif
      // niAssert(mGLFBOHandle != GLDRV_INVALID_HANDLE);
      _glBindFramebuffer(GL_FRAMEBUFFER, mGLFBOHandle);
      return eTrue;
    }

    if (!niFlagIs(mFormat.flags,eTextureFlags_RenderTarget)) {
      niError(niFmt("Texture is not a render target: %s.", mhspName));
      return eFalse;
    }

    _CheckShouldRestore();

    if (hasFBO) {

      tBool bNewHandle = eFalse;
      if (mGLFBOHandle == GLDRV_INVALID_HANDLE) {
        GLDRV_GEN_HANDLE(mGLFBOHandle, _glGenFramebuffers);
        bNewHandle = eTrue;
        niDebugFmt(("Create Render target -> %s", mGLFBOHandle));
      }

      // niDebugFmt(("glBindFramebuffer -> %s", mGLFBOHandle));
      _glBindFramebuffer(GL_FRAMEBUFFER, mGLFBOHandle);
      GLERR_RET(eFalse);
      sGLCache& cache = GLES3_GetCache(mpDriver);
      cache._renderTargetFBO = mGLFBOHandle;

      if (bNewHandle) {
        if (!this->_BindAsTexture()) {
          niError("Can't bind texture to attach to RT.");
          return eFalse;
        }
        _glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mGLHandle, 0);
        GLERR_RET(eFalse);
        // Clear RT to black
        GLES3_DoClear(mpDriver, eClearBuffersFlags_Color, 0, 1.0f, 0);
      }
    }

    return eTrue;
  }

  tBool __stdcall _BindAsDepthStencil() {
    const tBool isMainDS = niFlagIs(mFormat.flags,eTextureFlags_MainDS);
    if (isMainDS) {
#ifdef USE_FBO_MAINRT_IS_FBO
      if (mGLFBOHandle == GLDRV_INVALID_HANDLE) {
        _glGetIntegerv(GL_RENDERBUFFER_BINDING, reinterpret_cast<GLint*>(&mGLFBOHandle));
        fboMainDSHandle = mGLFBOHandle;
        niLog(Info,niFmt("GL3 MainDS FBO: %d",mGLFBOHandle));
      }
#endif
      niAssert(mGLFBOHandle != eInvalidHandle);
      _glBindRenderbuffer(GL_RENDERBUFFER, mGLFBOHandle);
      return eTrue;
    }

    if (!niFlagIs(mFormat.flags,eTextureFlags_DepthStencil)) {
      niError(niFmt("Texture is not a depth stencil: %s.", mhspName));
      return eFalse;
    }

    _CheckShouldRestore();

    if (hasFBO) {
      tBool bNewHandle = eFalse;
      if (mGLFBOHandle == GLDRV_INVALID_HANDLE) {
        GLDRV_GEN_HANDLE(mGLFBOHandle, _glGenRenderbuffers);
        bNewHandle = eTrue;
      }

      _glBindRenderbuffer(GL_RENDERBUFFER, mGLFBOHandle);
      GLERR_RET(eFalse);
      sGLCache& cache = GLES3_GetCache(mpDriver);
      cache._depthStencilFBO = mGLFBOHandle;

      if (bNewHandle) {
        _glRenderbufferStorage(GL_RENDERBUFFER, mFormat.internalformat, mnWidth, mnHeight);
        GLERR_RET(eFalse);
      }
      _glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mGLFBOHandle);
      GLERR_RET(eFalse);

      if (bNewHandle) {
        GLES3_DoClear(mpDriver, eClearBuffersFlags_Depth, 0, 1.0f, 0);
      }
    }

    return eTrue;
  }
#endif // #endif USE_FBO

  ///////////////////////////////////////////////
  virtual iDeviceResource* __stdcall Bind(iUnknown *apDevice) {
    if (apDevice == (iUnknown*)(eInvalidHandle)) {
#pragma niTodo("This is a crappy hack so that a render target doesnt get bound as a texture")
      return this;
    }
    if (!_BindAsTexture()) {
      niWarning("Can't bind OpenGL 2d texture.");
    }

#ifdef GL_DEBUG_MISSING_MIPMAPS
    if (mvDidUploadLevel.empty()) {
      niDebugFmt(("... Texture '%s' not uploaded.",mhspName));
      return NULL;
    }
    if (mvDidUploadLevel.size() != 1) {
      niLoop(i,mvDidUploadLevel.size()) {
        if (!mvDidUploadLevel[i]) {
          niDebugFmt(("... Texture '%s' level '%d' not uploaded.",mhspName,i));
          return NULL;
        }
      }
    }
#endif

    return this;
  }

  ///////////////////////////////////////////////
  void _DestroyTextureHandle() {
    if (niFlagIs(mFormat.flags,eTextureFlags_DontOwnGLHandles))
      return;

#ifdef USE_FBO
    if (niFlagIs(mFormat.flags,eTextureFlags_DepthStencil)) {
      if (mGLFBOHandle != GLDRV_INVALID_HANDLE) {
        GLDRV_GEN_DELETE_HANDLE(mGLFBOHandle, _glDeleteRenderbuffers);
        mGLFBOHandle = GLDRV_INVALID_HANDLE;
      }
    }
    else {
      if (mGLFBOHandle != GLDRV_INVALID_HANDLE) {
        GLDRV_GEN_DELETE_HANDLE(mGLFBOHandle, _glDeleteFramebuffers);
        mGLFBOHandle = GLDRV_INVALID_HANDLE;
      }
    }
#endif

    if (mGLHandle != GLDRV_INVALID_HANDLE) {
      GLDRV_GEN_DELETE_HANDLE(mGLHandle, _glDeleteTextures);
      mGLHandle = GLDRV_INVALID_HANDLE;
    }
  }

  ///////////////////////////////////////////////
  tBool _UploadBoundTexture2D(iBitmap2D* apBmp) {
    niAssert(mFormat._IsValidGLFormat());

    const tU32 numLevels = (mnNumMipMaps == 0) ? 1 : ni::ComputeNumPow2Levels(mnWidth,mnHeight);

    // GL_DEBUG_LOG((
        // "-- UploadBoundTexture2D: kind: %X, handle: %d (%p) (%dx%d pxf:%s name:%s levels:%d)",
        // mFormat.kind,mGLHandle, (void*)this,mnWidth,mnHeight,mFormat.pxf->GetFormat(),mhspName,numLevels));

    {
      tU32 i = 0;
      tU32 w = mnWidth;
      tU32 h = mnHeight;
      for ( ; i < numLevels; ++i) {
        // GL_DEBUG_LOG(("... [%s] uploading mipmap:%d (%dx%d)", mhspName, i, w, h));
        if (!_UploadBoundTextureLevel(i, mFormat, apBmp ? apBmp->GetLevel(i) : NULL, w, h, i)) {
          return eFalse;
        }
        if (w > 1) w >>= 1;
        if (h > 1) h >>= 1;
      }
      if (i < numLevels) {
        // fill down to 1x1 mip map... this is required by OpenGL ES
        for ( ; i < numLevels; ++i) {
          // GL_DEBUG_LOG(("... [%s] uploading padding mipmap:%d (%dx%d)", mhspName, i, w, h));
          if (!_UploadBoundTextureLevel(i, mFormat, apBmp ? apBmp->GetLevel(mnNumMipMaps) : NULL, w, h, i)) {
            return eFalse;
          }
          // GL_DEBUG_LOG(("GL3 Uploaded Dummy Mipmap '%d' to VidMem (%d)",i,++_kNumGL3TexUpload));
          if (w > 1) w >>= 1;
          if (h > 1) h >>= 1;
        }
      }
    }

    // GL_DEBUG_LOG(("Restoring content of %p done.",(void*)this));
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _UploadBoundTextureLevel(
      const tU32 anLevel,
      const sOpenglTextureFormat& aFormat,
      const iBitmap2D* apBmpLevel,
      const tU32 anW, const tU32 anH,
      const tU32 anMipmap)
  {
    niAssert(aFormat._IsValidGLFormat());

#ifdef GL_DEBUG_MISSING_MIPMAPS
    tPtr pTmp = NULL;
    ASTL_SCOPE_EXIT {
      if (pTmp) {
        niFree(pTmp);
      }
    };
#endif

    tPtr pData = apBmpLevel ? apBmpLevel->GetData() : NULL;

#ifdef GL_DEBUG_MISSING_MIPMAPS
    {
      const tU32 numLevels = (mnNumMipMaps == 0) ? 1 : ni::ComputeNumPow2Levels(mnWidth,mnHeight);
      if (mvDidUploadLevel.empty()) {
        mvDidUploadLevel.resize(numLevels, eFalse);
      }
      mvDidUploadLevel[anLevel] = !!pData;
    }
    if (!pData && anLevel != 0) {
      pData = pTmp = (tPtr)niMalloc(anW * anH * 4);
      niLoop(i,anW*anH) {
        ((tU32*)pTmp)[i] = ni::ULColorBuild(255,0,255,255);
      }
    }
#endif

    // GL_DEBUG_LOG((
        // "-- UploadBoundTextureLevel[%d]: kind: %X res:%dx%d pxf:%s mipmap:%d data:%p",
        // anMipmap, aFormat.kind, anW, anH, aFormat.pxf->GetFormat(), anMipmap, (tIntPtr)pData));

    _glTexImage2D(aFormat.kind, anMipmap,
                  aFormat.internalformat,
                  anW, anH, 0,
                  aFormat.format, aFormat.type,
                  pData);
    GLERR_RET(eFalse);

    // GL_DEBUG_LOG(("Restoring content of %p done.",(void*)this));
    return eTrue;
  }
};

///////////////////////////////////////////////
struct sOpenglTextureCube : public sOpenglTextureBase
{
  tU32              mnWidth;
  tU32              mnNumMipMaps;
  Ptr<sOpenglTexture>  mFaces[6];

 public:
  ///////////////////////////////////////////////
  sOpenglTextureCube(iGraphicsDriver* apDriver, iHString* ahspName)
  {
    ZeroMembers();
    mpDriver = apDriver;
    mhspName = ahspName;
    GetGraphics()->GetTextureDeviceResourceManager()->Register(this);
  }

  ///////////////////////////////////////////////
  ~sOpenglTextureCube() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    return eTrue;
  }
  void __stdcall ZeroMembers() {
    mnWidth = 0;
    mnNumMipMaps = 0;
    mGLHandle = GLDRV_INVALID_HANDLE;
  }
  void __stdcall Invalidate() {
    _DestroyCubeTextureHandle();
    if (mpDriver) {
      GetGraphics()->GetTextureDeviceResourceManager()->Unregister(this);
      mhspName = NULL;
      mpDriver = NULL;
    }
  }

  //// iTexture /////////////////////////////////
  tU32 __stdcall GetGLHandle() const {
    return mGLHandle;
  }
  tU32 __stdcall GetGLFBOHandle() const {
    return 0;
  }
  iHString* __stdcall GetDeviceResourceName() const {
    return mhspName;
  }
  virtual tBool  __stdcall HasDeviceResourceBeenReset (tBool abClearFlag) {
    tBool hasBeenReset = eFalse;
    if (mFaces[0].IsOK()) {
      niLoop(i,6) {
        hasBeenReset = mFaces[i]->HasDeviceResourceBeenReset(abClearFlag) || hasBeenReset;
      }
    }
    return hasBeenReset;
  }
  virtual tBool __stdcall ResetDeviceResource () {
    niLoop(i,6) {
      if (mFaces[i].IsOK()) {
        mFaces[i]->ResetDeviceResource();
      }
    }
    _DestroyCubeTextureHandle();
    return eTrue;
  }

  virtual iDeviceResource* __stdcall Bind(iUnknown *apDevice) {
#if defined USE_GL_ENABLE_FOR_TEXTURE
    _glEnable(GL_TEXTURE_CUBE_MAP);
#endif
    _CreateCubeTextureHandle();

    if (mGLHandle != GLDRV_INVALID_HANDLE) {
      _glBindTexture(GL_TEXTURE_CUBE_MAP,mGLHandle);
      GL_SetInitialTexParameters(
          GL_TEXTURE_CUBE_MAP,mnNumMipMaps,IsPow2(mnWidth));

      niLoop(i,6) {
        sOpenglTexture* pFace = mFaces[i];
        if (pFace && pFace->mbRestore) {
          pFace->mGLHandle = mGLHandle;
          pFace->_UploadBoundTexture2D(pFace->mptrBmpRestore);
          pFace->mbRestore = eFalse;
        }
      }
    }

    return this;
  }

  virtual eBitmapType __stdcall GetType() const {
    return eBitmapType_Cube;
  }
  virtual tU32 __stdcall GetWidth() const {
    return mnWidth;
  }
  virtual tU32 __stdcall GetHeight() const {
    return mnWidth;
  }
  virtual tU32 __stdcall GetDepth() const {
    return 0;
  }
  virtual iPixelFormat* __stdcall GetPixelFormat() const {
    return mFormat.pxf;
  }
  virtual tU32 __stdcall GetNumMipMaps() const {
    return mnNumMipMaps;
  }
  virtual tTextureFlags __stdcall GetFlags() const {
    return mFormat.flags;
  }
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    if (anIndex >= 6) return NULL;
    return mFaces[anIndex];
  }
  //// iTexture /////////////////////////////////


  ///////////////////////////////////////////////
  void _CreateCubeTextureHandle() {
    if (mGLHandle == GLDRV_INVALID_HANDLE) {
      GLDRV_GEN_HANDLE(mGLHandle, _glGenTextures);
      mhLastSamplerStateSet = 0; // reset the sampler state set
    }
  }
  void _DestroyCubeTextureHandle() {
    if (mGLHandle != GLDRV_INVALID_HANDLE) {
      GLDRV_GEN_DELETE_HANDLE(mGLHandle, _glDeleteTextures);
      mGLHandle = GLDRV_INVALID_HANDLE;
    }
  }

  ///////////////////////////////////////////////
  tBool _NewTexture(iGraphics* apGraphics,
                    const achar* aaszFormat,
                    tU32 anNumMipMaps,
                    tU32 aW,
                    const tTextureFlags aFlags)
  {
    mnWidth = aW;
    mnNumMipMaps = anNumMipMaps;
    if (!mFormat._Initialize(apGraphics,
                             eBitmapType_Cube,
                             aaszFormat,
                             anNumMipMaps,
                             aW,aW,0,
                             aFlags))
    {
      niError(niFmt(_A("CubeTexture [%s] %d: Can't initialize texture format."),
                    mhspName, mnWidth));
      return eFalse;
    }

    _CreateCubeTextureHandle();

    niLoop(i,6) {
      mFaces[i] = niNew sOpenglTexture(mpDriver,NULL,eTextureFlags_SubTexture);
      mFaces[i]->_InitCubeFace(apGraphics,mGLHandle,mFormat,(eBitmapCubeFace)i,mnWidth,mnNumMipMaps);
      if (!mFaces[i]->_BindAsTexture()) {
        niError(niFmt(_A("CubeTexture [%s] %d: Can't bind face %d."),
                      mhspName, mnWidth, i));
        return eFalse;
      }
      if (!mFaces[i]->_UploadBoundTexture2D(NULL)) {
        niError(niFmt(_A("CubeTexture [%s] %d: Can't initialize face %d."),
                      mhspName, mnWidth, i));
        return eFalse;
      }
    }

    return eTrue;
  }
};

//--------------------------------------------------------------------------------------------
//
//  GL Occlusion Queries
//
//--------------------------------------------------------------------------------------------
#ifdef USE_OQ

//////////////////////////////////////////////////////////////////////////////////////////////
// cGL3OcclusionQuery declaration.
class cGL3OcclusionQuery : public ni::ImplRC<ni::iOcclusionQuery>
{
  niBeginClass(cGL3OcclusionQuery);

 public:
  //! Constructor.
  cGL3OcclusionQuery() {
    ZeroMembers();
    GLDRV_GEN_HANDLE(mGLHandle, _glGenQueries);
    GLERR_RET(;);
  }
  //! Destructor.
  ~cGL3OcclusionQuery() {
    Invalidate();
  }

  //! Zeros all the class members.
  void ZeroMembers() {
    mGLHandle = GLDRV_INVALID_HANDLE;
    mStatus = eOcclusionQueryStatus_NotIssued;
    mnResult = eInvalidHandle;
  }

  //! Sanity check.
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cGL3OcclusionQuery);
    return mGLHandle != GLDRV_INVALID_HANDLE;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (mGLHandle != GLDRV_INVALID_HANDLE) {
      GLDRV_GEN_DELETE_HANDLE(mGLHandle, _glDeleteQueries);
      mGLHandle = GLDRV_INVALID_HANDLE;
    }
  }

  ///////////////////////////////////////////////
  iHString* __stdcall GetDeviceResourceName() const {
    return NULL;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eFalse;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Begin() {
    if (mGLHandle == GLDRV_INVALID_HANDLE)
      return eFalse;
    _glBeginQuery(GL_SAMPLES_PASSED_ARB,mGLHandle);
    mStatus = eOcclusionQueryStatus_Began;
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall End() {
    if (mGLHandle == GLDRV_INVALID_HANDLE)
      return eFalse;
    _glEndQuery(GL_SAMPLES_PASSED_ARB);
    mStatus = eOcclusionQueryStatus_Pending;
    return eTrue;
  }

  ///////////////////////////////////////////////
  ni::eOcclusionQueryStatus __stdcall GetStatus(tBool abWait) {
    if (mGLHandle == GLDRV_INVALID_HANDLE)
      mStatus = eOcclusionQueryStatus_Failed;

    if (mStatus == eOcclusionQueryStatus_Pending)
    {
      if (!abWait) {
        // not waiting for query to complete, so we check if the result
        // is available first, if not we return immediately
        GLint avail;
        _glGetQueryObjectiv(mGLHandle, GL_QUERY_RESULT_AVAILABLE, &avail);
        if (!avail) {
          mStatus = eOcclusionQueryStatus_Pending;
          return mStatus;
        }
      }

      GLuint fragments;
      _glGetQueryObjectuiv(mGLHandle,GL_QUERY_RESULT_ARB,&fragments);
      mStatus = eOcclusionQueryStatus_Successful;
      mnResult = fragments;
    }
    return mStatus;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetResult() const {
    if (niThis(cGL3OcclusionQuery)->GetStatus(eFalse) != eOcclusionQueryStatus_Successful)
      return eInvalidHandle;
    return mnResult;
  }

private:
  GLuint                  mGLHandle;
  eOcclusionQueryStatus   mStatus;
  tU32                    mnResult;
  niEndClass(cGL3OcclusionQuery);
};

#endif

//--------------------------------------------------------------------------------------------
//
//  OpenGL Buffer
//
//--------------------------------------------------------------------------------------------
struct sGLVertexArrayData : public ImplRC<iVertexArray,eImplFlags_DontInherit1,iDeviceResource> {
  __forceinline const GLenum GetBufferTarget() const { return GL_ARRAY_BUFFER; }
  __forceinline const tPtr GetBufferData() const { return (tPtr)mVertices.data(); }
  __forceinline const tU32 GetBufferDataSize() const { return (tU32)mVertices.size(); }
  __forceinline const tU32 GetBufferElSize() const { return mFVF.GetStride(); }
  __forceinline const tU32 GetBufferElCount() const { return (tU32)mVertices.size() / mFVF.GetStride(); }

  astl::vector<tU8> mVertices;
  cFVFDescription   mFVF;
};

struct sGLIndexArray32Data : public ImplRC<iIndexArray,eImplFlags_DontInherit1,iDeviceResource> {
  __forceinline const GLenum GetBufferTarget() const { return GL_ELEMENT_ARRAY_BUFFER; }
  __forceinline const tPtr GetBufferData() const { return (tPtr)mIndices.data(); }
  __forceinline const tU32 GetBufferDataSize() const { return (tU32)mIndices.size() * sizeof(tU32); }
  __forceinline const tU32 GetBufferElSize() const { return sizeof(tU32); }
  __forceinline const tU32 GetBufferElCount() const { return (tU32)mIndices.size(); }

  astl::vector<tU32> mIndices;
};

template <typename TDATA>
struct sGLBufferImpl : public TDATA {
  sGLBufferImpl(iDeviceResourceManager* apDevResMan, eArrayUsage aUsage) {
    mptrDevResMan = apDevResMan;
    mptrDevResMan->Register(this);

    mbRestore = eFalse;
    mGLHandle = GLDRV_INVALID_HANDLE;
    mUsage = aUsage;
    mLockFlags = eInvalidHandle;
    mLockFirst = 0;
    mLockNum = 0;
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING
    mbShouldOrphanDynamicBuffer = eFalse;
#endif
  }

  void __stdcall Invalidate() niImpl {
    if (!mptrDevResMan.IsOK())
      return;
    Unlock();
    mptrDevResMan->Unregister(this);
    mptrDevResMan = NULL;
    _DestroyBufferHandle();
  }

  iHString* __stdcall GetDeviceResourceName() const niImpl {
    return NULL;
  }

  tBool __stdcall IsOK() const niImpl {
    return eTrue;
  }

  eArrayUsage __stdcall GetUsage() const niImpl {
    return mUsage;
  }

  tPtr __stdcall Lock(tU32 ulFirst, tU32 ulNum, eLock aLock) niImpl {
    if (!mptrDevResMan.IsOK() || mLockFlags != eInvalidHandle)
      return NULL;

    if (ulNum == 0) {
      ulNum = this->GetBufferElCount() - ulFirst;
    }

    mLockFlags = aLock;
    mLockFirst = ulFirst;
    mLockNum = ulNum;

    return this->GetBufferData()+(this->GetBufferElSize()*ulFirst);
  }

  tBool __stdcall Unlock() niImpl {
    if (mLockFlags == eInvalidHandle)
      return eFalse;

    if (!niFlagIs(mLockFlags,eLock_ReadOnly)) {
      const GLenum bufferTarget = this->GetBufferTarget();
      const tPtr bufferData = this->GetBufferData();
      const tU32 elSize = this->GetBufferElSize();
      const tU32 elCount = this->GetBufferElCount();

      const tBool justCreated = _CreateBufferHandle();
      _glBindBuffer(bufferTarget,mGLHandle);
      if (mUsage == eArrayUsage_Static) {
        if (justCreated) {
          _glBufferData(
            bufferTarget,
            elSize*elCount,
            bufferData,
            GL_STATIC_DRAW);
        }
        else {
          _glBufferSubData(bufferTarget,
                           (elSize*mLockFirst),
                           (elSize*mLockNum),
                           bufferData+(elSize*mLockFirst));
        }
      }
      else {
        if (
            justCreated
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING
            || mbShouldOrphanDynamicBuffer
#endif
        )
        {
#if GL_DYNAMIC_BUFFER_DEBUG && (GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING)
          if (mbShouldOrphanDynamicBuffer) {
            niDebugFmt((
              "... GL_DYNAMIC_BUFFER '%p' bound since last update, orphaned. %d bytes total, %d bytes update.",
              (tIntPtr)this,
              elSize*elCount,
              elSize*mLockNum));
          }
#endif

          _glBufferData(
            bufferTarget,
            elSize*elCount,
            NULL,
            GL_DYNAMIC_DRAW);
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING
          mbShouldOrphanDynamicBuffer = eFalse;
#endif
        }
#if GL_DYNAMIC_BUFFER_DEBUG && (GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING)
        else {
            niDebugFmt((
              "... GL_DYNAMIC_BUFFER '%p' not bound since last update, use as-is. %d bytes total, %d bytes update.",
              (tIntPtr)this,
              elSize*elCount,
              elSize*mLockNum));
        }
#endif
        _glBufferSubData(bufferTarget,
                         (elSize*mLockFirst),
                         (elSize*mLockNum),
                         bufferData+(elSize*mLockFirst));
      }
      _glBindBuffer(bufferTarget,0);
    }

    mLockFlags = eInvalidHandle;
    return eTrue;
  }

  tBool __stdcall GetIsLocked() const niImpl {
    return mLockFlags != eInvalidHandle;
  }

  tBool _CreateBufferHandle() {
    if (mGLHandle == GLDRV_INVALID_HANDLE) {
      GLDRV_GEN_HANDLE(mGLHandle, _glGenBuffers);
      return eTrue;
    }
    return eFalse;
  }
  void _DestroyBufferHandle() {
    if (mGLHandle != GLDRV_INVALID_HANDLE) {
      GLDRV_GEN_DELETE_HANDLE(mGLHandle, _glDeleteBuffers);
    }
  }

  iDeviceResource* __stdcall Bind(iUnknown*) niImpl {
    const GLenum bufferTarget = this->GetBufferTarget();
    if (mbRestore) {
      GL3_TRACE_BUFFER((">>> Restoring GLBuffer: target: %d, usage: %s, elSize: %d, elCount: %d, dataSize: %db (%gMB).",
                  (tU32)bufferTarget,
                  niEnumToChars(eArrayUsage, mUsage),
                  this->GetBufferElSize(),
                  this->GetBufferElCount(),
                  this->GetBufferDataSize(),
                  ((tF64)this->GetBufferDataSize())/(1024.0*1024.0)));
      // Create a "fake" lock so that the whole buffer is restored when Unlock() is called
      mLockFirst = 0;
      mLockNum = this->GetBufferElCount();
      mLockFlags = eLock_Discard;
      Unlock();
      mbRestore = eFalse;
    }

    if (mGLHandle == GLDRV_INVALID_HANDLE) {
      niError(niFmt("Trying to bind a GLBuffer without data: target: %d, usage: %s, elSize: %d, elCount: %d, dataSize: %db (%gMB).",
                    (tU32)this->GetBufferTarget(),
                    niEnumToChars(eArrayUsage, mUsage),
                    this->GetBufferElSize(),
                    this->GetBufferElCount(),
                    this->GetBufferDataSize(),
                    ((tF64)this->GetBufferDataSize())/(1024.0*1024.0)));
      return NULL;
    }
    _glBindBuffer(bufferTarget,mGLHandle);
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING
    // If we're using orphaning we should orphan the buffer after its used for
    // rendering.
    mbShouldOrphanDynamicBuffer = eTrue;
#endif
    return this;
  }

  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  tBool __stdcall ResetDeviceResource() niImpl {
    _DestroyBufferHandle();
    mbRestore = eTrue;
    return eTrue;
  }

  Ptr<iDeviceResourceManager> mptrDevResMan;
  tU32                        mLockFlags;
  tU32                        mLockFirst;
  tU32                        mLockNum;
  GLuint                      mGLHandle;
  eArrayUsage                 mUsage;
  tBool                       mbRestore;
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING
  tBool                       mbShouldOrphanDynamicBuffer;
#endif
};

struct sGLVertexArray : public sGLBufferImpl<sGLVertexArrayData> {
  sGLVertexArray(iDeviceResourceManager* apDevResMan, tU32 anNumVertices, tFVF aFVF, eArrayUsage aUsage)
      : sGLBufferImpl(apDevResMan, aUsage)
  {
    mFVF.Setup(aFVF);
    mVertices.resize(mFVF.GetStride()*anNumVertices);
    ni::MemZero((tPtr)mVertices.data(),mVertices.size());

    GL3_TRACE_BUFFER((">>> Created VertexArray GLBuffer: %p, target: %d, usage: %s, elSize: %d, elCount: %d, dataSize: %db (%gMB).",
                (tIntPtr)this,
                (tU32)this->GetBufferTarget(),
                niEnumToChars(eArrayUsage, mUsage),
                this->GetBufferElSize(),
                this->GetBufferElCount(),
                this->GetBufferDataSize(),
                ((tF64)this->GetBufferDataSize())/(1024.0*1024.0)));
  }
  ~sGLVertexArray()
  {
    Invalidate();
  }

  tFVF __stdcall GetFVF() const {
    return mFVF.GetFVF();
  }
  tU32 __stdcall GetNumVertices() const {
    return this->GetBufferElCount();
  }
};

struct sGLIndexArray32 : public sGLBufferImpl<sGLIndexArray32Data> {
  sGLIndexArray32(
      iDeviceResourceManager* apDevResMan,
      eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices,
      tU32 anMaxVertexIndex, eArrayUsage aUsage)
      : sGLBufferImpl(apDevResMan, aUsage)
  {
    mnMaxVertexIndex = anMaxVertexIndex;
    mPrimitiveType = aPrimitiveType;
    mIndices.resize(anNumIndices);
    ni::MemZero((tPtr)mIndices.data(),mIndices.size() * sizeof(*mIndices.begin()));

    GL3_TRACE_BUFFER((">>> Created IndexArray32 GLBuffer: %p, target: %d, usage: %s, elSize: %d, elCount: %d, dataSize: %db (%gMB).",
                (tIntPtr)this,
                (tU32)this->GetBufferTarget(),
                niEnumToChars(eArrayUsage, mUsage),
                this->GetBufferElSize(),
                this->GetBufferElCount(),
                this->GetBufferDataSize(),
                ((tF64)this->GetBufferDataSize())/(1024.0*1024.0)));
  }
  ~sGLIndexArray32()
  {
    Invalidate();
  }

  eGraphicsPrimitiveType __stdcall GetPrimitiveType() const niImpl {
    return mPrimitiveType;
  }
  tU32 __stdcall GetNumIndices() const niImpl {
    return (tU32)mIndices.size();
  }
  tU32 __stdcall GetMaxVertexIndex() const niImpl {
    return mnMaxVertexIndex;
  }

  eGraphicsPrimitiveType  mPrimitiveType;
  tU32                    mnMaxVertexIndex;
};

//--------------------------------------------------------------------------------------------
//
// Graphics Context
//
//--------------------------------------------------------------------------------------------
class cGL3ContextWindow : public sGLContext
{
  niBeginClass(cGL3ContextWindow);

 public:
  iGraphicsDriver* mpDrv;
  Ptr<iOSWindow> mptrWindow;
  tU32 mnSwapInterval;
  tTextureFlags mnBBFlags;
  Ptr<iPixelFormat> mptrBBPxf, mptrDSPxf;
#ifdef TSGL_CONTEXT
  tsglContext* mpTSGLContext;
#endif

  cGL3ContextWindow(
      iGraphicsDriver* apParent,
      iOSWindow* apWindow,
      const achar* aaszBBPxf,
      const achar* aaszDSPxf,
      tU32 anSwapInterval,
      tTextureFlags anBBFlags)
      : sGLContext(apParent->GetGraphics(), apParent)
  {
#ifdef TSGL_CONTEXT
    mpTSGLContext = NULL;
#endif
    mpDrv = apParent;
    niAssert(mpDrv != NULL);
    mptrWindow = apWindow;
    mnSwapInterval = anSwapInterval;
    mnBBFlags = anBBFlags;
    mnSyncCounter = 0;

    // NOTE: GL libraries on Windows can only load after window created
#ifndef _WIN32
    if (tsglLoadLibrary() != TSGL_OK) {
      niError("Can't initialize TSGL.");
      return;
    }
#endif

    mptrBBPxf = mpDrv->GetGraphics()->CreatePixelFormat(aaszBBPxf);
    if (!mptrBBPxf.IsOK())
      return;

    mptrDSPxf = mpDrv->GetGraphics()->CreatePixelFormat(aaszDSPxf);
    if (!mptrDSPxf.IsOK())
      return;

    // ChangeWindow, setup the context size, etc...
    if (!_DoResizeContext()) {
      Invalidate();
      return;
    }
  }
  ~cGL3ContextWindow() {
    Invalidate();
  }

  ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) niImpl {
    if (aIID == niGetInterfaceUUID(ni::iOSWindow))
      return mptrWindow;
    return BaseImpl::QueryInterface(aIID);
  }
  void __stdcall ListInterfaces(ni::iMutableCollection* apLst, ni::tU32 anFlags) const niImpl {
    apLst->Add(niGetInterfaceUUID(ni::iOSWindow));
    BaseImpl::ListInterfaces(apLst,anFlags);
  }

#ifdef TSGL_CONTEXT
  tsglContext* __stdcall GetTSGLContext() const {
    return mpTSGLContext;
  }
#endif

  tBool __stdcall _DoResizeContext() {
    niCheck(mptrWindow.IsOK(),eFalse);

    if (hasContextLost
#ifdef TSGL_CONTEXT
        && mpTSGLContext
#endif
        )
    {
      GLES3_ResetContextDeviceResources(mpDrv);
      GLES3_ResetCache(mpDrv);
    }

    const tU32 w = mptrWindow->GetClientSize().x;
    const tU32 h = mptrWindow->GetClientSize().y;

    if (hasContextLost
        || !mptrRT[0].IsOK()
#ifdef TSGL_CONTEXT
        || !mpTSGLContext
#endif
        )
    {
#ifdef TSGL_CONTEXT
      if (niFlagIsNot(mnBBFlags,eTextureFlags_Virtual)) {
        // Re-create the context, this is so that we mimic Android's behavior as closely as possible
        _DestroyContext();
        if (tsglCreateContext(
              &mpTSGLContext,mptrWindow,
              32,24,8,GetNumAASamples(),
              mnSwapInterval) != TSGL_OK)
        {
          niError(_A("Can't create TSGL context."));
          return eFalse;
        }
        tsglMakeCurrent(mpTSGLContext);
      }
#endif

      // Initialize extensions
      if (!GL3_InitializeExt()) {
        niError("Can't initialize OpenGL extensions.");
        return eFalse;
      }

      // Initialize main RT
      {
        if (!mptrRT[0].IsOK()) {
          mptrRT[0] = niNew sOpenglTexture(mpDrv,_H("GLES3_MainRT"));
        }
        sOpenglTexture* pGLTex = (sOpenglTexture*)mptrRT[0].ptr();
        pGLTex->_InitMainRT(mptrBBPxf,w,h);
      }

      // Initialize main DS
      {
        if (!mptrDS.IsOK()) {
          mptrDS = niNew sOpenglTexture(mpDrv,_H("GLES3_MainDS"));
        }
        sOpenglTexture* pGLTex = (sOpenglTexture*)mptrDS.ptr();
        pGLTex->_InitMainDS(mptrDSPxf,w,h);
      }
    }
    else {
      // Resize-only stuff
      {
        sOpenglTexture* pGLTex = (sOpenglTexture*)mptrRT[0].ptr();
        pGLTex->mnWidth = w;
        pGLTex->mnHeight = h;
      }
      {
        sOpenglTexture* pGLTex = (sOpenglTexture*)mptrDS.ptr();
        pGLTex->mnWidth = w;
        pGLTex->mnHeight = h;
      }
    }

    mrectScissor = sRecti(0,0,w,h);
    mrectViewport = mrectScissor;

    mEncoder->_renderingInfo._rtSize = mrectScissor.GetSize();
    mEncoder->_renderingInfo._rtFlipped = niFlagIs(mptrRT[0]->GetFlags(),eTextureFlags_RTFlipped);
    mEncoder->_renderingInfo._viewport = mrectScissor;

    niDebugFmt((_A("GLES3 Context - Resized [%p]: %dx%d, BB: %s, DS: %s, AA: %s, VP: %s, SC: %s"),
                (tIntPtr)this,
                w,h,
                mptrBBPxf->GetFormat(),
                mptrDSPxf->GetFormat(),
                GetNumAASamples(),
                this->GetViewport(),
                this->GetScissorRect()));

    if (hasContextLost) {
      GLES3_InitContextDeviceResources(mpDrv);
    }
    return eTrue;
  }
  void _DestroyContext() {
#ifdef TSGL_CONTEXT
    if (mpTSGLContext) {
      tsglDestroyContext(mpTSGLContext);
      mpTSGLContext = NULL;
    }
#endif
  }

  const tU32 GetNumAASamples() const {
    return (mnBBFlags&eTextureFlags_RTAA_All)?4:0;
  }

  virtual tBool __stdcall IsOK() const {
    return mpDrv != nullptr;
  }

  virtual void __stdcall Invalidate() {
    if (mpDrv) {
      GLES3_ResetContextDeviceResources(mpDrv);
      _DestroyContext();
      mpDrv = NULL;
    }
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return mpDrv?mpDrv->GetGraphics():NULL;
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mpDrv;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    niCheck(_beganFrame,eFalse);
    _beganFrame = eFalse;

    GLES3_SwapBuffers(mpDrv,this,niFlagIs(aFlags,eGraphicsDisplayFlags_DoNotWait));
    ++mnSyncCounter; // Make sure the Viewport and scissor will be set next frame


    if (mptrWindow.IsOK()) {
      const sVec2i newSize = mptrWindow->GetClientSize();
      if ((newSize.x != (tI32)this->GetWidth()) || (newSize.y != (tI32)this->GetHeight())) {
        _DoResizeContext();
      }
    }

    // mE
    mEncoder->_EndRendering();
    mEncoder->_EndCmdBufferAndSubmit(0, 0);

    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    if (!mpDrv)
      return NULL;

    const tU32 w = this->GetWidth();
    const tU32 h = this->GetHeight();

    Ptr<iBitmap2D> ptrBmp = mpDrv->GetGraphics()->CreateBitmap2D(w,h,"R8G8B8A8");

    astl::vector<tU8> data;
    data.resize(w*h*4);
    _glReadBuffer(GL_BACK);
    _glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,data.data());

    // flip the captured image
    {
      const tPtr ptrData = ptrBmp->GetData();
      const tU32 pitch = ptrBmp->GetPitch();
      niLoop(i, h) {
        tPtr sptr = data.data() + (pitch*(h-i-1));
        tPtr dptr = ptrData + (pitch * i);
        memcpy(dptr,sptr,pitch);
      }
    }

    // mpDrv->GetGraphics()->SaveBitmap("d:/_screencap_gl2.png", ptrBmp, 0);
    return ptrBmp.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  // virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    // GLES3_ClearBuffers(mpDrv,this,clearBuffer,anColor,afDepth,anStencil);
  // }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    return GLES3_DrawOperation(mpDrv,this,apDrawOp,this->GetNumAASamples());
  }

  niEndClass(cGL3ContextWindow);
};

class cGL3ContextRT : public sGLContext
{
  niBeginClass(cGL3ContextRT);

 public:
  iGraphicsDriver* mpDrv;

  cGL3ContextRT(
      iGraphicsDriver* apParent,
      iTexture* apRT,
      iTexture* apDS,
      sGLContext* apToCopy)
      : sGLContext(apParent->GetGraphics(), apParent)
  {
    mpDrv = apParent;
    niAssert(mpDrv != NULL);
    mptrRT[0] = apRT;
    mptrDS = apDS;
    mrectViewport = Recti(0,0,apRT->GetWidth(),apRT->GetHeight());
    mrectScissor = mrectViewport;

    niDebugFmt(("Create Context RT %s", mrectScissor));

    mEncoder->_renderingInfo._rtSize = mrectScissor.GetSize();
    mEncoder->_renderingInfo._rtFlipped = niFlagIs(mptrRT[0]->GetFlags(),eTextureFlags_RTFlipped);
    mEncoder->_renderingInfo._viewport = mrectScissor;

  }
  ~cGL3ContextRT() {
    Invalidate();
  }

  virtual void __stdcall _CheckResizeContext() {
  }
#ifdef TSGL_CONTEXT
  tsglContext* __stdcall GetTSGLContext() const {
    return NULL;
  }
#endif

  const tU32 GetNumAASamples() const {
    return (mptrRT[0]->GetFlags()&eTextureFlags_RTAA_All)?4:0;
  }

  virtual tBool __stdcall IsOK() const {
    return mptrRT[0].IsOK();
  }

  virtual void __stdcall Invalidate() {
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return mpDrv?mpDrv->GetGraphics():NULL;
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mpDrv;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect);

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return NULL;
  }

  /////////////////////////////////////////////
  // virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    // GLES3_ClearBuffers(mpDrv,this,clearBuffer,anColor,afDepth,anStencil);
  // }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    return GLES3_DrawOperation(mpDrv,this,apDrawOp,this->GetNumAASamples());
  }

  niEndClass(cGL3ContextRT);
};

///////////////////////////////////////////////
static tBool GL3_ApplyContext(sGLCache& aCache, sGLContext* apCtx, tBool& isFlippedRT)
{
  GL_DEBUG_MARKER_GROUP(ApplyContext);

  const tTextureFlags rtTexFlags = apCtx->mptrRT[0]->GetFlags();
  isFlippedRT = niFlagIs(rtTexFlags,eTextureFlags_RTFlipped);

  tBool bShouldUpdate = eFalse;
  aCache.ShouldUpdate(&bShouldUpdate,eGLCache_Context,(tIntPtr)apCtx);
  aCache.ShouldUpdate(&bShouldUpdate,eGLCache_ContextSyncCounter,apCtx->mnSyncCounter);
  if (bShouldUpdate) {
#ifdef USE_FBO
    // Apply render target. Right after DS states because we need to know
    // whether depth test is enabled to validate the DS usage.
    if (hasFBO) {
      if (!apCtx->mptrRT[0].IsOK()) {
        niError("No render target set.");
        return eFalse;
      }

      Ptr<sOpenglTexture> pRT = (sOpenglTexture*)apCtx->mptrRT[0]->Bind((iUnknown*)eInvalidHandle);
      if (!pRT->_BindAsRenderTarget()) {
        niError("Can't bind render target.");
        return eFalse;
      }

      if (apCtx->mptrDS.IsOK()) {
        Ptr<sOpenglTexture> pDS = (sOpenglTexture*)apCtx->mptrDS->Bind((iUnknown*)eInvalidHandle);
        if (!pDS->_BindAsDepthStencil()) {
          niError("Can't bind depth stencil.");
          return eFalse;
        }
      }

      if (!niFlagIs(pRT->mFormat.flags,eTextureFlags_MainRT)) {
        GLFBO_RET(eFalse);
      }
    }
#endif

    const tI32 rtWidth = apCtx->mptrRT[0]->GetWidth();
    const tI32 rtHeight = apCtx->mptrRT[0]->GetHeight();

    sRecti vp = apCtx->GetViewport();
    if (vp.GetWidth() == 0)
      vp.SetWidth(rtWidth);
    if (vp.GetHeight() == 0)
      vp.SetHeight(rtHeight);
    if ((vp.x+vp.GetWidth()) > rtWidth)
      vp.SetWidth(vp.GetWidth() - ((vp.x+vp.GetWidth())-rtWidth));
    if ((vp.y+vp.GetHeight()) > rtHeight)
      vp.SetHeight(vp.GetHeight() - ((vp.y+vp.GetHeight())-rtHeight));

    const tU32 nVPY = isFlippedRT ? vp.y : (rtHeight-vp.y)-vp.GetHeight();
    GLCALL_WARN(_glViewport(vp.x,nVPY,vp.GetWidth(),vp.GetHeight()));

    const sRecti sc = vp.ClipRect(apCtx->GetScissorRect());

    // since we clip the vp with the scissor rect we only need to compare the size
    aCache._scissorTest = (rtWidth != sc.GetWidth() || rtHeight != sc.GetHeight());
    if (aCache._scissorTest) {
      _glEnable(GL_SCISSOR_TEST);
      const tU32 nSCY = isFlippedRT ? sc.y : (rtHeight-sc.y)-sc.GetHeight();
      GLCALL_WARN(_glScissor(sc.x,nSCY,sc.GetWidth(),sc.GetHeight()));
    }
    else {
      _glDisable(GL_SCISSOR_TEST);
    }
    GLERR_RET(eFalse);
  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  Fixed Pipeline Shader Library
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
static tBool GL3_ApplyMaterialChannel(
    sGLContext* apContext,
    iGraphics* apGraphics,
    sGLCache& aCache,
    tU32 anTSS, eMaterialChannel aChannel,
    const sMaterialDesc* apDOMat)
{
  _glActiveTexture(GL_TEXTURE0+anTSS);

  const sMaterialChannel& ch = apContext->_GetChannel(apDOMat, aChannel);

  sOpenglTextureBase* tex = (sOpenglTextureBase*)ch.mTexture.ptr();
  tex = tex ? (sOpenglTextureBase*)tex->Bind(NULL) : NULL;
  if (tex) {
    const sSamplerStatesDesc* pSS = NULL;
    tIntPtr hSS = ch.mhSS;
    if (!hSS) {
      if (tex->mhLastSamplerStateSet) {
        hSS = tex->mhLastSamplerStateSet;
      }
      else {
        hSS = eCompiledStates_SS_SmoothClamp;
      }
    }
    tBool bShouldUpdate = tex->mhLastSamplerStateSet != hSS;
    if (bShouldUpdate) {
      tex->mhLastSamplerStateSet = hSS;
      // niDebugFmt(("... GL3 Texture SetSamplerStates '%s': %p", tex->mhspName, hSS));

      {
        iSamplerStates* ss = apGraphics->GetCompiledSamplerStates(hSS);
        if (ss) {
          pSS = (const sSamplerStatesDesc*)ss->GetDescStructPtr();
        }
      }
      if (pSS) {
        const GLenum texKind = GL_Texture(tex->GetType());
        const tBool isOverlay = !!(tex->mFormat.flags & eTextureFlags_Overlay);
        GL_TexSamplerWrap(texKind, GL_TEXTURE_WRAP_S, pSS->mWrapS, isOverlay);
        GLERR_RET(eFalse);
        GL_TexSamplerWrap(texKind, GL_TEXTURE_WRAP_T, pSS->mWrapT, isOverlay);
        GLERR_RET(eFalse);
#ifdef GL_TEXTURE_WRAP_R
        if (hasWrapR) {
          GL_TexSamplerWrap(texKind, GL_TEXTURE_WRAP_R, pSS->mWrapT, isOverlay);
          GLERR_RET(eFalse);
        }
#endif
        const tU32 numMips = tex->GetNumMipMaps();
        GL_TexSamplerFilter(texKind, pSS->mFilter, numMips);
        GLERR_RET(eFalse);
      }
    }
  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
// GLES3GraphicsDriver
//
//--------------------------------------------------------------------------------------------
const achar* GL3Drv_GetName() { return _A("GL3"); }
const achar* GL3Drv_GetDesc() { return _A("GL3 Graphics Driver"); }

struct sOpenGLDriver : public ImplRC<iGraphicsDriver,eImplFlags_Default,iGraphicsDriverGpu>
{
  iGraphics* mpGraphics;
  sGLCache   mCache;
  tBool      mbUseColorA;
  Ptr<iGraphicsDrawOpCapture> mptrDOCapture;
  astl::vector<tHStringPtr> mvProfiles[eShaderUnit_Last];
  sFixedShaders mFixedShaders;
  Ptr<iFixedGpuPipelines> _fixedPipelines;


  sOpenGLDriver(iGraphics* apGraphics) {
    mpGraphics = apGraphics;
    mbUseColorA = eFalse;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return mpGraphics != NULL;
  }
  virtual void __stdcall Invalidate() {
    niLoop(i,eShaderUnit_Last) {
      mvProfiles[i].clear();
    }
    mpGraphics = NULL;
  }

  /////////////////////////////////////////////
  virtual iGraphics* __stdcall GetGraphics() const {
    return mpGraphics;
  }

  /////////////////////////////////////////////
  virtual const achar* __stdcall GetName() const { return GL3Drv_GetName(); }
  virtual const achar* __stdcall GetDesc() const { return GL3Drv_GetDesc(); }
  virtual const achar* __stdcall GetDeviceName() const {
    return _A("Default");
  }

  /////////////////////////////////////////////
  virtual iGraphicsContext* __stdcall CreateContextForWindow(
      iOSWindow* apWindow,
      const achar* aaszBBFormat, const achar* aaszDSFormat,
      tU32 anSwapInterval,
      tTextureFlags aBackBufferFlags)
  {
    niCheckIsOK(apWindow,NULL);

    Ptr<iGraphicsContext> ctx = niNew cGL3ContextWindow(
        this,
        apWindow,
        aaszBBFormat,aaszDSFormat,
        anSwapInterval,
        aBackBufferFlags);
    niCheckIsOK(ctx, NULL);

    if (!_fixedPipelines.IsOK()) {
      _fixedPipelines = niCheckNN(_fixedPipelines, CreateFixedGpuPipelines(this), nullptr);
    }

    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS)
  {
    niCheckIsOK(apRT0,NULL);

    Ptr<iGraphicsContextRT> ctx = niNew cGL3ContextRT(
      this,
      apRT0, apDS,
      NULL);
    niCheckIsOK(ctx, NULL);

    if (!_fixedPipelines.IsOK()) {
      _fixedPipelines = niCheckNN(_fixedPipelines, CreateFixedGpuPipelines(this), nullptr);
    }

    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall ResetAllCaches() {
    GLES3_ResetCache(this);
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tInt __stdcall GetCaps(eGraphicsCaps aCaps) const {
    switch (aCaps) {
      case eGraphicsCaps_Resize:
      case eGraphicsCaps_MultiContext:
      case eGraphicsCaps_ScissorTest:
      case eGraphicsCaps_OverlayTexture:
        return 1;
      case eGraphicsCaps_NumRenderTargetTextures:
#ifdef USE_FBO
        return hasFBO ? 1 : 0;
#else
        return 0;
#endif
      case eGraphicsCaps_DepthStencilTexture:
        return hasTexFmtDepth;
      case eGraphicsCaps_NumTextureUnits:
        return kGL3_MaxTU;
      case eGraphicsCaps_Texture2DMaxSize:
        return kGL3_MaxRegularTexSize;
      case eGraphicsCaps_TextureCubeMaxSize:
        return kGL3_MaxCubeTexSize;
      case eGraphicsCaps_Texture3DMaxSize:
        return 0;
      case eGraphicsCaps_MaxVertexIndex:
        return hasElementUInt ? 0xFFFFFFFF : 0xFFFF;
      case eGraphicsCaps_OrthoProjectionOffset:
        return 0;
#ifdef USE_OQ
      case eGraphicsCaps_OcclusionQueries:
        return !!hasOQ;
#endif
      case eGraphicsCaps_BlitBackBuffer:
        return eFalse; // This works, but glCopyTexSubImage2D is just too slow, especially on Mobile
      case eGraphicsCaps_Wireframe: {
#ifdef _glPolygonMode
        return eTrue;
#else
        return eFalse;
#endif
      }
      case eGraphicsCaps_IGpu:
        return eFalse;
      default:
        return 0;
    }
  }

  /////////////////////////////////////////////
  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const {
    return eGraphicsDriverImplFlags_IndexArrayObject|
        eGraphicsDriverImplFlags_VertexArrayObject;
  }

  ///////////////////////////////////////////////
  tBool _CheckRenderTargetSupport(const eBitmapType aType, const tTextureFlags aFlags) {
    if (niFlagIs(aFlags,eTextureFlags_RenderTarget) || niFlagIs(aFlags,eTextureFlags_DepthStencil))
    {
#ifdef USE_FBO
      if (niFlagIs(aFlags,eTextureFlags_DepthStencil) && !hasTexFmtDepth) {
        return eFalse;
      }
      return (aType == eBitmapType_2D) && hasFBO;
#else
      return eFalse;
#endif
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) {
    niCheckSilent(niIsOK(apFormat),eFalse);
    if (!_CheckRenderTargetSupport(apFormat->GetType(), aFlags))
      return eFalse;

    sOpenglTextureFormat texFormat;

    tU32 nWidth = apFormat->GetWidth();
    tU32 nHeight = apFormat->GetHeight();
    if (!hasPartialNP2 || !niFlagIs(aFlags,eTextureFlags_Overlay)) {
      nWidth = GetNearestPow2(nWidth);
      nHeight = GetNearestPow2(nHeight);
    }
    tU32 nMaxSize = niFlagIs(aFlags,eTextureFlags_Overlay) ?
        kGL3_MaxOverlayTexSize : kGL3_MaxRegularTexSize;

    if (nWidth > nMaxSize) {
      nWidth = nMaxSize;
    }
    if (nHeight > nMaxSize) {
      nHeight = nMaxSize;
    }
    apFormat->SetWidth(nWidth);
    apFormat->SetHeight(nHeight);

    if (!texFormat._Initialize(
            mpGraphics,
            apFormat->GetType(),
            apFormat->GetPixelFormat()->GetFormat(),
            apFormat->GetNumMipMaps(),
            apFormat->GetWidth(),
            apFormat->GetHeight(),
            apFormat->GetDepth(),
            aFlags))
      return eFalse;

    apFormat->SetPixelFormat(texFormat.pxf);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) {
    if (!_CheckRenderTargetSupport(aType, aFlags)) {
      niError("RenderTarget/DepthStencil textures not supported.");
      return NULL;
    }

    if (aType == eBitmapType_2D) {
      GL_DEBUG_LOG((_A("- REQUESTED CREATE 2D TEXTURE '%s' %dx%d (mips:%d) %s (%d)"),
                      niHStr(ahspName),
                      anWidth,anHeight,anNumMipMaps,aaszFormat,aFlags));

      // DepthStencil is always an "Overlay"
      if (niFlagIs(aFlags,eTextureFlags_DepthStencil)) {
        aFlags |= eTextureFlags_Overlay;
        // RT in GL are always flipped
        aFlags |= eTextureFlags_RTFlipped;
      }
      else if (niFlagIs(aFlags,eTextureFlags_RenderTarget)) {
        // RT in GL are always flipped
        aFlags |= eTextureFlags_RTFlipped;
      }

      {
        if (!hasPartialNP2 || !niFlagIs(aFlags,eTextureFlags_Overlay)) {
          anWidth = GetNearestPow2(anWidth);
          anHeight = GetNearestPow2(anHeight);
        }

        tU32 nMaxSize = niFlagIs(aFlags,eTextureFlags_Overlay) ?
            kGL3_MaxOverlayTexSize : kGL3_MaxRegularTexSize;

        // Correct the texture size
        if (anWidth > nMaxSize) {
          anWidth = nMaxSize;
        }
        if (anHeight > nMaxSize) {
          anHeight = nMaxSize;
        }

        const tBool isPow2 = IsPow2(anWidth) && IsPow2(anHeight);
        if (isPow2) {
          anNumMipMaps = ((anNumMipMaps == 0) && niFlagIs(aFlags,eTextureFlags_MipMaps)) ?
              ComputeNumPow2Levels(anWidth>>1,anHeight>>1) : anNumMipMaps;
        }
        else {
          anNumMipMaps = 0;
        }
      }

      GL_DEBUG_LOG((_A("- CREATE 2D TEXTURE '%s' %dx%d (mips:%d) %s (%d)"),
                      niHStr(ahspName),
                      anWidth,anHeight,anNumMipMaps,aaszFormat,aFlags));

      Ptr<sOpenglTexture> newTex = niNew sOpenglTexture(this,ahspName);
      niCheck(newTex.IsOK(),NULL);

      niCheck(
          newTex->_NewTexture(mpGraphics,
                              aaszFormat,
                              anNumMipMaps,
                              anWidth,
                              anHeight,
                              aFlags),
          NULL);

      GL_DEBUG_LOG(("- DONE"));
      niCheck(newTex.IsOK(),NULL);
      return newTex.GetRawAndSetNull();
    }
    else if (aType == eBitmapType_Cube) {
      GL_DEBUG_LOG((_A("- REQUESTED CREATE CUBE TEXTURE '%s' width:%d (mips:%d) %s (%d)"),
                      niHStr(ahspName),
                      anWidth,anNumMipMaps,aaszFormat,aFlags));

      if (!hasCubeMap) {
        niError("CubeMap not supported on this device.");
        return NULL;
      }

      {
        // Correct the texture size
        anWidth = GetNearestPow2(anWidth);
        if (anWidth > (tU32)kGL3_MaxCubeTexSize) {
          anWidth = kGL3_MaxCubeTexSize;
        }
        anNumMipMaps = ((anNumMipMaps == 0) && niFlagIs(aFlags,eTextureFlags_MipMaps)) ?
            ComputeNumPow2Levels(anWidth>>1,anHeight>>1) : anNumMipMaps;
      }

      GL_DEBUG_LOG((_A("- CREATE CUBE TEXTURE '%s' %d (mips:%d) %s (%d)"),
                      niHStr(ahspName),
                      anWidth,anNumMipMaps,aaszFormat,aFlags));

      Ptr<sOpenglTextureCube> newTex = niNew sOpenglTextureCube(this,ahspName);
      niCheck(newTex.IsOK(),NULL);

      niCheck(
          newTex->_NewTexture(mpGraphics,
                              aaszFormat,
                              anNumMipMaps,
                              anWidth,
                              aFlags),
          NULL);

      GL_DEBUG_LOG(("- DONE"));
      niCheck(newTex.IsOK(),NULL);
      return newTex.GetRawAndSetNull();
    }

    niError("Invalid texture type.");
    return NULL;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None) {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apDest->GetType() == eBitmapType_2D,eFalse);

    sOpenglTexture* tex = niStaticCast(sOpenglTexture*,apDest);
    niCheck(tex->_BlitFromBitmap2D(apSrc,
                                   anDestLevel,
                                   aSrcRect,
                                   aDestRect,
                                   aFlags),
            eFalse);

    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None) {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apSrc->GetType() == eBitmapType_2D,eFalse);

    sOpenglTexture* src = niStaticCast(sOpenglTexture*,apSrc);
    if (!src)
      return eFalse;

    if (src->mptrBmpRestore.IsOK()) {
      apDest->BlitStretch(
          src->mptrBmpRestore,
          aSrcRect.Left(),aSrcRect.Top(),
          aDestRect.Left(),aDestRect.Top(),
          aSrcRect.GetWidth(),aSrcRect.GetHeight(),
          aDestRect.GetWidth(),aDestRect.GetHeight());
    }
    else {
#if defined USE_FBO
      // TODO: Handle the main render target. Atm it should work, but on
      // desktop - and probably mobile the result will be flipped. This need
      // to be tested, or just marked as unsupported (and return eFalse).
      if (src->mGLFBOHandle != GLDRV_INVALID_HANDLE) {
        GL_DEBUG_LOG(("- BlitTextureToBitmap: USING glReadPixels"));
        if (aSrcRect.GetSize() != aDestRect.GetSize()) {
          GL_DEBUG_LOG(("- BlitTextureToBitmap: USING glReadPixels can't blit between different sizes"));
          return eFalse;
        }

        GLuint wasFBO;
        _glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&wasFBO));
        _glBindFramebuffer(GL_FRAMEBUFFER, src->mGLFBOHandle);

        const Ptr<iPixelFormat> pxfReadPixels = mpGraphics->CreatePixelFormat("R8G8B8A8");
        const tBool blitFullContent =
            (aDestRect.GetTopLeft() == sVec2i::Zero() &&
             aDestRect.GetSize() == Vec2i(apSrc->GetWidth(),apSrc->GetHeight())) &&
            apDest->GetPixelFormat()->IsSamePixelFormat(pxfReadPixels);

        _glReadBuffer(GL_BACK);
        if (blitFullContent)
        {
          GL_DEBUG_LOG(("- BlitTextureToBitmap: COPY FULL TEXTURE CONTENT"));
          _glReadPixels(aSrcRect.x,aSrcRect.y,aSrcRect.GetWidth(),aSrcRect.GetHeight(),GL_RGBA,GL_UNSIGNED_BYTE,apDest->GetData());
        }
        else {
          astl::vector<tU8> data;
          data.resize(aSrcRect.GetWidth()*aSrcRect.GetHeight()*4);
          Ptr<iBitmap2D> bmpTmp = mpGraphics->CreateBitmap2DMemoryEx(
              aSrcRect.GetWidth(),aSrcRect.GetHeight(),pxfReadPixels,
              aSrcRect.GetWidth()*4, data.data(), eFalse);

          _glReadPixels(aSrcRect.x,aSrcRect.y,aSrcRect.GetWidth(),aSrcRect.GetHeight(),GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid*)data.data());

          apDest->BlitStretch(
              bmpTmp,
              0,0,
              aDestRect.Left(),aDestRect.Top(),
              aSrcRect.GetWidth(),aSrcRect.GetHeight(),
              aDestRect.GetWidth(),aDestRect.GetHeight());
        }

        _glBindFramebuffer(GL_FRAMEBUFFER, wasFBO);
        return eTrue;
#endif
      }

#ifdef USE_GL_GET_TEX_IMAGE
      GL_DEBUG_LOG(("- BlitTextureToBitmap: Use glGetTexImage."));

      if (aDestRect.GetTopLeft() == sVec2i::Zero() &&
          aDestRect.GetSize() == Vec2i(apSrc->GetWidth(),apSrc->GetHeight()))
      {
        GL_DEBUG_LOG(("- BlitTextureToBitmap: COPY FULL TEXTURE CONTENT"));
        if (apSrc->GetPixelFormat()->IsSamePixelFormat(apDest->GetPixelFormat())) {
          GL_DEBUG_LOG(("- BlitTextureToBitmap: SAME PIXEL FORMAT, READ DIRECTLY IN DEST"));
          _glGetTexImage(GL_TEXTURE_2D,anSrcLevel,src->mFormat.format,src->mFormat.type,apDest->GetData());
          return eTrue;
        }
      }

      GL_DEBUG_LOG(("- BlitTextureToBitmap: USE TEMP BITMAP"));
      niCheck(src->_BindAsTexture(),eFalse);
      Ptr<iBitmap2D> bmpTmp = src->_CreateMatchingBitmap(mpGraphics);
      _glGetTexImage(GL_TEXTURE_2D,anSrcLevel,src->mFormat.format,src->mFormat.type,bmpTmp->GetData());
      apDest->BlitStretch(
          bmpTmp,
          aSrcRect.Left(),aSrcRect.Top(),
          aDestRect.Left(),aDestRect.Top(),
          aSrcRect.GetWidth(),aSrcRect.GetHeight(),
          aDestRect.GetWidth(),aDestRect.GetHeight());
      return eTrue;
#else
      return eFalse;
#endif
    }

    return eTrue;
  }

  static inline void _CopyToBoundTexture(
      const GLenum target, const tU32 level,
      const sRecti& srect, const tU32 aSrcHeight,
      const sRecti& drect, const tU32 aDstHeight,
      tBool upsidedown)
  {
    static const float eps = 1e-5f;

    const float xrel = (float)(srect.GetWidth()) / (float)(drect.GetWidth());
    const float yrel = (float)(srect.GetHeight()) / (float)(drect.GetHeight());
    if ((xrel - 1.0 < -eps) || (xrel - 1.0 > eps)) {
      niWarning("Doing a pixel by pixel copy from the framebuffer to a texture, expect major performance issues\n");
    }

    if (upsidedown &&
        !((xrel - 1.0 < -eps) || (xrel - 1.0 > eps)) &&
        !((yrel - 1.0 < -eps) || (yrel - 1.0 > eps)))
    {
      // Upside down copy without stretching is nice, one glCopyTexSubImage call will do
      _glCopyTexSubImage2D(target,
                           level,
                           srect.x1(), srect.y1(), /* xoffset, yoffset */
                           drect.x1(), drect.y1(),
                           drect.x2() - drect.x1(), drect.y2() - drect.y1());
    } else {
      //
      // Process this row by row to swap the image, otherwise it would be
      // upside down, so streching in y direction doesn't cost extra time.
      //
      // However, streching in x direction can be avoided if not necessary.
      //
      if ((xrel - 1.0 < -eps) || (xrel - 1.0 > eps)) {
        niWarning("Doing a pixel by pixel render target -> texture copy, expect performance issues\n");
        for (tI32 row = drect.y1(); row < drect.y2(); row++) {
          for (tI32 col = drect.x1(); col < drect.x2(); col++) {
            _glCopyTexSubImage2D(target,
                                 level,
                                 drect.x1() + col, aDstHeight - row - 1,
                                 srect.x1() + col * xrel, aSrcHeight - srect.y2() + row * yrel,
                                 1, 1);
          }
        }
      }
      else {
        for (tI32 row = drect.y1(), i = 0; row < drect.y2(); ++row, ++i) {
          _glCopyTexSubImage2D(target,
                               level,
                               drect.x1(), aDstHeight - i - (aDstHeight-drect.y2()),
                               srect.x1(), aSrcHeight - srect.y2() + i,
                               drect.x2()-drect.x1(), 1);
        }
      }
    }
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None) {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apSrc->GetType() == eBitmapType_2D,eFalse);
    niCheckSilent(apDest->GetType() == eBitmapType_2D,eFalse);

    if (apSrc->GetFlags() & eTextureFlags_MainRT) {
      _glFlush();
      GLES3_ResetCache(this);
      sOpenglTexture* src = niStaticCast(sOpenglTexture*,apSrc);
      sOpenglTexture* dst = niStaticCast(sOpenglTexture*,apDest);
      if (!src->_BindAsRenderTarget()) {
        niError("Can't bind the source texture as current render target.");
        return eFalse;
      }
      dst->Bind(NULL);

      // glCopyTexSubImage2D(GL_TEXTURE_2D,anDestLevel,
                          // aDestRect.x, aDestRect.y1(),
                          // aSrcRect.x, src->GetHeight() - aSrcRect.y2(), // aSrcRect.y,
                          // aDestRect.GetWidth(),aDestRect.GetHeight());
      // glCopyTexSubImage2D(GL_TEXTURE_2D,anDestLevel,0,0,0,0,src->GetWidth(),src->GetHeight());
      // glCopyTexImage2D(GL_TEXTURE_2D,anDestLevel,GL_RGB,0,0,src->GetWidth(),src->GetHeight(),0);

      _CopyToBoundTexture(
          GL_TEXTURE_2D, anDestLevel,
          aSrcRect, apSrc->GetHeight(),
          aDestRect, apDest->GetHeight()>>anDestLevel,
          eFalse);
      GLERR_RET(eFalse);
    }
    else {
      sOpenglTexture* src = niStaticCast(sOpenglTexture*,apSrc);
      if (!src || !src->mptrBmpRestore.IsOK())
        return eFalse;

      sOpenglTexture* tex = niStaticCast(sOpenglTexture*,apDest);
      niCheck(
          tex->_BlitFromBitmap2D(src->mptrBmpRestore, anDestLevel, aSrcRect, aDestRect, aFlags),
          eFalse);
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)
  {
    niError(_A("NOT IMPLEMENTED."));
    return eFalse;
  }
  virtual tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)
  {
    niError(_A("NOT IMPLEMENTED."));
    return eFalse;
  }

  /////////////////////////////////////////////
  tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const {
    if (aUnit >= eShaderUnit_Last) return 0;
    return (tU32)mvProfiles[aUnit].size();
  }
  iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const {
    if (anIndex >= GetNumShaderProfile(aUnit)) return NULL;
    return mvProfiles[aUnit][anIndex];
  }
  virtual iShader* __stdcall CreateShader(iHString* ahspName, iFile* apFile) {

    if (!niIsOK(apFile)) {
      niError(_A("Invalid file."));
      return NULL;
    }

    const tU32 fcc = apFile->ReadLE32();
    if (fcc != kfccD3DShader) {
      niError(niFmt("The file '%s' is not a D3D shader (fcc:(%c,%c,%c,%c)).",
                    ahspName, niFourCCA(fcc), niFourCCB(fcc), niFourCCC(fcc), niFourCCD(fcc)));
      return NULL;
    }

    const tU32 version = apFile->ReadLE32();
    if (version != niMakeVersion(1,0,0)) {
      niError(niFmt("Incompatible version '%d' for shader '%s'.", version, ahspName));
      return NULL;
    }

    apFile->BeginReadBits();
    tU8 nUnit = apFile->ReadBitsPackedU8();
    /*cString strName =*/ apFile->ReadBitsString();
    tHStringPtr hspProfileName = _H(apFile->ReadBitsString());
    tU32 nBufferSize = apFile->ReadBitsPackedU32();
    tBool bHasConstants = apFile->ReadBit();
    apFile->EndReadBits();

    astl::vector<tU8> vData;
    vData.resize(nBufferSize);
    apFile->ReadRaw(&vData[0],nBufferSize);

    if (StrStr(niHStr(hspProfileName),"vs_")) {
      hspProfileName = _H("glslv");
    }
    else if (StrStr(niHStr(hspProfileName),"ps_")) {
      hspProfileName = _H("glslf");
    }

    if (!HasShaderProfile(this,nUnit,hspProfileName)) {
      niError(niFmt(_A("Profile '%s' is not available, can't load the program."),hspProfileName));
      return NULL;
    }

    iShader* pProg = NULL;
    if (nUnit == eShaderUnit_Vertex)
    {

    }
    else if (nUnit == eShaderUnit_Pixel)
    {

    }
    if (!niIsOK(pProg)) {
      niSafeRelease(pProg);
      niError(_A("Can't create the shader instance."));
      return NULL;
    }

    if (bHasConstants) {
      if (!ni::GetLang()->SerializeObject(apFile,(iShaderConstants*)pProg->GetConstants(),eSerializeMode_ReadRaw,NULL)) {
        niSafeRelease(pProg);
        niError(_A("Can't load the shader constants."));
        return NULL;
      }
    }

    return pProg;
  }

  /////////////////////////////////////////////
  virtual iOcclusionQuery* __stdcall CreateOcclusionQuery() {
#ifdef USE_OQ
    if (hasOQ)
      return niNew cGL3OcclusionQuery();
#endif
    return NULL;
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture) {
    mptrDOCapture = niGetIfOK(apCapture);
  }
  virtual iGraphicsDrawOpCapture* __stdcall GetDrawOpCapture() const {
    return mptrDOCapture;
  }

  /////////////////////////////////////////////
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) {
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_SYSTEM_MEMORY
    if (aUsage == eArrayUsage_Static)
#endif
    {
      return niNew sGLVertexArray(
        mpGraphics->GetGenericDeviceResourceManager(),
        anNumVertices,anFVF,aUsage);
    }
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_SYSTEM_MEMORY
    else {
      return _CreateGenericVertexArray(anNumVertices, anFVF);
    }
#endif
  }
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_SYSTEM_MEMORY
    if (aUsage == eArrayUsage_Static)
#endif
    {
      return niNew sGLIndexArray32(
        mpGraphics->GetGenericDeviceResourceManager(),
        aPrimitiveType,anNumIndex,anMaxVertexIndex,aUsage);
    }
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_SYSTEM_MEMORY
    else {
      return _CreateGenericIndexArray(aPrimitiveType,anNumIndex,anMaxVertexIndex);
    }
#endif
  }

  ///////////////////////////////////////////////
  inline tBool DoDrawOp(
      sGLCache& aCache,
      iDrawOperation* apDrawOp,
      const sMatrixf& aViewMatrix,
      const sMatrixf& aProjMatrix,
      const sVec4f& aBaseColor,
      const sMaterialChannel* apChannels,
      iGLShader* apNativeShader)
  {
    // Geometry description buffers
    iVertexArray* pVA = apDrawOp->GetVertexArray();
    iIndexArray* pIA = apDrawOp->GetIndexArray();
    const tBool bUseColorA = mbUseColorA;

#define VADISABLE(ATTR)

    tU32 enabledVertexAttributes = 0;
#if !defined DISABLE_ALL_VAA_AFTER_DRAWOP
    // this is static and "ok" because _DrawOperation can't be multithreaded...
    static tU32 _prevEnabledVertexAttributes = 0;
#endif

    const tI32* vertexAttribLoc = apNativeShader->GetVertexAttributeLocationArray();
#define VAENABLE_(USAGE,SIZE,TYPE,BASE,NORMALIZED)                      \
    const tI32 _attr_##USAGE = vertexAttribLoc[eVertexStreamIndex_##USAGE]; \
    if (_attr_##USAGE >= 0) {                                           \
      GLCALL_WARN(_glEnableVertexAttribArray(_attr_##USAGE));           \
      GLCALL_WARN(_glVertexAttribPointer(_attr_##USAGE, SIZE, GL_##TYPE, NORMALIZED, nStride, reinterpret_cast<void*>(BASE))); \
      enabledVertexAttributes |= niBit(_attr_##USAGE);                  \
    }

#define VAENABLE(USAGE,SIZE,TYPE,BASE) VAENABLE_(USAGE,SIZE,TYPE,BASE,GL_FALSE)
#define VAENABLEN(USAGE,SIZE,TYPE,BASE) VAENABLE_(USAGE,SIZE,TYPE,BASE,GL_TRUE)

    {
      tPtr pVABase = (tPtr)0;
      const cFVFDescription* fvfDesc;

      if (pVA->GetUsage() == eArrayUsage_SystemMemory) {
        GLCALL_WARN(_glBindBuffer(GL_ARRAY_BUFFER,0));
        fvfDesc = &_GetGenericVertexArrayFVFDesc(pVA);
        pVABase = _GetGenericVertexArrayMemPtr(pVA);
      }
      else {
        const sGLVertexArray* pGLVA = (sGLVertexArray*)pVA;
        fvfDesc = &pGLVA->mFVF;
      }

      const GLenum pt = GL_Primitive(apDrawOp->GetPrimitiveType());
      const tU32 nStride = fvfDesc->GetStride();
      pVABase += apDrawOp->GetBaseVertexIndex()*nStride;

      {
        pVA->Bind(NULL); // Bind the VertexArray
        {
          VAENABLE(Position, 3, FLOAT, (void*)(pVABase));
          if (bUseColorA && fvfDesc->HasColorA()) {
            VAENABLEN(ColorA, 4, UNSIGNED_BYTE, pVABase+fvfDesc->GetColorAOffset());
          }
          else {
            VADISABLE(ColorA);
          }
          if (fvfDesc->HasNormal()) {
            VAENABLE(Normal, 3, FLOAT, pVABase+fvfDesc->GetNormalOffset());
          }
          else {
            VADISABLE(Normal);
          }
          if (fvfDesc->HasTexCoo(0)) {
            VAENABLE(Tex1, 2, FLOAT, pVABase+fvfDesc->GetTexCooOffset(0));
          }
          else {
            VADISABLE(Tex1);
          }
          if (fvfDesc->HasTexCoo(1)) {
            VAENABLE(Tex2, 2, FLOAT, pVABase+fvfDesc->GetTexCooOffset(1));
          }
          else {
            VADISABLE(Tex2);
          }
        }

#if !defined DISABLE_ALL_VAA_AFTER_DRAWOP
        // Disable the unused attributes, otherwise some drivers will crash nicely...
        niLoop(i,kGL3_MaxVertexAttrs) {
          const tU32 bitValue = niBit(i);
          if (
                  (_prevEnabledVertexAttributes&bitValue) // if it was set
              && !(enabledVertexAttributes&bitValue)  // and now isnt set
              )
          {
            // disable the attribute...
            GLCALL_WARN(_glDisableVertexAttribArray(i));
          }
        }
        _prevEnabledVertexAttributes = enabledVertexAttributes;
#endif
      }

      if (!apNativeShader->BeforeDraw()) {
        niWarning("Shader's BeforeDraw failed.");
        return eFalse;
      }

      if (pIA) {
        // Index array rendering
        tPtr pIABase = (tPtr)0;
        if (pIA->GetUsage() == eArrayUsage_SystemMemory) {
          GLCALL_WARN(_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0));
          pIABase = _GetGenericIndexArrayMemPtr(pIA);
        }

        {
          pIA->Bind(NULL);

          tU32 nNumIndices = apDrawOp->GetNumIndices();
          if (!nNumIndices)
            nNumIndices = pIA->GetNumIndices()-apDrawOp->GetFirstIndex();

          if (hasElementUInt) {
            GLCALL_WARN(
                _glDrawElements(pt,nNumIndices,GL_UNSIGNED_INT,
                                (void*)(pIABase+(apDrawOp->GetFirstIndex()*sizeof(tU32)))));
          }
          else {
            GLCALL_WARN(
                _glDrawElements(pt,nNumIndices,GL_UNSIGNED_SHORT,
                                (void*)(pIABase+(apDrawOp->GetFirstIndex()*sizeof(tU16)))));
          }
        }

#ifdef USE_GL_UNBIND_BUFFERS
        GLCALL_WARN(_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0));
#endif
      }
      else {
        const tU32 nNumVerts = pVA->GetNumVertices()-apDrawOp->GetBaseVertexIndex();
        _glDrawArrays(pt,0,nNumVerts);
      }

#ifdef USE_GL_UNBIND_BUFFERS
      GLCALL_WARN(_glBindBuffer(GL_ARRAY_BUFFER,0));
#endif
      GLERR_RET(eFalse);
    }

#ifdef DISABLE_ALL_VAA_AFTER_DRAWOP
    // We *have* to make sure to disable all the VAO used... (otherwise crash...)
    // TODO: Use the MojoShader lib to manage this even for the fixed shaders
    niLoop(i,kGL3_MaxVertexAttrs) {
      GLCALL_WARN(_glDisableVertexAttribArray(i));
    }
#endif

    apNativeShader->AfterDraw();
    return eTrue;
  }

  tBool __stdcall _DrawOperation(sGLContext* apContext, iDrawOperation* apDrawOp, const tU32 anAA) {

    niCheckSilent(niIsOK(apDrawOp),eFalse);
    niCheckSilent(niIsOK(apDrawOp->GetVertexArray()),eFalse);

#ifdef LAZY_CLEAR_BUFFERS
    GL_CheckClearBuffers(mCache,apContext,mptrDOCapture);
#endif

    if (mptrDOCapture.IsOK()) {
      if (!mptrDOCapture->BeginCaptureDrawOp(apContext,apDrawOp,sVec4i::Zero()))
        return eTrue;
    }

    GL_DEBUG_MARKER_GROUP(DrawOp);

    tBool r = eFalse;
    const sMaterialDesc* pMatDesc = NULL;
    sMatrixf viewMatrix, projMatrix;
    sVec4f texFactor;
    const tFVF fvf = apDrawOp->GetVertexArray()->GetFVF();

    // apply states //
    {
      Ptr<iMaterial> mat = apDrawOp->GetMaterial();
      niAssertMsg(mat.IsOK(),"No valid material in the draw operation.");
      niCheck(mat.IsOK(),eFalse);

      pMatDesc = (const sMaterialDesc*)mat->GetDescStructPtr();
      const tU32 matFlags = pMatDesc->mFlags;

      mbUseColorA =
          niFlagIs(matFlags,eMaterialFlags_Vertex) &&
          niFlagIs(fvf,eFVF_ColorA);

      // Depth stencil states
      {
        tIntPtr hDS = apContext->_GetDS(pMatDesc);
        if (!hDS) {
          hDS = eCompiledStates_DS_NoDepthTest;
        }
        tBool bShouldUpdate = eFalse;
        mCache.ShouldUpdate(&bShouldUpdate,eGLCache_DepthStencil,hDS);
        if (bShouldUpdate)
        {
          iDepthStencilStates* pDSStates = mpGraphics->GetCompiledDepthStencilStates(hDS);
          niCheck(pDSStates,eFalse);
          GL_ApplyDepthStencilStates(
              this->mCache,
              *(const sDepthStencilStatesDesc*)pDSStates->GetDescStructPtr());
        }
      }

      tBool isFlippedRT;
      if (!GL3_ApplyContext(mCache,apContext,isFlippedRT)) {
        niError(_A("Can't apply context."));
        return eFalse;
      }

      // Rasterizer states
      {
        const tIntPtr hRS = apContext->_GetRS(pMatDesc);
        tBool bShouldUpdate = eFalse;
        mCache.ShouldUpdate(&bShouldUpdate,eGLCache_Rasterizer,hRS);
        mCache.ShouldUpdate(&bShouldUpdate,eGLCache_RasterizerDoubleSided,
                            niFlagIs(matFlags,eMaterialFlags_DoubleSided));
        mCache.ShouldUpdate(&bShouldUpdate,eGLCache_RasterizerDepthOnly,
                            niFlagIs(matFlags,eMaterialFlags_DepthOnly));
        mCache.ShouldUpdate(&bShouldUpdate,eGLCache_RasterizerFlippedRT,isFlippedRT);
        if (bShouldUpdate)
        {
          iRasterizerStates* pRSStates = mpGraphics->GetCompiledRasterizerStates(hRS);
          niCheck(pRSStates,eFalse);
          GL_ApplyRasterizerStates(
              this->mCache,
              *(const sRasterizerStatesDesc*)pRSStates->GetDescStructPtr(),
              niFlagIs(matFlags,eMaterialFlags_DoubleSided),
              niFlagIs(matFlags,eMaterialFlags_DepthOnly),
              isFlippedRT);
        }
      }

      // Alpha to coverage
      if (anAA) {
        if (niFlagIs(pMatDesc->mFlags,eMaterialFlags_TransparentAA)) {
          _glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
        else {
          _glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
      }

      if (niFlagIs(pMatDesc->mFlags,eMaterialFlags_PolygonOffset)) {
        _glEnable(GL_POLYGON_OFFSET_FILL);
        _glPolygonOffset(pMatDesc->mvPolygonOffset.x,pMatDesc->mvPolygonOffset.y);
      }
      else {
        _glDisable(GL_POLYGON_OFFSET_FILL);
        _glPolygonOffset(0,0);
      }

      // Alpha blending
      {
        const eBlendMode blendMode = pMatDesc->mBlendMode;
        if (blendMode == eBlendMode_NoBlending) {
          GL_ApplyBlendMode(
              (niFlagIs(matFlags,eMaterialFlags_Translucent)) ?
              eBlendMode_Translucent : blendMode);
        }
        else {
          GL_ApplyBlendMode(blendMode);
        }
      }
    }

    // Shaders
    {
      iShader* pNS = pMatDesc->mShaders[eShaderUnit_Native];
      if (pNS) {
        QPtr<iGLShader> nativeShader = pNS->Bind(apDrawOp);
        if (!nativeShader.IsOK()) {
          niError("Can't bind native shader.");
          return eFalse;
        }
        r = DoDrawOp(
          mCache,apDrawOp,
          viewMatrix,projMatrix,texFactor,
          pMatDesc ? pMatDesc->mChannels : NULL,
          nativeShader);
      }
      else  {
        iShader* pVS = pMatDesc->mShaders[eShaderUnit_Vertex];
        iShader* pPS = pMatDesc->mShaders[eShaderUnit_Pixel];
        if (!pVS) {
          pVS = mFixedShaders.GetVertexShader(fvf,*pMatDesc);
          if (!pVS) {
            niError("Can't get fixed vertex shader.");
            return eFalse;
          }
          FixedShaders_UpdateConstants(apContext, (iShaderConstants*)pVS->GetConstants(), apDrawOp);
        }
        if (!pPS) {
          pPS = mFixedShaders.GetPixelShader(*pMatDesc);
          if (!pPS) {
            niError("Can't get fixed pixel shader.");
            return eFalse;
          }
          FixedShaders_UpdateConstants(apContext, (iShaderConstants*)pPS->GetConstants(), apDrawOp);
        }
#if 0
        iMojoShader* pMojoVS = (iMojoShader*)pVS->Bind(NULL);
        if (!pMojoVS) {
          static bool _didLogError = false;
          if (!_didLogError) {
            niError("Can't bind vertex shader.");
            _didLogError = true;
          }
          return eFalse;
        }
        cMojoShaderPixel* pMojoPS = (cMojoShaderPixel*)pPS->Bind(NULL);
        if (!pMojoPS) {
          static bool _didLogError = false;
          if (!_didLogError) {
            niError("Can't bind pixel shader.");
            _didLogError = true;
          }
          return eFalse;
        }

        MOJOSHADER_glBindShaders(
          pMojoVS->GetShader(),
          pMojoPS->GetShader());
        GLERR_RET(eFalse);

        Mojo_ApplyVertexShader(
          apContext,
          this->mCache,
          mpGraphics,
          pVS,
          apDrawOp,
          pMatDesc);
        GLERR_RET(eFalse);

        Mojo_ApplyPixelShader(
          apContext,
          this->mCache,
          mpGraphics,
          pPS,
          apDrawOp,
          pMatDesc);
        GLERR_RET(eFalse);

        r = DoDrawOp(
          mCache,apDrawOp,
          viewMatrix,projMatrix,texFactor,
          pMatDesc ? pMatDesc->mChannels : NULL,
          &_mojoShaderNative);
#endif
      }
    }

    if (mptrDOCapture.IsOK()) {
      mptrDOCapture->EndCaptureDrawOp(apContext,apDrawOp,sVec4i::Zero());
    }

    return r;
  }

  void _ResetContextDeviceResources() {
#ifndef __GLES3__
    if (_glUseProgram)
#endif
    {
      // Disable all shaders...
      _glUseProgram(0);
    }

#ifndef __GLES3__
    if (_glActiveTexture)
#endif
    {
      // Disable all texture units
      for (tI32 i = kGL3_MaxTU-1; i >= 0; --i) {
#if defined USE_GL_UNBIND_TEXTURES
        _glActiveTexture(GL_TEXTURE0+i);
        _glBindTexture(GL_TEXTURE_2D,0);
        _glBindTexture(GL_TEXTURE_CUBE_MAP,0);
#endif
#if defined USE_GL_ENABLE_FOR_TEXTURE
        _glDisable(GL_TEXTURE_2D);
        _glDisable(GL_TEXTURE_CUBE_MAP);
#endif
      }
    }

    // Reset Shaders
    {
      Ptr<iDeviceResourceManager> shRM = mpGraphics->GetShaderDeviceResourceManager();
      niLoop(i,shRM->GetSize()) {
        Ptr<iDeviceResource> r = shRM->GetFromIndex(i);
        if (r.IsOK()) {
          r->ResetDeviceResource();
        }
      }
    }
    // Reset Textures
    {
      Ptr<iDeviceResourceManager> texRM = mpGraphics->GetTextureDeviceResourceManager();
      niLoop(i,texRM->GetSize()) {
        Ptr<iDeviceResource> r = texRM->GetFromIndex(i);
        if (r.IsOK()) {
          r->ResetDeviceResource();
        }
      }
    }
    // Reset Vertex Arrays and IndexArray
    {
      Ptr<iDeviceResourceManager> genRM = mpGraphics->GetGenericDeviceResourceManager();
      niLoop(i,genRM->GetSize()) {
        Ptr<iDeviceResource> r = genRM->GetFromIndex(i);
        if (r.IsOK()) {
          r->ResetDeviceResource();
        }
      }
    }
  }
  void _InitContextDeviceResources() {
#ifndef __GLES3__
    if (!_glActiveTexture)
      return;
#endif
    // Disable all texture units
    for (tI32 i = kGL3_MaxTU-1; i >= 0; --i) {
#if defined USE_GL_UNBIND_TEXTURES
      _glActiveTexture(GL_TEXTURE0+i);
      _glBindTexture(GL_TEXTURE_2D,0);
      _glBindTexture(GL_TEXTURE_CUBE_MAP,0);
#endif
#if defined USE_GL_ENABLE_FOR_TEXTURE
      _glDisable(GL_TEXTURE_2D);
      _glDisable(GL_TEXTURE_CUBE_MAP);
#endif
    }
    // _glFlush();
  }

  //// iGraphicsDriverGpu ///////////////////////////////

  LocalIDGenerator _idGenerator;
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBuffer(iHString* ahspName, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl;
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromData(iHString* ahspName, iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl;
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromDataRaw(iHString* ahspName, tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl;
  virtual iHString* __stdcall GetGpuFunctionTarget() const niImpl;
  virtual Ptr<iGpuFunction> __stdcall CreateGpuFunction(eGpuFunctionType aType, iHString* ahspPath) niImpl;
  virtual Ptr<iGpuPipelineDesc> __stdcall CreateGpuPipelineDesc() niImpl;
  virtual Ptr<iGpuBlendMode> __stdcall CreateGpuBlendMode() niImpl;
  virtual Ptr<iGpuPipeline> __stdcall CreateGpuPipeline(iHString* ahspName, const iGpuPipelineDesc* apDesc) niImpl;
  virtual tBool __stdcall BlitManagedGpuBufferToSystemMemory(iGpuBuffer* apBuffer) niImpl;
  virtual Ptr<iRayGpuPipeline> __stdcall CreateRayPipeline(iHString* ahspName, iRayGpuFunctionTable* apFunctionTable) niImpl;
  virtual Ptr<iRayGpuFunctionTable> __stdcall CreateRayFunctionTable() niImpl;
  virtual Ptr<iAccelerationStructurePrimitives> __stdcall CreateAccelerationStructurePrimitives(iHString* ahspName) niImpl;
  virtual Ptr<iAccelerationStructureInstances> __stdcall CreateAccelerationStructureInstances(iHString* ahspName) niImpl;
  //// iGraphicsDriverGpu ///////////////////////////////

};

struct sOpenGLBuffer : public ImplRC<iGpuBuffer, eImplFlags_DontInherit1, iDeviceResource> {
  tHStringPtr _name;
  GLuint _glBuffer = GLDRV_INVALID_HANDLE;
  eGpuBufferMemoryMode _memMode;
  tGpuBufferUsageFlags _usage;
  tU32 _lockOffset = 0, _lockSize = 0;
  tU32 _modifiedOffset = 0, _modifiedSize = 0;
  tU32 _lockMode = eInvalidHandle;
  tBool _boundModifiedBuffer = eFalse;
  astl::vector<tU8> _bufferData;
  GLenum _bufferTarget = GL_ARRAY_BUFFER;

  sOpenGLBuffer(eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage)
    : _memMode(aMemMode) , _usage(aUsage) {

    if (niFlagIs(_usage, eGpuBufferUsageFlags_Vertex)) {
      _bufferTarget = GL_ARRAY_BUFFER;
    }
    else if (niFlagIs(_usage, eGpuBufferUsageFlags_Index)) {
      _bufferTarget = GL_ELEMENT_ARRAY_BUFFER;
    }
    else if (niFlagIs(_usage, eGpuBufferUsageFlags_Uniform)) {
      _bufferTarget = GL_UNIFORM_BUFFER;
    }
  }

  ~sOpenGLBuffer() {
    _DestroyBuffer();
  }

  tBool _CreateBuffer(tU32 anSize, tU32 anMinAlignment) {
    _glGenBuffers(1, &_glBuffer);
    _glBindBuffer(_bufferTarget, _glBuffer);

    GLenum usage = GL_STATIC_DRAW;
    switch (_memMode) {
      case eGpuBufferMemoryMode_Shared:
        usage = GL_DYNAMIC_DRAW;
        break;
      case eGpuBufferMemoryMode_Private:
        usage = GL_STATIC_DRAW;
        break;
      case eGpuBufferMemoryMode_Managed:
        usage = GL_DYNAMIC_DRAW;
        break;
    }

    _bufferData.resize(anSize);
    ni::MemZero(_bufferData.data(), _bufferData.size());

    _glBufferData(_bufferTarget, anSize, _bufferData.data(), usage);
    return eTrue;
  }

  void _DestroyBuffer() {
    if (_glBuffer != GLDRV_INVALID_HANDLE) {
      glDeleteBuffers(1, &_glBuffer);
      _glBuffer = GLDRV_INVALID_HANDLE;
    }
  }

  void _Untrack() {
    _modifiedOffset = _modifiedSize = 0;
    _boundModifiedBuffer = eFalse;
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _glBuffer != GLDRV_INVALID_HANDLE;
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return _name;
  }

  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }

  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }

  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    if (_glBuffer == GLDRV_INVALID_HANDLE) return NULL;
    glBindBuffer(_bufferTarget, _glBuffer);
    // Retrieve the uniform block index
    return this;
  }

  virtual tU32 __stdcall GetSize() const niImpl {
    return _bufferData.size();
  }

  virtual eGpuBufferMemoryMode __stdcall GetMemoryMode() const niImpl {
    return _memMode;
  }

  virtual tGpuBufferUsageFlags __stdcall GetUsageFlags() const niImpl {
    return _usage;
  }

  virtual tPtr __stdcall Lock(tU32 anOffset, tU32 anSize, eLock aLock) niImpl {
    niCheck(_memMode != eGpuBufferMemoryMode_Private, nullptr);
    niCheck(!GetIsLocked(), nullptr);

    _lockMode = aLock;
    _lockOffset = anOffset;
    _lockSize = anSize ? anSize : (GetSize() - anOffset);

    if (_modifiedSize == 0) {
      _modifiedOffset = _lockOffset;
      _modifiedSize = _lockSize;
    }
    else if ((_lockOffset < (_modifiedOffset + _modifiedSize)) &&
             (_lockOffset + _lockSize) > _modifiedOffset) {
      if (_boundModifiedBuffer) {
        niWarning(niFmt(
          "Lock(%d,%d,%d): %p: [lo:%d,ls:%d] [mo:%d,ms:%d] Locked inflight overlapping area.",
          anOffset, anSize, aLock,
          (tIntPtr)this,
          _lockOffset, _lockSize,
          _modifiedOffset, _modifiedSize));
      }
      const tU32 newStart = ni::Min(_modifiedOffset, _lockOffset);
      const tU32 newEnd = ni::Max(_modifiedOffset + _modifiedSize,
                                  _lockOffset + _lockSize);
      _modifiedOffset = newStart;
      _modifiedSize = newEnd - newStart;
    }

    return (tPtr)_bufferData.data() + _lockOffset;
  }

  virtual tBool __stdcall Unlock() niImpl {
    if (!GetIsLocked())
      return eFalse;

    if (!niFlagIs(_lockMode,eLock_ReadOnly)) {
      _glBindBuffer(_bufferTarget,_glBuffer);
      _glBufferSubData(_bufferTarget, _lockOffset, _lockSize, _bufferData.data()+_lockOffset);
      _glBindBuffer(_bufferTarget,0);
      // niDebugFmt(("Unlock() %s, %s", _lockOffset, _lockSize));
    }

    _lockMode = eInvalidHandle;
    _lockOffset = _lockSize = 0;
    return eTrue;
  }

  virtual tBool __stdcall GetIsLocked() const niImpl {
    return _lockSize != 0;
  }

};

#define NISH_OPENGL_TARGET glsl_macos41

_HDecl(NISH_OPENGL_TARGET);
static niInline iHString* _GetOpenGLGpuFunctionTarget() {
  return _HC(NISH_OPENGL_TARGET);
}

struct sOpenGLFunction : public ImplRC<iGpuFunction, eImplFlags_DontInherit1, iDeviceResource> {
  NN<iDataTable> _datatable = niDeferredInit(NN<iDataTable>);
  const eGpuFunctionType _functionType;
  const tU32 _id;
  tHStringPtr _hspName;
  GLuint _glShader = GLDRV_INVALID_HANDLE; // OpenGL ES shader object
  eGpuFunctionBindType _bindType;

  sOpenGLFunction(
    ain<eGpuFunctionType> aFuncType,
    ain<tU32> anID)
      : _functionType(aFuncType)
      , _id(anID)
  {}

  ~sOpenGLFunction() {
    if (_glShader) {
      glDeleteShader(_glShader); // Delete the shader object
      _glShader = GLDRV_INVALID_HANDLE;
    }
  }

  tBool _Compile(iHString* ahspPath) {
    GLenum type = _GetOpenGLShaderType(_functionType);
    if (!type) {
      niError(niFmt("Shader type [%s] is not supported", _functionType));
      return eFalse;
    }

    _hspName = ahspPath;

    // GpuFunctionDT_Load(niHStr(ahspPath), _GetOpenGLGpuFunctionTarget(), &_bindType);
    _datatable = niCheckNN(_datatable, GpuFunctionDT_Load(niHStr(ahspPath), _GetOpenGLGpuFunctionTarget(), &_bindType), eFalse);
    NN<iFile> glslSource = niCheckNN_(
      glslSource, GpuFunctionDT_GetSourceData(_datatable),
      niFmt("Can't get gpufunc data for target '%s' in '%s'.", _GetOpenGLGpuFunctionTarget(), ahspPath),
      eFalse);

    glslSource->SeekSet(0);
    astl::vector<tU8> data;
    data.resize(glslSource->GetSize());
    if (glslSource->ReadRaw((tPtr)data.data(), data.size()) != data.size()) {
      niError(niFmt("Can't read gpufunc data for target '%s' in '%s'.", _GetOpenGLGpuFunctionTarget(), ahspPath));
      return eFalse;
    }

    // Null-terminate the shader source string
    data.push_back(0);

    // Create and compile the shader
    _glShader = glCreateShader(type);
    GLERR_RET(eFalse);

    const char* source = reinterpret_cast<const char*>(data.data());
    glShaderSource(_glShader, 1, &source, nullptr);
    glCompileShader(_glShader);

    // Check for compilation errors
    GLint compileStatus;
    glGetShaderiv(_glShader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE) {
      GLchar infoLog[1024];
      glGetShaderInfoLog(_glShader, sizeof(infoLog), nullptr, infoLog);
      niError(niFmt("Shader compilation failed: %s", infoLog));
      glDeleteShader(_glShader);
      _glShader = GLDRV_INVALID_HANDLE;
      return eFalse;
    }

    // niDebugFmt(("Shader compilation succeed: %s", glslSource->GetSourcePath()));
    return eTrue;
  }

  virtual tU32 __stdcall GetFunctionId() const niImpl {
    return _id;
  }

  virtual eGpuFunctionType __stdcall GetFunctionType() const niImpl {
    return _functionType;
  }

  virtual eGpuFunctionBindType __stdcall GetFunctionBindType() const niImpl {
    return _bindType;
  }

  virtual iDataTable* __stdcall GetDataTable() const niImpl {
    return _datatable;
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return _hspName;
  }

  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }

  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }

  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    return this;
  }

    // Helper function to map eGpuFunctionType to OpenGL ES shader type
  GLenum _GetOpenGLShaderType(eGpuFunctionType type) const {
    switch (type) {
      case eGpuFunctionType_Vertex:   return GL_VERTEX_SHADER;
      case eGpuFunctionType_Pixel:    return GL_FRAGMENT_SHADER;
      default:                        return 0;
    }
  }
};

struct sOpenGLRasterPipeline :
  public ImplRC<iGpuPipeline, eImplFlags_DontInherit1, iDeviceResource>
{
  nn<sOpenGLDriver> _driver; // TODO: Should be a weakptr
  tHStringPtr _hspName;
  NN<iGpuPipelineDesc> _desc = niDeferredInit(NN<iGpuPipelineDesc>);
  eGpuFunctionBindType _gpufuncBindType = eGpuFunctionBindType_None;

  GLuint _programID = GLDRV_INVALID_HANDLE;
  GLuint _vaoID = GLDRV_INVALID_HANDLE;

  tBool _isBinded = eFalse;

  sOpenGLRasterPipeline(ain<nn<sOpenGLDriver>> aDriver)
    : _driver(aDriver)
  {}

  ~sOpenGLRasterPipeline() {
    _DestroyPipeline();
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return _hspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }

  void _UpdateVAO(tU32 offset) {
    GLCALL_WARN(glBindVertexArray(_vaoID));


#define ATTR(USAGE,SIZE,TYPE,BASE,NORMALIZED)                           \
    GLCALL_WARN(_glEnableVertexAttribArray(eVertexStreamIndex_##USAGE)); \
    GLCALL_WARN(_glVertexAttribPointer(eVertexStreamIndex_##USAGE, SIZE, TYPE, NORMALIZED, nStride, reinterpret_cast<void*>(BASE)));

    cFVFDescription fvfDesc(_desc->GetFVF());
    _desc->GetColorFormat(0);
    tPtr pVABase = reinterpret_cast<tPtr>(offset);
    const tU32 nStride = fvfDesc.GetStride();
    ATTR(Position, 3, GL_FLOAT, pVABase, GL_FALSE);
    if (fvfDesc.HasColorA()) {
      ATTR(ColorA, GL_BGRA, GL_UNSIGNED_BYTE, pVABase+fvfDesc.GetColorAOffset(), GL_TRUE);
    }

    if (fvfDesc.HasNormal()) {
      ATTR(Normal, 3, GL_FLOAT, pVABase+fvfDesc.GetNormalOffset(), GL_FALSE);
    }

    if (fvfDesc.HasTexCoo(0)) {
      ATTR(Tex1, 2, GL_FLOAT, pVABase+fvfDesc.GetTexCooOffset(0), GL_FALSE);
    }

    if (fvfDesc.HasTexCoo(1)) {
      ATTR(Tex2, 2, GL_FLOAT, pVABase+fvfDesc.GetTexCooOffset(1), GL_FALSE);
    }
  }

  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    niCheck(_programID != GLDRV_INVALID_HANDLE, NULL);
    glUseProgram(_programID);

    // GL_ApplyDepthStencilStates(_driver->mCache)

    // _desc->GetDepthFormat()

#if 0
    // Set up vertex attributes
    niLet vertexAttrs = OpenGL_CreateVertexInputDesc(_desc->GetFVF());
    for (const auto& attr : vertexAttrs) {
      glEnableVertexAttribArray(attr.location);
      glVertexAttribPointer(
        attr.location,
        attr.size,
        attr.type,
        attr.normalized,
        fvfDesc.GetStride(),
        (void*)(uintptr_t)attr.offset);
    }

    // Depth stencil
    niLet ds = GetGpuDepthStencilDesc(graphics, _desc->GetDepthStencilStates());
    if (ds->mbDepthTest) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(_ToGLCompareFunc(ds->mDepthTestCompare));
      glDepthMask(ds->mbDepthTestWrite ? GL_TRUE : GL_FALSE);
    } else {
      glDisable(GL_DEPTH_TEST);
    }

    if (ds->mStencilMode != eStencilMode_None) {
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(_ToGLCompareFunc(ds->mStencilFrontCompare), ds->mnStencilRef, ds->mnStencilMask);
      glStencilOp(
        _ToGLStencilOp(ds->mStencilFrontFail),
        _ToGLStencilOp(ds->mStencilFrontPassDepthPass),
        _ToGLStencilOp(ds->mStencilFrontPassDepthFail));
      if (ds->mStencilMode == eStencilMode_TwoSided) {
        glStencilFuncSeparate(GL_BACK, _ToGLCompareFunc(ds->mStencilBackCompare), ds->mnStencilRef, ds->mnStencilMask);
        glStencilOpSeparate(
          GL_BACK,
          _ToGLStencilOp(ds->mStencilBackFail),
          _ToGLStencilOp(ds->mStencilBackPassDepthPass),
          _ToGLStencilOp(ds->mStencilBackPassDepthFail));
      }
    } else {
      glDisable(GL_STENCIL_TEST);
    }

    // Blend mode
    if (_desc->GetBlendMode()) {
      const sGpuBlendModeDesc* bm = (const sGpuBlendModeDesc*)_desc->GetBlendMode()->GetDescStructPtr();
      glEnable(GL_BLEND);
      // glBlendFuncSeparate(
      // _ToGLBlendFunc(bm->mSrcRGB),
      // _ToGLBlendFunc(bm->mDstRGB),
      // _ToGLBlendFunc(bm->mSrcAlpha),
      // _ToGLBlendFunc(bm->mDstAlpha));
      // glBlendEquation(_ToGLBlendEquation(bm->mOp));
    } else {
      glDisable(GL_BLEND);
    }
#endif

    return this;
  }

  tBool _CreateNoneDescSetLayout() {
    // In OpenGL, descriptor sets are not used, so this function is a no-op.
    return eTrue;
  }

  tBool _CreateFixedDescSetLayout() {
    // In OpenGL, descriptor sets are not used, so this function is a no-op.
    return eTrue;
  }

  tBool _CreateOpenGLPipeline(iHString* ahspName, const iGpuPipelineDesc* apDesc) {
    niCheckIsOK(apDesc, eFalse);
    _hspName = ahspName;
    _desc = niCheckNN(_desc, apDesc->Clone(), eFalse);

    niLet graphics = as_nn(_driver->GetGraphics());

    // Shaders
    sOpenGLFunction* vs = (sOpenGLFunction*)_desc->GetFunction(eGpuFunctionType_Vertex);
    niCheck(vs, eFalse);
    sOpenGLFunction* ps = (sOpenGLFunction*)_desc->GetFunction(eGpuFunctionType_Pixel);
    niCheck(ps, eFalse);

    // Check that pipeline gpu functions use compatible bind types
    {
      niLet vsFuncBindType = vs->GetFunctionBindType();
      niLet psFuncBindType = ps->GetFunctionBindType();
      if (vsFuncBindType != psFuncBindType &&
          vsFuncBindType != eGpuFunctionBindType_None &&
          psFuncBindType != eGpuFunctionBindType_None)
      {
        niError(niFmt(
                  "Incompatible gpu function bind types: vertex='%d', pixel='%d'",
                  vsFuncBindType, psFuncBindType));
        return eFalse;
      }
      _gpufuncBindType = ni::Max(vsFuncBindType, psFuncBindType);
    }

    // Create the OpenGL program
    _programID = glCreateProgram();
    glAttachShader(_programID, vs->_glShader);
    glAttachShader(_programID, ps->_glShader);
    glLinkProgram(_programID);


    // Check for linking errors
    GLint success;
    glGetProgramiv(_programID, GL_LINK_STATUS, &success);
    if (!success) {
      GLchar infoLog[512];
      glGetProgramInfoLog(_programID, 512, nullptr, infoLog);
      niError(niFmt("Shader program linking failed: %s", infoLog));
      _programID = GLDRV_INVALID_HANDLE;
      return eFalse;
    }

    glGenVertexArrays(1, &_vaoID);

    niDebugFmt(("... Created OpenGL Pipeline -> name:%s vao:%s program:%s", ahspName, _vaoID, _programID));
    return eTrue;
  }

  virtual const iGpuPipelineDesc* __stdcall GetDesc() const niImpl {
    return _desc;
  }

  void _DestroyPipeline() {
    if (_programID != GLDRV_INVALID_HANDLE) {
      glDeleteProgram(_programID);
      _programID = GLDRV_INVALID_HANDLE;
    }

    if (_vaoID != GLDRV_INVALID_HANDLE) {
      glDeleteVertexArrays(1, &_vaoID);
      _vaoID = GLDRV_INVALID_HANDLE;
    }
  }
};

static Ptr<sOpenGLRasterPipeline> __stdcall CreateOpenGLRasterPipeline(
  ain_nn<sOpenGLDriver> aDriver,
  iHString* ahspName,
  const iGpuPipelineDesc* apDesc)
{
  niCheckIsOK(apDesc,nullptr);
  NN<sOpenGLRasterPipeline> pipeline = MakeNN<sOpenGLRasterPipeline>(aDriver);
  niCheck(pipeline->_CreateOpenGLPipeline(ahspName,apDesc), nullptr);
  return pipeline;
}

struct sOpenGLEncoderFrameData : public ImplRC<iUnknown> {
  ThreadEvent _eventFrameCompleted = ThreadEvent(eFalse);
  astl::vector<Ptr<sOpenGLBuffer>> _trackedBuffers;
  astl::vector<Ptr<sOpenglTexture>> _trackedTextures;
  astl::vector<Ptr<sOpenGLRasterPipeline>> _trackedPipelines;
  Ptr<iGpuStream> _stream;
  tBool _inFrame = eFalse;

  sOpenGLEncoderFrameData(ain<nn<sOpenGLDriver>> aDriver) {
    _stream = CreateGpuStream(
      aDriver,
      eGpuBufferUsageFlags_Vertex |
      eGpuBufferUsageFlags_Index |
      eGpuBufferUsageFlags_Uniform,
      0); // not needed for opengl we can define that later
  }

  void Destroy() {
    // No explicit destruction needed for OpenGL resources in this context
    // OpenGL resources are managed by the driver and released when the context is destroyed
  }

  niInline sOpenGLBuffer* BindBuffer(iGpuBuffer* apBuffer) {
    niLet buffer = (sOpenGLBuffer*)apBuffer;
    if (!buffer->_boundModifiedBuffer && buffer->_modifiedSize) {
      buffer->_boundModifiedBuffer = eTrue;
    }
    buffer->Bind(NULL);
    _trackedBuffers.push_back(buffer);
    return buffer;
  }

  niInline sOpenglTexture* BindTexture(iTexture* apTexture) {
    sOpenglTexture* texture = (sOpenglTexture*)apTexture->Bind(NULL); // Note: Bind() is a noop so we don't call it
    _trackedTextures.push_back(texture);
    return texture;
  }

  niInline sOpenGLRasterPipeline* BindPipeline(iGpuPipeline* apPipeline) {
    sOpenGLRasterPipeline* pipeline = (sOpenGLRasterPipeline*)apPipeline->Bind(NULL);
    _trackedPipelines.push_back(pipeline);
    // niDebugFmt(("_trackedPipelines.size() %s", _trackedPipelines.size()));
    return pipeline;
  }

  void OnBeginFrame() {
    niPanicAssert(_inFrame == eFalse);
    _inFrame = eTrue;
  }

  void OnEndFrame() {
    // Release tracked resources
    niLoop(i, _trackedBuffers.size()) {
      _trackedBuffers[i]->_Untrack();
    }
    _trackedBuffers.clear();
    _trackedTextures.clear();
    _trackedPipelines.clear();

    // Reset the stream
    _stream->Reset();

    // Signal that the frame is completed
    _inFrame = eFalse;
    _eventFrameCompleted.Signal();
  }

  void WaitFrameCompleted() {
    if (_inFrame) {
      _eventFrameCompleted.InfiniteWait();
    }
  }
};


sOpenGLCommandEncoder::sOpenGLCommandEncoder(iGraphicsDriver* aDriver, ain<tU32> aFrameMaxInFlight)
  : _driver(astl::as_non_null(static_cast<sOpenGLDriver*>(aDriver)))
{
  _frames.reserve(aFrameMaxInFlight);
  niLoop(i, aFrameMaxInFlight) {
    _frames.emplace_back(niNew sOpenGLEncoderFrameData(_driver));
  }
}

sOpenGLCommandEncoder::~sOpenGLCommandEncoder() {
  niDebugFmt(("~sOpenGLCommandEncoder"));
  niLoop(i, _frames.size()) {
    _frames[i]->WaitFrameCompleted();
    _frames[i]->Destroy();
  }
  if (_encoderInFlightFence) {
    glDeleteSync(_encoderInFlightFence);
    _encoderInFlightFence = 0;
  }
  if (_fbo) {
    glDeleteFramebuffers(1, &_fbo);
    _fbo = 0;
  }
}

sOpenGLEncoderFrameData* sOpenGLCommandEncoder::_GetCurrentFrame() {
  return _frames[_currentFrame];
}

tBool sOpenGLCommandEncoder::_CreateCommandBuffer() {
  niDebugFmt(("_CreateCommandBuffer()"));
  // niCheck(_driver->_context != nullptr, eFalse);
  // In OpenGL, we don't explicitly create command buffers, but we can create an FBO for rendering.
  // glGenFramebuffers(1, &_fbo);
  return eTrue;
}

tBool sOpenGLCommandEncoder::_BeginCmdBuffer() {
  niCheck(_beganCmdBuffer == eFalse, eFalse);
  // niDebugAssert(_fbo != 0);
  // if (!_fbo) {
    // glGenFramebuffers(1, &_fbo);
  // }

  _beganCmdBuffer = eTrue;
  _cache = sCache {};

  // glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  _GetCurrentFrame()->OnBeginFrame();
  return eTrue;
}

void sOpenGLCommandEncoder::_ResumeRendering() {
  _renderingInfo._ResumeRenderingInfo();
  // glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
}

void sOpenGLCommandEncoder::_EndRendering() {
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

tBool sOpenGLCommandEncoder::_EndCmdBufferAndSubmit(
  GLsync aImageAvailableSemaphore,
  GLsync aRendererFinishedSemaphore)
{
  niCheck(_beganCmdBuffer == eTrue, eFalse);
  niDefer {
    _beganCmdBuffer = eFalse;
  };

  // glFlush();

  // if (aImageAvailableSemaphore) {
    // glWaitSync(aImageAvailableSemaphore, 0, GL_TIMEOUT_IGNORED);
  // }

  // if (aRendererFinishedSemaphore) {
    // _encoderInFlightFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  // }

  // _inFrame = eFalse;
  _GetCurrentFrame()->OnEndFrame();
  return eTrue;
}

void sOpenGLCommandEncoder::_DoBindPipeline(iGpuPipeline* apPipeline, tFixedGpuPipelineId aFixedPipelineId) {
  sOpenGLRasterPipeline* pipeline = static_cast<sOpenGLRasterPipeline*>(apPipeline);
  _cache._lastPipeline = _GetCurrentFrame()->BindPipeline(apPipeline);
}

tBool sOpenGLCommandEncoder::_BindGpuFunction() {
  niLet pipeline = _cache._lastPipeline;
  niCheck(pipeline.IsOK(),eFalse)

  switch (pipeline->_gpufuncBindType) {
    case ni::eGpuFunctionBindType_None: {
      if (_cache._lastBuffer.IsOK()) {
      }
      break;
    }
    case ni::eGpuFunctionBindType_Fixed:

      break;
    case ni::eGpuFunctionBindType_Material:
      break;
    case ni::eGpuFunctionBindType_Bindless:
      break;
    case ni::eGpuFunctionBindType_Last:
      break;
  }
  return eTrue;
}

tBool sOpenGLCommandEncoder::_BegainDraw() {

      // Depth stencil states
      {
        tIntPtr hDS = apContext->_GetDS(pMatDesc);
        if (!hDS) {
          hDS = eCompiledStates_DS_NoDepthTest;
        }
        tBool bShouldUpdate = eFalse;
        mCache.ShouldUpdate(&bShouldUpdate,eGLCache_DepthStencil,hDS);
        if (bShouldUpdate)
        {
          iDepthStencilStates* pDSStates = mpGraphics->GetCompiledDepthStencilStates(hDS);
          niCheck(pDSStates,eFalse);
          GL_ApplyDepthStencilStates(
              this->mCache,
              *(const sDepthStencilStatesDesc*)pDSStates->GetDescStructPtr());
        }
      }

  return eTrue;
}

tBool sOpenGLCommandEncoder::_EndDraw() {
  return eTrue;
}

void __stdcall sOpenGLCommandEncoder::SetPipeline(iGpuPipeline* apPipeline) {
  niCheck(apPipeline != nullptr, ;);
  if ((tIntPtr)_cache._lastPipeline.raw_ptr() == (tIntPtr)apPipeline)
    return;
  _DoBindPipeline(apPipeline, 0);
}

void __stdcall sOpenGLCommandEncoder::_SetFixedPipeline(iGpuPipeline* apPipeline, tFixedGpuPipelineId aFixedPipelineId) {
  _DoBindPipeline(apPipeline, aFixedPipelineId);
}

void __stdcall sOpenGLCommandEncoder::SetVertexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) {
  niCheck(apBuffer != nullptr, ;);
  sOpenGLBuffer* buffer = _GetCurrentFrame()->BindBuffer(apBuffer);
  _cache._lastPipeline->_UpdateVAO(anOffset);
}

void __stdcall sOpenGLCommandEncoder::SetIndexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, eGpuIndexType aIndexType) {
  niCheck(apBuffer != nullptr, ;);
  sOpenGLBuffer* indexBuffer = _GetCurrentFrame()->BindBuffer(apBuffer);
}

void __stdcall sOpenGLCommandEncoder::SetUniformBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) {
  niCheck(apBuffer != nullptr, ;);
  sOpenGLBuffer* uniformBuffer = _GetCurrentFrame()->BindBuffer(apBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, anBinding, uniformBuffer->_glBuffer);
}

void __stdcall sOpenGLCommandEncoder::SetTexture(iTexture* apTexture, tU32 anBinding)  {
  niCheck(anBinding < eMaterialChannel_Last, ;);
  // Create a default white texture if the input is null
  if (!apTexture) {
    apTexture = (sOpenglTexture*)_driver->_fixedPipelines->GetWhiteTexture().raw_ptr();
  }

  sOpenglTexture* tex = _GetCurrentFrame()->BindTexture(apTexture);
  _cache._lastMaterial.mChannels[anBinding].mTexture = tex;
}

void __stdcall sOpenGLCommandEncoder::SetSamplerState(tIntPtr ahSS, tU32 anBinding)  {
  niCheck(anBinding < eMaterialChannel_Last, ;);
  _cache._lastMaterial.mChannels[anBinding].mhSS = ahSS;

  sOpenglTextureBase* tex = (sOpenglTextureBase*)_cache._lastMaterial.mChannels[anBinding].mTexture.ptr();
  tex = tex ? (sOpenglTextureBase*)tex->Bind(NULL) : NULL;
  if (tex) {
    const sSamplerStatesDesc* pSS = NULL;
    tIntPtr hSS = ahSS;
    if (!hSS) {
      if (tex->mhLastSamplerStateSet) {
        hSS = tex->mhLastSamplerStateSet;
      }
      else {
        hSS = eCompiledStates_SS_SmoothClamp;
      }
    }
    tBool bShouldUpdate = tex->mhLastSamplerStateSet != hSS;
    if (bShouldUpdate) {
      tex->mhLastSamplerStateSet = hSS;
      // niDebugFmt(("... GL3 Texture SetSamplerStates '%s': %p", tex->mhspName, hSS));
      {
        iSamplerStates* ss = _driver->GetGraphics()->GetCompiledSamplerStates(hSS);
        if (ss) {
          pSS = (const sSamplerStatesDesc*)ss->GetDescStructPtr();
        }
      }
      if (pSS) {
        const GLenum texKind = GL_Texture(tex->GetType());
        const tBool isOverlay = !!(tex->mFormat.flags & eTextureFlags_Overlay);
        GL_TexSamplerWrap(texKind, GL_TEXTURE_WRAP_S, pSS->mWrapS, isOverlay);
        GLERR_RET();
        GL_TexSamplerWrap(texKind, GL_TEXTURE_WRAP_T, pSS->mWrapT, isOverlay);
        GLERR_RET(;);
#ifdef GL_TEXTURE_WRAP_R
        if (hasWrapR) {
          GL_TexSamplerWrap(texKind, GL_TEXTURE_WRAP_R, pSS->mWrapT, isOverlay);
          GLERR_RET(;);
        }
#endif
        const tU32 numMips = tex->GetNumMipMaps();
        GL_TexSamplerFilter(texKind, pSS->mFilter, numMips);
        GLERR_RET();
      }
    }
  }
}

tBool __stdcall sOpenGLCommandEncoder::StreamVertexBuffer(const tPtr apData, tU32 anSize, tU32 anBinding)  {
  return UpdateGpuStreamToVertexBuffer(_GetCurrentFrame()->_stream, this, apData, anSize, anBinding);
}

tBool __stdcall sOpenGLCommandEncoder::StreamIndexBuffer(const tPtr apData, tU32 anSize, eGpuIndexType aIndexType)  {
  return UpdateGpuStreamToIndexBuffer(_GetCurrentFrame()->_stream, this, apData, anSize, aIndexType);
}

tBool __stdcall sOpenGLCommandEncoder::StreamUniformBuffer(const tPtr apData, tU32 anSize, tU32 anBinding)  {
  return UpdateGpuStreamToUniformBuffer(_GetCurrentFrame()->_stream, this, apData, anSize, anBinding);
}

void __stdcall sOpenGLCommandEncoder::SetPolygonOffset(const sVec2f& avOffset)  {
  glPolygonOffset(avOffset.x, avOffset.y);
}

void __stdcall sOpenGLCommandEncoder::SetViewport(const sRecti& aRect)  {
  niLet rtSize = _renderingInfo._rtSize;
  tBool isFlippedRT = _renderingInfo._rtFlipped;
  sRecti vp = aRect;
  if (vp.GetWidth() == 0) vp.SetWidth(rtSize.x);
  if (vp.GetHeight() == 0) vp.SetHeight(rtSize.y);
  if ((vp.x+vp.GetWidth()) > rtSize.x) vp.SetWidth(vp.GetWidth() - ((vp.x+vp.GetWidth())-rtSize.x));
  if ((vp.y+vp.GetHeight()) > rtSize.y) vp.SetHeight(vp.GetHeight() - ((vp.y+vp.GetHeight())-rtSize.y));

  _renderingInfo._viewport = vp;
  const tU32 nSCY = isFlippedRT ? vp.y : rtSize.y - vp.y -vp.GetHeight();
  _glViewport(vp.x, nSCY, vp.GetWidth(), vp.GetHeight());
}

void __stdcall sOpenGLCommandEncoder::SetScissorRect(const sRecti& aRect)  {
  _renderingInfo._scissor = aRect;
  const sRecti sc = _renderingInfo._viewport.ClipRect(aRect);
  niLet rtSize = _renderingInfo._rtSize;
  tBool usScissor = (rtSize.x != sc.GetWidth() || rtSize.y != sc.GetHeight());
  if (usScissor) {
    _glEnable(GL_SCISSOR_TEST);
    const tU32 nSCY = _renderingInfo._rtFlipped ? sc.y : rtSize.y -sc.y -sc.GetHeight();
    // const tU32 nSCY = rtSize.y - sc.y - sc.GetHeight();
    _glScissor(sc.x, nSCY, sc.GetWidth(), sc.GetHeight());
  }
  else {
    _glDisable(GL_SCISSOR_TEST);
  }
}

void __stdcall sOpenGLCommandEncoder::SetStencilReference(tI32 aRef)  {
  glStencilFunc(GL_ALWAYS, aRef, 0xFF);
}

void __stdcall sOpenGLCommandEncoder::SetStencilMask(tU32 aMask)  {
  _glStencilMask(aMask);
}

void __stdcall sOpenGLCommandEncoder::SetBlendColorConstant(const sColor4f& aColor)  {
  glBlendColor(aColor.x, aColor.y, aColor.z, aColor.w);
}

tBool __stdcall sOpenGLCommandEncoder::DrawIndexed(eGraphicsPrimitiveType aPrimType, tU32 anNumIndices, tU32 anFirstIndex) {
  niCheck(aPrimType <= eGraphicsPrimitiveType_Last, eFalse);
  glDrawElements(GL_Primitive(aPrimType), anNumIndices, GL_UNSIGNED_INT, (void*)(uintptr_t)(anFirstIndex * sizeof(tU32)));
  return eTrue;
}

tBool __stdcall sOpenGLCommandEncoder::Draw(eGraphicsPrimitiveType aPrimType, tU32 anVertexCount, tU32 anFirstVertex) {
  niCheck(aPrimType <= eGraphicsPrimitiveType_Last, eFalse);
  glDrawArrays(GL_Primitive(aPrimType), anFirstVertex, anVertexCount);
  return eTrue;
}

iGpuCommandEncoder* __stdcall sGLContext::GetCommandEncoder() {
  // tBool isFlipped;
  // GL3_ApplyContext(mEncoder->_driver->mCache, this, isFlipped);
  // mEncoder->SetViewport(GetViewport());
  // mEncoder->SetScissorRect(GetScissorRect());
  // if (!_beganFrame) {
  niCheck(_BeginFrame(),nullptr);
  // }
  return mEncoder;
}

static tBool _BindRenderTargets(sGLContext* apCtx) {
  if (!apCtx->mptrRT[0].IsOK()) {
    niError("No render target set.");
    return eFalse;
  }

  Ptr<sOpenglTexture> pRT = (sOpenglTexture*)apCtx->mptrRT[0]->Bind((iUnknown*)eInvalidHandle);
  if (!pRT->_BindAsRenderTarget()) {
    niError("Can't bind render target.");
    return eFalse;
  }

  if (apCtx->mptrDS.IsOK()) {
    Ptr<sOpenglTexture> pDS = (sOpenglTexture*)apCtx->mptrDS->Bind((iUnknown*)eInvalidHandle);
    if (!pDS->_BindAsDepthStencil()) {
      niError("Can't bind depth stencil.");
      return eFalse;
    }
  }

  if (!niFlagIs(pRT->mFormat.flags,eTextureFlags_MainRT)) {
    GLFBO_RET(eFalse);
  }
  return eTrue;
}

tBool sGLContext::_BeginFrame() {

  tBool isFlipped;
  GL3_ApplyContext(mEncoder->_driver->mCache, this, isFlipped);

  if (_beganFrame) return eTrue;

  for (tI32 i = kGL3_MaxTU-1; i >= 0; --i) {
    _glActiveTexture(GL_TEXTURE0+i);
    _glBindTexture(GL_TEXTURE_2D,0);
    _glBindTexture(GL_TEXTURE_CUBE_MAP,0);
  }

  // Begin buffer and rendering
  niCheck(mEncoder->_BeginCmdBuffer(),eFalse);
  // mEncoder->_BeginRendering(0,0);
  _beganFrame = eTrue;
  return eTrue;
}


inline void _ClearBuffers(tClearBuffersFlags aFlags, tU32 anColor, tF32 afDepth, tI32 anStencil) {
  tU32 flags = 0;
  if (aFlags & eClearBuffersFlags_Color) {
    _glClearColor(ULColorGetRf(anColor),
                  ULColorGetGf(anColor),
                  ULColorGetBf(anColor),
                  ULColorGetAf(anColor));
    flags |= GL_COLOR_BUFFER_BIT;
  }

  if (aFlags & eClearBuffersFlags_Depth) {
    _glDepthMask(GL_TRUE);
    _glClearDepthf(afDepth);
    flags |= GL_DEPTH_BUFFER_BIT;
  }

#ifndef NO_STENCIL_BUFFER
  if (aFlags & eClearBuffersFlags_Stencil) {
    _glClearStencil(anStencil);
    flags |= GL_STENCIL_BUFFER_BIT;
  }
#endif

  if (flags) {
    _glClear(flags);
  }
}

void __stdcall sGLContext::ClearBuffers(tClearBuffersFlags aFlags, tU32 anColor, tF32 afDepth, tI32 anStencil) {
  niUnused(anStencil);
  this->ClearBuffersRect(
    aFlags,
    Rectf(0,0,(tF32)this->GetWidth(),(tF32)this->GetHeight()),
    anColor, afDepth);
}

tBool __stdcall sGLContext::ClearBuffersRect(tClearBuffersFlags aFlags, const sRectf& aRect, tU32 anColor, tF32 afZ) {
  niCheck(_BeginFrame(),eFalse);

  mEncoder->SetScissorRect(aRect.ToInt());
  _ClearBuffers(aFlags, anColor, afZ, 0);
  _glDisable(GL_SCISSOR_TEST);
  return eTrue;

  // niLet pixelSize = Vec2f(2.0f / (tF32)this->GetWidth(), 2.0f / (tF32)this->GetHeight());
  // sOpenGLDriver* driver = static_cast<sOpenGLDriver*>(GetDriver());
  // return driver ? driver->_fixedPipelines->ClearRect(mEncoder,pixelSize,aFlags,aRect,anColor,afZ) : eFalse;
}

Ptr<iGpuBuffer> sOpenGLDriver::CreateGpuBuffer(iHString* ahspName, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) {
  niLet buffer = ni::MakeNN<sOpenGLBuffer>(aMemMode,aUsage);
  niCheck(buffer->_CreateBuffer(anSize,0),nullptr);
  return buffer;
}

Ptr<iGpuBuffer> sOpenGLDriver::CreateGpuBufferFromData(iHString* ahspName, iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) {
  niCheckIsOK(apFile,nullptr);
  astl::vector<tU8> data;
  data.resize(anSize);
  if (apFile->ReadRaw(data.data(),anSize) != anSize) {
    return nullptr;
  }
  return this->CreateGpuBufferFromDataRaw(ahspName,data.data(),anSize,aMemMode,aUsage);
}

Ptr<iGpuBuffer> sOpenGLDriver::CreateGpuBufferFromDataRaw(iHString* ahspName, tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) {
  niCheck(apData != nullptr, nullptr);
  niLet buffer = ni::MakeNN<sOpenGLBuffer>(aMemMode,aUsage);
  // TODO: Alignment should be a parameter or coming from a device cap
  niCheck(buffer->_CreateBuffer(anSize,0),nullptr);
  {
    niLet data = buffer->Lock(0,anSize,eLock_Discard);
    niCheck(data != nullptr,nullptr);
    memcpy(data,apData,anSize);
    buffer->Unlock();
  }
  return buffer;
}

iHString* sOpenGLDriver::GetGpuFunctionTarget() const {
  return _GetOpenGLGpuFunctionTarget();
}

Ptr<iGpuFunction> sOpenGLDriver::CreateGpuFunction(eGpuFunctionType aType, iHString* ahspPath) {
  niLet newId = _idGenerator.AllocID();
  NN<sOpenGLFunction> func = ni::MakeNN<sOpenGLFunction>(aType,newId);
  if (!func->_Compile(ahspPath)) {
    _idGenerator.FreeID(newId);
    niError(niFmt("Can't create gpu function '%s': Compilation failed.", ahspPath));
    return nullptr;
  }
  return func;
}

Ptr<iGpuPipelineDesc> sOpenGLDriver::CreateGpuPipelineDesc() {
  return ni::_CreateGpuPipelineDesc();
}

Ptr<iGpuBlendMode> sOpenGLDriver::CreateGpuBlendMode() {
  return ni::_CreateGpuBlendMode();
}

Ptr<iGpuPipeline> sOpenGLDriver::CreateGpuPipeline(iHString* ahspName, const iGpuPipelineDesc* apDesc) {
  return CreateOpenGLRasterPipeline(as_nn(this),ahspName,apDesc);
};

tBool sOpenGLDriver::BlitManagedGpuBufferToSystemMemory(iGpuBuffer* apBuffer) {
  niPanicUnreachable("BlitManagedGpuBufferToSystemMemory not unimplemented in opengl driver.");
  return eFalse;
}

Ptr<iRayGpuPipeline> sOpenGLDriver::CreateRayPipeline(iHString* ahspName, iRayGpuFunctionTable* apFunctionTable) {
  niPanicUnreachable("CreateRayPipeline not unimplemented in opengl driver.");
  return NULL;
}

Ptr<iRayGpuFunctionTable> sOpenGLDriver::CreateRayFunctionTable() {
  niPanicUnreachable("CreateRayFunctionTable not unimplemented in opengl driver.");
  return NULL;
}

Ptr<iAccelerationStructurePrimitives> sOpenGLDriver::CreateAccelerationStructurePrimitives(iHString* ahspName) {
  niPanicUnreachable("CreateAccelerationStructurePrimitives not unimplemented in opengl driver.");
  return NULL;
}

Ptr<iAccelerationStructureInstances> sOpenGLDriver::CreateAccelerationStructureInstances(iHString* ahspName) {
  niPanicUnreachable("CreateAccelerationStructureInstances not unimplemented in opengl driver.");
  return NULL;
}


/////////////////////////////////////////////
tBool __stdcall cGL3ContextRT::Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
  niCheck(_beganFrame,eFalse);
  // ResetAllCaches();
  _beganFrame = eFalse;

  ++mnSyncCounter; // Make sure the Viewport and scissor will be set next frame

  mEncoder->_EndRendering();
  mEncoder->_EndCmdBufferAndSubmit(0, 0);
  return eTrue;
}


static tBool GLES3_SwapBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tBool abDoNotWait) {
  GL_DEBUG_SWAP_BUFFERS();

  sOpenGLDriver* d = ((sOpenGLDriver*)apDrv);
  tBool isFlippedRT;
  if (!GL3_ApplyContext(d->mCache,apContext,isFlippedRT)) {
    niError(_A("Can't apply context."));
    return eFalse;
  }

#ifdef TSGL_CONTEXT
  tsglContext* ctx = apContext->GetTSGLContext();
  if (ctx) {
    GLCALL_ERR(tsglSwapBuffers(ctx,abDoNotWait),eFalse);
  }
#endif
  return eTrue;
}

static void GLES3_DoClear(iGraphicsDriver* apDrv, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
  sOpenGLDriver* d = ((sOpenGLDriver*)apDrv);
  sGLCache& cache = d->mCache;

  tU32 flags = 0;
  if (clearBuffer&eClearBuffersFlags_Color) {
    if (cache._colorWriteMask != eColorWriteMask_All) {
      _glColorMask(1,1,1,1);
    }
    _glClearColor(ULColorGetRf(anColor),
                  ULColorGetGf(anColor),
                  ULColorGetBf(anColor),
                  ULColorGetAf(anColor));
    flags |= GL_COLOR_BUFFER_BIT;
  }

  if (clearBuffer&eClearBuffersFlags_Depth) {
    if (!cache._depthMask) {
      _glDepthMask(GL_TRUE);
    }
    _glClearDepthf(afDepth);
    flags |= GL_DEPTH_BUFFER_BIT;
  }
#ifndef NO_STENCIL_BUFFER
  if (clearBuffer&eClearBuffersFlags_Stencil) {
    _glClearStencil(anStencil);
    flags |= GL_STENCIL_BUFFER_BIT;
  }
#endif

  if (flags) {
    _glClear(flags);
  }

  if (clearBuffer&eClearBuffersFlags_Depth) {
    if (!cache._depthMask) {
      _glDepthMask(GL_FALSE);
    }
  }
  if (clearBuffer&eClearBuffersFlags_Color) {
    if (cache._colorWriteMask != eColorWriteMask_All) {
      const tU32 m = cache._colorWriteMask;
      _glColorMask(niFlagTest(m,eColorWriteMask_Red),
                   niFlagTest(m,eColorWriteMask_Green),
                   niFlagTest(m,eColorWriteMask_Blue),
                   niFlagTest(m,eColorWriteMask_Alpha));
    }
  }
}

static void GLES3_ClearBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
  sOpenGLDriver* d = ((sOpenGLDriver*)apDrv);

  iGraphicsDrawOpCapture* pDrawOpCapture = d->mptrDOCapture;
  if (pDrawOpCapture) {
    if (!pDrawOpCapture->BeginCaptureDrawOp(
            apContext,NULL,
            Vec4<tI32>(clearBuffer,anColor,ftoul(afDepth),anStencil)))
      return;
  }

  GL_DEBUG_MARKER_GROUP(ClearBuffers);

  tBool isFlippedRT;
  if (!GL3_ApplyContext(d->mCache,apContext,isFlippedRT)) {
    niError(_A("Can't apply context."));
    return;
  }

  GLES3_DoClear(apDrv, clearBuffer, anColor, afDepth, anStencil);

  if (pDrawOpCapture) {
    pDrawOpCapture->EndCaptureDrawOp(
      apContext,NULL,
        Vec4<tI32>(clearBuffer,anColor,ftoul(afDepth),anStencil));
  }
}

static tBool GLES3_DrawOperation(iGraphicsDriver* apDrv, sGLContext* apContext, iDrawOperation* apDrawOp, const tU32 anAA)
{
  return ((sOpenGLDriver*)apDrv)->_DrawOperation(apContext,apDrawOp,anAA);
}

static tBool GLES3_ResetCache(iGraphicsDriver* apDrv) {
  ((sOpenGLDriver*)apDrv)->mCache.Reset();
#ifdef USE_GL_BIND_VAO
  if (hasBindVAO) {
    _glBindVertexArray(0);
  }
#endif
#ifdef USE_FBO
  if (hasFBO) {
#ifdef USE_FBO_MAINRT_IS_FBO
    if (fboMainRTHandle != eInvalidHandle) {
      _glBindFramebuffer(GL_FRAMEBUFFER, fboMainRTHandle);
    }
    if (fboMainDSHandle != eInvalidHandle) {
      _glBindRenderbuffer(GL_RENDERBUFFER, fboMainDSHandle);
    }
#else
    _glBindFramebuffer(GL_FRAMEBUFFER, 0);
    _glBindRenderbuffer(GL_RENDERBUFFER, 0);
#endif
  }
#endif
  _glBindBuffer(GL_ARRAY_BUFFER,0);
  _glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  _glUseProgram(0);
  niLoop(i,kGL3_MaxVertexAttrs) {
    GLCALL_WARN(_glDisableVertexAttribArray(i));
  }
  // Disable all texture units
  for (tI32 i = kGL3_MaxTU-1; i >= 0; --i) {
    _glActiveTexture(GL_TEXTURE0+i);
    _glBindTexture(GL_TEXTURE_2D,0);
    _glBindTexture(GL_TEXTURE_CUBE_MAP,0);
#if defined USE_GL_BIND_SAMPLER
    if (hasBindSampler) {
      _glBindSampler(i,0);
    }
#endif
#if defined USE_GL_ENABLE_FOR_TEXTURE
    _glDisable(GL_TEXTURE_2D);
    _glDisable(GL_TEXTURE_CUBE_MAP);
#endif
  }
  return eTrue;
}
static tBool GLES3_ResetContextDeviceResources(iGraphicsDriver* apDrv) {
  ((sOpenGLDriver*)apDrv)->_ResetContextDeviceResources();
  return eTrue;
}
static tBool GLES3_InitContextDeviceResources(iGraphicsDriver* apDrv) {
  ((sOpenGLDriver*)apDrv)->_InitContextDeviceResources();
  return eTrue;
}

static sGLCache& GLES3_GetCache(iGraphicsDriver* apDrv) {
  return ((sOpenGLDriver*)apDrv)->mCache;
}

niExportFunc(iUnknown*) New_GraphicsDriver_GL3(const Var& avarA, const Var&) {
  QPtr<iGraphics> ptrGraphics = avarA;
  niCheckIsOK(ptrGraphics,NULL);
  return (iGraphicsDriver*)niNew sOpenGLDriver(ptrGraphics);
}

#endif // GDRV_GL3
