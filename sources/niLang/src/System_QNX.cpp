#include "API/niLang/Types.h"

#ifdef niQNX
#include "API/niLang/IOSWindow.h"
#include "Lang.h"
#include "API/niLang_ModuleDef.h"
#include "API/niLang/IOSWindow.h"
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/Utils/TimerSleep.h>

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#include <sys/neutrino.h>
#include <screen/screen.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

using namespace ni;

class cQNXWindow : public ni::cIUnknownImpl<ni::iOSWindow,ni::eIUnknownImplFlags_Default,ni::iOSWindowQNX> {
  niBeginClass(cQNXWindow);

  public:
  cQNXWindow(sVec2i aSize)
  {
    mCursor = eOSCursor_Arrow;
    mbRequestedClose = eFalse;
    mbMouseOverClient = eFalse;
    mbDropTarget = eFalse;
    mfRefreshTimer = -1;
    mbIsActive = eFalse;

    mptrMT = tMessageHandlerSinkLst::Create();
    SetCursor(eOSCursor_Arrow);

    const EGLint s_configAttribs[] =
    {
	    EGL_RED_SIZE,   8,
      EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE,  8,
	    EGL_ALPHA_SIZE, 8,
	    EGL_DEPTH_SIZE, 24,
	    EGL_STENCIL_SIZE, 8,
	    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	    EGL_SAMPLE_BUFFERS, 0,
	    EGL_NONE
    };

    const EGLint eglSurfaceAttrs[] =
    {
      EGL_RENDER_BUFFER,  EGL_BACK_BUFFER,
      EGL_NONE
    };

    const EGLint eglContextAttribList[] =
    {
	    EGL_CONTEXT_CLIENT_VERSION, 2,
	    EGL_NONE
    };

    mRect = Recti(0,0,aSize.x,aSize.y);
    int size[2] = { aSize.x, aSize.y };
    int format = SCREEN_FORMAT_RGBA8888;
    int usage =  SCREEN_USAGE_OPENGL_ES2;
    int interval = 1;

    screen_create_context(&screen_ctx, 0);
    screen_create_window(&screen_win, screen_ctx);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SOURCE_SIZE, size);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SIZE, size);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SWAP_INTERVAL, &interval);
    screen_create_window_buffers(screen_win, 1);
    // screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)screen_buf);

    egldisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(eglGetError() == EGL_SUCCESS);

    eglInitialize(egldisplay, NULL, NULL);
    assert(eglGetError() == EGL_SUCCESS);

    eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(numconfigs == 1);

    eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, screen_win, eglSurfaceAttrs);

    eglcontext = eglCreateContext(egldisplay, eglconfig, EGL_NO_CONTEXT, eglContextAttribList);

    auto ret = eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
    if (ret == EGL_FALSE)
    {
      niError(niFmt("eglMakeCurrent failed:%s\n", ret));
      return;
    }

    if (interval)
    {
      eglSwapInterval(egldisplay, interval);
    }
  }

  ~cQNXWindow() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const niImpl {
    niClassIsOK(cQNXWindow);
    return screen_win != NULL;
  }

  ///////////////////////////////////////////////
  iOSWindow* __stdcall GetParent() const niImpl {
    return NULL;
  }

  tIntPtr __stdcall GetPID() const niImpl {
    return (tIntPtr)0;
  }

  void* __stdcall GetScreenHandle() const {
    return screen_win;
  }

    ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetHandle() const niImpl {
    return (tIntPtr)screen_win;
  }
  virtual tBool __stdcall GetIsHandleOwned() const niImpl {
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall SetClientAreaWindow(tIntPtr aHandle) niImpl {
  }
  tIntPtr __stdcall GetClientAreaWindow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall ActivateWindow() niImpl {
  }

  virtual tBool __stdcall SwitchIn(tU32 anReason) niImpl {
    _SendMessage(eOSWindowMessage_SwitchIn,anReason);
    return eTrue;
  }

  virtual tBool __stdcall SwitchOut(tU32 anReason) niImpl {
    _SendMessage(eOSWindowMessage_SwitchOut,anReason);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsActive() const niImpl {
    return mbIsActive;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTitle(const achar* aaszTitle) niImpl {
  }
  virtual const achar* __stdcall GetTitle() const niImpl {
    return NULL;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetStyle(tOSWindowStyleFlags aStyle) niImpl {
  }
  virtual tOSWindowStyleFlags __stdcall GetStyle() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetShow(tOSWindowShowFlags aShow) niImpl {
  }
  virtual tOSWindowShowFlags __stdcall GetShow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetZOrder(eOSWindowZOrder aZOrder) niImpl {
  }
  virtual eOSWindowZOrder __stdcall GetZOrder() const niImpl {
    return eOSWindowZOrder_Normal;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSize(const sVec2i& avSize) niImpl {
    sRecti rect = GetRect();
    rect.SetSize(avSize);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetSize() const niImpl {
    return GetRect().GetSize();
  }
  virtual void __stdcall SetPosition(const sVec2i& avPos) niImpl {
    sRecti rect = GetRect();
    rect.SetTopLeft(avPos);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetPosition() const niImpl {
    return sVec2i::Zero();
  }
  virtual void __stdcall SetRect(const sRecti& aRect) niImpl {
  }
  virtual sRecti __stdcall GetRect() const niImpl {
    return mRect;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetClientSize(const sVec2i& avSize) niImpl {
    return SetSize(avSize);
  }
  virtual sVec2i __stdcall GetClientSize() const niImpl {
    return GetSize();
  }

  ///////////////////////////////////////////////
  virtual tF32 __stdcall GetContentsScale() const {
    return 1.0f;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Clear() niImpl {
  }

  ///////////////////////////////////////////////
  virtual tMessageHandlerSinkLst* __stdcall GetMessageHandlers() const niImpl {
    return mptrMT;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall UpdateWindow(tBool abBlockingMessages) niImpl {
    // const tU32 numEvents = _PumpEvents();
    this->_SendMessage(eOSWindowMessage_Paint);
    eglSwapBuffers(egldisplay, eglsurface);
    return eFalse;
  }

  virtual tBool __stdcall RedrawWindow() niImpl {
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall CenterWindow() niImpl {
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetRequestedClose() const niImpl {
    return mbRequestedClose;
  }
  virtual void __stdcall SetRequestedClose(tBool abRequested) niImpl {
    mbRequestedClose = abRequested;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCursor(eOSCursor aCursor) niImpl {
    if (mCursor == aCursor)
      return;
    mCursor = aCursor;
  }

  virtual eOSCursor __stdcall GetCursor() const niImpl {
    return mCursor;
  }

  void _UpdateCursor(eOSCursor aCursor) {
  }

  virtual tBool __stdcall InitCustomCursor(tIntPtr aID, tU32 anWidth, tU32 anHeight, tU32 anPivotX, tU32 anPivotY, const tU32* apData) niImpl {
    return eTrue;
  }
  virtual tIntPtr __stdcall GetCustomCursorID() const niImpl {
    return 0;
  }

  virtual void __stdcall SetCursorPosition(const sVec2i& avCursorPos) niImpl {
  }
  virtual sVec2i __stdcall GetCursorPosition() const niImpl {
    return sVec2i::Zero();
  }
  virtual void __stdcall SetCursorCapture(tBool abCapture) niImpl {
  }
  virtual tBool __stdcall GetCursorCapture() const niImpl {
    return eFalse;
  }
  virtual tBool __stdcall GetIsCursorOverClient() const niImpl {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall TryClose() niImpl {
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDropTarget(tBool abDropTarget) niImpl
  {
  }

  virtual tBool __stdcall GetDropTarget() const niImpl
  {
    return false;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetMonitor() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetFullScreen(tU32 anScreenId) niImpl {
    return eFalse;
  }
  tU32 __stdcall GetFullScreen() const niImpl {
    return 1;
  }
  tBool __stdcall GetIsMinimized() const niImpl {
    return eFalse;
  }
  tBool __stdcall GetIsMaximized() const niImpl {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFocus() niImpl {
  }
  virtual tBool __stdcall GetHasFocus() const niImpl {
    return mbIsActive;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetRefreshTimer(tF32 afRefreshTimer) niImpl {
    mfRefreshTimer = -1;
  }
  virtual tF32 __stdcall GetRefreshTimer() const niImpl {
    return mfRefreshTimer;
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetParentHandle() const niImpl {
    return 0;
  }
  virtual tU32 __stdcall IsParentWindow(tIntPtr aHandle) const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall AttachGraphicsAPI(iOSGraphicsAPI* apAPI) {
    niCheckIsOK(apAPI,eFalse);

    return eTrue;
  }
  virtual iOSGraphicsAPI* __stdcall GetGraphicsAPI() const {
    return mptrAPI;
  }

  ///////////////////////////////////////////////
  tBool _SendMessage(eOSWindowMessage aMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) {
    return ni::SendMessages(mptrMT,aMsg,avarA,avarB);
  }

  screen_context_t screen_ctx;
  screen_window_t screen_win;
  EGLDisplay egldisplay;
  EGLConfig  eglconfig;
  EGLSurface eglsurface;
  EGLContext eglcontext;
  EGLint numconfigs;

  Ptr<tMessageHandlerSinkLst> mptrMT;
  tBool                       mbRequestedClose;
  tBool                       mbMouseOverClient;
  tBool                       mbDropTarget;
  tBool                       mbIsActive;
  tF32                        mfRefreshTimer;
  sRecti                      mRect;

  eOSCursor        mCursor;
  Ptr<iOSGraphicsAPI> mptrAPI;
  // screen_buffer_t screen_buf[2];

  niEndClass(cQNXWindow);
};

iOSWindow* __stdcall cLang::CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle) {
  niCheck(aRect.GetWidth() > 0,NULL);
  niCheck(aRect.GetWidth() < 0xFFFF,NULL);
  niCheck(aRect.GetHeight() > 0,NULL);
  niCheck(aRect.GetHeight() < 0xFFFF,NULL);

  Ptr<cQNXWindow> wnd = niNew cQNXWindow(aRect.GetSize());
  return wnd.GetRawAndSetNull();
}

#endif
