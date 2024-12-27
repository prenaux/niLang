#include "API/niLang/Types.h"

#ifdef niLinuxDesktop
#include "API/niLang/IOSWindow.h"
#include "Lang.h"
#include "API/niLang_ModuleDef.h"
#include "API/niLang/IOSWindow.h"
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/Utils/TimerSleep.h>
#include <niLang/STL/scope_guard.h>
#include <niLang/Utils/DLLLoader.h>
#include <niLang/Platforms/Linux/linuxgl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h> // default cursors defined here...
#include <X11/Xresource.h>
#include <GL/glx.h>

using namespace ni;

#define USE_X11_IM

static const int XA_CARDINAL = ((Atom) 6);
static const int MOUSE_WARP_DELAY = 200;
static const int knMinXwinWidth = 20;
static const int knMinXwinHeight = 20;
static const int knMaxXwinWidth = 50000;
static const int knMaxXwinHeight = 50000;

#define X11_MAX_MESSAGES_PER_FRAME 100

////////////////////////////////////////////////////////////////////////////
// ni_dll_load_glx
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_DECL(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_glx.h"
#undef NI_DLL_PROC

NI_DLL_BEGIN_LOADER(glx, "libGL.so");
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_LOAD(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_glx.h"
#undef NI_DLL_PROC
NI_DLL_END_LOADER(glx);

////////////////////////////////////////////////////////////////////////////
// ni_dll_load_x11
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_DECL(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_x11.h"
#undef NI_DLL_PROC

NI_DLL_BEGIN_LOADER(x11,"libX11.so");
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_LOAD(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_x11.h"
#undef NI_DLL_PROC
NI_DLL_END_LOADER(x11);

////////////////////////////////////////////////////////////////////////////
static struct
{
  KeySym keysym;
  eKey scancode;
} keysym_to_scancode[] =
{
  { XK_Escape, eKey_Escape },

  { XK_F1, eKey_F1 },
  { XK_F2, eKey_F2 },
  { XK_F3, eKey_F3 },
  { XK_F4, eKey_F4 },
  { XK_F5, eKey_F5 },
  { XK_F6, eKey_F6 },
  { XK_F7, eKey_F7 },
  { XK_F8, eKey_F8 },
  { XK_F9, eKey_F9 },
  { XK_F10, eKey_F10 },
  { XK_F11, eKey_F11 },
  { XK_F12, eKey_F12 },

  { XK_Print, eKey_PrintScreen },
  { XK_Scroll_Lock, eKey_Scroll },
  { XK_Pause, eKey_Pause },

  { XK_grave, eKey_Grave },
  { XK_quoteleft, eKey_Grave },
  { XK_asciitilde, eKey_Grave },
  { XK_1, eKey_n1 },
  { XK_2, eKey_n2 },
  { XK_3, eKey_n3 },
  { XK_4, eKey_n4 },
  { XK_5, eKey_n5 },
  { XK_6, eKey_n6 },
  { XK_7, eKey_n7 },
  { XK_8, eKey_n8 },
  { XK_9, eKey_n9 },
  { XK_0, eKey_n0 },
  { XK_minus, eKey_Minus },
  { XK_equal, eKey_Equals },
  { XK_backslash, eKey_BackSlash },
  { XK_BackSpace, eKey_BackSpace },

  { XK_Tab, eKey_Tab },
  { XK_q, eKey_Q },
  { XK_w, eKey_W },
  { XK_e, eKey_E },
  { XK_r, eKey_R },
  { XK_t, eKey_T },
  { XK_y, eKey_Y },
  { XK_u, eKey_U },
  { XK_i, eKey_I },
  { XK_o, eKey_O },
  { XK_p, eKey_P },
  { XK_bracketleft, eKey_LBracket },
  { XK_bracketright, eKey_RBracket },
  { XK_Return, eKey_Enter },

  { XK_Caps_Lock, eKey_CapsLock },
  { XK_a, eKey_A },
  { XK_s, eKey_S },
  { XK_d, eKey_D },
  { XK_f, eKey_F },
  { XK_g, eKey_G },
  { XK_h, eKey_H },
  { XK_j, eKey_J },
  { XK_k, eKey_K },
  { XK_l, eKey_L },
  { XK_semicolon, eKey_Semicolon },
  { XK_apostrophe, eKey_Apostrophe },

  { XK_Shift_L, eKey_LShift },
  { XK_z, eKey_Z },
  { XK_x, eKey_X },
  { XK_c, eKey_C },
  { XK_v, eKey_V },
  { XK_b, eKey_B },
  { XK_n, eKey_N },
  { XK_m, eKey_M },
  { XK_comma, eKey_Comma },
  { XK_period, eKey_Period },
  { XK_slash, eKey_Slash },
  { XK_Shift_R, eKey_RShift },

  { XK_Control_L, eKey_LControl },
  { XK_Meta_L, eKey_LAlt },
  { XK_Alt_L, eKey_LAlt },
  { XK_space, eKey_Space },
  { XK_Alt_R, eKey_RAlt },
  { XK_Meta_R, eKey_RAlt },
  { XK_Control_R, eKey_RControl },

  { XK_Menu, eKey_LWin },

  { XK_Insert, eKey_Insert },
  { XK_Home, eKey_Home },
  { XK_Delete, eKey_Delete },
  { XK_End, eKey_End },
  { XK_Prior, eKey_PgUp },
  { XK_Next, eKey_PgDn },

  { XK_Up, eKey_Up },
  { XK_Left, eKey_Left },
  { XK_Down, eKey_Down },
  { XK_Right, eKey_Right },

  { XK_Num_Lock, eKey_NumLock },
  { XK_KP_Divide, eKey_NumPadSlash },
  { XK_KP_Multiply, eKey_NumPadSlash },
  { XK_KP_Subtract, eKey_NumPadMinus },
  { XK_KP_Add, eKey_NumPadPlus },

  { XK_KP_Home, eKey_NumPad7 },
  { XK_KP_Up, eKey_NumPad8 },
  { XK_KP_Prior, eKey_NumPad9 },
  { XK_KP_Left, eKey_NumPad4 },
  { XK_KP_Begin, eKey_NumPad5 },
  { XK_KP_Right, eKey_NumPad6 },
  { XK_KP_End, eKey_NumPad1 },
  { XK_KP_Down, eKey_NumPad2 },
  { XK_KP_Next, eKey_NumPad3 },
  { XK_KP_Enter, eKey_NumPadEnter },
  { XK_KP_Insert, eKey_NumPad0 },
  { XK_KP_Delete, eKey_NumPadPeriod },

  { NoSymbol, eKey_Unknown },
};

struct sX11System : public Impl_HeapAlloc {
  tBool mbIsLoaded;
  struct sX11Monitor {
    tIntPtr mHandle;
    cString mstrName;
    sRecti mrectMonitor;
    tOSMonitorFlags mFlags;
  };
  astl::vector<sX11Monitor> mvMonitors;
  tF32 mfContentsScale = 1.0f;

  sX11System() {
    mbIsLoaded = ni_dll_load_x11();
    niCheck(mbIsLoaded, ;);

    Display* display = this->OpenDisplay();
    if (display) {
      int screenCount = dll_XScreenCount(display);
      niLog(Info, niFmt("XScreenCount found '%d' screens.", screenCount));
      niLoop(i, screenCount) {
        _AddMonitor(display, i);
      }
      mfContentsScale = _GetSystemContentsScale(display);
      dll_XCloseDisplay(display);
    }
    else {
      niLog(Info, "X11_OpenDisplay failed.");
    }

    niLog(Info, niFmt("X11 ContentsScale: %g.", mfContentsScale));
  }

  tBool IsOK() const {
    return mbIsLoaded;
  }

  void _AddMonitor(Display* display, int screen) {
    sX11Monitor m;
    m.mHandle = screen;
    m.mstrName.Format(_A("Screen%d"), screen);

    Screen* scr = ScreenOfDisplay(display, screen);
    m.mrectMonitor = ni::sRecti(0, 0, scr->width, scr->height);

    m.mFlags = 0;
    if (screen == DefaultScreen(display)) {
      m.mFlags |= eOSMonitorFlags_Primary;
    }

    mvMonitors.push_back(m);
    niLog(Info, niFmt(
      "X11: Monitor %d: ID:%X name:'%s' rect:%s flags:%d\n",
      mvMonitors.size()-1,
      m.mHandle, m.mstrName.Chars(),
      m.mrectMonitor,
      m.mFlags));
  }

  Display* OpenDisplay() {
    cString displayName = ni::GetProperty("X11.Display", nullptr);
    return dll_XOpenDisplay(displayName.IsNotEmpty() ? displayName.data() : nullptr);
  }

  static tF32 _GetSystemContentsScale(Display* disp) {
    float dpi = -1.0f;
    char* rms = dll_XResourceManagerString(disp);
    if (rms) {
      XrmDatabase db = dll_XrmGetStringDatabase(rms);
      if (db) {
        const char* valueString = nullptr;
        {
          XrmValue value;
          char* type;
          if (!dll_XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value)) {
            valueString = value.addr;
          }
          else if (dll_XrmGetResource(db, "Xft.dpi", "String", &type, &value)) {
            valueString = value.addr;
          }
        }
        if (valueString) {
          dpi = StringToDouble(valueString, 0, nullptr, eStringToDoubleFlags_Default, -1.0, -1);
        }
        dll_XrmDestroyDatabase(db);
      }
    }
    if (dpi < 10.0f || dpi > 1000.0f) { // cull non-sensical values
      return 1.0f;
    }
    else {
      return dpi / 96.f;
    }
  }

  void SetHints(Display* disp, Window win, int screen, int w, int h, tU32 flags)
  {
    XSizeHints *hints;

    hints = dll_XAllocSizeHints();
    if (hints) {
      if (flags & eOSWindowStyleFlags_FixedSize) {
        hints->min_width = hints->max_width = w;
        hints->min_height = hints->max_height = h;
      }
      else {
        hints->min_width = knMinXwinWidth;
        hints->min_height = knMinXwinHeight;
        hints->max_width = knMaxXwinWidth;
        hints->max_height = knMaxXwinHeight;
      }
      hints->flags = PMaxSize | PMinSize;
      if (flags & eOSWindowStyleFlags_FullScreen) {
        hints->x = 0;
        hints->y = 0;
        hints->flags |= USPosition;
      }

      dll_XSetWMNormalHints(disp, win, hints);
      dll_XFree(hints);
    }

    // Respect the window caption style
    if (flags & eOSWindowStyleFlags_Overlay) {
      tBool set = eFalse;
      Atom WM_HINTS;

      // First try to set MWM hints
      WM_HINTS = dll_XInternAtom(disp, "_MOTIF_WM_HINTS", True);
      if ( WM_HINTS != None ) {
        // Hints used by Motif compliant window managers
        struct {
          unsigned long flags;
          unsigned long functions;
          unsigned long decorations;
          long input_mode;
          unsigned long status;
        } MWMHints = { (1L << 1), 0, 0, 0, 0 };

        dll_XChangeProperty(disp, win,
                            WM_HINTS, WM_HINTS, 32,
                            PropModeReplace,
                            (unsigned char *)&MWMHints,
                            sizeof(MWMHints)/sizeof(long));
        set = eTrue;
      }
      // Now try to set KWM hints
      WM_HINTS = dll_XInternAtom(disp, "KWM_WIN_DECORATION", True);
      if ( WM_HINTS != None ) {
        long KWMHints = 0;
        dll_XChangeProperty(disp, win,
                            WM_HINTS, WM_HINTS, 32,
                            PropModeReplace,
                            (unsigned char *)&KWMHints,
                            sizeof(KWMHints)/sizeof(long));
        set = eTrue;
      }
      // Now try to set GNOME hints
      WM_HINTS = dll_XInternAtom(disp, "_WIN_HINTS", True);
      if ( WM_HINTS != None ) {
        long GNOMEHints = 0;
        dll_XChangeProperty(disp, win,
                            WM_HINTS, WM_HINTS, 32,
                            PropModeReplace,
                            (unsigned char *)&GNOMEHints,
                            sizeof(GNOMEHints)/sizeof(long));
        set = eTrue;
      }
      // Finally set the transient hints if necessary
      if (!set) {
        dll_XSetTransientForHint(disp, win, screen);
      }
    } else {
      tBool set = eFalse;
      Atom WM_HINTS;

      // First try to unset MWM hints
      WM_HINTS = dll_XInternAtom(disp, "_MOTIF_WM_HINTS", True);
      if ( WM_HINTS != None ) {
        dll_XDeleteProperty(disp, win, WM_HINTS);
        set = eTrue;
      }
      // Now try to unset KWM hints
      WM_HINTS = dll_XInternAtom(disp, "KWM_WIN_DECORATION", True);
      if ( WM_HINTS != None ) {
        dll_XDeleteProperty(disp, win, WM_HINTS);
        set = eTrue;
      }
      // Now try to unset GNOME hints
      WM_HINTS = dll_XInternAtom(disp, "_WIN_HINTS", True);
      if ( WM_HINTS != None ) {
        dll_XDeleteProperty(disp, win, WM_HINTS);
        set = eTrue;
      }
      // Finally unset the transient hints if necessary
      if (!set) {
        // NOTE: Does this work ?
        dll_XSetTransientForHint(disp, win, None);
      }
    }
  }
};

///////////////////////////////////////////////
static sX11System* _GetX11System() {
  static sX11System* _system = niNew sX11System();
  return _system;
}

///////////////////////////////////////////////
class cLinuxWindow : public ni::ImplRC<ni::iOSWindow,ni::eImplFlags_Default,ni::iOSWindowLinux> {
  niBeginClass(cLinuxWindow);

 public:
  cLinuxWindow(sVec2i aSize)
  {
    sX11System* x11 = _GetX11System();
    niCheck(x11->IsOK(), ;);

    mbRequestedClose = eFalse;
    mnStyle = eOSWindowStyleFlags_Regular;
    mrectWindow.Set(5,5,105,105);
    mbOwnedHandle = eFalse;
    mbIsActive = eFalse;
    mbMouseOverClient = eFalse;
    mbMouseCapture = eFalse;
    mpDisplay = nullptr;
    mpVisual = nullptr;
    mnScreen = 0;
    mHandle = 0;
    mGC = nullptr;
    mCursor = None;
    mCursorNone = None;
    mnCursorShape = 0;
    mvPrevMousePos = Vec2i(eInvalidHandle,eInvalidHandle);
    mvPrevMouseDelta = sVec2i::Zero();
    mbDropTarget = eFalse;
    mfRefreshTimer = -1;
    mptrMT = tMessageHandlerSinkLst::Create();
    mnEatRelativeMouseMove = 0;
    mWindowFullscreenState = {};

    // Init XWindow display
    {
      mpDisplay = x11->OpenDisplay();
      niCheck(mpDisplay != nullptr,;);
      mnScreen = DefaultScreen(mpDisplay);
      mpVisual = DefaultVisual(mpDisplay,mnScreen);
    }

    // Init IM
#ifdef USE_X11_IM
    {
      mIM = dll_XOpenIM(mpDisplay, nullptr, nullptr, nullptr);
      if (!mIM) {
        niWarning("XIM: XOpenIM failed.");
      }
    }
#endif

    // Init custom protocol
    {
      // Look up some useful Atoms
      WM_DELETE_WINDOW = dll_XInternAtom(mpDisplay, "WM_DELETE_WINDOW", False);
    }

    // Create default window
    {
      // Get white and black reference colors
      tU32 black = BlackPixel(mpDisplay,mnScreen);
      tU32 white = WhitePixel(mpDisplay,mnScreen);

      XSetWindowAttributes attr;
      memset(&attr,0,sizeof(attr));
      attr.override_redirect = True;
      attr.border_pixel = black;
      attr.background_pixel = white;
      tU32 mask = CWBackPixel | CWBorderPixel /*| CWColormap*/;

      // Create the window
      mHandle = dll_XCreateWindow(
        mpDisplay, DefaultRootWindow(mpDisplay),
        mrectWindow.x,mrectWindow.y,mrectWindow.GetWidth(),mrectWindow.GetHeight(),
        0,
        CopyFromParent,
        InputOutput,
        mpVisual,
        mask,
        &attr);
      niCheck(mHandle != 0,;);

      // Set the window's title
      this->SetTitle("niApp");

      // Create the GC
      mGC = dll_XCreateGC(mpDisplay, mHandle, 0, 0);
      niCheck(mGC != 0,;);

      // Set foreground and background colors
      dll_XSetBackground(mpDisplay, mGC, white);
      dll_XSetForeground(mpDisplay, mGC, black);

      // Create invisible X cursor
      Pixmap pixmap = dll_XCreatePixmap(mpDisplay,mHandle,1,1,1);
      if (pixmap != None) {
        tU32 gcmask = GCFunction | GCForeground | GCBackground;

        XGCValues gcvalues;
        gcvalues.function = GXcopy;
        gcvalues.foreground = 0;
        gcvalues.background = 0;

        GC temp_gc = dll_XCreateGC(mpDisplay, pixmap, gcmask, &gcvalues);
        dll_XDrawPoint(mpDisplay, pixmap, temp_gc, 0, 0);
        dll_XFreeGC(mpDisplay, temp_gc);

        XColor color;
        color.pixel = 0;
        color.red = color.green = color.blue = 0;
        color.flags = DoRed | DoGreen | DoBlue;

        mCursorNone = dll_XCreatePixmapCursor(mpDisplay, pixmap, pixmap,
                                              &color, &color, 0, 0);
        dll_XFreePixmap(mpDisplay, pixmap);
      }

      SetCursor(eOSCursor_Arrow);

      // Set the window size
      this->SetSize(aSize);

      // Clear the window and bring it to the top
      dll_XClearWindow(mpDisplay, mHandle);
      dll_XMapRaised(mpDisplay, mHandle);

      tU32 selectInputMask =
          KeyPressMask | KeyReleaseMask |
          EnterWindowMask | LeaveWindowMask |
          FocusChangeMask | ExposureMask |
          ButtonPressMask | ButtonReleaseMask |
          PointerMotionMask | PropertyChangeMask |
          StructureNotifyMask;

#ifdef USE_X11_IM
      if (mIM) {
        mIC = dll_XCreateIC(mIM, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, mHandle, NULL);
        if (mIC == nullptr) {
          niWarning("XIM: Can't open IC.");
        }
        else {
          tU32 mask = 0;
          if (!dll_XGetICValues(mIC, XNFilterEvents, &mask, NULL)) {
            niLog(Info, niFmt("XIM: Got IM mask of %x.", mask));
            selectInputMask |= mask;
          }
          else {
            niLog(Warning, niFmt("XIM: Could not get an IM mask.", mask));
          }
          dll_XSetICFocus(mIC);
        }
      }
#endif

      // Set the wanted inputs
      dll_XSelectInput(mpDisplay, mHandle, selectInputMask);


      // Wait for the first exposure event
      XEvent event;
      do {
        dll_XNextEvent(mpDisplay, &event);
      } while((event.type != Expose) || (event.xexpose.count != 0));

      dll_XSetWMProtocols(mpDisplay, mHandle, &WM_DELETE_WINDOW, 1);

      this->_UpdateStyle();
    }

    _InitKeyboard();
  }

  ~cLinuxWindow() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  tBool __stdcall _GLCreateContext() {
    niCheck(ni_dll_load_glx(), eFalse);
    niCheck(mpGLX == nullptr, eFalse);

    GLint attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XVisualInfo* xvi = dll_glXChooseVisual(mpDisplay, 0, attr);
    if (!xvi) {
      niError("glXChooseVisual failed.");
      return eFalse;
    }
    niLog(Info, niFmt("glXChooseVisual: %p",(void*)xvi)); // same output as glxinfo

    mpGLX = dll_glXCreateContext(mpDisplay, xvi, NULL, GL_TRUE);
    if (!mpGLX) {
      niError("glXCreateContext failed.");
      return eFalse;
    }

    dll_glXMakeCurrent(mpDisplay, mHandle, mpGLX);
    return eTrue;
  }
  tBool __stdcall _GLDestroyContext() {
    if (!mpGLX) {
      return eFalse;
    }
    dll_glXMakeCurrent(mpDisplay, None, nullptr);
    dll_glXDestroyContext(mpDisplay, mpGLX);
    mpGLX = nullptr;
    return eTrue;
  }
  tBool __stdcall _GLMakeCurrentContext() {
    if (!mpGLX) return eFalse;
    dll_glXMakeCurrent(mpDisplay, mHandle, mpGLX);
    return eTrue;
  }
  tBool __stdcall _GLSwapBuffers(tBool abDoNotWait) {
    niUnused(abDoNotWait);
    if (!mpGLX) return eFalse;
    dll_glXSwapBuffers(mpDisplay,mHandle);
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    _GLDestroyContext();
    if (mCursor != None) {
      dll_XUndefineCursor(mpDisplay,mHandle);
      if (mCursor != mCursorNone) {
        dll_XFreeCursor(mpDisplay,mCursor);
      }
      mCursor = None;
    }
    if (mCursorNone != None) {
      dll_XFreeCursor(mpDisplay,mCursorNone);
      mCursorNone = None;
    }
    if (mGC) {
      dll_XFreeGC(mpDisplay,mGC);
      mGC = nullptr;
    }
    if (mHandle) {
      dll_XDestroyWindow(mpDisplay,mHandle);
      mHandle = 0;
      mbOwnedHandle = eFalse;
      mbIsActive = eFalse;
    }
    if (mpDisplay) {
      mpVisual = nullptr;
      mnScreen = 0;
      dll_XCloseDisplay(mpDisplay);
      mpDisplay = nullptr;
    }
    if (mptrMT.IsOK()) {
      mptrMT->Invalidate();
      mptrMT = nullptr;
    }
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const niImpl {
    niClassIsOK(cLinuxWindow);
    return !!mHandle;
  }

  ///////////////////////////////////////////////
  iOSWindow* __stdcall GetParent() const niImpl {
    return nullptr;
  }

  tIntPtr __stdcall GetPID() const niImpl {
    return (tIntPtr)0;
  }

  void* __stdcall GetScreenHandle() const {
    return nullptr;
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetHandle() const niImpl {
    return (tIntPtr)mHandle;
  }
  virtual tBool __stdcall GetIsHandleOwned() const niImpl {
    return mbOwnedHandle;
  }

  ///////////////////////////////////////////////
  void __stdcall SetClientAreaWindow(tIntPtr aHandle) niImpl {
  }
  tIntPtr __stdcall GetClientAreaWindow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall ActivateWindow() niImpl {
    niCheckSilent(mHandle,;);
    dll_XMapRaised(mpDisplay, mHandle);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsActive() const niImpl {
    return mbIsActive;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchIn(tU32 anReason) niImpl {
    _SendMessage(eOSWindowMessage_SwitchIn,anReason);
    return eTrue;
  }
  virtual tBool __stdcall SwitchOut(tU32 anReason) niImpl {
    _SendMessage(eOSWindowMessage_SwitchOut,anReason);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTitle(const achar* aaszTitle) niImpl {
    niCheckSilent(mHandle,;);
    mstrTitle = aaszTitle;
    dll_XStoreName(mpDisplay,mHandle,mstrTitle.Chars());
  }
  virtual const achar* __stdcall GetTitle() const niImpl {
    niCheckSilent(mHandle,nullptr);
    return mstrTitle.Chars();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetStyle(tOSWindowStyleFlags aStyle) {
    niCheckSilent(mHandle,;);
    if (mnStyle == aStyle) return;
    mnStyle = aStyle;
    _UpdateStyle();
  }
  virtual tOSWindowStyleFlags __stdcall GetStyle() const {
    return mnStyle;
  }
  virtual void __stdcall _UpdateStyle() {
    sX11System* x11 = _GetX11System();
    x11->SetHints(mpDisplay,mHandle,mnScreen,
                  mrectWindow.GetWidth(),mrectWindow.GetHeight(),
                  mnStyle);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetShow(tOSWindowShowFlags aShow) niImpl {
    // TODO: IMPLEMENT
  }
  virtual tOSWindowShowFlags __stdcall GetShow() const niImpl {
    return 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetZOrder(eOSWindowZOrder aZOrder) niImpl {
    // TODO: IMPLEMENT
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
    // niDebugFmt(("... cLinuxWindow::SetPosition: rect: %s, avPos: %s", rect, avPos));
    rect.MoveTo(avPos);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetPosition() const niImpl {
    return mrectWindow.GetTopLeft();
  }
  virtual void __stdcall SetRect(const sRecti& aRect) niImpl {
    niCheckSilent(mHandle,;);

    mrectWindow = aRect;
    if (mrectWindow.GetWidth() <= knMinXwinWidth) {
      mrectWindow.SetWidth(knMinXwinWidth);
    }
    if (mrectWindow.GetHeight() <= knMinXwinHeight) {
      mrectWindow.SetHeight(knMinXwinHeight);
    }

    // niDebugFmt(("... cLinuxWindow::SetRect: mrectWindow: %s, aRect: %s", mrectWindow, aRect));
    dll_XMoveResizeWindow(mpDisplay,mHandle,
                          mrectWindow.x,mrectWindow.y,
                          mrectWindow.GetWidth(),mrectWindow.GetHeight());

    this->_UpdateStyle();
  }

  virtual sRecti __stdcall GetRect() const niImpl {
    return mrectWindow;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetClientSize(const sVec2i& avSize) niImpl {
    SetSize(avSize);
  }
  virtual sVec2i __stdcall GetClientSize() const niImpl {
    return GetSize();
  }

  ///////////////////////////////////////////////
  virtual tF32 __stdcall GetContentsScale() const {
    return _GetX11System()->mfContentsScale;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Clear() niImpl {
    niCheckSilent(mHandle,;);
    dll_XClearWindow(mpDisplay,mHandle);
  }

  ///////////////////////////////////////////////
  virtual tMessageHandlerSinkLst* __stdcall GetMessageHandlers() const niImpl {
    return mptrMT;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall UpdateWindow(tBool abBlockingMessages) niImpl {
    niCheckSilent(mHandle,eFalse);

    int events = dll_XEventsQueued(mpDisplay, QueuedAfterFlush);
    tU32 numProcessMessages = 0;
    while(events > 0) {
      _NextEvent();
      events--;
      // we allow maximum a limited number of messages
      ++numProcessMessages;
      if (numProcessMessages > X11_MAX_MESSAGES_PER_FRAME)
        break;
    }

    if (!mbIsActive) {
        return eFalse;
    }

    // TODO: IMPLEMENT
    // sVector2l curPos;
    // ::GetCursorPos((LPPOINT)&curPos);
    // ::ScreenToClient(mHandle,(LPPOINT)&curPos);
    // cRectanglel clientRect;
    // ::GetClientRect(mHandle,(LPRECT)&clientRect);
    // mbMouseOverClient = clientRect.Intersect(curPos);

    _SendMessage(eOSWindowMessage_Paint);
    return eTrue;
  }

  virtual tBool __stdcall RedrawWindow() niImpl {
    // TODO: IMPLEMENT if necessary
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall CenterWindow() niImpl {
    const tU32 nMonitor = GetMonitor();
    if (nMonitor == eInvalidHandle) {
      niWarning(niFmt("Can't center window: can't get monitor of window %x.", (tIntPtr)mHandle));
      return;
    }
    const sRecti monitorRect = ni::GetLang()->GetMonitorRect(nMonitor);
    const sRecti rect = CenterRect(monitorRect, this->GetSize());
    // niDebugFmt(("... CenterWindow: (%d) %s -> %s", nMonitor, monitorRect, rect));
    SetRect(rect);
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
    if (!mHandle) return;
    if (GetCursor() == aCursor) return;
    mnCursorShape = -1;
    dll_XUndefineCursor(mpDisplay,mHandle);
    if (mCursor != None && mCursor != mCursorNone) {
      dll_XFreeCursor(mpDisplay,mCursor);
      mCursor = None;
    }
    switch (aCursor) {
      case eOSCursor_None:
        mCursor = mCursorNone;
        dll_XDefineCursor(mpDisplay, mHandle, mCursor);
        return;
      case eOSCursor_Wait:
        mnCursorShape = XC_watch;
        break;
      case eOSCursor_ResizeHz:
        mnCursorShape = XC_sb_h_double_arrow;
        break;
      case eOSCursor_ResizeVt:
        mnCursorShape = XC_sb_v_double_arrow;
        break;
      default:
      case eOSCursor_Arrow:
        mnCursorShape = XC_left_ptr;
        break;
    }
    mCursor = dll_XCreateFontCursor(mpDisplay, mnCursorShape);
    dll_XDefineCursor(mpDisplay, mHandle, mCursor);
  }

  virtual eOSCursor __stdcall GetCursor() const niImpl {
    if (!mHandle) {
      return eOSCursor_Arrow;
    }
    if (mCursor == mCursorNone) {
      return eOSCursor_None;
    }
    switch (mnCursorShape) {
      case XC_watch: return eOSCursor_Wait;
      case XC_sb_h_double_arrow: return eOSCursor_ResizeHz;
      case XC_sb_v_double_arrow: return eOSCursor_ResizeVt;
      case XC_left_ptr: return eOSCursor_Arrow;
    }
    return eOSCursor_Arrow;
  }

  virtual sVec2i __stdcall GetCursorPosition() const niImpl {
    return mvPrevMousePos;
  }
  virtual tBool __stdcall GetIsCursorOverClient() const niImpl {
    return mbMouseOverClient;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall InitCustomCursor(tIntPtr aID, tU32 anWidth, tU32 anHeight, tU32 anPivotX, tU32 anPivotY, const tU32* apData) niImpl {
    // TODO: IMPLEMENT
    return eTrue;
  }
  virtual tIntPtr __stdcall GetCustomCursorID() const niImpl {
    // TODO: IMPLEMENT
    return 0;
  }
  Window _GetRootWindow() const {
    Display* display = _GetDisplay();
    return DefaultRootWindow(display);
  }
  virtual void __stdcall SetCursorPosition(const sVec2i& avCursorPos) niImpl {

    Display* display = _GetDisplay();
    dll_XWarpPointer(display, None, _GetWindow(),
                 0, 0, 0, 0,
                 avCursorPos.x, avCursorPos.y);
    dll_XSync(display, False);
  }
  virtual void __stdcall SetCursorCapture(tBool abCapture) niImpl {
    if (mbMouseCapture == abCapture)
      return;
    if (abCapture) {
      int grabStatus = dll_XGrabPointer(_GetDisplay(), _GetWindow(), True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                                      GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
      if (grabStatus == GrabSuccess) {
        SetCursor(ni::eOSCursor_None);
        mnEatRelativeMouseMove = 2;
      }
      else {
        niLog(Info, "X11_XGrabPointer failed to grab the mouse pointer.");
      }
    }
    else {
      if (mbMouseCapture) {
        dll_XUngrabPointer(_GetDisplay(), CurrentTime);
        // It's not worth it to restore the previous cursor unless we implement custom cursors
        SetCursor(ni::eOSCursor_Arrow);
      }
    }
    mbMouseCapture = abCapture;

    dll_XSync(_GetDisplay(), eFalse);
  }
  virtual tBool __stdcall GetCursorCapture() const niImpl {
    return mbMouseCapture;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall TryClose() niImpl {
    SetRequestedClose(eTrue);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDropTarget(tBool abDropTarget) niImpl {
  }
  virtual tBool __stdcall GetDropTarget() const niImpl {
    return false;
  }

  // This function will get the monitor which has more overlapping
  // area with the app window.
  virtual tU32 __stdcall GetMonitor() const niImpl {
    Display* display = _GetDisplay();
    if (!display) return eInvalidHandle;

    Window window = _GetWindow();
    // Check which monitor has the largest intersection with the window
    sX11System* x11 = _GetX11System();
    tU32 bestMonitor = eInvalidHandle;
    tI32 largestArea = 0;

    niLoop(i, x11->mvMonitors.size()) {
        const sRecti& monitorRect = x11->mvMonitors[i].mrectMonitor;
        if (monitorRect.IntersectRect(mrectWindow)) {
            const sRecti intersection = monitorRect.ClipRect(mrectWindow);
            const tI32 area = intersection.GetWidth() * intersection.GetHeight();
            if (area > largestArea) {
                largestArea = area;
                bestMonitor = i;
            }
        }
    }

    return bestMonitor;
  }

  tBool _GetWindowAttributes(Window window, XWindowAttributes& attrs) const {
    Display* display = _GetDisplay();
    if (!display) return eFalse;

    if (dll_XGetWindowAttributes(display, window, &attrs) == 0) {
        return eFalse;
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetFullScreen(tU32 anMonitor) niImpl {
    if (mWindowFullscreenState.fullscreenMonitor == anMonitor)
      return eTrue;  // Nothing to do.

    mWindowFullscreenState.fullscreenMonitor = anMonitor;
    Display* display = _GetDisplay();
    Window window = _GetWindow();

    if (mWindowFullscreenState.fullscreenMonitor != eInvalidHandle) {
      mWindowFullscreenState.fullscreenMonitor = anMonitor;
      // Store current window position and size for restoration
      mWindowFullscreenState.savedX = mrectWindow.x;
      mWindowFullscreenState.savedY = mrectWindow.y;
      mWindowFullscreenState.savedWidth = mrectWindow.GetWidth();
      mWindowFullscreenState.savedHeight = mrectWindow.GetHeight();

      // Switch to fullscreen
      sRecti monitorRect = ni::GetLang()->GetMonitorRect(anMonitor);

      Atom wm_state = dll_XInternAtom(display, "_NET_WM_STATE", False);
      Atom fullscreen = dll_XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

      XEvent xev = {0};
      xev.type = ClientMessage;
      xev.xclient.window = window;
      xev.xclient.message_type = wm_state;
      xev.xclient.format = 32;
      xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
      xev.xclient.data.l[1] = fullscreen;
      xev.xclient.data.l[2] = 0;

      dll_XSendEvent(display, DefaultRootWindow(display), False,
                     SubstructureNotifyMask | SubstructureRedirectMask, &xev);

      XSetWindowAttributes attSetter;
      attSetter.override_redirect = True;
      attSetter.border_pixel = 0;
      dll_XChangeWindowAttributes(display, window,
                                  CWOverrideRedirect | CWBorderPixel,
                                  &attSetter);

      dll_XMoveResizeWindow(display, window, 0, 0,
                            monitorRect.GetWidth(),
                            monitorRect.GetHeight());
    } else {
      mWindowFullscreenState.fullscreenMonitor = eInvalidHandle;
      // Restore windowed mode
      XEvent xev = {0};
      xev.type = ClientMessage;
      xev.xclient.window = window;
      xev.xclient.message_type = dll_XInternAtom(display, "_NET_WM_STATE", False);
      xev.xclient.format = 32;
      xev.xclient.data.l[0] = 0; // _NET_WM_STATE_REMOVE
      xev.xclient.data.l[1] = dll_XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

      dll_XSendEvent(display, DefaultRootWindow(display), False,
                     SubstructureNotifyMask | SubstructureRedirectMask, &xev);

      XSetWindowAttributes attSetter;
      attSetter.override_redirect = False;
      dll_XChangeWindowAttributes(display, window, CWOverrideRedirect, &attSetter);

      // Restore previous window position and size
      dll_XMoveResizeWindow(display, window,
                            mWindowFullscreenState.savedX,
                            mWindowFullscreenState.savedY,
                            mWindowFullscreenState.savedWidth,
                            mWindowFullscreenState.savedHeight);
    }

    dll_XMapRaised(display, window);
    dll_XSync(display, False);
    return true;
  }
  tU32 __stdcall GetFullScreen() const niImpl {
    return mWindowFullscreenState.fullscreenMonitor;
  }
  tBool __stdcall GetIsMinimized() const niImpl {
    XWindowAttributes attrs;
    if (_GetWindowAttributes(_GetWindow(), attrs)) {
      return (attrs.map_state == IsUnmapped || attrs.map_state == IsUnviewable);
    }
    return eFalse;
  }
  tBool __stdcall GetIsMaximized() const niImpl {
    XWindowAttributes attrs;
    if (_GetWindowAttributes(_GetWindow(), attrs)) {
      return attrs.map_state == IsViewable;
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFocus() niImpl {
    dll_XSetInputFocus(_GetDisplay(), _GetWindow(), RevertToParent, CurrentTime);
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
    mptrAttachedGraphicsAPI = apAPI;
    return mptrAttachedGraphicsAPI.IsOK();
  }
  virtual iOSGraphicsAPI* __stdcall GetGraphicsAPI() const {
    return mptrAttachedGraphicsAPI;
  }

  ///////////////////////////////////////////////
  tBool _SendMessage(eOSWindowMessage aMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) {
    return ni::SendMessages(mptrMT,aMsg,avarA,avarB);
  }

  ///////////////////////////////////////////////
  void _NextEvent(void)
  {
    if (!mpDisplay || !mHandle)
      return;

    XEvent event;
    XEvent *e = &event;
    dll_XNextEvent(mpDisplay, e);

    // filter events catches XIM events and sends them to the correct handler
    // Key press/release events are filtered in _HandleKeyEvent()
    if (e->type != KeyPress && e->type != KeyRelease) {
      if (dll_XFilterEvent(e, None)) {
        return;
      }
    }

    switch(e->type) {
      case KeyPress:
      case KeyRelease:
        _HandleKeyEvent(e);
        break;

      case FocusIn:
        mbIsActive = eTrue;
        _SendMessage(eOSWindowMessage_SwitchIn);
        break;

      case FocusOut:
        mbIsActive = eFalse;
        _SendMessage(eOSWindowMessage_SwitchOut);
        break;

      case ButtonPress:
        if (e->xbutton.button == Button4) {
          _HandleMouseWheel(1);
        }
        else if (e->xbutton.button == Button5) {
          _HandleMouseWheel(-1);
        }
        else {
          _HandleMouseButtonPress(e->xbutton.button);
        }
        break;

      case ButtonRelease:
        _HandleMouseButtonRelease(e->xbutton.button);
        break;

      case MotionNotify:
        {
            _HandleMouseMove(e->xmotion.x,e->xmotion.y);
          break;
        }

      case EnterNotify:
        {
          mbMouseOverClient = eTrue;
          niLoop(i,niCountOf(mDoubleClick)) {
            mDoubleClick[i].Reset();
          }
          break;
        }

      case LeaveNotify:
        {
          niLoop(i,niCountOf(mDoubleClick)) {
            mDoubleClick[i].Reset();
          }
          mbMouseOverClient = eFalse;
          break;
        }

      case Expose:
        // Request to redraw part of the window.
        break;

      case MappingNotify:
        // Keyboard mapping changed.
        _InitKeyboard();
        break;

      case ConfigureNotify:
        if (e->xconfigure.window == mHandle) {
          const sRecti oldRect = mrectWindow;
          const sRecti deco = _GetDecorationSizes();
          const sRecti xconfigRect = Recti(
            e->xconfigure.x,
            e->xconfigure.y,
            e->xconfigure.width,
            e->xconfigure.height);
          // Note/Todo: It isn't clear in the documentation whether this is
          // one border or both (left&right) and the WM I tested all have this
          // value at zero.
          //
          // https://tronche.com/gui/x/xlib/events/window-state-change/configure.html
          const auto xconfigBorder = e->xconfigure.border_width;
          mrectWindow.x = xconfigRect.x;
          // didnt find any documentation stating this but the y position is
          // offset by the window title's height so we adjust it so that it
          // matches our expecations.
          mrectWindow.y = xconfigRect.y-deco.GetTop();
          mrectWindow.SetWidth(xconfigRect.GetWidth()+xconfigBorder);
          mrectWindow.SetHeight(xconfigRect.GetHeight()+xconfigBorder);
          // niDebugFmt(("... ConfigureNotify: %s, old: %s, decorations: %s, xconfigRect: %s, xconfigBorder: %s",
          //             mrectWindow, oldRect, _GetDecorationSizes(), xconfigRect, xconfigBorder));
          if (mrectWindow.GetTopLeft() != oldRect.GetTopLeft()) {
            _SendMessage(eOSWindowMessage_Move);
          }
          if (mrectWindow.GetSize() != oldRect.GetSize()) {
            _SendMessage(eOSWindowMessage_Size);
          }
        }
        break;

      case ClientMessage: {
        if ((e->xclient.format == 32) && (e->xclient.data.l[0] == WM_DELETE_WINDOW)) {
          SetRequestedClose(eTrue);
          _SendMessage(eOSWindowMessage_Close);
        }
        break;
      }
    }
  }

  void _HandleMouseMove(int x, int y) {
    sVec2i vMousePos = Vec2i(x, y);
    sVec2i vRelMove = vMousePos - mvPrevMousePos;

    if (vRelMove != sVec2i::Zero()) {
      if (mnEatRelativeMouseMove <= 0) {
        _SendMessage(ni::eOSWindowMessage_RelativeMouseMove, vRelMove);
      }
      else {
        --mnEatRelativeMouseMove;
      }
    }

    if (mbMouseCapture) {
      // Only recenters when mouse goes away too much from the center
      const int centerX = mrectWindow.GetWidth() / 2;
      const int centerY = mrectWindow.GetHeight() / 2;
      const int threshold = 200; // Distance from center before recentering

      const int distFromCenterX = abs(x - centerX);
      const int distFromCenterY = abs(y - centerY);

      if (distFromCenterX > threshold || distFromCenterY > threshold) {
        SetCursorPosition(Vec2i(centerX, centerY));
        mnEatRelativeMouseMove = 2; // Skip one frame after recentering
        vMousePos = Vec2i(centerX, centerY);
      }
    }
    else {
      if (vRelMove != sVec2i::Zero()) {
        _SendMessage(ni::eOSWindowMessage_MouseMove, vMousePos, vRelMove);
      }
    }

    mvPrevMousePos = vMousePos;
  }
  void _HandleMouseWheel(int aDelta) {
    tF32 v = (tF32)aDelta;
    _SendMessage(eOSWindowMessage_MouseWheel,v);
  }
  void _HandleMouseButtonPress(int aBt) {
    switch (aBt) {
      case Button1:
        _SendMessage(eOSWindowMessage_MouseButtonDown,
                     (ni::tU32)ePointerButton_Left);
        if (mDoubleClick[0].Test(eTrue)) {
          _SendMessage(eOSWindowMessage_MouseButtonDoubleClick,
                       (ni::tU32)ePointerButton_Left);
        }
        break;
      case Button3:
        _SendMessage(eOSWindowMessage_MouseButtonDown,
                     (ni::tU32)ePointerButton_Right);
        if (mDoubleClick[2].Test(eTrue)) {
          _SendMessage(eOSWindowMessage_MouseButtonDoubleClick,
                       (ni::tU32)ePointerButton_Right);
        }
        break;
      case Button2:
        _SendMessage(eOSWindowMessage_MouseButtonDown,
                     (ni::tU32)ePointerButton_Middle);
        if (mDoubleClick[1].Test(eTrue)) {
          _SendMessage(eOSWindowMessage_MouseButtonDoubleClick,
                       (ni::tU32)ePointerButton_Middle);
        }
        break;
    }
  }
  void _HandleMouseButtonRelease(int aBt) {
    switch (aBt) {
      case Button1:
        _SendMessage(eOSWindowMessage_MouseButtonUp,
                     (ni::tU32)ePointerButton_Left);
        mDoubleClick[0].Test(eFalse);
        break;
      case Button3:
        _SendMessage(eOSWindowMessage_MouseButtonUp,
                     (ni::tU32)ePointerButton_Right);
        mDoubleClick[2].Test(eFalse);
        break;
      case Button2:
        _SendMessage(eOSWindowMessage_MouseButtonUp,
                     (ni::tU32)ePointerButton_Middle);
        mDoubleClick[1].Test(eFalse);
        break;
    }
  }

  void _InitKeyboard() {
    int i, j;
    int min_keycode;
    int max_keycode;
    KeySym keysym;

    for (i = 0; i < 256; i++) {
      // Clear mappings
      mXKeyToScan[i] = eKey_Unknown;
      // Clear pressed key flags
      mKeyPressed[i] = eFalse;
    }

    // Get the number of keycodes
    dll_XDisplayKeycodes(mpDisplay, &min_keycode, &max_keycode);
    if (min_keycode < 0)   min_keycode = 0;
    if (max_keycode > 255) max_keycode = 255;

    // Setup mappings
    for (i = min_keycode; i <= max_keycode; i++) {
      keysym = dll_XKeycodeToKeysym(mpDisplay, i, 0);
      if (keysym != NoSymbol) {
        for (j = 0; keysym_to_scancode[j].keysym != NoSymbol; j++) {
          if (keysym_to_scancode[j].keysym == keysym) {
            mXKeyToScan[i] = keysym_to_scancode[j].scancode;
            break;
          }
        }
      }
    }
  }

  void _HandleKeyEvent(XEvent* e) {
    const KeyCode xkeycode = e->xkey.keycode;

    EA_DISABLE_GCC_WARNING(-Wtautological-compare)
    EA_DISABLE_CLANG_WARNING(-Wtautological-compare)
    const eKey scode = ((xkeycode >= 0) && (xkeycode < 256)) ? mXKeyToScan[xkeycode] : eKey_Unknown;
    EA_RESTORE_CLANG_WARNING()
    EA_RESTORE_GCC_WARNING()

    auto sendKey = [&]() {
      if (scode != eKey_Unknown) {
        if (e->type == KeyPress) {
          if (!mKeyPressed[scode]) {
            mKeyPressed[scode] = eTrue;
            _SendMessage(eOSWindowMessage_KeyDown,(tU32)scode);
          }
        }
        else if (e->type == KeyRelease) {
          if (mKeyPressed[scode]) {
            mKeyPressed[scode] = eFalse;
            _SendMessage(eOSWindowMessage_KeyUp,(tU32)scode);
          }
        }
      }
    };

    if (dll_XFilterEvent(e,None)) {
      sendKey();
      return;
    }

    // Text input
    if (e->type == KeyPress &&
        // Xutf8LookupString converts those to text and we dont want that...
        (scode != eKey_Delete))
    {
      char text[64];
      text[0] = 0;
#ifdef USE_X11_IM
      if (mIC) {
        KeySym keysym;
        Status status;
        int numBytes = dll_Xutf8LookupString(mIC, &e->xkey, text, sizeof(text)-1, &keysym, &status);
        if (numBytes > 0 && (status == XLookupChars || status == XLookupBoth)) {
          text[numBytes] = '\0';
          StrCharIt itText(text);
          while (!itText.is_end()) {
            const tU32 ch = itText.next();
            if (ch >= 32) {
              _SendMessage(eOSWindowMessage_KeyChar, ch, (tU32)scode);
            }
          }
        }
      }
      else
#endif
      {
        KeySym keysym;
        int numChars = dll_XLookupString(&e->xkey, text, sizeof(text)-1, &keysym, nullptr);
        if (numChars > 0) {
          niLoop(i, numChars) {
            // only allow ascii7 characters
            const tU32 ch = text[i] > 0 ? (tU32)text[i] : 0;
            if (ch >= 32) {
              _SendMessage(eOSWindowMessage_KeyChar, ch, (tU32)scode);
            }
          }
        }
      }
    }

    sendKey();
  }

  inline Display* _GetDisplay() const { return (Display*)mpDisplay; }
  inline Visual* _GetVisual() const { return (Visual*)mpVisual; }
  inline int _GetScreen() const  { return  mnScreen; }
  inline Window _GetWindow() const { return (Window)mHandle; }
  inline GC _GetGC() const { return (GC)mGC; }

  Ptr<iOSGraphicsAPI>  mptrAttachedGraphicsAPI;
  tF32                 mfRefreshTimer;

  cString              mstrTitle;
  tBool                mbOwnedHandle;
  Ptr<tMessageHandlerSinkLst> mptrMT;
  tBool                mbIsActive;
  sRecti               mrectWindow;
  sVec2i               mvPrevMousePos;
  sVec2i               mvPrevMouseDelta;
  tBool                mbRequestedClose;
  tBool                mbMouseOverClient;
  tBool                mbDropTarget;
  tBool                mbMouseCapture;
  tOSWindowStyleFlags  mnStyle;
  Atom                 WM_DELETE_WINDOW; /* "close-window" protocol atom */
  tI32                 mnEatRelativeMouseMove;

  Display* mpDisplay;
  Visual*  mpVisual;
  int      mnScreen;
  Window   mHandle;
  GC       mGC;
  Cursor   mCursor;
  Cursor   mCursorNone;
  int      mnCursorShape;

#ifdef USE_X11_IM
  XIM mIM = nullptr;
  XIC mIC = nullptr;
#endif

  eKey  mXKeyToScan[256];
  tBool mKeyPressed[256];

  GLXContext mpGLX = nullptr;

  struct sDoubleClick {
    enum {
      NOT,
      START,
      RELEASE
    };
    tU32       status;
    tF64       timer;
    sDoubleClick() {
      Reset();
    }
    void Reset() {
      status = NOT;
      timer = ni::TimerInSeconds();
    }
    tBool Test(tBool abPressed) {
      tBool isDoubleClick = eFalse;
      if (status == NOT) {
        if (abPressed) {
          status = START;
        }
      }
      else if (status == START) {
        if (!abPressed) {
          // released for the first time
          status = RELEASE;
          timer = ni::TimerInSeconds();
        }
      }
      else if (status == RELEASE) {
        // 2nd click
        if (abPressed) {
          if ((ni::TimerInSeconds() - timer) < 0.3) {
            status = NOT;
            isDoubleClick = eTrue;
          }
          else {
            status = START;
          }
        }
        else {
          status = NOT;
        }
      }
      return isDoubleClick;
    }
  } mDoubleClick[3];

  struct sWindowFullscreenState {
    tU32 savedX = 0;
    tU32 savedY = 0;
    tU32 savedWidth = 0;
    tU32 savedHeight = 0;
    tU32 fullscreenMonitor = eInvalidHandle;
  } mWindowFullscreenState;

  // (borderLeft,titleHeight,borderRight,borderBottom)
  sRecti _GetDecorationSizes() const {
    // (borderLeft,titleHeight,borderRight,borderBottom)
    sRecti dec = sRecti::Null();
    if (!mHandle) return dec;

    // Try to get _NET_FRAME_EXTENTS property
    Atom frameExtents = dll_XInternAtom(mpDisplay, "_NET_FRAME_EXTENTS", True);
    if (frameExtents != None) {
      Atom actualType;
      int actualFormat;
      unsigned long numItems;
      unsigned long bytesAfter;
      long* extents = nullptr;

      int status = dll_XGetWindowProperty(
        mpDisplay, mHandle, frameExtents, 0, 4, False,
        XA_CARDINAL, &actualType, &actualFormat, &numItems, &bytesAfter,
        (unsigned char**)&extents);
      if (status == Success && extents && numItems >= 4) {
        dec.SetLeft(extents[0]);    // left border
        dec.SetTop(extents[2]);     // title height
        dec.SetRight(extents[1]);   // right border
        dec.SetBottom(extents[3]);  // bottom border
      }
      if (extents) {
        dll_XFree(extents);
      }
    }

    return dec;
  }

  niEndClass(cLinuxWindow);
};

niExportFunc(tBool) linuxGetOSWindowXWinHandles(iOSWindow* apWindow, sOSWindowXWinHandles& aOut) {
  niCheckIsOK(apWindow,eFalse);
  cLinuxWindow* pWindow = static_cast<cLinuxWindow*>(apWindow);
  aOut._display = pWindow->mpDisplay;
  aOut._visual = (tXWinVisual)pWindow->mpVisual;
  aOut._window = pWindow->mHandle;
  aOut._gc = pWindow->mGC;
  aOut._screen = pWindow->mnScreen;
  return eTrue;
}

niExportFunc(tBool) linuxglCreateContext(iOSWindow* apWindow) {
  cLinuxWindow* w = (cLinuxWindow*)apWindow;
  niPanicAssert(w != nullptr);
  return w->_GLCreateContext();
}
niExportFunc(tBool) linuxglDestroyContext(iOSWindow* apWindow) {
  cLinuxWindow* w = (cLinuxWindow*)apWindow;
  niPanicAssert(w != nullptr);
  return w->_GLDestroyContext();
}
niExportFunc(tBool) linuxglHasContext(iOSWindow* apWindow) {
  cLinuxWindow* w = (cLinuxWindow*)apWindow;
  niPanicAssert(w != nullptr);
  return w->mpGLX != nullptr;
}

niExportFunc(tBool) linuxglMakeContextCurrent(iOSWindow* apWindow) {
  cLinuxWindow* w = (cLinuxWindow*)apWindow;
  niPanicAssert(w != nullptr);
  return w->_GLMakeCurrentContext();
}

niExportFunc(tBool) linuxglSwapBuffers(iOSWindow* apWindow, tBool abDoNotWait) {
  cLinuxWindow* w = (cLinuxWindow*)apWindow;
  niPanicAssert(w != nullptr);
  return w->_GLSwapBuffers(abDoNotWait);
}

niExportFunc(void*) linuxglGetProcAddress(const achar* name) {
  niCheckSilent(ni_dll_load_glx(), nullptr);
  return dll_glXGetProcAddress(name);
}

iOSWindow* __stdcall cLang::CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle) {
  niCheck(aRect.GetWidth() > 0,nullptr);
  niCheck(aRect.GetWidth() < 0xFFFF,nullptr);
  niCheck(aRect.GetHeight() > 0,nullptr);
  niCheck(aRect.GetHeight() < 0xFFFF,nullptr);

  Ptr<cLinuxWindow> wnd = niNew cLinuxWindow(aRect.GetSize());
  return wnd.GetRawAndSetNull();
}

///////////////////////////////////////////////
tU32 cLang::GetNumMonitors() const {
  sX11System* x11 = _GetX11System();
  return (tU32)x11->mvMonitors.size();
}
tU32 cLang::GetMonitorIndex(tIntPtr aHandle) const {
  sX11System* x11 = _GetX11System();
  niLoop(i,x11->mvMonitors.size()) {
    if (x11->mvMonitors[i].mHandle == aHandle)
      return (tU32)i;
  }
  return eInvalidHandle;
}
tIntPtr cLang::GetMonitorHandle(tU32 anIndex) const {
  sX11System* x11 = _GetX11System();
  niCheckSilent(anIndex < x11->mvMonitors.size(),eInvalidHandle);
  return x11->mvMonitors[anIndex].mHandle;
}
const achar* cLang::GetMonitorName(tU32 anIndex) const {
  sX11System* x11 = _GetX11System();
  niCheckSilent(anIndex < x11->mvMonitors.size(),NULL);
  return x11->mvMonitors[anIndex].mstrName.Chars();
}
sRecti cLang::GetMonitorRect(tU32 anIndex) const {
  sX11System* x11 = _GetX11System();
  niCheckSilent(anIndex < x11->mvMonitors.size(),sRecti::Null());
  return x11->mvMonitors[anIndex].mrectMonitor;
}
tOSMonitorFlags cLang::GetMonitorFlags(tU32 anIndex) const {
  sX11System* x11 = _GetX11System();
  niCheckSilent(anIndex < x11->mvMonitors.size(),0);
  return x11->mvMonitors[anIndex].mFlags;
}

///////////////////////////////////////////////
void cLang::_PlatformStartup() {
}

iOSWindow* __stdcall cLang::CreateWindowEx(tIntPtr aOSWindowHandle, tOSWindowCreateFlags aCreate) {
  return NULL;
}
eOSMessageBoxReturn __stdcall cLang::MessageBox(iOSWindow* apParent, const achar* aaszTitle, const achar* aaszText, tOSMessageBoxFlags aFlags)
{
  cString strTitle = niIsStringOK(aaszTitle)?aaszTitle:"Message";
  cString strText = niIsStringOK(aaszText)?aaszText:_A("");
  niDebugFmt((_A("--- %s ---\n%s\n"),
              strTitle.Chars(),
              strText.Chars()));
  return eOSMessageBoxReturn_Yes;
}
cString __stdcall cLang::OpenFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  niError("Not implemented");
  return "<NOT IMPLEMENTED>";
}
cString __stdcall cLang::SaveFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  niError("Not implemented");
  return "<NOT IMPLEMENTED>";
}
cString __stdcall cLang::PickDirectoryDialog(iOSWindow* aParent, const achar* aTitle, const achar* aInitDir) {
  niError("Not implemented");
  return "<NOT IMPLEMENTED>";
}

///////////////////////////////////////////////
tU32 __stdcall cLang::GetNumGameCtrls() const {
  return 0;
}
iGameCtrl* __stdcall cLang::GetGameCtrl(tU32 aulIdx) const {
  return NULL;
}
#endif
