// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#ifdef GDRV_GL2
#include "GDRV_GLES2.h"

iVertexArray* _CreateGenericVertexArray(tU32 anNumVertices, tFVF anFVF);
const cFVFDescription& _GetGenericVertexArrayFVFDesc(iVertexArray* apVA);
tPtr _GetGenericVertexArrayMemPtr(iVertexArray* apVA);
iIndexArray* _CreateGenericIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex);
tPtr _GetGenericIndexArrayMemPtr(iIndexArray* apVA);

static GLint samplerFilterAnisotropySmooth = 4;
static GLint samplerFilterAnisotropySharp = 8;

#include "Graphics.h"
#include "GDRV_Utils.h"
#include "API/niUI_ModuleDef.h"

#include <niLang/Math/MathMatrix.h>

#ifdef niAndroid
#include <niLang/Utils/JNIUtils.h>
#endif

#include "GDRV_StateCache.h"

#include "../../nicgc/src/mojoshader/mojoshader.h"
#include "FixedShaders.h"

// #define GL_DEBUG_MISSING_MIPMAPS 4

//--------------------------------------------------------------------------------------------
//
//  Cache
//
//--------------------------------------------------------------------------------------------
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
    _scissorRectIsFullScreen = ni::eFalse;
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
  tBool             _scissorRectIsFullScreen;
  eMaterialChannel  _tuChannel[GLDRV_MAX_TEXTURE_UNIT];
};

//--------------------------------------------------------------------------------------------
//
//  Context
//
//--------------------------------------------------------------------------------------------
struct sGLContext : public sGraphicsContext<1,cIUnknownImpl<iGraphicsContextRT,eIUnknownImplFlags_DontInherit1,iGraphicsContext> > {
  sGLContext(iGraphics* apGraphics)
      : tGraphicsContextBase(apGraphics)
  {
    mrectScissor = mrectViewport = sRecti::Null();
    mnSyncCounter = 0;
  }

#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
  virtual tsglContext* __stdcall GetTSGLContext() const = 0;
#endif

  ///////////////////////////////////////////////
  void __stdcall SetViewport(const sRecti& aVal) niImpl {
    mrectViewport = aVal;
    mnSyncCounter++;
  }
  sRecti __stdcall GetViewport() const niImpl {
    return mrectViewport;
  }

  ///////////////////////////////////////////////
  void __stdcall SetScissorRect(const sRecti& aVal) niImpl {
    mrectScissor = aVal;
    mnSyncCounter++;
  }
  sRecti __stdcall GetScissorRect() const niImpl {
    return mrectScissor;
  }

  tU32 mnSyncCounter;
  sRecti mrectScissor;
  sRecti mrectViewport;
};

static tU32 _kNumGL2TexUpload = 0;

class cGL2ContextWindow;
class cGL2ContextRT;

static void GLES2_DoClear(iGraphicsDriver* apDrv, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil, tBool abScissorTest);
static void GLES2_ClearBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil);
static tBool GLES2_DrawOperation(iGraphicsDriver* apDrv, sGLContext* apContext, iDrawOperation* apDrawOp, const tU32 anAA);
static tBool GLES2_SwapBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tBool abDoNotWait);
static tBool GLES2_ResetCache(iGraphicsDriver* apDrv);
static tBool GLES2_ResetContextDeviceResources(iGraphicsDriver* apDrv);
static tBool GLES2_InitContextDeviceResources(iGraphicsDriver* apDrv);
static sGLCache& GLES2_GetCache(iGraphicsDriver* apDrv);
static tBool GL2_ApplyMaterialChannel(
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
static eMOJOSHADERTextureLod hasTextureLod = eMOJOSHADERTextureLod_None;
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

#if defined niWindows || defined niOSX
#define USE_GL_GET_TEX_IMAGE
#endif

#if defined __GLES2__ && !defined USE_GLES3
#define _glReadBuffer(X) // NOOP
#else
#define _glReadBuffer(X) glReadBuffer(X)
#endif
#define _glReadPixels glReadPixels

// FBOs MUST be tested for each platform before being enabled
#if defined niWindows || defined niJSCC || defined niAndroid || defined niOSX || defined niIOS || defined niQNX
#define USE_FBO
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
static bool hasFBO = false;
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

#if defined niWindows && defined __GL1__
// GL_ARB_vertex_buffer_object, GL_ARB_pixel_buffer_object
static PFNGLGENBUFFERSARBPROC       _glGenBuffers       = NULL;
static PFNGLBINDBUFFERARBPROC       _glBindBuffer       = NULL;
static PFNGLMAPBUFFERARBPROC        _glMapBuffer        = NULL;
static PFNGLUNMAPBUFFERARBPROC      _glUnmapBuffer      = NULL;
static PFNGLBUFFERDATAARBPROC       _glBufferData       = NULL;
static PFNGLBUFFERSUBDATAARBPROC    _glBufferSubData    = NULL;
static PFNGLDELETEBUFFERSARBPROC    _glDeleteBuffers    = NULL;
static PFNGLGETBUFFERSUBDATAARBPROC _glGetBufferSubData = NULL;
// GL_ARB_multitexture
static PFNGLACTIVETEXTUREARBPROC       _glActiveTexture       = NULL;
// GL_ARB_occlusion_query
static PFNGLGENQUERIESARBPROC        _glGenQueries;
static PFNGLDELETEQUERIESARBPROC     _glDeleteQueries;
static PFNGLBEGINQUERYARBPROC        _glBeginQuery;
static PFNGLENDQUERYARBPROC          _glEndQuery;
static PFNGLGETQUERYIVARBPROC        _glGetQueryiv;
static PFNGLGETQUERYOBJECTIVARBPROC  _glGetQueryObjectiv;
static PFNGLGETQUERYOBJECTUIVARBPROC _glGetQueryObjectuiv;
// GL_ARB_shading_language_100, GL_ARB_shader_objects, GL_ARB_fragment_shader, GL_ARB_vertex_shader
static PFNGLCREATEPROGRAMOBJECTARBPROC       _glCreateProgramObject      = NULL;
static PFNGLDELETEOBJECTARBPROC              _glDeleteObject             = NULL;
static PFNGLUSEPROGRAMOBJECTARBPROC          _glUseProgramObject         = NULL;
static PFNGLCREATESHADEROBJECTARBPROC        _glCreateShaderObject       = NULL;
static PFNGLSHADERSOURCEARBPROC              _glShaderSource             = NULL;
static PFNGLCOMPILESHADERARBPROC             _glCompileShader            = NULL;
static PFNGLGETOBJECTPARAMETERIVARBPROC      _glGetObjectParameteriv     = NULL;
static PFNGLATTACHOBJECTARBPROC              _glAttachObject             = NULL;
static PFNGLGETINFOLOGARBPROC                _glGetInfoLog               = NULL;
static PFNGLLINKPROGRAMARBPROC               _glLinkProgram              = NULL;
static PFNGLGETUNIFORMLOCATIONARBPROC        _glGetUniformLocation       = NULL;
static PFNGLUNIFORM1FARBPROC                 _glUniform1f                = NULL;
static PFNGLUNIFORM2FARBPROC                 _glUniform2f                = NULL;
static PFNGLUNIFORM3FARBPROC                 _glUniform3f                = NULL;
static PFNGLUNIFORM4FARBPROC                 _glUniform4f                = NULL;
static PFNGLUNIFORM1FVARBPROC                _glUniform1fv               = NULL;
static PFNGLUNIFORM2FVARBPROC                _glUniform2fv               = NULL;
static PFNGLUNIFORM3FVARBPROC                _glUniform3fv               = NULL;
static PFNGLUNIFORM4FVARBPROC                _glUniform4fv               = NULL;
static PFNGLUNIFORMMATRIX4FVARBPROC          _glUniformMatrix4fv         = NULL;
static PFNGLUNIFORM1IARBPROC                 _glUniform1i                = NULL;
static PFNGLBINDATTRIBLOCATIONARBPROC        _glBindAttribLocation       = NULL;
static PFNGLGETACTIVEUNIFORMARBPROC          _glGetActiveUniform         = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYARBPROC   _glEnableVertexAttribArray   = NULL;
static PFNGLDISABLEVERTEXATTRIBARRAYARBPROC  _glDisableVertexAttribArray  = NULL;
static PFNGLVERTEXATTRIBPOINTERARBPROC       _glVertexAttribPointer       = NULL;
static PFNGLGETSHADERINFOLOGPROC             _glGetShaderInfoLog          = NULL;
static PFNGLGETPROGRAMIVPROC                 _glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC            _glGetProgramInfoLog = NULL;

#define _glCreateShader  _glCreateShaderObject
#define _glDeleteShader  _glDeleteObject
#define _glCreateProgram _glCreateProgramObject
#define _glAttachShader  _glAttachObject
#define _glUseProgram    _glUseProgramObject
#define _glDeleteProgram _glDeleteObject

static void *getprocaddress(const char *name) {
  return wglGetProcAddress(name);
}
#endif

static GLint kGL2_MaxTU = GLDRV_MAX_TEXTURE_UNIT;
static GLint kGL2_MaxCubeTexSize = 8192;
static GLint kGL2_MaxRegularTexSize = 8192;
static GLint kGL2_MaxOverlayTexSize = 8192;
static GLint kGL2_MaxVertexAttrs = 16;
static GLint kGL2_MaxVertexUniforms = 0;
static GLint kGL2_MaxPixelUniforms = 0;

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

#ifdef niWindows
#define GL_FBOAPI WINAPI
#else
#define GL_FBOAPI
#endif

#if defined niOSX || defined niIOS || defined niJSCC || defined niAndroid
#define GL_FBO_DECLAPI(RET,NAME,PARAMS)         \
  typedef RET (GL_FBOAPI * tpfn_##NAME) PARAMS; \
  tpfn_##NAME _##NAME = NAME;
#elif defined niQNX
#define GL_FBO_DECLAPI(RET,NAME,PARAMS)                                 \
  typedef RET (GL_FBOAPI * tpfn_##NAME) PARAMS __attribute__((pcs("aapcs"))); \
  tpfn_##NAME _##NAME = NAME;
#elif defined niWindows
#define GL_FBO_DECLAPI(RET,NAME,PARAMS)         \
  typedef RET (GL_FBOAPI * tpfn_##NAME) PARAMS; \
  tpfn_##NAME _##NAME = NULL;
#else
#error "Unknown GL_FBO_DECLAPI for this platform."
#endif

GL_FBO_DECLAPI(GLboolean, glIsRenderbuffer, (GLuint renderbuffer));
GL_FBO_DECLAPI(void, glBindRenderbuffer, (GLenum target, GLuint renderbuffer));
GL_FBO_DECLAPI(void, glDeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers));
GL_FBO_DECLAPI(void, glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers));
GL_FBO_DECLAPI(void, glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height));
GL_FBO_DECLAPI(void, glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint *params));

