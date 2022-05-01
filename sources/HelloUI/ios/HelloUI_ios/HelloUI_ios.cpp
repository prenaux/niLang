#include <niAppLib.h>
using namespace ni;

#define HELLOUI_INCLUDE_DATA
#include "../../cpp/HelloUI_Widget.cpp"

extern app::AppContext gAppContext;

ni::Var OnAppStarted() {
  gAppContext._config.drawFPS = 2;
  gAppContext._uiContext->GetRootWidget()->AddSink(New_HelloUI_Widget());
  return ni::eTrue;
}
