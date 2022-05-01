#include "stdafx.h"

#if defined niOSX || defined niWindows

#include "GDRV_GLContext.h"

#ifdef _WIN32
#include "GL/wglext.h"

static PFNWGLGETEXTENSIONSSTRINGARBPROC _wglGetExtensionsStringARB = NULL;
static PFNWGLCHOOSEPIXELFORMATARBPROC _wglChoosePixelFormatARB = NULL;

struct AutoDestroyBSContext {
  tsglContext* _context;
  AutoDestroyBSContext() : _context(NULL) {}

  tBool Create() {
    if (tsglCreateContext(&_context,NULL,32,24,8,eFalse,1) == TSGL_OK) {
      tsglMakeCurrent(_context);
      return eTrue;
    }
    else {
      return eFalse;
    }
  }

  ~AutoDestroyBSContext() {
    if (_context) {
      wglMakeCurrent(NULL,NULL);
      if (_context->mhWnd) {
        ::DestroyWindow(_context->mhWnd);
      }
      tsglDestroyContext(_context);
    }
  }
};

#endif

//===========================================================================
//
//  Desktop
//
//===========================================================================

#  ifdef TSGL_DESKTOP
int tsglCreateContext(
    tsglContext** appCtx,
    iOSWindow* apWindow,
    tU32 anColorBits,
    tU32 anDepthBits,
    tU32 anStencilBits,
    tU32 anAA,
    tU32 anSwapInterval)
{
#    ifdef __APPLE__
  tsglContext* ctx = (tsglContext*)TSGL_MALLOC(sizeof(tsglContext));
  if (!ctx) {
    TSGL_RETURN_ERROR(ALLOCCTX,"Can't allocate memory for TSGL context.");
  }
  TSGL_ZEROMEMORY(ctx,sizeof(*ctx));

  ctx->mbInitialized = niOSX::osxglHasContext(apWindow);
  if (!ctx->mbInitialized) {
    if (anAA) {
      sOSXGLConfig glConfig;
      glConfig.swapInterval = anSwapInterval;
      glConfig.aaSamples = anAA;
      ctx->mbInitialized = niOSX::osxglCreateContext(apWindow,&glConfig);
    }
    if (!ctx->mbInitialized) {
      sOSXGLConfig glConfig;
      glConfig.swapInterval = anSwapInterval;
      ctx->mbInitialized = niOSX::osxglCreateContext(apWindow,&glConfig);
      if (!ctx->mbInitialized) {
        return TSGL_ERR_CREATECTX;
      }
    }
  }
  ctx->mpWindow = apWindow;

#    elif defined _WIN32
  tsglContext* ctx = (tsglContext*)TSGL_MALLOC(sizeof(tsglContext));
  if (!ctx) {
    TSGL_RETURN_ERROR(ALLOCCTX,"Can't allocate memory for TSGL context.");
  }
  TSGL_ZEROMEMORY(ctx,sizeof(*ctx));

  AutoDestroyBSContext bs;
  if (!apWindow) {
    // our hackish path to create a context to detect the WGL extensions (I'm
    // about to cry right now... how can this API be so bad...)
    ctx->mhWnd = CreateWindowA("STATIC"
                               , ""
                               , WS_POPUP|WS_DISABLED
                               , -32000
                               , -32000
                               , 0
                               , 0
                               , NULL
                               , NULL
                               , GetModuleHandle(NULL)
                               , 0
                               );
  }
  else {
    if (bs.Create()) {
      _wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
      if (_wglGetExtensionsStringARB) {
        const char* extensions = (const char*)_wglGetExtensionsStringARB(ctx->mhDC);
        niDebugFmt(("--- WGL Extensions ---\n%s",extensions));
      }
      _wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    }
    ctx->mhWnd = (HWND)apWindow->GetHandle();
  }

  ctx->mhDC = GetDC(ctx->mhWnd);
  if (!ctx->mhDC) {
    return TSGL_ERR_INVALIDHDC;
  }

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(pfd));
  int pf = 0;

  // try with an extended context
  if (!pf && _wglChoosePixelFormatARB) {
    tU32 detectedAA = 0;
    UINT numFormats = 0;

    // try AA
    if (anAA) {
      int attrsAA[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, anColorBits,
        WGL_DEPTH_BITS_ARB, anDepthBits,
        WGL_STENCIL_BITS_ARB, anStencilBits,
        WGL_SAMPLE_BUFFERS_ARB, 1, //Number of buffers (must be 1 at time of writing)
        WGL_SAMPLES_ARB, 4,        //Number of samples
        0
      };
      BOOL result = _wglChoosePixelFormatARB(ctx->mhDC, attrsAA, NULL, 1, &pf, &numFormats);
      if (!result || !numFormats) {
        pf = 0;
      }
      else {
        detectedAA = attrsAA[niCountOf(attrsAA)-2];
      }
    }

    // without AA
    if (!pf) {
      int attrs[] = {
        WGL_SAMPLE_BUFFERS_ARB, 0,
        WGL_SAMPLES_ARB, 0,
        WGL_SUPPORT_OPENGL_ARB, true,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_DRAW_TO_WINDOW_ARB, true,
        WGL_DOUBLE_BUFFER_ARB, true,
        WGL_COLOR_BITS_ARB, anColorBits,
        WGL_DEPTH_BITS_ARB, anDepthBits,
        WGL_STENCIL_BITS_ARB, anStencilBits,
        0
      };

      do
      {
        BOOL result = _wglChoosePixelFormatARB(ctx->mhDC, attrs, NULL, 1, &pf, &numFormats);
        if (!result || !numFormats) {
          attrs[3] >>= 1;
          attrs[1] = attrs[3] == 0 ? 0 : 1;
        }
      } while (!numFormats);
    }

    if (!DescribePixelFormat(ctx->mhDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd)) {
      memset(&pfd, 0, sizeof(pfd));
      pf = 0;
    }
    else {
      niDebugFmt((
          "WGL ARB PixelFormat: iPixelType: %d, cColorBits: %d, cAlphaBits: %d, cDepthBits: %d, cStencilBits: %d, AA: %d"
          , pfd.iPixelType
					, pfd.cColorBits
					, pfd.cAlphaBits
					, pfd.cDepthBits
					, pfd.cStencilBits
          , detectedAA));
    }
  }

  // initialize with the standard pixel format every time, we need to do that
  // because we can't use wglGetProcAddress before there's a context... warf wtf... reallly ???
  if (!pf) {
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = (BYTE)anColorBits;
    pfd.cDepthBits = (BYTE)anDepthBits;
    pfd.cStencilBits = (BYTE)anStencilBits;
    pfd.iLayerType   = PFD_MAIN_PLANE;
    pf = ChoosePixelFormat(ctx->mhDC, &pfd);

    niDebugFmt((
        "WGL Default PixelFormat: iPixelType: %d, cColorBits: %d, cAlphaBits: %d, cDepthBits: %d, cStencilBits: %d"
        , pfd.iPixelType
        , pfd.cColorBits
        , pfd.cAlphaBits
        , pfd.cDepthBits
        , pfd.cStencilBits));
  }
  if (!pf) {
    ctx->mhWnd = NULL;
    return TSGL_ERR_INVALIDPXF;
  }

  if (!SetPixelFormat(ctx->mhDC, pf, &pfd)) {
    ctx->mhWnd = NULL;
    return TSGL_ERR_SETPXF;
  }

  ctx->mhGLRC = wglCreateContext(ctx->mhDC);
  if (!ctx->mhGLRC) {
    return TSGL_ERR_CREATERC;
  }