GL_FBO_DECLAPI(GLboolean, glIsFramebuffer, (GLuint framebuffer));
GL_FBO_DECLAPI(void, glBindFramebuffer, (GLenum target, GLuint framebuffer));
GL_FBO_DECLAPI(void, glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers));
GL_FBO_DECLAPI(void, glGenFramebuffers, (GLsizei n, GLuint *framebuffers));
GL_FBO_DECLAPI(GLenum, glCheckFramebufferStatus, (GLenum target));

GL_FBO_DECLAPI(void, glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));

GL_FBO_DECLAPI(void, glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer));
GL_FBO_DECLAPI(void, glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint *params));
GL_FBO_DECLAPI(void, glGenerateMipmap, (GLenum target));

#if 0 // don't enable Desktop only extensions...
#if !defined __GLES2__
GL_FBO_DECLAPI(void, glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter));
GL_FBO_DECLAPI(void, glFramebufferTexture1D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));
GL_FBO_DECLAPI(void, glFramebufferTexture3D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset));
GL_FBO_DECLAPI(void, glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height));
GL_FBO_DECLAPI(void, glFramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer));
#endif
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

static tBool GL2_InitializeExt() {
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
    niDebugFmt((_A("--- GL2 Context Infos ---")));
    niDebugFmt((_A("GL_VENDOR: %s"),strVendor));
    niDebugFmt((_A("GL_RENDERER: %s"),strRenderer));
    niDebugFmt((_A("GL_VERSION: %s"),strVersion));
    niDebugFmt((_A("GL_EXTENSIONS: %s"),strExt));
    niDebugFmt(("--- GL2_CAPS ---"));
  }

  // max texture units
  {
    GLint maxTU = kGL2_MaxTU;
    _glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&maxTU);
    kGL2_MaxTU = ni::Min(maxTU,kGL2_MaxTU,GLDRV_MAX_TEXTURE_UNIT);
  }
  // shader caps
  {
    _glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&kGL2_MaxVertexAttrs);
#if defined __GL1__ || defined niOSX
    _glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB,&kGL2_MaxVertexUniforms);
    kGL2_MaxVertexUniforms /= 4;
    _glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB,&kGL2_MaxPixelUniforms);
    kGL2_MaxPixelUniforms /= 4;
#else
    _glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,&kGL2_MaxVertexUniforms);
    _glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS,&kGL2_MaxPixelUniforms);
#endif
  }
  // max texture size
  {
    GLint maxTexSize = 0;
    if (ni::GetLang()->HasProperty("GL2.GL_MAX_TEXTURE_SIZE")) {
      maxTexSize = ni::GetLang()->GetProperty("GL2.GL_MAX_TEXTURE_SIZE").Long();
    }
    if (maxTexSize < 4) {
      maxTexSize = kGL2_MaxRegularTexSize;
      _glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexSize);
    }
    if (ni::GetLang()->HasProperty("GL2.MaxRegularTexSize")) {
      kGL2_MaxRegularTexSize = ni::GetLang()->GetProperty("GL2.MaxRegularTexSize").Long();
    }
    else {
      kGL2_MaxRegularTexSize = ni::Min(maxTexSize,kGL2_MaxRegularTexSize);
    }
    if (ni::GetLang()->HasProperty("GL2.MaxOverlayTexSize")) {
      kGL2_MaxOverlayTexSize = ni::GetLang()->GetProperty("GL2.MaxOverlayTexSize").Long();
    }
    else {
      kGL2_MaxOverlayTexSize = ni::Min(maxTexSize,kGL2_MaxOverlayTexSize);
    }
    if (ni::GetLang()->HasProperty("GL2.MaxCubeTexSize")) {
      kGL2_MaxCubeTexSize = ni::GetLang()->GetProperty("GL2.MaxCubeTexSize").Long();
    }
    else {
      kGL2_MaxCubeTexSize = ni::Min(maxTexSize,kGL2_MaxCubeTexSize);
    }
  }
  // non power of 2 texture support
  {
#ifdef __GLES2__
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
#ifdef __GLES2__
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

  // textureLod in pixel shader
  {
    if (ni::GetLang()->HasProperty("GL2.hasTextureLod")) {
      hasTextureLod = (eMOJOSHADERTextureLod)ni::GetLang()->GetProperty("GL2.hasTextureLod").Long();
    }
    else {
      if (strExt.contains("GL_ARB_shader_texture_lod")) {
        hasTextureLod = eMOJOSHADERTextureLod_ARB;
      }
      else if (strExt.contains("GL_EXT_shader_texture_lod")) {
        hasTextureLod = eMOJOSHADERTextureLod_EXT;
      }
    }
    MOJOSHADER_GLSL_SetTextureLod(hasTextureLod);
  }

  // standard derivatives
  {
#ifdef niEmbedded
    hasStandardDerivatives = strExt.contains("OES_standard_derivatives");
#else
    if (ni::GetLang()->HasProperty("GL2.hasStandardDerivatives")) {
      hasStandardDerivatives = !!ni::GetLang()->GetProperty("GL2.hasStandardDerivatives").Bool(hasStandardDerivatives);
    }
    else {
      hasStandardDerivatives = true;
    }
#endif
    MOJOSHADER_GLSL_SetStandardDerivatives(hasStandardDerivatives);
  }

#ifdef USE_OQ
  if (strExt.contains("GL_ARB_occlusion_query"))
  {
    GLint bits;
    _glGetQueryiv = (PFNGLGETQUERYIVARBPROC)getprocaddress("glGetQueryivARB");
    if (_glGetQueryiv) {
      _glGetQueryiv(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bits);
      if (bits) {
        _glGenQueries =        (PFNGLGENQUERIESARBPROC)       getprocaddress("glGenQueriesARB");
        _glDeleteQueries =     (PFNGLDELETEQUERIESARBPROC)    getprocaddress("glDeleteQueriesARB");
        _glBeginQuery =        (PFNGLBEGINQUERYARBPROC)       getprocaddress("glBeginQueryARB");
        _glEndQuery =          (PFNGLENDQUERYARBPROC)         getprocaddress("glEndQueryARB");
        _glGetQueryObjectiv =  (PFNGLGETQUERYOBJECTIVARBPROC) getprocaddress("glGetQueryObjectivARB");
        _glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVARBPROC)getprocaddress("glGetQueryObjectuivARB");
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
        if (ni::GetLang()->HasProperty("GL2.samplerFilterAnisotropySmooth")) {
          samplerFilterAnisotropySmooth = ni::GetLang()->GetProperty("GL2.samplerFilterAnisotropySmooth").Long();
        }
        if (ni::GetLang()->HasProperty("GL2.samplerFilterAnisotropySharp")) {
          samplerFilterAnisotropySharp = ni::GetLang()->GetProperty("GL2.samplerFilterAnisotropySharp").Long();
        }
      }
    }
    samplerFilterAnisotropySmooth = ni::Min(samplerFilterAnisotropySmooth, maxAnisotropic);
    samplerFilterAnisotropySharp = ni::Min(samplerFilterAnisotropySharp, maxAnisotropic);
  }

#if !defined __GLES2__
  {
#define GET_REQUIRED_GL_PROC(PROTO,FUNC)                              \
    _##FUNC = (PROTO)getprocaddress(#FUNC);                           \
    if (!_##FUNC) {                                                   \
      _##FUNC = (PROTO)getprocaddress(#FUNC "ARB");                   \
      if (!_##FUNC) {                                                 \
        niError(niFmt("Can't get required GL function '%s'",#FUNC));  \
        return eFalse;                                                \
      }                                                               \
    }

    // GL_ARB_vertex_buffer_object, GL_ARB_pixel_buffer_object
    GET_REQUIRED_GL_PROC(PFNGLGENBUFFERSARBPROC,glGenBuffers);
    GET_REQUIRED_GL_PROC(PFNGLBINDBUFFERARBPROC,glBindBuffer);
    GET_REQUIRED_GL_PROC(PFNGLMAPBUFFERARBPROC,glMapBuffer);
    GET_REQUIRED_GL_PROC(PFNGLUNMAPBUFFERARBPROC,glUnmapBuffer);
    GET_REQUIRED_GL_PROC(PFNGLBUFFERDATAARBPROC,glBufferData);
    GET_REQUIRED_GL_PROC(PFNGLBUFFERSUBDATAARBPROC,glBufferSubData);
    GET_REQUIRED_GL_PROC(PFNGLDELETEBUFFERSARBPROC,glDeleteBuffers);
    GET_REQUIRED_GL_PROC(PFNGLGETBUFFERSUBDATAARBPROC,glGetBufferSubData);

    // GL_ARB_multitexture
    GET_REQUIRED_GL_PROC(PFNGLACTIVETEXTUREARBPROC,glActiveTexture);

    // GL_ARB_shading_language_100, GL_ARB_shader_objects,
    // GL_ARB_fragment_shader, GL_ARB_vertex_shader
    GET_REQUIRED_GL_PROC(PFNGLCREATEPROGRAMOBJECTARBPROC,glCreateProgramObject);
    GET_REQUIRED_GL_PROC(PFNGLDELETEOBJECTARBPROC,glDeleteObject);
    GET_REQUIRED_GL_PROC(PFNGLUSEPROGRAMOBJECTARBPROC,glUseProgramObject);
    GET_REQUIRED_GL_PROC(PFNGLCREATESHADEROBJECTARBPROC,glCreateShaderObject);
    GET_REQUIRED_GL_PROC(PFNGLSHADERSOURCEARBPROC,glShaderSource);
    GET_REQUIRED_GL_PROC(PFNGLCOMPILESHADERARBPROC,glCompileShader);
    GET_REQUIRED_GL_PROC(PFNGLGETOBJECTPARAMETERIVARBPROC,glGetObjectParameteriv);
    GET_REQUIRED_GL_PROC(PFNGLATTACHOBJECTARBPROC,glAttachObject);
    GET_REQUIRED_GL_PROC(PFNGLGETINFOLOGARBPROC,glGetInfoLog);
    GET_REQUIRED_GL_PROC(PFNGLLINKPROGRAMARBPROC,glLinkProgram);
    GET_REQUIRED_GL_PROC(PFNGLGETUNIFORMLOCATIONARBPROC,glGetUniformLocation);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM1FARBPROC,glUniform1f);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM2FARBPROC,glUniform2f);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM3FARBPROC,glUniform3f);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM4FARBPROC,glUniform4f);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM1FVARBPROC,glUniform1fv);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM2FVARBPROC,glUniform2fv);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM3FVARBPROC,glUniform3fv);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM4FVARBPROC,glUniform4fv);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORMMATRIX4FVARBPROC,glUniformMatrix4fv);
    GET_REQUIRED_GL_PROC(PFNGLUNIFORM1IARBPROC,glUniform1i);
    GET_REQUIRED_GL_PROC(PFNGLBINDATTRIBLOCATIONARBPROC,glBindAttribLocation);
    GET_REQUIRED_GL_PROC(PFNGLGETACTIVEUNIFORMARBPROC,glGetActiveUniform);
    GET_REQUIRED_GL_PROC(PFNGLENABLEVERTEXATTRIBARRAYARBPROC,glEnableVertexAttribArray);
    GET_REQUIRED_GL_PROC(PFNGLDISABLEVERTEXATTRIBARRAYARBPROC,glDisableVertexAttribArray);
    GET_REQUIRED_GL_PROC(PFNGLVERTEXATTRIBPOINTERARBPROC,glVertexAttribPointer);
    GET_REQUIRED_GL_PROC(PFNGLGETSHADERINFOLOGPROC,glGetShaderInfoLog);
    GET_REQUIRED_GL_PROC(PFNGLGETPROGRAMIVPROC,glGetProgramiv);
    GET_REQUIRED_GL_PROC(PFNGLGETPROGRAMINFOLOGPROC,glGetProgramInfoLog);
  }
