#pragma once
#ifndef __APPLIB_H_EC8785DC_A2B1_0A40_9E1A_DC4250925E56__
#define __APPLIB_H_EC8785DC_A2B1_0A40_9E1A_DC4250925E56__

#include "niAppLibConsole.h"

#include <niUI.h>
#include <niUI/Utils/WidgetSinkImpl.h>

#if defined niAndroid || defined niIOS
#  define niAppLib_Generic
#elif defined niWindows || defined niOSX || defined niLinux || defined niJSCC || defined niQNX
#  define niAppLib_Native
#else
#  error "AppLib doesn't support this platform."
#endif

namespace app {

struct AppConfig {
  ni::tHStringPtr   graphicsDriver;
  ni::tBool         drawFPS;
  ni::tU32          maxFPS;
  ni::tU32          fixedFPS;
  ni::tU32          swapInterval;
  ni::tTextureFlags backBufferFlags;
  astl::vector<astl::pair<ni::cString, ni::cString> > fonts;

  ni::tOSWindowStyleFlags windowStyle;
  ni::tBool               windowCentered;
  ni::tBool               windowShow;
  ni::sRecti              windowRect;
  ni::tF32                windowRefreshTimerForeground { 0.0f };
  ni::tF32                windowRefreshTimerBackground { -1.0f };

  ni::tBool backgroundUpdate;

  ni::tU32  clearBuffers;
  ni::tU32  clearColor;

  ni::tF32 contentsScale;

  AppConfig();

  ni::tF32 __stdcall GetContentsScale(ni::iOSWindow* apWindow) const {
    ni::tF32 windowContentsScale = apWindow ? apWindow->GetContentsScale() : 1.0f;
    return niFloatIsZero(this->contentsScale) ? windowContentsScale : this->contentsScale;
  }
};

struct AppContext : public ni::ImplRC<ni::iUnknown> {
  AppConfig                     _config;
  ni::tU32                      _appRenderCount;
  ni::Ptr<ni::iMessageHandler>  _appWnd;
  ni::Ptr<ni::iRunnable>        _appOnShutdown;
  ni::Ptr<ni::iMessageQueue>    _mq;
  ni::Ptr<ni::iOSWindow>        _window;
  ni::Ptr<ni::iGraphics>        _graphics;
  ni::Ptr<ni::iGraphicsContext> _graphicsContext;
  ni::Ptr<ni::iUIContext>       _uiContext;

  ni::tF32 __stdcall GetContentsScale() const {
    return _config.GetContentsScale(_window.raw_ptr());
  }
};

//! Initialize the application using the specified window and OnStarted runnable.
extern ni::tBool AppStartup(astl::non_null<AppContext*> apContext, ni::iOSWindow* apWindow, ni::iRunnable* apOnStarted, ni::iRunnable* apOnShutdown);
//! Shutdowns the application.
extern void AppShutdown(astl::non_null<AppContext*> apContext);

//! Update the application.
extern ni::tBool AppUpdate(astl::non_null<AppContext*> apContext);
//! Render the application.
extern void AppRender(astl::non_null<AppContext*> apContext);

#define niAppLib_SetBuildText() \
  ni::GetLang()->SetProperty("ni.app.build_text", niFmt("BUILD: %s - %s - %s", ni::GetLang()->GetProperty("ni.loa.arch"), __DATE__, __TIME__))

#ifdef niAppLib_Generic
//! Creates a generic window and then calls AppStartup with the window and the
//! specified OnStarted runnable.
extern ni::tBool AppGenericStartup(
    astl::non_null<AppContext*> apContext,
    const ni::achar* aaszTitle, ni::tI32 anWidth, ni::tI32 anHeight,
    ni::iRunnable* apOnStarted, ni::iRunnable* apOnShutdown);

//! Should be called when the app's window is resized.
extern void AppGenericResize(astl::non_null<AppContext*> apContext, ni::tI32 w, ni::tI32 h);
//! Sends keyboard event to the generic window.
extern void AppGenericKey(astl::non_null<AppContext*> apContext, ni::eKey aKey, ni::tBool abIsDown);
//! Sends text input to the generic window.
extern void AppGenericInputString(astl::non_null<AppContext*> apContext, const ni::achar* aaszString);
//! Sends a finger moved event to the generic window.
extern void AppGenericFingerMove(astl::non_null<AppContext*> apContext,
                                 ni::tI32 fingerId,
                                 ni::tF32 x, ni::tF32 y, ni::tF32 pressure);
//! Sends a finger relative move event to the generic window.
extern void AppGenericFingerRelativeMove(astl::non_null<AppContext*> apContext,
                                         ni::tI32 fingerId,
                                         ni::tF32 rx, ni::tF32 ry, ni::tF32 pressure);
//! Sends a finger press event to the generic window.
extern void AppGenericFingerPress(astl::non_null<AppContext*> apContext,
                                  ni::tI32 fingerId, ni::tBool isDown,
                                  ni::tF32 x, ni::tF32 y, ni::tF32 pressure);
//! Sends a mouse wheel event to the generic window.
extern void AppGenericMouseWheel(astl::non_null<AppContext*> apContext, ni::tF32 delta);
//! Sends a pinch gesture to the generic window.
extern void AppGenericPinch(astl::non_null<AppContext*> apContext, ni::tF32 scale, const ni::eGestureState aState);
#endif

#if defined niAppLib_Native
//! Creates a native window and then calls AppStartup with the window and the
//! specified OnStarted runnable.
ni::tBool AppNativeStartup(
    astl::non_null<AppContext*> apContext,
    const ni::achar* aaszTitle, ni::tI32 anWidth, ni::tI32 anHeight,
    ni::iRunnable* apOnStarted, ni::iRunnable* apOnShutdown);
//! A simple app main function.
//! \remark Example usage: int main() { return app::AppNativeMain(MyOnStarted); }
//! \remark This takes care of calling AppUpdate, AppRender and AppShutdown.
int AppNativeMainLoop(astl::non_null<AppContext*> apContext);
#endif

}
#endif // __APPLIB_H_EC8785DC_A2B1_0A40_9E1A_DC4250925E56__