#    elif defined niLinux
  tsglContext* ctx = (tsglContext*)TSGL_MALLOC(sizeof(tsglContext));
  if (!ctx) {
    TSGL_RETURN_ERROR(ALLOCCTX,"Can't allocate memory for TSGL context.");
  }
  TSGL_ZEROMEMORY(ctx,sizeof(*ctx));
  ctx->mpWindowDesc = (sXWindowDesc*)apWindow->GetHandle();
#    endif

  *appCtx = ctx;
  return TSGL_OK;
}

void tsglDestroyContext(tsglContext* apCtx)
{
#    ifdef __APPLE__
  if (apCtx->mpWindow) {
    niOSX::osxglDestroyContext(apCtx->mpWindow);
    apCtx->mbInitialized = eFalse;
    apCtx->mpWindow = NULL;
  }
#    elif defined _WIN32
  if (apCtx->mhGLRC) {
    wglMakeCurrent(apCtx->mhDC,apCtx->mhGLRC);
    wglDeleteContext(apCtx->mhGLRC);
    apCtx->mhGLRC = NULL;
  }
  if (apCtx->mhDC) {
    ::ReleaseDC(apCtx->mhWnd,apCtx->mhDC);
    apCtx->mhDC = NULL;
  }
#    elif defined niLinux
  apCtx->mpWindowDesc = NULL;
#    endif
  TSGL_FREE(apCtx,sizeof(*apCtx));
}