#endif

  if (ni::GetLang()->HasProperty("GL2.hasContextLost")) {
    hasContextLost = !!ni::GetLang()->GetProperty("GL2.hasContextLost").Bool(hasContextLost);
  }

#ifdef USE_FBO
  {
#if defined niOSX || defined niIOS || defined niJSCC || defined niAndroid || defined niQNX
    hasFBO = true;
#elif defined niWindows
    hasFBO = false;
#define LOAD_FBO_API(TYPE,NAME)                                         \
    if (hasFBO) {                                                       \
      _##NAME = (tpfn_##NAME)getprocaddress(#NAME #TYPE);               \
      hasFBO = (_##NAME != NULL);                                       \
      if (!hasFBO) { niWarning("Couldn't get FBO function: " #NAME); }  \
    }

    if (!hasFBO && strExt.contains("GL_EXT_framebuffer_object")) {
      hasFBO = true;
      LOAD_FBO_API(EXT,glIsRenderbuffer);
      LOAD_FBO_API(EXT,glBindRenderbuffer);
      LOAD_FBO_API(EXT,glDeleteRenderbuffers);
      LOAD_FBO_API(EXT,glGenRenderbuffers);
      LOAD_FBO_API(EXT,glRenderbufferStorage);
      LOAD_FBO_API(EXT,glGetRenderbufferParameteriv);
      LOAD_FBO_API(EXT,glIsFramebuffer);
      LOAD_FBO_API(EXT,glBindFramebuffer);
      LOAD_FBO_API(EXT,glDeleteFramebuffers);
      LOAD_FBO_API(EXT,glGenFramebuffers);
      LOAD_FBO_API(EXT,glCheckFramebufferStatus);
      LOAD_FBO_API(EXT,glFramebufferTexture2D);
      LOAD_FBO_API(EXT,glFramebufferRenderbuffer);
      LOAD_FBO_API(EXT,glGetFramebufferAttachmentParameteriv);
      LOAD_FBO_API(EXT,glGenerateMipmap);
      if (_printedInfos) {
        niDebugFmt(("GL_FBO: GL_EXT_framebuffer_object"));
      }
    }
#else
    #error "GL2 FBO not supported on this platform."
#endif
  }
#endif

  if (_printedInfos) {
#if GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_SYSTEM_MEMORY
    niDebugFmt(("GL2 GL_DYNAMIC_BUFFER_MODE: SYSTEM_MEMORY"));
#elif GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING
    niDebugFmt(("GL2 GL_DYNAMIC_BUFFER_MODE: ORPHANING"));
#elif GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_NONE
    niDebugFmt(("GL2 GL_DYNAMIC_BUFFER_MODE: NONE"));
#else
    #error "No GL_DYNAMIC_BUFFER_MODE defined."
#endif

    niDebugFmt(("GL2 hasContextLost: %d",hasContextLost));
    niDebugFmt(("GL2 hasPartialNP2: %d",hasPartialNP2));
    niDebugFmt(("GL2 hasCubeMap: %d",hasCubeMap));
    niDebugFmt(("GL2 hasElementUInt: %d",hasElementUInt));
    niDebugFmt(("GL2 hasTexFmtHalfFloat: %d",hasTexFmtHalfFloat));
    niDebugFmt(("GL2 hasTexFmtFloat: %d",hasTexFmtFloat));
    niDebugFmt(("GL2 hasTexFmtDepth: %d",hasTexFmtDepth));
#ifdef GL_TEXTURE_WRAP_R
    niDebugFmt(("GL2 hasWrapR: %d",hasWrapR));
#else
    niDebugFmt(("GL2 NO WarpR at compile time"));
#endif
#ifdef USE_GL_BIND_VAO
    niDebugFmt(("GL2 hasBindVAO: %d",hasBindVAO));
#else
    niDebugFmt(("GL2 NO VAO at compile time"));
#endif
#ifdef USE_GL_BIND_SAMPLER
    niDebugFmt(("GL2 hasBindSampler: %d",hasBindSampler));
#else
    niDebugFmt(("GL2 NO Sampler at compile time"));
#endif
#ifdef USE_FBO
    niDebugFmt(("GL2 hasFBO: %d",hasFBO));
#else
    niDebugFmt(("GL2 NO FBO at compile time"));
#endif
#ifdef USE_OQ
    niDebugFmt(("GL2 hasOQ: %d",hasOQ));
#else
    niDebugFmt(("GL2 NO OQ at compile time"));
#endif
    niDebugFmt(("GL2 hasStandardDerivatives: %d",hasStandardDerivatives));
    niDebugFmt(("GL2 hasTextureLod: %d",hasTextureLod));
    niDebugFmt(("GL2 samplerFilterAnisotropySmooth: %d",samplerFilterAnisotropySmooth));
    niDebugFmt(("GL2 samplerFilterAnisotropySharp: %d",samplerFilterAnisotropySharp));
    niDebugFmt(("GL_MAX_TEXTURE_SIZE (Regular): %d",kGL2_MaxRegularTexSize));
    niDebugFmt(("GL_MAX_TEXTURE_SIZE (Overlay): %d",kGL2_MaxOverlayTexSize));
    niDebugFmt(("GL_MAX_TEXTURE_SIZE (CubeMap): %d",kGL2_MaxCubeTexSize));
    niDebugFmt(("GL_MAX_TEXTURE_UNITS: %d",kGL2_MaxTU));
    niDebugFmt(("GL_MAX_VERTEX_ATTRIBS: %d",kGL2_MaxVertexAttrs));
    niDebugFmt(("GL_MAX_VERTEX_UNIFORM_VECTORS: %d",kGL2_MaxVertexUniforms));
    niDebugFmt(("GL_MAX_FRAGMENT_UNIFORM_VECTORS: %d",kGL2_MaxPixelUniforms));
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
  }
  return 0;
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
    case eSamplerFilter_SharpPoint:
      magfilter = GL_NEAREST;
      minfilter = anNumMipMaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
      break;
    case eSamplerFilter_Smooth:
    case eSamplerFilter_Sharp:
      magfilter = GL_LINEAR;
      minfilter = anNumMipMaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
      break;
  }
  if (anNumMipMaps) {
    if (aFilter == eSamplerFilter_Sharp || aFilter == eSamplerFilter_SharpPoint) {
      _glTexParameteri(texType, GLEXT_TEXTURE_MAX_ANISOTROPY, samplerFilterAnisotropySharp);
    }
    else {
      _glTexParameteri(texType, GLEXT_TEXTURE_MAX_ANISOTROPY, samplerFilterAnisotropySmooth);
    }
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
  // Scissor //
  // if (0)
  {
    aCache._scissorTest = v.mbScissorTest && !aCache._scissorRectIsFullScreen;
    if (aCache._scissorTest) _glEnable(GL_SCISSOR_TEST);
    else _glDisable(GL_SCISSOR_TEST);
    GLERR_RET(;);
  }
  // TODO: Depth Bias ? //
}

//--------------------------------------------------------------------------------------------
//
//  MojoShaders
//
//--------------------------------------------------------------------------------------------

struct iMojoShader : public iShader {
  niDeclareInterfaceUUID(iMojoShader,0x27897b00,0x0c78,0x4815,0x92,0x2e,0x17,0x38,0x5f,0xef,0x23,0x97);
  virtual MOJOSHADER_glShader* __stdcall GetShader() const = 0;
};

#if !defined __JSCC__
static void* mojo_getprocaddress(const char *name, void*) {
#if defined niWindows
  if (strcmp(name,"glGetString") == 0) return (void*)_glGetString;
  if (strcmp(name,"glGetError") == 0) return (void*)_glGetError;
  if (strcmp(name,"glGetIntegerv") == 0) return (void*)_glGetIntegerv;
  if (strcmp(name,"glEnable") == 0) return (void*)_glEnable;
  if (strcmp(name,"glDisable") == 0) return (void*)_glDisable;
  if (strcmp(name,"glFlush") == 0) return (void*)_glFlush;

  return (void*)getprocaddress(name);
#elif defined NO_TSGL
  if (strcmp(name,"glGetString") == 0) return (void*)_glGetString;
  if (strcmp(name,"glGetError") == 0) return (void*)_glGetError;
  if (strcmp(name,"glGetIntegerv") == 0) return (void*)_glGetIntegerv;
  if (strcmp(name,"glEnable") == 0) return (void*)_glEnable;
  if (strcmp(name,"glDisable") == 0) return (void*)_glDisable;
  if (strcmp(name,"glFlush") == 0) return (void*)_glFlush;

#if !defined NO_STENCIL_BUFFER
  if (strcmp(name,"glStencilFunc") == 0) return (void*)_glStencilFunc;
  if (strcmp(name,"glStencilOp") == 0) return (void*)_glStencilOp;
  if (strcmp(name,"glClearStencil") == 0) return (void*)_glClearStencil;
#endif

  if (strcmp(name,"glFinish") == 0) return (void*)_glFinish;
  if (strcmp(name,"glViewport") == 0) return (void*)_glViewport;
  if (strcmp(name,"glScissor") == 0) return (void*)_glScissor;
  if (strcmp(name,"glTexParameteri") == 0) return (void*)_glTexParameteri;
  if (strcmp(name,"glBlendFunc") == 0) return (void*)_glBlendFunc;
  if (strcmp(name,"glDepthMask") == 0) return (void*)_glDepthMask;
  if (strcmp(name,"glDepthFunc") == 0) return (void*)_glDepthFunc;
  if (strcmp(name,"glCullFace") == 0) return (void*)_glCullFace;
  if (strcmp(name,"glFrontFace") == 0) return (void*)_glFrontFace;
  if (strcmp(name,"glColorMask") == 0) return (void*)_glColorMask;
  if (strcmp(name,"glClear") == 0) return (void*)_glClear;
  if (strcmp(name,"glClearColor") == 0) return (void*)_glClearColor;
  if (strcmp(name,"glClearDepthf") == 0) return (void*)_glClearDepthf;
  if (strcmp(name,"glDeleteTextures") == 0) return (void*)_glDeleteTextures;
  if (strcmp(name,"glGenTextures") == 0) return (void*)_glGenTextures;
  if (strcmp(name,"glBindTexture") == 0) return (void*)_glBindTexture;
  if (strcmp(name,"glTexImage2D") == 0) return (void*)_glTexImage2D;
  if (strcmp(name,"glActiveTexture") == 0) return (void*)_glActiveTexture;
  if (strcmp(name,"glBindAttribLocation") == 0) return (void*)_glBindAttribLocation;
  if (strcmp(name,"glShaderSource") == 0) return (void*)_glShaderSource;
  if (strcmp(name,"glCompileShader") == 0) return (void*)_glCompileShader;
  if (strcmp(name,"glGetShaderInfoLog") == 0) return (void*)_glGetShaderInfoLog;
  if (strcmp(name,"glLinkProgram") == 0) return (void*)_glLinkProgram;
  if (strcmp(name,"glGetProgramiv") == 0) return (void*)_glGetProgramiv;
  if (strcmp(name,"glGetProgramInfoLog") == 0) return (void*)_glGetProgramInfoLog;
  if (strcmp(name,"glCreateShader") == 0) return (void*)_glCreateShader;
  if (strcmp(name,"glDeleteShader") == 0) return (void*)_glDeleteShader;
  if (strcmp(name,"glGetShaderiv") == 0) return (void*)_glGetShaderiv;
  if (strcmp(name,"glUseProgram") == 0) return (void*)_glUseProgram;
  if (strcmp(name,"glGetAttribLocation") == 0) return (void*)_glGetAttribLocation;
  if (strcmp(name,"glGetUniformLocation") == 0) return (void*)_glGetUniformLocation;
  if (strcmp(name,"glUniformMatrix4fv") == 0) return (void*)_glUniformMatrix4fv;
  if (strcmp(name,"glUniform4fv") == 0) return (void*)_glUniform4fv;
  if (strcmp(name,"glUniform1i") == 0) return (void*)_glUniform1i;
  if (strcmp(name,"glUniform1iv") == 0) return (void*)_glUniform1i;
  if (strcmp(name,"glUniform4iv") == 0) return (void*)_glUniform4iv;
  if (strcmp(name,"glUniform1f") == 0) return (void*)_glUniform1f;
  if (strcmp(name,"glCreateProgram") == 0) return (void*)_glCreateProgram;
  if (strcmp(name,"glAttachShader") == 0) return (void*)_glAttachShader;
  if (strcmp(name,"glDeleteProgram") == 0) return (void*)_glDeleteProgram;
  if (strcmp(name,"glGenBuffers") == 0) return (void*)_glGenBuffers;
  if (strcmp(name,"glBindBuffer") == 0) return (void*)_glBindBuffer;
  if (strcmp(name,"glDeleteBuffers") == 0) return (void*)_glDeleteBuffers;
  if (strcmp(name,"glBufferData") == 0) return (void*)_glBufferData;
  if (strcmp(name,"glEnableVertexAttribArray") == 0) return (void*)_glEnableVertexAttribArray;
  if (strcmp(name,"glDisableVertexAttribArray") == 0) return (void*)_glDisableVertexAttribArray;
  if (strcmp(name,"glVertexAttribPointer") == 0) return (void*)_glVertexAttribPointer;
  if (strcmp(name,"glDrawElements") == 0) return (void*)_glDrawElements;
  if (strcmp(name,"glDrawArrays") == 0) return (void*)_glDrawArrays;
  niDebugFmt(("# Can't find entry point '%s'", name));
  return NULL;
#else
  return tsglGL2GetProc(name);
#endif
}
#endif

template <typename T>
class cMojoShaderBase : public T, public sShaderDesc
{
 public:
  typedef cMojoShaderBase tMojoShaderBase;

  cMojoShaderBase(iGraphics* apGraphics, iHString* ahspName, iDeviceResourceManager* apDevResMan, iHString* ahspProfile, MOJOSHADER_glShader* apMojoShader, const astl::vector<tU8>& avData)
      : mvData(avData)
  {
    mpGraphics = apGraphics;
    mhspName = ahspName;
    mhspProfile = ahspProfile;
    mpShader = apMojoShader;
    mptrDevResMan = apDevResMan;
    mptrDevResMan->Register(this);
  }

  ~cMojoShaderBase() {
    Invalidate();
  }

  iHString* __stdcall GetDeviceResourceName() const
  {
    return mhspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    _DeleteShader();
    return eFalse;
  }

  virtual void __stdcall Invalidate()
  {
    mptrConstants = NULL;
    if (!mptrDevResMan.IsOK()) return;
    mptrDevResMan->Unregister(this);
    mptrDevResMan = NULL;
    _DeleteShader();
  }

  void _DeleteShader() {
    if (mpShader) {
      MOJOSHADER_glDeleteShader(mpShader);
      mpShader = NULL;
    }
  }

  iHString* __stdcall GetProfile() const
  {
    return mhspProfile;
  }

  iShaderConstants* __stdcall GetConstants() const
  {
    if (!mptrConstants.IsOK()) {
      niThis(cMojoShaderBase)->mptrConstants = mpGraphics->CreateShaderConstants(4096);
    }
    return mptrConstants;
  }

  MOJOSHADER_glShader* __stdcall GetShader() const {
    return mpShader;
  }

  tBool __stdcall GetHasConstants() const {
    return mptrConstants.IsOK();
  }

  tPtr __stdcall GetDescStructPtr() const {
    const sShaderDesc* d = this;
    return (tPtr)d;
  }

  iHString* __stdcall GetCode() const {
    return NULL;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    if (!mpShader) {
      if (mvData.empty())
        return NULL;

      // Shader is compiled here the first time because OpenGL really
      // doesn't want to do multithreading so shader compilation will
      // fail on PC since we'll usually compile shaders in a worker
      // thread.
      mpShader = MOJOSHADER_glCompileShader(
          mvData.data(),(tU32)mvData.size(),NULL,0);
      GL_SYNC_COMPILE_SHADER();
      if (!mpShader) {
        GLERR_WARN();
        niError(niFmt("Can't re-compile shader '%s': '%s'.",
                      mhspName,MOJOSHADER_glGetError()));
        mvData.clear();
        return NULL;
      }
    }
    return this;
  }

 private:
  iGraphics*      mpGraphics;
  Ptr<iDeviceResourceManager>   mptrDevResMan;
  MOJOSHADER_glShader* mpShader;
  astl::vector<tU8> mvData;
};

// Vertex program
class cMojoShaderVertex : public cMojoShaderBase<
  cIUnknownImpl<iMojoShader,
                eIUnknownImplFlags_DontInherit1|
                eIUnknownImplFlags_DontInherit2,
                iShader,iDeviceResource> >
{
 public:
  cMojoShaderVertex(iGraphics* apGraphics, iHString* ahspName, iDeviceResourceManager* apDevResMan, iHString* ahspProfile, MOJOSHADER_glShader* apShader, const astl::vector<tU8>& avData)
      : tMojoShaderBase(apGraphics,ahspName,apDevResMan,ahspProfile,apShader,avData)
  {
  }
  ~cMojoShaderVertex() {
  }
  eShaderUnit __stdcall GetUnit() const {
    return eShaderUnit_Vertex;
  }
};

// Pixel program
class cMojoShaderPixel : public cMojoShaderBase<
  cIUnknownImpl<iMojoShader,
                eIUnknownImplFlags_DontInherit1|
                eIUnknownImplFlags_DontInherit2,
                iShader,iDeviceResource> >
{
 public:
  cMojoShaderPixel(iGraphics* apGraphics, iHString* ahspName, iDeviceResourceManager* apDevResMan, iHString* ahspProfile, MOJOSHADER_glShader* apShader, const astl::vector<tU8>& avData)
      : tMojoShaderBase(apGraphics,ahspName,apDevResMan,ahspProfile,apShader,avData)
  {
  }
  ~cMojoShaderPixel() {
  }
  eShaderUnit __stdcall GetUnit() const {
    return eShaderUnit_Pixel;
  }
};

struct cMojoShaderNativeWrapper : public cIUnknownImpl<iGLShader,eIUnknownImplFlags_Local>
{
  virtual const tI32* __stdcall GetVertexAttributeLocationArray() const niImpl {
    return MOJOSHADER_glGetVertexAttribLocationArray(MOJOSHADER_USAGE_TEXCOORD);
  }
  virtual tBool __stdcall BeforeDraw() niImpl {
    MOJOSHADER_glPushUniforms();
    GLERR_RET(eFalse);
    return eTrue;
  }
  virtual void __stdcall AfterDraw() niImpl {
  }
};
static cMojoShaderNativeWrapper _mojoShaderNative;

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstFloat_(eShaderUnit aUnit, tU32 ulConst, const sVec4f* pV, tU32 ulNum)
{
  niAssert(aUnit == eShaderUnit_Pixel  || aUnit == eShaderUnit_Vertex);
  niAssert(ulConst >= 0 && ulConst < 256);
  if (aUnit == eShaderUnit_Pixel)
    MOJOSHADER_glSetPixelShaderUniformF(ulConst, pV->ptr(), ulNum);
  else
    MOJOSHADER_glSetVertexShaderUniformF(ulConst, pV->ptr(), ulNum);
  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstFloatMatrix_(
    eShaderUnit aUnit, tU32 ulConst,
    const sMatrixf* pMtx, tU32 ulNum,
    const sMatrixf& aMatrixOffset)
{
  for (tU32 i = 0; i < ulNum; ++i) {
    sMatrixf m = *pMtx * aMatrixOffset;
    Mojo_SetConstFloat_(aUnit, ulConst+(i*4), (sVec4f*)&m._11, 4);
    ++pMtx;
  }
  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstInt_(eShaderUnit aUnit, tU32 ulConst, const sVec4i* pV, tU32 ulNum)
{
  niAssert(aUnit == eShaderUnit_Pixel  || aUnit == eShaderUnit_Vertex);
  niAssert(ulConst >= 0 && ulConst < 256);
  if (aUnit == eShaderUnit_Pixel)
    MOJOSHADER_glSetPixelShaderUniformI(ulConst, (const int*)pV->ptr(), ulNum);
  else
    MOJOSHADER_glSetVertexShaderUniformI(ulConst, (const int*)pV->ptr(), ulNum);
  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstBool_(eShaderUnit aUnit, tU32 ulConst, const tBool* pV, tU32 ulNum)
{
  niAssert(aUnit == eShaderUnit_Pixel  || aUnit == eShaderUnit_Vertex);
  niAssert(ulConst >= 0 && ulConst < 256);
  if (aUnit == eShaderUnit_Pixel) {
    int tmp;
    for (tU32 i = 0; i < ulNum; ++i) {
      tmp = *pV++;
      MOJOSHADER_glSetPixelShaderUniformB(ulConst+i, &tmp, 1);
    }
  }
  else {
    int tmp;
    for (tU32 i = 0; i < ulNum; ++i) {
      tmp = *pV++;
      MOJOSHADER_glSetVertexShaderUniformB(ulConst+i, &tmp, 1);
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstFloatArray(eShaderUnit aUnit, tU32 ulConst, const tVec4fCVec* apV, tU32 ulNum)
{
  return Mojo_SetConstFloat_(aUnit,ulConst,&(*apV)[0],ulNum);
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstFloat(eShaderUnit aUnit, tU32 ulConst, const sVec4f& aV)
{
  return Mojo_SetConstFloat_(aUnit,ulConst,&aV,1);
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstFloatMatrixArray(
    eShaderUnit aUnit, tU32 ulConst,
    const tMatrixfCVec* apV, tU32 ulNum,
    const sMatrixf& aMatrixOffset)
{
  return Mojo_SetConstFloatMatrix_(aUnit,ulConst,&(*apV)[0],ulNum,aMatrixOffset);
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstFloatMatrix(
    eShaderUnit aUnit, tU32 ulConst, const sMatrixf& aV,
    const sMatrixf& aMatrixOffset)
{
  return Mojo_SetConstFloatMatrix_(aUnit,ulConst,&aV,1,aMatrixOffset);
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstIntArray(eShaderUnit aUnit, tU32 ulConst, const tVec4iCVec *apV, tU32 ulNum)
{
  return Mojo_SetConstInt_(aUnit,ulConst,&(*apV)[0],ulNum);
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstInt(eShaderUnit aUnit, tU32 ulConst, const sVec4i& aV)
{
  return Mojo_SetConstInt_(aUnit,ulConst,&aV,1);
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstBoolArray(eShaderUnit aUnit, tU32 ulConst, const tI8CVec* apV, tU32 ulNum)
{
  return Mojo_SetConstBool_(aUnit,ulConst,&(*apV)[0],ulNum);
}

///////////////////////////////////////////////
inline tBool __stdcall Mojo_SetConstBool(eShaderUnit aUnit, tU32 ulConst, tBool aV)
{
  return Mojo_SetConstBool_(aUnit,ulConst,&aV,1);
}

///////////////////////////////////////////////
static __forceinline tBool Mojo_UploadConstant(
    eShaderUnit aUnit,
    const sShaderConstantsDesc* constBuffer,
    const tU16 cbType,
    const tU32 anConstIndex,
    const tU32 anConstSize,
    const tU32 anConstHwIndex)
{
  switch (cbType) {
    case eShaderRegisterType_ConstFloat: {
      const sVec4f* rv = &constBuffer->mvFloatRegisters[anConstIndex];
      Mojo_SetConstFloat_(aUnit,anConstHwIndex,rv,anConstSize);
      break;
    }
    case eShaderRegisterType_ConstInt: {
      const sVec4i* rv = &constBuffer->mvIntRegisters[anConstIndex];
      Mojo_SetConstInt_(aUnit,anConstHwIndex,rv,anConstSize);
      break;
    }
    case eShaderRegisterType_ConstBool: {
      const sVec4i* rv = &constBuffer->mvIntRegisters[anConstIndex];
      niLoop(j,anConstSize) {
        Mojo_SetConstBool(
            aUnit,
            anConstHwIndex+j,
            rv[j].x ? eTrue : eFalse);
      }
      break;
    }
    default:
      niAssertUnreachable("Unreachable.");
      return eFalse;
  }
  return eTrue;
}
static __forceinline tBool Mojo_UploadUniform(
    eShaderUnit aUnit,
    iHString* ahspName,
    iShaderConstants* apConsts,
    const tU32 anConstSize,
    const tU32 anConstHwIndex)
{
  if (!apConsts) return eFalse;
  const sShaderConstantsDesc* ct = (const sShaderConstantsDesc*)apConsts->GetDescStructPtr();
  sShaderConstantsDesc::tConstMap::const_iterator it = ct->mmapConstants.find(ahspName);
  if (it != ct->mmapConstants.end() && it->second != eInvalidHandle) {
    const sShaderConstantsDesc::sConstant& c = ct->mvConstants[it->second];
    return Mojo_UploadConstant(
        aUnit,ct,
        c.Type,
        c.nDataIndex,
        ni::Min(c.nSize,anConstSize),
        anConstHwIndex);
  }
  return eFalse;
}

///////////////////////////////////////////////
tBool Mojo_ApplyShaderConstants(
    iGraphicsContext* apContext,
    iShaderConstants* apShaderConsts,
    iGraphics* apGraphics,
    eShaderUnit aUnit, iShader* apProg,
    iDrawOperation* apDrawOp,
    const sMaterialDesc* apMaterial)
{
  if (!apShaderConsts)
    return ni::eFalse;

  sShaderConstantsDesc* const constDesc = (sShaderConstantsDesc*)apShaderConsts->GetDescStructPtr();
  niLoop(i,constDesc->mvConstants.size()) {
    sShaderConstantsDesc::sConstant& c = constDesc->mvConstants[i];
    iHString* hspName = c.hspName;
    if (hspName) {
      // Look in the material's constants
      if (Mojo_UploadUniform(aUnit,hspName,apMaterial->mptrConstants,c.nSize,c.nHwIndex))
        continue;
    }

    // Use default value...
    Mojo_UploadConstant(aUnit,constDesc,
                        c.Type,
                        c.nDataIndex,
                        c.nSize,
                        c.nHwIndex);
  }

  return ni::eTrue;
}

///////////////////////////////////////////////
tBool Mojo_ApplyVertexShader(
    iGraphicsContext* apContext,
    sGLCache& aCache,
    iGraphics* apGraphics,
    iShader* apProg,
    iDrawOperation* apDrawOp,
    const sMaterialDesc* apMaterial)
{
  niAssert(apProg);

  Mojo_ApplyShaderConstants(
      apContext,
      (iShaderConstants*)apProg->GetConstants(),
      apGraphics,
      eShaderUnit_Vertex,apProg,
      apDrawOp,apMaterial);

  return eTrue;
}

///////////////////////////////////////////////
tBool Mojo_ApplyPixelShader(
    sGLContext* apContext,
    sGLCache& aCache,
    iGraphics* apGraphics,
    iShader* apProg,
    iDrawOperation* apDrawOp,
    const sMaterialDesc* apMaterial)
{

  Mojo_ApplyShaderConstants(
      apContext,
      (iShaderConstants*)apProg->GetConstants(),
      apGraphics,
      eShaderUnit_Pixel,apProg,
      apDrawOp,
      apMaterial);

  // If a fragment program is set, just bind all texture channels and set the samplers
  {
    const sMaterialDesc* m = apMaterial;
    niLoop(i,eMaterialChannel_Last) {
      eMaterialChannel channel = (eMaterialChannel)i;
      if (m->mChannels[channel].mTexture.IsOK()) {
        const tI8 nSamplerIndex = (tI8)i;
        GL2_ApplyMaterialChannel(
            apContext,
            apGraphics,
            aCache,
            nSamplerIndex,
            channel,
            m);
      }
    }
  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  Textures
//
//--------------------------------------------------------------------------------------------
struct sGL2TextureFormat
{
  GLenum kind;
  GLint  type;
  GLint  format;
  GLint  internalformat;
  tTextureFlags flags;
  Ptr<iPixelFormat> pxf;

  sGL2TextureFormat() {
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
struct sGL2TextureBase : public cIUnknownImpl<iGLTexture,eIUnknownImplFlags_DontInherit1|eIUnknownImplFlags_DontInherit2,iTexture,iDeviceResource> {
  iGraphicsDriver*  mpDriver;
  tHStringPtr       mhspName;
  sGL2TextureFormat mFormat;
  GLuint            mGLHandle;
  tIntPtr           mhLastSamplerStateSet;

  sGL2TextureBase() {
    mhLastSamplerStateSet = 0;
  }

  __forceinline iGraphics* GetGraphics() const {
    return mpDriver->GetGraphics();
  }
};

///////////////////////////////////////////////
struct sGL2Texture : public sGL2TextureBase
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
  sGL2Texture(iGraphicsDriver* apDriver, iHString* ahspName, tTextureFlags aTextureFlags = 0)
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
  ~sGL2Texture() {
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
  void _InitCubeFace(iGraphics* apGraphics, tIntPtr aGLHandle, const sGL2TextureFormat& aFmt, eBitmapCubeFace aFace, tU32 anW, tU32 anNumMipMaps) {
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
        sGLCache& cache = GLES2_GetCache(mpDriver);
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
        niLog(Info,niFmt("GL2 MainRT FBO: %d",mGLFBOHandle));
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
      }
      _glBindFramebuffer(GL_FRAMEBUFFER, mGLFBOHandle);
      GLERR_RET(eFalse);
      sGLCache& cache = GLES2_GetCache(mpDriver);
      cache._renderTargetFBO = mGLFBOHandle;

      if (bNewHandle) {
        if (!this->_BindAsTexture()) {
          niError("Can't bind texture to attach to RT.");
          return eFalse;
        }
        _glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mGLHandle, 0);
        GLERR_RET(eFalse);

        // Clear RT to black
        GLES2_DoClear(mpDriver, eClearBuffersFlags_Color, 0, 1.0f, 0, eFalse);
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
        niLog(Info,niFmt("GL2 MainDS FBO: %d",mGLFBOHandle));
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
      sGLCache& cache = GLES2_GetCache(mpDriver);
      cache._depthStencilFBO = mGLFBOHandle;

      if (bNewHandle) {
        _glRenderbufferStorage(GL_RENDERBUFFER, mFormat.internalformat, mnWidth, mnHeight);
        GLERR_RET(eFalse);
      }
      _glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mGLFBOHandle);
      GLERR_RET(eFalse);

      if (bNewHandle) {
        GLES2_DoClear(mpDriver, eClearBuffersFlags_Depth, 0, 1.0f, 0, eFalse);
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
          // GL_DEBUG_LOG(("GL2 Uploaded Dummy Mipmap '%d' to VidMem (%d)",i,++_kNumGL2TexUpload));
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
      const sGL2TextureFormat& aFormat,
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
struct sGL2TextureCube : public sGL2TextureBase
{
  tU32              mnWidth;
  tU32              mnNumMipMaps;
  Ptr<sGL2Texture>  mFaces[6];

 public:
  ///////////////////////////////////////////////
  sGL2TextureCube(iGraphicsDriver* apDriver, iHString* ahspName)
  {
    ZeroMembers();
    mpDriver = apDriver;
    mhspName = ahspName;
    GetGraphics()->GetTextureDeviceResourceManager()->Register(this);
  }

  ///////////////////////////////////////////////
  ~sGL2TextureCube() {
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
        sGL2Texture* pFace = mFaces[i];
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
      mFaces[i] = niNew sGL2Texture(mpDriver,NULL,eTextureFlags_SubTexture);
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
// cGL2OcclusionQuery declaration.
class cGL2OcclusionQuery : public ni::cIUnknownImpl<ni::iOcclusionQuery>
{
  niBeginClass(cGL2OcclusionQuery);

 public:
  //! Constructor.
  cGL2OcclusionQuery() {
    ZeroMembers();
    GLDRV_GEN_HANDLE(mGLHandle, _glGenQueries);
    GLERR_RET(;);
  }
  //! Destructor.
  ~cGL2OcclusionQuery() {
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
    niClassIsOK(cGL2OcclusionQuery);
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
    if (niThis(cGL2OcclusionQuery)->GetStatus(eFalse) != eOcclusionQueryStatus_Successful)
      return eInvalidHandle;
    return mnResult;
  }

 private:
  GLuint                  mGLHandle;
  eOcclusionQueryStatus   mStatus;
  tU32                    mnResult;
  niEndClass(cGL2OcclusionQuery);
};

#endif

//--------------------------------------------------------------------------------------------
//
//  OpenGL Buffer
//
//--------------------------------------------------------------------------------------------
struct sGLVertexArrayData : public cIUnknownImpl<iVertexArray,eIUnknownImplFlags_DontInherit1,iDeviceResource> {
  __forceinline const GLenum GetBufferTarget() const { return GL_ARRAY_BUFFER; }
  __forceinline const tPtr GetBufferData() const { return (tPtr)mVertices.data(); }
  __forceinline const tU32 GetBufferDataSize() const { return (tU32)mVertices.size(); }
  __forceinline const tU32 GetBufferElSize() const { return mFVF.GetStride(); }
  __forceinline const tU32 GetBufferElCount() const { return (tU32)mVertices.size() / mFVF.GetStride(); }

  astl::vector<tU8> mVertices;
  cFVFDescription   mFVF;
};

struct sGLIndexArray32Data : public cIUnknownImpl<iIndexArray,eIUnknownImplFlags_DontInherit1,iDeviceResource> {
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
    mnLastBoundFrame = ~0;
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
            || mnLastBoundFrame == ni::GetLang()->GetFrameNumber()
#endif
        )
        {
#if GL_DYNAMIC_BUFFER_DEBUG && (GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING)
          if (mnLastBoundFrame == ni::GetLang()->GetFrameNumber()) {
            niDebugFmt((
              "... GL_DYNAMIC_BUFFER '%p' already bound in frame '%d', orphaned.",
              (tIntPtr)this, ni::GetLang()->GetFrameNumber()));
          }
#endif

          _glBufferData(
            bufferTarget,
            elSize*elCount,
            NULL,
            GL_DYNAMIC_DRAW);
        }
#if GL_DYNAMIC_BUFFER_DEBUG && (GL_DYNAMIC_BUFFER_MODE == GL_DYNAMIC_BUFFER_MODE_ORPHANING)
        else {
          niDebugFmt((
            "... GL_DYNAMIC_BUFFER '%p' not yet bound in frame '%d', using as-is.",
            (tIntPtr)this, ni::GetLang()->GetFrameNumber()));
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
      GL2_TRACE_BUFFER((">>> Restoring GLBuffer: target: %d, usage: %s, elSize: %d, elCount: %d, dataSize: %db (%gMB).",
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
    mnLastBoundFrame = ni::GetLang()->GetFrameNumber();
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
  tU32                        mnLastBoundFrame;
#endif
};

struct sGLVertexArray : public sGLBufferImpl<sGLVertexArrayData> {
  sGLVertexArray(iDeviceResourceManager* apDevResMan, tU32 anNumVertices, tFVF aFVF, eArrayUsage aUsage)
      : sGLBufferImpl(apDevResMan, aUsage)
  {
    mFVF.Setup(aFVF);
    mVertices.resize(mFVF.GetStride()*anNumVertices);
    ni::MemZero((tPtr)mVertices.data(),mVertices.size());

    GL2_TRACE_BUFFER((">>> Created VertexArray GLBuffer: %p, target: %d, usage: %s, elSize: %d, elCount: %d, dataSize: %db (%gMB).",
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

    GL2_TRACE_BUFFER((">>> Created IndexArray32 GLBuffer: %p, target: %d, usage: %s, elSize: %d, elCount: %d, dataSize: %db (%gMB).",
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
class cGL2ContextWindow : public sGLContext
{
  niBeginClass(cGL2ContextWindow);

 public:
  iGraphicsDriver* mpDrv;
  Ptr<iOSWindow> mptrWindow;
  tU32 mnSwapInterval;
  tTextureFlags mnBBFlags;
  Ptr<iPixelFormat> mptrBBPxf, mptrDSPxf;
#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
  tsglContext* mpTSGLContext;
#endif

  cGL2ContextWindow(
      iGraphicsDriver* apParent,
      iOSWindow* apWindow,
      const achar* aaszBBPxf,
      const achar* aaszDSPxf,
      tU32 anSwapInterval,
      tTextureFlags anBBFlags)
      : sGLContext(apParent->GetGraphics())
  {
#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
    mpTSGLContext = NULL;
#endif
    mpDrv = apParent;
    niAssert(mpDrv != NULL);
    mptrWindow = apWindow;
    mnSwapInterval = anSwapInterval;
    mnBBFlags = anBBFlags;
    mnSyncCounter = 0;

#ifndef NO_TSGL
    if (tsglLoadLibrary(TSGL_LOAD_ALL) != TSGL_OK) {
      niError(_A("Can't initialize TSGL."));
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
    if (!_DoResizeContext())
      return;
  }
  ~cGL2ContextWindow() {
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

#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
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
      GLES2_ResetContextDeviceResources(mpDrv);
      GLES2_ResetCache(mpDrv);
    }

    const tU32 w = mptrWindow->GetClientSize().x;
    const tU32 h = mptrWindow->GetClientSize().y;

    if (hasContextLost
        || !mptrRT[0].IsOK()
#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
        || !mpTSGLContext
#endif
        )
    {
#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
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
      if (!GL2_InitializeExt()) {
        niError("Can't initialize OpenGL extensions.");
        return eFalse;
      }

      // Initialize main RT
      {
        if (!mptrRT[0].IsOK()) {
          mptrRT[0] = niNew sGL2Texture(mpDrv,_H("GLES2_MainRT"));
        }
        sGL2Texture* pGLTex = (sGL2Texture*)mptrRT[0].ptr();
        pGLTex->_InitMainRT(mptrBBPxf,w,h);
      }

      // Initialize main DS
      {
        if (!mptrDS.IsOK()) {
          mptrDS = niNew sGL2Texture(mpDrv,_H("GLES2_MainDS"));
        }
        sGL2Texture* pGLTex = (sGL2Texture*)mptrDS.ptr();
        pGLTex->_InitMainDS(mptrDSPxf,w,h);
      }
    }
    else {
      // Resize-only stuff
      {
        sGL2Texture* pGLTex = (sGL2Texture*)mptrRT[0].ptr();
        pGLTex->mnWidth = w;
        pGLTex->mnHeight = h;
      }
      {
        sGL2Texture* pGLTex = (sGL2Texture*)mptrDS.ptr();
        pGLTex->mnWidth = w;
        pGLTex->mnHeight = h;
      }
    }

    mrectScissor = sRecti(0,0,w,h);
    mrectViewport = mrectScissor;

    const sRecti vp = this->GetViewport();
    GLCALL_WARN(_glViewport(vp.x,vp.y,vp.GetWidth(),vp.GetHeight()));
    const sRecti sc = this->GetScissorRect();
    const tU32 nY = (this->GetHeight()-sc.y)-sc.GetHeight();
    GLCALL_WARN(_glScissor(sc.x,nY,sc.GetWidth(),sc.GetHeight()));

    niDebugFmt((_A("GLES2 Context - Resized [%p]: %dx%d, BB: %s, DS: %s, AA: %s, VP: %s, SC: %s"),
                (tIntPtr)this,
                w,h,
                mptrBBPxf->GetFormat(),
                mptrDSPxf->GetFormat(),
                GetNumAASamples(),
                this->GetViewport(),
                this->GetScissorRect()));

    if (hasContextLost) {
      GLES2_InitContextDeviceResources(mpDrv);
    }
    return eTrue;
  }
  void _DestroyContext() {
#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
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
    return mptrWindow.IsOK();
  }

  virtual void __stdcall Invalidate() {
    if (mpDrv) {
      GLES2_ResetContextDeviceResources(mpDrv);
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
    GLES2_SwapBuffers(mpDrv,this,niFlagIs(aFlags,eGraphicsDisplayFlags_DoNotWait));
    ++mnSyncCounter; // Make sure the Viewport and scissor will be set next frame

    if (mptrWindow.IsOK()) {
      const sVec2i newSize = mptrWindow->GetClientSize();
      if ((newSize.x != (tI32)this->GetWidth()) || (newSize.y != (tI32)this->GetHeight())) {
        _DoResizeContext();
      }
    }

    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
#ifdef _glReadPixels
    if (!mpDrv) return NULL;

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
#else
    return NULL;
#endif
  }

  /////////////////////////////////////////////
  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    GLES2_ClearBuffers(mpDrv,this,clearBuffer,anColor,afDepth,anStencil);
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    return GLES2_DrawOperation(mpDrv,this,apDrawOp,this->GetNumAASamples());
  }

  niEndClass(cGL2ContextWindow);
};

class cGL2ContextRT : public sGLContext
{
  niBeginClass(cGL2ContextRT);

 public:
  iGraphicsDriver* mpDrv;

  cGL2ContextRT(
      iGraphicsDriver* apParent,
      iTexture* apRT,
      iTexture* apDS,
      sGLContext* apToCopy)
      : sGLContext(apParent->GetGraphics())
  {
    mpDrv = apParent;
    niAssert(mpDrv != NULL);
    mptrRT[0] = apRT;
    mptrDS = apDS;
    mrectViewport = Recti(0,0,apRT->GetWidth(),apRT->GetHeight());
    mrectScissor = mrectViewport;
  }
  ~cGL2ContextRT() {
    Invalidate();
  }

  virtual void __stdcall _CheckResizeContext() {
  }
#if !defined niAndroid && !defined __TSGL_NOCONTEXT__
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
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    ++mnSyncCounter; // Make sure the Viewport and scissor will be set next frame
    return eTrue;
    }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    GLES2_ClearBuffers(mpDrv,this,clearBuffer,anColor,afDepth,anStencil);
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    return GLES2_DrawOperation(mpDrv,this,apDrawOp,this->GetNumAASamples());
  }

  niEndClass(cGL2ContextRT);
};

///////////////////////////////////////////////
static tBool GL2_ApplyContext(sGLCache& aCache, sGLContext* apCtx, tBool& isFlippedRT)
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

      Ptr<sGL2Texture> pRT = (sGL2Texture*)apCtx->mptrRT[0]->Bind((iUnknown*)eInvalidHandle);
      if (!pRT->_BindAsRenderTarget()) {
        niError("Can't bind render target.");
        return eFalse;
      }

      if (apCtx->mptrDS.IsOK()) {
        Ptr<sGL2Texture> pDS = (sGL2Texture*)apCtx->mptrDS->Bind((iUnknown*)eInvalidHandle);
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
    const tU32 nSCY = isFlippedRT ? sc.y : (rtHeight-sc.y)-sc.GetHeight();
    GLCALL_WARN(_glScissor(sc.x,nSCY,sc.GetWidth(),sc.GetHeight()));

    aCache._scissorRectIsFullScreen = (rtWidth == sc.GetWidth() && rtHeight == sc.GetHeight());
  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  Fixed Pipeline Shader Library
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
static tBool GL2_ApplyMaterialChannel(
    sGLContext* apContext,
    iGraphics* apGraphics,
    sGLCache& aCache,
    tU32 anTSS, eMaterialChannel aChannel,
    const sMaterialDesc* apDOMat)
{
  _glActiveTexture(GL_TEXTURE0+anTSS);

  const sMaterialChannel& ch = apContext->_GetChannel(apDOMat, aChannel);

  sGL2TextureBase* tex = (sGL2TextureBase*)ch.mTexture.ptr();
  tex = tex ? (sGL2TextureBase*)tex->Bind(NULL) : NULL;
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
      // niDebugFmt(("... GL2 Texture SetSamplerStates '%s': %p", tex->mhspName, hSS));

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
// GLES2GraphicsDriver
//
//--------------------------------------------------------------------------------------------
const achar* GL2Drv_GetName() { return _A("GL2"); }
const achar* GL2Drv_GetDesc() { return _A("GL2 Graphics Driver"); }

struct cGLES2GraphicsDriver : public cIUnknownImpl<iGraphicsDriver>
{
  iGraphics* mpGraphics;
  sGLCache   mCache;
  tBool      mbUseColorA;
  Ptr<iGraphicsDrawOpCapture> mptrDOCapture;
  astl::vector<tHStringPtr> mvProfiles[eShaderUnit_Last];
  MOJOSHADER_glContext* mpMojoContext;
  sFixedShaders mFixedShaders;

  cGLES2GraphicsDriver(iGraphics* apGraphics) {
    mpGraphics = apGraphics;
    mbUseColorA = eFalse;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return mpGraphics != NULL;
  }
  virtual void __stdcall Invalidate() {
    if (mpMojoContext) {
      MOJOSHADER_glDestroyContext(mpMojoContext);
      mpMojoContext = NULL;
    }
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
  virtual const achar* __stdcall GetName() const { return GL2Drv_GetName(); }
  virtual const achar* __stdcall GetDesc() const { return GL2Drv_GetDesc(); }
  virtual const achar* __stdcall GetDeviceName() const {
    return _A("Default");
  }

  ///////////////////////////////////////////////
  tBool _InitMojoShaders() {
    if (mpMojoContext)
      return eTrue;

    // Detect profiles
    if (mvProfiles[0].empty()) {
      niLoop(i,eShaderUnit_Last) {
        mvProfiles[i].clear();
      }

      mvProfiles[eShaderUnit_Vertex].push_back(_H("glslv"));
      mvProfiles[eShaderUnit_Vertex].push_back(_H("hlsl_vs_3_0"));
      mvProfiles[eShaderUnit_Pixel].push_back(_H("glslf"));
      mvProfiles[eShaderUnit_Pixel].push_back(_H("hlsl_ps_3_0"));
    }

    // Initializer mojo context
    mpMojoContext = MOJOSHADER_glCreateContext(
      MOJOSHADER_PROFILE_GLSL,
#if defined __JSCC__
      NULL,
#else
      mojo_getprocaddress,
#endif
      NULL,
      NULL,NULL,NULL);
    if (!mpMojoContext) {
      niError(niFmt("Can't create a GLSL compatible context: %s.",MOJOSHADER_glGetError()));
      return eFalse;
    }
    MOJOSHADER_glMakeContextCurrent(mpMojoContext);

    if (!mFixedShaders.Initialize(mpGraphics)) {
      niError("Can't load fixed shaders.");
      return eFalse;
    }

    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iGraphicsContext* __stdcall CreateContextForWindow(
      iOSWindow* apWindow,
      const achar* aaszBBFormat, const achar* aaszDSFormat,
      tU32 anSwapInterval,
      tTextureFlags aBackBufferFlags)
  {
    niCheckIsOK(apWindow,NULL);

    Ptr<iGraphicsContext> ctx = niNew cGL2ContextWindow(
        this,
        apWindow,
        aaszBBFormat,aaszDSFormat,
        anSwapInterval,
        aBackBufferFlags);
    niCheckIsOK(ctx, NULL);
    niCheck(_InitMojoShaders(),NULL);

    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS)
  {
    niCheckIsOK(apRT0,NULL);

    Ptr<iGraphicsContextRT> ctx = niNew cGL2ContextRT(
      this,
      apRT0, apDS,
      NULL);
    niCheckIsOK(ctx, NULL);
    niCheck(_InitMojoShaders(),NULL);

    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall ResetAllCaches() {
    GLES2_ResetCache(this);
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
        return kGL2_MaxTU;
      case eGraphicsCaps_Texture2DMaxSize:
        return kGL2_MaxRegularTexSize;
      case eGraphicsCaps_TextureCubeMaxSize:
        return kGL2_MaxCubeTexSize;
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
    }
    return 0;
  }

  /////////////////////////////////////////////
  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const {
    return eGraphicsDriverImplFlags_IndexArrayObject|
        eGraphicsDriverImplFlags_VertexArrayObject;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall GetIsInitialized() const {
    return eTrue;
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

    sGL2TextureFormat texFormat;

    tU32 nWidth = apFormat->GetWidth();
    tU32 nHeight = apFormat->GetHeight();
    if (!hasPartialNP2 || !niFlagIs(aFlags,eTextureFlags_Overlay)) {
      nWidth = GetNearestPow2(nWidth);
      nHeight = GetNearestPow2(nHeight);
    }
    tU32 nMaxSize = niFlagIs(aFlags,eTextureFlags_Overlay) ?
        kGL2_MaxOverlayTexSize : kGL2_MaxRegularTexSize;

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
            kGL2_MaxOverlayTexSize : kGL2_MaxRegularTexSize;

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

      Ptr<sGL2Texture> newTex = niNew sGL2Texture(this,ahspName);
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
        if (anWidth > (tU32)kGL2_MaxCubeTexSize) {
          anWidth = kGL2_MaxCubeTexSize;
        }
        anNumMipMaps = ((anNumMipMaps == 0) && niFlagIs(aFlags,eTextureFlags_MipMaps)) ?
            ComputeNumPow2Levels(anWidth>>1,anHeight>>1) : anNumMipMaps;
      }

      GL_DEBUG_LOG((_A("- CREATE CUBE TEXTURE '%s' %d (mips:%d) %s (%d)"),
                      niHStr(ahspName),
                      anWidth,anNumMipMaps,aaszFormat,aFlags));

      Ptr<sGL2TextureCube> newTex = niNew sGL2TextureCube(this,ahspName);
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

    sGL2Texture* tex = niStaticCast(sGL2Texture*,apDest);
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

    sGL2Texture* src = niStaticCast(sGL2Texture*,apSrc);
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
#if defined USE_FBO && defined _glReadPixels
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
      }
#endif

#ifdef USE_GL_GET_TEX_IMAGE
      GL_DEBUG_LOG(("- BlitTextureToBitmap: Use glGetTexImage."));

      if (aDestRect.GetTopLeft() == sVec2i::Zero() &&
          aDestRect.GetSize() == Vec2i(apSrc->GetWidth(),apSrc->GetHeight()))
      {
        GL_DEBUG_LOG(("- BlitTextureToBitmap: COPY FULL TEXTURE CONTENT"));
        if (apSrc->GetPixelFormat()->IsSamePixelFormat(apDest->GetPixelFormat())) {
          GL_DEBUG_LOG(("- BlitTextureToBitmap: SAME PIXEL FORMAT, READ DIRECTLY IN DEST"));
          glGetTexImage(GL_TEXTURE_2D,anSrcLevel,src->mFormat.format,src->mFormat.type,apDest->GetData());
          return eTrue;
        }
      }

      GL_DEBUG_LOG(("- BlitTextureToBitmap: USE TEMP BITMAP"));
      niCheck(src->_BindAsTexture(),eFalse);
      Ptr<iBitmap2D> bmpTmp = src->_CreateMatchingBitmap(mpGraphics);
      glGetTexImage(GL_TEXTURE_2D,anSrcLevel,src->mFormat.format,src->mFormat.type,bmpTmp->GetData());
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
      glCopyTexSubImage2D(target,
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
            glCopyTexSubImage2D(target,
                                level,
                                drect.x1() + col, aDstHeight - row - 1,
                                srect.x1() + col * xrel, aSrcHeight - srect.y2() + row * yrel,
                                1, 1);
          }
        }
      }
      else {
        for (tI32 row = drect.y1(), i = 0; row < drect.y2(); ++row, ++i) {
          glCopyTexSubImage2D(target,
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
      GLES2_ResetCache(this);
      sGL2Texture* src = niStaticCast(sGL2Texture*,apSrc);
      sGL2Texture* dst = niStaticCast(sGL2Texture*,apDest);
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
      sGL2Texture* src = niStaticCast(sGL2Texture*,apSrc);
      if (!src || !src->mptrBmpRestore.IsOK())
        return eFalse;

      sGL2Texture* tex = niStaticCast(sGL2Texture*,apDest);
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
    niCheck(mpMojoContext != NULL,NULL);

    if (!niIsOK(apFile)) {
      niError(_A("Invalid file."));
      return NULL;
    }

    if (apFile->ReadLE32() != kfccD3DShader) {
      niError(_A("The file is not a D3D shader."));
      return NULL;
    }

    if (apFile->ReadLE32() != niMakeVersion(1,0,0)) {
      niError(_A("Incompatible version."));
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
      MOJOSHADER_glShader* pVS = NULL;
      pProg = niNew cMojoShaderVertex(
          mpGraphics,ahspName,
          mpGraphics->GetShaderDeviceResourceManager(),
          hspProfileName,pVS,vData);
    }
    else if (nUnit == eShaderUnit_Pixel)
    {
      MOJOSHADER_glShader* pPS = NULL;
      pProg = niNew cMojoShaderPixel(
          mpGraphics,ahspName,
          mpGraphics->GetShaderDeviceResourceManager(),
          hspProfileName,pPS,vData);
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
      return niNew cGL2OcclusionQuery();
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
  virtual iShaderConstants* __stdcall CreateShaderConstants(tU32) const {
    return NULL;
  }
  virtual tIntPtr __stdcall CompileSamplerStates(iSamplerStates* apStates) {
    return NULL;
  }
  virtual tIntPtr __stdcall CompileRasterizerStates(iRasterizerStates* apStates) {
    return NULL;
  }
  virtual tIntPtr __stdcall CompileDepthStencilStates(iDepthStencilStates* apStates) {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall InitHardwareCursor(iTexture* apTex, const sRecti& aRect, const sVec2i& avPivot) {
    return eTrue;
  }
  virtual tBool __stdcall UpdateHardwareCursor(tI32 anX, tI32 anY, tBool abImmediate) {
    return eTrue;
  }
  virtual tBool __stdcall ShowHardwareCursor(tBool abShown) {
    return eTrue;
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
        niLoop(i,kGL2_MaxVertexAttrs) {
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
    niLoop(i,kGL2_MaxVertexAttrs) {
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
      if (!GL2_ApplyContext(mCache,apContext,isFlippedRT)) {
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
          glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
        else {
          glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
      }

      if (niFlagIs(pMatDesc->mFlags,eMaterialFlags_PolygonOffset)) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(pMatDesc->mvPolygonOffset.x,pMatDesc->mvPolygonOffset.y);
      }
      else {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0,0);
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
      }
    }

    if (mptrDOCapture.IsOK()) {
      mptrDOCapture->EndCaptureDrawOp(apContext,apDrawOp,sVec4i::Zero());
    }

    return r;
  }

  void _ResetContextDeviceResources() {
#ifndef __GLES2__
    if (_glUseProgram)
#endif
    {
      // Disable all shaders...
      _glUseProgram(0);
    }

#ifndef __GLES2__
    if (_glActiveTexture)
#endif
    {
      // Disable all texture units
      for (tI32 i = kGL2_MaxTU-1; i >= 0; --i) {
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
#ifndef __GLES2__
    if (!_glActiveTexture)
      return;
#endif
    // Disable all texture units
    for (tI32 i = kGL2_MaxTU-1; i >= 0; --i) {
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
};

static tBool GLES2_SwapBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tBool abDoNotWait) {
  GL_DEBUG_SWAP_BUFFERS();

  cGLES2GraphicsDriver* d = ((cGLES2GraphicsDriver*)apDrv);
  tBool isFlippedRT;
  if (!GL2_ApplyContext(d->mCache,apContext,isFlippedRT)) {
    niError(_A("Can't apply context."));
    return eFalse;
  }

#if !defined __TSGL_NOCONTEXT__ && !defined __JSCC__
  tsglContext* ctx = apContext->GetTSGLContext();
  if (ctx) {
    GLCALL_ERR(tsglSwapBuffers(ctx,abDoNotWait),eFalse);
  }
#endif
  return eTrue;
}

static void GLES2_DoClear(iGraphicsDriver* apDrv, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil, tBool abScissorTest) {
  cGLES2GraphicsDriver* d = ((cGLES2GraphicsDriver*)apDrv);
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
    if (abScissorTest) {
      if (!cache._scissorTest) {
        _glEnable(GL_SCISSOR_TEST);
      }
      _glClear(flags);
      if (!cache._scissorTest) {
        _glDisable(GL_SCISSOR_TEST);
      }
    }
    else {
      if (cache._scissorTest) {
        _glDisable(GL_SCISSOR_TEST);
      }
      _glClear(flags);
      if (cache._scissorTest) {
        _glEnable(GL_SCISSOR_TEST);
      }
    }
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

static void GLES2_ClearBuffers(iGraphicsDriver* apDrv, sGLContext* apContext, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
  cGLES2GraphicsDriver* d = ((cGLES2GraphicsDriver*)apDrv);

  iGraphicsDrawOpCapture* pDrawOpCapture = d->mptrDOCapture;
  if (pDrawOpCapture) {
    if (!pDrawOpCapture->BeginCaptureDrawOp(
            apContext,NULL,
            Vec4<tI32>(clearBuffer,anColor,ftoul(afDepth),anStencil)))
      return;
  }

  GL_DEBUG_MARKER_GROUP(ClearBuffers);

  tBool isFlippedRT;
  if (!GL2_ApplyContext(d->mCache,apContext,isFlippedRT)) {
    niError(_A("Can't apply context."));
    return;
  }

  GLES2_DoClear(apDrv, clearBuffer, anColor, afDepth, anStencil, eTrue);

  if (pDrawOpCapture) {
    pDrawOpCapture->EndCaptureDrawOp(
        apContext,NULL,
        Vec4<tI32>(clearBuffer,anColor,ftoul(afDepth),anStencil));
  }
}

static tBool GLES2_DrawOperation(iGraphicsDriver* apDrv, sGLContext* apContext, iDrawOperation* apDrawOp, const tU32 anAA)
{
  return ((cGLES2GraphicsDriver*)apDrv)->_DrawOperation(apContext,apDrawOp,anAA);
}

static tBool GLES2_ResetCache(iGraphicsDriver* apDrv) {
  ((cGLES2GraphicsDriver*)apDrv)->mCache.Reset();
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
  if (((cGLES2GraphicsDriver*)apDrv)->mpMojoContext) {
    MOJOSHADER_glBindProgram(NULL);
    _glUseProgram(0);
  }
  niLoop(i,kGL2_MaxVertexAttrs) {
    GLCALL_WARN(_glDisableVertexAttribArray(i));
  }
  // Disable all texture units
  for (tI32 i = kGL2_MaxTU-1; i >= 0; --i) {
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
static tBool GLES2_ResetContextDeviceResources(iGraphicsDriver* apDrv) {
  ((cGLES2GraphicsDriver*)apDrv)->_ResetContextDeviceResources();
  return eTrue;
}
static tBool GLES2_InitContextDeviceResources(iGraphicsDriver* apDrv) {
  ((cGLES2GraphicsDriver*)apDrv)->_InitContextDeviceResources();
  return eTrue;
}

static sGLCache& GLES2_GetCache(iGraphicsDriver* apDrv) {
  return ((cGLES2GraphicsDriver*)apDrv)->mCache;
}

niExportFunc(iUnknown*) New_GraphicsDriver_GL2(const Var& avarA, const Var&) {
  QPtr<iGraphics> ptrGraphics = avarA;
  niCheckIsOK(ptrGraphics,NULL);
  return (iGraphicsDriver*)niNew cGLES2GraphicsDriver(ptrGraphics);
}

#endif // GDRV_GL2
