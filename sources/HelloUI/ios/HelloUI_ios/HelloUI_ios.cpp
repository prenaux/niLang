#include <niAppLib.h>
using namespace ni;

#define HELLOUI_INCLUDE_DATA
#include "../../cpp/HelloUI_Widget.cpp"

extern astl::non_null<app::AppContext*> GetMyAppContext();

ni::Var OnAppStarted() {
  GetMyAppContext()->_config.drawFPS = 2;
  GetMyAppContext()->_uiContext->GetRootWidget()->AddSink(New_HelloUI_Widget());
  return ni::eTrue;
}