//===========================================================================
//
//  Embedded
//
//===========================================================================
#  else // TSGL_DESKTOP

int tsglCreateContext(
    tsglContext** appCtx,
    void* aOSWndHandle,
    int aDefaultWidth,
    int aDefaultHeight,
    const int* apConfig)
{
  EGLConfig cfgs[128];
  int numConfigs = 0;
  TSGL_UNUSED(aOSWndHandle);
  TSGL_UNUSED(aDefaultWidth);
  TSGL_UNUSED(aDefaultHeight);
  TSGL_UNUSED(apConfig);

  if (!tsglIsLoaded()) {
    TSGL_RETURN_ERROR(NOTLOADED,"TSGL not loaded.");
  }

  tsglContext* ctx = (tsglContext*)TSGL_MALLOC(sizeof(tsglContext));
  if (!ctx) {
    TSGL_RETURN_ERROR(ALLOCCTX,"Can't allocate memory for TSGL context.");
  }
  TSGL_ZEROMEMORY(ctx,sizeof(*ctx));

  ctx->width = aDefaultWidth;
  ctx->height = aDefaultHeight;

#    ifdef __GLES__
#      ifdef _WIN32
  {
    ctx->displayHWND = (HWND)aOSWndHandle;
    if (!ctx->displayHWND) {
      TSGL_RETURN_ERROR(INVALIDHWND,"Invalid HWND handle.");
    }
    ctx->displayHDC = GetDC(ctx->displayHWND);
    if (!ctx->displayHDC) {
      TSGL_RETURN_ERROR(INVALIDHDC,"Can't get HDC of OS Window.");
    }
    ctx->eglDisplay = _eglGetDisplay(ctx->displayHDC);
  }
#      else
  ctx->eglDisplay = _eglGetDisplay(EGL_DEFAULT_DISPLAY); /* display can be null on some platforms */
#      endif
#    endif

  if (_eglGetConfigs(ctx->eglDisplay,cfgs,128,&numConfigs)) {
    int i = 0;
    printf("- EGL Configs: %d\n",numConfigs);
    for (i = 0; i < numConfigs; ++i) {
    }
  }

  /// Android Setup
#    ifdef ANDROID
  ctx->displayWindow = _android_createDisplaySurface();
  if (!ctx->displayWindow) {
    TSGL_FREE(ctx,sizeof(*ctx));
    TSGL_RETURN_ERROR(DISPWND,"Can't create Android display surface.");
  }

  printf("- Android Window specs: %d*%d (def: %d*%d) format=%d\n",
         ctx->displayWindow->width,
         ctx->displayWindow->height,
         aDefaultWidth,aDefaultHeight,
         ctx->displayWindow->format);

  if (ctx->displayWindow->width == 0) {
    ctx->displayWindow->width = aDefaultWidth;
  }
  if (ctx->displayWindow->height == 0) {
    ctx->displayWindow->height = aDefaultHeight;
  }
  ctx->width = ctx->displayWindow->width;
  ctx->height = ctx->displayWindow->height;

  printf("- TSGL Context: %d*%d\n",ctx->width,ctx->height);
#    endif

  /// ST SH4 Setup
#    ifdef niSuperH
  ctx->fbdevWindow.width = aDefaultWidth;
  ctx->fbdevWindow.height = aDefaultHeight;
#    endif

#    ifdef __GLES__
  /// Initialize EGL
  _eglInitialize(ctx->eglDisplay,&ctx->majorVersion,&ctx->minorVersion);
  if (ctx->majorVersion == 0 && ctx->minorVersion == 0) {
    TSGL_FREE(ctx,sizeof(*ctx));
    TSGL_RETURN_ERROR(EGLINIT,"Can't initialize EGL.");
  }

#      if !defined __TSGL_NOBINDAPI__
  /// Bind the OpenGL ES API
  eglBindAPI(EGL_OPENGL_ES_API);
#      endif

  /// Find a config that matches our requirements
  if (!_eglChooseConfig(ctx->eglDisplay,apConfig,&ctx->eglConfig,1,&numConfigs)) {
    TSGL_FREE(ctx,sizeof(*ctx));
    TSGL_RETURN_ERROR(CHOOSECFG,"Can't choose EGL config.");
  }

  /// Create a surface to draw to
  ctx->eglSurface = _eglCreateWindowSurface(
      ctx->eglDisplay,
      ctx->eglConfig,
#      ifdef _WIN32
      ctx->displayHWND,
#      endif
#      ifdef ANDROID
      ctx->displayWindow,
#      endif
#      ifdef niSuperH
      (IDirectFBSurface*)&ctx->fbdevWindow,
#      endif
      NULL);
  if (!ctx->eglSurface) {
    TSGL_FREE(ctx,sizeof(*ctx));
    TSGL_RETURN_ERROR(CREATEWND,"Can't create EGL window surface.");
  }

  /// Create the context
  {
#      ifdef __GLES2__
    int attrib_list[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
#      endif
#      ifdef __GLES1__
    int attrib_list[] = { EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE };
#      endif
    ctx->eglContext = _eglCreateContext(
        ctx->eglDisplay,
        ctx->eglConfig,
        EGL_NO_CONTEXT,
        attrib_list);
    if (!ctx->eglContext) {
      TSGL_FREE(ctx,sizeof(*ctx));
      TSGL_RETURN_ERROR(CREATECTX,"Can't create EGL context.");
    }
  }

#    endif

  *appCtx = ctx;
  return TSGL_OK;
}

void tsglDestroyContext(tsglContext* apCtx)
{
  if (apCtx->eglDisplay) {
    _eglMakeCurrent(apCtx->eglDisplay, NULL, NULL, NULL);
    if (apCtx->eglContext) {
      _eglDestroyContext(apCtx->eglDisplay,apCtx->eglContext);
      apCtx->eglContext = NULL;
    }
    if (apCtx->eglSurface) {
      _eglDestroySurface(apCtx->eglDisplay,apCtx->eglSurface);
      apCtx->eglSurface = NULL;
    }
    _eglTerminate(apCtx->eglDisplay);
    apCtx->eglDisplay = NULL;
  }
  TSGL_FREE(apCtx,sizeof(*apCtx));
}

#  endif // TSGL_DESKTOP

//===========================================================================
//
//  Context & Buffers
//
//===========================================================================

/////////////////////////////////////////////////////////////////
// Inline Implementations
void tsglMakeCurrent(tsglContext* apCtx) {
#  if defined TSGL_DESKTOP
#    ifdef niOSX
  if (apCtx->mpWindow) {
    niOSX::osxglMakeContextCurrent(apCtx->mpWindow);
  }
#    elif defined _WIN32
  wglMakeCurrent(apCtx->mhDC,apCtx->mhGLRC);
#    elif defined niLinux
  glXMakeCurrent((Display*)apCtx->mpWindowDesc->mpDisplay,
                 (Window)apCtx->mpWindowDesc->mpWindow,
                 (GLXContext)apCtx->mpWindowDesc->mpGLX);
#    else
#      error "tsglMakeCurrent - Unsupported platform."
#    endif
#  else
#    ifdef niMSVC
  try {
    _eglMakeCurrent(apCtx->eglDisplay, apCtx->eglSurface, apCtx->eglSurface, apCtx->eglContext);
  }
  catch (...) {
  }
#    else
  _eglMakeCurrent(apCtx->eglDisplay, apCtx->eglSurface, apCtx->eglSurface, apCtx->eglContext);
#    endif
#  endif
}

void tsglSwapBuffers(tsglContext* apCtx, tBool abDoNotWait) {
#  if defined TSGL_DESKTOP
#    ifdef niOSX
  if (apCtx->mpWindow) {
    niOSX::osxglSwapBuffers(apCtx->mpWindow, abDoNotWait);
  }
#    elif defined niWindows
  wglMakeCurrent(apCtx->mhDC,apCtx->mhGLRC);
  ::SwapBuffers(apCtx->mhDC);
#    elif defined niLinux
  glXSwapBuffers((Display*)apCtx->mpWindowDesc->mpDisplay,(Window)apCtx->mpWindowDesc->mpWindow);
#    else
#      error "tsglSwapBuffers - Unsupported platform."
#    endif
#  else
  _eglSwapBuffers(apCtx->eglDisplay, apCtx->eglSurface);
#  endif
}

#endif
